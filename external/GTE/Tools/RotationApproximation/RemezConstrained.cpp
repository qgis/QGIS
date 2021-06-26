// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

#pragma once

#include "RemezConstrained.h"
using namespace gte;

RemezConstrained::RemezConstrained()
    :
    mTMin(0),
    mTMax(0),
    mDegree(0),
    mDegreeP1(0),
    mDegreeP2(0),
    mMaxRemezIterations(0),
    mMaxBisectionIterations(0),
    mDeletedIndex(0),
    mTNodes{},
    mFValues{},
    mEValues{},
    mTNodesDel{},
    mFValuesDel{},
    mEValuesDel{},
    mUCoefficients{},
    mVCoefficients{},
    mPartition{},
    mNextTNodes{},
    mPCoefficients{},
    mErrors{},
    mErrorSigns{},
    mEstimatedMaxError(0)
{
}

size_t RemezConstrained::Execute(double tMin, double tMax, size_t degree,
    size_t maxRemezIterations, size_t maxBisectionIterations)
{
    LogAssert(tMin < tMax && 0 < degree && maxRemezIterations > 0
        && maxBisectionIterations > 0, "Invalid argument.");

    mTMin = tMin;
    mTMax = tMax;
    mDegree = degree;
    mDegreeP1 = mDegree + 1;
    mDegreeP2 = mDegree + 2;
    mMaxRemezIterations = maxRemezIterations;
    mMaxBisectionIterations = maxBisectionIterations;

    mDeletedIndex = mDegreeP2 / 2;
    mTNodes.resize(mDegreeP2);
    mFValues.resize(mDegreeP2);
    mEValues.resize(mDegreeP2);
    mTNodesDel.resize(mDegreeP1);
    mFValuesDel.resize(mDegreeP1);
    mEValuesDel.resize(mDegreeP1);
    mUCoefficients.resize(mDegreeP1);
    mVCoefficients.resize(mDegreeP1);
    mPartition.resize(mDegreeP1);
    mNextTNodes.resize(mDegreeP2);

    mPCoefficients.resize(mDegreeP1);
    mErrors.resize(mDegreeP2);
    mErrorSigns.resize(mDegreeP2);
    mEstimatedMaxError = 0.0;

    ComputeEValues();
    ComputeInitialTNodes();
    size_t iteration;
    for (iteration = 0; iteration < mMaxRemezIterations; ++iteration)
    {
        // Solve the matrix system that determines the next set of
        // polynomial coefficients for P(t).
        ComputeFValues();
        ComputeNewtonCoefficients();
        ComputeEstimatedError();
        ComputePCoefficients();

        // Compute the next set of x-nodes.
        if (IsOscillatory())
        {
            ComputePartition();
            ComputeXExtremes();
        }
        else
        {
            break;
        }
    }

    mErrors.front() = 0.0;
    for (size_t i = 1; i < mDegreeP1; ++i)
    {
        mErrors[i] = F(mTNodes[i]) - EvaluateP(mTNodes[i]);
    }
    mErrors.back() = 0.0;
    return iteration;
}

void RemezConstrained::ComputeEValues()
{
    mEValues.front() = 0.0;
    for (size_t i = 1; i + 1 < mDegreeP2; ++i)
    {
        mEValues[i] = ((i & 1) ? 1 : -1);
    }
    mEValues.back() = 0.0;

    for (size_t i = 0, j = 0; i < mDegreeP2; ++i)
    {
        if (i != mDeletedIndex)
        {
            mEValuesDel[j++] = mEValues[i];
        }
    }
}

void RemezConstrained::ComputeInitialTNodes()
{
    // Get the Chebyshev nodes for the interval [-1,1].
    for (size_t i = 0, j = mDegreeP2 - 1; i < mDegreeP2; ++i, --j)
    {
        mTNodes[i] = std::cos((2.0 * j + 1.0) * GTE_C_PI / (2.0 * mDegreeP2));
    }

    // Transform the nodes to the interval [tMin, tMax]. Set the
    // endpoints to tMin and tMax to ensure F(t) and P(t) match at
    // the endpoints.
    double center = 0.5 * (mTMax + mTMin);
    double radius = 0.5 * (mTMax - mTMin);
    for (size_t i = 0; i < mDegreeP2; ++i)
    {
        mTNodes[i] = center + radius * mTNodes[i];
    }
    mTNodes.front() = mTMin;
    mTNodes.back() = mTMax;

    for (size_t i = 0, j = 0; i < mDegreeP2; ++i)
    {
        if (i != mDeletedIndex)
        {
            mTNodesDel[j++] = mTNodes[i];
        }
    }
}

