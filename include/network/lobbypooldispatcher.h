#ifndef INCLUDE__NETWORK__LPD_H
#define INCLUDE__NETWORK__LPD_H

#include "network/dispatcher.h"

/* Classe LobbyPoolDispatcher */

struct LobbyPool;

class LobbyPoolDispatcher : public Dispatcher
{
    public:
	LobbyPoolDispatcher(LobbyPool& lbp);

	size_t dispatch(uint8_t code, std::shared_ptr<Session> session, boost::asio::const_buffer const& buf,
			size_t bytes_transferred) override;

    private:
	size_t public_lobby(std::shared_ptr<Session> session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t create_lobby(std::shared_ptr<Session> session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	size_t join_lobby(std::shared_ptr<Session> session, boost::asio::const_buffer const& buf, size_t bytes_transferred);
	LobbyPool& lbp_;
};

#endif
