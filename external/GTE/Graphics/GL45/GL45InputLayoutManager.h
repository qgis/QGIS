// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GEInputLayoutManager.h>
#include <Graphics/GL45/GL45InputLayout.h>
#include <Mathematics/ThreadSafeMap.h>

namespace gte
{
    class GL45InputLayoutManager : public GEInputLayoutManager
    {
    public:
        // Construction and destruction.
        virtual ~GL45InputLayoutManager();
        GL45InputLayoutManager() = default;

        // Management functions.  The Unbind(vbuffer) removes all layouts that
        // involve vbuffer.  The Unbind(vshader) is stubbed out because GL45
        // does not require it, but we wish to have
        // Unbind(GraphicsObject const*) as a base-class GraphicsEngine
        // function.
        GL45InputLayout* Bind(GLuint programHandle, GLuint vbufferHandle, VertexBuffer const* vbuffer);
        virtual bool Unbind(VertexBuffer const* vbuffer) override;
        virtual bool Unbind(Shader const* vshader) override;
        virtual void UnbindAll() override;
        virtual bool HasElements() const override;

    private:
        typedef std::pair<VertexBuffer const*, GLuint> VBPPair;

        class LayoutMap : public ThreadSafeMap<VBPPair, std::shared_ptr<GL45InputLayout>>
        {
        public:
            virtual ~LayoutMap() = default;
            LayoutMap() = default;

            void GatherMatch(VertexBuffer const* vbuffer, std::vector<VBPPair>& matches);
        };

        LayoutMap mMap;
    };
}
