 #ifndef INCLUDE_DICE_ROLL_H
#define INCLUDE_DICE_ROLL_H

#include <iostream>
#include <math.h>
#include <random>
#include <array>
#include <algorithm>

#define NB_ATK_DICE 3
#define NB_DF_DICE 2


class Dice_roll {
    public :
        Dice_roll() {};
        
        void set_dice_values() {
            std::random_device rd; 
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(1, 6);

            for (int i = 0; i < NB_ATK_DICE; i++)
                attackers_values[i] = distrib(gen);
                
            for (int i = 0; i < NB_DF_DICE; i++)
                defenders_values[i] = distrib(gen);

            std::sort(attackers_values.begin(), attackers_values.end(), std::greater<uint8_t>());
            std::sort(defenders_values.begin(), defenders_values.end(), std::greater<uint8_t>());
        };
        
        uint8_t get_defenders_values(uint8_t dice) const {
            if (dice >= NB_DF_DICE)
                throw std::string("dé incorrect");
            return defenders_values[dice];
        };
        
        uint8_t get_attackers_values(uint8_t dice) const {
            if (dice >= NB_ATK_DICE)
                throw std::string("dé incorrect");
           return attackers_values[dice];
        };

    private :
        std::array<uint8_t, NB_ATK_DICE> attackers_values;
        std::array<uint8_t, NB_DF_DICE> defenders_values;
};

#endif