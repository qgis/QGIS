// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Polynomial1.h>
#include <Mathematics/RootsPolynomial.h>
#include <set>

// The 3D circle-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Circle3<Real>, Circle3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            int numClosestPairs;
            Vector3<Real> circle0Closest[2], circle1Closest[2];
            bool equidistant;
        };

        Result operator()(Circle3<Real> const& circle0, Circle3<Real> const& circle1)
        {
            Result result;
            Vector3<Real> const vzero = Vector3<Real>::Zero();
            Real const zero = (Real)0;

            Vector3<Real> N0 = circle0.normal, N1 = circle1.normal;
            Real r0 = circle0.radius, r1 = circle1.radius;
            Vector3<Real> D = circle1.center - circle0.center;
            Vector3<Real> N0xN1 = Cross(N0, N1);

            if (N0xN1 != vzero)
            {
                // Get parameters for constructing the degree-8 polynomial phi.
                Real const one = (Real)1, two = (Real)2;
                Real r0sqr = r0 * r0, r1sqr = r1 * r1;

                // Compute U1 and V1 for the plane of circle1.
                Vector3<Real> basis[3];
                basis[0] = circle1.normal;
                ComputeOrthogonalComplement(1, basis);
                Vector3<Real> U1 = basis[1], V1 = basis[2];

                // Construct the polynomial phi(cos(theta)).
                Vector3<Real> N0xD = Cross(N0, D);
                Vector3<Real> N0xU1 = Cross(N0, U1), N0xV1 = Cross(N0, V1);
                Real a0 = r1 * Dot(D, U1), a1 = r1 * Dot(D, V1);
                Real a2 = Dot(N0xD, N0xD), a3 = r1 * Dot(N0xD, N0xU1);
                Real a4 = r1 * Dot(N0xD, N0xV1), a5 = r1sqr * Dot(N0xU1, N0xU1);
                Real a6 = r1sqr * Dot(N0xU1, N0xV1), a7 = r1sqr * Dot(N0xV1, N0xV1);
                Polynomial1<Real> p0{ a2 + a7, two * a3, a5 - a7 };
                Polynomial1<Real> p1{ two * a4, two * a6 };
                Polynomial1<Real> p2{ zero, a1 };
                Polynomial1<Real> p3{ -a0 };
                Polynomial1<Real> p4{ -a6, a4, two * a6 };
                Polynomial1<Real> p5{ -a3, a7 - a5 };
                Polynomial1<Real> tmp0{ one, zero, -one };
                Polynomial1<Real> tmp1 = p2 * p2 + tmp0 * p3 * p3;
                Polynomial1<Real> tmp2 = two * p2 * p3;
                Polynomial1<Real> tmp3 = p4 * p4 + tmp0 * p5 * p5;
                Polynomial1<Real> tmp4 = two * p4 * p5;
                Polynomial1<Real> p6 = p0 * tmp1 + tmp0 * p1 * tmp2 - r0sqr * tmp3;
                Polynomial1<Real> p7 = p0 * tmp2 + p1 * tmp1 - r0sqr * tmp4;

                // The use of 'double' is intentional in case Real is a BSNumber or
                // BSRational type.  We want the bisections to terminate in a
                // reasonable amount of time.
                //unsigned int const maxIterations = GTE_C_MAX_BISECTIONS_GENERIC;
                unsigned int const maxIterations = 128;
                Real roots[8], sn, temp;
                int i, degree, numRoots;

                // The RootsPolynomial<Real>::Find(...) function currently does not
                // combine duplicate roots.  We need only the unique ones here.
                std::set<Real> uniqueRoots;

                std::array<std::pair<Real, Real>, 16> pairs;
                int numPairs = 0;
                if (p7.GetDegree() > 0 || p7[0] != zero)
                {
                    // H(cs,sn) = p6(cs) + sn * p7(cs)
                    Polynomial1<Real> phi = p6 * p6 - tmp0 * p7 * p7;
                    degree = static_cast<int>(phi.GetDegree());
                    LogAssert(degree > 0, "Unexpected degree for phi.");
                    numRoots = RootsPolynomial<Real>::Find(degree, &phi[0], maxIterations, roots);
                    for (i = 0; i < numRoots; ++i)
                    {
                        uniqueRoots.insert(roots[i]);
                    }

                    for (auto cs : uniqueRoots)
                    {
                        if (std::fabs(cs) <= one)
                        {
                            temp = p7(cs);
                            if (temp != zero)
                            {
                                sn = -p6(cs) / temp;
                                pairs[numPairs++] = std::make_pair(cs, sn);
                            }
                            else
                            {
                                temp = std::max(one - cs * cs, zero);
                                sn = std::sqrt(temp);
                                pairs[numPairs++] = std::make_pair(cs, sn);
                                if (sn != zero)
                                {
                                    pairs[numPairs++] = std::make_pair(cs, -sn);
                                }
                            }
                        }
                    }
                }
                else
                {
                    // H(cs,sn) = p6(cs)
                    degree = static_cast<int>(p6.GetDegree());
                    LogAssert(degree > 0, "Unexpected degree for p6.");
                    numRoots = RootsPolynomial<Real>::Find(degree, &p6[0], maxIterations, roots);
                    for (i = 0; i < numRoots; ++i)
                    {
                        uniqueRoots.insert(roots[i]);
                    }

                    for (auto cs : uniqueRoots)
                    {
                        if (std::fabs(cs) <= one)
                        {
                            temp = std::max(one - cs * cs, zero);
                            sn = std::sqrt(temp);
                            pairs[numPairs++] = std::make_pair(cs, sn);
                            if (sn != zero)
                            {
                                pairs[numPairs++] = std::make_pair(cs, -sn);
                            }
                        }
                    }
                }

                std::array<ClosestInfo, 16> candidates;
                for (i = 0; i < numPairs; ++i)
                {
                    ClosestInfo& info = candidates[i];
                    Vector3<Real> delta =
                        D + r1 * (pairs[i].first * U1 + pairs[i].second * V1);
                    info.circle1Closest = circle0.center + delta;
                    Real N0dDelta = Dot(N0, delta);
                    Real lenN0xDelta = Length(Cross(N0, delta));
                    if (lenN0xDelta > (Real)0)
                    {
                        Real diff = lenN0xDelta - r0;
                        info.sqrDistance = N0dDelta * N0dDelta + diff * diff;
                        delta -= N0dDelta * circle0.normal;
                        Normalize(delta);
                        info.circle0Closest = circle0.center + r0 * delta;
                        info.equidistant = false;
                    }
                    else
                    {
                        Vector3<Real> r0U0 = r0 * GetOrthogonal(N0, true);
                        Vector3<Real> diff = delta - r0U0;
                        info.sqrDistance = Dot(diff, diff);
                        info.circle0Closest = circle0.center + r0U0;
                        info.equidistant = true;
                    }
                }

                std::sort(candidates.begin(), candidates.begin() + numPairs);

                result.numClosestPairs = 1;
                result.sqrDistance = candidates[0].sqrDistance;
                result.circle0Closest[0] = candidates[0].circle0Closest;
                result.circle1Closest[0] = candidates[0].circle1Closest;
                result.equidistant = candidates[0].equidistant;
                if (numRoots > 1
                    && candidates[1].sqrDistance == candidates[0].sqrDistance)
                {
                    result.numClosestPairs = 2;
                    result.circle0Closest[1] = candidates[1].circle0Closest;
                    result.circle1Closest[1] = candidates[1].circle1Closest;
                }
            }
            else
            {
                // The planes of the circles are parallel.  Whether the planes
                // are the same or different, the problem reduces to
                // determining how two circles in the same plane are
                // separated, tangent with one circle outside the other,
                // overlapping, or one circle contained inside the other
                // circle.
                DoQueryParallelPlanes(circle0, circle1, D, result);
            }

            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        class SCPolynomial
        {
        public:
            SCPolynomial()
            {
            }

            SCPolynomial(Real oneTerm, Real cosTerm, Real sinTerm)
            {
                mPoly[0] = Polynomial1<Real>{ oneTerm, cosTerm };
                mPoly[1] = Polynomial1<Real>{ sinTerm };
            }

            inline Polynomial1<Real> const& operator[] (unsigned int i) const
            {
                return mPoly[i];
            }

            inline Polynomial1<Real>& operator[] (unsigned int i)
            {
                return mPoly[i];
            }

            SCPolynomial operator+(SCPolynomial const& object) const
            {
                SCPolynomial result;
                result.mPoly[0] = mPoly[0] + object.mPoly[0];
                result.mPoly[1] = mPoly[1] + object.mPoly[1];
                return result;
            }

            SCPolynomial operator-(SCPolynomial const& object) const
            {
                SCPolynomial result;
                result.mPoly[0] = mPoly[0] - object.mPoly[0];
                result.mPoly[1] = mPoly[1] - object.mPoly[1];
                return result;
            }

            SCPolynomial operator*(SCPolynomial const& object) const
            {
                // 1 - c^2
                Polynomial1<Real> omcsqr{ (Real)1, (Real)0, (Real)-1 };
                SCPolynomial result;
                result.mPoly[0] = mPoly[0] * object.mPoly[0] + omcsqr * mPoly[1] * object.mPoly[1];
                result.mPoly[1] = mPoly[0] * object.mPoly[1] + mPoly[1] * object.mPoly[0];
                return result;
            }

            SCPolynomial operator*(Real scalar) const
            {
                SCPolynomial result;
                result.mPoly[0] = scalar * mPoly[0];
                result.mPoly[1] = scalar * mPoly[1];
                return result;
            }

        private:
            // poly0(c) + s * poly1(c)
            Polynomial1<Real> mPoly[2];
        };

        struct ClosestInfo
        {
            Real sqrDistance;
            Vector3<Real> circle0Closest, circle1Closest;
            bool equidistant;

            inline bool operator< (ClosestInfo const& info) const
            {
                return sqrDistance < info.sqrDistance;
            }
        };

        // The two circles are in parallel planes where D = C1 - C0, the
        // difference of circle centers.
        void DoQueryParallelPlanes(Circle3<Real> const& circle0,
            Circle3<Real> const& circle1, Vector3<Real> const& D, Result& result)
        {
            Real N0dD = Dot(circle0.normal, D);
            Vector3<Real> normProj = N0dD * circle0.normal;
            Vector3<Real> compProj = D - normProj;
            Vector3<Real> U = compProj;
            Real d = Normalize(U);

            // The configuration is determined by the relative location of the
            // intervals of projection of the circles on to the D-line.
            // Circle0 projects to [-r0,r0] and circle1 projects to
            // [d-r1,d+r1].
            Real r0 = circle0.radius, r1 = circle1.radius;
            Real dmr1 = d - r1;
            Real distance;
            if (dmr1 >= r0)  // d >= r0 + r1
            {
                // The circles are separated (d > r0 + r1) or tangent with one
                // outside the other (d = r0 + r1).
                distance = dmr1 - r0;
                result.numClosestPairs = 1;
                result.circle0Closest[0] = circle0.center + r0 * U;
                result.circle1Closest[0] = circle1.center - r1 * U;
                result.equidistant = false;
            }
            else // d < r0 + r1
            {
                // The cases implicitly use the knowledge that d >= 0.
                Real dpr1 = d + r1;
                if (dpr1 <= r0)
                {
                    // Circle1 is inside circle0.
                    distance = r0 - dpr1;
                    result.numClosestPairs = 1;
                    if (d > (Real)0)
                    {
                        result.circle0Closest[0] = circle0.center + r0 * U;
                        result.circle1Closest[0] = circle1.center + r1 * U;
                        result.equidistant = false;
                    }
                    else
                    {
                        // The circles are concentric, so U = (0,0,0).
                        // Construct a vector perpendicular to N0 to use for
                        // closest points.
                        U = GetOrthogonal(circle0.normal, true);
                        result.circle0Closest[0] = circle0.center + r0 * U;
                        result.circle1Closest[0] = circle1.center + r1 * U;
                        result.equidistant = true;
                    }
                }
                else if (dmr1 <= -r0)
                {
                    // Circle0 is inside circle1.
                    distance = -r0 - dmr1;
                    result.numClosestPairs = 1;
                    if (d > (Real)0)
                    {
                        result.circle0Closest[0] = circle0.center - r0 * U;
                        result.circle1Closest[0] = circle1.center - r1 * U;
                        result.equidistant = false;
                    }
                    else
                    {
                        // The circles are concentric, so U = (0,0,0).
                        // Construct a vector perpendicular to N0 to use for
                        // closest points.
                        U = GetOrthogonal(circle0.normal, true);
                        result.circle0Closest[0] = circle0.center + r0 * U;
                        result.circle1Closest[0] = circle1.center + r1 * U;
                        result.equidistant = true;
                    }
                }
                else
                {
                    // The circles are overlapping.  The two points of
                    // intersection are C0 + s*(C1-C0) +/- h*Cross(N,U), where
                    // s = (1 + (r0^2 - r1^2)/d^2)/2 and
                    // h = sqrt(r0^2 - s^2 * d^2).
                    Real r0sqr = r0 * r0, r1sqr = r1 * r1, dsqr = d * d;
                    Real s = ((Real)1 + (r0sqr - r1sqr) / dsqr) / (Real)2;
                    Real arg = std::max(r0sqr - dsqr * s * s, (Real)0);
                    Real h = std::sqrt(arg);
                    Vector3<Real> midpoint = circle0.center + s * compProj;
                    Vector3<Real> hNxU = h * Cross(circle0.normal, U);
                    distance = (Real)0;
                    result.numClosestPairs = 2;
                    result.circle0Closest[0] = midpoint + hNxU;
                    result.circle0Closest[1] = midpoint - hNxU;
                    result.circle1Closest[0] = result.circle0Closest[0] + normProj;
                    result.circle1Closest[1] = result.circle0Closest[1] + normProj;
                    result.equidistant = false;
                }
            }

            result.sqrDistance = distance * distance + N0dD * N0dD;
        }
    };
}
