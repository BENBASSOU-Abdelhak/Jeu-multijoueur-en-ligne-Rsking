/* 
 * TODO: arguments à passer pour: (https://www.boost.org/doc/libs/1_75_0/doc/html/program_options/tutorial.html#id-1.3.32.4.3)
 * - nb threads
 * - port
 * - max lobbies
 */
#include "network/listener.h"

#include <thread>

int main(int argc, char* argv[])
{
	auto const address = boost::asio::ip::address_v4::loopback();
	auto const port = 42424;
	auto const threads = 1;

	// The io_context is required for all I/O
	boost::asio::io_context ioc{ threads };

	// Create and launch a listening port
	std::make_shared<Listener>(ioc, boost::asio::ip::tcp::endpoint{ address, port })->run();

	// Run the I/O service on the requested number of threads
	std::vector<std::thread> v;
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
		v.emplace_back([&ioc] { ioc.run(); });
	ioc.run();

	return EXIT_SUCCESS;
}
