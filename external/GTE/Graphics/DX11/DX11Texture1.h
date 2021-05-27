// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture1.h>
#include <Graphics/DX11/DX11TextureSingle.h>

namespace gte
{
    class DX11Texture1 : public DX11TextureSingle
    {
    public:
        // Construction.
        DX11Texture1(ID3D11Device* device, Texture1 const* texture);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline Texture1* GetTexture() const
        {
            return static_cast<Texture1*>(mGTObject);
        }

        inline ID3D11Texture1D* GetDXTexture() const
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
