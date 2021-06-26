// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11TextureRT.h>
using namespace gte;

DX11TextureRT::~DX11TextureRT()
{
    DX11::FinalRelease(mRTView);
}

DX11TextureRT::DX11TextureRT(ID3D11Device* device, TextureRT const* texture)
    :
    DX11Texture2(texture),
    mRTView(nullptr)
{
    // Specify the texture description.
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = texture->GetWidth();
    desc.Height = texture->GetHeight();
    desc.MipLevels = texture->GetNumLevels();
    desc.ArraySize = 1;
    desc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    desc.MiscFlags = (texture->IsShared() ?
        D3D11_RESOURCE_MISC_SHARED : D3D11_RESOURCE_MISC_NONE);

    if (texture->GetUsage() == Resource::SHADER_OUTPUT)
    {
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    if (texture->WantAutogenerateMipmaps() && !texture->IsShared())
    {
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    // Create the texture.
    ID3D11Texture2D* dxTexture = nullptr;
    if (texture->GetData())
    {
        unsigned int const numSubresources = texture->GetNumSubresources();
        std::vector<D3D11_SUBRESOURCE_DATA> data(numSubresources);
        for (unsigned int index = 0; index < numSubresources; ++index)
        {
            auto sr = texture->GetSubresource(index);
            data[index].pSysMem = sr.data;
            data[index].SysMemPitch = sr.rowPitch;
            data[index].SysMemSlicePitch = 0;
        }
        DX11Log(device->CreateTexture2D(&desc, &data[0], &dxTexture));
    }
    else
    {
        DX11Log(device->CreateTexture2D(&desc, nullptr, &dxTexture));
    }
    mDXObject = dxTexture;

    // Create views of the texture.
    CreateSRView(device, desc);
    CreateRTView(device, desc);
    if (texture->GetUsage() == Resource::SHADER_OUTPUT)
    {
        CreateUAView(device, desc);
    }

    // Create a staging texture if requested.
    if (texture->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }

    // Generate mipmaps if requested.
    if (texture->WantAutogenerateMipmaps() && mSRView)
    {
        ID3D11DeviceContext* context;
        device->GetImmediateContext(&context);
        context->GenerateMips(mSRView);
        DX11::SafeRelease(context);
    }
}

DX11TextureRT::DX11TextureRT(ID3D11Device* device, DX11TextureRT const* dxSharedTexture)
    :
    DX11Texture2(dxSharedTexture->GetTexture()),
    mRTView(nullptr)
{
    ID3D11Texture2D* dxShared = dxSharedTexture->CreateSharedDXObject(device);
    mDXObject = dxShared;
    D3D11_TEXTURE2D_DESC desc;
    dxShared->GetDesc(&desc);
    CreateRTView(device, desc);
}

std::shared_ptr<GEObject> DX11TextureRT::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_RT)
    {
        return std::make_shared<DX11TextureRT>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<TextureRT const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11TextureRT::SetName(std::string const& name)
{
    DX11Texture2::SetName(name);
    DX11Log(DX11::SetPrivateName(mRTView, name));
}

void DX11TextureRT::CreateRTView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;

    DX11Log(device->CreateRenderTargetView(GetDXTexture(), &desc, &mRTView));
}
