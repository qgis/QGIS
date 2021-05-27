// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.16

#pragma once

#include <Mathematics/Math.h>
#include <algorithm>
#include <array>

// The document
// https://www.geometrictools.com/Documentation/RobustEigenSymmetric3x3.pdf
// describes algorithms for solving the eigensystem associated with a 3x3
// symmetric real-valued matrix.  The iterative algorithm is implemented
// by class SymmmetricEigensolver3x3.  The noniterative algorithm is
// implemented by class NISymmetricEigensolver3x3.  The code does not use
// GTEngine objects.

namespace gte
{
    template <typename Real>
    class SortEigenstuff
    {
    public:
        void operator()(int sortType, bool isRotation,
            std::array<Real, 3>& eval, std::array<std::array<Real, 3>, 3>& evec)
        {
            if (sortType != 0)
            {
                // Sort the eigenvalues to eval[0] <= eval[1] <= eval[2].
                std::array<size_t, 3> index;
                if (eval[0] < eval[1])
                {
                    if (eval[2] < eval[0])
                    {
                        // even permutation
                        index[0] = 2;
                        index[1] = 0;
                        index[2] = 1;
                    }
                    else if (eval[2] < eval[1])
                    {
                        // odd permutation
                        index[0] = 0;
                        index[1] = 2;
                        index[2] = 1;
                        isRotation = !isRotation;
                    }
                    else
                    {
                        // even permutation
                        index[0] = 0;
                        index[1] = 1;
                        index[2] = 2;
                    }
                }
                else
                {
                    if (eval[2] < eval[1])
                    {
                        // odd permutation
                        index[0] = 2;
                        index[1] = 1;
                        index[2] = 0;
                        isRotation = !isRotation;
                    }
                    else if (eval[2] < eval[0])
                    {
                        // even permutation
                        index[0] = 1;
                        index[1] = 2;
                        index[2] = 0;
                    }
                    else
                    {
                        // odd permutation
                        index[0] = 1;
                        index[1] = 0;
                        index[2] = 2;
                        isRotation = !isRotation;
                    }
                }

                if (sortType == -1)
                {
                    // The request is for eval[0] >= eval[1] >= eval[2]. This
                    // requires an odd permutation, (i0,i1,i2) -> (i2,i1,i0).
                    std::swap(index[0], index[2]);
                    isRotation = !isRotation;
                }

                std::array<Real, 3> unorderedEVal = eval;
                std::array<std::array<Real, 3>, 3> unorderedEVec = evec;
                for (size_t j = 0; j < 3; ++j)
                {
                    size_t i = index[j];
                    eval[j] = unorderedEVal[i];
                    evec[j] = unorderedEVec[i];
                }
            }

            // Ensure the ordered eigenvectors form a right-handed basis.
            if (!isRotation)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    evec[2][j] = -evec[2][j];
                }
            }
        }
    };

    template <typename Real>
    class SymmetricEigensolver3x3
    {
    public:
        // The input matrix must be symmetric, so only the unique elements
        // must be specified: a00, a01, a02, a11, a12, and a22.
        //
        // If 'aggressive' is 'true', the iterations occur until a
        // superdiagonal entry is exactly zero.  If 'aggressive' is 'false',
        // the iterations occur until a superdiagonal entry is effectively
        // zero compared to the/ sum of magnitudes of its diagonal neighbors.
        // Generally, the nonaggressive convergence is acceptable.
        //
        // The order of the eigenvalues is specified by sortType:
        // -1 (decreasing), 0 (no sorting) or +1 (increasing).  When sorted,
        // the eigenvectors are ordered accordingly, and
        // {evec[0], evec[1], evec[2]} is guaranteed to/ be a right-handed
        // orthonormal set.  The return value is the number of iterations
        // used by the algorithm.

        int operator()(Real a00, Real a01, Real a02, Real a11, Real a12, Real a22,
            bool aggressive, int sortType, std::array<Real, 3>& eval,
            std::array<std::array<Real, 3>, 3>& evec) const
        {
            // Compute the Householder reflection H and B = H*A*H, where
            // b02 = 0.
            Real const zero = (Real)0, one = (Real)1, half = (Real)0.5;
            bool isRotation = false;
            Real c, s;
            GetCosSin(a12, -a02, c, s);
            Real Q[3][3] = { { c, s, zero }, { s, -c, zero }, { zero, zero, one } };
            Real term0 = c * a00 + s * a01;
            Real term1 = c * a01 + s * a11;
            Real b00 = c * term0 + s * term1;
            Real b01 = s * term0 - c * term1;
            term0 = s * a00 - c * a01;
            term1 = s * a01 - c * a11;
            Real b11 = s * term0 - c * term1;
            Real b12 = s * a02 - c * a12;
            Real b22 = a22;

            // Givens reflections, B' = G^T*B*G, preserve tridiagonal
            // matrices.
            int const maxIteration = 2 * (1 + std::numeric_limits<Real>::digits -
                std::numeric_limits<Real>::min_exponent);
            int iteration;
            Real c2, s2;

            if (std::fabs(b12) <= std::fabs(b01))
            {
                Real saveB00, saveB01, saveB11;
                for (iteration = 0; iteration < maxIteration; ++iteration)
                {
                    // Compute the Givens reflection.
                    GetCosSin(half * (b00 - b11), b01, c2, s2);
                    s = std::sqrt(half * (one - c2));  // >= 1/sqrt(2)
                    c = half * s2 / s;

                    // Update Q by the Givens reflection.
                    Update0(Q, c, s);
                    isRotation = !isRotation;

                    // Update B <- Q^T*B*Q, ensuring that b02 is zero and
                    // |b12| has strictly decreased.
                    saveB00 = b00;
                    saveB01 = b01;
                    saveB11 = b11;
                    term0 = c * saveB00 + s * saveB01;
                    term1 = c * saveB01 + s * saveB11;
                    b00 = c * term0 + s * term1;
                    b11 = b22;
                    term0 = c * saveB01 - s * saveB00;
                    term1 = c * saveB11 - s * saveB01;
                    b22 = c * term1 - s * term0;
                    b01 = s * b12;
                    b12 = c * b12;

                    if (Converged(aggressive, b00, b11, b01))
                    {
                        // Compute the Householder reflection.
                        GetCosSin(half * (b00 - b11), b01, c2, s2);
                        s = std::sqrt(half * (one - c2));
                        c = half * s2 / s;  // >= 1/sqrt(2)

                        // Update Q by the Householder reflection.
                        Update2(Q, c, s);
                        isRotation = !isRotation;

                        // Update D = Q^T*B*Q.
                        saveB00 = b00;
                        saveB01 = b01;
                        saveB11 = b11;
                        term0 = c * saveB00 + s * saveB01;
                        term1 = c * saveB01 + s * saveB11;
                        b00 = c * term0 + s * term1;
                        term0 = s * saveB00 - c * saveB01;
                        term1 = s * saveB01 - c * saveB11;
                        b11 = s * term0 - c * term1;
                        break;
                    }
                }
            }
            else
            {
                Real saveB11, saveB12, saveB22;
                for (iteration = 0; iteration < maxIteration; ++iteration)
                {
                    // Compute the Givens reflection.
                    GetCosSin(half * (b22 - b11), b12, c2, s2);
                    s = std::sqrt(half * (one - c2));  // >= 1/sqrt(2)
                    c = half * s2 / s;

                    // Update Q by the Givens reflection.
                    Update1(Q, c, s);
                    isRotation = !isRotation;

                    // Update B <- Q^T*B*Q, ensuring that b02 is zero and
                    // |b12| has strictly decreased.  MODIFY...
                    saveB11 = b11;
                    saveB12 = b12;
                    saveB22 = b22;
                    term0 = c * saveB22 + s * saveB12;
                    term1 = c * saveB12 + s * saveB11;
                    b22 = c * term0 + s * term1;
                    b11 = b00;
                    term0 = c * saveB12 - s * saveB22;
                    term1 = c * saveB11 - s * saveB12;
                    b00 = c * term1 - s * term0;
                    b12 = s * b01;
                    b01 = c * b01;

                    if (Converged(aggressive, b11, b22, b12))
                    {
                        // Compute the Householder reflection.
                        GetCosSin(half * (b11 - b22), b12, c2, s2);
                        s = std::sqrt(half * (one - c2));
                        c = half * s2 / s;  // >= 1/sqrt(2)

                        // Update Q by the Householder reflection.
                        Update3(Q, c, s);
                        isRotation = !isRotation;

                        // Update D = Q^T*B*Q.
                        saveB11 = b11;
                        saveB12 = b12;
                        saveB22 = b22;
                        term0 = c * saveB11 + s * saveB12;
                        term1 = c * saveB12 + s * saveB22;
                        b11 = c * term0 + s * term1;
                        term0 = s * saveB11 - c * saveB12;
                        term1 = s * saveB12 - c * saveB22;
                        b22 = s * term0 - c * term1;
                        break;
                    }
                }
            }

            eval = { b00, b11, b22 };
            for (size_t row = 0; row < 3; ++row)
            {
                for (size_t col = 0; col < 3; ++col)
                {
                    evec[row][col] = Q[col][row];
                }
            }

            SortEigenstuff<Real>()(sortType, isRotation, eval, evec);
            return iteration;
        }

    private:
        // Update Q = Q*G in-place using G = {{c,0,-s},{s,0,c},{0,0,1}}.
        void Update0(Real Q[3][3], Real c, Real s) const
        {
            for (int r = 0; r < 3; ++r)
            {
                Real tmp0 = c * Q[r][0] + s * Q[r][1];
                Real tmp1 = Q[r][2];
                Real tmp2 = c * Q[r][1] - s * Q[r][0];
                Q[r][0] = tmp0;
                Q[r][1] = tmp1;
                Q[r][2] = tmp2;
            }
        }

        // Update Q = Q*G in-place using G = {{0,1,0},{c,0,s},{-s,0,c}}.
        void Update1(Real Q[3][3], Real c, Real s) const
        {
            for (int r = 0; r < 3; ++r)
            {
                Real tmp0 = c * Q[r][1] - s * Q[r][2];
                Real tmp1 = Q[r][0];
                Real tmp2 = c * Q[r][2] + s * Q[r][1];
                Q[r][0] = tmp0;
                Q[r][1] = tmp1;
                Q[r][2] = tmp2;
            }
        }

        // Update Q = Q*H in-place using H = {{c,s,0},{s,-c,0},{0,0,1}}.
        void Update2(Real Q[3][3], Real c, Real s) const
        {
            for (int r = 0; r < 3; ++r)
            {
                Real tmp0 = c * Q[r][0] + s * Q[r][1];
                Real tmp1 = s * Q[r][0] - c * Q[r][1];
                Q[r][0] = tmp0;
                Q[r][1] = tmp1;
            }
        }

        // Update Q = Q*H in-place using H = {{1,0,0},{0,c,s},{0,s,-c}}.
        void Update3(Real Q[3][3], Real c, Real s) const
        {
            for (int r = 0; r < 3; ++r)
            {
                Real tmp0 = c * Q[r][1] + s * Q[r][2];
                Real tmp1 = s * Q[r][1] - c * Q[r][2];
                Q[r][1] = tmp0;
                Q[r][2] = tmp1;
            }
        }

        // Normalize (u,v) robustly, avoiding floating-point overflow in the
        // sqrt call.  The normalized pair is (cs,sn) with cs <= 0.  If
        // (u,v) = (0,0), the function returns (cs,sn) = (-1,0).  When used
        // to generate a Householder reflection, it does not matter whether
        // (cs,sn) or (-cs,-sn) is used.  When generating a Givens reflection,
        // cs = cos(2*theta) and sn = sin(2*theta).  Having a negative cosine
        // for the double-angle term ensures that the single-angle terms
        // c = cos(theta) and s = sin(theta) satisfy |c| <= |s|.
        void GetCosSin(Real u, Real v, Real& cs, Real& sn) const
        {
            Real maxAbsComp = std::max(std::fabs(u), std::fabs(v));
            if (maxAbsComp > (Real)0)
            {
                u /= maxAbsComp;  // in [-1,1]
                v /= maxAbsComp;  // in [-1,1]
                Real length = std::sqrt(u * u + v * v);
                cs = u / length;
                sn = v / length;
                if (cs > (Real)0)
                {
                    cs = -cs;
                    sn = -sn;
                }
            }
            else
            {
                cs = (Real)-1;
                sn = (Real)0;
            }
        }

        // The convergence test.  When 'aggressive' is 'true', the
        // superdiagonal test is "bSuper == 0".  When 'aggressive' is 'false',
        // the superdiagonal test is
        //   |bDiag0| + |bDiag1| + |bSuper| == |bDiag0| + |bDiag1|
        // which means bSuper is effectively zero compared to the sizes of the
        // diagonal entries.
        bool Converged(bool aggressive, Real bDiag0, Real bDiag1, Real bSuper) const
        {
            if (aggressive)
            {
                return bSuper == (Real)0;
            }
            else
            {
                Real sum = std::fabs(bDiag0) + std::fabs(bDiag1);
                return sum + std::fabs(bSuper) == sum;
            }
        }
    };


    template <typename Real>
    class NISymmetricEigensolver3x3
    {
    public:
        // The input matrix must be symmetric, so only the unique elements
        // must be specified: a00, a01, a02, a11, a12, and a22.  The
        // eigenvalues are sorted in ascending order: eval0 <= eval1 <= eval2.

        void operator()(Real a00, Real a01, Real a02, Real a11, Real a12, Real a22,
            int sortType, std::array<Real, 3>& eval, std::array<std::array<Real, 3>, 3>& evec) const
        {
            // Precondition the matrix by factoring out the maximum absolute
            // value of the components.  This guards against floating-point
            // overflow when computing the eigenvalues.
            Real max0 = std::max(std::fabs(a00), std::fabs(a01));
            Real max1 = std::max(std::fabs(a02), std::fabs(a11));
            Real max2 = std::max(std::fabs(a12), std::fabs(a22));
            Real maxAbsElement = std::max(std::max(max0, max1), max2);
            if (maxAbsElement == (Real)0)
            {
                // A is the zero matrix.
                eval[0] = (Real)0;
                eval[1] = (Real)0;
                eval[2] = (Real)0;
                evec[0] = { (Real)1, (Real)0, (Real)0 };
                evec[1] = { (Real)0, (Real)1, (Real)0 };
                evec[2] = { (Real)0, (Real)0, (Real)1 };
                return;
            }

            Real invMaxAbsElement = (Real)1 / maxAbsElement;
            a00 *= invMaxAbsElement;
            a01 *= invMaxAbsElement;
            a02 *= invMaxAbsElement;
            a11 *= invMaxAbsElement;
            a12 *= invMaxAbsElement;
            a22 *= invMaxAbsElement;

            Real norm = a01 * a01 + a02 * a02 + a12 * a12;
            if (norm > (Real)0)
            {
                // Compute the eigenvalues of A.

                // In the PDF mentioned previously, B = (A - q*I)/p, where
                // q = tr(A)/3 with tr(A) the trace of A (sum of the diagonal
                // entries of A) and where p = sqrt(tr((A - q*I)^2)/6).
                Real q = (a00 + a11 + a22) / (Real)3;

                // The matrix A - q*I is represented by the following, where
                // b00, b11 and b22 are computed after these comments,
                //   +-           -+
                //   | b00 a01 a02 |
                //   | a01 b11 a12 |
                //   | a02 a12 b22 |
                //   +-           -+
                Real b00 = a00 - q;
                Real b11 = a11 - q;
                Real b22 = a22 - q;

                // The is the variable p mentioned in the PDF.
                Real p = std::sqrt((b00 * b00 + b11 * b11 + b22 * b22 + norm * (Real)2) / (Real)6);

                // We need det(B) = det((A - q*I)/p) = det(A - q*I)/p^3.  The
                // value det(A - q*I) is computed using a cofactor expansion
                // by the first row of A - q*I.  The cofactors are c00, c01
                // and c02 and the determinant is b00*c00 - a01*c01 + a02*c02.
                // The det(B) is then computed finally by the division
                // with p^3.
                Real c00 = b11 * b22 - a12 * a12;
                Real c01 = a01 * b22 - a12 * a02;
                Real c02 = a01 * a12 - b11 * a02;
                Real det = (b00 * c00 - a01 * c01 + a02 * c02) / (p * p * p);

                // The halfDet value is cos(3*theta) mentioned in the PDF. The
                // acos(z) function requires |z| <= 1, but will fail silently
                // and return NaN if the input is larger than 1 in magnitude.
                // To avoid this problem due to rounding errors, the halfDet
                // value is clamped to [-1,1].
                Real halfDet = det * (Real)0.5;
                halfDet = std::min(std::max(halfDet, (Real)-1), (Real)1);

                // The eigenvalues of B are ordered as
                // beta0 <= beta1 <= beta2.  The number of digits in
                // twoThirdsPi is chosen so that, whether float or double,
                // the floating-point number is the closest to theoretical
                // 2*pi/3.
                Real angle = std::acos(halfDet) / (Real)3;
                Real const twoThirdsPi = (Real)2.09439510239319549;
                Real beta2 = std::cos(angle) * (Real)2;
                Real beta0 = std::cos(angle + twoThirdsPi) * (Real)2;
                Real beta1 = -(beta0 + beta2);

                // The eigenvalues of A are ordered as
                // alpha0 <= alpha1 <= alpha2.
                eval[0] = q + p * beta0;
                eval[1] = q + p * beta1;
                eval[2] = q + p * beta2;

                // Compute the eigenvectors so that the set
                // {evec[0], evec[1], evec[2]} is right handed and
                // orthonormal.
                if (halfDet >= (Real)0)
                {
                    ComputeEigenvector0(a00, a01, a02, a11, a12, a22, eval[2], evec[2]);
                    ComputeEigenvector1(a00, a01, a02, a11, a12, a22, evec[2], eval[1], evec[1]);
                    evec[0] = Cross(evec[1], evec[2]);
                }
                else
                {
                    ComputeEigenvector0(a00, a01, a02, a11, a12, a22, eval[0], evec[0]);
                    ComputeEigenvector1(a00, a01, a02, a11, a12, a22, evec[0], eval[1], evec[1]);
                    evec[2] = Cross(evec[0], evec[1]);
                }
            }
            else
            {
                // The matrix is diagonal.
                eval[0] = a00;
                eval[1] = a11;
                eval[2] = a22;
                evec[0] = { (Real)1, (Real)0, (Real)0 };
                evec[1] = { (Real)0, (Real)1, (Real)0 };
                evec[2] = { (Real)0, (Real)0, (Real)1 };
            }

            // The preconditioning scaled the matrix A, which scales the
            // eigenvalues.  Revert the scaling.
            eval[0] *= maxAbsElement;
            eval[1] *= maxAbsElement;
            eval[2] *= maxAbsElement;

            SortEigenstuff<Real>()(sortType, true, eval, evec);
        }

    private:
        static std::array<Real, 3> Multiply(Real s, std::array<Real, 3> const& U)
        {
            std::array<Real, 3> product = { s * U[0], s * U[1], s * U[2] };
            return product;
        }

        static std::array<Real, 3> Subtract(std::array<Real, 3> const& U, std::array<Real, 3> const& V)
        {
            std::array<Real, 3> difference = { U[0] - V[0], U[1] - V[1], U[2] - V[2] };
            return difference;
        }

        static std::array<Real, 3> Divide(std::array<Real, 3> const& U, Real s)
        {
            Real invS = (Real)1 / s;
            std::array<Real, 3> division = { U[0] * invS, U[1] * invS, U[2] * invS };
            return division;
        }

        static Real Dot(std::array<Real, 3> const& U, std::array<Real, 3> const& V)
        {
            Real dot = U[0] * V[0] + U[1] * V[1] + U[2] * V[2];
            return dot;
        }

        static std::array<Real, 3> Cross(std::array<Real, 3> const& U, std::array<Real, 3> const& V)
        {
            std::array<Real, 3> cross =
            {
                U[1] * V[2] - U[2] * V[1],
                U[2] * V[0] - U[0] * V[2],
                U[0] * V[1] - U[1] * V[0]
            };
            return cross;
        }

        void ComputeOrthogonalComplement(std::array<Real, 3> const& W,
            std::array<Real, 3>& U, std::array<Real, 3>& V) const
        {
            // Robustly compute a right-handed orthonormal set { U, V, W }.
            // The vector W is guaranteed to be unit-length, in which case
            // there is no need to worry about a division by zero when
            // computing invLength.
            Real invLength;
            if (std::fabs(W[0]) > std::fabs(W[1]))
            {
                // The component of maximum absolute value is either W[0]
                // or W[2].
                invLength = (Real)1 / std::sqrt(W[0] * W[0] + W[2] * W[2]);
                U = { -W[2] * invLength, (Real)0, +W[0] * invLength };
            }
            else
            {
                // The component of maximum absolute value is either W[1]
                // or W[2].
                invLength = (Real)1 / std::sqrt(W[1] * W[1] + W[2] * W[2]);
                U = { (Real)0, +W[2] * invLength, -W[1] * invLength };
            }
            V = Cross(W, U);
        }

        void ComputeEigenvector0(Real a00, Real a01, Real a02, Real a11, Real a12, Real a22,
            Real eval0, std::array<Real, 3>& evec0) const
        {
            // Compute a unit-length eigenvector for eigenvalue[i0].  The
            // matrix is rank 2, so two of the rows are linearly independent.
            // For a robust computation of the eigenvector, select the two
            // rows whose cross product has largest length of all pairs of
            // rows.
            std::array<Real, 3> row0 = { a00 - eval0, a01, a02 };
            std::array<Real, 3> row1 = { a01, a11 - eval0, a12 };
            std::array<Real, 3> row2 = { a02, a12, a22 - eval0 };
            std::array<Real, 3>  r0xr1 = Cross(row0, row1);
            std::array<Real, 3>  r0xr2 = Cross(row0, row2);
            std::array<Real, 3>  r1xr2 = Cross(row1, row2);
            Real d0 = Dot(r0xr1, r0xr1);
            Real d1 = Dot(r0xr2, r0xr2);
            Real d2 = Dot(r1xr2, r1xr2);

            Real dmax = d0;
            int imax = 0;
            if (d1 > dmax)
            {
                dmax = d1;
                imax = 1;
            }
            if (d2 > dmax)
            {
                imax = 2;
            }

            if (imax == 0)
            {
                evec0 = Divide(r0xr1, std::sqrt(d0));
            }
            else if (imax == 1)
            {
                evec0 = Divide(r0xr2, std::sqrt(d1));
            }
            else
            {
                evec0 = Divide(r1xr2, std::sqrt(d2));
            }
        }

        void ComputeEigenvector1(Real a00, Real a01, Real a02, Real a11, Real a12, Real a22,
            std::array<Real, 3> const& evec0, Real eval1, std::array<Real, 3>& evec1) const
        {
            // Robustly compute a right-handed orthonormal set
            // { U, V, evec0 }.
            std::array<Real, 3> U, V;
            ComputeOrthogonalComplement(evec0, U, V);

            // Let e be eval1 and let E be a corresponding eigenvector which
            // is a solution to the linear system (A - e*I)*E = 0.  The matrix
            // (A - e*I) is 3x3, not invertible (so infinitely many
            // solutions), and has rank 2 when eval1 and eval are different.
            // It has rank 1 when eval1 and eval2 are equal.  Numerically, it
            // is difficult to compute robustly the rank of a matrix.  Instead,
            // the 3x3 linear system is reduced to a 2x2 system as follows.
            // Define the 3x2 matrix J = [U V] whose columns are the U and V
            // computed previously.  Define the 2x1 vector X = J*E.  The 2x2
            // system is 0 = M * X = (J^T * (A - e*I) * J) * X where J^T is
            // the transpose of J and M = J^T * (A - e*I) * J is a 2x2 matrix.
            // The system may be written as
            //     +-                        -++-  -+       +-  -+
            //     | U^T*A*U - e  U^T*A*V     || x0 | = e * | x0 |
            //     | V^T*A*U      V^T*A*V - e || x1 |       | x1 |
            //     +-                        -++   -+       +-  -+
            // where X has row entries x0 and x1.

            std::array<Real, 3> AU =
            {
                a00 * U[0] + a01 * U[1] + a02 * U[2],
                a01 * U[0] + a11 * U[1] + a12 * U[2],
                a02 * U[0] + a12 * U[1] + a22 * U[2]
            };

            std::array<Real, 3> AV =
            {
                a00 * V[0] + a01 * V[1] + a02 * V[2],
                a01 * V[0] + a11 * V[1] + a12 * V[2],
                a02 * V[0] + a12 * V[1] + a22 * V[2]
            };

            Real m00 = U[0] * AU[0] + U[1] * AU[1] + U[2] * AU[2] - eval1;
            Real m01 = U[0] * AV[0] + U[1] * AV[1] + U[2] * AV[2];
            Real m11 = V[0] * AV[0] + V[1] * AV[1] + V[2] * AV[2] - eval1;

            // For robustness, choose the largest-length row of M to compute
            // the eigenvector.  The 2-tuple of coefficients of U and V in the
            // assignments to eigenvector[1] lies on a circle, and U and V are
            // unit length and perpendicular, so eigenvector[1] is unit length
            // (within numerical tolerance).
            Real absM00 = std::fabs(m00);
            Real absM01 = std::fabs(m01);
            Real absM11 = std::fabs(m11);
            Real maxAbsComp;
            if (absM00 >= absM11)
            {
                maxAbsComp = std::max(absM00, absM01);
                if (maxAbsComp > (Real)0)
                {
                    if (absM00 >= absM01)
                    {
                        m01 /= m00;
                        m00 = (Real)1 / std::sqrt((Real)1 + m01 * m01);
                        m01 *= m00;
                    }
                    else
                    {
                        m00 /= m01;
                        m01 = (Real)1 / std::sqrt((Real)1 + m00 * m00);
                        m00 *= m01;
                    }
                    evec1 = Subtract(Multiply(m01, U), Multiply(m00, V));
                }
                else
                {
                    evec1 = U;
                }
            }
            else
            {
                maxAbsComp = std::max(absM11, absM01);
                if (maxAbsComp > (Real)0)
                {
                    if (absM11 >= absM01)
                    {
                        m01 /= m11;
                        m11 = (Real)1 / std::sqrt((Real)1 + m01 * m01);
                        m01 *= m11;
                    }
                    else
                    {
                        m11 /= m01;
                        m01 = (Real)1 / std::sqrt((Real)1 + m11 * m11);
                        m11 *= m01;
                    }
                    evec1 = Subtract(Multiply(m11, U), Multiply(m01, V));
                }
                else
                {
                    evec1 = U;
                }
            }
        }
    };
}
