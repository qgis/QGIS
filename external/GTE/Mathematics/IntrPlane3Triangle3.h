// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Plane3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (plane and triangle
            // intersect at a single point [vertex]), 2 (plane and triangle
            // intersect in a segment), or 3 (triangle is in the plane).
            // When the number is 2, the segment is either interior to the
            // triangle or is an edge of the triangle, the distinction stored
            // in 'isInterior'.
            int numIntersections;
            bool isInterior;
        };

        Result operator()(Plane3<Real> const& plane, Triangle3<Real> const& triangle)
        {
            Result result;

            // Determine on which side of the plane the vertices lie.  The
            // table of possibilities is listed next with n = numNegative,
            // p = numPositive, and z = numZero.
            //
            //   n p z  intersection
            //   ------------------------------------
            //   0 3 0  none
            //   0 2 1  vertex
            //   0 1 2  edge
            //   0 0 3  triangle in the plane
            //   1 2 0  segment (2 edges clipped)
            //   1 1 1  segment (1 edge clipped)
            //   1 0 2  edge
            //   2 1 0  segment (2 edges clipped)
            //   2 0 1  vertex
            //   3 0 0  none

            Real s[3];
            int numPositive = 0, numNegative = 0, numZero = 0;
            for (int i = 0; i < 3; ++i)
            {
                s[i] = Dot(plane.normal, triangle.v[i]) - plane.constant;
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

            if (numZero == 0 && numPositive > 0 && numNegative > 0)
            {
                result.intersect = true;
                result.numIntersections = 2;
                result.isInterior = true;
                return result;
            }

            if (numZero == 1)
            {
                result.intersect = true;
                for (int i = 0; i < 3; ++i)
                {
                    if (s[i] == (Real)0)
                    {
                        if (numPositive == 2 || numNegative == 2)
                        {
                            result.numIntersections = 1;
                        }
                        else
                        {
                            result.numIntersections = 2;
                            result.isInterior = true;
                        }
                        break;
                    }
                }
                return result;
            }

            if (numZero == 2)
            {
                result.intersect = true;
                result.numIntersections = 2;
                result.isInterior = false;
                return result;
            }

            if (numZero == 3)
            {
                result.intersect = true;
                result.numIntersections = 3;
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
    class FIQuery<Real, Plane3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number is 0 (no intersection), 1 (plane and triangle
            // intersect at a single point [vertex]), 2 (plane and triangle
            // intersect in a segment), or 3 (triangle is in the plane).
            // When the number is 2, the segment is either interior to the
            // triangle or is an edge of the triangle, the distinction stored
            // in 'isInterior'.
            int numIntersections;
            bool isInterior;
            Vector3<Real> point[3];
        };

        Result operator()(Plane3<Real> const& plane, Triangle3<Real> const& triangle)
        {
            Result result;

            // Determine on which side of the plane the vertices lie.  The
            // table of possibilities is listed next with n = numNegative,
            // p = numPositive, and z = numZero.
            //
            //   n p z  intersection
            //   ------------------------------------
            //   0 3 0  none
            //   0 2 1  vertex
            //   0 1 2  edge
            //   0 0 3  triangle in the plane
            //   1 2 0  segment (2 edges clipped)
            //   1 1 1  segment (1 edge clipped)
            //   1 0 2  edge
            //   2 1 0  segment (2 edges clipped)
            //   2 0 1  vertex
            //   3 0 0  none

            Real s[3];
            int numPositive = 0, numNegative = 0, numZero = 0;
            for (int i = 0; i < 3; ++i)
            {
                s[i] = Dot(plane.normal, triangle.v[i]) - plane.constant;
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

            if (numZero == 0 && numPositive > 0 && numNegative > 0)
            {
                result.intersect = true;
                result.numIntersections = 2;
                result.isInterior = true;
                Real sign = (Real)3 - numPositive * (Real)2;
                for (int i0 = 0; i0 < 3; ++i0)
                {
                    if (sign * s[i0] > (Real)0)
                    {
                        int i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                        Real t1 = s[i1] / (s[i1] - s[i0]);
                        Real t2 = s[i2] / (s[i2] - s[i0]);
                        result.point[0] = triangle.v[i1] + t1 *
                            (triangle.v[i0] - triangle.v[i1]);
                        result.point[1] = triangle.v[i2] + t2 *
                            (triangle.v[i0] - triangle.v[i2]);
                        break;
                    }
                }
                return result;
            }

            if (numZero == 1)
            {
                result.intersect = true;
                for (int i0 = 0; i0 < 3; ++i0)
                {
                    if (s[i0] == (Real)0)
                    {
                        int i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                        result.point[0] = triangle.v[i0];
                        if (numPositive == 2 || numNegative == 2)
                        {
                            result.numIntersections = 1;
                        }
                        else
                        {
                            result.numIntersections = 2;
                            result.isInterior = true;
                            Real t = s[i1] / (s[i1] - s[i2]);
                            result.point[1] = triangle.v[i1] + t *
                                (triangle.v[i2] - triangle.v[i1]);
                        }
                        break;
                    }
                }
                return result;
            }

            if (numZero == 2)
            {
                result.intersect = true;
                result.numIntersections = 2;
                result.isInterior = false;
                for (int i0 = 0; i0 < 3; ++i0)
                {
                    if (s[i0] != (Real)0)
                    {
                        int i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                        result.point[0] = triangle.v[i1];
                        result.point[1] = triangle.v[i2];
                        break;
                    }
                }
                return result;
            }

            if (numZero == 3)
            {
                result.intersect = true;
                result.numIntersections = 3;
                result.point[0] = triangle.v[0];
                result.point[1] = triangle.v[1];
                result.point[2] = triangle.v[2];
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
