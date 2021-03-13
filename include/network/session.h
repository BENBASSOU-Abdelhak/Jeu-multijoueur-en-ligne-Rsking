#ifndef INCLUDE__NETWORK__SESSION_H
#define INCLUDE__NETWORK__SESSION_H

/* tiré de https://www.boost.org/doc/libs/develop/libs/beast/example/websocket/server/async/websocket_server_async.cpp
 *
 * Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
 */

#include <memory>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <sstream>

class Dispatcher; // forward declaration

// Echoes back all received WebSocket messages
class Session : public std::enable_shared_from_this<Session>
{
    public:
	// Take ownership of the socket
	explicit Session(boost::asio::ip::tcp::socket&& socket, std::unique_ptr<Dispatcher>&& dispatcher);

	// Get on the correct executor
	void run();

	// Start the asynchronous operation
	void on_run();

	void on_accept(boost::beast::error_code ec);

	void on_handshake(boost::beast::error_code);

	void do_read();

	void do_write(std::vector<char>&& to_write);

	void change_dispatcher(std::unique_ptr<Dispatcher>&& dispatcher);

    private:
	boost::beast::websocket::stream<boost::beast::net::ssl::stream<boost::beast::tcp_stream>> wss_;
	boost::beast::flat_buffer buffer_;
	boost::beast::flat_buffer to_write_;
	std::unique_ptr<Dispatcher> dispatcher_;

	void unserialize(size_t bytes_transferred);
	void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
	void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);


};

#endif
