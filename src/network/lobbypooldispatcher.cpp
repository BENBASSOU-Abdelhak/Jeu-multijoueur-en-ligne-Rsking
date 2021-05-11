#include "network/lobbypooldispatcher.h"
#include "network/session.h"
#include "network/messages.h"
#include "network/unserialize.h"
#include "network/lobbydispatcher.h"

#include "logic/gameparameters.h"
#include "logic/lobbypool.h"
#include "logic/lobby.h"

#include "jwt/jwt.h"

#include "logicexception.h"

#include <boost/log/trivial.hpp>

LobbyPoolDispatcher::LobbyPoolDispatcher(LobbyPool& lbp) : lbp_(lbp)
{
}

// pour les tests
__attribute__((weak)) size_t LobbyPoolDispatcher::dispatch(uint8_t code, Session& session,
							   boost::asio::const_buffer const& buf,
							   size_t bytes_transferred)
{
	switch (code) {
	case 0x10: // création Lobby
		return create_lobby(session, buf, bytes_transferred);
		break;
	case 0x12:
		return join_lobby(session, buf, bytes_transferred);
		break;
	case 0x18:
		return public_lobby(session, buf, bytes_transferred);
		break;
	default:
		BOOST_LOG_TRIVIAL(warning) << "Bad message id received in LobbyPoolDispatcher : " << std::hex
					   << static_cast<uint16_t>(code);
		send_error(session, 0x0, "ID non accepté par LobbyPoolDispatcher");
		return bytes_transferred;
	}
}

size_t LobbyPoolDispatcher::create_lobby(Session& session, boost::asio::const_buffer const& buf,
					 size_t bytes_transferred)
{
	if (bytes_transferred < sizeof(GameParameters)) {
		BOOST_LOG_TRIVIAL(warning) << "Message received with code 0x10 is too small";
		send_error(session, 0x0, "Message with code 0x10 is too small");
		return bytes_transferred;
	}

	struct GameParameters gp;
	size_t read = unserialize(buf, bytes_transferred, gp.nb_players, gp.id_map, gp.sec_by_turn);

	std::string jwt;
	read += unserialize(static_cast<const char*>(buf.data()) + read, bytes_transferred - read, jwt);

	if (jwt.size() == 0 || !JWT::get().verify(jwt)) {
		BOOST_LOG_TRIVIAL(warning) << "Invalid JWT received: " << jwt;
		send_error(session, 0x0, "JWT is invalid.");
		return read;
	}

	JWT_t token = JWT::get().decode(jwt);

	try {
		Lobby& lobby = lbp_.create_lobby(session, token.name, gp);
		session.change_dispatcher(std::make_unique<LobbyDispatcher>(lobby));
		send_message(session, static_cast<uint8_t>(0x11), lobby.id());
	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in LobbyPool::create_lobby";
		send_error(session, e.subcode(), e.what());
	}

	return read;
}

size_t LobbyPoolDispatcher::join_lobby(Session& session, boost::asio::const_buffer const& buf,
					 size_t bytes_transferred)
{
	if (bytes_transferred < sizeof(lobby_id_t)) {
		BOOST_LOG_TRIVIAL(warning) << "Message received with code 0x12 is too small";
		send_error(session, 0x0, "Message with code 0x12 is too small");
		return bytes_transferred;
	}

	lobby_id_t lid;
	size_t read = unserialize(buf, bytes_transferred, lid);

	std::string jwt;
	read += unserialize(static_cast<const char*>(buf.data()) + read, bytes_transferred - read, jwt);

	if (jwt.size() == 0 || !JWT::get().verify(jwt)) {
		BOOST_LOG_TRIVIAL(warning) << "Invalid JWT received: " << jwt;
		send_error(session, 0x0, "JWT is invalid.");
		return read;
	}

	JWT_t token = JWT::get().decode(jwt);

	try {
		Lobby& lobby = lbp_.join_lobby(lid, session, token.name);
		session.change_dispatcher(std::make_unique<LobbyDispatcher>(lobby));
		GameParameters const& gp = lobby.parameters();
		send_message(session, static_cast<uint8_t>(0x13), gp);

		broadcast(lobby, static_cast<uint8_t>(0x14), lobby);

	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in LobbyPool::join_lobby";
		send_error(session, e.subcode(), e.what());
	}

	return read;
}

size_t LobbyPoolDispatcher::public_lobby(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred)
{
	if (bytes_transferred < 1) {
		BOOST_LOG_TRIVIAL(warning) << "Message received with code 0x16 is too small";
		send_error(session, 0x0, "Message with code 0x16 is too small");
		return bytes_transferred;
	}

	std::string jwt;
	size_t read = unserialize(static_cast<const char*>(buf.data()), bytes_transferred, jwt);

	if (jwt.size() == 0 || !JWT::get().verify(jwt)) {
		BOOST_LOG_TRIVIAL(warning) << "Invalid JWT received: " << jwt;
		send_error(session, 0x0, "JWT is invalid.");
		return read;
	}

	JWT_t token = JWT::get().decode(jwt);

	auto id = lbp_.lobby_dispo(session, token.name);
	send_message(session, static_cast<uint8_t>(0x19), id);

	return read;
}
