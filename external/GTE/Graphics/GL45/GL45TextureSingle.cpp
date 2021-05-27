// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45TextureSingle.h>
using namespace gte;

GL45TextureSingle::~GL45TextureSingle()
{
    for (int level = 0; level < mNumLevels; ++level)
    {
        glDeleteBuffers(1, &mLevelPixelUnpackBuffer[level]);
        glDeleteBuffers(1, &mLevelPixelPackBuffer[level]);
    }
}

GL45TextureSingle::GL45TextureSingle(TextureSingle const* gtTexture, GLenum target, GLenum targetBinding)
    :
    GL45Texture(gtTexture, target, targetBinding)
{
    // Initially no staging buffers.
    std::fill(std::begin(mLevelPixelUnpackBuffer), std::end(mLevelPixelUnpackBuffer), 0);
    std::fill(std::begin(mLevelPixelPackBuffer), std::end(mLevelPixelPackBuffer), 0);
}

void GL45TextureSingle::Initialize()
{
    // Save current binding for this texture target in order to restore it
    // when done because the gl texture object is needed to be bound to this
    // texture target for the operations to follow.
    GLint prevBinding;
    glGetIntegerv(mTargetBinding, &prevBinding);
    glBindTexture(mTarget, mGLHandle);

    // The default is 4-byte alignment.  This allows byte alignment when data
    // from user buffers into textures and vice versa.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Set the range of levels.
    glTexParameteri(mTarget, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, mNumLevels-1);

    // Initialize with data?
    auto texture = GetTexture();
    if (texture->GetData())
    {
        if (CanAutoGenerateMipmaps())
        {
            // Initialize with the first mipmap level and then generate
            // the remaining mipmaps.
            auto data = texture->GetDataFor(0);
            if (data)
            {
                LoadTextureLevel(0, data);
                GenerateMipmaps();
            }
        }
        else
        {
            // Initialize with each mipmap level.
            for (int level = 0; level < mNumLevels; ++level)
            {
                auto data = texture->GetDataFor(level);
                if (data)
                {
                    LoadTextureLevel(level, data);
                }
            }
        }
    }

    glBindTexture(mTarget, prevBinding);
}

bool GL45TextureSingle::Update()
{
    auto texture = GetTexture();

    if (CanAutoGenerateMipmaps())
    {
        // Only update the level 0 texture and then generate the remaining
        // mipmaps from it.
        if (!Update(0))
        {
            return false;
        }
        GenerateMipmaps();
    }
    else
    {
        // Automatic generation of mipmaps is not enabled, so all mipmap
        // levels must be copied to the GPU.
        auto const numLevels = texture->GetNumLevels();
        for (unsigned int level = 0; level < numLevels; ++level)
        {
            if (!Update(level))
            {
                return false;
            }
        }
    }

    return true;
}

bool GL45TextureSingle::CopyCpuToGpu()
{
    auto texture = GetTexture();

    if (CanAutoGenerateMipmaps())
    {
        // Only update the level 0 texture and then generate the remaining
        // mipmaps from it.
        if (!CopyCpuToGpu(0))
        {
            return false;
        }
        GenerateMipmaps();
    }
    else
    {
        // Automatic generation of mipmaps is not enabled, so all mipmap
        // levels must be copied to the GPU.
        auto const numLevels = texture->GetNumLevels();
        for (unsigned int level = 0; level < numLevels; ++level)
        {
            if (!CopyCpuToGpu(level))
            {
                return false;
            }
        }
    }

    return true;
}

bool GL45TextureSingle::CopyGpuToCpu()
{
    auto texture = GetTexture();
    auto const numLevels = texture->GetNumLevels();
    for (unsigned int level = 0; level < numLevels; ++level)
    {
        if (!CopyGpuToCpu(level))
        {
            return false;
        }
    }

    return true;
}

bool GL45TextureSingle::Update(unsigned int level)
{
    auto texture = GetTexture();
    if (texture->GetUsage() != Resource::DYNAMIC_UPDATE)
    {
        LogError("Texture usage must be DYNAMIC_UPDATE.");
    }

    return DoCopyCpuToGpu(level);
}

bool GL45TextureSingle::CopyCpuToGpu(unsigned int level)
{
    if (!PreparedForCopy(GL_WRITE_ONLY))
    {
        return false;
    }

    return DoCopyCpuToGpu(level);
}

