// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.12.05

#pragma once

#include <Mathematics/ApprQuery.h>
#include <Mathematics/Array2.h>
#include <Mathematics/GMatrix.h>
#include <array>

// The samples are (x[i],y[i],z[i],w[i]) for 0 <= i < S. Think of w as a
// function of x, y, and z, say w = f(x,y,z). The function fits the samples
// with a polynomial of degree d0 in x, degree d1 in y, and degree d2 in z,
// say
//   w = sum_{i=0}^{d0} sum_{j=0}^{d1} sum_{k=0}^{d2} c[i][j][k]*x^i*y^j*z^k
// The method is a least-squares fitting algorithm. The mParameters stores
// c[i][j][k] = mParameters[i+(d0+1)*(j+(d1+1)*k)] for a total of
// (d0+1)*(d1+1)*(d2+1) coefficients. The observation type is
// std::array<Real,4>, which represents a tuple (x,y,z,w).
//
// WARNING. The fitting algorithm for polynomial terms
//   (1,x,x^2,...,x^d0), (1,y,y^2,...,y^d1), (1,z,z^2,...,z^d2)
// is known to be nonrobust for large degrees and for large magnitude data.
// One alternative is to use orthogonal polynomials
//   (f[0](x),...,f[d0](x)), (g[0](y),...,g[d1](y)), (h[0](z),...,h[d2](z))
// and apply the least-squares algorithm to these. Another alternative is to
// transform
//   (x',y',z',w') = ((x-xcen)/rng, (y-ycen)/rng, (z-zcen)/rng, w/rng)
// where xmin = min(x[i]), xmax = max(x[i]), xcen = (xmin+xmax)/2,
// ymin = min(y[i]), ymax = max(y[i]), ycen = (ymin+ymax)/2, zmin = min(z[i]),
// zmax = max(z[i]), zcen = (zmin+zmax)/2, and
// rng = max(xmax-xmin,ymax-ymin,zmax-zmin). Fit the (x',y',z',w') points,
//   w' = sum_{i=0}^{d0} sum_{j=0}^{d1} sum_{k=0}^{d2} c'[i][j][k] *
//          (x')^i*(y')^j*(z')^k
// The original polynomial is evaluated as
//   w = rng * sum_{i=0}^{d0} sum_{j=0}^{d1} sum_{k=0}^{d2} c'[i][j][k] *
//       ((x-xcen)/rng)^i * ((y-ycen)/rng)^j * ((z-zcen)/rng)^k

namespace gte
{
    template <typename Real>
    class ApprPolynomial4 : public ApprQuery<Real, std::array<Real, 4>>
    {
    public:
        // Initialize the model parameters to zero.
        ApprPolynomial4(int xDegree, int yDegree, int zDegree)
            :
            mXDegree(xDegree),
            mYDegree(yDegree),
            mZDegree(zDegree),
            mXDegreeP1(xDegree + 1),
            mYDegreeP1(yDegree + 1),
            mZDegreeP1(zDegree + 1),
            mSize(mXDegreeP1* mYDegreeP1* mZDegreeP1),
            mParameters(mSize, (Real)0),
            mYZCoefficient(mYDegreeP1 * mZDegreeP1, (Real)0),
            mZCoefficient(mZDegreeP1, (Real)0)
        {
            mXDomain[0] = std::numeric_limits<Real>::max();
            mXDomain[1] = -mXDomain[0];
            mYDomain[0] = std::numeric_limits<Real>::max();
            mYDomain[1] = -mYDomain[0];
            mZDomain[0] = std::numeric_limits<Real>::max();
            mZDomain[1] = -mZDomain[0];
        }

