// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLShader.h>
using namespace gte;

HLSLShader::HLSLShader(HLSLReflection const& reflector, GraphicsObjectType type)
    :
    Shader(type)
{
    mCompiledCode = reflector.GetCompiledCode();
    mNumXThreads = reflector.GetNumXThreads();
    mNumYThreads = reflector.GetNumYThreads();
    mNumZThreads = reflector.GetNumZThreads();

    mCBufferLayouts.resize(reflector.GetCBuffers().size());
    int i = 0;
    for (auto const& cb : reflector.GetCBuffers())
    {
        mData[ConstantBuffer::shaderDataLookup].push_back(
            Data(GT_CONSTANT_BUFFER, cb.GetName(), cb.GetBindPoint(),
                cb.GetNumBytes(), 0, false));

        cb.GenerateLayout(mCBufferLayouts[i]);
        ++i;
    }

    mTBufferLayouts.resize(reflector.GetTBuffers().size());
    i = 0;
    for (auto const& tb : reflector.GetTBuffers())
    {
        mData[TextureBuffer::shaderDataLookup].push_back(
            Data(GT_TEXTURE_BUFFER, tb.GetName(), tb.GetBindPoint(),
                tb.GetNumBytes(), 0, false));

        tb.GenerateLayout(mTBufferLayouts[i]);
        ++i;
    }

    for (auto const& sb : reflector.GetSBuffers())
    {
        unsigned int ctrtype = 0xFFFFFFFFu;
        switch (sb.GetType())
        {
        case HLSLStructuredBuffer::SBT_BASIC:
            ctrtype = StructuredBuffer::CT_NONE;
            break;

        case HLSLStructuredBuffer::SBT_APPEND:
        case HLSLStructuredBuffer::SBT_CONSUME:
            ctrtype = StructuredBuffer::CT_APPEND_CONSUME;
            break;

        case HLSLStructuredBuffer::SBT_COUNTER:
            ctrtype = StructuredBuffer::CT_COUNTER;
            break;

        default:
            LogError("Unexpected structured buffer option: " +
                std::to_string(static_cast<int>(sb.GetType())));
        }

        mData[StructuredBuffer::shaderDataLookup].push_back(
            Data(GT_STRUCTURED_BUFFER, sb.GetName(), sb.GetBindPoint(),
            sb.GetNumBytes(), ctrtype, sb.IsGpuWritable()));
    }

    for (auto const& rb : reflector.GetRBuffers())
    {
        mData[RawBuffer::shaderDataLookup].push_back(
            Data(GT_RAW_BUFFER, rb.GetName(), rb.GetBindPoint(),
            rb.GetNumBytes(), 0, rb.IsGpuWritable()));
    }

    for (auto const& tx : reflector.GetTextures())
    {
        mData[TextureSingle::shaderDataLookup].push_back(
            Data(GT_TEXTURE_SINGLE, tx.GetName(), tx.GetBindPoint(), 0,
            tx.GetNumDimensions(), tx.IsGpuWritable()));
    }

    for (auto const& ta : reflector.GetTextureArrays())
    {
        mData[TextureArray::shaderDataLookup].push_back(
            Data(GT_TEXTURE_ARRAY, ta.GetName(), ta.GetBindPoint(), 0,
            ta.GetNumDimensions(), ta.IsGpuWritable()));
    }

    for (auto const& s : reflector.GetSamplerStates())
    {
        mData[SamplerState::shaderDataLookup].push_back(
            Data(GT_SAMPLER_STATE, s.GetName(), s.GetBindPoint(), 0, 0, false));
    }
}

void HLSLShader::Set(std::string const& textureName, std::shared_ptr<TextureSingle> const& texture,
    std::string const& samplerName, std::shared_ptr<SamplerState> const& state)
{
    Shader::Set(textureName, texture);
    Shader::Set(samplerName, state);
}

