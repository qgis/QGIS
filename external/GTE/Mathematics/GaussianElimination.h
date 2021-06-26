// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.11.23

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/LexicoArray2.h>
#include <cstring>
#include <vector>

// The input matrix M must be NxN.  The storage convention for element lookup
// is determined by GTE_USE_ROW_MAJOR or GTE_USE_COL_MAJOR, whichever is
// active.  If you want the inverse of M, pass a nonnull pointer inverseM;
// this matrix must also be NxN and use the same storage convention as M.  If
// you do not want the inverse of M, pass a nullptr for inverseM.  If you want
// to solve M*X = B for X, where X and B are Nx1, pass nonnull pointers for B
// and X.  If you want to solve M*Y = C for Y, where X and C are NxK, pass
// nonnull pointers for C and Y and pass K to numCols.  In all cases, pass
// N to numRows.

namespace gte
{
    template <typename Real>
    class GaussianElimination
    {
    public:
        bool operator()(int numRows,
            Real const* M, Real* inverseM, Real& determinant,
            Real const* B, Real* X,
            Real const* C, int numCols, Real* Y) const
        {
            if (numRows <= 0 || !M
                || ((B != nullptr) != (X != nullptr))
                || ((C != nullptr) != (Y != nullptr))
                || (C != nullptr && numCols < 1))
            {
                LogError("Invalid input.");
            }

            int numElements = numRows * numRows;
            bool wantInverse = (inverseM != nullptr);
            std::vector<Real> localInverseM;
            if (!wantInverse)
            {
                localInverseM.resize(numElements);
                inverseM = localInverseM.data();
            }
            Set(numElements, M, inverseM);

            if (B)
            {
                Set(numRows, B, X);
            }

            if (C)
            {
                Set(numRows * numCols, C, Y);
            }

#if defined(GTE_USE_ROW_MAJOR)
            LexicoArray2<true, Real> matInvM(numRows, numRows, inverseM);
            LexicoArray2<true, Real> matY(numRows, numCols, Y);
#else
            LexicoArray2<false, Real> matInvM(numRows, numRows, inverseM);
            LexicoArray2<false, Real> matY(numRows, numCols, Y);
#endif

            std::vector<int> colIndex(numRows), rowIndex(numRows), pivoted(numRows);
            std::fill(pivoted.begin(), pivoted.end(), 0);

            Real const zero = (Real)0;
            Real const one = (Real)1;
            bool odd = false;
            determinant = one;

            // Elimination by full pivoting.
            int i1, i2, row = 0, col = 0;
            for (int i0 = 0; i0 < numRows; ++i0)
            {
                // Search matrix (excluding pivoted rows) for maximum absolute entry.
                Real maxValue = zero;
                for (i1 = 0; i1 < numRows; ++i1)
                {
                    if (!pivoted[i1])
                    {
                        for (i2 = 0; i2 < numRows; ++i2)
                        {
                            if (!pivoted[i2])
                            {
                                Real value = matInvM(i1, i2);
                                Real absValue = (value >= zero ? value : -value);
                                if (absValue > maxValue)
                                {
                                    maxValue = absValue;
                                    row = i1;
                                    col = i2;
                                }
                            }
                        }
                    }
                }

                if (maxValue == zero)
                {
                    // The matrix is not invertible.
                    if (wantInverse)
                    {
                        Set(numElements, nullptr, inverseM);
                    }
                    determinant = zero;

                    if (B)
                    {
                        Set(numRows, nullptr, X);
                    }

                    if (C)
                    {
                        Set(numRows * numCols, nullptr, Y);
                    }
                    return false;
                }

                pivoted[col] = true;

                // Swap rows so that the pivot entry is in row 'col'.
                if (row != col)
                {
                    odd = !odd;
                    for (int i = 0; i < numRows; ++i)
                    {
                        std::swap(matInvM(row, i), matInvM(col, i));
                    }

                    if (B)
                    {
                        std::swap(X[row], X[col]);
                    }

                    if (C)
                    {
                        for (int i = 0; i < numCols; ++i)
                        {
                            std::swap(matY(row, i), matY(col, i));
                        }
                    }
                }

                // Keep track of the permutations of the rows.
                rowIndex[i0] = row;
                colIndex[i0] = col;

                // Scale the row so that the pivot entry is 1.
                Real diagonal = matInvM(col, col);
                determinant *= diagonal;
                Real inv = one / diagonal;
                matInvM(col, col) = one;
                for (i2 = 0; i2 < numRows; ++i2)
                {
                    matInvM(col, i2) *= inv;
                }

                if (B)
                {
                    X[col] *= inv;
                }

                if (C)
                {
                    for (i2 = 0; i2 < numCols; ++i2)
                    {
                        matY(col, i2) *= inv;
                    }
                }

                // Zero out the pivot column locations in the other rows.
                for (i1 = 0; i1 < numRows; ++i1)
                {
                    if (i1 != col)
                    {
                        Real save = matInvM(i1, col);
                        matInvM(i1, col) = zero;
                        for (i2 = 0; i2 < numRows; ++i2)
                        {
                            matInvM(i1, i2) -= matInvM(col, i2) * save;
                        }

                        if (B)
                        {
                            X[i1] -= X[col] * save;
                        }

                        if (C)
                        {
                            for (i2 = 0; i2 < numCols; ++i2)
                            {
                                matY(i1, i2) -= matY(col, i2) * save;
                            }
                        }
                    }
                }
            }

            if (wantInverse)
            {
                // Reorder rows to undo any permutations in Gaussian elimination.
                for (i1 = numRows - 1; i1 >= 0; --i1)
                {
                    if (rowIndex[i1] != colIndex[i1])
                    {
                        for (i2 = 0; i2 < numRows; ++i2)
                        {
                            std::swap(matInvM(i2, rowIndex[i1]),
                                matInvM(i2, colIndex[i1]));
                        }
                    }
                }
            }

            if (odd)
            {
                determinant = -determinant;
            }

            return true;
        }

    private:
        // Support for copying source to target or to set target to zero.  If
        // source is nullptr, then target is set to zero; otherwise source is
        // copied to target.  This function hides the type traits used to
        // determine whether Real is native floating-point or otherwise (such
        // as BSNumber or BSRational).
        void Set(int numElements, Real const* source, Real* target) const
        {
            if (std::is_floating_point<Real>() == std::true_type())
            {
                // Fast set/copy for native floating-point.
                size_t numBytes = numElements * sizeof(Real);
                if (source)
                {
                    std::memcpy(target, source, numBytes);
                }
                else
                {
                    std::memset(target, 0, numBytes);
                }
            }
            else
            {
                // The inputs are not std containers, so ensure assignment works
                // correctly.
                if (source)
                {
                    for (int i = 0; i < numElements; ++i)
                    {
                        target[i] = source[i];
                    }
                }
                else
                {
                    Real const zero = (Real)0;
                    for (int i = 0; i < numElements; ++i)
                    {
                        target[i] = zero;
                    }
                }
            }
        }
    };
}
