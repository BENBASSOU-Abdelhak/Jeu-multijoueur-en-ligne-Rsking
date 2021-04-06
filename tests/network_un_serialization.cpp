#define BOOST_TEST_MODULE Network
#include <boost/test/included/unit_test.hpp>

#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>
#include <boost/log/trivial.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <thread>
#include <chrono>
#include <memory>

#include "network/listener.h"
#include "network/lobbypooldispatcher.h"
#include "network/unserialize.h"
#include "network/messages.h"

#include "logic/map.h"
#include "logic/atk_result.h"

#include "configuration.h"
#include <openssl/ssl.h>
#include "../utils/ssl_injection.cpp"

using namespace boost::asio;
using wss_stream = boost::beast::websocket::stream<ssl::stream<boost::beast::tcp_stream>>;

/* Mocks */
struct Mocks {
	uint8_t code;
	std::vector<char> data;
};

Mocks* cur_mock = nullptr;
boost::interprocess::interprocess_semaphore* cur_sem = nullptr;

// override
size_t LobbyPoolDispatcher::dispatch(uint8_t code, Session&, boost::asio::const_buffer const& buf,
				     size_t bytes_transferred)
{
	BOOST_LOG_TRIVIAL(debug) << "Entering mock";
	assert(cur_mock);
	cur_mock->code = code;
	std::copy(static_cast<const char*>(buf.data()), static_cast<const char*>(buf.data()) + buf.size(),
		  std::back_inserter(cur_mock->data));
	boost::ignore_unused(buf);
	assert(cur_sem);
	cur_sem->post();
	return bytes_transferred;
}

std::vector<std::string> stubv{ "Player1", "Player2", "Matheo" };

std::pair<Lobby::const_player_it, Lobby::const_player_it> Lobby::all_players() const
{
	return { std::cbegin(stubv), std::cend(stubv) };
}
Game::Game(GameParameters const&, Lobby& lb) : m_lobby(lb)
{
}
Lobby& Game::lobby() const
{
	return this->m_lobby;
}
Map const& Game::get_map() const
{
	static Map m;
	return m;
}
Map::Map() : m_info_square{}
{
	info_square a, b;
	a.player_id = 0;
	a.nb_troops = 0x42;
	a.id_region = 0x72;
	b.player_id = 0x1;
	b.nb_troops = 0x13;
	b.id_region = 0x8;

	m_info_square.push_back(a);
	m_info_square.push_back(b);
}

struct Fixture {
	boost::interprocess::interprocess_semaphore ok;
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	std::shared_ptr<ssl::context> sctx;
	std::shared_ptr<wss_stream> wss_s;
	std::shared_ptr<boost::thread> th;
	Mocks mocks;

	static constexpr auto port = 42424;
	static constexpr auto threads = 1;

	Fixture()
		: ok{ 0 }, ioc{ std::make_shared<io_context>(threads) },
		  lp{ std::make_shared<Listener>(*ioc, ip::tcp::endpoint{ ip::address_v4::loopback(), port }) },
		  sctx{ std::make_shared<ssl::context>(ssl::context::tls_client) }, wss_s{ std::make_shared<wss_stream>(
											    *ioc, *sctx) }
	{
		cur_mock = &mocks;
		cur_sem = &ok;

		boost::beast::websocket::stream_base::timeout opt{ std::chrono::seconds(30), // handshake timeout
								   std::chrono::seconds(2), // idle timeout
								   true };

		// Set the timeout options on the stream.
		wss_s->set_option(opt);
		wss_s->text(false);
		lp->run();
		th = std::make_shared<boost::thread>([&] { ioc->run() /*_for(std::chrono::milliseconds(1000))*/; });
		SSL_CTX_set_keylog_callback(sctx->native_handle(), dump_keys);
	}

	~Fixture() = default;

	void setup()
	{
		// connect (TCP)
		wss_s->next_layer().next_layer().connect(ip::tcp::endpoint{ ip::address_v4::loopback(), port });

		// connect (SSL handshake)
		wss_s->next_layer().handshake(ssl::stream_base::client);

		// Set a decorator to change the User-Agent of the handshake
		wss_s->set_option(
			boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req) {
				req.set(boost::beast::http::field::user_agent,
					std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client");
			}));

		// connect (WSS handshake)
		wss_s->handshake("risking.test.com", "/");
	}

	void teardown()
	{
		cur_mock = nullptr;
		cur_sem = nullptr;
		ioc->stop();
		th->join();
	}
};

BOOST_FIXTURE_TEST_SUITE(wss_serialization, Fixture)

BOOST_AUTO_TEST_CASE(basic_type, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	uint8_t val = 0x16;
	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, val);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);
	uint8_t rcv = 0;
	unserialize(mocks.data, mocks.data.size(), rcv);
	BOOST_TEST(val == rcv);
}

