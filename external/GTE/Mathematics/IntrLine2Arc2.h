// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrLine2Circle2.h>
#include <Mathematics/Arc2.h>

// The queries consider the arc to be a 1-dimensional object.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line2<Real>, Arc2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Line2<Real> const& line, Arc2<Real> const& arc)
        {
            Result result;
            FIQuery<Real, Line2<Real>, Arc2<Real>> laQuery;
            auto laResult = laQuery(line, arc);
            result.intersect = laResult.intersect;
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line2<Real>, Arc2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector2<Real>, 2> point;
        };

        Result operator()(Line2<Real> const& line, Arc2<Real> const& arc)
        {
            Result result;

            FIQuery<Real, Line2<Real>, Circle2<Real>> lcQuery;
            Circle2<Real> circle(arc.center, arc.radius);
            auto lcResult = lcQuery(line, circle);
            if (lcResult.intersect)
            {
                // Test whether line-circle intersections are on the arc.
                result.numIntersections = 0;
                for (int i = 0; i < lcResult.numIntersections; ++i)
                {
                    if (arc.Contains(lcResult.point[i]))
                    {
                        result.intersect = true;
                        result.parameter[result.numIntersections]
                            = lcResult.parameter[i];
                        result.point[result.numIntersections++]
                            = lcResult.point[i];
                    }
                }
            }
            else
            {
                result.intersect = false;
                result.numIntersections = 0;
            }

            return result;
        }
    };
}
