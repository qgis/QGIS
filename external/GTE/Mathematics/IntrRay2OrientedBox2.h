// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrRay2AlignedBox2.h>
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
    class TIQuery<Real, Ray2<Real>, OrientedBox2<Real>>
        :
        public TIQuery<Real, Ray2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public TIQuery<Real, Ray2<Real>, AlignedBox2<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray2<Real> const& ray, OrientedBox2<Real> const& box)
        {
            // Transform the ray to the oriented-box coordinate system.
            Vector2<Real> diff = ray.origin - box.center;
            Vector2<Real> rayOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<Real> rayDirection = Vector2<Real>
            {
                Dot(ray.direction, box.axis[0]),
                Dot(ray.direction, box.axis[1])
            };

            Result result;
            this->DoQuery(rayOrigin, rayDirection, box.extent, result);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray2<Real>, OrientedBox2<Real>>
        :
        public FIQuery<Real, Ray2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Ray2<Real>, AlignedBox2<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray2<Real> const& ray, OrientedBox2<Real> const& box)
        {
            // Transform the ray to the oriented-box coordinate system.
            Vector2<Real> diff = ray.origin - box.center;
            Vector2<Real> rayOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<Real> rayDirection = Vector2<Real>
            {
                Dot(ray.direction, box.axis[0]),
                    Dot(ray.direction, box.axis[1])
            };

            Result result;
            this->DoQuery(rayOrigin, rayDirection, box.extent, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }
    };
}
