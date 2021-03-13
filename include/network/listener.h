#ifndef INCLUDE__NETWORK__LISTENER_H
#define INCLUDE__NETWORK__LISTENER_H

/* tiré de https://www.boost.org/doc/libs/develop/libs/beast/example/websocket/server/async/websocket_server_async.cpp
 *
 * Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
 */

#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener>
{
    public:
	Listener(boost::beast::net::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint);

	// Start accepting incoming connections
	void run();

    private:
	void do_accept();

	void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

	boost::beast::net::io_context& ioc_;
	boost::asio::ip::tcp::acceptor acceptor_;
};

#endif
