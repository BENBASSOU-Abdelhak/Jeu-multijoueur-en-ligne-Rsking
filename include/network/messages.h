#ifndef INCLUDE__NETWORK__MESSAGES_H
#define INCLUDE__NETWORK__MESSAGES_H

#include "network/session.h"
#include "logic/lobby.h"
#include "logic/atk_result.h"
#include "logic/game.h"

#include <boost/endian/conversion.hpp>

#include <sstream>

/* Fonctions "simples" d'I/O */
/* Foward declarations */

// Fonction d'arrêt
template <typename T> void create_buf(T&);

// Fonction générique
template <typename Stream, typename T, typename... Args, typename = std::enable_if_t<sizeof...(Args) != 0>>
void create_buf(Stream& sbuf, T&& param, Args&&... params);

// Fonction types de base
template <typename Stream, typename Basic_Type,
	  typename = std::enable_if_t<std::is_arithmetic<std::remove_reference_t<std::remove_cv_t<Basic_Type>>>::value>>
void create_buf(Stream& sbuf, Basic_Type&& param);

/* Spécialisation */
template <typename Stream> void create_buf(Stream& sbuf, std::string const& str);
template <typename Stream> void create_buf(Stream& sbuf, atk_result const& res);
template <typename Stream, typename T> void create_buf(Stream& sbuf, std::vector<T> const& vect);
template <typename Stream> void create_buf(Stream& sbuf, Lobby const& lobby);
template <typename Stream> void create_buf(Stream& sbuf, Game const& game);
template <typename Stream> void create_buf(Stream& sbuf, Map const& map);

// crée un buffer (endianness)
template <typename Stream, typename Basic_Type, typename /*= std::enable_if_t<sizeof...(Args) != 0>*/>
void create_buf(Stream& sbuf, Basic_Type&& param)
{
	decltype(auto) t = boost::endian::native_to_little(param);
	for (size_t i = 0; i < sizeof(Basic_Type); ++i)
		sbuf.push_back(reinterpret_cast<const char*>(&t)[i]);
}

// crée buffer (string)
template <typename Stream> void create_buf(Stream& sbuf, std::string const& str)
{
	for (decltype(auto) c : str)
		create_buf(sbuf, c);
	create_buf(sbuf, '\0'); // IMPORTANT
}

// crée un buffer (fin)
template <typename T> void create_buf(T&)
{
}

// crée un buffer (générique)
template <typename Stream, typename T, typename... Args, typename /*= std::enable_if_t<sizeof...(Args) != 0>*/>
void create_buf(Stream& sbuf, T&& param, Args&&... params)
{
	create_buf(sbuf, param);
	create_buf(sbuf, std::forward<Args&&>(params)...);
}

/*
 * Spécialisations
 */
// sérialize atk_result
template <typename Stream> void create_buf(Stream& sbuf, atk_result const& res)
{
	create_buf(sbuf, static_cast<uint8_t>(res.square_conquered ? 1 : 0));
	create_buf(sbuf, res.nb_lost_troops_from_attacker, res.nb_lost_troops_from_defender);
	create_buf(sbuf, static_cast<uint16_t>(res.attackers_dice.size()), res.attackers_dice);
	create_buf(sbuf, static_cast<uint16_t>(res.defenders_dice.size()), res.defenders_dice);
}

// crée buffer (vector)
template <typename Stream, typename T> void create_buf(Stream& sbuf, std::vector<T> const& vect)
{
	for (T const& val : vect)
		create_buf(sbuf, val);
}

// sérialize Lobby
template <typename Stream> void create_buf(Stream& sbuf, Lobby const& lobby)
{
	auto view = lobby.all_players();
	std::for_each(view.first, view.second, [&sbuf](auto const& e) { create_buf(sbuf, e); });
}

// sérialize Game
template <typename Stream> void create_buf(Stream& sbuf, Game const& game)
{
	//TODO: vérifier que l'odre des joueurs est bon
	create_buf(sbuf, game.lobby());
	create_buf(sbuf, game.get_map());
}

// sérialize Map
template <typename Stream> void create_buf(Stream& sbuf, Map const& map)
{
	//TODO: vérifier que Karim a update avec des ID
	for (info_square is : map.m_info_square) {
		create_buf(sbuf, is.player_id);
		create_buf(sbuf, is.nb_troops);
	}
}

// sérialize GameParameters
template <typename Stream> void create_buf(Stream& sbuf, GameParameters const& gp)
{
	create_buf(sbuf, gp.nb_players, gp.id_map, gp.sec_by_turn);
}

// Envoi un message
template <typename T, typename... Args> void send_message(Session& session, T&& param, Args&&... params)
{
	std::vector<char> buf;
	create_buf(buf, std::forward<T&&>(param), std::forward<Args&&>(params)...);
	session.send(buf);
}

// Envoi une erreur
inline void send_error(Session& session, uint8_t subcode, std::string const& message)
{
	send_message(session, static_cast<uint8_t>(0), subcode, message);
}

template <typename T, typename... Args> void broadcast(Lobby const& lobby, T&& param, Args&&... params)
{
	std::vector<char> buf;
	create_buf(buf, std::forward<T&&>(param), std::forward<Args&&>(params)...);
	for (auto session : lobby.m_list_session) {
		session.get().send(buf);
	}
}

#endif
