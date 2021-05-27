// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrLine3AlignedBox3.h>
#include <Mathematics/Ray.h>

// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the six
// faces of the box.  The find-intersection queries use Liang-Barsky
// clipping.  The queries consider the box to be a solid. The algorithms
// are described in
// https://www.geometrictools.com/Documentation/IntersectionLineBox.pdf

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray3<Real>, AlignedBox3<Real>>
        :
        public TIQuery<Real, Line3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
            :
            public TIQuery<Real, Line3<Real>, AlignedBox3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray3<Real> const& ray, AlignedBox3<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector3<Real>::Unit(d).
            Vector3<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector3<Real> rayOrigin = ray.origin - boxCenter;

            Result result;
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& rayOrigin, Vector3<Real> const& rayDirection,
            Vector3<Real> const& boxExtent, Result& result)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (std::fabs(rayOrigin[i]) > boxExtent[i] && rayOrigin[i] * rayDirection[i] >= (Real)0)
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<Real, Line3<Real>, AlignedBox3<Real>>::DoQuery(rayOrigin, rayDirection, boxExtent, result);
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray3<Real>, AlignedBox3<Real>>
        :
        public FIQuery<Real, Line3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line3<Real>, AlignedBox3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray3<Real> const& ray, AlignedBox3<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector3<Real>::Unit(d).
            Vector3<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector3<Real> rayOrigin = ray.origin - boxCenter;

            Result result;
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            for (int i = 0; i < result.numPoints; ++i)
            {
                result.point[i] = ray.origin + result.lineParameter[i] * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& rayOrigin, Vector3<Real> const& rayDirection,
            Vector3<Real> const& boxExtent, Result& result)
        {
            FIQuery<Real, Line3<Real>, AlignedBox3<Real>>::DoQuery(rayOrigin, rayDirection, boxExtent, result);
            if (result.intersect)
            {
                // The line containing the ray intersects the box; the
                // t-interval is [t0,t1].  The ray intersects the box as long
                // as [t0,t1] overlaps the ray t-interval (0,+infinity).
                if (result.lineParameter[1] >= (Real)0)
                {
                    if (result.lineParameter[0] < (Real)0)
                    {
                        result.lineParameter[0] = (Real)0;
                    }
                }
                else
                {
                    result.intersect = false;
                    result.numPoints = 0;
                }
            }
        }
    };
}
