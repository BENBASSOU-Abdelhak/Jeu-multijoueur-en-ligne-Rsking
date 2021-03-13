#ifndef INCLUDE__NETWORK__FAIL_H
#define INCLUDE__NETWORK__FAIL_H

#include <boost/beast/core.hpp>

// Report a failure
void fail(boost::beast::error_code ec, char const* what);

#endif
