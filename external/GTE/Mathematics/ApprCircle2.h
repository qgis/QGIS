// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector2.h>

// Least-squares fit of a circle to a set of points. The algorithms are
// described in Section 5 of
//   https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf
// FitUsingLengths uses the algorithm of Section 5.1.
// FitUsingSquaredLengths uses the algorithm of Section 5.2.

namespace gte
{
    template <typename Real>
    class ApprCircle2
    {
    public:
        // The return value is 'true' when the linear system of the algorithm
        // is solvable, 'false' otherwise. If 'false' is returned, the circle
        // center and radius are set to zero values.
        bool FitUsingSquaredLengths(int numPoints, Vector2<Real> const* points, Circle2<Real>& circle)
        {
            // Compute the average of the data points.
            Real const zero(0);
            Vector2<Real> A = { zero, zero };
            for (int i = 0; i < numPoints; ++i)
            {
                A += points[i];
            }
            Real invNumPoints = ((Real)1) / static_cast<Real>(numPoints);
            A *= invNumPoints;

            // Compute the covariance matrix M of the Y[i] = X[i]-A and the
            // right-hand side R of the linear system M*(C-A) = R.
            Real M00 = zero, M01 = zero, M11 = zero;
            Vector2<Real> R = { zero, zero };
            for (int i = 0; i < numPoints; ++i)
            {
                Vector2<Real> Y = points[i] - A;
                Real Y0Y0 = Y[0] * Y[0];
                Real Y0Y1 = Y[0] * Y[1];
                Real Y1Y1 = Y[1] * Y[1];
                M00 += Y0Y0;
                M01 += Y0Y1;
                M11 += Y1Y1;
                R += (Y0Y0 + Y1Y1) * Y;
            }
            R *= (Real)0.5;

            // Solve the linear system M*(C-A) = R for the center C.
            Real det = M00 * M11 - M01 * M01;
            if (det != zero)
            {
                circle.center[0] = A[0] + (M11 * R[0] - M01 * R[1]) / det;
                circle.center[1] = A[1] + (M00 * R[1] - M01 * R[0]) / det;
                Real rsqr = zero;
                for (int i = 0; i < numPoints; ++i)
                {
                    Vector2<Real> delta = points[i] - circle.center;
                    rsqr += Dot(delta, delta);
                }
                rsqr *= invNumPoints;
                circle.radius = std::sqrt(rsqr);
                return true;
            }
            else
            {
                circle.center = { zero, zero };
                circle.radius = zero;
                return false;
            }
        }

        // Fit the points using lengths to drive the least-squares algorithm.
        // If initialCenterIsAverage is set to 'false', the initial guess for
        // the initial circle center is computed as the average of the data
        // points. If the data points are clustered along a small arc, the
        // algorithm is slow to converge. If initialCenterIsAverage is set to
        // 'true', the incoming circle center is used as-is to start the
        // iterative algorithm. This approach tends to converge more rapidly
        // than when using the average of points but can be much slower than
        // FitUsingSquaredLengths.
        //
        // The value epsilon may be chosen as a positive number for the
        // comparison of consecutive estimated circle centers, terminating the
        // iterations when the center difference has length less than or equal
        // to epsilon.
        //
        // The return value is the number of iterations used. If is is the
        // input maxIterations, you can either accept the result or polish the
        // result by calling the function again with initialCenterIsAverage
        // set to 'true'.
        unsigned int FitUsingLengths(int numPoints, Vector2<Real> const* points,
            unsigned int maxIterations, bool initialCenterIsAverage,
            Circle2<Real>& circle, Real epsilon = (Real)0)
        {
            // Compute the average of the data points.
            Vector2<Real> average = points[0];
            for (int i = 1; i < numPoints; ++i)
            {
                average += points[i];
            }
            Real invNumPoints = ((Real)1) / static_cast<Real>(numPoints);
            average *= invNumPoints;

            // The initial guess for the center.
            if (initialCenterIsAverage)
            {
                circle.center = average;
            }

            Real epsilonSqr = epsilon * epsilon;
            unsigned int iteration;
            for (iteration = 0; iteration < maxIterations; ++iteration)
            {
                // Update the iterates.
                Vector2<Real> current = circle.center;

                // Compute average L, dL/da, dL/db.
                Real lenAverage = (Real)0;
                Vector2<Real> derLenAverage = Vector2<Real>::Zero();
                for (int i = 0; i < numPoints; ++i)
                {
                    Vector2<Real> diff = points[i] - circle.center;
                    Real length = Length(diff);
                    if (length > (Real)0)
                    {
                        lenAverage += length;
                        Real invLength = ((Real)1) / length;
                        derLenAverage -= invLength * diff;
                    }
                }
                lenAverage *= invNumPoints;
                derLenAverage *= invNumPoints;

                circle.center = average + lenAverage * derLenAverage;
                circle.radius = lenAverage;

                Vector2<Real> diff = circle.center - current;
                Real diffSqrLen = Dot(diff, diff);
                if (diffSqrLen <= epsilonSqr)
                {
                    break;
                }
            }

            return ++iteration;
        }
    };
}
