// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/RasterizerState.h>
#include <Graphics/DX11/DX11DrawingState.h>

namespace gte
{
    class DX11RasterizerState : public DX11DrawingState
    {
    public:
        // Construction.
        virtual ~DX11RasterizerState() = default;
        DX11RasterizerState(ID3D11Device* device, RasterizerState const* rasterizerState);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline RasterizerState* GetRasterizerState()
        {
            return static_cast<RasterizerState*>(mGTObject);
        }

        inline ID3D11RasterizerState* GetDXRasterizerState()
        {
            return static_cast<ID3D11RasterizerState*>(mDXObject);
        }

        // Enable the rasterizer state.
        void Enable(ID3D11DeviceContext* context);

    private:
        // Conversions from GTEngine values to DX11 values.
        static D3D11_FILL_MODE const msFillMode[];
        static D3D11_CULL_MODE const msCullMode[];
    };
}
