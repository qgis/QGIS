// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture1Array.h>
#include <Graphics/DX11/DX11TextureArray.h>

namespace gte
{
    class DX11Texture1Array : public DX11TextureArray
    {
    public:
        // Construction and destruction.
        virtual ~DX11Texture1Array() = default;
        DX11Texture1Array(ID3D11Device* device, Texture1Array const* textureArray);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline Texture1Array* GetTextureArray() const
        {
            return static_cast<Texture1Array*>(mGTObject);
        }

        inline ID3D11Texture1D* GetDXTextureArray() const
        {
            return static_cast<ID3D11Texture1D*>(mDXObject);
        }

    private:
        // Support for construction.
        void CreateStaging(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
        void CreateSRView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
        void CreateUAView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
    };
}
