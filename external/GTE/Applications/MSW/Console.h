// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/ConsoleApplication.h>
#include <Graphics/GraphicsEngine.h>

namespace gte
{
    class Console : public ConsoleApplication
    {
    public:
        struct Parameters : public ConsoleApplication::Parameters
        {
            Parameters();

            Parameters(std::wstring const& inTitle);

            // For DX11, the device creation flags are passed to the function
            // D3D11CreateDevice during construction of a DX11Engine object.
            // See the documentation for D3D11CreateDevice for the available
            // flags. For GL45, set the flags to 0 for the default behavior;
            // no additional semantics occur on GL45Engine construction. Set
            // bit 0 of the flag to 1 to tell the GL45Engine construction to
            // write a text file that contains the OpenGL driver information.
            // The default value is 0.  When bit 0 is set to 1, a text file
            // named OpenGLDriverInfo.txt is generated that contains the
            // OpenGL driver information. Other bit flags may be defined at
            // a later date.
            unsigned int deviceCreationFlags;
        };

    public:
        // Abstract base class. Only WindowSystem may create windows.
        virtual ~Console();

    protected:
        Console(Parameters& parameters);

        // TODO: This is assigned mBaseEngine, which allows development of
        // the DX12 engine independently of DX11 and WGL. The DX12 engine
        // is a work in progress.
        std::shared_ptr<GraphicsEngine> mEngine;
    };
}

// Console and ConsoleSystem have a circular dependency that cannot be broken
// by forward declarations in either header. The includion of the following
// header file at this location breaks the cycle, because Console is defined
// previously in this file and is known to the compiler when it includes this
// file.
#include <Applications/MSW/ConsoleSystem.h>
