// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DisjointIntervals.h>
#include <functional>

namespace gte
{
    // Compute Boolean operations of disjoint sets of half-open rectangles of
    // the form [xmin,xmax)x[ymin,ymax) with xmin < xmax and ymin < ymax.
    template <typename Scalar>
    class DisjointRectangles
    {
    public:
        // Convenient type definition.
        typedef DisjointIntervals<Scalar> ISet;

        // Construction and destruction.  The non-default constructor requires
        // that xmin < xmax and ymin < ymax.
        DisjointRectangles()
            :
            mNumRectangles(0)
        {
        }

        DisjointRectangles(Scalar const& xmin, Scalar const& xmax, Scalar const& ymin, Scalar const& ymax)
        {
            if (xmin < xmax && ymin < ymax)
            {
                mNumRectangles = 1;
                mStrips.push_back(Strip(ymin, ymax, ISet(xmin, xmax)));
            }
            else
            {
                mNumRectangles = 0;
            }
        }

        ~DisjointRectangles()
        {
        }

        // Copy operations.
        DisjointRectangles(DisjointRectangles const& other)
        {
            *this = other;
        }

        DisjointRectangles& operator=(DisjointRectangles const& other)
        {
            mNumRectangles = other.mNumRectangles;
            mStrips = other.mStrips;
            return *this;
        }

        // Move operations.
        DisjointRectangles(DisjointRectangles&& other)
        {
            *this = std::move(other);
        }

        DisjointRectangles& operator=(DisjointRectangles&& other)
        {
            mNumRectangles = other.mNumRectangles;
            mStrips = std::move(other.mStrips);
            return *this;
        }

        // The rectangle set consists of y-strips of interval sets.
        class Strip
        {
        public:
            // Construction and destruction.
            Strip()
                :
                ymin((Scalar)0),
                ymax((Scalar)0)
            {
            }

            Strip(Scalar const& inYMin, Scalar const& inYMax, ISet const& inIntervalSet)
                :
                ymin(inYMin),
                ymax(inYMax),
                intervalSet(inIntervalSet)
            {
            }

            ~Strip()
            {
            }

            // Copy operations.
            Strip(Strip const& other)
            {
                *this = other;
            }

            Strip& operator=(Strip const& other)
            {
                ymin = other.ymin;
                ymax = other.ymax;
                intervalSet = other.intervalSet;
                return *this;
            }

            // Move operations.
            Strip(Strip&& other)
            {
                *this = std::move(other);
            }

            Strip& operator=(Strip&& other)
            {
                ymin = other.ymin;
                ymax = other.ymax;
                intervalSet = std::move(other.intervalSet);
                other.ymin = (Scalar)0;
                other.ymax = (Scalar)0;
                return *this;
            }

            // Member access.
            Scalar ymin, ymax;
            ISet intervalSet;
        };

        // The number of rectangles in the set.
        inline int GetNumRectangles() const
        {
            return mNumRectangles;
        }

        // The i-th rectangle is [xmin,xmax)x[ymin,ymax).  The values xmin,
        // xmax, ymin and ymax are valid when 0 <= i < GetNumRectangles().
        bool GetRectangle(int i, Scalar& xmin, Scalar& xmax, Scalar& ymin, Scalar& ymax) const
        {
            int totalQuantity = 0;
            for (auto const& strip : mStrips)
            {
                ISet const& intervalSet = strip.intervalSet;
                int xQuantity = intervalSet.GetNumIntervals();
                int nextTotalQuantity = totalQuantity + xQuantity;
                if (i < nextTotalQuantity)
                {
                    i -= totalQuantity;
                    intervalSet.GetInterval(i, xmin, xmax);
                    ymin = strip.ymin;
                    ymax = strip.ymax;
                    return true;
                }
                totalQuantity = nextTotalQuantity;
            }
            return false;
        }

        // Make this set empty.
        inline void Clear()
        {
            mNumRectangles = 0;
            mStrips.clear();
        }

        // The number of y-strips in the set.
        inline int GetNumStrips() const
        {
            return static_cast<int>(mStrips.size());
        }

