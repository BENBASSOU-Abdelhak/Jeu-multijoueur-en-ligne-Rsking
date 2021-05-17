#define BOOST_TEST_MODULE player_id
#include <boost/test/included/unit_test.hpp>

#include "logic/game.h"
#include "logic/gameparameters.h"
#include "logic/lobby.h"
#include "network/lobbydispatcher.h"
#include "network/lobbypooldispatcher.h"
#include "logic/lobbypool.h"
#include "logic/lobby.h"

#include "network/session.h"
#include "logicexception.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <filesystem>

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

	fd_map.close();
}

void remove_map()
{
	std::remove("1");
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

Map& Game::get_map()
{
	return m_map;
}

void Game::set_current_player(uint16_t i)
{
	m_i_current_player = i;
}

/******************************************************************************/

/**
 * @brief verification des methodes Game::player_id()
 */
BOOST_FIXTURE_TEST_CASE(get_player_id, CreateMap)
{
	GameParameters gp;
	gp.id_map = 1;
	gp.nb_players = 3;

	Lobby l(0, gp);
	l.join(s1, "p1");
	l.join(s2, "p2");
	l.join(s3, "p3");
	Game t_game(gp, l);

	BOOST_TEST(t_game.player_id(t_game.get_current_player()) == 0);
	BOOST_TEST(t_game.player_id(t_game.get_current_player().get_tag()) == 0);

	t_game.set_current_player(2);

	BOOST_TEST(t_game.player_id(t_game.get_current_player()) == 2);
	BOOST_TEST(t_game.player_id(t_game.get_current_player().get_tag()) == 2);
}
