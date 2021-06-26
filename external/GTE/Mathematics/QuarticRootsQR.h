// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/CubicRootsQR.h>

// An implementation of the QR algorithm described in "Matrix Computations,
// 2nd edition" by G. H. Golub and C. F. Van Loan, The Johns Hopkins
// University Press, Baltimore MD, Fourth Printing 1993.  In particular,
// the implementation is based on Chapter 7 (The Unsymmetric Eigenvalue
// Problem), Section 7.5 (The Practical QR Algorithm).  The algorithm is
// specialized for the companion matrix associated with a quartic polynomial.

namespace gte
{
    template <typename Real>
    class QuarticRootsQR
    {
    public:
        typedef std::array<std::array<Real, 4>, 4> Matrix;

        // Solve p(x) = c0 + c1 * x + c2 * x^2 + c3 * x^3 + x^4 = 0.
        uint32_t operator() (uint32_t maxIterations, Real c0, Real c1, Real c2, Real c3,
            uint32_t& numRoots, std::array<Real, 4>& roots) const
        {
            // Create the companion matrix for the polynomial.  The matrix is
            // in upper Hessenberg form.
            Matrix A;
            A[0][0] = (Real)0;
            A[0][1] = (Real)0;
            A[0][2] = (Real)0;
            A[0][3] = -c0;
            A[1][0] = (Real)1;
            A[1][1] = (Real)0;
            A[1][2] = (Real)0;
            A[1][3] = -c1;
            A[2][0] = (Real)0;
            A[2][1] = (Real)1;
            A[2][2] = (Real)0;
            A[2][3] = -c2;
            A[3][0] = (Real)0;
            A[3][1] = (Real)0;
            A[3][2] = (Real)1;
            A[3][3] = -c3;

            // Avoid the QR-cycle when c1 = c2 = 0 and avoid the slow
            // convergence when c1 and c2 are nearly zero.
            std::array<Real, 3> V{
                (Real)1,
                (Real)0.36602540378443865,
                (Real)0.36602540378443865 };
            DoIteration(V, A);

            return operator()(maxIterations, A, numRoots, roots);
        }

        // Compute the real eigenvalues of the upper Hessenberg matrix A.  The
        // matrix is modified by in-place operations, so if you need to
        // remember A, you must make your own copy before calling this
        // function.
        uint32_t operator() (uint32_t maxIterations, Matrix& A,
            uint32_t& numRoots, std::array<Real, 4>& roots) const
        {
            numRoots = 0;
            std::fill(roots.begin(), roots.end(), (Real)0);

            for (uint32_t numIterations = 0; numIterations < maxIterations; ++numIterations)
            {
                // Apply a Francis QR iteration.
                Real tr = A[2][2] + A[3][3];
                Real det = A[2][2] * A[3][3] - A[2][3] * A[3][2];
                std::array<Real, 3> X{
                    A[0][0] * A[0][0] + A[0][1] * A[1][0] - tr * A[0][0] + det,
                    A[1][0] * (A[0][0] + A[1][1] - tr),
                    A[1][0] * A[2][1] };
                std::array<Real, 3> V = House<3>(X);
                DoIteration(V, A);

                // Test for uncoupling of A.
                Real tr12 = A[1][1] + A[2][2];
                if (tr12 + A[2][1] == tr12)
                {
                    GetQuadraticRoots(0, 1, A, numRoots, roots);
                    GetQuadraticRoots(2, 3, A, numRoots, roots);
                    return numIterations;
                }

                Real tr01 = A[0][0] + A[1][1];
                if (tr01 + A[1][0] == tr01)
                {
                    numRoots = 1;
                    roots[0] = A[0][0];

                    // TODO: The cubic solver is not designed to process 3x3
                    // submatrices of an NxN matrix, so the copy of a
                    // submatrix of A to B is a simple workaround for running
                    // the solver.  Write general root-finding/ code that
                    // avoids such copying.
                    uint32_t subMaxIterations = maxIterations - numIterations;
                    typename CubicRootsQR<Real>::Matrix B;
                    for (int r = 0; r < 3; ++r)
                    {
                        for (int c = 0; c < 3; ++c)
                        {
                            B[r][c] = A[r + 1][c + 1];
                        }
                    }

                    uint32_t numSubroots = 0;
                    std::array<Real, 3> subroots;
                    uint32_t numSubiterations = CubicRootsQR<Real>()(subMaxIterations, B,
                        numSubroots, subroots);
                    for (uint32_t i = 0; i < numSubroots; ++i)
                    {
                        roots[numRoots++] = subroots[i];
                    }
                    return numIterations + numSubiterations;
                }

                Real tr23 = A[2][2] + A[3][3];
                if (tr23 + A[3][2] == tr23)
                {
                    numRoots = 1;
                    roots[0] = A[3][3];

                    // TODO: The cubic solver is not designed to process 3x3
                    // submatrices of an NxN matrix, so the copy of a
                    // submatrix of A to B is a simple workaround for running
                    // the solver.  Write general root-finding/ code that
                    // avoids such copying.
                    uint32_t subMaxIterations = maxIterations - numIterations;
                    typename CubicRootsQR<Real>::Matrix B;
                    for (int r = 0; r < 3; ++r)
                    {
                        for (int c = 0; c < 3; ++c)
                        {
                            B[r][c] = A[r][c];
                        }
                    }

                    uint32_t numSubroots = 0;
                    std::array<Real, 3> subroots;
                    uint32_t numSubiterations = CubicRootsQR<Real>()(subMaxIterations, B,
                        numSubroots, subroots);
                    for (uint32_t i = 0; i < numSubroots; ++i)
                    {
                        roots[numRoots++] = subroots[i];
                    }
                    return numIterations + numSubiterations;
                }
            }
            return maxIterations;
        }

