// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprQuery.h>
#include <Mathematics/Array2.h>
#include <Mathematics/GMatrix.h>
#include <array>

// The samples are (x[i],y[i],w[i]) for 0 <= i < S. Think of w as a function
// of x and y, say w = f(x,y). The function fits the samples with a
// polynomial of degree d0 in x and degree d1 in y, say
//   w = sum_{i=0}^{d0} sum_{j=0}^{d1} c[i][j]*x^i*y^j
// The method is a least-squares fitting algorithm.  The mParameters stores
// c[i][j] = mParameters[i+(d0+1)*j] for a total of (d0+1)*(d1+1)
// coefficients. The observation type is std::array<Real,3>, which represents
// a triple (x,y,w).
//
// WARNING. The fitting algorithm for polynomial terms
//   (1,x,x^2,...,x^d0), (1,y,y^2,...,y^d1)
// is known to be nonrobust for large degrees and for large magnitude data.
// One alternative is to use orthogonal polynomials
//   (f[0](x),...,f[d0](x)), (g[0](y),...,g[d1](y))
// and apply the least-squares algorithm to these. Another alternative is to
// transform
//   (x',y',w') = ((x-xcen)/rng, (y-ycen)/rng, w/rng)
// where xmin = min(x[i]), xmax = max(x[i]), xcen = (xmin+xmax)/2,
// ymin = min(y[i]), ymax = max(y[i]), ycen = (ymin+ymax)/2, and
// rng = max(xmax-xmin,ymax-ymin). Fit the (x',y',w') points,
//   w' = sum_{i=0}^{d0} sum_{j=0}^{d1} c'[i][j]*(x')^i*(y')^j
// The original polynomial is evaluated as
//   w = rng * sum_{i=0}^{d0} sum_{j=0}^{d1} c'[i][j] *
//       ((x-xcen)/rng)^i * ((y-ycen)/rng)^j

namespace gte
{
    template <typename Real>
    class ApprPolynomial3 : public ApprQuery<Real, std::array<Real, 3>>
    {
    public:
        // Initialize the model parameters to zero.
        ApprPolynomial3(int xDegree, int yDegree)
            :
            mXDegree(xDegree),
            mYDegree(yDegree),
            mXDegreeP1(xDegree + 1),
            mYDegreeP1(yDegree + 1),
            mSize(mXDegreeP1 * mYDegreeP1),
            mParameters(mSize, (Real)0),
            mYCoefficient(mYDegreeP1, (Real)0)
        {
            mXDomain[0] = std::numeric_limits<Real>::max();
            mXDomain[1] = -mXDomain[0];
            mYDomain[0] = std::numeric_limits<Real>::max();
            mYDomain[1] = -mYDomain[0];
        }

