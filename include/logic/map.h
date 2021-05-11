#ifndef INCLUDE__LOGIC__MAP_H
#define INCLUDE__LOGIC__MAP_H

#include <vector>
#include <string>


struct info_square {
    uint16_t id_region;
    uint16_t nb_troops;
    std::string player_tag;
    uint8_t player_id;
};


class Map {
    public :
        Map();
        Map(uint16_t id_map, std::vector<std::string> players_tag);
        void add_troops(uint16_t dst_square, uint16_t nb_troops);
        void remove_troops(uint16_t square, uint16_t nb_troops);
        void set_nb_troops(uint16_t square, uint16_t nb_troops);
        
        bool is_possible_transfer(std::string player_tag, uint16_t src_square, uint16_t dst_square) const;
        bool is_neighbor_square(uint16_t src_square, uint16_t dst_square) const;
        bool is_valid_square(uint16_t square) const;
        bool is_completely_conquered() const;

        uint16_t get_nb_square() const;
        uint16_t get_nb_area() const;
        uint16_t get_nb_troops_square(uint16_t square) const;
        std::string get_square_owner(uint16_t square) const;
        uint16_t get_area_value(uint16_t area) const;
        
        void set_square_owner(uint16_t square, std::string new_owner);
        uint16_t get_area_points_player(std::string player) const;
        uint16_t get_nb_area_player(std::string player) const;
        uint16_t get_nb_troops_player(std::string player) const;
        uint16_t get_nb_square_player(std::string player) const;

        uint16_t get_id_area(uint16_t square) const;

        template <typename Stream> 
        friend void create_buf(Stream& sbuf, Map const& map);

    private :
        std::vector<uint16_t> m_area_values;
        std::vector<struct info_square> m_info_square;
        std::vector<std::vector<bool>> m_matrix;
};


#endif
