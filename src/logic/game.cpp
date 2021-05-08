#include "logic/game.h"
#include "logic/lobby.h"

__attribute__((weak)) Game::Game(GameParameters const& params, Lobby& lobby)
	: m_phase(Placement), m_i_current_player(0), m_last_dead(""), m_lobby(lobby), m_waiting_transfer(false)
{
	std::vector<std::string> tags;
	auto ret = lobby.all_players();
	for (auto it = ret.first; it != ret.second; ++it) {
		tags.push_back(*(it));
	}

	Map map(params.id_map, tags);
	m_map = Map(params.id_map, tags);

	Dice_roll dr{};
	m_dices = dr;

	for (int i = 0; i < (int)tags.size(); i++) {
		Player p = Player(tags[i]);
		p.set_nb_troops(m_map.get_nb_troops_player(tags[i]));
		p.set_nb_square(m_map.get_nb_square_player(tags[i]));
		p.set_nb_area(m_map.get_nb_area_player(tags[i]));
		p.set_area_points(m_map.get_area_points_player(tags[i]));

		if (i == 0)
			p.set_remaining_deploy_troops(p.get_nb_square() / 3 + p.get_area_points());

		m_players.push_back(p);
	}
}

__attribute__((weak)) void Game::add_troops(Session const& player_asking, uint16_t dst_square, uint16_t nb_troops)
{
	if (get_current_player().get_tag().compare(lobby().get_gamertag(player_asking)))
		throw LogicException{ 0x70, "Ce n’est pas votre tour" };

	if (is_finished())
		throw LogicException{ 0x22, "La partie est finie" };

	if (current_phase() != Placement)
		throw LogicException{ 0x30, "Mauvaise phase de jeu pour deployer des troupes" };

	if (nb_troops <= 0)
		throw LogicException{ 0x41, "Nombre de troupe nulle" };

	if (get_square_owner_map(dst_square) != get_current_player())
		throw LogicException{ 0x40, "La case n'est pas au joueur" };

	if (nb_troops > troop_gained())
		throw LogicException{ 0x41, "Pas assez de troupes : Le joueur ne possede pas autant de troupes" };

	if (nb_troops > get_current_player().get_remaining_deploy_troops())
		throw LogicException{ 0x41, "Pas assez de troupes : Le joueur ne lui reste pas autant de troupes" };

	get_current_player().set_remaining_deploy_troops(get_current_player().get_remaining_deploy_troops() -
							 nb_troops);
	add_troops_map(dst_square, nb_troops);
}

__attribute__((weak)) atk_result Game::attack(Session const& player_asking, uint16_t src_square, uint16_t dst_square,
					      uint16_t nb_troops)
{
	if (get_current_player().get_tag().compare(lobby().get_gamertag(player_asking)))
		throw LogicException{ 0x70, "Ce n’est pas votre tour" };

	if (is_finished())
		throw LogicException{ 0x22, "La partie est finie" };

	if (current_phase() != Attack)
		throw LogicException{ 0x30, "Mauvaise phase de jeu pour attaquer" };

	if (get_square_owner_map(src_square) != get_current_player())
		throw LogicException{ 0x50, "Mauvaise case d'origine : La case n'appartient pas au joueur" };

	if (get_square_owner_map(dst_square) == get_current_player())
		throw LogicException{ 0x51, "Mauvaise case de destination : La case apprtient au joueur" };

	if (!m_map.is_neighbor_square(src_square, dst_square))
		throw LogicException{ 0x51, "La case destination n'est pas voisine" };

	if (m_map.get_nb_troops_square(src_square) < 2)
		throw LogicException{ 0x50, "Mauvais case d'origine : Pas assez de troupes pour attaquer" };

	if (nb_troops < 1 || nb_troops > 3)
		throw LogicException{ 0x52, "Nombre de troupes erroné : Le nombre de troupes donné n'est pas bon" };

	if (nb_troops >= m_map.get_nb_troops_square(src_square))
		throw LogicException{ 0x52, "Nombre de troupes erroné : Pas assez de troupes sur la case" };

	struct atk_result result;
	result.defender_loose_game = false;
	result.nb_lost_troops_from_defender = 0;
	result.nb_lost_troops_from_attacker = 0;

	uint16_t nb_opponents_troops = ((int)m_map.get_nb_troops_square(dst_square) > 1) ? 2 : 1;

	m_dices.set_dice_values();

	for (int i = 0; i < nb_troops; i++)
		result.attackers_dice.push_back(m_dices.get_attackers_values(i));
	for (int i = 0; i < nb_opponents_troops; i++)
		result.defenders_dice.push_back(m_dices.get_defenders_values(i));

	for (int i = 0; i < std::min(nb_opponents_troops, nb_troops); i++) {
		if (m_dices.get_defenders_values(i) >= m_dices.get_attackers_values(i))
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
			m_last_dead = beaten_player;
		}

		get_square_owner_map(dst_square).set_nb_square(get_square_owner_map(dst_square).get_nb_square() - 1);
		get_square_owner_map(src_square).set_nb_square(get_square_owner_map(src_square).get_nb_square() + 1);

		m_map.set_square_owner(dst_square, get_current_player().get_tag());

		get_square_owner_map(src_square)
			.set_nb_area(m_map.get_nb_area_player(get_square_owner_map(src_square).get_tag()));
		get_square_owner_map(src_square)
			.set_area_points(m_map.get_area_points_player(get_square_owner_map(src_square).get_tag()));

		get_player_by_tag(beaten_player).set_area_points(m_map.get_area_points_player(beaten_player));
		get_player_by_tag(beaten_player).set_nb_area(m_map.get_nb_area_player(beaten_player));

		remove_troops_map(src_square, 1);
		add_troops_map(dst_square, 1);
	}

	return result;
}

