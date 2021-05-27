// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/ConsoleApplication.h>
#include <Graphics/GraphicsEngine.h>

// This forward declaration avoids name conflicts caused by #include-ing
// X11/Xlib.h.
struct _XDisplay;

namespace gte
{
    class Console : public ConsoleApplication
    {
    public:
        struct Parameters : public ConsoleApplication::Parameters
        {
            Parameters();

            Parameters(std::wstring const& inTitle);

            _XDisplay* display;
            unsigned long window;
            unsigned int deviceCreationFlags;
        };

    public:
        // Abstract base class. Only WindowSystem may create windows.
        virtual ~Console();

    protected:
        Console(Parameters& parameters);
        std::shared_ptr<GraphicsEngine> mEngine;
    };
}

// Console and ConsoleSystem have a circular dependency that cannot be broken
// by forward declarations in either header. The includion of the following
// header file at this location breaks the cycle, because Console is defined
// previously in this file and is known to the compiler when it includes this
// file.
#include <Applications/GLX/ConsoleSystem.h>
