// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11TextureBuffer.h>
using namespace gte;

DX11TextureBuffer::~DX11TextureBuffer()
{
    DX11::FinalRelease(mSRView);
}

DX11TextureBuffer::DX11TextureBuffer(ID3D11Device* device, TextureBuffer const* tbuffer)
    :
    DX11Buffer(tbuffer),
    mSRView(nullptr)
{
    // Specify the buffer description.
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = tbuffer->GetNumBytes();
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
    desc.StructureByteStride = 0;

    Resource::Usage usage = tbuffer->GetUsage();
    if (usage == Resource::IMMUTABLE)
    {
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }
    else if (usage == Resource::DYNAMIC_UPDATE)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }

    // Create the buffer.
    ID3D11Buffer* buffer = nullptr;
    if (tbuffer->GetData())
    {
        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = tbuffer->GetData();
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        DX11Log(device->CreateBuffer(&desc, &data, &buffer));
    }
    else
    {
        DX11Log(device->CreateBuffer(&desc, nullptr, &buffer));
    }
    mDXObject = buffer;

    // Create a SRV for the texture to be readable.
    CreateSRView(device);

    // Create a staging buffer if requested.
    if (tbuffer->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

std::shared_ptr<GEObject> DX11TextureBuffer::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_BUFFER)
    {
        return std::make_shared<DX11TextureBuffer>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<TextureBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11TextureBuffer::SetName(std::string const& name)
{
    DX11Buffer::SetName(name);
    DX11Log(DX11::SetPrivateName(mSRView, name));
}

void DX11TextureBuffer::CreateSRView(ID3D11Device* device)
{
    TextureBuffer* tbuffer = GetTextureBuffer();
    ID3D11Buffer* dxBuffer = GetDXBuffer();

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = static_cast<DXGI_FORMAT>(tbuffer->GetFormat());
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = tbuffer->GetNumElements();
    DX11Log(device->CreateShaderResourceView(dxBuffer, &desc, &mSRView));
}
