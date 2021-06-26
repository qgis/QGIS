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
    class TIQuery<Real, Line2<Real>, Ray2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (line and ray intersect
            // in a single point) or std::numeric_limits<int>::max() (line
            // and ray are collinear).
            int numIntersections;
        };

        Result operator()(Line2<Real> const& line, Ray2<Real> const& ray)
        {
            Result result;
            FIQuery<Real, Line2<Real>, Line2<Real>> llQuery;
            auto llResult = llQuery(line, Line2<Real>(ray.origin, ray.direction));
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the ray.
                if (llResult.line1Parameter[0] >= (Real)0)
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
            else
            {
                result.intersect = llResult.intersect;
                result.numIntersections = llResult.numIntersections;
            }
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line2<Real>, Ray2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (line and ray intersect
            // in a single point) or std::numeric_limits<int>::max() (line
            // and ray are collinear).
            int numIntersections;

            // If numIntersections is 1, the intersection is
            //   point = line.origin + lineParameter[0] * line.direction
            //         = ray.origin + rayParameter[0] * ray.direction
            // If numIntersections is maxInt, point is not valid but the
            // intervals are
            //   lineParameter[] = { -maxReal, +maxReal }
            //   rayParameter[] = { 0, +maxReal }
            Real lineParameter[2], rayParameter[2];
            Vector2<Real> point;
        };

        Result operator()(Line2<Real> const& line, Ray2<Real> const& ray)
        {
            Result result;
            FIQuery<Real, Line2<Real>, Line2<Real>> llQuery;
            auto llResult = llQuery(line, Line2<Real>(ray.origin, ray.direction));
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the ray.
                if (llResult.line1Parameter[0] >= (Real)0)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.lineParameter[0] = llResult.line0Parameter[0];
                    result.rayParameter[0] = llResult.line1Parameter[0];
                    result.point = llResult.point;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int>::max())
            {
                result.intersect = true;
                result.numIntersections = std::numeric_limits<int>::max();
                Real maxReal = std::numeric_limits<Real>::max();
                result.lineParameter[0] = -maxReal;
                result.lineParameter[1] = +maxReal;
                result.rayParameter[0] = (Real)0;
                result.rayParameter[1] = +maxReal;
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
