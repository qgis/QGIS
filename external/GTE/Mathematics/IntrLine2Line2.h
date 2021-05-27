// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector2.h>
#include <Mathematics/Line.h>
#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <limits>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line2<Real>, Line2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (lines intersect in a
            // single point) or std::numeric_limits<int>::max() (lines are
            // the same).
            int numIntersections;
        };

        Result operator()(Line2<Real> const& line0, Line2<Real> const& line1)
        {
            Result result;

            // The intersection of two lines is a solution to P0 + s0*D0 =
            // P1 + s1*D1.  Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q.  If
            // DotPerp(D0, D1)) = 0, the lines are parallel.  Additionally, if
            // DotPerp(Q, D1)) = 0, the lines are the same.  If
            // Dotperp(D0, D1)) is not zero, then
            //   s0 = DotPerp(Q, D1))/DotPerp(D0, D1))
            // produces the point of intersection.  Also,
            //   s1 = DotPerp(Q, D0))/DotPerp(D0, D1))

            Vector2<Real> diff = line1.origin - line0.origin;
            Real D0DotPerpD1 = DotPerp(line0.direction, line1.direction);
            if (D0DotPerpD1 != (Real)0)
            {
                // The lines are not parallel.
                result.intersect = true;
                result.numIntersections = 1;
            }
            else
            {
                // The lines are parallel.
                Normalize(diff);
                Real diffNDotPerpD1 = DotPerp(diff, line1.direction);
                if (diffNDotPerpD1 != (Real)0)
                {
                    // The lines are parallel but distinct.
                    result.intersect = false;
                    result.numIntersections = 0;
                }
                else
                {
                    // The lines are the same.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int>::max();
                }
            }

            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line2<Real>, Line2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (lines intersect in a
            // single point) or std::numeric_limits<int>::max() (lines are
            // the same).
            int numIntersections;

            // If numIntersections is 1, the intersection is
            //   point = line0.origin + line0parameter[0] * line0.direction
            //         = line1.origin + line1parameter[0] * line1.direction
            // If numIntersections is maxInt, point is not valid but the
            // intervals are
            //   line0Parameter[] = { -maxReal, +maxReal }
            //   line1Parameter[] = { -maxReal, +maxReal }
            Real line0Parameter[2], line1Parameter[2];
            Vector2<Real> point;
        };

        Result operator()(Line2<Real> const& line0, Line2<Real> const& line1)
        {
            Result result;

            // The intersection of two lines is a solution to P0 + s0*D0 =
            // P1 + s1*D1.  Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q.  If
            // DotPerp(D0, D1)) = 0, the lines are parallel.  Additionally, if
            // DotPerp(Q, D1)) = 0, the lines are the same.  If
            // Dotperp(D0, D1)) is not zero, then
            //   s0 = DotPerp(Q, D1))/DotPerp(D0, D1))
            // produces the point of intersection.  Also,
            //   s1 = DotPerp(Q, D0))/DotPerp(D0, D1))

            Vector2<Real> diff = line1.origin - line0.origin;
            Real D0DotPerpD1 = DotPerp(line0.direction, line1.direction);
            if (D0DotPerpD1 != (Real)0)
            {
                // The lines are not parallel.
                result.intersect = true;
                result.numIntersections = 1;
                Real invD0DotPerpD1 = (Real)1 / D0DotPerpD1;
                Real diffDotPerpD0 = DotPerp(diff, line0.direction);
                Real diffDotPerpD1 = DotPerp(diff, line1.direction);
                Real s0 = diffDotPerpD1 * invD0DotPerpD1;
                Real s1 = diffDotPerpD0 * invD0DotPerpD1;
                result.line0Parameter[0] = s0;
                result.line1Parameter[0] = s1;
                result.point = line0.origin + s0 * line0.direction;
            }
            else
            {
                // The lines are parallel.
                Normalize(diff);
                Real diffNDotPerpD1 = DotPerp(diff, line1.direction);
                if (std::fabs(diffNDotPerpD1) != (Real)0)
                {
                    // The lines are parallel but distinct.
                    result.intersect = false;
                    result.numIntersections = 0;
                }
                else
                {
                    // The lines are the same.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int>::max();
                    Real maxReal = std::numeric_limits<Real>::max();
                    result.line0Parameter[0] = -maxReal;
                    result.line0Parameter[1] = +maxReal;
                    result.line1Parameter[0] = -maxReal;
                    result.line1Parameter[1] = +maxReal;
                }
            }

            return result;
        }
    };
}
