// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistLineSegment.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Line3<Real>, Triangle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real lineParameter, triangleParameter[3];
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Line3<Real> const& line, Triangle3<Real> const& triangle)
        {
            Result result;

            // Test if line intersects triangle.  If so, the squared distance
            // is zero.
            Vector3<Real> edge0 = triangle.v[1] - triangle.v[0];
            Vector3<Real> edge1 = triangle.v[2] - triangle.v[0];
            Vector3<Real> normal = UnitCross(edge0, edge1);
            Real NdD = Dot(normal, line.direction);
            if (std::fabs(NdD) > (Real)0)
            {
                // The line and triangle are not parallel, so the line
                // intersects/ the plane of the triangle.
                Vector3<Real> diff = line.origin - triangle.v[0];
                Vector3<Real> basis[3];  // {D, U, V}
                basis[0] = line.direction;
                ComputeOrthogonalComplement<Real>(1, basis);
                Real UdE0 = Dot(basis[1], edge0);
                Real UdE1 = Dot(basis[1], edge1);
                Real UdDiff = Dot(basis[1], diff);
                Real VdE0 = Dot(basis[2], edge0);
                Real VdE1 = Dot(basis[2], edge1);
                Real VdDiff = Dot(basis[2], diff);
                Real invDet = ((Real)1) / (UdE0 * VdE1 - UdE1 * VdE0);

                // Barycentric coordinates for the point of intersection.
                Real b1 = (VdE1 * UdDiff - UdE1 * VdDiff) * invDet;
                Real b2 = (UdE0 * VdDiff - VdE0 * UdDiff) * invDet;
                Real b0 = (Real)1 - b1 - b2;

                if (b0 >= (Real)0 && b1 >= (Real)0 && b2 >= (Real)0)
                {
                    // Line parameter for the point of intersection.
                    Real DdE0 = Dot(line.direction, edge0);
                    Real DdE1 = Dot(line.direction, edge1);
                    Real DdDiff = Dot(line.direction, diff);
                    result.lineParameter = b1 * DdE0 + b2 * DdE1 - DdDiff;

                    // Barycentric coordinates for the point of intersection.
                    result.triangleParameter[0] = b0;
                    result.triangleParameter[1] = b1;
                    result.triangleParameter[2] = b2;

                    // The intersection point is inside or on the triangle.
                    result.closestPoint[0] = line.origin + result.lineParameter * line.direction;
                    result.closestPoint[1] = triangle.v[0] + b1 * edge0 + b2 * edge1;

                    result.distance = (Real)0;
                    result.sqrDistance = (Real)0;
                    return result;
                }
            }

            // Either (1) the line is not parallel to the triangle and the
            // point of intersection of the line and the plane of the triangle
            // is outside the triangle or (2) the line and triangle are
            // parallel.  Regardless, the closest point on the triangle is on
            // an edge of the triangle.  Compare the line to all three edges
            // of the triangle.
            result.distance = std::numeric_limits<Real>::max();
            result.sqrDistance = std::numeric_limits<Real>::max();
            for (int i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                Vector3<Real> segCenter = (Real)0.5 * (triangle.v[i0] + triangle.v[i1]);
                Vector3<Real> segDirection = triangle.v[i1] - triangle.v[i0];
                Real segExtent = (Real)0.5 * Normalize(segDirection);
                Segment3<Real> segment(segCenter, segDirection, segExtent);

                DCPQuery<Real, Line3<Real>, Segment3<Real>> query;
                auto lsResult = query(line, segment);
                if (lsResult.sqrDistance < result.sqrDistance)
                {
                    result.sqrDistance = lsResult.sqrDistance;
                    result.distance = lsResult.distance;
                    result.lineParameter = lsResult.parameter[0];
                    result.triangleParameter[i0] = (Real)0.5 * ((Real)1 -
                        lsResult.parameter[0] / segExtent);
                    result.triangleParameter[i1] = (Real)1 -
                        result.triangleParameter[i0];
                    result.triangleParameter[3 - i0 - i1] = (Real)0;
                    result.closestPoint[0] = lsResult.closestPoint[0];
                    result.closestPoint[1] = lsResult.closestPoint[1];
                }
            }

            return result;
        }
    };
}
