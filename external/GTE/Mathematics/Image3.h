// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Image.h>
#include <array>
#include <string>

//#define GTE_THROW_ON_IMAGE3_ERRORS

namespace gte
{
    template <typename PixelType>
    class Image3 : public Image<PixelType>
    {
    public:
        // Construction and destruction.  The last constructor must have
        // positive dimensions; otherwise, the image is empty.
        virtual ~Image3()
        {
        }

        Image3()
        {
        }

        Image3(int dimension0, int dimension1, int dimension2)
            :
            Image<PixelType>(std::vector<int>{ dimension0, dimension1, dimension2 })
        {
        }

        // Support for copy semantics.
        Image3(Image3 const& image)
            :
            Image<PixelType>(image)
        {
        }

        Image3& operator= (Image3 const& image)
        {
            Image<PixelType>::operator=(image);
            return *this;
        }

        // Support for move semantics.
        Image3(Image3&& image)
        {
            *this = std::move(image);
        }

        Image3& operator= (Image3&& image)
        {
            Image<PixelType>::operator=(image);
            return *this;
        }

        // Support for changing the image dimensions.  All pixel data is lost
        // by this operation.
        void Reconstruct(int dimension0, int dimension1, int dimension2)
        {
            Image<PixelType>::Reconstruct(std::vector<int>{ dimension0, dimension1, dimension2 });
        }

        // Conversion between 1-dimensional indices and 3-dimensional
        // coordinates.
        inline size_t GetIndex(int x, int y, int z) const
        {
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (0 <= x && x < this->mDimensions[0]
                && 0 <= y && y < this->mDimensions[1]
                && 0 <= z && z < this->mDimensions[2])
            {
                return static_cast<size_t>(x) +
                    static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                        static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(x) + "," +
                    std::to_string(y) + "," + std::to_string(z) + ").");
            }
#else
            return static_cast<size_t>(x) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                    static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
#endif
        }

        inline size_t GetIndex(std::array<int, 3> const& coord) const
        {
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (0 <= coord[0] && coord[0] < this->mDimensions[0]
                && 0 <= coord[1] && coord[1] < this->mDimensions[1]
                && 0 <= coord[2] && coord[2] < this->mDimensions[2])
            {
                return static_cast<size_t>(coord[0]) +
                    static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                        static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(coord[2]));
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(coord[0]) + "," +
                    std::to_string(coord[1]) + "," + std::to_string(coord[2]) + ").");
            }
#else
            return static_cast<size_t>(coord[0]) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                    static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(coord[2]));
#endif
        }

        inline void GetCoordinates(size_t index, int& x, int& y, int& z) const
        {
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (index < this->mPixels.size())
            {
                x = static_cast<int>(index % this->mDimensions[0]);
                index /= this->mDimensions[0];
                y = static_cast<int>(index % this->mDimensions[1]);
                z = static_cast<int>(index / this->mDimensions[1]);
            }
            else
            {
                LogError(
                    "Invalid index " + std::to_string(index) + ".");
            }
#else
            x = static_cast<int>(index % this->mDimensions[0]);
            index /= this->mDimensions[0];
            y = static_cast<int>(index % this->mDimensions[1]);
            z = static_cast<int>(index / this->mDimensions[1]);
#endif
        }

        inline std::array<int, 3> GetCoordinates(size_t index) const
        {
            std::array<int, 3> coord;
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (index < this->mPixels.size())
            {
                coord[0] = static_cast<int>(index % this->mDimensions[0]);
                index /= this->mDimensions[0];
                coord[1] = static_cast<int>(index % this->mDimensions[1]);
                coord[2] = static_cast<int>(index / this->mDimensions[1]);
                return coord;
            }
            else
            {
                LogError(
                    "Invalid index " + std::to_string(index) + ".");
            }
#else
            coord[0] = static_cast<int>(index % this->mDimensions[0]);
            index /= this->mDimensions[0];
            coord[1] = static_cast<int>(index % this->mDimensions[1]);
            coord[2] = static_cast<int>(index / this->mDimensions[1]);
            return coord;
#endif
        }

        // Access the data as a 3-dimensional array.  The operator() functions
        // test for valid (x,y,z) when iterator checking is enabled and throw
        // on invalid (x,y,z).  The Get() functions test for valid (x,y,z) and
        // clamp when invalid; these functions cannot fail.
        inline PixelType& operator() (int x, int y, int z)
        {
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (0 <= x && x < this->mDimensions[0]
                && 0 <= y && y < this->mDimensions[1]
                && 0 <= z && z < this->mDimensions[2])
            {
                size_t i = static_cast<size_t>(x) +
                    static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                        static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
                return this->mPixels[i];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(x) + "," +
                    std::to_string(y) + "," + std::to_string(z) + ").");
            }
