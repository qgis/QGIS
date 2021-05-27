// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.11.16

#pragma once

#include <Mathematics/GMatrix.h>
#include <Mathematics/ParametricCurve.h>

namespace gte
{
    template <int N, typename Real>
    class NaturalSplineCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction and destruction.  The object copies the input arrays.
        // The number of points M must be at least 2.  The first constructor
        // is for a spline with second derivatives zero at the endpoints
        // (isFree = true) or a spline that is closed (isFree = false).  The
        // second constructor is for clamped splines, where you specify the
        // first derivatives at the endpoints.  Usually, derivative0 =
        // points[1] - points[0] at the first point and derivative1 =
        // points[M-1] - points[M-2].  To validate construction, create an
        // object as shown:
        //     NaturalSplineCurve<N, Real> curve(parameters);
        //     if (!curve) { <constructor failed, handle accordingly>; }
        NaturalSplineCurve(bool isFree, int numPoints,
            Vector<N, Real> const* points, Real const* times)
            :
            ParametricCurve<N, Real>(numPoints - 1, times)
        {
            if (numPoints < 2 || !points)
            {
                LogError("Invalid input.");
                return;
            }

            auto const numSegments = numPoints - 1;
            mCoefficients.resize(4 * numPoints + 1);
            mA = &mCoefficients[0];
            mB = mA + numPoints;
            mC = mB + numSegments;
            mD = mC + numSegments + 1;
            for (int i = 0; i < numPoints; ++i)
            {
                mA[i] = points[i];
            }

            if (isFree)
            {
                CreateFree();
            }
            else
            {
                CreateClosed();
            }

            this->mConstructed = true;
        }

        NaturalSplineCurve(int numPoints, Vector<N, Real> const* points,
            Real const* times, Vector<N, Real> const& derivative0,
            Vector<N, Real> const& derivative1)
            :
            ParametricCurve<N, Real>(numPoints - 1, times)
        {
            if (numPoints < 2 || !points)
            {
                LogError("Invalid input.");
                return;
            }

            auto const numSegments = numPoints - 1;
            mCoefficients.resize(numPoints + 3 * numSegments + 1);
            mA = &mCoefficients[0];
            mB = mA + numPoints;
            mC = mB + numSegments;
            mD = mC + numSegments + 1;
            for (int i = 0; i < numPoints; ++i)
            {
                mA[i] = points[i];
            }

            CreateClamped(derivative0, derivative1);
            this->mConstructed = true;
        }

        virtual ~NaturalSplineCurve() = default;

        // Member access.
        inline int GetNumPoints() const
        {
            return static_cast<int>((mCoefficients.size() - 1) / 4);
        }

        inline Vector<N, Real> const* GetPoints() const
        {
            return &mA[0];
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

            int key = 0;
            Real dt = (Real)0;
            GetKeyInfo(t, key, dt);

            // Compute position.
            jet[0] = mA[key] + dt * (mB[key] + dt * (mC[key] + dt * mD[key]));
            if (order >= 1)
            {
                // Compute first derivative.
                jet[1] = mB[key] + dt * ((Real)2 * mC[key] + (Real)3 * dt * mD[key]);
                if (order >= 2)
                {
                    // Compute second derivative.
                    jet[2] = (Real)2 * mC[key] + (Real)6 * dt * mD[key];
                    if (order == 3)
                    {
                        jet[3] = (Real)6 * mD[key];
                    }
                }
            }
        }

