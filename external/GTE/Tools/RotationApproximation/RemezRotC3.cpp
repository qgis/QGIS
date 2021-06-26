// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

#include "RemezRotC3.h"
using namespace gte;

// F(t) = (2(1 - cos(t)) - t sin(t)) / t^4
//      = sum_{i=0}^{inf} (-1)^i (2(i+1)/(2i+3)!) t^{2i}
//
// G(t) = F'(t) / (2t)
//      = (5t sin(t) + (8 - t^2) cos(t) - 8) / (2t^6)
//      = sum_{i=0}^{inf} (-1)^{i+1} (2(i+1)(i+2)/(2i+6)!) t^{2i}
//
// P(t) = sum_{i=0}^n c_i t^{2i}
//
// Q(t) = P'(t) / (2t)
//      = sum_{i=0}^{n-1} (i+1) c_{i+1} t^{2i}
//
// E(t) = F(t) - P(t)
//
// D(t) = G(t) - Q(t)
//
// A(t) = Taylor(n, F(t)) - P(t)
//      = sum_{i=0}^{n} [(-1)^{i} 2*(i+1)/(2i+4)! - c_{i}] t^{2i}
//
// B(t) = Taylor(n-1, G(t)) - Q(t)
//      = sum_{i=0}^{n-1} (i+1) [(-1)^{i+1} 2(i+2)/(2i+6)! - c_{i+1}] t^{2i}

RemezRotC3::RemezRotC3()
    :
    RemezConstrained()
{
}

double RemezRotC3::F(double t) const
{
    if (t > 0.0)
    {
        return (2.0 * (1.0 - std::cos(t)) - t * std::sin(t)) / t / t / t / t;
    }
    else
    {
        return 1.0 / 12.0;
    }
}

double RemezRotC3::FDer(double t) const
{
    if (t > 0.0)
    {
        return (5.0 * t * std::sin(t) + (8.0 - t * t) * std::cos(t) - 8.0) / t / t / t / t / t;
    }
    else
    {
        return 0.0;
    }
}

void RemezRotC3::ComputeACoefficients()
{
    mACoefficients.resize(mDegree + 1);

    mFactorial.resize(mDegree + 2);
    mFactorial[0] = 24;  // (2i+4)! at i = 0
    for (size_t i = 1, im1 = 0; i <= mDegree + 1; ++i, ++im1)
    {
        mFactorial[i] = mFactorial[im1] * Rational((2 * i + 3) * (2 * i + 4));
    }

    Rational negOnePow = 1;
    for (size_t i = 0; i <= mDegree; ++i)
    {
        Rational term0 = negOnePow * Rational(2 * (i + 1)) / mFactorial[i];
        Rational term1 = mPCoefficients[i];
        mACoefficients[i] = term0 - term1;
        negOnePow.SetSign(-negOnePow.GetSign());
    }
}

void RemezRotC3::ComputeBCoefficients()
{
    mBCoefficients.resize(mDegree);

    mFactorial.resize(mDegree + 1);
    mFactorial[0] = 720;  // (2i+6)! at i = 0
    for (size_t i = 1, im1 = 0; i <= mDegree; ++i, ++im1)
    {
        mFactorial[i] = mFactorial[im1] * Rational((2 * i + 5) * (2 * i + 6));
    }

    Rational negOnePow = -1;
    for (size_t i = 0; i < mDegree; ++i)
    {
        Rational term0 = negOnePow * Rational(2 * (i + 2)) / mFactorial[i];
        Rational term1 = mPCoefficients[i + 1];
        mBCoefficients[i] = Rational(i + 1) * (term0 - term1);
        negOnePow.SetSign(-negOnePow.GetSign());
    }
}

RemezRotC3::Interval RemezRotC3::GetIntervalF(double t,
    Interval const& iCos, Interval const& iSin) const
{
    if (t > 0.0)
    {
        Interval iOne(1.0), iTwo(2.0);
        Interval iT(t);
        Interval iTFourth = iT * iT * iT * iT;
        Interval iF = (iTwo * (iOne - iCos) - iT * iSin) / iTFourth;
        return iF;
    }
    else
    {
        return Interval(1.0) / Interval(12.0);
    }
}

RemezRotC3::Interval RemezRotC3::GetIntervalG(double t,
    Interval const& iCos, Interval const& iSin) const
{
    if (t > 0.0)
    {
        Interval iHalf(0.5), iFive(5.0), iEight(8.0);
        Interval iT(t);
        Interval iTSqr = iT * iT;
        Interval iTSix = iTSqr * iTSqr * iTSqr;
        Interval iG = iHalf * (iFive * iT * iSin + (iEight - iTSqr) * iCos - iEight) / iTSix;
        return iG;
    }
    else
    {
        return Interval(-1.0) / Interval(180.0);
    }
}

void RemezRotC3::GetFirstSignETerm(size_t j, Rational const& factor, Rational& term) const
{
    term = Rational(2 * (mDegree + j + 1)) * factor;
}

void RemezRotC3::GetSecondSignETerm(size_t j, Rational const& tSqr, Rational& factor,
    Rational& term) const
{
    factor *= tSqr / Rational((2 * (mDegree + j) + 5) * (2 * (mDegree + j) + 6));
    term = Rational(2 * (mDegree + j + 2)) * factor;
}

void RemezRotC3::GetFirstSignDTerm(size_t j, Rational const& factor, Rational& term) const
{
    term = Rational(mDegree + j) * Rational(2 * (mDegree + j + 1)) * factor;
}

void RemezRotC3::GetSecondSignDTerm(size_t j, Rational const& tSqr, Rational& factor,
    Rational& term) const
{
    factor *= tSqr / Rational((2 * (mDegree + j) + 5) * (2 * (mDegree + j) + 6));
    term = Rational(mDegree + j + 1) * Rational(2 * (mDegree + j + 2)) * factor;
}
