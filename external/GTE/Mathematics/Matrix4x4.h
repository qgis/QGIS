// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/Vector4.h>

namespace gte
{
    // Template alias for convenience.
    template <typename Real>
    using Matrix4x4 = Matrix<4, 4, Real>;

    // Geometric operations.
    template <typename Real>
    Matrix4x4<Real> Inverse(Matrix4x4<Real> const& M, bool* reportInvertibility = nullptr)
    {
        Matrix4x4<Real> inverse;
        bool invertible;
        Real a0 = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        Real a1 = M(0, 0) * M(1, 2) - M(0, 2) * M(1, 0);
        Real a2 = M(0, 0) * M(1, 3) - M(0, 3) * M(1, 0);
        Real a3 = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
        Real a4 = M(0, 1) * M(1, 3) - M(0, 3) * M(1, 1);
        Real a5 = M(0, 2) * M(1, 3) - M(0, 3) * M(1, 2);
        Real b0 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
        Real b1 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
        Real b2 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
        Real b3 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
        Real b4 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
        Real b5 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);
        Real det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
        if (det != (Real)0)
        {
            Real invDet = (Real)1 / det;
            inverse = Matrix4x4<Real>
            {
                (+M(1, 1) * b5 - M(1, 2) * b4 + M(1, 3) * b3) * invDet,
                (-M(0, 1) * b5 + M(0, 2) * b4 - M(0, 3) * b3) * invDet,
                (+M(3, 1) * a5 - M(3, 2) * a4 + M(3, 3) * a3) * invDet,
                (-M(2, 1) * a5 + M(2, 2) * a4 - M(2, 3) * a3) * invDet,
                (-M(1, 0) * b5 + M(1, 2) * b2 - M(1, 3) * b1) * invDet,
                (+M(0, 0) * b5 - M(0, 2) * b2 + M(0, 3) * b1) * invDet,
                (-M(3, 0) * a5 + M(3, 2) * a2 - M(3, 3) * a1) * invDet,
                (+M(2, 0) * a5 - M(2, 2) * a2 + M(2, 3) * a1) * invDet,
                (+M(1, 0) * b4 - M(1, 1) * b2 + M(1, 3) * b0) * invDet,
                (-M(0, 0) * b4 + M(0, 1) * b2 - M(0, 3) * b0) * invDet,
                (+M(3, 0) * a4 - M(3, 1) * a2 + M(3, 3) * a0) * invDet,
                (-M(2, 0) * a4 + M(2, 1) * a2 - M(2, 3) * a0) * invDet,
                (-M(1, 0) * b3 + M(1, 1) * b1 - M(1, 2) * b0) * invDet,
                (+M(0, 0) * b3 - M(0, 1) * b1 + M(0, 2) * b0) * invDet,
                (-M(3, 0) * a3 + M(3, 1) * a1 - M(3, 2) * a0) * invDet,
                (+M(2, 0) * a3 - M(2, 1) * a1 + M(2, 2) * a0) * invDet
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
    Matrix4x4<Real> Adjoint(Matrix4x4<Real> const& M)
    {
        Real a0 = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        Real a1 = M(0, 0) * M(1, 2) - M(0, 2) * M(1, 0);
        Real a2 = M(0, 0) * M(1, 3) - M(0, 3) * M(1, 0);
        Real a3 = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
        Real a4 = M(0, 1) * M(1, 3) - M(0, 3) * M(1, 1);
        Real a5 = M(0, 2) * M(1, 3) - M(0, 3) * M(1, 2);
        Real b0 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
        Real b1 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
        Real b2 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
        Real b3 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
        Real b4 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
        Real b5 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);

        return Matrix4x4<Real>
        {
            +M(1, 1) * b5 - M(1, 2) * b4 + M(1, 3) * b3,
            -M(0, 1) * b5 + M(0, 2) * b4 - M(0, 3) * b3,
            +M(3, 1) * a5 - M(3, 2) * a4 + M(3, 3) * a3,
            -M(2, 1) * a5 + M(2, 2) * a4 - M(2, 3) * a3,
            -M(1, 0) * b5 + M(1, 2) * b2 - M(1, 3) * b1,
            +M(0, 0) * b5 - M(0, 2) * b2 + M(0, 3) * b1,
            -M(3, 0) * a5 + M(3, 2) * a2 - M(3, 3) * a1,
            +M(2, 0) * a5 - M(2, 2) * a2 + M(2, 3) * a1,
            +M(1, 0) * b4 - M(1, 1) * b2 + M(1, 3) * b0,
            -M(0, 0) * b4 + M(0, 1) * b2 - M(0, 3) * b0,
            +M(3, 0) * a4 - M(3, 1) * a2 + M(3, 3) * a0,
            -M(2, 0) * a4 + M(2, 1) * a2 - M(2, 3) * a0,
            -M(1, 0) * b3 + M(1, 1) * b1 - M(1, 2) * b0,
            +M(0, 0) * b3 - M(0, 1) * b1 + M(0, 2) * b0,
            -M(3, 0) * a3 + M(3, 1) * a1 - M(3, 2) * a0,
            +M(2, 0) * a3 - M(2, 1) * a1 + M(2, 2) * a0
        };
    }

    template <typename Real>
    Real Determinant(Matrix4x4<Real> const& M)
    {
        Real a0 = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        Real a1 = M(0, 0) * M(1, 2) - M(0, 2) * M(1, 0);
        Real a2 = M(0, 0) * M(1, 3) - M(0, 3) * M(1, 0);
        Real a3 = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
        Real a4 = M(0, 1) * M(1, 3) - M(0, 3) * M(1, 1);
        Real a5 = M(0, 2) * M(1, 3) - M(0, 3) * M(1, 2);
        Real b0 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
        Real b1 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
        Real b2 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
        Real b3 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
        Real b4 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
        Real b5 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);
        Real det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
        return det;
    }

