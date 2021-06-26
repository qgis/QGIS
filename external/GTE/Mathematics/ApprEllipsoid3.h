// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.10.10

#pragma once

// An ellipsoid is defined implicitly by (X-C)^T * M * (X-C) = 1, where C is
// the center, M is a positive definite matrix and X is any point on the
// ellipsoid. The code implements a nonlinear least-squares fitting algorithm
// for the error function
//   F(C,M) = sum_{i=0}^{n-1} ((X[i] - C)^T * M * (X[i] - C) - 1)^2
// for n data points X[0] through X[n-1]. An Ellipsoid3<Real> object has
// member 'center' that corresponds to C. It also has axes with unit-length
// directions 'axis[]' and corresponding axis half-lengths 'extent[]'. The
// matrix is M = sum_{i=0}^2 axis[i] * axis[i]^T / extent[i]^2, where axis[i]
// is a 3x1 vector and axis[i]^T is a 1x3 vector.
//
// The minimizer uses a 2-step gradient descent algorithm.
//
// Given the current (C,M), locate a minimum of
//   G(t) = F(C - t * dF(C,M)/dC, M)
// for t > 0. The function G(t) >= 0 is a polynomial of degree 4 with
// derivative G'(t) that is a polynomial of degree 3. G'(t) must have a
// positive root because G(0) > 0 and G'(0) < 0 and the G-coefficient of t^4
// is positive. The positive root that T produces the smallest G-value is used
// to update the center C' = C - T * dF/dC(C,M).
//
// Given the current (C,M), locate a minimum of
//   H(t) = F(C, M - t * dF(C,M)/dM)
// for t > 0. The function H(t) >= 0 is a polynomial of degree 2 with
// derivative H'(t) that is a polynomial of degree 1. H'(t) must have a
// positive root because H(0) > 0 and H'(0) < 0 and the H-coefficient of t^2
// is positive. The positive root T that produces the smallest G-value is used
// to update the matrix M' = M - T * dF/dC(C,M) as long as M' is positive
// definite. If M' is not positive definite, the root is halved for a finite
// number of steps until M' is positive definite.

#include <Mathematics/ContOrientedBox3.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/RootsPolynomial.h>

namespace gte
{
    template <typename Real>
    class ApprEllipsoid3
    {
    public:
        // If you want this function to compute the initial guess for the
        // ellipsoid, set 'useEllipsoidForInitialGuess' to true. An oriented
        // bounding box containing the points is used to start the minimizer.
        // Set 'useEllipsoidForInitialGuess' to true if you want the initial
        // guess to be the input ellipsoid. This is useful if you want to
        // repeat the query. The returned 'Real' value is the error function
        // value for the output 'ellisoid'.
        Real operator()(std::vector<Vector3<Real>> const& points,
            size_t numIterations, bool useEllipsoidForInitialGuess,
            Ellipsoid3<Real>& ellipsoid)
        {
            Vector3<Real> C;
            Matrix3x3<Real> M;  // the zero matrix
            if (useEllipsoidForInitialGuess)
            {
                C = ellipsoid.center;
                for (int i = 0; i < 3; ++i)
                {
                    auto product = OuterProduct(ellipsoid.axis[i], ellipsoid.axis[i]);
                    M += product / (ellipsoid.extent[i] * ellipsoid.extent[i]);
                }
            }
            else
            {
                OrientedBox3<Real> box;
                GetContainer(static_cast<int>(points.size()), points.data(), box);
                C = box.center;
                for (int i = 0; i < 3; ++i)
                {
                    auto product = OuterProduct(box.axis[i], box.axis[i]);
                    M += product / (box.extent[i] * box.extent[i]);
                }
            }

            Real error = ErrorFunction(points, C, M);
            for (size_t i = 0; i < numIterations; ++i)
            {
                error = UpdateMatrix(points, C, M);
                error = UpdateCenter(points, M, C);
            }

            // Extract the ellipsoid axes and extents.
            SymmetricEigensolver3x3<Real> solver;
            std::array<Real, 3> eval;
            std::array<std::array<Real, 3>, 3> evec;
            solver(M(0, 0), M(0, 1), M(0, 2), M(1, 1), M(1, 2), M(2, 2),
                true, +1, eval, evec);

            Real const one = static_cast<Real>(1);
            ellipsoid.center = C;
            for (int i = 0; i < 3; ++i)
            {
                ellipsoid.axis[i] = { evec[i][0], evec[i][1], evec[i][2] };
                ellipsoid.extent[i] = one / std::sqrt(eval[i]);
            }

            return error;
        }

