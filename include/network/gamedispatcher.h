#ifndef INCLUDE__NETWORK__GD_H
#define INCLUDE__NETWORK__GD_H

#include "network/dispatcher.h"

/* Classe LobbyPoolDispatcher */

class Game;

class GameDispatcher : public Dispatcher
{
    public:
	GameDispatcher(Game& game);
	size_t dispatch(uint8_t code, std::shared_ptr<Session> session, boost::asio::const_buffer const& buf,
			size_t bytes_transferred) override;

    private:
	Game& game_;

	size_t place_troops(std::shared_ptr<Session> session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t attack(std::shared_ptr<Session> session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t transfer(std::shared_ptr<Session> session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t next_phase(std::shared_ptr<Session> session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
};

#endif
