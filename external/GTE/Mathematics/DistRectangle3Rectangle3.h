// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistSegment3Rectangle3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Rectangle3<Real>, Rectangle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real rectangle0Parameter[2], rectangle1Parameter[2];
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Rectangle3<Real> const& rectangle0, Rectangle3<Real> const& rectangle1)
        {
            Result result;

            DCPQuery<Real, Segment3<Real>, Rectangle3<Real>> srQuery;
            typename DCPQuery<Real, Segment3<Real>, Rectangle3<Real>>::Result srResult;
            result.sqrDistance = std::numeric_limits<Real>::max();

            // Compare edges of rectangle0 to the interior of rectangle1.
            for (int i1 = 0; i1 < 2; ++i1)
            {
                for (int i0 = -1; i0 <= 1; i0 += 2)
                {
                    Real s = i0 * rectangle0.extent[1 - i1];
                    Vector3<Real> segCenter = rectangle0.center +
                        s * rectangle0.axis[1 - i1];
                    Segment3<Real> edge(segCenter, rectangle0.axis[i1],
                        rectangle0.extent[i1]);

                    srResult = srQuery(edge, rectangle1);
                    if (srResult.sqrDistance < result.sqrDistance)
                    {
                        result.distance = srResult.distance;
                        result.sqrDistance = srResult.sqrDistance;
                        result.rectangle0Parameter[i1] = s;
                        result.rectangle0Parameter[1 - i1] =
                            srResult.segmentParameter;
                        result.rectangle1Parameter[0] =
                            srResult.rectangleParameter[0];
                        result.rectangle1Parameter[1] =
                            srResult.rectangleParameter[1];
                        result.closestPoint[0] = srResult.closestPoint[0];
                        result.closestPoint[1] = srResult.closestPoint[1];
                    }
                }
            }

            // Compare edges of rectangle1 to the interior of rectangle0.
            for (int i1 = 0; i1 < 2; ++i1)
            {
                for (int i0 = -1; i0 <= 1; i0 += 2)
                {
                    Real s = i0 * rectangle1.extent[1 - i1];
                    Vector3<Real> segCenter = rectangle1.center +
                        s * rectangle1.axis[1 - i1];
                    Segment3<Real> edge(segCenter, rectangle0.axis[i1],
                        rectangle0.extent[i1]);

                    srResult = srQuery(edge, rectangle0);
                    if (srResult.sqrDistance < result.sqrDistance)
                    {
                        result.distance = srResult.distance;
                        result.sqrDistance = srResult.sqrDistance;
                        result.rectangle0Parameter[0] =
                            srResult.rectangleParameter[0];
                        result.rectangle0Parameter[1] =
                            srResult.rectangleParameter[1];
                        result.rectangle1Parameter[i1] = s;
                        result.rectangle1Parameter[1 - i1] =
                            srResult.segmentParameter;
                        result.closestPoint[0] = srResult.closestPoint[1];
                        result.closestPoint[1] = srResult.closestPoint[0];
                    }
                }
            }
            return result;
        }
    };
}