void RemezConstrained::ComputeFValues()
{
    for (size_t i = 0; i < mDegreeP2; ++i)
    {
        mFValues[i] = F(mTNodes[i]);
    }

    for (size_t i = 0, j = 0; i < mDegreeP2; ++i)
    {
        if (i != mDeletedIndex)
        {
            mFValuesDel[j++] = mFValues[i];
        }
    }
}

void RemezConstrained::ComputeNewtonCoefficients()
{
    for (size_t i = 0; i < mDegreeP1; ++i)
    {
        mUCoefficients[i] = mFValuesDel[i];
        mVCoefficients[i] = mEValuesDel[i];
        for (size_t j = 0; j < i; ++j)
        {
            double tSqrI = mTNodesDel[i] * mTNodesDel[i];
            double tSqrJ = mTNodesDel[j] * mTNodesDel[j];
            double denom = tSqrI - tSqrJ;
            mUCoefficients[i] -= mUCoefficients[j];
            mUCoefficients[i] /= denom;
            mVCoefficients[i] -= mVCoefficients[j];
            mVCoefficients[i] /= denom;
        }
    }
}

void RemezConstrained::ComputeEstimatedError()
{
    double const powNegOne = (mDeletedIndex & 1 ? -1.0 : +1.0);
    double u, v;
    EvaluateNewton(mTNodes[mDeletedIndex], u, v);
    double f = mFValues[mDeletedIndex];
    mEstimatedMaxError = (u - f) / (v + powNegOne);
}

void RemezConstrained::ComputePCoefficients()
{
    std::vector<double> constant(mDegreeP1);
    for (size_t i = 0; i < mDegreeP1; ++i)
    {
        constant[i] = mUCoefficients[i] - mEstimatedMaxError * mVCoefficients[i];
    }

    size_t index = mDegree;
    Polynomial1<double> poly{ constant[index--] };
    for (size_t i = 1; i < mDegreeP1; ++i, --index)
    {
        double tSqrI = mTNodesDel[index] * mTNodesDel[index];
        Polynomial1<double> quadratic{ -tSqrI, 0.0, 1.0 };
        poly = constant[index] + quadratic * poly;
    }

    for (size_t i = 0; i < mDegreeP1; ++i)
    {
        mPCoefficients[i] = poly[static_cast<uint32_t>(2 * i)];
    }
}

bool RemezConstrained::IsOscillatory()
{
    ComputeACoefficients();

    // Compute the errors |F(x)-P(x)| for the current nodes and
    // verify they are oscillatory. The endpoint errors are 0, so
    // these are ignored in the oscillation test.
    mErrorSigns.front() = 0;
    for (size_t i = 1; i < mDegreeP1; ++i)
    {
        mErrorSigns[i] = GetSignE(mTNodes[i]);
    }
    mErrorSigns.back() = 0;

    for (size_t i0 = 1, i1 = 2; i1 < mDegreeP1; i0 = i1++)
    {
        if (mErrorSigns[i0] * mErrorSigns[i1] != -1)
        {
            // The process terminates when the errors are not
            // equioscillatory.
            return false;
        }
    }
    return true;
}

void RemezConstrained::ComputePartition()
{
    // Use bisection to compute the roots of E(t). The algorithm partitions
    // [tMin, tMax] into degree+2 subintervals, each subinterval with E(t)
    // positive or with E(t) negative.
    mPartition.front() = mTMin;
    mPartition.back() = mTMax;
    int32_t sign0 = mErrorSigns[1], sign1 = -sign0, signMid = 0;
    for (size_t i0 = 1, i1 = 2; i1 < mPartition.size(); i0 = i1++)
    {
        double t0 = mTNodes[i0], t1 = mTNodes[i1], tMid = 0.0;
        for (size_t iteration = 0; iteration < mMaxBisectionIterations; ++iteration)
        {
            tMid = 0.5 * (t0 + t1);
            if (tMid == t0 || tMid == t1)
            {
                // We are at the limit of floating-point precision for
                // the average of endpoints.
                break;
            }

            // Update the correct endpoint to the midpoint.
            signMid = GetSignE(tMid);
            if (signMid == sign0)
            {
                t0 = tMid;
            }
            else if (signMid == sign1)
            {
                t1 = tMid;
            }
            else
            {
                // Found a root (numerically rounded to zero).
                break;
            }
        }

        // It is possible that the maximum number of bisections was
        // applied without convergence. Use the last computed xMid
        // as the root.
        mPartition[i0] = tMid;

        sign0 = -sign0;
        sign1 = -sign1;
    }
}

