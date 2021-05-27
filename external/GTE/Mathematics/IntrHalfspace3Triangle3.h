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
#include <Mathematics/Triangle.h>

// Queries for intersection of objects with halfspaces.  These are useful for
// containment testing, object culling, and clipping.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Halfspace3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Halfspace3<Real> const& halfspace, Triangle3<Real> const& triangle)
        {
            Result result;

            // Project the triangle vertices onto the normal line.  The plane
            // of the halfspace occurs at the origin (zero) of the normal
            // line.
            Real s[3];
            for (int i = 0; i < 3; ++i)
            {
                s[i] = Dot(halfspace.normal, triangle.v[i]) - halfspace.constant;
            }

            // The triangle and halfspace intersect when the projection
            // interval maximum is nonnegative.
            result.intersect = (std::max(std::max(s[0], s[1]), s[2]) >= (Real)0);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Halfspace3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The triangle is clipped against the plane defining the
            // halfspace.  The 'numPoints' is either 0 (no intersection),
            // 1 (point), 2 (segment), 3 (triangle), or 4 (quadrilateral).
            int numPoints;
            Vector3<Real> point[4];
        };

        Result operator()(Halfspace3<Real> const& halfspace, Triangle3<Real> const& triangle)
        {
            Result result;

            // Determine on which side of the plane the vertices lie.  The
            // table of possibilities is listed next with n = numNegative,
            // p = numPositive, and z = numZero.
            //
            //   n p z  intersection
            //   ---------------------------------
            //   0 3 0  triangle (original)
            //   0 2 1  triangle (original)
            //   0 1 2  triangle (original)
            //   0 0 3  triangle (original)
            //   1 2 0  quad (2 edges clipped)
            //   1 1 1  triangle (1 edge clipped)
            //   1 0 2  edge
            //   2 1 0  triangle (2 edges clipped)
            //   2 0 1  vertex
            //   3 0 0  none

            Real s[3];
            int numPositive = 0, numNegative = 0, numZero = 0;
            for (int i = 0; i < 3; ++i)
            {
                s[i] = Dot(halfspace.normal, triangle.v[i]) - halfspace.constant;
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

            if (numNegative == 0)
            {
                // The triangle is in the halfspace.
                result.intersect = true;
                result.numPoints = 3;
                result.point[0] = triangle.v[0];
                result.point[1] = triangle.v[1];
                result.point[2] = triangle.v[2];
            }
            else if (numNegative == 1)
            {
                result.intersect = true;
                if (numPositive == 2)
                {
                    // The portion of the triangle in the halfspace is a
                    // quadrilateral.
                    result.numPoints = 4;
                    for (int i0 = 0; i0 < 3; ++i0)
                    {
                        if (s[i0] < (Real)0)
                        {
                            int i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                            result.point[0] = triangle.v[i1];
                            result.point[1] = triangle.v[i2];
                            Real t2 = s[i2] / (s[i2] - s[i0]);
                            Real t0 = s[i0] / (s[i0] - s[i1]);
                            result.point[2] = triangle.v[i2] + t2 *
                                (triangle.v[i0] - triangle.v[i2]);
                            result.point[3] = triangle.v[i0] + t0 *
                                (triangle.v[i1] - triangle.v[i0]);
                            break;
                        }
                    }
                }
                else if (numPositive == 1)
                {
                    // The portion of the triangle in the halfspace is a
                    // triangle with one vertex on the plane.
                    result.numPoints = 3;
                    for (int i0 = 0; i0 < 3; ++i0)
                    {
                        if (s[i0] == (Real)0)
                        {
                            int i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                            result.point[0] = triangle.v[i0];
                            Real t1 = s[i1] / (s[i1] - s[i2]);
                            Vector3<Real> p = triangle.v[i1] + t1 *
                                (triangle.v[i2] - triangle.v[i1]);
                            if (s[i1] > (Real)0)
                            {
                                result.point[1] = triangle.v[i1];
                                result.point[2] = p;
                            }
                            else
                            {
                                result.point[1] = p;
                                result.point[2] = triangle.v[i2];
                            }
                            break;
                        }
                    }
                }
                else
                {
                    // Only an edge of the triangle is in the halfspace.
                    result.numPoints = 0;
                    for (int i = 0; i < 3; ++i)
                    {
                        if (s[i] == (Real)0)
                        {
                            result.point[result.numPoints++] = triangle.v[i];
                        }
                    }
                }
            }
            else if (numNegative == 2)
            {
                result.intersect = true;
                if (numPositive == 1)
                {
                    // The portion of the triangle in the halfspace is a
                    // triangle.
                    result.numPoints = 3;
                    for (int i0 = 0; i0 < 3; ++i0)
                    {
                        if (s[i0] > (Real)0)
                        {
                            int i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                            result.point[0] = triangle.v[i0];
                            Real t0 = s[i0] / (s[i0] - s[i1]);
                            Real t2 = s[i2] / (s[i2] - s[i0]);
                            result.point[1] = triangle.v[i0] + t0 *
                                (triangle.v[i1] - triangle.v[i0]);
                            result.point[2] = triangle.v[i2] + t2 *
                                (triangle.v[i0] - triangle.v[i2]);
                            break;
                        }
                    }
                }
                else
                {
                    // Only a vertex of the triangle is in the halfspace.
                    result.numPoints = 1;
                    for (int i = 0; i < 3; ++i)
                    {
                        if (s[i] == (Real)0)
                        {
                            result.point[0] = triangle.v[i];
                            break;
                        }
                    }
                }
            }
            else  // numNegative == 3
            {
                // The triangle is outside the halfspace. (numNegative == 3)
                result.intersect = false;
                result.numPoints = 0;
            }

            return result;
        }
    };
}
