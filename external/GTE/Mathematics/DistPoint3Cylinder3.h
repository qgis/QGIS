// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Vector3.h>

// The queries consider the cylinder to be a solid.

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Vector3<Real>, Cylinder3<Real>>
    {
    public:
        struct Result
        {
            Real distance;
            Vector3<Real> cylinderClosest;
        };

        Result operator()(Vector3<Real> const& point, Cylinder3<Real> const& cylinder)
        {
            Result result;

            // Convert the point to the cylinder coordinate system.  In this
            // system, the point believes (0,0,0) is the cylinder axis origin
            // and (0,0,1) is the cylinder axis direction.
            Vector3<Real> basis[3];
            basis[0] = cylinder.axis.direction;
            ComputeOrthogonalComplement(1, basis);

            Vector3<Real> delta = point - cylinder.axis.origin;
            Vector3<Real> P
            {
                Dot(basis[1], delta),
                Dot(basis[2], delta),
                Dot(basis[0], delta)
            };

            if (cylinder.height == std::numeric_limits<Real>::max())
            {
                DoQueryInfiniteCylinder(P, cylinder.radius, result);
            }
            else
            {
                DoQueryFiniteCylinder(P, cylinder.radius, cylinder.height, result);
            }

            // Convert the closest point from the cylinder coordinate system
            // to the original coordinate system.
            result.cylinderClosest = cylinder.axis.origin +
                result.cylinderClosest[0] * basis[1] +
                result.cylinderClosest[1] * basis[2] +
                result.cylinderClosest[2] * basis[0];

            return result;
        }

    private:
        void DoQueryInfiniteCylinder(Vector3<Real> const& P, Real radius,
            Result& result)
        {
            Real sqrRadius = radius * radius;
            Real sqrDistance = P[0] * P[0] + P[1] * P[1];
            if (sqrDistance >= sqrRadius)
            {
                // The point is outside the cylinder or on the cylinder wall.
                Real distance = std::sqrt(sqrDistance);
                result.distance = distance - radius;
                Real temp = radius / distance;
                result.cylinderClosest[0] = P[0] * temp;
                result.cylinderClosest[1] = P[1] * temp;
                result.cylinderClosest[2] = P[2];
            }
            else
            {
                // The point is inside the cylinder.
                result.distance = (Real)0;
                result.cylinderClosest = P;
            }
        }

        void DoQueryFiniteCylinder(Vector3<Real> const& P, Real radius,
            Real height, Result& result)
        {
            DoQueryInfiniteCylinder(P, radius, result);

            // Clamp the infinite cylinder's closest point to the finite
            // cylinder.
            if (result.cylinderClosest[2] > height)
            {
                result.cylinderClosest[2] = height;
                result.distance = Length(result.cylinderClosest - P);
            }
            else if (result.cylinderClosest[2] < -height)
            {
                result.cylinderClosest[2] = -height;
                result.distance = Length(result.cylinderClosest - P);
            }
        }
    };
}
