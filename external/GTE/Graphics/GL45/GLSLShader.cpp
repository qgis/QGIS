// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GLSLShader.h>
using namespace gte;

GLSLShader::GLSLShader(GLSLReflection const& reflector, GraphicsObjectType type, int glslType)
    :
    Shader(type)
{
    // If this is a compute shader, then query the number of threads per
    // group.
    if (GLSLReflection::ST_COMPUTE == glslType)
    {
        GLint sizeX, sizeY, sizeZ;
        reflector.GetComputeShaderWorkGroupSize(sizeX, sizeY, sizeZ);
        mNumXThreads = sizeX;
        mNumYThreads = sizeY;
        mNumZThreads = sizeZ;
    }

    // Will need to access uniforms more than once.
    auto const& uniforms = reflector.GetUniforms();

    // Gather the uninforms information to create texture data.
    for (auto const& uni : uniforms)
    {
        if (uni.referencedBy[glslType])
        {
            // Only interested in these particular uniform types (for
            // gsampler* and gimage*).
            switch (uni.type)
            {
                case GL_SAMPLER_1D:
                case GL_INT_SAMPLER_1D:
                case GL_UNSIGNED_INT_SAMPLER_1D:
                    mData[TextureSingle::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_SINGLE, uni.name, uni.location, 0, 1, false));
                    mData[SamplerState::shaderDataLookup].push_back(
                        Data(GT_SAMPLER_STATE, uni.name, uni.location, 0, GT_TEXTURE1, false));
                    break;

                case GL_SAMPLER_2D:
                case GL_INT_SAMPLER_2D:
                case GL_UNSIGNED_INT_SAMPLER_2D:
                    mData[TextureSingle::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_SINGLE, uni.name, uni.location, 0, 2, false));
                    mData[SamplerState::shaderDataLookup].push_back(
                        Data(GT_SAMPLER_STATE, uni.name, uni.location, 0, GT_TEXTURE2, false));
                    break;

                case GL_SAMPLER_3D:
                case GL_INT_SAMPLER_3D:
                case GL_UNSIGNED_INT_SAMPLER_3D:
                    mData[TextureSingle::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_SINGLE, uni.name, uni.location, 0, 3, false));
                    mData[SamplerState::shaderDataLookup].push_back(
                        Data(GT_SAMPLER_STATE, uni.name, uni.location, 0, GT_TEXTURE3, false));
                    break;

                case GL_SAMPLER_1D_ARRAY:
                case GL_INT_SAMPLER_1D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 1, false));
                    mData[SamplerState::shaderDataLookup].push_back(
                        Data(GT_SAMPLER_STATE, uni.name, uni.location, 0, GT_TEXTURE1_ARRAY, false));
                    break;

                case GL_SAMPLER_2D_ARRAY:
                case GL_INT_SAMPLER_2D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 2, false));
                    mData[SamplerState::shaderDataLookup].push_back(
                        Data(GT_SAMPLER_STATE, uni.name, uni.location, 0, GT_TEXTURE2_ARRAY, false));
                    break;

                case GL_SAMPLER_CUBE:
                case GL_INT_SAMPLER_CUBE:
                case GL_UNSIGNED_INT_SAMPLER_CUBE:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 2, false));
                    mData[SamplerState::shaderDataLookup].push_back(
                        Data(GT_SAMPLER_STATE, uni.name, uni.location, 0, GT_TEXTURE_CUBE, false));
                    break;

                case GL_SAMPLER_CUBE_MAP_ARRAY:
                case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 3, false));
                    mData[SamplerState::shaderDataLookup].push_back(
                        Data(GT_SAMPLER_STATE, uni.name, uni.location, 0, GT_TEXTURE_CUBE_ARRAY, false));
                    break;

                case GL_IMAGE_1D:
                case GL_INT_IMAGE_1D:
                case GL_UNSIGNED_INT_IMAGE_1D:
                    mData[TextureSingle::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_SINGLE, uni.name, uni.location, 0, 1, true));
                    break;

                case GL_IMAGE_2D:
                case GL_INT_IMAGE_2D:
                case GL_UNSIGNED_INT_IMAGE_2D:
                    mData[TextureSingle::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_SINGLE, uni.name, uni.location, 0, 2, true));
                    break;

                case GL_IMAGE_3D:
                case GL_INT_IMAGE_3D:
                case GL_UNSIGNED_INT_IMAGE_3D:
                    mData[TextureSingle::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_SINGLE, uni.name, uni.location, 0, 3, true));
                    break;

                case GL_IMAGE_1D_ARRAY:
                case GL_INT_IMAGE_1D_ARRAY:
                case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 1, true));
                    break;

                case GL_IMAGE_2D_ARRAY:
                case GL_INT_IMAGE_2D_ARRAY:
                case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 2, true));
                    break;

                case GL_IMAGE_CUBE:
                case GL_INT_IMAGE_CUBE:
                case GL_UNSIGNED_INT_IMAGE_CUBE:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 2, true));
                    break;

                case GL_IMAGE_CUBE_MAP_ARRAY:
                case GL_INT_IMAGE_CUBE_MAP_ARRAY:
                case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
                    mData[TextureArray::shaderDataLookup].push_back(
                        Data(GT_TEXTURE_ARRAY, uni.name, uni.location, 0, 3, true));
                    break;
            }
        }
    }

    // Gather the uniform blocks information to create constant buffer data.
    auto const& uniformBlocks = reflector.GetUniformBlocks();
    int numUniformBlockReferences = 0;
    for (auto const& block : uniformBlocks)
    {
        if (block.referencedBy[glslType])
        {
            ++numUniformBlockReferences;
        }
    }
    if (numUniformBlockReferences > 0)
    {
        mCBufferLayouts.resize(numUniformBlockReferences);

        // Store information needed by GL4Engine for enabling/disabling the
        // constant buffers.
        int blockIndex = 0;
        int layoutIndex = 0;
        for (auto const& block : uniformBlocks)
        {
            if (block.referencedBy[glslType])
            {
                mData[ConstantBuffer::shaderDataLookup].push_back(
                    Data(GT_CONSTANT_BUFFER, block.name, block.bufferBinding,
                    block.bufferDataSize, 0, false));

                // Assemble the constant buffer layout information.
                for (auto const& uniform : uniforms)
                {
                    if (uniform.blockIndex != blockIndex)
                    {
                        continue;

                    }
                    MemberLayout item;
                    item.name = uniform.name;
                    item.offset = uniform.offset;
                    // TODO: The HLSL reflection has numElements of 0 when
                    // the item is not an array, but the actual number when
                    // it is an array.  ConstantBuffer::SetMember(...) uses
                    // this information, so we need to adhere to the pattern.
                    // Change this design in a refactor?
                    item.numElements =
                        (uniform.arraySize > 1 ? uniform.arraySize : 0);

                    mCBufferLayouts[layoutIndex].push_back(item);
                }

                ++layoutIndex;
            }

            ++blockIndex;
        }
    }

    // Gather the atomic counter buffer information to create atomic counter
    // buffer data.
    auto const& atomicCounterBuffers = reflector.GetAtomicCounterBuffers();
    int numAtomicCounterBufferReferences = 0;
    for (auto const& block : atomicCounterBuffers)
    {
        if (block.referencedBy[glslType])
        {
            ++numAtomicCounterBufferReferences;
        }
    }
    if (numAtomicCounterBufferReferences > 0)
    {
        unsigned blockIndex = 0;
        for (auto const& block : atomicCounterBuffers)
        {
            if (block.referencedBy[glslType])
            {
                // It is possible for the atomic counter buffer to indicate it
                // only has 4 bytes for a single counter located at offset=4.
                // But we will want to allocate a buffer large enough to store
                // from offset=0 to the last counter declared in the buffer.
                unsigned bufferDataSize = block.bufferDataSize;
                for (unsigned i=0; i < block.activeVariables.size(); ++i)
                {
                    auto const& ac = uniforms[block.activeVariables[i]];
                    unsigned const lastByte = ac.offset + 4;

                    bufferDataSize = std::max(bufferDataSize, lastByte);
                }

                mData[AtomicCounterBufferShaderDataLookup].push_back(
                    Data(GT_RESOURCE, "atomicCounterBuffer" + std::to_string(blockIndex),
                    block.bufferBinding, bufferDataSize, 0, true));
            }
            ++blockIndex;
        }
    }

    // Gather the buffer blocks information to create structured buffer data.
    auto const& bufferBlocks = reflector.GetBufferBlocks();
    int numBufferBlockReferences = 0;
    for (auto const& block : bufferBlocks)
    {
        if (block.referencedBy[glslType])
        {
            ++numBufferBlockReferences;
        }
    }
    if (numBufferBlockReferences > 0)
    {
        auto const& bufferVariables = reflector.GetBufferVariables();
        mSBufferLayouts.resize(numBufferBlockReferences);

        // Store information needed by GL4Engine for enabling/disabling the
        // structured buffers.
        int blockIndex = 0;
        int layoutIndex = 0;
        for (auto const& block : bufferBlocks)
        {
            if (block.referencedBy[glslType])
            {
                // Search through uniforms looking for atomic counter with
                // the same name and "Counter" suffix.  The ID is the index
                // for this uniform so that it can be looked up later.
                auto const counterName = block.name + "Counter";
                bool hasAtomicCounter = false;
                unsigned int idAtomicCounter = ~0U;
                for (auto const& uniform : uniforms)
                {
                    if ((counterName == uniform.name) && (uniform.atomicCounterBufferIndex >= 0))
                    {
                        hasAtomicCounter = true;
                        idAtomicCounter = static_cast<unsigned int>(mData[AtomicCounterShaderDataLookup].size());
                        mData[AtomicCounterShaderDataLookup].push_back(
                            Data(GT_STRUCTURED_BUFFER, uniform.name, uniform.atomicCounterBufferIndex,
                            4, uniform.offset, false));
                        break;
                    }
                }

                // Assemble the structured buffer layout information.  Only
                // interested in variables in the buffer that are part of a
                // top level array stride.  Anything up to this block is
                // ignored and anything after this block is ignored which
                // means only one top level array is supported.
                auto& layout = mSBufferLayouts[layoutIndex];
                GLint structSize = 0;
                for (unsigned v = 0; v < block.activeVariables.size(); ++v)
                {
                    auto const& bufferVar = bufferVariables[block.activeVariables[v]];

                    if (bufferVar.topLevelArrayStride != structSize)
                    {
                        // Stop when we were processing buffer variables with
                        // a certain top-level array stride and that changed.
                        if (0 != structSize)
                        {
                            break;
                        }
                        structSize = bufferVar.topLevelArrayStride;
                    }

                    // These are the variables in the structured buffer.
                    if (structSize > 0)
                    {
                        MemberLayout item;
                        item.name = bufferVar.name;
                        item.offset = bufferVar.offset;

                        // TODO: The HLSL reflection has numElements of 0 when
                        // the item is not an array, but the actual number
                        // when it is an array. ConstantBuffer::SetMember(...)
                        // uses this information, so we need to adhere to the
                        // pattern.  Change this design in a refactor?
                        item.numElements = (bufferVar.arraySize > 1 ? bufferVar.arraySize : 0);

                        layout.push_back(item);
                    }
                }

                // Use the top level array stride as a better indication
                // of the overall struct size.
                mData[StructuredBuffer::shaderDataLookup].push_back(
                    Data(GT_STRUCTURED_BUFFER, block.name, block.bufferBinding,
                    structSize, idAtomicCounter, hasAtomicCounter));

                // OpenGL implementions might store structured buffer member
                // information in different orders; for example, alphabetical
                // by name or by offset.  To produce a consistent layout, sort
                // the layout by offset.
                std::sort(layout.begin(), layout.end(),
                    [](MemberLayout const& layout0, MemberLayout const& layout1)
                    {
                        return layout0.offset < layout1.offset;
                    }
                );

                ++layoutIndex;
            }

            ++blockIndex;
        }
    }
}

