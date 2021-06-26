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
    class DepthStencilState : public DrawingState
    {
    public:
        enum WriteMask
        {
            MASK_ZERO,
            MASK_ALL
        };

        enum Comparison
        {
            NEVER,
            LESS,
            EQUAL,
            LESS_EQUAL,
            GREATER,
            NOT_EQUAL,
            GREATER_EQUAL,
            ALWAYS
        };

        enum Operation
        {
            OP_KEEP,
            OP_ZERO,
            OP_REPLACE,
            OP_INCR_SAT,
            OP_DECR_SAT,
            OP_INVERT,
            OP_INCR,
            OP_DECR
        };

        struct Face
        {
            Operation fail;
            Operation depthFail;
            Operation pass;
            Comparison comparison;
        };

        // Construction.
        DepthStencilState();

        // Member access.  The members are intended to be write-once before
        // you create an associated graphics state.
        bool depthEnable;                   // default: true
        WriteMask writeMask;                // default: MASK_ALL
        Comparison comparison;              // default: LESS_EQUAL
        bool stencilEnable;                 // default: false
        unsigned char stencilReadMask;      // default: 0xFF
        unsigned char stencilWriteMask;     // default: 0xFF
        Face frontFace;                     // default: (KEEP,KEEP,KEEP,ALWAYS)
        Face backFace;                      // default: (KEEP,KEEP,KEEP,ALWAYS)
        unsigned int reference;             // default: 0
    };
}
