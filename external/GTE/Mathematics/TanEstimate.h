// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// Minimax polynomial approximations to tan(x).  The polynomial p(x) of
// degree D has only odd-power terms, is required to have linear term x,
// and p(pi/4) = tan(pi/4) = 1.  It minimizes the quantity
// maximum{|tan(x) - p(x)| : x in [-pi/4,pi/4]} over all polynomials of
// degree D subject to the constraints mentioned.

namespace gte
{
    template <typename Real>
    class TanEstimate
    {
    public:
        // The input constraint is x in [-pi/4,pi/4].  For example,
        //   float x; // in [-pi/4,pi/4]
        //   float result = TanEstimate<float>::Degree<3>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            return Evaluate(degree<D>(), x);
        }

        // The input x can be any real number.  Range reduction is used to
        // generate a value y in [-pi/2,pi/2].  If |y| <= pi/4, then the
        // polynomial is evaluated.  If y in (pi/4,pi/2), set z = y - pi/4
        // and use the identity
        //   tan(y) = tan(z + pi/4) = [1 + tan(z)]/[1 - tan(z)]
        // Be careful when evaluating at y nearly pi/2, because tan(y)
        // becomes infinite.  For example,
        //   float x;  // x any real number
        //   float result = TanEstimate<float>::DegreeRR<3>(x);
        template <int D>
        inline static Real DegreeRR(Real x)
        {
            Real y;
            Reduce(x, y);
            if (std::fabs(y) <= (Real)GTE_C_QUARTER_PI)
            {
                return Degree<D>(y);
            }
            else if (y > (Real)GTE_C_QUARTER_PI)
            {
                Real poly = Degree<D>(y - (Real)GTE_C_QUARTER_PI);
                return ((Real)1 + poly) / ((Real)1 - poly);
            }
            else
            {
                Real poly = Degree<D>(y + (Real)GTE_C_QUARTER_PI);
                return -((Real)1 - poly) / ((Real)1 + poly);
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
            poly = (Real)GTE_C_TAN_DEG3_C1;
            poly = (Real)GTE_C_TAN_DEG3_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<5>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_TAN_DEG5_C2;
            poly = (Real)GTE_C_TAN_DEG5_C1 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG5_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<7>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_TAN_DEG7_C3;
            poly = (Real)GTE_C_TAN_DEG7_C2 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG7_C1 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG7_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<9>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_TAN_DEG9_C4;
            poly = (Real)GTE_C_TAN_DEG9_C3 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG9_C2 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG9_C1 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG9_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<11>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_TAN_DEG11_C5;
            poly = (Real)GTE_C_TAN_DEG11_C4 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG11_C3 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG11_C2 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG11_C1 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG11_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<13>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_TAN_DEG13_C6;
            poly = (Real)GTE_C_TAN_DEG13_C5 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG13_C4 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG13_C3 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG13_C2 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG13_C1 + poly * xsqr;
            poly = (Real)GTE_C_TAN_DEG13_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        // Support for range reduction.
        inline static void Reduce(Real x, Real& y)
        {
            // Map x to y in [-pi,pi], x = pi*quotient + remainder.
            y = std::fmod(x, (Real)GTE_C_PI);

            // Map y to [-pi/2,pi/2] with tan(y) = tan(x).
            if (y > (Real)GTE_C_HALF_PI)
            {
                y -= (Real)GTE_C_PI;
            }
            else if (y < (Real)-GTE_C_HALF_PI)
            {
                y += (Real)GTE_C_PI;
            }
        }
    };
}
