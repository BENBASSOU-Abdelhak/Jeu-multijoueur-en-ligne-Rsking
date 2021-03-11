/*
 * Check wether Boost::Test works and is integrated to cMake
 */
#define BOOST_TEST_MODULE Cmake CTest
#include <boost/test/included/unit_test.hpp>

#include "coverage.h"

BOOST_AUTO_TEST_CASE(build_ok) {
  //if this compiles, test is ok
  BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(coverage) {
	BOOST_TEST(foo() == 0);
}
