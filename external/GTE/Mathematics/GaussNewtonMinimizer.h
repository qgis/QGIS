// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.13

#pragma once

#include <Mathematics/CholeskyDecomposition.h>
#include <functional>

// Let F(p) = (F_{0}(p), F_{1}(p), ..., F_{n-1}(p)) be a vector-valued
// function of the parameters p = (p_{0}, p_{1}, ..., p_{m-1}).  The
// nonlinear least-squares problem is to minimize the real-valued error
// function E(p) = |F(p)|^2, which is the squared length of F(p).
//
// Let J = dF/dp = [dF_{r}/dp_{c}] denote the Jacobian matrix, which is the
// matrix of first-order partial derivatives of F.  The matrix has n rows and
// m columns, and the indexing (r,c) refers to row r and column c.  A
// first-order approximation is F(p + d) = F(p) + J(p)d, where d is an m-by-1
// vector with small length.  Consequently, an approximation to E is E(p + d)
// = |F(p + d)|^2 = |F(p) + J(p)d|^2.  The goal is to choose d to minimize
// |F(p) + J(p)d|^2 and, hopefully, with E(p + d) < E(p).  Choosing an initial
// p_{0}, the hope is that the algorithm generates a sequence p_{i} for which
// E(p_{i+1}) < E(p_{i}) and, in the limit, E(p_{j}) approaches the global
// minimum of E.  The algorithm is referred to as Gauss-Newton iteration.  If
// E does not decrease for a step of the algorithm, one can modify the
// algorithm to the Levenberg-Marquardt iteration.  See
// LevenbergMarquardtMinimizer.h for a description and an implementation.
//
// For a single Gauss-Newton iteration, we need to choose d to minimize
// |F(p) + J(p)d|^2 where p is fixed.  This is a linear least squares problem
// which can be formulated using the normal equations
// (J^T(p)*J(p))*d = -J^T(p)*F(p).  The matrix J^T*J is positive semidefinite.
// If it is invertible, then d = -(J^T(p)*J(p))^{-1}*F(p).  If it is not
// invertible, some other algorithm must be used to choose d; one option is
// to use gradient descent for the step.  A Cholesky decomposition can be
// used to solve the linear system.
//
// Although an implementation can allow the caller to pass an array of
// functions F_{i}(p) and an array of derivatives dF_{r}/dp_{c}, some
// applications might involve a very large n that precludes storing all
// the computed Jacobian matrix entries because of excessive memory
// requirements.  In such an application, it is better to compute instead
// the entries of the m-by-m matrix J^T*J and the m-by-1 vector J^T*F.
// Typically, m is small, so the memory requirements are not excessive.  Also,
// there might be additional structure to F for which the caller can take
// advantage; for example, 3-tuples of components of F(p) might correspond to
// vectors that can be manipulated using an already existing mathematics
// library.  The implementation here supports both approaches.

namespace gte
{
    template <typename Real>
    class GaussNewtonMinimizer
    {
    public:
        // Convenient types for the domain vectors, the range vectors, the
        // function F and the Jacobian J.
        typedef GVector<Real> DVector;  // numPDimensions
        typedef GVector<Real> RVector;  // numFDimensions
        typedef GMatrix<Real> JMatrix;  // numFDimensions-by-numPDimensions
        typedef GMatrix<Real> JTJMatrix;  // numPDimensions-by-numPDimensions
        typedef GVector<Real> JTFVector;  // numPDimensions
        typedef std::function<void(DVector const&, RVector&)> FFunction;
        typedef std::function<void(DVector const&, JMatrix&)> JFunction;
        typedef std::function<void(DVector const&, JTJMatrix&, JTFVector&)> JPlusFunction;

        // Create the minimizer that computes F(p) and J(p) directly.
        GaussNewtonMinimizer(int numPDimensions, int numFDimensions,
            FFunction const& inFFunction, JFunction const& inJFunction)
            :
            mNumPDimensions(numPDimensions),
            mNumFDimensions(numFDimensions),
            mFFunction(inFFunction),
            mJFunction(inJFunction),
            mF(mNumFDimensions),
            mJ(mNumFDimensions, mNumPDimensions),
            mJTJ(mNumPDimensions, mNumPDimensions),
            mNegJTF(mNumPDimensions),
            mDecomposer(mNumPDimensions),
            mUseJFunction(true)
        {
            LogAssert(mNumPDimensions > 0 && mNumFDimensions > 0, "Invalid dimensions.");
        }

