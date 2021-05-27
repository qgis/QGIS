// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.05.28

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11Buffer.h>
using namespace gte;

DX11Buffer::DX11Buffer(Buffer const* buffer)
    :
    DX11Resource(buffer),
    mUpdateMapMode(D3D11_MAP_WRITE_DISCARD)
{
}

bool DX11Buffer::Update(ID3D11DeviceContext* context)
{
    Buffer* buffer = GetBuffer();
    LogAssert(buffer->GetUsage() == Resource::DYNAMIC_UPDATE, "Buffer must be dynamic-update.");

    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Map the buffer.
        ID3D11Buffer* dxBuffer = GetDXBuffer();
        D3D11_MAPPED_SUBRESOURCE sub;
        DX11Log(context->Map(dxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub));

        // Copy from CPU memory.
        if (mUpdateMapMode != D3D11_MAP_WRITE_DISCARD)
        {
            unsigned int offsetInBytes = buffer->GetOffset() * buffer->GetElementSize();
            char const* source = buffer->GetData() + offsetInBytes;
            char* target = (char*)sub.pData + offsetInBytes;
            std::memcpy(target, source, numActiveBytes);
        }
        else
        {
            std::memcpy(sub.pData, buffer->GetData(), buffer->GetNumBytes());
        }
        context->Unmap(dxBuffer, 0);
    }
    else
    {
        LogWarning("Buffer has zero active bytes.");
    }
    return true;
}

bool DX11Buffer::CopyCpuToGpu(ID3D11DeviceContext* context)
{
    PreparedForCopy(D3D11_CPU_ACCESS_WRITE);

    Buffer* buffer = GetBuffer();
    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Map the staging buffer.
        D3D11_MAPPED_SUBRESOURCE sub;
        DX11Log(context->Map(mStaging, 0, D3D11_MAP_WRITE, 0, &sub));

        // Copy from CPU memory to staging buffer.  For buffers, the
        // 'box' members are specified in number of bytes.  The inputs
        // 'DstX', 'DstY', and 'DstZ' are also specified in number of bytes.
        unsigned int offsetInBytes = buffer->GetOffset() * buffer->GetElementSize();
        char const* source = buffer->GetData() + offsetInBytes;
        char* target = reinterpret_cast<char*>(sub.pData) + offsetInBytes;
        std::memcpy(target, source, numActiveBytes);
        context->Unmap(mStaging, 0);

        // Copy from staging buffer to GPU memory.  TODO: It seems that
        // buffer->GetOffset() should play a role in the copy.
        D3D11_BOX box = { offsetInBytes, 0, 0, offsetInBytes + numActiveBytes, 1, 1 };
        context->CopySubresourceRegion(GetDXBuffer(), 0, offsetInBytes, 0, 0, mStaging, 0, &box);
    }
    else
    {
        LogWarning("Buffer has zero active bytes.");
    }
    return true;
}

bool DX11Buffer::CopyGpuToCpu(ID3D11DeviceContext* context)
{
    PreparedForCopy(D3D11_CPU_ACCESS_READ);

    Buffer* buffer = GetBuffer();
    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from GPU memory to staging buffer.
        unsigned int offsetInBytes = buffer->GetOffset() * buffer->GetElementSize();
        D3D11_BOX box = { offsetInBytes, 0, 0, offsetInBytes + numActiveBytes, 1, 1 };
        context->CopySubresourceRegion(mStaging, 0, offsetInBytes, 0, 0, GetDXBuffer(), 0, &box);

        // Map the staging buffer.
        D3D11_MAPPED_SUBRESOURCE sub;
        DX11Log(context->Map(mStaging, 0, D3D11_MAP_READ, 0, &sub));

        // Copy from staging buffer to CPU memory.
        char const* source = reinterpret_cast<char*>(sub.pData) + offsetInBytes;
        char* target = buffer->GetData() + offsetInBytes;
        std::memcpy(target, source, numActiveBytes);
        context->Unmap(mStaging, 0);
    }
    else
    {
        LogWarning("Buffer has zero active bytes.");
    }
    return true;
}

void DX11Buffer::CopyGpuToGpu(ID3D11DeviceContext* context, ID3D11Resource* target)
{
    Buffer* buffer = GetBuffer();
    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        unsigned int offset = buffer->GetOffset();
        if (offset == 0 && numActiveBytes == buffer->GetNumBytes())
        {
            // Copy the entire source to the target.
            context->CopyResource(target, GetDXBuffer());
        }
        else
        {
            // Copy only a subset of the source to the target.
            unsigned int offsetInBytes = offset * buffer->GetElementSize();
            D3D11_BOX box = { offsetInBytes, 0, 0, offsetInBytes + numActiveBytes, 1, 1 };
            context->CopySubresourceRegion(target, 0, offsetInBytes, 0, 0, GetDXBuffer(), 0, &box);
        }
    }
    else
    {
        LogWarning("Buffer has zero active bytes.");
    }
}

bool DX11Buffer::Update(ID3D11DeviceContext*, unsigned int)
{
    LogError("Called polymorphically through DX11Resource.");
}

bool DX11Buffer::CopyCpuToGpu(ID3D11DeviceContext*, unsigned int)
{
    LogError("Called polymorphically through DX11Resource.");
}

bool DX11Buffer::CopyGpuToCpu(ID3D11DeviceContext*, unsigned int)
{
    LogError("Called polymorphically through DX11Resource.");
}

void DX11Buffer::CopyGpuToGpu(ID3D11DeviceContext*, ID3D11Resource*, unsigned int)
{
    LogError("Called polymorphically through DX11Resource.");
}

void DX11Buffer::CreateStaging(ID3D11Device* device, D3D11_BUFFER_DESC const& bf)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = bf.ByteWidth;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = D3D11_BIND_NONE;
    desc.CPUAccessFlags = msStagingAccess[GetBuffer()->GetCopyType()];
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
    desc.StructureByteStride = 0;

    DX11Log(device->CreateBuffer(&desc, nullptr, reinterpret_cast<ID3D11Buffer**>(&mStaging)));
}
