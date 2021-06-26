// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/GaussNewtonMinimizer.h>
#include <Mathematics/LevenbergMarquardtMinimizer.h>
#include <Mathematics/RootsPolynomial.h>

// The cone vertex is V, the unit-length axis direction is U and the
// cone angle is A in (0,pi/2).  The cone is defined algebraically by
// those points X for which
//     Dot(U,X-V)/Length(X-V) = cos(A)
// This can be written as a quadratic equation
//     (V-X)^T * (cos(A)^2 - U * U^T) * (V-X) = 0
// with the implicit constraint that Dot(U, X-V) > 0 (X is on the
// "positive" cone).  Define W = U/cos(A), so Length(W) > 1 and
//     F(X;V,W) = (V-X)^T * (I - W * W^T) * (V-X) = 0
// The nonlinear least squares fitting of points {X[i]}_{i=0}^{n-1}
// computes V and W to minimize the error function
//     E(V,W) = sum_{i=0}^{n-1} F(X[i];V,W)^2
// I recommend using the Gauss-Newton minimizer when your cone points
// are truly nearly a cone; otherwise, try the Levenberg-Marquardt
// minimizer.
//
// The mathematics used in this implementation are found in
//   https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf
// In particular, the details for choosing an initial cone for fitting
// are somewhat complicated.

namespace gte
{
    template <typename Real>
    class ApprCone3
    {
    public:
        ApprCone3()
            :
            mNumPoints(0),
            mPoints(nullptr)
        {
            // F[i](V,W) = D^T * (I - W * W^T) * D, D = V - X[i], P = (V,W)
            mFFunction = [this](GVector<Real> const& P, GVector<Real>& F)
            {
                Vector<3, Real> V = { P[0], P[1], P[2] };
                Vector<3, Real> W = { P[3], P[4], P[5] };
                for (int i = 0; i < mNumPoints; ++i)
                {
                    Vector<3, Real> delta = V - mPoints[i];
                    Real deltaDotW = Dot(delta, W);
                    F[i] = Dot(delta, delta) - deltaDotW * deltaDotW;
                }
            };

            // dF[i]/dV = 2 * (D - Dot(W, D) * W)
            // dF[i]/dW = -2 * Dot(W, D) * D
            mJFunction = [this](GVector<Real> const& P, GMatrix<Real>& J)
            {
                Vector<3, Real> V = { P[0], P[1], P[2] };
                Vector<3, Real> W = { P[3], P[4], P[5] };
                for (int row = 0; row < mNumPoints; ++row)
                {
                    Vector<3, Real> delta = V - mPoints[row];
                    Real deltaDotW = Dot(delta, W);
                    Vector<3, Real> temp0 = delta - deltaDotW * W;
                    Vector<3, Real> temp1 = deltaDotW * delta;
                    for (int col = 0; col < 3; ++col)
                    {
                        J(row, col) = (Real)2 * temp0[col];
                        J(row, col + 3) = (Real)-2 * temp1[col];
                    }
                }
            };
        }

        // If you want to specify that coneVertex, coneAxis and coneAngle
        // are the initial guesses for the minimizer, set the parameter
        // useConeInputAsInitialGuess to 'true'.  If you want the function
        // to compute initial guesses, set that parameter to 'false'.
        // A Gauss-Newton minimizer is used to fit a cone using nonlinear
        // least-squares.  The fitted cone is returned in coneVertex,
        // coneAxis and coneAngle.  See GaussNewtonMinimizer.h for a
        // description of the least-squares algorithm and the parameters
        // that it requires.
        typename GaussNewtonMinimizer<Real>::Result
        operator()(int numPoints, Vector<3, Real> const* points,
            size_t maxIterations, Real updateLengthTolerance, Real errorDifferenceTolerance,
            bool useConeInputAsInitialGuess,
            Vector<3, Real>& coneVertex, Vector<3, Real>& coneAxis, Real& coneAngle)
        {
            mNumPoints = numPoints;
            mPoints = points;
            GaussNewtonMinimizer<Real> minimizer(6, mNumPoints, mFFunction, mJFunction);

            Real coneCosAngle;
            if (useConeInputAsInitialGuess)
            {
                Normalize(coneAxis);
                coneCosAngle = std::cos(coneAngle);
            }
            else
            {
                ComputeInitialCone(coneVertex, coneAxis, coneCosAngle);
            }

            // The initial guess for the cone vertex.
            GVector<Real> initial(6);
            initial[0] = coneVertex[0];
            initial[1] = coneVertex[1];
            initial[2] = coneVertex[2];

            // The initial guess for the weighted cone axis.
            initial[3] = coneAxis[0] / coneCosAngle;
            initial[4] = coneAxis[1] / coneCosAngle;
            initial[5] = coneAxis[2] / coneCosAngle;

            auto result = minimizer(initial, maxIterations, updateLengthTolerance,
                errorDifferenceTolerance);

            // No test is made for result.converged so that we return some
            // estimates of the cone.  The caller can decide how to respond
            // when result.converged is false.
            for (int i = 0; i < 3; ++i)
            {
                coneVertex[i] = result.minLocation[i];
                coneAxis[i] = result.minLocation[i + 3];
            }

            // We know that coneCosAngle will be nonnegative.  The std::min
            // call guards against rounding errors leading to a number
            // slightly larger than 1.  The clamping ensures std::acos will
            // not return a NaN.
            coneCosAngle = std::min((Real)1 / Normalize(coneAxis), (Real)1);
            coneAngle = std::acos(coneCosAngle);

            mNumPoints = 0;
            mPoints = nullptr;
            return result;
        }

