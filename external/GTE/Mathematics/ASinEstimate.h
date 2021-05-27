// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ACosEstimate.h>

// Approximations to asin(x) of the form f(x) = pi/2 - sqrt(1-x)*p(x)
// where the polynomial p(x) of degree D minimizes the quantity
// maximum{|acos(x)/sqrt(1-x) - p(x)| : x in [0,1]} over all
// polynomials of degree D.  We use the identity asin(x) = pi/2 - acos(x).

namespace gte
{
    template <typename Real>
    class ASinEstimate
    {
    public:
        // The input constraint is x in [0,1].  For example,
        //   float x; // in [0,1]
        //   float result = ASinEstimate<float>::Degree<3>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            return (Real)GTE_C_HALF_PI - ACosEstimate<Real>::Degree<D>(x);
        }
    };
}
