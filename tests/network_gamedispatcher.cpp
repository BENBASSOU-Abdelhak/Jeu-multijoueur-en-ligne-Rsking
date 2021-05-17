#define BOOST_TEST_MODULE Network
#include <boost/test/included/unit_test.hpp>

#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>
#include <boost/log/trivial.hpp>

#include <thread>
#include <chrono>
#include <memory>

#include "network/listener.h"
#include "network/lobbypooldispatcher.h"
#include "network/messages.h"
#include "network/unserialize.h"

#include "logic/lobbypool.h"
#include "logic/lobby.h"
#include "logicexception.h"

#include "jwt/jwt.h"

#include "configuration.h"
#include <openssl/ssl.h>
#include <dbms.h>
#include "../utils/ssl_injection.cpp"

using namespace boost::asio;
using wss_stream = boost::beast::websocket::stream<ssl::stream<boost::beast::tcp_stream>>;

struct Mocks {
	std::string gtag;
	GameParameters gp;
};

Mocks* cur_mock = nullptr;

/* stubs */
LobbyPool* lpp = nullptr;
Lobby* lb = new Lobby(0, GameParameters{});
LobbyPool& LobbyPool::get()
{
	lpp = new LobbyPool(0);
	return *lpp;
}
Lobby& LobbyPool::create_lobby(std::shared_ptr<Session> s, const std::string& gamertag, const GameParameters& params)
{
	cur_mock->gtag = gamertag;
	cur_mock->gp = params;

	if (gamertag == "FAUX")
		throw LogicException{ 0x2, "DUMMY ERROR" };

	lb->join(s, gamertag);
	return *lb;
}
constexpr lobby_id_t BAD_LID = 0xffffffff;
Lobby& LobbyPool::join_lobby(lobby_id_t lid, std::shared_ptr<Session> s, const std::string& gtag)
{
	if (lid == BAD_LID)
		throw LogicException{ 0x3, "DUMMY EXCEPTION" };

	lb->join(s, gtag);
	return *lb;
}
Lobby::Lobby(lobby_id_t id, const GameParameters&) : m_id{ id }
{
}
GameParameters const& Lobby::parameters() const
{
	static GameParameters gp{};
	gp.nb_players = 2;
	gp.id_map = 3;
	gp.sec_by_turn = 4;

	return gp;
}
void Lobby::join(std::shared_ptr<Session> session, const std::string& gamertag)
{
	m_list_session.push_back(session);
	m_gamertag_list.push_back(gamertag);
}
std::shared_ptr<Session> Lobby::ban(std::shared_ptr<Session> se, const std::string& gamertag)
{
	if (gamertag == "FAUX")
		throw LogicException{ 0x2, "DUMMY ERROR" };
	return se;
}
std::pair<Lobby::const_player_it, Lobby::const_player_it> Lobby::all_players() const
{
	//assert(m_gamertag_list.size() > 0);
	return { m_gamertag_list.cbegin(), m_gamertag_list.cend() };
}
Game& Lobby::start_game(const std::shared_ptr<Session>)
{
	static Game g{ GameParameters{}, *this };
	return g;
}

