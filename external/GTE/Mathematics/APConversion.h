// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.03.15

#pragma once

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/QFNumber.h>

// The conversion functions here are used to obtain arbitrary-precision
// approximations to rational numbers and to quadratic field numbers.
// The arbitrary-precision arithmetic is described in
// https://www.geometrictools.com/Documentation/ArbitraryPrecision.pdf
// The quadratic field numbers and conversions are described in
// https://www.geometrictools.com/Documentation/QuadraticFields.pdf

namespace gte
{
    template <typename Rational>
    class APConversion
    {
    public:
        using QFN1 = QFNumber<Rational, 1>;
        using QFN2 = QFNumber<Rational, 2>;

        // Construction and destruction.
        APConversion(int32_t precision, uint32_t maxIterations)
            :
            mZero(0),
            mOne(1),
            mThree(3),
            mFive(5),
            mPrecision(precision),
            mMaxIterations(maxIterations),
            mThreshold(std::ldexp(mOne, -mPrecision))
        {
            LogAssert(precision > 0, "Invalid precision.");
            LogAssert(maxIterations > 0, "Invalid maximum iterations.");
        }

        ~APConversion()
        {
        }

        // Member access.
        void SetPrecision(int32_t precision)
        {
            LogAssert(precision > 0, "Invalid precision.");
            mPrecision = precision;
            mThreshold = std::ldexp(mOne, -mPrecision);
        }

        void SetMaxIterations(uint32_t maxIterations)
        {
            LogAssert(maxIterations > 0, "Invalid maximum iterations.");
            mMaxIterations = maxIterations;
        }

        inline int32_t GetPrecision() const
        {
            return mPrecision;
        }

        inline uint32_t GetMaxIterations() const
        {
            return mMaxIterations;
        }

        // Disallow copying and moving.
        APConversion(APConversion const&) = delete;
        APConversion(APConversion&&) = delete;
        APConversion& operator=(APConversion const&) = delete;
        APConversion& operator=(APConversion&&) = delete;

        // The input a^2 is rational, but a itself is usually irrational,
        // although a rational value is allowed. Compute a bounding interval
        // for the root, aMin <= a <= aMax, where the endpoints are both
        // within the specified precision.
        uint32_t EstimateSqrt(Rational const& aSqr, Rational& aMin, Rational& aMax)
        {
            // Factor a^2 = r^2 * 2^e, where r^2 in [1/2,1). Compute s^2 and
            // the exponent used to generate the estimate of sqrt(a^2).
            Rational sSqr;
            int exponentA;
            PreprocessSqr(aSqr, sSqr, exponentA);

            // Use the FPU to estimate s = sqrt(sSqr) to 53-bit precision with
            // rounding up. Multiply by the appropriate exponent to obtain
            // upper bound aMax > a.
            aMax = GetMaxOfSqrt(sSqr, exponentA);

            // Compute a lower bound aMin < a.
            aMin = aSqr / aMax;

            // Compute Newton iterates until convergence. The estimate closest
            // to a is aMin with aMin <= a <= aMax and a - aMin <= aMax - a.
            uint32_t iterate;
            for (iterate = 1; iterate <= mMaxIterations; ++iterate)
            {
                if (aMax - aMin < mThreshold)
                {
                    break;
                }
                // Compute the average aMax = (aMin + aMax) / 2. Round up
                // to twice the precision to avoid quadratic growth in the
                // number of bits and to ensure that aMin can increase.
                aMax = std::ldexp(aMin + aMax, -1);
                Convert(aMax, 2 * mPrecision, FE_UPWARD, aMax);
                aMin = aSqr / aMax;
            }
            return iterate;
        }

        // Compute an estimate of the root when you do not need a bounding
        // interval.
        uint32_t EstimateSqrt(Rational const& aSqr, Rational& a)
        {
            // Compute a bounding interval aMin <= a <= aMax.
            Rational aMin, aMax;
            uint32_t numIterates = EstimateSqrt(aSqr, aMin, aMax);
            // Use the average of the interval endpoints as the estimate.
            a = std::ldexp(aMin + aMax, -1);
            return numIterates;
        }

