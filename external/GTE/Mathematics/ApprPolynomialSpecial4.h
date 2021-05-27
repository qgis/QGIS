// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprQuery.h>
#include <Mathematics/GMatrix.h>
#include <array>

// Fit the data with a polynomial of the form
//     w = sum_{i=0}^{n-1} c[i]*x^{p[i]}*y^{q[i]}*z^{r[i]}
// where <p[i],q[i],r[i]> are distinct triples of nonnegative powers provided
// by the caller. A least-squares fitting algorithm is used, but the input
// data is first mapped to (x,y,z,w) in [-1,1]^4 for numerical robustness.

namespace gte
{
    template <typename Real>
    class ApprPolynomialSpecial4 : public ApprQuery<Real, std::array<Real, 4>>
    {
    public:
        // Initialize the model parameters to zero.  The degrees must be
        // nonnegative and strictly increasing.
        ApprPolynomialSpecial4(std::vector<int> const& xDegrees,
            std::vector<int> const& yDegrees, std::vector<int> const& zDegrees)
            :
            mXDegrees(xDegrees),
            mYDegrees(yDegrees),
            mZDegrees(zDegrees),
            mParameters(mXDegrees.size() * mYDegrees.size() * mZDegrees.size(), (Real)0)
        {
#if !defined(GTE_NO_LOGGER)
            LogAssert(mXDegrees.size() == mYDegrees.size()
                && mXDegrees.size() == mZDegrees.size(),
                "The input arrays must have the same size.");

            LogAssert(mXDegrees.size() > 0, "The input array must have elements.");
            int lastDegree = -1;
            for (auto degree : mXDegrees)
            {
                LogAssert(degree > lastDegree, "Degrees must be increasing.");
                lastDegree = degree;
            }

            LogAssert(mYDegrees.size() > 0, "The input array must have elements.");
            lastDegree = -1;
            for (auto degree : mYDegrees)
            {
                LogAssert(degree > lastDegree, "Degrees must be increasing.");
                lastDegree = degree;
            }

            LogAssert(mZDegrees.size() > 0, "The input array must have elements.");
            lastDegree = -1;
            for (auto degree : mZDegrees)
            {
                LogAssert(degree > lastDegree, "Degrees must be increasing.");
                lastDegree = degree;
            }
#endif

            mXDomain[0] = std::numeric_limits<Real>::max();
            mXDomain[1] = -mXDomain[0];
            mYDomain[0] = std::numeric_limits<Real>::max();
            mYDomain[1] = -mYDomain[0];
            mZDomain[0] = std::numeric_limits<Real>::max();
            mZDomain[1] = -mZDomain[0];
            mWDomain[0] = std::numeric_limits<Real>::max();
            mWDomain[1] = -mWDomain[0];

            mScale[0] = (Real)0;
            mScale[1] = (Real)0;
            mScale[2] = (Real)0;
            mScale[3] = (Real)0;
            mInvTwoWScale = (Real)0;

            // Powers of x, y, and z are computed up to twice the powers when
            // constructing the fitted polynomial. Powers of x, y, and z are
            // computed up to the powers for the evaluation of the fitted
            // polynomial.
            mXPowers.resize(2 * mXDegrees.back() + 1);
            mXPowers[0] = (Real)1;
            mYPowers.resize(2 * mYDegrees.back() + 1);
            mYPowers[0] = (Real)1;
            mZPowers.resize(2 * mZDegrees.back() + 1);
            mZPowers[0] = (Real)1;
        }

        // Basic fitting algorithm. See ApprQuery.h for the various Fit(...)
        // functions that you can call.
        virtual bool FitIndexed(
            size_t numObservations, std::array<Real, 4> const* observations,
            size_t numIndices, int const* indices) override
        {
            if (this->ValidIndices(numObservations, observations, numIndices, indices))
            {
                // Transform the observations to [-1,1]^4 for numerical
                // robustness.
                std::vector<std::array<Real, 4>> transformed;
                Transform(observations, numIndices, indices, transformed);

                // Fit the transformed data using a least-squares algorithm.
                return DoLeastSquares(transformed);
            }

            std::fill(mParameters.begin(), mParameters.end(), (Real)0);
            return false;
        }

