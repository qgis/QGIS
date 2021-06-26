// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ViewVolume.h>
#include <Graphics/Lighting.h>
#include <memory>

namespace gte
{
    class Light : public ViewVolume
    {
    public:
        // Construction.  The depth range for DirectX is [0,1] and for OpenGL
        // is [-1,1].  For DirectX, set isDepthRangeZeroToOne to true.  For
        // OpenGL, set isDepthRangeZeroOne to false.
        Light(bool isPerspective, bool isDepthRangeZeroOne);

        std::shared_ptr<Lighting> lighting;
    };
}
