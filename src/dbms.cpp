#include <boost/test/tools/old/interface.hpp>
#include "dbms.h"
#include "logic/game.h"

void DBMS::login(const std::string& connect_str)
{
	// Multi-threading support
	otl_connect::otl_initialize(1);
	// Try to make a connection (you should handle exceptions)
	// Set auto_commit to 1 (its seems that there are no way to commit else with MySQL/MariaDB)
	m_con.rlogon(connect_str.c_str(), 1);
}

void DBMS::logout()
{
	m_con.logoff();
}

/**
 * Vérifier si un joueur peut jouer ou non. Un joueur peut jouer s'il n'est actuellement pas banni.
 *
 * @param gamertag Le nom d'utilisateur du joueur
 * @return Un boolean pour indiquer si oui (true) ou non (false) il peut jouer
 * @throw otl_exception Si la requête SQL n'a pas pu être exécutée correctement
 */
bool DBMS::can_join(std::string const& gamertag)
{
	otl_stream i(2, "SELECT id FROM ban WHERE user_id = (SELECT id FROM user WHERE gamertag = :f1<char[46]>)",
		     m_con);
	i << gamertag.c_str();

	// EOF <=> 1 => utilisateur non banni, 0 s'il est banni (et ne peut donc pas rejoindre)
	return i.eof();
}

/**
 * Bannir un utilisateur. La durée du bannissement est gérée dynamiquement par la procédure SQL derrière cette fonction.
 *
 * @param gamertag Le nom d'utilisateur du joueur
 * @param reason La raison du bannissement
 * @return Un boolean pour indiquer si le bannissement a correctement été insérée (true). Si le gamertag ne correspond à aucun utilisateur, false sera retourné
 * @throw otl_exception Si la requête SQL n'a pas pu être exécutée correctement
 */
bool DBMS::ban(std::string const& gamertag, std::string const& reason)
{
	// Utilisons notre fonction pour insérer automatiquement le bannissement avec la bonne durée
	otl_stream i(1, "SELECT ban (:f1<char[46]>, :f2<char[256]>)", m_con);
	i << gamertag.c_str() << reason.c_str();

	// Store function result (functions always return one result)
	otl_long_string ban_uid{ 16 };
	i >> ban_uid;

	// Le joueur a été banni si le ban UID n'est pas null
	return !i.is_null();
}

/**
 * Sauvegarder une partie dans la base de données (incluant la partie et les données de tous les joueurs).
 *
 * @param game La partie à sauvegarder
 * @return Un boolean pour indiquer si la partie ainsi que les données de tous les joueurs ont été insérée. Si au moins une donnée n'a pas été insérée, false sera retourné
 * @throw otl_exception Si la requête SQL n'a pas pu être exécutée correctement
 */
__attribute__((weak)) bool DBMS::add_game(Game& game)
{
	unsigned long insert_count{ 0 };
	otl_long_string game_uid{ 16 }; // Store game UID

	// Création de la partie en elle même
	otl_stream i(1, "SELECT create_game (:f1<unsigned>)", m_con);
	i << static_cast<unsigned int>(game.m_eliminated_players.size() + 1);
	i >> game_uid;

	// Insertion des données de tous les joueurs morts
	unsigned int rank{ static_cast<unsigned int>(game.m_eliminated_players.size()) + 1 };
	for (const auto& player : game.m_eliminated_players) {
		otl_stream oi(
			1,
			"INSERT INTO game_has_user (rank, game_id, user_id) VALUE (:f1<unsigned>, :f2<raw[16]>, (SELECT id FROM user WHERE gamertag = :f3<char[46]>))",
			m_con);
		oi << rank << game_uid << player.get_tag().c_str();
		insert_count += oi.get_rpc();
		--rank;
	}
	// Insertion des données du gagnant
	otl_stream oi(
		1,
		"INSERT INTO game_has_user (rank, game_id, user_id) VALUE (:f1<unsigned>, :f2<raw[16]>, (SELECT id FROM user WHERE gamertag = :f3<char[46]>))",
		m_con);
	oi << rank << game_uid << game.winner().c_str();
	insert_count += oi.get_rpc();

	return insert_count == game.m_eliminated_players.size() + 1;
}