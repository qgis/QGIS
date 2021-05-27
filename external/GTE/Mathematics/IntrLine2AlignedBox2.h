// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/Vector2.h>

// The queries consider the box to be a solid.
//
// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the four
// edges of the box.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Line2<Real> const& line, AlignedBox2<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<Real>::Unit(d).
            Vector2<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the line to the aligned-box coordinate system.
            Vector2<Real> lineOrigin = line.origin - boxCenter;

            Result result;
            DoQuery(lineOrigin, line.direction, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& lineOrigin,
            Vector2<Real> const& lineDirection, Vector2<Real> const& boxExtent,
            Result& result)
        {
            Real LHS = std::fabs(DotPerp(lineDirection, lineOrigin));
            Real RHS =
                boxExtent[0] * std::fabs(lineDirection[1]) +
                boxExtent[1] * std::fabs(lineDirection[0]);
            result.intersect = (LHS <= RHS);
        }
    };

    template <typename Real>
    class FIQuery<Real, Line2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector2<Real>, 2> point;
        };

        Result operator()(Line2<Real> const& line, AlignedBox2<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<Real>::Unit(d).
            Vector2<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the line to the aligned-box coordinate system.
            Vector2<Real> lineOrigin = line.origin - boxCenter;

            Result result;
            DoQuery(lineOrigin, line.direction, boxExtent, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& lineOrigin,
            Vector2<Real> const& lineDirection, Vector2<Real> const& boxExtent,
            Result& result)
        {
            // The line t-values are in the interval (-infinity,+infinity).
            // Clip the line against all four planes of an aligned box in
            // centered form.  The result.numPoints is
            //  0, no intersection
            //  1, intersect in a single point (t0 is line parameter of point)
            //  2, intersect in a segment (line parameter interval is [t0,t1])
            Real t0 = -std::numeric_limits<Real>::max();
            Real t1 = std::numeric_limits<Real>::max();
            if (Clip(+lineDirection[0], -lineOrigin[0] - boxExtent[0], t0, t1) &&
                Clip(-lineDirection[0], +lineOrigin[0] - boxExtent[0], t0, t1) &&
                Clip(+lineDirection[1], -lineOrigin[1] - boxExtent[1], t0, t1) &&
                Clip(-lineDirection[1], +lineOrigin[1] - boxExtent[1], t0, t1))
            {
                result.intersect = true;
                if (t1 > t0)
                {
                    result.numIntersections = 2;
                    result.parameter[0] = t0;
                    result.parameter[1] = t1;
                }
                else
                {
                    result.numIntersections = 1;
                    result.parameter[0] = t0;
                    result.parameter[1] = t0;  // Used by derived classes.
                }
                return;
            }

            result.intersect = false;
            result.numIntersections = 0;
        }

    private:
        // Test whether the current clipped segment intersects the current
        // test plane.  If the return value is 'true', the segment does
        // intersect the plane and is clipped; otherwise, the segment is
        // culled (no intersection with box).
        static bool Clip(Real denom, Real numer, Real& t0, Real& t1)
        {
            if (denom > (Real)0)
            {
                if (numer > denom * t1)
                {
                    return false;
                }
                if (numer > denom * t0)
                {
                    t0 = numer / denom;
                }
                return true;
            }
            else if (denom < (Real)0)
            {
                if (numer > denom * t0)
                {
                    return false;
                }
                if (numer > denom * t1)
                {
                    t1 = numer / denom;
                }
                return true;
            }
            else
            {
                return numer <= (Real)0;
            }
        }
    };
}
