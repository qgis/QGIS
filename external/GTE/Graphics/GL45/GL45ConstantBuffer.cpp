// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45ConstantBuffer.h>
using namespace gte;

GL45ConstantBuffer::GL45ConstantBuffer(ConstantBuffer const* cbuffer)
    :
    GL45Buffer(cbuffer, GL_UNIFORM_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL45ConstantBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_CONSTANT_BUFFER)
    {
        return std::make_shared<GL45ConstantBuffer>(
            static_cast<ConstantBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void GL45ConstantBuffer::AttachToUnit(GLint uniformBufferUnit)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, uniformBufferUnit, mGLHandle);
}
