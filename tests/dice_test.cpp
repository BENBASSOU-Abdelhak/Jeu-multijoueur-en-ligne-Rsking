#define BOOST_TEST_MODULE dice
#include <boost/test/included/unit_test.hpp>

#include "logic/dice_roll.h"

/**
 * @brief verification des arguments donnés aux getters
 */
BOOST_AUTO_TEST_CASE(check_entry)
{
	Dice_roll d;
	d.set_dice_values();

	for (int i = -5; i <= 5; i++) {
		if (i >= 0 && i <= 2) {
			try {
				d.get_attackers_values(i);
				BOOST_TEST(true);
			} catch (std::invalid_argument const& e) {
				BOOST_TEST(false);
			}
		} else if (i >= 0 && i <= 1) {
			try {
				d.get_defenders_values(i);
				BOOST_TEST(true);
			} catch (std::invalid_argument const& e) {
				BOOST_TEST(false);
			}
		} else {
			try {
				d.get_defenders_values(i);
				BOOST_TEST(false);
			} catch (std::invalid_argument const& e) {
				BOOST_TEST(true);
			}

			try {
				d.get_attackers_values(i);
				BOOST_TEST(false);
			} catch (std::invalid_argument const& e) {
				BOOST_TEST(true);
			}
		}
	}
}

/**
 * @brief verification valeurs du dé entre 1 et 6
 */
BOOST_AUTO_TEST_CASE(check_value_in_boudary)
{
	Dice_roll d;

	for (int i = 0; i < 50; i++) {
		d.set_dice_values();

		for (int j = 0; j <= 2; j++)
			if (d.get_attackers_values(j) <= 0 || d.get_attackers_values(j) > 6)
				BOOST_TEST(false);

		for (int j = 0; j <= 1; j++)
			if (d.get_defenders_values(j) <= 0 || d.get_defenders_values(j) > 6)
				BOOST_TEST(false);
	}
}

/**
 * @brief verification du tri des valeurs du dé
 */
BOOST_AUTO_TEST_CASE(increasing_value)
{
	Dice_roll d;

	for (int i = 0; i < 50; i++) {
		d.set_dice_values();

		for (int j = 1; j < NB_ATK_DICE; j++)
			if (d.get_attackers_values(j) > d.get_attackers_values(j - 1))
				BOOST_TEST(false);

		for (int j = 1; j < NB_DF_DICE; j++)
			if (d.get_defenders_values(j) > d.get_defenders_values(j - 1))
				BOOST_TEST(false);
	}
}
