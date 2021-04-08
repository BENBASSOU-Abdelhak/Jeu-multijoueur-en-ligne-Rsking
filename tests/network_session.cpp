/*
 * Test if Session is working as expected
 */
#define BOOST_TEST_MODULE Network
#include <boost/test/included/unit_test.hpp>

#include "network/listener.h"
#include "network/session.h"
#include "network/dispatcher.h"
#include "network/lobbypooldispatcher.h"
#include "logic/lobbypool.h"

#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>
#include <boost/log/trivial.hpp>

#include "configuration.h"
#include <openssl/ssl.h>
#include "../utils/ssl_injection.cpp"

#include <thread>
#include <chrono>

using namespace boost::asio;
using wss_stream = boost::beast::websocket::stream<ssl::stream<boost::beast::tcp_stream>>;

#include <memory>

constexpr unsigned short PORT = 42424;

class Mock_dispatcher : public Dispatcher
{
    public:
	size_t dispatch(uint8_t code, Session& session, boost::asio::const_buffer const& buf,
			size_t bytes_transferred) override
	{
		uint16_t c = code;
		BOOST_LOG_TRIVIAL(debug)
			<< "received message of " << bytes_transferred << "bytes with code 0x" << std::hex << c;
		buf_ = &buf;
		code_ = code;
		bytes_transferred_ = bytes_transferred;
		boost::ignore_unused(session);
		return bytes_transferred;
	}
	static boost::asio::const_buffer const* buf_;
	static uint8_t code_;
	static size_t bytes_transferred_;
};
boost::asio::const_buffer const* Mock_dispatcher::buf_ = nullptr;
uint8_t Mock_dispatcher::code_ = 0;
size_t Mock_dispatcher::bytes_transferred_ = 0;

// override
size_t LobbyPoolDispatcher::dispatch(uint8_t code, Session& session, boost::asio::const_buffer const& buf,
				     size_t bytes_transferred)
{
	uint16_t c = code;
	BOOST_LOG_TRIVIAL(debug)
		<< "received message of " << bytes_transferred << "bytes with code 0x" << std::hex << c;
	session.change_dispatcher(std::make_unique<Mock_dispatcher>());
	boost::ignore_unused(buf);
	return bytes_transferred;
}

void setup(std::shared_ptr<io_context>& io, std::shared_ptr<Listener>& lp, std::shared_ptr<ssl::context>& sctx,
	   std::shared_ptr<wss_stream>& wss_s, std::shared_ptr<boost::beast::tcp_stream>& tcps)
{
	auto const address = ip::address_v4::loopback();
	auto const port = PORT;
	auto const threads = 1;

	// The io_context is required for all I/O
	io = std::make_shared<io_context>(threads);
	tcps = std::make_shared<boost::beast::tcp_stream>(*io);

	// Create and launch a listening port
	lp = std::make_shared<Listener>(*io, ip::tcp::endpoint{ address, port });
	//lp->run();
	//ioc.run();

	sctx = std::make_shared<ssl::context>(ssl::context::tls_client);
	wss_s = std::make_shared<wss_stream>(*io, *sctx);
	wss_s->text(false);
	SSL_CTX_set_keylog_callback(sctx->native_handle(), dump_keys);
}

BOOST_AUTO_TEST_CASE(constructor, *boost::unit_test::timeout(1))
{
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	std::shared_ptr<ssl::context> sctx;
	std::shared_ptr<wss_stream> wss_s;
	std::shared_ptr<boost::beast::tcp_stream> tcps;

	setup(ioc, lp, sctx, wss_s, tcps);

	lp->run();
	boost::thread t{ [&ioc]() { ioc->run(); } };

	t.interrupt();
	t.try_join_for(boost::chrono::milliseconds(50));
}

BOOST_AUTO_TEST_CASE(wss_handshake, *boost::unit_test::timeout(1))
{
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	std::shared_ptr<ssl::context> sctx;
	std::shared_ptr<wss_stream> wss_s;
	std::shared_ptr<boost::beast::tcp_stream> tcps;

	setup(ioc, lp, sctx, wss_s, tcps);

	lp->run();
	boost::thread t{ [&ioc]() { ioc->run_for(std::chrono::milliseconds(200)); } };

	wss_s->next_layer().next_layer().connect(ip::tcp::endpoint{ ip::address_v4::loopback(), PORT });

	wss_s->next_layer().handshake(ssl::stream_base::client);

	// Set a decorator to change the User-Agent of the handshake
	wss_s->set_option(
		boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req) {
			req.set(boost::beast::http::field::user_agent,
				std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client");
		}));

	wss_s->handshake("risking.test.com", "/");

	t.try_join_for(boost::chrono::milliseconds(500));
}

