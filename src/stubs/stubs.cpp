#include "logic/lobby.h"
#include "logic/lobbypool.h"
#include "logic/game.h"

#include "network/lobbypooldispatcher.h"
#include "network/session.h"

#include "jwt/jwt.h"

#include <boost/log/trivial.hpp>

__attribute__((weak)) Lobby& Game::lobby() const
{
	return m_lobby;
}
__attribute__((weak)) Game::Game(GameParameters const&, Lobby& l) : m_lobby(l)
{
}
void Game::add_troups(const Session& player_asking, uint16_t dst_square, uint16_t nb_troops)
{
}
atk_result Game::attack(const Session& player_asking, uint16_t src_square, uint16_t dst_square, uint16_t nb_troops)
{
	return atk_result{ true, true, 1, 1, { 1 }, { 1 } };
}
void Game::transfer(const Session& player_asking, uint16_t src_square, uint16_t dst_square, uint16_t nb_troops)
{
}
void Game::skip(const Session& player_asking)
{
}
Gamephase Game::current_phase() const
{
	return Gamephase::Placement;
}
size_t Game::nb_alive() const
{
	return 2;
}
bool Game::is_finished() const
{
	return false;
}
uint16_t Game::time_left() const
{
	return 42;
}
std::string const& Game::last_dead() const
{
	static std::string c{ "bouchon_last_dead" };
	return c;
}
std::string const& Game::current_player() const
{
	static std::string c{ "bouchon_current_player" };
	return c;
}
std::string const& Game::winner() const
{
	static std::string c{ "bouchon_winner" };
	return c;
}
uint16_t Game::troop_gained(Session const&) const
{
	return 69;
}
__attribute__((weak)) Map const& Game::get_map() const
{
	return m_map;
}

JWT::JWT()
{
}

JWT& JWT::get()
{
	static JWT inst{};
	return inst;
}

__attribute__((weak)) bool JWT::verify(std::string const&)
{
	return true;
}

__attribute__((weak)) JWT_t JWT::decode(std::string const&)
{
	return JWT_t{};
}

__attribute__((weak)) Map::Map()
{
}
