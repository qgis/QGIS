// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/DistPointSegment.h>
#include <Mathematics/IntrHalfspace2Polygon2.h>
#include <Mathematics/Sector2.h>

// The OrientedBox2 object is considered to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox2<Real>, Sector2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(OrientedBox2<Real> const& box, Sector2<Real> const& sector)
        {
            Result result;

            // Determine whether the vertex is inside the box.
            Vector2<Real> CmV = box.center - sector.vertex;
            Vector2<Real> P{ Dot(box.axis[0], CmV), Dot(box.axis[1], CmV) };
            if (std::fabs(P[0]) <= box.extent[0] && std::fabs(P[1]) <= box.extent[1])
            {
                // The vertex is inside the box.
                result.intersect = true;
                return result;
            }

            // Test whether the box is outside the right ray boundary of the
            // sector.
            Vector2<Real> U0
            {
                +sector.cosAngle * sector.direction[0] + sector.sinAngle * sector.direction[1],
                -sector.sinAngle * sector.direction[0] + sector.cosAngle * sector.direction[1]
            };
            Vector2<Real> N0 = Perp(U0);
            Real prjcen0 = Dot(N0, CmV);
            Real radius0 = box.extent[0] * std::fabs(Dot(N0, box.axis[0]))
                + box.extent[1] * std::fabs(Dot(N0, box.axis[1]));
            if (prjcen0 > radius0)
            {
                result.intersect = false;
                return result;
            }

            // Test whether the box is outside the ray of the left boundary
            // of the sector.
            Vector2<Real> U1
            {
                +sector.cosAngle * sector.direction[0] - sector.sinAngle * sector.direction[1],
                +sector.sinAngle * sector.direction[0] + sector.cosAngle * sector.direction[1]
            };
            Vector2<Real> N1 = -Perp(U1);
            Real prjcen1 = Dot(N1, CmV);
            Real radius1 = box.extent[0] * std::fabs(Dot(N1, box.axis[0]))
                + box.extent[1] * std::fabs(Dot(N1, box.axis[1]));
            if (prjcen1 > radius1)
            {
                result.intersect = false;
                return result;
            }

            // Initialize the polygon of intersection to be the box.
            Vector2<Real> e0U0 = box.extent[0] * box.axis[0];
            Vector2<Real> e1U1 = box.extent[1] * box.axis[1];
            std::vector<Vector2<Real>> polygon;
            polygon.reserve(8);
            polygon.push_back(box.center - e0U0 - e1U1);
            polygon.push_back(box.center + e0U0 - e1U1);
            polygon.push_back(box.center + e0U0 + e1U1);
            polygon.push_back(box.center - e0U0 + e1U1);

            FIQuery<Real, Halfspace<2, Real>, std::vector<Vector2<Real>>> hpQuery;
            typename FIQuery<Real, Halfspace<2, Real>, std::vector<Vector2<Real>>>::Result hpResult;
            Halfspace<2, Real> halfspace;

            // Clip the box against the right-ray sector boundary.
            if (prjcen0 >= -radius0)
            {
                halfspace.normal = -N0;
                halfspace.constant = Dot(halfspace.normal, sector.vertex);
                hpResult = hpQuery(halfspace, polygon);
                polygon = std::move(hpResult.polygon);
            }

            // Clip the box against the left-ray sector boundary.
            if (prjcen1 >= -radius1)
            {
                halfspace.normal = -N1;
                halfspace.constant = Dot(halfspace.normal, sector.vertex);
                hpResult = hpQuery(halfspace, polygon);
                polygon = std::move(hpResult.polygon);
            }

            DCPQuery<Real, Vector2<Real>, Segment2<Real>> psQuery;
            typename DCPQuery<Real, Vector2<Real>, Segment2<Real>>::Result psResult;
            int const numVertices = static_cast<int>(polygon.size());
            if (numVertices >= 2)
            {
                for (int i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
                {
                    Segment2<Real> segment(polygon[i0], polygon[i1]);
                    psResult = psQuery(sector.vertex, segment);
                    if (psResult.distance <= sector.radius)
                    {
                        result.intersect = true;
                        return result;
                    }
                }
            }

            result.intersect = false;
            return result;
        }
    };
}
