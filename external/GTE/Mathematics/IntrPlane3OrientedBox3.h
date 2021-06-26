// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/DistPoint3Plane3.h>
#include <Mathematics/OrientedBox.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Plane3<Real>, OrientedBox3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Plane3<Real> const& plane, OrientedBox3<Real> const& box)
        {
            Result result;

            Real radius =
                std::fabs(box.extent[0] * Dot(plane.normal, box.axis[0])) +
                std::fabs(box.extent[1] * Dot(plane.normal, box.axis[1])) +
                std::fabs(box.extent[2] * Dot(plane.normal, box.axis[2]));

            DCPQuery<Real, Vector3<Real>, Plane3<Real>> ppQuery;
            auto ppResult = ppQuery(box.center, plane);
            result.intersect = (ppResult.distance <= radius);
            return result;
        }
    };
}
