// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

#pragma once

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/FPInterval.h>
#include <Mathematics/Polynomial1.h>
#include <functional>

namespace gte
{
    class RemezConstrained
    {
    public:
        RemezConstrained();

        size_t Execute(double tMin, double tMax, size_t degree,
            size_t maxRemezIterations, size_t maxBisectionIterations);

        inline std::vector<double> const& GetCoefficients() const
        {
            return mPCoefficients;
        }

        inline double GetEstimatedMaxError() const
        {
            return mEstimatedMaxError;
        }

        inline std::vector<double> const& GetTNodes() const
        {
            return mTNodes;
        }

        inline std::vector<double> const& GetErrors() const
        {
            return mErrors;
        }

    protected:
        // The exact signs of D(t) must be computed. This is done using
        // rational arithmetic and floating-point interval arithmetic.
        using Rational = BSRational<UIntegerAP32>;
        using Interval = FPInterval<double>;

        // Virtual functions specific to each RotC? estimation. The input
        // t-values must be nonnegative. The function F(t) corresponds to
        // one of the RotC?(t) functions. Other functions used in the
        // algorithm are
        //   G(t) = F'(t) / (2t)
        //   P(t) = sum_{i=0}^n c_i t^{2i}
        //   Q(t) = P'(t) / (2t)
        //   E(t) = F(t) - P(t)
        //   D(t) = G(t) - Q(t)
        //   A(t) = Taylor(n-1, F(t)) - P(t)
        //   B(t) = Taylor(n-1, G(t)) - Q(t)
        virtual double F(double t) const = 0;
        virtual double FDer(double t) const = 0;

        // Compute the coefficients for A(t) and B(t). These are the
        // coefficients for the Taylor polynomials for E(t) = F(t) - P(t)
        // and D(t) = (F'(t) - P'(t)) / t, respectively.
        virtual void ComputeACoefficients() = 0;
        virtual void ComputeBCoefficients() = 0;

        // Compute the interval that contains the true value of F(t).
        virtual Interval GetIntervalF(double t, Interval const& iCos,
            Interval const& iSin) const = 0;

        // Compute the interval that contains the true value of G(t).
        virtual Interval GetIntervalG(double t, Interval const& iCos,
            Interval const& iSin) const = 0;

        // Support for GetSignE(...).
        virtual void GetFirstSignETerm(size_t j, Rational const& factor,
            Rational& term) const = 0;

        virtual void GetSecondSignETerm(size_t j, Rational const& tSqr,
            Rational& factor, Rational& term) const = 0;

        // Support for GetSignD(...).
        virtual void GetFirstSignDTerm(size_t j, Rational const& factor,
            Rational& term) const = 0;

        virtual void GetSecondSignDTerm(size_t j, Rational const& tSqr,
            Rational& factor, Rational& term) const = 0;

        // The steps in Execute(...).
        void ComputeEValues();
        void ComputeInitialTNodes();
        void ComputeFValues();
        void ComputeNewtonCoefficients();
        void ComputeEstimatedError();
        void ComputePCoefficients();
        bool IsOscillatory();
        void ComputePartition();
        void ComputeXExtremes();

        // Support for polynomial evaluations.
        void EvaluateNewton(double t, double& uResult, double& vResult) const;
        double EvaluateP(double t) const;
        double EvaluatePDer(double t) const;

        // Support for exact sign computations. The maxSignTestIndex is
        // selected arbitrarily and appears to be large enough for estimating
        // the polynomials RotC?.
        static size_t constexpr mMaxSignTestIndex = 32;
        Interval GetIntervalP(double t) const;
        Interval GetIntervalQ(double t) const;
        Interval GetIntervalE(double t) const;
        Interval GetIntervalD(double t) const;
        int32_t GetSignE(double t) const;
        int32_t GetSignD(double t) const;

        // Inputs to Execute(...).
        double mTMin, mTMax;
        size_t mDegree, mDegreeP1, mDegreeP2;
        size_t mMaxRemezIterations, mMaxBisectionIterations;

        // Members used in the intermediate computations.
        size_t mDeletedIndex;
        std::vector<double> mTNodes, mFValues, mEValues;
        std::vector<double> mTNodesDel, mFValuesDel, mEValuesDel;
        std::vector<double> mUCoefficients, mVCoefficients;
        std::vector<double> mPartition;
        std::vector<double> mNextTNodes;

        // Members used for exact sign computations. The A-coefficients are
        // for computing signs of E(t) = F(t) - P(t). The B-coefficients are
        // for computing signs of D(t) = (F'(t) - P'(t)) / t.
        std::vector<Rational> mACoefficients;
        std::vector<Rational> mBCoefficients;
        std::vector<Rational> mFactorial;

        // Outputs from Execute(...).
        std::vector<double> mPCoefficients;
        std::vector<double> mErrors;
        std::vector<int32_t> mErrorSigns;
        double mEstimatedMaxError;
    };
}
