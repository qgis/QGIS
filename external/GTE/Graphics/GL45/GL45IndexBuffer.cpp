// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45IndexBuffer.h>
using namespace gte;

GL45IndexBuffer::GL45IndexBuffer(IndexBuffer const* ibuffer)
    :
    GL45Buffer(ibuffer, GL_ELEMENT_ARRAY_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL45IndexBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_INDEX_BUFFER)
    {
        return std::make_shared<GL45IndexBuffer>(
            static_cast<IndexBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void GL45IndexBuffer::Enable()
{
    glBindBuffer(mType, mGLHandle);
}

void GL45IndexBuffer::Disable()
{
    glBindBuffer(mType, 0);
}
