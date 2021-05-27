// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/GaussianElimination.h>

// Convert points and transformations between two coordinate systems.
// The mathematics involves a change of basis.  See the document
//   https://www.geometrictools.com/Documentation/ConvertingBetweenCoordinateSystems.pdf
// for the details.  Typical usage for 3D conversion is shown next.
//
// // Linear change of basis.  The columns of U are the basis vectors for the
// // source coordinate system.  A vector X = { x0, x1, x2 } in the source
// // coordinate system is represented by
// //   X = x0*(1,0,0) + x1*(0,1,0) + x2*(0,0,1)
// // The Cartesian coordinates for the point are the combination of these
// // terms,
// //   X = (x0, x1, x2)
// // The columns of V are the basis vectors for the target coordinate system.
// // A vector Y = { y0, y1, y2 } in the target coordinate system is
// // represented by
// //   Y = y0*(1,0,0,0) + y1*(0,0,1) + y2*(0,1,0)
// // The Cartesian coordinates for the vector are the combination of these
// // terms,
// //   Y = (y0, y2, y1)
// // The call Y = convert.UToV(X) computes y0, y1 and y2 so that the Cartesian
// // coordinates for X and for Y are the same.  For example,
// //   X = { 1.0, 2.0, 3.0 }
// //     = 1.0*(1,0,0) + 2.0*(0,1,0) + 3.0*(0,0,1)
// //     = (1, 2, 3)
// //   Y = { 1.0, 3.0, 2.0 }
// //     = 1.0*(1,0,0) + 3.0*(0,0,1) + 2.0*(0,1,0)
// //     = (1, 2, 3)
// // X and Y represent the same vector (equal Cartesian coordinates) but have
// // different representations in the source and target coordinates.
//
// ConvertCoordinates<3, double> convert;
// Vector<3, double> X, Y, P0, P1, diff;
// Matrix<3, 3, double> U, V, A, B;
// bool isRHU, isRHV;
// U.SetCol(0, Vector3<double>{1.0, 0.0, 0.0});
// U.SetCol(1, Vector3<double>{0.0, 1.0, 0.0});
// U.SetCol(2, Vector3<double>{0.0, 0.0, 1.0});
// V.SetCol(0, Vector3<double>{1.0, 0.0, 0.0});
// V.SetCol(1, Vector3<double>{0.0, 0.0, 1.0});
// V.SetCol(2, Vector3<double>{0.0, 1.0, 0.0});
// convert(U, true, V, true);
// isRHU = convert.IsRightHandedU();  // true
// isRHV = convert.IsRightHandedV();  // false
// X = { 1.0, 2.0, 3.0 };
// Y = convert.UToV(X);  // { 1.0, 3.0, 2.0 }
// P0 = U*X;
// P1 = V*Y;
// diff = P0 - P1;  // { 0, 0, 0 }
// Y = { 0.0, 1.0, 2.0 };
// X = convert.VToU(Y);  // { 0.0, 2.0, 1.0 }
// P0 = U*X;
// P1 = V*Y;
// diff = P0 - P1;  // { 0, 0, 0 }
// double cs = 0.6, sn = 0.8;  // cs*cs + sn*sn = 1
// A.SetCol(0, Vector3<double>{  c,   s, 0.0});
// A.SetCol(1, Vector3<double>{ -s,   c, 0.0});
// A.SetCol(2, Vector3<double>{0.0, 0.0, 1.0});
// B = convert.UToV(A);
//   // B.GetCol(0) = { c, 0, s}
//   // B.GetCol(1) = { 0, 1, 0}
//   // B.GetCol(2) = {-s, 0, c}
// X = A*X;  // U is VOR
// Y = B*Y;  // V is VOR
// P0 = U*X;
// P1 = V*Y;
// diff = P0 - P1;  // { 0, 0, 0 }
//
// // Affine change of basis.  The first three columns of U are the basis
// // vectors for the source coordinate system and must have last components
// // set to 0.  The last column is the origin for that system and must have
// // last component set to 1.  A point X = { x0, x1, x2, 1 } in the source
// // coordinate system is represented by
// //   X = x0*(-1,0,0,0) + x1*(0,0,1,0) + x2*(0,-1,0,0) + 1*(1,2,3,1)
// // The Cartesian coordinates for the point are the combination of these
// // terms,
// //   X = (-x0 + 1, -x2 + 2, x1 + 3, 1)
// // The first three columns of V are the basis vectors for the target
// // coordinate system and must have last components set to 0.  The last
// // column is the origin for that system and must have last component set
// // to 1.  A point Y = { y0, y1, y2, 1 } in the target coordinate system is
// // represented by
// //   Y = y0*(0,1,0,0) + y1*(-1,0,0,0) + y2*(0,0,1,0) + 1*(4,5,6,1)
// // The Cartesian coordinates for the point are the combination of these
// // terms,
// //   Y = (-y1 + 4, y0 + 5, y2 + 6, 1)
// // The call Y = convert.UToV(X) computes y0, y1 and y2 so that the Cartesian
// // coordinates for X and for Y are the same.  For example,
// //   X = { -1.0, 4.0, -3.0, 1.0 }
// //     = -1.0*(-1,0,0,0) + 4.0*(0,0,1,0) - 3.0*(0,-1,0,0) + 1.0*(1,2,3,1)
// //     = (2, 5, 7, 1)
// //   Y = { 0.0, 2.0, 1.0, 1.0 }
// //     = 0.0*(0,1,0,0) + 2.0*(-1,0,0,0) + 1.0*(0,0,1,0) + 1.0*(4,5,6,1)
// //     = (2, 5, 7, 1)
// // X and Y represent the same point (equal Cartesian coordinates) but have
// // different representations in the source and target affine coordinates.
//
// ConvertCoordinates<4, double> convert;
// Vector<4, double> X, Y, P0, P1, diff;
// Matrix<4, 4, double> U, V, A, B;
// bool isRHU, isRHV;
// U.SetCol(0, Vector4<double>{-1.0, 0.0, 0.0, 0.0});
// U.SetCol(1, Vector4<double>{0.0, 0.0, 1.0, 0.0});
// U.SetCol(2, Vector4<double>{0.0, -1.0, 0.0, 0.0});
// U.SetCol(3, Vector4<double>{1.0, 2.0, 3.0, 1.0});
// V.SetCol(0, Vector4<double>{0.0, 1.0, 0.0, 0.0});
// V.SetCol(1, Vector4<double>{-1.0, 0.0, 0.0, 0.0});
// V.SetCol(2, Vector4<double>{0.0, 0.0, 1.0, 0.0});
// V.SetCol(3, Vector4<double>{4.0, 5.0, 6.0, 1.0});
// convert(U, true, V, false);
// isRHU = convert.IsRightHandedU();  // false
// isRHV = convert.IsRightHandedV();  // true
// X = { -1.0, 4.0, -3.0, 1.0 };
// Y = convert.UToV(X);  // { 0.0, 2.0, 1.0, 1.0 }
// P0 = U*X;
// P1 = V*Y;
// diff = P0 - P1;  // { 0, 0, 0, 0 }
// Y = { 1.0, 2.0, 3.0, 1.0 };
// X = convert.VToU(Y);  // { -1.0, 6.0, -4.0, 1.0 }
// P0 = U*X;
// P1 = V*Y;
// diff = P0 - P1;  // { 0, 0, 0, 0 }
// double c = 0.6, s = 0.8;  // c*c + s*s = 1
// A.SetCol(0, Vector4<double>{  c,  s,   0.0, 0.0});
// A.SetCol(1, Vector4<double>{ -s,  c,   0.0, 0.0});
// A.SetCol(2, Vector4<double>{0.0, 0.0,  1.0, 0.0});
// A.SetCol(3, Vector4<double>{0.3, 1.0, -2.0, 1.0});
// B = convert.UToV(A);
// // B.GetCol(0) = {   1,    0,    0, 0 }
// // B.GetCol(1) = {   0,    c,    s, 0 }
// // B.GetCol(2) = {   0,   -s,    c, 0 }
// // B.GetCol(3) = { 2.0, -0.9, -2.6, 1 }
// X = A*X;  // U is VOR
// Y = Y*B;  // V is VOL (not VOR)
// P0 = U*X;
// P1 = V*Y;
// diff = P0 - P1;  // { 0, 0, 0, 0 }

