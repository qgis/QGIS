// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistRaySegment.h>
#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Capsule3.h>

// The queries consider the capsule to be a solid.
//
// The test-intersection queries are based on distance computations.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray3<Real>, Capsule3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Ray3<Real> const& ray, Capsule3<Real> const& capsule)
        {
            Result result;
            DCPQuery<Real, Ray3<Real>, Segment3<Real>> rsQuery;
            auto rsResult = rsQuery(ray, capsule.segment);
            result.intersect = (rsResult.distance <= capsule.radius);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray3<Real>, Capsule3<Real>>
        :
        public FIQuery<Real, Line3<Real>, Capsule3<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line3<Real>, Capsule3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray3<Real> const& ray, Capsule3<Real> const& capsule)
        {
            Result result;
            DoQuery(ray.origin, ray.direction, capsule, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& rayOrigin,
            Vector3<Real> const& rayDirection, Capsule3<Real> const& capsule,
            Result& result)
        {
            FIQuery<Real, Line3<Real>, Capsule3<Real>>::DoQuery(rayOrigin,
                rayDirection, capsule, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the capsule; the
                // t-interval is [t0,t1].  The ray intersects the capsule as
                // long as [t0,t1] overlaps the ray t-interval [0,+infinity).
                std::array<Real, 2> rayInterval = { (Real)0, std::numeric_limits<Real>::max() };
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
