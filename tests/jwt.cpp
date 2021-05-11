#define BOOST_TEST_MODULE JWT
#include <boost/test/included/unit_test.hpp>

#include "jwt/jwt.h"
#include "network/ssl.h"

#include <boost/filesystem.hpp>

#include <fstream>

BOOST_AUTO_TEST_CASE(bad_certificate)
{
	try {
		auto& ssl = SslContext::get();
		boost::ignore_unused(ssl);
		boost::filesystem::rename("certificate.cert", "tmp.cert");
		JWT::get().verify("BAD_JWT");
		BOOST_TEST(false);
	} catch (std::runtime_error const& e) {
	}
	boost::filesystem::rename("tmp.cert", "certificate.cert");
}

BOOST_AUTO_TEST_CASE(incorrect_certificates)
{
	try {
		auto& ssl = SslContext::get();
		boost::ignore_unused(ssl);
		boost::filesystem::rename("certificate.cert", "tmp.cert");

		std::ofstream out("certificate.cert");
		out << "BAD CERTIFICATE FORMAT\n";
		out.close();

		JWT::get().verify("BAD_JWT");
		BOOST_TEST(false);
	} catch (std::runtime_error const& e) {
	}
	boost::filesystem::remove("certificate.cert");
	boost::filesystem::rename("tmp.cert", "certificate.cert");
}

BOOST_AUTO_TEST_CASE(bad_jwt)
{
	std::string const bad_jwt = "BAD_JWT";
	BOOST_TEST(JWT::get().verify(bad_jwt) == false);
}

BOOST_AUTO_TEST_CASE(forged_jwt)
{
	std::string forged_jwt =
		"eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJyaXNraW5nLnplZnJlc2suY29tIiwic3ViIjoiMTIzNDU2Nzg5MCIsIm5hbWUiOiJKb2huIERvZSIsImFkbWluIjp0cnVlLCJpYXQiOjE1MTYyMzkwMjIsImV4cCI6MTUyMDAwMDAwMCwibmJmIjoxNTE2MjM5MDIyLCJqdGkiOiIwMDAwMDAwMDAwIn0.UtOuEydmV_5wcWuPJ8cJsWjYV4M4A8l47-lfkpI8I0r2A7n35MiLGCSSnrB5daqkJU1nN9S0lYgtqtZxGw4muAtSShOK0fpDFI-X19_Xecz3mNzVkrFAIUxfhhNIlccZga3o_Yp31JbUW1ikymK_Q7WTZZjLdrilEEdKUUl6Re_EX6aV_S0gxpYImSpOi8h3HYhLYAbijotb5eylT1Sz4N3quqzrkuaRjFo13fI9C_NlIARBJp7pv6-wGWPypO_I10EdMXO0ciGpdkjoawP8CCw0eq7BPHNh91WIakEdzS3HCUnxbl3AfpVtIrdxl2hRv_06qsvXDwJH-6t7FdGQQA";

	BOOST_TEST(JWT::get().verify(forged_jwt) == false);
}

BOOST_AUTO_TEST_CASE(good_jwt)
{
	std::string const good_jwt =
		"eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJyaXNraW5nLnplZnJlc2suY29tIiwic3ViIjoiMSIsIm5hbWUiOiJNYXRoZW8iLCJpYXQiOjE1MTYyMzkwMjIsImV4cCI6MjAwMDAwMDAwMCwibmJmIjoxNTE2MjM5MDIyLCJqdGkiOiIwMDAwMDAwMDAwIn0.us4IxoCXZJi4h9NnswMjJ1kf1C5Dy7PoKa28VSNynS3SE6hB-nqKxiDSmw7SGN8yW0kbmjP27pSHFWUVSqWKP_n5ETPFHX8XFQUr-yyi1daaFJMWrQmBrOPfEkKgIjWqLzHIELPjhnIbK-vDHEOzFNgcsfkcxv7Bdhjyda94Ud2XqyJLg1NNg-fgSY5mjmPS9rsbkKh5f53McBQAdVgtRFvZMUyS15XFJSLROTs2536uAQMAAu-RxFNk_QND612YEZmCZJm9N39FIXvvU49xRtQnGWx8tpUtbUI-ZGzQw-kggyHvBUz_I9PKI68FnnYiZMvBbXnb8PbVZw9DDiitNg";

	BOOST_TEST(JWT::get().verify(good_jwt) == true);

	auto jwt = JWT::get().decode(good_jwt);

	constexpr auto ns = 1000000000;

	BOOST_TEST(jwt.iss == "risking.zefresk.com");
	BOOST_TEST(jwt.uid == "1");
	BOOST_TEST(jwt.name == "Matheo");
	BOOST_TEST(jwt.iat.time_since_epoch().count() / ns == 1516239022);
	BOOST_TEST(jwt.exp.time_since_epoch().count() / ns == 2000000000);
	BOOST_TEST(jwt.nbf.time_since_epoch().count() / ns == 1516239022);
	BOOST_TEST(jwt.jti == "0000000000");
}
