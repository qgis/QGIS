// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <array>

// The interpolator is for uniformly spaced (x,y)-values.  The input samples
// F must be stored in row-major order to represent f(x,y); that is,
// F[c + xBound*r] corresponds to f(x,y), where c is the index corresponding
// to x and r is the index corresponding to y.  Exact interpolation is
// achieved by setting catmullRom to 'true', giving you the Catmull-Rom
// blending matrix.  If a smooth interpolation is desired, set catmullRom to
// 'false' to obtain B-spline blending.

namespace gte
{
    template <typename Real>
    class IntpBicubic2
    {
    public:
        // Construction.
        IntpBicubic2(int xBound, int yBound, Real xMin, Real xSpacing,
            Real yMin, Real ySpacing, Real const* F, bool catmullRom)
            :
            mXBound(xBound),
            mYBound(yBound),
            mQuantity(xBound* yBound),
            mXMin(xMin),
            mXSpacing(xSpacing),
            mYMin(yMin),
            mYSpacing(ySpacing),
            mF(F)
        {
            // At least a 3x3 block of data points are needed to construct the
            // estimates of the boundary derivatives.
            LogAssert(mXBound >= 3 && mYBound >= 3 && mF != nullptr, "Invalid input.");
            LogAssert(mXSpacing > (Real)0 && mYSpacing > (Real)0, "Invalid input.");

            mXMax = mXMin + mXSpacing * static_cast<Real>(mXBound - 1);
            mInvXSpacing = (Real)1 / mXSpacing;
            mYMax = mYMin + mYSpacing * static_cast<Real>(mYBound - 1);
            mInvYSpacing = (Real)1 / mYSpacing;

            if (catmullRom)
            {
                mBlend[0][0] = (Real)0;
                mBlend[0][1] = (Real)-0.5;
                mBlend[0][2] = (Real)1;
                mBlend[0][3] = (Real)-0.5;
                mBlend[1][0] = (Real)1;
                mBlend[1][1] = (Real)0;
                mBlend[1][2] = (Real)-2.5;
                mBlend[1][3] = (Real)1.5;
                mBlend[2][0] = (Real)0;
                mBlend[2][1] = (Real)0.5;
                mBlend[2][2] = (Real)2;
                mBlend[2][3] = (Real)-1.5;
                mBlend[3][0] = (Real)0;
                mBlend[3][1] = (Real)0;
                mBlend[3][2] = (Real)-0.5;
                mBlend[3][3] = (Real)0.5;
            }
            else
            {
                mBlend[0][0] = (Real)1 / (Real)6;
                mBlend[0][1] = (Real)-3 / (Real)6;
                mBlend[0][2] = (Real)3 / (Real)6;
                mBlend[0][3] = (Real)-1 / (Real)6;;
                mBlend[1][0] = (Real)4 / (Real)6;
                mBlend[1][1] = (Real)0 / (Real)6;
                mBlend[1][2] = (Real)-6 / (Real)6;
                mBlend[1][3] = (Real)3 / (Real)6;
                mBlend[2][0] = (Real)1 / (Real)6;
                mBlend[2][1] = (Real)3 / (Real)6;
                mBlend[2][2] = (Real)3 / (Real)6;
                mBlend[2][3] = (Real)-3 / (Real)6;
                mBlend[3][0] = (Real)0 / (Real)6;
                mBlend[3][1] = (Real)0 / (Real)6;
                mBlend[3][2] = (Real)0 / (Real)6;
                mBlend[3][3] = (Real)1 / (Real)6;
            }
        }

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
            // Compute x-index and clamp to image.
            Real xIndex = (x - mXMin) * mInvXSpacing;
            int ix = static_cast<int>(xIndex);
            if (ix < 0)
            {
                ix = 0;
            }
            else if (ix >= mXBound)
            {
                ix = mXBound - 1;
            }

            // Compute y-index and clamp to image.
            Real yIndex = (y - mYMin) * mInvYSpacing;
            int iy = static_cast<int>(yIndex);
            if (iy < 0)
            {
                iy = 0;
            }
            else if (iy >= mYBound)
            {
                iy = mYBound - 1;
            }

            std::array<Real, 4> U;
            U[0] = (Real)1;
            U[1] = xIndex - ix;
            U[2] = U[1] * U[1];
            U[3] = U[1] * U[2];

            std::array<Real, 4> V;
            V[0] = (Real)1;
            V[1] = yIndex - iy;
            V[2] = V[1] * V[1];
            V[3] = V[1] * V[2];

            // Compute P = M*U and Q = M*V.
            std::array<Real, 4> P, Q;
            for (int row = 0; row < 4; ++row)
            {
                P[row] = (Real)0;
                Q[row] = (Real)0;
                for (int col = 0; col < 4; ++col)
                {
                    P[row] += mBlend[row][col] * U[col];
                    Q[row] += mBlend[row][col] * V[col];
                }
            }

