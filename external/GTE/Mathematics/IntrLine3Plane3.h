// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/DistPoint3Plane3.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line3<Real>, Plane3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Line3<Real> const& line, Plane3<Real> const& plane)
        {
            Result result;

            Real DdN = Dot(line.direction, plane.normal);
            if (DdN != (Real)0)
            {
                // The line is not parallel to the plane, so they must
                // intersect.
                result.intersect = true;
            }
            else
            {
                // The line and plane are parallel.
                DCPQuery<Real, Vector3<Real>, Plane3<Real>> vpQuery;
                result.intersect = (vpQuery(line.origin, plane).distance == (Real)0);
            }

            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line3<Real>, Plane3<Real>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                parameter((Real)0),
                point{ (Real)0, (Real)0, (Real)0 }
            {
            }

            bool intersect;

            // The number of intersections is 0 (no intersection), 1 (linear
            // component and plane intersect in a point), or
            // std::numeric_limits<int>::max() (linear component is on the
            // plane).  If the linear component is on the plane, 'point'
            // component's origin and 'parameter' is zero.
            int numIntersections;
            Real parameter;
            Vector3<Real> point;
        };

        Result operator()(Line3<Real> const& line, Plane3<Real> const& plane)
        {
            Result result;
            DoQuery(line.origin, line.direction, plane, result);
            if (result.intersect)
            {
                result.point = line.origin + result.parameter * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& lineOrigin,
            Vector3<Real> const& lineDirection, Plane3<Real> const& plane,
            Result& result)
        {
            Real DdN = Dot(lineDirection, plane.normal);
            DCPQuery<Real, Vector3<Real>, Plane3<Real>> vpQuery;
            auto vpResult = vpQuery(lineOrigin, plane);

            if (DdN != (Real)0)
            {
                // The line is not parallel to the plane, so they must
                // intersect.
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter = -vpResult.signedDistance / DdN;
            }
            else
            {
                // The line and plane are parallel.  Determine whether the
                // line is on the plane.
                if (vpResult.distance == (Real)0)
                {
                    // The line is coincident with the plane, so choose t = 0
                    // for the parameter.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int>::max();
                    result.parameter = (Real)0;
                }
                else
                {
                    // The line is not on the plane.
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
        }
    };
}
