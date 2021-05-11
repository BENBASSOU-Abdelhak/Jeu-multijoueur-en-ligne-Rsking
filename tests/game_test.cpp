#define BOOST_TEST_MODULE game_test
#include <boost/test/included/unit_test.hpp>

#include "logic/game.h"
#include "logic/gameparameters.h"
#include "logic/lobby.h"
#include "network/lobbydispatcher.h"
#include "network/lobbypooldispatcher.h"
#include "logic/lobbypool.h"
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

/************** Creation outils et environnement pour les tests ***************/
void create_map()
{
	std::string const map_file("1");
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
	std::remove("1");
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

void Player::reset_rem_troops()
{
	m_remaining_deploy_troops = 0;
}

void Player::set_connected()
{
	m_is_online = true;
}

Map& Game::get_map()
{
	return m_map;
}

void Game::set_current_player(uint16_t i)
{
	m_i_current_player = i;
}

void Game::set_square_owner_map(uint16_t square, std::string new_owner)
{
	m_map.set_square_owner(square, new_owner);
	get_player_by_tag(new_owner).set_nb_square(m_map.get_nb_square_player(new_owner));
	get_player_by_tag(new_owner).set_nb_area(m_map.get_nb_area_player(new_owner));
	get_player_by_tag(new_owner).set_area_points(m_map.get_area_points_player(new_owner));
}

void Game::maj_score_player(std::string player)
{
	get_player_by_tag(player).set_nb_square(m_map.get_nb_square_player(player));
	get_player_by_tag(player).set_nb_area(m_map.get_nb_area_player(player));
	get_player_by_tag(player).set_area_points(m_map.get_area_points_player(player));
}

void Map::set_nb_troops(uint16_t square, uint16_t nb_troops)
{
	m_info_square[square].nb_troops = nb_troops;
}
/******************************************************************************/

/**
 * @brief test des fonctions :
 *	get_current_player(), current_phase(), is_finished()
 *	apres des appels de skip(*s1), set_player_left()
 */
BOOST_FIXTURE_TEST_CASE(current_player_and_phase, CreateMap)
{
	/************ setup ****************/
	GameParameters gp;
	gp.id_map = 1;
	gp.nb_players = 3;
	Lobby l{ 1, gp };
	l.join(*s1, "p1");
	l.join(*s2, "p2");
	l.join(*s3, "p3");
	Game t_game(gp, l);
	/*******************/

	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p1"));
	BOOST_CHECK(t_game.current_phase() == 1);
	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s1);
	BOOST_CHECK(t_game.current_phase() == 2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p1"));
	t_game.skip(*s1);
	BOOST_CHECK(t_game.current_phase() == 3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p1"));
	t_game.skip(*s1);
	BOOST_CHECK(t_game.current_phase() == 1);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));
	try {
		t_game.skip(*s2);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s2);
	BOOST_CHECK(t_game.current_phase() == 2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.skip(*s2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.skip(*s2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p3"));
	BOOST_CHECK(t_game.get_current_player().is_already_transfered() == false);
	try {
		t_game.skip(*s3);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p3"));

	t_game.skip(*s3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p3"));

	t_game.skip(*s3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p1"));
	BOOST_CHECK(t_game.current_phase() == 1);
	BOOST_CHECK(t_game.nb_alive() == 3);
	BOOST_CHECK(t_game.is_finished() == false);
	t_game.player_quit(*s1, t_game.get_current_player().get_tag());
	BOOST_CHECK(t_game.current_phase() == 1);
	BOOST_CHECK(t_game.nb_alive() == 2);
	BOOST_CHECK(!t_game.get_player_by_tag("p1").is_online());
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s2);
	BOOST_CHECK(t_game.current_phase() == 2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.skip(*s2);
	BOOST_CHECK(t_game.current_phase() == 3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.skip(*s2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p3"));

	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p3"));

	t_game.skip(*s3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p3"));

	t_game.skip(*s3);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.get_current_player().reset_rem_troops();
	t_game.get_player_by_tag("p3").set_disconnect();
	BOOST_CHECK(t_game.nb_alive() == 1);
	t_game.get_player_by_tag("p1").set_connected();
	BOOST_CHECK(t_game.nb_alive() == 2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));
	BOOST_CHECK(t_game.current_phase() == 1);

	t_game.skip(*s2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.skip(*s2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p2"));

	t_game.skip(*s2);
	BOOST_CHECK(!t_game.get_current_player().get_tag().compare("p1"));
	t_game.get_current_player().set_disconnect();
	BOOST_CHECK(t_game.nb_alive() == 1);
	BOOST_CHECK(t_game.is_finished());
	BOOST_CHECK(!t_game.winner().compare("p2"));
}

/**
 * @brief test du calcul des troupes gagnées : get_troops_gained()
 */
BOOST_FIXTURE_TEST_CASE(troops_gained, CreateMap)
{
	/************ setup ****************/
	GameParameters gp;
	gp.id_map = 1;
	gp.nb_players = 3;
	Lobby l{ 1, gp };
	l.join(*s1, "p1");
	l.join(*s2, "p2");
	l.join(*s3, "p3");
	Game t_game(gp, l);
	/*******************/

	for (int i = 0; i < (int)t_game.get_map().get_nb_square(); i++)
		t_game.get_map().set_square_owner(i, "");

	for (int i = 0; i <= 2; i++)
		t_game.set_square_owner_map(i, "p1");
	for (int i = 3; i <= 5; i++)
		t_game.set_square_owner_map(i, "p2");
	for (int i = 6; i <= 8; i++)
		t_game.set_square_owner_map(i, "p3");
	BOOST_CHECK(t_game.troop_gained() == 3);
	t_game.set_current_player(1);
	BOOST_CHECK(t_game.troop_gained() == 1);
	t_game.set_current_player(2);
	BOOST_CHECK(t_game.troop_gained() == 2);

	t_game.set_square_owner_map(3, "p1");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");

	t_game.set_current_player(0);
	BOOST_CHECK(t_game.troop_gained() == 5);
	t_game.set_current_player(1);
	BOOST_CHECK(t_game.troop_gained() == 0);
	t_game.set_current_player(2);
	BOOST_CHECK(t_game.troop_gained() == 2);

	t_game.set_square_owner_map(7, "p2");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");

	t_game.set_current_player(0);
	BOOST_CHECK(t_game.troop_gained() == 5);
	t_game.set_current_player(1);
	BOOST_CHECK(t_game.troop_gained() == 1);
	t_game.set_current_player(2);
	BOOST_CHECK(t_game.troop_gained() == 0);

	t_game.set_square_owner_map(8, "p2");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");

	t_game.set_current_player(0);
	BOOST_CHECK(t_game.troop_gained() == 5);
	t_game.set_current_player(1);
	BOOST_CHECK(t_game.troop_gained() == 2);
	t_game.set_current_player(2);
	BOOST_CHECK(t_game.troop_gained() == 0);

	t_game.set_square_owner_map(6, "p2");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");

	t_game.set_current_player(0);
	BOOST_CHECK(t_game.troop_gained() == 5);
	t_game.set_current_player(1);
	BOOST_CHECK(t_game.troop_gained() == 3);
	t_game.set_current_player(2);
	BOOST_CHECK(t_game.troop_gained() == 0);
}

/**
 * @brief test du bon fonctionnement de la fonction add_troops()
 */
BOOST_FIXTURE_TEST_CASE(add_troops, CreateMap)
{
	/************ setup ****************/
	GameParameters gp;
	gp.id_map = 1;
	gp.nb_players = 3;
	Lobby l{ 1, gp };
	l.join(*s1, "p1");
	l.join(*s2, "p2");
	l.join(*s3, "p3");
	Game t_game(gp, l);
	/*******************/

	for (int i = 0; i < (int)t_game.get_map().get_nb_square(); i++)
		t_game.get_map().set_square_owner(i, "");
	for (int i = 0; i <= 2; i++)
		t_game.set_square_owner_map(i, "p1");
	for (int i = 3; i <= 5; i++)
		t_game.set_square_owner_map(i, "p2");
	for (int i = 6; i <= 8; i++)
		t_game.set_square_owner_map(i, "p3");
	for (int i = 0; i <= 8; i++) {
		t_game.get_map().set_nb_troops(i, 2);
	}
	t_game.get_player_by_tag("p1").set_remaining_deploy_troops(3);
	t_game.get_player_by_tag("p2").set_remaining_deploy_troops(1);
	t_game.get_player_by_tag("p3").set_remaining_deploy_troops(2);

	/*** etat actuel ***/
	// p1 : troop_gained() == 3
	// p2 : troop_gained() == 1
	// p3 : troop_gained() == 2
	// tout les territoires contiennent 2 troupes

	// case n'appartient au joueur
	for (int i = 3; i <= 8; i++) {
		try {
			t_game.add_troops(*s1, i, 1);
			BOOST_TEST(false);
		} catch (LogicException const& e) {
			BOOST_TEST(true);
		}
	}

	// plus de troupe que le joueur en possede
	try {
		t_game.add_troops(*s1, 0, 4);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 2);
	t_game.add_troops(*s1, 0, 2);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 4);

	// nb troupe nul
	try {
		t_game.add_troops(*s1, 0, 0);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	// pas son tour
	try {
		t_game.add_troops(*s2, 0, 0);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	// le joueur ne possede plus assez de troupe
	try {
		t_game.add_troops(*s1, 0, 2);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	// le joeur essaie de passer a la phase suivante alors qu'il lui reste 1 troupe a placer
	try {
		t_game.skip(*s1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 4);
	t_game.add_troops(*s1, 0, 1);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 5);
	//t_game.skip(*s1);

	// le joeur reessaie de transferer des troupes dans une autre phase
	try {
		t_game.add_troops(*s1, 0, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	t_game.skip(*s1);
	t_game.skip(*s1);

	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 2);
	t_game.add_troops(*s2, 3, 1);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 3);
	//t_game.skip(*s2);
	t_game.skip(*s2);
	t_game.skip(*s2);

	BOOST_TEST(t_game.get_map().get_nb_troops_square(6) == 2);
	t_game.add_troops(*s3, 6, 2);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(6) == 4);
	//t_game.skip(*s3);
	t_game.skip(*s3);
	try {
		t_game.add_troops(*s3, 6, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	try {
		t_game.add_troops(*s3, 0, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	}

	// test que la maj de get_troops_gained() est faite
	t_game.set_square_owner_map(3, "p1");
	t_game.skip(*s3);

	// il ne reste plus que un joueur dans la partie
	t_game.get_player_by_tag("p2").set_disconnect();
	t_game.get_player_by_tag("p3").set_disconnect();
	try {
		t_game.add_troops(*s1, 1, 5);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};
	BOOST_TEST(t_game.winner() == "p1");

	t_game.get_player_by_tag("p2").set_connected();
	BOOST_TEST(t_game.get_map().get_nb_troops_square(1) == 2);
	t_game.add_troops(*s1, 1, 5);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(1) == 7);
	t_game.skip(*s1);
}

/**
 * @brief test du bon fonctionnement de la fonction Game::transfer()
 */
BOOST_FIXTURE_TEST_CASE(transfer_troops, CreateMap)
{
	/************ setup ****************/
	GameParameters gp;
	gp.id_map = 1;
	gp.nb_players = 3;
	Lobby l{ 1, gp };
	l.join(*s1, "p1");
	l.join(*s2, "p2");
	l.join(*s3, "p3");
	Game t_game(gp, l);
	/*******************/

	for (int i = 0; i < (int)t_game.get_map().get_nb_square(); i++)
		t_game.get_map().set_square_owner(i, "");
	for (int i = 0; i <= 2; i++)
		t_game.set_square_owner_map(i, "p1");
	for (int i = 3; i <= 5; i++)
		t_game.set_square_owner_map(i, "p2");
	for (int i = 6; i <= 8; i++)
		t_game.set_square_owner_map(i, "p3");
	for (int i = 0; i <= 8; i++) {
		t_game.get_map().set_nb_troops(i, 2);
	}
	t_game.get_map().add_troops(0, 2);

	/*** etat actuel ***/
	// p1 : territoire 0 à 2; p1 : territoire 3 à 5; p3 : territoire 6 à 8
	// territoire 0 contient 4 troupes et tous les autres 2 troupes

	// mauvaise phase pour transferer
	try {
		t_game.transfer(*s1, 0, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s1);
	t_game.skip(*s1);

	// pas son tour
	try {
		t_game.transfer(*s2, 0, 9, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// case origine innexistante
	try {
		t_game.transfer(*s1, 0, 9, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// case dst innexistante
	try {
		t_game.transfer(*s1, 11, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// mauvaise case de destination de transfert
	try {
		t_game.transfer(*s1, 0, 3, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// mauvaise case d'origine de transfert
	try {
		t_game.transfer(*s1, 4, 2, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// mauvaise case d'origine de transfert
	try {
		t_game.transfer(*s1, 0, 0, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// nb troupe nul
	try {
		t_game.transfer(*s1, 0, 1, 0);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// nb troupe egale a ceux present sur le territoire
	try {
		t_game.transfer(*s1, 0, 1, 4);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// nb de troupe trop grand
	try {
		t_game.transfer(*s1, 0, 1, 5);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 4);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(1) == 2);
	t_game.transfer(*s1, 0, 1, 1);
	// etat : territoire0 : 3, territoire1 : 3, les autres : 2 troupes
	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 3);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(1) == 3);
	for (int i = 2; i <= 8; i++)
		BOOST_TEST(t_game.get_map().get_nb_troops_square(i) == 2);

	// transferer deux fois des troupes dans la meme phase
	try {
		t_game.transfer(*s1, 0, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	t_game.skip(*s1);
	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s2);

	// pas son tour de skip
	try {
		t_game.skip(*s1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};
	t_game.skip(*s2);

	// aucun chemin entre case src et dst
	try {
		t_game.transfer(*s2, 3, 4, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// aucun chemin entre case src et dst
	try {
		t_game.transfer(*s2, 4, 3, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// aucun chemin entre case src et dst
	try {
		t_game.transfer(*s2, 5, 4, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	// aucun chemin entre case src et dst
	try {
		t_game.transfer(*s2, 4, 5, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};

	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 2);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(5) == 2);
	t_game.transfer(*s2, 3, 5, 1);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 1);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(5) == 3);

	t_game.get_player_by_tag("p1").set_disconnect();
	t_game.skip(*s2);
	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s3);
	t_game.skip(*s3);
	t_game.get_player_by_tag("p2").set_disconnect();
	try {
		t_game.transfer(*s3, 6, 7, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) {
		BOOST_TEST(true);
	};
}

BOOST_FIXTURE_TEST_CASE(test_mark_player_as_eliminated, CreateMap)
{
	/************ setup ****************/
	GameParameters gp;
	gp.id_map = 1;
	gp.nb_players = 3;
	Lobby l{ 1, gp };
	l.join(*s1, "p1");
	l.join(*s2, "p2");
	l.join(*s3, "p3");
	Game t_game(gp, l);
	/*******************/

	t_game.player_quit(*s1, "p1");
	BOOST_TEST(t_game.last_dead() == "p1");

	t_game.player_quit(*s2, "p2");
	BOOST_TEST(t_game.last_dead() == "p2");
	BOOST_TEST(t_game.winner() == "p3");
}