        // Get the parameters for the best fit.
        std::vector<Real> const& GetParameters() const
        {
            return mParameters;
        }

        virtual size_t GetMinimumRequired() const override
        {
            return mParameters.size();
        }

        // Compute the model error for the specified observation for the
        // current model parameters. The returned value for observation
        // (x0,y0,z0,w0) is |w(x0,y0,z0) - w0|, where w(x,y,z) is the fitted
        // polynomial.
        virtual Real Error(std::array<Real, 4> const& observation) const override
        {
            Real w = Evaluate(observation[0], observation[1], observation[2]);
            Real error = std::fabs(w - observation[3]);
            return error;
        }

        virtual void CopyParameters(ApprQuery<Real, std::array<Real, 4>> const* input) override
        {
            auto source = dynamic_cast<ApprPolynomialSpecial4 const*>(input);
            if (source)
            {
                *this = *source;
            }
        }

        // Evaluate the polynomial. The domain interval is provided so you can
        // interpolate ((x,y,z) in domain) or extrapolate ((x,y,z) not in
        // domain).
        std::array<Real, 2> const& GetXDomain() const
        {
            return mXDomain;
        }

        std::array<Real, 2> const& GetYDomain() const
        {
            return mYDomain;
        }

        std::array<Real, 2> const& GetZDomain() const
        {
            return mZDomain;
        }

        Real Evaluate(Real x, Real y, Real z) const
        {
            // Transform (x,y,z) to (x',y',z') in [-1,1]^3.
            x = (Real)-1 + (Real)2 * mScale[0] * (x - mXDomain[0]);
            y = (Real)-1 + (Real)2 * mScale[1] * (y - mYDomain[0]);
            z = (Real)-1 + (Real)2 * mScale[2] * (z - mZDomain[0]);

            // Compute relevant powers of x, y, and z.
            int jmax = mXDegrees.back();;
            for (int j = 1; j <= jmax; ++j)
            {
                mXPowers[j] = mXPowers[j - 1] * x;
            }

            jmax = mYDegrees.back();;
            for (int j = 1; j <= jmax; ++j)
            {
                mYPowers[j] = mYPowers[j - 1] * y;
            }

            jmax = mZDegrees.back();;
            for (int j = 1; j <= jmax; ++j)
            {
                mZPowers[j] = mZPowers[j - 1] * z;
            }

            Real w = (Real)0;
            int isup = static_cast<int>(mXDegrees.size());
            for (int i = 0; i < isup; ++i)
            {
                Real xp = mXPowers[mXDegrees[i]];
                Real yp = mYPowers[mYDegrees[i]];
                Real zp = mYPowers[mZDegrees[i]];
                w += mParameters[i] * xp * yp * zp;
            }

            // Transform w from [-1,1] back to the original space.
            w = (w + (Real)1) * mInvTwoWScale + mWDomain[0];
            return w;
        }

    private:
        // Transform the (x,y,z,w) values to (x',y',z',w') in [-1,1]^4.
        void Transform(std::array<Real, 4> const* observations, size_t numIndices,
            int const* indices, std::vector<std::array<Real, 4>> & transformed)
        {
            int numSamples = static_cast<int>(numIndices);
            transformed.resize(numSamples);

            std::array<Real, 4> omin = observations[indices[0]];
            std::array<Real, 4> omax = omin;
            std::array<Real, 4> obs;
            int s, i;
            for (s = 1; s < numSamples; ++s)
            {
                obs = observations[indices[s]];
                for (i = 0; i < 4; ++i)
                {
                    if (obs[i] < omin[i])
                    {
                        omin[i] = obs[i];
                    }
                    else if (obs[i] > omax[i])
                    {
                        omax[i] = obs[i];
                    }
                }
            }

            mXDomain[0] = omin[0];
            mXDomain[1] = omax[0];
            mYDomain[0] = omin[1];
            mYDomain[1] = omax[1];
            mZDomain[0] = omin[2];
            mZDomain[1] = omax[2];
            mWDomain[0] = omin[3];
            mWDomain[1] = omax[3];
            for (i = 0; i < 4; ++i)
            {
                mScale[i] = (Real)1 / (omax[i] - omin[i]);
            }

            for (s = 0; s < numSamples; ++s)
            {
                obs = observations[indices[s]];
                for (i = 0; i < 4; ++i)
                {
                    transformed[s][i] = (Real)-1 + (Real)2 * mScale[i] * (obs[i] - omin[i]);
                }
            }
            mInvTwoWScale = (Real)0.5 / mScale[3];
        }

