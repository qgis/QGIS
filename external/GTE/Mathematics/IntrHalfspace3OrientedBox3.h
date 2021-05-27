// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/Halfspace.h>
#include <Mathematics/OrientedBox.h>

// Queries for intersection of objects with halfspaces.  These are useful for
// containment testing, object culling, and clipping.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Halfspace3<Real>, OrientedBox3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Halfspace3<Real> const& halfspace, OrientedBox3<Real> const& box)
        {
            Result result;

            // Project the box center onto the normal line.  The plane of the
            // halfspace occurs at the origin (zero) of the normal line.
            Real center = Dot(halfspace.normal, box.center) - halfspace.constant;

            // Compute the radius of the interval of projection.
            Real radius =
                std::fabs(box.extent[0] * Dot(halfspace.normal, box.axis[0])) +
                std::fabs(box.extent[1] * Dot(halfspace.normal, box.axis[1])) +
                std::fabs(box.extent[2] * Dot(halfspace.normal, box.axis[2]));

            // The box and halfspace intersect when the projection interval
            // maximum is nonnegative.
            result.intersect = (center + radius >= (Real)0);
            return result;
        }
    };
}
