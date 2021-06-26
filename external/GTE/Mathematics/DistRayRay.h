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
    class DCPQuery<Real, Ray<N, Real>, Ray<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real parameter[2];
            Vector<N, Real> closestPoint[2];
        };

        Result operator()(Ray<N, Real> const& ray0, Ray<N, Real> const& ray1)
        {
            Result result;

            Vector<N, Real> diff = ray0.origin - ray1.origin;
            Real a01 = -Dot(ray0.direction, ray1.direction);
            Real b0 = Dot(diff, ray0.direction), b1;
            Real s0, s1;

            if (std::fabs(a01) < (Real)1)
            {
                // Rays are not parallel.
                b1 = -Dot(diff, ray1.direction);
                s0 = a01 * b1 - b0;
                s1 = a01 * b0 - b1;

                if (s0 >= (Real)0)
                {
                    if (s1 >= (Real)0)  // region 0 (interior)
                    {
                        // Minimum at two interior points of rays.
                        Real det = (Real)1 - a01 * a01;
                        s0 /= det;
                        s1 /= det;
                    }
                    else  // region 3 (side)
                    {
                        s1 = (Real)0;
                        if (b0 >= (Real)0)
                        {
                            s0 = (Real)0;
                        }
                        else
                        {
                            s0 = -b0;
                        }
                    }
                }
                else
                {
                    if (s1 >= (Real)0)  // region 1 (side)
                    {
                        s0 = (Real)0;
                        if (b1 >= (Real)0)
                        {
                            s1 = (Real)0;
                        }
                        else
                        {
                            s1 = -b1;
                        }
                    }
                    else  // region 2 (corner)
                    {
                        if (b0 < (Real)0)
                        {
                            s0 = -b0;
                            s1 = (Real)0;
                        }
                        else
                        {
                            s0 = (Real)0;
                            if (b1 >= (Real)0)
                            {
                                s1 = (Real)0;
                            }
                            else
                            {
                                s1 = -b1;
                            }
                        }
                    }
                }
            }
            else
            {
                // Rays are parallel.
                if (a01 > (Real)0)
                {
                    // Opposite direction vectors.
                    s1 = (Real)0;
                    if (b0 >= (Real)0)
                    {
                        s0 = (Real)0;
                    }
                    else
                    {
                        s0 = -b0;
                    }
                }
                else
                {
                    // Same direction vectors.
                    if (b0 >= (Real)0)
                    {
                        b1 = -Dot(diff, ray1.direction);
                        s0 = (Real)0;
                        s1 = -b1;
                    }
                    else
                    {
                        s0 = -b0;
                        s1 = (Real)0;
                    }
                }
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closestPoint[0] = ray0.origin + s0 * ray0.direction;
            result.closestPoint[1] = ray1.origin + s1 * ray1.direction;
            diff = result.closestPoint[0] - result.closestPoint[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPRayRay = DCPQuery<Real, Ray<N, Real>, Ray<N, Real>>;

    template <typename Real>
    using DCPRay2Ray2 = DCPRayRay<2, Real>;

    template <typename Real>
    using DCPRay3Ray3 = DCPRayRay<3, Real>;
}