    template <typename Real>
    Real Trace(Matrix4x4<Real> const& M)
    {
        Real trace = M(0, 0) + M(1, 1) + M(2, 2) + M(3, 3);
        return trace;
    }

    // Multiply M and V according to the user-selected convention.  If it is
    // GTE_USE_MAT_VEC, the function returns M*V.  If it is GTE_USE_VEC_MAT,
    // the function returns V*M.  This function is provided to hide the
    // preprocessor symbols in the GTEngine sample applications.
    template <typename Real>
    Vector4<Real> DoTransform(Matrix4x4<Real> const& M, Vector4<Real> const& V)
    {
#if defined(GTE_USE_MAT_VEC)
        return M * V;
#else
        return V * M;
#endif
    }

    template <typename Real>
    Matrix4x4<Real> DoTransform(Matrix4x4<Real> const& A, Matrix4x4<Real> const& B)
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
    void SetBasis(Matrix4x4<Real>& M, int i, Vector4<Real> const& V)
    {
#if defined(GTE_USE_MAT_VEC)
        return M.SetCol(i, V);
#else
        return M.SetRow(i, V);
#endif
    }

    template <typename Real>
    Vector4<Real> GetBasis(Matrix4x4<Real> const& M, int i)
    {
#if defined(GTE_USE_MAT_VEC)
        return M.GetCol(i);
#else
        return M.GetRow(i);
#endif
    }

    // Special matrices.  In the comments, the matrices are shown using the
    // GTE_USE_MAT_VEC multiplication convention.

