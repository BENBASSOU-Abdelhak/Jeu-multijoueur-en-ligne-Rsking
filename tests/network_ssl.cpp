/*
 * Test if SSL_CONTEXT is working as expected
 */
#define BOOST_TEST_MODULE Network
#include <boost/test/included/unit_test.hpp>

#include "network/ssl.h"

#include <boost/filesystem.hpp>

BOOST_AUTO_TEST_CASE(bad_certificate)
{
	try {
		boost::filesystem::rename("certificate.cert", "tmp.cert");
		SslContext::get();
		BOOST_TEST(false);
	} catch (std::runtime_error const& e) {
	}
	boost::filesystem::rename("tmp.cert", "certificate.cert");
}

BOOST_AUTO_TEST_CASE(bad_primary_key)
{
	try {
		boost::filesystem::rename("pri.key", "tmp.key");
		SslContext::get();
		BOOST_TEST(false);
	} catch (std::runtime_error const& e) {
	}
	boost::filesystem::rename("tmp.key", "pri.key");
}

BOOST_AUTO_TEST_CASE(test_certificates)
{
	try {
		SslContext::get();
		BOOST_TEST(SslContext::get().path_to_certificate() != "");
		BOOST_TEST(SslContext::get().path_to_key() != "");
	} catch (std::runtime_error const& e) {
		BOOST_TEST(false);
	}
}

BOOST_AUTO_TEST_CASE(conf_dir)
{
	SslContext::conf_dir("./");
	BOOST_TEST(true);
}
