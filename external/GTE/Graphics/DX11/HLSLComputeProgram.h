// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ComputeProgram.h>

namespace gte
{
    class HLSLComputeProgram : public ComputeProgram
    {
    public:
        // A simple stub to add HLSL as part of the program type.  This allows
        // polymorphism for the program factory classes, which in turn allows
        // us to hide the graphics-API-dependent program factory used by the
        // Window class (have a member mProgramFactory similar to mEngine that
        // is created according to the desired graphics API).
        virtual ~HLSLComputeProgram() = default;
        HLSLComputeProgram() = default;
    };
}
