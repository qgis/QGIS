// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrRay2Circle2.h>
#include <Mathematics/Arc2.h>

// The queries consider the arc to be a 1-dimensional object.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray2<Real>, Arc2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Ray2<Real> const& ray, Arc2<Real> const& arc)
        {
            Result result;
            FIQuery<Real, Ray2<Real>, Arc2<Real>> raQuery;
            auto raResult = raQuery(ray, arc);
            result.intersect = raResult.intersect;
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray2<Real>, Arc2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector2<Real>, 2> point;
        };

        Result operator()(Ray2<Real> const& ray, Arc2<Real> const& arc)
        {
            Result result;

            FIQuery<Real, Ray2<Real>, Circle2<Real>> rcQuery;
            Circle2<Real> circle(arc.center, arc.radius);
            auto rcResult = rcQuery(ray, circle);
            if (rcResult.intersect)
            {
                // Test whether ray-circle intersections are on the arc.
                result.numIntersections = 0;
                for (int i = 0; i < rcResult.numIntersections; ++i)
                {
                    if (arc.Contains(rcResult.point[i]))
                    {
                        result.intersect = true;
                        result.parameter[result.numIntersections]
                            = rcResult.parameter[i];
                        result.point[result.numIntersections++]
                            = rcResult.point[i];
                    }
                }
            }
            else
            {
                result.intersect = false;
                result.numIntersections = 0;
            }

            return result;
        }
    };
}
