// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture.h>

namespace gte
{
    class TextureSingle : public Texture
    {
    protected:
        // Abstract base class.
        TextureSingle(DFType format, unsigned int numDimensions,
            unsigned int dim0, unsigned int dim1, unsigned int dim2,
            bool hasMipmaps, bool createStorage);

    public:
        // Mipmap information.
        inline unsigned int GetOffsetFor(unsigned int level) const
        {
            return Texture::GetOffsetFor(0, level);
        }

        inline char const* GetDataFor(unsigned int level) const
        {
            return Texture::GetDataFor(0, level);
        }

        inline char* GetDataFor(unsigned int level)
        {
            return Texture::GetDataFor(0, level);
        }

        template <typename T>
        inline T const* GetFor(unsigned int level) const
        {
            return Texture::GetFor<T>(0, level);
        }

        template <typename T>
        inline T* GetFor(unsigned int level)
        {
            return Texture::GetFor<T>(0, level);
        }

    public:
        // For use by the Shader class for storing reflection information.
        static int const shaderDataLookup = 4;
    };
}
