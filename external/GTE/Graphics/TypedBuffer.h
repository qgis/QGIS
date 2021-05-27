// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Buffer.h>

// TypedBuffer is currently supported only in the DirectX graphics engine.

namespace gte
{
    class TypedBuffer : public Buffer
    {
    public:
        // Abstract base class.
        virtual ~TypedBuffer() = default;
    protected:
        TypedBuffer(unsigned int numElements, size_t elementSize, bool createStorage = true);
    };
}
