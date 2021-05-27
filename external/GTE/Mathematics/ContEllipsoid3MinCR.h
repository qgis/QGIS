// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix3x3.h>
#include <random>

// Compute the minimum-volume ellipsoid, (X-C)^T R D R^T (X-C) = 1, given the
// center C and orientation matrix R.  The columns of R are the axes of the
// ellipsoid.  The algorithm computes the diagonal matrix D.  The minimum
// volume is (4*pi/3)/sqrt(D[0]*D[1]*D[2]), where D = diag(D[0],D[1],D[2]).
// The problem is equivalent to maximizing the product D[0]*D[1]*D[2] for a
// given C and R, and subject to the constraints
//   (P[i]-C)^T R D R^T (P[i]-C) <= 1
// for all input points P[i] with 0 <= i < N.  Each constraint has the form
//   A[0]*D[0] + A[1]*D[1] + A[2]*D[2] <= 1
// where A[0] >= 0, A[1] >= 0, and A[2] >= 0.

namespace gte
{
    template <typename Real>
    class ContEllipsoid3MinCR
    {
    public:
        void operator()(int numPoints, Vector3<Real> const* points,
            Vector3<Real> const& C, Matrix3x3<Real> const& R, Real D[3]) const
        {
            // Compute the constraint coefficients, of the form (A[0],A[1])
            // for each i.
            std::vector<Vector3<Real>> A(numPoints);
            for (int i = 0; i < numPoints; ++i)
            {
                Vector3<Real> diff = points[i] - C;  // P[i] - C
                Vector3<Real> prod = diff * R;  // R^T*(P[i] - C) = (u,v,w)
                A[i] = prod * prod;  // (u^2, v^2, w^2)
            }

            // TODO:  Sort the constraints to eliminate redundant ones.  It
            // is clear how to do this in ContEllipse2MinCR.  How to do this
            // in 3D?

            MaxProduct(A, D);
        }

    private:
        void FindEdgeMax(std::vector<Vector3<Real>>& A, int& plane0, int& plane1, Real D[3]) const
        {
            // Compute direction to local maximum point on line of
            // intersection.
            Real xDir = A[plane0][1] * A[plane1][2] - A[plane1][1] * A[plane0][2];
            Real yDir = A[plane0][2] * A[plane1][0] - A[plane1][2] * A[plane0][0];
            Real zDir = A[plane0][0] * A[plane1][1] - A[plane1][0] * A[plane0][1];

            // Build quadratic Q'(t) = (d/dt)(x(t)y(t)z(t)) = a0+a1*t+a2*t^2.
            Real a0 = D[0] * D[1] * zDir + D[0] * D[2] * yDir + D[1] * D[2] * xDir;
            Real a1 = (Real)2 * (D[2] * xDir * yDir + D[1] * xDir * zDir + D[0] * yDir * zDir);
            Real a2 = (Real)3 * (xDir * yDir * zDir);

            // Find root to Q'(t) = 0 corresponding to maximum.
            Real tFinal;
            if (a2 != (Real)0)
            {
                Real invA2 = (Real)1 / a2;
                Real discr = a1 * a1 - (Real)4 * a0 * a2;
                discr = std::sqrt(std::max(discr, (Real)0));
                tFinal = (Real)-0.5 * (a1 + discr) * invA2;
                if (a1 + (Real)2 * a2 * tFinal > (Real)0)
                {
                    tFinal = (Real)0.5 * (-a1 + discr) * invA2;
                }
            }
            else if (a1 != (Real)0)
            {
                tFinal = -a0 / a1;
            }
            else if (a0 != (Real)0)
            {
                Real fmax = std::numeric_limits<Real>::max();
                tFinal = (a0 >= (Real)0 ? fmax : -fmax);
            }
            else
            {
                return;
            }

            if (tFinal < (Real)0)
            {
                // Make (xDir,yDir,zDir) point in direction of increase of Q.
                tFinal = -tFinal;
                xDir = -xDir;
                yDir = -yDir;
                zDir = -zDir;
            }

            // Sort remaining planes along line from current point to local
            // maximum.
            Real tMax = tFinal;
            int plane2 = -1;
            int numPoints = static_cast<int>(A.size());
            for (int i = 0; i < numPoints; ++i)
            {
                if (i == plane0 || i == plane1)
                {
                    continue;
                }

                Real norDotDir = A[i][0] * xDir + A[i][1] * yDir + A[i][2] * zDir;
                if (norDotDir <= (Real)0)
                {
                    continue;
                }

                // Theoretically the numerator must be nonnegative since an
                // invariant in the algorithm is that (x0,y0,z0) is on the
                // convex hull of the constraints.  However, some numerical
                // error may make this a small negative number.  In that case
                // set tmax = 0 (no change in position).
                Real numer = (Real)1 - A[i][0] * D[0] - A[i][1] * D[1] - A[i][2] * D[2];
                LogAssert(numer >= (Real)0, "Unexpected condition.");

                Real t = numer / norDotDir;
                if (0 <= t && t < tMax)
                {
                    plane2 = i;
                    tMax = t;
                }
            }

            D[0] += tMax * xDir;
            D[1] += tMax * yDir;
            D[2] += tMax * zDir;

            if (tMax == tFinal)
            {
                return;
            }

            if (tMax > (Real)0)
            {
                plane0 = plane2;
                FindFacetMax(A, plane0, D);
                return;
            }

            // tmax == 0, so return with D[0], D[1], and D[2] unchanged.
        }

