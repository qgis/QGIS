// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureArray.h>
#include <Graphics/DX11/DX11Texture.h>

namespace gte
{
    class DX11TextureArray : public DX11Texture
    {
    public:
        // Abstract base class, a shim to distinguish between single
        // textures and texture arrays.
        virtual ~DX11TextureArray() = default;
    protected:
        DX11TextureArray(TextureArray const* gtTextureArray);

    public:
        // Member access.
        inline TextureArray* GetTextureArray() const
        {
            return static_cast<TextureArray*>(mGTObject);
        }
    };
}
