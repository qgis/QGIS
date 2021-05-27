// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <Mathematics/Array2.h>
#include <algorithm>
#include <array>
#include <cstring>

// The interpolator is for uniformly spaced (x,y)-values.  The input samples
// F must be stored in row-major order to represent f(x,y); that is,
// F[c + xBound*r] corresponds to f(x,y), where c is the index corresponding
// to x and r is the index corresponding to y.

namespace gte
{
    template <typename Real>
    class IntpAkimaUniform2
    {
    public:
        // Construction and destruction.
        IntpAkimaUniform2(int xBound, int yBound, Real xMin, Real xSpacing,
            Real yMin, Real ySpacing, Real const* F)
            :
            mXBound(xBound),
            mYBound(yBound),
            mQuantity(xBound* yBound),
            mXMin(xMin),
            mXSpacing(xSpacing),
            mYMin(yMin),
            mYSpacing(ySpacing),
            mF(F),
            mPoly(xBound - 1, mYBound - 1)
        {
            // At least a 3x3 block of data points is needed to construct the
            // estimates of the boundary derivatives.
            LogAssert(mXBound >= 3 && mYBound >= 3 && mF != nullptr, "Invalid input.");
            LogAssert(mXSpacing > (Real)0 && mYSpacing > (Real)0, "Invalid input.");

            mXMax = mXMin + mXSpacing * static_cast<Real>(mXBound - 1);
            mYMax = mYMin + mYSpacing * static_cast<Real>(mYBound - 1);

            // Create a 2D wrapper for the 1D samples.
            Array2<Real> Fmap(mXBound, mYBound, const_cast<Real*>(mF));

            // Construct first-order derivatives.
            Array2<Real> FX(mXBound, mYBound), FY(mXBound, mYBound);
            GetFX(Fmap, FX);
            GetFY(Fmap, FY);

            // Construct second-order derivatives.
            Array2<Real> FXY(mXBound, mYBound);
            GetFXY(Fmap, FXY);

            // Construct polynomials.
            GetPolynomials(Fmap, FX, FY, FXY);
        }

        ~IntpAkimaUniform2() = default;

        // Member access.
        inline int GetXBound() const
        {
            return mXBound;
        }

        inline int GetYBound() const
        {
            return mYBound;
        }

        inline int GetQuantity() const
        {
            return mQuantity;
        }

        inline Real const* GetF() const
        {
            return mF;
        }

        inline Real GetXMin() const
        {
            return mXMin;
        }

        inline Real GetXMax() const
        {
            return mXMax;
        }

        inline Real GetXSpacing() const
        {
            return mXSpacing;
        }

        inline Real GetYMin() const
        {
            return mYMin;
        }

        inline Real GetYMax() const
        {
            return mYMax;
        }

        inline Real GetYSpacing() const
        {
            return mYSpacing;
        }

        // Evaluate the function and its derivatives.  The functions clamp the
        // inputs to xmin <= x <= xmax and ymin <= y <= ymax.  The first
        // operator is for function evaluation.  The second operator is for
        // function or derivative evaluations.  The xOrder argument is the
        // order of the x-derivative and the yOrder argument is the order of
        // the y-derivative.  Both orders are zero to get the function value
        // itself.
        Real operator()(Real x, Real y) const
        {
            x = std::min(std::max(x, mXMin), mXMax);
            y = std::min(std::max(y, mYMin), mYMax);
            int ix, iy;
            Real dx, dy;
            XLookup(x, ix, dx);
            YLookup(y, iy, dy);
            return mPoly[iy][ix](dx, dy);
        }

        Real operator()(int xOrder, int yOrder, Real x, Real y) const
        {
            x = std::min(std::max(x, mXMin), mXMax);
            y = std::min(std::max(y, mYMin), mYMax);
            int ix, iy;
            Real dx, dy;
            XLookup(x, ix, dx);
            YLookup(y, iy, dy);
            return mPoly[iy][ix](xOrder, yOrder, dx, dy);
        }

