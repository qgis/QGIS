// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/GMatrix.h>
#include <array>

// WARNING.  The implementation allows you to transform the inputs (x,y) to
// the unit square and perform the interpolation in that space.  The idea is
// to keep the floating-point numbers to order 1 for numerical stability of
// the algorithm.  The classical thin-plate spline algorithm does not include
// this transformation.  The interpolation is invariant to translations and
// rotations of (x,y) but not to scaling.  The following document is about
// thin plate splines.
//   https://www.geometrictools.com/Documentation/ThinPlateSplines.pdf

namespace gte
{
    template <typename Real>
    class IntpThinPlateSpline2
    {
    public:
        // Construction.  Data points are (x,y,f(x,y)).  The smoothing
        // parameter must be nonnegative.
        IntpThinPlateSpline2(int numPoints, Real const* X, Real const* Y,
            Real const* F, Real smooth, bool transformToUnitSquare)
            :
            mNumPoints(numPoints),
            mX(numPoints),
            mY(numPoints),
            mSmooth(smooth),
            mA(numPoints),
            mInitialized(false)
        {
            LogAssert(numPoints >= 3 && X != nullptr && Y != nullptr &&
                F != nullptr && smooth >= (Real)0, "Invalid input.");

            int i, row, col;

            if (transformToUnitSquare)
            {
                // Map input (x,y) to unit square.  This is not part of the
                // classical thin-plate spline algorithm because the
                // interpolation is not invariant to scalings.
                auto extreme = std::minmax_element(X, X + mNumPoints);
                mXMin = *extreme.first;
                mXMax = *extreme.second;
                mXInvRange = (Real)1 / (mXMax - mXMin);
                for (i = 0; i < mNumPoints; ++i)
                {
                    mX[i] = (X[i] - mXMin) * mXInvRange;
                }

                extreme = std::minmax_element(Y, Y + mNumPoints);
                mYMin = *extreme.first;
                mYMax = *extreme.second;
                mYInvRange = (Real)1 / (mYMax - mYMin);
                for (i = 0; i < mNumPoints; ++i)
                {
                    mY[i] = (Y[i] - mYMin) * mYInvRange;
                }
            }
            else
            {
                // The classical thin-plate spline uses the data as is.  The
                // values mXMax and mYMax are not used, but they are
                // initialized anyway (to irrelevant numbers).
                mXMin = (Real)0;
                mXMax = (Real)1;
                mXInvRange = (Real)1;
                mYMin = (Real)0;
                mYMax = (Real)1;
                mYInvRange = (Real)1;
                std::copy(X, X + mNumPoints, mX.begin());
                std::copy(Y, Y + mNumPoints, mY.begin());
            }

            // Compute matrix A = M + lambda*I [NxN matrix].
            GMatrix<Real> AMat(mNumPoints, mNumPoints);
            for (row = 0; row < mNumPoints; ++row)
            {
                for (col = 0; col < mNumPoints; ++col)
                {
                    if (row == col)
                    {
                        AMat(row, col) = mSmooth;
                    }
                    else
                    {
                        Real dx = mX[row] - mX[col];
                        Real dy = mY[row] - mY[col];
                        Real t = std::sqrt(dx * dx + dy * dy);
                        AMat(row, col) = Kernel(t);
                    }
                }
            }

            // Compute matrix B [Nx3 matrix].
            GMatrix<Real> BMat(mNumPoints, 3);
            for (row = 0; row < mNumPoints; ++row)
            {
                BMat(row, 0) = (Real)1;
                BMat(row, 1) = mX[row];
                BMat(row, 2) = mY[row];
            }

            // Compute A^{-1}.
            bool invertible;
            GMatrix<Real> invAMat = Inverse(AMat, &invertible);
            if (!invertible)
            {
                return;
            }

            // Compute P = B^T A^{-1}  [3xN matrix].
            GMatrix<Real> PMat = MultiplyATB(BMat, invAMat);

            // Compute Q = P B = B^T A^{-1} B  [3x3 matrix].
            GMatrix<Real> QMat = PMat * BMat;

            // Compute Q^{-1}.
            GMatrix<Real> invQMat = Inverse(QMat, &invertible);
            if (!invertible)
            {
                return;
            }

            // Compute P*z.
            std::array<Real, 3> prod;
            for (row = 0; row < 3; ++row)
            {
                prod[row] = (Real)0;
                for (i = 0; i < mNumPoints; ++i)
                {
                    prod[row] += PMat(row, i) * F[i];
                }
            }

            // Compute 'b' vector for smooth thin plate spline.
            for (row = 0; row < 3; ++row)
            {
                mB[row] = (Real)0;
                for (i = 0; i < 3; ++i)
                {
                    mB[row] += invQMat(row, i) * prod[i];
                }
            }

            // Compute z-B*b.
            std::vector<Real> tmp(mNumPoints);
            for (row = 0; row < mNumPoints; ++row)
            {
                tmp[row] = F[row];
                for (i = 0; i < 3; ++i)
                {
                    tmp[row] -= BMat(row, i) * mB[i];
                }
            }

            // Compute 'a' vector for smooth thin plate spline.
            for (row = 0; row < mNumPoints; ++row)
            {
                mA[row] = (Real)0;
                for (i = 0; i < mNumPoints; ++i)
                {
                    mA[row] += invAMat(row, i) * tmp[i];
                }
            }

            mInitialized = true;
        }