    private:
        void DoIteration(std::array<Real, 3> const& V, Matrix& A) const
        {
            Real multV = (Real)-2 / (V[0] * V[0] + V[1] * V[1] + V[2] * V[2]);
            std::array<Real, 3> MV{ multV * V[0], multV * V[1], multV * V[2] };
            RowHouse<3>(0, 2, 0, 3, V, MV, A);
            ColHouse<3>(0, 3, 0, 2, V, MV, A);

            std::array<Real, 3> X{ A[1][0], A[2][0], A[3][0] };
            std::array<Real, 3> locV = House<3>(X);
            multV = (Real)-2 / (locV[0] * locV[0] + locV[1] * locV[1] + locV[2] * locV[2]);
            MV = { multV * locV[0], multV * locV[1], multV * locV[2] };
            RowHouse<3>(1, 3, 0, 3, locV, MV, A);
            ColHouse<3>(0, 3, 1, 3, locV, MV, A);

            std::array<Real, 2> Y{ A[2][1], A[3][1] };
            std::array<Real, 2> W = House<2>(Y);
            Real multW = (Real)-2 / (W[0] * W[0] + W[1] * W[1]);
            std::array<Real, 2> MW = { multW * W[0], multW * W[1] };
            RowHouse<2>(2, 3, 0, 3, W, MW, A);
            ColHouse<2>(0, 3, 2, 3, W, MW, A);
        }

        template <int N>
        std::array<Real, N> House(std::array<Real, N> const& X) const
        {
            std::array<Real, N> V;
            Real length = (Real)0;
            for (int i = 0; i < N; ++i)
            {
                length += X[i] * X[i];
            }
            length = std::sqrt(length);
            if (length != (Real)0)
            {
                Real sign = (X[0] >= (Real)0 ? (Real)1 : (Real)-1);
                Real denom = X[0] + sign * length;
                for (int i = 1; i < N; ++i)
                {
                    V[i] = X[i] / denom;
                }
            }
            else
            {
                V.fill((Real)0);
            }
            V[0] = (Real)1;
            return V;
        }

        template <int N>
        void RowHouse(int rmin, int rmax, int cmin, int cmax,
            std::array<Real, N> const& V, std::array<Real, N> const& MV, Matrix& A) const
        {
            // Only elements cmin through cmax are used.
            std::array<Real, 4> W;

            for (int c = cmin; c <= cmax; ++c)
            {
                W[c] = (Real)0;
                for (int r = rmin, k = 0; r <= rmax; ++r, ++k)
                {
                    W[c] += V[k] * A[r][c];
                }
            }

            for (int r = rmin, k = 0; r <= rmax; ++r, ++k)
            {
                for (int c = cmin; c <= cmax; ++c)
                {
                    A[r][c] += MV[k] * W[c];
                }
            }
        }

        template <int N>
        void ColHouse(int rmin, int rmax, int cmin, int cmax,
            std::array<Real, N> const& V, std::array<Real, N> const& MV, Matrix& A) const
        {
            // Only elements rmin through rmax are used.
            std::array<Real, 4> W;

            for (int r = rmin; r <= rmax; ++r)
            {
                W[r] = (Real)0;
                for (int c = cmin, k = 0; c <= cmax; ++c, ++k)
                {
                    W[r] += V[k] * A[r][c];
                }
            }

            for (int r = rmin; r <= rmax; ++r)
            {
                for (int c = cmin, k = 0; c <= cmax; ++c, ++k)
                {
                    A[r][c] += W[r] * MV[k];
                }
            }
        }

        void GetQuadraticRoots(int i0, int i1, Matrix const& A,
            uint32_t& numRoots, std::array<Real, 4>& roots) const
        {
            // Solve x^2 - t * x + d = 0, where t is the trace and d is the
            // determinant of the 2x2 matrix defined by indices i0 and i1.
            // The discriminant is D = (t/2)^2 - d.  When D >= 0, the roots
            // are real values t/2 - sqrt(D) and t/2 + sqrt(D).  To avoid
            // potential numerical issues with subtractive cancellation, the
            // roots are computed as
            //   r0 = t/2 + sign(t/2)*sqrt(D), r1 = trace - r0
            Real trace = A[i0][i0] + A[i1][i1];
            Real halfTrace = trace * (Real)0.5;
            Real determinant = A[i0][i0] * A[i1][i1] - A[i0][i1] * A[i1][i0];
            Real discriminant = halfTrace * halfTrace - determinant;
            if (discriminant >= (Real)0)
            {
                Real sign = (trace >= (Real)0 ? (Real)1 : (Real)-1);
                Real root = halfTrace + sign * std::sqrt(discriminant);
                roots[numRoots++] = root;
                roots[numRoots++] = trace - root;
            }
        }
    };
}
