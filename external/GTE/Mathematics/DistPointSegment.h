// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Vector<N, Real>, Segment<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real segmentParameter;  // t in [0,1]
            Vector<N, Real> segmentClosest;  // (1-t)*p[0] + t*p[1]
        };

        Result operator()(Vector<N, Real> const& point, Segment<N, Real> const& segment)
        {
            Result result;

            // The direction vector is not unit length.  The normalization is
            // deferred until it is needed.
            Vector<N, Real> direction = segment.p[1] - segment.p[0];
            Vector<N, Real> diff = point - segment.p[1];
            Real t = Dot(direction, diff);
            if (t >= (Real)0)
            {
                result.segmentParameter = (Real)1;
                result.segmentClosest = segment.p[1];
            }
            else
            {
                diff = point - segment.p[0];
                t = Dot(direction, diff);
                if (t <= (Real)0)
                {
                    result.segmentParameter = (Real)0;
                    result.segmentClosest = segment.p[0];
                }
                else
                {
                    Real sqrLength = Dot(direction, direction);
                    if (sqrLength > (Real)0)
                    {
                        t /= sqrLength;
                        result.segmentParameter = t;
                        result.segmentClosest = segment.p[0] + t * direction;
                    }
                    else
                    {
                        result.segmentParameter = (Real)0;
                        result.segmentClosest = segment.p[0];
                    }
                }
            }

            diff = point - result.segmentClosest;
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPPointSegment = DCPQuery<Real, Vector<N, Real>, Segment<N, Real>>;

    template <typename Real>
    using DCPPoint2Segment2 = DCPPointSegment<2, Real>;

    template <typename Real>
    using DCPPoint3Segment3 = DCPPointSegment<3, Real>;
}