        // Create the minimizer that computes J^T(p)*J(p) and -J(p)*F(p).
        GaussNewtonMinimizer(int numPDimensions, int numFDimensions,
            FFunction const& inFFunction, JPlusFunction const& inJPlusFunction)
            :
            mNumPDimensions(numPDimensions),
            mNumFDimensions(numFDimensions),
            mFFunction(inFFunction),
            mJPlusFunction(inJPlusFunction),
            mF(mNumFDimensions),
            mJ(mNumFDimensions, mNumPDimensions),
            mJTJ(mNumPDimensions, mNumPDimensions),
            mNegJTF(mNumPDimensions),
            mDecomposer(mNumPDimensions),
            mUseJFunction(false)
        {
            LogAssert(mNumPDimensions > 0 && mNumFDimensions > 0, "Invalid dimensions.");
        }

        // Disallow copy, assignment and move semantics.
        GaussNewtonMinimizer(GaussNewtonMinimizer const&) = delete;
        GaussNewtonMinimizer& operator=(GaussNewtonMinimizer const&) = delete;
        GaussNewtonMinimizer(GaussNewtonMinimizer&&) = delete;
        GaussNewtonMinimizer& operator=(GaussNewtonMinimizer&&) = delete;

        inline int GetNumPDimensions() const
        {
            return mNumPDimensions;
        }

        inline int GetNumFDimensions() const
        {
            return mNumFDimensions;
        }

        struct Result
        {
            DVector minLocation;
            Real minError;
            Real minErrorDifference;
            Real minUpdateLength;
            size_t numIterations;
            bool converged;
        };

        Result operator()(DVector const& p0, size_t maxIterations,
            Real updateLengthTolerance, Real errorDifferenceTolerance)
        {
            Result result;
            result.minLocation = p0;
            result.minError = std::numeric_limits<Real>::max();
            result.minErrorDifference = std::numeric_limits<Real>::max();
            result.minUpdateLength = (Real)0;
            result.numIterations = 0;
            result.converged = false;

            // As a simple precaution, ensure the tolerances are nonnegative.
            updateLengthTolerance = std::max(updateLengthTolerance, (Real)0);
            errorDifferenceTolerance = std::max(errorDifferenceTolerance, (Real)0);

            // Compute the initial error.
            mFFunction(p0, mF);
            result.minError = Dot(mF, mF);

            // Do the Gauss-Newton iterations.
            auto pCurrent = p0;
            for (result.numIterations = 1; result.numIterations <= maxIterations; ++result.numIterations)
            {
                ComputeLinearSystemInputs(pCurrent);
                if (!mDecomposer.Factor(mJTJ))
                {
                    // TODO: The matrix mJTJ is positive semi-definite, so the
                    // failure can occur when mJTJ has a zero eigenvalue in
                    // which case mJTJ is not invertible.  Generate an iterate
                    // anyway, perhaps using gradient descent?
                    return result;
                }
                mDecomposer.SolveLower(mJTJ, mNegJTF);
                mDecomposer.SolveUpper(mJTJ, mNegJTF);

                auto pNext = pCurrent + mNegJTF;
                mFFunction(pNext, mF);
                Real error = Dot(mF, mF);
                if (error < result.minError)
                {
                    result.minErrorDifference = result.minError - error;
                    result.minUpdateLength = Length(mNegJTF);
                    result.minLocation = pNext;
                    result.minError = error;
                    if (result.minErrorDifference <= errorDifferenceTolerance
                        || result.minUpdateLength <= updateLengthTolerance)
                    {
                        result.converged = true;
                        return result;
                    }
                }

                pCurrent = pNext;
            }

            return result;
        }

    private:
        void ComputeLinearSystemInputs(DVector const& pCurrent)
        {
            if (mUseJFunction)
            {
                mJFunction(pCurrent, mJ);
                mJTJ = MultiplyATB(mJ, mJ);
                mNegJTF = -(mF * mJ);
            }
            else
            {
                mJPlusFunction(pCurrent, mJTJ, mNegJTF);
            }
        }

        int mNumPDimensions, mNumFDimensions;
        FFunction mFFunction;
        JFunction mJFunction;
        JPlusFunction mJPlusFunction;

        // Storage for J^T(p)*J(p) and -J^T(p)*F(p) during the iterations.
        RVector mF;
        JMatrix mJ;
        JTJMatrix mJTJ;
        JTFVector mNegJTF;

        CholeskyDecomposition<Real> mDecomposer;

        bool mUseJFunction;
    };
}
