// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/LogToOutputWindow.h>
#include <Windows.h>
using namespace gte;

LogToOutputWindow::LogToOutputWindow(int flags)
    :
    Logger::Listener(flags)
{
}

void LogToOutputWindow::Report(std::string const& message)
{
    std::wstring text(message.begin(), message.end());
    OutputDebugString(text.c_str());
}