BOOST_AUTO_TEST_CASE(wss_exchange, *boost::unit_test::timeout(1))
{
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	std::shared_ptr<ssl::context> sctx;
	std::shared_ptr<wss_stream> wss_s;
	std::shared_ptr<boost::beast::tcp_stream> tcps;

	setup(ioc, lp, sctx, wss_s, tcps);

	lp->run();
	boost::thread t{ [&ioc]() { ioc->run_for(std::chrono::milliseconds(200)); } };

	wss_s->next_layer().next_layer().connect(ip::tcp::endpoint{ ip::address_v4::loopback(), PORT });

	wss_s->next_layer().handshake(ssl::stream_base::client);

	// Set a decorator to change the User-Agent of the handshake
	wss_s->set_option(
		boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req) {
			req.set(boost::beast::http::field::user_agent,
				std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client");
		}));

	wss_s->handshake("risking.test.com", "/");

	// Message
	uint8_t code = 0x21;

	std::vector<unsigned char> vect{ { code, 0x00, 0x01, 0x02 } };
	boost::asio::mutable_buffer data(vect.data(), vect.size());

	wss_s->write(data); // pour changer le dispatcher
	wss_s->write(data); // pour les tests avec le MOCK

	t.try_join_for(boost::chrono::milliseconds(500));

	// tests des valeurs
	BOOST_TEST(Mock_dispatcher::code_ == code);
	BOOST_TEST(Mock_dispatcher::bytes_transferred_ == vect.size() - 1);

	const unsigned char* raw = static_cast<const unsigned char*>(Mock_dispatcher::buf_->data());
	for (size_t i = 0; i < vect.size() - 1; ++i) {
		BOOST_TEST(raw[i] == vect[i + 1]);
	}
}

BOOST_AUTO_TEST_CASE(wss_empty_message, *boost::unit_test::timeout(1))
{
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	std::shared_ptr<ssl::context> sctx;
	std::shared_ptr<wss_stream> wss_s;
	std::shared_ptr<boost::beast::tcp_stream> tcps;

	setup(ioc, lp, sctx, wss_s, tcps);

	lp->run();
	boost::thread t{ [&ioc]() { ioc->run_for(std::chrono::milliseconds(200)); } };

	wss_s->next_layer().next_layer().connect(ip::tcp::endpoint{ ip::address_v4::loopback(), PORT });

	wss_s->next_layer().handshake(ssl::stream_base::client);

	// Set a decorator to change the User-Agent of the handshake
	wss_s->set_option(
		boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req) {
			req.set(boost::beast::http::field::user_agent,
				std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client");
		}));

	wss_s->handshake("risking.test.com", "/");

	// Message
	std::vector<unsigned char> vect{};
	boost::asio::mutable_buffer data(vect.data(), vect.size());

	wss_s->write(data); // pour recevoir message vide

	boost::beast::flat_buffer buf;
	size_t read = wss_s->read(buf);

	t.try_join_for(boost::chrono::milliseconds(500));

	BOOST_TEST(read == 15);

	// unserialize
	auto dt = buf.cdata();
	const unsigned char* raw = static_cast<const unsigned char*>(dt.data());
	uint8_t code = raw[0];
	uint8_t subcode = raw[1];
	std::string msg{ raw + 2, raw + read - 1 }; // pas de \0 en fin

	// tests
	BOOST_TEST(code == 0);
	BOOST_TEST(subcode == 0);
	BOOST_TEST(msg == std::string{ "message vide" });

	std::vector<unsigned char> expected{
		0x0, 0x0, 'm', 'e', 's', 's', 'a', 'g', 'e', ' ', 'v', 'i', 'd', 'e', '\0'
	};
	std::vector<unsigned char> got;
	std::copy(raw, raw + read, std::back_inserter(got));
	BOOST_TEST(expected == got, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(session_cmp, *boost::unit_test::timeout(1)) {
	io_context ctx{1};
	auto s1 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ctx}, std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));
	auto s2 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ctx}, std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));

	BOOST_CHECK(*s1 != *s2);
}