    private:
        class Polynomial
        {
        public:
            Polynomial()
            {
                for (size_t i = 0; i < 4; ++i)
                {
                    mCoeff[i].fill((Real)0);
                }
            }

            // P(x,y) = (1,x,x^2,x^3)*A*(1,y,y^2,y^3).  The matrix term A[ix][iy]
            // corresponds to the polynomial term x^{ix} y^{iy}.
            Real& A(int ix, int iy)
            {
                return mCoeff[ix][iy];
            }

            Real operator()(Real x, Real y) const
            {
                std::array<Real, 4> B;
                for (int i = 0; i <= 3; ++i)
                {
                    B[i] = mCoeff[i][0] + y * (mCoeff[i][1] + y * (mCoeff[i][2] + y * mCoeff[i][3]));
                }

                return B[0] + x * (B[1] + x * (B[2] + x * B[3]));
            }

            Real operator()(int xOrder, int yOrder, Real x, Real y) const
            {
                std::array<Real, 4> xPow;
                switch (xOrder)
                {
                case 0:
                    xPow[0] = (Real)1;
                    xPow[1] = x;
                    xPow[2] = x * x;
                    xPow[3] = x * x * x;
                    break;
                case 1:
                    xPow[0] = (Real)0;
                    xPow[1] = (Real)1;
                    xPow[2] = (Real)2 * x;
                    xPow[3] = (Real)3 * x * x;
                    break;
                case 2:
                    xPow[0] = (Real)0;
                    xPow[1] = (Real)0;
                    xPow[2] = (Real)2;
                    xPow[3] = (Real)6 * x;
                    break;
                case 3:
                    xPow[0] = (Real)0;
                    xPow[1] = (Real)0;
                    xPow[2] = (Real)0;
                    xPow[3] = (Real)6;
                    break;
                default:
                    return (Real)0;
                }

                std::array<Real, 4> yPow;
                switch (yOrder)
                {
                case 0:
                    yPow[0] = (Real)1;
                    yPow[1] = y;
                    yPow[2] = y * y;
                    yPow[3] = y * y * y;
                    break;
                case 1:
                    yPow[0] = (Real)0;
                    yPow[1] = (Real)1;
                    yPow[2] = (Real)2 * y;
                    yPow[3] = (Real)3 * y * y;
                    break;
                case 2:
                    yPow[0] = (Real)0;
                    yPow[1] = (Real)0;
                    yPow[2] = (Real)2;
                    yPow[3] = (Real)6 * y;
                    break;
                case 3:
                    yPow[0] = (Real)0;
                    yPow[1] = (Real)0;
                    yPow[2] = (Real)0;
                    yPow[3] = (Real)6;
                    break;
                default:
                    return (Real)0;
                }

                Real p = (Real)0;
                for (size_t iy = 0; iy <= 3; ++iy)
                {
                    for (size_t ix = 0; ix <= 3; ++ix)
                    {
                        p += mCoeff[ix][iy] * xPow[ix] * yPow[iy];
                    }
                }

                return p;
            }

        private:
            std::array<std::array<Real, 4>, 4> mCoeff;
        };

        // Support for construction.
        void GetFX(Array2<Real> const& F, Array2<Real>& FX)
        {
            Array2<Real> slope(mXBound + 3, mYBound);
            Real invDX = (Real)1 / mXSpacing;
            int ix, iy;
            for (iy = 0; iy < mYBound; ++iy)
            {
                for (ix = 0; ix < mXBound - 1; ++ix)
                {
                    slope[iy][ix + 2] = (F[iy][ix + 1] - F[iy][ix]) * invDX;
                }

                slope[iy][1] = (Real)2 * slope[iy][2] - slope[iy][3];
                slope[iy][0] = (Real)2 * slope[iy][1] - slope[iy][2];
                slope[iy][mXBound + 1] = (Real)2 * slope[iy][mXBound] - slope[iy][mXBound - 1];
                slope[iy][mXBound + 2] = (Real)2 * slope[iy][mXBound + 1] - slope[iy][mXBound];
            }

            for (iy = 0; iy < mYBound; ++iy)
            {
                for (ix = 0; ix < mXBound; ++ix)
                {
                    FX[iy][ix] = ComputeDerivative(slope[iy] + ix);
                }
            }
        }

