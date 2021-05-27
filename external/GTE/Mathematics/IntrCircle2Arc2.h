// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrCircle2Circle2.h>
#include <Mathematics/Arc2.h>

namespace gte
{
    template <typename Real>
    class FIQuery<Real, Circle2<Real>, Arc2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number of intersections is 0, 1, 2 or maxInt =
            // std::numeric_limits<int>::max().  When 1, the arc and circle
            // intersect in a single point.  When 2, the arc is not on the
            // circle and they intersect in two points.  When maxInt, the
            // arc is on the circle.
            int numIntersections;

            // Valid only when numIntersections = 1 or 2.
            Vector2<Real> point[2];

            // Valid only when numIntersections = maxInt.
            Arc2<Real> arc;
        };

        Result operator()(Circle2<Real> const& circle, Arc2<Real> const& arc)
        {
            Result result;

            Circle2<Real> circleOfArc(arc.center, arc.radius);
            FIQuery<Real, Circle2<Real>, Circle2<Real>> ccQuery;
            auto ccResult = ccQuery(circle, circleOfArc);
            if (!ccResult.intersect)
            {
                result.intersect = false;
                result.numIntersections = 0;
                return result;
            }

            if (ccResult.numIntersections == std::numeric_limits<int>::max())
            {
                // The arc is on the circle.
                result.intersect = true;
                result.numIntersections = std::numeric_limits<int>::max();
                result.arc = arc;
                return result;
            }

            // Test whether circle-circle intersection points are on the arc.
            for (int i = 0; i < ccResult.numIntersections; ++i)
            {
                result.numIntersections = 0;
                if (arc.Contains(ccResult.point[i]))
                {
                    result.point[result.numIntersections++] = ccResult.point[i];
                    result.intersect = true;
                }
            }
            return result;
        }
    };
}
