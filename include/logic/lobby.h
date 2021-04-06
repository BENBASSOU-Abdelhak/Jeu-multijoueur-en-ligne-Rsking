#ifndef __LOBBY__
#define __LOBBY__

#include <string>
#include <map>
#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <exception>
#include "logicexception.h"
#include "network/session.h"
#include "logic/game.h"
#include "logic/gameparameters.h"

using lobby_id_t = uint64_t;
using sec_by_turn_t = uint16_t;
using id_map_t = uint16_t;
using nb_players_t = uint8_t;

class Lobby
{
    public:
	Lobby(lobby_id_t id, GameParameters const& params);
	using const_player_it = std::vector<std::string>::const_iterator;
	std::pair<const_player_it, const_player_it> all_players() const;
	Session& ban(Session const& origine, std::string const& gamertag);
	Session& ban_in_game(std::string const& gamertag);
	void exit(std::string const& gamertag);
	void join(Session& session, std::string const& gamertag);
	Game& start_game(Session const& origine);
	nb_players_t get_nb_player() const;
	lobby_id_t id() const;
	id_map_t get_id_map() const;
	sec_by_turn_t get_sec_by_turn() const;
	nb_players_t get_remaining_place() const;
	std::string const& get_gamertag(Session const& session) const;
	GameParameters const& parameters() const;
	bool verification_join(std::string const& gamertag) const;
	std::shared_ptr<Game> get_started() const;

	~Lobby() = default;

	template <typename T, typename... Args> friend void broadcast(Lobby const& lobby, T&& param, Args&&... params);

    private:
	lobby_id_t m_id;
	std::vector<std::string> m_gamertag_list;
	std::vector<std::string> m_gamertag_ban_list;
	std::vector<std::reference_wrapper<Session>> m_list_session;
	GameParameters m_parameters;
	nb_players_t m_remaining_place;
	std::shared_ptr<Game> m_game;
};

#endif