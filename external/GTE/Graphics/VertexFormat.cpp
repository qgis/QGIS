// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/VertexFormat.h>
#include <Mathematics/Logger.h>
using namespace gte;

VertexFormat::VertexFormat()
    :
    mNumAttributes(0),
    mVertexSize(0)
{
}

void VertexFormat::Reset()
{
    mNumAttributes = 0;
    mVertexSize = 0;
    for (int i = 0; i < VA_MAX_ATTRIBUTES; ++i)
    {
        mAttributes[i] = Attribute();
    }
}

void VertexFormat::Bind(VASemantic semantic, DFType type, unsigned int unit)
{
    // Validate the inputs.
    LogAssert(0 <= mNumAttributes && mNumAttributes < VA_MAX_ATTRIBUTES, "Exceeded maximum attributes.");
    if (semantic == VA_COLOR)
    {
        LogAssert(unit < VA_MAX_COLOR_UNITS, "Invalid color unit.");
    }
    else if (semantic == VA_TEXCOORD)
    {
        LogAssert(unit < VA_MAX_TCOORD_UNITS, "Invalid texture unit.");
    }
    else
    {
        LogAssert(unit == 0, "Invalid semantic unit.");
    }

    // Accept the attribute.
    Attribute& attribute = mAttributes[mNumAttributes];
    attribute.semantic = semantic;
    attribute.type = type;
    attribute.unit = unit;
    attribute.offset = mVertexSize;
    ++mNumAttributes;

    // Advance the offset.
    mVertexSize += DataFormat::GetNumBytesPerStruct(type);
}

void VertexFormat::GetAttribute(int i, VASemantic& semantic, DFType& type,
    unsigned int& unit, unsigned int& offset) const
{
    LogAssert(0 <= i && i < mNumAttributes, "Invalid index " + std::to_string(i) + ".");
    Attribute const& attribute = mAttributes[i];
    semantic = attribute.semantic;
    type = attribute.type;
    unit = attribute.unit;
    offset = attribute.offset;
}

int VertexFormat::GetIndex(VASemantic semantic, unsigned int unit) const
{
    for (int i = 0; i < mNumAttributes; ++i)
    {
        Attribute const& attribute = mAttributes[i];
        if (attribute.semantic == semantic && attribute.unit == unit)
        {
            return i;
        }
    }

    return -1;
}

DFType VertexFormat::GetType(int i) const
{
    LogAssert(0 <= i && i < mNumAttributes, "Invalid index " + std::to_string(i) + ".");
    return mAttributes[i].type;
}

unsigned int VertexFormat::GetOffset(int i) const
{
    LogAssert(0 <= i && i < mNumAttributes, "Invalid index " + std::to_string(i) + ".");
    return mAttributes[i].offset;
}
