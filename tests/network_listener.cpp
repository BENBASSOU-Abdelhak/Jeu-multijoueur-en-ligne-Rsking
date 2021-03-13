/*
 * Test if Listener is working as expected
 */
#define BOOST_TEST_MODULE Network
#include <boost/test/included/unit_test.hpp>


#include <boost/thread.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <thread>

#include "network/listener.h"

using namespace boost::asio;

constexpr unsigned short PORT = 42424;

void setup(std::shared_ptr<io_context>& io, std::shared_ptr<Listener>& lp) {
    auto const address = ip::address_v4::loopback();
    auto const port = PORT;
    auto const threads = 1;

    // The io_context is required for all I/O
    io = std::make_shared<io_context>(threads);

    // Create and launch a listening port
    lp = std::make_shared<Listener>(*io, ip::tcp::endpoint{address, port});
    //lp->run();
    //ioc.run();
}

BOOST_AUTO_TEST_CASE(constructor, * boost::unit_test::timeout(1)) {
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	setup(ioc, lp);
	BOOST_TEST(lp != nullptr);
}

BOOST_AUTO_TEST_CASE(accept_connect, * boost::unit_test::timeout(1)) {
	std::shared_ptr<io_context> ioc;
	std::shared_ptr<Listener> lp;
	setup(ioc, lp);

	lp->run();
	boost::thread t{[&ioc]() {ioc->run_one()/*(std::chrono::seconds{100})*/;}};

	ip::tcp::socket sock{*ioc};
	boost::system::error_code ec;
	sock.connect(ip::tcp::endpoint{ip::address_v4::loopback(), PORT}, ec);
	BOOST_TEST(!ec);

	t.try_join_for(boost::chrono::milliseconds(500));
}

