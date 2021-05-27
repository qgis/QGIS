// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11GraphicsObject.h>
using namespace gte;

DX11GraphicsObject::~DX11GraphicsObject()
{
    if (mGTObject && mGTObject->IsDrawingState())
    {
        // Sampler, blend, depth-stencil, and rasterizer states have only a
        // finite number of possibilities in DX11.  If you create a state
        // whose settings duplicate one already in existence, DX11 gives you
        // a pointer to the existing one, incrementing the reference count
        // internally.  GTE does not track the duplicates, so we cannot
        // assert that the reference count is zero.
        DX11::SafeRelease(mDXObject);
    }
    else
    {
        DX11::FinalRelease(mDXObject);
    }
}

DX11GraphicsObject::DX11GraphicsObject(GraphicsObject const* gtObject)
    :
    GEObject(gtObject),
    mDXObject(nullptr)
{
}

void DX11GraphicsObject::SetName(std::string const& name)
{
    mName = name;
    DX11Log(DX11::SetPrivateName(mDXObject, mName));
}
