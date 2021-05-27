// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ConstantBuffer.h>
#include <Graphics/GL45/GL45Buffer.h>

namespace gte
{
    class GL45ConstantBuffer : public GL45Buffer
    {
    public:
        // Construction.
        virtual ~GL45ConstantBuffer() = default;
        GL45ConstantBuffer(ConstantBuffer const* cbuffer);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline ConstantBuffer* GetConstantBuffer() const
        {
            return static_cast<ConstantBuffer*>(mGTObject);
        }

        // Bind the constant buffer data to the specified uniform buffer unit.
        void AttachToUnit(GLint uniformBufferUnit);
    };
}
