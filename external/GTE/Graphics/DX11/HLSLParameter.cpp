// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLParameter.h>
using namespace gte;

HLSLParameter::HLSLParameter(D3D_SIGNATURE_PARAMETER_DESC const& desc)
{
    mDesc.semanticName = std::string(desc.SemanticName);
    mDesc.semanticIndex = desc.SemanticIndex;
    mDesc.registerIndex = desc.Register;
    mDesc.systemValueType = desc.SystemValueType;
    mDesc.componentType = desc.ComponentType;
    mDesc.mask = desc.Mask;
    mDesc.readWriteMask = desc.ReadWriteMask;
    mDesc.stream = desc.Stream;
    mDesc.minPrecision = desc.MinPrecision;
}

void HLSLParameter::Print(std::ofstream& output) const
{
    output << "semantic name = " << mDesc.semanticName << std::endl;
    output << "semantic index = " << mDesc.semanticIndex << std::endl;
    output << "register index = " << mDesc.registerIndex << std::endl;
    output << "system value type = " << msSVName[mDesc.systemValueType] << std::endl;
    output << "register component type = " << msComponentType[mDesc.componentType] << std::endl;

    output << std::hex << std::showbase;
    output << "mask = " << mDesc.mask << std::endl;
    output << "read-write mask = " << mDesc.readWriteMask << std::endl;
    output << std::dec << std::noshowbase;

    output << "stream = " << mDesc.stream << std::endl;

    int i = static_cast<int>(mDesc.minPrecision);
    if (i & 0x000000F0)
    {
        i = 6 + (i & 1);
    }
    output << "min precision = " << msMinPrecision[i] << std::endl;
}


std::string const HLSLParameter::msSVName[] =
{
    "D3D_NAME_UNDEFINED",
    "D3D_NAME_POSITION",
    "D3D_NAME_CLIP_DISTANCE",
    "D3D_NAME_CULL_DISTANCE",
    "D3D_NAME_RENDER_TARGET_ARRAY_INDEX",
    "D3D_NAME_VIEWPORT_ARRAY_INDEX",
    "D3D_NAME_VERTEX_ID",
    "D3D_NAME_PRIMITIVE_ID",
    "D3D_NAME_INSTANCE_ID",
    "D3D_NAME_IS_FRONT_FACE",
    "D3D_NAME_SAMPLE_INDEX",
    "D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR",
    "D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR",
    "D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR",
    "D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR",
    "D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR",
    "D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR",
    "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", // 17-63 unused
    "D3D_NAME_TARGET",
    "D3D_NAME_DEPTH",
    "D3D_NAME_COVERAGE",
    "D3D_NAME_DEPTH_GREATER_EQUAL",
    "D3D_NAME_DEPTH_LESS_EQUAL"
};

std::string const HLSLParameter::msComponentType[] =
{
    "D3D_REGISTER_COMPONENT_UNKNOWN",
    "D3D_REGISTER_COMPONENT_UINT32",
    "D3D_REGISTER_COMPONENT_SINT32",
    "D3D_REGISTER_COMPONENT_FLOAT32"
};

std::string const HLSLParameter::msMinPrecision[] =
{
    "D3D_MIN_PRECISION_DEFAULT",
    "D3D_MIN_PRECISION_FLOAT_16",
    "D3D_MIN_PRECISION_FLOAT_2_8",
    "D3D_MIN_PRECISION_RESERVED",
    "D3D_MIN_PRECISION_SINT_16",
    "D3D_MIN_PRECISION_UINT_16",
    "D3D_MIN_PRECISION_ANY_16",
    "D3D_MIN_PRECISION_ANY_10"
};
