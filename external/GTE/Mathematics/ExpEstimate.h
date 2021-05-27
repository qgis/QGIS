// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.12.23

#pragma once

#include <Mathematics/Exp2Estimate.h>

// Minimax polynomial approximations to 2^x.  The polynomial p(x) of
// degree D minimizes the quantity maximum{|2^x - p(x)| : x in [0,1]}
// over all polynomials of degree D.  The natural exponential is
// computed using exp(x) = 2^{x/log(2)}, where log(2) is the natural
// logarithm of 2.

namespace gte
{
    template <typename Real>
    class ExpEstimate
    {
    public:
        // The input constraint is x in [0,1].  For example,
        //   float x; // in [0,1]
        //   float result = ExpEstimate<float>::Degree<3>(x);
        template <int D>
        inline static Real Degree(Real x)
        {
            return Exp2Estimate<Real>::Degree<D>(x * (Real)GTE_C_INV_LN_2);
        }

        // The input x can be any real number.  Range reduction is used to
        // generate a value y in [0,1], call Degree(y), and combine the output
        // with the proper exponent to obtain the approximation.  For example,
        //   float x;  // x >= 0
        //   float result = ExpEstimate<float>::DegreeRR<3>(x);
        template <int D>
        inline static Real DegreeRR(Real x)
        {
            return Exp2Estimate<Real>::DegreeRR<D>(x * (Real)GTE_C_INV_LN_2);
        }
    };
}
