// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistLine3Rectangle3.h>
#include <Mathematics/DistPoint3Rectangle3.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Segment3<Real>, Rectangle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real segmentParameter, rectangleParameter[2];
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Segment3<Real> const& segment, Rectangle3<Real> const& rectangle)
        {
            Result result;

            Vector3<Real> segCenter, segDirection;
            Real segExtent;
            segment.GetCenteredForm(segCenter, segDirection, segExtent);

            Line3<Real> line(segCenter, segDirection);
            DCPQuery<Real, Line3<Real>, Rectangle3<Real>> lrQuery;
            auto lrResult = lrQuery(line, rectangle);

            if (lrResult.lineParameter >= -segExtent)
            {
                if (lrResult.lineParameter <= segExtent)
                {
                    result.distance = lrResult.distance;
                    result.sqrDistance = lrResult.sqrDistance;
                    result.segmentParameter = lrResult.lineParameter;
                    result.rectangleParameter[0] = lrResult.rectangleParameter[0];
                    result.rectangleParameter[1] = lrResult.rectangleParameter[1];
                    result.closestPoint[0] = lrResult.closestPoint[0];
                    result.closestPoint[1] = lrResult.closestPoint[1];
                }
                else
                {
                    DCPQuery<Real, Vector3<Real>, Rectangle3<Real>> prQuery;
                    Vector3<Real> point = segCenter + segExtent * segDirection;
                    auto prResult = prQuery(point, rectangle);
                    result.sqrDistance = prResult.sqrDistance;
                    result.distance = prResult.distance;
                    result.segmentParameter = segExtent;
                    result.closestPoint[0] = point;
                    result.closestPoint[1] = prResult.rectangleClosestPoint;
                }
            }
            else
            {
                DCPQuery<Real, Vector3<Real>, Rectangle3<Real>> prQuery;
                Vector3<Real> point = segCenter - segExtent * segDirection;
                auto prResult = prQuery(point, rectangle);
                result.sqrDistance = prResult.sqrDistance;
                result.distance = prResult.distance;
                result.segmentParameter = segExtent;
                result.closestPoint[0] = point;
                result.closestPoint[1] = prResult.rectangleClosestPoint;
            }
            return result;
        }
    };
}
