#define BOOST_TEST_MODULE Lobby_test
#include <boost/test/included/unit_test.hpp>
#include "logic/lobby.h"
#include "logic/lobbypool.h"
#include "network/lobbypooldispatcher.h"
#include "network/session.h"
#include "logicexception.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

/************** Creation outils et environnement pour les tests ***************/
void create_map()
{
	std::string const map_file("4");
	std::ofstream fd_map(map_file.c_str());
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

void remove_map()
{
	std::remove("4");
	return;
}

struct CreateMap {
	CreateMap()
	{
		create_map();
	}

	~CreateMap()
	{
		remove_map();
	}
};

boost::asio::io_context ctx{ 1 };
auto s1 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ ctx },
				    std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));
auto s2 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ ctx },
				    std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));
auto s3 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ ctx },
				    std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));
auto s4 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ ctx },
				    std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));

GameParameters param_lobby_one{ 3, 4, 10 };

BOOST_FIXTURE_TEST_CASE(test_constructeur_id, CreateMap)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.id() == 5);
}
BOOST_FIXTURE_TEST_CASE(test_constructeur_id_map, CreateMap)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.get_id_map() == 4);
}

BOOST_FIXTURE_TEST_CASE(test_constructeur_nb_player, CreateMap)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.get_nb_player() == 3);
}

BOOST_FIXTURE_TEST_CASE(test_constructeur_sec_by_turn, CreateMap)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.get_sec_by_turn() == 10);
}

BOOST_FIXTURE_TEST_CASE(test_return_list_player, CreateMap)
{
	Lobby lobby{ 5, param_lobby_one };
	lobby.join(s1, "Hicheme");
	auto r = lobby.all_players();
	BOOST_TEST(*(r.first) == "Hicheme");
}

BOOST_FIXTURE_TEST_CASE(test_return_list_sessions, CreateMap)
{
	Lobby lobby{ 5, param_lobby_one };
	lobby.join(s1, "Hicheme");
	lobby.join(s2, "Leo");
	auto r = lobby.all_sessions();
	BOOST_ASSERT(*(r.first->lock()) == *s1);
	BOOST_ASSERT(*(++r.first)->lock() == *s2);
}

BOOST_FIXTURE_TEST_CASE(test_ban_player_not_exist, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.ban(s1, "Matheo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_ban_player_no_right_ban, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Matheo");
		lobby.ban(s2, "Hicheme");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x15)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_ban_player_ok, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Matheo");
		lobby.ban(s1, "Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_ban_player_not_exist_in_game, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.ban_in_game("Matheo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_ban_player_in_game_ok, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Matheo");
		lobby.ban_in_game("Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_exit_player_dont_exist, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.exit("Matheo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_exit_player_ok, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Matheo");
		lobby.exit("Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_join_lobby_full, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Leo");
		lobby.join(s3, "Matheo");
		lobby.join(s4, "Karim");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x12)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_join_ok, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Leo");
		lobby.join(s3, "Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x12)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_start_game_not_enough_player, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.start_game(s1);
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x20)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_start_game_not_right, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Leo");
		lobby.start_game(s2);
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x21)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_start_game_ok, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.join(s2, "Leo");
		lobby.start_game(s1);
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x20)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_get_gamertag_error, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.get_gamertag(s2);
		BOOST_TEST(false);
	}

	catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_get_gamertag_ok, CreateMap)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(s1, "Hicheme");
		lobby.get_gamertag(s1);
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_parameters, CreateMap)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.parameters().id_map == param_lobby_one.id_map);
	BOOST_TEST(lobby.parameters().nb_players == param_lobby_one.nb_players);
	BOOST_TEST(lobby.parameters().sec_by_turn == param_lobby_one.sec_by_turn);
}
