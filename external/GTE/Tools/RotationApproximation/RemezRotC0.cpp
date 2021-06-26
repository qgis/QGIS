// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2020.08.11

#include "RemezRotC0.h"
using namespace gte;

// F(t) = sin(t) / t
//      = sum_{i=0}^{inf} (-1)^{i} (1/(2i+1)!) t^{2i}
//
// G(t) = F'(t) / (2t)
//      = (t cos(t) - sin(t)) / (2t^3)
//      = sum_{i=0}^{inf} (-1)^{i+1} ((i+1)/(2i+3)!) t^{2i}
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
//      = sum_{i=0}^{n} [(-1)^{i}/(2i+1)! - c_{i}] t^{2i}
//
// B(t) = Taylor(n-1, G(t)) - Q(t)
//      = sum_{i=0}^{n-1} (i+1) [(-1)^{i+1}/(2i+3)! - c_{i+1}] t^{2i}

RemezRotC0::RemezRotC0()
    :
    RemezConstrained()
{
}

double RemezRotC0::F(double t) const
{
    if (t > 0.0)
    {
        return std::sin(t) / t;
    }
    else
    {
        return 1.0;
    }
}

double RemezRotC0::FDer(double t) const
{
    if (t > 0.0)
    {
        return (t * std::cos(t) - std::sin(t)) / t / t;
    }
    else
    {
        return 0.0;
    }
}

void RemezRotC0::ComputeACoefficients()
{
    mACoefficients.resize(mDegree + 1);

    mFactorial.resize(mDegree + 2);
    mFactorial[0] = 1;  // (2i+1)! at i = 0
    for (size_t i = 1, im1 = 0; i <= mDegree + 1; ++i, ++im1)
    {
        mFactorial[i] = mFactorial[im1] * Rational((2 * i) * (2 * i + 1));
    }

    Rational negOnePow = 1;
    for (size_t i = 0; i <= mDegree; ++i)
    {
        Rational term0 = negOnePow / mFactorial[i];
        Rational term1 = mPCoefficients[i];
        mACoefficients[i] = term0 - term1;
        negOnePow.SetSign(-negOnePow.GetSign());
    }
}

void RemezRotC0::ComputeBCoefficients()
{
    mBCoefficients.resize(mDegree);

    mFactorial.resize(mDegree + 1);
    mFactorial[0] = 6;  // (2i+3)! at i = 0
    for (size_t i = 1, im1 = 0; i <= mDegree; ++i, ++im1)
    {
        mFactorial[i] = mFactorial[im1] * Rational((2 * i + 2) * (2 * i + 3));
    }

    Rational negOnePow = -1;
    for (size_t i = 0; i < mDegree; ++i)
    {
        Rational term0 = negOnePow / mFactorial[i];
        Rational term1 = mPCoefficients[i + 1];
        mBCoefficients[i] = Rational(i + 1) * (term0 - term1);
        negOnePow.SetSign(-negOnePow.GetSign());
    }
}

RemezRotC0::Interval RemezRotC0::GetIntervalF(double t,
    Interval const&, Interval const& iSin) const
{
    if (t > 0.0)
    {
        Interval iT(t);
        Interval iF = iSin / iT;
        return iF;
    }
    else
    {
        return Interval(1.0);
    }
}

RemezRotC0::Interval RemezRotC0::GetIntervalG(double t,
    Interval const& iCos, Interval const& iSin) const
{
    if (t > 0.0)
    {
        Interval iT(t);
        Interval iTSqr = iT * iT;
        Interval iTCube = iT * iTSqr;
        Interval iG = Interval(0.5) * (iT * iCos - iSin) / iTCube;
        return iG;
    }
    else
    {
        return Interval(-1.0) / Interval(6.0);
    }
}

void RemezRotC0::GetFirstSignETerm(size_t, Rational const& factor, Rational& term) const
{
    term = factor;
}

void RemezRotC0::GetSecondSignETerm(size_t j, Rational const& tSqr, Rational&,
    Rational& term) const
{
    term *= tSqr / Rational((2 * (mDegree + j) + 2) * (2 * (mDegree + j) + 3));
}

void RemezRotC0::GetFirstSignDTerm(size_t j, Rational const& factor, Rational& term) const
{
    term = Rational(mDegree + j) * factor;
}

void RemezRotC0::GetSecondSignDTerm(size_t j, Rational const& tSqr, Rational& factor,
    Rational& term) const
{
    factor *= tSqr / Rational((2 * (mDegree + j) + 2) * (2 * (mDegree + j) + 3));
    term = Rational(mDegree + j + 1) * factor;
}
