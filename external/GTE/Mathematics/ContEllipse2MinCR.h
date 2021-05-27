// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Matrix2x2.h>

// Compute the minimum-area ellipse, (X-C)^T R D R^T (X-C) = 1, given the
// center C and the orientation matrix R.  The columns of R are the axes of
// the ellipse.  The algorithm computes the diagonal matrix D.  The minimum
// area is pi/sqrt(D[0]*D[1]), where D = diag(D[0],D[1]).  The problem is
// equivalent to maximizing the product D[0]*D[1] for a given C and R, and
// subject to the constraints
//   (P[i]-C)^T R D R^T (P[i]-C) <= 1
// for all input points P[i] with 0 <= i < N.  Each constraint has the form
//   A[0]*D[0] + A[1]*D[1] <= 1
// where A[0] >= 0 and A[1] >= 0.

namespace gte
{
    template <typename Real>
    class ContEllipse2MinCR
    {
    public:
        void operator()(int numPoints, Vector2<Real> const* points,
            Vector2<Real> const& C, Matrix2x2<Real> const& R, Real D[2]) const
        {
            // Compute the constraint coefficients, of the form (A[0],A[1])
            // for each i.
            std::vector<Vector2<Real>> A(numPoints);
            for (int i = 0; i < numPoints; ++i)
            {
                Vector2<Real> diff = points[i] - C;  // P[i] - C
                Vector2<Real> prod = diff * R;  // R^T*(P[i] - C) = (u,v)
                A[i] = prod * prod;  // (u^2, v^2)
            }

            // Use a lexicographical sort to eliminate redundant constraints.
            // Remove all but the first entry in blocks with x0 = x1 because
            // the corresponding constraint lines for the first entry hides
            // all the others from the origin.
            std::sort(A.begin(), A.end(),
                [](Vector2<Real> const& P0, Vector2<Real> const& P1)
                {
                    if (P0[0] > P1[0]) { return true; }
                    if (P0[0] < P1[0]) { return false; }
                    return P0[1] > P1[1];
                }
            );
            auto end = std::unique(A.begin(), A.end(),
                [](Vector2<Real> const& P0, Vector2<Real> const& P1)
                {
                    return P0[0] == P1[0];
                }
            );
            A.erase(end, A.end());

            // Use a lexicographical sort to eliminate redundant constraints.
            // Remove all but the first entry in blocks/ with y0 = y1 because
            // the corresponding constraint lines for the first entry hides
            // all the others from the origin.
            std::sort(A.begin(), A.end(),
                [](Vector2<Real> const& P0, Vector2<Real> const& P1)
                {
                    if (P0[1] > P1[1])
                    {
                        return true;
                    }

                    if (P0[1] < P1[1])
                    {
                        return false;
                    }

                    return P0[0] > P1[0];
                }
            );
            end = std::unique(A.begin(), A.end(),
                [](Vector2<Real> const& P0, Vector2<Real> const& P1)
                {
                    return P0[1] == P1[1];
                }
            );
            A.erase(end, A.end());

            MaxProduct(A, D);
        }

