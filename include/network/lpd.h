#ifndef INCLUDE__NETWORK__LPD_H
#define INCLUDE__NETWORK__LPD_H

#include "network/dispatcher.h"

/* Classe LobbyPoolDispatcher */

class LobbyPoolDispatcher : public Dispatcher {
	public:
	size_t dispatch(uint8_t code, Session& session, boost::asio::const_buffer& buf, size_t bytes_transferred) override;
};

#endif