        void FindFacetMax(std::vector<Vector3<Real>>& A, int& plane0, Real D[3]) const
        {
            Real tFinal, xDir, yDir, zDir;

            if (A[plane0][0] > (Real)0
                && A[plane0][1] > (Real)0
                && A[plane0][2] > (Real)0)
            {
                // Compute local maximum point on plane.
                Real oneThird = (Real)1 / (Real)3;
                Real xMax = oneThird / A[plane0][0];
                Real yMax = oneThird / A[plane0][1];
                Real zMax = oneThird / A[plane0][2];

                // Compute direction to local maximum point on plane.
                tFinal = (Real)1;
                xDir = xMax - D[0];
                yDir = yMax - D[1];
                zDir = zMax - D[2];
            }
            else
            {
                tFinal = std::numeric_limits<Real>::max();

                if (A[plane0][0] > (Real)0)
                {
                    xDir = (Real)0;
                }
                else
                {
                    xDir = (Real)1;
                }

                if (A[plane0][1] > (Real)0)
                {
                    yDir = (Real)0;
                }
                else
                {
                    yDir = (Real)1;
                }

                if (A[plane0][2] > (Real)0)
                {
                    zDir = (Real)0;
                }
                else
                {
                    zDir = (Real)1;
                }
            }

            // Sort remaining planes along line from current point.
            Real tMax = tFinal;
            int plane1 = -1;
            int numPoints = static_cast<int>(A.size());
            for (int i = 0; i < numPoints; ++i)
            {
                if (i == plane0)
                {
                    continue;
                }

                Real norDotDir = A[i][0] * xDir + A[i][1] * yDir + A[i][2] * zDir;
                if (norDotDir <= (Real)0)
                {
                    continue;
                }

                // Theoretically the numerator must be nonnegative because an
                // invariant in the algorithm is that (x0,y0,z0) is on the
                // convex hull of the constraints.  However, some numerical
                // error may make this a small negative number.  In that case,
                // set tmax = 0 (no change in position).
                Real numer = (Real)1 - A[i][0] * D[0] - A[i][1] * D[1] - A[i][2] * D[2];
                LogAssert(numer >= (Real)0, "Unexpected condition.");

                Real t = numer / norDotDir;
                if (0 <= t && t < tMax)
                {
                    plane1 = i;
                    tMax = t;
                }
            }

            D[0] += tMax * xDir;
            D[1] += tMax * yDir;
            D[2] += tMax * zDir;

            if (tMax == (Real)1)
            {
                return;
            }

            if (tMax > (Real)0)
            {
                plane0 = plane1;
                FindFacetMax(A, plane0, D);
                return;
            }

            FindEdgeMax(A, plane0, plane1, D);
        }

        void MaxProduct(std::vector<Vector3<Real>>& A, Real D[3]) const
        {
            // Maximize x*y*z subject to x >= 0, y >= 0, z >= 0, and
            // A[i]*x+B[i]*y+C[i]*z <= 1 for 0 <= i < N where A[i] >= 0,
            // B[i] >= 0, and C[i] >= 0.

            // Jitter the lines to avoid cases where more than three planes
            // intersect at the same point.  Should also break parallelism
            // and planes parallel to the coordinate planes.
            std::mt19937 mte;
            std::uniform_real_distribution<Real> rnd((Real)0, (Real)1);
            Real maxJitter = (Real)1e-12;
            int numPoints = static_cast<int>(A.size());
            int i;
            for (i = 0; i < numPoints; ++i)
            {
                A[i][0] += maxJitter * rnd(mte);
                A[i][1] += maxJitter * rnd(mte);
                A[i][2] += maxJitter * rnd(mte);
            }

            // Sort lines along the z-axis (x = 0 and y = 0).
            int plane = -1;
            Real zmax = (Real)0;
            for (i = 0; i < numPoints; ++i)
            {
                if (A[i][2] > zmax)
                {
                    zmax = A[i][2];
                    plane = i;
                }
            }
            LogAssert(plane != -1, "Unexpected condition.");

            // Walk along convex hull searching for maximum.
            D[0] = (Real)0;
            D[1] = (Real)0;
            D[2] = (Real)1 / zmax;
            FindFacetMax(A, plane, D);
        }
    };
}