        // The i-th strip.  The returned values are valid when
        // 0 <= i < GetStripQuantity().
        bool GetStrip(int i, Scalar& ymin, Scalar& ymax, ISet& xIntervalSet) const
        {
            if (0 <= i && i < GetNumStrips())
            {
                Strip const& strip = mStrips[i];
                ymin = strip.ymin;
                ymax = strip.ymax;
                xIntervalSet = strip.intervalSet;
                return true;
            }
            return false;
        }

        // Insert [xmin,xmax)x[ymin,ymax) into the set.  This is a Boolean
        // union operation.  The operation is successful only when xmin < xmax
        // and ymin < ymax.
        bool Insert(Scalar const& xmin, Scalar const& xmax, Scalar const& ymin, Scalar const& ymax)
        {
            if (xmin < xmax && ymin < ymax)
            {
                DisjointRectangles input(xmin, xmax, ymin, ymax);
                DisjointRectangles output = *this | input;
                *this = std::move(output);
                return true;
            }
            return false;
        }

        // Remove [xmin,xmax)x[ymin,ymax) from the set.  This is a Boolean
        // difference operation.  The operation is successful only when
        // xmin < xmax and ymin < ymax.
        bool Remove(Scalar const& xmin, Scalar const& xmax, Scalar const& ymin, Scalar const& ymax)
        {
            if (xmin < xmax && ymin < ymax)
            {
                DisjointRectangles input(xmin, xmax, ymin, ymax);
                DisjointRectangles output = *this - input;
                *this = std::move(output);
                return true;
            }
            return false;
        }

        // Get the union of the rectangle sets sets, input0 union input1.
        friend DisjointRectangles operator|(DisjointRectangles const& input0, DisjointRectangles const& input1)
        {
            return Execute(
                [](ISet const& i0, ISet const& i1) { return i0 | i1; },
                true, true, input0, input1);
        }

        // Get the intersection of the rectangle sets, input0 intersect is1.
        friend DisjointRectangles operator&(DisjointRectangles const& input0, DisjointRectangles const& input1)
        {
            return Execute(
                [](ISet const& i0, ISet const& i1) { return i0 & i1; },
                false, false, input0, input1);
        }

        // Get the differences of the rectangle sets, input0 minus input1.
        friend DisjointRectangles operator-(DisjointRectangles const& input0, DisjointRectangles const& input1)
        {
            return Execute(
                [](ISet const& i0, ISet const& i1) { return i0 - i1; },
                false, true, input0, input1);
        }

        // Get the exclusive or of the rectangle sets, input0 xor input1 =
        // (input0 minus input1) or (input1 minus input0).
        friend DisjointRectangles operator^(DisjointRectangles const& input0, DisjointRectangles const& input1)
        {
            return Execute(
                [](ISet const& i0, ISet const& i1) { return i0 ^ i1; },
                true, true, input0, input1);
        }