Game::Game(GameParameters const&, Lobby& l) : m_lobby{ l }
{
}
void Game::add_troops(const std::shared_ptr<Session>, uint16_t dst_square, uint16_t)
{
	if (dst_square == 0x0)
		throw LogicException{ 0x40, "DUMMY_ERROR" };
}
atk_result Game::attack(const std::shared_ptr<Session>, uint16_t src_square, uint16_t, uint16_t)
{
	static atk_result atr{};
	if (src_square == 0x0)
		throw LogicException{ 0x50, "DUMMY_ERROR" };
	return atr;
}
void Game::transfer(const std::shared_ptr<Session>, uint16_t, uint16_t, uint16_t nb_troops)
{
	if (nb_troops == 0x0)
		throw LogicException{ 0x62, "DUMMY_ERROR" };
}
Gamephase Game::current_phase() const
{
	static uint8_t t = 1;
	return static_cast<Gamephase>(t++ % 3);
}
uint16_t Game::troop_gained()
{
	return 0;
}
std::string const& Game::current_player() const
{
	static std::string gtag{ "GTAG" };
	return gtag;
}
uint16_t Game::time_left() const
{
	return 0xabcd;
}
std::string const& Game::winner()
{
	static std::string w{ "WINNER" };
	return w;
}
Lobby& Game::lobby() const
{
	return *lb;
}
std::string const& Game::last_dead() const
{
	static std::string str{ "LAST_DEAD" };
	return str;
}
void Game::skip(const std::shared_ptr<Session>)
{
	static int i = 0;
	++i;
	if (i >= 3)
		throw LogicException{ 0x71, "DUMMY_ERROR" };
}
size_t Game::nb_alive() const
{
	static int i = 0;
	++i;
	if (i > 3)
		return 1;
	return 2;
}
bool Game::is_finished() const
{
	return true;
}

lobby_id_t Lobby::id() const
{
	return m_id;
}
bool JWT::verify(std::string const&)
{
	return true;
}
JWT_t JWT::decode(std::string const& jwt)
{
	JWT_t ret;
	ret.name = jwt;
	return ret;
}
bool DBMS::add_game(Game&)
{
	return true;
}

struct Fixture {
	Mocks moc;
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	std::shared_ptr<ssl::context> sctx;
	std::shared_ptr<wss_stream> wss_s;
	std::shared_ptr<boost::thread> th;

	static constexpr auto port = 42424;
	static constexpr auto threads = 1;

	Fixture()
		: ioc{ std::make_shared<io_context>(threads) },
		  lp{ std::make_shared<Listener>(*ioc, ip::tcp::endpoint{ ip::address_v4::loopback(), port }) },
		  sctx{ std::make_shared<ssl::context>(ssl::context::tls_client) }, wss_s{ std::make_shared<wss_stream>(
											    *ioc, *sctx) }
	{
		boost::beast::websocket::stream_base::timeout opt{ std::chrono::seconds(30), // handshake timeout
								   std::chrono::seconds(2), // idle timeout
								   true };

		// Set the timeout options on the stream.
		wss_s->set_option(opt);
		wss_s->text(false);
		lp->run();
		th = std::make_shared<boost::thread>([&] { ioc->run() /*_for(std::chrono::milliseconds(1000))*/; });
		SSL_CTX_set_keylog_callback(sctx->native_handle(), dump_keys);

		cur_mock = &moc;
	}

	~Fixture() = default;

	void setup()
	{
		lb = new Lobby(0, GameParameters{});
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

		// création du LobbyDispatcher par simulation de client
		uint8_t code = 0x10;
		std::string jwt{ "JWT_DUMMY" };
		GameParameters gp;
		gp.nb_players = 2;
		gp.id_map = 13;
		gp.sec_by_turn = 42;

		std::vector<char> msg;
		create_buf(msg, code, gp, jwt);
		boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

		wss_s->write(cbuf);
		msg.clear();

		boost::beast::flat_buffer buf;
		size_t read = wss_s->read(buf);

		unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
		BOOST_TEST(code == 0x11);

		lobby_id_t lb_id;
		unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, lb_id);
		BOOST_TEST(lb_id == 0);

		// lancement de la partie
		code = 0x20;

		create_buf(msg, code);
		cbuf = boost::asio::const_buffer{ msg.data(), msg.size() };

		wss_s->write(cbuf);

		buf.clear();
		read = wss_s->read(buf);

		unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
		BOOST_TEST(code == 0x21);

		buf.clear();
		read = wss_s->read(buf);
		unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
		BOOST_TEST(code == 0x30);
	}

	void teardown()
	{
		delete lb;
		lb = nullptr;
		delete lpp;
		lpp = nullptr;
		cur_mock = nullptr;
		ioc->stop();
		th->join();
	}
};

