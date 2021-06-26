// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/GMatrix.h>

namespace gte
{
    // Implementation for size known at compile time.
    template <typename Real, int N = 0>
    class CholeskyDecomposition
    {
    public:
        // Ensure that N > 0 at compile time.
        CholeskyDecomposition()
        {
            static_assert(N > 0, "Invalid size in CholeskyDecomposition constructor.");
        }

        // Disallow copies and moves.
        CholeskyDecomposition(CholeskyDecomposition const&) = delete;
        CholeskyDecomposition& operator=(CholeskyDecomposition const&) = delete;
        CholeskyDecomposition(CholeskyDecomposition&&) = delete;
        CholeskyDecomposition& operator=(CholeskyDecomposition&&) = delete;

        // On input, A is symmetric.  Only the lower-triangular portion is
        // modified.  On output, the lower-triangular portion is L where
        // A = L * L^T.
        bool Factor(Matrix<N, N, Real>& A)
        {
            for (int c = 0; c < N; ++c)
            {
                if (A(c, c) <= (Real)0)
                {
                    return false;
                }
                A(c, c) = std::sqrt(A(c, c));

                for (int r = c + 1; r < N; ++r)
                {
                    A(r, c) /= A(c, c);
                }

                for (int k = c + 1; k < N; ++k)
                {
                    for (int r = k; r < N; ++r)
                    {
                        A(r, k) -= A(r, c) * A(k, c);
                    }
                }
            }
            return true;
        }

        // Solve L*Y = B, where L is lower triangular and invertible.  The
        // input value of Y is B.  On output, Y is the solution.
        void SolveLower(Matrix<N, N, Real> const& L, Vector<N, Real>& Y)
        {
            for (int r = 0; r < N; ++r)
            {
                for (int c = 0; c < r; ++c)
                {
                    Y[r] -= L(r, c) * Y[c];
                }
                Y[r] /= L(r, r);
            }
        }

        // Solve L^T*X = Y, where L is lower triangular (L^T is upper
        // triangular) and invertible.  The input value of X is Y.  On
        // output, X is the solution.
        void SolveUpper(Matrix<N, N, Real> const& L, Vector<N, Real>& X)
        {
            for (int r = N - 1; r >= 0; --r)
            {
                for (int c = r + 1; c < N; ++c)
                {
                    X[r] -= L(c, r) * X[c];
                }
                X[r] /= L(r, r);
            }
        }
    };

    // Implementation for size known only at run time.
    template <typename Real>
    class CholeskyDecomposition<Real, 0>
    {
    public:
        int const N;

        // Ensure that N > 0 at run time.
        CholeskyDecomposition(int n)
            :
            N(n)
        {
        }

        // Disallow copies and moves.  This is required to avoid compiler
        // complaints about the 'int const N' member.
        CholeskyDecomposition(CholeskyDecomposition const&) = delete;
        CholeskyDecomposition& operator=(CholeskyDecomposition const&) = delete;
        CholeskyDecomposition(CholeskyDecomposition&&) = delete;
        CholeskyDecomposition& operator=(CholeskyDecomposition&&) = delete;

        // On input, A is symmetric.  Only the lower-triangular portion is
        // modified.  On output, the lower-triangular portion is L where
        // A = L * L^T.
        bool Factor(GMatrix<Real>& A)
        {
            if (A.GetNumRows() == N && A.GetNumCols() == N)
            {
                for (int c = 0; c < N; ++c)
                {
                    if (A(c, c) <= (Real)0)
                    {
                        return false;
                    }
                    A(c, c) = std::sqrt(A(c, c));

                    for (int r = c + 1; r < N; ++r)
                    {
                        A(r, c) /= A(c, c);
                    }

                    for (int k = c + 1; k < N; ++k)
                    {
                        for (int r = k; r < N; ++r)
                        {
                            A(r, k) -= A(r, c) * A(k, c);
                        }
                    }
                }
                return true;
            }
            LogError("Matrix must be square.");
        }

        // Solve L*Y = B, where L is lower triangular and invertible.  The
        // input value of Y is B.  On output, Y is the solution.
        void SolveLower(GMatrix<Real> const& L, GVector<Real>& Y)
        {
            if (L.GetNumRows() == N && L.GetNumCols() == N && Y.GetSize() == N)
            {
                for (int r = 0; r < N; ++r)
                {
                    for (int c = 0; c < r; ++c)
                    {
                        Y[r] -= L(r, c) * Y[c];
                    }
                    Y[r] /= L(r, r);
                }
                return;
            }
            LogError("Invalid size.");
        }

