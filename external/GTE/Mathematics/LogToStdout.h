// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <iostream>

namespace gte
{
    class LogToStdout : public Logger::Listener
    {
    public:
        LogToStdout(int flags)
            :
            Logger::Listener(flags)
        {
        }

    private:
        virtual void Report(std::string const& message)
        {
            std::cout << message.c_str() << std::flush;
        }
    };
}
