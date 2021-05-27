// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FastMarch.h>
#include <Mathematics/Math.h>

// The topic of fast marching methods are discussed in the book
//   Level Set Methods and Fast Marching Methods:
//     Evolving Interfaces in Computational Geometry, Fluid Mechanics,
//     Computer Vision, and Materials Science
//   J.A. Sethian,
//   Cambridge University Press, 1999

namespace gte
{
    template <typename Real>
    class FastMarch2 : public FastMarch<Real>
    {
    public:
        // Construction and destruction.
        FastMarch2(size_t xBound, size_t yBound, Real xSpacing, Real ySpacing,
            std::vector<size_t> const& seeds, std::vector<Real> const& speeds)
            :
            FastMarch<Real>(xBound * yBound, seeds, speeds)
        {
            Initialize(xBound, yBound, xSpacing, ySpacing);
        }

        FastMarch2(size_t xBound, size_t yBound, Real xSpacing, Real ySpacing,
            std::vector<size_t> const& seeds, Real speed)
            :
            FastMarch<Real>(xBound * yBound, seeds, speed)
        {
            Initialize(xBound, yBound, xSpacing, ySpacing);
        }

        virtual ~FastMarch2()
        {
        }

        // Member access.
        inline size_t GetXBound() const
        {
            return mXBound;
        }

        inline size_t GetYBound() const
        {
            return mYBound;
        }

        inline Real GetXSpacing() const
        {
            return mXSpacing;
        }

        inline Real GetYSpacing() const
        {
            return mYSpacing;
        }

        inline size_t Index(size_t x, size_t y) const
        {
            return x + mXBound * y;
        }

        // Pixel classification.
        virtual void GetBoundary(std::vector<size_t>& boundary) const override
        {
            for (size_t i = 0; i < this->mQuantity; ++i)
            {
                if (this->IsValid(i) && !this->IsTrial(i))
                {
                    if (this->IsTrial(i - 1)
                        || this->IsTrial(i + 1)
                        || this->IsTrial(i - mXBound)
                        || this->IsTrial(i + mXBound))
                    {
                        boundary.push_back(i);
                    }
                }
            }
        }

        virtual bool IsBoundary(size_t i) const override
        {
            if (this->IsValid(i) && !this->IsTrial(i))
            {
                if (this->IsTrial(i - 1)
                    || this->IsTrial(i + 1)
                    || this->IsTrial(i - mXBound)
                    || this->IsTrial(i + mXBound))
                {
                    return true;
                }
            }
            return false;
        }

        // Run one step of the fast marching algorithm.
        virtual void Iterate() override
        {
            // Remove the minimum trial value from the heap.
            size_t i;
            Real value;
            this->mHeap.Remove(i, value);

            // Promote the trial value to a known value.  The value was
            // negative but is now nonnegative (the heap stores only
            // nonnegative numbers).
            this->mTrials[i] = nullptr;

            // All trial pixels must be updated.  All far neighbors must become trial
            // pixels.
            size_t iM1 = i - 1;
            if (this->IsTrial(iM1))
            {
                ComputeTime(iM1);
                this->mHeap.Update(this->mTrials[iM1], this->mTimes[iM1]);
            }
            else if (this->IsFar(iM1))
            {
                ComputeTime(iM1);
                this->mTrials[iM1] = this->mHeap.Insert(iM1, this->mTimes[iM1]);
            }

            size_t iP1 = i + 1;
            if (this->IsTrial(iP1))
            {
                ComputeTime(iP1);
                this->mHeap.Update(this->mTrials[iP1], this->mTimes[iP1]);
            }
            else if (this->IsFar(iP1))
            {
                ComputeTime(iP1);
                this->mTrials[iP1] = this->mHeap.Insert(iP1, this->mTimes[iP1]);
            }

            size_t iMXB = i - mXBound;
            if (this->IsTrial(iMXB))
            {
                ComputeTime(iMXB);
                this->mHeap.Update(this->mTrials[iMXB], this->mTimes[iMXB]);
            }
            else if (this->IsFar(iMXB))
            {
                ComputeTime(iMXB);
                this->mTrials[iMXB] = this->mHeap.Insert(iMXB, this->mTimes[iMXB]);
            }

            size_t iPXB = i + mXBound;
            if (this->IsTrial(iPXB))
            {
                ComputeTime(iPXB);
                this->mHeap.Update(this->mTrials[iPXB], this->mTimes[iPXB]);
            }
            else if (this->IsFar(iPXB))
            {
                ComputeTime(iPXB);
                this->mTrials[iPXB] = this->mHeap.Insert(iPXB, this->mTimes[iPXB]);
            }
        }