    // The projection plane is Dot(N,X-P) = 0 where N is a 3-by-1 unit-length
    // normal vector and P is a 3-by-1 point on the plane.  The projection is
    // oblique to the plane, in the direction of the 3-by-1 vector D.
    // Necessarily Dot(N,D) is not zero for this projection to make sense.
    // Given a 3-by-1 point U, compute the intersection of the line U+t*D with
    // the plane to obtain t = -Dot(N,U-P)/Dot(N,D); then
    //
    //   projection(U) = P + [I - D*N^T/Dot(N,D)]*(U-P)
    //
    // A 4-by-4 homogeneous transformation representing the projection is
    //
    //       +-                               -+
    //   M = | D*N^T - Dot(N,D)*I   -Dot(N,P)D |
    //       |          0^T          -Dot(N,D) |
    //       +-                               -+
    //
    // where M applies to [U^T 1]^T by M*[U^T 1]^T.  The matrix is chosen so
    // that M[3][3] > 0 whenever Dot(N,D) < 0; the projection is onto the
    // "positive side" of the plane.
    template <typename Real>
    Matrix4x4<Real> MakeObliqueProjection(Vector4<Real> const& origin,
        Vector4<Real> const& normal, Vector4<Real> const& direction)
    {
        Matrix4x4<Real> M;

        Real const zero = (Real)0;
        Real dotND = Dot(normal, direction);
        Real dotNO = Dot(origin, normal);

#if defined(GTE_USE_MAT_VEC)
        M(0, 0) = direction[0] * normal[0] - dotND;
        M(0, 1) = direction[0] * normal[1];
        M(0, 2) = direction[0] * normal[2];
        M(0, 3) = -dotNO * direction[0];
        M(1, 0) = direction[1] * normal[0];
        M(1, 1) = direction[1] * normal[1] - dotND;
        M(1, 2) = direction[1] * normal[2];
        M(1, 3) = -dotNO * direction[1];
        M(2, 0) = direction[2] * normal[0];
        M(2, 1) = direction[2] * normal[1];
        M(2, 2) = direction[2] * normal[2] - dotND;
        M(2, 3) = -dotNO * direction[2];
        M(3, 0) = zero;
        M(3, 1) = zero;
        M(3, 2) = zero;
        M(3, 3) = -dotND;
#else
        M(0, 0) = direction[0] * normal[0] - dotND;
        M(1, 0) = direction[0] * normal[1];
        M(2, 0) = direction[0] * normal[2];
        M(3, 0) = -dotNO * direction[0];
        M(0, 1) = direction[1] * normal[0];
        M(1, 1) = direction[1] * normal[1] - dotND;
        M(2, 1) = direction[1] * normal[2];
        M(3, 1) = -dotNO * direction[1];
        M(0, 2) = direction[2] * normal[0];
        M(1, 2) = direction[2] * normal[1];
        M(2, 2) = direction[2] * normal[2] - dotND;
        M(3, 2) = -dotNO * direction[2];
        M(0, 2) = zero;
        M(1, 3) = zero;
        M(2, 3) = zero;
        M(3, 3) = -dotND;
#endif

        return M;
    }

