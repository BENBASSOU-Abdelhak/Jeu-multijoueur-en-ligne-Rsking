/* Tests du main et des arguments */
#define BOOST_TEST_MODULE Main
#include <boost/test/included/unit_test.hpp>

#include <boost/process.hpp>
// namespace bp = boost::process; //we will assume this for all further examples
// int result = bp::system("g++ main.cpp");

BOOST_AUTO_TEST_CASE(few_args)
{
	auto result = boost::process::system("./risking");
	BOOST_TEST(result == EXIT_FAILURE);
}

/*BOOST_AUTO_TEST_CASE(fail_db)
{
	auto result = boost::process::system("./risking 0.0.0.0 42424 1 ./");
	BOOST_TEST(result == EXIT_FAILURE);
}*/

