// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.12.05

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GLX/GLXEngine.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
using namespace gte;

GLXEngine::~GLXEngine()
{
    Terminate();
}

GLXEngine::GLXEngine(Display* display, unsigned long window, GLXContext context,
    int xSize, int ySize, bool useDepth24Stencil8, bool saveDriverInfo, int requiredMajor, int requiredMinor)
    :
    GL45Engine(),
    mDisplay(display),
    mWindow(window),
    mImmediate(context),
    mIsComputeWindow(false)
{
    mXSize = xSize;
    mYSize = ySize;
    Initialize(requiredMajor, requiredMinor, useDepth24Stencil8, saveDriverInfo);
}

GLXEngine::GLXEngine(bool useDepth24Stencil8, bool saveDriverInfo, int requiredMajor, int requiredMinor)
    :
    GL45Engine(),
    mDisplay(nullptr),
    mWindow(0),
    mImmediate(nullptr),
    mIsComputeWindow(false)
{
    // Connect to the X server.
    mDisplay = XOpenDisplay(0);
    if (!mDisplay)
    {
        LogError("XOpenDisplay failed.");
        return;
    }

    // Make sure the X server has OpenGL GLX extensions.
    int errorBase, eventBase;
    Bool success = glXQueryExtension(mDisplay, &errorBase, &eventBase);
    if (!success)
    {
        LogError("glXQueryExtension failed, errorBase = "
            + std::to_string(errorBase)
            + ", eventBase = "
            + std::to_string(eventBase)
            + ".");
        return;
    }

    // Select the attributes for the frame buffer.
    int attributes[256];

    // Create a 32-bit RGBA buffer.
    int i = 0;
    attributes[i++] = GLX_RGBA;
    attributes[i++] = GLX_RED_SIZE;
    attributes[i++] = 8;
    attributes[i++] = GLX_GREEN_SIZE;
    attributes[i++] = 8;
    attributes[i++] = GLX_BLUE_SIZE;
    attributes[i++] = 8;
    attributes[i++] = GLX_ALPHA_SIZE;
    attributes[i++] = 8;

    // depthStencilFormat is ignored, create 24-8 depthstencil buffer.
    if (useDepth24Stencil8)
    {
        attributes[i++] = GLX_DEPTH_SIZE;
        attributes[i++] = 24;
        attributes[i++] = GLX_STENCIL_SIZE;
        attributes[i++] = 8;
    }
    else
    {
        attributes[i++] = GLX_DEPTH_SIZE;
        attributes[i++] = 32;
        attributes[i++] = GLX_STENCIL_SIZE;
        attributes[i++] = 0;
    }

    // Use double buffering.
    attributes[i++] = GLX_DOUBLEBUFFER;
    attributes[i++] = 1;

    // The list is zero terminated.
    attributes[i] = 0;

    // Get an OpenGL-capable visual.
    int screen = DefaultScreen(mDisplay);
    XVisualInfo* visualInfo = glXChooseVisual(mDisplay, screen, attributes);
    if (!visualInfo)
    {
        LogError("glXChooseVisual failed.");
        return;
    }

    // Create an OpenGL rendering context.
    GLXContext sharedList = nullptr;
    Bool directRender = True;
    mImmediate = glXCreateContext(mDisplay, visualInfo, sharedList, directRender);
    if (!mImmediate)
    {
        LogError("glXCreateContext failed.");
        return;
    }

    // Create an X Window with the visual information created by the renderer
    // constructor.  The visual information might not be the default, so
    // create an X colormap to use.
    XID rootWindow = RootWindow(mDisplay, visualInfo->screen);
    Colormap colorMap = XCreateColormap(mDisplay, rootWindow, visualInfo->visual, AllocNone);

    // Set the event mask to include exposure (paint), button presses (mouse),
    // and key presses (keyboard).
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = colorMap;
    windowAttributes.border_pixel = 0;
    windowAttributes.event_mask =
        ButtonPressMask |
        ButtonReleaseMask |
        PointerMotionMask |
        Button1MotionMask |
        Button2MotionMask |
        Button3MotionMask |
        KeyPressMask |
        KeyReleaseMask |
        ExposureMask |
        StructureNotifyMask;

    unsigned int xOrigin = 0, yOrigin = 0, xSize = 16, ySize = 16;
    unsigned int borderWidth = 0;
    unsigned long valueMask = CWBorderPixel | CWColormap | CWEventMask;
    mWindow = XCreateWindow(mDisplay, rootWindow, xOrigin,
        yOrigin, xSize, ySize, borderWidth,
        visualInfo->depth, InputOutput, visualInfo->visual, valueMask, &windowAttributes);

    XSizeHints hints;
    hints.flags = PPosition | PSize;
    hints.x = xOrigin;
    hints.y = yOrigin;
    hints.width = xSize;
    hints.height = ySize;
    XSetNormalHints(mDisplay, mWindow, &hints);

    std::string title = "GL4ComputeWindowClass";
    Pixmap iconPixmap = None;
    int numArguments = 0;
    char** arguments = nullptr;
    XSetStandardProperties(mDisplay, mWindow, title.c_str(),
        title.c_str(), iconPixmap, arguments, numArguments, &hints);

    // Intercept the close-window event when the user selects the
    // window close button.  The event is a "client message".
    Atom wmDelete = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, mWindow, &wmDelete, 1);

    mIsComputeWindow = true;
    Initialize(requiredMajor, requiredMinor, useDepth24Stencil8, saveDriverInfo);
}

bool GLXEngine::IsActive() const
{
    return mImmediate == glXGetCurrentContext();
}

void GLXEngine::MakeActive()
{
    if (mImmediate != glXGetCurrentContext())
    {
        glXMakeCurrent(mDisplay, mWindow, mImmediate);
    }
}

void GLXEngine::DisplayColorBuffer(unsigned int syncInterval)
{
    // TODO: Disable vertical sync if possible.
    (void)syncInterval;

    glXSwapBuffers(mDisplay, mWindow);
}

bool GLXEngine::Initialize(int requiredMajor, int requiredMinor, bool useDepth24Stencil8, bool saveDriverInfo)
{
    if (!glXMakeCurrent(mDisplay, mWindow, mImmediate))
    {
        LogError("glXMakeCurrent failed.");
        glXDestroyContext(mDisplay, mImmediate);
        mDisplay = nullptr;
        mWindow = 0;
        mImmediate = nullptr;
        return false;
    }

    // Get the function pointers for OpenGL; initialize the viewport,
    // default global state, and default font.
    return GL45Engine::Initialize(requiredMajor, requiredMinor, useDepth24Stencil8, saveDriverInfo);
}

void GLXEngine::Terminate()
{
    GL45Engine::Terminate();

    if (mDisplay && mImmediate)
    {
        glXDestroyContext(mDisplay, mImmediate);
    }

    if (mIsComputeWindow)
    {
        XDestroyWindow(mDisplay, mWindow);
        XCloseDisplay(mDisplay);
    }
}