        void GetFY(Array2<Real> const& F, Array2<Real>& FY)
        {
            Array2<Real> slope(mYBound + 3, mXBound);
            Real invDY = (Real)1 / mYSpacing;
            int ix, iy;
            for (ix = 0; ix < mXBound; ++ix)
            {
                for (iy = 0; iy < mYBound - 1; ++iy)
                {
                    slope[ix][iy + 2] = (F[iy + 1][ix] - F[iy][ix]) * invDY;
                }

                slope[ix][1] = (Real)2 * slope[ix][2] - slope[ix][3];
                slope[ix][0] = (Real)2 * slope[ix][1] - slope[ix][2];
                slope[ix][mYBound + 1] = (Real)2 * slope[ix][mYBound] - slope[ix][mYBound - 1];
                slope[ix][mYBound + 2] = (Real)2 * slope[ix][mYBound + 1] - slope[ix][mYBound];
            }

            for (ix = 0; ix < mXBound; ++ix)
            {
                for (iy = 0; iy < mYBound; ++iy)
                {
                    FY[iy][ix] = ComputeDerivative(slope[ix] + iy);
                }
            }
        }

        void GetFXY(Array2<Real> const& F, Array2<Real>& FXY)
        {
            int xBoundM1 = mXBound - 1;
            int yBoundM1 = mYBound - 1;
            int ix0 = xBoundM1, ix1 = ix0 - 1, ix2 = ix1 - 1;
            int iy0 = yBoundM1, iy1 = iy0 - 1, iy2 = iy1 - 1;
            int ix, iy;

            Real invDXDY = (Real)1 / (mXSpacing * mYSpacing);

            // corners
            FXY[0][0] = (Real)0.25 * invDXDY * (
                (Real)9 * F[0][0]
                - (Real)12 * F[0][1]
                + (Real)3 * F[0][2]
                - (Real)12 * F[1][0]
                + (Real)16 * F[1][1]
                - (Real)4 * F[1][2]
                + (Real)3 * F[2][0]
                - (Real)4 * F[2][1]
                + F[2][2]);

            FXY[0][xBoundM1] = (Real)0.25 * invDXDY * (
                (Real)9 * F[0][ix0]
                - (Real)12 * F[0][ix1]
                + (Real)3 * F[0][ix2]
                - (Real)12 * F[1][ix0]
                + (Real)16 * F[1][ix1]
                - (Real)4 * F[1][ix2]
                + (Real)3 * F[2][ix0]
                - (Real)4 * F[2][ix1]
                + F[2][ix2]);

            FXY[yBoundM1][0] = (Real)0.25 * invDXDY * (
                (Real)9 * F[iy0][0]
                - (Real)12 * F[iy0][1]
                + (Real)3 * F[iy0][2]
                - (Real)12 * F[iy1][0]
                + (Real)16 * F[iy1][1]
                - (Real)4 * F[iy1][2]
                + (Real)3 * F[iy2][0]
                - (Real)4 * F[iy2][1]
                + F[iy2][2]);

            FXY[yBoundM1][xBoundM1] = (Real)0.25 * invDXDY * (
                (Real)9 * F[iy0][ix0]
                - (Real)12 * F[iy0][ix1]
                + (Real)3 * F[iy0][ix2]
                - (Real)12 * F[iy1][ix0]
                + (Real)16 * F[iy1][ix1]
                - (Real)4 * F[iy1][ix2]
                + (Real)3 * F[iy2][ix0]
                - (Real)4 * F[iy2][ix1]
                + F[iy2][ix2]);

            // x-edges
            for (ix = 1; ix < xBoundM1; ++ix)
            {
                FXY[0][ix] = (Real)0.25 * invDXDY * (
                    (Real)3 * (F[0][ix - 1] - F[0][ix + 1])
                    - (Real)4 * (F[1][ix - 1] - F[1][ix + 1])
                    + (F[2][ix - 1] - F[2][ix + 1]));

                FXY[yBoundM1][ix] = (Real)0.25 * invDXDY * (
                    (Real)3 * (F[iy0][ix - 1] - F[iy0][ix + 1])
                    - (Real)4 * (F[iy1][ix - 1] - F[iy1][ix + 1])
                    + (F[iy2][ix - 1] - F[iy2][ix + 1]));
            }

            // y-edges
            for (iy = 1; iy < yBoundM1; ++iy)
            {
                FXY[iy][0] = (Real)0.25 * invDXDY * (
                    (Real)3 * (F[iy - 1][0] - F[iy + 1][0])
                    - (Real)4 * (F[iy - 1][1] - F[iy + 1][1])
                    + (F[iy - 1][2] - F[iy + 1][2]));

                FXY[iy][xBoundM1] = (Real)0.25 * invDXDY * (
                    (Real)3 * (F[iy - 1][ix0] - F[iy + 1][ix0])
                    - (Real)4 * (F[iy - 1][ix1] - F[iy + 1][ix1])
                    + (F[iy - 1][ix2] - F[iy + 1][ix2]));
            }

            // interior
            for (iy = 1; iy < yBoundM1; ++iy)
            {
                for (ix = 1; ix < xBoundM1; ++ix)
                {
                    FXY[iy][ix] = (Real)0.25 * invDXDY * (F[iy - 1][ix - 1] -
                        F[iy - 1][ix + 1] - F[iy + 1][ix - 1] + F[iy + 1][ix + 1]);
                }
            }
        }