        // Solve L^T*X = Y, where L is lower triangular (L^T is upper
        // triangular) and invertible.  The input value of X is Y.  On
        // output, X is the solution.
        void SolveUpper(GMatrix<Real> const& L, GVector<Real>& X)
        {
            if (L.GetNumRows() == N && L.GetNumCols() == N && X.GetSize() == N)
            {
                for (int r = N - 1; r >= 0; --r)
                {
                    for (int c = r + 1; c < N; ++c)
                    {
                        X[r] -= L(c, r) * X[c];
                    }
                    X[r] /= L(r, r);
                }
            }
            else
            {
                LogError("Invalid size.");
            }
        }
    };


    // Implementation for sizes known at compile time.
    template <typename Real, int BlockSize = 0, int NumBlocks = 0>
    class BlockCholeskyDecomposition
    {
    public:
        // Let B represent the block size and N represent the number of
        // blocks.  The matrix A is (N*B)-by-(N*B) but partitioned into an
        // N-by-N matrix of blocks, each block of size B-by-B.  The value
        // N*B is NumDimensions.
        enum
        {
            NumDimensions = NumBlocks * BlockSize
        };

        typedef std::array<Vector<BlockSize, Real>, NumBlocks> BlockVector;
        typedef std::array<std::array<Matrix<BlockSize, BlockSize, Real>, NumBlocks>, NumBlocks> BlockMatrix;

        // Ensure that BlockSize > 0 and NumBlocks > 0 at compile time.
        BlockCholeskyDecomposition()
        {
            static_assert(BlockSize > 0 && NumBlocks > 0, "Invalid size in BlockCholeskyDecomposition constructor.");
        }

        // Disallow copies and moves.
        BlockCholeskyDecomposition(BlockCholeskyDecomposition const&) = delete;
        BlockCholeskyDecomposition& operator=(BlockCholeskyDecomposition const&) = delete;
        BlockCholeskyDecomposition(BlockCholeskyDecomposition&&) = delete;
        BlockCholeskyDecomposition& operator=(BlockCholeskyDecomposition&&) = delete;

        // Treating the matrix as a 2D table of scalars with NUM_DIMENSIONS
        // rows and NUM_DIMENSIONS columns, look up the correct block that
        // stores the requested element and return a reference.
        Real Get(BlockMatrix const& M, int row, int col)
        {
            int b0 = col / BlockSize, b1 = row / BlockSize;
            int i0 = col % BlockSize, i1 = row % BlockSize;
            auto const& block = M[b1][b0];
            return block(i1, i0);
        }

        void Set(BlockMatrix& M, int row, int col, Real value)
        {
            int b0 = col / BlockSize, b1 = row / BlockSize;
            int i0 = col % BlockSize, i1 = row % BlockSize;
            auto& block = M[b1][b0];
            block(i1, i0) = value;
        }

        bool Factor(BlockMatrix& A)
        {
            for (int c = 0; c < NumBlocks; ++c)
            {
                if (!mDecomposer.Factor(A[c][c]))
                {
                    return false;
                }

                for (int r = c + 1; r < NumBlocks; ++r)
                {
                    LowerTriangularSolver(r, c, A);
                }

                for (int k = c + 1; k < NumBlocks; ++k)
                {
                    for (int r = k; r < NumBlocks; ++r)
                    {
                        SubtractiveUpdate(r, k, c, A);
                    }
                }
            }
            return true;
        }

        // Solve L*Y = B, where L is an invertible lower-triangular block
        // matrix whose diagonal blocks are lower-triangular matrices.
        // The input B is a block vector of commensurate size.  The input
        // value of Y is B.  On output, Y is the solution.
        void SolveLower(BlockMatrix const& L, BlockVector& Y)
        {
            for (int r = 0; r < NumBlocks; ++r)
            {
                auto& Yr = Y[r];
                for (int c = 0; c < r; ++c)
                {
                    auto const& Lrc = L[r][c];
                    auto const& Yc = Y[c];
                    for (int i = 0; i < BlockSize; ++i)
                    {
                        for (int j = 0; j < BlockSize; ++j)
                        {
                            Yr[i] -= Lrc(i, j) * Yc[j];
                        }
                    }
                }
                mDecomposer.SolveLower(L[r][r], Yr);
            }
        }

