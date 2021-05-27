// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/GVector.h>
#include <Mathematics/Minimize1.h>
#include <cstring>

// The Cartesian-product domain provided to GetMinimum(*) has minimum values
// stored in t0[0..d-1] and maximum values stored in t1[0..d-1], where d is
// 'dimensions'. The domain is searched along lines through the current
// estimate of the minimum location. Each such line is searched for a minimum
// using a Minimize1<Real> object. This is called "Powell's Direction Set
// Method". The parameters 'maxLevel' and 'maxBracket' are used by
// Minimize1<Real>, so read the documentation for that class (in its header
// file) to understand what these mean. The input 'maxIterations' is the
// number of iterations for the direction-set method.

namespace gte
{
    template <typename Real>
    class MinimizeN
    {
    public:
        // Construction.
        MinimizeN(int dimensions, std::function<Real(Real const*)> const& F,
            int maxLevel, int maxBracket, int maxIterations, Real epsilon = (Real)1e-06)
            :
            mDimensions(dimensions),
            mFunction(F),
            mMaxIterations(maxIterations),
            mEpsilon(0),
            mDirections(dimensions + 1),
            mDConjIndex(dimensions),
            mDCurrIndex(0),
            mTCurr(dimensions),
            mTSave(dimensions),
            mMinimizer([this](Real t){ return mFunction(&(mTCurr + t * mDirections[mDCurrIndex])[0]); }, maxLevel, maxBracket)
        {
            SetEpsilon(epsilon);
            for (auto& direction : mDirections)
            {
                direction.SetSize(dimensions);
            }
        }

        // Member access.
        inline void SetEpsilon(Real epsilon)
        {
            mEpsilon = (epsilon > (Real)0 ? epsilon : (Real)0);
        }

        inline Real GetEpsilon() const
        {
            return mEpsilon;
        }

        // Find the minimum on the Cartesian-product domain whose minimum
        // values are stored in t0[0..d-1] and whose maximum values are stored
        // in t1[0..d-1], where d is 'dimensions'. An initial guess is
        // specified in tInitial[0..d-1]. The location of the minimum is
        // tMin[0..d-1] and the value of the minimum is 'fMin'.
        void GetMinimum(Real const* t0, Real const* t1, Real const* tInitial, Real* tMin, Real& fMin)
        {
            // The initial guess.
            size_t numBytes = mDimensions * sizeof(Real);
            mFCurr = mFunction(tInitial);
            std::memcpy(&mTSave[0], tInitial, numBytes);
            std::memcpy(&mTCurr[0], tInitial, numBytes);

            // Initialize the direction set to the standard Euclidean basis.
            for (int i = 0; i < mDimensions; ++i)
            {
                mDirections[i].MakeUnit(i);
            }

            Real ell0, ell1, ellMin;
            for (int iter = 0; iter < mMaxIterations; ++iter)
            {
                // Find minimum in each direction and update current location.
                for (int i = 0; i < mDimensions; ++i)
                {
                    mDCurrIndex = i;
                    ComputeDomain(t0, t1, ell0, ell1);
                    mMinimizer.GetMinimum(ell0, ell1, (Real)0, ellMin, mFCurr);
                    mTCurr += ellMin * mDirections[i];
                }

                // Estimate a unit-length conjugate direction.
                mDirections[mDConjIndex] = mTCurr - mTSave;
                Real length = Length(mDirections[mDConjIndex]);
                if (length <= mEpsilon)
                {
                    // New position did not change significantly from old one.
                    // Should there be a better convergence criterion here?
                    break;
                }

                mDirections[mDConjIndex] /= length;

                // Minimize in conjugate direction.
                mDCurrIndex = mDConjIndex;
                ComputeDomain(t0, t1, ell0, ell1);
                mMinimizer.GetMinimum(ell0, ell1, (Real)0, ellMin, mFCurr);
                mTCurr += ellMin * mDirections[mDCurrIndex];

                // Cycle the directions and add conjugate direction to set.
                mDConjIndex = 0;
                for (int i = 0; i < mDimensions; ++i)
                {
                    mDirections[i] = mDirections[i + 1];
                }

                // Set parameters for next pass.
                mTSave = mTCurr;
            }

            std::memcpy(tMin, &mTCurr[0], numBytes);
            fMin = mFCurr;
        }

    private:
        // The current estimate of the minimum location is mTCurr[0..d-1]. The
        // direction of the current line to search is mDCurr[0..d-1]. This
        // line must be clipped against the Cartesian-product domain, a
        // process implemented in this function.  If the line is
        // mTCurr+s*mDCurr, the clip result is the s-interval [ell0,ell1].
        void ComputeDomain(Real const* t0, Real const* t1, Real& ell0, Real& ell1)
        {
            ell0 = -std::numeric_limits<Real>::max();
            ell1 = +std::numeric_limits<Real>::max();

            for (int i = 0; i < mDimensions; ++i)
            {
                Real value = mDirections[mDCurrIndex][i];
                if (value != (Real)0)
                {
                    Real b0 = t0[i] - mTCurr[i];
                    Real b1 = t1[i] - mTCurr[i];
                    Real inv = ((Real)1) / value;
                    if (value > (Real)0)
                    {
                        // The valid t-interval is [b0,b1].
                        b0 *= inv;
                        if (b0 > ell0)
                        {
                            ell0 = b0;
                        }
                        b1 *= inv;
                        if (b1 < ell1)
                        {
                            ell1 = b1;
                        }
                    }
                    else
                    {
                        // The valid t-interval is [b1,b0].
                        b0 *= inv;
                        if (b0 < ell1)
                        {
                            ell1 = b0;
                        }
                        b1 *= inv;
                        if (b1 > ell0)
                        {
                            ell0 = b1;
                        }
                    }
                }
            }

            // Correction if numerical errors lead to values nearly zero.
            if (ell0 > (Real)0)
            {
                ell0 = (Real)0;
            }
            if (ell1 < (Real)0)
            {
                ell1 = (Real)0;
            }
        }

        int mDimensions;
        std::function<Real(Real const*)> mFunction;
        int mMaxIterations;
        Real mEpsilon;
        std::vector<GVector<Real>> mDirections;
        int mDConjIndex;
        int mDCurrIndex;
        GVector<Real> mTCurr;
        GVector<Real> mTSave;
        Real mFCurr;
        Minimize1<Real> mMinimizer;
    };
}
