// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Texture2Array.h>
using namespace gte;

Texture2Array::Texture2Array(unsigned int numItems, DFType format,
    unsigned int width, unsigned int height, bool hasMipmaps,
    bool createStorage)
    :
    TextureArray(numItems, format, 2, width, height, 1, hasMipmaps, createStorage)
{
    mType = GT_TEXTURE2_ARRAY;
}
