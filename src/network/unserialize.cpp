#include "network/unserialize.h"

size_t deflate(raw_type const& buf, size_t left, std::string& str)
{
	assert(left > 0 && "trying to deflate empty string");
	str.clear();
	size_t i;
	for (i = 0; i < left; ++i) {
		const char c = static_cast<char>(buf[i]);
		if (c == '\0') {
			i += 1;
			break;
		}

		str.push_back(c);
	}
	return i;
}

size_t unserialize(raw_type, size_t)
{
	return 0;
}

size_t deflate(raw_type const& buf, size_t left, bool& b)
{
	assert(left > 0 && "trying to deflate empty buffer into bool");
	((void)left);
	b = buf[0];
	return 1;
}
