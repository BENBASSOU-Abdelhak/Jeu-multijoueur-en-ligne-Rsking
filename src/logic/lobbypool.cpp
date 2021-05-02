#include "logic/lobbypool.h"

__attribute__((weak)) LobbyPool::LobbyPool(size_t max_lobbies) : m_nb_lobby{ 0 }, m_max_lobby{ max_lobbies }
{
	std::random_device rd;
	std::mt19937 m_gen(rd());
	std::uniform_int_distribution<lobby_id_t> m_distrib(0, std::numeric_limits<lobby_id_t>::max());
}

__attribute__((weak)) LobbyPool& LobbyPool::get()
{
	static LobbyPool lp(64);
	return lp;
}

Lobby& LobbyPool::getLobby(lobby_id_t id)
{
	return m_lobby_list.at(id);
}

Lobby const& LobbyPool::getLobby(lobby_id_t id) const
{
	return m_lobby_list.at(id);
}

__attribute__((weak)) Lobby& LobbyPool::create_lobby(Session& session, std::string const& gamertag,
						     GameParameters const& params)
{
	if (m_nb_lobby == m_max_lobby)
		throw LogicException(0x17, "Nombre maximum de salons atteint !");

	lobby_id_t new_id = m_distrib(m_gen);
	lobby_id_t new_id_deux = new_id;
	m_lobby_list.emplace(std::make_pair<lobby_id_t, Lobby>(std::move(new_id_deux), Lobby{ new_id, params }));
	m_lobby_list.at(new_id).join(session, gamertag);

	m_nb_lobby++;
	return m_lobby_list.at(new_id);
}

__attribute__((weak)) Lobby& LobbyPool::join_lobby(lobby_id_t lobby_id, Session& session, std::string const& gamertag)
{
	auto itr = m_lobby_list.find(lobby_id);
	if (itr == m_lobby_list.end())
		throw LogicException(0x11, "ID de salon incorrect !");

	if (m_lobby_list.at(lobby_id).get_started() != nullptr)
		throw LogicException(0x13, "Partie déjà en cours!");

	bool his_ban = m_lobby_list.at(lobby_id).verification_join(gamertag);
	if (his_ban == true)
		throw LogicException(0x14, "Joueur exclu du salon!");

	m_lobby_list.at(lobby_id).join(session, gamertag);

	return m_lobby_list.at(lobby_id);
}

void LobbyPool::destroy_lobby(lobby_id_t id)
{
	auto itr = m_lobby_list.find(id);
	if (itr == m_lobby_list.end())
		throw LogicException(0x11, "ID de salon incorrect !");
	assert(m_lobby_list.at(id).get_remaining_place() == m_lobby_list.at(id).get_nb_player() &&
	       " Impossible de detruire le lobby, il y a des joueurs !");

	m_lobby_list.erase(id);
	m_nb_lobby--;
}

size_t LobbyPool::get_nb_lobby() const
{
	return m_nb_lobby;
}

size_t LobbyPool::get_max_lobby() const
{
	return m_max_lobby;
}
