// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrLine3Plane3.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray3<Real>, Plane3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Ray3<Real> const& ray, Plane3<Real> const& plane)
        {
            Result result;

            // Compute the (signed) distance from the ray origin to the plane.
            DCPQuery<Real, Vector3<Real>, Plane3<Real>> vpQuery;
            auto vpResult = vpQuery(ray.origin, plane);

            Real DdN = Dot(ray.direction, plane.normal);
            if (DdN > (Real)0)
            {
                // The ray is not parallel to the plane and is directed toward
                // the +normal side of the plane.
                result.intersect = (vpResult.signedDistance <= (Real)0);
            }
            else if (DdN < (Real)0)
            {
                // The ray is not parallel to the plane and is directed toward
                // the -normal side of the plane.
                result.intersect = (vpResult.signedDistance >= (Real)0);
            }
            else
            {
                // The ray and plane are parallel.
                result.intersect = (vpResult.distance == (Real)0);
            }

            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray3<Real>, Plane3<Real>>
        :
        public FIQuery<Real, Line3<Real>, Plane3<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line3<Real>, Plane3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray3<Real> const& ray, Plane3<Real> const& plane)
        {
            Result result;
            DoQuery(ray.origin, ray.direction, plane, result);
            if (result.intersect)
            {
                result.point = ray.origin + result.parameter * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& rayOrigin,
            Vector3<Real> const& rayDirection, Plane3<Real> const& plane,
            Result& result)
        {
            FIQuery<Real, Line3<Real>, Plane3<Real>>::DoQuery(rayOrigin,
                rayDirection, plane, result);
            if (result.intersect)
            {
                // The line intersects the plane in a point that might not be
                // on the ray.
                if (result.parameter < (Real)0)
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
        }
    };
}
