// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Line<N, Real>, Line<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real parameter[2];
            Vector<N, Real> closestPoint[2];
        };

        Result operator()(Line<N, Real> const& line0, Line<N, Real> const& line1)
        {
            Result result;

            Vector<N, Real> diff = line0.origin - line1.origin;
            Real a01 = -Dot(line0.direction, line1.direction);
            Real b0 = Dot(diff, line0.direction);
            Real s0, s1;

            if (std::fabs(a01) < (Real)1)
            {
                // Lines are not parallel.
                Real det = (Real)1 - a01 * a01;
                Real b1 = -Dot(diff, line1.direction);
                s0 = (a01 * b1 - b0) / det;
                s1 = (a01 * b0 - b1) / det;
            }
            else
            {
                // Lines are parallel, select any pair of closest points.
                s0 = -b0;
                s1 = (Real)0;
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closestPoint[0] = line0.origin + s0 * line0.direction;
            result.closestPoint[1] = line1.origin + s1 * line1.direction;
            diff = result.closestPoint[0] - result.closestPoint[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPLineLine = DCPQuery<Real, Line<N, Real>, Line<N, Real>>;

    template <typename Real>
    using DCPLine2Line2 = DCPLineLine<2, Real>;

    template <typename Real>
    using DCPLine3Line3 = DCPLineLine<3, Real>;
}
