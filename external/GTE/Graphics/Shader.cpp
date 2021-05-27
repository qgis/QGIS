// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Shader.h>
#include <functional>
using namespace gte;

Shader::Shader(GraphicsObjectType type)
    :
    GraphicsObject(type),
    mNumXThreads(0),
    mNumYThreads(0),
    mNumZThreads(0)
{
}

int Shader::Get(std::string const& name) const
{
    for (int lookup = 0; lookup < NUM_LOOKUP_INDICES; ++lookup)
    {
        int handle = 0;
        for (auto const& data : mData[lookup])
        {
            if (name == data.name)
            {
                return handle;
            }
            ++handle;
        }
    }
    return -1;
}

unsigned int Shader::GetConstantBufferSize(int handle) const
{
    auto const& data = mData[ConstantBuffer::shaderDataLookup];
    LogAssert(0 <= handle && handle < static_cast<int>(data.size()),
        "Invalid handle for object.");
    return data[handle].numBytes;
}

unsigned int Shader::GetConstantBufferSize(std::string const& name) const
{
    int handle = 0;
    for (auto& data : mData[ConstantBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            return data.numBytes;
        }
        ++handle;
    }
    LogError("Cannot find object " + name + ".");
}

unsigned int Shader::GetTextureBufferSize(int handle) const
{
    auto const& data = mData[TextureBuffer::shaderDataLookup];
    LogAssert(0 <= handle && handle < static_cast<int>(data.size()),
        "Invalid handle for object.");
    return data[handle].numBytes;
}

unsigned int Shader::GetTextureBufferSize(std::string const& name) const
{
    int handle = 0;
    for (auto& data : mData[TextureBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            return data.numBytes;
        }
        ++handle;
    }
    LogError("Cannot find object " + name + ".");
}

unsigned int Shader::GetStructuredBufferSize(int handle) const
{
    auto const& data = mData[StructuredBuffer::shaderDataLookup];
    LogAssert(0 <= handle && handle < static_cast<int>(data.size()),
        "Invalid handle for object.");
    return data[handle].numBytes;
}

unsigned int Shader::GetStructuredBufferSize(std::string const& name) const
{
    int handle = 0;
    for (auto& data : mData[StructuredBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            return data.numBytes;
        }
        ++handle;
    }
    LogError("Cannot find object " + name + ".");
}

void Shader::GetConstantBufferLayout(int handle, BufferLayout& layout) const
{
    auto const& data = mData[ConstantBuffer::shaderDataLookup];
    LogAssert(0 <= handle && handle < static_cast<int>(data.size()),
        "Invalid handle for object.");
    layout = mCBufferLayouts[handle];
}

void Shader::GetConstantBufferLayout(std::string const& name, BufferLayout& layout) const
{
    int handle = 0;
    for (auto& data : mData[ConstantBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            layout = mCBufferLayouts[handle];
            return;
        }
        ++handle;
    }
    LogError("Cannot find object " + name + ".");
}

void Shader::GetTextureBufferLayout(int handle, BufferLayout& layout) const
{
    auto const& data = mData[TextureBuffer::shaderDataLookup];
    LogAssert(0 <= handle && handle < static_cast<int>(data.size()),
        "Invalid handle for object.");
    layout = mTBufferLayouts[handle];
}

void Shader::GetTextureBufferLayout(std::string const& name, BufferLayout& layout) const
{
    int handle = 0;
    for (auto& data : mData[TextureBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            layout = mTBufferLayouts[handle];
            return;
        }
        ++handle;
    }
    LogError("Cannot find object " + name + ".");
}

void Shader::GetStructuredBufferLayout(int handle, BufferLayout& layout) const
{
    auto const& data = mData[StructuredBuffer::shaderDataLookup];
    LogAssert(0 <= handle && handle < static_cast<int>(data.size()),
        "Invalid handle for object.");
    layout = mSBufferLayouts[handle];
}

void Shader::GetStructuredBufferLayout(std::string const& name, BufferLayout& layout) const
{
    int handle = 0;
    for (auto& data : mData[StructuredBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            layout = mSBufferLayouts[handle];
            return;
        }
        ++handle;
    }
    LogError("Cannot find object " + name + ".");
}
