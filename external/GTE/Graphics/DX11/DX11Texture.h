// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture.h>
#include <Graphics/DX11/DX11Resource.h>

namespace gte
{
    class DX11Texture : public DX11Resource
    {
    public:
        // Abstract base class.
        virtual ~DX11Texture();
    protected:
        DX11Texture(Texture const* gtTexture);

    public:
        // Member access.
        inline Texture* GetTexture() const
        {
            return static_cast<Texture*>(mGTObject);
        }

        inline ID3D11ShaderResourceView* GetSRView() const
        {
            return mSRView;
        }

        inline ID3D11UnorderedAccessView* GetUAView() const
        {
            return mUAView;
        }

        // Copy of data between CPU and GPU.
        virtual bool Update(ID3D11DeviceContext* context, unsigned int sri) override;
        virtual bool Update(ID3D11DeviceContext* context) override;
        virtual bool CopyCpuToGpu(ID3D11DeviceContext* context, unsigned int sri) override;
        virtual bool CopyCpuToGpu(ID3D11DeviceContext* context) override;
        virtual bool CopyGpuToCpu(ID3D11DeviceContext* context, unsigned int sri) override;
        virtual bool CopyGpuToCpu(ID3D11DeviceContext* context) override;
        virtual void CopyGpuToGpu(ID3D11DeviceContext* context,
            ID3D11Resource* target, unsigned int sri) override;
        virtual void CopyGpuToGpu(ID3D11DeviceContext* context,
            ID3D11Resource* target) override;

        // Support for the DX11 debug layer; see comments in the file
        // DX11GraphicsObject.h about usage.
        virtual void SetName(std::string const& name) override;

    protected:
        // Support for copy of row-pitched and slice-pitched (noncontiguous)
        // texture memory.
        static void CopyPitched2(unsigned int numRows, unsigned int srcRowPitch,
            void const* srcData, unsigned int trgRowPitch, void* trgData);

        static void CopyPitched3(unsigned int numRows, unsigned int numSlices,
            unsigned int srcRowPitch, unsigned int srcSlicePitch,
            void const* srcData, unsigned int trgRowPitch,
            unsigned int trgSlicePitch, void* trgData);

        ID3D11ShaderResourceView* mSRView;
        ID3D11UnorderedAccessView* mUAView;
    };
}
