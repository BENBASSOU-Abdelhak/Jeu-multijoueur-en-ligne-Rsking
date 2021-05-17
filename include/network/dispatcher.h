#ifndef INCLUDE__NETWORK__DISPATCHER_H
#define INCLUDE__NETWORK__DISPATCHER_H

#include <boost/asio/buffer.hpp>

class Session;

/* Classe Dispatcher générique */

class Dispatcher
{
    public:
	virtual size_t dispatch(uint8_t code, std::shared_ptr<Session> session, boost::asio::const_buffer const& buf,
				size_t bytes_transferred) = 0;
};

#endif
