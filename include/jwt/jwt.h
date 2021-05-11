#ifndef INCLUDE__JWT__JWT_H
#define INCLUDE__JWT__JWT_H

/* JWT */

#include <chrono>
#include <string>

//#ifdef JWT
#include <jwt-cpp/jwt.h>
//#endif

using uuid_t = std::string; // uint128_t
using timepoint = std::chrono::time_point<std::chrono::system_clock>;
inline uuid_t to_uuid(std::string const& str)
{
	return str; //std::stoll(str);
}

struct JWT_t {
	std::string iss;
	uuid_t uid;
	std::string name;
	timepoint iat;
	timepoint exp;
	timepoint nbf;
	std::string jti;
};

class JWT
{
    public:
	static JWT& get();

	bool verify(std::string const& jwt);

	JWT_t decode(std::string const& jwt);

    private:
	JWT(std::string const& path_to_key, std::string const& path_to_certificate);

	//#ifdef JWT
	decltype(jwt::verify()) verifier;
	//#endif
};

#endif
