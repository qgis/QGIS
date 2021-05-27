// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistPointTriangle.h>
#include <Mathematics/Tetrahedron3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Vector3<Real>, Tetrahedron3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Vector3<Real> tetrahedronClosestPoint;
        };

        Result operator()(Vector3<Real> const& point, Tetrahedron3<Real> const& tetrahedron)
        {
            Result result;

            // Construct the planes for the faces of the tetrahedron.  The
            // normals are outer pointing, but specified not to be unit
            // length.  We only need to know sidedness of the query point,
            // so we will save cycles by not computing unit-length normals.
            Plane3<Real> planes[4];
            tetrahedron.GetPlanes(planes);

            // Determine which faces are visible to the query point.  Only
            // these need to be processed by point-to-triangle distance
            // queries.
            result.sqrDistance = std::numeric_limits<Real>::max();
            result.tetrahedronClosestPoint = Vector3<Real>::Zero();
            for (int i = 0; i < 4; ++i)
            {
                if (Dot(planes[i].normal, point) >= planes[i].constant)
                {
                    int indices[3] = { 0, 0, 0 };
                    tetrahedron.GetFaceIndices(i, indices);
                    Triangle3<Real> triangle(
                        tetrahedron.v[indices[0]],
                        tetrahedron.v[indices[1]],
                        tetrahedron.v[indices[2]]);

                    DCPQuery<Real, Vector3<Real>, Triangle3<Real>> query;
                    auto ptResult = query(point, triangle);
                    if (ptResult.sqrDistance < result.sqrDistance)
                    {
                        result.sqrDistance = ptResult.sqrDistance;
                        result.tetrahedronClosestPoint = ptResult.closest;
                    }
                }
            }

            if (result.sqrDistance == std::numeric_limits<Real>::max())
            {
                // The query point is inside the solid tetrahedron.  Report a
                // zero distance.  The closest points are identical.
                result.sqrDistance = (Real)0;
                result.tetrahedronClosestPoint = point;
            }
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };
}