BOOST_AUTO_TEST_CASE(endianness_2_bytes, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	uint16_t val = 0x1234;
	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, val);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);
	decltype(val) rcv = 0;
	unserialize(mocks.data, mocks.data.size(), rcv);
	BOOST_TEST(val == rcv);
}

BOOST_AUTO_TEST_CASE(endianness_4_bytes, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	uint32_t val = 0x12345678;
	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, val);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);
	decltype(val) rcv = 0;
	unserialize(mocks.data, mocks.data.size(), rcv);
	BOOST_TEST(val == rcv);
}

BOOST_AUTO_TEST_CASE(endianness_8_bytes, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	uint64_t val = 0x0123456789abcdef;
	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, val);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);
	decltype(val) rcv = 0;
	unserialize(mocks.data, mocks.data.size(), rcv);
	BOOST_TEST(val == rcv);
}

BOOST_AUTO_TEST_CASE(string, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	std::string val{ "test sur une longue chaine de caracteres" };
	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, val);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);
	decltype(val) rcv{};
	unserialize(mocks.data, mocks.data.size(), rcv);
	BOOST_TEST(val == rcv);
	BOOST_TEST(val.size() == rcv.size());
}

BOOST_AUTO_TEST_CASE(lobby, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	GameParameters gp{};
	Lobby lb{ 0, gp };
	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, lb);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);
	std::vector<std::string> rcv{ 3 };
	unserialize(mocks.data, mocks.data.size(), rcv[0], rcv[1], rcv[2]);
	BOOST_TEST(stubv == rcv, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(game_and_map, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	GameParameters gp{};
	Lobby lb{ 0, gp };
	Game g{ gp, lb };
	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, g);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);

	// gamertags
	std::vector<std::string> rcv{ 3 };
	// info_square
	std::vector<info_square> rcvs{ 2 };

	unserialize(mocks.data, mocks.data.size(), rcv[0], rcv[1], rcv[2], rcvs[0].player_id, rcvs[0].nb_troops,
		    rcvs[1].player_id, rcvs[1].nb_troops);

	BOOST_TEST(stubv == rcv, boost::test_tools::per_element());
	BOOST_TEST(rcvs[0].player_id == 0);
	BOOST_TEST(rcvs[1].player_id == 1);
	BOOST_TEST(rcvs[0].nb_troops == 0x42);
	BOOST_TEST(rcvs[1].nb_troops == 0x13);
}

BOOST_AUTO_TEST_CASE(serialization_atk_result, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x42;
	atk_result res;
	res.square_conquered = true;
	res.nb_lost_troops_from_attacker = 0x42;
	res.nb_lost_troops_from_defender = 0x69;
	res.attackers_dice = { 0x6, 0x6 };
	res.defenders_dice = { 0x1 };

	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, res);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);
	decltype(res) rcv;
	rcv.attackers_dice.resize(res.attackers_dice.size());
	rcv.defenders_dice.resize(res.defenders_dice.size());
	uint16_t nb_atk, nb_def;
	unserialize(mocks.data, mocks.data.size(), rcv.square_conquered, rcv.nb_lost_troops_from_attacker,
		    rcv.nb_lost_troops_from_defender, nb_atk, rcv.attackers_dice[0], rcv.attackers_dice[1], nb_def,
		    rcv.defenders_dice[0]);

	BOOST_TEST(res.square_conquered == rcv.square_conquered);
	BOOST_TEST(res.nb_lost_troops_from_attacker == rcv.nb_lost_troops_from_attacker);
	BOOST_TEST(res.nb_lost_troops_from_defender == rcv.nb_lost_troops_from_defender);
	BOOST_TEST(nb_atk == res.attackers_dice.size());
	BOOST_TEST(nb_def == res.defenders_dice.size());
	BOOST_TEST(res.attackers_dice == rcv.attackers_dice, boost::test_tools::per_element());
	BOOST_TEST(res.defenders_dice == rcv.defenders_dice, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(serialization_gameparameters)
{
	uint8_t code = 0x42;
	GameParameters res;
	res.id_map = 0x13;
	res.nb_players = 0x16;
	res.sec_by_turn = 0x32;

	std::vector<char> msg{ /* 't', 'e', 's', 't', '\0' */ };
	create_buf(msg, code, res);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);
	boost::beast::flat_buffer buf;

	ok.wait();

	BOOST_TEST(code == mocks.code);

	decltype(res) rcv;
	unserialize(mocks.data, mocks.data.size(), rcv.nb_players, rcv.id_map, rcv.sec_by_turn);

	BOOST_TEST(res.id_map == rcv.id_map);
	BOOST_TEST(res.nb_players == rcv.nb_players);
	BOOST_TEST(res.sec_by_turn == rcv.sec_by_turn);
}

BOOST_AUTO_TEST_SUITE_END()
