// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ParametricCurve.h>

namespace gte
{
    template <int N, typename Real>
    class TCBSplineCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction and destruction.  The object copies the input arrays.
        // The number of points must be at least 2.  To validate construction,
        // create an object as shown:
        //     TCBSplineCurve<N, Real> curve(parameters);
        //     if (!curve) { <constructor failed, handle accordingly>; }
        TCBSplineCurve(int numPoints, Vector<N, Real> const* points,
            Real const* times, Real const* tension, Real const* continuity,
            Real const* bias)
            :
            ParametricCurve<N, Real>(numPoints - 1, times)
        {
            LogAssert(numPoints >= 2 && points != nullptr, "Invalid input.");

            mPoints.resize(numPoints);
            mTension.resize(numPoints);
            mContinuity.resize(numPoints);
            mBias.resize(numPoints);
            std::copy(points, points + numPoints, mPoints.begin());
            std::copy(tension, tension + numPoints, mTension.begin());
            std::copy(continuity, continuity + numPoints, mContinuity.begin());
            std::copy(bias, bias + numPoints, mBias.begin());

            int numSegments = numPoints - 1;
            mA.resize(numSegments);
            mB.resize(numSegments);
            mC.resize(numSegments);
            mD.resize(numSegments);

            // For now, treat the first point as if it occurred twice.
            ComputePoly(0, 0, 1, 2);

            for (int i = 1; i < numSegments - 1; ++i)
            {
                ComputePoly(i - 1, i, i + 1, i + 2);
            }

            // For now, treat the last point as if it occurred twice.
            ComputePoly(numSegments - 2, numSegments - 1, numSegments, numSegments);

            this->mConstructed = true;
        }

        virtual ~TCBSplineCurve()
        {
        }

        // Member access.
        inline int GetNumPoints() const
        {
            return static_cast<int>(mPoints.size());
        }

        inline Vector<N, Real> const* GetPoints() const
        {
            return &mPoints[0];
        }

        inline Real const* GetTensions() const
        {
            return &mTension[0];
        }

        inline Real const* GetContinuities() const
        {
            return &mContinuity[0];
        }

        inline Real const* GetBiases() const
        {
            return &mBias[0];
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

            int key;
            Real dt;
            GetKeyInfo(t, key, dt);
            dt /= (this->mTime[key + 1] - this->mTime[key]);

            // Compute position.
            jet[0] = mA[key] + dt * (mB[key] + dt * (mC[key] + dt * mD[key]));
            if (order >= 1)
            {
                // Compute first derivative.
                jet[1] = mB[key] + dt * ((Real)2 * mC[key] + ((Real)3 * dt) * mD[key]);
                if (order >= 2)
                {
                    // Compute second derivative.
                    jet[2] = (Real)2 * mC[key] + ((Real)6 * dt) * mD[key];
                    if (order == 3)
                    {
                        jet[3] = (Real)6 * mD[key];
                    }
                }
            }
        }

    protected:
        // Support for construction.
        void ComputePoly(int i0, int i1, int i2, int i3)
        {
            Vector<N, Real> diff = mPoints[i2] - mPoints[i1];
            Real dt = this->mTime[i2] - this->mTime[i1];

            // Build multipliers at P1.
            Real oneMinusT0 = (Real)1 - mTension[i1];
            Real oneMinusC0 = (Real)1 - mContinuity[i1];
            Real onePlusC0 = (Real)1 + mContinuity[i1];
            Real oneMinusB0 = (Real)1 - mBias[i1];
            Real onePlusB0 = (Real)1 + mBias[i1];
            Real adj0 = (Real)2 * dt / (this->mTime[i2] - this->mTime[i0]);
            Real out0 = (Real)0.5 * adj0 * oneMinusT0 * onePlusC0 * onePlusB0;
            Real out1 = (Real)0.5 * adj0 * oneMinusT0 * oneMinusC0 * oneMinusB0;

            // Build outgoing tangent at P1.
            Vector<N, Real> tOut = out1 * diff + out0 * (mPoints[i1] - mPoints[i0]);

            // Build multipliers at point P2.
            Real oneMinusT1 = (Real)1 - mTension[i2];
            Real oneMinusC1 = (Real)1 - mContinuity[i2];
            Real onePlusC1 = (Real)1 + mContinuity[i2];
            Real oneMinusB1 = (Real)1 - mBias[i2];
            Real onePlusB1 = (Real)1 + mBias[i2];
            Real adj1 = (Real)2 * dt / (this->mTime[i3] - this->mTime[i1]);
            Real in0 = (Real)0.5 * adj1 * oneMinusT1 * oneMinusC1 * onePlusB1;
            Real in1 = (Real)0.5 * adj1 * oneMinusT1 * onePlusC1 * oneMinusB1;

            // Build incoming tangent at P2.
            Vector<N, Real> tIn = in1 * (mPoints[i3] - mPoints[i2]) + in0 * diff;

            mA[i1] = mPoints[i1];
            mB[i1] = tOut;
            mC[i1] = (Real)3 * diff - (Real)2 * tOut - tIn;
            mD[i1] = (Real)-2 * diff + tOut + tIn;
        }

        // Determine the index i for which times[i] <= t < times[i+1].
        void GetKeyInfo(Real t, int& key, Real& dt) const
        {
            int numSegments = static_cast<int>(mA.size());
            if (t <= this->mTime[0])
            {
                key = 0;
                dt = (Real)0;
                return;
            }

            if (t < this->mTime[numSegments])
            {
                for (int i = 0; i < numSegments; ++i)
                {
                    if (t < this->mTime[i + 1])
                    {
                        key = i;
                        dt = t - this->mTime[i];
                        return;
                    }
                }
            }

            key = numSegments - 1;
            dt = this->mTime[numSegments] - this->mTime[numSegments - 1];
        }

        std::vector<Vector<N, Real>> mPoints;
        std::vector<Real> mTension, mContinuity, mBias;

        // Polynomial coefficients.  mA are the degree 0 coefficients,  mB are
        // the degree 1 coefficients, mC are the degree 2 coefficients, and mD
        // are the degree 3 coefficients.
        std::vector<Vector<N, Real>> mA, mB, mC, mD;
    };
}
