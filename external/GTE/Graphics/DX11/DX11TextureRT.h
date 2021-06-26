// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureRT.h>
#include <Graphics/DX11/DX11Texture2.h>

namespace gte
{
    class DX11TextureRT : public DX11Texture2
    {
    public:
        // Construction and destruction.
        virtual ~DX11TextureRT();
        DX11TextureRT(ID3D11Device* device, TextureRT const* texture);
        DX11TextureRT(ID3D11Device* device, DX11TextureRT const* dxSharedTexture);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline ID3D11RenderTargetView* GetRTView() const
        {
            return mRTView;
        }

        // Support for the DX11 debug layer; see comments in the file
        // DX11GraphicsObject.h about usage.
        virtual void SetName(std::string const& name);

    private:
        // Support for construction.
        void CreateRTView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);

        ID3D11RenderTargetView* mRTView;
    };
}
