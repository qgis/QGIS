// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/LCPSolver.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/Rectangle.h>
#include <Mathematics/Vector3.h>

// Compute the distance between a rectangle and an aligned box in 3D.  The
// algorithm is based on using an LCP solver for the convex quadratic
// programming problem.  For details, see
// https://www.geometrictools.com/Documentation/ConvexQuadraticProgramming.pdf

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Rectangle3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
        {
            bool queryIsSuccessful;

            // These members are valid only when queryIsSuccessful is true;
            // otherwise, they are all set to zero.
            Real distance, sqrDistance;
            std::array<Real, 2> rectangleParameter;
            std::array<Real, 3> boxParameter;
            Vector3<Real> closestPoint[2];

            // The number of iterations used by LCPSolver regardless of
            // whether the query is successful.
            int numLCPIterations;
        };

        // The default maximum iterations is 81 (n = 9, maxIterations = n*n).
        // If the solver fails to converge, try increasing the maximum number
        // of iterations.
        void SetMaxLCPIterations(int maxLCPIterations)
        {
            mLCP.SetMaxIterations(maxLCPIterations);
        }

        Result operator()(Rectangle3<Real> const& rectangle, AlignedBox3<Real> const& box)
        {
            Result result;

            // Translate the rectangle and aligned box so that the aligned
            // box becomes a canonical box.
            Vector3<Real> K = box.max - box.min;
            Vector3<Real> V = rectangle.center - box.min;

            // Convert the oriented rectangle to a regular one (origin at a
            // corner).
            Vector3<Real> scaledE0 = rectangle.axis[0] * rectangle.extent[0];
            Vector3<Real> scaledE1 = rectangle.axis[1] * rectangle.extent[1];
            Vector3<Real> E0 = scaledE0 * (Real)2;
            Vector3<Real> E1 = scaledE1 * (Real)2;
            V -= scaledE0 + scaledE1;

            // Compute quantities to initialize q and M in the LCP.
            Real dotVE0 = Dot(V, E0);
            Real dotVE1 = Dot(V, E1);
            Real dotE0E0 = Dot(E0, E0);
            Real dotE1E1 = Dot(E1, E1);

            // The LCP has 5 variables and 5 (nontrivial) inequality
            // constraints.
            std::array<Real, 10> q =
            {
                -V[0], -V[1], -V[2], dotVE0, dotVE1, K[0], K[1], K[2], (Real)1, (Real)1
            };

            std::array<std::array<Real, 10>, 10> M;
            M[0] = { (Real)1, (Real)0, (Real)0, -E0[0], -E1[0], (Real)1, (Real)0, (Real)0, (Real)0, (Real)0 };
            M[1] = { (Real)0, (Real)1, (Real)0, -E0[1], -E1[1], (Real)0, (Real)1, (Real)0, (Real)0, (Real)0 };
            M[2] = { (Real)0, (Real)0, (Real)1, -E0[2], -E1[2], (Real)0, (Real)0, (Real)1, (Real)0 , (Real)0 };
            M[3] = { -E0[0], -E0[1], -E0[2], dotE0E0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)1, (Real)0 };
            M[4] = { -E1[0], -E1[1], -E1[2], (Real)0, dotE1E1, (Real)0, (Real)0, (Real)0, (Real)0, (Real)1 };
            M[5] = { (Real)-1, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0 };
            M[6] = { (Real)0, (Real)-1, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0 };
            M[7] = { (Real)0, (Real)0, (Real)-1, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0 };
            M[8] = { (Real)0, (Real)0, (Real)0, (Real)-1, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0 };
            M[9] = { (Real)0, (Real)0, (Real)0, (Real)0, (Real)-1, (Real)0, (Real)0, (Real)0, (Real)0, (Real)0 };

            std::array<Real, 10> w, z;
            if (mLCP.Solve(q, M, w, z))
            {
                result.queryIsSuccessful = true;
                Real t0 = (z[3] * (Real)2 - (Real)1) * rectangle.extent[0];
                Real t1 = (z[4] * (Real)2 - (Real)1) * rectangle.extent[1];
                result.rectangleParameter[0] = t0;
                result.rectangleParameter[1] = t1;
                result.closestPoint[0] = rectangle.center + t0 * rectangle.axis[0] + t1 * rectangle.axis[1];
                for (int i = 0; i < 3; ++i)
                {
                    result.boxParameter[i] = z[i] + box.min[i];
                    result.closestPoint[1][i] = result.boxParameter[i];
                }

                Vector3<Real> diff = result.closestPoint[1] - result.closestPoint[0];
                result.sqrDistance = Dot(diff, diff);
                result.distance = std::sqrt(result.sqrDistance);
            }
            else
            {
                // If you reach this case, the maximum number of iterations
                // was not specified to be large enough or there is a problem
                // due to floating-point rounding errors.  If you believe the
                // latter is true, file a bug report.
                result.queryIsSuccessful = false;

                for (int i = 0; i < 2; ++i)
                {
                    result.rectangleParameter[i] = (Real)0;
                }
                for (int i = 0; i < 3; ++i)
                {
                    result.boxParameter[i] = (Real)0;
                    result.closestPoint[0][i] = (Real)0;
                    result.closestPoint[1][i] = (Real)0;
                }
                result.distance = (Real)0;
                result.sqrDistance = (Real)0;
            }

            result.numLCPIterations = mLCP.GetNumIterations();
            return result;
        }

    private:
        LCPSolver<Real, 10> mLCP;
    };
}
