// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11PixelShader.h>
using namespace gte;

DX11PixelShader::DX11PixelShader(ID3D11Device* device, Shader const* shader)
    :
    DX11Shader(shader)
{
    std::vector<unsigned char> const& code = shader->GetCompiledCode();

    ID3D11ClassLinkage* linkage = nullptr;
    ID3D11PixelShader* dxShader = nullptr;
    DX11Log(device->CreatePixelShader(&code[0], code.size(), linkage, &dxShader));

    mDXObject = dxShader;
}

std::shared_ptr<GEObject> DX11PixelShader::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_PIXEL_SHADER)
    {
        return std::make_shared<DX11PixelShader>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<Shader const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11PixelShader::Enable(ID3D11DeviceContext* context)
{
    if (mDXObject)
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        ID3D11PixelShader* dxShader = static_cast<ID3D11PixelShader*>(mDXObject);
        context->PSSetShader(dxShader, instances, numInstances);
    }
}

void DX11PixelShader::Disable(ID3D11DeviceContext* context)
{
    if (mDXObject)
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        ID3D11PixelShader* dxShader = nullptr;
        context->PSSetShader(dxShader, instances, numInstances);
    }
}

void DX11PixelShader::EnableCBuffer(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11Buffer* buffer)
{
    if (mDXObject)
    {
        ID3D11Buffer* buffers[1] = { buffer };
        context->PSSetConstantBuffers(bindPoint, 1, buffers);
    }
}

void DX11PixelShader::DisableCBuffer(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11Buffer* buffers[1] = { nullptr };
        context->PSSetConstantBuffers(bindPoint, 1, buffers);
    }
}

void DX11PixelShader::EnableSRView(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11ShaderResourceView* srView)
{
    if (mDXObject)
    {
        ID3D11ShaderResourceView* views[1] = { srView };
        context->PSSetShaderResources(bindPoint, 1, views);
    }
}

void DX11PixelShader::DisableSRView(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11ShaderResourceView* views[1] = { nullptr };
        context->PSSetShaderResources(bindPoint, 1, views);
    }
}

void DX11PixelShader::EnableUAView(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11UnorderedAccessView* uaView,
    unsigned int initialCount)
{
    if (mDXObject)
    {
        ID3D11UnorderedAccessView* uaViews[1] = { uaView };
        unsigned int initialCounts[1] = { initialCount };
        context->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
            bindPoint, 1, uaViews, initialCounts);
    }
}

void DX11PixelShader::DisableUAView(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11UnorderedAccessView* uaViews[1] = { nullptr };
        unsigned int initialCounts[1] = { 0xFFFFFFFFu };
        context->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
            bindPoint, 1, uaViews, initialCounts);
    }
}

void DX11PixelShader::EnableSampler(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11SamplerState* state)
{
    if (mDXObject)
    {
        ID3D11SamplerState* states[1] = { state };
        context->PSSetSamplers(bindPoint, 1, states);
    }
}

void DX11PixelShader::DisableSampler(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11SamplerState* states[1] = { nullptr };
        context->PSSetSamplers(bindPoint, 1, states);
    }
}
