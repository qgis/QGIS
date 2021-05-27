// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// Let f(t,A) = sin(t*A)/sin(A).  The slerp of quaternions q0 and q1 is
//   slerp(t,q0,q1) = f(1-t,A)*q0 + f(t,A)*q1.
// Let y = 1-cos(A); we allow A in [0,pi], so y in [0,1].  As a function of y,
// a series representation for f(t,y) is
//   f(t,y) = sum_{i=0}^{infinity} c_{i}(t) y^{i}
// where c_0(t) = t, c_{i}(t) = c_{i-1}(t)*(i^2 - t^2)/(i*(2*i+1)) for i >= 1.
// The c_{i}(t) are polynomials in t of degree 2*i+1.  The document
// https://www.geometrictools/com/Documentation/FastAndAccurateSlerp.pdf
// derives an approximation
//   g(t,y) = sum_{i=0}^{n-1} c_{i}(t) y^{i} + (1+u_n) c_{n}(t) y^n
// which has degree 2*n+1 in t and degree n in y.
//
// Given q0 and q1 such that cos(A) = dot(q0,q1) in [0,1], in which case
// A in [0,pi/2], let qh = (q0+q1)/|q0 + q1| = slerp(1/2,q0,q1).  Note that
// |q0 + q1| = 2*cos(A/2) because
//   sin(A/2)/sin(A) = sin(A/2)/(2*sin(A/2)*cos(A/2)) = 1/(2*cos(A/2))
// The angle between q0 and qh is the same as the angle between qh and q1,
// namely, A/2 in [0,pi/4].  It may be shown that
//                    +--
//   slerp(t,q0,q1) = | slerp(2*t,q0,qh),     0 <= t <= 1/2
//                    | slerp(2*t-1,qh,q1), 1/2 <= t <= 1
//                    +--
// The slerp functions on the right-hand side involve angles in [0,pi/4], so
// the approximation is more accurate for those evaluations than using the
// original.
//
// TODO: The constants in GetEstimate are those of the published paper.
// Modify these to match the aforementioned GT document.

namespace gte
{
    template <typename Real>
    class ChebyshevRatio
    {
    public:
        // Compute f(t,A) = sin(t*A)/sin(A), where t in [0,1], A in [0,pi/2],
        // cosA = cos(A), f0 = f(1-t,A), and f1 = f(t,A).
        static void Get(Real t, Real cosA, Real& f0, Real& f1)
        {
            if (cosA < (Real)1)
            {
                // The angle A is in (0,pi/2].
                Real A = std::acos(cosA);
                Real invSinA = (Real)1 / std::sin(A);
                f0 = std::sin(((Real)1 - t) * A) * invSinA;
                f1 = std::sin(t * A) * invSinA;
            }
            else
            {
                // The angle theta is 0.
                f0 = (Real)1 - t;
                f1 = (Real)t;
            }
        }

