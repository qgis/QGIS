// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrAlignedBox3Cylinder3.h>
#include <Mathematics/OrientedBox.h>

// The query considers the cylinder and box to be solids.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox3<Real>, Cylinder3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(OrientedBox3<Real> const& box, Cylinder3<Real> const& cylinder)
        {
            // Transform the box and cylinder so that the box is axis-aligned.
            AlignedBox3<Real> aabb(-box.extent, box.extent);
            Vector3<Real> diff = cylinder.axis.origin - box.center;
            Cylinder3<Real> transformedCylinder;
            transformedCylinder.radius = cylinder.radius;
            transformedCylinder.height = cylinder.height;
            for (int i = 0; i < 3; ++i)
            {
                transformedCylinder.axis.origin[i] = Dot(box.axis[i], diff);
                transformedCylinder.axis.direction[i] = Dot(box.axis[i], cylinder.axis.direction);
            }

            TIQuery<Real, AlignedBox3<Real>, Cylinder3<Real>> aabbCylinderQuery;
            auto aabbCylinderResult = aabbCylinderQuery(aabb, transformedCylinder);
            Result result;
            result.intersect = aabbCylinderResult.intersect;
            return result;
        }
    };
}
