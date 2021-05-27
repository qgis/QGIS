// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DrawTarget.h>
#include <Graphics/DX11/DX11DrawTarget.h>
using namespace gte;

DX11DrawTarget::DX11DrawTarget(DrawTarget const* target,
    std::vector<DX11TextureRT*>& rtTextures, DX11TextureDS* dsTexture)
    :
    GEDrawTarget(target),
    mRTTextures(rtTextures),
    mDSTexture(dsTexture),
    mRTViews(target->GetNumTargets()),
    mDSView(nullptr),
    mSaveRTViews(target->GetNumTargets()),
    mSaveDSView(nullptr)
{
    unsigned int const numTargets = mTarget->GetNumTargets();
    for (unsigned int i = 0; i < numTargets; ++i)
    {
        mRTViews[i] = mRTTextures[i]->GetRTView();
        mSaveRTViews[i] = nullptr;
#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
        mRTTextures[i]->SetName(target->GetRTTexture(i)->GetName());
#endif
    }

    if (mDSTexture)
    {
        mDSView = mDSTexture->GetDSView();
#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
        mDSTexture->SetName(target->GetDSTexture()->GetName());
#endif
    }
}

std::shared_ptr<GEDrawTarget> DX11DrawTarget::Create(DrawTarget const* target,
    std::vector<GEObject*>& rtTextures, GEObject* dsTexture)
{
    std::vector<DX11TextureRT*> dxRTTextures(rtTextures.size());
    for (size_t i = 0; i < rtTextures.size(); ++i)
    {
        dxRTTextures[i] = static_cast<DX11TextureRT*>(rtTextures[i]);
    }
    DX11TextureDS* dxDSTexture = static_cast<DX11TextureDS*>(dsTexture);

    return std::make_shared<DX11DrawTarget>(target, dxRTTextures, dxDSTexture);
}

void DX11DrawTarget::Enable (ID3D11DeviceContext* context)
{
    UINT numViewports = 1;
    context->RSGetViewports(&numViewports, &mSaveViewport);

    UINT const numTargets = (UINT)mTarget->GetNumTargets();
    context->OMGetRenderTargets(numTargets, &mSaveRTViews[0], &mSaveDSView);

    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<float>(mTarget->GetWidth());
    viewport.Height = static_cast<float>(mTarget->GetHeight());
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    context->OMSetRenderTargets(numTargets, &mRTViews[0], mDSView);
}

void DX11DrawTarget::Disable (ID3D11DeviceContext* context)
{
    context->RSSetViewports(1, &mSaveViewport);

    UINT const numTargets = (UINT)mTarget->GetNumTargets();
    context->OMSetRenderTargets(numTargets, &mSaveRTViews[0], mSaveDSView);
    for (unsigned int i = 0; i < numTargets; ++i)
    {
        if (mSaveRTViews[i])
        {
            DX11::SafeRelease(mSaveRTViews[i]);
        }
    }
    if (mSaveDSView)
    {
        DX11::SafeRelease(mSaveDSView);
    }
}
