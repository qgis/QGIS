// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GEObject.h>
#include <Graphics/DX11/DX11.h>

namespace gte
{
    class DX11GraphicsObject : public GEObject
    {
    public:
        // Abstract base class.
        virtual ~DX11GraphicsObject();
    protected:
        DX11GraphicsObject(GraphicsObject const* gtObject);

    public:
        // Member access.
        inline ID3D11DeviceChild* GetDXDeviceChild() const
        {
            return mDXObject;
        }

        // Support for the DX11 debug layer.  Set the name if you want to have
        // ID3D11DeviceChild destruction messages show your name rather than
        // "<unnamed>".  The typical usage is
        //   auto texture = std::make_shared<Texture2>(...);
        //   engine->Bind(texture)->SetName("MyTexture");
        // The virtual override is used to allow derived classes to use the
        // same name for associated resources.
        virtual void SetName(std::string const& name) override;

    protected:
        ID3D11DeviceChild* mDXObject;
    };
}
