// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <vector>

namespace gte
{
    class LogToStringArray : public Logger::Listener
    {
    public:
        LogToStringArray(std::string const& name, int flags)
            :
            Logger::Listener(flags),
            mName(name)
        {
        }

        inline std::string const& GetName() const
        {
            return mName;
        }

        inline std::vector<std::string> const& GetMessages() const
        {
            return mMessages;
        }

        inline std::vector<std::string>& GetMessages()
        {
            return mMessages;
        }

    private:
        virtual void Report(std::string const& message)
        {
            mMessages.push_back(message);
        }

        std::string mName;
        std::vector<std::string> mMessages;
    };
}
