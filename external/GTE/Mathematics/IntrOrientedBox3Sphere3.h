// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistPointOrientedBox.h>
#include <Mathematics/IntrAlignedBox3Sphere3.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox3<Real>, Sphere3<Real>>
        :
        public TIQuery<Real, AlignedBox3<Real>, Sphere3<Real>>
    {
    public:
        // The intersection query considers the box and sphere to be solids.
        // For example, if the sphere is strictly inside the box (does not
        // touch the box faces), the objects intersect.
        struct Result
            :
            public TIQuery<Real, AlignedBox3<Real>, Sphere3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(OrientedBox3<Real> const& box, Sphere3<Real> const& sphere)
        {
            DCPQuery<Real, Vector3<Real>, OrientedBox3<Real>> pbQuery;
            auto pbResult = pbQuery(sphere.center, box);
            Result result;
            result.intersect = (pbResult.sqrDistance <= sphere.radius * sphere.radius);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, OrientedBox3<Real>, Sphere3<Real>>
        :
        public FIQuery<Real, AlignedBox3<Real>, Sphere3<Real>>
    {
    public:
        // Currently, only a dynamic query is supported.  The static query
        // must compute the intersection set of (solid) box and sphere.
        struct Result
            :
            public FIQuery<Real, AlignedBox3<Real>, Sphere3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(OrientedBox3<Real> const& box, Vector3<Real> const& boxVelocity,
            Sphere3<Real> const& sphere, Vector3<Real> const& sphereVelocity)
        {
            Result result;
            result.intersectionType = 0;
            result.contactTime = (Real)0;
            result.contactPoint = { (Real)0, (Real)0, (Real)0 };

            // Transform the sphere and box so that the box center becomes
            // the origin and the box is axis aligned.  Compute the velocity
            // of the sphere relative to the box.
            Vector3<Real> temp = sphere.center - box.center;
            Vector3<Real> C
            {
                Dot(temp, box.axis[0]),
                Dot(temp, box.axis[1]),
                Dot(temp, box.axis[2])
            };

            temp = sphereVelocity - boxVelocity;
            Vector3<Real> V
            {
                Dot(temp, box.axis[0]),
                Dot(temp, box.axis[1]),
                Dot(temp, box.axis[2])
            };

            this->DoQuery(box.extent, C, sphere.radius, V, result);

            // Transform back to the original coordinate system.
            if (result.intersectionType != 0)
            {
                auto& P = result.contactPoint;
                P = box.center + P[0] * box.axis[0] + P[1] * box.axis[1] + P[2] * box.axis[2];
            }
            return result;
        }
    };
}