        // Solve L^T*X = Y, where L is an invertible lower-triangular block
        // matrix (L^T is an upper-triangular block matrix) whose diagonal
        // blocks are lower-triangular matrices.  The input value of X is Y.
        // On output, X is the solution.
        void SolveUpper(BlockMatrix const& L, BlockVector& X)
        {
            for (int r = NumBlocks - 1; r >= 0; --r)
            {
                auto& Xr = X[r];
                for (int c = r + 1; c < NumBlocks; ++c)
                {
                    auto const& Lcr = L[c][r];
                    auto const& Xc = X[c];
                    for (int i = 0; i < BlockSize; ++i)
                    {
                        for (int j = 0; j < BlockSize; ++j)
                        {
                            Xr[i] -= Lcr(j, i) * Xc[j];
                        }
                    }
                }
                mDecomposer.SolveUpper(L[r][r], Xr);
            }
        }

    private:
        // Solve G(c,c)*G(r,c)^T = A(r,c)^T for G(r,c).  The matrices
        // G(c,c) and A(r,c) are known quantities, and G(c,c) occupies
        // the lower triangular portion of A(c,c).  The solver stores
        // its results in-place, so A(r,c) stores the G(r,c) result.
        void LowerTriangularSolver(int r, int c, BlockMatrix& A)
        {
            auto const& Acc = A[c][c];
            auto& Arc = A[r][c];
            for (int j = 0; j < BlockSize; ++j)
            {
                for (int i = 0; i < j; ++i)
                {
                    Real Lji = Acc(j, i);
                    for (int k = 0; k < BlockSize; ++k)
                    {
                        Arc(k, j) -= Lji * Arc(k, i);
                    }
                }

                Real Ljj = Acc(j, j);
                for (int k = 0; k < BlockSize; ++k)
                {
                    Arc(k, j) /= Ljj;
                }
            }
        }

        void SubtractiveUpdate(int r, int k, int c, BlockMatrix& A)
        {
            auto const& Arc = A[r][c];
            auto const& Akc = A[k][c];
            auto& Ark = A[r][k];
            for (int j = 0; j < BlockSize; ++j)
            {
                for (int i = 0; i < BlockSize; ++i)
                {
                    for (int m = 0; m < BlockSize; ++m)
                    {
                        Ark(j, i) -= Arc(j, m) * Akc(i, m);
                    }
                }
            }
        }

        CholeskyDecomposition<Real, BlockSize> mDecomposer;
    };

    // Implementation for sizes known only at run time.
    template <typename Real>
    class BlockCholeskyDecomposition<Real, 0, 0>
    {
    public:
        // Let B represent the block size and N represent the number of
        // blocks.  The matrix A is (N*B)-by-(N*B) but partitioned into an
        // N-by-N matrix of blocks, each block of size B-by-B.  The value
        // N*B is NumDimensions.
        int const BlockSize;
        int const NumBlocks;
        int const NumDimensions;

        // The number of elements in a BlockVector object must be NumBlocks
        // and each GVector element has BlockSize components.
        typedef std::vector<GVector<Real>> BlockVector;

        // The BlockMatrix is an array of NumBlocks-by-NumBlocks matrices.
        // Each block matrix is stored in row-major order.  The BlockMatrix
        // elements themselves are stored in row-major order.  The block
        // matrix element M = BlockMatrix[col + NumBlocks * row] is of size
        // BlockSize-by-BlockSize (in row-major order) and is in the (row,col)
        // location of the full matrix of blocks.
        typedef std::vector<GMatrix<Real>> BlockMatrix;

        // Ensure that BlockSize > 0 and NumDimensions > 0 at run time.
        BlockCholeskyDecomposition(int blockSize, int numBlocks)
            :
            BlockSize(blockSize),
            NumBlocks(numBlocks),
            NumDimensions(numBlocks * blockSize),
            mDecomposer(blockSize)
        {
            LogAssert(blockSize > 0 && numBlocks > 0, "Invalid input.");
        }

        // Disallow copies and moves.  This is required to avoid compiler
        // complaints about the 'int const' members.
        BlockCholeskyDecomposition(BlockCholeskyDecomposition const&) = delete;
        BlockCholeskyDecomposition& operator=(BlockCholeskyDecomposition const&) = delete;
        BlockCholeskyDecomposition(BlockCholeskyDecomposition&&) = delete;
        BlockCholeskyDecomposition& operator=(BlockCholeskyDecomposition&&) = delete;

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and return a reference.
        Real Get(BlockMatrix const& M, int row, int col)
        {
            int b0 = col / BlockSize, b1 = row / BlockSize;
            int i0 = col % BlockSize, i1 = row % BlockSize;
            auto const& block = M[GetIndex(b1, b0)];
            return block(i1, i0);
        }

        void Set(BlockMatrix& M, int row, int col, Real value)
        {
            int b0 = col / BlockSize, b1 = row / BlockSize;
            int i0 = col % BlockSize, i1 = row % BlockSize;
            auto& block = M[GetIndex(b1, b0)];
            block(i1, i0) = value;
        }

