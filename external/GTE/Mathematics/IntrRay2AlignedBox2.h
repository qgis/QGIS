// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2AlignedBox2.h>
#include <Mathematics/Ray.h>
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
    class TIQuery<Real, Ray2<Real>, AlignedBox2<Real>>
        :
        public TIQuery<Real, Line2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public TIQuery<Real, Line2<Real>, AlignedBox2<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray2<Real> const& ray, AlignedBox2<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<Real>::Unit(d).
            Vector2<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector2<Real> rayOrigin = ray.origin - boxCenter;

            Result result;
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& rayOrigin,
            Vector2<Real> const& rayDirection, Vector2<Real> const& boxExtent,
            Result& result)
        {
            for (int i = 0; i < 2; ++i)
            {
                if (std::fabs(rayOrigin[i]) > boxExtent[i]
                    && rayOrigin[i] * rayDirection[i] >= (Real)0)
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<Real, Line2<Real>, AlignedBox2<Real>>::DoQuery(rayOrigin,
                rayDirection, boxExtent, result);
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray2<Real>, AlignedBox2<Real>>
        :
        public FIQuery<Real, Line2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line2<Real>, AlignedBox2<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray2<Real> const& ray, AlignedBox2<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<Real>::Unit(d).
            Vector2<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector2<Real> rayOrigin = ray.origin - boxCenter;

            Result result;
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& rayOrigin,
            Vector2<Real> const& rayDirection, Vector2<Real> const& boxExtent,
            Result& result)
        {
            FIQuery<Real, Line2<Real>, AlignedBox2<Real>>::DoQuery(rayOrigin,
                rayDirection, boxExtent, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the box; the
                // t-interval is [t0,t1].  The ray intersects the box as long
                // as [t0,t1] overlaps the ray t-interval [0,+infinity).
                std::array<Real, 2> rayInterval =
                { (Real)0, std::numeric_limits<Real>::max() };
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, rayInterval);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
                result.parameter = iiResult.overlap;
            }
        }
    };
}
