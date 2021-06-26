// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DX11/HLSLResource.h>

namespace gte
{
    class HLSLByteAddressBuffer : public HLSLResource
    {
    public:
        // Construction and destruction.
        virtual ~HLSLByteAddressBuffer() = default;

        HLSLByteAddressBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc);
        HLSLByteAddressBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc, unsigned int index);

        // Member access.
        inline bool IsGpuWritable() const
        {
            return mGpuWritable;
        }

    private:
        bool mGpuWritable;
    };
}