    protected:
        // Called by the constructors.
        void Initialize(size_t xBound, size_t yBound, Real xSpacing, Real ySpacing)
        {
            mXBound = xBound;
            mYBound = yBound;
            mXBoundM1 = mXBound - 1;
            mYBoundM1 = mYBound - 1;
            mXSpacing = xSpacing;
            mYSpacing = ySpacing;
            mInvXSpacing = (Real)1 / xSpacing;
            mInvYSpacing = (Real)1 / ySpacing;

            // Boundary pixels are marked as zero speed to allow us to avoid
            // having to process the boundary pixels separately during the
            // iteration.
            size_t x, y, i;

            // vertex (0,0)
            i = Index(0, 0);
            this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
            this->mTimes[i] = -std::numeric_limits<Real>::max();

            // vertex (xmax,0)
            i = Index(mXBoundM1, 0);
            this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
            this->mTimes[i] = -std::numeric_limits<Real>::max();

            // vertex (0,ymax)
            i = Index(0, mYBoundM1);
            this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
            this->mTimes[i] = -std::numeric_limits<Real>::max();

            // vertex (xmax,ymax)
            i = Index(mXBoundM1, mYBoundM1);
            this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
            this->mTimes[i] = -std::numeric_limits<Real>::max();

            // edges (x,0) and (x,ymax)
            for (x = 0; x < mXBound; ++x)
            {
                i = Index(x, 0);
                this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
                this->mTimes[i] = -std::numeric_limits<Real>::max();
                i = Index(x, mYBoundM1);
                this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
                this->mTimes[i] = -std::numeric_limits<Real>::max();
            }

            // edges (0,y) and (xmax,y)
            for (y = 0; y < mYBound; ++y)
            {
                i = Index(0, y);
                this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
                this->mTimes[i] = -std::numeric_limits<Real>::max();
                i = Index(mXBoundM1, y);
                this->mInvSpeeds[i] = std::numeric_limits<Real>::max();
                this->mTimes[i] = -std::numeric_limits<Real>::max();
            }

            // Compute the first batch of trial pixels.  These are pixels a
            // grid distance of one away from the seed pixels.
            for (y = 1; y < mYBoundM1; ++y)
            {
                for (x = 1; x < mXBoundM1; ++x)
                {
                    i = Index(x, y);
                    if (this->IsFar(i))
                    {
                        if ((this->IsValid(i - 1) && !this->IsTrial(i - 1))
                            || (this->IsValid(i + 1) && !this->IsTrial(i + 1))
                            || (this->IsValid(i - mXBound) && !this->IsTrial(i - mXBound))
                            || (this->IsValid(i + mXBound) && !this->IsTrial(i + mXBound)))
                        {
                            ComputeTime(i);
                            this->mTrials[i] = this->mHeap.Insert(i, this->mTimes[i]);
                        }
                    }
                }
            }
        }

        // Called by Iterate().
        void ComputeTime(size_t i)
        {
            bool hasXTerm;
            Real xConst;
            if (this->IsValid(i - 1))
            {
                hasXTerm = true;
                xConst = this->mTimes[i - 1];
                if (this->IsValid(i + 1))
                {
                    if (this->mTimes[i + 1] < xConst)
                    {
                        xConst = this->mTimes[i + 1];
                    }
                }
            }
            else if (this->IsValid(i + 1))
            {
                hasXTerm = true;
                xConst = this->mTimes[i + 1];
            }
            else
            {
                hasXTerm = false;
                xConst = (Real)0;
            }

            bool hasYTerm;
            Real yConst;
            if (this->IsValid(i - mXBound))
            {
                hasYTerm = true;
                yConst = this->mTimes[i - mXBound];
                if (this->IsValid(i + mXBound))
                {
                    if (this->mTimes[i + mXBound] < yConst)
                    {
                        yConst = this->mTimes[i + mXBound];
                    }
                }
            }
            else if (this->IsValid(i + mXBound))
            {
                hasYTerm = true;
                yConst = this->mTimes[i + mXBound];
            }
            else
            {
                hasYTerm = false;
                yConst = (Real)0;
            }

            if (hasXTerm)
            {
                if (hasYTerm)
                {
                    Real sum = xConst + yConst;
                    Real diff = xConst - yConst;
                    Real discr = (Real)2 * this->mInvSpeeds[i] * this->mInvSpeeds[i] - diff * diff;
                    if (discr >= (Real)0)
                    {
                        // The quadratic equation has a real-valued solution.
                        // Choose the largest positive root for the crossing
                        // time.
                        this->mTimes[i] = (Real)0.5 * (sum + std::sqrt(discr));
                    }
                    else
                    {
                        // The quadratic equation does not have a real-valued
                        // solution.  This can happen when the speed is so
                        // large that the time gradient has very small length,
                        // which means that the time has not changed
                        // significantly from the neighbors to the current
                        // pixel.  Just choose the maximum time of the
                        // neighbors.  (Is there a better choice?)
                        this->mTimes[i] = (diff >= (Real)0 ? xConst : yConst);
                    }
                }
                else
                {
                    // The equation is linear.
                    this->mTimes[i] = this->mInvSpeeds[i] + xConst;
                }
            }
            else if (hasYTerm)
            {
                // The equation is linear.
                this->mTimes[i] = this->mInvSpeeds[i] + yConst;
            }
            else
            {
                // Assert: The pixel must have at least one known neighbor.
            }
        }

        size_t mXBound, mYBound, mXBoundM1, mYBoundM1;
        Real mXSpacing, mYSpacing, mInvXSpacing, mInvYSpacing;
    };
}
