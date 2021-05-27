// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrLine2Line2.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line2<Real>, Segment2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (line and segment
            // intersect in a single point) or std::numeric_limits<int>::max()
            // (line and segment are collinear).
            int numIntersections;
        };

        Result operator()(Line2<Real> const& line, Segment2<Real> const& segment)
        {
            Result result;
            Vector2<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            FIQuery<Real, Line2<Real>, Line2<Real>> llQuery;
            auto llResult = llQuery(line, Line2<Real>(segOrigin, segDirection));
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the segment.
                if (std::fabs(llResult.line1Parameter[0]) <= segExtent)
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
    class FIQuery<Real, Line2<Real>, Segment2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (line and segment
            // intersect in a single point) or std::numeric_limits<int>::max()
            // (line and segment are collinear).
            int numIntersections;

            // If numIntersections is 1, the intersection is
            //   point = line.origin + lineParameter[0] * line.direction
            //         = segment.origin +
            //           segmentParameter[0] * segment.direction
            // If numIntersections is maxInt, point is not valid but the
            // intervals are
            //   lineParameter[] = { -maxReal, +maxReal }
            //   segmentParameter[] = { -segmentExtent, segmentExtent }
            Real lineParameter[2], segmentParameter[2];
            Vector2<Real> point;
        };

        Result operator()(Line2<Real> const& line, Segment2<Real> const& segment)
        {
            Result result;
            Vector2<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            FIQuery<Real, Line2<Real>, Line2<Real>> llQuery;
            auto llResult = llQuery(line, Line2<Real>(segOrigin, segDirection));
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the ray.
                if (std::fabs(llResult.line1Parameter[0]) <= segExtent)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.lineParameter[0] = llResult.line0Parameter[0];
                    result.segmentParameter[0] = llResult.line1Parameter[0];
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
                result.segmentParameter[0] = -segExtent;
                result.segmentParameter[1] = +segExtent;
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
