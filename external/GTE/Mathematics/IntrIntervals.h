// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.05.06

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <array>

// The intervals are of the form [t0,t1], [t0,+infinity) or (-infinity,t1].
// Degenerate intervals are allowed (t0 = t1). The queries do not perform
// validation on the input intervals to test whether t0 <= t1.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, std::array<Real, 2>, std::array<Real, 2>>
    {
    public:
        // The query tests overlap, whether a single point or an entire 
        // interval.
        struct Result
        {
            Result()
                :
                intersect(false),
                firstTime(static_cast<Real>(0)),
                lastTime(static_cast<Real>(0))
            {
            }

            bool intersect;

            // Dynamic queries (intervals moving with constant speeds). If
            // 'intersect' is true, the contact times are valid and
            //     0 <= firstTime <= lastTime,  firstTime <= maxTime
            // If 'intersect' is false, there are two cases reported. If the
            // intervals will intersect at firstTime > maxTime, the contact
            // times are reported just as when 'intersect' is true. However,
            // if the intervals will not intersect, then firstTime and
            // lastTime are both set to zero (invalid because 'intersect' is
            // false).
            Real firstTime, lastTime;
        };

        // Static query. The firstTime and lastTime values are set to zero by
        // the Result constructor, but they are invalid for the static query
        // regardless of the value of 'intersect'.
        Result operator()(std::array<Real, 2> const& interval0, std::array<Real, 2> const& interval1)
        {
            Result result{};
            result.intersect = (interval0[0] <= interval1[1] && interval0[1] >= interval1[0]);
            return result;
        }

        // Static queries where at least one interval is semiinfinite. The
        // two types of semiinfinite intervals are [a,+infinity), which I call
        // a positive-infinite interval, and (-infinity,a], which I call a
        // negative-infinite interval. The firstTime and lastTime values are
        // set to zero by the Result constructor, but they are invalid for the
        // static query regardless of the value of 'intersect'.
        Result operator()(std::array<Real, 2> const& finite, Real const& a, bool isPositiveInfinite)
        {
            Result result{};

            if (isPositiveInfinite)
            {
                result.intersect = (finite[1] >= a);
            }
            else  // is negative-infinite
            {
                result.intersect = (finite[0] <= a);
            }

            return result;
        }

        Result operator()(Real const& a0, bool isPositiveInfinite0,
            Real const& a1, bool isPositiveInfinite1)
        {
            Result result{};

            if (isPositiveInfinite0)
            {
                if (isPositiveInfinite1)
                {
                    result.intersect = true;
                }
                else  // interval1 is negative-infinite
                {
                    result.intersect = (a0 <= a1);
                }
            }
            else  // interval0 is negative-infinite
            {
                if (isPositiveInfinite1)
                {
                    result.intersect = (a0 >= a1);
                }
                else  // interval1 is negative-infinite
                {
                    result.intersect = true;
                }
            }

            return result;
        }

        // Dynamic query. Current time is 0, maxTime > 0 is required.
        Result operator()(Real maxTime, std::array<Real, 2> const& interval0,
            Real speed0, std::array<Real, 2> const& interval1, Real speed1)
        {
            Real const zero = static_cast<Real>(0);
            Result result{};

            if (interval0[1] < interval1[0])
            {
                // interval0 initially to the left of interval1.
                Real diffSpeed = speed0 - speed1;
                if (diffSpeed > zero)
                {
                    // The intervals must move towards each other. 'intersect'
                    // is true when the intervals will intersect by maxTime.
                    Real diffPos = interval1[0] - interval0[1];
                    result.intersect = (diffPos <= maxTime * diffSpeed);
                    result.firstTime = diffPos / diffSpeed;
                    result.lastTime = (interval1[1] - interval0[0]) / diffSpeed;
                    return result;
                }
            }
            else if (interval0[0] > interval1[1])
            {
                // interval0 initially to the right of interval1.
                Real diffSpeed = speed1 - speed0;
                if (diffSpeed > zero)
                {
                    // The intervals must move towards each other. 'intersect'
                    // is true when the intervals will intersect by maxTime.
                    Real diffPos = interval0[0] - interval1[1];
                    result.intersect = (diffPos <= maxTime * diffSpeed);
                    result.firstTime = diffPos / diffSpeed;
                    result.lastTime = (interval0[1] - interval1[0]) / diffSpeed;
                    return result;
                }
            }
            else
            {
                // The intervals are initially intersecting.
                result.intersect = true;
                result.firstTime = zero;
                if (speed1 > speed0)
                {
                    result.lastTime = (interval0[1] - interval1[0]) / (speed1 - speed0);
                }
                else if (speed1 < speed0)
                {
                    result.lastTime = (interval1[1] - interval0[0]) / (speed0 - speed1);
                }
                else
                {
                    result.lastTime = std::numeric_limits<Real>::max();
                }
                return result;
            }

            // The Result constructor set 'intersect' to false and the
            // 'firstTime' and 'lastTime' to zero.
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>>
    {
    public:
        // The query finds overlap, whether a single point or an entire
        // interval.
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                overlap{ static_cast<Real>(0), static_cast<Real>(0) },
                type(isEmpty),
                firstTime(static_cast<Real>(0)),
                lastTime(static_cast<Real>(0))
            {
            }

            bool intersect;

            // Static queries (no motion of intervals over time). The number
            // of number of intersections is 0 (no overlap), 1 (intervals are
            // just touching), or 2 (intervals overlap in an interval). If
            // 'intersect' is false, numIntersections is 0 and 'overlap' is
            // set to [0,0]. If 'intersect' is true, numIntersections is
            // 1 or 2. When 1, 'overlap' is set to [x,x], which is degenerate
            // and represents the single intersection point x. When 2,
            // 'overlap' is the interval of intersection.
            int numIntersections;
            std::array<Real, 2> overlap;

            // No intersection.
            static int const isEmpty = 0;

            // Intervals touch at an endpoint, [t0,t0].
            static int const isPoint = 1;

            // Finite-length interval of intersection, [t0,t1].
            static int const isFinite = 2;

            // Smiinfinite interval of intersection, [t0,+infinity). The
            // result.overlap[0] is t0 and result.overlap[1] is +1 as a
            // message that the right endpoint is +infinity (you still need
            // the result.type to know this interpretation).
            static int const isPositiveInfinite = 3;

            // Semiinfinite interval of intersection, (-infinity,t1]. The
            // result.overlap[0] is -1 as a message that the left endpoint is
            // -infinity (you still need the result.type to know this
            // interpretation). The result.overlap[1] is t1.
            static int const isNegativeInfinite = 4;

            // The dynamic queries all set the type to isDynamicQuery because
            // the queries look for time of first and last contact.
            static int const isDynamicQuery = 5;

            // The type is one of isEmpty, isPoint, isFinite,
            // isPositiveInfinite, isNegativeInfinite or isDynamicQuery.
            int type;

            // Dynamic queries (intervals moving with constant speeds). If
            // 'intersect' is true, the contact times are valid and
            //     0 <= firstTime <= lastTime,  firstTime <= maxTime
            // If 'intersect' is false, there are two cases reported. If the
            // intervals will intersect at firstTime > maxTime, the contact
            // times are reported just as when 'intersect' is true. However,
            // if the intervals will not intersect, then firstTime and
            // lastTime are both set to zero (invalid because 'intersect' is
            // false).
            Real firstTime, lastTime;
        };

        // Static query.
        Result operator()(std::array<Real, 2> const& interval0, std::array<Real, 2> const& interval1)
        {
            Result result{};

            if (interval0[1] < interval1[0] || interval0[0] > interval1[1])
            {
                result.numIntersections = 0;
                result.overlap[0] = static_cast<Real>(0);
                result.overlap[1] = static_cast<Real>(0);
                result.type = Result::isEmpty;
            }
            else if (interval0[1] > interval1[0])
            {
                if (interval0[0] < interval1[1])
                {
                    result.overlap[0] = (interval0[0] < interval1[0] ? interval1[0] : interval0[0]);
                    result.overlap[1] = (interval0[1] > interval1[1] ? interval1[1] : interval0[1]);
                    if (result.overlap[0] < result.overlap[1])
                    {
                        result.numIntersections = 2;
                        result.type = Result::isFinite;
                    }
                    else
                    {
                        result.numIntersections = 1;
                        result.type = Result::isPoint;
                    }
                }
                else  // interval0[0] == interval1[1]
                {
                    result.numIntersections = 1;
                    result.overlap[0] = interval0[0];
                    result.overlap[1] = result.overlap[0];
                    result.type = Result::isPoint;
                }
            }
            else  // interval0[1] == interval1[0]
            {
                result.numIntersections = 1;
                result.overlap[0] = interval0[1];
                result.overlap[1] = result.overlap[0];
                result.type = Result::isPoint;
            }

            result.intersect = (result.numIntersections > 0);
            return result;
        }

        // Static queries where at least one interval is semiinfinite. The
        // two types of semiinfinite intervals are [a,+infinity), which I call
        // a positive-infinite interval, and (-infinity,a], which I call a
        // negative-infinite interval.
        Result operator()(std::array<Real, 2> const& finite, Real const& a, bool isPositiveInfinite)
        {
            Result result{};

            if (isPositiveInfinite)
            {
                if (finite[1] > a)
                {
                    result.overlap[0] = std::max(finite[0], a);
                    result.overlap[1] = finite[1];
                    if (result.overlap[0] < result.overlap[1])
                    {
                        result.numIntersections = 2;
                        result.type = Result::isFinite;
                    }
                    else
                    {
                        result.numIntersections = 1;
                        result.type = Result::isPoint;
                    }
                }
                else if (finite[1] == a)
                {
                    result.numIntersections = 1;
                    result.overlap[0] = a;
                    result.overlap[1] = result.overlap[0];
                    result.type = Result::isPoint;
                }
                else
                {
                    result.numIntersections = 0;
                    result.overlap[0] = static_cast<Real>(0);
                    result.overlap[1] = static_cast<Real>(0);
                    result.type = Result::isEmpty;
                }
            }
            else  // is negative-infinite
            {
                if (finite[0] < a)
                {
                    result.overlap[0] = finite[0];
                    result.overlap[1] = std::min(finite[1], a);
                    if (result.overlap[0] < result.overlap[1])
                    {
                        result.numIntersections = 2;
                        result.type = Result::isFinite;
                    }
                    else
                    {
                        result.numIntersections = 1;
                        result.type = Result::isPoint;
                    }
                }
                else if (finite[0] == a)
                {
                    result.numIntersections = 1;
                    result.overlap[0] = a;
                    result.overlap[1] = result.overlap[0];
                    result.type = Result::isPoint;
                }
                else
                {
                    result.numIntersections = 0;
                    result.overlap[0] = static_cast<Real>(0);
                    result.overlap[1] = static_cast<Real>(0);
                    result.type = Result::isEmpty;
                }
            }

            result.intersect = (result.numIntersections > 0);
            return result;
        }

        Result operator()(Real const& a0, bool isPositiveInfinite0,
            Real const& a1, bool isPositiveInfinite1)
        {
            Result result{};

            if (isPositiveInfinite0)
            {
                if (isPositiveInfinite1)
                {
                    // overlap[1] is +infinity, but set it to +1 because Real
                    // might not have a representation for +infinity. The
                    // type indicates the interval is positive-infinite, so
                    // the +1 is a reminder that overlap[1] is +infinity.
                    result.numIntersections = 1;
                    result.overlap[0] = std::max(a0, a1);
                    result.overlap[1] = static_cast<Real>(+1);
                    result.type = Result::isPositiveInfinite;
                }
                else  // interval1 is negative-infinite
                {
                    if (a0 > a1)
                    {
                        result.numIntersections = 0;
                        result.overlap[0] = static_cast<Real>(0);
                        result.overlap[1] = static_cast<Real>(0);
                        result.type = Result::isEmpty;
                    }
                    else if (a0 < a1)
                    {
                        result.numIntersections = 2;
                        result.overlap[0] = a0;
                        result.overlap[1] = a1;
                        result.type = Result::isFinite;
                    }
                    else  // a0 == a1
                    {
                        result.numIntersections = 1;
                        result.overlap[0] = a0;
                        result.overlap[1] = result.overlap[0];
                        result.type = Result::isPoint;
                    }
                }
            }
            else  // interval0 is negative-infinite
            {
                if (isPositiveInfinite1)
                {
                    if (a0 < a1)
                    {
                        result.numIntersections = 0;
                        result.overlap[0] = static_cast<Real>(0);
                        result.overlap[1] = static_cast<Real>(0);
                        result.type = Result::isEmpty;
                    }
                    else if (a0 > a1)
                    {
                        result.numIntersections = 2;
                        result.overlap[0] = a1;
                        result.overlap[1] = a0;
                        result.type = Result::isFinite;
                    }
                    else
                    {
                        result.numIntersections = 1;
                        result.overlap[0] = a1;
                        result.overlap[1] = result.overlap[0];
                        result.type = Result::isPoint;
                    }
                    result.intersect = (a0 >= a1);
                }
                else  // interval1 is negative-infinite
                {
                    // overlap[0] is -infinity, but set it to -1 because Real
                    // might not have a representation for -infinity. The
                    // type indicates the interval is negative-infinite, so
                    // the -1 is a reminder that overlap[0] is -infinity.
                    result.numIntersections = 1;
                    result.overlap[0] = static_cast<Real>(-1);
                    result.overlap[1] = std::min(a0, a1);
                    result.type = Result::isNegativeInfinite;
                }
            }

            result.intersect = (result.numIntersections > 0);
            return result;
        }

        // Dynamic query. Current time is 0, maxTime > 0 is required.
        Result operator()(Real maxTime, std::array<Real, 2> const& interval0,
            Real speed0, std::array<Real, 2> const& interval1, Real speed1)
        {
            Result result{};
            result.type = Result::isDynamicQuery;

            if (interval0[1] < interval1[0])
            {
                // interval0 initially to the left of interval1.
                Real diffSpeed = speed0 - speed1;
                if (diffSpeed > static_cast<Real>(0))
                {
                    // The intervals must move towards each other. 'intersect'
                    // is true when the intervals will intersect by maxTime.
                    Real diffPos = interval1[0] - interval0[1];
                    result.intersect = (diffPos <= maxTime * diffSpeed);
                    result.numIntersections = 1;
                    result.firstTime = diffPos / diffSpeed;
                    result.lastTime = (interval1[1] - interval0[0]) / diffSpeed;
                    result.overlap[0] = interval0[0] + result.firstTime * speed0;
                    result.overlap[1] = result.overlap[0];
                    return result;
                }
            }
            else if (interval0[0] > interval1[1])
            {
                // interval0 initially to the right of interval1.
                Real diffSpeed = speed1 - speed0;
                if (diffSpeed > static_cast<Real>(0))
                {
                    // The intervals must move towards each other. 'intersect'
                    // is true when the intervals will intersect by maxTime.
                    Real diffPos = interval0[0] - interval1[1];
                    result.intersect = (diffPos <= maxTime * diffSpeed);
                    result.numIntersections = 1;
                    result.firstTime = diffPos / diffSpeed;
                    result.lastTime = (interval0[1] - interval1[0]) / diffSpeed;
                    result.overlap[0] = interval1[1] + result.firstTime * speed1;
                    result.overlap[1] = result.overlap[0];
                    return result;
                }
            }
            else
            {
                // The intervals are initially intersecting.
                result.intersect = true;
                result.firstTime = static_cast<Real>(0);
                if (speed1 > speed0)
                {
                    result.lastTime = (interval0[1] - interval1[0]) / (speed1 - speed0);
                }
                else if (speed1 < speed0)
                {
                    result.lastTime = (interval1[1] - interval0[0]) / (speed0 - speed1);
                }
                else
                {
                    result.lastTime = std::numeric_limits<Real>::max();
                }

                if (interval0[1] > interval1[0])
                {
                    if (interval0[0] < interval1[1])
                    {
                        result.numIntersections = 2;
                        result.overlap[0] = (interval0[0] < interval1[0] ? interval1[0] : interval0[0]);
                        result.overlap[1] = (interval0[1] > interval1[1] ? interval1[1] : interval0[1]);
                    }
                    else  // interval0[0] == interval1[1]
                    {
                        result.numIntersections = 1;
                        result.overlap[0] = interval0[0];
                        result.overlap[1] = result.overlap[0];
                    }
                }
                else  // interval0[1] == interval1[0]
                {
                    result.numIntersections = 1;
                    result.overlap[0] = interval0[1];
                    result.overlap[1] = result.overlap[0];
                }
                return result;
            }

            // The Result constructor sets the correct state for no-intersection.
            return result;
        }
    };

    // Template aliases for convenience.
    template <typename Real>
    using TIIntervalInterval = TIQuery<Real, std::array<Real, 2>, std::array<Real, 2>>;

    template <typename Real>
    using FIIntervalInterval = FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>>;
}
