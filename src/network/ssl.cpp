#include "network/ssl.h"
#include "network/fail.h"

namespace ssl = boost::asio::ssl;

boost::asio::ssl::context& SSL_CONTEXT::get()
{
	static SSL_CONTEXT inst{ ssl::context::tls_server, "certificate.cert", "pri.key" };
	return inst.ctx_;
}

SSL_CONTEXT::SSL_CONTEXT(ssl::context::method method, std::string const& path_to_certificate,
			 std::string const& path_to_key)
	: ctx_{ method }
{
	boost::system::error_code ec;

	ctx_.use_certificate_file(path_to_certificate, ssl::context::pem, ec);
	if (ec) {
		fail(ec, "SSL certificate");
		throw std::runtime_error{ "A problem occured with the SSL certificate: " + path_to_certificate };
	}

	ctx_.use_private_key_file(path_to_key, ssl::context::pem, ec);
	if (ec) {
		fail(ec, "SSL key");
		throw std::runtime_error{ "A problem occured with the SSL key: " + path_to_key };
	}
}