__attribute__((weak)) void Game::transfer(Session const& player_asking, uint16_t src_square, uint16_t dst_square,
					  uint16_t nb_troops)
{
	if (m_waiting_transfer)
		transfer_after_attack(player_asking, src_square, dst_square, nb_troops);
	else {
		if (get_current_player().get_tag().compare(lobby().get_gamertag(player_asking)))
			throw LogicException{ 0x70, "Ce n’est pas votre tour" };

		if (is_finished())
			throw LogicException{ 0x22, "La partie est finie" };

		if (current_phase() != Transfer)
			throw LogicException{ 0x30, "Mauvaise phase de jeu pour transferer des troupes" };

		if (get_current_player().is_already_transfered())
			throw LogicException{ 0x63, "Transfert de troupes deja effectué" };

		if (!m_map.is_valid_square(src_square))
			throw LogicException{ 0x60, "Mauvaise case d'origine : ID case d'origine innexistant" };

		if (get_square_owner_map(src_square) != get_current_player())
			throw LogicException{ 0x60,
					      "Mauvaise case d'origine : Case d'origine n'appartient pas au joueur" };

		if (!m_map.is_valid_square(dst_square))
			throw LogicException{ 0x61, "Mauvaise case de destination : ID case destination innexistant" };

		if (get_square_owner_map(dst_square) != get_current_player())
			throw LogicException{ 0x60,
					      "Mauvaise case d'origine : Case destination n'appartient pas au joueur" };

		if (!m_map.is_possible_transfer(get_current_player().get_tag(), src_square, dst_square))
			throw LogicException{
				0x61, "Mauvaise case de destination : Aucun chemin entre case d'origine et destination"
			};

		if (src_square == dst_square)
			throw LogicException{ 0x61,
					      "Mauvaise case de destination : Meme case d'origine et de destination" };

		if (nb_troops >= m_map.get_nb_troops_square(src_square))
			throw LogicException{ 0x62,
					      "Nombre de troupes erroné : Case d'origine n'a pas assez de troupe" };

		if (nb_troops == 0)
			throw LogicException{ 0x62, "Nombre de troupes erroné : Nombre de troupe nul" };

		get_current_player().set_already_transfered(true);
		remove_troops_map(src_square, nb_troops);
		add_troops_map(dst_square, nb_troops);
	}
}

__attribute__((weak)) void Game::skip(Session const& player_asking)
{
	if (get_current_player().get_tag().compare(lobby().get_gamertag(player_asking)))
		throw LogicException{ 0x70, "Ce n’est pas votre tour" };

	if (m_phase == Placement && get_current_player().get_remaining_deploy_troops() > 0)
		throw LogicException{ 0x71, "Phase non passable : Le joueur n'a pas deployé toutes ses troupes" };

	m_waiting_transfer = false;

	if (m_phase == Placement)
		m_phase = Attack;
	else if (m_phase == Attack)
		m_phase = Transfer;
	else {
		m_phase = Placement;
		set_next_current_player();
	}
}

void Game::player_quit(Session const& player_asking, std::string const& gamertag)
{
	bool find = false;
	auto ret = lobby().all_players();

	for (auto it = ret.first; it != ret.second; ++it)
		if (!lobby().get_gamertag(player_asking).compare(*(it)))
			find = true;

	if (!find)
		throw LogicException{ 0x16, "Le joueur n'existe pas dans le lobby" };

	find = false;
	for (auto p : m_players)
		if (!p.get_tag().compare(gamertag))
			find = true;

	if (!find)
		throw LogicException{ 0x16, "Le joueur n'existe pas dans la game" };

	get_player_by_tag(gamertag).set_disconnect();
	m_last_dead = get_player_by_tag(gamertag).get_tag();
	// TODO prevenir que un joueur a quitté

	if (get_current_player() == get_player_by_tag(gamertag)) {
		m_phase = Placement;
		set_next_current_player();
	}
}

__attribute__((weak)) bool Game::is_finished() const
{
	return nb_alive() < 2;
}

Player& Game::get_current_player()
{
	return m_players[m_i_current_player];
}

