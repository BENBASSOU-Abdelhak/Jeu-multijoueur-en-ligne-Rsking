#ifndef INCLUDE_MAP_H
#define INCLUDE_MAP_H

#include <vector>
#include <string>

struct info_square {
	uint16_t id_region;
	uint16_t nb_troops;
	uint8_t player_id;
};

class Map
{
    public:
	Map();
	template <typename Stream> friend void create_buf(Stream& sbuf, Map const& map);

    private:
	std::vector<uint16_t> m_area_values;
	std::vector<struct info_square> m_info_square;
	std::vector<std::vector<bool>> m_matrix;
};

#endif
