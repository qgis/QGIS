// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#if defined(GTE_USE_MSWINDOWS)
#include <Applications/MSW/Window.h>
#endif
#if defined(GTE_USE_LINUX)
#include <Applications/GLX/Window.h>
#endif
#include <Graphics/OverlayEffect.h>

// The 2D application support is for GTEngine samples and is not a
// general-purpose 2D windowing layer.  The window resizing is disabled to
// avoid the complications of resizing the overlay texture and redrawing
// objects that were initially defined in the original window coordinates.

namespace gte
{
    class Window2 : public Window
    {
    protected:
        // Abstract base class.  The window creation requires
        // parameters.allowResize to be 'false'; otherwise, the creation
        // fails.
        Window2(Parameters& parameters);

    public:
        // Display functions.  The OnResize function always returns 'false';
        // that is, you cannot resize the window.  The DrawScreenOverlay
        // function is called after the screen texture is drawn but before the
        // swap-buffers call is made.  This allows you to draw text or GUI
        // elements on top of the screen texture.
        virtual bool OnResize(int xSize, int ySize) override;
        virtual void OnDisplay() override;
        virtual void DrawScreenOverlay();

        // Drawing functions.  Each color is packed as R8G8B8A8 with the alpha
        // channel the most significant bits and red the least significant
        // bits.  For example, (r,g,b,a) = (1,2,3,4) is 0x04030201.

        // Set all pixels to the specified color.
        void ClearScreen(unsigned int color);

        // Set the pixel at location (x,y) to the specified color.
        void SetPixel(int x, int y, unsigned int color);

        // Get the pixel color at location (x,y).
        unsigned int GetPixel(int x, int y);

        // Set the pixels (x',y') for x-thick <= x' <= x+thick and
        // y-thick <= y' <= y+thick.
        void DrawThickPixel(int x, int y, int thick, unsigned int color);

        // Use Bresenham's algorithm to draw the line from (x0,y0) to (x1,y1)
        // using the specified color for the drawn pixels.  The algorithm is
        // biased in that the pixels set by DrawLine(x0,y0,x1,y1) are not
        // necessarily the same as those set by DrawLine(x1,y1,x0,y0).
        // TODO: Implement the midpoint algorithm to avoid the bias.
        void DrawLine(int x0, int y0, int x1, int y1, unsigned int color);
        void DrawThickLine(int x0, int y0, int x1, int y1, int thick, unsigned int color);

        // Draw an axis-aligned rectangle using the specified color.  The
        // 'solid' parameter indicates whether or not to fill the rectangle.
        void DrawRectangle(int xMin, int yMin, int xMax, int yMax, unsigned int color, bool solid);
        void DrawThickRectangle(int xMin, int yMin, int xMax, int yMax, int thick, unsigned int color, bool solid);

        // Use Bresenham's algorithm to draw the circle centered at
        // (xCenter,yCenter) with the specified 'radius' and using the
        // specified color.  The 'solid' parameter indicates whether or not
        // to fill the circle.
        void DrawCircle(int xCenter, int yCenter, int radius, unsigned int color, bool solid);
        void DrawThickCircle(int xCenter, int yCenter, int radius, int thick, unsigned int color, bool solid);

        // Use Bresenham's algorithm to draw the axis-aligned ellipse
        // ((x-xc)/a)^2 + ((y-yc)/b)^2 = 1, where xCenter is xc, yCenter
        // is yc, xExtent is a, and yExtent is b.
        void DrawEllipse(int xCenter, int yCenter, int xExtent, int yExtent, unsigned int color);
        void DrawThickEllipse(int xCenter, int yCenter, int xExtent, int yExtent, int thick, unsigned int color);

        // Flood-fill a region whose pixels are of color 'backColor' by
        // changing their color to 'foreColor'.  The fill treats the screen
        // as 4-connected; that is, after (x,y) is visited, then (x-1,y),
        // (x+1,y), (x,y-1), and (x,y+1) are visited (as long as they are in
        // the screen boundary).  The function simulates recursion by using
        // stacks, which avoids the expense of true recursion and the
        // potential to overflow the calling stack.
        void DrawFloodFill4(int x, int y, unsigned int foreColor, unsigned int backColor);

    protected:
        std::shared_ptr<OverlayEffect> mOverlay;
        std::shared_ptr<Texture2> mScreenTexture;
        std::shared_ptr<DepthStencilState> mNoDepthStencilState;
        std::function<void(int, int)> mDrawPixel;
        std::function<void(int, int)> mDrawThickPixel;
        unsigned int mPixelColor;
        int mThick;
        bool mClampToWindow;
        bool mDoFlip;
        bool mScreenTextureNeedsUpdate;
    };
}