    private:
        static void MaxProduct(std::vector<Vector2<Real>>& A, Real D[2])
        {
            // Keep track of which constraint lines have already been used in
            // the search.
            int numConstraints = static_cast<int>(A.size());
            std::vector<bool> used(A.size());
            std::fill(used.begin(), used.end(), false);

            // Find the constraint line whose y-intercept (0,ymin) is closest
            // to the origin.  This line contributes to the convex hull of the
            // constraints and the search for the maximum starts here.  Also
            // find the constraint line whose x-intercept (xmin,0) is closest
            // to the origin.  This line contributes to the convex hull of the
            // constraints and the search for the maximum terminates before or
            // at this line.
            int i, iYMin = -1;
            int iXMin = -1;
            Real axMax = (Real)0, ayMax = (Real)0;  // A[i] >= (0,0) by design
            for (i = 0; i < numConstraints; ++i)
            {
                // The minimum x-intercept is 1/A[iXMin][0] for A[iXMin][0]
                // the maximum of the A[i][0].
                if (A[i][0] > axMax)
                {
                    axMax = A[i][0];
                    iXMin = i;
                }

                // The minimum y-intercept is 1/A[iYMin][1] for A[iYMin][1]
                // the maximum of the A[i][1].
                if (A[i][1] > ayMax)
                {
                    ayMax = A[i][1];
                    iYMin = i;
                }
            }
            LogAssert(iXMin != -1 && iYMin != -1, "Unexpected condition.");
            used[iYMin] = true;

            // The convex hull is searched in a clockwise manner starting with
            // the constraint line constructed above.  The next vertex of the
            // hull occurs as the closest point to the first vertex on the
            // current constraint line.  The following loop finds each
            // consecutive vertex.
            Real x0 = (Real)0, xMax = ((Real)1) / axMax;
            int j;
            for (j = 0; j < numConstraints; ++j)
            {
                // Find the line whose intersection with the current line is
                // closest to the last hull vertex.  The last vertex is at
                // (x0,y0) on the current line.
                Real x1 = xMax;
                int line = -1;
                for (i = 0; i < numConstraints; ++i)
                {
                    if (!used[i])
                    {
                        // This line not yet visited, process it.  Given
                        // current constraint line a0*x+b0*y =1 and candidate
                        // line a1*x+b1*y = 1, find the point of intersection.
                        // The determinant of the system is d = a0*b1-a1*b0.
                        // We care only about lines that have more negative
                        // slope than the previous one, that is,
                        // -a1/b1 < -a0/b0, in which case we process only
                        // lines for which d < 0.
                        Real det = DotPerp(A[iYMin], A[i]);
                        if (det < (Real)0)
                        {
                            // Compute the x-value for the point of
                            // intersection, (x1,y1).  There may be floating
                            // point error issues in the comparision
                            // 'D[0] <= x1'.  Consider modifying to
                            // 'D[0] <= x1 + epsilon'.
                            D[0] = (A[i][1] - A[iYMin][1]) / det;
                            if (x0 < D[0] && D[0] <= x1)
                            {
                                line = i;
                                x1 = D[0];
                            }
                        }
                    }
                }

                // Next vertex is at (x1,y1) whose x-value was computed above.
                // First check for the maximum of x*y on the current line for
                // x in [x0,x1].  On this interval the function is
                // f(x) = x*(1-a0*x)/b0.  The derivative is
                // f'(x) = (1-2*a0*x)/b0 and f'(r) = 0 when r = 1/(2*a0).
                // The three candidates for the maximum are f(x0), f(r) and
                // f(x1).  Comparisons are made between r and the endpoints
                // x0 and x1.  Because a0 = 0 is possible (constraint line is
                // horizontal and f is increasing on line), the division in r
                // is not performed and the comparisons are made between
                // 1/2 = a0*r and a0*x0 or a0*x1.

                // Compare r < x0.
                if ((Real)0.5 < A[iYMin][0] * x0)
                {
                    // The maximum is f(x0) since the quadratic f decreases
                    // for x > r.  The value D[1] is f(x0).
                    D[0] = x0;
                    D[1] = ((Real)1 - A[iYMin][0] * D[0]) / A[iYMin][1];
                    break;
                }

                // Compare r < x1.
                if ((Real)0.5 < A[iYMin][0] * x1)
                {
                    // The maximum is f(r).  The search ends here because the
                    // current line is tangent to the level curve of
                    // f(x) = f(r) and x*y can therefore only decrease as we
                    // traverse farther around the hull in the clockwise
                    // direction.  The value D[1] is f(r).
                    D[0] = (Real)0.5 / A[iYMin][0];
                    D[1] = (Real)0.5 / A[iYMin][1];
                    break;
                }

                // The maximum is f(x1).  The function x*y is potentially
                // larger on the next line, so continue the search.
                LogAssert(line != -1, "Unexpected condition.");
                x0 = x1;
                x1 = xMax;
                used[line] = true;
                iYMin = line;
            }

            LogAssert(j < numConstraints, "Unexpected condition.");
        }
    };
}
