#ifndef INCLUDE__NETWORK__SSL_H
#define INCLUDE__NETWORK__SSL_H

/* Classe Singleton chargé dynamiquement et une seule fois à son premier appel
 *
 * Elle s'occupe de charger le contexte automatiquement et propose une conversion vers boost::asio::ssl::context
 *
 */

#include <boost/asio/ssl.hpp>

class SSL_CONTEXT
{
    public:
	static boost::asio::ssl::context& get();

    private:
	/**
		 * @brief Crée un contexte depuis un certain type avec un certificat sous forme de fichier
		 *
		 * @param method parmis www.boost.org/doc/libs/1_75_0/doc/html/boost_asio/reference/ssl__context/method.html
		 * @param path_to_certificate chemin vers le certificat (PEM)
		 * @param path_to_key chemin vers la clé liée au certificat (PEM)
		 */
	SSL_CONTEXT(boost::asio::ssl::context::method method, std::string const& path_to_certificate,
		    std::string const& path_to_key);

	boost::asio::ssl::context ctx_;
};

#endif
