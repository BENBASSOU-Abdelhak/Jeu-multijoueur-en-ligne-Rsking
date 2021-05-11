#include "logic/dice_roll.h"

#include "generator.h"

#include <algorithm>
#include <random>
#include <stdexcept>

void Dice_roll::set_dice_values()
{
	std::random_device rd;
	std::mt19937& gen = Gen::get();
	std::uniform_int_distribution<> distrib(1, 6);

	for (int i = 0; i < NB_ATK_DICE; i++)
		attackers_values[i] = distrib(gen);

	for (int i = 0; i < NB_DF_DICE; i++)
		defenders_values[i] = distrib(gen);

	std::sort(attackers_values.begin(), attackers_values.end(), std::greater<uint8_t>());
	std::sort(defenders_values.begin(), defenders_values.end(), std::greater<uint8_t>());
}

uint8_t Dice_roll::get_defenders_values(uint8_t dice) const
{
	if (dice >= NB_DF_DICE)
		throw std::invalid_argument("dé incorrect");
	return defenders_values[dice];
}

uint8_t Dice_roll::get_attackers_values(uint8_t dice) const
{
	if (dice >= NB_ATK_DICE)
		throw std::invalid_argument("dé incorrect");
	return attackers_values[dice];
}
