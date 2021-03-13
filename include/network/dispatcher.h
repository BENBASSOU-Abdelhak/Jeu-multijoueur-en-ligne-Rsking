#ifndef INCLUDE__NETWORK__DISPATCHER_H
#define INCLUDE__NETWORK__DISPATCHER_H

#include "network/session.h"

/* Classe Dispatcher générique */

class Dispatcher {
	public:
	virtual size_t dispatch(uint8_t code, Session& session, boost::asio::const_buffer& buf, size_t bytes_transferred)=0;
};

#endif
