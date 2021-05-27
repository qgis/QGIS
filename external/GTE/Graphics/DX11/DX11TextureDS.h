// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureDS.h>
#include <Graphics/DX11/DX11Texture2.h>

namespace gte
{
    class DX11TextureDS : public DX11Texture2
    {
    public:
        // Construction and destruction.
        virtual ~DX11TextureDS();
        DX11TextureDS(ID3D11Device* device, TextureDS const* texture);
        DX11TextureDS(ID3D11Device* device, DX11TextureDS const* dxSharedTexture);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline ID3D11DepthStencilView* GetDSView() const
        {
            return mDSView;
        }

        // Support for the DX11 debug layer; see comments in the file
        // DX11GraphicsObject.h about usage.
        virtual void SetName(std::string const& name);

    private:
        // Support for construction.
        void CreateDSView(ID3D11Device* device);
        void CreateDSSRView(ID3D11Device* device);
        DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT depthFormat);
        DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthFormat);

        ID3D11DepthStencilView* mDSView;
    };
}