        // Basic fitting algorithm. See ApprQuery.h for the various Fit(...)
        // functions that you can call.
        virtual bool FitIndexed(
            size_t numObservations, std::array<Real, 4> const* observations,
            size_t numIndices, int const* indices) override
        {
            if (this->ValidIndices(numObservations, observations, numIndices, indices))
            {
                int s, i0, j0, k0, n0, i1, j1, k1, n1;

                // Compute the powers of x, y, and z.
                int numSamples = static_cast<int>(numIndices);
                int twoXDegree = 2 * mXDegree;
                int twoYDegree = 2 * mYDegree;
                int twoZDegree = 2 * mZDegree;
                Array2<Real> xPower(twoXDegree + 1, numSamples);
                Array2<Real> yPower(twoYDegree + 1, numSamples);
                Array2<Real> zPower(twoZDegree + 1, numSamples);
                for (s = 0; s < numSamples; ++s)
                {
                    Real x = observations[indices[s]][0];
                    Real y = observations[indices[s]][1];
                    Real z = observations[indices[s]][2];
                    mXDomain[0] = std::min(x, mXDomain[0]);
                    mXDomain[1] = std::max(x, mXDomain[1]);
                    mYDomain[0] = std::min(y, mYDomain[0]);
                    mYDomain[1] = std::max(y, mYDomain[1]);
                    mZDomain[0] = std::min(z, mZDomain[0]);
                    mZDomain[1] = std::max(z, mZDomain[1]);

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

                    zPower[s][0] = (Real)1;
                    for (k0 = 1; k0 <= twoZDegree; ++k0)
                    {
                        zPower[s][k0] = z * zPower[s][k0 - 1];
                    }
                }

                // Matrix A is the Vandermonde matrix and vector B is the
                // right-hand side of the linear system A*X = B.
                GMatrix<Real> A(mSize, mSize);
                GVector<Real> B(mSize);
                for (k0 = 0; k0 <= mZDegree; ++k0)
                {
                    for (j0 = 0; j0 <= mYDegree; ++j0)
                    {
                        for (i0 = 0; i0 <= mXDegree; ++i0)
                        {
                            Real sum = (Real)0;
                            n0 = i0 + mXDegreeP1 * (j0 + mYDegreeP1 * k0);
                            for (s = 0; s < numSamples; ++s)
                            {
                                Real w = observations[indices[s]][3];
                                sum += w * xPower[s][i0] * yPower[s][j0] * zPower[s][k0];
                            }

                            B[n0] = sum;

                            for (k1 = 0; k1 <= mZDegree; ++k1)
                            {
                                for (j1 = 0; j1 <= mYDegree; ++j1)
                                {
                                    for (i1 = 0; i1 <= mXDegree; ++i1)
                                    {
                                        sum = (Real)0;
                                        n1 = i1 + mXDegreeP1 * (j1 + mYDegreeP1 * k1);
                                        for (s = 0; s < numSamples; ++s)
                                        {
                                            sum += xPower[s][i0 + i1] * yPower[s][j0 + j1] * zPower[s][k0 + k1];
                                        }

                                        A(n0, n1) = sum;
                                    }
                                }
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
            auto source = dynamic_cast<ApprPolynomial4 const*>(input);
            if (source)
            {
                *this = *source;
            }
        }

        // Evaluate the polynomial. The domain intervals are provided so you
        // can interpolate ((x,y,z) in domain) or extrapolate ((x,y,z) not in
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
            int i0, i1, i2;
            Real w;

            for (i2 = 0; i2 <= mZDegree; ++i2)
            {
                for (i1 = 0; i1 <= mYDegree; ++i1)
                {
                    i0 = mXDegree;
                    w = mParameters[i0 + mXDegreeP1 * (i1 + mYDegreeP1 * i2)];
                    while (--i0 >= 0)
                    {
                        w = mParameters[i0 + mXDegreeP1 * (i1 + mYDegreeP1 * i2)] + w * x;
                    }
                    mYZCoefficient[i1 + mYDegree * i2] = w;
                }
            }

            for (i2 = 0; i2 <= mZDegree; ++i2)
            {
                i1 = mYDegree;
                w = mYZCoefficient[i1 + mYDegreeP1 * i2];
                while (--i1 >= 0)
                {
                    w = mParameters[i1 + mYDegreeP1 * i2] + w * y;
                }
                mZCoefficient[i2] = w;
            }

            i2 = mZDegree;
            w = mZCoefficient[i2];
            while (--i2 >= 0)
            {
                w = mZCoefficient[i2] + w * z;
            }

            return w;
        }

    private:
        int mXDegree, mYDegree, mZDegree;
        int mXDegreeP1, mYDegreeP1, mZDegreeP1, mSize;
        std::array<Real, 2> mXDomain, mYDomain, mZDomain;
        std::vector<Real> mParameters;

        // These arrays are used by Evaluate() to avoid reallocation of the
        // 'vector's for each call. The member is mutable because, to the
        // user, the call to Evaluate does not modify the polynomial.
        mutable std::vector<Real> mYZCoefficient;
        mutable std::vector<Real> mZCoefficient;
    };
}
