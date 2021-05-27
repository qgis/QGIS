// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DrawingState.h>
#include <Graphics/DX11/DX11GraphicsObject.h>

namespace gte
{
    class DX11DrawingState : public DX11GraphicsObject
    {
    public:
        // Abstract base class.
        virtual ~DX11DrawingState() = default;
    protected:
        DX11DrawingState(DrawingState const* gtState);
    };
}
