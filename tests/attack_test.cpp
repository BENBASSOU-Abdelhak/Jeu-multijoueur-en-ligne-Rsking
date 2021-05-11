/**
 * @brief fichier de test du bon fonctionnement de Game::attack() (tests realisé avec 
 *	Game::attack_test(*s1, ) pour pallier au pb de generation aleatoires des valeurs
 * 	de lancé de dé)
 */

#define BOOST_TEST_MODULE attack_test
#include <boost/test/included/unit_test.hpp>

#include "logic/game.h"
#include "logic/gameparameters.h"
#include "logic/lobby.h"
#include "network/lobbydispatcher.h"
#include "network/lobbypooldispatcher.h"
#include "logic/lobbypool.h"
#include "logic/lobby.h"

boost::asio::io_context ctx{ 1 };
auto s1 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ ctx },
std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));
auto s2 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ ctx },
std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));
auto s3 = std::make_shared<Session>(boost::asio::ip::tcp::socket{ ctx },
std::make_unique<LobbyPoolDispatcher>(LobbyPool::get()));

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

void Player::reset_rem_troops() {
	m_remaining_deploy_troops = 0;
}

void Player::set_connected() {
	m_is_online = true;
}

Map& Game::get_map() {
	return m_map;
}

void Game::set_square_owner_map(uint16_t square, std::string new_owner) {
	
	m_map.set_square_owner(square, new_owner);
	get_player_by_tag(new_owner).set_nb_square(m_map.get_nb_square_player(new_owner));
	get_player_by_tag(new_owner).set_nb_area(m_map.get_nb_area_player(new_owner));
	get_player_by_tag(new_owner).set_area_points(m_map.get_area_points_player(new_owner));
}

void Game::maj_score_player(std::string player) {
	get_player_by_tag(player).set_nb_square(m_map.get_nb_square_player(player));
	get_player_by_tag(player).set_nb_area(m_map.get_nb_area_player(player));
	get_player_by_tag(player).set_area_points(m_map.get_area_points_player(player));
    get_player_by_tag(player).set_nb_troops(m_map.get_nb_troops_player(player));
}


Player& Game::get_player_by_id(int i) {
    return m_players[i];
}