    protected:
        // Support for construction.
        void CreateFree()
        {
            int numSegments = GetNumPoints() - 1;
            WorkingData wd(numSegments);
            for (int i = 0; i < numSegments; ++i)
            {
                wd.dt[i] = this->mTime[i + 1] - this->mTime[i];
            }

            std::vector<Real> d2t(numSegments);
            for (int i = 1; i < numSegments; ++i)
            {
                wd.d2t[i] = this->mTime[i + 1] - this->mTime[i - 1];
            }

            std::vector<Vector<N, Real>> alpha(numSegments);
            for (int i = 1; i < numSegments; ++i)
            {
                Vector<N, Real> numer = (Real)3 * (wd.dt[i - 1] * mA[i + 1] - wd.d2t[i] * mA[i] + wd.dt[i] * mA[i - 1]);
                Real invDenom = (Real)1 / (wd.dt[i - 1] * wd.dt[i]);
                wd.alpha[i] = invDenom * numer;
            }

            std::vector<Real> ell(numSegments + 1);
            std::vector<Real> mu(numSegments);
            std::vector<Vector<N, Real>> z(numSegments + 1);
            Real inv;

            wd.ell[0] = (Real)1;
            wd.mu[0] = (Real)0;
            wd.z[0].MakeZero();
            for (int i = 1; i < numSegments; ++i)
            {
                wd.ell[i] = (Real)2 * wd.d2t[i] - wd.dt[i - 1] * wd.mu[i - 1];
                inv = (Real)1 / wd.ell[i];
                wd.mu[i] = inv * wd.dt[i];
                wd.z[i] = inv * (wd.alpha[i] - wd.dt[i - 1] * wd.z[i - 1]);
            }
            wd.ell[numSegments] = (Real)1;
            wd.z[numSegments].MakeZero();

            Real const oneThird = (Real)1 / (Real)3;
            mC[numSegments].MakeZero();
            for (int i = numSegments - 1; i >= 0; --i)
            {
                mC[i] = wd.z[i] - wd.mu[i] * mC[i + 1];
                inv = (Real)1 / wd.dt[i];
                mB[i] = inv * (mA[i + 1] - mA[i]) - oneThird * wd.dt[i] * (mC[i + 1] + (Real)2 * mC[i]);
                mD[i] = oneThird * inv * (mC[i + 1] - mC[i]);
            }
        }

        void CreateClosed()
        {
            // TODO:  A general linear system solver is used here.  The matrix
            // corresponding to this case is actually "cyclic banded", so a
            // faster linear solver can be used.  The current linear system
            // code does not have such a solver.

            int numSegments = GetNumPoints() - 1;
            std::vector<Real> dt(numSegments);
            for (int i = 0; i < numSegments; ++i)
            {
                dt[i] = this->mTime[i + 1] - this->mTime[i];
            }

            // Construct matrix of system.
            GMatrix<Real> mat(numSegments + 1, numSegments + 1);
            mat(0, 0) = (Real)1;
            mat(0, numSegments) = (Real)-1;
            for (int i = 1; i <= numSegments - 1; ++i)
            {
                mat(i, i - 1) = dt[i - 1];
                mat(i, i) = (Real)2 * (dt[i - 1] + dt[i]);
                mat(i, i + 1) = dt[i];
            }
            mat(numSegments, numSegments - 1) = dt[numSegments - 1];
            mat(numSegments, 0) = (Real)2 * (dt[numSegments - 1] + dt[0]);
            mat(numSegments, 1) = dt[0];

            // Construct right-hand side of system.
            mC[0].MakeZero();
            Real inv0, inv1;
            for (int i = 1; i <= numSegments - 1; ++i)
            {
                inv0 = (Real)1 / dt[i];
                inv1 = (Real)1 / dt[i - 1];
                mC[i] = (Real)3 * (inv0 * (mA[i + 1] - mA[i]) -
                    inv1 * (mA[i] - mA[i - 1]));
            }
            inv0 = (Real)1 / dt[0];
            inv1 = (Real)1 / dt[numSegments - 1];
            mC[numSegments] = (Real)3 * (inv0 * (mA[1] - mA[0]) -
                inv1 * (mA[0] - mA[numSegments - 1]));

            // Solve the linear systems.
            GMatrix<Real> invMat = Inverse(mat);
            GVector<Real> input(numSegments + 1);
            GVector<Real> output(numSegments + 1);
            for (int j = 0; j < N; ++j)
            {
                for (int i = 0; i <= numSegments; ++i)
                {
                    input[i] = mC[i][j];
                }
                output = invMat * input;
                for (int i = 0; i <= numSegments; ++i)
                {
                    mC[i][j] = output[i];
                }
            }

            Real const oneThird = (Real)1 / (Real)3;
            for (int i = 0; i < numSegments; ++i)
            {
                inv0 = (Real)1 / dt[i];
                mB[i] = inv0 * (mA[i + 1] - mA[i]) - oneThird * (mC[i + 1] + (Real)2 * mC[i]) * dt[i];
                mD[i] = oneThird * inv0 * (mC[i + 1] - mC[i]);
            }
        }

