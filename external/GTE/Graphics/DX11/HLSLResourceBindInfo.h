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
    class HLSLResourceBindInfo : public HLSLBaseBuffer
    {
    public:
        // Construction and destruction.
        virtual ~HLSLResourceBindInfo() = default;

        HLSLResourceBindInfo(D3D_SHADER_INPUT_BIND_DESC const& desc,
            unsigned int numBytes, std::vector<Member> const& members);

        HLSLResourceBindInfo(D3D_SHADER_INPUT_BIND_DESC const& desc,
            unsigned int index, unsigned int numBytes,
            std::vector<Member> const& members);
    };
}
