// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ApprOrthogonalPlane3.h>
#include <Mathematics/RootsPolynomial.h>
#include <Mathematics/GaussNewtonMinimizer.h>
#include <Mathematics/LevenbergMarquardtMinimizer.h>

// Let the torus center be C with plane of symmetry containing C and having
// directions D0 and D1.  The axis of symmetry is the line containing C and
// having direction N (the plane normal).  The radius from the center of the
// torus is r0 and the radius of the tube of the torus is r1.  A point P may
// be written as P = C + x*D0 + y*D1 + z*N, where matrix [D0 D1 N] is
// orthogonal and has determinant 1.  Thus, x = Dot(D0,P-C), y = Dot(D1,P-C)
// and z = Dot(N,P-C).  The implicit equation defining the torus is
//     (|P-C|^2 + r0^2 - r1^2)^2 - 4*r0^2*(|P-C|^2 - (Dot(N,P-C))^2) = 0
// Observe that D0 and D1 are not present in the equation, which is to be
// expected by the symmetry.
//
// Define u = r0^2 and v = r0^2 - r1^2.  Define
//     F(X;C,N,u,v) = (|P-C|^2 + v)^2 - 4*u*(|P-C|^2 - (Dot(N,P-C))^2)
// The nonlinear least-squares fitting of points {X[i]}_{i=0}^{n-1} computes
// C, N, u and v to minimize the error function
//     E(C,N,u,v) = sum_{i=0}^{n-1} F(X[i];C,N,u,v)^2
// When the sample points are distributed so that there is large coverage
// by a purported fitted torus, a variation on fitting is the following.
// Compute the least-squares plane with origin C and normal N that fits the
// points.  Define G(X;u,v) = F(X;C,N,u,v); the only variables now are u and
// v.  Define L[i] = |X[i]-C|^2 and S[i] = 4 * (L[i] - (Dot(N,X[i]-C))^2).
// Define the error function
//     H(u,v) = sum_{i=0}^{n-1} G(X[i];u,v)^2
//            = sum_{i=0}^{n-1} ((v + L[i])^2 - S[i]*u)^2
// The first-order partial derivatives are
//     dH/du = -2 sum_{i=0}^{n-1} ((v + L[i])^2 - S[i]*u) * S[i]
//     dH/dv =  4 sum_{i=0}^{n-1} ((v + L[i])^2 - S[i]*u) * (v + L[i])
// Setting these to zero and expanding the terms, we have
//     0 = a2 * v^2 + a1 * v + a0 - b0 * u
//     0 = c3 * v^3 + c2 * v^2 + c1 * v + c0 - u * (d1 * v + d0)
// where a2 = sum(S[i]), a1 = 2*sum(S[i]*L[i]), a2 = sum(S[i]*L[i]^2),
// b0 = sum(S[i]^2), c3 = sum(1) = n, c2 = 3*sum(L[i]), c1 = 3*sum(L[i]^2),
// c0 = sum(L[i]^3), d1 = sum(S[i]) = a2 and d0 = sum(S[i]*L[i]) = a1/2.
// The first equation is solved for
//     u = (a2 * v^2 + a1 * v + a0) / b0 = e2 * v^2 + e1 * v + e0
// and substituted into the second equation to obtain a cubic polynomial
// equation
//     0 = f3 * v^3 + f2 * v^2 + f1 * v + f0
// where f3 = c3 - d1 * e2, f2 = c2 - d1 * e1 - d0 * e2,
// f1 = c1 - d1 * e0 - d0 * e1 and f0 = c0 - d0 * e0.  The positive v-roots
// are computed.  For each root compute the corresponding u.  For all pairs
// (u,v) with u > v > 0, evaluate H(u,v) and choose the pair that minimizes
// H(u,v).  The torus radii are r0 = sqrt(u) and r1 = sqrt(u - v).

