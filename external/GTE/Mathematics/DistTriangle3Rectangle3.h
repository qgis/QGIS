// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistSegment3Rectangle3.h>
#include <Mathematics/DistSegment3Triangle3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Triangle3<Real>, Rectangle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real triangleParameter[3], rectangleParameter[2];
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Triangle3<Real> const& triangle, Rectangle3<Real> const& rectangle)
        {
            Result result;

            result.sqrDistance = std::numeric_limits<Real>::max();

            // Compare edges of triangle to the interior of rectangle.
            for (int i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                Vector3<Real> segCenter = (Real)0.5 * (triangle.v[i0] + triangle.v[i1]);
                Vector3<Real> segDirection = triangle.v[i1] - triangle.v[i0];
                Real segExtent = (Real)0.5 * Normalize(segDirection);
                Segment3<Real> edge(segCenter, segDirection, segExtent);

                DCPQuery<Real, Segment3<Real>, Rectangle3<Real>> srQuery;
                auto srResult = srQuery(edge, rectangle);
                if (srResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = srResult.distance;
                    result.sqrDistance = srResult.sqrDistance;
                    // ratio is in [-1,1]
                    Real ratio = srResult.segmentParameter / segExtent;
                    result.triangleParameter[i0] = (Real)0.5 * ((Real)1 - ratio);
                    result.triangleParameter[i1] = (Real)1 - result.triangleParameter[i0];
                    result.triangleParameter[3 - i0 - i1] = (Real)0;
                    result.rectangleParameter[0] = srResult.rectangleParameter[0];
                    result.rectangleParameter[1] = srResult.rectangleParameter[1];
                    result.closestPoint[0] = srResult.closestPoint[0];
                    result.closestPoint[1] = srResult.closestPoint[1];
                }
            }

            // Compare edges of rectangle to the interior of triangle.
            for (int i1 = 0; i1 < 2; ++i1)
            {
                for (int i0 = -1; i0 <= 1; i0 += 2)
                {
                    Real s = i0 * rectangle.extent[1 - i1];
                    Vector3<Real> segCenter = rectangle.center + s * rectangle.axis[1 - i1];
                    Segment3<Real> edge(segCenter, rectangle.axis[i1], rectangle.extent[i1]);

                    DCPQuery<Real, Segment3<Real>, Triangle3<Real>> stQuery;
                    auto stResult = stQuery(edge, triangle);
                    if (stResult.sqrDistance < result.sqrDistance)
                    {
                        result.distance = stResult.distance;
                        result.sqrDistance = stResult.sqrDistance;
                        result.triangleParameter[0] = stResult.triangleParameter[0];
                        result.triangleParameter[1] = stResult.triangleParameter[1];
                        result.triangleParameter[2] = stResult.triangleParameter[2];
                        result.rectangleParameter[i1] = s;
                        result.rectangleParameter[1 - i1] = stResult.segmentParameter;
                        result.closestPoint[0] = stResult.closestPoint[1];
                        result.closestPoint[1] = stResult.closestPoint[0];
                    }
                }
            }
            return result;
        }
    };
}
