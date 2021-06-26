// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.03.08

#pragma once

#include <Mathematics/Integration.h>
#include <Mathematics/RootsBisection.h>
#include <Mathematics/Vector.h>

namespace gte
{
    template <int N, typename Real>
    class ParametricCurve
    {
    protected:
        // Abstract base class for a parameterized curve X(t), where t is the
        // parameter in [tmin,tmax] and X is an N-tuple position.  The first
        // constructor is for single-segment curves. The second constructor is
        // for multiple-segment curves. The times must be strictly increasing.
        ParametricCurve(Real tmin, Real tmax)
            :
            mTime(2),
            mSegmentLength(1, (Real)0),
            mAccumulatedLength(1, (Real)0),
            mRombergOrder(DEFAULT_ROMBERG_ORDER),
            mMaxBisections(DEFAULT_MAX_BISECTIONS),
            mConstructed(false)
        {
            mTime[0] = tmin;
            mTime[1] = tmax;
        }

        ParametricCurve(int numSegments, Real const* times)
            :
            mTime(numSegments + 1),
            mSegmentLength(numSegments, (Real)0),
            mAccumulatedLength(numSegments, (Real)0),
            mRombergOrder(DEFAULT_ROMBERG_ORDER),
            mMaxBisections(DEFAULT_MAX_BISECTIONS),
            mConstructed(false)
        {
            std::copy(times, times + numSegments + 1, mTime.begin());
        }

    public:
        virtual ~ParametricCurve()
        {
        }

        // To validate construction, create an object as shown:
        //     DerivedClassCurve<N, Real> curve(parameters);
        //     if (!curve) { <constructor failed, handle accordingly>; }
        inline operator bool() const
        {
            return mConstructed;
        }

        // Member access.
        inline Real GetTMin() const
        {
            return mTime.front();
        }

        inline Real GetTMax() const
        {
            return mTime.back();
        }

        inline int GetNumSegments() const
        {
            return static_cast<int>(mSegmentLength.size());
        }

        inline Real const* GetTimes() const
        {
            return &mTime[0];
        }

        // This function applies only when the first constructor is used (two
        // times rather than a sequence of three or more times).
        void SetTimeInterval(Real tmin, Real tmax)
        {
            if (mTime.size() == 2)
            {
                mTime[0] = tmin;
                mTime[1] = tmax;
            }
        }

        // Parameters used in GetLength(...), GetTotalLength() and
        // GetTime(...).

        // The default value is 8.
        inline void SetRombergOrder(int order)
        {
            mRombergOrder = std::max(order, 1);
        }

        // The default value is 1024.
        inline void SetMaxBisections(unsigned int maxBisections)
        {
            mMaxBisections = std::max(maxBisections, 1u);
        }

        // Evaluation of the curve.  The function supports derivative
        // calculation through order 3; that is, order <= 3 is required.  If
        // you want/ only the position, pass in order of 0.  If you want the
        // position and first derivative, pass in order of 1, and so on.  The
        // output array 'jet' must have enough storage to support the maximum
        // order.  The values are ordered as: position, first derivative,
        // second derivative, third derivative.
        enum { SUP_ORDER = 4 };
        virtual void Evaluate(Real t, unsigned int order, Vector<N, Real>* jet) const = 0;

        void Evaluate(Real t, unsigned int order, Real* values) const
        {
            Evaluate(t, order, reinterpret_cast<Vector<N, Real>*>(values));
        }

        // Differential geometric quantities.
        Vector<N, Real> GetPosition(Real t) const
        {
            Vector<N, Real> position;
            Evaluate(t, 0, &position);
            return position;
        }

        Vector<N, Real> GetTangent(Real t) const
        {
            std::array<Vector<N, Real>, 2> jet;  // (position, tangent)
            Evaluate(t, 1, jet.data());
            Normalize(jet[1]);
            return jet[1];
        }

        Real GetSpeed(Real t) const
        {
            std::array<Vector<N, Real>, 2> jet;  // (position, tangent)
            Evaluate(t, 1, jet.data());
            return Length(jet[1]);
        }

