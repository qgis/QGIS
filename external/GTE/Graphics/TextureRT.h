// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture2.h>

namespace gte
{
    class TextureRT : public Texture2
    {
    public:
        // Construction for render targets.
        TextureRT(DFType format, unsigned int width, unsigned int height,
            bool hasMipmaps = false, bool createStorage = true);
    };
}
