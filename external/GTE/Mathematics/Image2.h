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

//#define GTE_THROW_ON_IMAGE2_ERRORS

namespace gte
{
    template <typename PixelType>
    class Image2 : public Image<PixelType>
    {
    public:
        // Construction and destruction.  The last constructor must have
        // positive dimensions; otherwise, the image is empty.
        virtual ~Image2()
        {
        }

        Image2()
        {
        }

        Image2(int dimension0, int dimension1)
            :
            Image<PixelType>(std::vector<int>{ dimension0, dimension1 })
        {
        }

        // Support for copy semantics.
        Image2(Image2 const& image)
            :
            Image<PixelType>(image)
        {
        }

        Image2& operator=(Image2 const& image)
        {
            Image<PixelType>::operator=(image);
            return *this;
        }

        // Support for move semantics.
        Image2(Image2&& image)
        {
            *this = std::move(image);
        }

        Image2& operator= (Image2&& image)
        {
            Image<PixelType>::operator=(image);
            return *this;
        }

        // Support for changing the image dimensions.  All pixel data is lost
        // by this operation.
        void Reconstruct(int dimension0, int dimension1)
        {
            Image<PixelType>::Reconstruct(std::vector<int>{ dimension0, dimension1 });
        }

        // Conversion between 1-dimensional indices and 2-dimensional
        // coordinates.
        inline size_t GetIndex(int x, int y) const
        {
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (0 <= x && x < this->mDimensions[0]
                && 0 <= y && y < this->mDimensions[1])
            {
                return static_cast<size_t>(x) +
                    static_cast<size_t>(this->mDimensions[0]) * static_cast<size_t>(y);
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(x) + "," +
                    std::to_string(y) + ").");
            }
#else
            return static_cast<size_t>(x) +
                static_cast<size_t>(this->mDimensions[0]) * static_cast<size_t>(y);
#endif
        }

        inline size_t GetIndex(std::array<int, 2> const& coord) const
        {
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (0 <= coord[0] && coord[0] < this->mDimensions[0]
                && 0 <= coord[1] && coord[1] < this->mDimensions[1])
            {
                return static_cast<size_t>(coord[0]) +
                    static_cast<size_t>(this->mDimensions[0]) * static_cast<size_t>(coord[1]);
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(coord[0]) + "," +
                    std::to_string(coord[1]) + ").");
            }
#else
            return static_cast<size_t>(coord[0]) +
                static_cast<size_t>(this->mDimensions[0]) * static_cast<size_t>(coord[1]);
#endif
        }

        inline void GetCoordinates(size_t index, int& x, int& y) const
        {
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (index < this->mPixels.size())
            {
                x = static_cast<int>(index % this->mDimensions[0]);
                y = static_cast<int>(index / this->mDimensions[0]);
            }
            else
            {
                LogError(
                    "Invalid index " + std::to_string(index) + ".");
            }
#else
            x = static_cast<int>(index % this->mDimensions[0]);
            y = static_cast<int>(index / this->mDimensions[0]);
#endif
        }

        inline std::array<int, 2> GetCoordinates(size_t index) const
        {
            std::array<int, 2> coord;
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (index < this->mPixels.size())
            {
                coord[0] = static_cast<int>(index % this->mDimensions[0]);
                coord[1] = static_cast<int>(index / this->mDimensions[0]);
                return coord;
            }
            else
            {
                LogError(
                    "Invalid index " + std::to_string(index) + ".");
            }
#else
            coord[0] = static_cast<int>(index % this->mDimensions[0]);
            coord[1] = static_cast<int>(index / this->mDimensions[0]);
            return coord;
#endif
        }

        // Access the data as a 2-dimensional array.  The operator() functions
        // test for valid (x,y) when iterator checking is enabled and throw
        // on invalid (x,y).  The Get() functions test for valid (x,y) and
        // clamp when invalid; these functions cannot fail.
        inline PixelType& operator() (int x, int y)
        {
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (0 <= x && x < this->mDimensions[0]
                && 0 <= y && y < this->mDimensions[1])
            {
                return this->mPixels[x + this->mDimensions[0] * y];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(x) + "," +
                    std::to_string(y) + ").");
            }
#else
            return this->mPixels[x + this->mDimensions[0] * y];
#endif
        }

        inline PixelType const& operator() (int x, int y) const
        {
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (0 <= x && x < this->mDimensions[0]
                && 0 <= y && y < this->mDimensions[1])
            {
                return this->mPixels[x + this->mDimensions[0] * y];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(x) + "," +
                    std::to_string(y) + ").");
            }
