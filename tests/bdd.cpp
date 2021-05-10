/*
 * Test if the Database is working as expected
 */
#define BOOST_TEST_MODULE BDD

#include <boost/test/included/unit_test.hpp>
#include <logic/player.h>
#include <logic/gameparameters.h>
#include <logic/lobby.h>
#include <network/lobbypooldispatcher.h>
#include <logic/lobbypool.h>

#include "dbms.h"

/************** Creation outils et environnement pour les tests ***************/
void create_map ()
{
    std::string const map_file ("1");
    std::ofstream fd_map (map_file.c_str ());
    if (!fd_map) {
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

void remove_map ()
{
    std::remove ("1");
    return;
}

struct CreateMap
{
    CreateMap ()
    {
        create_map ();
    }

    ~CreateMap ()
    {
        remove_map ();
    }
};

BOOST_AUTO_TEST_CASE(establish_database_connection)
{
    try {
        DBMS::get ().login ("DSN=risking-test");
    } catch (const otl_exception &oe) {
        BOOST_FAIL(&oe.msg);
    }
}

BOOST_AUTO_TEST_CASE(can_login_unknown_player)
{
    try {
        BOOST_CHECK (DBMS::get ().can_join ("unknown_name"));
    } catch (const otl_exception &oe) {
        BOOST_FAIL(&oe.msg);
    }
}

BOOST_AUTO_TEST_CASE(can_login_valid_player)
{
    try {
        BOOST_CHECK (DBMS::get ().can_join ("valid1"));
    } catch (const otl_exception &oe) {
        BOOST_FAIL(&oe.msg);
    }
}

BOOST_AUTO_TEST_CASE(create_ban_unknown_player)
{
    try {
        BOOST_CHECK(!DBMS::get ().ban ("unknown_name", "Unit-test"));
    } catch (const otl_exception &oe) {
        BOOST_FAIL(&oe.msg);
    }
}

BOOST_AUTO_TEST_CASE(create_ban_valid_player)
{
    try {
        BOOST_CHECK(DBMS::get ().ban ("valid1", "Unit-test"));
    } catch (const otl_exception &oe) {
        BOOST_FAIL(&oe.msg);
    }
}

BOOST_FIXTURE_TEST_CASE(create_valid_game, CreateMap)
{
    /************ setup ****************/
    boost::asio::io_context ctx{1};
    auto s1 = std::make_shared<Session> (boost::asio::ip::tcp::socket{ctx},
                                         std::make_unique<LobbyPoolDispatcher> (LobbyPool::get ()));
    auto s2 = std::make_shared<Session> (boost::asio::ip::tcp::socket{ctx},
                                         std::make_unique<LobbyPoolDispatcher> (LobbyPool::get ()));

    GameParameters gp;
    gp.id_map = 1;
    gp.nb_players = 2;
    Lobby l{1, gp};
    l.join (*s1, "valid1");
    l.join (*s2, "valid2");
    Game t_game (gp, l);
    /*******************/

    // Elimination de s1
    t_game.player_quit (*s1, "valid1");

    try {
        BOOST_CHECK(DBMS::get ().add_game (t_game));
    } catch (const otl_exception &oe) {
        BOOST_FAIL(&oe.msg);
    }
}

BOOST_AUTO_TEST_CASE(close_database_connection)
{
    try {
        DBMS::get ().logout ();
    } catch (const otl_exception &oe) {
        BOOST_FAIL(&oe.msg);
    }
}