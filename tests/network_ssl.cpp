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
		SSL_CONTEXT::get();
		BOOST_TEST(false);
	} catch (std::runtime_error const& e) {
	}
	boost::filesystem::rename("tmp.cert", "certificate.cert");
}

BOOST_AUTO_TEST_CASE(bad_primary_key)
{
	try {
		boost::filesystem::rename("pri.key", "tmp.key");
		SSL_CONTEXT::get();
		BOOST_TEST(false);
	} catch (std::runtime_error const& e) {
	}
	boost::filesystem::rename("tmp.key", "pri.key");
}

BOOST_AUTO_TEST_CASE(test_certificates)
{
	try {
		SSL_CONTEXT::get();
	} catch (std::runtime_error const& e) {
		BOOST_TEST(false);
	}
}

