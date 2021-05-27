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
    class Texture1 : public TextureSingle
    {
    public:
        // Construction.
        Texture1(DFType format, unsigned int length, bool hasMipmaps = false, bool createStorage = true);

        // Texture dimensions.
        inline unsigned int GetLength() const
        {
            return TextureSingle::GetDimension(0);
        }
    };
}