        uint32_t EstimateApB(Rational const& aSqr, Rational const& bSqr,
            Rational& tMin, Rational& tMax)
        {
            // Factor a^2 = r^2 * 2^e, where r^2 in [1/2,1). Compute u^2 and
            // the exponent used to generate the estimate of sqrt(a^2).
            Rational uSqr;
            int32_t exponentA;
            PreprocessSqr(aSqr, uSqr, exponentA);

            // Factor b^2 = s^2 * 2^e, where s^2 in [1/2,1). Compute v^2 and
            // the exponent used to generate the estimate of sqrt(b^2).
            Rational vSqr;
            int32_t exponentB;
            PreprocessSqr(bSqr, vSqr, exponentB);

            // Use the FPU to estimate u = sqrt(u^2) and v = sqrt(v^2) to
            // 53 bits of precision with rounding up. Multiply by the
            // appropriate exponents to obtain upper bounds aMax > a and
            // bMax > b. This ensures tMax = aMax + bMax > a + b.
            Rational aMax = GetMaxOfSqrt(uSqr, exponentA);
            Rational bMax = GetMaxOfSqrt(vSqr, exponentB);
            tMax = aMax + bMax;

            // Compute a lower bound tMin < a + b.
            Rational a2pb2 = aSqr + bSqr;
            Rational a2mb2 = aSqr - bSqr;
            Rational a2mb2Sqr = a2mb2 * a2mb2;
            Rational tMaxSqr = tMax * tMax;
            tMin = (a2pb2 * tMaxSqr - a2mb2Sqr) / (tMax * (tMaxSqr - a2pb2));

            // Compute Newton iterates until convergence. The estimate closest
            // to a + b is tMin with tMin < a + b < tMax and
            // (a + b) - tMin < tMax - (a + b).
            uint32_t iterate;
            for (iterate = 1; iterate <= mMaxIterations; ++iterate)
            {
                if (tMax - tMin < mThreshold)
                {
                    break;
                }
                // Compute the weighted average tMax = (3*tMin + tMax) / 4.
                // Round up to twice the precision to avoid quadratic growth
                // in the number of bits and to ensure that tMin can increase.
                tMax = std::ldexp(mThree * tMax + tMin, -2);
                Convert(tMax, 2 * mPrecision, FE_UPWARD, tMax);
                tMaxSqr = tMax * tMax;
                tMin = (a2pb2 * tMaxSqr - a2mb2Sqr) / (tMax * (tMaxSqr - a2pb2));
            }
            return iterate;
        }

