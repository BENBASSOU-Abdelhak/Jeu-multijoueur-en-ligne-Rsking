#ifndef INCLUDE__JWT__JWT_H
#define INCLUDE__JWT__JWT_H

/* JWT */

#include <chrono>
#include <string>

using uuid_t = uint64_t;
using timepoint = std::chrono::time_point<std::chrono::system_clock>;

struct JWT_t {
	std::string iss;
	uuid_t uid;
	std::string name;
	timepoint iat;
	timepoint exp;
	timepoint nbf;
	uint64_t jti;
};

class JWT
{
    public:
	static JWT& get();

	bool verify(std::string const& jwt);

	JWT_t decode(std::string const& jwt);

    private:
	JWT();
};

#endif
