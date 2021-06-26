// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>
#include <algorithm>
#include <array>

namespace gte
{
    template <typename Real>
    class SymmetricEigensolver2x2
    {
    public:
        // The input matrix must be symmetric, so only the unique elements
        // must be specified: a00, a01, and a11.
        //
        // The order of the eigenvalues is specified by sortType:
        // -1 (decreasing), 0 (no sorting) or +1 (increasing).  When sorted,
        // the eigenvectors are ordered accordingly, and {evec[0], evec[1]}
        // is guaranteed to be a right-handed orthonormal set.

        void operator()(Real a00, Real a01, Real a11, int sortType,
            std::array<Real, 2>& eval, std::array<std::array<Real, 2>, 2>& evec) const
        {
            // Normalize (c2,s2) robustly, avoiding floating-point overflow
            // in the sqrt call.
            Real const zero = (Real)0, one = (Real)1, half = (Real)0.5;
            Real c2 = half * (a00 - a11), s2 = a01;
            Real maxAbsComp = std::max(std::fabs(c2), std::fabs(s2));
            if (maxAbsComp > zero)
            {
                c2 /= maxAbsComp;  // in [-1,1]
                s2 /= maxAbsComp;  // in [-1,1]
                Real length = std::sqrt(c2 * c2 + s2 * s2);
                c2 /= length;
                s2 /= length;
                if (c2 > zero)
                {
                    c2 = -c2;
                    s2 = -s2;
                }
            }
            else
            {
                c2 = -one;
                s2 = zero;
            }

            Real s = std::sqrt(half * (one - c2));  // >= 1/sqrt(2)
            Real c = half * s2 / s;

            Real diagonal[2];
            Real csqr = c * c, ssqr = s * s, mid = s2 * a01;
            diagonal[0] = csqr * a00 + mid + ssqr * a11;
            diagonal[1] = csqr * a11 - mid + ssqr * a00;

            if (sortType == 0 || sortType * diagonal[0] <= sortType * diagonal[1])
            {
                eval[0] = diagonal[0];
                eval[1] = diagonal[1];
                evec[0][0] = c;
                evec[0][1] = s;
                evec[1][0] = -s;
                evec[1][1] = c;
            }
            else
            {
                eval[0] = diagonal[1];
                eval[1] = diagonal[0];
                evec[0][0] = s;
                evec[0][1] = -c;
                evec[1][0] = c;
                evec[1][1] = s;
            }
        }
    };
}
