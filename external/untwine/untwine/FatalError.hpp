#pragma once

#include <stdexcept>

namespace untwine
{

class  FatalError : public std::runtime_error
{
public:
    inline FatalError(std::string const& msg) : std::runtime_error(msg)
        {}
};

} // namespace untwine
