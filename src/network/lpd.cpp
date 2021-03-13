#include <network/lpd.h>

size_t LobbyPoolDispatcher::dispatch(uint8_t code, Session &session, boost::asio::const_buffer &buf, size_t bytes_transferred) {
	//TODO
	boost::ignore_unused(code, session, buf, bytes_transferred);
	return 0;
}