    private:
        Real UpdateCenter(std::vector<Vector3<Real>> const& points,
            Matrix3x3<Real> const& M, Vector3<Real>& C)
        {
            Real const zero = static_cast<Real>(0);
            Real const one = static_cast<Real>(1);
            Real const two = static_cast<Real>(2);
            Real const three = static_cast<Real>(3);
            Real const four = static_cast<Real>(4);
            Real const epsilon = static_cast<Real>(1e-06);

            std::vector<Vector3<Real>> MDelta(points.size());
            std::vector<Real> a(points.size());
            Real invQuantity = one / static_cast<Real>(points.size());
            Vector3<Real> negDFDC = Vector3<Real>::Zero();
            Real aMean = zero, aaMean = zero;
            for (size_t i = 0; i < points.size(); ++i)
            {
                Vector3<Real> Delta = points[i] - C;
                MDelta[i] = M * Delta;
                a[i] = Dot(Delta, MDelta[i]) - one;
                aMean += a[i];
                aaMean += a[i] * a[i];
                negDFDC += a[i] * MDelta[i];
            }
            aMean *= invQuantity;
            aaMean *= invQuantity;
            if (Normalize(negDFDC) < epsilon)
            {
                return aaMean;
            }

            Real bMean = zero, abMean = zero, bbMean = zero;
            Real c = Dot(negDFDC, M * negDFDC);
            for (size_t i = 0; i < points.size(); ++i)
            {
                Real b = Dot(negDFDC, MDelta[i]);
                bMean += b;
                abMean += a[i] * b;
                bbMean += b * b;
            }
            bMean *= invQuantity;
            abMean *= invQuantity;
            bbMean *= invQuantity;

            // Compute the coefficients of the quartic polynomial q(t) that
            // represents the error function on the given line in the gradient
            // descent minimization.
            std::array<Real, 5> q;
            q[0] = aaMean;
            q[1] = -four * abMean;
            q[2] = four * bbMean + two * c * aMean;
            q[3] = -four * c * bMean;
            q[4] = c * c;

            // Compute the coefficients of q'(t).
            std::array<Real, 4> dq;
            dq[0] = q[1];
            dq[1] = two * q[2];
            dq[2] = three * q[3];
            dq[3] = four * q[4];

            // Compute the roots of q'(t).
            std::map<Real, int> rmMap;
            RootsPolynomial<Real>::SolveCubic(dq[0], dq[1], dq[2], dq[3], rmMap);

            // Choose the root that leads to the minimum along the gradient
            // descent line and update the center to that point.
            Real minError = aaMean;
            Real minRoot = zero;
            for (auto const& rm : rmMap)
            {
                Real root = rm.first;
                if (root > zero)
                {
                    Real error = q[0] + root * (q[1] + root * (q[2] + root * (q[3] + root * q[4])));
                    if (error < minError)
                    {
                        minError = error;
                        minRoot = root;
                    }
                }
            }

            if (minRoot > zero)
            {
                C += minRoot * negDFDC;
                return minError;
            }
            return aaMean;
        }

