// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistLine3Rectangle3.h>
#include <Mathematics/DistPoint3Rectangle3.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Ray3<Real>, Rectangle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real rayParameter, rectangleParameter[2];
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Ray3<Real> const& ray, Rectangle3<Real> const& rectangle)
        {
            Result result;

            Line3<Real> line(ray.origin, ray.direction);
            DCPQuery<Real, Line3<Real>, Rectangle3<Real>> lrQuery;
            auto lrResult = lrQuery(line, rectangle);

            if (lrResult.lineParameter >= (Real)0)
            {
                result.distance = lrResult.distance;
                result.sqrDistance = lrResult.sqrDistance;
                result.rayParameter = lrResult.lineParameter;
                result.rectangleParameter[0] = lrResult.rectangleParameter[0];
                result.rectangleParameter[1] = lrResult.rectangleParameter[1];
                result.closestPoint[0] = lrResult.closestPoint[0];
                result.closestPoint[1] = lrResult.closestPoint[1];
            }
            else
            {
                DCPQuery<Real, Vector3<Real>, Rectangle3<Real>> prQuery;
                auto prResult = prQuery(ray.origin, rectangle);
                result.distance = prResult.distance;
                result.sqrDistance = prResult.sqrDistance;
                result.rayParameter = (Real)0;
                result.rectangleParameter[0] = prResult.rectangleParameter[0];
                result.rectangleParameter[1] = prResult.rectangleParameter[1];
                result.closestPoint[0] = ray.origin;
                result.closestPoint[1] = prResult.rectangleClosestPoint;
            }
            return result;
        }
    };
}
