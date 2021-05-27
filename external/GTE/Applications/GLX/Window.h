// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/WindowApplication.h>
#include <Graphics/GraphicsEngine.h>

// This forward declaration avoids name conflicts caused by #include-ing
// X11/Xlib.h.
struct _XDisplay;

namespace gte
{
    class Window : public WindowApplication
    {
    public:
        struct Parameters : public WindowApplication::Parameters
        {
            Parameters();

            Parameters(std::wstring const& inTitle,
                int inXOrigin, int inYOrigin, int inXSize, int inYSize);

            _XDisplay* display;
            unsigned long window;
            unsigned int deviceCreationFlags;
        };

    protected:
        // Abstract base class.  Only WindowSystem may create windows.
        friend class WindowSystem;
        Window(Parameters& parameters);
    public:
        virtual ~Window();

        virtual void SetTitle(std::wstring const& title);

        // Draw the window.
        void ShowWindow();

        // Mouse position information.
        virtual void SetMousePosition(int x, int y) override;
        virtual void GetMousePosition(int& x, int& y) const override;

        // Actions to take before the window closes.
        virtual void OnClose() override;

        // The event handler.  TODO: This should not be public, but for now
        // WindowSystem needs to call it.  Make WindowSystem a friend?
        enum
        {
            EVT_NONE_PENDING,
            EVT_PROCESSED,
            EVT_QUIT
        };
        int ProcessedEvent();

    protected:
        _XDisplay* mDisplay;
        unsigned long mWindow;
        std::array<bool, 8> mButtonDown;
        bool mShiftDown;
        bool mControlDown;
        bool mAltDown;
        bool mCommandDown;
        std::shared_ptr<GraphicsEngine> mEngine;
    };
}

// Window and WindowSystem have a circular dependency that cannot be broken
// by forward declarations in either header. The includion of the following
// header file at this location breaks the cycle, because Window is defined
// previously in this file and is known to the compiler when it includes this
// file.
#include <Applications/GLX/WindowSystem.h>
