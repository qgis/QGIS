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
    class Texture2Array : public TextureArray
    {
    public:
        // Construction.
        Texture2Array(unsigned int numItems, DFType format, unsigned int width,
            unsigned int height, bool hasMipmaps = false, bool createStorage = true);

        // Texture dimensions.
        inline unsigned int GetWidth() const
        {
            return TextureArray::GetDimension(0);
        }

        inline unsigned int GetHeight() const
        {
            return TextureArray::GetDimension(1);
        }
    };
}
