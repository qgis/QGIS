// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/LogReporter.h>
using namespace gte;

LogReporter::~LogReporter()
{
    if (mLogToStdout)
    {
        Logger::Unsubscribe(mLogToStdout.get());
    }

    if (mLogToFile)
    {
        Logger::Unsubscribe(mLogToFile.get());
    }

#if defined(GTE_USE_MSWINDOWS)
    if (mLogToOutputWindow)
    {
        Logger::Unsubscribe(mLogToOutputWindow.get());
    }

    if (mLogToMessageBox)
    {
        Logger::Unsubscribe(mLogToMessageBox.get());
    }
#endif
}

LogReporter::LogReporter(std::string const& logFile, int logFileFlags,
    int logStdoutFlags, int logMessageBoxFlags, int logOutputWindowFlags)
    :
    mLogToFile(nullptr),
    mLogToStdout(nullptr)
#if defined(GTE_USE_MSWINDOWS)
    ,
    mLogToMessageBox(nullptr),
    mLogToOutputWindow(nullptr)
#endif
{
    if (logFileFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToFile = std::make_unique<LogToFile>(logFile, logFileFlags);
        Logger::Subscribe(mLogToFile.get());
    }

    if (logStdoutFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToStdout = std::make_unique<LogToStdout>(logStdoutFlags);
        Logger::Subscribe(mLogToStdout.get());
    }

#if defined(GTE_USE_MSWINDOWS)
    if (logMessageBoxFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToMessageBox = std::make_unique<LogToMessageBox>(logMessageBoxFlags);
        Logger::Subscribe(mLogToMessageBox.get());
    }

    if (logOutputWindowFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToOutputWindow = std::make_unique<LogToOutputWindow>(logOutputWindowFlags);
        Logger::Subscribe(mLogToOutputWindow.get());
    }
#endif
}
