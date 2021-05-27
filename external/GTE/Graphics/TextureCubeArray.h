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
    class TextureCubeArray : public TextureArray
    {
    public:
        // Construction.  Cube maps must be square; the 'length' parameter is
        // the shared value for width and height of a face.  The 'numCubes' is
        // the number of 6-tuples of cube maps.
        TextureCubeArray(unsigned int numCubes, DFType format, unsigned int length,
            bool hasMipmaps = false, bool createStorage = true);

        // Member access.
        inline unsigned int GetNumCubes() const
        {
            return mNumCubes;
        }

        // The texture width and height are the same value.
        inline unsigned int GetLength() const
        {
            return TextureArray::GetDimension(0);
        }

        // Faces for all the of the cubes are stored contiguously in one large
        // array so GetNumItems() will return a number that is the same as
        // 6*GetNumCubes().  These methods allow mapping between the array
        // itemIndex and the corresponding (cubeIndex, faceIndex) pair.
        inline unsigned int GetItemIndexFor(unsigned int cube, unsigned int face) const
        {
            return cube * 6 + face;
        }

        inline unsigned int GetCubeIndexFor(unsigned int item) const
        {
            return item / 6;
        }

        inline unsigned int GetFaceIndexFor(unsigned int item) const
        {
            return item % 6;
        }

        // Mipmap information.
        inline unsigned int GetOffsetFor(unsigned int cube, unsigned int face, unsigned int level) const
        {
            return TextureArray::GetOffsetFor(GetItemIndexFor(cube, face), level);
        }

        inline char const* GetDataFor(unsigned int cube, unsigned int face, unsigned int level) const
        {
            return TextureArray::GetDataFor(GetItemIndexFor(cube, face), level);
        }

        inline char* GetDataFor(unsigned int cube, unsigned int face, unsigned int level)
        {
            return TextureArray::GetDataFor(GetItemIndexFor(cube, face), level);
        }

        template <typename T>
        inline T const* GetFor(unsigned int cube, unsigned int face, unsigned int level) const
        {
            return TextureArray::GetFor<T>(GetItemIndexFor(cube, face), level);
        }

        template <typename T>
        inline T* GetFor(unsigned int cube, unsigned int face, unsigned int level)
        {
            return TextureArray::GetFor<T>(GetItemIndexFor(cube, face), level);
        }

        // Subresource indexing:  index = numLevels*item + level
        // where item = cube*6 + face
        inline unsigned int GetIndex(unsigned int cube, unsigned int face, unsigned int level) const
        {
            return mNumLevels * (6 * cube + face) + level;
        }

    private:
        unsigned int mNumCubes;
    };
}
