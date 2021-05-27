// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/MinHeap.h>
#include <limits>

// The topic of fast marching methods are discussed in the book
//   Level Set Methods and Fast Marching Methods:
//     Evolving Interfaces in Computational Geometry, Fluid Mechanics,
//     Computer Vision, and Materials Science
//   J.A. Sethian,
//   Cambridge University Press, 1999

namespace gte
{
    template <typename Real>
    class FastMarch
    {
        // Abstract base class.
    public:
        virtual ~FastMarch()
        {
        }

    protected:
        // The seed points have a crossing time of 0.  As the iterations
        // occur, some of the non-seed points are visited by the moving
        // front.  Define maxReal to be std::numeric_limits<Real>::max().
        // The valid crossing times are 0 <= t < maxReal.  A value of
        // maxReal indicates the pixel has not yet been reached by the
        // moving front.  If the speed value at a pixel is 0, the pixel
        // is marked with a time of -maxReal.  Such pixels can never be
        // visited; the minus sign distinguishes these from pixels not yet
        // reached during iteration.
        //
        // Trial pixels are identified by having min-heap records
        // associated with them.  Known or far pixels have no associated
        // record.
        //
        // The speeds must be nonnegative and are inverted because the
        // reciprocals are all that are needed in the numerical method.

        FastMarch(size_t quantity, std::vector<size_t> const& seeds, std::vector<Real> const& speeds)
            :
            mQuantity(quantity),
            mTimes(quantity, std::numeric_limits<Real>::max()),
            mInvSpeeds(quantity),
            mHeap(static_cast<int>(quantity)),
            mTrials(quantity, nullptr)
        {
            for (auto seed : seeds)
            {
                mTimes[seed] = (Real)0;
            }

            for (size_t i = 0; i < mQuantity; ++i)
            {
                if (speeds[i] > (Real)0)
                {
                    mInvSpeeds[i] = (Real)1 / speeds[i];
                }
                else
                {
                    mInvSpeeds[i] = std::numeric_limits<Real>::max();
                    mTimes[i] = -std::numeric_limits<Real>::max();
                }
            }
        }

        FastMarch(size_t quantity, std::vector<size_t> const& seeds, Real speed)
            :
            mQuantity(quantity),
            mTimes(quantity, std::numeric_limits<Real>::max()),
            mInvSpeeds(quantity, (Real)1 / speed),
            mHeap(static_cast<int>(quantity)),
            mTrials(quantity, nullptr)
        {
            for (auto seed : seeds)
            {
                mTimes[seed] = (Real)0;
            }
        }

    public:
        // Member access.
        inline size_t GetQuantity() const
        {
            return mQuantity;
        }

        inline void SetTime(size_t i, Real time)
        {
            mTimes[i] = time;
        }

        inline Real GetTime(size_t i) const
        {
            return mTimes[i];
        }

        void GetTimeExtremes(Real& minValue, Real& maxValue) const
        {
            minValue = std::numeric_limits<Real>::max();
            maxValue = -std::numeric_limits<Real>::max();
            size_t i;
            for (i = 0; i < mQuantity; ++i)
            {
                if (IsValid(i))
                {
                    minValue = mTimes[i];
                    maxValue = minValue;
                    break;
                }
            }

            // Assert:  At least one time must be valid, in which case
            // i < mQuantity at this point.  If all times are invalid,
            // minValue = +maxReal and maxValue = -maxReal on exit.

            for (/**/; i < mQuantity; ++i)
            {
                if (IsValid(i))
                {
                    if (mTimes[i] < minValue)
                    {
                        minValue = mTimes[i];
                    }
                    else if (mTimes[i] > maxValue)
                    {
                        maxValue = mTimes[i];
                    }
                }
            }
        }

        // Image element classification.
        inline bool IsValid(size_t i) const
        {
            return (Real)0 <= mTimes[i] && mTimes[i] < std::numeric_limits<Real>::max();
        }

        inline bool IsTrial(size_t i) const
        {
            return mTrials[i] != nullptr;
        }

        inline bool IsFar(size_t i) const
        {
            return mTimes[i] == std::numeric_limits<Real>::max();
        }

        inline bool IsZeroSpeed(size_t i) const
        {
            return mTimes[i] == -std::numeric_limits<Real>::max();
        }

        inline bool IsInterior(size_t i) const
        {
            return IsValid(i) && !IsTrial(i);
        }

        void GetInterior(std::vector<size_t>& interior) const
        {
            interior.clear();
            for (size_t i = 0; i < mQuantity; ++i)
            {
                if (IsValid(i) && !IsTrial(i))
                {
                    interior.push_back(i);
                }
            }
        }

        virtual void GetBoundary(std::vector<size_t>& boundary) const = 0;
        virtual bool IsBoundary(size_t i) const = 0;

        // Run one step of the fast marching algorithm.
        virtual void Iterate() = 0;

    protected:
        size_t mQuantity;
        std::vector<Real> mTimes;
        std::vector<Real> mInvSpeeds;
        MinHeap<size_t, Real> mHeap;
        std::vector<typename MinHeap<size_t, Real>::Record*> mTrials;
    };
}