bool GL45TextureSingle::CopyGpuToCpu(unsigned int level)
{
    if (!PreparedForCopy(GL_READ_ONLY))
    {
        return false;
    }

    auto texture = GetTexture();

    // Make sure level is valid.
    auto const numLevels = texture->GetNumLevels();
    if (level >= numLevels)
    {
        LogError("Level for Texture is out of range");
    }

    auto pixBuffer = mLevelPixelPackBuffer[level];
    if (0 == pixBuffer)
    {
        LogError("Staging buffer not defined for level " + level);
    }

    auto data = texture->GetDataFor(level);
    auto numBytes = texture->GetNumBytesFor(level);
    if ((nullptr == data) || (0 == numBytes))
    {
        LogError("No target data for texture level " + level);
    }

    auto const target = GetTarget();
    glBindTexture(target, mGLHandle);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffer);
    glGetTexImage(target, level, mExternalFormat, mExternalType, 0);
    glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, numBytes, data);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glBindTexture(target, 0);

    return true;
}

bool GL45TextureSingle::GenerateMipmaps()
{
    if (CanAutoGenerateMipmaps())
    {
        // Save current binding for this texture target in order to restore it
        // when done because the gl texture object is needed to be bound to
        // this texture target for the operations to follow.
        GLint prevBinding;
        glGetIntegerv(mTargetBinding, &prevBinding);
        glBindTexture(mTarget, mGLHandle);

        // Generate the mipmaps.  All of this binding save and restore is not
        // necessary in OpenGL 4.5, where glGenerateTextureMipamap(mGLHandle)
        // can simply be used.
        glGenerateMipmap(mTarget);

        glBindTexture(mTarget, prevBinding);

        return true;
    }
    return false;
}

bool GL45TextureSingle::DoCopyCpuToGpu(unsigned int level)
{
    auto texture = GetTexture();

    if (CanAutoGenerateMipmaps() && (level > 0))
    {
        LogError("Cannot update automatically generated mipmaps in GPU");
    }

    // Make sure level is valid.
    auto const numLevels = texture->GetNumLevels();
    if (level >= numLevels)
    {
        LogError("Level for texture is out of range");
    }

    auto data = texture->GetDataFor(level);
    auto numBytes = texture->GetNumBytesFor(level);
    if ((nullptr == data) || (0 == numBytes))
    {
        LogError("No source data for texture level " + level);
    }

    auto const target = GetTarget();
    glBindTexture(target, mGLHandle);

    // Use staging buffer if present.
    auto pixBuffer = mLevelPixelUnpackBuffer[level];
    if (0 != pixBuffer)
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixBuffer);
        glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, numBytes, data);
        LoadTextureLevel(level, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
    else
    {
        LoadTextureLevel(level, data);
    }

    glBindTexture(target, 0);

    return true;
}

void GL45TextureSingle::CreateStaging()
{
    auto texture = GetTexture();
    auto const copyType = texture->GetCopyType();

    auto const createPixelUnpackBuffers = 
        (copyType == Resource::COPY_CPU_TO_STAGING) ||
        (copyType == Resource::COPY_BIDIRECTIONAL);

    auto const createPixelPackBuffers =
        (copyType == Resource::COPY_STAGING_TO_CPU) ||
        (copyType == Resource::COPY_BIDIRECTIONAL);

    // TODO:  Determine frequency and nature of usage for this staging buffer
    // when created by calling glBufferData.
    GLenum usage = GL_DYNAMIC_DRAW;

    if (createPixelUnpackBuffers)
    {
        for (int level = 0; level < mNumLevels; ++level)
        {
            auto& pixBuffer = mLevelPixelUnpackBuffer[level];
            if (0 == pixBuffer)
            {
                auto numBytes = texture->GetNumBytesFor(level);

                // Create pixel buffer for staging.
                glGenBuffers(1, &pixBuffer);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixBuffer);
                glBufferData(GL_PIXEL_UNPACK_BUFFER, numBytes, 0, usage);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
        }
    }

    if (createPixelPackBuffers)
    {
        for (int level = 0; level < mNumLevels; ++level)
        {
            auto& pixBuffer = mLevelPixelPackBuffer[level];
            if (0 == pixBuffer)
            {
                auto numBytes = texture->GetNumBytesFor(level);

                // Create pixel buffer for staging.
                glGenBuffers(1, &pixBuffer);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffer);
                glBufferData(GL_PIXEL_PACK_BUFFER, numBytes, 0, usage);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            }
        }
    }
}
