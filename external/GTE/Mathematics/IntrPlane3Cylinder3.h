// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/DistPoint3Plane3.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Ellipse3.h>
#include <Mathematics/Line.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Plane3<Real>, Cylinder3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        // The cylinder must have finite height.
        Result operator()(Plane3<Real> const& plane, Cylinder3<Real> const& cylinder)
        {
            LogAssert(cylinder.height != std::numeric_limits<Real>::max(),
                "Cylinder height must be finite.");

            Result result;

            // Compute extremes of signed distance Dot(N,X)-d for points on
            // the cylinder.  These are
            //   min = (Dot(N,C)-d) - r*sqrt(1-Dot(N,W)^2) - (h/2)*|Dot(N,W)|
            //   max = (Dot(N,C)-d) + r*sqrt(1-Dot(N,W)^2) + (h/2)*|Dot(N,W)|
            DCPQuery<Real, Vector3<Real>, Plane3<Real>> vpQuery;
            Real distance = vpQuery(cylinder.axis.origin, plane).distance;
            Real absNdW = std::fabs(Dot(plane.normal, cylinder.axis.direction));
            Real root = std::sqrt(std::max((Real)1 - absNdW * absNdW, (Real)0));
            Real term = cylinder.radius * root + (Real)0.5 * cylinder.height * absNdW;

            // Intersection occurs if and only if 0 is in the interval
            // [min,max].
            result.intersect = (distance <= term);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Plane3<Real>, Cylinder3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;

            // The type of intersection.
            //   0: none
            //   1: single line (cylinder is tangent to plane), line[0] valid
            //   2: two parallel lines (plane cuts cylinder in two lines)
            //   3: circle (cylinder axis perpendicular to plane)
            //   4: ellipse (cylinder axis neither parallel nor perpendicular)
            int type;
            Line3<Real> line[2];
            Circle3<Real> circle;
            Ellipse3<Real> ellipse;
        };

        // The cylinder must have infinite height.
        Result operator()(Plane3<Real> const& plane, Cylinder3<Real> const& cylinder)
        {
            LogAssert(cylinder.height != std::numeric_limits<Real>::max(),
                "Cylinder height must be finite.");

            Result result;

            DCPQuery<Real, Vector3<Real>, Plane3<Real>> vpQuery;
            Real sdistance = vpQuery(cylinder.axis.origin, plane).signedDistance;
            Vector3<Real> center = cylinder.axis.origin - sdistance * plane.normal;
            Real cosTheta = Dot(cylinder.axis.direction, plane.normal);
            Real absCosTheta = std::fabs(cosTheta);

            if (absCosTheta > (Real)0)
            {
                // The cylinder axis intersects the plane in a unique point.
                result.intersect = true;
                if (absCosTheta < (Real)1)
                {
                    result.type = 4;
                    result.ellipse.normal = plane.normal;
                    result.ellipse.center = cylinder.axis.origin -
                        (sdistance / cosTheta) * cylinder.axis.direction;
                    result.ellipse.axis[0] = cylinder.axis.direction -
                        cosTheta * plane.normal;
                    Normalize(result.ellipse.axis[0]);
                    result.ellipse.axis[1] = UnitCross(plane.normal,
                        result.ellipse.axis[0]);
                    result.ellipse.extent[0] = cylinder.radius / absCosTheta;
                    result.ellipse.extent[1] = cylinder.radius;
                }
                else
                {
                    result.type = 3;
                    result.circle.normal = plane.normal;
                    result.circle.center = center;
                    result.circle.radius = cylinder.radius;
                }
            }
            else
            {
                // The cylinder is parallel to the plane.
                Real distance = std::fabs(sdistance);
                if (distance < cylinder.radius)
                {
                    result.intersect = true;
                    result.type = 2;

                    Vector3<Real> offset = Cross(cylinder.axis.direction, plane.normal);
                    Real extent = std::sqrt(cylinder.radius * cylinder.radius - sdistance * sdistance);

                    result.line[0].origin = center - extent * offset;
                    result.line[0].direction = cylinder.axis.direction;
                    result.line[1].origin = center + extent * offset;
                    result.line[1].direction = cylinder.axis.direction;
                }
                else if (distance == cylinder.radius)
                {
                    result.intersect = true;
                    result.type = 1;
                    result.line[0].origin = center;
                    result.line[0].direction = cylinder.axis.direction;
                }
                else
                {
                    result.intersect = false;
                    result.type = 0;
                }
            }

            return result;
        }
    };
}