namespace gte
{
    template <typename Real>
    class ApprTorus3
    {
    public:
        ApprTorus3()
        {
            // The unit-length normal is
            //   N = (cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi)
            // for theta in [0,2*pi) and phi in [0,*pi).  The radii are
            // encoded as
            //   u = r0^2, v = r0^2 - r1^2
            // with 0 < v < u.  Let D = C - X[i] where X[i] is a sample point.
            // The parameters P = (C0,C1,C2,theta,phi,u,v).

            // F[i](C,theta,phi,u,v) =
            //   (|D|^2 + v)^2 - 4*u*(|D|^2 - Dot(N,D)^2)
            mFFunction = [this](GVector<Real> const& P, GVector<Real>& F)
            {
                Real csTheta = std::cos(P[3]);
                Real snTheta = std::sin(P[3]);
                Real csPhi = std::cos(P[4]);
                Real snPhi = std::sin(P[4]);
                Vector<3, Real> C = { P[0], P[1], P[2] };
                Vector<3, Real> N = { csTheta * snPhi, snTheta * snPhi, csPhi };
                Real u = P[5];
                Real v = P[6];
                for (int i = 0; i < mNumPoints; ++i)
                {
                    Vector<3, Real> D = C - mPoints[i];
                    Real DdotD = Dot(D, D), NdotD = Dot(N, D);
                    Real sum = DdotD + v;
                    F[i] = sum * sum - (Real)4 * u * (DdotD - NdotD * NdotD);
                }
            };

            // dF[i]/dC = 4 * (|D|^2 + v) * D - 8 * u * (I - N*N^T) * D
            // dF[i]/dTheta = 8 * u * Dot(dN/dTheta, D)
            // dF[i]/dPhi = 8 * u * Dot(dN/dPhi, D)
            // dF[i]/du = -4 * u * (|D|^2 - Dot(N,D)^2)
            // dF[i]/dv = 2 * (|D|^2 + v)
            mJFunction = [this](GVector<Real> const& P, GMatrix<Real>& J)
            {
                Real const r2(2), r4(4), r8(8);
                Real csTheta = std::cos(P[3]);
                Real snTheta = std::sin(P[3]);
                Real csPhi = std::cos(P[4]);
                Real snPhi = std::sin(P[4]);
                Vector<3, Real> C = { P[0], P[1], P[2] };
                Vector<3, Real> N = { csTheta * snPhi, snTheta * snPhi, csPhi };
                Real u = P[5];
                Real v = P[6];
                for (int row = 0; row < mNumPoints; ++row)
                {
                    Vector<3, Real> D = C - mPoints[row];
                    Real DdotD = Dot(D, D), NdotD = Dot(N, D);
                    Real sum = DdotD + v;
                    Vector<3, Real> dNdTheta{ -snTheta * snPhi, csTheta * snPhi, (Real)0 };
                    Vector<3, Real> dNdPhi{ csTheta * csPhi, snTheta * csPhi, -snPhi };
                    Vector<3, Real> temp = r4 * sum * D - r8 * u * (D - NdotD * N);
                    J(row, 0) = temp[0];
                    J(row, 1) = temp[1];
                    J(row, 2) = temp[2];
                    J(row, 3) = r8 * u * Dot(dNdTheta, D);
                    J(row, 4) = r8 * u * Dot(dNdPhi, D);
                    J(row, 5) = -r4 * u * (DdotD - NdotD * NdotD);
                    J(row, 6) = r2 * sum;
                }
            };
        }

