// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11IndexBuffer.h>
using namespace gte;

DX11IndexBuffer::DX11IndexBuffer(ID3D11Device* device, IndexBuffer const* ibuffer)
    :
    DX11Buffer(ibuffer),
    mFormat(ibuffer->GetElementSize() == sizeof(unsigned int) ?
        DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT)
{
    // Specify the buffer description.
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = ibuffer->GetNumBytes();
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
    desc.StructureByteStride = 0;
    Resource::Usage usage = ibuffer->GetUsage();
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
    if (ibuffer->GetData())
    {
        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = ibuffer->GetData();
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        DX11Log(device->CreateBuffer(&desc, &data, &buffer));
    }
    else
    {
        DX11Log(device->CreateBuffer(&desc, nullptr, &buffer));
    }
    mDXObject = buffer;

    // Create a staging buffer if requested.
    if (ibuffer->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

std::shared_ptr<GEObject> DX11IndexBuffer::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_INDEX_BUFFER)
    {
        return std::make_shared<DX11IndexBuffer>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<IndexBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void DX11IndexBuffer::Enable(ID3D11DeviceContext* context)
{
    if (mDXObject)
    {
        ID3D11Buffer* dxBuffer = static_cast<ID3D11Buffer*>(mDXObject);
        context->IASetIndexBuffer(dxBuffer, mFormat, 0);
    }
}

void DX11IndexBuffer::Disable(ID3D11DeviceContext* context)
{
    if (mDXObject)
    {
        context->IASetIndexBuffer(0, DXGI_FORMAT_UNKNOWN, 0);
    }
}
