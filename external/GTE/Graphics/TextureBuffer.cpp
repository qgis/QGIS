// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TextureBuffer.h>
#include <cstring>
using namespace gte;

TextureBuffer::TextureBuffer(DFType format, unsigned int numElements, bool allowDynamicUpdate)
    :
    Buffer(numElements, DataFormat::GetNumBytesPerStruct(format), true),
    mFormat(format)
{
    mType = GT_TEXTURE_BUFFER;
    mUsage = (allowDynamicUpdate ? DYNAMIC_UPDATE : IMMUTABLE);
    std::memset(mData, 0, mNumBytes);
}

DFType TextureBuffer::GetFormat() const
{
    return mFormat;
}

bool TextureBuffer::HasMember(std::string const& name) const
{
    auto iter = std::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });
    return iter != mLayout.end();
}
