// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture2.h>
#include <Graphics/DX11/DX11TextureSingle.h>

namespace gte
{
    class DX11Texture2 : public DX11TextureSingle
    {
    public:
        // Construction and destruction.
        virtual ~DX11Texture2() = default;
        DX11Texture2(ID3D11Device* device, Texture2 const* texture);
        DX11Texture2(ID3D11Device* device, DX11Texture2 const* dxSharedTexture);

        // Create a texture with already known ID3D11Texture2 and
        // ID3D11ShaderResourceView interfaces.  TODO: For now, this allows
        // creation of DDS textures to be used only as shader resource views,
        // not as unordered access views, and there is no associated staging
        // texture.
        DX11Texture2(Texture2 const* texture, ID3D11Texture2D* dxTexture,
            ID3D11ShaderResourceView* dxSRView);

    protected:
        // Constructors for DX11TextureRT and DX11TextureDS.
        DX11Texture2(Texture2 const* texture);

    public:
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline Texture2* GetTexture() const
        {
            return static_cast<Texture2*>(mGTObject);
        }

        inline ID3D11Texture2D* GetDXTexture() const
        {
            return static_cast<ID3D11Texture2D*>(mDXObject);
        }

    protected:
        // Support for construction.
        ID3D11Texture2D* CreateSharedDXObject(ID3D11Device* device) const;
        void CreateStaging(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
        void CreateSRView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
        void CreateUAView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    };
}