        uint32_t EstimateAmB(Rational const& aSqr, Rational const& bSqr,
            Rational& tMin, Rational& tMax)
        {
            // The return value of the function.
            uint32_t iterate = 0;

            // Compute various quantities that are used later in the code.
            Rational a2tb2 = aSqr * bSqr;               // a^2 * b^2
            Rational a2pb2 = aSqr + bSqr;               // a^2 + b^2
            Rational a2mb2 = aSqr - bSqr;               // a^2 - b^2
            Rational a2mb2Sqr = a2mb2 * a2mb2;          // (a^2 - b^2)^2
            Rational twoa2pb2 = std::ldexp(a2pb2, 1);   // 2 * (a^2 + b^2)

            // Factor a^2 = r^2 * 2^e, where r^2 in [1/2,1). Compute u^2 and
            // the exponent used to generate the estimate of sqrt(a^2).
            Rational uSqr;
            int32_t exponentA;
            PreprocessSqr(aSqr, uSqr, exponentA);

            // Factor b^2 = s^2 * 2^e, where s^2 in [1/2,1). Compute v^2 and
            // the exponent used to generate the estimate of sqrt(b^2).
            Rational vSqr;
            int32_t exponentB;
            PreprocessSqr(bSqr, vSqr, exponentB);

            // Compute the sign of f''(a-b)/8 = a^2 - 3*a*b + b^2. It can be
            // shown that Sign(a^2-3*a*b+b^2) = Sign(a^4-7*a^2*b^2+b^4) =
            // Sign((a^2-b^2)^2-5*a^2*b^2).
            Rational signSecDer = a2mb2Sqr - mFive * a2tb2;

            // Local variables shared by the two main blocks of code.
            Rational aMin, aMax, bMin, bMax, tMinSqr, tMaxSqr, tMid, tMidSqr, f;

            if (signSecDer > mZero)
            {
                // Choose an initial guess tMin < a-b. Use the FPU to
                // estimate u = sqrt(u^2) and v = sqrt(v^2) to 53 bits of
                // precision with specified rounding. Multiply by the
                // appropriate exponents to obtain tMin = aMin - bMax < a-b.
                aMin = GetMinOfSqrt(uSqr, exponentA);
                bMax = GetMaxOfSqrt(vSqr, exponentB);
                tMin = aMin - bMax;

                // When a-b is nearly zero, it is possible the lower bound is
                // negative. Clamp tMin to zero to stay on the nonnegative
                // t-axis where the f"-positive basin is.
                if (tMin < mZero)
                {
                    tMin = mZero;
                }

                // Test whether tMin is in the positive f"(t) basin containing
                // a-b. If it is not, compute a tMin that is in the basis. The
                // sign test is applied to f"(t)/4 = 3*t^2 - (a^2+b^2).
                tMinSqr = tMin * tMin;
                signSecDer = mThree * tMinSqr - a2pb2;
                if (signSecDer < mZero)
                {
                    // The initial guess satisfies f"(tMin) < 0. Compute an
                    // upper bound tMax > a-b and bisect [tMin,tMax] until
                    // either the t-value is an estimate to a-b within the
                    // specified precision or until f"(t) >= 0 and f(t) >= 0.
                    // In the latter case, continue on to Newton's method,
                    // which is then guaranteed to converge.
                    aMax = GetMaxOfSqrt(uSqr, exponentA);
                    bMin = GetMinOfSqrt(vSqr, exponentB);
                    tMax = aMax - bMin;

                    for (iterate = 1; iterate <= mMaxIterations; ++iterate)
                    {
                        if (tMax - tMin < mThreshold)
                        {
                            return iterate;
                        }

                        tMid = std::ldexp(tMin + tMax, -1);
                        tMidSqr = tMid * tMid;
                        signSecDer = mThree * tMidSqr - a2pb2;
                        if (signSecDer >= mZero)
                        {
                            f = tMidSqr * (tMidSqr - twoa2pb2) + a2mb2Sqr;
                            if (f >= mZero)
                            {
                                tMin = tMid;
                                tMinSqr = tMidSqr;
                                break;
                            }
                            else
                            {
                                // Round up to twice the precision to avoid
                                // quadratic growth in the number of bits.
                                tMax = tMid;
                                Convert(tMax, 2 * mPrecision, FE_UPWARD, tMax);
                            }
                        }
                        else
                        {
                            // Round down to twice the precision to avoid
                            // quadratic growth in the number of bits.
                            tMin = tMid;
                            Convert(tMin, 2 * mPrecision, FE_DOWNWARD, tMin);
                        }
                    }
                }

                // Compute an upper bound tMax > a-b.
                tMax = (a2pb2 * tMinSqr - a2mb2Sqr) / (tMin * (tMinSqr - a2pb2));

                // Compute Newton iterates until convergence. The estimate
                // closest to a-b is tMax with tMin < a-b < tMax and
                // tMax - (a-b) < (a-b) - tMin.
                for (iterate = 1; iterate <= mMaxIterations; ++iterate)
                {
                    if (tMax - tMin < mThreshold)
                    {
                        break;
                    }
                    // Compute the weighted average tMin = (3*tMin+tMax)/4.
                    // Round down to twice the precision to avoid quadratic
                    // growth in the number of bits and to ensure that tMax
                    // can decrease.
                    tMin = std::ldexp(mThree * tMin + tMax, -2);
                    Convert(tMin, 2 * mPrecision, FE_DOWNWARD, tMin);
                    tMinSqr = tMin * tMin;
                    tMax = (a2pb2 * tMinSqr - a2mb2Sqr) / (tMin * (tMinSqr - a2pb2));
                }
                return iterate;
            }

            if (signSecDer < mZero)
            {
                // Choose an initial guess tMax > a-b. Use the FPU to
                // estimate u = sqrt(u^2) and v = sqrt(v^2) to 53 bits of
                // precision with specified rounding. Multiply by the
                // appropriate exponents to obtain tMax = aMax - bMin > a-b.
                aMax = GetMaxOfSqrt(uSqr, exponentA);
                bMin = GetMinOfSqrt(vSqr, exponentB);
                tMax = aMax - bMin;

                // Test whether tMax is in the negative f"(t) basin containing
                // a-b. If it is not, compute a tMax that is in the basis. The
                // sign test is applied to f"(t)/4 = 3*t^2 - (a^2+b^2).
                tMaxSqr = tMax * tMax;
                signSecDer = mThree * tMaxSqr - a2pb2;
                if (signSecDer > mZero)
                {
                    // The initial guess satisfies f"(tMax) > 0. Compute a
                    // lower bound tMin < a-b and bisect [tMin,tMax] until
                    // either the t-value is an estimate to a-b within the
                    // specified precision or until f"(t) <= 0 and f(t) <= 0.
                    // In the latter case, continue on to Newton's method,
                    // which is then guaranteed to converge.
                    aMin = GetMinOfSqrt(uSqr, exponentA);
                    bMax = GetMaxOfSqrt(vSqr, exponentB);
                    tMin = aMin - bMax;

                    for (iterate = 1; iterate <= mMaxIterations; ++iterate)
                    {
                        if (tMax - tMin < mThreshold)
                        {
                            return iterate;
                        }

                        tMid = std::ldexp(tMin + tMax, -1);
                        tMidSqr = tMid * tMid;
                        signSecDer = mThree * tMidSqr - a2pb2;
                        if (signSecDer <= mZero)
                        {
                            f = tMidSqr * (tMidSqr - twoa2pb2) + a2mb2Sqr;
                            if (f <= mZero)
                            {
                                tMax = tMid;
                                tMaxSqr = tMidSqr;
                                break;
                            }
                            else
                            {
                                // Round down to twice the precision to avoid
                                // quadratic growth in the number of bits.
                                tMin = tMid;
                                Convert(tMin, 2 * mPrecision, FE_DOWNWARD, tMin);
                            }
                        }
                        else
                        {
                            // Round up to twice the precision to avoid
                            // quadratic growth in the number of bits.
                            tMax = tMid;
                            Convert(tMax, 2 * mPrecision, FE_UPWARD, tMax);
                        }
                    }
                }

                // Compute a lower bound tMin < a-b.
                tMin = (a2pb2 * tMaxSqr - a2mb2Sqr) / (tMax * (tMaxSqr - a2pb2));

                // Compute Newton iterates until convergence. The estimate
                // closest to a-b is tMin with tMin < a - b < tMax and
                // (a-b) - tMin < tMax - (a-b).
                for (iterate = 1; iterate <= mMaxIterations; ++iterate)
                {
                    if (tMax - tMin < mThreshold)
                    {
                        break;
                    }
                    // Compute the weighted average tMax = (3*tMax+tMin)/4.
                    // Round up to twice the precision to avoid quadratic
                    // growth in the number of bits and to ensure that tMin
                    // can increase.
                    tMax = std::ldexp(mThree * tMax + tMin, -2);
                    Convert(tMax, 2 * mPrecision, FE_UPWARD, tMax);
                    tMaxSqr = tMax * tMax;
                    tMin = (a2pb2 * tMaxSqr - a2mb2Sqr) / (tMax * (tMaxSqr - a2pb2));
                }
                return iterate;
            }

            // The sign of the second derivative is Sign(a^4-7*a^2*b^2+b^4)
            // and cannot be zero. Define rational r = a^2/b^2 so that
            // a^4-7*a^2*b^2+b^4 = 0. This implies r^2 - 7*r^2 + 1 = 0. The
            // irrational roots are r = (7 +- sqrt(45))/2, which is a
            // contradiction.
            LogError("This second derivative cannot be zero at a-b.");
        }

