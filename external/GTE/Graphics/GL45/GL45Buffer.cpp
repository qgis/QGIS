// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45Buffer.h>
using namespace gte;

GL45Buffer::~GL45Buffer()
{
    glDeleteBuffers(1, &mGLHandle);
}

GL45Buffer::GL45Buffer(Buffer const* buffer, GLenum type)
    :
    GL45Resource(buffer),
    mType(type)
{
    glGenBuffers(1, &mGLHandle);

    Resource::Usage usage = buffer->GetUsage();
    if (usage == Resource::IMMUTABLE)
    {
        mUsage = GL_STATIC_DRAW;
    }
    else if (usage == Resource::DYNAMIC_UPDATE)
    {
        mUsage = GL_DYNAMIC_DRAW;
    }
    else  // usage == Resource::SHADER_OUTPUT
    {
        // TODO: In GLSL, is it possible to write to a buffer other than a
        // vertex buffer?
        if (mType == GL_ARRAY_BUFFER)
        {
            mUsage = GL_STREAM_DRAW;
        }
        else if (mType == GL_SHADER_STORAGE_BUFFER)
        {
            mUsage = GL_DYNAMIC_DRAW;
        }
        else
        {
            LogWarning("Can this buffer type be a shader output?");
            mUsage = GL_STATIC_DRAW;
        }
    }
}

void GL45Buffer::Initialize()
{
    glBindBuffer(mType, mGLHandle);

    // Access the buffer.
    auto buffer = GetBuffer();

    // Create and initialize a buffer.
    glBufferData(mType, buffer->GetNumBytes(), buffer->GetData(), mUsage);

    glBindBuffer(mType, 0);
}

bool GL45Buffer::Update()
{
    Buffer* buffer = GetBuffer();
    if (buffer->GetUsage() != Resource::DYNAMIC_UPDATE)
    {
        LogWarning("Buffer usage is not DYNAMIC_UPDATE.");
        return false;
    }

    GLuint numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
        GLintptr offsetInBytes = buffer->GetOffset() * buffer->GetElementSize();
        char const* source = buffer->GetData() + offsetInBytes;
        glBindBuffer(mType, mGLHandle);
        glBufferSubData(mType, offsetInBytes, numActiveBytes, source);
        glBindBuffer(mType, 0);
    }
    else
    {
        LogInformation("Buffer has zero active bytes.");
    }
    return true;
}

bool GL45Buffer::CopyCpuToGpu()
{
    if (!PreparedForCopy(GL_WRITE_ONLY))
    {
        return false;
    }

    Buffer* buffer = GetBuffer();
    GLuint numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
        GLintptr offsetInBytes = buffer->GetOffset() * buffer->GetElementSize();
        char const* source = buffer->GetData() + offsetInBytes;
        glBindBuffer(mType, mGLHandle);
        glBufferSubData(mType, offsetInBytes, numActiveBytes, source);
        glBindBuffer(mType, 0);
    }
    else
    {
        LogInformation("Buffer has zero active bytes.");
    }
    return true;
}

bool GL45Buffer::CopyGpuToCpu()
{
    if (!PreparedForCopy(GL_READ_ONLY))
    {
        return false;
    }

    Buffer* buffer = GetBuffer();
    GLuint numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from GPU memory to CPU memory.
        GLintptr offsetInBytes = buffer->GetOffset() * buffer->GetElementSize();
        char* target = buffer->GetData() + offsetInBytes;
        glBindBuffer(mType, mGLHandle);
        glGetBufferSubData(mType, offsetInBytes, numActiveBytes, target);
        glBindBuffer(mType, 0);
    }
    else
    {
        LogInformation("Buffer has zero active bytes.");
    }
    return true;
}
