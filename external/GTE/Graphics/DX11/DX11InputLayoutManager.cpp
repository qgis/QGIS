// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11InputLayoutManager.h>
using namespace gte;

DX11InputLayoutManager::~DX11InputLayoutManager()
{
    if (mMap.HasElements())
    {
        LogWarning("Input layout map is not empty on destruction.");
        UnbindAll();
    }
}

DX11InputLayout* DX11InputLayoutManager::Bind(ID3D11Device* device,
    VertexBuffer const* vbuffer, Shader const* vshader)
{
    LogAssert(vshader != nullptr, "Invalid input.");

    std::shared_ptr<DX11InputLayout> layout;
    if (vbuffer)
    {
        if (!mMap.Get(std::make_pair(vbuffer, vshader), layout))
        {
            layout = std::make_shared<DX11InputLayout>(device, vbuffer, vshader);
            mMap.Insert(std::make_pair(vbuffer, vshader), layout);

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
            std::string vbname = vbuffer->GetName();
            std::string vsname = vshader->GetName();
            if (vbname != "" || vsname != "")
            {
                layout->SetName(vbname + " | " + vsname);
            }
#endif
        }
    }
    // else: A null vertex buffer is passed when an effect wants to bypass
    // the input assembler.

    return layout.get();
}

bool DX11InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
{
    LogAssert(vbuffer != nullptr, "Invalid input.");

    std::vector<VBSPair> matches;
    mMap.GatherMatch(vbuffer, matches);
    for (auto match : matches)
    {
        std::shared_ptr<DX11InputLayout> layout;
        mMap.Remove(match, layout);
    }
    return true;
}

bool DX11InputLayoutManager::Unbind(Shader const* vshader)
{
    LogAssert(vshader != nullptr, "Invalid input.");

    std::vector<VBSPair> matches;
    mMap.GatherMatch(vshader, matches);
    for (auto match : matches)
    {
        std::shared_ptr<DX11InputLayout> layout;
        mMap.Remove(match, layout);
    }
    return true;
}

void DX11InputLayoutManager::UnbindAll()
{
    mMap.RemoveAll();
}

bool DX11InputLayoutManager::HasElements() const
{
    return mMap.HasElements();
}

void DX11InputLayoutManager::LayoutMap::GatherMatch(
    VertexBuffer const* vbuffer, std::vector<VBSPair>& matches)
{
    this->mMutex.lock();
    {
        for (auto vbs : this->mMap)
        {
            if (vbuffer == vbs.first.first)
            {
                matches.push_back(vbs.first);
            }
        }
    }
    this->mMutex.unlock();
}

void DX11InputLayoutManager::LayoutMap::GatherMatch(Shader const* vshader,
    std::vector<VBSPair>& matches)
{
    this->mMutex.lock();
    {
        for (auto vbs : this->mMap)
        {
            if (vshader == vbs.first.second)
            {
                matches.push_back(vbs.first);
            }
        }
    }
    this->mMutex.unlock();
}