    // The perspective projection of a point onto a plane is
    //
    //     +-                                                 -+
    // M = | Dot(N,E-P)*I - E*N^T    -(Dot(N,E-P)*I - E*N^T)*E |
    //     |        -N^t                      Dot(N,E)         |
    //     +-                                                 -+
    //
    // where E is the eye point, P is a point on the plane, and N is a
    // unit-length plane normal.
    template <typename Real>
    Matrix4x4<Real> MakePerspectiveProjection(Vector4<Real> const& origin,
        Vector4<Real> const& normal, Vector4<Real> const& eye)
    {
        Matrix4x4<Real> M;

        Real dotND = Dot(normal, eye - origin);

#if defined(GTE_USE_MAT_VEC)
        M(0, 0) = dotND - eye[0] * normal[0];
        M(0, 1) = -eye[0] * normal[1];
        M(0, 2) = -eye[0] * normal[2];
        M(0, 3) = -(M(0, 0) * eye[0] + M(0, 1) * eye[1] + M(0, 2) * eye[2]);
        M(1, 0) = -eye[1] * normal[0];
        M(1, 1) = dotND - eye[1] * normal[1];
        M(1, 2) = -eye[1] * normal[2];
        M(1, 3) = -(M(1, 0) * eye[0] + M(1, 1) * eye[1] + M(1, 2) * eye[2]);
        M(2, 0) = -eye[2] * normal[0];
        M(2, 1) = -eye[2] * normal[1];
        M(2, 2) = dotND - eye[2] * normal[2];
        M(2, 3) = -(M(2, 0) * eye[0] + M(2, 1) * eye[1] + M(2, 2) * eye[2]);
        M(3, 0) = -normal[0];
        M(3, 1) = -normal[1];
        M(3, 2) = -normal[2];
        M(3, 3) = Dot(eye, normal);
#else
        M(0, 0) = dotND - eye[0] * normal[0];
        M(1, 0) = -eye[0] * normal[1];
        M(2, 0) = -eye[0] * normal[2];
        M(3, 0) = -(M(0, 0) * eye[0] + M(0, 1) * eye[1] + M(0, 2) * eye[2]);
        M(0, 1) = -eye[1] * normal[0];
        M(1, 1) = dotND - eye[1] * normal[1];
        M(2, 1) = -eye[1] * normal[2];
        M(3, 1) = -(M(1, 0) * eye[0] + M(1, 1) * eye[1] + M(1, 2) * eye[2]);
        M(0, 2) = -eye[2] * normal[0];
        M(1, 2) = -eye[2] * normal[1];
        M(2, 2) = dotND - eye[2] * normal[2];
        M(3, 2) = -(M(2, 0) * eye[0] + M(2, 1) * eye[1] + M(2, 2) * eye[2]);
        M(0, 3) = -normal[0];
        M(1, 3) = -normal[1];
        M(2, 3) = -normal[2];
        M(3, 3) = Dot(eye, normal);
#endif

        return M;
    }

    // The reflection of a point through a plane is
    //     +-                         -+
    // M = | I-2*N*N^T    2*Dot(N,P)*N |
    //     |     0^T            1      |
    //     +-                         -+
    //
    // where P is a point on the plane and N is a unit-length plane normal.
    template <typename Real>
    Matrix4x4<Real> MakeReflection(Vector4<Real> const& origin,
        Vector4<Real> const& normal)
    {
        Matrix4x4<Real> M;

        Real const zero = (Real)0, one = (Real)1, two = (Real)2;
        Real twoDotNO = two * Dot(origin, normal);

#if defined(GTE_USE_MAT_VEC)
        M(0, 0) = one - two * normal[0] * normal[0];
        M(0, 1) = -two * normal[0] * normal[1];
        M(0, 2) = -two * normal[0] * normal[2];
        M(0, 3) = twoDotNO * normal[0];
        M(1, 0) = M(0, 1);
        M(1, 1) = one - two * normal[1] * normal[1];
        M(1, 2) = -two * normal[1] * normal[2];
        M(1, 3) = twoDotNO * normal[1];
        M(2, 0) = M(0, 2);
        M(2, 1) = M(1, 2);
        M(2, 2) = one - two * normal[2] * normal[2];
        M(2, 3) = twoDotNO * normal[2];
        M(3, 0) = zero;
        M(3, 1) = zero;
        M(3, 2) = zero;
        M(3, 3) = one;
#else
        M(0, 0) = one - two * normal[0] * normal[0];
        M(1, 0) = -two * normal[0] * normal[1];
        M(2, 0) = -two * normal[0] * normal[2];
        M(3, 0) = twoDotNO * normal[0];
        M(0, 1) = M(1, 0);
        M(1, 1) = one - two * normal[1] * normal[1];
        M(2, 1) = -two * normal[1] * normal[2];
        M(3, 1) = twoDotNO * normal[1];
        M(0, 2) = M(2, 0);
        M(1, 2) = M(2, 1);
        M(2, 2) = one - two * normal[2] * normal[2];
        M(3, 2) = twoDotNO * normal[2];
        M(0, 3) = zero;
        M(1, 3) = zero;
        M(2, 3) = zero;
        M(3, 3) = one;
#endif

        return M;
    }
}
