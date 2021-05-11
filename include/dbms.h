#ifndef INCLUDE_DBMS_H
#define INCLUDE_DBMS_H

#include <string>

#include "otlv4.h"

struct Game;

class DBMS
{
	DBMS() = default;

	otl_connect m_con{};

    public:
	DBMS(DBMS const&) = delete;

	DBMS& operator=(DBMS const&) = delete;

	static DBMS& get()
	{
		static DBMS db;
		return db;
	}

	void login(const std::string& connect_str);

	void logout();

	bool can_join(std::string const& gamertag);

	bool ban(std::string const& gamertag, std::string const& reason);

	bool add_game(Game& game);
};

#endif