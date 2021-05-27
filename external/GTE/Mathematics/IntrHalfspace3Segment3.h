// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/Halfspace.h>
#include <Mathematics/Segment.h>

// Queries for intersection of objects with halfspaces.  These are useful for
// containment testing, object culling, and clipping.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Halfspace3<Real>, Segment3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Halfspace3<Real> const& halfspace, Segment3<Real> const& segment)
        {
            Result result;

            // Project the segment endpoints onto the normal line.  The plane
            // of the halfspace occurs at the origin (zero) of the normal
            // line.
            Real s[2];
            for (int i = 0; i < 2; ++i)
            {
                s[i] = Dot(halfspace.normal, segment.p[i]) - halfspace.constant;
            }

            // The segment and halfspace intersect when the projection
            // interval maximum is nonnegative.
            result.intersect = (std::max(s[0], s[1]) >= (Real)0);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Halfspace3<Real>, Segment3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The segment is clipped against the plane defining the 
            // halfspace.  The 'numPoints' is either 0 (no intersection),
            // 1 (point), or 2 (segment).
            int numPoints;
            Vector3<Real> point[2];
        };

        Result operator()(Halfspace3<Real> const& halfspace, Segment3<Real> const& segment)
        {
            // Determine on which side of the plane the endpoints lie.  The
            // table of possibilities is listed next with n = numNegative,
            // p = numPositive, and z = numZero.
            //
            //   n p z  intersection
            //   -------------------------
            //   0 2 0  segment (original)
            //   0 1 1  segment (original)
            //   0 0 2  segment (original)
            //   1 1 0  segment (clipped)
            //   1 0 1  point (endpoint)
            //   2 0 0  none

            Real s[2];
            int numPositive = 0, numNegative = 0, numZero = 0;
            for (int i = 0; i < 2; ++i)
            {
                s[i] = Dot(halfspace.normal, segment.p[i]) - halfspace.constant;
                if (s[i] > (Real)0)
                {
                    ++numPositive;
                }
                else if (s[i] < (Real)0)
                {
                    ++numNegative;
                }
                else
                {
                    ++numZero;
                }
            }

            Result result;

            if (numNegative == 0)
            {
                // The segment is in the halfspace.
                result.intersect = true;
                result.numPoints = 2;
                result.point[0] = segment.p[0];
                result.point[1] = segment.p[1];
            }
            else if (numNegative == 1)
            {
                result.intersect = true;
                result.numPoints = 1;
                if (numPositive == 1)
                {
                    // The segment is intersected at an interior point.
                    result.point[0] = segment.p[0] +
                        (s[0] / (s[0] - s[1])) * (segment.p[1] - segment.p[0]);
                }
                else  // numZero = 1
                {
                    // One segment endpoint is on the plane.
                    if (s[0] == (Real)0)
                    {
                        result.point[0] = segment.p[0];
                    }
                    else
                    {
                        result.point[0] = segment.p[1];
                    }
                }
            }
            else  // numNegative == 2
            {
                // The segment is outside the halfspace. (numNegative == 2)
                result.intersect = false;
                result.numPoints = 0;
            }

            return result;
        }
    };
}