        // Basic fitting algorithm. See ApprQuery.h for the various Fit(...)
        // functions that you can call.
        virtual bool FitIndexed(
            size_t numObservations, std::array<Real, 3> const* observations,
            size_t numIndices, int const* indices) override
        {
            if (this->ValidIndices(numObservations, observations, numIndices, indices))
            {
                int s, i0, j0, k0, i1, j1, k1;

                // Compute the powers of x and y.
                int numSamples = static_cast<int>(numIndices);
                int twoXDegree = 2 * mXDegree;
                int twoYDegree = 2 * mYDegree;
                Array2<Real> xPower(twoXDegree + 1, numSamples);
                Array2<Real> yPower(twoYDegree + 1, numSamples);
                for (s = 0; s < numSamples; ++s)
                {
                    Real x = observations[indices[s]][0];
                    Real y = observations[indices[s]][1];
                    mXDomain[0] = std::min(x, mXDomain[0]);
                    mXDomain[1] = std::max(x, mXDomain[1]);
                    mYDomain[0] = std::min(y, mYDomain[0]);
                    mYDomain[1] = std::max(y, mYDomain[1]);

                    xPower[s][0] = (Real)1;
                    for (i0 = 1; i0 <= twoXDegree; ++i0)
                    {
                        xPower[s][i0] = x * xPower[s][i0 - 1];
                    }

                    yPower[s][0] = (Real)1;
                    for (j0 = 1; j0 <= twoYDegree; ++j0)
                    {
                        yPower[s][j0] = y * yPower[s][j0 - 1];
                    }
                }

                // Matrix A is the Vandermonde matrix and vector B is the
                // right-hand side of the linear system A*X = B.
                GMatrix<Real> A(mSize, mSize);
                GVector<Real> B(mSize);
                for (j0 = 0; j0 <= mYDegree; ++j0)
                {
                    for (i0 = 0; i0 <= mXDegree; ++i0)
                    {
                        Real sum = (Real)0;
                        k0 = i0 + mXDegreeP1 * j0;
                        for (s = 0; s < numSamples; ++s)
                        {
                            Real w = observations[indices[s]][2];
                            sum += w * xPower[s][i0] * yPower[s][j0];
                        }

                        B[k0] = sum;

                        for (j1 = 0; j1 <= mYDegree; ++j1)
                        {
                            for (i1 = 0; i1 <= mXDegree; ++i1)
                            {
                                sum = (Real)0;
                                k1 = i1 + mXDegreeP1 * j1;
                                for (s = 0; s < numSamples; ++s)
                                {
                                    sum += xPower[s][i0 + i1] * yPower[s][j0 + j1];
                                }

                                A(k0, k1) = sum;
                            }
                        }
                    }
                }

                // Solve for the polynomial coefficients.
                GVector<Real> coefficients = Inverse(A) * B;
                bool hasNonzero = false;
                for (int i = 0; i < mSize; ++i)
                {
                    mParameters[i] = coefficients[i];
                    if (coefficients[i] != (Real)0)
                    {
                        hasNonzero = true;
                    }
                }
                return hasNonzero;
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
            return static_cast<size_t>(mSize);
        }

        // Compute the model error for the specified observation for the
        // current model parameters. The returned value for observation
        // (x0,y0,w0) is |w(x0,y0) - w0|, where w(x,y) is the fitted
        // polynomial.
        virtual Real Error(std::array<Real, 3> const& observation) const override
        {
            Real w = Evaluate(observation[0], observation[1]);
            Real error = std::fabs(w - observation[2]);
            return error;
        }

        virtual void CopyParameters(ApprQuery<Real, std::array<Real, 3>> const* input) override
        {
            auto source = dynamic_cast<ApprPolynomial3 const*>(input);
            if (source)
            {
                *this = *source;
            }
        }

        // Evaluate the polynomial. The domain intervals are provided so you
        // can interpolate ((x,y) in domain) or extrapolate ((x,y) not in
        // domain).
        std::array<Real, 2> const& GetXDomain() const
        {
            return mXDomain;
        }

        std::array<Real, 2> const& GetYDomain() const
        {
            return mYDomain;
        }

        Real Evaluate(Real x, Real y) const
        {
            int i0, i1;
            Real w;

            for (i1 = 0; i1 <= mYDegree; ++i1)
            {
                i0 = mXDegree;
                w = mParameters[i0 + mXDegreeP1 * i1];
                while (--i0 >= 0)
                {
                    w = mParameters[i0 + mXDegreeP1 * i1] + w * x;
                }
                mYCoefficient[i1] = w;
            }

            i1 = mYDegree;
            w = mYCoefficient[i1];
            while (--i1 >= 0)
            {
                w = mYCoefficient[i1] + w * y;
            }

            return w;
        }

    private:
        int mXDegree, mYDegree, mXDegreeP1, mYDegreeP1, mSize;
        std::array<Real, 2> mXDomain, mYDomain;
        std::vector<Real> mParameters;

        // This array is used by Evaluate() to avoid reallocation of the
        // 'vector' for each call. The member is mutable because, to the
        // user, the call to Evaluate does not modify the polynomial.
        mutable std::vector<Real> mYCoefficient;
    };

}