void GLSLShader::Set(std::string const&, std::shared_ptr<TextureSingle> const& texture,
    std::string const& samplerName, std::shared_ptr<SamplerState> const& state)
{
    Shader::Set(samplerName, texture);
    Shader::Set(samplerName, state);
}

void GLSLShader::Set(std::string const&, std::shared_ptr<TextureArray> const& texture,
    std::string const& samplerName, std::shared_ptr<SamplerState> const& state)
{
    Shader::Set(samplerName, texture);
    Shader::Set(samplerName, state);
}

bool GLSLShader::IsValid(Data const& goal, ConstantBuffer* resource) const
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

bool GLSLShader::IsValid(Data const& goal, TextureBuffer* resource) const
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

bool GLSLShader::IsValid(Data const& goal, StructuredBuffer* resource) const
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

    // GL4 reflection does not provide information about writable access of
    // buffer objects in a shader because by definition, shader storage buffer
    // objects can be read-write by shaders.  For GL4, the isGpuWritable flag
    // is used to indicate whether the structured buffer has a counter
    // attached or not.  Thus, the test that is performed in the DX11 IsValid
    // code cannot be used here.

    // OpenGL does not have the concept of an append-consume type structured
    // buffer nor does it have the concept of a structured buffer with
    // counter.  But, this GL4 support does associate an atomic counter with
    // a structured buffer as long as it has the same name.  If the shader is
    // expecting a counter, then the structured buffer needs to be declared
    // with one.
    if (goal.isGpuWritable && (StructuredBuffer::CT_NONE == resource->GetCounterType()))
    {
        // mismatch of counter type
        return false;
    }

    return true;
}

