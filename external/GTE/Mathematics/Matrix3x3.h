// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    // Template alias for convenience.
    template <typename Real>
    using Matrix3x3 = Matrix<3, 3, Real>;

    // Geometric operations.
    template <typename Real>
    Matrix3x3<Real> Inverse(Matrix3x3<Real> const& M, bool* reportInvertibility = nullptr)
    {
        Matrix3x3<Real> inverse;
        bool invertible;
        Real c00 = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1);
        Real c10 = M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2);
        Real c20 = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0);
        Real det = M(0, 0) * c00 + M(0, 1) * c10 + M(0, 2) * c20;
        if (det != (Real)0)
        {
            Real invDet = (Real)1 / det;
            inverse = Matrix3x3<Real>
            {
                c00 * invDet,
                (M(0, 2) * M(2, 1) - M(0, 1) * M(2, 2)) * invDet,
                (M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1)) * invDet,
                c10 * invDet,
                (M(0, 0) * M(2, 2) - M(0, 2) * M(2, 0)) * invDet,
                (M(0, 2) * M(1, 0) - M(0, 0) * M(1, 2)) * invDet,
                c20 * invDet,
                (M(0, 1) * M(2, 0) - M(0, 0) * M(2, 1)) * invDet,
                (M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0)) * invDet
            };
            invertible = true;
        }
        else
        {
            inverse.MakeZero();
            invertible = false;
        }

        if (reportInvertibility)
        {
            *reportInvertibility = invertible;
        }
        return inverse;
    }

    template <typename Real>
    Matrix3x3<Real> Adjoint(Matrix3x3<Real> const& M)
    {
        return Matrix3x3<Real>
        {
            M(1, 1)* M(2, 2) - M(1, 2) * M(2, 1),
                M(0, 2)* M(2, 1) - M(0, 1) * M(2, 2),
                M(0, 1)* M(1, 2) - M(0, 2) * M(1, 1),
                M(1, 2)* M(2, 0) - M(1, 0) * M(2, 2),
                M(0, 0)* M(2, 2) - M(0, 2) * M(2, 0),
                M(0, 2)* M(1, 0) - M(0, 0) * M(1, 2),
                M(1, 0)* M(2, 1) - M(1, 1) * M(2, 0),
                M(0, 1)* M(2, 0) - M(0, 0) * M(2, 1),
                M(0, 0)* M(1, 1) - M(0, 1) * M(1, 0)
        };
    }

    template <typename Real>
    Real Determinant(Matrix3x3<Real> const& M)
    {
        Real c00 = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1);
        Real c10 = M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2);
        Real c20 = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0);
        Real det = M(0, 0) * c00 + M(0, 1) * c10 + M(0, 2) * c20;
        return det;
    }

    template <typename Real>
    Real Trace(Matrix3x3<Real> const& M)
    {
        Real trace = M(0, 0) + M(1, 1) + M(2, 2);
        return trace;
    }

    // Multiply M and V according to the user-selected convention.  If it is
    // GTE_USE_MAT_VEC, the function returns M*V.  If it is GTE_USE_VEC_MAT,
    // the function returns V*M.  This function is provided to hide the
    // preprocessor symbols in the GTEngine sample applications.
    template <typename Real>
    Vector3<Real> DoTransform(Matrix3x3<Real> const& M, Vector3<Real> const& V)
    {
#if defined(GTE_USE_MAT_VEC)
        return M * V;
#else
        return V * M;
#endif
    }

    template <typename Real>
    Matrix3x3<Real> DoTransform(Matrix3x3<Real> const& A, Matrix3x3<Real> const& B)
    {
#if defined(GTE_USE_MAT_VEC)
        return A * B;
#else
        return B * A;
#endif
    }

    // For GTE_USE_MAT_VEC, the columns of an invertible matrix form a basis
    // for the range of the matrix.  For GTE_USE_VEC_MAT, the rows of an
    // invertible matrix form a basis for the range of the matrix.  These
    // functions allow you to access the basis vectors.  The caller is
    // responsible for ensuring that the matrix is invertible (although the
    // inverse is not calculated by these functions).
    template <typename Real>
    void SetBasis(Matrix3x3<Real>& M, int i, Vector3<Real> const& V)
    {
#if defined(GTE_USE_MAT_VEC)
        return M.SetCol(i, V);
#else
        return M.SetRow(i, V);
#endif
    }

    template <typename Real>
    Vector3<Real> GetBasis(Matrix3x3<Real> const& M, int i)
    {
#if defined(GTE_USE_MAT_VEC)
        return M.GetCol(i);
#else
        return M.GetRow(i);
#endif
    }
}
