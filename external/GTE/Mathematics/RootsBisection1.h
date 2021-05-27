// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.6.2020.02.05

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <functional>

// Estimate a root on an interval [tMin,tMax] for a continuous function F(t)
// defined on that interval. If a root is found, the function returns it via
// tRoot. Additionally, fAtTRoot = F(tRoot) is returned in case the caller
// wants to know how close to zero the function is at the root; numerical
// rounding errors can cause fAtTRoot not to be exactly zero. The returned
// uint32_t is the number of iterations used by the bisector. If that number
// is 0, F(tMin)*F(tMax) > 0 and it is unknown whether [tMin,tMax] contains
// a root. If that number is 1, either F(tMin) = 0 or F(tMax) = 0 (exactly),
// and tRoot is the corresponding interval endpoint. If that number is 2 or
// larger, the bisection is applied until tRoot is found for which F(tRoot)
// is exactly 0 or until the current root estimate is equal to tMin or tMax.
// The latter conditions can occur because of the fixed precision used in
// the computations (24-bit precision for 'float', 53-bit precision for
// 'double' or a user-specified precision for arbitrary-precision numbers.

namespace gte
{
    template <typename Real>
    class RootsBisection1
    {
    public:
        // Use this constructor when Real is a floating-point type.
        template <typename Dummy = Real>
        RootsBisection1(uint32_t maxIterations,
            typename std::enable_if<!is_arbitrary_precision<Dummy>::value>::type* = nullptr)
            :
            mPrecision(0),
            mMaxIterations(maxIterations)
        {
            static_assert(!is_arbitrary_precision<Real>::value,
                "Template parameter is not a floating-point type.");
            LogAssert(mMaxIterations > 0, "Invalid maximum iterations.");
        }

        // Use this constructor when Real is an arbitrary-precision type.
        // If you want infinite precision (no rounding of any computational
        // results), set precision to std::numeric_limits<uint32_t>::max().
        // For rounding of each computational result throughout the process,
        // set precision to be a positive number smaller than the maximum of
        // uint32_t.
        template <typename Dummy = Real>
        RootsBisection1(uint32_t precision, uint32_t maxIterations,
            typename std::enable_if<is_arbitrary_precision<Dummy>::value>::type* = nullptr)
            :
            mPrecision(precision),
            mMaxIterations(maxIterations)
        {
            static_assert(is_arbitrary_precision<Real>::value,
                "Template parameter is not an arbitrary-precision type.");
            LogAssert(mMaxIterations > 0, "Invalid maximum iterations.");
            LogAssert(mPrecision > 0, "Invalid precision.");
        }

        // Disallow copy and move semantics.
        RootsBisection1(RootsBisection1 const&) = delete;
        RootsBisection1(RootsBisection1&&) = delete;
        RootsBisection1& operator=(RootsBisection1 const&) = delete;
        RootsBisection1& operator=(RootsBisection1&&) = delete;

        // Use this function when F(tMin) and F(tMax) are not already known.
        uint32_t operator()(std::function<Real(Real const&)> F,
            Real const& tMin, Real const& tMax, Real& tRoot, Real& fAtTRoot)
        {
            LogAssert(tMin < tMax, "Invalid ordering of t-interval endpoints.");

            // Use floating-point inputs as is. Round arbitrary-precision
            // inputs to the specified precision.
            Real t0, t1;
            RoundInitial(tMin, tMax, t0, t1);
            Real f0 = F(t0), f1 = F(t1);
            return operator()(F, t0, t1, f0, f1, tRoot, fAtTRoot);
        }

        // Use this function when fAtTMin = F(tMin) and fAtTMax = F(tMax) are
        // already known. This is useful when |fAtTMin| or |fAtTMax| is
        // infinite, whereby you can pass sign(fAtTMin) or sign(fAtTMax)
        // rather than then an infinity because the bisector cares only about
        // the signs of F(t).
        uint32_t operator()(std::function<Real(Real const&)> F,
            Real const& tMin, Real const& tMax, Real const& fMin, Real const& fMax,
            Real& tRoot, Real& fAtTRoot)
        {
            LogAssert(tMin < tMax, "Invalid ordering of t-interval endpoints.");

            Real const zero(0);
            int sign0 = (fMin > zero ? +1 : (fMin < zero ? -1 : 0));
            if (sign0 == 0)
            {
                tRoot = tMin;
                fAtTRoot = zero;
                return 1;
            }

            int sign1 = (fMax > zero ? +1 : (fMax < zero ? -1 : 0));
            if (sign1 == 0)
            {
                tRoot = tMax;
                fAtTRoot = zero;
                return 1;
            }

            if (sign0 == sign1)
            {
                // It is unknown whether the interval contains a root.
                tRoot = zero;
                fAtTRoot = zero;
                LogWarning("Interval might not contain a root.");
                return 0;
            }

            // The bisection steps.
            Real t0 = tMin, t1 = tMax;
            uint32_t iteration;
            for (iteration = 2; iteration <= mMaxIterations; ++iteration)
            {
                // Use the floating-point average as is. Round the
                // arbitrary-precision average to the specified precision.
                tRoot = RoundAverage(t0, t1);
                fAtTRoot = F(tRoot);

                // If the function is exactly zero, a root is found. For
                // fixed precision, the average of two consecutive numbers
                // might one of the current interval endpoints.
                int signRoot = (fAtTRoot > zero ? +1 : (fAtTRoot < zero ? -1 : 0));
                if (signRoot == 0 || tRoot == t0 || tRoot == t1)
                {
                    break;
                }

                // Update the correct endpoint to the midpoint.
                if (signRoot == sign0)
                {
                    t0 = tRoot;
                }
                else // signRoot == sign1
                {
                    t1 = tRoot;
                }
            }
            return iteration;
        }

    private:
        // Floating-point numbers are used without rounding.
        template <typename Dummy = Real>
        typename std::enable_if<!is_arbitrary_precision<Dummy>::value, void>::type
        RoundInitial(Real const& inT0, Real const& inT1, Real& t0, Real& t1)
        {
            t0 = inT0;
            t1 = inT1;
        }

        template <typename Dummy = Real>
        typename std::enable_if<!is_arbitrary_precision<Dummy>::value, Real>::type
        RoundAverage(Real const& t0, Real const& t1)
        {
            Real average = static_cast<Real>(0.5)* (t0 + t1);
            return average;
        }

        // Arbitrary-precision numbers are used with rounding.
        template <typename Dummy = Real>
        typename std::enable_if<is_arbitrary_precision<Dummy>::value, void>::type
        RoundInitial(Real const& inT0, Real const& inT1, Real& t0, Real& t1)
        {
            if (mPrecision < std::numeric_limits<uint32_t>::max())
            {
                Convert(inT0, mPrecision, FE_TONEAREST, t0);
                Convert(inT1, mPrecision, FE_TONEAREST, t1);
            }
            else
            {
                t0 = inT0;
                t1 = inT1;
            }
        }

        template <typename Dummy = Real>
        typename std::enable_if<is_arbitrary_precision<Dummy>::value, Real>::type
        RoundAverage(Real const& t0, Real const& t1)
        {
            Real average = std::ldexp(t0 + t1, -1);  // = (t0 + t1) / 2
            if (mPrecision < std::numeric_limits<uint32_t>::max())
            {
                Real roundedAverage;
                Convert(average, mPrecision, FE_TONEAREST, roundedAverage);
                return roundedAverage;
            }
            else
            {
                return average;
            }
        }

        uint32_t mPrecision, mMaxIterations;
    };
}
