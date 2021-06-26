// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/BlendState.h>
#include <Graphics/DX11/DX11DrawingState.h>

namespace gte
{
    class DX11BlendState : public DX11DrawingState
    {
    public:
        // Construction.
        virtual ~DX11BlendState() = default;
        DX11BlendState(ID3D11Device* device, BlendState const* blendState);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline BlendState* GetBlendState()
        {
            return static_cast<BlendState*>(mGTObject);
        }

        inline ID3D11BlendState* GetDXBlendState()
        {
            return static_cast<ID3D11BlendState*>(mDXObject);
        }

        // Enable the blend state.
        void Enable(ID3D11DeviceContext* context);

    private:
        // Conversions from GTEngine values to DX11 values.
        static D3D11_BLEND const msMode[];
        static D3D11_BLEND_OP const msOperation[];
    };
}