atk_result Game::attack_test(Session const& player_asking, uint16_t src_square, uint16_t dst_square,
    uint16_t nb_troops, int d1, int d2, int d3, int d4, int d5) 
{
	if (get_current_player().get_tag().compare(lobby().get_gamertag(player_asking)))
        throw LogicException{0x70, "Ce n’est pas votre tour"};

    if (is_finished())
        throw LogicException{0x22, "La partie est finie"};

    if (current_phase() != Attack)
        throw LogicException{0x30, "Mauvaise phase de jeu pour attaquer"};
    
    if (get_square_owner_map(src_square) != get_current_player())
        throw LogicException{0x50, 
        "Mauvaise case d'origine : La case n'appartient pas au joueur"};
    
    if (get_square_owner_map(dst_square) == get_current_player())
        throw LogicException{0x51,
        "Mauvaise case de destination : La case apprtient au joueur"};
    
    if (!m_map.is_neighbor_square(src_square, dst_square))
        throw LogicException{0x51, "La case destination n'est pas voisine"};

    if (m_map.get_nb_troops_square(src_square) < 2)
        throw LogicException{0x50, 
        "Mauvais case d'origine : Pas assez de troupes pour attaquer"};
        
    if (nb_troops < 1 && nb_troops > 3)
        throw LogicException{0x52,
        "Nombre de troupes erroné : Le nombre de troupes donné n'est pas bon"};
    
    if (nb_troops >= m_map.get_nb_troops_square(src_square))
        throw LogicException{0x52,
        "Nombre de troupes erroné : Pas assez de troupes sur la case"};
    
    struct atk_result result;
    result.defender_loose_game = false;
    result.nb_lost_troops_from_defender = 0;
    result.nb_lost_troops_from_attacker = 0;

    uint16_t nb_opponents_troops
        =((int)m_map.get_nb_troops_square(dst_square) > 1) ? 2 : 1;

    //m_dices.set_dice_values();
    /*for (int i = 0; i < nb_troops; i++)
        result.attackers_dice.push_back(m_dices.get_attackers_values(i));
    for (int i = 0; i < nb_opponents_troops; i++)
        result.defenders_dice.push_back(m_dices.get_defenders_values(i));*/
    
    result.attackers_dice.push_back(d1);
    result.attackers_dice.push_back(d2);
    result.attackers_dice.push_back(d3);
    result.defenders_dice.push_back(d4);
    result.defenders_dice.push_back(d5);
    
    for (int i = 0; i < std::min(nb_opponents_troops, nb_troops); i++)
    {
        /*if (m_dices.get_defenders_values(i) >= m_dices.get_attackers_values(i))
            result.nb_lost_troops_from_attacker++;
        else
            result.nb_lost_troops_from_defender++;*/
        
        if (result.defenders_dice[i] >= result.attackers_dice[i])
            result.nb_lost_troops_from_attacker++;
        else
            result.nb_lost_troops_from_defender++;
    }

    remove_troops_map(src_square, result.nb_lost_troops_from_attacker);
    remove_troops_map(dst_square, result.nb_lost_troops_from_defender);
    result.square_conquered = m_map.get_nb_troops_square(dst_square) < 1;
    
    if (result.square_conquered) {
        m_waiting_transfer = true;
		get_current_player().set_square_from_last_atk(src_square);
        get_current_player().set_last_atk_square(dst_square);
		
		std::string beaten_player = get_square_owner_map(dst_square).get_tag();
        // si l'adversaire a perdu la partie
        if (get_player_by_tag(beaten_player).get_nb_square() == 1) {
            get_player_by_tag(beaten_player).set_disconnect();

            result.defender_loose_game = true;
            mark_player_as_eliminated(get_player_by_tag (beaten_player));
        }

        get_square_owner_map(dst_square).set_nb_square
            (get_square_owner_map(dst_square).get_nb_square() - 1);
        get_square_owner_map(src_square).set_nb_square
            (get_square_owner_map(src_square).get_nb_square() + 1);
        
        m_map.set_square_owner(dst_square, get_current_player().get_tag());

        get_square_owner_map(src_square).set_nb_area(
            m_map.get_nb_area_player( get_square_owner_map(src_square).get_tag()));
        get_square_owner_map(src_square).set_area_points(
            m_map.get_area_points_player( get_square_owner_map(src_square).get_tag()));

        get_player_by_tag(beaten_player).set_area_points(
            m_map.get_area_points_player(beaten_player));
        get_player_by_tag(beaten_player).set_nb_area(
            m_map.get_nb_area_player(beaten_player));
        
        
        remove_troops_map(src_square, 1);
        add_troops_map(dst_square, 1);
    }
        
    return result;
}
/******************************************************************************/


/**
 * @brief test des verifications preliminaires avant une attaque
 */
