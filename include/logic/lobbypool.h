#ifndef INCLUDE__LOGIC__LOBBYPOOL_H
#define INCLUDE__LOGIC__LOBBYPOOL_H

/*
 * Classe LobbyPool
 */

#include "lobby.h"
#include <random>
#include <limits>

#include <string>

using lobby_id_t = uint64_t;
struct Session;

class LobbyPool
{
    public:
	static LobbyPool& get();
	Lobby& create_lobby(Session& session, std::string const& gamertag, GameParameters const& params);
	Lobby& join_lobby(lobby_id_t lobby_id, Session& session, std::string const& gamertag);
	void destroy_lobby(lobby_id_t lobby_id);
	Lobby& getLobby(lobby_id_t lobby_id);
	Lobby const& getLobby(lobby_id_t lobby_id) const;
	size_t get_nb_lobby() const;
	size_t get_max_lobby() const;

    private:
	LobbyPool(size_t max_lobbies);
	size_t m_nb_lobby; // nb de lobbies actuel
	size_t m_max_lobby;
	std::mt19937 m_gen;
	std::uniform_int_distribution<lobby_id_t> m_distrib;
	std::map<lobby_id_t, Lobby> m_lobby_list;
};

#endif
