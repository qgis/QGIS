// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.9.2021.05.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/GMatrix.h>

// Factor a positive symmetric matrix A = L * D * L^T, where L is a lower
// triangular matrix with diagonal entries all 1 (L is lower unit triangular)
// and where D is a diagonal matrix with diagonal entries all positive.

namespace gte
{
    template <typename T, int32_t...> class LDLTDecomposition;
    template <typename T, int32_t...> class BlockLDLTDecomposition;

    // Implementation for sizes known at compile time.
    template <typename T, int32_t N>
    class LDLTDecomposition<T, N>
    {
    public:
        LDLTDecomposition()
        {
            static_assert(
                N > 0,
                "Invalid size.");
        }

        // The matrix A must be positive definite. The implementation uses
        // only the lower-triangular portion of A. On output, L is lower
        // unit triangular and D is diagonal.
        bool Factor(Matrix<N, N, T> const& A, Matrix<N, N, T>& L, Matrix<N, N, T>& D)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);

            L.MakeZero();
            D.MakeZero();
            for (int32_t j = 0; j < N; ++j)
            {
                T Djj = A(j, j);
                for (int32_t k = 0; k < j; ++k)
                {
                    T Ljk = L(j, k);
                    T Dkk = D(k, k);
                    Djj -= Ljk * Ljk * Dkk;
                }
                D(j, j) = Djj;
                if (Djj == zero)
                {
                    return false;
                }

                L(j, j) = one;
                for (int32_t i = j + 1; i < N; ++i)
                {
                    T Lij = A(i, j);
                    for (int32_t k = 0; k < j; ++k)
                    {
                        T Lik = L(i, k);
                        T Ljk = L(j, k);
                        T Dkk = D(k, k);
                        Lij -= Lik * Ljk * Dkk;
                    }
                    Lij /= Djj;
                    L(i, j) = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(Matrix<N, N, T> const& L, Matrix<N, N, T> const& D,
            Vector<N, T> const& B, Vector<N, T>& X)
        {
            // Solve L * Z = L * (D * L^T * X) = B for Z.
            for (int32_t r = 0; r < N; ++r)
            {
                X[r] = B[r];
                for (int32_t c = 0; c < r; ++c)
                {
                    X[r] -= L(r, c) * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (int32_t r = 0; r < N; ++r)
            {
                X[r] /= D(r, r);
            }

            // Solve L^T * Y = Z for X.
            for (int32_t r = N - 1; r >= 0; --r)
            {
                for (int32_t c = r + 1; c < N; ++c)
                {
                    X[r] -= L(c, r) * X[c];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring during the call.
        void Solve(Matrix<N, N, T> const& A, Vector<N, T> const& B, Vector<N, T>& X)
        {
            Matrix<N, N, T> L, D;
            Factor(A, L, D);
            Solve(L, D, B, X);
        }
    };

    // Implementation for sizes known only at run time.
    template <typename T>
    class LDLTDecomposition<T>
    {
    public:
        int32_t const N;

        LDLTDecomposition(int32_t inN)
            :
            N(inN)
        {
            LogAssert(
                N > 0,
                "Invalid size.");
        }

        // The matrix A must be positive definite. The implementation uses
        // only the lower-triangular portion of A. On output, L is lower
        // unit triangular and D is diagonal.
        bool Factor(GMatrix<T> const& A, GMatrix<T>& L, GMatrix<T>& D)
        {
            LogAssert(
                A.GetNumRows() == N && A.GetNumCols() == N,
                "Invalid size.");

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            L.SetSize(N, N);
            L.MakeZero();
            D.SetSize(N, N);
            D.MakeZero();

            for (int32_t j = 0; j < N; ++j)
            {
                T Djj = A(j, j);
                for (int32_t k = 0; k < j; ++k)
                {
                    T Ljk = L(j, k);
                    T Dkk = D(k, k);
                    Djj -= Ljk * Ljk * Dkk;
                }
                D(j, j) = Djj;
                if (Djj == zero)
                {
                    return false;
                }

                L(j, j) = one;
                for (int32_t i = j + 1; i < N; ++i)
                {
                    T Lij = A(i, j);
                    for (int32_t k = 0; k < j; ++k)
                    {
                        T Lik = L(i, k);
                        T Ljk = L(j, k);
                        T Dkk = D(k, k);
                        Lij -= Lik * Ljk * Dkk;
                    }

                    Lij /= Djj;
                    L(i, j) = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(GMatrix<T> const& L, GMatrix<T> const& D,
            GVector<T> const& B, GVector<T>& X)
        {
            LogAssert(
                L.GetNumRows() == N && L.GetNumCols() == N &&
                D.GetNumRows() == N && D.GetNumCols() && B.GetSize() == N,
                "Invalid size.");

            X.SetSize(N);

            // Solve L * Z = L * (D * L^T * X) = B for Z.
            for (int32_t r = 0; r < N; ++r)
            {
                X[r] = B[r];
                for (int32_t c = 0; c < r; ++c)
                {
                    X[r] -= L(r, c) * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (int32_t r = 0; r < N; ++r)
            {
                X[r] /= D(r, r);
            }

            // Solve L^T * Y = Z for X.
            for (int32_t r = N - 1; r >= 0; --r)
            {
                for (int32_t c = r + 1; c < N; ++c)
                {
                    X[r] -= L(c, r) * X[c];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T.
        void Solve(GMatrix<T> const& A, GVector<T> const& B, GVector<T>& X)
        {
            LogAssert(
                A.GetNumRows() == N && A.GetNumCols() == N && B.GetSize() == N,
                "Invalid size.");

            GMatrix<T> L, D;
            Factor(A, L, D);
            Solve(L, D, B, X);
        }
    };

    // Implementation for sizes known at compile time.
    template <typename T, int32_t BlockSize, int32_t NumBlocks>
    class BlockLDLTDecomposition<T, BlockSize, NumBlocks>
    {
    public:
        // Let B represent the block size and N represent the number of
        // blocks. The matrix A is (N*B)-by-(N*B) but partitioned into an
        // N-by-N matrix of blocks, each block of size B-by-B. The value
        // N*B is NumDimensions.
        enum
        {
            NumDimensions = NumBlocks * BlockSize
        };

        using BlockVector = std::array<Vector<BlockSize, T>, NumBlocks>;
        using BlockMatrix = std::array<std::array<Matrix<BlockSize, BlockSize, T>, NumBlocks>, NumBlocks>;

        BlockLDLTDecomposition()
        {
            static_assert(
                BlockSize > 0 && NumBlocks > 0,
                "Invalid size.");
        }

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and return a reference.
        void Get(BlockMatrix const& M, int32_t row, int32_t col, T& value)
        {
            int32_t b0 = col / BlockSize;
            int32_t b1 = row / BlockSize;
            int32_t i0 = col - BlockSize * b0;
            int32_t i1 = row - BlockSize * b1;
            auto const& MBlock = M[b1][b0];
            value = MBlock(i1, i0);
        }

        void Set(BlockMatrix& M, int32_t row, int32_t col, T const& value)
        {
            int32_t b0 = col / BlockSize;
            int32_t b1 = row / BlockSize;
            int32_t i0 = col - BlockSize * b0;
            int32_t i1 = row - BlockSize * b1;
            auto& MBlock = M[b1][b0];
            MBlock(i1, i0) = value;
        }

        // Convert from a matrix to a block matrix.
        void Convert(Matrix<NumDimensions, NumDimensions, T> const& M, BlockMatrix& MBlock) const
        {
            for (int32_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (int32_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize)
                {
                    auto& current = MBlock[r][c];
                    for (int32_t j = 0; j < BlockSize; ++j)
                    {
                        for (int32_t i = 0; i < BlockSize; ++i)
                        {
                            current(j, i) = M(rb + j, cb + i);
                        }
                    }
                }
            }
        }

        // Convert from a vector to a block vector.
        void Convert(Vector<NumDimensions, T> const& V, BlockVector& VBlock) const
        {
            for (int32_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto& current = VBlock[r];
                for (int32_t j = 0; j < BlockSize; ++j)
                {
                    current[j] = V[rb + j];
                }
            }
        }

        // Convert from a block matrix to a matrix.
        void Convert(BlockMatrix const& MBlock, Matrix<NumDimensions, NumDimensions, T>& M) const
        {
            for (int32_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (int32_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize)
                {
                    auto const& current = MBlock[r][c];
                    for (int32_t j = 0; j < BlockSize; ++j)
                    {
                        for (int32_t i = 0; i < BlockSize; ++i)
                        {
                            M(rb + j, cb + i) = current(j, i);
                        }
                    }
                }
            }
        }

        // Convert from a block vector to a vector.
        void Convert(BlockVector const& VBlock, Vector<NumDimensions, T>& V) const
        {
            for (int32_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto const& current = VBlock[r];
                for (int32_t j = 0; j < BlockSize; ++j)
                {
                    V[rb + j] = current[j];
                }
            }
        }

        // The block matrix A must be positive definite. The implementation
        // uses only the lower-triangular blocks of A. On output, the block
        // matrix L is lower unit triangular (diagonal blocks are BxB identity
        // matrices) and the block matrix D is diagonal (diagonal blocks are
        // BxB diagonal matrices).
        bool Factor(BlockMatrix const& A, BlockMatrix& L, BlockMatrix& D)
        {
            for (int32_t row = 0; row < NumBlocks; ++row)
            {
                for (int32_t col = 0; col < NumBlocks; ++col)
                {
                    L[row][col].MakeZero();
                    D[row][col].MakeZero();
                }
            }

            for (int32_t j = 0; j < NumBlocks; ++j)
            {
                Matrix<BlockSize, BlockSize, T> Djj = A[j][j];
                for (int32_t k = 0; k < j; ++k)
                {
                    auto const& Ljk = L[j][k];
                    auto const& Dkk = D[k][k];
                    Djj -= MultiplyABT(Ljk * Dkk, Ljk);
                }
                D[j][j] = Djj;
                bool invertible = false;
                Matrix<BlockSize, BlockSize, T> invDjj = Inverse(Djj, &invertible);
                if (!invertible)
                {
                    return false;
                }

                L[j][j].MakeIdentity();
                for (int32_t i = j + 1; i < NumBlocks; ++i)
                {
                    Matrix<BlockSize, BlockSize, T> Lij = A[i][j];
                    for (int32_t k = 0; k < j; ++k)
                    {
                        auto const& Lik = L[i][k];
                        auto const& Ljk = L[j][k];
                        auto const& Dkk = D[k][k];
                        Lij -= MultiplyABT(Lik * Dkk, Ljk);
                    }
                    Lij = Lij * invDjj;
                    L[i][j] = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(BlockMatrix const& L, BlockMatrix const& D,
            BlockVector const& B, BlockVector& X)
        {
            // Solve L * Z = L * (D * L^T * X) = B for Z.
            for (int32_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = B[r];
                for (int32_t c = 0; c < r; ++c)
                {
                    X[r] -= L[r][c] * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (int32_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = Inverse(D[r][r]) * X[r];
            }

            // Solve L^T * Y = Z for X.
            for (int32_t r = NumBlocks - 1; r >= 0; --r)
            {
                for (int32_t c = r + 1; c < NumBlocks; ++c)
                {
                    X[r] -= X[c] * L[c][r];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring during the call.
        void Solve(BlockMatrix const& A, BlockVector const& B, BlockVector& X)
        {
            BlockMatrix L, D;
            Factor(A, L, D);
            Solve(L, D, B, X);
        }
    };

    // Implementation for sizes known only at run time.
    template <typename T>
    class BlockLDLTDecomposition<T>
    {
    public:
        // Let B represent the block size and N represent the number of
        // blocks. The matrix A is (N*B)-by-(N*B) but partitioned into an
        // N-by-N matrix of blocks, each block of size B-by-B and stored in
        // row-major order. The value N*B is NumDimensions.
        int32_t const BlockSize;
        int32_t const NumBlocks;
        int32_t const NumDimensions;

        // The number of elements in a BlockVector object must be NumBlocks
        // and each GVector element has BlockSize components.
        using BlockVector = std::vector<GVector<T>>;

        // The BlockMatrix is an array of NumBlocks-by-NumBlocks matrices.
        // Each block matrix is stored in row-major order. The BlockMatrix
        // elements themselves are stored in row-major order. The block
        // matrix element M = BlockMatrix[col + NumBlocks * row] is of size
        // BlockSize-by-BlockSize (in row-major order) and is in the (row,col)
        // location of the full matrix of blocks.
        using BlockMatrix = std::vector<GMatrix<T>>;

        BlockLDLTDecomposition(int32_t blockSize, int32_t numBlocks)
            :
            BlockSize(blockSize),
            NumBlocks(numBlocks),
            NumDimensions(blockSize* numBlocks)
        {
            LogAssert(
                blockSize > 0 && numBlocks > 0,
                "Invalid size.");
        }

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and return a reference. NOTE: You
        // are responsible for ensuring that M has NumBlocks-by-NumBlocks
        // elements, each M[] having BlockSize-by-BlockSize elements.
        void Get(BlockMatrix const& M, int32_t row, int32_t col, T& value, bool verifySize = true)
        {
            if (verifySize)
            {
                LogAssert(
                    M.size() == NumBlocks * NumBlocks,
                    "Invalid size.");
            }

            int32_t b0 = col / BlockSize;
            int32_t b1 = row / BlockSize;
            int32_t i0 = col - BlockSize * b0;
            int32_t i1 = row - BlockSize * b1;
            auto const& MBlock = M[GetIndex(b1, b0)];

            if (verifySize)
            {
                LogAssert(
                    MBlock.GetNumRows() == BlockSize &&
                    MBlock.GetNumCols() == BlockSize,
                    "Invalid size.");
            }

            value = MBlock(i1, i0);
        }

        void Set(BlockMatrix& M, int32_t row, int32_t col, T const& value, bool verifySize = true)
        {
            if (verifySize)
            {
                LogAssert(
                    M.size() == NumBlocks * NumBlocks,
                    "Invalid size.");
            }

            int32_t b0 = col / BlockSize;
            int32_t b1 = row / BlockSize;
            int32_t i0 = col - BlockSize * b0;
            int32_t i1 = row - BlockSize * b1;
            auto& MBlock = M[GetIndex(b1, b0)];

            if (verifySize)
            {
                LogAssert(
                    MBlock.GetNumRows() == BlockSize &&
                    MBlock.GetNumCols() == BlockSize,
                    "Invalid size.");
            }

            MBlock(i1, i0) = value;
        }

        // Convert from a matrix to a block matrix.
        void Convert(GMatrix<T> const& M, BlockMatrix& MBlock, bool verifySize = true) const
        {
            if (verifySize)
            {
                LogAssert(
                    M.GetNumRows() == NumDimensions &&
                    M.GetNumCols() == NumDimensions,
                    "Invalid size.");
            }

            size_t const szNumBlocks = static_cast<size_t>(NumBlocks);
            MBlock.resize(szNumBlocks * szNumBlocks);
            for (int32_t r = 0, rb = 0, index = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (int32_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize, ++index)
                {
                    auto& current = MBlock[index];
                    current.SetSize(BlockSize, BlockSize);
                    for (int32_t j = 0; j < BlockSize; ++j)
                    {
                        for (int32_t i = 0; i < BlockSize; ++i)
                        {
                            current(j, i) = M(rb + j, cb + i);
                        }
                    }
                }
            }
        }

        // Convert from a vector to a block vector.
        void Convert(GVector<T> const& V, BlockVector& VBlock, bool verifySize = true) const
        {
            if (verifySize)
            {
                LogAssert(
                    V.GetSize() == NumDimensions,
                    "Invalid size.");
            }

            VBlock.resize(static_cast<size_t>(NumBlocks));
            for (int32_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto& current = VBlock[r];
                current.SetSize(BlockSize);
                for (int32_t j = 0; j < BlockSize; ++j)
                {
                    current[j] = V[rb + j];
                }
            }
        }

        // Convert from a block matrix to a matrix.
        void Convert(BlockMatrix const& MBlock, GMatrix<T>& M, bool verifySize = true) const
        {
            if (verifySize)
            {
                LogAssert(
                    MBlock.size() == NumBlocks * NumBlocks,
                    "Invalid size.");

                for (auto const& current : MBlock)
                {
                    LogAssert(
                        current.GetNumRows() == NumBlocks &&
                        current.GetNumCols() == NumBlocks,
                        "Invalid size.");
                }
            }

            M.SetSize(NumDimensions, NumDimensions);
            for (int32_t r = 0, rb = 0, index = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (int32_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize, ++index)
                {
                    auto const& current = MBlock[index];
                    for (int32_t j = 0; j < BlockSize; ++j)
                    {
                        for (int32_t i = 0; i < BlockSize; ++i)
                        {
                            M(rb + j, cb + i) = current(j, i);
                        }
                    }
                }
            }
        }

        // Convert from a block vector to a vector.
        void Convert(BlockVector const& VBlock, GVector<T>& V, bool verifySize = true) const
        {
            if (verifySize)
            {
                LogAssert(
                    VBlock.size() == static_cast<size_t>(NumBlocks),
                    "Invalid size.");

                for (auto const& current : VBlock)
                {
                    LogAssert(
                        current.GetSize() == NumBlocks,
                        "Invalid size.");
                }
            }

            V.SetSize(NumDimensions);
            for (int32_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto const& current = VBlock[r];
                for (int32_t j = 0; j < BlockSize; ++j)
                {
                    V[rb + j] = current[j];
                }
            }
        }

        // The block matrix A must be positive definite. The implementation
        // uses only the lower-triangular blocks of A. On output, the block
        // matrix L is lower unit triangular (diagonal blocks are BxB identity
        // matrices) and the block matrix D is diagonal (diagonal blocks are
        // BxB diagonal matrices).
        bool Factor(BlockMatrix const& A, BlockMatrix& L, BlockMatrix& D, bool verifySize = true)
        {
            if (verifySize)
            {
                size_t szNumBlocks = static_cast<size_t>(NumBlocks);
                LogAssert(
                    A.size() == szNumBlocks * szNumBlocks,
                    "Invalid size.");

                for (size_t i = 0; i < A.size(); ++i)
                {
                    LogAssert(
                        A[i].GetNumRows() == BlockSize &&
                        A[i].GetNumCols() == BlockSize,
                        "Invalid size.");
                }
            }

            L.resize(A.size());
            D.resize(A.size());
            for (size_t i = 0; i < L.size(); ++i)
            {
                L[i].SetSize(BlockSize, BlockSize);
                L[i].MakeZero();
                D[i].SetSize(BlockSize, BlockSize);
                D[i].MakeZero();
            }

            for (int32_t j = 0; j < NumBlocks; ++j)
            {
                GMatrix<T> Djj = A[GetIndex(j, j)];
                for (int32_t k = 0; k < j; ++k)
                {
                    auto const& Ljk = L[GetIndex(j, k)];
                    auto const& Dkk = D[GetIndex(k, k)];
                    Djj -= MultiplyABT(Ljk * Dkk, Ljk);
                }
                D[GetIndex(j, j)] = Djj;
                bool invertible = false;
                GMatrix<T> invDjj = Inverse(Djj, &invertible);
                if (!invertible)
                {
                    return false;
                }

                L[GetIndex(j, j)].MakeIdentity();
                for (int32_t i = j + 1; i < NumBlocks; ++i)
                {
                    GMatrix<T> Lij = A[GetIndex(i, j)];
                    for (int32_t k = 0; k < j; ++k)
                    {
                        auto const& Lik = L[GetIndex(i, k)];
                        auto const& Ljk = L[GetIndex(j, k)];
                        auto const& Dkk = D[GetIndex(k, k)];
                        Lij -= MultiplyABT(Lik * Dkk, Ljk);
                    }
                    Lij = Lij * invDjj;
                    L[GetIndex(i, j)] = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(BlockMatrix const& L, BlockMatrix const& D,
            BlockVector const& B, BlockVector& X, bool verifySize = true)
        {
            if (verifySize)
            {
                size_t const szNumBlocks = static_cast<size_t>(NumBlocks);
                size_t const LDsize = szNumBlocks * szNumBlocks;
                LogAssert(
                    L.size() == LDsize &&
                    D.size() == LDsize &&
                    B.size() == szNumBlocks,
                    "Invalid size.");

                for (size_t i = 0; i < L.size(); ++i)
                {
                    LogAssert(
                        L[i].GetNumRows() == BlockSize &&
                        L[i].GetNumCols() == BlockSize &&
                        D[i].GetNumRows() == BlockSize &&
                        D[i].GetNumCols() == BlockSize,
                        "Invalid size.");
                }

                for (size_t i = 0; i < B.size(); ++i)
                {
                    LogAssert(
                        B[i].GetSize() == BlockSize,
                        "Invalid size.");
                }
            }

            // Solve L * Z = L * (D * L^T * X) = B for Z.
            X.resize(static_cast<size_t>(NumBlocks));
            for (int32_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = B[r];
                for (int32_t c = 0; c < r; ++c)
                {
                    X[r] -= L[GetIndex(r, c)] * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (int32_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = Inverse(D[GetIndex(r, r)]) * X[r];
            }

            // Solve L^T * Y = Z for X.
            for (int32_t r = NumBlocks - 1; r >= 0; --r)
            {
                for (int32_t c = r + 1; c < NumBlocks; ++c)
                {
                    X[r] -= X[c] * L[GetIndex(c, r)];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring during the call.
        void Solve(BlockMatrix const& A, BlockVector const& B, BlockVector& X,
            bool verifySize = true)
        {
            if (verifySize)
            {
                size_t const szNumBlocks = static_cast<size_t>(NumBlocks);
                LogAssert(
                    A.size() == szNumBlocks * szNumBlocks &&
                    B.size() == szNumBlocks,
                    "Invalid size.");

                for (size_t i = 0; i < A.size(); ++i)
                {
                    LogAssert(
                        A[i].GetNumRows() == BlockSize &&
                        A[i].GetNumCols() == BlockSize,
                        "Invalid size.");
                }

                for (size_t i = 0; i < B.size(); ++i)
                {
                    LogAssert(
                        B[i].GetSize() == BlockSize,
                        "Invalid size.");
                }
            }

            BlockMatrix L, D;
            Factor(A, L, D, false);
            Solve(L, D, B, X, false);
        }

    private:
        // Compute the 1-dimensional index of the block matrix in a
        // 2-dimensional BlockMatrix object.
        inline size_t GetIndex(int32_t row, int32_t col) const
        {
            return static_cast<size_t>(col + row * NumBlocks);
        }
    };
}
