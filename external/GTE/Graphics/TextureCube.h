// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureArray.h>

namespace gte
{
    class TextureCube : public TextureArray
    {
    public:
        // Construction.  Cube maps must be square; the 'length' parameter is
        // the shared value for width and height.
        TextureCube(DFType format, unsigned int length, bool hasMipmaps = false,
            bool createStorage = true);

        // The texture width and height are the same value.
        inline unsigned int GetLength() const
        {
            return TextureArray::GetDimension(0);
        }
    };
}
