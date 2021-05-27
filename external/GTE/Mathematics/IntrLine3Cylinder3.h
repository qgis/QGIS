// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Vector3.h>

// The queries consider the cylinder to be a solid.

namespace gte
{
    template <typename Real>
    class FIQuery<Real, Line3<Real>, Cylinder3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector3<Real>, 2> point;
        };

        Result operator()(Line3<Real> const& line, Cylinder3<Real> const& cylinder)
        {
            Result result;
            DoQuery(line.origin, line.direction, cylinder, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& lineOrigin,
            Vector3<Real> const& lineDirection, Cylinder3<Real> const& cylinder,
            Result& result)
        {
            // Initialize the result as if there is no intersection.  If we
            // discover an intersection, these values will be modified
            // accordingly.
            result.intersect = false;
            result.numIntersections = 0;

            // Create a coordinate system for the cylinder.  In this system,
            // the cylinder segment center C is the origin and the cylinder
            // axis direction W is the z-axis.  U and V are the other
            // coordinate axis directions.  If P = x*U+y*V+z*W, the cylinder
            // is x^2 + y^2 = r^2, where r is the cylinder radius.  The end
            // caps are |z| = h/2, where h is the cylinder height.
            Vector3<Real> basis[3];  // {W, U, V}
            basis[0] = cylinder.axis.direction;
            ComputeOrthogonalComplement(1, basis);
            Real halfHeight = (Real)0.5 * cylinder.height;
            Real rSqr = cylinder.radius * cylinder.radius;

            // Convert incoming line origin to capsule coordinates.
            Vector3<Real> diff = lineOrigin - cylinder.axis.origin;
            Vector3<Real> P{ Dot(basis[1], diff), Dot(basis[2], diff), Dot(basis[0], diff) };

            // Get the z-value, in cylinder coordinates, of the incoming
            // line's unit-length direction.
            Real dz = Dot(basis[0], lineDirection);
            if (std::fabs(dz) == (Real)1)
            {
                // The line is parallel to the cylinder axis.  Determine
                // whether the line intersects the cylinder end disks.
                Real radialSqrDist = rSqr - P[0] * P[0] - P[1] * P[1];
                if (radialSqrDist >= (Real)0)
                {
                    // The line intersects the cylinder end disks.
                    result.intersect = true;
                    result.numIntersections = 2;
                    if (dz > (Real)0)
                    {
                        result.parameter[0] = -P[2] - halfHeight;
                        result.parameter[1] = -P[2] + halfHeight;
                    }
                    else
                    {
                        result.parameter[0] = P[2] - halfHeight;
                        result.parameter[1] = P[2] + halfHeight;
                    }
                }
                // else:  The line is outside the cylinder, no intersection.
                return;
            }

            // Convert the incoming line unit-length direction to cylinder
            // coordinates.
            Vector3<Real> D{ Dot(basis[1], lineDirection), Dot(basis[2], lineDirection), dz };

            Real a0, a1, a2, discr, root, inv, tValue;

            if (D[2] == (Real)0)
            {
                // The line is perpendicular to the cylinder axis.
                if (std::fabs(P[2]) <= halfHeight)
                {
                    // Test intersection of line P+t*D with infinite cylinder
                    // x^2+y^2 = r^2.  This reduces to computing the roots of
                    // a quadratic equation.  If P = (px,py,pz) and
                    // D = (dx,dy,dz), then the quadratic equation is
                    //   (dx^2+dy^2)*t^2 + 2*(px*dx+py*dy)*t
                    //     + (px^2+py^2-r^2) = 0
                    a0 = P[0] * P[0] + P[1] * P[1] - rSqr;
                    a1 = P[0] * D[0] + P[1] * D[1];
                    a2 = D[0] * D[0] + D[1] * D[1];
                    discr = a1 * a1 - a0 * a2;
                    if (discr > (Real)0)
                    {
                        // The line intersects the cylinder in two places.
                        result.intersect = true;
                        result.numIntersections = 2;
                        root = std::sqrt(discr);
                        inv = ((Real)1) / a2;
                        result.parameter[0] = (-a1 - root) * inv;
                        result.parameter[1] = (-a1 + root) * inv;
                    }
                    else if (discr == (Real)0)
                    {
                        // The line is tangent to the cylinder.
                        result.intersect = true;
                        result.numIntersections = 1;
                        result.parameter[0] = -a1 / a2;
                        // Used by derived classes.
                        result.parameter[1] = result.parameter[0];
                    }
                    // else: The line does not intersect the cylinder.
                }
                // else: The line is outside the planes of the cylinder end
                // disks.
                return;
            }

            // Test for intersections with the planes of the end disks.
            inv = (Real)1 / D[2];

            Real t0 = (-halfHeight - P[2]) * inv;
            Real xTmp = P[0] + t0 * D[0];
            Real yTmp = P[1] + t0 * D[1];
            if (xTmp * xTmp + yTmp * yTmp <= rSqr)
            {
                // Plane intersection inside the top cylinder end disk.
                result.parameter[result.numIntersections++] = t0;
            }

            Real t1 = (+halfHeight - P[2]) * inv;
            xTmp = P[0] + t1 * D[0];
            yTmp = P[1] + t1 * D[1];
            if (xTmp * xTmp + yTmp * yTmp <= rSqr)
            {
                // Plane intersection inside the bottom cylinder end disk.
                result.parameter[result.numIntersections++] = t1;
            }

            if (result.numIntersections < 2)
            {
                // Test for intersection with the cylinder wall.
                a0 = P[0] * P[0] + P[1] * P[1] - rSqr;
                a1 = P[0] * D[0] + P[1] * D[1];
                a2 = D[0] * D[0] + D[1] * D[1];
                discr = a1 * a1 - a0 * a2;
                if (discr > (Real)0)
                {
                    root = std::sqrt(discr);
                    inv = (Real)1 / a2;
                    tValue = (-a1 - root) * inv;
                    if (t0 <= t1)
                    {
                        if (t0 <= tValue && tValue <= t1)
                        {
                            result.parameter[result.numIntersections++] = tValue;
                        }
                    }
                    else
                    {
                        if (t1 <= tValue && tValue <= t0)
                        {
                            result.parameter[result.numIntersections++] = tValue;
                        }
                    }

                    if (result.numIntersections < 2)
                    {
                        tValue = (-a1 + root) * inv;
                        if (t0 <= t1)
                        {
                            if (t0 <= tValue && tValue <= t1)
                            {
                                result.parameter[result.numIntersections++] = tValue;
                            }
                        }
                        else
                        {
                            if (t1 <= tValue && tValue <= t0)
                            {
                                result.parameter[result.numIntersections++] = tValue;
                            }
                        }
                    }
                    // else: Line intersects end disk and cylinder wall.
                }
                else if (discr == (Real)0)
                {
                    tValue = -a1 / a2;
                    if (t0 <= t1)
                    {
                        if (t0 <= tValue && tValue <= t1)
                        {
                            result.parameter[result.numIntersections++] = tValue;
                        }
                    }
                    else
                    {
                        if (t1 <= tValue && tValue <= t0)
                        {
                            result.parameter[result.numIntersections++] = tValue;
                        }
                    }
                }
                // else: Line does not intersect cylinder wall.
            }
            // else: Line intersects both top and bottom cylinder end disks.

            if (result.numIntersections == 2)
            {
                result.intersect = true;
                if (result.parameter[0] > result.parameter[1])
                {
                    std::swap(result.parameter[0], result.parameter[1]);
                }
            }
            else if (result.numIntersections == 1)
            {
                result.intersect = true;
                // Used by derived classes.
                result.parameter[1] = result.parameter[0];
            }
        }
    };
}
