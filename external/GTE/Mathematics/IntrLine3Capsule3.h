// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/DistLineSegment.h>
#include <Mathematics/Capsule.h>
#include <Mathematics/Vector3.h>

// The queries consider the capsule to be a solid.
//
// The test-intersection queries are based on distance computations.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line3<Real>, Capsule3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Line3<Real> const& line, Capsule3<Real> const& capsule)
        {
            Result result;
            DCPQuery<Real, Line3<Real>, Segment3<Real>> lsQuery;
            auto lsResult = lsQuery(line, capsule.segment);
            result.intersect = (lsResult.distance <= capsule.radius);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line3<Real>, Capsule3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector3<Real>, 2> point;
        };

        Result operator()(Line3<Real> const& line, Capsule3<Real> const& capsule)
        {
            Result result;
            DoQuery(line.origin, line.direction, capsule, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& lineOrigin,
            Vector3<Real> const& lineDirection, Capsule3<Real> const& capsule,
            Result& result)
        {
            // Initialize the result as if there is no intersection.  If we
            // discover an intersection, these values will be modified
            // accordingly.
            result.intersect = false;
            result.numIntersections = 0;

            // Create a coordinate system for the capsule.  In this system,
            // the capsule segment center C is the origin and the capsule axis
            // direction W is the z-axis.  U and V are the other coordinate
            // axis directions.  If P = x*U+y*V+z*W, the cylinder containing
            // the capsule wall is x^2 + y^2 = r^2, where r is the capsule
            // radius.  The finite cylinder that makes up the capsule minus
            // its hemispherical end caps has z-values |z| <= e, where e is
            // the extent of the capsule segment.  The top hemisphere cap is
            // x^2+y^2+(z-e)^2 = r^2 for z >= e, and the bottom hemisphere cap
            // is x^2+y^2+(z+e)^2 = r^2 for z <= -e.

            Vector3<Real> segOrigin, segDirection;
            Real segExtent;
            capsule.segment.GetCenteredForm(segOrigin, segDirection, segExtent);
            Vector3<Real> basis[3];  // {W, U, V}
            basis[0] = segDirection;
            ComputeOrthogonalComplement(1, basis);
            Real rSqr = capsule.radius * capsule.radius;

            // Convert incoming line origin to capsule coordinates.
            Vector3<Real> diff = lineOrigin - segOrigin;
            Vector3<Real> P{ Dot(basis[1], diff), Dot(basis[2], diff), Dot(basis[0], diff) };

            // Get the z-value, in capsule coordinates, of the incoming line's
            // unit-length direction.
            Real dz = Dot(basis[0], lineDirection);
            if (std::fabs(dz) == (Real)1)
            {
                // The line is parallel to the capsule axis.  Determine
                // whether the line intersects the capsule hemispheres.
                Real radialSqrDist = rSqr - P[0] * P[0] - P[1] * P[1];
                if (radialSqrDist >= (Real)0)
                {
                    // The line intersects the hemispherical caps.
                    result.intersect = true;
                    result.numIntersections = 2;
                    Real zOffset = std::sqrt(radialSqrDist) + segExtent;
                    if (dz > (Real)0)
                    {
                        result.parameter[0] = -P[2] - zOffset;
                        result.parameter[1] = -P[2] + zOffset;
                    }
                    else
                    {
                        result.parameter[0] = P[2] - zOffset;
                        result.parameter[1] = P[2] + zOffset;
                    }
                }
                // else: The line outside the capsule's cylinder, no
                // intersection.
                return;
            }

            // Convert the incoming line unit-length direction to capsule
            // coordinates.
            Vector3<Real> D{ Dot(basis[1], lineDirection), Dot(basis[2], lineDirection), dz };

            // Test intersection of line P+t*D with infinite cylinder
            // x^2+y^2 = r^2.  This reduces to computing the roots of a
            // quadratic equation.  If P = (px,py,pz) and D = (dx,dy,dz), then
            // the quadratic equation is
            //   (dx^2+dy^2)*t^2 + 2*(px*dx+py*dy)*t + (px^2+py^2-r^2) = 0
            Real a0 = P[0] * P[0] + P[1] * P[1] - rSqr;
            Real a1 = P[0] * D[0] + P[1] * D[1];
            Real a2 = D[0] * D[0] + D[1] * D[1];
            Real discr = a1 * a1 - a0 * a2;
            if (discr < (Real)0)
            {
                // The line does not intersect the infinite cylinder, so it
                // cannot intersect the capsule.
                return;
            }

            Real root, inv, tValue, zValue;
            if (discr > (Real)0)
            {
                // The line intersects the infinite cylinder in two places.
                root = std::sqrt(discr);
                inv = (Real)1 / a2;
                tValue = (-a1 - root) * inv;
                zValue = P[2] + tValue * D[2];
                if (std::fabs(zValue) <= segExtent)
                {
                    result.intersect = true;
                    result.parameter[result.numIntersections++] = tValue;
                }

                tValue = (-a1 + root) * inv;
                zValue = P[2] + tValue * D[2];
                if (std::fabs(zValue) <= segExtent)
                {
                    result.intersect = true;
                    result.parameter[result.numIntersections++] = tValue;
                }

                if (result.numIntersections == 2)
                {
                    // The line intersects the capsule wall in two places.
                    return;
                }
            }
            else
            {
                // The line is tangent to the infinite cylinder but intersects
                // the cylinder in a single point.
                tValue = -a1 / a2;
                zValue = P[2] + tValue * D[2];
                if (std::fabs(zValue) <= segExtent)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.parameter[0] = tValue;
                    // Used by derived classes.
                    result.parameter[1] = result.parameter[0];
                    return;
                }
            }

            // Test intersection with bottom hemisphere.  The quadratic
            // equation is
            //   t^2 + 2*(px*dx+py*dy+(pz+e)*dz)*t
            //     + (px^2+py^2+(pz+e)^2-r^2) = 0
            // Use the fact that currently a1 = px*dx+py*dy and
            // a0 = px^2+py^2-r^2.  The leading coefficient is a2 = 1, so no
            // need to include in the construction.
            Real PZpE = P[2] + segExtent;
            a1 += PZpE * D[2];
            a0 += PZpE * PZpE;
            discr = a1 * a1 - a0;
            if (discr > (Real)0)
            {
                root = std::sqrt(discr);
                tValue = -a1 - root;
                zValue = P[2] + tValue * D[2];
                if (zValue <= -segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }

                tValue = -a1 + root;
                zValue = P[2] + tValue * D[2];
                if (zValue <= -segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }
            else if (discr == (Real)0)
            {
                tValue = -a1;
                zValue = P[2] + tValue * D[2];
                if (zValue <= -segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }

            // Test intersection with top hemisphere.  The quadratic equation
            // is
            //   t^2 + 2*(px*dx+py*dy+(pz-e)*dz)*t
            //     + (px^2+py^2+(pz-e)^2-r^2) = 0
            // Use the fact that currently a1 = px*dx+py*dy+(pz+e)*dz and
            // a0 = px^2+py^2+(pz+e)^2-r^2. The leading coefficient is a2 = 1,
            // so no need to include in the construction.
            a1 -= ((Real)2) * segExtent * D[2];
            a0 -= ((Real)4) * segExtent * P[2];
            discr = a1 * a1 - a0;
            if (discr > (Real)0)
            {
                root = std::sqrt(discr);
                tValue = -a1 - root;
                zValue = P[2] + tValue * D[2];
                if (zValue >= segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }

                tValue = -a1 + root;
                zValue = P[2] + tValue * D[2];
                if (zValue >= segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }
            else if (discr == (Real)0)
            {
                tValue = -a1;
                zValue = P[2] + tValue * D[2];
                if (zValue >= segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }

            if (result.numIntersections == 1)
            {
                // Used by derived classes.
                result.parameter[1] = result.parameter[0];
            }
        }
    };
}
