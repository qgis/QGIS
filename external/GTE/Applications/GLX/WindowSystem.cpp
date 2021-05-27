// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/GLX/WindowSystem.h>
#include <Applications/GLX/Window.h>
#include <Graphics/GL45/GLX/GLXEngine.h>
#include <Graphics/GL45/GLSLProgramFactory.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
using namespace gte;

// The singleton used to create and destroy windows for applications.
namespace gte
{
    WindowSystem TheWindowSystem;
}

WindowSystem::~WindowSystem()
{
    XCloseDisplay(mDisplay);
}

WindowSystem::WindowSystem()
    :
    mDisplayName("GTEngineWindow"),
    mDisplay(nullptr)
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
}

bool WindowSystem::Create(_XDisplay*& display, __GLXcontextRec*& context, unsigned long& window, bool useDepth24Stencil8)
{
    display = mDisplay;
    context = nullptr;
    window = 0;

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
        return false;
    }

    // Create an OpenGL rendering context.
    GLXContext sharedList = nullptr;
    Bool directRender = True;
    context = glXCreateContext(mDisplay, visualInfo, sharedList, directRender);
    if (!context)
    {
        LogError("glXCreateContext failed.");
        return false;
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
    window = XCreateWindow(mDisplay, rootWindow, xOrigin,
        yOrigin, xSize, ySize, borderWidth,
        visualInfo->depth, InputOutput, visualInfo->visual, valueMask, &windowAttributes);

    XSizeHints hints;
    hints.flags = PPosition | PSize;
    hints.x = xOrigin;
    hints.y = yOrigin;
    hints.width = xSize;
    hints.height = ySize;
    XSetNormalHints(mDisplay, window, &hints);

    std::string title = "GL4ComputeWindowClass";
    Pixmap iconPixmap = None;
    int numArguments = 0;
    char** arguments = nullptr;
    XSetStandardProperties(mDisplay, window, title.c_str(),
        title.c_str(), iconPixmap, arguments, numArguments, &hints);

    // Intercept the close-window event when the user selects the
    // window close button.  The event is a "client message".
    Atom wmDelete = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, window, &wmDelete, 1);
    return true;
}

void WindowSystem::CreateFrom(Window::Parameters& parameters)
{
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
    if (parameters.useDepth24Stencil8)
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
        parameters.created = false;
        return;
    }

    // Create an OpenGL rendering context.
    GLXContext sharedList = nullptr;
    Bool directRender = True;
    GLXContext context = glXCreateContext(mDisplay, visualInfo, sharedList, directRender);
    if (!context)
    {
        LogError("glXCreateContext failed.");
        parameters.created = false;
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

    unsigned int borderWidth = 0;
    unsigned long valueMask = CWBorderPixel | CWColormap | CWEventMask;
    parameters.window = XCreateWindow(mDisplay, rootWindow, parameters.xOrigin,
        parameters.yOrigin, parameters.xSize, parameters.ySize, borderWidth,
        visualInfo->depth, InputOutput, visualInfo->visual, valueMask, &windowAttributes);

    XSizeHints hints;
    hints.flags = PPosition | PSize;
    hints.x = parameters.xOrigin;
    hints.y = parameters.yOrigin;
    hints.width = parameters.xSize;
    hints.height = parameters.ySize;
    XSetNormalHints(mDisplay, parameters.window, &hints);

    std::string title(parameters.title.begin(), parameters.title.end());
    Pixmap iconPixmap = None;
    int numArguments = 0;
    char** arguments = nullptr;
    XSetStandardProperties(mDisplay, parameters.window, title.c_str(),
        title.c_str(), iconPixmap, arguments, numArguments, &hints);

    // Intercept the close-window event when the user selects the
    // window close button.  The event is a "client message".
    Atom wmDelete = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, parameters.window, &wmDelete, 1);

    // Create a GLX rendering engine.
    auto engine = std::make_shared<GLXEngine>(mDisplay, parameters.window, context,
        parameters.xSize, parameters.ySize, parameters.useDepth24Stencil8, (parameters.deviceCreationFlags != 0));
    if (!engine->MeetsRequirements())
    {
        LogError("OpenGL 4.3 or later is required.");
        parameters.display = nullptr;
        parameters.engine = nullptr;
        parameters.factory = nullptr;
        parameters.created = false;
        return;
    }

    if (engine->GetDisplay())
    {
        parameters.display = mDisplay;
        parameters.engine = engine;
        parameters.factory = std::make_shared<GLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create graphics engine.");
        parameters.display = nullptr;
        parameters.window = 0;
        parameters.engine = nullptr;
        parameters.factory = nullptr;
        parameters.created = false;
    }
}
