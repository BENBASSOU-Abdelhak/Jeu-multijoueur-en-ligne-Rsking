#ifndef INCLUDE__NETWORK__UNSERIALIZE_H
#define INCLUDE__NETWORK__UNSERIALIZE_H

#include <boost/endian/conversion.hpp>

#include <type_traits>
#include <cassert>
#include <string>

/* Désérialize un buffer dans les paramètres en prenant en compte l'endianness */

using raw_type = const char*;

template <typename Basic_Type,
	  typename = std::enable_if_t<std::is_arithmetic<std::remove_reference_t<Basic_Type>>::value>>
size_t deflate(raw_type const& buf, size_t left, Basic_Type& param)
{
	assert(sizeof(Basic_Type) <= left && "not enough space left to deflate");
	param = boost::endian::native_to_little(*reinterpret_cast<const Basic_Type* const>(buf));
	return sizeof(Basic_Type);
}

size_t deflate(raw_type const& buf, size_t left, std::string& str);
size_t deflate(raw_type const& buf, size_t left, bool& b);

size_t unserialize(raw_type, size_t);

template <typename T, typename... Args> size_t unserialize(raw_type buf, size_t size, T& param, Args&... args)
{
	const auto read = deflate(buf, size, param);
	buf += read;
	size -= read;
	return read + unserialize(buf, size, std::forward<Args&>(args)...);
}

template <typename Buffer, typename T, typename... Args>
size_t unserialize(Buffer& buf, size_t size, T& param, Args&... args)
{
	raw_type raw = static_cast<raw_type>(buf.data());
	return unserialize(raw, size, std::forward<T&>(param), std::forward<Args&>(args)...);
}

#endif
