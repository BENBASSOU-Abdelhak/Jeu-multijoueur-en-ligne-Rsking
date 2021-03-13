#include "logicexception.h"

LogicException::LogicException(uint8_t subcode, std::string const& what) : logic_error{ what }, sc_(subcode)
{
}

uint8_t LogicException::subcode() const noexcept
{
	return sc_;
}