        // When the samples are distributed approximately uniformly near a
        // torus, use this method.  For example, if the purported torus has
        // center (0,0,0) and normal (0,0,1), you want the (x,y,z) samples
        // to occur in all 8 octants.  If the samples occur, say, only in
        // one octant, this method will estimate a C and N that are nowhere
        // near (0,0,0) and (0,0,1).  The function sets the output variables
        // C, N, r0 and r1 as the fitted torus.
        //
        // The return value is a pair <bool,Real>.  The first element is
        // 'true' when the estimate is valid, in which case the second
        // element is the least-squares error for that estimate.  If any
        // unexpected condition occurs that prevents computing an estimate,
        // the first element is 'false' and the second element is
        // std::numeric_limits<Real>::max().
        std::pair<bool, Real>
        operator()(int numPoints, Vector<3, Real> const* points,
            Vector<3, Real>& C, Vector<3, Real>& N, Real& r0, Real& r1) const
        {
            ApprOrthogonalPlane3<Real> fitter;
            if (!fitter.Fit(numPoints, points))
            {
                return std::make_pair(false, std::numeric_limits<Real>::max());
            }
            C = fitter.GetParameters().first;
            N = fitter.GetParameters().second;

            Real const zero(0);
            Real a0 = zero, a1 = zero, a2 = zero, b0 = zero;
            Real c0 = zero, c1 = zero, c2 = zero, c3 = (Real)numPoints;
            for (int i = 0; i < numPoints; ++i)
            {
                Vector<3, Real> delta = points[i] - C;
                Real dot = Dot(N, delta);
                Real L = Dot(delta, delta), L2 = L * L, L3 = L * L2;
                Real S = (Real)4 * (L - dot * dot), S2 = S * S;
                a2 += S;
                a1 += S * L;
                a0 += S * L2;
                b0 += S2;
                c2 += L;
                c1 += L2;
                c0 += L3;
            }
            Real d1 = a2;
            Real d0 = a1;
            a1 *= (Real)2;
            c2 *= (Real)3;
            c1 *= (Real)3;
            Real invB0 = (Real)1 / b0;
            Real e0 = a0 * invB0;
            Real e1 = a1 * invB0;
            Real e2 = a2 * invB0;

            Rational f0 = c0 - d0 * e0;
            Rational f1 = c1 - d1 * e0 - d0 * e1;
            Rational f2 = c2 - d1 * e1 - d0 * e2;
            Rational f3 = c3 - d1 * e2;
            std::map<Real, int> rmMap;
            RootsPolynomial<Real>::SolveCubic(f0, f1, f2, f3, rmMap);

            Real hmin = std::numeric_limits<Real>::max();
            Real umin = zero, vmin = zero;
            for (auto const& element : rmMap)
            {
                Real v = element.first;
                if (v > zero)
                {
                    Real u = e0 + v * (e1 + v * e2);
                    if (u > v)
                    {
                        Real h = zero;
                        for (int i = 0; i < numPoints; ++i)
                        {
                            Vector<3, Real> delta = points[i] - C;
                            Real dot = Dot(N, delta);
                            Real L = Dot(delta, delta);
                            Real S = (Real)4 * (L - dot * dot);
                            Real sum = v + L;
                            Real term = sum * sum - S * u;
                            h += term * term;
                        }
                        if (h < hmin)
                        {
                            hmin = h;
                            umin = u;
                            vmin = v;
                        }
                    }
                }
            }

            if (hmin == std::numeric_limits<Real>::max())
            {
                return std::make_pair(false, std::numeric_limits<Real>::max());
            }

            r0 = std::sqrt(umin);
            r1 = std::sqrt(umin - vmin);
            return std::make_pair(true, hmin);
        }

        // If you want to specify that C, N, r0 and r1 are the initial guesses
        // for the minimizer, set the parameter useTorusInputAsInitialGuess to
        // 'true'.  If you want the function to compute initial guesses, set
        // useTorusInputAsInitialGuess to 'false'.  A Gauss-Newton minimizer
        // is used to fit a torus using nonlinear least-squares.  The fitted
        // torus is returned in C, N, r0 and r1. See GteGaussNewtonMinimizer.h
        // for a description of the least-squares algorithm and the parameters
        // that it requires.
        typename GaussNewtonMinimizer<Real>::Result
        operator()(int numPoints, Vector<3, Real> const* points,
            size_t maxIterations, Real updateLengthTolerance, Real errorDifferenceTolerance,
            bool useTorusInputAsInitialGuess,
            Vector<3, Real>& C, Vector<3, Real>& N, Real& r0, Real& r1) const
        {
            mNumPoints = numPoints;
            mPoints = points;
            GaussNewtonMinimizer<Real> minimizer(7, mNumPoints, mFFunction, mJFunction);

            if (!useTorusInputAsInitialGuess)
            {
                operator()(numPoints, points, C, N, r0, r1);
            }

            GVector<Real> initial(7);

            // The initial guess for the plane origin.
            initial[0] = C[0];
            initial[1] = C[1];
            initial[2] = C[2];

            // The initial guess for the plane normal.  The angles must be
            // extracted for spherical coordinates.
            if (std::fabs(N[2]) < (Real)1)
            {
                initial[3] = std::atan2(N[1], N[0]);
                initial[4] = std::acos(N[2]);
            }
            else
            {
                initial[3] = (Real)0;
                initial[4] = (Real)0;
            }

            // The initial guess for the radii-related parameters.
            initial[5] = r0 * r0;
            initial[6] = initial[5] - r1 * r1;

            auto result = minimizer(initial, maxIterations, updateLengthTolerance,
                errorDifferenceTolerance);

            // No test is made for result.converged so that we return some
            // estimates of the torus.  The caller can decide how to respond
            // when result.converged is false.
            C[0] = result.minLocation[0];
            C[1] = result.minLocation[1];
            C[2] = result.minLocation[2];

            Real theta = result.minLocation[3];
            Real phi = result.minLocation[4];
            Real csTheta = std::cos(theta);
            Real snTheta = std::sin(theta);
            Real csPhi = std::cos(phi);
            Real snPhi = std::sin(phi);
            N[0] = csTheta * snPhi;
            N[1] = snTheta * snPhi;
            N[2] = csPhi;

            Real u = result.minLocation[5];
            Real v = result.minLocation[6];
            r0 = std::sqrt(u);
            r1 = std::sqrt(u - v);

            mNumPoints = 0;
            mPoints = nullptr;
            return result;
        }