#else
            return this->mPixels[x + this->mDimensions[0] * y];
#endif
        }

        inline PixelType& operator() (std::array<int, 2> const& coord)
        {
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (0 <= coord[0] && coord[0] < this->mDimensions[0]
                && 0 <= coord[1] && coord[1] < this->mDimensions[1])
            {
                return this->mPixels[coord[0] + this->mDimensions[0] * coord[1]];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(coord[0]) + "," +
                    std::to_string(coord[1]) + ").");
            }
#else
            return this->mPixels[coord[0] + this->mDimensions[0] * coord[1]];
#endif
        }

        inline PixelType const& operator() (std::array<int, 2> const& coord) const
        {
#if defined(GTE_THROW_ON_IMAGE2_ERRORS)
            if (0 <= coord[0] && coord[0] < this->mDimensions[0]
                && 0 <= coord[1] && coord[1] < this->mDimensions[1])
            {
                return this->mPixels[coord[0] + this->mDimensions[0] * coord[1]];
            }
            else
            {
                LogError(
                    "Invalid coordinates (" + std::to_string(coord[0]) + "," +
                    std::to_string(coord[1]) + ").");
            }
#else
            return this->mPixels[coord[0] + this->mDimensions[0] * coord[1]];
#endif
        }

        inline PixelType& Get(int x, int y)
        {
            // Clamp to valid (x,y).
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

            return this->mPixels[x + this->mDimensions[0] * y];
        }

        inline PixelType const& Get(int x, int y) const
        {
            // Clamp to valid (x,y).
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

            return this->mPixels[x + this->mDimensions[0] * y];
        }

        inline PixelType& Get(std::array<int, 2> coord)
        {
            // Clamp to valid (x,y).
            for (int i = 0; i < 2; ++i)
            {
                if (coord[i] < 0)
                {
                    coord[i] = 0;
                }
                else if (coord[i] >= this->mDimensions[i])
                {
                    coord[i] = this->mDimensions[i] - 1;
                }
            }

            return this->mPixels[coord[0] + this->mDimensions[0] * coord[1]];
        }

        inline PixelType const& Get(std::array<int, 2> coord) const
        {
            // Clamp to valid (x,y).
            for (int i = 0; i < 2; ++i)
            {
                if (coord[i] < 0)
                {
                    coord[i] = 0;
                }
                else if (coord[i] >= this->mDimensions[i])
                {
                    coord[i] = this->mDimensions[i] - 1;
                }
            }

            return this->mPixels[coord[0] + this->mDimensions[0] * coord[1]];
        }

        // In the following discussion, u and v are in {-1,1}.  Given a pixel
        // (x,y), the 4-connected neighbors have relative offsets (u,0) and
        // (0,v).  The 8-connected neighbors include the 4-connected neighbors
        // and have additional relative offsets (u,v).  The corner neighbors
        // have relative offsets (0,0), (1,0), (0,1), and (1,1) in that order.
        // The full neighborhood is the set of 3x3 pixels centered at (x,y).

        // The neighborhoods can be accessed as 1-dimensional indices using
        // these functions.  The first four functions provide 1-dimensional
        // indices relative to any pixel location; these depend only on the
        // image dimensions.  The last four functions provide 1-dimensional
        // indices for the actual pixels in the neighborhood; no clamping is
        // used when (x,y) is on the boundary.
        void GetNeighborhood(std::array<int, 4>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            nbr[0] = -1;        // (x-1,y)
            nbr[1] = +1;        // (x+1,y)
            nbr[2] = -dim0;     // (x,y-1)
            nbr[3] = +dim0;     // (x,y+1)
        }

        void GetNeighborhood(std::array<int, 8>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            nbr[0] = -1;            // (x-1,y)
            nbr[1] = +1;            // (x+1,y)
            nbr[2] = -dim0;         // (x,y-1)
            nbr[3] = +dim0;         // (x,y+1)
            nbr[4] = -1 - dim0;     // (x-1,y-1)
            nbr[5] = +1 - dim0;     // (x+1,y-1)
            nbr[6] = -1 + dim0;     // (x-1,y+1)
            nbr[7] = +1 + dim0;     // (x+1,y+1)
        }

        void GetCorners(std::array<int, 4>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            nbr[0] = 0;         // (x,y)
            nbr[1] = 1;         // (x+1,y)
            nbr[2] = dim0;      // (x,y+1)
            nbr[3] = dim0 + 1;  // (x+1,y+1)
        }

        void GetFull(std::array<int, 9>& nbr) const
        {
            int dim0 = this->mDimensions[0];
            nbr[0] = -1 - dim0;     // (x-1,y-1)
            nbr[1] = -dim0;         // (x,y-1)
            nbr[2] = +1 - dim0;     // (x+1,y-1)
            nbr[3] = -1;            // (x-1,y)
            nbr[4] = 0;             // (x,y)
            nbr[5] = +1;            // (x+1,y)
            nbr[6] = -1 + dim0;     // (x-1,y+1)
            nbr[7] = +dim0;         // (x,y+1)
            nbr[8] = +1 + dim0;     // (x+1,y+1)
        }

        void GetNeighborhood(int x, int y, std::array<size_t, 4>& nbr) const
        {
            size_t index = GetIndex(x, y);
            std::array<int, 4> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 4; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        void GetNeighborhood(int x, int y, std::array<size_t, 8>& nbr) const
        {
            size_t index = GetIndex(x, y);
            std::array<int, 8> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 8; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        void GetCorners(int x, int y, std::array<size_t, 4>& nbr) const
        {
            size_t index = GetIndex(x, y);
            std::array<int, 4> inbr;
            GetCorners(inbr);
            for (int i = 0; i < 4; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        void GetFull(int x, int y, std::array<size_t, 9>& nbr) const
        {
            size_t index = GetIndex(x, y);
            std::array<int, 9> inbr;
            GetFull(inbr);
            for (int i = 0; i < 9; ++i)
            {
                nbr[i] = index + inbr[i];
            }
        }

        // The neighborhoods can be accessed as 2-tuples using these
        // functions.  The first four functions provide 2-tuples relative to
        // any pixel location; these depend only on the image dimensions.  The
        // last four functions provide 2-tuples for the actual pixels in the
        // neighborhood; no clamping is used when (x,y) is on the boundary.
        void GetNeighborhood(std::array<std::array<int, 2>, 4>& nbr) const
        {
            nbr[0] = { { -1, 0 } };
            nbr[1] = { { +1, 0 } };
            nbr[2] = { { 0, -1 } };
            nbr[3] = { { 0, +1 } };
        }

        void GetNeighborhood(std::array<std::array<int, 2>, 8>& nbr) const
        {
            nbr[0] = { { -1, -1 } };
            nbr[1] = { { 0, -1 } };
            nbr[2] = { { +1, -1 } };
            nbr[3] = { { -1, 0 } };
            nbr[4] = { { +1, 0 } };
            nbr[5] = { { -1, +1 } };
            nbr[6] = { { 0, +1 } };
            nbr[7] = { { +1, +1 } };
        }

        void GetCorners(std::array<std::array<int, 2>, 4>& nbr) const
        {
            nbr[0] = { { 0, 0 } };
            nbr[1] = { { 1, 0 } };
            nbr[2] = { { 0, 1 } };
            nbr[3] = { { 1, 1 } };
        }

        void GetFull(std::array<std::array<int, 2>, 9>& nbr) const
        {
            nbr[0] = { { -1, -1 } };
            nbr[1] = { { 0, -1 } };
            nbr[2] = { { +1, -1 } };
            nbr[3] = { { -1, 0 } };
            nbr[4] = { { 0, 0 } };
            nbr[5] = { { +1, 0 } };
            nbr[6] = { { -1, +1 } };
            nbr[7] = { { 0, +1 } };
            nbr[8] = { { +1, +1 } };
        }

        void GetNeighborhood(int x, int y, std::array<std::array<size_t, 2>, 4>& nbr) const
        {
            std::array<std::array<int, 2>, 4> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 4; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
            }
        }

        void GetNeighborhood(int x, int y, std::array<std::array<size_t, 2>, 8>& nbr) const
        {
            std::array<std::array<int, 2>, 8> inbr;
            GetNeighborhood(inbr);
            for (int i = 0; i < 8; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
            }
        }

        void GetCorners(int x, int y, std::array<std::array<size_t, 2>, 4>& nbr) const
        {
            std::array<std::array<int, 2>, 4> inbr;
            GetCorners(inbr);
            for (int i = 0; i < 4; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
            }
        }

        void GetFull(int x, int y, std::array<std::array<size_t, 2>, 9>& nbr) const
        {
            std::array<std::array<int, 2>, 9> inbr;
            GetFull(inbr);
            for (int i = 0; i < 9; ++i)
            {
                nbr[i][0] = static_cast<size_t>(x) + inbr[i][0];
                nbr[i][1] = static_cast<size_t>(y) + inbr[i][1];
            }
        }
    };
}
