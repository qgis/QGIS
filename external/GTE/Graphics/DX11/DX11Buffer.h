// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Buffer.h>
#include <Graphics/DX11/DX11Resource.h>

namespace gte
{
    class DX11Buffer : public DX11Resource
    {
    public:
        // Abstract base class.
        virtual ~DX11Buffer() = default;
    protected:
        DX11Buffer(Buffer const* buffer);

    public:
        // Member access.
        inline Buffer* GetBuffer() const;
        inline ID3D11Buffer* GetDXBuffer() const;

        // Copy data from CPU to GPU via mapped memory.  Buffers use only
        // subresource 0, so the subresource index (sri) is not exposed.
        virtual bool Update(ID3D11DeviceContext* context) override;
        virtual bool CopyCpuToGpu(ID3D11DeviceContext* context) override;
        virtual bool CopyGpuToCpu(ID3D11DeviceContext* context) override;

        // Copy from GPU to GPU directly.  The first function copies the specified
        // subresource.  The second function copies all subresources.
        virtual void CopyGpuToGpu(ID3D11DeviceContext* context, ID3D11Resource* target) override;

    private:
        // Buffers use only subresource 0, so these overrides are stubbed out.
        virtual bool Update(ID3D11DeviceContext* context, unsigned int sri) override;
        virtual bool CopyCpuToGpu(ID3D11DeviceContext* context, unsigned int sri) override;
        virtual bool CopyGpuToCpu(ID3D11DeviceContext* context, unsigned int sri) override;
        virtual void CopyGpuToGpu(ID3D11DeviceContext* context, ID3D11Resource* target, unsigned int sri) override;

    protected:
        // Support for creating staging buffers.
        void CreateStaging(ID3D11Device* device, D3D11_BUFFER_DESC const& bf);

        // Dynamic constant buffers in D3D11.0 cannot be mapped using
        // D3D11_MAP_WRITE_NO_OVERWRITE, but they can in D3D11.1.  The MSDN
        // web page for D3D11_MAP has a note about this and suggests how to
        // test whether no-overwrite may be used by calling CheckFeatureSupport
        // on the device with feature D3D11_FEATURE_D3D11_OPTIONS.  Unfortunately,
        // the documentation for D3D11_FEATURE_D3D11_OPTIONS may only be used
        // for D3D11.1 and later.  This mechanism fails on an NVIDIA Quadro
        // K2200 (driver 362.13 and previous).  A call to the device
        // GetFeatureLevel returns D3D_FEATURE_LEVEL_11_0 but a call to
        // CheckFeatureSupport shows that MapNoOverwriteOnDynamicConstantBuffer
        // is 1 (so no-overwrite is supposed to be allowed).  Unfortunately,
        // this appears to cause problems in rendering.  Worse is that our text
        // rendering (TextEffect) uses dynamic vertex buffers and has strange
        // behavior when using no-overwrite.  All renders correctly with the
        // discard mode.  SO FOR NOW:  mUpdateMapMode has default value
        // D3D11_MAP_WRITE_DISCARD but is set to D3D11_MAP_WRITE_NO_OVERWRITE
        // when the feature level is found to be D3D_FEATURE_LEVEL_11_1 or
        // later.
        D3D11_MAP mUpdateMapMode;
    };

    inline Buffer* DX11Buffer::GetBuffer() const
    {
        return static_cast<Buffer*>(mGTObject);
    }

    inline ID3D11Buffer* DX11Buffer::GetDXBuffer() const
    {
        return static_cast<ID3D11Buffer*>(mDXObject);
    }
}
