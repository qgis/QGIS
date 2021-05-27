// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix2x2.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Matrix4x4.h>
#include <Mathematics/GaussianElimination.h>
#include <map>

// Solve linear systems of equations where the matrix A is NxN.  The return
// value of a function is 'true' when A is invertible.  In this case the
// solution X and the solution is valid.  If the return value is 'false', A
// is not invertible and X and Y are invalid, so do not use them.  When a
// matrix is passed as Real*, the storage order is assumed to be the one
// consistent with your choice of GTE_USE_ROW_MAJOR or GTE_USE_COL_MAJOR.
//
// The linear solvers that use the conjugate gradient algorithm are based
// on the discussion in "Matrix Computations, 2nd edition" by G. H. Golub
// and Charles F. Van Loan, The Johns Hopkins Press, Baltimore MD, Fourth
// Printing 1993.

namespace gte
{
    template <typename Real>
    class LinearSystem
    {
    public:
        // Solve 2x2, 3x3, and 4x4 systems by inverting the matrix directly.
        // This avoids the overhead of Gaussian elimination in small
        // dimensions.
        static bool Solve(Matrix2x2<Real> const& A, Vector2<Real> const& B, Vector2<Real>& X)
        {
            bool invertible;
            Matrix2x2<Real> invA = Inverse(A, &invertible);
            if (invertible)
            {
                X = invA * B;
            }
            else
            {
                X = Vector2<Real>::Zero();
            }
            return invertible;
        }

        static bool Solve(Matrix3x3<Real> const& A, Vector3<Real> const& B, Vector3<Real>& X)
        {
            bool invertible;
            Matrix3x3<Real> invA = Inverse(A, &invertible);
            if (invertible)
            {
                X = invA * B;
            }
            else
            {
                X = Vector3<Real>::Zero();
            }
            return invertible;
        }

        static bool Solve(Matrix4x4<Real> const& A, Vector4<Real> const& B, Vector4<Real>& X)
        {
            bool invertible;
            Matrix4x4<Real> invA = Inverse(A, &invertible);
            if (invertible)
            {
                X = invA * B;
            }
            else
            {
                X = Vector4<Real>::Zero();
            }
            return invertible;
        }

        // Solve A*X = B, where B is Nx1 and the solution X is Nx1.
        static bool Solve(int N, Real const* A, Real const* B, Real* X)
        {
            Real determinant;
            return GaussianElimination<Real>()(N, A, nullptr, determinant, B, X,
                nullptr, 0, nullptr);
        }

        // Solve A*X = B, where B is NxM and the solution X is NxM.
        static bool Solve(int N, int M, Real const* A, Real const* B, Real* X)
        {
            Real determinant;
            return GaussianElimination<Real>()(N, A, nullptr, determinant, nullptr,
                nullptr, B, M, X);
        }

        // Solve A*X = B, where A is tridiagonal.  The function expects the
        // subdiagonal, diagonal, and superdiagonal of A.  The diagonal input
        // must have N elements.  The subdiagonal and superdiagonal inputs
        // must have N-1 elements.
        static bool SolveTridiagonal(int N, Real const* subdiagonal,
            Real const* diagonal, Real const* superdiagonal, Real const* B, Real* X)
        {
            if (diagonal[0] == (Real)0)
            {
                return false;
            }

            std::vector<Real> tmp(N - 1);
            Real expr = diagonal[0];
            Real invExpr = ((Real)1) / expr;
            X[0] = B[0] * invExpr;

            int i0, i1;
            for (i0 = 0, i1 = 1; i1 < N; ++i0, ++i1)
            {
                tmp[i0] = superdiagonal[i0] * invExpr;
                expr = diagonal[i1] - subdiagonal[i0] * tmp[i0];
                if (expr == (Real)0)
                {
                    return false;
                }
                invExpr = ((Real)1) / expr;
                X[i1] = (B[i1] - subdiagonal[i0] * X[i0]) * invExpr;
            }

            for (i0 = N - 1, i1 = N - 2; i1 >= 0; --i0, --i1)
            {
                X[i1] -= tmp[i1] * X[i0];
            }
            return true;
        }

        // Solve A*X = B, where A is tridiagonal.  The function expects the
        // subdiagonal, diagonal, and superdiagonal of A.  Moreover, the
        // subdiagonal elements are a constant, the diagonal elements are a
        // constant, and the superdiagonal elements are a constant.
        static bool SolveConstantTridiagonal(int N, Real subdiagonal,
            Real diagonal, Real superdiagonal, Real const* B, Real* X)
        {
            if (diagonal == (Real)0)
            {
                return false;
            }

            std::vector<Real> tmp(N - 1);
            Real expr = diagonal;
            Real invExpr = ((Real)1) / expr;
            X[0] = B[0] * invExpr;

            int i0, i1;
            for (i0 = 0, i1 = 1; i1 < N; ++i0, ++i1)
            {
                tmp[i0] = superdiagonal * invExpr;
                expr = diagonal - subdiagonal * tmp[i0];
                if (expr == (Real)0)
                {
                    return false;
                }
                invExpr = ((Real)1) / expr;
                X[i1] = (B[i1] - subdiagonal * X[i0]) * invExpr;
            }

            for (i0 = N - 1, i1 = N - 2; i1 >= 0; --i0, --i1)
            {
                X[i1] -= tmp[i1] * X[i0];
            }
            return true;
        }

