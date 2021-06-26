// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TextureArray.h>
using namespace gte;

TextureArray::TextureArray(unsigned int numItems, DFType format,
    unsigned int numDimensions, unsigned int dim0, unsigned int dim1,
    unsigned int dim2, bool hasMipmaps, bool createStorage)
    :
    Texture(numItems, format, numDimensions, dim0, dim1, dim2, hasMipmaps, createStorage)
{
    mType = GT_TEXTURE_ARRAY;
}
