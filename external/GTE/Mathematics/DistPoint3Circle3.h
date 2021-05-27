// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>

// The 3D point-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Vector3<Real>, Circle3<Real>>
    {
    public:
        // Either a single point on the circle is closest to 'point', in
        // which case 'equidistant' is false, or the entire circle is
        // closest to 'point', in which case 'equidistant' is true.  In the
        // latter case, the query returns the circle point C+r*U, where C is
        // the circle center, r is the circle radius, and U is a vector
        // perpendicular to the normal N for the plane of the circle.
        struct Result
        {
            Real distance, sqrDistance;
            Vector3<Real> circleClosest;
            bool equidistant;
        };

        Result operator()(Vector3<Real> const& point, Circle3<Real> const& circle)
        {
            Result result;

            // Projection of P-C onto plane is Q-C = P-C - Dot(N,P-C)*N.
            Vector3<Real> PmC = point - circle.center;
            Vector3<Real> QmC = PmC - Dot(circle.normal, PmC) * circle.normal;
            Real lengthQmC = Length(QmC);
            if (lengthQmC > (Real)0)
            {
                result.circleClosest = circle.center + (circle.radius / lengthQmC) * QmC;
                result.equidistant = false;
            }
            else
            {
                // All circle points are equidistant from P.  Return one of
                // them.
                Vector3<Real> basis[3];
                basis[0] = circle.normal;
                ComputeOrthogonalComplement(1, basis);
                result.circleClosest = circle.center + circle.radius * basis[1];
                result.equidistant = true;
            }

            Vector3<Real> diff = point - result.circleClosest;
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };
}
