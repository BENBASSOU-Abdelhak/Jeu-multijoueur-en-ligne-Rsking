/*
 * TODO: arguments à passer pour: (https://www.boost.org/doc/libs/1_75_0/doc/html/program_options/tutorial.html#id-1.3.32.4.3)
 * - nb threads
 * - port
 * - max lobbies
 */
#include "network/listener.h"
#include "network/ssl.h"

#include <iostream>
#include <thread>

#include <boost/asio/ip/address_v4.hpp>
#include <dbms.h>

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cerr << "usage: risking <IPv4> <port> [<nbthreads> <config dir>]" << std::endl;
		return EXIT_FAILURE;
	}
	auto const address = boost::asio::ip::make_address_v4(argv[1]);
	auto const port = static_cast<unsigned short>(std::stoi(argv[2]));
	auto const threads = argc > 3 ? std::stoi(argv[3]) : 1;
	if (argc > 4)
		SslContext::conf_dir(argv[4]);

	// Connexion à la BDD
	try {
		DBMS::get().login("DSN=risking");
	} catch (const otl_exception& oe) {
		std::cerr << oe.msg << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Starting WSS server on : " << address << ":" << port << " using " << threads << " threads."
		  << std::endl;

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

	// Deconnexion de la BDD
	try {
		DBMS::get().logout();
	} catch (const otl_exception& oe) {
		std::cerr << oe.msg << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
