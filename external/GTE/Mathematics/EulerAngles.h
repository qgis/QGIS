// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector.h>

// The Euler angle data structure for representing rotations.  See the
// document
//   https://www.geometrictools.com/Documentation/EulerAngles.pdf

namespace gte
{
    // Factorization into Euler angles is not necessarily unique.  Let the
    // integer indices for the axes be (N0,N1,N2), which must be in the set
    //   {(0,1,2),(0,2,1),(1,0,2),(1,2,0),(2,0,1),(2,1,0),
    //    (0,1,0),(0,2,0),(1,0,1),(1,2,1),(2,0,2),(2,1,2)}
    // Let the corresponding angles be (angleN0,angleN1,angleN2).  If the
    // result is ER_NOT_UNIQUE_SUM, then the multiple solutions occur because
    // angleN2+angleN0 is constant.  If the result is ER_NOT_UNIQUE_DIF, then
    // the multiple solutions occur because angleN2-angleN0 is constant.  In
    // either type of nonuniqueness, the function returns angleN0=0.
    enum EulerResult
    {
        // The solution is invalid (incorrect axis indices).
        ER_INVALID,

        // The solution is unique.
        ER_UNIQUE,

        // The solution is not unique.  A sum of angles is constant.
        ER_NOT_UNIQUE_SUM,

        // The solution is not unique.  A difference of angles is constant.
        ER_NOT_UNIQUE_DIF
    };

    template <typename Real>
    class EulerAngles
    {
    public:
        EulerAngles()
            :
            axis{0, 0, 0},
            angle{ (Real)0, (Real)0, (Real)0 },
            result(ER_INVALID)
        {
        }

        EulerAngles(int i0, int i1, int i2, Real a0, Real a1, Real a2)
            :
            axis{ i0, i1, i2 },
            angle{ a0, a1, a2 },
            result(ER_UNIQUE)
        {
        }

        std::array<int, 3> axis;
        std::array<Real, 3> angle;

        // This member is set during conversions from rotation matrices,
        // quaternions, or axis-angles.
        EulerResult result;
    };
}
