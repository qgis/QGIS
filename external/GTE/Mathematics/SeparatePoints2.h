// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.01

#pragma once

#include <Mathematics/ConvexHull2.h>

// Separate two point sets, if possible, by computing a line for which the
// point sets lie on opposite sides.  The algorithm computes the convex hull
// of the point sets, then uses the method of separating axes to determine
// whether the two convex polygons are disjoint.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf

namespace gte
{
    template <typename Real, typename ComputeType>
    class SeparatePoints2
    {
    public:
        // The return value is 'true' if and only if there is a separation.
        // If 'true', the returned line is a separating line.  The code
        // assumes that each point set has at least 3 noncollinear points.
        bool operator()(int numPoints0, Vector2<Real> const* points0,
            int numPoints1, Vector2<Real> const* points1,
            Line2<Real>& separatingLine) const
        {
            // Construct convex hull of point set 0.
            ConvexHull2<Real> ch0;
            ch0(numPoints0, points0, (Real)0);
            if (ch0.GetDimension() != 2)
            {
                return false;
            }

            // Construct convex hull of point set 1.
            ConvexHull2<Real> ch1;
            ch1(numPoints1, points1, (Real)0);
            if (ch1.GetDimension() != 2)
            {
                return false;
            }

            int numEdges0 = static_cast<int>(ch0.GetHull().size());
            int const* edges0 = &ch0.GetHull()[0];
            int numEdges1 = static_cast<int>(ch1.GetHull().size());
            int const* edges1 = &ch1.GetHull()[0];

            // Test edges of hull 0 for possible separation of points.
            int j0, j1, i0, i1, side0, side1;
            Vector2<Real> lineNormal;
            Real lineConstant;
            for (j1 = 0, j0 = numEdges0 - 1; j1 < numEdges0; j0 = j1++)
            {
                // Look up edge (assert: i0 != i1 ).
                i0 = edges0[j0];
                i1 = edges0[j1];

                // Compute potential separating line
                // (assert: (xNor,yNor) != (0,0)).
                separatingLine.origin = points0[i0];
                separatingLine.direction = points0[i1] - points0[i0];
                Normalize(separatingLine.direction);
                lineNormal = Perp(separatingLine.direction);
                lineConstant = Dot(lineNormal, separatingLine.origin);

                // Determine whether hull 1 is on same side of line.
                side1 = OnSameSide(lineNormal, lineConstant, numEdges1, edges1,
                    points1);

                if (side1)
                {
                    // Determine on which side of line hull 0 lies.
                    side0 = WhichSide(lineNormal, lineConstant, numEdges0,
                        edges0, points0);

                    if (side0 * side1 <= 0)  // Line separates hulls.
                    {
                        return true;
                    }
                }
            }

            // Test edges of hull 1 for possible separation of points.
            for (j1 = 0, j0 = numEdges1 - 1; j1 < numEdges1; j0 = j1++)
            {
                // Look up edge (assert: i0 != i1 ).
                i0 = edges1[j0];
                i1 = edges1[j1];

                // Compute perpendicular to edge
                // (assert: (xNor,yNor) != (0,0)).
                separatingLine.origin = points1[i0];
                separatingLine.direction = points1[i1] - points1[i0];
                Normalize(separatingLine.direction);
                lineNormal = Perp(separatingLine.direction);
                lineConstant = Dot(lineNormal, separatingLine.origin);

                // Determine whether hull 0 is on same side of line.
                side0 = OnSameSide(lineNormal, lineConstant, numEdges0, edges0,
                    points0);

                if (side0)
                {
                    // Determine on which side of line hull 1 lies.
                    side1 = WhichSide(lineNormal, lineConstant, numEdges1,
                        edges1, points1);

                    if (side0 * side1 <= 0)  // Line separates hulls.
                    {
                        return true;
                    }
                }
            }

            return false;
        }

    private:
        int OnSameSide(Vector2<Real> const& lineNormal, Real lineConstant,
            int numEdges, int const* edges, Vector2<Real> const* points) const
        {
            // Test whether all points on same side of line Dot(N,X) = c.
            Real c0;
            int posSide = 0, negSide = 0;

            for (int i1 = 0, i0 = numEdges - 1; i1 < numEdges; i0 = i1++)
            {
                c0 = Dot(lineNormal, points[edges[i0]]);
                if (c0 > lineConstant)
                {
                    ++posSide;
                }
                else if (c0 < lineConstant)
                {
                    ++negSide;
                }

                if (posSide && negSide)
                {
                    // Line splits point set.
                    return 0;
                }

                c0 = Dot(lineNormal, points[edges[i1]]);
                if (c0 > lineConstant)
                {
                    ++posSide;
                }
                else if (c0 < lineConstant)
                {
                    ++negSide;
                }

                if (posSide && negSide)
                {
                    // Line splits point set.
                    return 0;
                }
            }

            return (posSide ? +1 : -1);
        }

        int WhichSide(Vector2<Real> const& lineNormal, Real lineConstant,
            int numEdges, int const* edges, Vector2<Real> const* points) const
        {
            // Establish which side of line hull is on.
            Real c0;
            for (int i1 = 0, i0 = numEdges - 1; i1 < numEdges; i0 = i1++)
            {
                c0 = Dot(lineNormal, points[edges[i0]]);
                if (c0 > lineConstant)
                {
                    // Hull on positive side.
                    return +1;
                }
                if (c0 < lineConstant)
                {
                    // Hull on negative side.
                    return -1;
                }

                c0 = Dot(lineNormal, points[edges[i1]]);
                if (c0 > lineConstant)
                {
                    // Hull on positive side.
                    return +1;
                }
                if (c0 < lineConstant)
                {
                    // Hull on negative side.
                    return -1;
                }
            }

            // Hull is effectively collinear.
            return 0;
        }
    };
}
