#ifndef INCLUDE__LOGIC__GAME_H
#define INCLUDE__LOGIC__GAME_H

#include "logic/atk_result.h"
#include "logic/map.h"
#include "logic/dice_roll.h"
#include "logic/player.h"

#include "dbms.h"

enum Gamephase : uint8_t { Placement = 1, Attack, Transfer };

struct GameParameters;
struct Lobby;
struct Session;

class Game
{
    public:
	Game(GameParameters const& params, Lobby& lobby);
	void add_troops(Session const& player_asking, uint16_t dst_square, uint16_t nb_troops);
	atk_result attack(Session const& player_asking, uint16_t src_square, uint16_t dst_square, uint16_t nb_troops);
	void transfer(Session const& player_asking, uint16_t src_square, uint16_t dst_square, uint16_t nb_troops);
	void transfer_after_attack(Session const& player_asking, uint16_t src_square, uint16_t dst_square,
				   uint16_t nb_troops);

	void skip(Session const& player_asking);
	//quitté la partie, ban ou perdu
	void player_quit(Session const& player_asking, std::string const& gamertag);
	bool is_finished() const;

	Player& get_current_player();
	void set_next_current_player();

	uint16_t troop_gained();

	//void set_ban_player(Player player);
	//void set_end_game(Player player);

	void add_troops_map(uint16_t dst_square, uint16_t nb_troops);
	void remove_troops_map(uint16_t dst_square, uint16_t nb_troops);
	Player& get_square_owner_map(uint16_t square);

	uint16_t time_left() const;
	std::string const& current_player() const;
	Gamephase current_phase() const;
	size_t nb_alive() const;
	std::string const& last_dead() const;
	std::string const& winner();

	Lobby& lobby() const;
	uint8_t player_id(Player& player) const;
	uint8_t player_id(std::string const& player) const;
	Map const& get_map() const;

	// utilisé pour le debug et la realisation des tests
	Player& get_player_by_tag(std::string tag);
	Map& get_map();
	void set_current_player(uint16_t i);
	void set_square_owner_map(uint16_t square, std::string new_owner);
	void maj_score_player(std::string player);
	atk_result attack_test(Session const& player_asking, uint16_t src_square, uint16_t dst_square,
			       uint16_t nb_troops, int d1, int d2, int d3, int d4, int d5);
	Player& get_player_by_id(int i);

	void mark_player_as_eliminated(Player& player);

	friend bool DBMS::add_game(Game& game);

    private:
	Map m_map;
	Dice_roll m_dices;
	Gamephase m_phase;

	std::vector<Player> m_players;
	uint16_t m_i_current_player;

	std::vector<Player> m_eliminated_players;

	Lobby& m_lobby;

	bool m_waiting_transfer;
};

#endif