namespace gte
{
    template <int N, typename Real>
    class ConvertCoordinates
    {
    public:
        // Construction of the change of basis matrix.  The implementation
        // supports both linear change of basis and affine change of basis.
        ConvertCoordinates()
            :
            mIsVectorOnRightU(true),
            mIsVectorOnRightV(true),
            mIsRightHandedU(true),
            mIsRightHandedV(true)
        {
            mC.MakeIdentity();
            mInverseC.MakeIdentity();
        }

        // Compute a change of basis between two coordinate systems.  The
        // return value is 'true' iff U and V are invertible.  The
        // matrix-vector multiplication conventions affect the conversion of
        // matrix transformations.  The Boolean inputs indicate how you want
        // the matrices to be interpreted when applied as transformations of
        // a vector.
        bool operator()(
            Matrix<N, N, Real> const& U, bool vectorOnRightU,
            Matrix<N, N, Real> const& V, bool vectorOnRightV)
        {
            // Initialize in case of early exit.
            mC.MakeIdentity();
            mInverseC.MakeIdentity();
            mIsVectorOnRightU = true;
            mIsVectorOnRightV = true;
            mIsRightHandedU = true;
            mIsRightHandedV = true;

            Matrix<N, N, Real> inverseU;
            Real determinantU;
            bool invertibleU = GaussianElimination<Real>()(N, &U[0], &inverseU[0],
                determinantU, nullptr, nullptr, nullptr, 0, nullptr);
            if (!invertibleU)
            {
                return false;
            }

            Matrix<N, N, Real> inverseV;
            Real determinantV;
            bool invertibleV = GaussianElimination<Real>()(N, &V[0], &inverseV[0],
                determinantV, nullptr, nullptr, nullptr, 0, nullptr);
            if (!invertibleV)
            {
                return false;
            }

            mC = inverseU * V;
            mInverseC = inverseV * U;
            mIsVectorOnRightU = vectorOnRightU;
            mIsVectorOnRightV = vectorOnRightV;
            mIsRightHandedU = (determinantU > (Real)0);
            mIsRightHandedV = (determinantV > (Real)0);
            return true;
        }

