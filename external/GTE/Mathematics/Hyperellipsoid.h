// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/SymmetricEigensolver.h>

// A hyperellipsoid has center K; axis directions U[0] through U[N-1], all
// unit-length vectors; and extents e[0] through e[N-1], all positive numbers.
// A point X = K + sum_{d=0}^{N-1} y[d]*U[d] is on the hyperellipsoid whenever
// sum_{d=0}^{N-1} (y[d]/e[d])^2 = 1.  An algebraic representation for the
// hyperellipsoid is (X-K)^T * M * (X-K) = 1, where M is the NxN symmetric
// matrix M = sum_{d=0}^{N-1} U[d]*U[d]^T/e[d]^2, where the superscript T
// denotes transpose.  Observe that U[i]*U[i]^T is a matrix, not a scalar dot
// product.  The hyperellipsoid is also represented by a quadratic equation
// 0 = C + B^T*X + X^T*A*X, where C is a scalar, B is an Nx1 vector, and A is
// an NxN symmetric matrix with positive eigenvalues.  The coefficients can be
// stored from lowest degree to highest degree,
//   C = k[0]
//   B = k[1], ..., k[N]
//   A = k[N+1], ..., k[(N+1)(N+2)/2 - 1]
// where the A-coefficients are the upper-triangular elements of A listed in
// row-major order.  For N = 2, X = (x[0],x[1]) and
//   0 = k[0] +
//       k[1]*x[0] + k[2]*x[1] + 
//       k[3]*x[0]*x[0] + k[4]*x[0]*x[1]
//                      + k[5]*x[1]*x[1]
// For N = 3, X = (x[0],x[1],x[2]) and
//   0 = k[0] +
//       k[1]*x[0] + k[2]*x[1] + k[3]*x[2] +
//       k[4]*x[0]*x[0] + k[5]*x[0]*x[1] + k[6]*x[0]*x[2] +
//                      + k[7]*x[1]*x[1] + k[8]*x[1]*x[2] +
//                                       + k[9]*x[2]*x[2]
// This equation can be factored to the form (X-K)^T * M * (X-K) = 1, where
// K = -A^{-1}*B/2, M = A/(B^T*A^{-1}*B/4-C).

namespace gte
{
    template <int N, typename Real>
    class Hyperellipsoid
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // center to Vector<N,Real>::Zero(), the axes to
        // Vector<N,Real>::Unit(d), and all extents to 1.
        Hyperellipsoid()
        {
            center.MakeZero();
            for (int d = 0; d < N; ++d)
            {
                axis[d].MakeUnit(d);
                extent[d] = (Real)1;
            }
        }

        Hyperellipsoid(Vector<N, Real> const& inCenter,
            std::array<Vector<N, Real>, N> const inAxis,
            Vector<N, Real> const& inExtent)
            :
            center(inCenter),
            axis(inAxis),
            extent(inExtent)
        {
        }

        // Compute M = sum_{d=0}^{N-1} U[d]*U[d]^T/e[d]^2.
        void GetM(Matrix<N, N, Real>& M) const
        {
            M.MakeZero();
            for (int d = 0; d < N; ++d)
            {
                Vector<N, Real> ratio = axis[d] / extent[d];
                M += OuterProduct<N, N, Real>(ratio, ratio);
            }
        }

        // Compute M^{-1} = sum_{d=0}^{N-1} U[d]*U[d]^T*e[d]^2.
        void GetMInverse(Matrix<N, N, Real>& MInverse) const
        {
            MInverse.MakeZero();
            for (int d = 0; d < N; ++d)
            {
                Vector<N, Real> product = axis[d] * extent[d];
                MInverse += OuterProduct<N, N, Real>(product, product);
            }
        }

        // Construct the coefficients in the quadratic equation that represents
        // the hyperellipsoid.
        void ToCoefficients(std::array<Real, (N + 1) * (N + 2) / 2> & coeff) const
        {
            int const numCoefficients = (N + 1) * (N + 2) / 2;
            Matrix<N, N, Real> A;
            Vector<N, Real> B;
            Real C;
            ToCoefficients(A, B, C);
            Convert(A, B, C, coeff);

            // Arrange for one of the coefficients of the quadratic terms
            // to be 1.
            int quadIndex = numCoefficients - 1;
            int maxIndex = quadIndex;
            Real maxValue = std::fabs(coeff[quadIndex]);
            for (int d = 2; d < N; ++d)
            {
                quadIndex -= d;
                Real absValue = std::fabs(coeff[quadIndex]);
                if (absValue > maxValue)
                {
                    maxIndex = quadIndex;
                    maxValue = absValue;
                }
            }

            Real invMaxValue = (Real)1 / maxValue;
            for (int i = 0; i < numCoefficients; ++i)
            {
                if (i != maxIndex)
                {
                    coeff[i] *= invMaxValue;
                }
                else
                {
                    coeff[i] = (Real)1;
                }
            }
        }

        void ToCoefficients(Matrix<N, N, Real>& A, Vector<N, Real>& B, Real& C) const
        {
            GetM(A);
            Vector<N, Real> product = A * center;
            B = (Real)-2 * product;
            C = Dot(center, product) - (Real)1;
        }

