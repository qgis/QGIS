// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/BasisFunction.h>
#include <Mathematics/BandedMatrix.h>

// The algorithm implemented here is based on the document
// https://www.geometrictools.com/Documentation/BSplineCurveLeastSquaresFit.pdf

namespace gte
{
    template <typename Real>
    class BSplineCurveFit
    {
    public:
        // Construction.  The preconditions for calling the constructor are
        //   1 <= degree && degree < numControls <= numSamples
        // The samples points are contiguous blocks of 'dimension' real values
        // stored in sampleData.
        BSplineCurveFit(int dimension, int numSamples, Real const* sampleData,
            int degree, int numControls)
            :
            mDimension(dimension),
            mNumSamples(numSamples),
            mSampleData(sampleData),
            mDegree(degree),
            mNumControls(numControls),
            mControlData(dimension * numControls)
        {
            LogAssert(dimension >= 1, "Invalid dimension.");
            LogAssert(1 <= degree && degree < numControls, "Invalid degree.");
            LogAssert(sampleData, "Invalid sample data.");
            LogAssert(numControls <= numSamples, "Invalid number of controls.");

            BasisFunctionInput<Real> input;
            input.numControls = numControls;
            input.degree = degree;
            input.uniform = true;
            input.periodic = false;
            input.numUniqueKnots = numControls - degree + 1;
            input.uniqueKnots.resize(input.numUniqueKnots);
            input.uniqueKnots[0].t = (Real)0;
            input.uniqueKnots[0].multiplicity = degree + 1;
            int last = input.numUniqueKnots - 1;
            Real factor = ((Real)1) / (Real)last;
            for (int i = 1; i < last; ++i)
            {
                input.uniqueKnots[i].t = factor * (Real)i;
                input.uniqueKnots[i].multiplicity = 1;
            }
            input.uniqueKnots[last].t = (Real)1;
            input.uniqueKnots[last].multiplicity = degree + 1;
            mBasis.Create(input);

            // Fit the data points with a B-spline curve using a least-squares
            // error metric.  The problem is of the form A^T*A*Q = A^T*P,
            // where A^T*A is a banded matrix, P contains the sample data, and
            // Q is the unknown vector of control points.
            Real tMultiplier = ((Real)1) / (Real)(mNumSamples - 1);
            Real t;
            int i0, i1, i2, imin, imax, j;

            // Construct the matrix A^T*A.
            int degp1 = mDegree + 1;
            int numBands = (mNumControls > degp1 ? degp1 : mDegree);
            BandedMatrix<Real> ATAMat(mNumControls, numBands, numBands);
            for (i0 = 0; i0 < mNumControls; ++i0)
            {
                for (i1 = 0; i1 < i0; ++i1)
                {
                    ATAMat(i0, i1) = ATAMat(i1, i0);
                }

                int i1Max = i0 + mDegree;
                if (i1Max >= mNumControls)
                {
                    i1Max = mNumControls - 1;
                }

                for (i1 = i0; i1 <= i1Max; ++i1)
                {
                    Real value = (Real)0;
                    for (i2 = 0; i2 < mNumSamples; ++i2)
                    {
                        t = tMultiplier * (Real)i2;
                        mBasis.Evaluate(t, 0, imin, imax);
                        if (imin <= i0 && i0 <= imax && imin <= i1 && i1 <= imax)
                        {
                            Real b0 = mBasis.GetValue(0, i0);
                            Real b1 = mBasis.GetValue(0, i1);
                            value += b0 * b1;
                        }
                    }
                    ATAMat(i0, i1) = value;
                }
            }

            // Construct the matrix A^T.
            Array2<Real> ATMat(mNumSamples, mNumControls);
            std::memset(ATMat[0], 0, mNumControls * mNumSamples * sizeof(Real));
            for (i0 = 0; i0 < mNumControls; ++i0)
            {
                for (i1 = 0; i1 < mNumSamples; ++i1)
                {
                    t = tMultiplier * (Real)i1;
                    mBasis.Evaluate(t, 0, imin, imax);
                    if (imin <= i0 && i0 <= imax)
                    {
                        ATMat[i0][i1] = mBasis.GetValue(0, i0);
                    }
                }
            }

            // Compute X0 = (A^T*A)^{-1}*A^T by solving the linear system
            // A^T*A*X = A^T.
            bool solved = ATAMat.template SolveSystem<true>(ATMat[0], mNumSamples);
            LogAssert(solved, "Failed to solve linear system.");

            // The control points for the fitted curve are stored in the
            // vector Q = X0*P, where P is the vector of sample data.
            std::fill(mControlData.begin(), mControlData.end(), (Real)0);
            for (i0 = 0; i0 < mNumControls; ++i0)
            {
                Real* Q = &mControlData[i0 * mDimension];
                for (i1 = 0; i1 < mNumSamples; ++i1)
                {
                    Real const* P = mSampleData + i1 * mDimension;
                    Real xValue = ATMat[i0][i1];
                    for (j = 0; j < mDimension; ++j)
                    {
                        Q[j] += xValue * P[j];
                    }
                }
            }

            // Set the first and last output control points to match the first
            // and last input samples.  This supports the application of
            // fitting keyframe data with B-spline curves.  The user expects
            // that the curve passes through the first and last positions in
            // order to support matching two consecutive keyframe sequences.
            Real* cEnd0 = &mControlData[0];
            Real const* sEnd0 = mSampleData;
            Real* cEnd1 = &mControlData[mDimension * (mNumControls - 1)];
            Real const* sEnd1 = &mSampleData[mDimension * (mNumSamples - 1)];
            for (j = 0; j < mDimension; ++j)
            {
                *cEnd0++ = *sEnd0++;
                *cEnd1++ = *sEnd1++;
            }
        }

        // Access to input sample information.
        inline int GetDimension() const
        {
            return mDimension;
        }

        inline int GetNumSamples() const
        {
            return mNumSamples;
        }

        inline Real const* GetSampleData() const
        {
            return mSampleData;
        }

        // Access to output control point and curve information.
        inline int GetDegree() const
        {
            return mDegree;
        }

        inline int GetNumControls() const
        {
            return mNumControls;
        }

        inline Real const* GetControlData() const
        {
            return &mControlData[0];
        }

        inline BasisFunction<Real> const& GetBasis() const
        {
            return mBasis;
        }

        // Evaluation of the B-spline curve.  It is defined for 0 <= t <= 1.
        // If a t-value is outside [0,1], an open spline clamps it to [0,1].
        // The caller must ensure that position[] has at least 'dimension'
        // elements.
        void Evaluate(Real t, unsigned int order, Real* value) const
        {
            int imin, imax;
            mBasis.Evaluate(t, order, imin, imax);

            Real const* source = &mControlData[mDimension * imin];
            Real basisValue = mBasis.GetValue(order, imin);
            for (int j = 0; j < mDimension; ++j)
            {
                value[j] = basisValue * (*source++);
            }

            for (int i = imin + 1; i <= imax; ++i)
            {
                basisValue = mBasis.GetValue(order, i);
                for (int j = 0; j < mDimension; ++j)
                {
                    value[j] += basisValue * (*source++);
                }
            }
        }

        void GetPosition(Real t, Real* position) const
        {
            Evaluate(t, 0, position);
        }

    private:
        // Input sample information.
        int mDimension;
        int mNumSamples;
        Real const* mSampleData;

        // The fitted B-spline curve, open and with uniform knots.
        int mDegree;
        int mNumControls;
        std::vector<Real> mControlData;
        BasisFunction<Real> mBasis;
    };
}
