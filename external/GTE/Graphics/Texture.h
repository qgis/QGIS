// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DataFormat.h>
#include <Graphics/Resource.h>
#include <array>
#include <functional>
#include <vector>

namespace gte
{
    class Texture : public Resource
    {
    protected:
        // Abstract base class for single textures and for texture arrays.
        // All items in a texture array have the same format, number of
        // dimensions, dimension values and mipmap status.
        Texture(unsigned int numItems, DFType format, unsigned int numDimensions,
            unsigned int dim0, unsigned int dim1, unsigned int dim2,
            bool hasMipmaps, bool createStorage);

    public:
        // Member access.
        inline unsigned int GetNumItems() const
        {
            return mNumItems;
        }

        inline DFType GetFormat() const
        {
            return mFormat;
        }

        inline unsigned int GetNumDimensions() const
        {
            return mNumDimensions;
        }

        inline unsigned int GetDimension(int i) const
        {
            return mLDimension[0][i];
        }

        // Subresource information.
        struct Subresource
        {
            unsigned int item;
            unsigned int level;
            char* data;
            unsigned int rowPitch;
            unsigned int slicePitch;
        };

        // Mipmap information.
        enum { MAX_MIPMAP_LEVELS = 16 };

        inline bool HasMipmaps() const
        {
            return mHasMipmaps;
        }

        inline unsigned int GetNumLevels() const
        {
            return mNumLevels;
        }

        inline unsigned int GetDimensionFor(unsigned int level, int i) const
        {
            return mLDimension[level][i];
        }

        inline unsigned int GetNumElementsFor(unsigned int level) const
        {
            return mLNumBytes[level] / mElementSize;
        }

        inline unsigned int GetNumBytesFor(unsigned int level) const
        {
            return mLNumBytes[level];
        }

        inline unsigned int GetOffsetFor(unsigned int item, unsigned int level) const
        {
            return mLOffset[item][level];
        }

        inline char const* GetDataFor(unsigned int item, unsigned int level) const
        {
            return mData ? (mData + mLOffset[item][level]) : nullptr;
        }

        inline char* GetDataFor(unsigned int item, unsigned int level)
        {
            return mData ? (mData + mLOffset[item][level]) : nullptr;
        }

        template <typename T>
        inline T const* GetFor(unsigned int item, unsigned int level) const
        {
            return reinterpret_cast<T const*>(GetDataFor(item, level));
        }

        template <typename T>
        inline T* GetFor(unsigned int item, unsigned int level)
        {
            return reinterpret_cast<T*>(GetDataFor(item, level));
        }

        // Subresource indexing:  index = numLevels*item + level
        inline unsigned int GetNumSubresources() const
        {
            return mNumItems * mNumLevels;
        }

        unsigned int GetIndex(unsigned int item, unsigned int level) const;

        Subresource GetSubresource(unsigned int index) const;

        // Request that the GPU compute mipmap levels when the base-level texture
        // data is modified.  The AutogenerateMipmaps call should be made before
        // binding the texture to the engine.  If the texture does not have mipmaps,
        // the AutogenerateMipmaps call will not set mAutogenerateMipmaps to true.
        void AutogenerateMipmaps();

        inline bool WantAutogenerateMipmaps() const
        {
            return mAutogenerateMipmaps;
        }

    protected:
        // Support for computing the numElements parameter for the Resource
        // constructor.  This is necessary when mipmaps are requested.
        static unsigned int GetTotalElements(unsigned int numItems,
            unsigned int dim0, unsigned int dim1, unsigned int dim2,
            bool hasMipmaps);

        unsigned int mNumItems;
        DFType mFormat;
        unsigned int mNumDimensions;
        unsigned int mNumLevels;
        std::array<std::array<unsigned int, 3>, MAX_MIPMAP_LEVELS> mLDimension;
        std::array<unsigned int, MAX_MIPMAP_LEVELS> mLNumBytes;
        std::vector<std::array<unsigned int, MAX_MIPMAP_LEVELS>> mLOffset;
        bool mHasMipmaps;
        bool mAutogenerateMipmaps;
    };

    typedef std::function<void(std::shared_ptr<Texture> const&)> TextureUpdater;
    typedef std::function<void(std::shared_ptr<Texture> const&, unsigned int)> TextureLevelUpdater;
}
