// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GL45.h>
#include <EGL/egl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

bool gUseEGLGetProcAddress = false;

void* GetOpenGLFunctionPointer(char const* name)
{
    if (gUseEGLGetProcAddress)
    {
        return (void*)(*eglGetProcAddress)(name);
    }
    else
    {
        return (void*)(*glXGetProcAddress)((GLubyte const*)name);
    }
}
