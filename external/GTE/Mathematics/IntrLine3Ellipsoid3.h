// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.02.10

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Line.h>
#include <Mathematics/Matrix3x3.h>

// The queries consider the ellipsoid to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line3<Real>, Ellipsoid3<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Line3<Real> const& line, Ellipsoid3<Real> const& ellipsoid)
        {
            // The ellipsoid is (X-K)^T*M*(X-K)-1 = 0 and the line is
            // X = P+t*D.  Substitute the line equation into the ellipsoid
            // equation to obtain a quadratic equation
            //   Q(t) = a2*t^2 + 2*a1*t + a0 = 0
            // where a2 = D^T*M*D, a1 = D^T*M*(P-K) and
            // a0 = (P-K)^T*M*(P-K)-1.
            Real constexpr zero = 0;
            Result result{};

            Matrix3x3<Real> M;
            ellipsoid.GetM(M);

            Vector3<Real> diff = line.origin - ellipsoid.center;
            Vector3<Real> matDir = M * line.direction;
            Vector3<Real> matDiff = M * diff;
            Real a2 = Dot(line.direction, matDir);
            Real a1 = Dot(line.direction, matDiff);
            Real a0 = Dot(diff, matDiff) - (Real)1;

            // Intersection occurs when Q(t) has real roots.
            Real discr = a1 * a1 - a0 * a2;
            result.intersect = (discr >= zero);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line3<Real>, Ellipsoid3<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{},
                point{}
            {
                Real constexpr rmax = std::numeric_limits<Real>::max();
                parameter.fill(rmax);
                point.fill(Vector3<Real>{ rmax, rmax, rmax });
            }

            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector3<Real>, 2> point;
        };

        Result operator()(Line3<Real> const& line, Ellipsoid3<Real> const& ellipsoid)
        {
            Result result{};
            DoQuery(line.origin, line.direction, ellipsoid, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }


    protected:
        void DoQuery(Vector3<Real> const& lineOrigin,
            Vector3<Real> const& lineDirection, Ellipsoid3<Real> const& ellipsoid,
            Result& result)
        {
            // The ellipsoid is (X-K)^T*M*(X-K)-1 = 0 and the line is
            // X = P+t*D.  Substitute the line equation into the ellipsoid
            // equation to obtain a quadratic equation
            //   Q(t) = a2*t^2 + 2*a1*t + a0 = 0
            // where a2 = D^T*M*D, a1 = D^T*M*(P-K) and
            // a0 = (P-K)^T*M*(P-K)-1.
            Real constexpr zero = 0;
            Matrix3x3<Real> M;
            ellipsoid.GetM(M);

            Vector3<Real> diff = lineOrigin - ellipsoid.center;
            Vector3<Real> matDir = M * lineDirection;
            Vector3<Real> matDiff = M * diff;
            Real a2 = Dot(lineDirection, matDir);
            Real a1 = Dot(lineDirection, matDiff);
            Real a0 = Dot(diff, matDiff) - (Real)1;

            // Intersection occurs when Q(t) has real roots.
            Real discr = a1 * a1 - a0 * a2;
            if (discr > zero)
            {
                // The line intersects the ellipsoid in 2 distinct points.
                Real constexpr one = 1;
                result.intersect = true;
                result.numIntersections = 2;
                Real root = std::sqrt(discr);
                Real inv = one / a2;
                result.parameter[0] = (-a1 - root) * inv;
                result.parameter[1] = (-a1 + root) * inv;
            }
            else if (discr < zero)
            {
                // The line does not intersect the ellipsoid. The parameter[]
                // values are initialized to invalid numbers, but they should
                // not be used by the caller.
                Real constexpr rmax = std::numeric_limits<Real>::max();
                result.intersect = false;
                result.numIntersections = 0;
                result.parameter[0] = +rmax;
                result.parameter[1] = -rmax;
            }
            else
            {
                // The line is tangent to the ellipsoid, so the intersection
                // is a single point. The parameter[1] value is set, because
                // callers will access the degenerate interval
                // [-a1 / a2, -a1 / a2].
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter[0] = -a1 / a2;
                result.parameter[1] = result.parameter[0];
            }
        }
    };
}
