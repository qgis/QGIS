// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/RawBuffer.h>
#include <Graphics/DX11/DX11Buffer.h>

namespace gte
{
    class DX11RawBuffer : public DX11Buffer
    {
    public:
        // Construction and destruction.
        virtual ~DX11RawBuffer();
        DX11RawBuffer(ID3D11Device* device, RawBuffer const* rbuffer);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline RawBuffer* GetRawBuffer() const
        {
            return static_cast<RawBuffer*>(mGTObject);
        }

        inline ID3D11ShaderResourceView* GetSRView() const
        {
            return mSRView;
        }

        inline ID3D11UnorderedAccessView* GetUAView() const
        {
            return mUAView;
        }

        // Support for the DX11 debug layer; see comments in the file
        // DX11GraphicsObject.h about usage.
        virtual void SetName(std::string const& name);

    private:
        // Support for construction.
        void CreateSRView(ID3D11Device* device);
        void CreateUAView(ID3D11Device* device);

        ID3D11ShaderResourceView* mSRView;
        ID3D11UnorderedAccessView* mUAView;
    };
}
