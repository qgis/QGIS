// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/LogToFile.h>
#include <Mathematics/LogToStdout.h>
#if defined(GTE_USE_MSWINDOWS)
#include <Applications/MSW/LogToMessageBox.h>
#include <Applications/MSW/LogToOutputWindow.h>
#endif
#include <memory>

namespace gte
{
    class LogReporter
    {
    public:
        // Construction and destruction.  Create one of these objects in an
        // application for logging.  The GenerateProject tool creates such
        // code.  If you do not want a particular logger, set the flags to
        // LISTEN_FOR_NOTHING and set logFile to "" if you do not want a file.
        ~LogReporter();

        LogReporter(std::string const& logFile, int logFileFlags, int logStdoutFlags,
            int logMessageBoxFlags = 0, int logOutputWindowFlags = 0);

    private:
        std::unique_ptr<LogToFile> mLogToFile;
        std::unique_ptr<LogToStdout> mLogToStdout;

#if defined(GTE_USE_MSWINDOWS)
        std::unique_ptr<LogToMessageBox> mLogToMessageBox;
        std::unique_ptr<LogToOutputWindow> mLogToOutputWindow;
#endif
    };
}
