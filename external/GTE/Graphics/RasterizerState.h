// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DrawingState.h>

namespace gte
{
    class RasterizerState : public DrawingState
    {
    public:
        enum FillMode
        {
            FILL_SOLID,
            FILL_WIREFRAME
        };

        enum CullMode
        {
            CULL_NONE,
            CULL_FRONT,
            CULL_BACK
        };

        // Construction.
        RasterizerState();

        // Member access.  The members are intended to be write-once before
        // you create an associated graphics state.
        FillMode fillMode;              // default: FILL_SOLID
        CullMode cullMode;              // default: CULL_BACK
        bool frontCCW;                  // default: true
        int depthBias;                  // default: 0
        float depthBiasClamp;           // default: 0
        float slopeScaledDepthBias;     // default: 0
        bool enableDepthClip;           // default: true
        bool enableScissor;             // default: false
        bool enableMultisample;         // default: false
        bool enableAntialiasedLine;     // default: false
    };
}
