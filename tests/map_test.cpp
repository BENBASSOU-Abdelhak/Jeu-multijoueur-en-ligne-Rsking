/*
 * Check wether Boost::Test works and is integrated to cMake
 */
#define BOOST_TEST_MODULE map
#include <boost/test/included/unit_test.hpp>

#include <filesystem>
#include "logic/map.h"

/************** Creation outils et environnement pour les tests ***************/
void create_map() {
    std::string const map_file("1");
    std::ofstream fd_map(map_file.c_str());
    if (!fd_map)    
    {
        std::cout << "ERREUR: Impossible de creer le fichier." << std::endl;
        return;
    }

    fd_map << "carte de test" << std::endl;
    fd_map << "9" << std::endl;
    fd_map << "4" << std::endl;
    fd_map << "2" << std::endl;
    fd_map << "2" << std::endl;
    fd_map << "1" << std::endl;
    fd_map << "1" << std::endl;
    fd_map << "0 2" << std::endl;
    fd_map << "1 3" << std::endl;
    fd_map << "4 6 5" << std::endl;
    fd_map << "7 8" << std::endl;
    fd_map << "region 0" << std::endl;
    fd_map << "region 1" << std::endl;
    fd_map << "region 2" << std::endl;
    fd_map << "region 3" << std::endl;
    fd_map << "0 1 1 1 0 0 0 0 0" << std::endl;
    fd_map << "1 0 0 1 0 0 0 0 0" << std::endl;
    fd_map << "1 0 0 1 1 0 1 0 0" << std::endl;
    fd_map << "1 1 1 0 0 1 1 0 0" << std::endl;
    fd_map << "0 0 1 0 0 0 1 1 0" << std::endl;
    fd_map << "0 0 0 1 0 0 1 0 0" << std::endl;
    fd_map << "0 0 1 1 1 1 0 1 0" << std::endl;
    fd_map << "0 0 0 0 1 0 1 0 1" << std::endl;
    fd_map << "0 0 0 0 0 0 0 1 0";

    return;
}

void remove_map() {
    std::remove("1");
    return;
}

struct CreateMap {
	CreateMap() {
		create_map();
	}

	~CreateMap() {
		remove_map();
	}
};
/******************************************************************************/


/**
 * @brief test de la fonction parser des map avec des maps ayant un mauvais format
 */
BOOST_AUTO_TEST_CASE(check_format_file)
{
    std::vector<std::string> players {"p1", "p2", "p3"};
    for (int i = 100; i <= 114; i++) {
        char file[5];
        sprintf(file, "%d", i);
        char oldf[50] = "testmaps/";
        std::strcat(oldf, file);
        std::rename(oldf, file);

        try {
            Map map(i, players);
            BOOST_TEST(false);
        } catch (std::string const& e) { BOOST_TEST(true);}
        std::rename(file, oldf);
    }

    //fichier n'exitse pas
    try {
        Map map(0, players);
        BOOST_TEST(false);
    } catch (std::runtime_error const& e) { BOOST_TEST(true);}

    //donneé negative
    char file[5];
    sprintf(file, "%d", 116);
    char oldf[50] = "testmaps/";
    std::strcat(oldf, file);
    std::rename(oldf, file);

    try {
        Map map(116, players);
        BOOST_TEST(false);
    } catch (std::string const& e) { BOOST_TEST(true);}
    std::rename(file, oldf);
}

/**
 * @brief test avec une fichier de map tres grand
 */
BOOST_AUTO_TEST_CASE(big_map_file)
{
    std::vector<std::string> players {"p1", "p2", "p3"};
    char oldf[50] = "testmaps/115";
    std::rename(oldf, "115");

    try {
        Map map(115, players);
        BOOST_TEST(true);
    } catch (std::string const& e) { BOOST_TEST(false);}
    std::rename("115", oldf);   
}


/**
 * @brief verification des données recuperées par le parser des map
 */
BOOST_FIXTURE_TEST_CASE(check_map_value, CreateMap)
{
    std::vector<std::string> players {"p1", "p2", "p3"};
	Map map(1, players);

    BOOST_REQUIRE_EQUAL(map.get_nb_area(), 4);
    BOOST_REQUIRE_EQUAL(map.get_nb_square(), 9);

    BOOST_TEST(map.get_id_area(0) == 0);
    BOOST_TEST(map.get_id_area(2) == 0);
    BOOST_TEST(map.get_id_area(1) == 1);
    BOOST_TEST(map.get_id_area(3) == 1);

    for (int i = 4; i <= 6; i++)
        BOOST_TEST(map.get_id_area(i) == 2);
    for (int i = 7; i <= 8; i++)
        BOOST_TEST(map.get_id_area(i) == 3);

    for (int i = 0; i <= 1; i++)
        BOOST_TEST(map.get_area_value(i) == 2);
    for (int i = 2; i <= 3; i++)
        BOOST_TEST(map.get_area_value(i) == 1);
    
    BOOST_TEST(map.is_neighbor_square(0, 2) == true);
    BOOST_TEST(map.is_neighbor_square(5, 6) == true);
    BOOST_TEST(map.is_neighbor_square(1, 2) == false);
    BOOST_TEST(map.is_neighbor_square(4, 5) == false);
}


