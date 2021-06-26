// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Shader.h>
#include <Graphics/DX11/DX11Shader.h>

namespace gte
{
    class DX11VertexShader : public DX11Shader
    {
    public:
        // Construction and destruction.
        virtual ~DX11VertexShader() = default;
        DX11VertexShader(ID3D11Device* device, Shader const* shader);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Overrides to hide DX11 functions that have "VS" embedded in their
        // names.

        // Calls to ID3D11DeviceContext::VSSetShader.
        virtual void Enable(ID3D11DeviceContext* context) override;
        virtual void Disable(ID3D11DeviceContext* context) override;

        // Calls to ID3D11DeviceContext::VSSetConstantBuffers.
        virtual void EnableCBuffer(ID3D11DeviceContext* context,
            unsigned int bindPoint, ID3D11Buffer* buffer) override;
        virtual void DisableCBuffer(ID3D11DeviceContext* context,
            unsigned int bindPoint) override;

        // Calls to ID3D11DeviceContext::VSSetShaderResources.
        virtual void EnableSRView(ID3D11DeviceContext* context,
            unsigned int bindPoint, ID3D11ShaderResourceView* srView) override;
        virtual void DisableSRView(ID3D11DeviceContext* context,
            unsigned int bindPoint) override;

        // Unordered access views are supported in vertex shaders starting
        // with D3D11.1.
        virtual void EnableUAView(ID3D11DeviceContext* context,
            unsigned int bindPoint, ID3D11UnorderedAccessView* uaView,
            unsigned int initialCount) override;
        virtual void DisableUAView(ID3D11DeviceContext* context,
            unsigned int bindPoint) override;

        // Calls to ID3D11DeviceContext::VSSetSamplers.
        virtual void EnableSampler(ID3D11DeviceContext* context,
            unsigned int bindPoint, ID3D11SamplerState* state) override;
        virtual void DisableSampler(ID3D11DeviceContext* context,
            unsigned int bindPoint) override;
    };
}
