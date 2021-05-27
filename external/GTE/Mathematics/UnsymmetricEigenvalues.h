// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

// An implementation of the QR algorithm described in "Matrix Computations,
// 2nd edition" by G. H. Golub and C. F. Van Loan, The Johns Hopkins
// University Press, Baltimore MD, Fourth Printing 1993.  In particular,
// the implementation is based on Chapter 7 (The Unsymmetric Eigenvalue
// Problem), Section 7.5 (The Practical QR Algorithm).

namespace gte
{
    template <typename Real>
    class UnsymmetricEigenvalues
    {
    public:
        // The solver processes NxN matrices (not necessarily symmetric),
        // where N >= 3 ('size' is N) and the matrix is stored in row-major
        // order.  The maximum number of iterations ('maxIterations') must
        // be specified for reducing an upper Hessenberg matrix to an upper
        // quasi-triangular matrix (upper triangular matrix of blocks where
        // the diagonal blocks are 1x1 or 2x2).  The goal is to compute the
        // real-valued eigenvalues.
        UnsymmetricEigenvalues(int32_t size, uint32_t maxIterations)
            :
            mSize(0),
            mSizeM1(0),
            mMaxIterations(0),
            mNumEigenvalues(0)
        {
            if (size >= 3 && maxIterations > 0)
            {
                mSize = size;
                mSizeM1 = size - 1;
                mMaxIterations = maxIterations;
                mMatrix.resize(size * size);
                mX.resize(size);
                mV.resize(size);
                mScaledV.resize(size);
                mW.resize(size);
                mFlagStorage.resize(size + 1);
                std::fill(mFlagStorage.begin(), mFlagStorage.end(), 0);
                mSubdiagonalFlag = &mFlagStorage[1];
                mEigenvalues.resize(mSize);
            }
        }

        // A copy of the NxN input is made internally.  The order of the
        // eigenvalues is specified by sortType: -1 (decreasing), 0 (no
        // sorting), or +1 (increasing).  When sorted, the eigenvectors are
        // ordered accordingly.  The return value is the number of iterations
        // consumed when convergence occurred, 0xFFFFFFFF when convergence did
        // not occur, or 0 when N <= 1 was passed to the constructor.
        uint32_t Solve(Real const* input, int32_t sortType)
        {
            if (mSize > 0)
            {
                std::copy(input, input + mSize * mSize, mMatrix.begin());
                ReduceToUpperHessenberg();

                std::array<int, 2> block;
                bool found = GetBlock(block);
                uint32_t numIterations;
                for (numIterations = 0; numIterations < mMaxIterations; ++numIterations)
                {
                    if (found)
                    {
                        // Solve the current subproblem.
                        FrancisQRStep(block[0], block[1] + 1);

                        // Find another subproblem (if any).
                        found = GetBlock(block);
                    }
                    else
                    {
                        break;
                    }
                }

                // The matrix is fully uncoupled, upper Hessenberg with 1x1 or
                // 2x2 diagonal blocks. Golub and Van Loan call this "upper
                // quasi-triangular".
                mNumEigenvalues = 0;
                std::fill(mEigenvalues.begin(), mEigenvalues.end(), (Real)0);
                for (int i = 0; i < mSizeM1; ++i)
                {
                    if (mSubdiagonalFlag[i] == 0)
                    {
                        if (mSubdiagonalFlag[i - 1] == 0)
                        {
                            // We have a 1x1 block with a real eigenvalue.
                            mEigenvalues[mNumEigenvalues++] = A(i, i);
                        }
                    }
                    else
                    {
                        if (mSubdiagonalFlag[i - 1] == 0 && mSubdiagonalFlag[i + 1] == 0)
                        {
                            // We have a 2x2 block that might have real
                            // eigenvalues.
                            Real a00 = A(i, i);
                            Real a01 = A(i, i + 1);
                            Real a10 = A(i + 1, i);
                            Real a11 = A(i + 1, i + 1);
                            Real tr = a00 + a11;
                            Real det = a00 * a11 - a01 * a10;
                            Real halfTr = tr * (Real)0.5;
                            Real discr = halfTr * halfTr - det;
                            if (discr >= (Real)0)
                            {
                                Real rootDiscr = std::sqrt(discr);
                                mEigenvalues[mNumEigenvalues++] = halfTr - rootDiscr;
                                mEigenvalues[mNumEigenvalues++] = halfTr + rootDiscr;
                            }
                        }
                        // else:
                        // The QR iteration failed to converge at this block.
                        // It must also be the case that
                        // numIterations == mMaxIterations.  TODO: The caller
                        // will be aware of this when testing the returned
                        // numIterations.  Is there a remedy for such a case?
                        // This happened with root finding using the companion
                        // matrix of a polynomial.)
                    }
                }

                if (sortType != 0 && mNumEigenvalues > 1)
                {
                    if (sortType > 0)
                    {
                        std::sort(mEigenvalues.begin(),
                            mEigenvalues.begin() + mNumEigenvalues, std::less<Real>());
                    }
                    else
                    {
                        std::sort(mEigenvalues.begin(),
                            mEigenvalues.begin() + mNumEigenvalues, std::greater<Real>());
                    }
                }

                return numIterations;
            }
            return 0;
        }

