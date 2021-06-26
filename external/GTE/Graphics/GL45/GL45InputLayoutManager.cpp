// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Mathematics/Logger.h>
#include <Graphics/GL45/GL45InputLayoutManager.h>
using namespace gte;

GL45InputLayoutManager::~GL45InputLayoutManager()
{
    if (mMap.HasElements())
    {
        LogWarning("Input layout map is not empty on destruction.");
        UnbindAll();
    }
}

GL45InputLayout* GL45InputLayoutManager::Bind(GLuint programHandle,
    GLuint vbufferHandle, VertexBuffer const* vbuffer)
{
    std::shared_ptr<GL45InputLayout> layout;
    if (programHandle)
    {
        if (vbuffer)
        {
            if (!mMap.Get(std::make_pair(vbuffer, programHandle), layout))
            {
                layout = std::make_shared<GL45InputLayout>(programHandle, vbufferHandle, vbuffer);
                mMap.Insert(std::make_pair(vbuffer, programHandle), layout);
            }
        }
        // else: A null vertex buffer is passed when an effect wants to
        // bypass the input assembler.
        return layout.get();
    }
    else
    {
        LogError("Program must exist.");
    }
}

bool GL45InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
{
    if (vbuffer)
    {
        std::vector<VBPPair> matches;
        mMap.GatherMatch(vbuffer, matches);
        for (auto match : matches)
        {
            std::shared_ptr<GL45InputLayout> layout;
            mMap.Remove(match, layout);
        }
        return true;
    }
    else
    {
        LogError("Vertex buffer must be nonnull.");
    }
}

bool GL45InputLayoutManager::Unbind(Shader const*)
{
    return true;
}

void GL45InputLayoutManager::UnbindAll()
{
    mMap.RemoveAll();
}

bool GL45InputLayoutManager::HasElements() const
{
    return mMap.HasElements();
}

void GL45InputLayoutManager::LayoutMap::GatherMatch(
    VertexBuffer const* vbuffer, std::vector<VBPPair>& matches)
{
    mMutex.lock();
    {
        for (auto vbp : mMap)
        {
            if (vbuffer == vbp.first.first)
            {
                matches.push_back(vbp.first);
            }
        }
    }
    mMutex.unlock();
}
