#ifndef INCLUDE__NETWORK__GD_H
#define INCLUDE__NETWORK__GD_H

#include "network/dispatcher.h"

/* Classe LobbyPoolDispatcher */

class Game;

class GameDispatcher : public Dispatcher
{
    public:
	GameDispatcher(Game& game);
	size_t dispatch(uint8_t code, Session& session, boost::asio::const_buffer const& buf,
			size_t bytes_transferred) override;

    private:
	Game& game_;

	size_t place_troops(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t attack(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t transfer(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t next_phase(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
};

#endif