        // Compute a bounding interval for the root, qMin <= q <= qMax, where
        // the endpoints are both within the specified precision.
        uint32_t Estimate(QFN1 const& q, Rational& qMin, Rational& qMax)
        {
            Rational const& x = q.x[0];
            Rational const& y = q.x[1];
            Rational const& d = q.d;

            uint32_t numIterates;
            if (d != mZero && y != mZero)
            {
                Rational aSqr = y * y * d;
                numIterates = EstimateSqrt(aSqr, qMin, qMax);
                if (y > mZero)
                {
                    qMin = x + qMin;
                    qMax = x + qMax;
                }
                else
                {
                    Rational diff = x - qMax;
                    qMax = x - qMin;
                    qMin = diff;
                }
            }
            else
            {
                numIterates = 0;
                qMin = x;
                qMax = x;
            }

            return numIterates;
        }

        // Compute an estimate of the root when you do not need a bounding
        // interval.
        uint32_t Estimate(QFN1 const& q, Rational& qEstimate)
        {
            // Compute a bounding interval qMin <= q <= qMax.
            Rational qMin, qMax;
            uint32_t numIterates = Estimate(q, qMin, qMax);
            // Use the average of the interval endpoints as the estimate.
            qEstimate = std::ldexp(qMin + qMax, -1);
            return numIterates;
        }