void Game::set_next_current_player()
{
	uint16_t previous_i_player = m_i_current_player;
	uint16_t check = m_players.size() + 1;

	do {
		m_i_current_player = (m_i_current_player + 1) % m_players.size();
		check--;
	} while (!get_current_player().is_online() && check > 0);

	// plus aucun joueur dans la partie
	if (check == 0)
		throw LogicException{ 0x22, "La partie est finie, plus aucun joueur dans la partie" };

	// c'est le seul joueur restant
	if (m_i_current_player == previous_i_player)
		throw LogicException{ 0x22, "La partie est finie" };

	//on initialise les attributs du nouveau joueur courrant
	get_current_player().set_already_transfered(false);
	get_current_player().set_remaining_deploy_troops(troop_gained());
	get_current_player().set_last_atk_square(0);
	get_current_player().set_square_from_last_atk(0);
}

__attribute__((weak)) uint16_t Game::troop_gained()
{
	return (get_current_player().get_nb_square() / 3) + (get_current_player().get_area_points());
}

__attribute__((weak)) std::string const& Game::winner()
{
	for (uint16_t i = 0; i < (int)m_players.size(); i++)
		if (m_players.at(i).is_online() && m_players.at(i).get_nb_square() != 0)
			return get_square_owner_map(i).get_tag();
	return get_current_player().get_tag();
}

void Game::add_troops_map(uint16_t dst_square, uint16_t nb_troops)
{
	m_map.add_troops(dst_square, nb_troops);
	get_square_owner_map(dst_square).set_nb_troops(get_square_owner_map(dst_square).get_nb_troops() + nb_troops);
}

void Game::remove_troops_map(uint16_t dst_square, uint16_t nb_troops)
{
	m_map.remove_troops(dst_square, nb_troops);
	get_square_owner_map(dst_square).set_nb_troops(get_square_owner_map(dst_square).get_nb_troops() - nb_troops);
}

Player& Game::get_square_owner_map(uint16_t square)
{
	std::string tag = m_map.get_square_owner(square);
	int i = std::distance(m_players.begin(), std::find(m_players.begin(), m_players.end(), tag));
	return m_players[i];
}

__attribute__((weak)) std::string const& Game::current_player() const
{
	return m_players[m_i_current_player].get_tag();
}

__attribute__((weak)) Gamephase Game::current_phase() const
{
	return m_phase;
}

__attribute__((weak)) size_t Game::nb_alive() const
{
	uint16_t cpt{ 0 };
	for (auto p : m_players)
		if (p.is_online())
			cpt++;

	return cpt;
}

__attribute__((weak)) std::string const& Game::last_dead() const
{
	return m_last_dead;
}

__attribute__((weak)) Lobby& Game::lobby() const
{
	return m_lobby;
}

Player& Game::get_player_by_tag(std::string tag)
{
	for (int i = 0; i < (int)m_players.size(); i++)
		if (!m_players[i].get_tag().compare(tag))
			return m_players[i];

	std::fprintf(stderr, "joueur non trouvé par get_player_by_tag()\n");
	exit(1);
}

uint8_t Game::player_id(Player& player) const
{
	for (uint8_t i = 0; i < m_players.size(); i++)
		if (m_players[i] == player)
			return i;
	std::fprintf(stderr, "joueur non trouvé par player_id()\n");
	exit(1);
}

uint8_t Game::player_id(std::string const& player) const
{
	for (uint8_t i = 0; i < m_players.size(); i++)
		if (m_players[i] == player)
			return i;
	std::fprintf(stderr, "joueur non trouvé par player_id()\n");
	exit(1);
}

void Game::transfer_after_attack(Session const& player_asking, uint16_t src_square, uint16_t dst_square,
				 uint16_t nb_troops)
{
	if (get_current_player().get_tag().compare(lobby().get_gamertag(player_asking)))
		throw LogicException{ 0x70, "Ce n’est pas votre tour" };

	if (get_square_owner_map(src_square).get_last_atk_square() != dst_square)
		throw LogicException{ 0x61, "Ce ne sont pas les territoires de l'attaque (destination)" };

	if (get_square_owner_map(src_square).get_square_from_last_atk() != src_square)
		throw LogicException{ 0x60, "Ce ne sont pas les territoires de l'attaque (origine)" };

	if (is_finished())
		throw LogicException{ 0x22, "La partie est finie" };

	if (current_phase() != Attack)
		throw LogicException{ 0x30, "Mauvaise phase de jeu pour attaquer" };

	if (get_square_owner_map(src_square) != get_current_player())
		throw LogicException{ 0x60, "Mauvaise case d'origine : La case n'appartient pas au joueur" };

	if (get_square_owner_map(dst_square) != get_current_player())
		throw LogicException{ 0x61, "Mauvaise case de destination : La case n'appartient pas au joueur" };

	if (!m_map.is_neighbor_square(src_square, dst_square))
		throw LogicException{ 0x61, "La case destination n'est pas voisine" };

	if (m_map.get_nb_troops_square(src_square) <= nb_troops)
		throw LogicException{ 0x62, "Pas assez de troupe" };

	remove_troops_map(src_square, nb_troops);
	add_troops_map(dst_square, nb_troops);
	m_waiting_transfer = false;
}

__attribute__((weak)) Map const& Game::get_map() const
{
	return m_map;
}

__attribute__((weak)) uint16_t Game::time_left() const
{ //TODO
	return 1;
}
