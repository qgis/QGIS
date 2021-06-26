// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.05.21

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <algorithm>
#include <cstdint>
#include <functional>

// Search for a minimum of F(t) on [t0,t1] using successive parabolic
// interpolation. The search is recursive based on the polyline associated
// with (t,F(t)) at the endpoints and the midpoint of an interval. Let
// f0 = F(t0), f1 = F(t1), tm is in (t0,t1) and fm = F(tm). The polyline is
// {(t0,f0),(tm,fm),(t1,f1)}.
//
// If the polyline is V-shaped, the interval [t0,t1] contains a minimum
// point. The polyline is fit with a parabola whose vertex tv is in (t0,t1).
// Let fv = tv. If {(t0,f0),(tv,fv),(tm,fm)}} is a minimum bracket, the
// parabolic interpolation continues in [t0,tm]. If instead
// {(tm,fm),(tv,fv),(t1,f1)}} is a minimum bracket, the parabolic
// interpolation continues in [tm,t1].
//
// If the polyline is not V-shaped, both subintervals [t0,tm] and [tm,t1]
// are searched for a minimum.

namespace gte
{
    template <typename T>
    class Minimize1
    {
    public:
        // Construction.
        Minimize1(std::function<T(T)> const& F, int32_t maxSubdivisions,
            int32_t maxBisections, T epsilon = static_cast<T>(1e-08),
            T tolerance = static_cast<T>(1e-04))
            :
            mFunction(F),
            mMaxSubdivisions(maxSubdivisions),
            mMaxBisections(maxBisections),
            mEpsilon(std::max(epsilon, static_cast<T>(0))),
            mTolerance(std::max(tolerance, static_cast<T>(0)))
        {
            LogAssert(
                mMaxSubdivisions > 0 && mMaxBisections > 0,
                "Invalid argument.");

            SetEpsilon(epsilon);
            SetTolerance(tolerance);
        }

        // Member access.
        inline void SetEpsilon(T epsilon)
        {
            mEpsilon = std::max(epsilon, static_cast<T>(0));
        }

        inline void SetTolerance(T tolerance)
        {
            mTolerance = std::max(tolerance, static_cast<T>(0));
        }

        inline T GetEpsilon() const
        {
            return mEpsilon;
        }

        inline T GetTolerance() const
        {
            return mTolerance;
        }

        // Search for a minimum of F(t) on the interval [t0,t1] using an
        // initial guess of (t0+t1)/2. The location of the minimum is tMin
        // and the value of the minimum is fMin = F(tMin).
        void GetMinimum(T t0, T t1, T& tMin, T& fMin)
        {
            GetMinimum(t0, t1, static_cast<T>(0.5) * (t0 + t1), tMin, fMin);
        }

        // Search for a minimum of F(t) on the interval [t0,t1] using an
        // initial guess of tInitial. The location of the minimum is tMin
        // and the value of the minimum is fMin = F(tMin).
        void GetMinimum(T t0, T t1, T tInitial, T& tMin, T& fMin)
        {
            LogAssert(
                t0 <= tInitial && tInitial <= t1,
                "Invalid initial t value.");

            // Compute the minimum for the 3 initial points.
            mTMin = std::numeric_limits<T>::max();
            mFMin = std::numeric_limits<T>::max();

            T f0 = mFunction(t0);
            if (f0 < mFMin)
            {
                mTMin = t0;
                mFMin = f0;
            }

            T fInitial = mFunction(tInitial);
            if (fInitial < mFMin)
            {
                mTMin = tInitial;
                mFMin = fInitial;
            }

            T f1 = mFunction(t1);
            if (f1 < mFMin)
            {
                mTMin = t1;
                mFMin = f1;
            }

            // Search for the global minimum on [t0,t1] with tInitial chosen
            // hopefully to start with a minimum bracket.
            if (((fInitial < f0) && (f1 >= fInitial)) ||
                ((f1 > fInitial) && (f0 >= fInitial)))
            {
                // The polyline {(f0,f0), (tInitial,fInitial), (t1,f1)} is V-shaped.
                GetBracketedMinimum(t0, f0, tInitial, fInitial, t1, f1, mMaxSubdivisions);
            }
            else
            {
                // The polyline {(f0,f0), (tInitial,fInitial), (t1,f1)} is not
                // V-shaped, so continue searching in subintervals
                // [t0,tInitial] and [tInitial,t1].
                Subdivide(t0, f0, tInitial, fInitial, mMaxSubdivisions);
                Subdivide(tInitial, fInitial, t1, f1, mMaxSubdivisions);
            }

            tMin = mTMin;
            fMin = mFMin;
        }

