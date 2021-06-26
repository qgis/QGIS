// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.05.06

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Triangle2.h>
#include <Mathematics/Ray.h>

// The queries consider the triangle to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Ray2<Real> const& ray, Triangle2<Real> const& triangle)
        {
            Result result{};
            FIQuery<Real, Ray2<Real>, Triangle2<Real>> rtQuery;
            result.intersect = rtQuery(ray, triangle).intersect;
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray2<Real>, Triangle2<Real>>
        :
        public FIQuery<Real, Line2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line2<Real>, Triangle2<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray2<Real> const& ray, Triangle2<Real> const& triangle)
        {
            Result result{};
            DoQuery(ray.origin, ray.direction, triangle, result);
            if (result.numIntersections == 2)
            {
                result.point[0] = ray.origin + result.parameter[0] * ray.direction;
                result.point[1] = ray.origin + result.parameter[1] * ray.direction;
            }
            else if (result.numIntersections == 1)
            {
                result.point[0] = ray.origin + result.parameter[0] * ray.direction;
                result.point[1] = result.point[0];
            }
            // else: result set to no-intersection in DoQuery(...)
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& rayOrigin,
            Vector2<Real> const& rayDirection, Triangle2<Real> const& triangle,
            Result& result)
        {
            FIQuery<Real, Line2<Real>, Triangle2<Real>>::DoQuery(rayOrigin,
                rayDirection, triangle, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the triangle; the
                // t-interval is [t0,t1]. The ray intersects the triangle as
                // long as [t0,t1] overlaps the ray t-interval [0,+infinity).
                std::array<Real, 2> rayInterval{ static_cast<Real>(0), std::numeric_limits<Real>::max() };
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, rayInterval);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // Set the state to no-intersection.
                    result = Result{};
                }
            }
        }
    };
}
