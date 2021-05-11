#include "generator.h"

#include <chrono>

std::mt19937& Gen::get()
{
	static Gen inst_{};
	return inst_.m_t;
}

Gen::Gen() : m_t{}
{
	std::seed_seq seq{ std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(),
			   static_cast<decltype(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count())>(std::random_device{}())
				   , std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 1000000000 };
	m_t.seed(seq);
}
