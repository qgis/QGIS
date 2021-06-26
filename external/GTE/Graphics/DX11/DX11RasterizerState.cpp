// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11RasterizerState.h>
using namespace gte;

DX11RasterizerState::DX11RasterizerState(ID3D11Device* device, RasterizerState const* rasterizerState)
    :
    DX11DrawingState(rasterizerState)
{
    // Specify the rasterizer state description.
    D3D11_RASTERIZER_DESC desc;
    desc.FillMode = msFillMode[rasterizerState->fillMode];
    desc.CullMode = msCullMode[rasterizerState->cullMode];
    desc.FrontCounterClockwise = (rasterizerState->frontCCW ? TRUE : FALSE);
    desc.DepthBias = rasterizerState->depthBias;
    desc.DepthBiasClamp = rasterizerState->depthBiasClamp;
    desc.SlopeScaledDepthBias = rasterizerState->slopeScaledDepthBias;
    desc.DepthClipEnable = (rasterizerState->enableDepthClip ? TRUE : FALSE);
    desc.ScissorEnable = (rasterizerState->enableScissor ? TRUE : FALSE);
    desc.MultisampleEnable = (rasterizerState->enableMultisample ? TRUE : FALSE);
    desc.AntialiasedLineEnable = (rasterizerState->enableAntialiasedLine ? TRUE : FALSE);

    // Create the rasterizer state.
    ID3D11RasterizerState* state = nullptr;
    DX11Log(device->CreateRasterizerState(&desc, &state));
    mDXObject = state;
}

std::shared_ptr<GEObject> DX11RasterizerState::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_RASTERIZER_STATE)
    {
        return std::make_shared<DX11RasterizerState>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<RasterizerState const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11RasterizerState::Enable(ID3D11DeviceContext* context)
{
    context->RSSetState(GetDXRasterizerState());
}


D3D11_FILL_MODE const DX11RasterizerState::msFillMode[] =
{
    D3D11_FILL_SOLID,
    D3D11_FILL_WIREFRAME
};

D3D11_CULL_MODE const DX11RasterizerState::msCullMode[] =
{
    D3D11_CULL_NONE,
    D3D11_CULL_FRONT,
    D3D11_CULL_BACK
};
