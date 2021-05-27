// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLShaderType.h>
using namespace gte;

HLSLShaderType::HLSLShaderType()
    :
    mName("")
{
}

void HLSLShaderType::SetDescription(D3D_SHADER_TYPE_DESC const& desc)
{
    mDesc.varClass = desc.Class;
    mDesc.varType = desc.Type;
    mDesc.numRows = desc.Rows;
    mDesc.numColumns = desc.Columns;
    mDesc.numElements = desc.Elements;
    mDesc.numChildren = desc.Members;
    mDesc.offset = desc.Offset;
    mDesc.typeName = std::string(desc.Name ? desc.Name : "");

    if (desc.Members > 0)
    {
        mChildren.resize(desc.Members);
    }
    else
    {
        mChildren.clear();
    }
}

HLSLShaderType& HLSLShaderType::GetChild(unsigned int i)
{
    LogAssert(i < mDesc.numChildren, "Invalid index.");
    return mChildren[i];
}

HLSLShaderType const& HLSLShaderType::GetChild(unsigned int i) const
{
    LogAssert(i < mDesc.numChildren, "Invalid index.");
    return mChildren[i];
}

void HLSLShaderType::Print(std::ofstream& output, int indent) const
{
    std::string prefix = "";
    for (int i = 0; i < indent; ++i)
    {
        prefix += "    ";
    }

    output << prefix << "name = " << mName << std::endl;
    output << prefix << "variable class = " << msVarClass[mDesc.varClass] << std::endl;
    output << prefix << "variable type = " << msVarType[mDesc.varType] << std::endl;
    output << prefix << "rows = " << mDesc.numRows << std::endl;
    output << prefix << "columns = " << mDesc.numColumns << std::endl;
    output << prefix << "elements = " << mDesc.numElements << std::endl;
    output << prefix << "children = " << mDesc.numChildren << std::endl;
    output << prefix << "offset = " << mDesc.offset << std::endl;
    output << prefix << "type name = " << mDesc.typeName << std::endl;

    ++indent;
    for (auto const& child : mChildren)
    {
        child.Print(output, indent);
    }
}


std::string const HLSLShaderType::msVarClass[] =
{
    "D3D_SVC_SCALAR",
    "D3D_SVC_VECTOR",
    "D3D_SVC_MATRIX_ROWS",
    "D3D_SVC_MATRIX_COLUMNS",
    "D3D_SVC_OBJECT",
    "D3D_SVC_STRUCT",
    "D3D_SVC_INTERFACE_CLASS",
    "D3D_SVC_INTERFACE_POINTER"
};

std::string const HLSLShaderType::msVarType[] =
{
    "D3D_SVT_VOID",
    "D3D_SVT_BOOL",
    "D3D_SVT_INT",
    "D3D_SVT_FLOAT",
    "D3D_SVT_STRING",
    "D3D_SVT_TEXTURE",
    "D3D_SVT_TEXTURE1D",
    "D3D_SVT_TEXTURE2D",
    "D3D_SVT_TEXTURE3D",
    "D3D_SVT_TEXTURECUBE",
    "D3D_SVT_SAMPLER",
    "D3D_SVT_SAMPLER1D",
    "D3D_SVT_SAMPLER2D",
    "D3D_SVT_SAMPLER3D",
    "D3D_SVT_SAMPLERCUBE",
    "D3D_SVT_PIXELSHADER",
    "D3D_SVT_VERTEXSHADER",
    "D3D_SVT_PIXELFRAGMENT",
    "D3D_SVT_VERTEXFRAGMENT",
    "D3D_SVT_UINT",
    "D3D_SVT_UINT8",
    "D3D_SVT_GEOMETRYSHADER",
    "D3D_SVT_RASTERIZER",
    "D3D_SVT_DEPTHSTENCIL",
    "D3D_SVT_BLEND",
    "D3D_SVT_BUFFER",
    "D3D_SVT_CBUFFER",
    "D3D_SVT_TBUFFER",
    "D3D_SVT_TEXTURE1DARRAY",
    "D3D_SVT_TEXTURE2DARRAY",
    "D3D_SVT_RENDERTARGETVIEW",
    "D3D_SVT_DEPTHSTENCILVIEW",
    "D3D_SVT_TEXTURE2DMS",
    "D3D_SVT_TEXTURE2DMSARRAY",
    "D3D_SVT_TEXTURECUBEARRAY",
    "D3D_SVT_HULLSHADER",
    "D3D_SVT_DOMAINSHADER",
    "D3D_SVT_INTERFACE_POINTER",
    "D3D_SVT_COMPUTESHADER",
    "D3D_SVT_DOUBLE",
    "D3D_SVT_RWTEXTURE1D",
    "D3D_SVT_RWTEXTURE1DARRAY",
    "D3D_SVT_RWTEXTURE2D",
    "D3D_SVT_RWTEXTURE2DARRAY",
    "D3D_SVT_RWTEXTURE3D",
    "D3D_SVT_RWBUFFER",
    "D3D_SVT_BYTEADDRESS_BUFFER",
    "D3D_SVT_RWBYTEADDRESS_BUFFER",
    "D3D_SVT_STRUCTURED_BUFFER",
    "D3D_SVT_RWSTRUCTURED_BUFFER",
    "D3D_SVT_APPEND_STRUCTURED_BUFFER",
    "D3D_SVT_CONSUME_STRUCTURED_BUFFER",
    "D3D_SVT_MIN8FLOAT",
    "D3D_SVT_MIN10FLOAT",
    "D3D_SVT_MIN16FLOAT",
    "D3D_SVT_MIN12INT",
    "D3D_SVT_MIN16INT",
    "D3D_SVT_MIN16UINT"
};
