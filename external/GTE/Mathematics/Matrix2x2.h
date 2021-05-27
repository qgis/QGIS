// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    // Template alias for convenience.
    template <typename Real>
    using Matrix2x2 = Matrix<2, 2, Real>;

    // Create a rotation matrix from an angle (in radians).  The matrix is
    // [GTE_USE_MAT_VEC]
    //   R(t) = {{c,-s},{s,c}}
    // [GTE_USE_VEC_MAT]
    //   R(t) = {{c,s},{-s,c}}
    // where c = cos(t), s = sin(t), and the inner-brace pairs are rows of the
    // matrix.
    template <typename Real>
    void MakeRotation(Real angle, Matrix2x2<Real>& rotation)
    {
        Real cs = std::cos(angle);
        Real sn = std::sin(angle);
#if defined(GTE_USE_MAT_VEC)
        rotation(0, 0) = cs;
        rotation(0, 1) = -sn;
        rotation(1, 0) = sn;
        rotation(1, 1) = cs;
#else
        rotation(0, 0) = cs;
        rotation(0, 1) = sn;
        rotation(1, 0) = -sn;
        rotation(1, 1) = cs;
#endif
    }

    // Get the angle (radians) from a rotation matrix.  The caller is
    // responsible for ensuring the matrix is a rotation.
    template <typename Real>
    Real GetRotationAngle(Matrix2x2<Real> const& rotation)
    {
#if defined(GTE_USE_MAT_VEC)
        return std::atan2(rotation(1, 0), rotation(0, 0));
#else
        return std::atan2(rotation(0, 1), rotation(0, 0));
#endif
    }

    // Geometric operations.
    template <typename Real>
    Matrix2x2<Real> Inverse(Matrix2x2<Real> const& M, bool* reportInvertibility = nullptr)
    {
        Matrix2x2<Real> inverse;
        bool invertible;
        Real det = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        if (det != (Real)0)
        {
            Real invDet = ((Real)1) / det;
            inverse = Matrix2x2<Real>
            {
                M(1, 1) * invDet, -M(0, 1) * invDet,
                    -M(1, 0) * invDet, M(0, 0) * invDet
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
    Matrix2x2<Real> Adjoint(Matrix2x2<Real> const& M)
    {
        return Matrix2x2<Real>
        {
            M(1, 1), -M(0, 1),
            -M(1, 0), M(0, 0)
        };
    }

    template <typename Real>
    Real Determinant(Matrix2x2<Real> const& M)
    {
        Real det = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        return det;
    }

    template <typename Real>
    Real Trace(Matrix2x2<Real> const& M)
    {
        Real trace = M(0, 0) + M(1, 1);
        return trace;
    }

    // Multiply M and V according to the user-selected convention.  If it is
    // GTE_USE_MAT_VEC, the function returns M*V.  If it is GTE_USE_VEC_MAT,
    // the function returns V*M.  This function is provided to hide the
    // preprocessor symbols in the GTEngine sample applications.
    template <typename Real>
    Vector2<Real> DoTransform(Matrix2x2<Real> const& M, Vector2<Real> const& V)
    {
#if defined(GTE_USE_MAT_VEC)
        return M * V;
#else
        return V * M;
#endif
    }

    template <typename Real>
    Matrix2x2<Real> DoTransform(Matrix2x2<Real> const& A, Matrix2x2<Real> const& B)
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
    void SetBasis(Matrix2x2<Real>& M, int i, Vector2<Real> const& V)
    {
#if defined(GTE_USE_MAT_VEC)
        return M.SetCol(i, V);
#else
        return M.SetRow(i, V);
#endif
    }

    template <typename Real>
    Vector2<Real> GetBasis(Matrix2x2<Real> const& M, int i)
    {
#if defined(GTE_USE_MAT_VEC)
        return M.GetCol(i);
#else
        return M.GetRow(i);
#endif
    }
}