    private:
        // Search [t0,t1] recursively for a global minimum.
        void Subdivide(T t0, T f0, T t1, T f1, int32_t subdivisionsRemaining)
        {
            if (subdivisionsRemaining-- == 0)
            {
                // The maximum number of subdivisions has been reached.
                return;
            }

            // Compute the function at the midpoint of [t0,t1].
            T tm = static_cast<T>(0.5) * (t0 + t1);
            T fm = mFunction(tm);
            if (fm < mFMin)
            {
                mTMin = tm;
                mFMin = fm;
            }

            if (((fm < f0) && (f1 >= fm)) || ((f1 > fm) && (f0 >= fm)))
            {
                // The polyline {(f0,f0), (tm,fm), (t1,f1)} is V-shaped.
                GetBracketedMinimum(t0, f0, tm, fm, t1, f1, subdivisionsRemaining);
            }
            else
            {
                // The polyline {(f0,f0), (tm,fm), (t1,f1)} is not V-shaped,
                // so continue searching in subintervals [t0,tm] and [tm,t1].
                Subdivide(t0, f0, tm, fm, subdivisionsRemaining);
                Subdivide(tm, fm, t1, f1, subdivisionsRemaining);
            }
        }

        // This is called when {f0,f1,f2} brackets a minimum.
        void GetBracketedMinimum(T t0, T f0, T tm, T fm, T t1, T f1,
            int32_t subdivisionsRemaining)
        {
            for (int32_t i = 0; i < mMaxBisections; ++i)
            {
                // Update the minimum location and value.
                if (fm < mFMin)
                {
                    mTMin = tm;
                    mFMin = fm;
                }

                // Test for convergence.
                T dt10 = t1 - t0;
                T dtBound = static_cast<T>(2) * mTolerance * std::fabs(tm) + mEpsilon;
                if (dt10 <= dtBound)
                {
                    break;
                }

                // Compute the vertex of the interpolating parabola.
                T dt0m = t0 - tm;
                T dt1m = t1 - tm;
                T df0m = f0 - fm;
                T df1m = f1 - fm;
                T tmp0 = dt0m * df1m;
                T tmp1 = dt1m * df0m;
                T denom = tmp1 - tmp0;
                if (std::fabs(denom) <= mEpsilon)
                {
                    return;
                }

                // Compute tv and clamp to [t0,t1] to offset floating-point
                // rounding errors.
                T tv = tm + static_cast<T>(0.5) * (dt1m * tmp1 - dt0m * tmp0) / denom;
                tv = std::max(t0, std::min(tv, t1));
                T fv = mFunction(tv);
                if (fv < mFMin)
                {
                    mTMin = tv;
                    mFMin = fv;
                }

                if (tv < tm)
                {
                    if (fv < fm)
                    {
                        t1 = tm;
                        f1 = fm;
                        tm = tv;
                        fm = fv;
                    }
                    else
                    {
                        t0 = tv;
                        f0 = fv;
                    }
                }
                else if (tv > tm)
                {
                    if (fv < fm)
                    {
                        t0 = tm;
                        f0 = fm;
                        tm = tv;
                        fm = fv;
                    }
                    else
                    {
                        t1 = tv;
                        f1 = fv;
                    }
                }
                else
                {
                    // The vertex of parabola is located at the middle sample
                    // point. A global minimum could occur on either
                    // subinterval.
                    Subdivide(t0, f0, tm, fm, subdivisionsRemaining);
                    Subdivide(tm, fm, t1, f1, subdivisionsRemaining);
                }
            }
        }

        std::function<T(T)> mFunction;
        int32_t mMaxSubdivisions;
        int32_t mMaxBisections;
        T mTMin, mFMin;
        T mEpsilon, mTolerance;
    };
}
