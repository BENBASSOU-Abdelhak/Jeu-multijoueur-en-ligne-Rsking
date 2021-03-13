#include "network/listener.h"
#include "network/session.h"
#include "network/ssl.h"
#include "network/fail.h"
#include "network/lpd.h"


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/log/trivial.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

Listener::Listener(net::io_context& ioc, tcp::endpoint endpoint) : ioc_(ioc), acceptor_(ioc)
{
	beast::error_code ec;

	// Open the acceptor
	acceptor_.open(endpoint.protocol(), ec);
	if (ec) {
		fail(ec, "open");
		return;
	}

	// Allow address reuse
	acceptor_.set_option(net::socket_base::reuse_address(true), ec);
	if (ec) {
		fail(ec, "set_option");
		return;
	}

	// Bind to the server address
	acceptor_.bind(endpoint, ec);
	if (ec) {
		fail(ec, "bind");
		return;
	}

	// Start listening for connections
	acceptor_.listen(net::socket_base::max_listen_connections, ec);
	if (ec) {
		fail(ec, "listen");
		return;
	}
}

void Listener::run()
{
	do_accept();
}

void Listener::do_accept()
{
	// The new connection gets its own strand
	acceptor_.async_accept(net::make_strand(ioc_),
			       beast::bind_front_handler(&Listener::on_accept, shared_from_this()));
}

void Listener::on_accept(beast::error_code ec, tcp::socket socket)
{
	if (ec) {
		fail(ec, "accept");
	} else {
		BOOST_LOG_TRIVIAL(debug) << "client accepted";
		// Create LobbyPoolDispatcher
		std::unique_ptr<Dispatcher> lpd = std::make_unique<LobbyPoolDispatcher>();
		// Create the session and run it
		std::make_shared<Session>(std::move(socket), std::move(lpd))->run();
	}

	// Accept another connection
	do_accept();
}