        // Construct C, U[i], and e[i] from the equation.  The return value is
        // 'true' if and only if the input coefficients represent a
        // hyperellipsoid.  If the function returns 'false', the hyperellipsoid
        // data members are undefined.
        bool FromCoefficients(std::array<Real, (N + 1) * (N + 2) / 2> const& coeff)
        {
            Matrix<N, N, Real> A;
            Vector<N, Real> B;
            Real C;
            Convert(coeff, A, B, C);
            return FromCoefficients(A, B, C);
        }

        bool FromCoefficients(Matrix<N, N, Real> const& A, Vector<N, Real> const& B, Real C)
        {
            // Compute the center K = -A^{-1}*B/2.
            bool invertible;
            Matrix<N, N, Real> invA = Inverse(A, &invertible);
            if (!invertible)
            {
                return false;
            }

            center = ((Real)-0.5) * (invA * B);

            // Compute B^T*A^{-1}*B/4 - C = K^T*A*K - C = -K^T*B/2 - C.
            Real rightSide = (Real)-0.5 * Dot(center, B) - C;
            if (rightSide == (Real)0)
            {
                return false;
            }

            // Compute M = A/(K^T*A*K - C).
            Real invRightSide = (Real)1 / rightSide;
            Matrix<N, N, Real> M = invRightSide * A;

            // Factor into M = R*D*R^T.  M is symmetric, so it does not matter whether
            // the matrix is stored in row-major or column-major order; they are
            // equivalent.  The output R, however, is in row-major order.
            SymmetricEigensolver<Real> es(N, 32);
            Matrix<N, N, Real> rotation;
            std::array<Real, N> diagonal;
            es.Solve(&M[0], +1);  // diagonal[i] are nondecreasing
            es.GetEigenvalues(&diagonal[0]);
            es.GetEigenvectors(&rotation[0]);
            if (es.GetEigenvectorMatrixType() == 0)
            {
                auto negLast = -rotation.GetCol(N - 1);
                rotation.SetCol(N - 1, negLast);
            }

            for (int d = 0; d < N; ++d)
            {
                if (diagonal[d] <= (Real)0)
                {
                    return false;
                }

                extent[d] = (Real)1 / std::sqrt(diagonal[d]);
                axis[d] = rotation.GetCol(d);
            }

            return true;
        }

        // Public member access.
        Vector<N, Real> center;
        std::array<Vector<N, Real>, N> axis;
        Vector<N, Real> extent;

    private: 
        static void Convert(std::array<Real, (N + 1) * (N + 2) / 2> const& coeff,
            Matrix<N, N, Real>& A, Vector<N, Real>& B, Real& C)
        {
            int i = 0;
            C = coeff[i++];

            for (int j = 0; j < N; ++j)
            {
                B[j] = coeff[i++];
            }

            for (int r = 0; r < N; ++r)
            {
                for (int c = 0; c < r; ++c)
                {
                    A(r, c) = A(c, r);
                }

                A(r, r) = coeff[i++];

                for (int c = r + 1; c < N; ++c)
                {
                    A(r, c) = coeff[i++] * (Real)0.5;
                }
            }
        }

        static void Convert(Matrix<N, N, Real> const& A, Vector<N, Real> const& B,
            Real C, std::array<Real, (N + 1) * (N + 2) / 2> & coeff)
        {
            int i = 0;
            coeff[i++] = C;

            for (int j = 0; j < N; ++j)
            {
                coeff[i++] = B[j];
            }

            for (int r = 0; r < N; ++r)
            {
                coeff[i++] = A(r, r);
                for (int c = r + 1; c < N; ++c)
                {
                    coeff[i++] = A(r, c) * (Real)2;
                }
            }
        }

    public:
        // Comparisons to support sorted containers.
        bool operator==(Hyperellipsoid const& hyperellipsoid) const
        {
            return center == hyperellipsoid.center && axis == hyperellipsoid.axis
                && extent == hyperellipsoid.extent;
        }

        bool operator!=(Hyperellipsoid const& hyperellipsoid) const
        {
            return !operator==(hyperellipsoid);
        }

        bool operator< (Hyperellipsoid const& hyperellipsoid) const
        {
            if (center < hyperellipsoid.center)
            {
                return true;
            }

            if (center > hyperellipsoid.center)
            {
                return false;
            }

            if (axis < hyperellipsoid.axis)
            {
                return true;
            }

            if (axis > hyperellipsoid.axis)
            {
                return false;
            }

            return extent < hyperellipsoid.extent;
        }

        bool operator<=(Hyperellipsoid const& hyperellipsoid) const
        {
            return !hyperellipsoid.operator<(*this);
        }

        bool operator> (Hyperellipsoid const& hyperellipsoid) const
        {
            return hyperellipsoid.operator<(*this);
        }

        bool operator>=(Hyperellipsoid const& hyperellipsoid) const
        {
            return !operator<(hyperellipsoid);
        }
    };

    // Template aliases for convenience.
    template <typename Real>
    using Ellipse2 = Hyperellipsoid<2, Real>;

    template <typename Real>
    using Ellipsoid3 = Hyperellipsoid<3, Real>;
}
