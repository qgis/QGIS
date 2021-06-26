// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DX11/HLSLBaseBuffer.h>

namespace gte
{
    class HLSLConstantBuffer : public HLSLBaseBuffer
    {
    public:
        // TODO: Global constants in the HLSL file are placed as variables
        // into an implicit constant buffer named $Global.  We need to modify
        // Variable to store number of bytes and default values (if any).
        // 1. If the application writer must create the constant buffer and
        //    attach, how do we get the default data into the buffer?
        // 2. Maybe $Global needs to be created implicitly, filled with
        //    default values, then application queries for it to modify.  The
        //    idea of shader->Set("SomeCBuffer",cbuffer) was to allow sharing
        //    of constant buffers between shaders.

        // Construction and destruction.
        virtual ~HLSLConstantBuffer() = default;

        HLSLConstantBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
            unsigned int numBytes, std::vector<Member> const& members);

        HLSLConstantBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
            unsigned int index, unsigned int numBytes,
            std::vector<Member> const& members);
    };
}
