// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// Minimax polynomial approximations to cos(x).  The polynomial p(x) of
// degree D has only even-power terms, is required to have constant term 1,
// and p(pi/2) = cos(pi/2) = 0.  It minimizes the quantity
// maximum{|cos(x) - p(x)| : x in [-pi/2,pi/2]} over all polynomials of
// degree D subject to the constraints mentioned.

namespace gte
{
    template <typename Real>
    class CosEstimate
    {
    public:
        // The input constraint is x in [-pi/2,pi/2].  For example,
        //   float x; // in [-pi/2,pi/2]
        //   float result = CosEstimate<float>::Degree<4>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            return Evaluate(degree<D>(), x);
        }

        // The input x can be any real number.  Range reduction is used to
        // generate a value y in [-pi/2,pi/2] and a sign s for which
        // cos(y) = s*cos(x).  For example,
        //   float x;  // x any real number
        //   float result = CosEstimate<float>::DegreeRR<3>(x);
        template <int D>
        inline static Real DegreeRR(Real x)
        {
            Real y, sign;
            Reduce(x, y, sign);
            Real poly = sign * Degree<D>(y);
            return poly;
        }

    private:
        // Metaprogramming and private implementation to allow specialization
        // of a template member function.
        template <int D> struct degree {};

        inline static Real Evaluate(degree<2>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_COS_DEG2_C1;
            poly = (Real)GTE_C_COS_DEG2_C0 + poly * xsqr;
            return poly;
        }

        inline static Real Evaluate(degree<4>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_COS_DEG4_C2;
            poly = (Real)GTE_C_COS_DEG4_C1 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG4_C0 + poly * xsqr;
            return poly;
        }

        inline static Real Evaluate(degree<6>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_COS_DEG6_C3;
            poly = (Real)GTE_C_COS_DEG6_C2 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG6_C1 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG6_C0 + poly * xsqr;
            return poly;
        }

        inline static Real Evaluate(degree<8>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_COS_DEG8_C4;
            poly = (Real)GTE_C_COS_DEG8_C3 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG8_C2 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG8_C1 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG8_C0 + poly * xsqr;
            return poly;
        }

        inline static Real Evaluate(degree<10>, Real x)
        {
            Real xsqr = x * x;
            Real poly;
            poly = (Real)GTE_C_COS_DEG10_C5;
            poly = (Real)GTE_C_COS_DEG10_C4 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG10_C3 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG10_C2 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG10_C1 + poly * xsqr;
            poly = (Real)GTE_C_COS_DEG10_C0 + poly * xsqr;
            return poly;
        }

        // Support for range reduction.
        inline static void Reduce(Real x, Real& y, Real& sign)
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
            y = x - (Real)GTE_C_TWO_PI * quotient;

            // Map y to [-pi/2,pi/2] with cos(y) = sign*cos(x).
            if (y > (Real)GTE_C_HALF_PI)
            {
                y = (Real)GTE_C_PI - y;
                sign = (Real)-1;
            }
            else if (y < (Real)-GTE_C_HALF_PI)
            {
                y = (Real)-GTE_C_PI - y;
                sign = (Real)-1;
            }
            else
            {
                sign = (Real)1;
            }
        }
    };
}