        Real UpdateMatrix(std::vector<Vector3<Real>> const& points,
            Vector3<Real> const& C, Matrix3x3<Real>& M)
        {
            Real const zero = static_cast<Real>(0);
            Real const one = static_cast<Real>(1);
            Real const two = static_cast<Real>(2);
            Real const half = static_cast<Real>(0.5);
            Real const epsilon = static_cast<Real>(1e-06);

            std::vector<Vector3<Real>> Delta(points.size());
            std::vector<Real> a(points.size());
            Real invQuantity = one / static_cast<Real>(points.size());
            Matrix3x3<Real> negDFDM;  // zero matrix, symmetric

            Real aaMean = zero;
            for (size_t i = 0; i < points.size(); ++i)
            {
                Delta[i] = points[i] - C;
                a[i] = Dot(Delta[i], M * Delta[i]) - one;
                Real twoA = two * a[i];
                negDFDM(0, 0) -= a[i] * Delta[i][0] * Delta[i][0];
                negDFDM(0, 1) -= twoA * Delta[i][0] * Delta[i][1];
                negDFDM(0, 2) -= twoA * Delta[i][0] * Delta[i][2];
                negDFDM(1, 1) -= a[i] * Delta[i][1] * Delta[i][1];
                negDFDM(1, 2) -= twoA * Delta[i][1] * Delta[i][2];
                negDFDM(2, 2) -= a[i] * Delta[i][2] * Delta[i][2];
                aaMean += a[i] * a[i];
            }
            aaMean *= invQuantity;

            // Normalize the matrix as if it were a vector of numbers.
            Real length = std::sqrt(
                negDFDM(0, 0) * negDFDM(0, 0) + negDFDM(0, 1) * negDFDM(0, 1) +
                negDFDM(0, 2) * negDFDM(0, 2) + negDFDM(1, 1) * negDFDM(1, 1) +
                negDFDM(1, 2) * negDFDM(1, 2) + negDFDM(2, 2) * negDFDM(2, 2));
            if (length < epsilon)
            {
                return aaMean;
            }
            Real invLength = one / length;
            negDFDM(0, 0) *= invLength;
            negDFDM(0, 1) *= invLength;
            negDFDM(0, 2) *= invLength;
            negDFDM(1, 1) *= invLength;
            negDFDM(1, 2) *= invLength;
            negDFDM(2, 2) *= invLength;

            // Fill in the lower triangular portion because negGradM is a
            // symmetric matrix.
            negDFDM(1, 0) = negDFDM(0, 1);
            negDFDM(2, 0) = negDFDM(0, 2);
            negDFDM(2, 1) = negDFDM(1, 2);

            Real abMean = zero, bbMean = zero;
            for (size_t i = 0; i < points.size(); ++i)
            {
                Real b = Dot(Delta[i], negDFDM * Delta[i]);
                abMean += a[i] * b;
                bbMean += b * b;
            }
            abMean *= invQuantity;
            bbMean *= invQuantity;

            // Compute the coefficients of the quadratic polynomial q(t) that
            // represents the error function on the given line in the gradient
            // descent minimization.
            std::array<Real, 3> q;
            q[0] = aaMean;
            q[1] = two * abMean;
            q[2] = bbMean;

            // Compute the coefficients of q'(t).
            std::array<Real, 2> dq;
            dq[0] = q[1];
            dq[1] = two * q[2];

            // Compute the root as long as it is positive and
            // M + root * negGradM is a positive definite matrix.
            Real root = -dq[0] / dq[1];
            if (root > zero)
            {
                // Use Sylvester's criterion for testing positive definitess.
                // A for(;;) loop terminates for floating-point arithmetic but
                // not for rational (BSRational<UInteger>) arithmetic. Limit
                // the number of iterations so that the loop terminates for
                // rational arithmetic but 'return' occurs for floating-point
                // arithmetic.
                for (size_t k = 0; k < 2048; ++k)
                {
                    Matrix3x3<Real> nextM = M + root * negDFDM;
                    if (nextM(0, 0) > zero)
                    {
                        Real det = nextM(0, 0) * nextM(1, 1) - nextM(0, 1) * nextM(1, 0);
                        if (det > zero)
                        {
                            det = Determinant(nextM);
                            if (det > zero)
                            {
                                M = nextM;
                                Real minError = q[0] + root * (q[1] + root * q[2]);
                                return minError;
                            }
                        }
                    }
                    root *= half;
                }
            }
            return aaMean;
        }

        Real ErrorFunction(std::vector<Vector3<Real>> const& points,
            Vector3<Real> const& C, Matrix3x3<Real> const& M) const
        {
            Real error = static_cast<Real>(0);
            for (auto const& P : points)
            {
                Vector3<Real> Delta = P - C;
                Real a = Dot(Delta, M * Delta) - static_cast<Real>(1);
                error += a * a;
            }
            error /= static_cast<Real>(points.size());
            return error;
        }
    };
}
