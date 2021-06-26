// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/RawBuffer.h>
#include <Graphics/GL45/GL45Buffer.h>

namespace gte
{
    class GL45AtomicCounterBuffer : public GL45Buffer
    {
    public:
        // Construction.
        virtual ~GL45AtomicCounterBuffer() = default;
        GL45AtomicCounterBuffer(RawBuffer const* cbuffer);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline RawBuffer* GetRawBuffer() const
        {
            return static_cast<RawBuffer*>(mGTObject);
        }

        // Bind the raw buffer data to the specified atomic counter buffer unit.
        void AttachToUnit(GLint atomicCounterBufferUnit);
    };
}
