#ifndef INCLUDE_RISKING_LOGICEXCEPTION_H
#define INCLUDE_RISKING_LOGICEXCEPTION_H

/* Exceptions de logique de base du jeu */

#include <stdexcept>

class LogicException : public std::logic_error
{
    public:
	LogicException(uint8_t subcode, std::string const& what);
	uint8_t subcode() const noexcept;

    private:
	uint8_t sc_;
};

#endif
