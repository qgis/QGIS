// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TextureCubeArray.h>
using namespace gte;

TextureCubeArray::TextureCubeArray(unsigned int numCubes, DFType format,
    unsigned int length, bool hasMipmaps, bool createStorage)
    :
    TextureArray(CubeFaceCount * numCubes, format, 2, length, length, 1, hasMipmaps, createStorage),
    mNumCubes(numCubes)
{
    mType = GT_TEXTURE_CUBE_ARRAY;
}
