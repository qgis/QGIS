// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrSegment2AlignedBox2.h>
#include <Mathematics/OrientedBox.h>

// The queries consider the box to be a solid.
//
// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the four
// edges of the box.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Segment2<Real>, OrientedBox2<Real>>
        :
        public TIQuery<Real, Segment2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public TIQuery<Real, Segment2<Real>, AlignedBox2<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Segment2<Real> const& segment, OrientedBox2<Real> const& box)
        {
            // Transform the segment to the oriented-box coordinate system.
            Vector2<Real> tmpOrigin, tmpDirection;
            Real segExtent;
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector2<Real> diff = tmpOrigin - box.center;
            Vector2<Real> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<Real> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1])
            };

            Result result;
            this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Segment2<Real>, OrientedBox2<Real>>
        :
        public FIQuery<Real, Segment2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Segment2<Real>, AlignedBox2<Real>>::Result
        {
            // The base class parameter[] values are t-values for the
            // segment parameterization (1-t)*p[0] + t*p[1], where t in [0,1].
            // The values in this class are s-values for the centered form
            // C + s * D, where s in [-e,e] and e is the extent of the
            // segment.
            std::array<Real, 2> cdeParameter;
        };

        Result operator()(Segment2<Real> const& segment, OrientedBox2<Real> const& box)
        {
            // Transform the segment to the oriented-box coordinate system.
            Vector2<Real> tmpOrigin, tmpDirection;
            Real segExtent;
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector2<Real> diff = tmpOrigin - box.center;
            Vector2<Real> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<Real> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1])
            };

            Result result;
            this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                // Compute the segment in the aligned-box coordinate system
                // and then translate it back to the original coordinates
                // using the box cener.
                result.point[i] = box.center + (segOrigin + result.parameter[i] * segDirection);
                result.cdeParameter[i] = result.parameter[i];

                // Convert the parameters from the centered form to the
                // endpoint form.
                result.parameter[i] = (result.parameter[i] / segExtent + (Real)1) * (Real)0.5;
            }
            return result;
        }
    };
}
