// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector4.h>

namespace gte
{
    struct LightCameraGeometry
    {
        // Construction.
        LightCameraGeometry();

        Vector4<float> lightModelPosition;      // default: (0,0,0,1)
        Vector4<float> lightModelDirection;     // default: (0,0,-1,0)
        Vector4<float> lightModelUp;            // default: (0,1,0,0)
        Vector4<float> lightModelRight;         // default: (1,0,0,0)

        Vector4<float> cameraModelPosition;     // default: (0,0,0,1)
        Vector4<float> cameraModelDirection;    // default: (0,0,-1,0)
        Vector4<float> cameraModelUp;           // default: (0,1,0,0)
        Vector4<float> cameraModelRight;        // default: (1,0,0,0)
    };
}
