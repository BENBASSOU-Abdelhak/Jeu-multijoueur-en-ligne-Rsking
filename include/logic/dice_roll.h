#ifndef INCLUDE_DICE_ROLL_H
#define INCLUDE_DICE_ROLL_H

#include <array>

#include "generator.h"

#define NB_ATK_DICE 3
#define NB_DF_DICE 2

class Dice_roll
{
    public:
	Dice_roll() = default;

	void set_dice_values();

	uint8_t get_defenders_values(uint8_t dice) const;
	uint8_t get_attackers_values(uint8_t dice) const;

    private:
	std::array<uint8_t, NB_ATK_DICE> attackers_values;
	std::array<uint8_t, NB_DF_DICE> defenders_values;
};

#endif
