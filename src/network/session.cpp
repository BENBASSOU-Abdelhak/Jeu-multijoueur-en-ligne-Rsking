#include "network/session.h"
#include "network/fail.h"
#include "network/ssl.h"
#include "network/lobbypooldispatcher.h"
#include "network/messages.h"

#include "logicexception.h"

#include "configuration.h"

#include <boost/log/trivial.hpp>
#include <boost/asio/dispatch.hpp>

#include <random>
#include <chrono>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

session_id_t gen_session_id();

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
	: internal_id{ gen_session_id() }, wss_(std::move(socket), SslContext::get()), dispatcher_(std::move(dis))
{
	wss_.text(false);
}

Session::~Session()
{
	//TODO: prévenir que le client a quitté
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
	// Turn off the timeout on the tcp_stream, because
	// the websocket stream has its own timeout system.
	beast::get_lowest_layer(wss_).expires_never();
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

void Session::send(std::vector<char> const& data)
{
	// Post our work to the strand, this ensures
	// that the members of `this` will not be
	// accessed concurrently.
	auto pt = std::make_shared<std::remove_const_t<std::remove_reference_t<decltype(data)>>>(data);
	net::post(wss_.get_executor(), beast::bind_front_handler(&Session::on_send, shared_from_this(), std::move(pt)));
}

void Session::on_send(std::shared_ptr<std::vector<char>> const& buf)
{
	// Always add to queue
	queue_.push_back(buf);

	// Are we already writing?
	if (queue_.size() > 1)
		return;

	// We are not currently writing, so send this immediately
	wss_.async_write(net::buffer(*queue_.front()),
			 beast::bind_front_handler(&Session::on_write, shared_from_this()));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
	// This indicates that the session was closed
	if (ec == websocket::error::closed) {
		BOOST_LOG_TRIVIAL(debug) << "WSS session closed";
		return;
	}

	if (ec && ec.value() != boost::system::errc::operation_canceled)
		fail(ec, "read");

	if (bytes_transferred == 0) { // no message
#ifdef EMPTY_MESSAGE_IS_ERROR
		BOOST_LOG_TRIVIAL(warning) << "Received an empty message";
		send_error(*this, 0, "message vide");
#endif
	} else {

		BOOST_LOG_TRIVIAL(debug) << "Received: " << bytes_transferred << " bytes";
		unserialize(bytes_transferred);
	}
	do_read();
}

void Session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
	if (ec)
		return fail(ec, "write");

	// Remove the string from the queue
	queue_.erase(queue_.begin());
	BOOST_LOG_TRIVIAL(debug) << "sent " << bytes_transferred << "bytes";

	// Send the next message if any
	if (!queue_.empty())
		wss_.async_write(net::buffer(*queue_.front()),
				 beast::bind_front_handler(&Session::on_write, shared_from_this()));
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

// lobby_id_t
session_id_t gen_session_id()
{
	// TODO: génération globale et sécurisée.
	static std::mt19937_64 gen{ static_cast<unsigned>(
		std::chrono::system_clock::now().time_since_epoch().count()) };
	static std::uniform_int_distribution<session_id_t> dis;
	return dis(gen);
}

bool operator==(Session const& lhs, Session const& rhs)
{
	return lhs.internal_id == rhs.internal_id;
}

bool operator!=(Session const& lhs, Session const& rhs)
{
	return !(lhs == rhs);
}
