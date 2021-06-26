// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GraphicsObject.h>

namespace gte
{
    class DrawingState : public GraphicsObject
    {
    protected:
        // Abstract base class for grouping state classes.  This supports
        // simplification and reduction of member functions in the graphics
        // engine code.
        DrawingState();
    };
}
