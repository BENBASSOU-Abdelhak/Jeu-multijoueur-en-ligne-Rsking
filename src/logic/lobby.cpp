#include "logic/lobby.h"
#include "logic/game.h"
#include "logicexception.h"
#include "network/session.h"

#include <algorithm>

__attribute__((weak)) Lobby::Lobby(lobby_id_t id, GameParameters const& params)
	: m_id{ id }, m_parameters{ params }, m_remaining_place{ m_parameters.nb_players }, m_game{ nullptr }
{
}

__attribute__((weak)) std::shared_ptr<Session> Lobby::ban(std::shared_ptr<Session> origine, std::string const& gamertag)
{
	auto itr = std::find(std::begin(m_gamertag_list), std::end(m_gamertag_list), gamertag);
	if (itr == m_gamertag_list.end())
		throw LogicException(0x16, "Le joueur n’existe pas dans ce salon !");

	size_t index = std::distance(std::begin(m_gamertag_list), itr);
	auto sp = m_list_session[0].lock();
	if (!sp || *sp != *origine) {
		auto spi = m_list_session[index].lock();
		if (!spi || *spi != *origine) {
			throw LogicException(0x15, "Le joueur n’a pas le droit d’expulser !");
		}
	}

	m_gamertag_ban_list.push_back(gamertag);
	m_gamertag_list.erase(itr);
	std::shared_ptr<Session> session_to_return = m_list_session[index].lock();
	m_list_session.erase(std::begin(m_list_session) + index);
	m_remaining_place++;
	return session_to_return;
}

std::shared_ptr<Session> Lobby::ban_in_game(std::string const& gamertag)
{
	auto itr = std::find(std::begin(m_gamertag_list), std::end(m_gamertag_list), gamertag);
	if (itr == m_gamertag_list.end())
		throw LogicException(0x16, "Le joueur n’existe pas dans ce salon !");

	size_t index = std::distance(std::begin(m_gamertag_list), itr);
	m_gamertag_ban_list.push_back(gamertag);
	m_gamertag_list.erase(itr);
	std::shared_ptr<Session> session_to_return = m_list_session[index].lock();
	m_list_session.erase(std::begin(m_list_session) + index);
	m_remaining_place++;
	return session_to_return;
}

void Lobby::exit(std::string const& gamertag)
{
	auto itr = std::find(std::begin(m_gamertag_list), std::end(m_gamertag_list), gamertag);
	if (itr == m_gamertag_list.end())
		throw LogicException(0x16, "Le joueur n’existe pas dans ce salon !");

	m_gamertag_list.erase(itr);
	size_t index = std::distance(std::begin(m_gamertag_list), itr);
	m_list_session.erase(std::begin(m_list_session) + index);
	m_remaining_place++;
}

__attribute__((weak)) void Lobby::join(std::shared_ptr<Session> session, std::string const& gamertag)
{
	if (m_remaining_place == 0)
		throw LogicException(0x12, "Salon déjà plein !");

	m_gamertag_list.push_back(gamertag);
	m_list_session.push_back(session);
	m_remaining_place--;
}

__attribute__((weak)) Game& Lobby::start_game(std::shared_ptr<Session> origine)
{
	if (m_parameters.nb_players - 1 <= m_remaining_place)
		throw LogicException(0x20, "2 joueurs minimum pour lancer la partie!");
	auto sp = m_list_session[0].lock();
	if (!sp || *sp != *origine)
		throw LogicException(0x21, "Pas le droit de lancer la partie!");

	m_game = std::make_unique<Game>(m_parameters, *this);
	return *m_game;
}

nb_players_t Lobby::get_nb_player() const
{
	return m_parameters.nb_players;
}

__attribute__((weak)) lobby_id_t Lobby::id() const
{
	return m_id;
}

id_map_t Lobby::get_id_map() const
{
	return m_parameters.id_map;
}

sec_by_turn_t Lobby::get_sec_by_turn() const
{
	return m_parameters.sec_by_turn;
}

nb_players_t Lobby::get_remaining_place() const
{
	return m_remaining_place;
}

std::string const& Lobby::get_gamertag(std::shared_ptr<Session> session) const
{
	auto itr = std::find_if(std::begin(m_list_session), std::end(m_list_session), [&](auto const& lhs){auto sp = lhs.lock(); return sp && (*sp == *session);});
	if (itr == m_list_session.end())
		throw LogicException(0x16, "Le joueur n’existe pas dans ce salon !");

	size_t index = std::distance(std::begin(m_list_session), itr);

	return m_gamertag_list[index];
}

std::shared_ptr<Game> Lobby::get_started() const
{
	return m_game;
}

bool Lobby::is_started() const {
	return !((get_started() == nullptr) || m_game->is_finished());
}


__attribute__((weak)) GameParameters const& Lobby::parameters() const
{
	return m_parameters;
}

bool Lobby::verification_join(std::string const& gamertag) const
{
	for (unsigned int i = 0; i < m_gamertag_ban_list.size(); i++) {
		if (m_gamertag_ban_list[i] == gamertag)
			return true;
	}
	auto itr = std::find(std::begin(m_gamertag_list), std::end(m_gamertag_list), gamertag);
	if (itr != m_gamertag_list.end())
		return true;

	return false;
}

__attribute__((weak)) std::pair<Lobby::const_player_it, Lobby::const_player_it> Lobby::all_players() const
{
	return { m_gamertag_list.cbegin(), m_gamertag_list.cend() };
}

__attribute__((weak)) std::pair<Lobby::it_session, Lobby::it_session> Lobby::all_sessions()
{
	return { m_list_session.begin(), m_list_session.end() };
}
