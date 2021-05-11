#define BOOST_TEST_MODULE Lobby_test
#include <boost/test/included/unit_test.hpp>
#include "logic/lobby.h"
#include "logic/lobbypool.h"
#include "network/lobbypooldispatcher.h"
#include "network/session.h"
#include "logicexception.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

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

BOOST_AUTO_TEST_CASE(test_constructeur_id)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.id() == 5);
}
BOOST_AUTO_TEST_CASE(test_constructeur_id_map)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.get_id_map() == 4);
}

BOOST_AUTO_TEST_CASE(test_constructeur_nb_player)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.get_nb_player() == 3);
}

BOOST_AUTO_TEST_CASE(test_constructeur_sec_by_turn)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.get_sec_by_turn() == 10);
}

BOOST_AUTO_TEST_CASE(test_return_list_player)
{
	Lobby lobby{ 5, param_lobby_one };
	lobby.join(*s1, "Hicheme");
	auto r = lobby.all_players();
	BOOST_TEST(*(r.first) == "Hicheme");
}

BOOST_AUTO_TEST_CASE(test_return_list_sessions)
{
	Lobby lobby{ 5, param_lobby_one };
	lobby.join(*s1, "Hicheme");
	lobby.join(*s2, "Leo");
	auto r = lobby.all_sessions();
	BOOST_ASSERT(*r.first == *s1);
	BOOST_ASSERT(*(++r.first) == *s2);
}

BOOST_AUTO_TEST_CASE(test_ban_player_not_exist)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.ban(*s1, "Matheo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_ban_player_no_right_ban)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Matheo");
		lobby.ban(*s2, "Hicheme");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x15)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_ban_player_ok)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Matheo");
		lobby.ban(*s1, "Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_ban_player_not_exist_in_game)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.ban_in_game("Matheo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_ban_player_in_game_ok)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Matheo");
		lobby.ban_in_game("Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_exit_player_dont_exist)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.exit("Matheo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_exit_player_ok)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Matheo");
		lobby.exit("Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_join_lobby_full)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Leo");
		lobby.join(*s3, "Matheo");
		lobby.join(*s4, "Karim");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x12)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_join_ok)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Leo");
		lobby.join(*s3, "Matheo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x12)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_start_game_not_enough_player)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.start_game(*s1);
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x20)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_start_game_not_right)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Leo");
		lobby.start_game(*s2);
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x21)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_start_game_ok)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.join(*s2, "Leo");
		lobby.start_game(*s1);
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x20)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_get_gamertag_error)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.get_gamertag(*s2);
		BOOST_TEST(false);
	}

	catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_get_gamertag_ok)
{
	try {
		Lobby lobby{ 5, param_lobby_one };
		lobby.join(*s1, "Hicheme");
		lobby.get_gamertag(*s1);
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x16)
			BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(test_parameters)
{
	Lobby lobby{ 5, param_lobby_one };
	BOOST_TEST(lobby.parameters().id_map == param_lobby_one.id_map);
	BOOST_TEST(lobby.parameters().nb_players == param_lobby_one.nb_players);
	BOOST_TEST(lobby.parameters().sec_by_turn == param_lobby_one.sec_by_turn);
}
