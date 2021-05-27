// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/Halfspace.h>
#include <Mathematics/Hypersphere.h>

// Queries for intersection of objects with halfspaces.  These are useful for
// containment testing, object culling, and clipping.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Halfspace3<Real>, Sphere3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Halfspace3<Real> const& halfspace, Sphere3<Real> const& sphere)
        {
            Result result;

            // Project the sphere center onto the normal line.  The plane of
            // the halfspace occurs at the origin (zero) of the normal line.
            Real center = Dot(halfspace.normal, sphere.center) - halfspace.constant;

            // The sphere and halfspace intersect when the projection interval
            // maximum is nonnegative.
            result.intersect = (center + sphere.radius >= (Real)0);
            return result;
        }
    };
}