BOOST_FIXTURE_TEST_CASE(before_attack, CreateMap) {
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
	t_game.maj_score_player("p1");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");
	t_game.get_current_player().set_remaining_deploy_troops(t_game.troop_gained());	
	t_game.add_troops(*s1, 0, 3-1);
	t_game.get_map().remove_troops(1, 1);
	t_game.maj_score_player("p1");
	
	/*** etat actuel ***/
	// p1 : territoire 0 à 2; p1 : territoire 3 à 5; p3 : territoire 6 à 8
	// territoire 0 contient 5 troupes et tous les autres 2 troupes


	// pas la bonne phase
	try {
		t_game.attack_test(*s1, 0, 4, 1, 1, 1, 1, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) { BOOST_TEST(true);};

	//t_game.skip(*s1);
	t_game.add_troops(*s1, 0, 1);


	// nb de troupe superieur a 3
	try {
		t_game.attack_test(*s1, 0, 4, 4, 1, 1, 1, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) { BOOST_TEST(true);};

	// case origine n'apparient pas au joueur
	try {
		t_game.attack_test(*s1, 8, 4, 1, 1, 1, 1, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) { BOOST_TEST(true);};

	// case dest apparient au joueur
	try {
		t_game.attack_test(*s1, 0, 1, 1, 1, 1, 1, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) { BOOST_TEST(true);};

	// case dest n'est pas voisine
	try {
		t_game.attack_test(*s1, 0, 5, 1, 1, 1, 1, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) { BOOST_TEST(true);};

	// case d'origine ne contient qu'une troupe
	try {
		t_game.attack_test(*s1, 1, 3, 1, 1, 1, 1, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) { BOOST_TEST(true);};

	t_game.get_player_by_tag("p2").set_disconnect();
	t_game.get_player_by_tag("p3").set_disconnect();

	// la partie est finie
	try {
		t_game.attack_test(*s1, 0, 3, 1, 1, 1, 1, 1, 1);
		BOOST_TEST(false);
	} catch (LogicException const& e) { BOOST_TEST(true);};
	
	t_game.get_player_by_tag("p2").set_connected();
	t_game.attack_test(*s1, 0, 3, 3, 6, 6, 6, 5, 5);
}




/**
 * @brief scenario : l'attaquant attaque avec le max de troupe le defenseur aussi,
 * 	tout les dés de l'attaquant sont gagnant
 */
BOOST_FIXTURE_TEST_CASE(attack_1, CreateMap) {
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
	t_game.maj_score_player("p1");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");
	t_game.get_current_player().set_remaining_deploy_troops(t_game.troop_gained());	
	t_game.add_troops(*s1, 0, 3);
	
	/*** etat actuel ***/
	// p1 : territoire 0 à 2; p1 : territoire 3 à 5; p3 : territoire 6 à 8
	// territoire 0 contient 5 troupes et tous les autres 2 troupes


	//t_game.skip(*s1);

	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 5);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 2);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_troops() == 9);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_troops() == 6);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_square() == 3);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_square() == 3);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_area() == 1);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_area() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_area_points() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_area_points() == 0);
	BOOST_TEST(t_game.get_square_owner_map(0).get_tag() == "p1");
	BOOST_TEST(t_game.get_square_owner_map(3).get_tag() == "p2");

	struct atk_result ar;
	ar = t_game.attack_test(*s1, 0, 3, 3, 6, 6, 6, 5, 5);
	BOOST_TEST(ar.square_conquered == true);
	BOOST_TEST(ar.defender_loose_game == false);
	BOOST_TEST(ar.nb_lost_troops_from_attacker == 0);
	BOOST_TEST(ar.nb_lost_troops_from_defender == 2);
	BOOST_TEST(ar.attackers_dice[0] == 6);
	BOOST_TEST(ar.attackers_dice[1] == 6);
	BOOST_TEST(ar.attackers_dice[2] == 6);
	BOOST_TEST(ar.defenders_dice[0] == 5);
	BOOST_TEST(ar.defenders_dice[1] == 5);

	BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 4);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 1);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_troops() == 9);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_troops() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_square() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_square() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_area() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_area() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_area_points() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_area_points() == 0);
	BOOST_TEST(t_game.get_square_owner_map(0).get_tag() == "p1");
	BOOST_TEST(t_game.get_square_owner_map(3).get_tag() == "p1");

	t_game.skip(*s1);
	t_game.skip(*s1);
	t_game.get_player_by_tag("p3").set_disconnect();
	t_game.get_current_player().reset_rem_troops();
	t_game.skip(*s2);
	t_game.skip(*s2);
	t_game.skip(*s2);

	BOOST_TEST(t_game.get_current_player().get_tag() == "p1");
	BOOST_TEST(t_game.current_player() == "p1");
	BOOST_TEST(t_game.player_id(t_game.get_current_player()) == 0);
	BOOST_TEST(t_game.player_id(t_game.get_current_player().get_tag()) == 0);
	BOOST_TEST(t_game.troop_gained() == 5);
	BOOST_TEST(t_game.get_current_player().get_remaining_deploy_troops() == 5);
	BOOST_TEST(t_game.get_map().is_completely_conquered() == false);
}



/**
 * @brief scenario : l'attaquant attaque avec le max de troupe le defenseur aussi,
 * 	tout les dés de l'attaquant sont perdants
 */
