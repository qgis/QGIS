// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45Resource.h>
using namespace gte;

GL45Resource::GL45Resource(Resource const* gtResource)
    :
    GL45GraphicsObject(gtResource)
{
}

void* GL45Resource::MapForWrite(GLenum target)
{
    glBindBuffer(target, mGLHandle);
    GLvoid* mapped = glMapBuffer(target, GL_WRITE_ONLY);
    glBindBuffer(target, 0);
    return mapped;
}

void GL45Resource::Unmap(GLenum target)
{
    glBindBuffer(target, mGLHandle);
    glUnmapBuffer(target);
    glBindBuffer(target, 0);
}

bool GL45Resource::PreparedForCopy(GLenum access) const
{
    // TODO: DX11 requires a staging resource when copying between CPU and
    // GPU.  Does OpenGL hide the double hop?

    // Verify existence of objects.
    LogAssert(mGLHandle != 0, "GL object does not exist.");

    // Verify the copy type.
    Resource::CopyType copyType = GetResource()->GetCopyType();
    if (copyType == Resource::COPY_CPU_TO_STAGING)  // CPU -> GPU
    {
        if (access == GL_WRITE_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::COPY_STAGING_TO_CPU)  // GPU -> CPU
    {
        if (access == GL_READ_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::COPY_BIDIRECTIONAL)
    {
        if (access == GL_READ_WRITE)
        {
            return true;
        }
        if (access == GL_WRITE_ONLY)
        {
            return true;
        }
        if (access == GL_READ_ONLY)
        {
            return true;
        }
    }

    LogError("Resource has incorrect copy type.");
}
