// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.05.06

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector2.h>

// The queries consider the triangle to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Line2<Real> const& line, Triangle2<Real> const& triangle)
        {
            Result result{};

            // Determine on which side of the line the vertices lie. The
            // table of possibilities is listed next with n = numNegative,
            // p = numPositive and z = numZero.
            //
            //   n p z  intersection
            //   ------------------------------------
            //   0 3 0  none
            //   0 2 1  vertex
            //   0 1 2  edge
            //   0 0 3  none (degenerate triangle)
            //   1 2 0  segment (2 edges clipped)
            //   1 1 1  segment (1 edge clipped)
            //   1 0 2  edge
            //   2 1 0  segment (2 edges clipped)
            //   2 0 1  vertex
            //   3 0 0  none
            // The case (n,p,z) = (0,0,3) is treated as a no-intersection
            // because the triangle is degenerate.

            // The s-array is not necessary for the algorithm because a local
            // variable in the loop can store DotPerp. However, the s-array is
            // useful for the unit-testing framework.
            Real const zero = static_cast<Real>(0);
            int32_t numPositive = 0, numNegative = 0, numZero = 0;
            for (size_t i = 0; i < 3; ++i)
            {
                Vector2<Real> diff = triangle.v[i] - line.origin;
                Real s = DotPerp(line.direction, diff);
                if (s > zero)
                {
                    ++numPositive;
                }
                else if (s < zero)
                {
                    ++numNegative;
                }
                else
                {
                    ++numZero;
                }
            }

            result.intersect =
                (numZero == 0 && numPositive > 0 && numNegative > 0) ||
                (numZero == 1) ||
                (numZero == 2);

            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ static_cast<Real>(0), static_cast<Real>(0) },
                point{
                    Vector2<Real>{ static_cast<Real>(0), static_cast<Real>(0) },
                    Vector2<Real>{ static_cast<Real>(0), static_cast<Real>(0) }}
            {
            }

            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector2<Real>, 2> point;
        };

        Result operator()(Line2<Real> const& line, Triangle2<Real> const& triangle)
        {
            Result result{};
            DoQuery(line.origin, line.direction, triangle, result);
            if (result.numIntersections == 2)
            {
                result.point[0] = line.origin + result.parameter[0] * line.direction;
                result.point[1] = line.origin + result.parameter[1] * line.direction;
            }
            else if (result.numIntersections == 1)
            {
                result.point[0] = line.origin + result.parameter[0] * line.direction;
                result.point[1] = result.point[0];
            }
            // else: result set to no-intersection in its constructor
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& lineOrigin,
            Vector2<Real> const& lineDirection, Triangle2<Real> const& triangle,
            Result& result)
        {
            // Determine on which side of the line the vertices lie. The
            // table of possibilities is listed next with n = numNegative,
            // p = numPositive and z = numZero.
            //
            //   n p z  intersection
            //   ------------------------------------
            //   0 3 0  none
            //   0 2 1  vertex
            //   0 1 2  edge
            //   0 0 3  none (degenerate triangle)
            //   1 2 0  segment (2 edges clipped)
            //   1 1 1  segment (1 edge clipped)
            //   1 0 2  edge
            //   2 1 0  segment (2 edges clipped)
            //   2 0 1  vertex
            //   3 0 0  none
            // The case (n,p,z) = (0,0,3) is treated as a no-intersection
            // because the triangle is degenerate.

            Real const zero = static_cast<Real>(0);
            std::array<Real, 3> s{ zero, zero, zero };
            int32_t numPositive = 0, numNegative = 0, numZero = 0;
            for (size_t i = 0; i < 3; ++i)
            {
                Vector2<Real> diff = triangle.v[i] - lineOrigin;
                s[i] = DotPerp(lineDirection, diff);
                if (s[i] > zero)
                {
                    ++numPositive;
                }
                else if (s[i] < zero)
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
                Real sign = (3 > numPositive * 2 ? static_cast<Real>(1) : static_cast<Real>(-1));
                for (size_t i0 = 0; i0 < 3; ++i0)
                {
                    if (sign * s[i0] > zero)
                    {
                        size_t i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                        Real s1 = s[i1] / (s[i1] - s[i0]);
                        Vector2<Real> p1 = (triangle.v[i1] - lineOrigin) +
                            s1 * (triangle.v[i0] - triangle.v[i1]);
                        result.parameter[0] = Dot(lineDirection, p1);
                        Real s2 = s[i2] / (s[i2] - s[i0]);
                        Vector2<Real> p2 = (triangle.v[i2] - lineOrigin) +
                            s2 * (triangle.v[i0] - triangle.v[i2]);
                        result.parameter[1] = Dot(lineDirection, p2);
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        break;
                    }
                }
                return;
            }

            if (numZero == 1)
            {
                result.intersect = true;
                for (size_t i0 = 0; i0 < 3; ++i0)
                {
                    if (s[i0] == zero)
                    {
                        size_t i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                        result.parameter[0] =
                            Dot(lineDirection, triangle.v[i0] - lineOrigin);
                        if (numPositive == 2 || numNegative == 2)
                        {
                            result.numIntersections = 1;

                            // Used by derived classes.
                            result.parameter[1] = result.parameter[0];
                        }
                        else
                        {
                            result.numIntersections = 2;
                            Real s1 = s[i1] / (s[i1] - s[i2]);
                            Vector2<Real> p1 = (triangle.v[i1] - lineOrigin) +
                                s1 * (triangle.v[i2] - triangle.v[i1]);
                            result.parameter[1] = Dot(lineDirection, p1);
                            if (result.parameter[0] > result.parameter[1])
                            {
                                std::swap(result.parameter[0], result.parameter[1]);
                            }
                        }
                        break;
                    }
                }
                return;
            }

            if (numZero == 2)
            {
                result.intersect = true;
                result.numIntersections = 2;
                for (size_t i0 = 0; i0 < 3; ++i0)
                {
                    if (s[i0] != zero)
                    {
                        size_t i1 = (i0 + 1) % 3, i2 = (i0 + 2) % 3;
                        result.parameter[0] =
                            Dot(lineDirection, triangle.v[i1] - lineOrigin);
                        result.parameter[1] =
                            Dot(lineDirection, triangle.v[i2] - lineOrigin);
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        break;
                    }
                }
                return;
            }

            // (n,p,z) is one of (3,0,0), (0,3,0), (0,0,3). The constructor
            // for Result initializes all members to zero, so no additional
            // assignments are needed for 'result'.
        }
    };
}
