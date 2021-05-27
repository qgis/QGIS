// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11Resource.h>
using namespace gte;

DX11Resource::~DX11Resource()
{
    DX11::FinalRelease(mStaging);
}

DX11Resource::DX11Resource(Resource const* gtResource)
    :
    DX11GraphicsObject(gtResource),
    mStaging(nullptr)
{
    // Derived classes must create the staging resource, because DX11 does
    // not have a generic description structure that could be used here
    // otherwise.
}

D3D11_MAPPED_SUBRESOURCE DX11Resource::MapForWrite(ID3D11DeviceContext* context, unsigned int sri)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    DX11Log(context->Map(GetDXResource(), sri, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    return mapped;
}

void DX11Resource::Unmap(ID3D11DeviceContext* context, unsigned int sri)
{
    context->Unmap(GetDXResource(), sri);
}

void DX11Resource::SetName(std::string const& name)
{
    DX11GraphicsObject::SetName(name);
    DX11Log(DX11::SetPrivateName(mStaging, name));
}

void DX11Resource::PreparedForCopy(D3D11_CPU_ACCESS_FLAG access) const
{
    // Verify existence of objects.
    LogAssert(mDXObject != nullptr, "DX object does not exist.");
    LogAssert(mStaging != nullptr, "Staging object does not exist.");

    // Verify the copy type.
    LogAssert((msStagingAccess[GetResource()->GetCopyType()] & access) != 0, "Invalid copy type.");
}


UINT const DX11Resource::msStagingAccess[] =
{
    D3D11_CPU_ACCESS_NONE,          // COPY_NONE
    D3D11_CPU_ACCESS_WRITE,         // COPY_CPU_TO_STAGING
    D3D11_CPU_ACCESS_READ,          // COPY_STAGING_TO_CPU
    D3D11_CPU_ACCESS_READ_WRITE     // COPY_BIDIRECTIONAL
};