        // Compute estimates for f(t,y) = sin(t*A)/sin(A), where t in [0,1],
        // A in [0,pi/2], y = 1 - cos(A), f0 is the estimate for f(1-t,y), and
        // f1 is the estimate for f(t,y).  The approximating function is a
        // polynomial of two variables. The template parameter N must be in
        // {1..16}.  The degree in t is 2*N+1 and the degree in Y is N.
        template <int N>
        static void GetEstimate(Real t, Real y, Real & f0, Real & f1)
        {
            static_assert(1 <= N && N <= 16, "Invalid degree.");

            // The ASM output shows that the constants/ in these arrays are
            // loaded to XMM registers as literal values, and only those
            // constants required for the specified degree D are loaded.
            // That is, the compiler does a good job of optimizing the code.

            Real const onePlusMu[16] =
            {
                (Real)1.62943436108234530,
                (Real)1.73965850021313961,
                (Real)1.79701067629566813,
                (Real)1.83291820510335812,
                (Real)1.85772477879039977,
                (Real)1.87596835698904785,
                (Real)1.88998444919711206,
                (Real)1.90110745351730037,
                (Real)1.91015881189952352,
                (Real)1.91767344933047190,
                (Real)1.92401541194159076,
                (Real)1.92944142668012797,
                (Real)1.93413793373091059,
                (Real)1.93824371262559758,
                (Real)1.94186426368404708,
                (Real)1.94508125972497303
            };

            Real const a[16] =
            {
                (N != 1 ? (Real)1 : onePlusMu[0]) / ((Real)1 * (Real)3),
                (N != 2 ? (Real)1 : onePlusMu[1]) / ((Real)2 * (Real)5),
                (N != 3 ? (Real)1 : onePlusMu[2]) / ((Real)3 * (Real)7),
                (N != 4 ? (Real)1 : onePlusMu[3]) / ((Real)4 * (Real)9),
                (N != 5 ? (Real)1 : onePlusMu[4]) / ((Real)5 * (Real)11),
                (N != 6 ? (Real)1 : onePlusMu[5]) / ((Real)6 * (Real)13),
                (N != 7 ? (Real)1 : onePlusMu[6]) / ((Real)7 * (Real)15),
                (N != 8 ? (Real)1 : onePlusMu[7]) / ((Real)8 * (Real)17),
                (N != 9 ? (Real)1 : onePlusMu[8]) / ((Real)9 * (Real)19),
                (N != 10 ? (Real)1 : onePlusMu[9]) / ((Real)10 * (Real)21),
                (N != 11 ? (Real)1 : onePlusMu[10]) / ((Real)11 * (Real)23),
                (N != 12 ? (Real)1 : onePlusMu[11]) / ((Real)12 * (Real)25),
                (N != 13 ? (Real)1 : onePlusMu[12]) / ((Real)13 * (Real)27),
                (N != 14 ? (Real)1 : onePlusMu[13]) / ((Real)14 * (Real)29),
                (N != 15 ? (Real)1 : onePlusMu[14]) / ((Real)15 * (Real)31),
                (N != 16 ? (Real)1 : onePlusMu[15]) / ((Real)16 * (Real)33)
            };

            Real const b[16] =
            {
                (N != 1 ? (Real)1 : onePlusMu[0]) * (Real)1 / (Real)3,
                (N != 2 ? (Real)1 : onePlusMu[1]) * (Real)2 / (Real)5,
                (N != 3 ? (Real)1 : onePlusMu[2]) * (Real)3 / (Real)7,
                (N != 4 ? (Real)1 : onePlusMu[3]) * (Real)4 / (Real)9,
                (N != 5 ? (Real)1 : onePlusMu[4]) * (Real)5 / (Real)11,
                (N != 6 ? (Real)1 : onePlusMu[5]) * (Real)6 / (Real)13,
                (N != 7 ? (Real)1 : onePlusMu[6]) * (Real)7 / (Real)15,
                (N != 8 ? (Real)1 : onePlusMu[7]) * (Real)8 / (Real)17,
                (N != 9 ? (Real)1 : onePlusMu[8]) * (Real)9 / (Real)19,
                (N != 10 ? (Real)1 : onePlusMu[9]) * (Real)10 / (Real)21,
                (N != 11 ? (Real)1 : onePlusMu[10]) * (Real)11 / (Real)23,
                (N != 12 ? (Real)1 : onePlusMu[11]) * (Real)12 / (Real)25,
                (N != 13 ? (Real)1 : onePlusMu[12]) * (Real)13 / (Real)27,
                (N != 14 ? (Real)1 : onePlusMu[13]) * (Real)14 / (Real)29,
                (N != 15 ? (Real)1 : onePlusMu[14]) * (Real)15 / (Real)31,
                (N != 16 ? (Real)1 : onePlusMu[15]) * (Real)16 / (Real)33
            };

            Real term0 = (Real)1 - t, term1 = t;
            Real sqr0 = term0 * term0, sqr1 = term1 * term1;
            f0 = term0;
            f1 = term1;
            for (int i = 0; i < N; ++i)
            {
                term0 *= (b[i] - a[i] * sqr0) * y;
                term1 *= (b[i] - a[i] * sqr1) * y;
                f0 += term0;
                f1 += term1;
            }
        }
    };
}