        // Member access.
        inline Matrix<N, N, Real> const& GetC() const
        {
            return mC;
        }

        inline Matrix<N, N, Real> const& GetInverseC() const
        {
            return mInverseC;
        }

        inline bool IsVectorOnRightU() const
        {
            return mIsVectorOnRightU;
        }

        inline bool IsVectorOnRightV() const
        {
            return mIsVectorOnRightV;
        }

        inline bool IsRightHandedU() const
        {
            return mIsRightHandedU;
        }

        inline bool IsRightHandedV() const
        {
            return mIsRightHandedV;
        }

        // Convert points between coordinate systems.  The names of the
        // systems are U and V to make it clear which inputs of operator()
        // they are associated with.  The X vector stores coordinates for the
        // U-system and the Y vector stores coordinates for the V-system.

        // Y = C^{-1}*X
        inline Vector<N, Real> UToV(Vector<N, Real> const& X) const
        {
            return mInverseC * X;
        }

        // X = C*Y
        inline Vector<N, Real> VToU(Vector<N, Real> const& Y) const
        {
            return mC * Y;
        }

        // Convert transformations between coordinate systems.  The outputs
        // are computed according to the tables shown before the function
        // declarations. The superscript T denotes the transpose operator.
        // vectorOnRightU = true:  transformation is X' = A*X
        // vectorOnRightU = false: transformation is (X')^T = X^T*A
        // vectorOnRightV = true:  transformation is Y' = B*Y
        // vectorOnRightV = false: transformation is (Y')^T = Y^T*B

        // vectorOnRightU  | vectorOnRightV  | output
        // ----------------+-----------------+---------------------
        // true            | true            | C^{-1} * A * C
        // true            | false           | (C^{-1} * A * C)^T 
        // false           | true            | C^{-1} * A^T * C
        // false           | false           | (C^{-1} * A^T * C)^T
        Matrix<N, N, Real> UToV(Matrix<N, N, Real> const& A) const
        {
            Matrix<N, N, Real> product;

            if (mIsVectorOnRightU)
            {
                product = mInverseC * A * mC;
                if (mIsVectorOnRightV)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
            else
            {
                product = mInverseC * MultiplyATB(A, mC);
                if (mIsVectorOnRightV)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
        }

        // vectorOnRightU  | vectorOnRightV  | output
        // ----------------+-----------------+---------------------
        // true            | true            | C * B * C^{-1}
        // true            | false           | C * B^T * C^{-1}
        // false           | true            | (C * B * C^{-1})^T
        // false           | false           | (C * B^T * C^{-1})^T
        Matrix<N, N, Real> VToU(Matrix<N, N, Real> const& B) const
        {
            // vectorOnRightU  | vectorOnRightV  | output
            // ----------------+-----------------+---------------------
            // true            | true            | C * B * C^{-1}
            // true            | false           | C * B^T * C^{-1}
            // false           | true            | (C * B * C^{-1})^T
            // false           | false           | (C * B^T * C^{-1})^T
            Matrix<N, N, Real> product;

            if (mIsVectorOnRightV)
            {
                product = mC * B * mInverseC;
                if (mIsVectorOnRightU)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
            else
            {
                product = mC * MultiplyATB(B, mInverseC);
                if (mIsVectorOnRightU)
                {
                    return product;
                }
                else
                {
                    return Transpose(product);
                }
            }
        }

    private:
        // C = U^{-1}*V, C^{-1} = V^{-1}*U
        Matrix<N, N, Real> mC, mInverseC;
        bool mIsVectorOnRightU, mIsVectorOnRightV;
        bool mIsRightHandedU, mIsRightHandedV;
    };
}
