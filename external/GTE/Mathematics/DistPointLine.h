// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Vector<N, Real>, Line<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real lineParameter;  // t in (-infinity,+infinity)
            Vector<N, Real> lineClosest;  // origin + t * direction
        };

        Result operator()(Vector<N, Real> const& point, Line<N, Real> const& line)
        {
            Result result;

            Vector<N, Real> diff = point - line.origin;
            result.lineParameter = Dot(line.direction, diff);
            result.lineClosest = line.origin + result.lineParameter * line.direction;

            diff = point - result.lineClosest;
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPPointLine = DCPQuery<Real, Vector<N, Real>, Line<N, Real>>;

    template <typename Real>
    using DCPPoint2Line2 = DCPPointLine<2, Real>;

    template <typename Real>
    using DCPPoint3Line3 = DCPPointLine<3, Real>;
}