        void GetPolynomials(Array2<Real> const& F, Array2<Real> const& FX,
            Array2<Real> const& FY, Array2<Real> const& FXY)
        {
            int xBoundM1 = mXBound - 1;
            int yBoundM1 = mYBound - 1;
            for (int iy = 0; iy < yBoundM1; ++iy)
            {
                for (int ix = 0; ix < xBoundM1; ++ix)
                {
                    // Note the 'transposing' of the 2x2 blocks (to match
                    // notation used in the polynomial definition).
                    Real G[2][2] =
                    {
                        { F[iy][ix], F[iy + 1][ix] },
                        { F[iy][ix + 1], F[iy + 1][ix + 1] }
                    };

                    Real GX[2][2] =
                    {
                        { FX[iy][ix], FX[iy + 1][ix] },
                        { FX[iy][ix + 1], FX[iy + 1][ix + 1] }
                    };

                    Real GY[2][2] =
                    {
                        { FY[iy][ix], FY[iy + 1][ix] },
                        { FY[iy][ix + 1], FY[iy + 1][ix + 1] }
                    };

                    Real GXY[2][2] =
                    {
                        { FXY[iy][ix], FXY[iy + 1][ix] },
                        { FXY[iy][ix + 1], FXY[iy + 1][ix + 1] }
                    };

                    Construct(mPoly[iy][ix], G, GX, GY, GXY);
                }
            }
        }

        Real ComputeDerivative(Real const* slope) const
        {
            if (slope[1] != slope[2])
            {
                if (slope[0] != slope[1])
                {
                    if (slope[2] != slope[3])
                    {
                        Real ad0 = std::fabs(slope[3] - slope[2]);
                        Real ad1 = std::fabs(slope[0] - slope[1]);
                        return (ad0 * slope[1] + ad1 * slope[2]) / (ad0 + ad1);
                    }
                    else
                    {
                        return slope[2];
                    }
                }
                else
                {
                    if (slope[2] != slope[3])
                    {
                        return slope[1];
                    }
                    else
                    {
                        return (Real)0.5 * (slope[1] + slope[2]);
                    }
                }
            }
            else
            {
                return slope[1];
            }
        }

