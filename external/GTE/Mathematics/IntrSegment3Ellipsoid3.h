// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.02.10

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Ellipsoid3.h>
#include <Mathematics/Segment.h>
#include <Mathematics/Matrix3x3.h>

// The queries consider the ellipsoid to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Segment3<Real>, Ellipsoid3<Real>>
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

        Result operator()(Segment3<Real> const& segment, Ellipsoid3<Real> const& ellipsoid)
        {
            // The ellipsoid is (X-K)^T*M*(X-K)-1 = 0 and the line is
            // X = P+t*D.  Substitute the line equation into the ellipsoid
            // equation to obtain a quadratic equation
            //   Q(t) = a2*t^2 + 2*a1*t + a0 = 0
            // where a2 = D^T*M*D, a1 = D^T*M*(P-K) and
            // a0 = (P-K)^T*M*(P-K)-1.
            Real constexpr zero = 0;
            Result result{};

            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Matrix3x3<Real> M;
            ellipsoid.GetM(M);

            Real constexpr one = 1;
            Vector3<Real> diff = segOrigin - ellipsoid.center;
            Vector3<Real> matDir = M * segDirection;
            Vector3<Real> matDiff = M * diff;
            Real a2 = Dot(segDirection, matDir);
            Real a1 = Dot(segDirection, matDiff);
            Real a0 = Dot(diff, matDiff) - one;

            Real discr = a1 * a1 - a0 * a2;
            if (discr >= zero)
            {
                // Test whether ray origin is inside ellipsoid.
                if (a0 <= zero)
                {
                    result.intersect = true;
                }
                else
                {
                    // At this point, Q(0) = a0 > 0 and Q(t) has real roots.
                    // It is also the case that a2 > 0, since M is positive
                    // definite, implying that D^T*M*D > 0 for any nonzero
                    // vector D.
                    Real q, qder;
                    if (a1 >= zero)
                    {
                        // Roots are possible only on [-e,0], e is the segment
                        // extent.  At least one root occurs if Q(-e) <= 0 or
                        // if Q(-e) > 0 and Q'(-e) < 0.
                        Real constexpr negTwo = -2;
                        q = a0 + segExtent * (negTwo * a1 + a2 * segExtent);
                        if (q <= zero)
                        {
                            result.intersect = true;
                        }
                        else
                        {
                            qder = a1 - a2 * segExtent;
                            result.intersect = (qder < zero);
                        }
                    }
                    else
                    {
                        // Roots are only possible on [0,e], e is the segment
                        // extent.  At least one root occurs if Q(e) <= 0 or
                        // if Q(e) > 0 and Q'(e) > 0.
                        Real constexpr two = 2;
                        q = a0 + segExtent * (two * a1 + a2 * segExtent);
                        if (q <= zero)
                        {
                            result.intersect = true;
                        }
                        else
                        {
                            qder = a1 + a2 * segExtent;
                            result.intersect = (qder < zero);
                        }
                    }
                }
            }
            else
            {
                // No intersection if Q(t) has no real roots.
                result.intersect = false;
            }

            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Segment3<Real>, Ellipsoid3<Real>>
        :
        public FIQuery<Real, Line3<Real>, Ellipsoid3<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line3<Real>, Ellipsoid3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Segment3<Real> const& segment, Ellipsoid3<Real> const& ellipsoid)
        {
            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, ellipsoid, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = segOrigin + result.parameter[i] * segDirection;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& segOrigin,
            Vector3<Real> const& segDirection, Real segExtent,
            Ellipsoid3<Real> const& ellipsoid, Result& result)
        {
            FIQuery<Real, Line3<Real>, Ellipsoid3<Real>>::DoQuery(segOrigin,
                segDirection, ellipsoid, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the ellipsoid;
                // the t-interval is [t0,t1].  The segment intersects the
                // ellipsoid as long as [t0,t1] overlaps the segment
                // t-interval [-segExtent,+segExtent].
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
