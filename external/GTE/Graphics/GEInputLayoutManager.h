// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

namespace gte
{
    class Shader;
    class VertexBuffer;

    class GEInputLayoutManager
    {
    public:
        // Abstract base interface.
        virtual ~GEInputLayoutManager() = default;
        GEInputLayoutManager() = default;

        virtual bool Unbind(VertexBuffer const* vbuffer) = 0;
        virtual bool Unbind(Shader const* vshader) = 0;
        virtual void UnbindAll() = 0;
        virtual bool HasElements() const = 0;
    };
}