        bool Factor(BlockMatrix& A)
        {
            for (int c = 0; c < NumBlocks; ++c)
            {
                if (!mDecomposer.Factor(A[GetIndex(c, c)]))
                {
                    return false;
                }

                for (int r = c + 1; r < NumBlocks; ++r)
                {
                    LowerTriangularSolver(r, c, A);
                }

                for (int k = c + 1; k < NumBlocks; ++k)
                {
                    for (int r = k; r < NumBlocks; ++r)
                    {
                        SubtractiveUpdate(r, k, c, A);
                    }
                }
            }
            return true;
        }

        // Solve L*Y = B, where L is an invertible lower-triangular block
        // matrix whose diagonal blocks are lower-triangular matrices.
        // The input B is a block vector of commensurate size.  The input
        // value of Y is B.  On output, Y is the solution.
        void SolveLower(BlockMatrix const& L, BlockVector& Y)
        {
            for (int r = 0; r < NumBlocks; ++r)
            {
                auto& Yr = Y[r];
                for (int c = 0; c < r; ++c)
                {
                    auto const& Lrc = L[GetIndex(r, c)];
                    auto const& Yc = Y[c];
                    for (int i = 0; i < NumBlocks; ++i)
                    {
                        for (int j = 0; j < NumBlocks; ++j)
                        {
                            Yr[i] -= Lrc[GetIndex(i, j)] * Yc[j];
                        }
                    }
                }
                mDecomposer.SolveLower(L[GetIndex(r, r)], Yr);
            }
        }

        // Solve L^T*X = Y, where L is an invertible lower-triangular block
        // matrix (L^T is an upper-triangular block matrix) whose diagonal
        // blocks are lower-triangular matrices.  The input value of X is Y.
        // On output, X is the solution.
        void SolveUpper(BlockMatrix const& L, BlockVector& X)
        {
            for (int r = NumBlocks - 1; r >= 0; --r)
            {
                auto& Xr = X[r];
                for (int c = r + 1; c < NumBlocks; ++c)
                {
                    auto const& Lcr = L[GetIndex(c, r)];
                    auto const& Xc = X[c];
                    for (int i = 0; i < BlockSize; ++i)
                    {
                        for (int j = 0; j < BlockSize; ++j)
                        {
                            Xr[i] -= Lcr[GetIndex(j, i)] * Xc[j];
                        }
                    }
                }
                mDecomposer.SolveUpper(L[GetIndex(r, r)], Xr);
            }
        }

    private:
        // Compute the 1-dimensional index of the block matrix in a
        // 2-dimensional BlockMatrix object.
        inline int GetIndex(int row, int col) const
        {
            return col + row * NumBlocks;
        }

        // Solve G(c,c)*G(r,c)^T = A(r,c)^T for G(r,c).  The matrices
        // G(c,c) and A(r,c) are known quantities, and G(c,c) occupies
        // the lower triangular portion of A(c,c).  The solver stores
        // its results in-place, so A(r,c) stores the G(r,c) result.
        void LowerTriangularSolver(int r, int c, BlockMatrix& A)
        {
            auto const& Acc = A[GetIndex(c, c)];
            auto& Arc = A[GetIndex(r, c)];
            for (int j = 0; j < BlockSize; ++j)
            {
                for (int i = 0; i < j; ++i)
                {
                    Real Lji = Acc[GetIndex(j, i)];
                    for (int k = 0; k < BlockSize; ++k)
                    {
                        Arc[GetIndex(k, j)] -= Lji * Arc[GetIndex(k, i)];
                    }
                }

                Real Ljj = Acc[GetIndex(j, j)];
                for (int k = 0; k < BlockSize; ++k)
                {
                    Arc[GetIndex(k, j)] /= Ljj;
                }
            }
        }

        void SubtractiveUpdate(int r, int k, int c, BlockMatrix& A)
        {
            auto const& Arc = A[GetIndex(r, c)];
            auto const& Akc = A[GetIndex(k, c)];
            auto& Ark = A[GetIndex(r, k)];
            for (int j = 0; j < BlockSize; ++j)
            {
                for (int i = 0; i < BlockSize; ++i)
                {
                    for (int m = 0; m < BlockSize; ++m)
                    {
                        Ark[GetIndex(j, i)] -= Arc[GetIndex(j, m)] * Akc[GetIndex(i, m)];
                    }
                }
            }
        }

        // The decomposer has size BlockSize.
        CholeskyDecomposition<Real> mDecomposer;
    };
}
