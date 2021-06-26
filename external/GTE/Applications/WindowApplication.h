// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Application.h>
#include <Applications/OnIdleTimer.h>

namespace gte
{
    class WindowApplication : public Application
    {
    public:
        struct Parameters : public Application::Parameters
        {
            Parameters();

            Parameters(std::wstring const& inTitle,
                int inXOrigin, int inYOrigin, int inXSize, int inYSize);

            std::wstring title;
            int xOrigin, yOrigin, xSize, ySize;
            bool allowResize, useDepth24Stencil8, created;
        };

    public:
        // Abstract base class. Only WindowSystem may create windows.
        virtual ~WindowApplication() = default;
    protected:
        WindowApplication(Parameters& parameters);

    public:
        // Member access.
        virtual void SetTitle(std::wstring const& title)
        {
            mTitle = title;
        }

        inline std::wstring GetTitle() const
        {
            return mTitle;
        }

        inline int GetXOrigin() const
        {
            return mXOrigin;
        }

        inline int GetYOrigin() const
        {
            return mYOrigin;
        }

        inline int GetXSize() const
        {
            return mXSize;
        }

        inline int GetYSize() const
        {
            return mYSize;
        }

        inline bool IsMinimized() const
        {
            return mIsMinimized;
        }

        inline bool IsMaximized() const
        {
            return mIsMaximized;
        }

        inline float GetAspectRatio() const
        {
            return static_cast<float>(mXSize) / static_cast<float>(mYSize);
        }

        // Display callbacks.
        virtual void OnMove(int x, int y);
        virtual bool OnResize(int xSize, int ySize);
        virtual void OnMinimize();
        virtual void OnMaximize();
        virtual void OnDisplay();
        virtual void OnIdle();

        // Keyboard callbacks. OnCharPress allows you to distinguish between
        // upper-case and lower-case letters; OnKeyDown and OnKeyUp do not.
        // For OnCharPress, pressing KEY_ESCAPE terminates the application.
        // Pressing ' ' resets the application timer.
        virtual bool OnCharPress(unsigned char key, int x, int y);
        virtual bool OnKeyDown(int key, int x, int y);
        virtual bool OnKeyUp(int key, int x, int y);

        // Mouse callbacks and state information.
        // TODO: HACK FOR NOW. Once these are removed, all the sample
        // applications must have their signatures changed.
        typedef int MouseButton;
        typedef int MouseState;
        // END TODO;
        virtual bool OnMouseClick(int button, int state, int x, int y, unsigned int modifiers);
        virtual bool OnMouseMotion(int button, int x, int y, unsigned int modifiers);
        virtual bool OnMouseWheel(int delta, int x, int y, unsigned int modifiers);
        virtual void SetMousePosition(int x, int y);
        virtual void GetMousePosition(int& x, int& y) const;

        // Actions to take before the window closes.
        virtual void OnClose();

        // Key identifiers. These are platform-specific, so classes that
        // derived from WindowApplication must define these variables. They
        // are not defined by WindowApplication itself.
        static int const KEY_ESCAPE;
        static int const KEY_LEFT;
        static int const KEY_RIGHT;
        static int const KEY_UP;
        static int const KEY_DOWN;
        static int const KEY_HOME;
        static int const KEY_END;
        static int const KEY_PAGE_UP;
        static int const KEY_PAGE_DOWN;
        static int const KEY_INSERT;
        static int const KEY_DELETE;
        static int const KEY_F1;
        static int const KEY_F2;
        static int const KEY_F3;
        static int const KEY_F4;
        static int const KEY_F5;
        static int const KEY_F6;
        static int const KEY_F7;
        static int const KEY_F8;
        static int const KEY_F9;
        static int const KEY_F10;
        static int const KEY_F11;
        static int const KEY_F12;
        static int const KEY_BACKSPACE;
        static int const KEY_TAB;
        static int const KEY_ENTER;
        static int const KEY_RETURN;

        // Keyboard modifiers.
        static int const KEY_SHIFT;
        static int const KEY_CONTROL;
        static int const KEY_ALT;
        static int const KEY_COMMAND;

        // Mouse buttons.
        static int const MOUSE_NONE;
        static int const MOUSE_LEFT;
        static int const MOUSE_MIDDLE;
        static int const MOUSE_RIGHT;

        // Mouse state.
        static int const MOUSE_UP;
        static int const MOUSE_DOWN;

        // Mouse modifiers.
        static int const MODIFIER_CONTROL;
        static int const MODIFIER_LBUTTON;
        static int const MODIFIER_MBUTTON;
        static int const MODIFIER_RBUTTON;
        static int const MODIFIER_SHIFT;

    protected:
        // Standard window information.
        std::wstring mTitle;
        int mXOrigin, mYOrigin, mXSize, mYSize;
        bool mAllowResize;
        bool mIsMinimized;
        bool mIsMaximized;

        OnIdleTimer mTimer;
    };
}