        // The least-squares fitting algorithm for the transformed data.
        bool DoLeastSquares(std::vector<std::array<Real, 4>> & transformed)
        {
            // Set up a linear system A*X = B, where X are the polynomial
            // coefficients.
            int size = static_cast<int>(mXDegrees.size());
            GMatrix<Real> A(size, size);
            A.MakeZero();
            GVector<Real> B(size);
            B.MakeZero();

            int numSamples = static_cast<int>(transformed.size());
            int twoMaxXDegree = 2 * mXDegrees.back();
            int twoMaxYDegree = 2 * mYDegrees.back();
            int twoMaxZDegree = 2 * mZDegrees.back();
            int row, col;
            for (int i = 0; i < numSamples; ++i)
            {
                // Compute relevant powers of x, y, and z.
                Real x = transformed[i][0];
                Real y = transformed[i][1];
                Real z = transformed[i][2];
                Real w = transformed[i][3];
                for (int j = 1; j <= twoMaxXDegree; ++j)
                {
                    mXPowers[j] = mXPowers[j - 1] * x;
                }
                for (int j = 1; j <= twoMaxYDegree; ++j)
                {
                    mYPowers[j] = mYPowers[j - 1] * y;
                }
                for (int j = 1; j <= twoMaxZDegree; ++j)
                {
                    mZPowers[j] = mZPowers[j - 1] * z;
                }

                for (row = 0; row < size; ++row)
                {
                    // Update the upper-triangular portion of the symmetric
                    // matrix.
                    Real xp, yp, zp;
                    for (col = row; col < size; ++col)
                    {
                        xp = mXPowers[mXDegrees[row] + mXDegrees[col]];
                        yp = mYPowers[mYDegrees[row] + mYDegrees[col]];
                        zp = mZPowers[mZDegrees[row] + mZDegrees[col]];
                        A(row, col) += xp * yp * zp;
                    }

                    // Update the right-hand side of the system.
                    xp = mXPowers[mXDegrees[row]];
                    yp = mYPowers[mYDegrees[row]];
                    zp = mZPowers[mZDegrees[row]];
                    B[row] += xp * yp * zp * w;
                }
            }

            // Copy the upper-triangular portion of the symmetric matrix to
            // the lower-triangular portion.
            for (row = 0; row < size; ++row)
            {
                for (col = 0; col < row; ++col)
                {
                    A(row, col) = A(col, row);
                }
            }

            // Precondition by normalizing the sums.
            Real invNumSamples = (Real)1 / (Real)numSamples;
            A *= invNumSamples;
            B *= invNumSamples;

            // Solve for the polynomial coefficients.
            GVector<Real> coefficients = Inverse(A) * B;
            bool hasNonzero = false;
            for (int i = 0; i < size; ++i)
            {
                mParameters[i] = coefficients[i];
                if (coefficients[i] != (Real)0)
                {
                    hasNonzero = true;
                }
            }
            return hasNonzero;
        }

        std::vector<int> mXDegrees, mYDegrees, mZDegrees;
        std::vector<Real> mParameters;

        // Support for evaluation. The coefficients were generated for the
        // samples mapped to [-1,1]^4. The Evaluate() function must transform
        // (x,y,z) to (x',y',z') in [-1,1]^3, compute w' in [-1,1], then
        // transform w' to w.
        std::array<Real, 2> mXDomain, mYDomain, mZDomain, mWDomain;
        std::array<Real, 4> mScale;
        Real mInvTwoWScale;

        // This array is used by Evaluate() to avoid reallocation of the
        // 'vector's for each call. The members are mutable because, to the
        // user, the call to Evaluate does not modify the polynomial.
        mutable std::vector<Real> mXPowers, mYPowers, mZPowers;
    };
}
