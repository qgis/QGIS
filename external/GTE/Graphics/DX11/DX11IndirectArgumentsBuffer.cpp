// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11IndirectArgumentsBuffer.h>
using namespace gte;

DX11IndirectArgumentsBuffer::DX11IndirectArgumentsBuffer(ID3D11Device* device, IndirectArgumentsBuffer const* iabuffer)
    :
    DX11Buffer(iabuffer)
{
    // Specify the counter buffer description.
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = iabuffer->GetNumBytes();
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_NONE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    desc.StructureByteStride = 0;

    // Create the counter buffer.
    ID3D11Buffer* buffer = nullptr;
    if (iabuffer->GetData())
    {
        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = iabuffer->GetData();
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        DX11Log(device->CreateBuffer(&desc, &data, &buffer));
    }
    else
    {
        DX11Log(device->CreateBuffer(&desc, nullptr, &buffer));
    }
    mDXObject = buffer;
}

std::shared_ptr<GEObject> DX11IndirectArgumentsBuffer::Create(void* device, GraphicsObject const* object)
{
    if (object->GetType() == GT_INDIRECT_ARGUMENTS_BUFFER)
    {
        return std::make_shared<DX11IndirectArgumentsBuffer>(
            reinterpret_cast<ID3D11Device*>(device),
            static_cast<IndirectArgumentsBuffer const*>(object));
    }

    LogError("Invalid object type.");
}