void HLSLShader::Set(std::string const& textureName, std::shared_ptr<TextureArray> const& texture,
    std::string const& samplerName, std::shared_ptr<SamplerState> const& state)
{
    Shader::Set(textureName, texture);
    Shader::Set(samplerName, state);
}

bool HLSLShader::IsValid(Data const& goal, ConstantBuffer* resource) const
{
    if (!resource)
    {
        // resource is null
        return false;
    }

    if (goal.type != GT_CONSTANT_BUFFER)
    {
        // mismatch of buffer type
        return false;
    }

    if (resource->GetNumBytes() >= static_cast<size_t>(goal.numBytes))
    {
        return true;
    }

    // invalid number of bytes
    return false;
}

bool HLSLShader::IsValid(Data const& goal, TextureBuffer* resource) const
{
    if (!resource)
    {
        // resource is null
        return false;
    }

    if (goal.type != GT_TEXTURE_BUFFER)
    {
        // mismatch of buffer type
        return false;
    }

    if (resource->GetNumBytes() >= static_cast<size_t>(goal.numBytes))
    {
        return true;
    }

    // invalid number of bytes
    return false;
}

bool HLSLShader::IsValid(Data const& goal, StructuredBuffer* resource) const
{
    if (!resource)
    {
        // resource is null
        return false;
    }

    if (goal.type != GT_STRUCTURED_BUFFER)
    {
        // invalid number of bytes
        return false;
    }

    if (goal.isGpuWritable && resource->GetUsage() != Resource::SHADER_OUTPUT)
    {
        // mismatch of GPU write flag
        return false;
    }

    // A countered structure buffer can be attached as a read-only input to
    // a shader.  We care about the mismatch in counter type only when the
    // shader needs a countered structure buffer but the attached resource
    // does not have one.
    if (goal.extra != 0 && goal.extra != static_cast<unsigned int>(resource->GetCounterType()))
    {
        // mismatch of counter type
        return false;
    }

    return true;
}

bool HLSLShader::IsValid(Data const& goal, RawBuffer* resource) const
{
    if (!resource)
    {
        // resource is null
        return false;
    }

    if (goal.type != GT_RAW_BUFFER)
    {
        // mismatch of buffer type
        return false;
    }

    if (goal.isGpuWritable && resource->GetUsage() != Resource::SHADER_OUTPUT)
    {
        // mismatch of GPU write flag
        return false;
    }

    return true;
}

bool HLSLShader::IsValid(Data const& goal, TextureSingle* resource) const
{
    if (!resource)
    {
        // resource is null
        return false;
    }

    if (goal.type != GT_TEXTURE_SINGLE)
    {
        // mismatch of texture type
        return false;
    }

    if (goal.isGpuWritable && resource->GetUsage() != Resource::SHADER_OUTPUT)
    {
        // mismatch of GPU write flag
        return false;
    }

    if (goal.extra != resource->GetNumDimensions())
    {
        // mismatch of texture dimensions
        return false;
    }

    // TODO: Add validation for HLSLTexture::Component and number of
    // components (requires comparison to TextureFormat value).
    return true;
}

bool HLSLShader::IsValid(Data const& goal, TextureArray* resource) const
{
    if (!resource)
    {
        // resource is null
        return false;
    }

    if (goal.type != GT_TEXTURE_ARRAY)
    {
        // mismatch of texture type
        return false;
    }

    if (goal.isGpuWritable && resource->GetUsage() != Resource::SHADER_OUTPUT)
    {
        // mismatch of GPU write flag
        return false;
    }

    if (goal.extra != resource->GetNumDimensions())
    {
        // mismatch of texture dimensions
        return false;
    }

    // TODO: Add validation for HLSLTexture::Component and number of
    // components (requires comparison to TextureFormat value).
    return true;
}

bool HLSLShader::IsValid(Data const& goal, SamplerState* resource) const
{
    if (!resource)
    {
        // resource is null
        return false;
    }

    if (goal.type != GT_SAMPLER_STATE)
    {
        // mismatch of state
        return false;
    }

    return true;
}
