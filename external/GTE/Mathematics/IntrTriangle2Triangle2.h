// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrConvexPolygonHyperplane.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector2.h>

// The test-intersection queries are based on the document
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection query for stationary triangles is based on clipping
// one triangle against the edges of the other to compute the intersection
// set (if it exists).  The find-intersection query for moving triangles is
// based on the previously mentioned document about the method of separating
// axes.

namespace gte
{
    // Test whether two triangles intersect using the method of separating
    // axes.  The set of intersection, if it exists, is not computed.  The
    // input triangles' vertices must be counterclockwise ordered.
    template <typename Real>
    class TIQuery<Real, Triangle2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Triangle2<Real> const& triangle0, Triangle2<Real> const& triangle1)
        {
            Result result =
            {
                !Separated(triangle0, triangle1) && !Separated(triangle1, triangle0)
            };
            return result;
        }

    protected:
        // The triangle vertices are projected to t-values for the line P+t*D.
        // The D-vector is nonzero but does not have to be unit length.  The
        // return value is +1 if all t >= 0, -1 if all t <= 0, but 0 otherwise
        // in which case the line splits the triangle into two subtriangles,
        // each of positive area.
        int WhichSide(Triangle2<Real> const& triangle, Vector2<Real> const& P, Vector2<Real> const& D) const
        {
            int positive = 0, negative = 0;
            for (int i = 0; i < 3; ++i)
            {
                Real t = Dot(D, triangle.v[i] - P);
                if (t > (Real)0)
                {
                    ++positive;
                }
                else if (t < (Real)0)
                {
                    --negative;
                }

                if (positive && negative)
                {
                    // The triangle has vertices strictly on both sides of
                    // the line, so the line splits the triangle into two
                    // subtriangles each of positive area.
                    return 0;
                }
            }

            // Either positive > 0 or negative > 0 but not both are positive.
            return (positive > 0 ? +1 : -1);
        }

        bool Separated(Triangle2<Real> const& triangle0, Triangle2<Real> const& triangle1) const
        {
            // Test edges of triangle0 for separation.  Because of the
            // counterclockwise ordering, the projection interval for
            // triangle0 is [T,0] for some T < 0.  Determine whether
            // triangle1 is on the positive side of the line; if it is,
            // the triangles are separated.
            for (int i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                // The potential separating axis is P+t*D.
                Vector2<Real> P = triangle0.v[i0];
                Vector2<Real> D = Perp(triangle0.v[i1] - triangle0.v[i0]);
                if (WhichSide(triangle1, P, D) > 0)
                {
                    // The triangle1 projection interval is [a,b] where a > 0,
                    // so the triangles are separated.
                    return true;
                }
            }
            return false;
        }
    };

    // Find the convex polygon, segment or point of intersection of two
    // triangles.  The input triangles' vertices must be counterclockwise
    // ordered.
    template <typename Real>
    class FIQuery<Real, Triangle2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
        {
            // An intersection exists iff intersection.size() > 0.
            std::vector<Vector2<Real>> intersection;
        };

        Result operator()(Triangle2<Real> const& triangle0, Triangle2<Real> const& triangle1)
        {
            Result result;

            // Start with triangle1 and clip against the edges of triangle0.
            std::vector<Vector2<Real>> polygon =
            {
                triangle1.v[0], triangle1.v[1], triangle1.v[2]
            };

            typedef FIQuery<Real, std::vector<Vector<2, Real>>, Hyperplane<2, Real>> PPQuery;
            PPQuery ppQuery;

            for (int i1 = 2, i0 = 0; i0 < 3; i1 = i0++)
            {
                // Create the clipping line for the current edge.  The edge
                // normal N points inside the triangle.
                Vector2<Real> P = triangle0.v[i0];
                Vector2<Real> N = Perp(triangle0.v[i1] - triangle0.v[i0]);
                Hyperplane<2, Real> clippingLine(N, Dot(N, P));

                // Do the clipping operation.
                auto ppResult = ppQuery(polygon, clippingLine);
                if (ppResult.positivePolygon.size() == 0)
                {
                    // The current clipped polygon is outside triangle0.
                    return result;
                }
                polygon = std::move(ppResult.positivePolygon);
            }

            result.intersection = polygon;
            return result;
        }
    };
}
