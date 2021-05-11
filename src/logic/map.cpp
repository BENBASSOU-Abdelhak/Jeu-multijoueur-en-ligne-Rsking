#include "logic/map.h"
#include <stdexcept>
#include <sstream>
#include <fstream>

#include "generator.h"

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>

__attribute__((weak)) Map::Map(uint16_t id_map, std::vector<std::string> players_tag)
{
	std::ifstream fd_map(std::to_string(id_map));
	if (!fd_map)
		throw std::runtime_error("ouverture fichier impossible");

	uint16_t cpt{ 0 }, nb_square, nb_area{ 0 }, tmp_int;
	char tmp_char{ ' ' };
	bool tmp_bool;
	std::string tmp_line;

	std::getline(fd_map, tmp_line); // j'ignore le nom de la map
	if (!(fd_map >> nb_square) || nb_square <= 0)
		throw std::logic_error("bad format file");
	if (!(fd_map >> nb_area) || nb_area <= 0)
		throw std::logic_error("bad format file");

	//recup le valeur des regions
	m_area_values.resize(nb_area);
	for (uint16_t i = 0; i < nb_area; i++) {
		if (!(fd_map >> tmp_int))
			throw std::logic_error("bad format file");
		m_area_values[i] = tmp_int;
	}

	// matrice de stockages des territoires des regions
	std::vector<std::vector<uint16_t>> square_areas;
	square_areas.resize(nb_area, std::vector<uint16_t>(0));

	// recup id region des territoires
	std::vector<bool> check_square(nb_square, false);
	m_info_square.resize(nb_square);
	fd_map.ignore();
	for (uint16_t i = 0; i < nb_area; i++) {
		std::getline(fd_map, tmp_line);
		std::stringstream ss_line(tmp_line);
		while (ss_line >> tmp_int) {
			if (tmp_int >= nb_square)
				throw std::logic_error("bad format file");
			if (check_square[tmp_int])
				throw std::logic_error("bad format file");
			check_square[tmp_int] = true;
			m_info_square[tmp_int].id_region = i;

			square_areas[i].push_back(tmp_int);
		}
	}
	for (auto i : check_square)
		if (!i)
			throw std::logic_error("bad format file");

	//on ignore les noms des regions
	while (cpt < nb_area) {
		fd_map.get(tmp_char);
		if (tmp_char == '\n')
			cpt++;
	}

	// recup matrice
	m_matrix.resize(nb_square, std::vector<bool>(nb_square));
	for (int i = 0; i < nb_square; i++)
		for (int j = 0; j < nb_square; j++) {
			if (fd_map >> tmp_bool)
				m_matrix[i][j] = tmp_bool;
			else
				throw std::logic_error("bad format file");
		}

	// test fin de fichier
	if (fd_map >> tmp_char)
		throw std::logic_error("bad format file");

	/***************************** repartition des troupes et territoires *****/

	// melanges de l'orde des territoires des regions
	for (int i = 0; i < nb_area; i++)
		std::shuffle(std::begin(square_areas[i]), std::end(square_areas[i]), Gen::get());

	std::mt19937& gen = Gen::get();
	std::uniform_int_distribution<> distrib(1, 3);

	const int nb_player = players_tag.size();
	int nb_troop_max = ((int)(nb_square / nb_player)) * 2;

	std::vector<std::vector<uint8_t>> troops_distribution;
	troops_distribution.resize(nb_player, std::vector<uint8_t>((nb_square / nb_player) + 1));

	// definition nb des territoires pour chaque joueur
	for (int i = 0; i < nb_player; i++) {
		if (i == (nb_player - 1))
			troops_distribution[i].assign(nb_square - (((int)(nb_square / nb_player)) * i), 1);
		else
			troops_distribution[i].assign(nb_square / nb_player, 1);
	}

	// def du nb de troupe sur chaque territoire
	for (int i = 0; i < nb_player; i++) {
		int j = 0;
		int somme = std::accumulate(troops_distribution[i].begin(), troops_distribution[i].end(), 0);
		while (somme != nb_troop_max && j < (int)troops_distribution[i].size()) {
			troops_distribution[i][j] = distrib(gen);
			somme = std::accumulate(troops_distribution[i].begin(), troops_distribution[i].end(), 0);

			if (somme > nb_troop_max) {
				while (somme != nb_troop_max) {
					troops_distribution[i][j]--;
					somme = std::accumulate(troops_distribution[i].begin(),
								troops_distribution[i].end(), 0);
				}

				somme = std::accumulate(troops_distribution[i].begin(), troops_distribution[i].end(),
							0);
			}
			j++;
		}

		// si il n'y a pas assez de troupe
		if (somme < nb_troop_max) {
			j = 0;
			while (somme != nb_troop_max && j < (int)troops_distribution[i].size()) {
				if (troops_distribution[i][j] < 3)
					troops_distribution[i][j]++;
				somme = std::accumulate(troops_distribution[i].begin(), troops_distribution[i].end(),
							0);
				// cas extreme
				if (j == ((int)troops_distribution[i].size() - 1) && somme != nb_troop_max)
					troops_distribution[i][j] += nb_troop_max - somme;

				j++;
			}
		}
	}

	// attribution des troupes et territoires
	int cpt2 = 0;
	for (int i = 0; i < nb_area; i++) {
		for (int j = 0; j < (int)square_areas[i].size(); j++) {
			if (i == (nb_area - 1) && (j == (int)square_areas[i].size() - 1) &&
			    (nb_square % players_tag.size() != 0)) {
				m_info_square[square_areas[i][j]].nb_troops =
					troops_distribution[players_tag.size() - 1].back();
				m_info_square[square_areas[i][j]].player_tag = players_tag[players_tag.size() - 1];
				m_info_square[square_areas[i][j]].player_id = players_tag.size();
			} else {
				m_info_square[square_areas[i][j]].nb_troops =
					troops_distribution[cpt2 % players_tag.size()].back();
				m_info_square[square_areas[i][j]].player_tag = players_tag[cpt2 % players_tag.size()];
				m_info_square[square_areas[i][j]].player_id = 1 + (cpt2 % players_tag.size());
			}
			troops_distribution[cpt2 % players_tag.size()].pop_back();
			cpt2++;
		}
	}

	// verification de repartition des troupes
	for (auto t : players_tag) {
		if (get_nb_troops_player(t) < nb_troop_max) {
			for (int i = 0; i < nb_square && get_nb_troops_player(t) < nb_troop_max; i++) {
				if ((!get_square_owner(i).compare(t)) && get_nb_troops_square(i) < 3)
					add_troops(i, 1);
			}
		}
	}
}

