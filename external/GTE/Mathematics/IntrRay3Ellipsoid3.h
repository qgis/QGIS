// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.02.10

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Ellipsoid3.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Matrix3x3.h>

// The queries consider the ellipsoid to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray3<Real>, Ellipsoid3<Real>>
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

        Result operator()(Ray3<Real> const& ray, Ellipsoid3<Real> const& ellipsoid)
        {
            // The ellipsoid is (X-K)^T*M*(X-K)-1 = 0 and the line is
            // X = P+t*D.  Substitute the line equation into the ellipsoid
            // equation to obtain a quadratic equation
            //   Q(t) = a2*t^2 + 2*a1*t + a0 = 0
            // where a2 = D^T*M*D, a1 = D^T*M*(P-K) and
            // a0 = (P-K)^T*M*(P-K)-1.
            Real constexpr zero = 0;
            Result result{};

            Matrix3x3<Real> M;
            ellipsoid.GetM(M);

            Vector3<Real> diff = ray.origin - ellipsoid.center;
            Vector3<Real> matDir = M * ray.direction;
            Vector3<Real> matDiff = M * diff;
            Real a2 = Dot(ray.direction, matDir);
            Real a1 = Dot(ray.direction, matDiff);
            Real a0 = Dot(diff, matDiff) - (Real)1;

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
                    // vector D.  Thus, an intersection occurs only when
                    // Q'(0) < 0.
                    result.intersect = (a1 < zero);
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
    class FIQuery<Real, Ray3<Real>, Ellipsoid3<Real>>
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

        Result operator()(Ray3<Real> const& ray, Ellipsoid3<Real> const& ellipsoid)
        {
            Result result{};
            DoQuery(ray.origin, ray.direction, ellipsoid, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& rayOrigin,
            Vector3<Real> const& rayDirection, Ellipsoid3<Real> const& ellipsoid,
            Result& result)
        {
            FIQuery<Real, Line3<Real>, Ellipsoid3<Real>>::DoQuery(rayOrigin,
                rayDirection, ellipsoid, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the ellipsoid; the
                // t-interval is [t0,t1].  The ray intersects the capsule as
                // long as [t0,t1] overlaps the ray t-interval [0,+infinity).
                Real constexpr zero = 0;
                Real constexpr rmax = std::numeric_limits<Real>::max();
                std::array<Real, 2> rayInterval = { zero, rmax };
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, rayInterval);
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
