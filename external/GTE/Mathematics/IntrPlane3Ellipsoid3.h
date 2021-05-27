// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/DistPoint3Plane3.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Matrix3x3.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Plane3<Real>, Ellipsoid3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Plane3<Real> const& plane, Ellipsoid3<Real> const& ellipsoid)
        {
            Result result;
            Matrix3x3<Real> MInverse;
            ellipsoid.GetMInverse(MInverse);
            Real discr = Dot(plane.normal, MInverse * plane.normal);
            Real root = std::sqrt(std::max(discr, (Real)0));
            DCPQuery<Real, Vector3<Real>, Plane3<Real>> vpQuery;
            Real distance = vpQuery(ellipsoid.center, plane).distance;
            result.intersect = (distance <= root);
            return result;
        }
    };
}
