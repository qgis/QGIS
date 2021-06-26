// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2AlignedBox2.h>
#include <Mathematics/Segment.h>
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
    class TIQuery<Real, Segment2<Real>, AlignedBox2<Real>>
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

        Result operator()(Segment2<Real> const& segment, AlignedBox2<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<Real>::Unit(d).
            Vector2<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector2<Real> transformedP0 = segment.p[0] - boxCenter;
            Vector2<Real> transformedP1 = segment.p[1] - boxCenter;
            Segment2<Real> transformedSegment(transformedP0, transformedP1);
            Vector2<Real> segOrigin, segDirection;
            Real segExtent;
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result;
            DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& segOrigin,
            Vector2<Real> const& segDirection, Real segExtent,
            Vector2<Real> const& boxExtent, Result& result)
        {
            for (int i = 0; i < 2; ++i)
            {
                Real lhs = std::fabs(segOrigin[i]);
                Real rhs = boxExtent[i] + segExtent * std::fabs(segDirection[i]);
                if (lhs > rhs)
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<Real, Line2<Real>, AlignedBox2<Real>>::DoQuery(segOrigin,
                segDirection, boxExtent, result);
        }
    };

    template <typename Real>
    class FIQuery<Real, Segment2<Real>, AlignedBox2<Real>>
        :
        public FIQuery<Real, Line2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line2<Real>, AlignedBox2<Real>>::Result
        {
            // The base class parameter[] values are t-values for the
            // segment parameterization (1-t)*p[0] + t*p[1], where t in [0,1].
            // The values in this class are s-values for the centered form
            // C + s * D, where s in [-e,e] and e is the extent of the
            // segment.
            std::array<Real, 2> cdeParameter;
        };

        Result operator()(Segment2<Real> const& segment, AlignedBox2<Real> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<Real>::Unit(d).
            Vector2<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector2<Real> transformedP0 = segment.p[0] - boxCenter;
            Vector2<Real> transformedP1 = segment.p[1] - boxCenter;
            Segment2<Real> transformedSegment(transformedP0, transformedP1);
            Vector2<Real> segOrigin, segDirection;
            Real segExtent;
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result;
            DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                // Compute the segment in the aligned-box coordinate system
                // and then translate it back to the original coordinates
                // using the box cener.
                result.point[i] = boxCenter + (segOrigin + result.parameter[i] * segDirection);
                result.cdeParameter[i] = result.parameter[i];

                // Convert the parameters from the centered form to the
                // endpoint form.
                result.parameter[i] = (result.parameter[i] / segExtent + (Real)1) * (Real)0.5;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& segOrigin,
            Vector2<Real> const& segDirection, Real segExtent,
            Vector2<Real> const& boxExtent, Result& result)
        {
            FIQuery<Real, Line2<Real>, AlignedBox2<Real>>::DoQuery(segOrigin,
                segDirection, boxExtent, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the box; the
                // t-interval is [t0,t1].  The segment intersects the box as
                // long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                std::array<Real, 2> segInterval = { -segExtent, segExtent };
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, segInterval);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
                result.parameter = iiResult.overlap;
            }
        }
    };
}