__attribute__((weak)) Map::Map()
{
}

uint16_t Map::get_nb_square() const
{
	return (uint16_t)m_matrix[0].size();
}

uint16_t Map::get_nb_area() const
{
	return m_area_values.size();
}

bool Map::is_valid_square(uint16_t square) const
{
	return square < get_nb_square();
}

bool Map::is_neighbor_square(uint16_t src_square, uint16_t dst_square) const
{
	return m_matrix[src_square][dst_square];
}

bool Map::is_possible_transfer(std::string player_tag, uint16_t src_square, uint16_t dst_square) const
{
	int n = get_nb_square();
	std::vector<std::vector<int>> matrix_player;
	matrix_player.resize(n, std::vector<int>(n));

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
			if (m_matrix[i][j] && !get_square_owner(i).compare(player_tag) &&
			    !get_square_owner(j).compare(player_tag))
				matrix_player[i][j] = 1;
			else
				matrix_player[i][j] = 0;
		}

	// on verifie si le territoire est voisin, si c'est le cas pas besoin de
	// derouler l'algo.
	if (matrix_player[src_square][dst_square])
		return true;

	std::vector<std::vector<int>> m, u, v;
	m.resize(n, std::vector<int>(n));
	u.resize(n, std::vector<int>(n));
	v.resize(n, std::vector<int>(n));

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
			if (matrix_player[i][j])
				m[i][j] = u[i][j] = v[i][j] = 1;
			else
				m[i][j] = u[i][j] = v[i][j] = 0;
		}

	for (int k = 1; u[src_square][dst_square] == 0 && k < n; k++)
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				for (int l = 0; l < n; l++) {
					v[i][j] += v[i][l] * m[l][j];
					u[i][j] += v[i][j];
				}

	return u[src_square][dst_square] != 0;
}

uint16_t Map::get_nb_troops_square(uint16_t square) const
{
	return m_info_square[square].nb_troops;
}

void Map::add_troops(uint16_t dst_square, uint16_t nb_troops)
{
	m_info_square[dst_square].nb_troops += nb_troops;
}

void Map::remove_troops(uint16_t square, uint16_t nb_troops)
{
	m_info_square[square].nb_troops -= nb_troops;
}

std::string Map::get_square_owner(uint16_t square) const
{
	return m_info_square[square].player_tag;
}

void Map::set_square_owner(uint16_t square, std::string new_owner)
{
	m_info_square[square].player_tag = new_owner;
}

uint16_t Map::get_area_points_player(std::string player) const
{
	std::vector<bool> occupied_area(get_nb_area(), true);
	uint16_t cpt_pts = 0;
	occupied_area.assign(get_nb_area(), true);

	for (uint16_t i = 0; i < get_nb_area(); i++)
		occupied_area[i] = true;

	for (uint16_t i = 0; i < get_nb_square(); i++)
		if (m_info_square[i].player_tag.compare(player))
			occupied_area[m_info_square[i].id_region] = false;

	for (int i = 0; i < get_nb_area(); i++)
		if (occupied_area[i])
			cpt_pts += get_area_value(i);

	return cpt_pts;
}

uint16_t Map::get_nb_area_player(std::string player) const
{
	std::vector<bool> occupied_area(get_nb_area(), true);
	uint16_t cpt_nb = 0;

	for (uint16_t i = 0; i < get_nb_square(); i++)
		if (m_info_square[i].player_tag.compare(player))
			occupied_area[m_info_square[i].id_region] = false;

	for (int i = 0; i < get_nb_area(); i++)
		if (occupied_area[i])
			cpt_nb++;

	return cpt_nb;
}

uint16_t Map::get_nb_troops_player(std::string player) const
{
	uint16_t cpt{ 0 };
	for (uint16_t i = 0; i < get_nb_square(); i++)
		if (!m_info_square[i].player_tag.compare(player))
			cpt += m_info_square[i].nb_troops;
	return cpt;
}

uint16_t Map::get_nb_square_player(std::string player) const
{
	uint16_t cpt{ 0 };
	for (uint16_t i = 0; i < get_nb_square(); i++)
		if (!m_info_square[i].player_tag.compare(player))
			cpt++;
	return cpt;
}

bool Map::is_completely_conquered() const
{
	for (uint16_t i = 1; i < get_nb_square(); i++)
		if (m_info_square[i].player_tag.compare(m_info_square[0].player_tag))
			return false;

	return true;
}

uint16_t Map::get_area_value(uint16_t area) const
{
	return m_area_values[area];
}

uint16_t Map::get_id_area(uint16_t square) const
{
	return m_info_square[square].id_region;
}
