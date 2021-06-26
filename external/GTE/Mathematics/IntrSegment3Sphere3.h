// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.02.10

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Sphere3.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Segment3<Real>, Sphere3<Real>>
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

        Result operator()(Segment3<Real> const& segment, Sphere3<Real> const& sphere)
        {
            // The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D.
            // Substitute the line equation into the sphere equation to
            // obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where
            // a1 = D^T*(P-C) and a0 = (P-C)^T*(P-C)-1.
            Real constexpr zero = 0;
            Result result{};

            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Vector3<Real> diff = segOrigin - sphere.center;
            Real a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            Real a1 = Dot(segDirection, diff);
            Real discr = a1 * a1 - a0;
            if (discr < zero)
            {
                result.intersect = false;
                return result;
            }

            Real constexpr two = 2;
            Real tmp0 = segExtent * segExtent + a0;
            Real tmp1 = two * a1 * segExtent;
            Real qm = tmp0 - tmp1;
            Real qp = tmp0 + tmp1;
            if (qm * qp <= zero)
            {
                result.intersect = true;
                return result;
            }

            result.intersect = (qm > zero && std::fabs(a1) < segExtent);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Segment3<Real>, Sphere3<Real>>
        :
        public FIQuery<Real, Line3<Real>, Sphere3<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line3<Real>, Sphere3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Segment3<Real> const& segment, Sphere3<Real> const& sphere)
        {
            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, sphere, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = segOrigin + result.parameter[i] * segDirection;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& segOrigin,
            Vector3<Real> const& segDirection, Real segExtent,
            Sphere3<Real> const& sphere, Result& result)
        {
            FIQuery<Real, Line3<Real>, Sphere3<Real>>::DoQuery(segOrigin,
                segDirection, sphere, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the sphere; the
                // t-interval is [t0,t1].  The segment intersects the sphere
                // as long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                std::array<Real, 2> segInterval = { -segExtent, segExtent };
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, segInterval);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
        }
    };
}
