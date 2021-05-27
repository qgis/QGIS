// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Segment.h>

// Compute the closest points on the line segments P(s) = (1-s)*P0 + s*P1 and
// Q(t) = (1-t)*Q0 + t*Q1 for 0 <= s <= 1 and 0 <= t <= 1.  The algorithm
// relies on exact rational arithmetic.  See the document
//   https://www.geometrictools.com/Documentation/DistanceLine3Line3.pdf
// for details.

namespace gte
{
    template <int N, typename Rational>
    class DistanceSegmentSegmentExact
    {
    public:
        struct Result
        {
            Rational sqrDistance;
            Rational parameter[2];
            Vector<N, Rational> closest[2];
        };

        Result operator()(Segment<N, Rational> const& segment0,
            Segment<N, Rational> const& segment1)
        {
            return operator()(segment0.p[0], segment0.p[1], segment1.p[0], segment1.p[1]);
        }

        Result operator()(
            Vector<N, Rational> const& P0, Vector<N, Rational> const& P1,
            Vector<N, Rational> const& Q0, Vector<N, Rational> const& Q1)
        {
            Vector<N, Rational> P1mP0 = P1 - P0;
            Vector<N, Rational> Q1mQ0 = Q1 - Q0;
            Vector<N, Rational> P0mQ0 = P0 - Q0;
            Rational a = Dot(P1mP0, P1mP0);
            Rational b = Dot(P1mP0, Q1mQ0);
            Rational c = Dot(Q1mQ0, Q1mQ0);
            Rational d = Dot(P1mP0, P0mQ0);
            Rational e = Dot(Q1mQ0, P0mQ0);
            Rational const zero = (Rational)0;
            Rational const one = (Rational)1;
            Rational det = a * c - b * b;
            Rational s, t, nd, bmd, bte, ctd, bpe, ate, btd;

            if (det > zero)
            {
                bte = b * e;
                ctd = c * d;
                if (bte <= ctd)  // s <= 0
                {
                    s = zero;
                    if (e <= zero)  // t <= 0
                    {
                        // region 6
                        t = zero;
                        nd = -d;
                        if (nd >= a)
                        {
                            s = one;
                        }
                        else if (nd > zero)
                        {
                            s = nd / a;
                        }
                        // else: s is already zero
                    }
                    else if (e < c)  // 0 < t < 1
                    {
                        // region 5
                        t = e / c;
                    }
                    else  // t >= 1
                    {
                        // region 4
                        t = one;
                        bmd = b - d;
                        if (bmd >= a)
                        {
                            s = one;
                        }
                        else if (bmd > zero)
                        {
                            s = bmd / a;
                        }
                        // else:  s is already zero
                    }
                }
                else  // s > 0
                {
                    s = bte - ctd;
                    if (s >= det)  // s >= 1
                    {
                        // s = 1
                        s = one;
                        bpe = b + e;
                        if (bpe <= zero)  // t <= 0
                        {
                            // region 8
                            t = zero;
                            nd = -d;
                            if (nd <= zero)
                            {
                                s = zero;
                            }
                            else if (nd < a)
                            {
                                s = nd / a;
                            }
                            // else: s is already one
                        }
                        else if (bpe < c)  // 0 < t < 1
                        {
                            // region 1
                            t = bpe / c;
                        }
                        else  // t >= 1
                        {
                            // region 2
                            t = one;
                            bmd = b - d;
                            if (bmd <= zero)
                            {
                                s = zero;
                            }
                            else if (bmd < a)
                            {
                                s = bmd / a;
                            }
                            // else:  s is already one
                        }
                    }
                    else  // 0 < s < 1
                    {
                        ate = a * e;
                        btd = b * d;
                        if (ate <= btd)  // t <= 0
                        {
                            // region 7
                            t = zero;
                            nd = -d;
                            if (nd <= zero)
                            {
                                s = zero;
                            }
                            else if (nd >= a)
                            {
                                s = one;
                            }
                            else
                            {
                                s = nd / a;
                            }
                        }
                        else  // t > 0
                        {
                            t = ate - btd;
                            if (t >= det)  // t >= 1
                            {
                                // region 3
                                t = one;
                                bmd = b - d;
                                if (bmd <= zero)
                                {
                                    s = zero;
                                }
                                else if (bmd >= a)
                                {
                                    s = one;
                                }
                                else
                                {
                                    s = bmd / a;
                                }
                            }
                            else  // 0 < t < 1
                            {
                                // region 0
                                s /= det;
                                t /= det;
                            }
                        }
                    }
                }
            }
            else
            {
                // The segments are parallel.  The quadratic factors to
                //   R(s,t) = a*(s-(b/a)*t)^2 + 2*d*(s - (b/a)*t) + f
                // where a*c = b^2, e = b*d/a, f = |P0-Q0|^2, and b is not
                // zero.  R is constant along lines of the form s-(b/a)*t = k
                // and its occurs on the line a*s - b*t + d = 0.  This line
                // must intersect both the s-axis and the t-axis because 'a'
                // and 'b' are not zero.  Because of parallelism, the line is
                // also represented by -b*s + c*t - e = 0.
                //
                // The code determines an edge of the domain [0,1]^2 that
                // intersects the minimum line, or if none of the edges
                // intersect, it determines the closest corner to the minimum
                // line.  The conditionals are designed to test first for
                // intersection with the t-axis (s = 0) using
                // -b*s + c*t - e = 0 and then with the s-axis (t = 0) using
                // a*s - b*t + d = 0.

                // When s = 0, solve c*t - e = 0 (t = e/c).
                if (e <= zero)  // t <= 0
                {
                    // Now solve a*s - b*t + d = 0 for t = 0 (s = -d/a).
                    t = zero;
                    nd = -d;
                    if (nd <= zero)  // s <= 0
                    {
                        // region 6
                        s = zero;
                    }
                    else if (nd >= a)  // s >= 1
                    {
                        // region 8
                        s = one;
                    }
                    else  // 0 < s < 1
                    {
                        // region 7
                        s = nd / a;
                    }
                }
                else if (e >= c)  // t >= 1
                {
                    // Now solve a*s - b*t + d = 0 for t = 1 (s = (b-d)/a).
                    t = one;
                    bmd = b - d;
                    if (bmd <= zero)  // s <= 0
                    {
                        // region 4
                        s = zero;
                    }
                    else if (bmd >= a)  // s >= 1
                    {
                        // region 2
                        s = one;
                    }
                    else  // 0 < s < 1
                    {
                        // region 3
                        s = bmd / a;
                    }
                }
                else  // 0 < t < 1
                {
                    // The point (0,e/c) is on the line and domain, so we have
                    // one point at which R is a minimum.
                    s = zero;
                    t = e / c;
                }
            }

            Result result;
            result.parameter[0] = s;
            result.parameter[1] = t;
            result.closest[0] = P0 + s * P1mP0;
            result.closest[1] = Q0 + t * Q1mQ0;
            Vector<N, Rational> diff = result.closest[1] - result.closest[0];
            result.sqrDistance = Dot(diff, diff);
            return result;
        }
    };
}
