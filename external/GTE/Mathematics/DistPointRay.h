// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Vector<N, Real>, Ray<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real rayParameter;  // t in [0,+infinity)
            Vector<N, Real> rayClosest;  // origin + t * direction
        };

        Result operator()(Vector<N, Real> const& point, Ray<N, Real> const& ray)
        {
            Result result;

            Vector<N, Real> diff = point - ray.origin;
            result.rayParameter = Dot(ray.direction, diff);
            if (result.rayParameter > (Real)0)
            {
                result.rayClosest = ray.origin + result.rayParameter * ray.direction;
            }
            else
            {
                result.rayClosest = ray.origin;
            }

            diff = point - result.rayClosest;
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPPointRay = DCPQuery<Real, Vector<N, Real>, Ray<N, Real>>;

    template <typename Real>
    using DCPPoint2Ray2 = DCPPointRay<2, Real>;

    template <typename Real>
    using DCPPoint3Ray3 = DCPPointRay<3, Real>;
}
