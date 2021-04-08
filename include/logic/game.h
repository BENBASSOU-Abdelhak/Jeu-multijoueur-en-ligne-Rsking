#ifndef INCLUDE__LOGIC__GAME_H
#define INCLUDE__LOGIC__GAME_H

/*
 * Game
 */

#include <string>
#include <vector>

#include "logic/atk_result.h"
#include "logic/map.h"

struct GameParameters;
struct Session;
struct Lobby;

enum Gamephase : uint8_t { Placement = 1, Attack = 2, Transfer = 3 };

class Game
{
    public:
	Game(GameParameters const& params, Lobby& lobby);

	void add_troups(Session const& player_asking, uint16_t dst_square, uint16_t nb_troops);

	atk_result attack(Session const& player_asking, uint16_t src_square, uint16_t dst_square, uint16_t nb_troops);

	void transfer(Session const& player_asking, uint16_t src_square, uint16_t dst_square, uint16_t nb_troops);

	void skip(Session const& player_asking);

	void end_turn();

	void player_quit(Session const& player_asking, std::string const& gamertag);

	Gamephase current_phase() const;

	uint16_t troop_gained() const;

	uint16_t time_left() const;

	std::string const& current_player() const;

	size_t nb_alive() const;

	std::string const& last_dead() const;

	bool is_finished() const;

	std::string const& winner() const;

	Lobby& lobby() const;

	Map const& get_map() const;

    private:
	Lobby& m_lobby;
	Map m_map;
};

#endif
