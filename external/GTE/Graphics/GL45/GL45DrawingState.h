// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DrawingState.h>
#include <Graphics/GL45/GL45GraphicsObject.h>

namespace gte
{
    class GL45DrawingState : public GL45GraphicsObject
    {
    public:
        // Abstract base class.
        virtual ~GL45DrawingState() = default;
    protected:
        GL45DrawingState(DrawingState const* gtState);
    };
}
