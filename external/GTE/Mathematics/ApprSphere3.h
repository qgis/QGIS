// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector3.h>

// Least-squares fit of a sphere to a set of points. The algorithms are
// described in Section 5 of
//   https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf
// FitUsingLengths uses the algorithm of Section 5.1.
// FitUsingSquaredLengths uses the algorithm of Section 5.2.

namespace gte
{
    template <typename Real>
    class ApprSphere3
    {
    public:
        // The return value is 'true' when the linear system of the algorithm
        // is solvable, 'false' otherwise. If 'false' is returned, the sphere
        // center and radius are set to zero values.
        bool FitUsingSquaredLengths(int numPoints, Vector3<Real> const* points, Sphere3<Real>& sphere)
        {
            // Compute the average of the data points.
            Real const zero(0);
            Vector3<Real> A = { zero, zero, zero };
            for (int i = 0; i < numPoints; ++i)
            {
                A += points[i];
            }
            Real invNumPoints = ((Real)1) / static_cast<Real>(numPoints);
            A *= invNumPoints;

            // Compute the covariance matrix M of the Y[i] = X[i]-A and the
            // right-hand side R of the linear system M*(C-A) = R.
            Real M00 = zero, M01 = zero, M02 = zero, M11 = zero, M12 = zero, M22 = zero;
            Vector3<Real> R = { zero, zero, zero };
            for (int i = 0; i < numPoints; ++i)
            {
                Vector3<Real> Y = points[i] - A;
                Real Y0Y0 = Y[0] * Y[0];
                Real Y0Y1 = Y[0] * Y[1];
                Real Y0Y2 = Y[0] * Y[2];
                Real Y1Y1 = Y[1] * Y[1];
                Real Y1Y2 = Y[1] * Y[2];
                Real Y2Y2 = Y[2] * Y[2];
                M00 += Y0Y0;
                M01 += Y0Y1;
                M02 += Y0Y2;
                M11 += Y1Y1;
                M12 += Y1Y2;
                M22 += Y2Y2;
                R += (Y0Y0 + Y1Y1 + Y2Y2) * Y;
            }
            R *= (Real)0.5;

            // Solve the linear system M*(C-A) = R for the center C.
            Real cof00 = M11 * M22 - M12 * M12;
            Real cof01 = M02 * M12 - M01 * M22;
            Real cof02 = M01 * M12 - M02 * M11;
            Real det = M00 * cof00 + M01 * cof01 + M02 * cof02;
            if (det != zero)
            {
                Real cof11 = M00 * M22 - M02 * M02;
                Real cof12 = M01 * M02 - M00 * M12;
                Real cof22 = M00 * M11 - M01 * M01;
                sphere.center[0] = A[0] + (cof00 * R[0] + cof01 * R[1] + cof02 * R[2]) / det;
                sphere.center[1] = A[1] + (cof01 * R[0] + cof11 * R[1] + cof12 * R[2]) / det;
                sphere.center[2] = A[2] + (cof02 * R[0] + cof12 * R[1] + cof22 * R[2]) / det;
                Real rsqr = zero;
                for (int i = 0; i < numPoints; ++i)
                {
                    Vector3<Real> delta = points[i] - sphere.center;
                    rsqr += Dot(delta, delta);
                }
                rsqr *= invNumPoints;
                sphere.radius = std::sqrt(rsqr);
                return true;
            }
            else
            {
                sphere.center = { zero, zero, zero };
                sphere.radius = zero;
                return false;
            }
        }

        // Fit the points using lengths to drive the least-squares algorithm.
        // If initialCenterIsAverage is set to 'false', the initial guess for
        // the initial sphere center is computed as the average of the data
        // points. If the data points are clustered along a small solid angle,
        // the algorithm is slow to converge. If initialCenterIsAverage is set
        // to 'true', the incoming sphere center is used as-is to start the
        // iterative algorithm. This approach tends to converge more rapidly
        // than when using the average of points but can be much slower than
        // FitUsingSquaredLengths.
        //
        // The value epsilon may be chosen as a positive number for the
        // comparison of consecutive estimated sphere centers, terminating the
        // iterations when the center difference has length less than or equal
        // to epsilon.
        //
        // The return value is the number of iterations used. If is is the
        // input maxIterations, you can either accept the result or polish the
        // result by calling the function again with initialCenterIsAverage
        // set to 'true'.
        unsigned int FitUsingLengths(int numPoints, Vector3<Real> const* points,
            unsigned int maxIterations, bool initialCenterIsAverage,
            Sphere3<Real>& sphere, Real epsilon = (Real)0)
        {
            // Compute the average of the data points.
            Vector3<Real> average = points[0];
            for (int i = 1; i < numPoints; ++i)
            {
                average += points[i];
            }
            Real invNumPoints = ((Real)1) / static_cast<Real>(numPoints);
            average *= invNumPoints;

            // The initial guess for the center.
            if (initialCenterIsAverage)
            {
                sphere.center = average;
            }

            Real epsilonSqr = epsilon * epsilon;
            unsigned int iteration;
            for (iteration = 0; iteration < maxIterations; ++iteration)
            {
                // Update the iterates.
                Vector3<Real> current = sphere.center;

                // Compute average L, dL/da, dL/db, dL/dc.
                Real lenAverage = (Real)0;
                Vector3<Real> derLenAverage = Vector3<Real>::Zero();
                for (int i = 0; i < numPoints; ++i)
                {
                    Vector3<Real> diff = points[i] - sphere.center;
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

                sphere.center = average + lenAverage * derLenAverage;
                sphere.radius = lenAverage;

                Vector3<Real> diff = sphere.center - current;
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