            // Compute (M*U)^t D (M*V) where D is the 4x4 subimage 
            // containing (x,y).
            --ix;
            --iy;
            Real result = (Real)0;
            for (int row = 0; row < 4; ++row)
            {
                int yClamp = iy + row;
                if (yClamp < 0)
                {
                    yClamp = 0;
                }
                else if (yClamp > mYBound - 1)
                {
                    yClamp = mYBound - 1;
                }

                for (int col = 0; col < 4; ++col)
                {
                    int xClamp = ix + col;
                    if (xClamp < 0)
                    {
                        xClamp = 0;
                    }
                    else if (xClamp > mXBound - 1)
                    {
                        xClamp = mXBound - 1;
                    }

                    result += P[col] * Q[row] * mF[xClamp + mXBound * yClamp];
                }
            }

            return result;
        }

        Real operator()(int xOrder, int yOrder, Real x, Real y) const
        {
            // Compute x-index and clamp to image.
            Real xIndex = (x - mXMin) * mInvXSpacing;
            int ix = static_cast<int>(xIndex);
            if (ix < 0)
            {
                ix = 0;
            }
            else if (ix >= mXBound)
            {
                ix = mXBound - 1;
            }

            // Compute y-index and clamp to image.
            Real yIndex = (y - mYMin) * mInvYSpacing;
            int iy = static_cast<int>(yIndex);
            if (iy < 0)
            {
                iy = 0;
            }
            else if (iy >= mYBound)
            {
                iy = mYBound - 1;
            }

            std::array<Real, 4> U;
            Real dx, xMult;
            switch (xOrder)
            {
            case 0:
                dx = xIndex - ix;
                U[0] = (Real)1;
                U[1] = dx;
                U[2] = dx * U[1];
                U[3] = dx * U[2];
                xMult = (Real)1;
                break;
            case 1:
                dx = xIndex - ix;
                U[0] = (Real)0;
                U[1] = (Real)1;
                U[2] = (Real)2 * dx;
                U[3] = (Real)3 * dx * dx;
                xMult = mInvXSpacing;
                break;
            case 2:
                dx = xIndex - ix;
                U[0] = (Real)0;
                U[1] = (Real)0;
                U[2] = (Real)2;
                U[3] = (Real)6 * dx;
                xMult = mInvXSpacing * mInvXSpacing;
                break;
            case 3:
                U[0] = (Real)0;
                U[1] = (Real)0;
                U[2] = (Real)0;
                U[3] = (Real)6;
                xMult = mInvXSpacing * mInvXSpacing * mInvXSpacing;
                break;
            default:
                return (Real)0;
            }

            std::array<Real, 4> V;
            Real dy, yMult;
            switch (yOrder)
            {
            case 0:
                dy = yIndex - iy;
                V[0] = (Real)1;
                V[1] = dy;
                V[2] = dy * V[1];
                V[3] = dy * V[2];
                yMult = (Real)1;
                break;
            case 1:
                dy = yIndex - iy;
                V[0] = (Real)0;
                V[1] = (Real)1;
                V[2] = (Real)2 * dy;
                V[3] = (Real)3 * dy * dy;
                yMult = mInvYSpacing;
                break;
            case 2:
                dy = yIndex - iy;
                V[0] = (Real)0;
                V[1] = (Real)0;
                V[2] = (Real)2;
                V[3] = (Real)6 * dy;
                yMult = mInvYSpacing * mInvYSpacing;
                break;
            case 3:
                V[0] = (Real)0;
                V[1] = (Real)0;
                V[2] = (Real)0;
                V[3] = (Real)6;
                yMult = mInvYSpacing * mInvYSpacing * mInvYSpacing;
                break;
            default:
                return (Real)0;
            }

            // Compute P = M*U and Q = M*V.
            std::array<Real, 4> P, Q;
            for (int row = 0; row < 4; ++row)
            {
                P[row] = (Real)0;
                Q[row] = (Real)0;
                for (int col = 0; col < 4; ++col)
                {
                    P[row] += mBlend[row][col] * U[col];
                    Q[row] += mBlend[row][col] * V[col];
                }
            }

            // Compute (M*U)^t D (M*V) where D is the 4x4 subimage containing (x,y).
            --ix;
            --iy;
            Real result = (Real)0;
            for (int row = 0; row < 4; ++row)
            {
                int yClamp = iy + row;
                if (yClamp < 0)
                {
                    yClamp = 0;
                }
                else if (yClamp > mYBound - 1)
                {
                    yClamp = mYBound - 1;
                }

                for (int col = 0; col < 4; ++col)
                {
                    int xClamp = ix + col;
                    if (xClamp < 0)
                    {
                        xClamp = 0;
                    }
                    else if (xClamp > mXBound - 1)
                    {
                        xClamp = mXBound - 1;
                    }

                    result += P[col] * Q[row] * mF[xClamp + mXBound * yClamp];
                }
            }
            result *= xMult * yMult;

            return result;
        }

    private:
        int mXBound, mYBound, mQuantity;
        Real mXMin, mXMax, mXSpacing, mInvXSpacing;
        Real mYMin, mYMax, mYSpacing, mInvYSpacing;
        Real const* mF;
        std::array<std::array<Real, 4>, 4> mBlend;
    };
}
