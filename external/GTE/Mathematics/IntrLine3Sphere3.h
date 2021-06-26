// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.02.10

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Line.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line3<Real>, Sphere3<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            };

            bool intersect;
        };

        Result operator()(Line3<Real> const& line, Sphere3<Real> const& sphere)
        {
            // The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D.
            // Substitute the line equation into the sphere equation to
            // obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where
            // a1 = D^T*(P-C) and a0 = (P-C)^T*(P-C)-1.
            Real constexpr zero = 0;
            Result result{};

            Vector3<Real> diff = line.origin - sphere.center;
            Real a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            Real a1 = Dot(line.direction, diff);

            // Intersection occurs when Q(t) has real roots.
            Real discr = a1 * a1 - a0;
            result.intersect = (discr >= zero);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line3<Real>, Sphere3<Real>>
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

        Result operator()(Line3<Real> const& line, Sphere3<Real> const& sphere)
        {
            Result result{};
            DoQuery(line.origin, line.direction, sphere, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& lineOrigin,
            Vector3<Real> const& lineDirection, Sphere3<Real> const& sphere,
            Result& result)
        {
            // The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D.
            // Substitute the line equation into the sphere equation to
            // obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where
            // a1 = D^T*(P-C) and a0 = (P-C)^T*(P-C)-1.
            Real constexpr zero = 0;
            Vector3<Real> diff = lineOrigin - sphere.center;
            Real a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            Real a1 = Dot(lineDirection, diff);

            // Intersection occurs when Q(t) has real roots.
            Real discr = a1 * a1 - a0;
            if (discr > zero)
            {
                // The line intersects the sphere in 2 distinct points.
                result.intersect = true;
                result.numIntersections = 2;
                Real root = std::sqrt(discr);
                result.parameter[0] = -a1 - root;
                result.parameter[1] = -a1 + root;
            }
            else if (discr < zero)
            {
                // The line does not intersect the sphere. The parameter[]
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
                // The line is tangent to the sphere, so the intersection is
                // a single point. The parameter[1] value is set, because
                // callers will access the degenerate interval [-a1,-a1].
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter[0] = -a1;
                result.parameter[1] = result.parameter[0];
            }
        }
    };
}
