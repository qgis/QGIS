// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11TextureDS.h>
using namespace gte;

DX11TextureDS::~DX11TextureDS()
{
    DX11::FinalRelease(mDSView);
}

DX11TextureDS::DX11TextureDS(ID3D11Device* device, TextureDS const* texture)
    :
    DX11Texture2(texture),
    mDSView(nullptr)
{
    // Specify the texture description.
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = texture->GetWidth();
    desc.Height = texture->GetHeight();
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = GetDepthResourceFormat(static_cast<DXGI_FORMAT>(texture->GetFormat()));
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    desc.MiscFlags = (texture->IsShared() ?
        D3D11_RESOURCE_MISC_SHARED : D3D11_RESOURCE_MISC_NONE);
    if (texture->IsShaderInput())
    {
        desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    // Create the texture.  Depth-stencil textures are not initialized by
    // system memory data.
    ID3D11Texture2D* dxTexture = nullptr;
    DX11Log(device->CreateTexture2D(&desc, nullptr, &dxTexture));
    mDXObject = dxTexture;

    // Create a view of the texture.
    CreateDSView(device);

    // Create a shader resource view if the depth-stencil is to be used as an
    // input to shaders.
    if (texture->IsShaderInput())
    {
        CreateDSSRView(device);
    }

    // Create a staging texture if requested.
    if (texture->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

DX11TextureDS::DX11TextureDS(ID3D11Device* device, DX11TextureDS const* dxSharedTexture)
    :
    DX11Texture2(dxSharedTexture->GetTexture()),
    mDSView(nullptr)
{
    ID3D11Texture2D* dxShared = dxSharedTexture->CreateSharedDXObject(device);
    mDXObject = dxShared;
    CreateDSView(device);
}

std::shared_ptr<GEObject> DX11TextureDS::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_DS)
    {
        return std::make_shared<DX11TextureDS>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<TextureDS const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11TextureDS::SetName(std::string const& name)
{
    DX11Texture2::SetName(name);
    DX11Log(DX11::SetPrivateName(mDSView, name));
}

void DX11TextureDS::CreateDSView(ID3D11Device* device)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
    desc.Format = static_cast<DXGI_FORMAT>(GetTexture()->GetFormat());
    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    desc.Flags = 0;
    desc.Texture2D.MipSlice = 0;
    DX11Log(device->CreateDepthStencilView(GetDXTexture(), &desc, &mDSView));
}

void DX11TextureDS::CreateDSSRView(ID3D11Device* device)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = GetDepthSRVFormat(static_cast<DXGI_FORMAT>(GetTexture()->GetFormat()));
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MostDetailedMip = 0;
    desc.Texture2D.MipLevels = 1;
    DX11Log(device->CreateShaderResourceView(GetDXTexture(), &desc, &mSRView));
}

DXGI_FORMAT DX11TextureDS::GetDepthResourceFormat(DXGI_FORMAT depthFormat)
{
    if (depthFormat == DXGI_FORMAT_D16_UNORM)
    {
        return DXGI_FORMAT_R16_TYPELESS;
    }

    if (depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
    {
        return DXGI_FORMAT_R24G8_TYPELESS;
    }

    if (depthFormat == DXGI_FORMAT_D32_FLOAT)
    {
        return DXGI_FORMAT_R32_TYPELESS;
    }

    if (depthFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
    {
        return DXGI_FORMAT_R32G8X24_TYPELESS;
    }

    LogError("Invalid depth format.");
}

DXGI_FORMAT DX11TextureDS::GetDepthSRVFormat(DXGI_FORMAT depthFormat)
{
    if (depthFormat == DXGI_FORMAT_D16_UNORM)
    {
        return DXGI_FORMAT_R16_UNORM;
    }

    if (depthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
    {
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    }

    if (depthFormat == DXGI_FORMAT_D32_FLOAT)
    {
        return DXGI_FORMAT_R32_FLOAT;
    }

    if (depthFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
    {
        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
    }

    LogError("Invalid depth format.");
}
