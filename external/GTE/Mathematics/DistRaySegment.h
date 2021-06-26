// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Ray<N, Real>, Segment<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real parameter[2];
            Vector<N, Real> closestPoint[2];
        };

        // The centered form of the 'segment' is used.  Thus, parameter[1] of
        // the result is in [-e,e], where e = |segment.p[1] - segment.p[0]|/2.
        Result operator()(Ray<N, Real> const& ray, Segment<N, Real> const& segment)
        {
            Result result;

            Vector<N, Real> segCenter, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segCenter, segDirection, segExtent);

            Vector<N, Real> diff = ray.origin - segCenter;
            Real a01 = -Dot(ray.direction, segDirection);
            Real b0 = Dot(diff, ray.direction);
            Real s0, s1;

            if (std::fabs(a01) < (Real)1)
            {
                // The ray and segment are not parallel.
                Real det = (Real)1 - a01 * a01;
                Real extDet = segExtent * det;
                Real b1 = -Dot(diff, segDirection);
                s0 = a01 * b1 - b0;
                s1 = a01 * b0 - b1;

                if (s0 >= (Real)0)
                {
                    if (s1 >= -extDet)
                    {
                        if (s1 <= extDet)  // region 0
                        {
                            // Minimum at interior points of ray and segment.
                            s0 /= det;
                            s1 /= det;
                        }
                        else  // region 1
                        {
                            s1 = segExtent;
                            s0 = std::max(-(a01 * s1 + b0), (Real)0);
                        }
                    }
                    else  // region 5
                    {
                        s1 = -segExtent;
                        s0 = std::max(-(a01 * s1 + b0), (Real)0);
                    }
                }
                else
                {
                    if (s1 <= -extDet)  // region 4
                    {
                        s0 = -(-a01 * segExtent + b0);
                        if (s0 > (Real)0)
                        {
                            s1 = -segExtent;
                        }
                        else
                        {
                            s0 = (Real)0;
                            s1 = -b1;
                            if (s1 < -segExtent)
                            {
                                s1 = -segExtent;
                            }
                            else if (s1 > segExtent)
                            {
                                s1 = segExtent;
                            }
                        }
                    }
                    else if (s1 <= extDet)  // region 3
                    {
                        s0 = (Real)0;
                        s1 = -b1;
                        if (s1 < -segExtent)
                        {
                            s1 = -segExtent;
                        }
                        else if (s1 > segExtent)
                        {
                            s1 = segExtent;
                        }
                    }
                    else  // region 2
                    {
                        s0 = -(a01 * segExtent + b0);
                        if (s0 > (Real)0)
                        {
                            s1 = segExtent;
                        }
                        else
                        {
                            s0 = (Real)0;
                            s1 = -b1;
                            if (s1 < -segExtent)
                            {
                                s1 = -segExtent;
                            }
                            else if (s1 > segExtent)
                            {
                                s1 = segExtent;
                            }
                        }
                    }
                }
            }
            else
            {
                // Ray and segment are parallel.
                if (a01 > (Real)0)
                {
                    // Opposite direction vectors.
                    s1 = -segExtent;
                }
                else
                {
                    // Same direction vectors.
                    s1 = segExtent;
                }

                s0 = std::max(-(a01 * s1 + b0), (Real)0);
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closestPoint[0] = ray.origin + s0 * ray.direction;
            result.closestPoint[1] = segCenter + s1 * segDirection;
            diff = result.closestPoint[0] - result.closestPoint[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPRaySegment = DCPQuery<Real, Ray<N, Real>, Segment<N, Real>>;

    template <typename Real>
    using DCPRay2Segment2 = DCPRaySegment<2, Real>;

    template <typename Real>
    using DCPRay3Segment3 = DCPRaySegment<3, Real>;
}