bool GLSLShader::IsValid(Data const& goal, RawBuffer* resource) const
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

bool GLSLShader::IsValid(Data const& goal, TextureSingle* resource) const
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

    // GL4 reflection does not provide information about writable access of
    // gimage* and gsampler* objects in a shader.  For GL4, the isGpuWritable
    // flag is used to indicate whether the texture could be writable in
    // shader which is false for gshader* objects and is true for gimage*
    // objects.  Thus, the test that is performed in the DX11 IsValid code
    // cannot be used here.

    if (goal.extra != resource->GetNumDimensions())
    {
        // mismatch of texture dimensions
        return false;
    }

    return true;
}

bool GLSLShader::IsValid(Data const& goal, TextureArray* resource) const
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

    // GL4 reflection does not provide information about writable access of
    // gimage* and gsampler* objects in a shader.  For GL4, the isGpuWritable
    // flag is used to indicate whether the texture could be writable in
    // shader which is false for gshader* objects and is true for gimage*
    // objects.  Thus, the test that is performed in the DX11 IsValid code
    // cannot be used here.

    if (goal.extra != resource->GetNumDimensions())
    {
        // mismatch of texture dimensions
        return false;
    }

    return true;
}

bool GLSLShader::IsValid(Data const& goal, SamplerState* resource) const
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
