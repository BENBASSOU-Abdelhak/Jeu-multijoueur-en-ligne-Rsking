#ifndef INCLUDE__NETWORK__MESSAGES_H
#define INCLUDE__NETWORK__MESSAGES_H

#include "network/session.h"

#include <boost/endian/conversion.hpp>

#include <sstream>

/* Fonctions simple d'I/O */

// crée un buffer (endianness)
template <typename Stream, typename Basic_Type, typename = std::enable_if_t<std::is_arithmetic<Basic_Type>::value>>
void create_buf(Stream& sbuf, Basic_Type param) {
	Basic_Type t = boost::endian::native_to_little(param);
	for (size_t i = 0; i < sizeof(Basic_Type); ++i)
		sbuf.push_back(reinterpret_cast<char*>(&t)[0]);
}

// crée buffer (générique)
template <typename Stream>
void create_buf(Stream& sbuf, std::string const& str) {
	for (const char c : str)
		create_buf(sbuf, c);
	create_buf(sbuf, '\0'); // IMPORTANT
}

// crée un buffer (fin)
template <typename T>
void create_buf(T&) {}

// crée un buffer (générique)
template <typename Stream, typename T, typename... Args>
void create_buf(Stream& sbuf, T&& param, Args&&... params) {
	create_buf(sbuf, param);
	create_buf(sbuf, std::forward<Args>(params)...);
}

// Envoi un message
template <typename T, typename... Args>
void send_message(Session& session, T&& param, Args&&... params) {
	std::vector<char> buf;
	create_buf(buf, std::forward<T&&>(param), std::forward<Args>(params)...);
	session.do_write(std::move(buf));
}

// Envoi une erreur
inline void send_error(Session& session, uint8_t subcode, std::string const& message) {
	send_message(session, static_cast<uint8_t>(0), subcode, message);
}

// TODO: (quand Lobby est fait)
struct Lobby;
template <typename T, typename... Args>
void broadcast(Lobby const& lobby, T&& param, Args&&... params);

#endif
