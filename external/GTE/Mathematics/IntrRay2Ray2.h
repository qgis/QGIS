// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrLine2Line2.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray2<Real>, Ray2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (rays intersect in a
            // single point), 2 (rays are collinear and intersect in a 
            // segment; ray directions are opposite of each other), or
            // std::numeric_limits<int>::max() (intersection is a ray; ray
            // directions are the same).
            int numIntersections;
        };

        Result operator()(Ray2<Real> const& ray0, Ray2<Real> const& ray1)
        {
            Result result;
            FIQuery<Real, Line2<Real>, Line2<Real>> llQuery;
            Line2<Real> line0(ray0.origin, ray0.direction);
            Line2<Real> line1(ray1.origin, ray1.direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the rays.
                if (llResult.line0Parameter[0] >= (Real)0
                    && llResult.line1Parameter[0] >= (Real)0)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int>::max())
            {
                if (Dot(ray0.direction, ray1.direction) > (Real)0)
                {
                    // The rays are collinear and in the same direction, so
                    // they must overlap.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int>::max();
                }
                else
                {
                    // The rays are collinear but in opposite directions.
                    // Test whether they overlap.  Ray0 has interval
                    // [0,+infinity) and ray1 has interval (-infinity,t]
                    // relative to ray0.direction.
                    Vector2<Real> diff = ray1.origin - ray0.origin;
                    Real t = Dot(ray0.direction, diff);
                    if (t > (Real)0)
                    {
                        result.intersect = true;
                        result.numIntersections = 2;
                    }
                    else if (t < (Real)0)
                    {
                        result.intersect = false;
                        result.numIntersections = 0;
                    }
                    else  // t == 0
                    {
                        result.intersect = true;
                        result.numIntersections = 1;
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

    template <typename Real>
    class FIQuery<Real, Ray2<Real>, Ray2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (rays intersect in a
            // single point), 2 (rays are collinear and intersect in a 
            // segment; ray directions are opposite of each other), or
            // std::numeric_limits<int>::max() (intersection is a ray; ray
            // directions are the same).
            int numIntersections;

            // If numIntersections is 1, the intersection is
            //   point[0] = ray0.origin + ray0Parameter[0] * ray0.direction
            //            = ray1.origin + ray1Parameter[0] * ray1.direction
            // If numIntersections is 2, the segment of intersection is formed
            // by the ray origins,
            //   ray0Parameter[0] = ray1Parameter[0] = 0
            //   point[0] = ray0.origin
            //            = ray1.origin + ray1Parameter[1] * ray1.direction
            //   point[1] = ray1.origin
            //            = ray0.origin + ray0Parameter[1] * ray0.direction
            // where ray0Parameter[1] >= 0 and ray1Parameter[1] >= 0.
            // If numIntersections is maxInt, let
            //   ray1.origin = ray0.origin + t * ray0.direction
            // then
            //   ray0Parameter[] = { max(t,0), +maxReal }
            //   ray1Parameter[] = { -min(t,0), +maxReal }
            //   point[0] = ray0.origin + ray0Parameter[0] * ray0.direction
            Real ray0Parameter[2], ray1Parameter[2];
            Vector2<Real> point[2];
        };

        Result operator()(Ray2<Real> const& ray0, Ray2<Real> const& ray1)
        {
            Result result;
            FIQuery<Real, Line2<Real>, Line2<Real>> llQuery;
            Line2<Real> line0(ray0.origin, ray0.direction);
            Line2<Real> line1(ray1.origin, ray1.direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the rays.
                if (llResult.line0Parameter[0] >= (Real)0
                    && llResult.line1Parameter[0] >= (Real)0)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.ray0Parameter[0] = llResult.line0Parameter[0];
                    result.ray1Parameter[0] = llResult.line1Parameter[0];
                    result.point[0] = llResult.point;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int>::max())
            {
                // Compute t for which ray1.origin =
                // ray0.origin + t*ray0.direction.
                Real maxReal = std::numeric_limits<Real>::max();
                Vector2<Real> diff = ray1.origin - ray0.origin;
                Real t = Dot(ray0.direction, diff);
                if (Dot(ray0.direction, ray1.direction) > (Real)0)
                {
                    // The rays are collinear and in the same direction, so
                    // they must overlap.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int>::max();
                    if (t >= (Real)0)
                    {
                        result.ray0Parameter[0] = t;
                        result.ray0Parameter[1] = maxReal;
                        result.ray1Parameter[0] = (Real)0;
                        result.ray1Parameter[1] = maxReal;
                        result.point[0] = ray1.origin;
                    }
                    else
                    {
                        result.ray0Parameter[0] = (Real)0;
                        result.ray0Parameter[1] = maxReal;
                        result.ray1Parameter[0] = -t;
                        result.ray1Parameter[1] = maxReal;
                        result.point[0] = ray0.origin;
                    }
                }
                else
                {
                    // The rays are collinear but in opposite directions.
                    // Test whether they overlap.  Ray0 has interval
                    // [0,+infinity) and ray1 has interval (-infinity,t1]
                    // relative to ray0.direction.
                    if (t >= (Real)0)
                    {
                        result.intersect = true;
                        result.numIntersections = 2;
                        result.ray0Parameter[0] = (Real)0;
                        result.ray0Parameter[1] = t;
                        result.ray1Parameter[0] = (Real)0;
                        result.ray1Parameter[1] = t;
                        result.point[0] = ray0.origin;
                        result.point[1] = ray1.origin;
                    }
                    else
                    {
                        result.intersect = false;
                        result.numIntersections = 0;
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
