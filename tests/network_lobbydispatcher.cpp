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
#include "../utils/ssl_injection.cpp"

using namespace boost::asio;
using wss_stream = boost::beast::websocket::stream<ssl::stream<boost::beast::tcp_stream>>;

struct Mocks {
	std::string gtag;
	GameParameters gp;
};

Mocks* cur_mock = nullptr;

/* stubs */
std::shared_ptr<Lobby> lb = std::make_shared<Lobby>(0, GameParameters{});
Lobby& LobbyPool::create_lobby(Session& s, const std::string& gamertag, const GameParameters& params)
{
	cur_mock->gtag = gamertag;
	cur_mock->gp = params;

	if (gamertag == "FAUX")
		throw LogicException{ 0x2, "DUMMY ERROR" };

	lb->join(s, gamertag);
	return *lb;
}
constexpr lobby_id_t BAD_LID = 0xffffffff;
Lobby& LobbyPool::join_lobby(lobby_id_t lid, Session& s, const std::string& gtag)
{
	if (lid == BAD_LID)
		throw LogicException{0x3, "DUMMY EXCEPTION" };

	lb->join(s, gtag);
	return *lb;
}
Lobby::Lobby(lobby_id_t id, const GameParameters&) : m_id{id}
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
void Lobby::join(Session &session, const std::string &gamertag) {
	m_list_session.push_back(session);
	m_gamertag_list.push_back(gamertag);
}
Session& Lobby::ban(const Session & se, const std::string &gamertag) {
	if (gamertag == "FAUX")
		throw LogicException{ 0x2, "DUMMY ERROR" };
	return const_cast<Session&>(se);
}
std::pair<Lobby::const_player_it, Lobby::const_player_it> Lobby::all_players() const {
	assert(m_gamertag_list.size() > 0);
	return { m_gamertag_list.cbegin(), m_gamertag_list.cend() };
}
std::pair<Lobby::it_session, Lobby::it_session> Lobby::all_sessions() {
	assert(m_list_session.size() > 0);
	return { m_list_session.begin(), m_list_session.end() };
}
	
Game& Lobby::start_game(const Session &) {
	static auto t = 0;
	++t;
	if (t == 1)
		throw LogicException{0x21, "DUMMY ERROR"};

	static Game g{GameParameters{}, *this};
	return g;
}

Game::Game(GameParameters const&, Lobby& l) : m_lobby{l}
{}
std::string const& Game::current_player() const {
	static std::string p{"PLAYER"};
	return p;
}
uint16_t Game::troop_gained() {
	return 0;
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

		boost::beast::flat_buffer buf;
		size_t read = wss_s->read(buf);

		unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
		BOOST_TEST(code == 0x11);

		lobby_id_t lb_id;
		unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, lb_id);
		BOOST_TEST(lb_id == 0);
	}

	void teardown()
	{
		cur_mock = nullptr;
		ioc->stop();
		th->join();
	}
};

BOOST_FIXTURE_TEST_SUITE(lobby, Fixture)

BOOST_AUTO_TEST_CASE(expulsion_0x15, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x15;
	std::string jwt{ "JWT_DUMMY" };

	std::vector<char> msg;
	create_buf(msg, code, jwt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x16);

	std::string motif;
	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, motif);
	BOOST_TEST(motif != "");
}

BOOST_AUTO_TEST_CASE(expulsion_0x15_toosmall, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x15;
	std::string jwt{ "JWT_DUMMY" };

	std::vector<char> msg;
	create_buf(msg, code);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(expulsion_0x15_badjwt, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x15;
	std::string jwt{ "" };

	std::vector<char> msg;
	create_buf(msg, code, jwt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(expulsion_0x15_logicexception, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x15;
	std::string jwt{ "FAUX" };

	std::vector<char> msg;
	create_buf(msg, code, jwt);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(start_0x20_logicexception, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x20;

	std::vector<char> msg;
	create_buf(msg, code);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);

	//reset
	lb = std::make_shared<Lobby>(0, GameParameters{});
}

BOOST_AUTO_TEST_CASE(start_0x20, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x20;

	std::vector<char> msg;
	create_buf(msg, code);
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x21);

	buf.clear();
	read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x30);

	std::string gtag;
	uint16_t nb_troops;
	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code, gtag, nb_troops);
	BOOST_TEST(code == 0x30);
	BOOST_TEST(gtag == "PLAYER");
	BOOST_TEST(nb_troops == 0);
}

BOOST_AUTO_TEST_CASE(start_0x20_toobig, *boost::unit_test::timeout(1))
{
	uint8_t code = 0x20;

	std::vector<char> msg;
	create_buf(msg, code, std::string{"padding"});
	boost::asio::const_buffer cbuf{ msg.data(), msg.size() };

	wss_s->write(cbuf);

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	unserialize(static_cast<raw_type>(buf.cdata().data()), read, code);
	BOOST_TEST(code == 0x0);
}

BOOST_AUTO_TEST_CASE(bad_msg_id, *boost::unit_test::timeout(1))
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

BOOST_AUTO_TEST_SUITE_END()