    private:
        static DisjointRectangles Execute(
            std::function<ISet(ISet const&, ISet const&)> const& operation,
            bool unionExclusiveOr, bool unionExclusiveOrDifference,
            DisjointRectangles const& input0, DisjointRectangles const& input1)
        {
            DisjointRectangles output;

            size_t const numStrips0 = input0.GetNumStrips();
            size_t const numStrips1 = input1.GetNumStrips();
            size_t i0 = 0, i1 = 0;
            bool getOriginal0 = true, getOriginal1 = true;
            Scalar ymin0 = (Scalar)0;
            Scalar ymax0 = (Scalar)0;
            Scalar ymin1 = (Scalar)0;
            Scalar ymax1 = (Scalar)0;

            while (i0 < numStrips0 && i1 < numStrips1)
            {
                ISet const& intr0 = input0.mStrips[i0].intervalSet;
                if (getOriginal0)
                {
                    ymin0 = input0.mStrips[i0].ymin;
                    ymax0 = input0.mStrips[i0].ymax;
                }

                ISet const& intr1 = input1.mStrips[i1].intervalSet;
                if (getOriginal1)
                {
                    ymin1 = input1.mStrips[i1].ymin;
                    ymax1 = input1.mStrips[i1].ymax;
                }

                // Case 1.
                if (ymax1 <= ymin0)
                {
                    // operator(empty,strip1)
                    if (unionExclusiveOr)
                    {
                        output.mStrips.push_back(Strip(ymin1, ymax1, intr1));
                    }

                    ++i1;
                    getOriginal0 = false;
                    getOriginal1 = true;
                    continue;  // using next ymin1/ymax1
                }

                // Case 11.
                if (ymin1 >= ymax0)
                {
                    // operator(strip0,empty)
                    if (unionExclusiveOrDifference)
                    {
                        output.mStrips.push_back(Strip(ymin0, ymax0, intr0));
                    }

                    ++i0;
                    getOriginal0 = true;
                    getOriginal1 = false;
                    continue;  // using next ymin0/ymax0
                }

                // Reduce cases 2, 3, 4 to cases 5, 6, 7.
                if (ymin1 < ymin0)
                {
                    // operator(empty,[ymin1,ymin0))
                    if (unionExclusiveOr)
                    {
                        output.mStrips.push_back(Strip(ymin1, ymin0, intr1));
                    }

                    ymin1 = ymin0;
                    getOriginal1 = false;
                }

                // Reduce cases 8, 9, 10 to cases 5, 6, 7.
                if (ymin1 > ymin0)
                {
                    // operator([ymin0,ymin1),empty)
                    if (unionExclusiveOrDifference)
                    {
                        output.mStrips.push_back(Strip(ymin0, ymin1, intr0));
                    }

                    ymin0 = ymin1;
                    getOriginal0 = false;
                }

                // Case 5.
                if (ymax1 < ymax0)
                {
                    // operator(strip0,[ymin1,ymax1))
                    auto result = operation(intr0, intr1);
                    output.mStrips.push_back(Strip(ymin1, ymax1, result));

                    ymin0 = ymax1;
                    ++i1;
                    getOriginal0 = false;
                    getOriginal1 = true;
                    continue;  // using next ymin1/ymax1
                }

                // Case 6.
                if (ymax1 == ymax0)
                {
                    // operator(strip0,[ymin1,ymax1))
                    auto result = operation(intr0, intr1);
                    output.mStrips.push_back(Strip(ymin1, ymax1, result));

                    ++i0;
                    ++i1;
                    getOriginal0 = true;
                    getOriginal1 = true;
                    continue;  // using next ymin0/ymax0 and ymin1/ymax1
                }

                // Case 7.
                if (ymax1 > ymax0)
                {
                    // operator(strip0,[ymin1,ymax0))
                    auto result = operation(intr0, intr1);
                    output.mStrips.push_back(Strip(ymin1, ymax0, result));

                    ymin1 = ymax0;
                    ++i0;
                    getOriginal0 = true;
                    getOriginal1 = false;
                    // continue;  using current ymin1/ymax1
                }
            }

            if (unionExclusiveOrDifference)
            {
                while (i0 < numStrips0)
                {
                    if (getOriginal0)
                    {
                        ymin0 = input0.mStrips[i0].ymin;
                        ymax0 = input0.mStrips[i0].ymax;
                    }
                    else
                    {
                        getOriginal0 = true;
                    }

                    // operator(strip0,empty)
                    output.mStrips.push_back(Strip(ymin0, ymax0,
                        input0.mStrips[i0].intervalSet));

                    ++i0;
                }
            }

            if (unionExclusiveOr)
            {
                while (i1 < numStrips1)
                {
                    if (getOriginal1)
                    {
                        ymin1 = input1.mStrips[i1].ymin;
                        ymax1 = input1.mStrips[i1].ymax;
                    }
                    else
                    {
                        getOriginal1 = true;
                    }

                    // operator(empty,strip1)
                    output.mStrips.push_back(Strip(ymin1, ymax1,
                        input1.mStrips[i1].intervalSet));

                    ++i1;
                }
            }

            output.ComputeRectangleQuantity();
            return output;
        }

        void ComputeRectangleQuantity()
        {
            mNumRectangles = 0;
            for (auto strip : mStrips)
            {
                mNumRectangles += strip.intervalSet.GetNumIntervals();
            }
        }

        // The number of rectangles in the set.
        int mNumRectangles;

        // The y-strips of the set, each containing an x-interval set.
        std::vector<Strip> mStrips;
    };
}
