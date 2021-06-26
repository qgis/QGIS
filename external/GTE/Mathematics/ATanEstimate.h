// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// Minimax polynomial approximations to atan(x).  The polynomial p(x) of
// degree D has only odd-power terms, is required to have linear term x,
// and p(1) = atan(1) = pi/4.  It minimizes the quantity
// maximum{|atan(x) - p(x)| : x in [-1,1]} over all polynomials of
// degree D subject to the constraints mentioned.

namespace gte
{
    template <typename Real>
    class ATanEstimate
    {
    public:
        // The input constraint is x in [-1,1].  For example,
        //   float x; // in [-1,1]
        //   float result = ATanEstimate<float>::Degree<3>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            return Evaluate(degree<D>(), x);
        }

        // The input x can be any real number.  Range reduction is used via
        // the identities atan(x) = pi/2 - atan(1/x) for x > 0, and
        // atan(x) = -pi/2 - atan(1/x) for x < 0.  For example,
        //   float x;  // x any real number
        //   float result = ATanEstimate<float>::DegreeRR<3>(x);
        template <int D>
        inline static Real DegreeRR(Real x)
        {
            if (std::fabs(x) <= (Real)1)
            {
                return Degree<D>(x);
            }
            else if (x > (Real)1)
            {
                return (Real)GTE_C_HALF_PI - Degree<D>((Real)1 / x);
            }
            else
            {
                return (Real)-GTE_C_HALF_PI - Degree<D>((Real)1 / x);
            }
        }

    private:
        // Metaprogramming and private implementation to allow specialization
        // of a template member function.
        template <int D> struct degree {};

        inline static Real Evaluate(degree<3>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_ATAN_DEG3_C1;
            poly = (Real)GTE_C_ATAN_DEG3_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<5>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_ATAN_DEG5_C2;
            poly = (Real)GTE_C_ATAN_DEG5_C1 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG5_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<7>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_ATAN_DEG7_C3;
            poly = (Real)GTE_C_ATAN_DEG7_C2 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG7_C1 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG7_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<9>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_ATAN_DEG9_C4;
            poly = (Real)GTE_C_ATAN_DEG9_C3 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG9_C2 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG9_C1 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG9_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<11>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_ATAN_DEG11_C5;
            poly = (Real)GTE_C_ATAN_DEG11_C4 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG11_C3 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG11_C2 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG11_C1 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG11_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<13>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_ATAN_DEG13_C6;
            poly = (Real)GTE_C_ATAN_DEG13_C5 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG13_C4 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG13_C3 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG13_C2 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG13_C1 + poly * xsqr;
            poly = (Real)GTE_C_ATAN_DEG13_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }
    };
}