        Real GetLength(Real t0, Real t1) const
        {
            std::function<Real(Real)> speed = [this](Real t)
            {
                return GetSpeed(t);
            };

            if (mSegmentLength[0] == (Real)0)
            {
                // Lazy initialization of lengths of segments.
                int const numSegments = static_cast<int>(mSegmentLength.size());
                Real accumulated = (Real)0;
                for (int i = 0; i < numSegments; ++i)
                {
                    mSegmentLength[i] = Integration<Real>::Romberg(mRombergOrder,
                        mTime[i], mTime[i + 1], speed);
                    accumulated += mSegmentLength[i];
                    mAccumulatedLength[i] = accumulated;
                }
            }

            t0 = std::max(t0, GetTMin());
            t1 = std::min(t1, GetTMax());
            auto iter0 = std::lower_bound(mTime.begin(), mTime.end(), t0);
            int index0 = static_cast<int>(iter0 - mTime.begin());
            auto iter1 = std::lower_bound(mTime.begin(), mTime.end(), t1);
            int index1 = static_cast<int>(iter1 - mTime.begin());

            Real length;
            if (index0 < index1)
            {
                length = (Real)0;
                if (t0 < *iter0)
                {
                    length += Integration<Real>::Romberg(mRombergOrder, t0,
                        mTime[index0], speed);
                }

                int isup;
                if (t1 < *iter1)
                {
                    length += Integration<Real>::Romberg(mRombergOrder,
                        mTime[index1 - 1], t1, speed);
                    isup = index1 - 1;
                }
                else
                {
                    isup = index1;
                }
                for (int i = index0; i < isup; ++i)
                {
                    length += mSegmentLength[i];
                }
            }
            else
            {
                length = Integration<Real>::Romberg(mRombergOrder, t0, t1, speed);
            }
            return length;
        }

        Real GetTotalLength() const
        {
            if (mAccumulatedLength.back() == (Real)0)
            {
                // Lazy evaluation of the accumulated length array.
                return GetLength(mTime.front(), mTime.back());
            }

            return mAccumulatedLength.back();
        }

        // Inverse mapping of s = Length(t) given by t = Length^{-1}(s).  The
        // inverse length function generally cannot be written in closed form,
        // in which case it is not directly computable.  Instead, we can
        // specify s and estimate the root t for F(t) = Length(t) - s.  The
        // derivative is F'(t) = Speed(t) >= 0, so F(t) is nondecreasing.  To
        // be robust, we use bisection to locate the root, although it is
        // possible to use a hybrid of Newton's method and bisection.  For
        // details, see the document
        // https://www.geometrictools.com/Documentation/MovingAlongCurveSpecifiedSpeed.pdf
        Real GetTime(Real length) const
        {
            if (length > (Real)0)
            {
                if (length < GetTotalLength())
                {
                    std::function<Real(Real)> F = [this, &length](Real t)
                    {
                        return Integration<Real>::Romberg(mRombergOrder,
                            mTime.front(), t, [this](Real z) { return GetSpeed(z); })
                            - length;
                    };

                    // We know that F(tmin) < 0 and F(tmax) > 0, which allows us to
                    // use bisection.  Rather than bisect the entire interval, let's
                    // narrow it down with a reasonable initial guess.
                    Real ratio = length / GetTotalLength();
                    Real omratio = (Real)1 - ratio;
                    Real tmid = omratio * mTime.front() + ratio * mTime.back();
                    Real fmid = F(tmid);
                    if (fmid > (Real)0)
                    {
                        RootsBisection<Real>::Find(F, mTime.front(), tmid, (Real)-1,
                            (Real)1, mMaxBisections, tmid);
                    }
                    else if (fmid < (Real)0)
                    {
                        RootsBisection<Real>::Find(F, tmid, mTime.back(), (Real)-1,
                            (Real)1, mMaxBisections, tmid);
                    }
                    return tmid;
                }
                else
                {
                    return mTime.back();
                }
            }
            else
            {
                return mTime.front();
            }
        }

        // Compute a subset of curve points according to the specified attribute.
        // The input 'numPoints' must be two or larger.
        void SubdivideByTime(int numPoints, Vector<N, Real>* points) const
        {
            Real delta = (mTime.back() - mTime.front()) / (Real)(numPoints - 1);
            for (int i = 0; i < numPoints; ++i)
            {
                Real t = mTime.front() + delta * i;
                points[i] = GetPosition(t);
            }
        }

        void SubdivideByLength(int numPoints, Vector<N, Real>* points) const
        {
            Real delta = GetTotalLength() / (Real)(numPoints - 1);
            for (int i = 0; i < numPoints; ++i)
            {
                Real length = delta * i;
                Real t = GetTime(length);
                points[i] = GetPosition(t);
            }
        }

    protected:
        enum
        {
            DEFAULT_ROMBERG_ORDER = 8,
            DEFAULT_MAX_BISECTIONS = 1024
        };

        std::vector<Real> mTime;
        mutable std::vector<Real> mSegmentLength;
        mutable std::vector<Real> mAccumulatedLength;
        int mRombergOrder;
        unsigned int mMaxBisections;
        bool mConstructed;
    };
}
