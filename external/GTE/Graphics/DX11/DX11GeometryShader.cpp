// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11GeometryShader.h>
using namespace gte;

DX11GeometryShader::DX11GeometryShader(ID3D11Device* device, Shader const* shader)
    :
    DX11Shader(shader)
{
    std::vector<unsigned char> const& code = shader->GetCompiledCode();

    ID3D11ClassLinkage* linkage = nullptr;
    ID3D11GeometryShader* dxShader = nullptr;
    DX11Log(device->CreateGeometryShader(&code[0], code.size(), linkage, &dxShader));

    mDXObject = dxShader;
}

std::shared_ptr<GEObject> DX11GeometryShader::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_GEOMETRY_SHADER)
    {
        return std::make_shared<DX11GeometryShader>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<Shader const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11GeometryShader::Enable(ID3D11DeviceContext* context)
{
    if (mDXObject)
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        ID3D11GeometryShader* dxShader = static_cast<ID3D11GeometryShader*>(mDXObject);
        context->GSSetShader(dxShader, instances, numInstances);
    }
}

void DX11GeometryShader::Disable(ID3D11DeviceContext* context)
{
    if (mDXObject)
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        ID3D11GeometryShader* dxShader = nullptr;
        context->GSSetShader(dxShader, instances, numInstances);
    }
}

void DX11GeometryShader::EnableCBuffer(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11Buffer* buffer)
{
    if (mDXObject)
    {
        ID3D11Buffer* buffers[1] = { buffer };
        context->GSSetConstantBuffers(bindPoint, 1, buffers);
    }
}

void DX11GeometryShader::DisableCBuffer(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11Buffer* buffers[1] = { nullptr };
        context->GSSetConstantBuffers(bindPoint, 1, buffers);
    }
}

void DX11GeometryShader::EnableSRView(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11ShaderResourceView* srView)
{
    if (mDXObject)
    {
        ID3D11ShaderResourceView* views[1] = { srView };
        context->GSSetShaderResources(bindPoint, 1, views);
    }
}

void DX11GeometryShader::DisableSRView(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11ShaderResourceView* views[1] = { nullptr };
        context->GSSetShaderResources(bindPoint, 1, views);
    }
}

void DX11GeometryShader::EnableUAView(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11UnorderedAccessView* uaView,
    unsigned int initialCount)
{
    if (mDXObject)
    {
        ID3D11Device* device = nullptr;
        context->GetDevice(&device);
        LogAssert(device != nullptr, "Cannot access device of context.");
        LogAssert(device->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_1,
            "D3D11.1 is required for UAVs in geometry shaders.");

        ID3D11UnorderedAccessView* uaViews[1] = { uaView };
        unsigned int initialCounts[1] = { initialCount };
        context->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
            bindPoint, 1, uaViews, initialCounts);

        DX11::SafeRelease(device);
    }
}

void DX11GeometryShader::DisableUAView(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11Device* device = nullptr;
        context->GetDevice(&device);
        LogAssert(device != nullptr, "Cannot access device of context.");
        LogAssert(device->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_1,
            "D3D11.1 is required for UAVs in geometry shaders.");

        ID3D11UnorderedAccessView* uaViews[1] = { nullptr };
        unsigned int initialCounts[1] = { 0xFFFFFFFFu };
        context->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
            bindPoint, 1, uaViews, initialCounts);

        DX11::SafeRelease(device);
    }
}

void DX11GeometryShader::EnableSampler(ID3D11DeviceContext* context,
    unsigned int bindPoint, ID3D11SamplerState* state)
{
    if (mDXObject)
    {
        ID3D11SamplerState* states[1] = { state };
        context->GSSetSamplers(bindPoint, 1, states);
    }
}

void DX11GeometryShader::DisableSampler(ID3D11DeviceContext* context,
    unsigned int bindPoint)
{
    if (mDXObject)
    {
        ID3D11SamplerState* states[1] = { nullptr };
        context->GSSetSamplers(bindPoint, 1, states);
    }
}