        // If you want to specify that C, N, r0 and r1 are the initial guesses
        // for the minimizer, set the parameter useTorusInputAsInitialGuess to
        // 'true'.  If you want the function to compute initial guesses, set
        // useTorusInputAsInitialGuess to 'false'.  A Gauss-Newton minimizer
        // is used to fit a torus using nonlinear least-squares.  The fitted
        // torus is returned in C, N, r0 and r1. See GteGaussNewtonMinimizer.h
        // for a description of the least-squares algorithm and the parameters
        // that it requires.
        typename LevenbergMarquardtMinimizer<Real>::Result
        operator()(int numPoints, Vector<3, Real> const* points,
            size_t maxIterations, Real updateLengthTolerance, Real errorDifferenceTolerance,
            Real lambdaFactor, Real lambdaAdjust, size_t maxAdjustments,
            bool useTorusInputAsInitialGuess,
            Vector<3, Real>& C, Vector<3, Real>& N, Real& r0, Real& r1) const
        {
            mNumPoints = numPoints;
            mPoints = points;
            LevenbergMarquardtMinimizer<Real> minimizer(7, mNumPoints, mFFunction, mJFunction);

            if (!useTorusInputAsInitialGuess)
            {
                operator()(numPoints, points, C, N, r0, r1);
            }

            GVector<Real> initial(7);

            // The initial guess for the plane origin.
            initial[0] = C[0];
            initial[1] = C[1];
            initial[2] = C[2];

            // The initial guess for the plane normal.  The angles must be
            // extracted for spherical coordinates.
            if (std::fabs(N[2]) < (Real)1)
            {
                initial[3] = std::atan2(N[1], N[0]);
                initial[4] = std::acos(N[2]);
            }
            else
            {
                initial[3] = (Real)0;
                initial[4] = (Real)0;
            }

            // The initial guess for the radii-related parameters.
            initial[5] = r0 * r0;
            initial[6] = initial[5] - r1 * r1;

            auto result = minimizer(initial, maxIterations, updateLengthTolerance,
                errorDifferenceTolerance, lambdaFactor, lambdaAdjust, maxAdjustments);

            // No test is made for result.converged so that we return some
            // estimates of the torus.  The caller can decide how to respond
            // when result.converged is false.
            C[0] = result.minLocation[0];
            C[1] = result.minLocation[1];
            C[2] = result.minLocation[2];

            Real theta = result.minLocation[3];
            Real phi = result.minLocation[4];
            Real csTheta = std::cos(theta);
            Real snTheta = std::sin(theta);
            Real csPhi = std::cos(phi);
            Real snPhi = std::sin(phi);
            N[0] = csTheta * snPhi;
            N[1] = snTheta * snPhi;
            N[2] = csPhi;

            Real u = result.minLocation[5];
            Real v = result.minLocation[6];
            r0 = std::sqrt(u);
            r1 = std::sqrt(u - v);

            mNumPoints = 0;
            mPoints = nullptr;
            return result;
        }

    private:
        typedef BSRational<UIntegerAP32> Rational;

        mutable int mNumPoints;
        mutable Vector<3, Real> const* mPoints;
        std::function<void(GVector<Real> const&, GVector<Real>&)> mFFunction;
        std::function<void(GVector<Real> const&, GMatrix<Real>&)> mJFunction;
    };
}