/**
 * @brief test si tous les joueurs sont presents dans la map
 *  // TODO tester si tous les joeurs ont le meme nb de territoire en debut de jeu
 */
BOOST_FIXTURE_TEST_CASE(troops_placing, CreateMap)
{
    std::vector<std::string> players {"p1", "p2", "p3"};
	Map map(1, players);

    std::vector<std::string> players_on_map;
    for (int i = 0; i < (int)map.get_nb_square(); i++)
        players_on_map.push_back(map.get_square_owner(i));

    for (int i = 0; i < (int)players.size(); i++) {
        std::vector<std::string>::iterator it; 
        it = std::find(players_on_map.begin(), players_on_map.end(), players[i]);
        bool b = it != players_on_map.end();
        BOOST_TEST(b);
    }
}


/**
 * @brief test des getters du score des joueurs : get_nb_troops_player(), 
 *  get_nb_area_player(), get_nb_square_player() and get_nb_area_point_player().
 */
BOOST_FIXTURE_TEST_CASE(player_score, CreateMap) 
{
    std::vector<std::string> players {"p1", "p2", "p3"};
	Map map(1, players);

    std::vector<uint16_t> area_values;
    std::vector<struct info_square> info_square;
    info_square.resize(map.get_nb_square());
    area_values.resize(map.get_nb_area());

    for (int i = 0; i < map.get_nb_square(); i++) {
        info_square[i].id_region = map.get_id_area(i);
        info_square[i].nb_troops = map.get_nb_troops_square(i);
        info_square[i].player_tag = map.get_square_owner(i);
    }

    for (int i = 0; i < map.get_nb_area(); i++)
        area_values[i] = map.get_area_value(i);

    uint16_t p_data[3] = {0, 0, 0};

    // check nb troops test
    for (int i = 0; i < map.get_nb_square(); i++) {
        int a = map.get_square_owner(i)[1] - '0' - 1;
        p_data[a] += map.get_nb_troops_square(i);
    }

    for (int i = 0; i < (int)players.size(); i++)
        BOOST_TEST(p_data[i] == map.get_nb_troops_player(players[i]));


    // check nb area test
    for (int k = 0; k < (int)players.size(); k++) {
        p_data[k] = 0;
        std::vector<bool> occupied_area {};
        occupied_area.assign(map.get_nb_area(), true);
        
        for (int i = 0; i < map.get_nb_square(); i++)
        {
            if ((info_square[i].player_tag.compare(players[k])) 
                || (!info_square[i].player_tag.compare(players[k]) && info_square[i].nb_troops < 1))
                    occupied_area[info_square[i].id_region] = false;
        }

        for (int i = 0; i < map.get_nb_area(); i++)
            if (occupied_area[i])
                p_data[k]++;
        
        BOOST_TEST(p_data[k] == map.get_nb_area_player(players[k]));

        // check area point test
        p_data[k] = 0;
        for (int i = 0; i < map.get_nb_area(); i++)
            if (occupied_area[i])
                p_data[k]+=area_values[i];
        
        BOOST_TEST(p_data[k] == map.get_area_points_player(players[k]));
    }
        
    
    // check nb square test
    for (int k = 0; k < (int)players.size(); k++) {
        p_data[k] = 0;
        for (int i = 0; i < map.get_nb_square(); i++)
            if (!info_square[i].player_tag.compare(players[k]))
                p_data[k]++;
        
        BOOST_TEST(p_data[k] == map.get_nb_square_player(players[k]));
    }
}

/**
 * @brief verification du bon fonctionnement de Map::is_possible_transfer()  
 */
BOOST_FIXTURE_TEST_CASE(player_move, CreateMap) {
    std::vector<std::string> players {"p1", "p2", "p3"};
	Map map(1, players);

    for (int i = 0; i < map.get_nb_square(); i++)
        map.set_square_owner(i, "p1");

    BOOST_TEST(map.is_completely_conquered());

    for (int i = 0; i < map.get_nb_square(); i++)
        for (int j = 0; j < map.get_nb_square(); j++) {
            BOOST_TEST(map.is_possible_transfer("p1", i, j));
            BOOST_TEST(map.is_possible_transfer("p2", i, j) == false);
            BOOST_TEST(map.is_possible_transfer("p3", i, j) == false);
        }
    
    map.set_square_owner(0, "p2");
    map.set_square_owner(2, "p2");
    map.set_square_owner(4, "p2");
    BOOST_TEST(map.is_possible_transfer("p2", 0, 2));
    BOOST_TEST(map.is_possible_transfer("p2", 4, 0));
    BOOST_TEST(map.is_possible_transfer("p1", 4, 0) == false);

    BOOST_TEST(map.is_possible_transfer("p2", 2, 6) == false);
    map.set_square_owner(6, "p2");
    BOOST_TEST(map.is_possible_transfer("p2", 2, 6)); 
}