    private:
        void PreprocessSqr(Rational const& aSqr, Rational& rSqr, int& exponentA)
        {
            // Factor a^2 = r^2 * 2^e, where r^2 in [1/2,1).
            int32_t exponentASqr;
            rSqr = std::frexp(aSqr, &exponentASqr);
            if (exponentASqr & 1)  // odd exponent
            {
                // a = sqrt(2*r^2) * 2^{(e-1)/2}
                exponentA = (exponentASqr - 1) / 2;
                rSqr = std::ldexp(rSqr, 1);  // = 2*rSqr
                // rSqr in [1,2)
            }
            else  // even exponent
            {
                // a = sqrt(r^2) * 2^{e/2}
                exponentA = exponentASqr / 2;
                // rSqr in [1/2,1)
            }
        }

        Rational GetMinOfSqrt(Rational const& rSqr, int exponent)
        {
            // Compute a lower bound on the square root of r^2.
            double lowerRSqr = 0.0;
            Convert(rSqr, FE_DOWNWARD, lowerRSqr);
            double sqrtLowerRSqr = std::sqrt(lowerRSqr);
            Rational aMin = std::nextafter(sqrtLowerRSqr,
                -std::numeric_limits<double>::max());
            aMin = std::ldexp(aMin, exponent);
            return aMin;
        }

        Rational GetMaxOfSqrt(Rational const& rSqr, int exponent)
        {
            // Compute an upper bound on the square root of r^2.
            double upperRSqr = 0.0;
            Convert(rSqr, FE_UPWARD, upperRSqr);
            double sqrtUpperRSqr = std::sqrt(upperRSqr);
            Rational aMax = std::nextafter(sqrtUpperRSqr,
                +std::numeric_limits<double>::max());
            aMax = std::ldexp(aMax, exponent);
            return aMax;
        }

        Rational const mZero, mOne, mThree, mFive;
        int32_t mPrecision;
        uint32_t mMaxIterations;
        Rational mThreshold;
    };
}