void RemezConstrained::ComputeXExtremes()
{
    ComputeBCoefficients();
    mNextTNodes.front() = mTMin;
    mNextTNodes.back() = mTMax;
    int32_t sign0 = mErrorSigns[1], sign1 = -sign0, signMid = 0;
    for (size_t i0 = 0, i1 = 1; i1 < mPartition.size(); i0 = i1++)
    {
        double t0 = mPartition[i0], t1 = mPartition[i1], tMid = 0.0;
        for (size_t iteration = 0; iteration < mMaxBisectionIterations; ++iteration)
        {
            tMid = 0.5 * (t0 + t1);
            if (tMid == t0 || tMid == t1)
            {
                // We are at the limit of floating-point precision for
                // the average of endpoints.
                break;
            }

            signMid = GetSignD(tMid);
            if (signMid == sign0)
            {
                t0 = tMid;
            }
            else if (signMid == sign1)
            {
                t1 = tMid;
            }
            else
            {
                // Found a root (numerically rounded to zero).
                break;
            }
        }

        // It is possible that the maximum number of bisections was
        // applied without convergence. Use the last computed xMid
        // as the root.
        mNextTNodes[i1] = tMid;

        sign0 = -sign0;
        sign1 = -sign1;
    }

    std::copy(mNextTNodes.begin(), mNextTNodes.end(), mTNodes.begin());

    for (size_t i = 0, j = 0; i < mDegreeP2; ++i)
    {
        if (i != mDeletedIndex)
        {
            mTNodesDel[j++] = mTNodes[i];
        }
    }
}

void RemezConstrained::EvaluateNewton(double t, double& uResult, double& vResult) const
{
    double const tSqr = t * t;
    size_t index = mDegree;
    uResult = mUCoefficients[index];
    vResult = mVCoefficients[index];
    --index;
    for (size_t i = 1; i < mDegreeP1; ++i, --index)
    {
        double const tSqrI = mTNodesDel[index] * mTNodesDel[index];
        double tDiff = tSqr - tSqrI;
        uResult = mUCoefficients[index] + tDiff * uResult;
        vResult = mVCoefficients[index] + tDiff * vResult;
    }
};

double RemezConstrained::EvaluateP(double t) const
{
    double const tSqr = t * t;
    size_t index = mDegree;
    double result = mPCoefficients[index--];
    for (size_t i = 1; i < mPCoefficients.size(); ++i, --index)
    {
        result = mPCoefficients[index] + tSqr * result;
    }
    return result;
}

double RemezConstrained::EvaluatePDer(double t) const
{
    double const tSqr = t * t;
    size_t index = mDegree;
    double result = static_cast<double>(index) * mPCoefficients[index];
    --index;
    for (size_t i = 2; i < mPCoefficients.size(); ++i, --index)
    {
        result = static_cast<double>(index) * mPCoefficients[index] + tSqr * result;
    }
    result *= 2.0 * t;
    return result;
}

RemezConstrained::Interval RemezConstrained::GetIntervalP(double t) const
{
    if (t > 0.0)
    {
        Interval iT(t);
        Interval iTSqr = iT * iT;
        size_t index = mDegree;
        Interval iIndex(static_cast<double>(index));
        Interval iP(mPCoefficients[index--]);
        for (size_t i = 1; i < mPCoefficients.size(); ++i, --index)
        {
            Interval iPCoefficient(mPCoefficients[index]);
            iP = iPCoefficient + iTSqr * iP;
        }
        return iP;
    }
    else
    {
        return Interval(mPCoefficients[0]);
    }
}

RemezConstrained::Interval RemezConstrained::GetIntervalQ(double t) const
{
    if (t > 0.0)
    {
        Interval iT(t);
        Interval iTSqr = iT * iT;
        size_t index = mDegree;
        Interval iIndex(static_cast<double>(index));
        Interval iQ = iIndex * Interval(mPCoefficients[index--]);
        for (size_t i = 2; i < mPCoefficients.size(); ++i, --index)
        {
            iIndex = Interval(static_cast<double>(index));
            Interval iPCoefficient(mPCoefficients[index]);
            iQ = iIndex * iPCoefficient + iTSqr * iQ;
        }
        return iQ;
    }
    else
    {
        return Interval(mPCoefficients[1]);
    }
}