        // Get the real-valued eigenvalues of the matrix passed to Solve(...).
        // The input 'eigenvalues' must have at least N elements.
        void GetEigenvalues(uint32_t& numEigenvalues, Real* eigenvalues) const
        {
            if (mSize > 0)
            {
                numEigenvalues = mNumEigenvalues;
                std::memcpy(eigenvalues, mEigenvalues.data(), numEigenvalues * sizeof(Real));
            }
            else
            {
                numEigenvalues = 0;
            }
        }

    private:
        // 2D accessors to elements of mMatrix[].
        inline Real const& A(int r, int c) const
        {
            return mMatrix[c + r * mSize];
        }

        inline Real& A(int r, int c)
        {
            return mMatrix[c + r * mSize];
        }

        // Compute the Householder vector for (X[rmin],...,x[rmax]).  The
        // input vector is stored in mX in the index range [rmin,rmax].  The
        // output vector V is stored in mV in the index range [rmin,rmax].
        // The scaled vector is S = (-2/Dot(V,V))*V and is stored in mScaledV
        // in the index range [rmin,rmax].
        void House(int rmin, int rmax)
        {
            Real length = (Real)0;
            for (int r = rmin; r <= rmax; ++r)
            {
                length += mX[r] * mX[r];
            }
            length = std::sqrt(length);
            if (length != (Real)0)
            {
                Real sign = (mX[rmin] >= (Real)0 ? (Real)1 : (Real)-1);
                Real invDenom = (Real)1 / (mX[rmin] + sign * length);
                for (int r = rmin + 1; r <= rmax; ++r)
                {
                    mV[r] = mX[r] * invDenom;
                }
            }
            mV[rmin] = (Real)1;

            Real dot = (Real)1;
            for (int r = rmin + 1; r <= rmax; ++r)
            {
                dot += mV[r] * mV[r];
            }
            Real scale = (Real)-2 / dot;
            for (int r = rmin; r <= rmax; ++r)
            {
                mScaledV[r] = scale * mV[r];
            }
        }

        // Support for replacing matrix A by P^T*A*P, where P is a Householder
        // reflection computed using House(...).
        void RowHouse(int rmin, int rmax, int cmin, int cmax)
        {
            for (int c = cmin; c <= cmax; ++c)
            {
                mW[c] = (Real)0;
                for (int r = rmin; r <= rmax; ++r)
                {
                    mW[c] += mScaledV[r] * A(r, c);
                }
            }

            for (int r = rmin; r <= rmax; ++r)
            {
                for (int c = cmin; c <= cmax; ++c)
                {
                    A(r, c) += mV[r] * mW[c];
                }
            }
        }