        // The parameters coneVertex, coneAxis and coneAngle are in/out
        // variables.  The caller must provide initial guesses for these.
        // The function estimates the cone parameters and returns them.  See
        // GteGaussNewtonMinimizer.h for a description of the least-squares
        // algorithm and the parameters that it requires.
        typename LevenbergMarquardtMinimizer<Real>::Result
        operator()(int numPoints, Vector<3, Real> const* points,
            size_t maxIterations, Real updateLengthTolerance, Real errorDifferenceTolerance,
            Real lambdaFactor, Real lambdaAdjust, size_t maxAdjustments,
            bool useConeInputAsInitialGuess,
            Vector<3, Real>& coneVertex, Vector<3, Real>& coneAxis, Real& coneAngle)
        {
            mNumPoints = numPoints;
            mPoints = points;
            LevenbergMarquardtMinimizer<Real> minimizer(6, mNumPoints, mFFunction, mJFunction);

            Real coneCosAngle;
            if (useConeInputAsInitialGuess)
            {
                Normalize(coneAxis);
                coneCosAngle = std::cos(coneAngle);
            }
            else
            {
                ComputeInitialCone(coneVertex, coneAxis, coneCosAngle);
            }

            // The initial guess for the cone vertex.
            GVector<Real> initial(6);
            initial[0] = coneVertex[0];
            initial[1] = coneVertex[1];
            initial[2] = coneVertex[2];

            // The initial guess for the weighted cone axis.
            initial[3] = coneAxis[0] / coneCosAngle;
            initial[4] = coneAxis[1] / coneCosAngle;
            initial[5] = coneAxis[2] / coneCosAngle;

            auto result = minimizer(initial, maxIterations, updateLengthTolerance,
                errorDifferenceTolerance, lambdaFactor, lambdaAdjust, maxAdjustments);

            // No test is made for result.converged so that we return some
            // estimates of the cone.  The caller can decide how to respond
            // when result.converged is false.
            for (int i = 0; i < 3; ++i)
            {
                coneVertex[i] = result.minLocation[i];
                coneAxis[i] = result.minLocation[i + 3];
            }

            // We know that coneCosAngle will be nonnegative.  The std::min
            // call guards against rounding errors leading to a number
            // slightly larger than 1.  The clamping ensures std::acos will
            // not return a NaN.
            coneCosAngle = std::min((Real)1 / Normalize(coneAxis), (Real)1);
            coneAngle = std::acos(coneCosAngle);

            mNumPoints = 0;
            mPoints = nullptr;
            return result;
        }

