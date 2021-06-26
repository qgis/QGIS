// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11TextureCubeArray.h>
using namespace gte;

DX11TextureCubeArray::DX11TextureCubeArray(ID3D11Device* device, TextureCubeArray const* textureCubeArray)
    :
    DX11TextureArray(textureCubeArray)
{
    // Specify the texture description.  TODO: Support texture cube RTs?
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = textureCubeArray->GetLength();
    desc.Height = textureCubeArray->GetLength();
    desc.MipLevels = textureCubeArray->GetNumLevels();
    desc.ArraySize = textureCubeArray->GetNumItems();
    desc.Format = static_cast<DXGI_FORMAT>(textureCubeArray->GetFormat());
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    Resource::Usage usage = textureCubeArray->GetUsage();
    if (usage == Resource::IMMUTABLE)
    {
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }
    else if (usage == Resource::DYNAMIC_UPDATE)
    {
        // DX11 does not allow a cube map to be a dynamic-update resource.
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }
    else  // usage == Resource::SHADER_OUTPUT
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }

    if (textureCubeArray->WantAutogenerateMipmaps())
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    // Create the texture.
    ID3D11Texture2D* dxTexture = nullptr;
    if (textureCubeArray->GetData())
    {
        unsigned int const numSubresources = textureCubeArray->GetNumSubresources();
        std::vector<D3D11_SUBRESOURCE_DATA> data(numSubresources);
        for (unsigned int index = 0; index < numSubresources; ++index)
        {
            auto sr = textureCubeArray->GetSubresource(index);
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
    if (textureCubeArray->GetUsage() == Resource::SHADER_OUTPUT)
    {
        CreateUAView(device, desc);
    }

    // Create a staging texture if requested.
    if (textureCubeArray->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }

    // Generate mipmaps if requested.
    if (textureCubeArray->WantAutogenerateMipmaps() && mSRView)
    {
        ID3D11DeviceContext* context;
        device->GetImmediateContext(&context);
        context->GenerateMips(mSRView);
        DX11::SafeRelease(context);
    }
}

std::shared_ptr<GEObject> DX11TextureCubeArray::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_CUBE_ARRAY)
    {
        return std::make_shared<DX11TextureCubeArray>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<TextureCubeArray const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11TextureCubeArray::CreateStaging(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = tx.Width;
    desc.Height = tx.Height;
    desc.MipLevels = tx.MipLevels;
    desc.ArraySize = tx.ArraySize;
    desc.Format = tx.Format;
    desc.SampleDesc.Count = tx.SampleDesc.Count;
    desc.SampleDesc.Quality = tx.SampleDesc.Quality;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = D3D11_BIND_NONE;
    desc.CPUAccessFlags = msStagingAccess[GetTextureArray()->GetCopyType()];
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;

    DX11Log(device->CreateTexture2D(&desc, nullptr, reinterpret_cast<ID3D11Texture2D**>(&mStaging)));
}

void DX11TextureCubeArray::CreateSRView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
    desc.TextureCubeArray.MostDetailedMip = 0;
    desc.TextureCubeArray.MipLevels = tx.MipLevels;
    desc.TextureCubeArray.First2DArrayFace = 0;
    desc.TextureCubeArray.NumCubes = tx.ArraySize / 6;

    DX11Log(device->CreateShaderResourceView(GetDXTextureArray(), &desc, &mSRView));
}

void DX11TextureCubeArray::CreateUAView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MipSlice = 0;
    desc.Texture2DArray.FirstArraySlice = 0;
    desc.Texture2DArray.ArraySize = tx.ArraySize;

    DX11Log(device->CreateUnorderedAccessView(GetDXTextureArray(), &desc, &mUAView));
}
