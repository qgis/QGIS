// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Buffer.h>

// RawBuffer is currently supported only in the DirectX graphics engine.

namespace gte
{
    class RawBuffer : public Buffer
    {
    public:
        // Construction.  The element size is always 4 bytes.
        RawBuffer(unsigned int numElements, bool createStorage = true);

    public:
        // For use by the Shader class for storing reflection information.
        static int const shaderDataLookup = 3;
    };
}
