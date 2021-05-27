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
// Q(t) = (1-t)*Q0 + t*Q1 for 0 <= s <= 1 and 0 <= t <= 1.  The algorithm is
// robust even for nearly parallel segments.  Effectively, it uses a conjugate
// gradient search for the minimum of the squared distance function, which
// avoids the numerical problems introduced by divisions in the case the
// minimum is located at an interior point of the domain.  See the document
//   https://www.geometrictools.com/Documentation/DistanceLine3Line3.pdf
// for details.

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Segment<N, Real>, Segment<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real parameter[2];
            Vector<N, Real> closest[2];
        };

        Result operator()(Segment<N, Real> const& segment0,
            Segment<N, Real> const& segment1)
        {
            return operator()(segment0.p[0], segment0.p[1], segment1.p[0], segment1.p[1]);
        }

        Result operator()(Vector<N, Real> const& P0, Vector<N, Real> const& P1,
            Vector<N, Real> const& Q0, Vector<N, Real> const& Q1)
        {
            Result result;

            // The code allows degenerate line segments; that is, P0 and P1
            // can be the same point or Q0 and Q1 can be the same point.  The
            // quadratic function for squared distance between the segment is
            //   R(s,t) = a*s^2 - 2*b*s*t + c*t^2 + 2*d*s - 2*e*t + f
            // for (s,t) in [0,1]^2 where
            //   a = Dot(P1-P0,P1-P0), b = Dot(P1-P0,Q1-Q0), c = Dot(Q1-Q0,Q1-Q0),
            //   d = Dot(P1-P0,P0-Q0), e = Dot(Q1-Q0,P0-Q0), f = Dot(P0-Q0,P0-Q0)
            Vector<N, Real> P1mP0 = P1 - P0;
            Vector<N, Real> Q1mQ0 = Q1 - Q0;
            Vector<N, Real> P0mQ0 = P0 - Q0;
            mA = Dot(P1mP0, P1mP0);
            mB = Dot(P1mP0, Q1mQ0);
            mC = Dot(Q1mQ0, Q1mQ0);
            mD = Dot(P1mP0, P0mQ0);
            mE = Dot(Q1mQ0, P0mQ0);

            mF00 = mD;
            mF10 = mF00 + mA;
            mF01 = mF00 - mB;
            mF11 = mF10 - mB;

            mG00 = -mE;
            mG10 = mG00 - mB;
            mG01 = mG00 + mC;
            mG11 = mG10 + mC;

            if (mA > (Real)0 && mC > (Real)0)
            {
                // Compute the solutions to dR/ds(s0,0) = 0 and
                // dR/ds(s1,1) = 0.  The location of sI on the s-axis is
                // stored in classifyI (I = 0 or 1).  If sI <= 0, classifyI
                // is -1.  If sI >= 1, classifyI is 1.  If 0 < sI < 1,
                // classifyI is 0.  This information helps determine where to
                // search for the minimum point (s,t).  The fij values are
                // dR/ds(i,j) for i and j in {0,1}.

                Real sValue[2];
                sValue[0] = GetClampedRoot(mA, mF00, mF10);
                sValue[1] = GetClampedRoot(mA, mF01, mF11);

                int classify[2];
                for (int i = 0; i < 2; ++i)
                {
                    if (sValue[i] <= (Real)0)
                    {
                        classify[i] = -1;
                    }
                    else if (sValue[i] >= (Real)1)
                    {
                        classify[i] = +1;
                    }
                    else
                    {
                        classify[i] = 0;
                    }
                }

                if (classify[0] == -1 && classify[1] == -1)
                {
                    // The minimum must occur on s = 0 for 0 <= t <= 1.
                    result.parameter[0] = (Real)0;
                    result.parameter[1] = GetClampedRoot(mC, mG00, mG01);
                }
                else if (classify[0] == +1 && classify[1] == +1)
                {
                    // The minimum must occur on s = 1 for 0 <= t <= 1.
                    result.parameter[0] = (Real)1;
                    result.parameter[1] = GetClampedRoot(mC, mG10, mG11);
                }
                else
                {
                    // The line dR/ds = 0 intersects the domain [0,1]^2 in a
                    // nondegenerate segment.  Compute the endpoints of that
                    // segment, end[0] and end[1].  The edge[i] flag tells you
                    // on which domain edge end[i] lives: 0 (s=0), 1 (s=1),
                    // 2 (t=0), 3 (t=1).
                    int edge[2];
                    Real end[2][2];
                    ComputeIntersection(sValue, classify, edge, end);

                    // The directional derivative of R along the segment of
                    // intersection is
                    //   H(z) = (end[1][1]-end[1][0]) *
                    //          dR/dt((1-z)*end[0] + z*end[1])
                    // for z in [0,1].  The formula uses the fact that
                    // dR/ds = 0 on the segment.  Compute the minimum of
                    // H on [0,1].
                    ComputeMinimumParameters(edge, end, result.parameter);
                }
            }
            else
            {
                if (mA > (Real)0)
                {
                    // The Q-segment is degenerate (Q0 and Q1 are the same
                    // point) and the quadratic is R(s,0) = a*s^2 + 2*d*s + f
                    // and has (half) first derivative F(t) = a*s + d.  The
                    // closest P-point is interior to the P-segment when
                    // F(0) < 0 and F(1) > 0.
                    result.parameter[0] = GetClampedRoot(mA, mF00, mF10);
                    result.parameter[1] = (Real)0;
                }
                else if (mC > (Real)0)
                {
                    // The P-segment is degenerate (P0 and P1 are the same
                    // point) and the quadratic is R(0,t) = c*t^2 - 2*e*t + f
                    // and has (half) first derivative G(t) = c*t - e.  The
                    // closest Q-point is interior to the Q-segment when
                    // G(0) < 0 and G(1) > 0.
                    result.parameter[0] = (Real)0;
                    result.parameter[1] = GetClampedRoot(mC, mG00, mG01);
                }
                else
                {
                    // P-segment and Q-segment are degenerate.
                    result.parameter[0] = (Real)0;
                    result.parameter[1] = (Real)0;
                }
            }


            result.closest[0] =
                ((Real)1 - result.parameter[0]) * P0 + result.parameter[0] * P1;
            result.closest[1] =
                ((Real)1 - result.parameter[1]) * Q0 + result.parameter[1] * Q1;
            Vector<N, Real> diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        // Compute the root of h(z) = h0 + slope*z and clamp it to the interval
        // [0,1].  It is required that for h1 = h(1), either (h0 < 0 and h1 > 0)
        // or (h0 > 0 and h1 < 0).
        Real GetClampedRoot(Real slope, Real h0, Real h1)
        {
            // Theoretically, r is in (0,1).  However, when the slope is
            // nearly zero, then so are h0 and h1.  Significant numerical
            // rounding problems can occur when using floating-point
            // arithmetic.  If the rounding causes r to be outside the
            // interval, clamp it.  It is possible that r is in (0,1) and has
            // rounding errors, but because h0 and h1 are both nearly zero,
            // the quadratic is nearly constant on (0,1).  Any choice of p
            // should not cause undesirable accuracy problems for the final
            // distance computation.
            //
            // NOTE:  You can use bisection to recompute the root or even use
            // bisection to compute the root and skip the division.  This is
            // generally slower, which might be a problem for high-performance
            // applications.

            Real r;
            if (h0 < (Real)0)
            {
                if (h1 > (Real)0)
                {
                    r = -h0 / slope;
                    if (r > (Real)1)
                    {
                        r = (Real)0.5;
                    }
                    // The slope is positive and -h0 is positive, so there is
                    // no need to test for a negative value and clamp it.
                }
                else
                {
                    r = (Real)1;
                }
            }
            else
            {
                r = (Real)0;
            }
            return r;
        }

        // Compute the intersection of the line dR/ds = 0 with the domain
        // [0,1]^2.  The direction of the line dR/ds is conjugate to (1,0),
        // so the algorithm for minimization is effectively the conjugate
        // gradient algorithm for a quadratic function.
        void ComputeIntersection(Real const sValue[2], int const classify[2],
            int edge[2], Real end[2][2])
        {
            // The divisions are theoretically numbers in [0,1].  Numerical
            // rounding errors might cause the result to be outside the
            // interval.  When this happens, it must be that both numerator
            // and denominator are nearly zero.  The denominator is nearly
            // zero when the segments are nearly perpendicular.  The
            // numerator is nearly zero when the P-segment is nearly
            // degenerate (mF00 = a is small).  The choice of 0.5 should not
            // cause significant accuracy problems.
            //
            // NOTE:  You can use bisection to recompute the root or even use
            // bisection to compute the root and skip the division.  This is
            // generally slower, which might be a problem for high-performance
            // applications.

            if (classify[0] < 0)
            {
                edge[0] = 0;
                end[0][0] = (Real)0;
                end[0][1] = mF00 / mB;
                if (end[0][1] < (Real)0 || end[0][1] > (Real)1)
                {
                    end[0][1] = (Real)0.5;
                }

                if (classify[1] == 0)
                {
                    edge[1] = 3;
                    end[1][0] = sValue[1];
                    end[1][1] = (Real)1;
                }
                else  // classify[1] > 0
                {
                    edge[1] = 1;
                    end[1][0] = (Real)1;
                    end[1][1] = mF10 / mB;
                    if (end[1][1] < (Real)0 || end[1][1] > (Real)1)
                    {
                        end[1][1] = (Real)0.5;
                    }
                }
            }
            else if (classify[0] == 0)
            {
                edge[0] = 2;
                end[0][0] = sValue[0];
                end[0][1] = (Real)0;

                if (classify[1] < 0)
                {
                    edge[1] = 0;
                    end[1][0] = (Real)0;
                    end[1][1] = mF00 / mB;
                    if (end[1][1] < (Real)0 || end[1][1] > (Real)1)
                    {
                        end[1][1] = (Real)0.5;
                    }
                }
                else if (classify[1] == 0)
                {
                    edge[1] = 3;
                    end[1][0] = sValue[1];
                    end[1][1] = (Real)1;
                }
                else
                {
                    edge[1] = 1;
                    end[1][0] = (Real)1;
                    end[1][1] = mF10 / mB;
                    if (end[1][1] < (Real)0 || end[1][1] > (Real)1)
                    {
                        end[1][1] = (Real)0.5;
                    }
                }
            }
            else  // classify[0] > 0
            {
                edge[0] = 1;
                end[0][0] = (Real)1;
                end[0][1] = mF10 / mB;
                if (end[0][1] < (Real)0 || end[0][1] > (Real)1)
                {
                    end[0][1] = (Real)0.5;
                }

                if (classify[1] == 0)
                {
                    edge[1] = 3;
                    end[1][0] = sValue[1];
                    end[1][1] = (Real)1;
                }
                else
                {
                    edge[1] = 0;
                    end[1][0] = (Real)0;
                    end[1][1] = mF00 / mB;
                    if (end[1][1] < (Real)0 || end[1][1] > (Real)1)
                    {
                        end[1][1] = (Real)0.5;
                    }
                }
            }
        }

        // Compute the location of the minimum of R on the segment of
        // intersection for the line dR/ds = 0 and the domain [0,1]^2.
        void ComputeMinimumParameters(int const edge[2], Real const end[2][2],
            Real parameter[2])
        {
            Real delta = end[1][1] - end[0][1];
            Real h0 = delta * (-mB * end[0][0] + mC * end[0][1] - mE);
            if (h0 >= (Real)0)
            {
                if (edge[0] == 0)
                {
                    parameter[0] = (Real)0;
                    parameter[1] = GetClampedRoot(mC, mG00, mG01);
                }
                else if (edge[0] == 1)
                {
                    parameter[0] = (Real)1;
                    parameter[1] = GetClampedRoot(mC, mG10, mG11);
                }
                else
                {
                    parameter[0] = end[0][0];
                    parameter[1] = end[0][1];
                }
            }
            else
            {
                Real h1 = delta * (-mB * end[1][0] + mC * end[1][1] - mE);
                if (h1 <= (Real)0)
                {
                    if (edge[1] == 0)
                    {
                        parameter[0] = (Real)0;
                        parameter[1] = GetClampedRoot(mC, mG00, mG01);
                    }
                    else if (edge[1] == 1)
                    {
                        parameter[0] = (Real)1;
                        parameter[1] = GetClampedRoot(mC, mG10, mG11);
                    }
                    else
                    {
                        parameter[0] = end[1][0];
                        parameter[1] = end[1][1];
                    }
                }
                else  // h0 < 0 and h1 > 0
                {
                    Real z = std::min(std::max(h0 / (h0 - h1), (Real)0), (Real)1);
                    Real omz = (Real)1 - z;
                    parameter[0] = omz * end[0][0] + z * end[1][0];
                    parameter[1] = omz * end[0][1] + z * end[1][1];
                }
            }
        }

        // The coefficients of R(s,t), not including the constant term.
        Real mA, mB, mC, mD, mE;

        // dR/ds(i,j) at the four corners of the domain
        Real mF00, mF10, mF01, mF11;

        // dR/dt(i,j) at the four corners of the domain
        Real mG00, mG10, mG01, mG11;
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPSegmentSegment = DCPQuery<Real, Segment<N, Real>, Segment<N, Real>>;

    template <typename Real>
    using DCPSegment2Segment2 = DCPSegmentSegment<2, Real>;

    template <typename Real>
    using DCPSegment3Segment3 = DCPSegmentSegment<3, Real>;
}
