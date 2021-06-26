// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GL45/GL45Engine.h>

namespace gte
{
    class WGLEngine : public GL45Engine
    {
    public:
        // Construction and destruction.  The first constructor is for
        // windowed graphics applications.  The second constructor is for
        // windowless compute-program applications.  If useDepth24Stencil8
        // is 'true', the standard 24-bits depth and 8-bits stencil buffer
        // is created.  If the value is 'false', a 32-bit depth buffer is
        // created (no stencil support).  TODO: Currently, OpenGL 4.5 is
        // required for compute shaders and for OpenGL-specific API calls.
        // See the comment in GL45Engine.h for 'MeetsRequirements()'.
        virtual ~WGLEngine();
        WGLEngine(HWND handle, bool useDepth24Stencil8, bool saveDriverInfo, int requiredMajor = 4, int requiredMinor = 5);
        WGLEngine(bool useDepth24Stencil8 = true, bool saveDriverInfo = false, int requiredMajor = 4, int requiredMinor = 5);

        // Access to members that correspond to constructor inputs.
        inline HDC GetDevice() const
        {
            return mDevice;
        }

        inline HGLRC GetImmediate() const
        {
            return mImmediate;
        }

        // Allow the user to switch between OpenGL contexts when there are
        // multiple instances of GL4Engine in an application.
        virtual bool IsActive() const override;
        virtual void MakeActive() override;

        // Support for clearing the color, depth, and stencil back buffers.
        virtual void DisplayColorBuffer(unsigned int syncInterval) override;

    private:
        // Helpers for construction and destruction.
        virtual bool Initialize(int requiredMajor, int requiredMinor, bool useDepth24Stencil8, bool saveDriverInfo) override;
        void Terminate();

        // Inputs to the constructor.
        HWND mHandle;

        // Objects created by the constructors.
        HDC mDevice;
        HGLRC mImmediate;
        std::wstring mComputeWindowClass;
        ATOM mComputeWindowAtom;
    };
}
