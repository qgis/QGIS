// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <functional>

// Compute a root of a function F(t) on an interval [t0, t1].  The caller
// specifies the maximum number of iterations, in case you want limited
// accuracy for the root.  However, the function is designed for native types
// (Real = float/double).  If you specify a sufficiently large number of
// iterations, the root finder bisects until either F(t) is identically zero
// [a condition dependent on how you structure F(t) for evaluation] or the
// midpoint (t0 + t1)/2 rounds numerically to tmin or tmax.  Of course, it
// is required that t0 < t1.  The return value of Find is:
//   0: F(t0)*F(t1) > 0, we cannot determine a root
//   1: F(t0) = 0 or F(t1) = 0
//   2..maxIterations:  the number of bisections plus one
//   maxIterations+1:  the loop executed without a break (no convergence)

namespace gte
{
    template <typename Real>
    class RootsBisection
    {
    public:
        // Use this function when F(t0) and F(t1) are not already known.
        static unsigned int Find(std::function<Real(Real)> const& F, Real t0,
            Real t1, unsigned int maxIterations, Real& root)
        {
            // Set 'root' initially to avoid "potentially uninitialized
            // variable" warnings by a compiler.
            root = t0;

            if (t0 < t1)
            {
                // Test the endpoints to see whether F(t) is zero.
                Real f0 = F(t0);
                if (f0 == (Real)0)
                {
                    root = t0;
                    return 1;
                }

                Real f1 = F(t1);
                if (f1 == (Real)0)
                {
                    root = t1;
                    return 1;
                }

                if (f0 * f1 > (Real)0)
                {
                    // It is not known whether the interval bounds a root.
                    return 0;
                }

                unsigned int i;
                for (i = 2; i <= maxIterations; ++i)
                {
                    root = (Real)0.5 * (t0 + t1);
                    if (root == t0 || root == t1)
                    {
                        // The numbers t0 and t1 are consecutive
                        // floating-point numbers.
                        break;
                    }

                    Real fm = F(root);
                    Real product = fm * f0;
                    if (product < (Real)0)
                    {
                        t1 = root;
                        f1 = fm;
                    }
                    else if (product > (Real)0)
                    {
                        t0 = root;
                        f0 = fm;
                    }
                    else
                    {
                        break;
                    }
                }
                return i;
            }
            else
            {
                // The interval endpoints are invalid.
                return 0;
            }
        }

        // If f0 = F(t0) and f1 = F(t1) are already known, pass them to the
        // bisector.  This is useful when |f0| or |f1| is infinite, and you
        // can pass sign(f0) or sign(f1) rather than then infinity because
        // the bisector cares only about the signs of f.
        static unsigned int Find(std::function<Real(Real)> const& F, Real t0,
            Real t1, Real f0, Real f1, unsigned int maxIterations, Real& root)
        {
            // Set 'root' initially to avoid "potentially uninitialized
            // variable" warnings by a compiler.
            root = t0;

            if (t0 < t1)
            {
                // Test the endpoints to see whether F(t) is zero.
                if (f0 == (Real)0)
                {
                    root = t0;
                    return 1;
                }

                if (f1 == (Real)0)
                {
                    root = t1;
                    return 1;
                }

                if (f0 * f1 > (Real)0)
                {
                    // It is not known whether the interval bounds a root.
                    return 0;
                }

                unsigned int i;
                root = t0;
                for (i = 2; i <= maxIterations; ++i)
                {
                    root = (Real)0.5 * (t0 + t1);
                    if (root == t0 || root == t1)
                    {
                        // The numbers t0 and t1 are consecutive
                        // floating-point numbers.
                        break;
                    }

                    Real fm = F(root);
                    Real product = fm * f0;
                    if (product < (Real)0)
                    {
                        t1 = root;
                        f1 = fm;
                    }
                    else if (product > (Real)0)
                    {
                        t0 = root;
                        f0 = fm;
                    }
                    else
                    {
                        break;
                    }
                }
                return i;
            }
            else
            {
                // The interval endpoints are invalid.
                return 0;
            }
        }
    };
}
