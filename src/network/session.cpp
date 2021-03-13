#include "network/session.h"
#include "network/fail.h"
#include "network/ssl.h"
#include "network/lpd.h"
#include "network/messages.h"

#include "logicexception.h"

#include "configuration.h"

#include <boost/log/trivial.hpp>
#include <boost/asio/dispatch.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

/* Aide :
 *
 * pour écrire :
 *
 * wss_.async_write(buffer_.data(), beast::bind_front_handler(&Session::on_write, shared_from_this()));
 *
 * pour lire :
 * wss_.async_read(buffer_, beast::bind_front_handler(&Session::on_read, shared_from_this()));
*/

Session::Session(boost::asio::ip::tcp::socket&& socket, std::unique_ptr<Dispatcher>&& dis)
	: wss_(std::move(socket), SSL_CONTEXT::get()), dispatcher_(std::move(dis))
{
}

void Session::run()
{
	// We need to be executing within a strand to perform async operations
	// on the I/O objects in this session. Although not strictly necessary
	// for single-threaded contexts, this example code is written to be
	// thread-safe by default.
	net::dispatch(wss_.get_executor(), beast::bind_front_handler(&Session::on_run, shared_from_this()));
}

void Session::on_run()
{
	beast::get_lowest_layer(wss_).expires_after(std::chrono::seconds(30));

	// Perform the SSL handshake
	wss_.next_layer().async_handshake(net::ssl::stream_base::server,
					  beast::bind_front_handler(&Session::on_handshake, shared_from_this()));
}

void Session::on_accept(beast::error_code ec)
{
	if (ec)
		return fail(ec, "accept");

	// Read a message
	do_read();
}

void Session::on_handshake(beast::error_code)
{
	// Set suggested timeout settings for the websocket
	wss_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

	// Set a decorator to change the Server of the handshake
	wss_.set_option(websocket::stream_base::decorator(
		[](websocket::response_type& res) { res.set(http::field::server, PROG_FULLNAME); }));
	// Accept the websocket handshake
	wss_.async_accept(beast::bind_front_handler(&Session::on_accept, shared_from_this()));
}

void Session::do_read()
{
	// Read a message into our buffer
	wss_.async_read(buffer_, beast::bind_front_handler(&Session::on_read, shared_from_this()));
}

void Session::do_write(std::vector<char>&& buf)
{
	decltype(auto) wd = to_write_.prepare(buf.size());
	std::copy(buf.cbegin(), buf.cend(), static_cast<char*>(wd.data()));
	wss_.async_write(wd, beast::bind_front_handler(&Session::on_write, shared_from_this()));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
	// This indicates that the session was closed
	if (ec == websocket::error::closed)
		return;

	if (ec)
		fail(ec, "read");

	if (bytes_transferred == 0) { // no message
		BOOST_LOG_TRIVIAL(warning) << "Received an empty message";
		send_error(*this, 0, "message vide");
	} else {
		BOOST_LOG_TRIVIAL(debug) << "Received: " << bytes_transferred << " bytes";
		unserialize(bytes_transferred);
		do_read();
	}
}

void Session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
	if (ec)
		return fail(ec, "write");

	BOOST_LOG_TRIVIAL(debug) << "sent " << bytes_transferred << "bytes";
	to_write_.commit(bytes_transferred);
	to_write_.consume(bytes_transferred);
}

void Session::change_dispatcher(std::unique_ptr<Dispatcher>&& dispatcher)
{
	dispatcher_ = std::move(dispatcher);
}

void Session::unserialize(size_t bytes_transferred)
{
	assert(buffer_.size() > 0 && "Buffer is empty ???");
	uint8_t code;
	decltype(auto) data = buffer_.cdata();
	const uint8_t* raw_data = static_cast<const uint8_t*>(data.data());
	code = raw_data[0];
	buffer_.consume(sizeof(uint8_t));
	data = buffer_.cdata();

	buffer_.consume(dispatcher_->dispatch(code, *this, data, bytes_transferred - 1));
}
