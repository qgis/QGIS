// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrSegment3AlignedBox3.h>
#include <Mathematics/OrientedBox.h>

// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the six
// faces of the box.  The find-intersection queries use Liang-Barsky
// clipping.  The queries consider the box to be a solid.  The algorithms
// are described in
// https://www.geometrictools.com/Documentation/IntersectionLineBox.pdf

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Segment3<Real>, OrientedBox3<Real>>
        :
        public TIQuery<Real, Segment3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
            :
            public TIQuery<Real, Segment3<Real>, AlignedBox3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Segment3<Real> const& segment, OrientedBox3<Real> const& box)
        {
            // Transform the segment to the oriented-box coordinate system.
            Vector3<Real> tmpOrigin, tmpDirection;
            Real segExtent;
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector3<Real> diff = tmpOrigin - box.center;
            Vector3<Real> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1]),
                Dot(diff, box.axis[2])
            };
            Vector3<Real> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1]),
                Dot(tmpDirection, box.axis[2])
            };

            Result result;
            this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Segment3<Real>, OrientedBox3<Real>>
        :
        public FIQuery<Real, Segment3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Segment3<Real>, AlignedBox3<Real>>::Result
        {
            // No additional relevant information to compute.
        };

        Result operator()(Segment3<Real> const& segment, OrientedBox3<Real> const& box)
        {
            // Transform the segment to the oriented-box coordinate system.
            Vector3<Real> tmpOrigin, tmpDirection;
            Real segExtent;
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector3<Real> diff = tmpOrigin - box.center;
            Vector3<Real> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1]),
                Dot(diff, box.axis[2])
            };
            Vector3<Real> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1]),
                Dot(tmpDirection, box.axis[2])
            };

            Result result;
            this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
            for (int i = 0; i < result.numPoints; ++i)
            {
                // Compute the intersection point in the oriented-box
                // coordinate system.
                Vector3<Real> y = segOrigin + result.lineParameter[i] * segDirection;

                // Transform the intersection point to the original coordinate
                // system.
                result.point[i] = box.center;
                for (int j = 0; j < 3; ++j)
                {
                    result.point[i] += y[j] * box.axis[j];
                }
            }

            return result;
        }
    };
}
