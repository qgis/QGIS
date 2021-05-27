// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <fstream>

namespace gte
{
    class LogToFile : public Logger::Listener
    {
    public:
        LogToFile(std::string const& filename, int flags)
            :
            Logger::Listener(flags),
            mFilename(filename)
        {
            std::ofstream logFile(filename);
            if (logFile)
            {
                // This clears the file contents from any previous runs.
                logFile.close();
            }
            else
            {
                // The file cannot be opened.  Use a null string for Report to
                // know not to attempt opening the file for append.
                mFilename = "";
            }
        }

    private:
        virtual void Report(std::string const& message)
        {
            if (mFilename != "")
            {
                // Open for append.
                std::ofstream logFile(mFilename, std::ios_base::out | std::ios_base::app);
                if (logFile)
                {
                    logFile << message.c_str();
                    logFile.close();
                }
                else
                {
                    // The file cannot be opened.  Use a null string for
                    // Report not to attempt opening the file for append on
                    // the next call.
                    mFilename = "";
                }
            }
        }

        std::string mFilename;
    };
}
