// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/SamplerState.h>
#include <Graphics/DX11/DX11DrawingState.h>

namespace gte
{
    class DX11SamplerState : public DX11DrawingState
    {
    public:
        // Construction.
        virtual ~DX11SamplerState() = default;
        DX11SamplerState(ID3D11Device* device, SamplerState const* samplerState);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline SamplerState* GetSamplerState()
        {
            return static_cast<SamplerState*>(mGTObject);
        }

        inline ID3D11SamplerState* GetDXSamplerState()
        {
            return static_cast<ID3D11SamplerState*>(mDXObject);
        }

    private:
        // Conversions from GTEngine values to DX11 values.
        static D3D11_FILTER const msFilter[];
        static D3D11_TEXTURE_ADDRESS_MODE const msMode[];
        static D3D11_COMPARISON_FUNC const msComparison[];
    };
}