#else
            size_t i = static_cast<size_t>(x) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                    static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
            return this->mPixels[i];
#endif
        }

        inline PixelType const& operator() (int x, int y, int z) const
        {
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (0 <= x && x < this->mDimensions[0]
                && 0 <= y && y < this->mDimensions[1]
                && 0 <= z && z < this->mDimensions[2])
            {
                size_t i = static_cast<size_t>(x) +
                    static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                        static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
                return this->mPixels[i];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(x) + "," +
                    std::to_string(y) + "," + std::to_string(z) + ").");
            }
#else
            size_t i = static_cast<size_t>(x) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                    static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
            return this->mPixels[i];
#endif
        }

        inline PixelType& operator() (std::array<int, 3> const& coord)
        {
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (0 <= coord[0] && coord[0] < this->mDimensions[0]
                && 0 <= coord[1] && coord[1] < this->mDimensions[1]
                && 0 <= coord[2] && coord[2] < this->mDimensions[2])
            {
                size_t i = static_cast<size_t>(coord[0]) +
                    static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                        static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(coord[2]));
                return this->mPixels[i];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(coord[0]) + "," +
                    std::to_string(coord[1]) + "," + std::to_string(coord[2]) + ").");
            }
#else
            size_t i = static_cast<size_t>(coord[0]) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                    static_cast<size_t>(this->mDimensions[1] * coord[2]));
            return this->mPixels[i];
#endif
        }

        inline PixelType const& operator() (std::array<int, 3> const& coord) const
        {
#if defined(GTE_THROW_ON_IMAGE3_ERRORS)
            if (0 <= coord[0] && coord[0] < this->mDimensions[0]
                && 0 <= coord[1] && coord[1] < this->mDimensions[1]
                && 0 <= coord[2] && coord[2] < this->mDimensions[2])
            {
                size_t i = static_cast<size_t>(coord[0]) +
                    static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                        static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(coord[2]));
                return this->mPixels[i];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(coord[0]) + "," +
                    std::to_string(coord[1]) + "," + std::to_string(coord[2]) + ").");
            }
#else
            size_t i = static_cast<size_t>(coord[0]) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                    static_cast<size_t>(this->mDimensions[1] * coord[2]));
            return this->mPixels[i];
