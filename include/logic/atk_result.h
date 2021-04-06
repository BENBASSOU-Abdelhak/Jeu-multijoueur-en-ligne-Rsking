#ifndef INCLUDE__LOGIC__ATK_RESULT_H
#define INCLUDE__LOGIC__ATK_RESULT_H

#include <vector>
#include <cstdint>

struct atk_result {
	bool square_conquered;
	bool defender_loose_game;
	uint16_t nb_lost_troops_from_attacker;
	uint16_t nb_lost_troops_from_defender;
	std::vector<uint8_t> attackers_dice;
	std::vector<uint8_t> defenders_dice;
};

#endif
