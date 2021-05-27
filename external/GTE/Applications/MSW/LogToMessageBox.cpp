// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/LogToMessageBox.h>
#include <Windows.h>
using namespace gte;

LogToMessageBox::LogToMessageBox(int flags)
    :
    Logger::Listener(flags)
{
}

void LogToMessageBox::Report(std::string const& message)
{
    std::string output = message + "Do you want to debug?";

    std::wstring text(output.begin(), output.end());
    int selection = MessageBox(nullptr, text.c_str(), L"Report",
        MB_ICONERROR | MB_YESNOCANCEL | MB_APPLMODAL | MB_TOPMOST);

    switch (selection)
    {
    case IDYES:
        // Break and debug.
        __debugbreak();
        break;

    case IDNO:
        // Continue execution.
        break;

    case IDCANCEL:
    default:
        // Terminate execution.
        exit(0);
        break;
    }
}