BOOST_FIXTURE_TEST_SUITE(gd, Fixture)

BOOST_AUTO_TEST_CASE(placement_0x40, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x40;
	uint16_t dst = 0x27, nbt = 0xff;

	std::vector<char> msg;
	create_buf(msg, code, dst, nbt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x41);

	uint16_t rd, rn;
	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, rd, rn);
	BOOST_TEST(rd == dst);
	BOOST_TEST(rn == nbt);

	// broadcast
	buf.clear();
	read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x71);
}

BOOST_AUTO_TEST_CASE(placement_0x40_toosmall, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x40;
	uint16_t dst = 0x27;

	std::vector<char> msg;
	create_buf(msg, code, dst);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(placement_0x40_logicexception, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x40;
	uint16_t dst = 0x00, nbt = 0xff;

	std::vector<char> msg;
	create_buf(msg, code, dst, nbt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(attack_0x50, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x50;
	uint16_t src = 0x27, dst = 0x42, nbt = 0xff;

	std::vector<char> msg;
	create_buf(msg, code, src, dst, nbt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x51);

	uint16_t rs, rd, rn;
	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, rs, rd, rn);
	BOOST_TEST(rs == src);
	BOOST_TEST(rd == dst);
	//BOOST_TEST(rn == nbt); // nbt n'est pas dans le message de réponse !

	// broadcast
	buf.clear();
	read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x71);
}

BOOST_AUTO_TEST_CASE(attack_0x50_winner, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x50;
	uint16_t src = 0x27, dst = 0x42, nbt = 0xff;

	std::vector<char> msg;
	create_buf(msg, code, src, dst, nbt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x51);

	uint16_t rs, rd, rn;
	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, rs, rd, rn);
	BOOST_TEST(rs == src);
	BOOST_TEST(rd == dst);
	//BOOST_TEST(rn == nbt); // nbt n'est pas dans le message de réponse !

	// broadcast
	buf.clear();
	read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x23); // nom du mort

	// broadcast
	buf.clear();
	read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x22); // nom du gagnant
}

BOOST_AUTO_TEST_CASE(attack_0x50_toosmall, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x50;
	uint16_t src = 0x27, dst = 0x42;

	std::vector<char> msg;
	create_buf(msg, code, src, dst);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(attack_0x50_logicexception, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x50;
	uint16_t src = 0x0, dst = 0x42, nbt = 0xff;

	std::vector<char> msg;
	create_buf(msg, code, src, dst, nbt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(transfer_0x60, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x60;
	uint16_t src = 0x26, dst = 0x27, nbt = 0xff;

	std::vector<char> msg;
	create_buf(msg, code, src, dst, nbt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x61);

	uint16_t rs, rd, rn;
	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, rs, rd, rn);
	BOOST_TEST(rs == src);
	BOOST_TEST(rd == dst);
	BOOST_TEST(rn == nbt);
}

BOOST_AUTO_TEST_CASE(transfer_0x60_toosmall, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x60;
	uint16_t src = 0x26, dst = 0x27;

	std::vector<char> msg;
	create_buf(msg, code, src, dst);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(transfer_0x60_logicexception, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x60;
	uint16_t src = 0x26, dst = 0x27, nbt = 0x00;

	std::vector<char> msg;
	create_buf(msg, code, src, dst, nbt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(next_0x70_new_turn, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x70;

	std::vector<char> msg;
	create_buf(msg, code);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x71);
}

BOOST_AUTO_TEST_CASE(next_0x70_same_turn, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x70;

	std::vector<char> msg;
	create_buf(msg, code);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x71);
}

BOOST_AUTO_TEST_CASE(next_0x70_toobig, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x70;
	uint16_t dst = 0x27;

	std::vector<char> msg;
	create_buf(msg, code, dst);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(next_0x70_logicexception, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x70;

	std::vector<char> msg;
	create_buf(msg, code);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(bad_msg_id, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x10;

	std::vector<char> msg;
	create_buf(msg, code);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_SUITE_END()
