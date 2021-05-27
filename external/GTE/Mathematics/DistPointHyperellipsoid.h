// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.29

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Vector.h>

// Compute the distance from a point to a hyperellipsoid.  In 2D, this is a
// point-ellipse distance query.  In 3D, this is a point-ellipsoid distance
// query.  The following document describes the algorithm.
//   https://www.geometrictools.com/Documentation/DistancePointEllipseEllipsoid.pdf
// The hyperellipsoid can have arbitrary center and orientation; that is, it
// does not have to be axis-aligned with center at the origin.
//
// For the 2D query,
//   Vector2<Real> point;  // initialized to something
//   Ellipse2<Real> ellipse;  // initialized to something
//   DCPPoint2Ellipse2<Real> query;
//   auto result = query(point, ellipse);
//   Real distance = result.distance;
//   Vector2<Real> closestEllipsePoint = result.closest;
//
// For the 3D query,
//   Vector3<Real> point;  // initialized to something
//   Ellipsoid3<Real> ellipsoid;  // initialized to something
//   DCPPoint3Ellipsoid3<Real> query;
//   auto result = query(point, ellipsoid);
//   Real distance = result.distance;
//   Vector3<Real> closestEllipsoidPoint = result.closest;

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Vector<N, Real>, Hyperellipsoid<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Vector<N, Real> closest;
        };

        // The query for any hyperellipsoid.
        Result operator()(Vector<N, Real> const& point,
            Hyperellipsoid<N, Real> const& hyperellipsoid)
        {
            Result result;

            // Compute the coordinates of Y in the hyperellipsoid coordinate
            // system.
            Vector<N, Real> diff = point - hyperellipsoid.center;
            Vector<N, Real> y;
            for (int i = 0; i < N; ++i)
            {
                y[i] = Dot(diff, hyperellipsoid.axis[i]);
            }

            // Compute the closest hyperellipsoid point in the axis-aligned
            // coordinate system.
            Vector<N, Real> x;
            result.sqrDistance = SqrDistance(hyperellipsoid.extent, y, x);
            result.distance = std::sqrt(result.sqrDistance);

            // Convert back to the original coordinate system.
            result.closest = hyperellipsoid.center;
            for (int i = 0; i < N; ++i)
            {
                result.closest += x[i] * hyperellipsoid.axis[i];
            }

            return result;
        }

        // The 'hyperellipsoid' is assumed to be axis-aligned and centered at the
        // origin , so only the extent[] values are used.
        Result operator()(Vector<N, Real> const& point, Vector<N, Real> const& extent)
        {
            Result result;
            result.sqrDistance = SqrDistance(extent, point, result.closest);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        // The hyperellipsoid is sum_{d=0}^{N-1} (x[d]/e[d])^2 = 1 with no
        // constraints on the orderind of the e[d].  The query point is
        // (y[0],...,y[N-1]) with no constraints on the signs of the components.
        // The function returns the squared distance from the query point to the
        // hyperellipsoid.   It also computes the hyperellipsoid point
        // (x[0],...,x[N-1]) that is closest to (y[0],...,y[N-1]).
        Real SqrDistance(Vector<N, Real> const& e,
            Vector<N, Real> const& y, Vector<N, Real>& x)
        {
            // Determine negations for y to the first octant.
            std::array<bool, N> negate;
            int i, j;
            for (i = 0; i < N; ++i)
            {
                negate[i] = (y[i] < (Real)0);
            }

            // Determine the axis order for decreasing extents.
            std::array<std::pair<Real, int>, N> permute;
            for (i = 0; i < N; ++i)
            {
                permute[i].first = -e[i];
                permute[i].second = i;
            }
            std::sort(permute.begin(), permute.end());

            std::array<int, N> invPermute;
            for (i = 0; i < N; ++i)
            {
                invPermute[permute[i].second] = i;
            }

            Vector<N, Real> locE, locY;
            for (i = 0; i < N; ++i)
            {
                j = permute[i].second;
                locE[i] = e[j];
                locY[i] = std::fabs(y[j]);
            }

            Vector<N, Real> locX;
            Real sqrDistance = SqrDistanceSpecial(locE, locY, locX);

            // Restore the axis order and reflections.
            for (i = 0; i < N; ++i)
            {
                j = invPermute[i];
                if (negate[i])
                {
                    locX[j] = -locX[j];
                }
                x[i] = locX[j];
            }

            return sqrDistance;
        }

        // The hyperellipsoid is sum_{d=0}^{N-1} (x[d]/e[d])^2 = 1 with the e[d]
        // positive and nonincreasing:  e[d] >= e[d + 1] for all d.  The query
        // point is (y[0],...,y[N-1]) with y[d] >= 0 for all d.  The function
        // returns the squared distance from the query point to the
        // hyperellipsoid.  It also computes the hyperellipsoid point
        // (x[0],...,x[N-1]) that is closest to (y[0],...,y[N-1]), where
        // x[d] >= 0 for all d.
        Real SqrDistanceSpecial(Vector<N, Real> const& e,
            Vector<N, Real> const& y, Vector<N, Real>& x)
        {
            Real sqrDistance = (Real)0;

            Vector<N, Real> ePos, yPos, xPos;
            int numPos = 0;
            int i;
            for (i = 0; i < N; ++i)
            {
                if (y[i] > (Real)0)
                {
                    ePos[numPos] = e[i];
                    yPos[numPos] = y[i];
                    ++numPos;
                }
                else
                {
                    x[i] = (Real)0;
                }
            }

            if (y[N - 1] > (Real)0)
            {
                sqrDistance = Bisector(numPos, ePos, yPos, xPos);
            }
            else  // y[N-1] = 0
            {
                Vector<N - 1, Real> numer, denom;
                Real eNm1Sqr = e[N - 1] * e[N - 1];
                for (i = 0; i < numPos; ++i)
                {
                    numer[i] = ePos[i] * yPos[i];
                    denom[i] = ePos[i] * ePos[i] - eNm1Sqr;
                }

                bool inSubHyperbox = true;
                for (i = 0; i < numPos; ++i)
                {
                    if (numer[i] >= denom[i])
                    {
                        inSubHyperbox = false;
                        break;
                    }
                }

                bool inSubHyperellipsoid = false;
                if (inSubHyperbox)
                {
                    // yPos[] is inside the axis-aligned bounding box of the
                    // subhyperellipsoid.  This intermediate test is designed
                    // to guard against the division by zero when
                    // ePos[i] == e[N-1] for some i.
                    Vector<N - 1, Real> xde;
                    Real discr = (Real)1;
                    for (i = 0; i < numPos; ++i)
                    {
                        xde[i] = numer[i] / denom[i];
                        discr -= xde[i] * xde[i];
                    }
                    if (discr > (Real)0)
                    {
                        // yPos[] is inside the subhyperellipsoid.  The
                        // closest hyperellipsoid point has x[N-1] > 0.
                        sqrDistance = (Real)0;
                        for (i = 0; i < numPos; ++i)
                        {
                            xPos[i] = ePos[i] * xde[i];
                            Real diff = xPos[i] - yPos[i];
                            sqrDistance += diff * diff;
                        }
                        x[N - 1] = e[N - 1] * std::sqrt(discr);
                        sqrDistance += x[N - 1] * x[N - 1];
                        inSubHyperellipsoid = true;
                    }
                }

                if (!inSubHyperellipsoid)
                {
                    // yPos[] is outside the subhyperellipsoid.  The closest
                    // hyperellipsoid point has x[N-1] == 0 and is on the
                    // domain-boundary hyperellipsoid.
                    x[N - 1] = (Real)0;
                    sqrDistance = Bisector(numPos, ePos, yPos, xPos);
                }
            }

            // Fill in those x[] values that were not zeroed out initially.
            for (i = 0, numPos = 0; i < N; ++i)
            {
                if (y[i] > (Real)0)
                {
                    x[i] = xPos[numPos];
                    ++numPos;
                }
            }

            return sqrDistance;
        }

        // The bisection algorithm to find the unique root of F(t).
        Real Bisector(int numComponents, Vector<N, Real> const& e,
            Vector<N, Real> const& y, Vector<N, Real>& x)
        {
            Vector<N, Real> z;
            Real sumZSqr = (Real)0;
            int i;
            for (i = 0; i < numComponents; ++i)
            {
                z[i] = y[i] / e[i];
                sumZSqr += z[i] * z[i];
            }

            if (sumZSqr == (Real)1)
            {
                // The point is on the hyperellipsoid.
                for (i = 0; i < numComponents; ++i)
                {
                    x[i] = y[i];
                }
                return (Real)0;
            }

            Real emin = e[numComponents - 1];
            Vector<N, Real> pSqr, numerator;
            pSqr.MakeZero();
            numerator.MakeZero();
            for (i = 0; i < numComponents; ++i)
            {
                Real p = e[i] / emin;
                pSqr[i] = p * p;
                numerator[i] = pSqr[i] * z[i];
            }

            Real s = (Real)0, smin = z[numComponents - 1] - (Real)1, smax;
            if (sumZSqr < (Real)1)
            {
                // The point is strictly inside the hyperellipsoid.
                smax = (Real)0;
            }
            else
            {
                // The point is strictly outside the hyperellipsoid.
                smax = Length(numerator, true) - (Real)1;
            }

            // The use of 'double' is intentional in case Real is a BSNumber
            // or BSRational type.  We want the bisections to terminate in a
            // reasonable/ amount of time.
            unsigned int const jmax = GTE_C_MAX_BISECTIONS_GENERIC;
            for (unsigned int j = 0; j < jmax; ++j)
            {
                s = (smin + smax) * (Real)0.5;
                if (s == smin || s == smax)
                {
                    break;
                }

                Real g = (Real)-1;
                for (i = 0; i < numComponents; ++i)
                {
                    Real ratio = numerator[i] / (s + pSqr[i]);
                    g += ratio * ratio;
                }

                if (g > (Real)0)
                {
                    smin = s;
                }
                else if (g < (Real)0)
                {
                    smax = s;
                }
                else
                {
                    break;
                }
            }

            Real sqrDistance = (Real)0;
            for (i = 0; i < numComponents; ++i)
            {
                x[i] = pSqr[i] * y[i] / (s + pSqr[i]);
                Real diff = x[i] - y[i];
                sqrDistance += diff * diff;
            }
            return sqrDistance;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPPointHyperellipsoid = DCPQuery<Real, Vector<N, Real>, Hyperellipsoid<N, Real>>;

    template <typename Real>
    using DCPPoint2Ellipse2 = DCPPointHyperellipsoid<2, Real>;

    template <typename Real>
    using DCPPoint3Ellipsoid3 = DCPPointHyperellipsoid<3, Real>;
}
