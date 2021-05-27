// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11SamplerState.h>
using namespace gte;

DX11SamplerState::DX11SamplerState(ID3D11Device* device, SamplerState const* samplerState)
    :
    DX11DrawingState(samplerState)
{
    // Specify the sampler state description.
    D3D11_SAMPLER_DESC desc;
    desc.Filter = msFilter[samplerState->filter];
    desc.AddressU = msMode[samplerState->mode[0]];
    desc.AddressV = msMode[samplerState->mode[1]];
    desc.AddressW = msMode[samplerState->mode[2]];
    desc.MipLODBias = samplerState->mipLODBias;
    desc.MaxAnisotropy = samplerState->maxAnisotropy;
    desc.ComparisonFunc = msComparison[samplerState->comparison];
    desc.BorderColor[0] = samplerState->borderColor[0];
    desc.BorderColor[1] = samplerState->borderColor[1];
    desc.BorderColor[2] = samplerState->borderColor[2];
    desc.BorderColor[3] = samplerState->borderColor[3];
    desc.MinLOD = samplerState->minLOD;
    desc.MaxLOD = samplerState->maxLOD;

    // Create the sampler state.
    ID3D11SamplerState* state = nullptr;
    DX11Log(device->CreateSamplerState(&desc, &state));
    mDXObject = state;
}

std::shared_ptr<GEObject> DX11SamplerState::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_SAMPLER_STATE)
    {
        return std::make_shared<DX11SamplerState>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<SamplerState const*>(object));
    }

    LogError("Invalid object type.");
}


D3D11_FILTER const DX11SamplerState::msFilter[] =
{
    D3D11_FILTER_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_ANISOTROPIC,
    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_ANISOTROPIC
};

D3D11_TEXTURE_ADDRESS_MODE const DX11SamplerState::msMode[] =
{
    D3D11_TEXTURE_ADDRESS_WRAP,
    D3D11_TEXTURE_ADDRESS_MIRROR,
    D3D11_TEXTURE_ADDRESS_CLAMP,
    D3D11_TEXTURE_ADDRESS_BORDER,
    D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
};

D3D11_COMPARISON_FUNC const DX11SamplerState::msComparison[] =
{
    D3D11_COMPARISON_NEVER,
    D3D11_COMPARISON_LESS,
    D3D11_COMPARISON_EQUAL,
    D3D11_COMPARISON_LESS_EQUAL,
    D3D11_COMPARISON_GREATER,
    D3D11_COMPARISON_NOT_EQUAL,
    D3D11_COMPARISON_GREATER_EQUAL,
    D3D11_COMPARISON_ALWAYS
};
