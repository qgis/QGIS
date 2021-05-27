// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistLine3AlignedBox3.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Ray3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real rayParameter;
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Ray3<Real> const& ray, AlignedBox3<Real> const& box)
        {
            Result result;

            Line3<Real> line(ray.origin, ray.direction);
            DCPQuery<Real, Line3<Real>, AlignedBox3<Real>> lbQuery;
            auto lbResult = lbQuery(line, box);

            if (lbResult.lineParameter >= (Real)0)
            {
                result.sqrDistance = lbResult.sqrDistance;
                result.distance = lbResult.distance;
                result.rayParameter = lbResult.lineParameter;
                result.closestPoint[0] = lbResult.closestPoint[0];
                result.closestPoint[1] = lbResult.closestPoint[1];
            }
            else
            {
                DCPQuery<Real, Vector3<Real>, AlignedBox3<Real>> pbQuery;
                auto pbResult = pbQuery(ray.origin, box);
                result.sqrDistance = pbResult.sqrDistance;
                result.distance = pbResult.distance;
                result.rayParameter = (Real)0;
                result.closestPoint[0] = ray.origin;
                result.closestPoint[1] = pbResult.boxClosest;
            }
            return result;
        }
    };
}
