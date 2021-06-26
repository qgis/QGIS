// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11RawBuffer.h>
using namespace gte;

DX11RawBuffer::~DX11RawBuffer()
{
    DX11::FinalRelease(mSRView);
    DX11::FinalRelease(mUAView);
}

DX11RawBuffer::DX11RawBuffer(ID3D11Device* device, RawBuffer const* rbuffer)
    :
    DX11Buffer(rbuffer),
    mSRView(nullptr),
    mUAView(nullptr)
{
    // Specify the buffer description.
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = rbuffer->GetNumBytes();
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    desc.StructureByteStride = 0;
    Resource::Usage usage = rbuffer->GetUsage();
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
    else  // usage == Resource::SHADER_OUTPUT
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }

    // Create the buffer.
    ID3D11Buffer* buffer = nullptr;
    if (rbuffer->GetData())
    {
        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = rbuffer->GetData();
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        DX11Log(device->CreateBuffer(&desc, &data, &buffer));
    }
    else
    {
        DX11Log(device->CreateBuffer(&desc, nullptr, &buffer));
    }
    mDXObject = buffer;

    // Create views of the buffer.
    CreateSRView(device);
    if (usage == Resource::SHADER_OUTPUT)
    {
        CreateUAView(device);
    }

    // Create a staging buffer if requested.
    if (rbuffer->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

std::shared_ptr<GEObject> DX11RawBuffer::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_RAW_BUFFER)
    {
        return std::make_shared<DX11RawBuffer>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<RawBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11RawBuffer::SetName(std::string const& name)
{
    DX11Buffer::SetName(name);
    DX11Log(DX11::SetPrivateName(mSRView, name));
    DX11Log(DX11::SetPrivateName(mUAView, name));
}

void DX11RawBuffer::CreateSRView(ID3D11Device* device)
{
    RawBuffer* buffer = GetRawBuffer();
    ID3D11Buffer* dxBuffer = GetDXBuffer();

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;
    desc.BufferEx.NumElements = buffer->GetNumElements();
    desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    DX11Log(device->CreateShaderResourceView(dxBuffer, &desc, &mSRView));
}

void DX11RawBuffer::CreateUAView(ID3D11Device* device)
{
    RawBuffer* buffer = GetRawBuffer();
    ID3D11Buffer* dxBuffer = GetDXBuffer();

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = buffer->GetNumElements();
    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
    DX11Log(device->CreateUnorderedAccessView(dxBuffer, &desc, &mUAView));
}
