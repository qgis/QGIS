// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/WindowApplication.h>
using namespace gte;

WindowApplication::Parameters::Parameters()
    :
    title(L""),
    xOrigin(0),
    yOrigin(0),
    xSize(0),
    ySize(0),
    allowResize(false),
    useDepth24Stencil8(true),
    created(false)
{
}

WindowApplication::Parameters::Parameters(std::wstring const& inTitle,
    int inXOrigin, int inYOrigin, int inXSize, int inYSize)
    :
    title(inTitle),
    xOrigin(inXOrigin),
    yOrigin(inYOrigin),
    xSize(inXSize),
    ySize(inYSize),
    allowResize(false),
    useDepth24Stencil8(true),
    created(false)
{
}

WindowApplication::WindowApplication(Parameters& parameters)
    :
    Application(parameters),
    mTitle(parameters.title),
    mXOrigin(parameters.xOrigin),
    mYOrigin(parameters.yOrigin),
    mXSize(parameters.xSize),
    mYSize(parameters.ySize),
    mAllowResize(parameters.allowResize),
    mIsMinimized(false),
    mIsMaximized(false)
{
}

void WindowApplication::OnMove(int x, int y)
{
    mXOrigin = x;
    mYOrigin = y;
}

bool WindowApplication::OnResize(int xSize, int ySize)
{
    mIsMinimized = false;
    mIsMaximized = false;

    if (xSize != mXSize || ySize != mYSize)
    {
        mXSize = xSize;
        mYSize = ySize;

        if (mBaseEngine)
        {
            mBaseEngine->Resize(xSize, ySize);
        }
        return true;
    }

    return false;
}

void WindowApplication::OnMinimize()
{
    mIsMinimized = true;
    mIsMaximized = false;
}

void WindowApplication::OnMaximize()
{
    mIsMinimized = false;
    mIsMaximized = true;
}

void WindowApplication::OnDisplay()
{
    // Stub for derived classes.
}

void WindowApplication::OnIdle()
{
    // Stub for derived classes.
}

bool WindowApplication::OnCharPress(unsigned char key, int, int)
{
    if (key == KEY_ESCAPE)
    {
        // Quit the application when the 'escape' key is pressed.
        OnClose();
        return true;
    }

    if (key == ' ')
    {
        mTimer.Reset();
        return true;
    }

    return false;
}

bool WindowApplication::OnKeyDown(int, int, int)
{
    // Stub for derived classes.
    return false;
}

bool WindowApplication::OnKeyUp(int, int, int)
{
    // Stub for derived classes.
    return false;
}

bool WindowApplication::OnMouseClick(int, int, int, int, unsigned int)
{
    // stub for derived classes
    return false;
}

bool WindowApplication::OnMouseMotion(int, int, int, unsigned int)
{
    // stub for derived classes
    return false;
}

bool WindowApplication::OnMouseWheel(int, int, int, unsigned int)
{
    // Stub for derived classes.
    return false;
}

void WindowApplication::SetMousePosition(int, int)
{
    // Stub for derived classes.
}

void WindowApplication::GetMousePosition(int&, int&) const
{
    // Stub for derived classes.
}

void WindowApplication::OnClose()
{
    // Stub for derived classes.
}
