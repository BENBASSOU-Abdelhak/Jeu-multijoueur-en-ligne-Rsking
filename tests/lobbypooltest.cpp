#define BOOST_TEST_MODULE LobbyPool_test
#include <boost/test/included/unit_test.hpp>
#include "logic/lobbypool.h"
#include "network/lobbypooldispatcher.h"

#include "network/session.h"
#include "logicexception.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

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


BOOST_FIXTURE_TEST_CASE(test_constructeur_nb_lobby_et_get, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	BOOST_TEST(lp.get_nb_lobby() == 0);
}

BOOST_FIXTURE_TEST_CASE(no_dispo_lobby_no_player_in_lobby, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
	lp.getLobby(id.id()).exit("Hicheme");
	lobby_id_t lobby_id = lp.lobby_dispo(s2, "Matheo");
	BOOST_TEST(lobby_id == 0);
}


BOOST_FIXTURE_TEST_CASE(no_dispo_lobby_no_place, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
	lp.join_lobby(id.id(), s2, "Leo");
	lp.join_lobby(id.id(), s3, "Karim");
	lobby_id_t lobby_id = lp.lobby_dispo(s4, "Matheo");
	BOOST_TEST(lobby_id == 0);
}

BOOST_FIXTURE_TEST_CASE(no_dispo_lobby_game_started, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
	lp.join_lobby(id.id(), s2, "Leo");
	lp.getLobby(id.id()).start_game(s1);
	lobby_id_t lobby_id = lp.lobby_dispo(s3, "Matheo");
	BOOST_TEST(lobby_id == 0);
}

BOOST_FIXTURE_TEST_CASE(no_dispo_lobby_player_ban, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
	lp.join_lobby(id.id(), s2, "Leo");
	lp.join_lobby(id.id(), s3, "Karim");
	lp.getLobby(id.id()).ban(s1, "Karim");
	lobby_id_t lobby_id = lp.lobby_dispo(s1, "Karim");
	BOOST_TEST(lobby_id == 0);
}

BOOST_FIXTURE_TEST_CASE(dispo_lobby_ok, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
	lp.join_lobby(id.id(), s2, "Leo");
	lobby_id_t lobby_id = lp.lobby_dispo(s1, "Karim");
	BOOST_TEST(lobby_id != 0);
}

BOOST_FIXTURE_TEST_CASE(test_getlobby, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
	BOOST_TEST(lp.getLobby(id.id()).id() == id.id());
}

BOOST_FIXTURE_TEST_CASE(test_getlobby_const, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	LobbyPool const& lp_deux = LobbyPool::get();
	Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
	BOOST_TEST(lp_deux.getLobby(id.id()).id() == id.id());
}

BOOST_FIXTURE_TEST_CASE(test_constructeur_max_lobby, CreateMap)
{
	LobbyPool& lp = LobbyPool::get();
	BOOST_TEST(lp.get_max_lobby() == MAX_LOBBY);
}

BOOST_FIXTURE_TEST_CASE(test_create_lobby_ok, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		lp.create_lobby(s1, "Hicheme", param_lobby_one);
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x17)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_join_lobby_id__less_0_error, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		lp.create_lobby(s1, "Hicheme", param_lobby_one);
		lp.join_lobby(-1, s2, "Leo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x11)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_join_lobby_id_false_error, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
		lp.join_lobby(id.id() + 2, s2, "Leo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x11)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_join_lobby_id_ok, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
		lp.join_lobby(id.id(), s2, "Leo");
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x11)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_join_lobby_game_launched_cant_join, CreateMap)
{
	create_map();
	try {
		LobbyPool& lp = LobbyPool::get();
		Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
		lp.join_lobby(id.id(), s2, "Leo");
		lp.getLobby(id.id()).start_game(s1);
		lp.join_lobby(id.id(), s3, "Karim");
		remove_map();
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x13)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_join_lobby_player_is_banned, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
		lp.join_lobby(id.id(), s2, "Leo");
		lp.getLobby(id.id()).ban(s1, "Leo");
		lp.join_lobby(id.id(), s2, "Leo");
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x14)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_destroy_lobby_ok, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);
		lp.getLobby(id.id()).ban(s1, "Hicheme");
		lp.destroy_lobby(id.id());
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x11)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_destroy_lobby_bad_id, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		Lobby& id = lp.create_lobby(s1, "Hicheme", param_lobby_one);

		lp.destroy_lobby(id.id() + 2);
		BOOST_TEST(true);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x11)
			BOOST_TEST(false);
	}
}

BOOST_FIXTURE_TEST_CASE(test_create_lobby_not_right_max_lobby_atteint, CreateMap)
{
	try {
		LobbyPool& lp = LobbyPool::get();
		for (int i = 0; i < MAX_LOBBY; i++) {
			lp.create_lobby(s1, "Hicheme", param_lobby_one);
		}
		lp.create_lobby(s2, "Hicheme", param_lobby_one);
		BOOST_TEST(false);
	} catch (const LogicException& e) {
		if (e.subcode() != 0x17)
			BOOST_TEST(false);
	}
}
