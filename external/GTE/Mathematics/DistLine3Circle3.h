// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.05.30

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Line.h>
#include <Mathematics/RootsBisection.h>
#include <Mathematics/RootsPolynomial.h>

// The 3D line-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Line3<Real>, Circle3<Real>>
    {
    public:
        // The possible number of closest line-circle pairs is 1, 2 or all
        // circle points.  If 1 or 2, numClosestPairs is set to this number
        // and 'equidistant' is false; the number of valid elements in
        // lineClosest[] and circleClosest[] is numClosestPairs.  If all
        // circle points are closest, the line must be C+t*N where C is the
        // circle center, N is the normal to the plane of the circle, and
        // lineClosest[0] is set to C.  In this case, 'equidistant' is true
        // and circleClosest[0] is set to C+r*U, where r is the circle
        // and U is a vector perpendicular to N.
        struct Result
        {
            Real distance, sqrDistance;
            int numClosestPairs;
            Vector3<Real> lineClosest[2], circleClosest[2];
            bool equidistant;
        };

        // The polynomial-based algorithm.  Type Real can be floating-point or
        // rational.
        Result operator()(Line3<Real> const& line, Circle3<Real> const& circle)
        {
            Result result;
            Vector3<Real> const vzero = Vector3<Real>::Zero();
            Real const zero = (Real)0;

            Vector3<Real> D = line.origin - circle.center;
            Vector3<Real> NxM = Cross(circle.normal, line.direction);
            Vector3<Real> NxD = Cross(circle.normal, D);
            Real t;

            if (NxM != vzero)
            {
                if (NxD != vzero)
                {
                    Real NdM = Dot(circle.normal, line.direction);
                    if (NdM != zero)
                    {
                        // H(t) = (a*t^2 + 2*b*t + c)*(t + d)^2
                        //        - r^2*(a*t + b)^2
                        //      = h0 + h1*t + h2*t^2 + h3*t^3 + h4*t^4
                        Real a = Dot(NxM, NxM), b = Dot(NxM, NxD);
                        Real c = Dot(NxD, NxD), d = Dot(line.direction, D);
                        Real rsqr = circle.radius * circle.radius;
                        Real asqr = a * a, bsqr = b * b, dsqr = d * d;
                        Real h0 = c * dsqr - bsqr * rsqr;
                        Real h1 = (Real)2 * (c * d + b * dsqr - a * b * rsqr);
                        Real h2 = c + (Real)4 * b * d + a * dsqr - asqr * rsqr;
                        Real h3 = (Real)2 * (b + a * d);
                        Real h4 = a;

                        std::map<Real, int> rmMap;
                        RootsPolynomial<Real>::template SolveQuartic<Real>(
                            h0, h1, h2, h3, h4, rmMap);
                        std::array<ClosestInfo, 4> candidates;
                        int numRoots = 0;
                        for (auto const& rm : rmMap)
                        {
                            t = rm.first;
                            ClosestInfo info;
                            Vector3<Real> NxDelta = NxD + t * NxM;
                            if (NxDelta != vzero)
                            {
                                GetPair(line, circle, D, t, info.lineClosest,
                                    info.circleClosest);
                                info.equidistant = false;
                            }
                            else
                            {
                                Vector3<Real> U = GetOrthogonal(circle.normal, true);
                                info.lineClosest = circle.center;
                                info.circleClosest =
                                    circle.center + circle.radius * U;
                                info.equidistant = true;
                            }
                            Vector3<Real> diff = info.lineClosest - info.circleClosest;
                            info.sqrDistance = Dot(diff, diff);
                            candidates[numRoots++] = info;
                        }

                        std::sort(candidates.begin(), candidates.begin() + numRoots);

                        result.numClosestPairs = 1;
                        result.lineClosest[0] = candidates[0].lineClosest;
                        result.circleClosest[0] = candidates[0].circleClosest;
                        if (numRoots > 1
                            && candidates[1].sqrDistance == candidates[0].sqrDistance)
                        {
                            result.numClosestPairs = 2;
                            result.lineClosest[1] = candidates[1].lineClosest;
                            result.circleClosest[1] = candidates[1].circleClosest;
                        }
                    }
                    else
                    {
                        // The line is parallel to the plane of the circle.
                        // The polynomial has the form
                        // H(t) = (t+v)^2*[(t+v)^2-(r^2-u^2)].
                        Real u = Dot(NxM, D), v = Dot(line.direction, D);
                        Real discr = circle.radius * circle.radius - u * u;
                        if (discr > zero)
                        {
                            result.numClosestPairs = 2;
                            Real rootDiscr = std::sqrt(discr);
                            t = -v + rootDiscr;
                            GetPair(line, circle, D, t, result.lineClosest[0],
                                result.circleClosest[0]);
                            t = -v - rootDiscr;
                            GetPair(line, circle, D, t, result.lineClosest[1],
                                result.circleClosest[1]);
                        }
                        else
                        {
                            result.numClosestPairs = 1;
                            t = -v;
                            GetPair(line, circle, D, t, result.lineClosest[0],
                                result.circleClosest[0]);
                        }
                    }
                }
                else
                {
                    // The line is C+t*M, where M is not parallel to N.  The
                    // polynomial is
                    // H(t) = |Cross(N,M)|^2*t^2*(t^2 - r^2*|Cross(N,M)|^2)
                    // where root t = 0 does not correspond to the global
                    // minimum.  The other roots produce the global minimum.
                    result.numClosestPairs = 2;
                    t = circle.radius * Length(NxM);
                    GetPair(line, circle, D, t, result.lineClosest[0],
                        result.circleClosest[0]);
                    t = -t;
                    GetPair(line, circle, D, t, result.lineClosest[1],
                        result.circleClosest[1]);
                }
                result.equidistant = false;
            }
            else
            {
                if (NxD != vzero)
                {
                    // The line is A+t*N (perpendicular to plane) but with
                    // A != C.  The polyhomial is
                    // H(t) = |Cross(N,D)|^2*(t + Dot(M,D))^2.
                    result.numClosestPairs = 1;
                    t = -Dot(line.direction, D);
                    GetPair(line, circle, D, t, result.lineClosest[0],
                        result.circleClosest[0]);
                    result.equidistant = false;
                }
                else
                {
                    // The line is C+t*N, so C is the closest point for the
                    // line and all circle points are equidistant from it.
                    Vector3<Real> U = GetOrthogonal(circle.normal, true);
                    result.numClosestPairs = 1;
                    result.lineClosest[0] = circle.center;
                    result.circleClosest[0] = circle.center + circle.radius * U;
                    result.equidistant = true;
                }
            }

            Vector3<Real> diff = result.lineClosest[0] - result.circleClosest[0];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

        // The nonpolynomial-based algorithm that uses bisection.  Because the
        // bisection is iterative, you should choose Real to be a
        // floating-point type.  However, the algorithm will still work for a
        // rational type, but it is costly because of the increase in
        // arbitrary-size integers used during the bisection.
        Result Robust(Line3<Real> const& line, Circle3<Real> const& circle)
        {
            // The line is P(t) = B+t*M.  The circle is |X-C| = r with
            // Dot(N,X-C)=0.
            Result result;
            Vector3<Real> vzero = Vector3<Real>::Zero();
            Real const zero = (Real)0;

            Vector3<Real> D = line.origin - circle.center;
            Vector3<Real> MxN = Cross(line.direction, circle.normal);
            Vector3<Real> DxN = Cross(D, circle.normal);

            Real m0sqr = Dot(MxN, MxN);
            if (m0sqr > zero)
            {
                // Compute the critical points s for F'(s) = 0.
                Real s, t;
                int numRoots = 0;
                std::array<Real, 3> roots;

                // The line direction M and the plane normal N are not
                // parallel.  Move the line origin B = (b0,b1,b2) to
                // B' = B + lambda*line.direction = (0,b1',b2').
                Real m0 = std::sqrt(m0sqr);
                Real rm0 = circle.radius * m0;
                Real lambda = -Dot(MxN, DxN) / m0sqr;
                Vector3<Real> oldD = D;
                D += lambda * line.direction;
                DxN += lambda * MxN;
                Real m2b2 = Dot(line.direction, D);
                Real b1sqr = Dot(DxN, DxN);
                if (b1sqr > zero)
                {
                    // B' = (0,b1',b2') where b1' != 0.  See Sections 1.1.2
                    // and 1.2.2 of the PDF documentation.
                    Real b1 = std::sqrt(b1sqr);
                    Real rm0sqr = circle.radius * m0sqr;
                    if (rm0sqr > b1)
                    {
                        Real const twoThirds = (Real)2 / (Real)3;
                        Real sHat = std::sqrt(std::pow(rm0sqr * b1sqr, twoThirds) - b1sqr) / m0;
                        Real gHat = rm0sqr * sHat / std::sqrt(m0sqr * sHat * sHat + b1sqr);
                        Real cutoff = gHat - sHat;
                        if (m2b2 <= -cutoff)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2, -m2b2 + rm0);
                            roots[numRoots++] = s;
                            if (m2b2 == -cutoff)
                            {
                                roots[numRoots++] = -sHat;
                            }
                        }
                        else if (m2b2 >= cutoff)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -m2b2);
                            roots[numRoots++] = s;
                            if (m2b2 == cutoff)
                            {
                                roots[numRoots++] = sHat;
                            }
                        }
                        else
                        {
                            if (m2b2 <= zero)
                            {
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2, -m2b2 + rm0);
                                roots[numRoots++] = s;
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -sHat);
                                roots[numRoots++] = s;
                            }
                            else
                            {
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -m2b2);
                                roots[numRoots++] = s;
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, sHat, -m2b2 + rm0);
                                roots[numRoots++] = s;
                            }
                        }
                    }
                    else
                    {
                        if (m2b2 < zero)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2, -m2b2 + rm0);
                        }
                        else if (m2b2 > zero)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -m2b2);
                        }
                        else
                        {
                            s = zero;
                        }
                        roots[numRoots++] = s;
                    }
                }
                else
                {
                    // The new line origin is B' = (0,0,b2').
                    if (m2b2 < zero)
                    {
                        s = -m2b2 + rm0;
                        roots[numRoots++] = s;
                    }
                    else if (m2b2 > zero)
                    {
                        s = -m2b2 - rm0;
                        roots[numRoots++] = s;
                    }
                    else
                    {
                        s = -m2b2 + rm0;
                        roots[numRoots++] = s;
                        s = -m2b2 - rm0;
                        roots[numRoots++] = s;
                    }
                }

                std::array<ClosestInfo, 4> candidates;
                for (int i = 0; i < numRoots; ++i)
                {
                    t = roots[i] + lambda;
                    ClosestInfo info;
                    Vector3<Real> NxDelta =
                        Cross(circle.normal, oldD + t * line.direction);
                    if (NxDelta != vzero)
                    {
                        GetPair(line, circle, oldD, t, info.lineClosest,
                            info.circleClosest);
                        info.equidistant = false;
                    }
                    else
                    {
                        Vector3<Real> U = GetOrthogonal(circle.normal, true);
                        info.lineClosest = circle.center;
                        info.circleClosest = circle.center + circle.radius * U;
                        info.equidistant = true;
                    }
                    Vector3<Real> diff = info.lineClosest - info.circleClosest;
                    info.sqrDistance = Dot(diff, diff);
                    candidates[i] = info;
                }

                std::sort(candidates.begin(), candidates.begin() + numRoots);

                result.numClosestPairs = 1;
                result.lineClosest[0] = candidates[0].lineClosest;
                result.circleClosest[0] = candidates[0].circleClosest;
                if (numRoots > 1
                    && candidates[1].sqrDistance == candidates[0].sqrDistance)
                {
                    result.numClosestPairs = 2;
                    result.lineClosest[1] = candidates[1].lineClosest;
                    result.circleClosest[1] = candidates[1].circleClosest;
                }

                result.equidistant = false;
            }
            else
            {
                // The line direction and the plane normal are parallel.
                if (DxN != vzero)
                {
                    // The line is A+t*N but with A != C.
                    result.numClosestPairs = 1;
                    GetPair(line, circle, D, -Dot(line.direction, D),
                        result.lineClosest[0], result.circleClosest[0]);
                    result.equidistant = false;
                }
                else
                {
                    // The line is C+t*N, so C is the closest point for the
                    // line and all circle points are equidistant from it.
                    Vector3<Real> U = GetOrthogonal(circle.normal, true);
                    result.numClosestPairs = 1;
                    result.lineClosest[0] = circle.center;
                    result.circleClosest[0] = circle.center + circle.radius * U;
                    result.equidistant = true;
                }
            }

            Vector3<Real> diff = result.lineClosest[0] - result.circleClosest[0];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        // Support for operator(...).
        struct ClosestInfo
        {
            Real sqrDistance;
            Vector3<Real> lineClosest, circleClosest;
            bool equidistant;

            bool operator< (ClosestInfo const& info) const
            {
                return sqrDistance < info.sqrDistance;
            }
        };

        void GetPair(Line3<Real> const& line, Circle3<Real> const& circle,
            Vector3<Real> const& D, Real t, Vector3<Real>& lineClosest,
            Vector3<Real>& circleClosest)
        {
            Vector3<Real> delta = D + t * line.direction;
            lineClosest = circle.center + delta;
            delta -= Dot(circle.normal, delta) * circle.normal;
            Normalize(delta);
            circleClosest = circle.center + circle.radius * delta;
        }

        // Support for Robust(...).  Bisect the function
        //   F(s) = s + m2b2 - r*m0sqr*s/sqrt(m0sqr*s*s + b1sqr)
        // on the specified interval [smin,smax].
        Real Bisect(Real m2b2, Real rm0sqr, Real m0sqr, Real b1sqr, Real smin, Real smax)
        {
            std::function<Real(Real)> G = [&, m2b2, rm0sqr, m0sqr, b1sqr](Real s)
            {
                return s + m2b2 - rm0sqr * s / std::sqrt(m0sqr * s * s + b1sqr);
            };

            // The function is known to be increasing, so we can specify -1 and +1
            // as the function values at the bounding interval endpoints.  The use
            // of 'double' is intentional in case Real is a BSNumber or BSRational
            // type.  We want the bisections to terminate in a reasonable amount of
            // time.
            unsigned int const maxIterations = GTE_C_MAX_BISECTIONS_GENERIC;
            Real root;
            RootsBisection<Real>::Find(G, smin, smax, (Real)-1, (Real)+1, maxIterations, root);
            return root;
        }
    };
}
