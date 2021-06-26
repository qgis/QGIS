// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3AlignedBox3.h>
#include <Mathematics/Segment.h>

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
    class TIQuery<Real, Segment3<Real>, AlignedBox3<Real>>
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

        Result operator()(Segment3<Real> const& segment, AlignedBox3<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector3<Real>::Unit(d).
            Vector3<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector3<Real> transformedP0 = segment.p[0] - boxCenter;
            Vector3<Real> transformedP1 = segment.p[1] - boxCenter;
            Segment3<Real> transformedSegment(transformedP0, transformedP1);
            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result;
            DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& segOrigin, Vector3<Real> const& segDirection,
            Real segExtent, Vector3<Real> const& boxExtent, Result& result)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (std::fabs(segOrigin[i]) > boxExtent[i] + segExtent * std::fabs(segDirection[i]))
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<Real, Line3<Real>, AlignedBox3<Real>>::DoQuery(segOrigin, segDirection, boxExtent, result);
        }
    };

    template <typename Real>
    class FIQuery<Real, Segment3<Real>, AlignedBox3<Real>>
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

        Result operator()(Segment3<Real> const& segment, AlignedBox3<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector3<Real>::Unit(d).
            Vector3<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector3<Real> transformedP0 = segment.p[0] - boxCenter;
            Vector3<Real> transformedP1 = segment.p[1] - boxCenter;
            Segment3<Real> transformedSegment(transformedP0, transformedP1);
            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result;
            DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);

            // The segment origin is in aligned-box coordinates.  Transform it
            // back to the original space.
            segOrigin += boxCenter;
            for (int i = 0; i < result.numPoints; ++i)
            {
                result.point[i] = segOrigin + result.lineParameter[i] * segDirection;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& segOrigin, Vector3<Real> const& segDirection,
            Real segExtent, Vector3<Real> const& boxExtent, Result& result)
        {
            FIQuery<Real, Line3<Real>, AlignedBox3<Real>>::DoQuery(segOrigin, segDirection, boxExtent, result);
            if (result.intersect)
            {
                // The line containing the segment intersects the box; the
                // t-interval is [t0,t1].  The segment intersects the box as
                // long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;

                std::array<Real, 2> interval0 =
                {
                    result.lineParameter[0], result.lineParameter[1]
                };

                std::array<Real, 2> interval1 =
                {
                    -segExtent, segExtent
                };

                auto iiResult = iiQuery(interval0, interval1);
                if (iiResult.numIntersections > 0)
                {
                    result.numPoints = iiResult.numIntersections;
                    for (int i = 0; i < result.numPoints; ++i)
                    {
                        result.lineParameter[i] = iiResult.overlap[i];
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
