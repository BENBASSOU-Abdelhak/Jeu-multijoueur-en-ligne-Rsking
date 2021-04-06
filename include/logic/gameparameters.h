#ifndef INCLUDE__LOGIC__GAMEPARAMETERS_H
#define INCLUDE__LOGIC__GAMEPARAMETERS_H

/*
 * structure GameParameters
 */

#include <cstdint>

struct GameParameters {
	uint8_t nb_players;
	uint16_t id_map;
	uint16_t sec_by_turn;
};

#endif
