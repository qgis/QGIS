// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistSegment3Triangle3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Triangle3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real triangle0Parameter[3], triangle1Parameter[3];
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Triangle3<Real> const& triangle0, Triangle3<Real> const& triangle1)
        {
            Result result;

            DCPQuery<Real, Segment3<Real>, Triangle3<Real>> stQuery;
            typename DCPQuery<Real, Segment3<Real>, Triangle3<Real>>::Result
                stResult;
            result.sqrDistance = std::numeric_limits<Real>::max();

            // Compare edges of triangle0 to the interior of triangle1.
            for (int i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                Vector3<Real> segCenter = (Real)0.5 * (triangle0.v[i0] + triangle0.v[i1]);
                Vector3<Real> segDirection = triangle0.v[i1] - triangle0.v[i0];
                Real segExtent = (Real)0.5 * Normalize(segDirection);
                Segment3<Real> edge(segCenter, segDirection, segExtent);

                stResult = stQuery(edge, triangle1);
                if (stResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = stResult.distance;
                    result.sqrDistance = stResult.sqrDistance;
                    // ratio is in [-1,1]
                    Real ratio = stResult.segmentParameter / segExtent;
                    result.triangle0Parameter[i0] = (Real)0.5 * ((Real)1 - ratio);
                    result.triangle0Parameter[i1] = (Real)1 - result.triangle0Parameter[i0];
                    result.triangle0Parameter[3 - i0 - i1] = (Real)0;
                    result.triangle1Parameter[0] = stResult.triangleParameter[0];
                    result.triangle1Parameter[1] = stResult.triangleParameter[1];
                    result.triangle1Parameter[2] = stResult.triangleParameter[2];
                    result.closestPoint[0] = stResult.closestPoint[0];
                    result.closestPoint[1] = stResult.closestPoint[1];
                }
            }

            // Compare edges of triangle1 to the interior of triangle0.
            for (int i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                Vector3<Real> segCenter = (Real)0.5 * (triangle1.v[i0] + triangle1.v[i1]);
                Vector3<Real> segDirection = triangle1.v[i1] - triangle1.v[i0];
                Real segExtent = (Real)0.5 * Normalize(segDirection);
                Segment3<Real> edge(segCenter, segDirection, segExtent);

                stResult = stQuery(edge, triangle0);
                if (stResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = stResult.distance;
                    result.sqrDistance = stResult.sqrDistance;
                    Real ratio = stResult.segmentParameter / segExtent;  // in [-1,1]
                    result.triangle0Parameter[0] = stResult.triangleParameter[0];
                    result.triangle0Parameter[1] = stResult.triangleParameter[1];
                    result.triangle0Parameter[2] = stResult.triangleParameter[2];
                    result.triangle1Parameter[i0] = (Real)0.5 * ((Real)1 - ratio);
                    result.triangle1Parameter[i1] = (Real)1 - result.triangle0Parameter[i0];
                    result.triangle1Parameter[3 - i0 - i1] = (Real)0;
                    result.closestPoint[0] = stResult.closestPoint[0];
                    result.closestPoint[1] = stResult.closestPoint[1];
                }
            }
            return result;
        }
    };
}
