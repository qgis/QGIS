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
    class HLSLStructuredBuffer : public HLSLResource
    {
    public:
        enum Type
        {
            SBT_INVALID,
            SBT_BASIC,
            SBT_APPEND,
            SBT_CONSUME,
            SBT_COUNTER
        };

        // Construction and destruction.
        virtual ~HLSLStructuredBuffer() = default;

        HLSLStructuredBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc);
        HLSLStructuredBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc, unsigned int index);

        // Member access.
        inline HLSLStructuredBuffer::Type GetType() const
        {
            return mType;
        }

        inline bool IsGpuWritable() const
        {
            return mGpuWritable;
        }

    private:
        void Initialize(D3D_SHADER_INPUT_BIND_DESC const& desc);

        Type mType;
        bool mGpuWritable;
    };
}
