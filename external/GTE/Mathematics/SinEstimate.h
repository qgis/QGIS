// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// Minimax polynomial approximations to sin(x).  The polynomial p(x) of
// degree D has only odd-power terms, is required to have linear term x,
// and p(pi/2) = sin(pi/2) = 1.  It minimizes the quantity
// maximum{|sin(x) - p(x)| : x in [-pi/2,pi/2]} over all polynomials of
// degree D subject to the constraints mentioned.

namespace gte
{
    template <typename Real>
    class SinEstimate
    {
    public:
        // The input constraint is x in [-pi/2,pi/2].  For example,
        //   float x; // in [-pi/2,pi/2]
        //   float result = SinEstimate<float>::Degree<3>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            return Evaluate(degree<D>(), x);
        }

        // The input x can be any real number.  Range reduction is used to
        // generate a value y in [-pi/2,pi/2] for which sin(y) = sin(x).
        // For example,
        //   float x;  // x any real number
        //   float result = SinEstimate<float>::DegreeRR<3>(x);
        template <int D>
        inline static Real DegreeRR(Real x)
        {
            return Degree<D>(Reduce(x));
        }

    private:
        // Metaprogramming and private implementation to allow specialization
        // of a template member function.
        template <int D> struct degree {};

        inline static Real Evaluate(degree<3>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_SIN_DEG3_C1;
            poly = (Real)GTE_C_SIN_DEG3_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<5>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_SIN_DEG5_C2;
            poly = (Real)GTE_C_SIN_DEG5_C1 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG5_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<7>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_SIN_DEG7_C3;
            poly = (Real)GTE_C_SIN_DEG7_C2 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG7_C1 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG7_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<9>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_SIN_DEG9_C4;
            poly = (Real)GTE_C_SIN_DEG9_C3 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG9_C2 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG9_C1 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG9_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        inline static Real Evaluate(degree<11>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_SIN_DEG11_C5;
            poly = (Real)GTE_C_SIN_DEG11_C4 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG11_C3 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG11_C2 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG11_C1 + poly * xsqr;
            poly = (Real)GTE_C_SIN_DEG11_C0 + poly * xsqr;
            poly = poly * x;
            return poly;
        }

        // Support for range reduction.
        inline static Real Reduce(Real x)
        {
            // Map x to y in [-pi,pi], x = 2*pi*quotient + remainder.
            Real quotient = (Real)GTE_C_INV_TWO_PI * x;
            if (x >= (Real)0)
            {
                quotient = (Real)((int)(quotient + (Real)0.5));
            }
            else
            {
                quotient = (Real)((int)(quotient - (Real)0.5));
            }
            Real y = x - (Real)GTE_C_TWO_PI * quotient;

            // Map y to [-pi/2,pi/2] with sin(y) = sin(x).
            if (y > (Real)GTE_C_HALF_PI)
            {
                y = (Real)GTE_C_PI - y;
            }
            else if (y < (Real)-GTE_C_HALF_PI)
            {
                y = (Real)-GTE_C_PI - y;
            }
            return y;
        }
    };
}