        void Construct(Polynomial& poly, Real const F[2][2], Real const FX[2][2],
            Real const FY[2][2], Real const FXY[2][2])
        {
            Real dx = mXSpacing;
            Real dy = mYSpacing;
            Real invDX = (Real)1 / dx, invDX2 = invDX * invDX;
            Real invDY = (Real)1 / dy, invDY2 = invDY * invDY;
            Real b0, b1, b2, b3;

            poly.A(0, 0) = F[0][0];
            poly.A(1, 0) = FX[0][0];
            poly.A(0, 1) = FY[0][0];
            poly.A(1, 1) = FXY[0][0];

            b0 = (F[1][0] - poly(0, 0, dx, (Real)0)) * invDX2;
            b1 = (FX[1][0] - poly(1, 0, dx, (Real)0)) * invDX;
            poly.A(2, 0) = (Real)3 * b0 - b1;
            poly.A(3, 0) = ((Real)-2 * b0 + b1) * invDX;

            b0 = (F[0][1] - poly(0, 0, (Real)0, dy)) * invDY2;
            b1 = (FY[0][1] - poly(0, 1, (Real)0, dy)) * invDY;
            poly.A(0, 2) = (Real)3 * b0 - b1;
            poly.A(0, 3) = ((Real)-2 * b0 + b1) * invDY;

            b0 = (FY[1][0] - poly(0, 1, dx, (Real)0)) * invDX2;
            b1 = (FXY[1][0] - poly(1, 1, dx, (Real)0)) * invDX;
            poly.A(2, 1) = (Real)3 * b0 - b1;
            poly.A(3, 1) = ((Real)-2 * b0 + b1) * invDX;

            b0 = (FX[0][1] - poly(1, 0, (Real)0, dy)) * invDY2;
            b1 = (FXY[0][1] - poly(1, 1, (Real)0, dy)) * invDY;
            poly.A(1, 2) = (Real)3 * b0 - b1;
            poly.A(1, 3) = ((Real)-2 * b0 + b1) * invDY;

            b0 = (F[1][1] - poly(0, 0, dx, dy)) * invDX2 * invDY2;
            b1 = (FX[1][1] - poly(1, 0, dx, dy)) * invDX * invDY2;
            b2 = (FY[1][1] - poly(0, 1, dx, dy)) * invDX2 * invDY;
            b3 = (FXY[1][1] - poly(1, 1, dx, dy)) * invDX * invDY;
            poly.A(2, 2) = (Real)9 * b0 - (Real)3 * b1 - (Real)3 * b2 + b3;
            poly.A(3, 2) = ((Real)-6 * b0 + (Real)3 * b1 + (Real)2 * b2 - b3) * invDX;
            poly.A(2, 3) = ((Real)-6 * b0 + (Real)2 * b1 + (Real)3 * b2 - b3) * invDY;
            poly.A(3, 3) = ((Real)4 * b0 - (Real)2 * b1 - (Real)2 * b2 + b3) * invDX * invDY;
        }

        // Support for evaluation.
        void XLookup(Real x, int& xIndex, Real& dx) const
        {
            for (xIndex = 0; xIndex + 1 < mXBound; ++xIndex)
            {
                if (x < mXMin + mXSpacing * (xIndex + 1))
                {
                    dx = x - (mXMin + mXSpacing * xIndex);
                    return;
                }
            }

            --xIndex;
            dx = x - (mXMin + mXSpacing * xIndex);
        }

        void YLookup(Real y, int& yIndex, Real& dy) const
        {
            for (yIndex = 0; yIndex + 1 < mYBound; ++yIndex)
            {
                if (y < mYMin + mYSpacing * (yIndex + 1))
                {
                    dy = y - (mYMin + mYSpacing * yIndex);
                    return;
                }
            }

            yIndex--;
            dy = y - (mYMin + mYSpacing * yIndex);
        }

        int mXBound, mYBound, mQuantity;
        Real mXMin, mXMax, mXSpacing;
        Real mYMin, mYMax, mYSpacing;
        Real const* mF;
        Array2<Polynomial> mPoly;
    };
}
