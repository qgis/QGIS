// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Line<N, Real>, Ray<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real parameter[2];
            Vector<N, Real> closestPoint[2];
        };

        Result operator()(Line<N, Real> const& line, Ray<N, Real> const& ray)
        {
            Result result;

            Vector<N, Real> diff = line.origin - ray.origin;
            Real a01 = -Dot(line.direction, ray.direction);
            Real b0 = Dot(diff, line.direction);
            Real s0, s1;

            if (std::fabs(a01) < (Real)1)
            {
                Real b1 = -Dot(diff, ray.direction);
                s1 = a01 * b0 - b1;

                if (s1 >= (Real)0)
                {
                    // Two interior points are closest, one on line and one
                    // on ray.
                    Real det = (Real)1 - a01 * a01;
                    s0 = (a01 * b1 - b0) / det;
                    s1 /= det;
                }
                else
                {
                    // Origin of ray and interior point of line are closest.
                    s0 = -b0;
                    s1 = (Real)0;
                }
            }
            else
            {
                // Lines are parallel, closest pair with one point at ray
                // origin.
                s0 = -b0;
                s1 = (Real)0;
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closestPoint[0] = line.origin + s0 * line.direction;
            result.closestPoint[1] = ray.origin + s1 * ray.direction;
            diff = result.closestPoint[0] - result.closestPoint[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPLineRay = DCPQuery<Real, Line<N, Real>, Ray<N, Real>>;

    template <typename Real>
    using DCPLine2Ray2 = DCPLineRay<2, Real>;

    template <typename Real>
    using DCPLine3Ray3 = DCPLineRay<3, Real>;
}
