// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45VertexBuffer.h>
using namespace gte;

GL45VertexBuffer::GL45VertexBuffer(VertexBuffer const* vbuffer)
    :
    GL45Buffer(vbuffer, GL_ARRAY_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL45VertexBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_VERTEX_BUFFER)
    {
        return std::make_shared<GL45VertexBuffer>(
            static_cast<VertexBuffer const*>(object));
    }

    LogError("Invalid object type.");
}
