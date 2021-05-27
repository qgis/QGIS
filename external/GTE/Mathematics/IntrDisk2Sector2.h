// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Sector2.h>

// The Circle2 object is considered to be a disk whose points X satisfy the
// constraint |X-C| <= R, where C is the disk center and R is the disk
// radius.  The Sector2 object is also considered to be a solid.  Also,
// the Sector2 object is required to be convex, so the sector angle must
// be in (0,pi/2], even though the Sector2 definition allows for angles
// larger than pi/2 (leading to nonconvex sectors).  The sector vertex is
// V, the radius is L, the axis direction is D, and the angle is A. Sector
// points X satisfy |X-V| <= L and Dot(D,X-V) >= cos(A)|X-V| >= 0.
//
// A subproblem for the test-intersection query is to determine whether
// the disk intersects the cone of the sector.  Although the query is in
// 2D, it is analogous to the 3D problem of determining whether a sphere
// and cone overlap.  That algorithm is described in
//   https://www.geometrictools.com/Documentation/IntersectionSphereCone.pdf
// The algorithm leads to coordinate-free pseudocode that applies to 2D
// as well as 3D.  That function is the first SphereIntersectsCone on
// page 4 of the PDF.
//
// If the disk is outside the cone, there is no intersection.  If the disk
// overlaps the cone, we then need to test whether the disk overlaps the
// disk of the sector.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Circle2<Real>, Sector2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Circle2<Real> const& disk, Sector2<Real> const& sector)
        {
            Result result;

            // Test whether the disk and the disk of the sector overlap.
            Vector2<Real> CmV = disk.center - sector.vertex;
            Real sqrLengthCmV = Dot(CmV, CmV);
            Real lengthCmV = std::sqrt(sqrLengthCmV);
            if (lengthCmV > disk.radius + sector.radius)
            {
                // The disk is outside the disk of the sector.
                result.intersect = false;
                return result;
            }

            // Test whether the disk and cone of the sector overlap.  The
            // comments about K, K', and K" refer to the PDF mentioned
            // previously.
            Vector2<Real> U = sector.vertex - (disk.radius / sector.sinAngle) * sector.direction;
            Vector2<Real> CmU = disk.center - U;
            Real lengthCmU = Length(CmU);
            if (Dot(sector.direction, CmU) < lengthCmU * sector.cosAngle)
            {
                // The disk center is outside K" (in the white or gray
                // regions).
                result.intersect = false;
                return result;
            }
            // The disk center is inside K" (in the red, orange, blue, or
            // green regions).
            Real dotDirCmV = Dot(sector.direction, CmV);
            if (-dotDirCmV >= lengthCmV * sector.sinAngle)
            {
                // The disk center is inside K" and inside K' (in the blue
                // or green regions).
                if (lengthCmV <= disk.radius)
                {
                    // The disk center is in the blue region, in which case
                    // the disk contains the sector's vertex.
                    result.intersect = true;
                }
                else
                {
                    // The disk center is in the green region.
                    result.intersect = false;
                }
                return result;
            }

            // To reach here, we know that the disk overlaps the sector's disk
            // and the sector's cone.  The disk center is in the orange region
            // or in the red region (not including the segments that separate
            // the red and blue regions).

            // Test whether the ray of the right boundary of the sector
            // overlaps the disk.  The ray direction U0 is a clockwise
            // rotation of the cone axis by the cone angle.
            Vector2<Real> U0
            {
                +sector.cosAngle * sector.direction[0] + sector.sinAngle * sector.direction[1],
                -sector.sinAngle * sector.direction[0] + sector.cosAngle * sector.direction[1]
            };
            Real dp0 = Dot(U0, CmV);
            Real discr0 = disk.radius * disk.radius + dp0 * dp0 - sqrLengthCmV;
            if (discr0 >= (Real)0)
            {
                // The ray intersects the disk.  Now test whether the sector
                // boundary segment contained by the ray overlaps the disk.
                // The quadratic root tmin generates the ray-disk point of
                // intersection closest to the sector vertex.
                Real tmin = dp0 - std::sqrt(discr0);
                if (sector.radius >= tmin)
                {
                    // The segment overlaps the disk.
                    result.intersect = true;
                    return result;
                }
                else
                {
                    // The segment does not overlap the disk.  We know the
                    // disks overlap, so if the disk center is outside the
                    // sector cone or on the right-boundary ray, the overlap
                    // occurs outside the cone, which implies the disk and
                    // sector do not intersect.
                    if (dotDirCmV <= lengthCmV * sector.cosAngle)
                    {
                        // The disk center is not inside the sector cone.
                        result.intersect = false;
                        return result;
                    }
                }
            }

            // Test whether the ray of the left boundary of the sector
            // overlaps the disk.  The ray direction U1 is a counterclockwise
            // rotation of the cone axis by the cone angle.
            Vector2<Real> U1
            {
                +sector.cosAngle * sector.direction[0] - sector.sinAngle * sector.direction[1],
                +sector.sinAngle * sector.direction[0] + sector.cosAngle * sector.direction[1]
            };
            Real dp1 = Dot(U1, CmV);
            Real discr1 = disk.radius * disk.radius + dp1 * dp1 - sqrLengthCmV;
            if (discr1 >= (Real)0)
            {
                // The ray intersects the disk.  Now test whether the sector
                // boundary segment contained by the ray overlaps the disk.
                // The quadratic root tmin generates the ray-disk point of
                // intersection closest to the sector vertex.
                Real tmin = dp1 - std::sqrt(discr1);
                if (sector.radius >= tmin)
                {
                    result.intersect = true;
                    return result;
                }
                else
                {
                    // The segment does not overlap the disk.  We know the
                    // disks overlap, so if the disk center is outside the
                    // sector cone or on the right-boundary ray, the overlap
                    // occurs outside the cone, which implies the disk and
                    // sector do not intersect.
                    if (dotDirCmV <= lengthCmV * sector.cosAngle)
                    {
                        // The disk center is not inside the sector cone.
                        result.intersect = false;
                        return result;
                    }
                }
            }

            // To reach here, a strict subset of the sector's arc boundary
            // must intersect the disk.
            result.intersect = true;
            return result;
        }
    };
}
