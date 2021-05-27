// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VertexBuffer.h>
#include <Graphics/GL45/GL45Buffer.h>

namespace gte
{
    class GL45VertexBuffer : public GL45Buffer
    {
    public:
        // Construction.
        virtual ~GL45VertexBuffer() = default;
        GL45VertexBuffer(VertexBuffer const* vbuffer);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline VertexBuffer* GetVertexBuffer() const
        {
            return static_cast<VertexBuffer*>(mGTObject);
        }

        // TODO: Drawing support?  Currently, the enable/disable is in the
        // GL45InputLayout class, which assumes OpenGL 4.5 or later.  What if
        // the application machine does not have OpenGL 4.5?  Fall back to the
        // glBindBuffer paradigm?
    };
}
