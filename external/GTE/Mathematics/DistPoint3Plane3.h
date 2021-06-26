// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Vector3<Real>, Plane3<Real>>
    {
    public:
        struct Result
        {
            Real distance, signedDistance;
            Vector3<Real> planeClosestPoint;
        };

        Result operator()(Vector3<Real> const& point, Plane3<Real> const& plane)
        {
            Result result;
            result.signedDistance = Dot(plane.normal, point) - plane.constant;
            result.distance = std::fabs(result.signedDistance);
            result.planeClosestPoint = point - result.signedDistance * plane.normal;
            return result;
        }
    };
}
