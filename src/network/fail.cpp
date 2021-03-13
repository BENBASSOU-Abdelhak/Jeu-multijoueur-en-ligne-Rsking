#include "network/fail.h"
#include <boost/log/trivial.hpp>

// Report a failure
void fail(boost::beast::error_code ec, char const* what)
{
	BOOST_LOG_TRIVIAL(error) << what << ": " << ec.message() << "\n";
}
