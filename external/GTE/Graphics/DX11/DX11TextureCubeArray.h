// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureCubeArray.h>
#include <Graphics/DX11/DX11TextureArray.h>

namespace gte
{
    class DX11TextureCubeArray : public DX11TextureArray
    {
    public:
        // Construction and destruction.
        virtual ~DX11TextureCubeArray() = default;
        DX11TextureCubeArray(ID3D11Device* device, TextureCubeArray const* textureCubeArray);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline TextureCubeArray* GetTextureArray() const
        {
            return static_cast<TextureCubeArray*>(mGTObject);
        }

        inline ID3D11Texture2D* GetDXTextureArray() const
        {
            return static_cast<ID3D11Texture2D*>(mDXObject);
        }

    private:
        // Support for construction.
        void CreateStaging(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
        void CreateSRView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
        void CreateUAView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    };
}
