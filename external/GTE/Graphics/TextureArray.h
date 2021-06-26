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
    class TextureArray : public Texture
    {
    protected:
        // Abstract base class (shim).  All items in the array have the same
        // format, number of dimensions, dimension values, and mipmap status.
        TextureArray(unsigned int numItems, DFType format,
            unsigned int numDimensions, unsigned int dim0, unsigned int dim1,
            unsigned int dim2, bool hasMipmaps, bool createStorage);

    public:
        // Mipmap information.
        inline unsigned int GetOffsetFor(unsigned int item, unsigned int level) const
        {
            return Texture::GetOffsetFor(item, level);
        }

        inline char const* GetDataFor(unsigned int item, unsigned int level) const
        {
            return Texture::GetDataFor(item, level);
        }

        inline char* GetDataFor(unsigned int item, unsigned int level)
        {
            return Texture::GetDataFor(item, level);
        }

        template <typename T>
        inline T const* GetFor(unsigned int item, unsigned int level) const
        {
            return Texture::GetFor<T>(item, level);
        }

        template <typename T>
        inline T* GetFor(unsigned int item, unsigned int level)
        {
            return Texture::GetFor<T>(item, level);
        }

    public:
        // For use by the Shader class for storing reflection information.
        static int const shaderDataLookup = 5;

        // Used as face index in TextureCube and TextureCubeArray.
        static const unsigned int CubeFacePositiveX = 0;
        static const unsigned int CubeFaceNegativeX = 1;
        static const unsigned int CubeFacePositiveY = 2;
        static const unsigned int CubeFaceNegativeY = 3;
        static const unsigned int CubeFacePositiveZ = 4;
        static const unsigned int CubeFaceNegativeZ = 5;
        static const unsigned int CubeFaceCount = 6;
    };

    typedef std::function<void(std::shared_ptr<TextureArray> const&)> TextureArrayUpdater;
    typedef std::function<void(std::shared_ptr<TextureArray> const&, unsigned int)> TextureArrayLevelUpdater;
}
