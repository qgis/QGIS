// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VertexBuffer.h>
#include <Graphics/DX11/DX11Buffer.h>

namespace gte
{
    class DX11VertexBuffer : public DX11Buffer
    {
    public:
        // Construction.
        virtual ~DX11VertexBuffer() = default;
        DX11VertexBuffer(ID3D11Device* device, VertexBuffer const* vbuffer);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline VertexBuffer* GetVertexBuffer() const
        {
            return static_cast<VertexBuffer*>(mGTObject);
        }

        // Drawing support.
        void Enable(ID3D11DeviceContext* context);
        void Disable(ID3D11DeviceContext* context);
    };
}
