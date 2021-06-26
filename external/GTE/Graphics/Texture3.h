// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureSingle.h>

namespace gte
{
    class Texture3 : public TextureSingle
    {
    public:
        // Construction.
        Texture3(DFType format, unsigned int width, unsigned int height,
            unsigned int thickness, bool hasMipmaps = false, bool createStorage = true);

        // Texture dimensions.
        inline unsigned int GetWidth() const
        {
            return TextureSingle::GetDimension(0);
        }

        inline unsigned int GetHeight() const
        {
            return TextureSingle::GetDimension(1);
        }

        inline unsigned int GetThickness() const
        {
            return TextureSingle::GetDimension(2);
        }
    };
}
