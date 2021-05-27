// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

#include "RemezRotC2.h"
using namespace gte;

// F(t) = (sin(t) - t cos(t)) / t^3
//      = sum_{i=0}^{inf} (-1)^i (2(i+1)/(2i+3)!) t^{2i}
//
// G(t) = F'(t) / (2t)
//      = (3t cos(t) + (t^2 - 3) sin(t)) / (2t^5)
//      = sum_{i=0}^{inf} (-1)^{i+1} (2(i+1)(i+2)/(2i+5)!) t^{2i}
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
//      = sum_{i=0}^{n} [(-1)^{i} 2*(i+1)/(2i+3)! - c_{i}] t^{2i}
//
// B(t) = Taylor(n-1, G(t)) - Q(t)
//      = sum_{i=0}^{n-1} (i+1) [(-1)^{i+1} 2(i+2)/(2i+5)! - c_{i+1}] t^{2i}

RemezRotC2::RemezRotC2()
    :
    RemezConstrained()
{
}

double RemezRotC2::F(double t) const
{
    if (t > 0.0)
    {
        return (std::sin(t) - t * std::cos(t)) / t / t / t;
    }
    else
    {
        return 1.0 / 3.0;
    }
}

double RemezRotC2::FDer(double t) const
{
    if (t > 0.0)
    {
        return (3.0 * t * std::cos(t) + (t * t - 3.0) * std::sin(t)) / t / t / t / t;
    }
    else
    {
        return 0.0;
    }
}

void RemezRotC2::ComputeACoefficients()
{
    mACoefficients.resize(mDegree + 1);

    mFactorial.resize(mDegree + 2);
    mFactorial[0] = 6;  // (2i+3)! at i = 0
    for (size_t i = 1, im1 = 0; i <= mDegree + 1; ++i, ++im1)
    {
        mFactorial[i] = mFactorial[im1] * Rational((2 * i + 2) * (2 * i + 3));
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

void RemezRotC2::ComputeBCoefficients()
{
    mBCoefficients.resize(mDegree);

    mFactorial.resize(mDegree + 1);
    mFactorial[0] = 120;  // (2i+5)! at i = 0
    for (size_t i = 1, im1 = 0; i <= mDegree; ++i, ++im1)
    {
        mFactorial[i] = mFactorial[im1] * Rational((2 * i + 4) * (2 * i + 5));
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

RemezRotC2::Interval RemezRotC2::GetIntervalF(double t,
    Interval const& iCos, Interval const& iSin) const
{
    if (t > 0.0)
    {
        Interval iT(t);
        Interval iTCube = iT * iT * iT;
        Interval iF = (iSin - iT * iCos) / iTCube;
        return iF;
    }
    else
    {
        return Interval(1.0) / Interval(3.0);
    }
}

RemezRotC2::Interval RemezRotC2::GetIntervalG(double t,
    Interval const& iCos, Interval const& iSin) const
{
    if (t > 0.0)
    {
        Interval iHalf(0.5), iThree(3.0);
        Interval iT(t);
        Interval iTSqr = iT * iT;
        Interval iTFive = iTSqr * iTSqr * iT;
        Interval iG = iHalf * (iThree * iT * iCos + (iTSqr - iThree) * iSin) / iTFive;
        return iG;
    }
    else
    {
        return Interval(-1.0) / Interval(30.0);
    }
}

void RemezRotC2::GetFirstSignETerm(size_t j, Rational const& factor, Rational& term) const
{
    term = Rational(2 * (mDegree + j + 1)) * factor;
}

void RemezRotC2::GetSecondSignETerm(size_t j, Rational const& tSqr, Rational& factor,
    Rational& term) const
{
    factor *= tSqr / Rational((2 * (mDegree + j) + 4) * (2 * (mDegree + j) + 5));
    term = Rational(2 * (mDegree + j + 2)) * factor;
}

void RemezRotC2::GetFirstSignDTerm(size_t j, Rational const& factor, Rational& term) const
{
    term = Rational(mDegree + j) * Rational(2 * (mDegree + j + 1)) * factor;
}

void RemezRotC2::GetSecondSignDTerm(size_t j, Rational const& tSqr, Rational& factor,
    Rational& term) const
{
    factor *= tSqr / Rational((2 * (mDegree + j) + 4) * (2 * (mDegree + j) + 5));
    term = Rational(mDegree + j + 1) * Rational(2 * (mDegree + j + 2)) * factor;
}
