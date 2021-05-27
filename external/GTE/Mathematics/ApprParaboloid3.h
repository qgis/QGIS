// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/LinearSystem.h>
#include <Mathematics/Matrix.h>
#include <Mathematics/Vector3.h>

// Least-squares fit of a paraboloid to a set of point.  The paraboloid is
// of the form z = c0*x^2+c1*x*y+c2*y^2+c3*x+c4*y+c5.  A successful fit is
// indicated by return value of 'true'.
//
// Given a set of samples (x_i,y_i,z_i) for 0 <= i < N, and assuming
// that the true values lie on a paraboloid
//     z = p0*x*x + p1*x*y + p2*y*y + p3*x + p4*y + p5  = Dot(P,Q(x,y))
// where P = (p0,p1,p2,p3,p4,p5) and Q(x,y) = (x*x,x*y,y*y,x,y,1),
// select P to minimize the sum of squared errors
//     E(P) = sum_{i=0}^{N-1} [Dot(P,Q_i)-z_i]^2
// where Q_i = Q(x_i,y_i).
//
// The minimum occurs when the gradient of E is the zero vector,
//     grad(E) = 2 sum_{i=0}^{N-1} [Dot(P,Q_i)-z_i] Q_i = 0
// Some algebra converts this to a system of 6 equations in 6 unknowns:
//     [(sum_{i=0}^{N-1} Q_i Q_i^t] P = sum_{i=0}^{N-1} z_i Q_i
// The product Q_i Q_i^t is a product of the 6x1 matrix Q_i with the
// 1x6 matrix Q_i^t, the result being a 6x6 matrix.
//
// Define the 6x6 symmetric matrix A = sum_{i=0}^{N-1} Q_i Q_i^t and the 6x1
// vector B = sum_{i=0}^{N-1} z_i Q_i.  The choice for P is the solution to
// the linear system of equations A*P = B.  The entries of A and B indicate
// summations over the appropriate product of variables.  For example,
// s(x^3 y) = sum_{i=0}^{N-1} x_i^3 y_i.
//
// +-                                                     -++  +   +-      -+
// | s(x^4) s(x^3 y)   s(x^2 y^2) s(x^3)   s(x^2 y) s(x^2) ||p0|   |s(z x^2)|
// |        s(x^2 y^2) s(x y^3)   s(x^2 y) s(x y^2) s(x y) ||p1|   |s(z x y)|
// |                   s(y^4)     s(x y^2) s(y^3)   s(y^2) ||p2| = |s(z y^2)|
// |                              s(x^2)   s(x y)   s(x)   ||p3|   |s(z x)  |
// |                                       s(y^2)   s(y)   ||p4|   |s(z y)  |
// |                                                s(1)   ||p5|   |s(z)    |
// +-                                                     -++  +   +-      -+

namespace gte
{
    template <typename Real>
    class ApprParaboloid3
    {
    public:
        bool operator()(int numPoints, Vector3<Real> const* points, Real coefficients[6]) const
        {
            Matrix<6, 6, Real> A;
            Vector<6, Real> B;
            B.MakeZero();

            for (int i = 0; i < numPoints; i++)
            {
                Real x2 = points[i][0] * points[i][0];
                Real xy = points[i][0] * points[i][1];
                Real y2 = points[i][1] * points[i][1];
                Real zx = points[i][2] * points[i][0];
                Real zy = points[i][2] * points[i][1];
                Real x3 = points[i][0] * x2;
                Real x2y = x2 * points[i][1];
                Real xy2 = points[i][0] * y2;
                Real y3 = points[i][1] * y2;
                Real zx2 = points[i][2] * x2;
                Real zxy = points[i][2] * xy;
                Real zy2 = points[i][2] * y2;
                Real x4 = x2 * x2;
                Real x3y = x3 * points[i][1];
                Real x2y2 = x2 * y2;
                Real xy3 = points[i][0] * y3;
                Real y4 = y2 * y2;

                A(0, 0) += x4;
                A(0, 1) += x3y;
                A(0, 2) += x2y2;
                A(0, 3) += x3;
                A(0, 4) += x2y;
                A(0, 5) += x2;
                A(1, 2) += xy3;
                A(1, 4) += xy2;
                A(1, 5) += xy;
                A(2, 2) += y4;
                A(2, 4) += y3;
                A(2, 5) += y2;
                A(3, 3) += x2;
                A(3, 5) += points[i][0];
                A(4, 5) += points[i][1];

                B[0] += zx2;
                B[1] += zxy;
                B[2] += zy2;
                B[3] += zx;
                B[4] += zy;
                B[5] += points[i][2];
            }

            A(1, 0) = A(0, 1);
            A(1, 1) = A(0, 2);
            A(1, 3) = A(0, 4);
            A(2, 0) = A(0, 2);
            A(2, 1) = A(1, 2);
            A(2, 3) = A(1, 4);
            A(3, 0) = A(0, 3);
            A(3, 1) = A(1, 3);
            A(3, 2) = A(2, 3);
            A(3, 4) = A(1, 5);
            A(4, 0) = A(0, 4);
            A(4, 1) = A(1, 4);
            A(4, 2) = A(2, 4);
            A(4, 3) = A(3, 4);
            A(4, 4) = A(2, 5);
            A(5, 0) = A(0, 5);
            A(5, 1) = A(1, 5);
            A(5, 2) = A(2, 5);
            A(5, 3) = A(3, 5);
            A(5, 4) = A(4, 5);
            A(5, 5) = static_cast<Real>(numPoints);

            return LinearSystem<Real>().Solve(6, &A[0], &B[0], &coefficients[0]);
        }
    };
}
