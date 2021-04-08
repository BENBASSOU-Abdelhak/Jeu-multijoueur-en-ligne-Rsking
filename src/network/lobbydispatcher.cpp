#include "network/lobbydispatcher.h"

#include "network/messages.h"
#include "network/unserialize.h"
#include "network/lobbypooldispatcher.h"
#include "network/gamedispatcher.h"

#include "logicexception.h"

#include "logic/lobby.h"
#include "logic/lobbypool.h"

#include <boost/log/trivial.hpp>

LobbyDispatcher::LobbyDispatcher(Lobby& lb) : lb_(lb)
{
}

size_t LobbyDispatcher::dispatch(uint8_t code, Session& session, boost::asio::const_buffer const& buf,
				 size_t bytes_transferred)
{
	switch (code) {
	case 0x15: // Expulsion de joueur
		return kick_player(session, buf, bytes_transferred);
		break;
	case 0x20: // Début de partie
		return start_game(session, buf, bytes_transferred);
		break;
	default:
		BOOST_LOG_TRIVIAL(warning)
			<< "Bad message id received in LobbyDispatcher : " << std::hex << static_cast<uint16_t>(code);
		send_error(session, 0x0, "ID non accepté par LobbyDispatcher");
		return bytes_transferred;
	}
}

size_t LobbyDispatcher::kick_player(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred)
{
	if (bytes_transferred <= 1) {
		BOOST_LOG_TRIVIAL(warning) << "Message received with code 0x15 is too small";
		send_error(session, 0x0, "Message with code 0x15 is too small");
		return bytes_transferred;
	}

	std::string gtag;
	size_t read = unserialize(buf, bytes_transferred, gtag);

	try {
		Session& kicked = lb_.ban(session, gtag);
		send_message(kicked, static_cast<uint8_t>(0x16), "Vous avez été kick du salon");

		broadcast(lb_, static_cast<uint8_t>(0x14), lb_);
		kicked.change_dispatcher(std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));

	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in Lobby::ban";
		//TODO: ban
		BOOST_LOG_TRIVIAL(fatal) << "TODO: ban";
		send_error(session, e.subcode(), e.what());
	}

	return read;
}

size_t LobbyDispatcher::start_game(Session& session, boost::asio::const_buffer const&, size_t bytes_transferred)
{
	if (bytes_transferred > 0) {
		BOOST_LOG_TRIVIAL(warning) << "Message with code 0x20 should be empty";
		send_error(session, 0x0, "Message with code 0x20 should be empty");
		return bytes_transferred;
	}

	try {
		Game& new_game = lb_.start_game(session);
		broadcast(lb_, static_cast<uint8_t>(0x21), new_game);

		session.change_dispatcher(std::make_unique<GameDispatcher>(new_game));
	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in Lobby::start_game";
		//TODO: ban
		BOOST_LOG_TRIVIAL(fatal) << "TODO: ban";
		send_error(session, e.subcode(), e.what());
	}

	return bytes_transferred;
}
