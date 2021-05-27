// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector2.h>

// The queries consider the box to be a solid.
//
// The test-intersection query uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The set of potential separating directions includes the 2 edge normals
// of box0 and the 2 edge normals of box1.  The integer 'separating'
// identifies the axis that reported separation; there may be more than one
// but only one is reported.  The value is 0 when box0.axis[0] separates,
// 1 when box0.axis[1] separates, 2 when box1.axis[0] separates, or 3 when
// box1.axis[1] separates.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, AlignedBox2<Real>, OrientedBox2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            int separating;
        };

        Result operator()(AlignedBox2<Real> const& box0, OrientedBox2<Real> const& box1)
        {
            Result result;

            // Get the centered form of the aligned box.  The axes are
            // implicitly A0[0] = (1,0) and A0[1] = (0,1).
            Vector2<Real> C0, E0;
            box0.GetCenteredForm(C0, E0);

            // Convenience variables.
            Vector2<Real> const& C1 = box1.center;
            Vector2<Real> const* A1 = &box1.axis[0];
            Vector2<Real> const& E1 = box1.extent;

            // Compute difference of box centers.
            Vector2<Real> D = C1 - C0;

            Real absDot01[2][2], rSum;

            // Test box0.axis[0] = (1,0).
            absDot01[0][0] = std::fabs(A1[0][0]);
            absDot01[0][1] = std::fabs(A1[1][0]);
            rSum = E0[0] + E1[0] * absDot01[0][0] + E1[1] * absDot01[0][1];
            if (std::fabs(D[0]) > rSum)
            {
                result.intersect = false;
                result.separating = 0;
                return result;
            }

            // Test axis box0.axis[1] = (0,1).
            absDot01[1][0] = std::fabs(A1[0][1]);
            absDot01[1][1] = std::fabs(A1[1][1]);
            rSum = E0[1] + E1[0] * absDot01[1][0] + E1[1] * absDot01[1][1];
            if (std::fabs(D[1]) > rSum)
            {
                result.intersect = false;
                result.separating = 1;
                return result;
            }

            // Test axis box1.axis[0].
            rSum = E1[0] + E0[0] * absDot01[0][0] + E0[1] * absDot01[1][0];
            if (std::fabs(Dot(A1[0], D)) > rSum)
            {
                result.intersect = false;
                result.separating = 2;
                return result;
            }

            // Test axis box1.axis[1].
            rSum = E1[1] + E0[0] * absDot01[0][1] + E0[1] * absDot01[1][1];
            if (std::fabs(Dot(A1[1], D)) > rSum)
            {
                result.intersect = false;
                result.separating = 3;
                return result;
            }

            result.intersect = true;
            return result;
        }
    };
}