#endif
        }

        inline PixelType& Get(int x, int y, int z)
        {
            // Clamp to valid (x,y,z).
            if (x < 0)
            {
                x = 0;
            }
            else if (x >= this->mDimensions[0])
            {
                x = this->mDimensions[0] - 1;
            }

            if (y < 0)
            {
                y = 0;
            }
            else if (y >= this->mDimensions[1])
            {
                y = this->mDimensions[1] - 1;
            }

            if (z < 0)
            {
                z = 0;
            }
            else if (z >= this->mDimensions[2])
            {
                z = this->mDimensions[2] - 1;
            }

            size_t i = static_cast<size_t>(x) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                    static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
            return this->mPixels[i];
        }

        inline PixelType const& Get(int x, int y, int z) const
        {
            // Clamp to valid (x,y,z).
            if (x < 0)
            {
                x = 0;
            }
            else if (x >= this->mDimensions[0])
            {
                x = this->mDimensions[0] - 1;
            }

            if (y < 0)
            {
                y = 0;
            }
            else if (y >= this->mDimensions[1])
            {
                y = this->mDimensions[1] - 1;
            }

            if (z < 0)
            {
                z = 0;
            }
            else if (z >= this->mDimensions[2])
            {
                z = this->mDimensions[2] - 1;
            }

            size_t i = static_cast<size_t>(x) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(y) +
                    static_cast<size_t>(this->mDimensions[1]) * static_cast<size_t>(z));
            return this->mPixels[i];
        }

        inline PixelType& Get(std::array<int, 3> coord)
        {
            // Clamp to valid (x,y,z).
            for (int d = 0; d < 3; ++d)
            {
                if (coord[d] < 0)
                {
                    coord[d] = 0;
                }
                else if (coord[d] >= this->mDimensions[d])
                {
                    coord[d] = this->mDimensions[d] - 1;
                }
            }

            size_t i = static_cast<size_t>(coord[0]) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                    static_cast<size_t>(this->mDimensions[1] * coord[2]));
            return this->mPixels[i];
        }

        inline PixelType const& Get(std::array<int, 3> coord) const
        {
            // Clamp to valid (x,y,z).
            for (int d = 0; d < 3; ++d)
            {
                if (coord[d] < 0)
                {
                    coord[d] = 0;
                }
                else if (coord[d] >= this->mDimensions[d])
                {
                    coord[d] = this->mDimensions[d] - 1;
                }
            }

            size_t i = static_cast<size_t>(coord[0]) +
                static_cast<size_t>(this->mDimensions[0]) * (static_cast<size_t>(coord[1]) +
                    static_cast<size_t>(this->mDimensions[1] * coord[2]));
            return this->mPixels[i];
        }

        // In the following discussion, u, v and w are in {-1,1}.  Given a
        // voxel (x,y,z), the 6-connected neighbors have relative offsets
        // (u,0,0), (0,v,0), and (0,0,w).  The 18-connected neighbors include
        // the 6-connected neighbors and have additional relative offsets
        // (u,v,0), (u,0,w), and (0,v,w).  The 26-connected neighbors include
        // the 18-connected neighbors and have additional relative offsets
        // (u,v,w).  The corner neighbors have offsets (0,0,0), (1,0,0),
        // (0,1,0), (1,1,0), (0,0,1), (1,0,1), (0,1,1), and (1,1,1) in that
        // order.  The full neighborhood is the set of 3x3x3 pixels centered
        // at (x,y).

        // The neighborhoods can be accessed as 1-dimensional indices using
        // these functions.  The first five functions provide 1-dimensional
        // indices relative to any voxel location; these depend only on the
        // image dimensions.  The last five functions provide 1-dimensional
        // indices for the actual voxels in the neighborhood; no clamping is
        // used when (x,y,z) is on the boundary.
        void GetNeighborhood(std::array<int, 6>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            int dim01 = this->mDimensions[0] * this->mDimensions[1];
            nbr[0] = -1;        // (x-1,y,z)
            nbr[1] = +1;        // (x+1,y,z)
            nbr[2] = -dim0;     // (x,y-1,z)
            nbr[3] = +dim0;     // (x,y+1,z)
            nbr[4] = -dim01;    // (x,y,z-1)
            nbr[5] = +dim01;    // (x,y,z+1)
        }

        void GetNeighborhood(std::array<int, 18>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            int dim01 = this->mDimensions[0] * this->mDimensions[1];
            nbr[0] = -1;                // (x-1,y,z)
            nbr[1] = +1;                // (x+1,y,z)
            nbr[2] = -dim0;             // (x,y-1,z)
            nbr[3] = +dim0;             // (x,y+1,z)
            nbr[4] = -dim01;            // (x,y,z-1)
            nbr[5] = +dim01;            // (x,y,z+1)
            nbr[6] = -1 - dim0;         // (x-1,y-1,z)
            nbr[7] = +1 - dim0;         // (x+1,y-1,z)
            nbr[8] = -1 + dim0;         // (x-1,y+1,z)
            nbr[9] = +1 + dim0;         // (x+1,y+1,z)
            nbr[10] = -1 + dim01;       // (x-1,y,z+1)
            nbr[11] = +1 + dim01;       // (x+1,y,z+1)
            nbr[12] = -dim0 + dim01;    // (x,y-1,z+1)
            nbr[13] = +dim0 + dim01;    // (x,y+1,z+1)
            nbr[14] = -1 - dim01;       // (x-1,y,z-1)
            nbr[15] = +1 - dim01;       // (x+1,y,z-1)
            nbr[16] = -dim0 - dim01;    // (x,y-1,z-1)
            nbr[17] = +dim0 - dim01;    // (x,y+1,z-1)
        }

        void GetNeighborhood(std::array<int, 26>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            int dim01 = this->mDimensions[0] * this->mDimensions[1];
            nbr[0] = -1;                    // (x-1,y,z)
            nbr[1] = +1;                    // (x+1,y,z)
            nbr[2] = -dim0;                 // (x,y-1,z)
            nbr[3] = +dim0;                 // (x,y+1,z)
            nbr[4] = -dim01;                // (x,y,z-1)
            nbr[5] = +dim01;                // (x,y,z+1)
            nbr[6] = -1 - dim0;             // (x-1,y-1,z)
            nbr[7] = +1 - dim0;             // (x+1,y-1,z)
            nbr[8] = -1 + dim0;             // (x-1,y+1,z)
            nbr[9] = +1 + dim0;             // (x+1,y+1,z)
            nbr[10] = -1 + dim01;           // (x-1,y,z+1)
            nbr[11] = +1 + dim01;           // (x+1,y,z+1)
            nbr[12] = -dim0 + dim01;        // (x,y-1,z+1)
            nbr[13] = +dim0 + dim01;        // (x,y+1,z+1)
            nbr[14] = -1 - dim01;           // (x-1,y,z-1)
            nbr[15] = +1 - dim01;           // (x+1,y,z-1)
            nbr[16] = -dim0 - dim01;        // (x,y-1,z-1)
            nbr[17] = +dim0 - dim01;        // (x,y+1,z-1)
            nbr[18] = -1 - dim0 - dim01;    // (x-1,y-1,z-1)
            nbr[19] = +1 - dim0 - dim01;    // (x+1,y-1,z-1)
            nbr[20] = -1 + dim0 - dim01;    // (x-1,y+1,z-1)
            nbr[21] = +1 + dim0 - dim01;    // (x+1,y+1,z-1)
            nbr[22] = -1 - dim0 + dim01;    // (x-1,y-1,z+1)
            nbr[23] = +1 - dim0 + dim01;    // (x+1,y-1,z+1)
            nbr[24] = -1 + dim0 + dim01;    // (x-1,y+1,z+1)
            nbr[25] = +1 + dim0 + dim01;    // (x+1,y+1,z+1)
        }

        void GetCorners(std::array<int, 8>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            int dim01 = this->mDimensions[0] * this->mDimensions[1];
            nbr[0] = 0;                 // (x,y,z)
            nbr[1] = 1;                 // (x+1,y,z)
            nbr[2] = dim0;              // (x,y+1,z)
            nbr[3] = dim0 + 1;          // (x+1,y+1,z)
            nbr[4] = dim01;             // (x,y,z+1)
            nbr[5] = dim01 + 1;         // (x+1,y,z+1)
            nbr[6] = dim01 + dim0;      // (x,y+1,z+1)
            nbr[7] = dim01 + dim0 + 1;  // (x+1,y+1,z+1)
        }

        void GetFull(std::array<int, 27>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            int dim01 = this->mDimensions[0] * this->mDimensions[1];
            nbr[0] = -1 - dim0 - dim01;     // (x-1,y-1,z-1)
            nbr[1] = -dim0 - dim01;         // (x,  y-1,z-1)
            nbr[2] = +1 - dim0 - dim01;     // (x+1,y-1,z-1)
            nbr[3] = -1 - dim01;            // (x-1,y,  z-1)
            nbr[4] = -dim01;                // (x,  y,  z-1)
            nbr[5] = +1 - dim01;            // (x+1,y,  z-1)
            nbr[6] = -1 + dim0 - dim01;     // (x-1,y+1,z-1)
            nbr[7] = +dim0 - dim01;         // (x,  y+1,z-1)
            nbr[8] = +1 + dim0 - dim01;     // (x+1,y+1,z-1)
            nbr[9] = -1 - dim0;             // (x-1,y-1,z)
            nbr[10] = -dim0;                // (x,  y-1,z)
            nbr[11] = +1 - dim0;            // (x+1,y-1,z)
            nbr[12] = -1;                   // (x-1,y,  z)
            nbr[13] = 0;                    // (x,  y,  z)
            nbr[14] = +1;                   // (x+1,y,  z)
            nbr[15] = -1 + dim0;            // (x-1,y+1,z)
            nbr[16] = +dim0;                // (x,  y+1,z)
            nbr[17] = +1 + dim0;            // (x+1,y+1,z)
            nbr[18] = -1 - dim0 + dim01;    // (x-1,y-1,z+1)
            nbr[19] = -dim0 + dim01;        // (x,  y-1,z+1)
            nbr[20] = +1 - dim0 + dim01;    // (x+1,y-1,z+1)
            nbr[21] = -1 + dim01;           // (x-1,y,  z+1)
            nbr[22] = +dim01;               // (x,  y,  z+1)
            nbr[23] = +1 + dim01;           // (x+1,y,  z+1)
            nbr[24] = -1 + dim0 + dim01;    // (x-1,y+1,z+1)
            nbr[25] = +dim0 + dim01;        // (x,  y+1,z+1)
            nbr[26] = +1 + dim0 + dim01;    // (x+1,y+1,z+1)
        }

        void GetNeighborhood(int x, int y, int z, std::array<size_t, 6>& nbr) const
        {
            size_t index = GetIndex(x, y, z);
            std::array<int, 6> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 6; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        void GetNeighborhood(int x, int y, int z, std::array<size_t, 18>& nbr) const
        {
            size_t index = GetIndex(x, y, z);
            std::array<int, 18> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 18; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        void GetNeighborhood(int x, int y, int z, std::array<size_t, 26>& nbr) const
        {
            size_t index = GetIndex(x, y, z);
            std::array<int, 26> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 26; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        void GetCorners(int x, int y, int z, std::array<size_t, 8>& nbr) const
        {
            size_t index = GetIndex(x, y, z);
            std::array<int, 8> inbr;
            GetCorners(inbr);
            for (int i = 0; i < 8; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        void GetFull(int x, int y, int z, std::array<size_t, 27>& nbr) const
        {
            size_t index = GetIndex(x, y, z);
            std::array<int, 27> inbr;
            GetFull(inbr);
            for (int i = 0; i < 27; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        // The neighborhoods can be accessed as 3-tuples using these
        // functions.  The first five functions provide 3-tuples relative to
        // any voxel location; these depend only on the image dimensions.  The
        // last five functions provide 3-tuples for the actual voxels in the
        // neighborhood; no clamping is used when (x,y,z) is on the boundary.
        void GetNeighborhood(std::array<std::array<int, 3>, 6>& nbr) const
        {
            nbr[0] = { { -1, 0, 0 } };
            nbr[1] = { { +1, 0, 0 } };
            nbr[2] = { { 0, -1, 0 } };
            nbr[3] = { { 0, +1, 0 } };
            nbr[4] = { { 0, 0, -1 } };
            nbr[5] = { { 0, 0, +1 } };
        }

        void GetNeighborhood(std::array<std::array<int, 3>, 18>& nbr) const
        {
            nbr[0] = { { -1, 0, 0 } };
            nbr[1] = { { +1, 0, 0 } };
            nbr[2] = { { 0, -1, 0 } };
            nbr[3] = { { 0, +1, 0 } };
            nbr[4] = { { 0, 0, -1 } };
            nbr[5] = { { 0, 0, +1 } };
            nbr[6] = { { -1, -1, 0 } };
            nbr[7] = { { +1, -1, 0 } };
            nbr[8] = { { -1, +1, 0 } };
            nbr[9] = { { +1, +1, 0 } };
            nbr[10] = { { -1, 0, +1 } };
            nbr[11] = { { +1, 0, +1 } };
            nbr[12] = { { 0, -1, +1 } };
            nbr[13] = { { 0, +1, +1 } };
            nbr[14] = { { -1, 0, -1 } };
            nbr[15] = { { +1, 0, -1 } };
            nbr[16] = { { 0, -1, -1 } };
            nbr[17] = { { 0, +1, -1 } };
        }

        void GetNeighborhood(std::array<std::array<int, 3>, 26>& nbr) const
        {
            nbr[0] = { { -1, 0, 0 } };
            nbr[1] = { { +1, 0, 0 } };
            nbr[2] = { { 0, -1, 0 } };
            nbr[3] = { { 0, +1, 0 } };
            nbr[4] = { { 0, 0, -1 } };
            nbr[5] = { { 0, 0, +1 } };
            nbr[6] = { { -1, -1, 0 } };
            nbr[7] = { { +1, -1, 0 } };
            nbr[8] = { { -1, +1, 0 } };
            nbr[9] = { { +1, +1, 0 } };
            nbr[10] = { { -1, 0, +1 } };
            nbr[11] = { { +1, 0, +1 } };
            nbr[12] = { { 0, -1, +1 } };
            nbr[13] = { { 0, +1, +1 } };
            nbr[14] = { { -1, 0, -1 } };
            nbr[15] = { { +1, 0, -1 } };
            nbr[16] = { { 0, -1, -1 } };
            nbr[17] = { { 0, +1, -1 } };
            nbr[18] = { { -1, -1, -1 } };
            nbr[19] = { { +1, -1, -1 } };
            nbr[20] = { { -1, +1, -1 } };
            nbr[21] = { { +1, +1, -1 } };
            nbr[22] = { { -1, -1, +1 } };
            nbr[23] = { { +1, -1, +1 } };
            nbr[24] = { { -1, +1, +1 } };
            nbr[25] = { { +1, +1, +1 } };
        }

        void GetCorners(std::array<std::array<int, 3>, 8>& nbr) const
        {
            nbr[0] = { { 0, 0, 0 } };
            nbr[1] = { { 1, 0, 0 } };
            nbr[2] = { { 0, 1, 0 } };
            nbr[3] = { { 1, 1, 0 } };
            nbr[4] = { { 0, 0, 1 } };
            nbr[5] = { { 1, 0, 1 } };
            nbr[6] = { { 0, 1, 1 } };
            nbr[7] = { { 1, 1, 1 } };
        }

        void GetFull(std::array<std::array<int, 3>, 27>& nbr) const
        {
            nbr[0] = { { -1, -1, -1 } };
            nbr[1] = { { 0, -1, -1 } };
            nbr[2] = { { +1, -1, -1 } };
            nbr[3] = { { -1, 0, -1 } };
            nbr[4] = { { 0, 0, -1 } };
            nbr[5] = { { +1, 0, -1 } };
            nbr[6] = { { -1, +1, -1 } };
            nbr[7] = { { 0, +1, -1 } };
            nbr[8] = { { +1, +1, -1 } };
            nbr[9] = { { -1, -1, 0 } };
            nbr[10] = { { 0, -1, 0 } };
            nbr[11] = { { +1, -1, 0 } };
            nbr[12] = { { -1, 0, 0 } };
            nbr[13] = { { 0, 0, 0 } };
            nbr[14] = { { +1, 0, 0 } };
            nbr[15] = { { -1, +1, 0 } };
            nbr[16] = { { 0, +1, 0 } };
            nbr[17] = { { +1, +1, 0 } };
            nbr[18] = { { -1, -1, +1 } };
            nbr[19] = { { 0, -1, +1 } };
            nbr[20] = { { +1, -1, +1 } };
            nbr[21] = { { -1, 0, +1 } };
            nbr[22] = { { 0, 0, +1 } };
            nbr[23] = { { +1, 0, +1 } };
            nbr[24] = { { -1, +1, +1 } };
            nbr[25] = { { 0, +1, +1 } };
            nbr[26] = { { +1, +1, +1 } };
        }

        void GetNeighborhood(int x, int y, int z, std::array<std::array<size_t, 3>, 6>& nbr) const
        {
            std::array<std::array<int, 3>, 6> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 6; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
                nbr[i][2] = static_cast<size_t>(z) + inbr[i][2];
            }
        }

        void GetNeighborhood(int x, int y, int z, std::array<std::array<size_t, 3>, 18>& nbr) const
        {
            std::array<std::array<int, 3>, 18> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 18; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
                nbr[i][2] = static_cast<size_t>(z) + inbr[i][2];
            }
        }

        void GetNeighborhood(int x, int y, int z, std::array<std::array<size_t, 3>, 26>& nbr) const
        {
            std::array<std::array<int, 3>, 26> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 26; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
                nbr[i][2] = static_cast<size_t>(z) + inbr[i][2];
            }
        }

        void GetCorners(int x, int y, int z, std::array<std::array<size_t, 3>, 8>& nbr) const
        {
            std::array<std::array<int, 3>, 8> inbr;
            GetCorners(inbr);
            for (int i = 0; i < 8; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
                nbr[i][2] = static_cast<size_t>(z) + inbr[i][2];
            }
        }

        void GetFull(int x, int y, int z, std::array<std::array<size_t, 3>, 27>& nbr) const
        {
            std::array<std::array<int, 3>, 27> inbr;
            GetFull(inbr);
            for (int i = 0; i < 27; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
                nbr[i][2] = static_cast<size_t>(z) + inbr[i][2];
            }
        }
    };
}