BOOST_FIXTURE_TEST_CASE(attack_2, CreateMap) {
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
	t_game.maj_score_player("p1");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");
	t_game.get_current_player().set_remaining_deploy_troops(t_game.troop_gained());
	t_game.add_troops(*s1, 0, 3);
	
	/*** etat actuel ***/
	// p1 : territoire 0 à 2; p1 : territoire 3 à 5; p3 : territoire 6 à 8
	// territoire 0 contient 5 troupes et tous les autres 2 troupes

    //t_game.skip(*s1);

	struct atk_result ar;
	ar = t_game.attack_test(*s1, 0, 3, 3, 5, 5, 5, 5, 5);
	BOOST_TEST(ar.square_conquered == false);
	BOOST_TEST(ar.defender_loose_game == false);
	BOOST_TEST(ar.nb_lost_troops_from_attacker == 2);
	BOOST_TEST(ar.nb_lost_troops_from_defender == 0);
	BOOST_TEST(ar.attackers_dice[0] == 5);
	BOOST_TEST(ar.attackers_dice[1] == 5);
	BOOST_TEST(ar.attackers_dice[2] == 5);
	BOOST_TEST(ar.defenders_dice[0] == 5);
	BOOST_TEST(ar.defenders_dice[1] == 5);

    BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 3);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 2);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_troops() == 7);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_troops() == 6);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_square() == 3);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_square() == 3);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_area() == 1);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_area() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_area_points() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_area_points() == 0);
	BOOST_TEST(t_game.get_square_owner_map(0).get_tag() == "p1");
	BOOST_TEST(t_game.get_square_owner_map(3).get_tag() == "p2");
}



/**
 * @brief scenario : l'attaquant attaque avec le max de troupe le defenseur aussi,
 * 	seulement un dé de l'attaquant est gagnant
 */
BOOST_FIXTURE_TEST_CASE(attack_3, CreateMap) {
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
	t_game.maj_score_player("p1");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");
	t_game.get_current_player().set_remaining_deploy_troops(t_game.troop_gained());
	t_game.add_troops(*s1, 0, 3);
	
	/*** etat actuel ***/
	// p1 : territoire 0 à 2; p1 : territoire 3 à 5; p3 : territoire 6 à 8
	// territoire 0 contient 5 troupes et tous les autres 2 troupes

    //t_game.skip(*s1);

	struct atk_result ar;
	ar = t_game.attack_test(*s1, 0, 3, 3, 6, 5, 5, 5, 5);
	BOOST_TEST(ar.square_conquered == false);
	BOOST_TEST(ar.defender_loose_game == false);
	BOOST_TEST(ar.nb_lost_troops_from_attacker == 1);
	BOOST_TEST(ar.nb_lost_troops_from_defender == 1);
	BOOST_TEST(ar.attackers_dice[0] == 6);
	BOOST_TEST(ar.attackers_dice[1] == 5);
	BOOST_TEST(ar.attackers_dice[2] == 5);
	BOOST_TEST(ar.defenders_dice[0] == 5);
	BOOST_TEST(ar.defenders_dice[1] == 5);

    BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 4);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 1);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_troops() == 8);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_troops() == 5);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_square() == 3);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_square() == 3);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_area() == 1);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_area() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_area_points() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_area_points() == 0);
	BOOST_TEST(t_game.get_square_owner_map(0).get_tag() == "p1");
	BOOST_TEST(t_game.get_square_owner_map(3).get_tag() == "p2");
}


/**
 * @brief scenario : l'attaquant attaque avec le max de troupe le defenseur avec une troupe,
 * 	les dés de l'attaquant sont gagnant
 */