        // Solve A*X = B using the conjugate gradient method, where A is
        // symmetric.  You must specify the maximum number of iterations and a
        // tolerance for terminating the iterations.  Reasonable choices for
        // tolerance are 1e-06f for 'float' or 1e-08 for 'double'.
        static unsigned int SolveSymmetricCG(int N, Real const* A, Real const* B,
            Real* X, unsigned int maxIterations, Real tolerance)
        {
            // The first iteration.
            std::vector<Real> tmpR(N), tmpP(N), tmpW(N);
            Real* R = &tmpR[0];
            Real* P = &tmpP[0];
            Real* W = &tmpW[0];
            size_t numBytes = N * sizeof(Real);
            std::memset(X, 0, numBytes);
            std::memcpy(R, B, numBytes);
            Real rho0 = Dot(N, R, R);
            std::memcpy(P, R, numBytes);
            Mul(N, A, P, W);
            Real alpha = rho0 / Dot(N, P, W);
            UpdateX(N, X, alpha, P);
            UpdateR(N, R, alpha, W);
            Real rho1 = Dot(N, R, R);

            // The remaining iterations.
            unsigned int iteration;
            for (iteration = 1; iteration <= maxIterations; ++iteration)
            {
                Real root0 = std::sqrt(rho1);
                Real norm = Dot(N, B, B);
                Real root1 = std::sqrt(norm);
                if (root0 <= tolerance * root1)
                {
                    break;
                }

                Real beta = rho1 / rho0;
                UpdateP(N, P, beta, R);
                Mul(N, A, P, W);
                alpha = rho1 / Dot(N, P, W);
                UpdateX(N, X, alpha, P);
                UpdateR(N, R, alpha, W);
                rho0 = rho1;
                rho1 = Dot(N, R, R);
            }
            return iteration;
        }

        // Solve A*X = B using the conjugate gradient method, where A is 
        // sparse and symmetric.  The nonzero entries of the symmetrix matrix
        // A are stored in a map whose keys are pairs (i,j) and whose values
        // are real numbers.  The pair (i,j) is the location of the value in
        // the array.  Only one of (i,j) and (j,i) should be stored since A is
        // symmetric.  The column vector B is stored as an array of contiguous
        // values.  You must specify the maximum number of iterations and a
        // tolerance for terminating the iterations.  Reasonable choices for
        // tolerance are 1e-06f for 'float' or 1e-08 for 'double'.
        typedef std::map<std::array<int, 2>, Real> SparseMatrix;
        static unsigned int SolveSymmetricCG(int N, SparseMatrix const& A,
            Real const* B, Real* X, unsigned int maxIterations, Real tolerance)
        {
            // The first iteration.
            std::vector<Real> tmpR(N), tmpP(N), tmpW(N);
            Real* R = &tmpR[0];
            Real* P = &tmpP[0];
            Real* W = &tmpW[0];
            size_t numBytes = N * sizeof(Real);
            std::memset(X, 0, numBytes);
            std::memcpy(R, B, numBytes);
            Real rho0 = Dot(N, R, R);
            std::memcpy(P, R, numBytes);
            Mul(N, A, P, W);
            Real alpha = rho0 / Dot(N, P, W);
            UpdateX(N, X, alpha, P);
            UpdateR(N, R, alpha, W);
            Real rho1 = Dot(N, R, R);

            // The remaining iterations.
            unsigned int iteration;
            for (iteration = 1; iteration <= maxIterations; ++iteration)
            {
                Real root0 = std::sqrt(rho1);
                Real norm = Dot(N, B, B);
                Real root1 = std::sqrt(norm);
                if (root0 <= tolerance * root1)
                {
                    break;
                }

                Real beta = rho1 / rho0;
                UpdateP(N, P, beta, R);
                Mul(N, A, P, W);
                alpha = rho1 / Dot(N, P, W);
                UpdateX(N, X, alpha, P);
                UpdateR(N, R, alpha, W);
                rho0 = rho1;
                rho1 = Dot(N, R, R);
            }
            return iteration;
        }

    private:
        // Support for the conjugate gradient method.
        static Real Dot(int N, Real const* U, Real const* V)
        {
            Real dot = (Real)0;
            for (int i = 0; i < N; ++i)
            {
                dot += U[i] * V[i];
            }
            return dot;
        }

        static void Mul(int N, Real const* A, Real const* X, Real* P)
        {
#if defined(GTE_USE_ROW_MAJOR)
            LexicoArray2<true, Real> matA(N, N, const_cast<Real*>(A));
#else
            LexicoArray2<false, Real> matA(N, N, const_cast<Real*>(A));
#endif

            std::memset(P, 0, N * sizeof(Real));
            for (int row = 0; row < N; ++row)
            {
                for (int col = 0; col < N; ++col)
                {
                    P[row] += matA(row, col) * X[col];
                }
            }
        }

        static void Mul(int N, SparseMatrix const& A, Real const* X, Real* P)
        {
            std::memset(P, 0, N * sizeof(Real));
            for (auto const& element : A)
            {
                int i = element.first[0];
                int j = element.first[1];
                Real value = element.second;
                P[i] += value * X[j];
                if (i != j)
                {
                    P[j] += value * X[i];
                }
            }
        }

        static void UpdateX(int N, Real* X, Real alpha, Real const* P)
        {
            for (int i = 0; i < N; ++i)
            {
                X[i] += alpha * P[i];
            }
        }

        static void UpdateR(int N, Real* R, Real alpha, Real const* W)
        {
            for (int i = 0; i < N; ++i)
            {
                R[i] -= alpha * W[i];
            }
        }

        static void UpdateP(int N, Real* P, Real beta, Real const* R)
        {
            for (int i = 0; i < N; ++i)
            {
                P[i] = R[i] + beta * P[i];
            }
        }
    };
}