        // Check this after the constructor call to see whether the thin plate
        // spline coefficients were successfully computed.  If so, then calls
        // to operator()(Real,Real) will work properly.  TODO:  This needs to
        // be removed because the constructor now throws exceptions?
        inline bool IsInitialized() const
        {
            return mInitialized;
        }

        // Evaluate the interpolator.  If IsInitialized() returns 'false', the
        // operator will return std::numeric_limits<Real>::max().
        Real operator()(Real x, Real y) const
        {
            if (mInitialized)
            {
                // Map (x,y) to the unit square.
                x = (x - mXMin) * mXInvRange;
                y = (y - mYMin) * mYInvRange;

                Real result = mB[0] + mB[1] * x + mB[2] * y;
                for (int i = 0; i < mNumPoints; ++i)
                {
                    Real dx = x - mX[i];
                    Real dy = y - mY[i];
                    Real t = std::sqrt(dx * dx + dy * dy);
                    result += mA[i] * Kernel(t);
                }
                return result;
            }

            return std::numeric_limits<Real>::max();
        }

        // Compute the functional value a^T*M*a when lambda is zero or
        // lambda*w^T*(M+lambda*I)*w when lambda is positive.  See the thin
        // plate splines PDF for a description of these quantities.
        Real ComputeFunctional() const
        {
            Real functional = (Real)0;
            for (int row = 0; row < mNumPoints; ++row)
            {
                for (int col = 0; col < mNumPoints; ++col)
                {
                    if (row == col)
                    {
                        functional += mSmooth * mA[row] * mA[col];
                    }
                    else
                    {
                        Real dx = mX[row] - mX[col];
                        Real dy = mY[row] - mY[col];
                        Real t = std::sqrt(dx * dx + dy * dy);
                        functional += Kernel(t) * mA[row] * mA[col];
                    }
                }
            }

            if (mSmooth > (Real)0)
            {
                functional *= mSmooth;
            }

            return functional;
        }

    private:
        // Kernel(t) = t^2 * log(t^2)
        static Real Kernel(Real t)
        {
            if (t > (Real)0)
            {
                Real t2 = t * t;
                return t2 * std::log(t2);
            }
            return (Real)0;
        }

        // Input data.
        int mNumPoints;
        std::vector<Real> mX;
        std::vector<Real> mY;
        Real mSmooth;

        // Thin plate spline coefficients. The A[] coefficients are associated
        // with the Green's functions G(x,y,*) and the B[] coefficients are
        // associated with the affine term B[0] + B[1]*x + B[2]*y.
        std::vector<Real> mA;  // mNumPoints elements
        std::array<Real, 3> mB;

        // Extent of input data.
        Real mXMin, mXMax, mXInvRange;
        Real mYMin, mYMax, mYInvRange;

        bool mInitialized;
    };
}
