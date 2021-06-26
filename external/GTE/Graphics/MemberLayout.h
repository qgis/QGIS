// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <string>
#include <vector>

namespace gte
{
    // Support for generation of lookup tables for constant buffers and
    // texture buffers.  Given the name of a member of a buffer, get the
    // offset into the buffer memory where the member lives.  The layout
    // is specific to the shading language (HLSL, GLSL).
    struct MemberLayout
    {
        std::string name;
        unsigned int offset;
        unsigned int numElements;
    };

    typedef std::vector<MemberLayout> BufferLayout;
}
