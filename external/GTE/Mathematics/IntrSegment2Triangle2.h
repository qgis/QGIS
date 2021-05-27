// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.05.06

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Triangle2.h>
#include <Mathematics/Segment.h>

// The queries consider the triangle to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Segment2<Real>, Triangle2<Real>>
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

        Result operator()(Segment2<Real> const& segment, Triangle2<Real> const& triangle)
        {
            Result result{};
            FIQuery<Real, Segment2<Real>, Triangle2<Real>> stQuery;
            result.intersect = stQuery(segment, triangle).intersect;
            return result;
        }
    };

    template <typename Real>
    class FIQuery <Real, Segment2<Real>, Triangle2<Real>>
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

        Result operator()(Segment2<Real> const& segment, Triangle2<Real> const& triangle)
        {
            Vector2<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, triangle, result);
            if (result.numIntersections == 2)
            {
                result.point[0] = segOrigin + result.parameter[0] * segDirection;
                result.point[1] = segOrigin + result.parameter[1] * segDirection;
            }
            else if (result.numIntersections == 1)
            {
                result.point[0] = segOrigin + result.parameter[0] * segDirection;
                result.point[1] = result.point[0];
            }
            // else: result set to no-intersection in DoQuery(...)
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& segOrigin,
            Vector2<Real> const& segDirection, Real segExtent,
            Triangle2<Real> const& triangle, Result& result)
        {
            FIQuery<Real, Line2<Real>, Triangle2<Real>>::DoQuery(segOrigin,
                segDirection, triangle, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the triangle;
                // the t-interval is [t0,t1]. The segment intersects the
                // triangle as long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                std::array<Real, 2> segInterval{ -segExtent, segExtent };
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, segInterval);
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
