// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/DrawTarget.h>
#include <Mathematics/Logger.h>
using namespace gte;

DrawTarget::~DrawTarget()
{
    msLFDMutex.lock();
    {
        for (auto listener : msLFDSet)
        {
            listener->OnDestroy(this);
        }
    }
    msLFDMutex.unlock();
}

DrawTarget::DrawTarget(unsigned int numRenderTargets, DFType rtFormat,
    unsigned int width, unsigned int height, bool hasRTMipmaps,
    bool createRTStorage, DFType dsFormat, bool createDSStorage)
{
    LogAssert(numRenderTargets > 0, "Number of targets must be at least one.");
    mRTTextures.resize(numRenderTargets);
    for (auto& texture : mRTTextures)
    {
        texture = std::make_shared<TextureRT>(rtFormat, width, height,
            hasRTMipmaps, createRTStorage);
    }

    if (dsFormat != DF_UNKNOWN)
    {
        if (DataFormat::IsDepth(dsFormat))
        {
            mDSTexture = std::make_shared<TextureDS>(dsFormat, width,
                height, createDSStorage);
            return;
        }
        LogError("Invalid depth-stencil format.");
    }
}

DFType DrawTarget::GetRTFormat() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->GetFormat();
}

unsigned int DrawTarget::GetWidth() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->GetWidth();
}

unsigned int DrawTarget::GetHeight() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->GetHeight();
}

bool DrawTarget::HasRTMipmaps() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->HasMipmaps();
}

DFType DrawTarget::GetDSFormat() const
{
    LogAssert(mDSTexture != nullptr, "Unexpected condition.");
    return mDSTexture->GetFormat();
}

std::shared_ptr<TextureRT> const DrawTarget::GetRTTexture(unsigned int i) const
{
    LogAssert(i < static_cast<unsigned int>(mRTTextures.size()), "Unexpected condition.");
    return mRTTextures[i];
}

void DrawTarget::AutogenerateRTMipmaps()
{
    if (HasRTMipmaps())
    {
        for (auto& texture : mRTTextures)
        {
            texture->AutogenerateMipmaps();
        }
    }
}

bool DrawTarget::WantAutogenerateRTMipmaps() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->WantAutogenerateMipmaps();
}

void DrawTarget::SubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.insert(listener);
    }
    msLFDMutex.unlock();
}

void DrawTarget::UnsubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.erase(listener);
    }
    msLFDMutex.unlock();
}


std::mutex DrawTarget::msLFDMutex;
std::set<std::shared_ptr<DrawTarget::ListenerForDestruction>> DrawTarget::msLFDSet;
