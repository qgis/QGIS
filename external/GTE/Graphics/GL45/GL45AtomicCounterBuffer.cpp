// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45AtomicCounterBuffer.h>
using namespace gte;

GL45AtomicCounterBuffer::GL45AtomicCounterBuffer(RawBuffer const* cbuffer)
    :
    GL45Buffer(cbuffer, GL_ATOMIC_COUNTER_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL45AtomicCounterBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_RAW_BUFFER)
    {
        return std::make_shared<GL45AtomicCounterBuffer>(
            static_cast<RawBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void GL45AtomicCounterBuffer::AttachToUnit(GLint atomicCounterBufferUnit)
{
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBufferUnit, mGLHandle);
}
