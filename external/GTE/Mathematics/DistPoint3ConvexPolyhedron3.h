// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/LCPSolver.h>
#include <Mathematics/ConvexPolyhedron3.h>
#include <memory>

// Compute the distance between a point and a convex polyhedron in 3D.  The
// algorithm is based on using an LCP solver for the convex quadratic
// programming problem.  For details, see
// https://www.geometrictools.com/Documentation/ConvexQuadraticProgramming.pdf


namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Vector3<Real>, ConvexPolyhedron3<Real>>
    {
    public:
        // Construction.  If you have no knowledge of the number of faces
        // for the convex polyhedra you plan on applying the query to, pass
        // 'numTriangles' of zero.  This is a request to the operator()
        // function to create the LCP solver for each query, and this
        // requires memory allocation and deallocation per query.  If you
        // plan on applying the query multiple/ times to a single polyhedron,
        // even if the vertices of the polyhedron are modified for each query,
        // then pass 'numTriangles' to be the number of triangle faces for
        // that polyhedron.  This lets the operator() function know to create
        // the LCP solver once at construction time, thus avoiding the memory
        // management costs during the query.
        DCPQuery(int numTriangles = 0)
        {
            if (numTriangles > 0)
            {
                int const n = numTriangles + 3;
                mLCP = std::make_unique<LCPSolver<Real>>(n);
                mMaxLCPIterations = mLCP->GetMaxIterations();
            }
            else
            {
                mMaxLCPIterations = 0;
            }
        }

        struct Result
        {
            bool queryIsSuccessful;

            // These members are valid only when queryIsSuccessful is true;
            // otherwise, they are all set to zero.
            Real distance, sqrDistance;
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
            mMaxLCPIterations = maxLCPIterations;
            if (mLCP)
            {
                mLCP->SetMaxIterations(mMaxLCPIterations);
            }
        }

        Result operator()(Vector3<Real> const& point, ConvexPolyhedron3<Real> const& polyhedron)
        {
            Result result;

            int const numTriangles = static_cast<int>(polyhedron.planes.size());
            if (numTriangles == 0)
            {
                // The polyhedron planes and aligned box need to be created.
                result.queryIsSuccessful = false;
                for (int i = 0; i < 3; ++i)
                {
                    result.closestPoint[0][i] = (Real)0;
                    result.closestPoint[1][i] = (Real)0;
                }
                result.distance = (Real)0;
                result.sqrDistance = (Real)0;
                result.numLCPIterations = 0;
                return result;
            }

            int const n = numTriangles + 3;

            // Translate the point and convex polyhedron so that the
            // polyhedron is in the first octant.  The translation is not
            // explicit; rather, the q and M for the LCP are initialized using
            // the translation information.
            Vector4<Real> hmin = HLift(polyhedron.alignedBox.min, (Real)1);

            std::vector<Real> q(n);
            for (int r = 0; r < 3; ++r)
            {
                q[r] = polyhedron.alignedBox.min[r] - point[r];
            }
            for (int r = 3, t = 0; r < n; ++r, ++t)
            {
                q[r] = -Dot(polyhedron.planes[t], hmin);
            }

            std::vector<Real> M(n * n);
            M[0] = (Real)1;  M[1] = (Real)0;  M[2] = (Real)0;
            M[n] = (Real)0;  M[n + 1] = (Real)1;  M[n + 2] = (Real)0;
            M[2 * n] = (Real)0;  M[2 * n + 1] = (Real)0;  M[2 * n + 2] = (Real)1;
            for (int t = 0, c = 3; t < numTriangles; ++t, ++c)
            {
                Vector3<Real> normal = HProject(polyhedron.planes[t]);
                for (int r = 0; r < 3; ++r)
                {
                    M[c + n * r] = normal[r];
                    M[r + n * c] = -normal[r];
                }
            }
            for (int r = 3; r < n; ++r)
            {
                for (int c = 3; c < n; ++c)
                {
                    M[c + n * r] = (Real)0;
                }
            }

            bool needsLCP = (mLCP == nullptr);
            if (needsLCP)
            {
                mLCP = std::make_unique<LCPSolver<Real>>(n);
                if (mMaxLCPIterations > 0)
                {
                    mLCP->SetMaxIterations(mMaxLCPIterations);
                }
            }

            std::vector<Real> w(n), z(n);
            if (mLCP->Solve(q, M, w, z))
            {
                result.queryIsSuccessful = true;
                result.closestPoint[0] = point;
                for (int i = 0; i < 3; ++i)
                {
                    result.closestPoint[1][i] = z[i] + polyhedron.alignedBox.min[i];
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
            }

            result.numLCPIterations = mLCP->GetNumIterations();
            if (needsLCP)
            {
                mLCP = nullptr;
            }
            return result;
        }

    private:
        int mMaxLCPIterations;
        std::unique_ptr<LCPSolver<Real>> mLCP;
    };
}