    private:
        void ComputeInitialCone(Vector<3, Real>& coneVertex, Vector<3, Real>& coneAxis, Real& coneCosAngle)
        {
            // Compute the average of the sample points.
            Vector<3, Real> center{ (Real)0, (Real)0, (Real)0 };
            Real const invNumPoints = (Real)1 / (Real)mNumPoints;
            for (int i = 0; i < mNumPoints; ++i)
            {
                center += mPoints[i];
            }
            center *= invNumPoints;

            // The cone axis is estimated from ZZTZ (see the PDF).
            coneAxis = { (Real)0, (Real)0, (Real)0 };
            for (int i = 0; i < mNumPoints; ++i)
            {
                Vector<3, Real> diff = mPoints[i] - center;
                coneAxis += Dot(diff, diff) * diff;
            }
            coneAxis *= invNumPoints;
            Normalize(coneAxis);

            // Compute the averages of powers and products of powers of
            // a[i] = Dot(U,X[i]-C) and b[i] = Dot(X[i]-C,X[i]-C).
            Real c10 = (Real)0, c20 = (Real)0, c30 = (Real)0, c01 = (Real)0;
            Real c02 = (Real)0, c11 = (Real)0, c21 = (Real)0;
            for (int i = 0; i < mNumPoints; ++i)
            {
                Vector<3, Real> diff = mPoints[i] - center;
                Real ai = Dot(coneAxis, diff);
                Real aisqr = ai * ai;
                Real bi = Dot(diff, diff);
                c10 += ai;
                c20 += aisqr;
                c30 += aisqr * ai;
                c01 += bi;
                c02 += bi * bi;
                c11 += ai * bi;
                c21 += aisqr * bi;
            }
            c10 *= invNumPoints;
            c20 *= invNumPoints;
            c30 *= invNumPoints;
            c01 *= invNumPoints;
            c02 *= invNumPoints;
            c11 *= invNumPoints;
            c21 *= invNumPoints;

            // Compute the coefficients of p3(t) and q3(t).
            Real e0 = (Real)3 * c10;
            Real e1 = (Real)2 * c20 + c01;
            Real e2 = c11;
            Real e3 = (Real)3 * c20;
            Real e4 = c30;

            // Compute the coefficients of g(t).
            Real g0 = c11 * c21 - c02 * c30;
            Real g1 = c01 * c21 - (Real)3 * c02 * c20 + (Real)2 * (c20 * c21 - c11 * (c30 - c11));
            Real g2 = (Real)3 * (c11 * (c01 - c20) + c10 * (c21 - c02));
            Real g3 = c21 - c02 + c01 * (c01 + c20) + (Real)2 * (c10 * (c30 - c11) - c20 * c20);
            Real g4 = c30 - c11 + c10 * (c01 - c20);

            // Compute the roots of g(t) = 0.
            std::map<Real, int> rmMap;
            RootsPolynomial<Real>::SolveQuartic(g0, g1, g2, g3, g4, rmMap);

            // Locate the positive root t that leads to an s = cos(theta)
            // in (0,1) and that has minimum least-squares error.  In theory,
            // there must always be such a root, but floating-point rounding
            // errors might lead to no such root.  The implementation returns
            // the center as the estimate of V and pi/4 as the estimate of
            // the angle (s = 1/2).
            std::vector<std::array<Real, 3>> info;
            Real s, t;
            for (auto const& element : rmMap)
            {
                t = element.first;
                if (t > (Real)0)
                {
                    Real p3 = e2 + t * (e1 + t * (e0 + t));
                    if (p3 != (Real)0)
                    {
                        Real q3 = e4 + t * (e3 + t * (e0 + t));
                        s = q3 / p3;
                        if ((Real)0 < s && s < (Real)1)
                        {
                            Real error(0);
                            for (int i = 0; i < mNumPoints; ++i)
                            {
                                Vector<3, Real> diff = mPoints[i] - center;
                                Real ai = Dot(coneAxis, diff);
                                Real bi = Dot(diff, diff);
                                Real tpai = t + ai;
                                Real Fi = s * (bi + t * ((Real)2 * ai + t)) - tpai * tpai;
                                error += Fi * Fi;
                            }
                            error *= invNumPoints;
                            std::array<Real, 3> item = { s, t, error };
                            info.push_back(item);
                        }
                    }
                }
            }

            Real minError = std::numeric_limits<Real>::max();
            std::array<Real, 3> minItem = { (Real)0, (Real)0, minError };
            for (auto const& item : info)
            {
                if (item[2] < minError)
                {
                    minItem = item;
                }
            }

            if (minItem[2] < std::numeric_limits<Real>::max())
            {
                // minItem = { minS, minT, minError }
                coneVertex = center - minItem[1] * coneAxis;
                coneCosAngle = std::sqrt(minItem[0]);
            }
            else
            {
                coneVertex = center;
                coneCosAngle = (Real)GTE_C_INV_SQRT_2;
            }
        }

        int mNumPoints;
        Vector<3, Real> const* mPoints;
        std::function<void(GVector<Real> const&, GVector<Real>&)> mFFunction;
        std::function<void(GVector<Real> const&, GMatrix<Real>&)> mJFunction;
    };
}
