// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLResource.h>
using namespace gte;

HLSLResource::HLSLResource(D3D_SHADER_INPUT_BIND_DESC const& desc, unsigned int numBytes)
    :
    mNumBytes(numBytes)
{
    mDesc.name = std::string(desc.Name);
    mDesc.bindPoint = desc.BindPoint;
    mDesc.bindCount = desc.BindCount;
    mDesc.type = desc.Type;
    mDesc.flags = desc.uFlags;
    mDesc.returnType = desc.ReturnType;
    mDesc.dimension = desc.Dimension;
    mDesc.numSamples = desc.NumSamples;
}

HLSLResource::HLSLResource(D3D_SHADER_INPUT_BIND_DESC const& desc, unsigned int index, unsigned int numBytes)
    :
    mNumBytes(numBytes)
{
    mDesc.name = std::string(desc.Name) + "[" + std::to_string(index) + "]";
    mDesc.bindPoint = desc.BindPoint + index;
    mDesc.bindCount = 1;
    mDesc.type = desc.Type;
    mDesc.flags = desc.uFlags;
    mDesc.returnType = desc.ReturnType;
    mDesc.dimension = desc.Dimension;
    mDesc.numSamples = desc.NumSamples;
}

void HLSLResource::Print(std::ofstream& output) const
{
    output << "name = " << mDesc.name << std::endl;
    output << "shader input type = " << msSIType[mDesc.type] << std::endl;
    output << "bind point = " << mDesc.bindPoint << std::endl;
    output << "bind count = " << mDesc.bindCount << std::endl;
    output << "flags = " << mDesc.flags << std::endl;
    output << "return type = " << msReturnType[mDesc.returnType] << std::endl;
    output << "dimension = " << msSRVDimension[mDesc.dimension] << std::endl;
    if (mDesc.numSamples == 0xFFFFFFFFu)
    {
        output << "samples = -1" << std::endl;
    }
    else
    {
        output << "samples = " << mDesc.numSamples << std::endl;
    }
    output << "number of bytes = " << mNumBytes << std::endl;
}


std::string const HLSLResource::msSIType[] =
{
    "D3D_SIT_CBUFFER",
    "D3D_SIT_TBUFFER",
    "D3D_SIT_TEXTURE",
    "D3D_SIT_SAMPLER",
    "D3D_SIT_UAV_RWTYPED",
    "D3D_SIT_STRUCTURED",
    "D3D_SIT_UAV_RWSTRUCTURED",
    "D3D_SIT_BYTEADDRESS",
    "D3D_SIT_UAV_RWBYTEADDRESS",
    "D3D_SIT_UAV_APPEND_STRUCTURED",
    "D3D_SIT_UAV_CONSUME_STRUCTURED",
    "D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER"
};

std::string const HLSLResource::msReturnType[] =
{
    "none",  // There is no D3D_RESOURCE_RETURN_TYPE for value 0.
    "D3D_RETURN_TYPE_UNORM",
    "D3D_RETURN_TYPE_SNORM",
    "D3D_RETURN_TYPE_SINT",
    "D3D_RETURN_TYPE_UINT",
    "D3D_RETURN_TYPE_FLOAT",
    "D3D_RETURN_TYPE_MIXED",
    "D3D_RETURN_TYPE_DOUBLE",
    "D3D_RETURN_TYPE_CONTINUED"
};

std::string const HLSLResource::msSRVDimension[] =
{
    "D3D_SRV_DIMENSION_UNKNOWN",
    "D3D_SRV_DIMENSION_BUFFER",
    "D3D_SRV_DIMENSION_TEXTURE1D",
    "D3D_SRV_DIMENSION_TEXTURE1DARRAY",
    "D3D_SRV_DIMENSION_TEXTURE2D",
    "D3D_SRV_DIMENSION_TEXTURE2DARRAY",
    "D3D_SRV_DIMENSION_TEXTURE2DMS",
    "D3D_SRV_DIMENSION_TEXTURE2DMSARRAY",
    "D3D_SRV_DIMENSION_TEXTURE3D",
    "D3D_SRV_DIMENSION_TEXTURECUBE",
    "D3D_SRV_DIMENSION_TEXTURECUBEARRAY",
    "D3D_SRV_DIMENSION_BUFFEREX"
};
