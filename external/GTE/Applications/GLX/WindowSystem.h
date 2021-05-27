// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/GLX/Window.h>
#include <memory>

// These forward declarations avoid name conflicts caused by #include-ing
// X11/Xlib.h.
struct _XDisplay;
struct __GLXcontextRec;

namespace gte
{
    class WindowSystem
    {
    public:
        // Construction and destruction. This is a singleton class.
        ~WindowSystem();
        WindowSystem();

        // Create and destroy windows.  Derived classes may extend the inputs
        // using a nested class derived from Window::Parameters
        template <typename WindowType>
        std::shared_ptr<WindowType> Create(typename WindowType::Parameters& parameters);

        // Creation of information used for a compute-shader-only GLXEngine.
        bool Create(_XDisplay*& display, __GLXcontextRec*& context, unsigned long& window, bool useDepth24Stencil8);

        template <typename WindowType>
        void Destroy(std::shared_ptr<WindowType>& window);

        enum
        {
            DEFAULT_ACTION = 0,
            NO_IDLE_LOOP = 1
        };

        template <typename WindowType>
        void MessagePump(std::shared_ptr<WindowType> const& window, unsigned int flags);

    private:
        // Window creation and destruction.
        void CreateFrom(Window::Parameters& parameters);

        char const* mDisplayName;
        _XDisplay* mDisplay;
        std::map<unsigned long, std::shared_ptr<Window>> mWindowMap;
    };

    extern WindowSystem TheWindowSystem;

    template <typename WindowType>
    std::shared_ptr<WindowType> WindowSystem::Create(typename WindowType::Parameters& parameters)
    {
        CreateFrom(parameters);
        if (parameters.created)
        {
            std::shared_ptr<WindowType> window = std::make_shared<WindowType>(parameters);
            mWindowMap[parameters.window] = window;
            if (parameters.created)
            {
                return window;
            }
            Destroy(window);
        }
        // else: CreateFrom will report the problem via the logger system.
        return nullptr;
    }

    template <typename WindowType>
    void WindowSystem::Destroy(std::shared_ptr<WindowType>& window)
    {
        if (window)
        {
            mWindowMap.erase(window->mWindow);
            window->OnClose();
            window = nullptr;
        }
    }

    template <typename WindowType>
    void WindowSystem::MessagePump(std::shared_ptr<WindowType> const& window, unsigned int flags)
    {
        if (window)
        {
            window->ShowWindow();

            for (;;)
            {
                int result = window->ProcessedEvent();
                if (result == Window::EVT_QUIT)
                {
                    return;
                }

                if (result == Window::EVT_NONE_PENDING)
                {
                    if (!(flags & NO_IDLE_LOOP))
                    {
                        if (!window->IsMinimized())
                        {
                            window->OnIdle();
                        }
                    }
                }
            }
        }
    }
}