BOOST_FIXTURE_TEST_CASE(attack_4, CreateMap) {
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
    
    
	t_game.maj_score_player("p1");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");
    t_game.get_map().remove_troops(3, 1);
    t_game.maj_score_player("p2");

	t_game.get_current_player().set_remaining_deploy_troops(t_game.troop_gained());
	t_game.add_troops(*s1, 0, 3);
	
	/*** etat actuel ***/
	// p1 : territoire 0 à 2; p1 : territoire 3 à 5; p3 : territoire 6 à 8
	// territoire 0 contient 5 troupes et tous les autres 2 troupes

    //t_game.skip(*s1);

	struct atk_result ar;
	ar = t_game.attack_test(*s1, 0, 3, 3, 6, 5, 5, 5, 5);
	BOOST_TEST(ar.square_conquered == true);
	BOOST_TEST(ar.defender_loose_game == false);
	BOOST_TEST(ar.nb_lost_troops_from_attacker == 0);
	BOOST_TEST(ar.nb_lost_troops_from_defender == 1);
	BOOST_TEST(ar.attackers_dice[0] == 6);
	BOOST_TEST(ar.attackers_dice[1] == 5);
	BOOST_TEST(ar.attackers_dice[2] == 5);
	BOOST_TEST(ar.defenders_dice[0] == 5);
	BOOST_TEST(ar.defenders_dice[1] == 5);

    BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 4);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 1);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_troops() == 9);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_troops() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_square() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_square() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_area() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_area() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_area_points() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_area_points() == 0);
	BOOST_TEST(t_game.get_square_owner_map(0).get_tag() == "p1");
	BOOST_TEST(t_game.get_square_owner_map(3).get_tag() == "p1");
}


/**
 * @brief scenario : l'attaquant attaque avec le max de troupe le defenseur aussi,
 *  le defenseur ne possede qu'un seul territoire
 * 	les dés de l'attaquant sont gagnant
 */
BOOST_FIXTURE_TEST_CASE(attack_5, CreateMap) {
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
	for (int i = 3; i <= 3; i++)
        t_game.set_square_owner_map(i, "p2");
	for (int i = 4; i <= 8; i++)
        t_game.set_square_owner_map(i, "p3");
    
	t_game.maj_score_player("p1");
	t_game.maj_score_player("p2");
	t_game.maj_score_player("p3");

	t_game.get_current_player().set_remaining_deploy_troops(t_game.troop_gained());
	t_game.add_troops(*s1, 0, 3);
	
	/*** etat actuel ***/
	// p1 : territoire 0 à 2; p1 : territoire 3 à 5; p3 : territoire 6 à 8
	// territoire 0 contient 5 troupes et tous les autres 2 troupes

    //t_game.skip(*s1);

    BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 5);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 2);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_troops() == 9);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_troops() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_square() == 3);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_square() == 1);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_area() == 1);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_area() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_area_points() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_area_points() == 0);
	BOOST_TEST(t_game.get_square_owner_map(0).get_tag() == "p1");
	BOOST_TEST(t_game.get_square_owner_map(3).get_tag() == "p2");

	struct atk_result ar;
	ar = t_game.attack_test(*s1, 0, 3, 3, 4, 3, 1, 3, 2);
	BOOST_TEST(ar.square_conquered == true);
	BOOST_TEST(ar.defender_loose_game == true);
	BOOST_TEST(ar.nb_lost_troops_from_attacker == 0);
	BOOST_TEST(ar.nb_lost_troops_from_defender == 2);
	BOOST_TEST(ar.attackers_dice[0] == 4);
	BOOST_TEST(ar.attackers_dice[1] == 3);
	BOOST_TEST(ar.attackers_dice[2] == 1);
	BOOST_TEST(ar.defenders_dice[0] == 3);
	BOOST_TEST(ar.defenders_dice[1] == 2);

    BOOST_TEST(t_game.get_map().get_nb_troops_square(0) == 4);
	BOOST_TEST(t_game.get_map().get_nb_troops_square(3) == 1);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_troops() == 9);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_troops() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_square() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_square() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_nb_area() == 2);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_nb_area() == 0);
	BOOST_TEST(t_game.get_player_by_tag("p1").get_area_points() == 4);
	BOOST_TEST(t_game.get_player_by_tag("p2").get_area_points() == 0);
	BOOST_TEST(t_game.get_square_owner_map(0).get_tag() == "p1");
	BOOST_TEST(t_game.get_square_owner_map(3).get_tag() == "p1");

    BOOST_TEST((t_game.get_player_by_id(1).is_online() == false 
		&& t_game.get_player_by_id(1).get_tag() == "p2"));
    BOOST_TEST(t_game.last_dead() == "p2");

}
