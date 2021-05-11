/*
 * Check wether Boost::Test works and is integrated to cMake
 */
#define BOOST_TEST_MODULE troops_distribution
#include <boost/test/included/unit_test.hpp>

#include <filesystem>
#include "logic/map.h"
#define NB_SQUARE 48
#define DIFF_SQUARE 1 //ecart du nb de territoires autorisé
#define DIFF_TROOPS 1 //ecart du nb de troupes autorisé
#define NB_TEST 20

BOOST_AUTO_TEST_CASE(check_distribution)
{
    char file[5];
    sprintf(file, "%d", 200);
    char oldf[50] = "testmaps/";
    std::strcat(oldf, file);
    std::rename(oldf, file);

    std::vector<std::string> next {"p3", "p4", "p5", "p6"};
    std::vector<std::string> players {"p1", "p2"};
    
    for (int i = 0; i < (int)next.size(); i++) {
        for (int j = 0; j < NB_TEST; j++)
        {
            Map map(200, players);
            for (auto p : players) {
                BOOST_TEST(map.get_area_points_player(p) == 0);    
                BOOST_TEST(map.get_nb_area_player(p) == 0);
                BOOST_TEST((map.get_nb_troops_player(p) >= (((NB_SQUARE*2)/players.size()) - DIFF_TROOPS) 
                    && map.get_nb_troops_player(p) <= (((NB_SQUARE*2)/players.size()) + DIFF_TROOPS)));
                BOOST_TEST((map.get_nb_square_player(p) >= ((NB_SQUARE/players.size()) - DIFF_SQUARE)
                    &&  map.get_nb_square_player(p) <= ((NB_SQUARE/players.size()) + DIFF_SQUARE))); 
            }
        }
        players.push_back(next[i]);
    }
    
        
    std::rename(file, oldf);
}