        void CreateClamped(Vector<N, Real> const& derivative0, Vector<N, Real> const& derivative1)
        {
            int numSegments = GetNumPoints() - 1;
            std::vector<Real> dt(numSegments);
            for (int i = 0; i < numSegments; ++i)
            {
                dt[i] = this->mTime[i + 1] - this->mTime[i];
            }

            std::vector<Real> d2t(numSegments);
            for (int i = 1; i < numSegments; ++i)
            {
                d2t[i] = this->mTime[i + 1] - this->mTime[i - 1];
            }

            std::vector<Vector<N, Real>> alpha(numSegments + 1);
            Real inv = (Real)1 / dt[0];
            alpha[0] = (Real)3 * (inv * (mA[1] - mA[0]) - derivative0);
            inv = (Real)1 / dt[numSegments - 1];
            alpha[numSegments] = (Real)3 * (derivative1 -
                inv * (mA[numSegments] - mA[numSegments - 1]));
            for (int i = 1; i < numSegments; ++i)
            {
                Vector<N, Real> numer = (Real)3 * (dt[i - 1] * mA[i + 1] -
                    d2t[i] * mA[i] + dt[i] * mA[i - 1]);
                Real invDenom = (Real)1 / (dt[i - 1] * dt[i]);
                alpha[i] = invDenom * numer;
            }

            std::vector<Real> ell(numSegments + 1);
            std::vector<Real> mu(numSegments);
            std::vector<Vector<N, Real>> z(numSegments + 1);

            ell[0] = (Real)2 * dt[0];
            mu[0] = (Real)0.5;
            inv = (Real)1 / ell[0];
            z[0] = inv * alpha[0];

            for (int i = 1; i < numSegments; ++i)
            {
                ell[i] = (Real)2 * d2t[i] - dt[i - 1] * mu[i - 1];
                inv = (Real)1 / ell[i];
                mu[i] = inv * dt[i];
                z[i] = inv * (alpha[i] - dt[i - 1] * z[i - 1]);
            }
            ell[numSegments] = dt[numSegments - 1] * ((Real)2 - mu[numSegments - 1]);
            inv = (Real)1 / ell[numSegments];
            z[numSegments] = inv * (alpha[numSegments] - dt[numSegments - 1] *
                z[numSegments - 1]);

            Real const oneThird = (Real)1 / (Real)3;
            mC[numSegments] = z[numSegments];
            for (int i = numSegments - 1; i >= 0; --i)
            {
                mC[i] = z[i] - mu[i] * mC[i + 1];
                inv = (Real)1 / dt[i];
                mB[i] = inv * (mA[i + 1] - mA[i]) - oneThird * dt[i] * (mC[i + 1] +
                    (Real)2 * mC[i]);
                mD[i] = oneThird * inv * (mC[i + 1] - mC[i]);
            }
        }

        // Determine the index i for which times[i] <= t < times[i+1].
        void GetKeyInfo(Real t, int& key, Real& dt) const
        {
            int numSegments = GetNumPoints() - 1;
            if (t <= this->mTime[0])
            {
                key = 0;
                dt = (Real)0;
            }
            else if (t >= this->mTime[numSegments])
            {
                key = numSegments - 1;
                dt = this->mTime[numSegments] - this->mTime[numSegments - 1];
            }
            else
            {
                for (int i = 0; i < numSegments; ++i)
                {
                    if (t < this->mTime[i + 1])
                    {
                        key = i;
                        dt = t - this->mTime[i];
                        break;
                    }
                }
            }
        }

        // Polynomial coefficients.  mA are the points (constant coefficients of
        // polynomials.  mB are the degree 1 coefficients, mC are the degree 2
        // coefficients, and mD are the degree 3 coefficients.
        Vector<N, Real>* mA;
        Vector<N, Real>* mB;
        Vector<N, Real>* mC;
        Vector<N, Real>* mD;

    private:
        std::vector<Vector<N, Real>> mCoefficients;

        struct WorkingData
        {
            WorkingData(int numSegments)
            {
                data.resize(4 * numSegments + 1 + N * (2 * numSegments + 1));
                dt = &data[0];
                d2t = dt + numSegments;
                alpha = reinterpret_cast<Vector<N, Real>*>(d2t + numSegments);
                ell = reinterpret_cast<Real*>(alpha + numSegments);
                mu = ell + numSegments + 1;
                z = reinterpret_cast<Vector<N, Real>*>(mu + numSegments);
            }

            Real* dt;
            Real* d2t;
            Vector<N, Real>* alpha;
            Real* ell;
            Real* mu;
            Vector<N, Real>* z;

            std::vector<Real> data;
        };
    };
}
