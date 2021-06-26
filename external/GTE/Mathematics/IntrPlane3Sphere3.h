// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/DistPoint3Plane3.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Circle3.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Plane3<Real>, Sphere3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Plane3<Real> const& plane, Sphere3<Real> const& sphere)
        {
            Result result;
            DCPQuery<Real, Vector3<Real>, Plane3<Real>> ppQuery;
            auto ppResult = ppQuery(sphere.center, plane);
            result.intersect = (ppResult.distance <= sphere.radius);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Plane3<Real>, Sphere3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // If 'intersect' is true, the intersection is either a point or a
            // circle.  When 'isCircle' is true, 'circle' is valid.  When
            // 'isCircle' is false, 'point' is valid.
            bool isCircle;
            Circle3<Real> circle;
            Vector3<Real> point;
        };

        Result operator()(Plane3<Real> const& plane, Sphere3<Real> const& sphere)
        {
            Result result;
            DCPQuery<Real, Vector3<Real>, Plane3<Real>> ppQuery;
            auto ppResult = ppQuery(sphere.center, plane);
            if (ppResult.distance < sphere.radius)
            {
                result.intersect = true;
                result.isCircle = true;
                result.circle.center = sphere.center - ppResult.signedDistance * plane.normal;
                result.circle.normal = plane.normal;

                // The sum and diff are both positive numbers.
                Real sum = sphere.radius + ppResult.distance;
                Real dif = sphere.radius - ppResult.distance;

                // arg = sqr(sphere.radius) - sqr(ppResult.distance)
                Real arg = sum * dif;

                result.circle.radius = std::sqrt(arg);
                return result;
            }
            else if (ppResult.distance == sphere.radius)
            {
                result.intersect = true;
                result.isCircle = false;
                result.point = sphere.center - ppResult.signedDistance * plane.normal;
                return result;
            }
            else
            {
                result.intersect = false;
                return result;
            }
        }
    };
}
