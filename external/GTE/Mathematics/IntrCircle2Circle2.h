// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Circle2<Real>, Circle2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Circle2<Real> const& circle0, Circle2<Real> const& circle1)
        {
            Result result;
            Vector2<Real> diff = circle0.center - circle1.center;
            result.intersect = (Length(diff) <= circle0.radius + circle1.radius);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Circle2<Real>, Circle2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The number of intersections is 0, 1, 2 or maxInt =
            // std::numeric_limits<int>::max().  When 1, the circles are
            // tangent and intersect in a single point.  When 2, circles have
            // two transverse intersection points.  When maxInt, the circles
            // are the same.
            int numIntersections;

            // Valid only when numIntersections = 1 or 2.
            Vector2<Real> point[2];

            // Valid only when numIntersections = maxInt.
            Circle2<Real> circle;
        };

        Result operator()(Circle2<Real> const& circle0, Circle2<Real> const& circle1)
        {
            // The two circles are |X-C0| = R0 and |X-C1| = R1.  Define
            // U = C1 - C0 and V = Perp(U) where Perp(x,y) = (y,-x).  Note
            // that Dot(U,V) = 0 and |V|^2 = |U|^2.  The intersection points X
            // can be written in the form X = C0+s*U+t*V and
            // X = C1+(s-1)*U+t*V.  Squaring the circle equations and
            // substituting these formulas into them yields
            //   R0^2 = (s^2 + t^2)*|U|^2
            //   R1^2 = ((s-1)^2 + t^2)*|U|^2.
            // Subtracting and solving for s yields
            //   s = ((R0^2-R1^2)/|U|^2 + 1)/2
            // Then replace in the first equation and solve for t^2
            //   t^2 = (R0^2/|U|^2) - s^2.
            // In order for there to be solutions, the right-hand side must be
            // nonnegative.  Some algebra leads to the condition for existence
            // of solutions,
            //   (|U|^2 - (R0+R1)^2)*(|U|^2 - (R0-R1)^2) <= 0.
            // This reduces to
            //   |R0-R1| <= |U| <= |R0+R1|.
            // If |U| = |R0-R1|, then the circles are side-by-side and just
            // tangent.  If |U| = |R0+R1|, then the circles are nested and
            // just tangent.  If |R0-R1| < |U| < |R0+R1|, then the two circles
            // to intersect in two points.

            Result result;

            Vector2<Real> U = circle1.center - circle0.center;
            Real USqrLen = Dot(U, U);
            Real R0 = circle0.radius, R1 = circle1.radius;
            Real R0mR1 = R0 - R1;
            if (USqrLen == (Real)0 && R0mR1 == (Real)0)
            {
                // Circles are the same.
                result.intersect = true;
                result.numIntersections = std::numeric_limits<int>::max();
                result.circle = circle0;
                return result;
            }

            Real R0mR1Sqr = R0mR1 * R0mR1;
            if (USqrLen < R0mR1Sqr)
            {
                // The circles do not intersect.
                result.intersect = false;
                result.numIntersections = 0;
                return result;
            }

            Real R0pR1 = R0 + R1;
            Real R0pR1Sqr = R0pR1 * R0pR1;
            if (USqrLen > R0pR1Sqr)
            {
                // The circles do not intersect.
                result.intersect = false;
                result.numIntersections = 0;
                return result;
            }

            if (USqrLen < R0pR1Sqr)
            {
                if (R0mR1Sqr < USqrLen)
                {
                    Real invUSqrLen = (Real)1 / USqrLen;
                    Real s = (Real)0.5 * ((R0 * R0 - R1 * R1) * invUSqrLen + (Real)1);
                    Vector2<Real> tmp = circle0.center + s * U;

                    // In theory, discr is nonnegative.  However, numerical round-off
                    // errors can make it slightly negative.  Clamp it to zero.
                    Real discr = R0 * R0 * invUSqrLen - s * s;
                    if (discr < (Real)0)
                    {
                        discr = (Real)0;
                    }
                    Real t = std::sqrt(discr);
                    Vector2<Real> V{ U[1], -U[0] };
                    result.point[0] = tmp - t * V;
                    result.point[1] = tmp + t * V;
                    result.numIntersections = (t > (Real)0 ? 2 : 1);
                }
                else
                {
                    // |U| = |R0-R1|, circles are tangent.
                    result.numIntersections = 1;
                    result.point[0] = circle0.center + (R0 / R0mR1) * U;
                }
            }
            else
            {
                // |U| = |R0+R1|, circles are tangent.
                result.numIntersections = 1;
                result.point[0] = circle0.center + (R0 / R0pR1) * U;
            }

            // The circles intersect in 1 or 2 points.
            result.intersect = true;
            return result;
        }
    };
}
