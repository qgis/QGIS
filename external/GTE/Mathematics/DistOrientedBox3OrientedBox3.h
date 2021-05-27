// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/LCPSolver.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector3.h>

// Compute the distance between oriented boxes in 3D.  The algorithm is based
// on using an LCP solver for the convex quadratic programming problem.  For
// details, see
// https://www.geometrictools.com/Documentation/ConvexQuadraticProgramming.pdf

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, OrientedBox3<Real>, OrientedBox3<Real>>
    {
    public:
        struct Result
        {
            bool queryIsSuccessful;

            // These members are valid only when queryIsSuccessful is true;
            // otherwise, they are all set to zero.
            Real distance, sqrDistance;
            std::array<Real, 3> box0Parameter, box1Parameter;
            Vector3<Real> closestPoint[2];

            // The number of iterations used by LCPSolver regardless of
            // whether the query is successful.
            int numLCPIterations;
        };

        // Default maximum iterations is 144 (n = 12, maxIterations = n*n).
        // If the solver fails to converge, try increasing the maximum number
        // of iterations.
        void SetMaxLCPIterations(int maxLCPIterations)
        {
            mLCP.SetMaxIterations(maxLCPIterations);
        }

        Result operator()(OrientedBox3<Real> const& box0, OrientedBox3<Real> const& box1)
        {
            Result result;

            // Translate the center of box0 to the origin.  Modify the
            // oriented box coefficients to be nonnegative.
            Vector3<Real> delta = box1.center - box0.center;
            for (int i = 0; i < 3; ++i)
            {
                delta += box0.extent[i] * box0.axis[i];
                delta -= box1.extent[i] * box1.axis[i];
            }

            Vector3<Real> R0Delta, R1Delta;
            for (int i = 0; i < 3; ++i)
            {
                R0Delta[i] = Dot(box0.axis[i], delta);
                R1Delta[i] = Dot(box1.axis[i], delta);
            }

            std::array<std::array<Real, 3>, 3> R0TR1;
            for (int r = 0; r < 3; ++r)
            {
                for (int c = 0; c < 3; ++c)
                {
                    R0TR1[r][c] = Dot(box0.axis[r], box1.axis[c]);
                }
            }

            Vector3<Real> twoExtent0 = box0.extent * (Real)2;
            Vector3<Real> twoExtent1 = box1.extent * (Real)2;

            // The LCP has 6 variables and 6 (nontrivial) inequality
            // constraints.
            std::array<Real, 12> q =
            {
                -R0Delta[0], -R0Delta[1], -R0Delta[2], R1Delta[0], R1Delta[1], R1Delta[2],
                twoExtent0[0], twoExtent0[1], twoExtent0[2], twoExtent1[0], twoExtent1[1], twoExtent1[2]
            };

            std::array<std::array<Real, 12>, 12> M;
            {
                Real const z = (Real)0;
                Real const p = (Real)1;
                Real const m = (Real)-1;
                M[0] = { p, z, z, -R0TR1[0][0], -R0TR1[0][1], -R0TR1[0][2], p, z, z, z, z, z };
                M[1] = { z, p, z, -R0TR1[1][0], -R0TR1[1][1], -R0TR1[1][2], z, p, z, z, z, z };
                M[2] = { z, z, p, -R0TR1[2][0], -R0TR1[2][1], -R0TR1[2][2], z, z, p, z, z, z };
                M[3] = { -R0TR1[0][0], -R0TR1[1][0], -R0TR1[2][0], p, z, z, z, z, z, p, z, z };
                M[4] = { -R0TR1[0][1], -R0TR1[1][1], -R0TR1[2][1], z, p, z, z, z, z, z, p, z };
                M[5] = { -R0TR1[0][2], -R0TR1[1][2], -R0TR1[2][2], z, z, p, z, z, z, z, z, p };
                M[6] = { m, z, z, z, z, z, z, z, z, z, z, z };
                M[7] = { z, m, z, z, z, z, z, z, z, z, z, z };
                M[8] = { z, z, m, z, z, z, z, z, z, z, z, z };
                M[9] = { z, z, z, m, z, z, z, z, z, z, z, z };
                M[10] = { z, z, z, z, m, z, z, z, z, z, z, z };
                M[11] = { z, z, z, z, z, m, z, z, z, z, z, z };
            }

            std::array<Real, 12> w, z;
            if (mLCP.Solve(q, M, w, z))
            {
                result.queryIsSuccessful = true;

                result.closestPoint[0] = box0.center;
                for (int i = 0; i < 3; ++i)
                {
                    result.box0Parameter[i] = z[i] - box0.extent[i];
                    result.closestPoint[0] += result.box0Parameter[i] * box0.axis[i];
                }

                result.closestPoint[1] = box1.center;
                for (int i = 0, j = 3; i < 3; ++i, ++j)
                {
                    result.box1Parameter[i] = z[j] - box1.extent[i];
                    result.closestPoint[1] += result.box1Parameter[i] * box1.axis[i];
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

                for (int i = 0; i < 3; ++i)
                {
                    result.box0Parameter[i] = (Real)0;
                    result.box1Parameter[i] = (Real)0;
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
        LCPSolver<Real, 12> mLCP;
    };
}
