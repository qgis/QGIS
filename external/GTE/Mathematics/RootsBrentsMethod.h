// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <cmath>
#include <functional>

// This is an implementation of Brent's Method for computing a root of a
// function on an interval [t0,t1] for which F(t0)*F(t1) < 0.  The method
// uses inverse quadratic interpolation to generate a root estimate but
// falls back to inverse linear interpolation (secant method) if
// necessary.  Moreover, based on previous iterates, the method will fall
// back to bisection when it appears the interpolated estimate is not of
// sufficient quality.
//
//   maxIterations:
//       The maximum number of iterations used to locate a root.  This
//       should be positive.
//   negFTolerance, posFTolerance:
//       The root estimate t is accepted when the function value F(t)
//       satisfies negFTolerance <= F(t) <= posFTolerance.  The values
//       must satisfy:  negFTolerance <= 0, posFTolerance >= 0.
//   stepTTolerance:
//       Brent's Method requires additional tests before an interpolated
//       t-value is accepted as the next root estimate.  One of these
//       tests compares the difference of consecutive iterates and
//       requires it to be larger than a user-specified t-tolerance (to
//       ensure progress is made).  This parameter is that tolerance
//       and should be nonnegative.
//   convTTolerance:
//       The root search is allowed to terminate when the current
//       subinterval [tsub0,tsub1] is sufficiently small, say,
//       |tsub1 - tsub0| <= tolerance.  This parameter is that tolerance
//       and should be nonnegative.

namespace gte
{
    template <typename Real>
    class RootsBrentsMethod
    {
    public:
        // It is necessary that F(t0)*F(t1) <= 0, in which case the function
        // returns 'true' and the 'root' is valid; otherwise, the function
        // returns 'false' and 'root' is invalid (do not use it).  When
        // F(t0)*F(t1) > 0, the interval may very well contain a root but we
        // cannot know that.  The function also returns 'false' if t0 >= t1.

        static bool Find(std::function<Real(Real)> const& F, Real t0, Real t1,
            unsigned int maxIterations, Real negFTolerance, Real posFTolerance,
            Real stepTTolerance, Real convTTolerance, Real& root)
        {
            // Parameter validation.
            if (t1 <= t0
                || maxIterations == 0
                || negFTolerance > (Real)0
                || posFTolerance < (Real)0
                || stepTTolerance < (Real)0
                || convTTolerance < (Real)0)
            {
                // The input is invalid.
                return false;
            }

            Real f0 = F(t0);
            if (negFTolerance <= f0 && f0 <= posFTolerance)
            {
                // This endpoint is an approximate root that satisfies the
                // function tolerance.
                root = t0;
                return true;
            }

            Real f1 = F(t1);
            if (negFTolerance <= f1 && f1 <= posFTolerance)
            {
                // This endpoint is an approximate root that satisfies the
                // function tolerance.
                root = t1;
                return true;
            }

            if (f0 * f1 > (Real)0)
            {
                // The input interval must bound a root.
                return false;
            }

            if (std::fabs(f0) < std::fabs(f1))
            {
                // Swap t0 and t1 so that |F(t1)| <= |F(t0)|.  The number t1
                // is considered to be the best estimate of the root.
                std::swap(t0, t1);
                std::swap(f0, f1);
            }

            // Initialize values for the root search.
            Real t2 = t0, t3 = t0, f2 = f0;
            bool prevBisected = true;

            // The root search.
            for (unsigned int i = 0; i < maxIterations; ++i)
            {
                Real fDiff01 = f0 - f1, fDiff02 = f0 - f2, fDiff12 = f1 - f2;
                Real invFDiff01 = ((Real)1) / fDiff01;
                Real s;
                if (fDiff02 != (Real)0 && fDiff12 != (Real)0)
                {
                    // Use inverse quadratic interpolation.
                    Real infFDiff02 = ((Real)1) / fDiff02;
                    Real invFDiff12 = ((Real)1) / fDiff12;
                    s =
                        t0 * f1 * f2 * invFDiff01 * infFDiff02 -
                        t1 * f0 * f2 * invFDiff01 * invFDiff12 +
                        t2 * f0 * f1 * infFDiff02 * invFDiff12;
                }
                else
                {
                    // Use inverse linear interpolation (secant method).
                    s = (t1 * f0 - t0 * f1) * invFDiff01;
                }

                // Compute values need in the accept-or-reject tests.
                Real tDiffSAvr = s - ((Real)0.75) * t0 - ((Real)0.25) * t1;
                Real tDiffS1 = s - t1;
                Real absTDiffS1 = std::fabs(tDiffS1);
                Real absTDiff12 = std::fabs(t1 - t2);
                Real absTDiff23 = std::fabs(t2 - t3);

                bool currBisected = false;
                if (tDiffSAvr * tDiffS1 > (Real)0)
                {
                    // The value s is not between 0.75*t0 + 0.25*t1 and t1.
                    // NOTE: The algorithm sometimes has t0 < t1 but sometimes
                    // t1 < t0, so the between-ness test does not use simple
                    // comparisons.
                    currBisected = true;
                }
                else if (prevBisected)
                {
                    // The first of Brent's tests to determine whether to
                    // accept the interpolated s-value.
                    currBisected =
                        (absTDiffS1 >= ((Real)0.5) * absTDiff12) ||
                        (absTDiff12 <= stepTTolerance);
                }
                else
                {
                    // The second of Brent's tests to determine whether to
                    // accept the interpolated s-value.
                    currBisected =
                        (absTDiffS1 >= ((Real)0.5) * absTDiff23) ||
                        (absTDiff23 <= stepTTolerance);
                }

                if (currBisected)
                {
                    // One of the additional tests failed, so reject the
                    // interpolated s-value and use bisection instead.
                    s = ((Real)0.5) * (t0 + t1);
                    if (s == t0 || s == t1)
                    {
                        // The numbers t0 and t1 are consecutive
                        // floating-point numbers.
                        root = s;
                        return true;
                    }
                    prevBisected = true;
                }
                else
                {
                    prevBisected = false;
                }

                // Evaluate the function at the new estimate and test for
                // convergence.
                Real fs = F(s);
                if (negFTolerance <= fs && fs <= posFTolerance)
                {
                    root = s;
                    return true;
                }

                // Update the subinterval to include the new estimate as an
                // endpoint.
                t3 = t2;
                t2 = t1;
                f2 = f1;
                if (f0 * fs < (Real)0)
                {
                    t1 = s;
                    f1 = fs;
                }
                else
                {
                    t0 = s;
                    f0 = fs;
                }

                // Allow the algorithm to terminate when the subinterval is
                // sufficiently small.
                if (std::fabs(t1 - t0) <= convTTolerance)
                {
                    root = t1;
                    return true;
                }

                // A loop invariant is that t1 is the root estimate,
                // F(t0)*F(t1) < 0 and |F(t1)| <= |F(t0)|.
                if (std::fabs(f0) < std::fabs(f1))
                {
                    std::swap(t0, t1);
                    std::swap(f0, f1);
                }
            }

            // Failed to converge in the specified number of iterations.
            return false;
        }
    };
}
