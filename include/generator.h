#ifndef INCLUDE_GENERATOR_R_H
#define INCLUDE_GENERATOR_R_H
/* *Common generator */

#include <random>

class Gen
{
    public:
	static std::mt19937& get();

    private:
	Gen();
	std::mt19937 m_t;
};

#endif
