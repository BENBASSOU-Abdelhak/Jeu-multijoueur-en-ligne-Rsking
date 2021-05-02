#ifndef INCLUDE__LOGIC__PLAYER_H
#define INCLUDE__LOGIC__PLAYER_H

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <bits/stdc++.h>



class Player {
    public :
        Player(std::string tag);
        bool operator==(Player const& player) const;
        bool operator==(std::string const& gamertag) const;
        bool operator!=(Player const& player) const;
        
        uint16_t get_nb_troops() const;
        uint16_t get_area_points() const;
        uint16_t get_nb_square() const;
        uint16_t get_nb_area() const;
        bool is_online() const;
        bool is_already_transfered() const;
        uint16_t get_remaining_deploy_troops() const;
        std::string const& get_tag() const;

        void set_nb_troops(uint16_t nb_troops);
        void set_area_points(uint16_t value);
        void set_nb_square(uint16_t nb_square);
        void set_nb_area(uint16_t nb_area);
        void set_disconnect();
        void set_already_transfered(bool value);
        void set_remaining_deploy_troops(uint16_t nb_troops);

        uint16_t get_last_atk_square() const;
        uint16_t get_square_from_last_atk() const;
        bool is_already_transfered_after_atk() const;
        void set_last_atk_square(uint16_t square);
        void set_square_from_last_atk(uint16_t square);
        void set_already_transfered_after_atk(bool b);
        
        void reset_rem_troops();
        void set_connected();

    private :
        std::string m_tag;
        bool m_is_online;
        bool m_already_transfered;
        uint16_t m_remaining_deploy_troops;
        
        uint16_t m_last_attacked_square;
        uint16_t m_square_from_last_attack;
        bool m_already_transfered_after_attack;

        uint16_t m_nb_troops;
        uint16_t m_nb_square;
        uint16_t m_nb_area;
        uint16_t m_area_points;
};

#endif