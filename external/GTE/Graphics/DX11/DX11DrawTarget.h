// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GEDrawTarget.h>
#include <Graphics/DX11/DX11TextureDS.h>
#include <Graphics/DX11/DX11TextureRT.h>

namespace gte
{
    class DX11DrawTarget : public GEDrawTarget
    {
    public:
        // Construction and destruction.
        virtual ~DX11DrawTarget() = default;
        DX11DrawTarget(DrawTarget const* target,
            std::vector<DX11TextureRT*>& rtTextures, DX11TextureDS* dsTexture);

        static std::shared_ptr<GEDrawTarget> Create(DrawTarget const* target,
            std::vector<GEObject*>& rtTextures, GEObject* dsTexture);

        // Member access.
        inline DX11TextureRT* GetRTTexture(unsigned int i) const
        {
            return mRTTextures[i];
        }

        inline DX11TextureDS* GetDSTexture() const
        {
            return mDSTexture;
        }

        // Used in the Renderer::Draw function.
        void Enable(ID3D11DeviceContext* context);
        void Disable(ID3D11DeviceContext* context);

    private:
        std::vector<DX11TextureRT*> mRTTextures;
        DX11TextureDS* mDSTexture;

        // Convenient storage for enable/disable of targets.
        std::vector<ID3D11RenderTargetView*> mRTViews;
        ID3D11DepthStencilView* mDSView;

        // Temporary storage during enable/disable of targets.
        D3D11_VIEWPORT mSaveViewport;
        std::vector<ID3D11RenderTargetView*> mSaveRTViews;
        ID3D11DepthStencilView* mSaveDSView;
    };
}
