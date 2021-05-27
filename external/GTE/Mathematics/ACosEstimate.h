// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// Approximations to acos(x) of the form f(x) = sqrt(1-x)*p(x)
// where the polynomial p(x) of degree D minimizes the quantity
// maximum{|acos(x)/sqrt(1-x) - p(x)| : x in [0,1]} over all
// polynomials of degree D.

namespace gte
{
    template <typename Real>
    class ACosEstimate
    {
    public:
        // The input constraint is x in [0,1].  For example,
        //   float x; // in [0,1]
        //   float result = ACosEstimate<float>::Degree<3>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            return Evaluate(degree<D>(), x);
        }

    private:
        // Metaprogramming and private implementation to allow specialization
        // of a template member function.
        template <int D> struct degree {};

        inline static Real Evaluate(degree<1>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG1_C1;
            poly = (Real)GTE_C_ACOS_DEG1_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }

        inline static Real Evaluate(degree<2>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG2_C2;
            poly = (Real)GTE_C_ACOS_DEG2_C1 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG2_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }

        inline static Real Evaluate(degree<3>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG3_C3;
            poly = (Real)GTE_C_ACOS_DEG3_C2 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG3_C1 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG3_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }

        inline static Real Evaluate(degree<4>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG4_C4;
            poly = (Real)GTE_C_ACOS_DEG4_C3 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG4_C2 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG4_C1 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG4_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }

        inline static Real Evaluate(degree<5>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG5_C5;
            poly = (Real)GTE_C_ACOS_DEG5_C4 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG5_C3 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG5_C2 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG5_C1 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG5_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }

        inline static Real Evaluate(degree<6>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG6_C6;
            poly = (Real)GTE_C_ACOS_DEG6_C5 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG6_C4 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG6_C3 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG6_C2 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG6_C1 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG6_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }

        inline static Real Evaluate(degree<7>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG7_C7;
            poly = (Real)GTE_C_ACOS_DEG7_C6 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG7_C5 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG7_C4 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG7_C3 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG7_C2 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG7_C1 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG7_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }

        inline static Real Evaluate(degree<8>, Real x)
        {
            Real poly;
            poly = (Real)GTE_C_ACOS_DEG8_C8;
            poly = (Real)GTE_C_ACOS_DEG8_C7 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG8_C6 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG8_C5 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG8_C4 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG8_C3 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG8_C2 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG8_C1 + poly * x;
            poly = (Real)GTE_C_ACOS_DEG8_C0 + poly * x;
            poly = poly * std::sqrt((Real)1 - x);
            return poly;
        }
    };
}
