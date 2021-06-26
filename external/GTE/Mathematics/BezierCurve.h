// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Array2.h>
#include <Mathematics/ParametricCurve.h>

namespace gte
{
    template <int N, typename Real>
    class BezierCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction and destruction.  The number of control points must be
        // degree + 1.  This object copies the input array.  The domain is t
        // in [0,1].  To validate construction, create an object as shown:
        //     BezierCurve<N, Real> curve(parameters);
        //     if (!curve) { <constructor failed, handle accordingly>; }
        BezierCurve(int degree, Vector<N, Real> const* controls)
            :
            ParametricCurve<N, Real>((Real)0, (Real)1),
            mDegree(degree),
            mNumControls(degree + 1),
            mChoose(mNumControls, mNumControls)
        {
            LogAssert(degree >= 2 && controls != nullptr, "Invalid input.");

            // Copy the controls.
            mControls[0].resize(mNumControls);
            std::copy(controls, controls + mNumControls, mControls[0].begin());

            // Compute first-order differences.
            mControls[1].resize(mNumControls - 1);
            for (int i = 0; i < mNumControls - 1; ++i)
            {
                mControls[1][i] = mControls[0][i + 1] - mControls[0][i];
            }

            // Compute second-order differences.
            mControls[2].resize(mNumControls - 2);
            for (int i = 0; i < mNumControls - 2; ++i)
            {
                mControls[2][i] = mControls[1][i + 1] - mControls[1][i];
            }

            // Compute third-order differences.
            if (degree >= 3)
            {
                mControls[3].resize(mNumControls - 3);
                for (int i = 0; i < mNumControls - 3; ++i)
                {
                    mControls[3][i] = mControls[2][i + 1] - mControls[2][i];
                }
            }

            // Compute combinatorial values Choose(n,k) and store in mChoose[n][k].
            // The values mChoose[r][c] are invalid for r < c; that is, we use only
            // the entries for r >= c.
            mChoose[0][0] = (Real)1;
            mChoose[1][0] = (Real)1;
            mChoose[1][1] = (Real)1;
            for (int i = 2; i <= mDegree; ++i)
            {
                mChoose[i][0] = (Real)1;
                mChoose[i][i] = (Real)1;
                for (int j = 1; j < i; ++j)
                {
                    mChoose[i][j] = mChoose[i - 1][j - 1] + mChoose[i - 1][j];
                }
            }

            this->mConstructed = true;
        }

        virtual ~BezierCurve()
        {
        }

        // Member access.
        inline int GetDegree() const
        {
            return mDegree;
        }

        inline int GetNumControls() const
        {
            return mNumControls;
        }

        inline Vector<N, Real> const* GetControls() const
        {
            return &mControls[0][0];
        }

        // Evaluation of the curve.  The function supports derivative
        // calculation through order 3; that is, order <= 3 is required.  If
        // you want/ only the position, pass in order of 0.  If you want the
        // position and first derivative, pass in order of 1, and so on.  The
        // output array 'jet' must have enough storage to support the maximum
        // order.  The values are ordered as: position, first derivative,
        // second derivative, third derivative.
        virtual void Evaluate(Real t, unsigned int order, Vector<N, Real>* jet) const override
        {
            unsigned int const supOrder = ParametricCurve<N, Real>::SUP_ORDER;
            if (!this->mConstructed || order >= supOrder)
            {
                // Return a zero-valued jet for invalid state.
                for (unsigned int i = 0; i < supOrder; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            // Compute position.
            Real omt = (Real)1 - t;
            jet[0] = Compute(t, omt, 0);
            if (order >= 1)
            {
                // Compute first derivative.
                jet[1] = Compute(t, omt, 1);
                if (order >= 2)
                {
                    // Compute second derivative.
                    jet[2] = Compute(t, omt, 2);
                    if (order >= 3)
                    {
                        // Compute third derivative.
                        if (mDegree >= 3)
                        {
                            jet[3] = Compute(t, omt, 3);
                        }
                        else
                        {
                            jet[3].MakeZero();
                        }
                    }
                }
            }
        }

    protected:
        // Support for Evaluate(...).
        Vector<N, Real> Compute(Real t, Real omt, int order) const
        {
            Vector<N, Real> result = omt * mControls[order][0];

            Real tpow = t;
            int isup = mDegree - order;
            for (int i = 1; i < isup; ++i)
            {
                Real c = mChoose[isup][i] * tpow;
                result = (result + c * mControls[order][i]) * omt;
                tpow *= t;
            }
            result = (result + tpow * mControls[order][isup]);

            int multiplier = 1;
            for (int i = 0; i < order; ++i)
            {
                multiplier *= mDegree - i;
            }
            result *= (Real)multiplier;

            return result;
        }

        int mDegree, mNumControls;
        std::array<std::vector<Vector<N, Real>>, ParametricCurve<N, Real>::SUP_ORDER> mControls;
        Array2<Real> mChoose;
    };
}
