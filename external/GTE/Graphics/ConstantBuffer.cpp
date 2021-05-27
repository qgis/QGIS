// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ConstantBuffer.h>
#include <algorithm>
#include <cstring>
using namespace gte;

ConstantBuffer::ConstantBuffer(size_t numBytes, bool allowDynamicUpdate)
    :
    Buffer(1, GetRoundedNumBytes(numBytes), true)
{
    mType = GT_CONSTANT_BUFFER;
    mUsage = (allowDynamicUpdate ? DYNAMIC_UPDATE : IMMUTABLE);
    std::memset(mData, 0, mNumBytes);
}

bool ConstantBuffer::HasMember(std::string const& name) const
{
    auto iter = std::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });
    return iter != mLayout.end();
}

size_t ConstantBuffer::GetRoundedNumBytes(size_t numBytes)
{
    if (numBytes > 0)
    {
        size_t remainder = numBytes % CBUFFER_REQUIRED_MINIMUM_BYTES;
        if (remainder == 0)
        {
            // The number is already the correct multiple.
            return numBytes;
        }
        else
        {
            // Round up to the nearest multiple.
            return numBytes + CBUFFER_REQUIRED_MINIMUM_BYTES - remainder;
        }
    }
    else
    {
        // At least one register must be allocated.
        return CBUFFER_REQUIRED_MINIMUM_BYTES;
    }
}
