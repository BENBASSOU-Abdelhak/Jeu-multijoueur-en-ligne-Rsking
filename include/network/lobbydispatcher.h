#ifndef INCLUDE__NETWORK__PD_H
#define INCLUDE__NETWORK__PD_H

#include "network/dispatcher.h"

/* Classe LobbyPoolDispatcher */

struct Lobby;

class LobbyDispatcher : public Dispatcher
{
    public:
	LobbyDispatcher(Lobby& lb);
	size_t dispatch(uint8_t code, Session& session, boost::asio::const_buffer const& buf,
			size_t bytes_transferred) override;

    private:
	size_t kick_player(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t start_game(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred);

	Lobby& lb_;
};

#endif
