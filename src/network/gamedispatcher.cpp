#include "network/gamedispatcher.h"
#include "network/lobbydispatcher.h"

#include "network/messages.h"
#include "network/unserialize.h"

#include "logic/game.h"

#include "logicexception.h"

#include <boost/log/trivial.hpp>
#include <dbms.h>

GameDispatcher::GameDispatcher(Game& game) : game_(game)
{
}

size_t GameDispatcher::dispatch(uint8_t code, Session& session, boost::asio::const_buffer const& buf,
				size_t bytes_transferred)
{
	switch (code) {
	case 0x40: // Placement troupes
		return place_troops(session, buf, bytes_transferred);
		break;
	case 0x50: // Attaquer
		return attack(session, buf, bytes_transferred);
		break;
	case 0x60: // Transfert de troupes
		return transfer(session, buf, bytes_transferred);
		break;
	case 0x70: // Phase suivante
		return next_phase(session, buf, bytes_transferred);
		break;
	default:
		BOOST_LOG_TRIVIAL(warning)
			<< "Bad message id received in GameDispatcher : " << std::hex << static_cast<uint16_t>(code);
		send_error(session, 0x0, "ID non accepté par GameDispatcher");
		return bytes_transferred;
	}
}

size_t GameDispatcher::place_troops(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred)
{
	if (bytes_transferred < 2 * sizeof(uint16_t)) {
		BOOST_LOG_TRIVIAL(warning) << "Message with code 0x40 should contain 2 uint16_t";
		send_error(session, 0x0, "Message with code 0x40 should contain 2 uint16_t");
		return bytes_transferred;
	}

	uint16_t dst, nbt;
	size_t read = unserialize(buf, bytes_transferred, dst, nbt);

	try {
		auto cp = game_.current_phase();
		game_.add_troops(session, dst, nbt);

		broadcast(game_.lobby(), static_cast<uint8_t>(0x41), dst, nbt);

		if (cp != game_.current_phase()) { // toutes les troupes placées
			broadcast(game_.lobby(), static_cast<uint8_t>(0x71),
				  static_cast<uint8_t>(game_.current_phase()), game_.time_left());
		}
	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in Game::add_troups";
		//TODO: ban
		BOOST_LOG_TRIVIAL(fatal) << "TODO: ban";
		send_error(session, e.subcode(), e.what());
	}
	return read;
}

size_t GameDispatcher::attack(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred)
{
	if (bytes_transferred < 3 * sizeof(uint16_t)) {
		BOOST_LOG_TRIVIAL(warning) << "Message with code 0x50 should contain 3 uint16_t";
		send_error(session, 0x0, "Message with code 0x50 should contain 3 uint16_t");
		return bytes_transferred;
	}

	uint16_t src, dst, nbt;
	size_t read = unserialize(buf, bytes_transferred, src, dst, nbt);

	try {
		auto cp = game_.current_phase();
		auto nbp = game_.nb_alive();
		auto res = game_.attack(session, src, dst, nbt);

		broadcast(game_.lobby(), static_cast<uint8_t>(0x51), src, dst, res);

		if (nbp != game_.nb_alive()) { // mort
			broadcast(game_.lobby(), static_cast<uint8_t>(0x23), game_.last_dead());
			if (game_.is_finished()) { // partie finie
				broadcast(game_.lobby(), static_cast<uint8_t>(0x22), game_.winner());

				// Sauvegarder la partie dans la BDD
				try {
				    if (!DBMS::get ().add_game(game_)) {
                        BOOST_LOG_TRIVIAL(error) << "DBMS::add_game has returned false";
				    }
				} catch (const otl_exception &oe) {
                    BOOST_LOG_TRIVIAL(error) << "Can't connect to database";
				}

                auto all_sessions = game_.lobby().all_sessions();
				std::for_each(all_sessions.first, all_sessions.second, [&](Session& session) {
					session.change_dispatcher(std::make_unique<LobbyDispatcher>(game_.lobby()));
				});
			}
		}

		if (cp != game_.current_phase()) { // Phase finie
			broadcast(game_.lobby(), static_cast<uint8_t>(0x71),
				  static_cast<uint8_t>(game_.current_phase()), game_.time_left());
		}
	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in Game::attack";
		//TODO: ban
		BOOST_LOG_TRIVIAL(fatal) << "TODO: ban";
		send_error(session, e.subcode(), e.what());
	}

	return read;
}

size_t GameDispatcher::transfer(Session& session, boost::asio::const_buffer const& buf, size_t bytes_transferred)
{
	if (bytes_transferred < 3 * sizeof(uint16_t)) {
		BOOST_LOG_TRIVIAL(warning) << "Message with code 0x60 should contain 3 uint16_t";
		send_error(session, 0x0, "Message with code 0x60 should contain 3 uint16_t");
		return bytes_transferred;
	}

	uint16_t src, dst, nbt;
	size_t read = unserialize(buf, bytes_transferred, src, dst, nbt);

	try {
		auto cp = game_.current_phase();
		game_.transfer(session, src, dst, nbt);

		broadcast(game_.lobby(), static_cast<uint8_t>(0x61), src, dst, nbt);

		if (cp != game_.current_phase()) { // toutes les troupes placées
			broadcast(game_.lobby(), static_cast<uint8_t>(0x71),
				  static_cast<uint8_t>(game_.current_phase()), game_.time_left());
		}
	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in Game::transfer";
		//TODO: ban
		BOOST_LOG_TRIVIAL(fatal) << "TODO: ban";
		send_error(session, e.subcode(), e.what());
	}

	return read;
}

size_t GameDispatcher::next_phase(Session& session, boost::asio::const_buffer const&, size_t bytes_transferred)
{
	if (bytes_transferred > 0) {
		BOOST_LOG_TRIVIAL(warning) << "Message with code 0x70 should be empty";
		send_error(session, 0x0, "Message with code 0x70 should be empty");
		return bytes_transferred;
	}

	try {
		game_.skip(session);
		Gamephase cp = game_.current_phase();
		if (cp == Gamephase::Placement) { // nouveau tour
			broadcast(game_.lobby(), static_cast<uint8_t>(0x30), game_.current_player(),
				  game_.troop_gained());
		} else { // même tour
			broadcast(game_.lobby(), static_cast<uint8_t>(0x71),
				  static_cast<uint8_t>(game_.current_phase()), game_.time_left());
		}
	} catch (LogicException const& e) {
		BOOST_LOG_TRIVIAL(warning) << "LogicException in Game::skip";
		//TODO: ban
		BOOST_LOG_TRIVIAL(fatal) << "TODO: ban";
		send_error(session, e.subcode(), e.what());
	}

	return 0;
}
