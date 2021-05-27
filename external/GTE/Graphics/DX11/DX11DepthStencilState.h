// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DepthStencilState.h>
#include <Graphics/DX11/DX11DrawingState.h>

namespace gte
{
    class DX11DepthStencilState : public DX11DrawingState
    {
    public:
        // Construction.
        virtual ~DX11DepthStencilState() = default;
        DX11DepthStencilState(ID3D11Device* device, DepthStencilState const* depthStencilState);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline DepthStencilState* GetDepthStencilState()
        {
            return static_cast<DepthStencilState*>(mGTObject);
        }

        inline ID3D11DepthStencilState* GetDXDepthStencilState()
        {
            return static_cast<ID3D11DepthStencilState*>(mDXObject);
        }

        // Enable the depth-stencil state.
        void Enable(ID3D11DeviceContext* context);

    private:
        // Conversions from GTEngine values to DX11 values.
        static D3D11_DEPTH_WRITE_MASK const msWriteMask[];
        static D3D11_COMPARISON_FUNC const msComparison[];
        static D3D11_STENCIL_OP const msOperation[];
    };
}