RemezConstrained::Interval RemezConstrained::GetIntervalE(double t) const
{
    if (t > 0.0)
    {
        auto saveMode = std::fegetround();
        std::fesetround(FE_DOWNWARD);
        double cosTDown = std::cos(t);
        double sinTDown = std::sin(t);
        std::fesetround(FE_UPWARD);
        double cosTUp = std::cos(t);
        double sinTUp = std::sin(t);
        std::fesetround(saveMode);
        Interval iCos(std::min(cosTDown, cosTUp), std::max(cosTDown, cosTUp));
        Interval iSin(std::min(sinTDown, sinTUp), std::max(sinTDown, sinTUp));
        Interval iF = GetIntervalF(t, iCos, iSin);
        Interval iP = GetIntervalP(t);
        Interval iE = iF - iP;
        return iE;
    }
    else
    {
        return Interval(0.0);
    }
}

RemezConstrained::Interval RemezConstrained::GetIntervalD(double t) const
{
    if (t > 0.0)
    {
        auto saveMode = std::fegetround();
        std::fesetround(FE_DOWNWARD);
        double cosTDown = std::cos(t);
        double sinTDown = std::sin(t);
        std::fesetround(FE_UPWARD);
        double cosTUp = std::cos(t);
        double sinTUp = std::sin(t);
        std::fesetround(saveMode);
        Interval iCos(std::min(cosTDown, cosTUp), std::max(cosTDown, cosTUp));
        Interval iSin(std::min(sinTDown, sinTUp), std::max(sinTDown, sinTUp));
        Interval iG = GetIntervalG(t, iCos, iSin);
        Interval iQ = GetIntervalQ(t);
        Interval iD = iG - iQ;
        return iD;
    }
    else
    {
        Interval iG = GetIntervalG(0.0, Interval(1.0), Interval(0.0));
        Interval iQ = GetIntervalQ(0.0);
        Interval iD = iG - iQ;
        return iD;
    }
}

int32_t RemezConstrained::GetSignE(double t) const
{
    Interval iE = GetIntervalE(t);
    if (iE[0] > 0.0)
    {
        return +1;
    }

    if (iE[1] < 0.0)
    {
        return -1;
    }

    int32_t const sign = ((mDegree & 1) ? +1 : -1);
    Rational const rt(t);
    Rational const tSqr = rt * rt;

    // Compute A(t).
    Rational A = mACoefficients[mDegree];
    for (size_t i = 0, index = mDegree - 1; i <= mDegree - 1; ++i, --index)
    {
        A = mACoefficients[index] + A * tSqr;
    }

    if (sign * A.GetSign() >= 0)
    {
        return sign;
    }

    // Compute tPow = t^{2*(N+1)}.
    Rational tPow = tSqr;
    for (size_t i = 1; i <= mDegree; ++i)
    {
        tPow *= tSqr;
    }

    Rational factor = tPow / mFactorial.back();
    Rational term;
    for (size_t j = 1; j <= mMaxSignTestIndex; j += 2)
    {
        GetFirstSignETerm(j, factor, term);
        term.SetSign(sign);
        A += term;
        if (sign * A.GetSign() <= 0)
        {
            return -sign;
        }

        GetSecondSignETerm(j, tSqr, factor, term);
        term.SetSign(-sign);
        A += term;
        if (sign * A.GetSign() >= 0)
        {
            return sign;
        }
    }

    return std::numeric_limits<int32_t>::max();
}

int32_t RemezConstrained::GetSignD(double t) const
{
    Interval iD = GetIntervalD(t);
    if (iD[0] > 0.0)
    {
        return +1;
    }

    if (iD[1] < 0.0)
    {
        return -1;
    }

    int32_t const sign = ((mDegree & 1) ? +1 : -1);
    Rational const rt(t);
    Rational const tSqr = rt * rt;

    // Compute B(t).
    Rational B = mBCoefficients[mDegree - 1];
    for (size_t i = 0, index = mDegree - 2; i <= mDegree - 2; ++i, --index)
    {
        B = mBCoefficients[index] + B * tSqr;
    }

    if (sign * B.GetSign() >= 0)
    {
        return sign;
    }

    // Compute tPow = t^{2*N}.
    Rational tPow = tSqr;
    for (size_t i = 1; i < mDegree; ++i)
    {
        tPow *= tSqr;
    }

    Rational factor = tPow / mFactorial.back();
    Rational term;
    for (size_t j = 1; j <= mMaxSignTestIndex; j += 2)
    {
        GetFirstSignDTerm(j, factor, term);
        term.SetSign(sign);
        B += term;
        if (sign * B.GetSign() <= 0)
        {
            return -sign;
        }

        GetSecondSignDTerm(j, tSqr, factor, term);
        term.SetSign(-sign);
        B += term;
        if (sign * B.GetSign() >= 0)
        {
            return sign;
        }
    }

    return std::numeric_limits<int32_t>::max();
}
