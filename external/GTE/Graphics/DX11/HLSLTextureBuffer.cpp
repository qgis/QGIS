// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLTextureBuffer.h>
using namespace gte;

HLSLTextureBuffer::HLSLTextureBuffer(
    D3D_SHADER_INPUT_BIND_DESC const& desc, unsigned int numBytes,
    std::vector<Member> const& members)
    :
    HLSLBaseBuffer(desc, numBytes, members)
{
}

HLSLTextureBuffer::HLSLTextureBuffer(
    D3D_SHADER_INPUT_BIND_DESC const& desc, unsigned int index,
    unsigned int numBytes, std::vector<Member> const& members)
    :
    HLSLBaseBuffer(desc, index, numBytes, members)
{
}
