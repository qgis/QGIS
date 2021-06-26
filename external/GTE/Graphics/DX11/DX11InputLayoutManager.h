// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GEInputLayoutManager.h>
#include <Graphics/DX11/DX11InputLayout.h>
#include <Mathematics/ThreadSafeMap.h>

namespace gte
{
    class DX11InputLayoutManager : public GEInputLayoutManager
    {
    public:
        // Construction and destruction.
        virtual ~DX11InputLayoutManager();
        DX11InputLayoutManager() = default;

        // Management functions.  The Unbind(vbuffer) removes all pairs that
        // involve vbuffer.  The Unbind(vshader) removes all pairs that
        // involve vshader.
        DX11InputLayout* Bind(ID3D11Device* device, VertexBuffer const* vbuffer, Shader const* vshader);
        virtual bool Unbind(VertexBuffer const* vbuffer) override;
        virtual bool Unbind(Shader const* vshader) override;
        virtual void UnbindAll() override;
        virtual bool HasElements() const override;

    private:
        typedef std::pair<VertexBuffer const*, Shader const*> VBSPair;

        class LayoutMap : public ThreadSafeMap<VBSPair, std::shared_ptr<DX11InputLayout>>
        {
        public:
            virtual ~LayoutMap() = default;
            LayoutMap() = default;

            void GatherMatch(VertexBuffer const* vbuffer, std::vector<VBSPair>& matches);
            void GatherMatch(Shader const* vshader, std::vector<VBSPair>& matches);
        };

        LayoutMap mMap;
    };
}
