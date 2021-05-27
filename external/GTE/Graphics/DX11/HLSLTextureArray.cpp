// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLTextureArray.h>
using namespace gte;

HLSLTextureArray::HLSLTextureArray(D3D_SHADER_INPUT_BIND_DESC const& desc)
    :
    HLSLResource(desc, 0)
{
    Initialize(desc);
}

HLSLTextureArray::HLSLTextureArray(D3D_SHADER_INPUT_BIND_DESC const& desc,
    unsigned int index)
    :
    HLSLResource(desc, index, 0)
{
    Initialize(desc);
}

void HLSLTextureArray::Initialize(D3D_SHADER_INPUT_BIND_DESC const& desc)
{
    mNumComponents = ((desc.uFlags >> 2) + 1);

    switch (desc.Dimension)
    {
    case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
        mNumDimensions = 1;
        break;
    case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
    case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
    case D3D_SRV_DIMENSION_TEXTURECUBE:
    case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
        mNumDimensions = 2;
        break;
    default:
        mNumDimensions = 0;
        break;
    }

    mGpuWritable = (desc.Type == D3D_SIT_UAV_RWTYPED);
}
