// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// Minimax polynomial approximations to 1/sqrt(x).  The polynomial p(x) of
// degree D minimizes the quantity maximum{|1/sqrt(x) - p(x)| : x in [1,2]}
// over all polynomials of degree D.

namespace gte
{
    template <typename Real>
    class InvSqrtEstimate
    {
    public:
        // The input constraint is x in [1,2].  For example,
        //   float x; // in [1,2]
        //   float result = InvSqrtEstimate<float>::Degree<3>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            Real t = x - (Real)1;  // t in (0,1]
            return Evaluate(degree<D>(), t);
        }

        // The input constraint is x > 0.  Range reduction is used to generate
        // a/ value y in [1,2], call Evaluate(y), and combine the output with
        // the proper exponent to obtain the approximation.  For example,
        //   float x;  // x > 0
        //   float result = InvSqrtEstimate<float>::DegreeRR<3>(x);
        template <int D>
        inline static Real DegreeRR(Real x)
        {
            Real adj, y;
            int p;
            Reduce(x, adj, y, p);
            Real poly = Degree<D>(y);
            Real result = Combine(adj, poly, p);
            return result;
        }

    private:
        // Metaprogramming and private implementation to allow specialization
        // of a template member function.
        template <int D> struct degree {};

        inline static Real Evaluate(degree<1>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG1_C1;
            poly = (Real)GTE_C_INVSQRT_DEG1_C0 + poly * t;
            return poly;
        }

        inline static Real Evaluate(degree<2>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG2_C2;
            poly = (Real)GTE_C_INVSQRT_DEG2_C1 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG2_C0 + poly * t;
            return poly;
        }

        inline static Real Evaluate(degree<3>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG3_C3;
            poly = (Real)GTE_C_INVSQRT_DEG3_C2 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG3_C1 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG3_C0 + poly * t;
            return poly;
        }

        inline static Real Evaluate(degree<4>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG4_C4;
            poly = (Real)GTE_C_INVSQRT_DEG4_C3 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG4_C2 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG4_C1 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG4_C0 + poly * t;
            return poly;
        }

        inline static Real Evaluate(degree<5>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG5_C5;
            poly = (Real)GTE_C_INVSQRT_DEG5_C4 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG5_C3 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG5_C2 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG5_C1 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG5_C0 + poly * t;
            return poly;
        }

        inline static Real Evaluate(degree<6>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG6_C6;
            poly = (Real)GTE_C_INVSQRT_DEG6_C5 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG6_C4 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG6_C3 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG6_C2 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG6_C1 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG6_C0 + poly * t;
            return poly;
        }

        inline static Real Evaluate(degree<7>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG7_C7;
            poly = (Real)GTE_C_INVSQRT_DEG7_C6 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG7_C5 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG7_C4 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG7_C3 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG7_C2 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG7_C1 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG7_C0 + poly * t;
            return poly;
        }

        inline static Real Evaluate(degree<8>, Real t)
        {
            Real poly;
            poly = (Real)GTE_C_INVSQRT_DEG8_C8;
            poly = (Real)GTE_C_INVSQRT_DEG8_C7 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG8_C6 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG8_C5 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG8_C4 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG8_C3 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG8_C2 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG8_C1 + poly * t;
            poly = (Real)GTE_C_INVSQRT_DEG8_C0 + poly * t;
            return poly;
        }

        // Support for range reduction.
        inline static void Reduce(Real x, Real& adj, Real& y, int& p)
        {
            y = std::frexp(x, &p);  // y in [1/2,1)
            y = ((Real)2) * y;  // y in [1,2)
            --p;
            adj = (1 & p) * (Real)GTE_C_INV_SQRT_2 + (1 & ~p) * (Real)1;
            p = -(p >> 1);
        }

        inline static Real Combine(Real adj, Real y, int p)
        {
            return adj * std::ldexp(y, p);
        }
    };
}
