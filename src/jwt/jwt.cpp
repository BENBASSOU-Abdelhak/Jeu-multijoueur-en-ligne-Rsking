#include "jwt/jwt.h"
#include "network/ssl.h"

#include <fstream>

#include <boost/log/trivial.hpp>

__attribute__((weak)) JWT& JWT::get()
{
	static JWT inst_{ SslContext::get().path_to_key(), SslContext::get().path_to_certificate() };
	return inst_;
}

std::string read_all(std::string const& path)
{
	std::ifstream ifs{ path, std::ios::binary | std::ios::ate };
	if (!ifs)
		throw std::runtime_error{ "error: cannot open file " + path };

	auto const size = ifs.tellg();
	std::string ret(size, '\0');
	ifs.seekg(0);
	if (!ifs.read(&ret[0], size))
		throw std::runtime_error{ "error: cannot read file " + path };

	return ret;
}

__attribute__((weak)) JWT::JWT(std::string const& path_to_key, std::string const& path_to_certificate)
	: verifier(jwt::verify())
{
	std::string prikey = read_all(path_to_key);
	std::string cert = read_all(path_to_certificate);

	std::error_code ec;
	auto pubkey = jwt::helper::extract_pubkey_from_cert(cert, "", ec);
	if (ec)
		throw std::runtime_error{ "error" + std::to_string(ec.value()) +
					  "cannot extract public key from certificate:" + ec.message() };

	verifier = jwt::verify().allow_algorithm(jwt::algorithm::rs256(pubkey, prikey));
}

__attribute__((weak)) bool JWT::verify(const std::string& jwt)
{
	try {
		auto decoded = jwt::decode(jwt); //TODO: mettre en cache
		std::error_code ec;
		verifier.verify(decoded, ec);
		if (ec)
			BOOST_LOG_TRIVIAL(warning) << "Forged JWT detected: " << ec.message();
		return !ec;
	} catch (std::exception const& e) {
		BOOST_LOG_TRIVIAL(warning) << "Bad JWT detected: " << e.what();
		return false;
	}
}

__attribute__((weak)) JWT_t JWT::decode(std::string const& jwt)
{
	auto decoded = jwt::decode(jwt); //TODO: mettre en cache
	std::error_code ec;
	verifier.verify(decoded, ec);
	auto payload = decoded.get_payload_claims();

	return JWT_t{ decoded.get_issuer(),    to_uuid(decoded.get_subject()), payload["name"].as_string(),
		      decoded.get_issued_at(), decoded.get_expires_at(),       decoded.get_not_before(),
		      decoded.get_id() };
}
