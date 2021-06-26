// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/SymmetricEigensolver.h>

namespace gte
{
    // The quadratic fit is
    //   0 = C[0] + C[1]*X + C[2]*Y + C[3]*X^2 + C[4]*Y^2 + C[5]*X*Y
    // subject to Length(C) = 1.  Minimize E(C) = C^t M C with Length(C) = 1
    // and M = (sum_i V_i)(sum_i V_i)^t where
    //   V = (1, X, Y, X^2, Y^2, X*Y)
    // The minimum value is the smallest eigenvalue of M and C is a
    // corresponding unit length eigenvector.
    //
    // Input:
    //   n = number of points to fit
    //   p[0..n-1] = array of points to fit
    //
    // Output:
    //   c[0..5] = coefficients of quadratic fit (the eigenvector)
    //   return value of function is nonnegative and a measure of the fit
    //   (the minimum eigenvalue; 0 = exact fit, positive otherwise)
    //
    // Canonical forms. The quadratic equation can be factored into
    // P^T A P + B^T P + K = 0 where P = (X,Y,Z), K = C[0],
    // B = (C[1],C[2],C[3]), and A is a 3x3 symmetric matrix with
    // A00 = C[4], A11 = C[5], A22 = C[6], A01 = C[7]/2, A02 = C[8]/2,
    // and A12 = C[9]/2. Matrix A = R^T D R where R is orthogonal and
    // D is diagonal (using an eigendecomposition). Define
    // V = R P = (v0,v1,v2), E = R B = (e0,e1,e2), D = diag(d0,d1,d2)
    // and f = K to obtain
    //   d0 v0^2 + d1 v1^2 + d2 v^2 + e0 v0 + e1 v1 + e2 v2 + f = 0
    // The characterization depends on the signs of the d_i.

    template <typename Real>
    class ApprQuadratic2
    {
    public:
        Real operator()(int numPoints, Vector2<Real> const* points, Real coefficients[6])
        {
            Matrix<6, 6, Real> A;  // constructor sets A to zero
            for (int i = 0; i < numPoints; ++i)
            {
                Real x = points[i][0];
                Real y = points[i][1];
                Real x2 = x * x;
                Real y2 = y * y;
                Real xy = x * y;
                Real x3 = x * x2;
                Real xy2 = x * y2;
                Real x2y = x * xy;
                Real y3 = y * y2;
                Real x4 = x * x3;
                Real x2y2 = x * xy2;
                Real x3y = x * x2y;
                Real y4 = y * y3;
                Real xy3 = x * y3;

                A(0, 1) += x;
                A(0, 2) += y;
                A(0, 3) += x2;
                A(0, 4) += y2;
                A(0, 5) += xy;
                A(1, 3) += x3;
                A(1, 4) += xy2;
                A(1, 5) += x2y;
                A(2, 4) += y3;
                A(3, 3) += x4;
                A(3, 4) += x2y2;
                A(3, 5) += x3y;
                A(4, 4) += y4;
                A(4, 5) += xy3;
            }

            A(0, 0) = static_cast<Real>(numPoints);
            A(1, 1) = A(0, 3);
            A(1, 2) = A(0, 5);
            A(2, 2) = A(0, 4);
            A(2, 3) = A(1, 5);
            A(2, 5) = A(1, 4);
            A(5, 5) = A(3, 4);

            for (int row = 0; row < 6; ++row)
            {
                for (int col = 0; col < row; ++col)
                {
                    A(row, col) = A(col, row);
                }
            }

            Real invNumPoints = (Real)1 / static_cast<Real>(numPoints);
            for (int row = 0; row < 6; ++row)
            {
                for (int col = 0; col < 6; ++col)
                {
                    A(row, col) *= invNumPoints;
                }
            }

            SymmetricEigensolver<Real> es(6, 1024);
            es.Solve(&A[0], +1);
            es.GetEigenvector(0, &coefficients[0]);

            // For an exact fit, numeric round-off errors might make the
            // minimum eigenvalue just slightly negative. Return the
            // absolute value because the application might rely on the
            // return value being nonnegative.
            return std::fabs(es.GetEigenvalue(0));
        }
    };


    // If you think your points are nearly circular, use this. The circle is
    // of the form C'[0]+C'[1]*X+C'[2]*Y+C'[3]*(X^2+Y^2), where
    // Length(C') = 1.  The function returns
    // C = (C'[0]/C'[3],C'[1]/C'[3],C'[2]/C'[3]), so the fitted circle is
    // C[0]+C[1]*X+C[2]*Y+X^2+Y^2. The center is (xc,yc) = -0.5*(C[1],C[2])
    // and the radius is r = sqrt(xc*xc+yc*yc-C[0]).

    template <typename Real>
    class ApprQuadraticCircle2
    {
    public:
        Real operator()(int numPoints, Vector2<Real> const* points, Circle2<Real>& circle)
        {
            Matrix<4, 4, Real> A;  // constructor sets A to zero
            for (int i = 0; i < numPoints; ++i)
            {
                Real x = points[i][0];
                Real y = points[i][1];
                Real x2 = x * x;
                Real y2 = y * y;
                Real xy = x * y;
                Real r2 = x2 + y2;
                Real xr2 = x * r2;
                Real yr2 = y * r2;
                Real r4 = r2 * r2;

                A(0, 1) += x;
                A(0, 2) += y;
                A(0, 3) += r2;
                A(1, 1) += x2;
                A(1, 2) += xy;
                A(1, 3) += xr2;
                A(2, 2) += y2;
                A(2, 3) += yr2;
                A(3, 3) += r4;
            }

            A(0, 0) = static_cast<Real>(numPoints);

            for (int row = 0; row < 4; ++row)
            {
                for (int col = 0; col < row; ++col)
                {
                    A(row, col) = A(col, row);
                }
            }

            Real invNumPoints = (Real)1 / static_cast<Real>(numPoints);
            for (int row = 0; row < 4; ++row)
            {
                for (int col = 0; col < 4; ++col)
                {
                    A(row, col) *= invNumPoints;
                }
            }

            SymmetricEigensolver<Real> es(4, 1024);
            es.Solve(&A[0], +1);
            Vector<4, Real> evector;
            es.GetEigenvector(0, &evector[0]);

            // TODO: Guard against zero divide?
            Real inv = (Real)1 / evector[3];
            Real coefficients[3];
            for (int row = 0; row < 3; ++row)
            {
                coefficients[row] = inv * evector[row];
            }

            circle.center[0] = (Real)-0.5 * coefficients[1];
            circle.center[1] = (Real)-0.5 * coefficients[2];
            circle.radius = std::sqrt(std::fabs(Dot(circle.center, circle.center) - coefficients[0]));

            // For an exact fit, numeric round-off errors might make the
            // minimum eigenvalue just slightly negative. Return the
            // absolute value because the application might rely on the
            // return value being nonnegative.
            return std::fabs(es.GetEigenvalue(0));
        }
    };
}
