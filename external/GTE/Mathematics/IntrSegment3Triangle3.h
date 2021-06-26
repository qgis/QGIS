// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Segment.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Segment3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Segment3<Real> const& segment, Triangle3<Real> const& triangle)
        {
            Result result;

            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            // Compute the offset origin, edges, and normal.
            Vector3<Real> diff = segOrigin - triangle.v[0];
            Vector3<Real> edge1 = triangle.v[1] - triangle.v[0];
            Vector3<Real> edge2 = triangle.v[2] - triangle.v[0];
            Vector3<Real> normal = Cross(edge1, edge2);

            // Solve Q + t*D = b1*E1 + b2*E2 (Q = diff, D = segment direction,
            // E1 = edge1, E2 = edge2, N = Cross(E1,E2)) by
            //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
            //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
            //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
            Real DdN = Dot(segDirection, normal);
            Real sign;
            if (DdN > (Real)0)
            {
                sign = (Real)1;
            }
            else if (DdN < (Real)0)
            {
                sign = (Real)-1;
                DdN = -DdN;
            }
            else
            {
                // Segment and triangle are parallel, call it a "no
                // intersection" even if the segment does intersect.
                result.intersect = false;
                return result;
            }

            Real DdQxE2 = sign * DotCross(segDirection, diff, edge2);
            if (DdQxE2 >= (Real)0)
            {
                Real DdE1xQ = sign * DotCross(segDirection, edge1, diff);
                if (DdE1xQ >= (Real)0)
                {
                    if (DdQxE2 + DdE1xQ <= DdN)
                    {
                        // Line intersects triangle, check whether segment
                        // does.
                        Real QdN = -sign * Dot(diff, normal);
                        Real extDdN = segExtent * DdN;
                        if (-extDdN <= QdN && QdN <= extDdN)
                        {
                            // Segment intersects triangle.
                            result.intersect = true;
                            return result;
                        }
                        // else: |t| > extent, no intersection
                    }
                    // else: b1+b2 > 1, no intersection
                }
                // else: b2 < 0, no intersection
            }
            // else: b1 < 0, no intersection

            result.intersect = false;
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Segment3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                parameter((Real)0),
                triangleBary{ (Real)0, (Real)0, (Real)0 },
                point{ (Real)0, (Real)0, (Real)0 }
            {
            }

            bool intersect;
            Real parameter;
            std::array<Real, 3> triangleBary;
            Vector3<Real> point;
        };

        Result operator()(Segment3<Real> const& segment, Triangle3<Real> const& triangle)
        {
            Result result;

            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            // Compute the offset origin, edges, and normal.
            Vector3<Real> diff = segOrigin - triangle.v[0];
            Vector3<Real> edge1 = triangle.v[1] - triangle.v[0];
            Vector3<Real> edge2 = triangle.v[2] - triangle.v[0];
            Vector3<Real> normal = Cross(edge1, edge2);

            // Solve Q + t*D = b1*E1 + b2*E2 (Q = diff, D = segment direction,
            // E1 = edge1, E2 = edge2, N = Cross(E1,E2)) by
            //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
            //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
            //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
            Real DdN = Dot(segDirection, normal);
            Real sign;
            if (DdN > (Real)0)
            {
                sign = (Real)1;
            }
            else if (DdN < (Real)0)
            {
                sign = (Real)-1;
                DdN = -DdN;
            }
            else
            {
                // Segment and triangle are parallel, call it a "no
                // intersection" even if the segment does intersect.
                result.intersect = false;
                return result;
            }

            Real DdQxE2 = sign * DotCross(segDirection, diff, edge2);
            if (DdQxE2 >= (Real)0)
            {
                Real DdE1xQ = sign * DotCross(segDirection, edge1, diff);
                if (DdE1xQ >= (Real)0)
                {
                    if (DdQxE2 + DdE1xQ <= DdN)
                    {
                        // Line intersects triangle, check whether segment
                        // does.
                        Real QdN = -sign * Dot(diff, normal);
                        Real extDdN = segExtent * DdN;
                        if (-extDdN <= QdN && QdN <= extDdN)
                        {
                            // Segment intersects triangle.
                            result.intersect = true;
                            Real inv = ((Real)1) / DdN;
                            result.parameter = QdN * inv;
                            result.triangleBary[1] = DdQxE2 * inv;
                            result.triangleBary[2] = DdE1xQ * inv;
                            result.triangleBary[0] =
                                (Real)1 - result.triangleBary[1] - result.triangleBary[2];
                            result.point = segOrigin + result.parameter * segDirection;
                            return result;
                        }
                        // else: |t| > extent, no intersection
                    }
                    // else: b1+b2 > 1, no intersection
                }
                // else: b2 < 0, no intersection
            }
            // else: b1 < 0, no intersection

            result.intersect = false;
            return result;
        }
    };
}