        void ColHouse(int rmin, int rmax, int cmin, int cmax)
        {
            for (int r = rmin; r <= rmax; ++r)
            {
                mW[r] = (Real)0;
                for (int c = cmin; c <= cmax; ++c)
                {
                    mW[r] += mScaledV[c] * A(r, c);
                }
            }

            for (int r = rmin; r <= rmax; ++r)
            {
                for (int c = cmin; c <= cmax; ++c)
                {
                    A(r, c) += mW[r] * mV[c];
                }
            }
        }

        void ReduceToUpperHessenberg()
        {
            for (int c = 0, cp1 = 1; c <= mSize - 3; ++c, ++cp1)
            {
                for (int r = cp1; r <= mSizeM1; ++r)
                {
                    mX[r] = A(r, c);
                }

                House(cp1, mSizeM1);
                RowHouse(cp1, mSizeM1, c, mSizeM1);
                ColHouse(0, mSizeM1, cp1, mSizeM1);
            }
        }

        void FrancisQRStep(int rmin, int rmax)
        {
            // Apply the double implicit shift step.
            int const i0 = rmax - 1, i1 = rmax;
            Real a00 = A(i0, i0);
            Real a01 = A(i0, i1);
            Real a10 = A(i1, i0);
            Real a11 = A(i1, i1);
            Real tr = a00 + a11;
            Real det = a00 * a11 - a01 * a10;

            int const j0 = rmin, j1 = j0 + 1, j2 = j1 + 1;
            Real b00 = A(j0, j0);
            Real b01 = A(j0, j1);
            Real b10 = A(j1, j0);
            Real b11 = A(j1, j1);
            Real b21 = A(j2, j1);
            mX[rmin] = b00 * (b00 - tr) + b01 * b10 + det;
            mX[rmin + 1] = b10 * (b00 + b11 - tr);
            mX[rmin + 2] = b10 * b21;

            House(rmin, rmin + 2);
            RowHouse(rmin, rmin + 2, rmin, rmax);
            ColHouse(rmin, std::min(rmax, rmin + 3), rmin, rmin + 2);

            // Apply Householder reflections to restore the matrix to upper
            // Hessenberg form.
            for (int c = 0, cp1 = 1; c <= mSize - 3; ++c, ++cp1)
            {
                int kmax = std::min(cp1 + 2, mSizeM1);
                for (int r = cp1; r <= kmax; ++r)
                {
                    mX[r] = A(r, c);
                }

                House(cp1, kmax);
                RowHouse(cp1, kmax, c, mSizeM1);
                ColHouse(0, mSizeM1, cp1, kmax);
            }
        }

        bool GetBlock(std::array<int, 2>& block)
        {
            for (int i = 0; i < mSizeM1; ++i)
            {
                Real a00 = A(i, i);
                Real a11 = A(i + 1, i + 1);
                Real a21 = A(i + 1, i);
                Real sum0 = a00 + a11;
                Real sum1 = sum0 + a21;
                mSubdiagonalFlag[i] = (sum1 != sum0 ? 1 : 0);
            }

            for (int i = 0; i < mSizeM1; ++i)
            {
                if (mSubdiagonalFlag[i] == 1)
                {
                    block = { i, -1 };
                    while (i < mSizeM1 && mSubdiagonalFlag[i] == 1)
                    {
                        block[1] = i++;
                    }
                    if (block[1] != block[0])
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        // The number N of rows and columns of the matrices to be processed.
        int32_t mSize, mSizeM1;

        // The maximum number of iterations for reducing the tridiagonal
        // matrix to a diagonal matrix.
        uint32_t mMaxIterations;

        // The internal copy of a matrix passed to the solver.
        std::vector<Real> mMatrix;  // NxN elements

        // Temporary storage to compute Householder reflections.
        std::vector<Real> mX, mV, mScaledV, mW;  // N elements

        // Flags about the zeroness of the subdiagonal entries.  This is used
        // to detect uncoupled submatrices and apply the QR algorithm to the
        // corresponding subproblems.  The storage is padded on both ends with
        // zeros to avoid additional code logic when packing the eigenvalues
        // for access by the caller.
        std::vector<int> mFlagStorage;
        int* mSubdiagonalFlag;

        int mNumEigenvalues;
        std::vector<Real> mEigenvalues;
    };
}
