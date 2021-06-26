// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ContScribeCircle2.h>
#include <vector>

// The ellipse is (x/a)^2 + (y/b)^2 = 1, but only the portion in the first
// quadrant (x >= 0 and y >= 0) is approximated.  Generate numArcs >= 2 arcs
// by constructing points corresponding to the weighted averages of the
// curvatures at the ellipse points (a,0) and (0,b).  The returned input point
// array has numArcs+1 elements and the returned input center and radius
// arrays each have numArc elements.  The arc associated with points[i] and
// points[i+1] has center centers[i] and radius radii[i].  The algorithm
// is described in
//   https://www.geometrictools.com/Documentation/ApproximateEllipse.pdf

namespace gte
{
    // The function returns 'true' when the approximation succeeded, in which
    // case the output arrays are nonempty.  If the 'numArcs' is smaller than
    // 2 or a == b or one of the calls to Circumscribe fails, the function
    // returns 'false'.

    template <typename Real>
    bool ApproximateEllipseByArcs(Real a, Real b, int numArcs,
        std::vector<Vector2<Real>>& points, std::vector<Vector2<Real>>& centers,
        std::vector<Real>& radii)
    {
        if (numArcs < 2 || a == b)
        {
            // At least 2 arcs are required.  The ellipse cannot already be a
            // circle.
            points.clear();
            centers.clear();
            radii.clear();
            return false;
        }

        points.resize(numArcs + 1);
        centers.resize(numArcs);
        radii.resize(numArcs);

        // Compute intermediate ellipse quantities.
        Real a2 = a * a, b2 = b * b, ab = a * b;
        Real invB2mA2 = (Real)1 / (b2 - a2);

        // Compute the endpoints of the ellipse in the first quadrant.  The
        // points are generated in counterclockwise order.
        points[0] = { a, (Real)0 };
        points[numArcs] = { (Real)0, b };

        // Compute the curvature at the endpoints.  These are used when
        // computing the arcs.
        Real curv0 = a / b2;
        Real curv1 = b / a2;

        // Select the ellipse points based on curvature properties.
        Real invNumArcs = (Real)1 / numArcs;
        for (int i = 1; i < numArcs; ++i)
        {
            // The curvature at a new point is a weighted average of curvature
            // at the endpoints.
            Real weight1 = static_cast<Real>(i) * invNumArcs;
            Real weight0 = (Real)1 - weight1;
            Real curv = weight0 * curv0 + weight1 * curv1;

            // Compute point having this curvature.
            Real tmp = std::pow(ab / curv, (Real)2 / (Real)3);
            points[i][0] = a * std::sqrt(std::fabs((tmp - a2) * invB2mA2));
            points[i][1] = b * std::sqrt(std::fabs((tmp - b2) * invB2mA2));
        }

        // Compute the arc at (a,0).
        Circle2<Real> circle;
        Vector2<Real> const& p0 = points[0];
        Vector2<Real> const& p1 = points[1];
        if (!Circumscribe(Vector2<Real>{ p1[0], -p1[1] }, p0, p1, circle))
        {
            // This should not happen for the arc-fitting algorithm.
            points.clear();
            centers.clear();
            radii.clear();
            return false;
        }
        centers[0] = circle.center;
        radii[0] = circle.radius;

        // Compute arc at (0,b).
        int last = numArcs - 1;
        Vector2<Real> const& pNm1 = points[last];
        Vector2<Real> const& pN = points[numArcs];
        if (!Circumscribe(Vector2<Real>{ -pNm1[0], pNm1[1] }, pN, pNm1, circle))
        {
            // This should not happen for the arc-fitting algorithm.
            points.clear();
            centers.clear();
            radii.clear();
            return false;
        }

        centers[last] = circle.center;
        radii[last] = circle.radius;

        // Compute arcs at intermediate points between (a,0) and (0,b).
        for (int iM = 0, i = 1, iP = 2; i < last; ++iM, ++i, ++iP)
        {
            Circumscribe(points[iM], points[i], points[iP], circle);
            centers[i] = circle.center;
            radii[i] = circle.radius;
        }
        return true;
    }
}
