// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Hypersphere.h>
#include <Mathematics/LinearSystem.h>

namespace gte
{
    // All functions return 'true' if the circle has been constructed, 'false'
    // otherwise (input points are linearly dependent).

    // Circle circumscribing triangle.
    template <typename Real>
    bool Circumscribe(Vector2<Real> const& v0, Vector2<Real> const& v1,
        Vector2<Real> const& v2, Circle2<Real>& circle)
    {
        Vector2<Real> e10 = v1 - v0;
        Vector2<Real> e20 = v2 - v0;
        Matrix2x2<Real> A{ e10[0], e10[1], e20[0], e20[1] };
        Vector2<Real> B{ (Real)0.5 * Dot(e10, e10), (Real)0.5 * Dot(e20, e20) };
        Vector2<Real> solution;
        if (LinearSystem<Real>::Solve(A, B, solution))
        {
            circle.center = v0 + solution;
            circle.radius = Length(solution);
            return true;
        }
        return false;
    }

    // Circle inscribing triangle.
    template <typename Real>
    bool Inscribe(Vector2<Real> const& v0, Vector2<Real> const& v1,
        Vector2<Real> const& v2, Circle2<Real>& circle)
    {
        Vector2<Real> d10 = v1 - v0;
        Vector2<Real> d20 = v2 - v0;
        Vector2<Real> d21 = v2 - v1;
        Real len10 = Length(d10);
        Real len20 = Length(d20);
        Real len21 = Length(d21);
        Real perimeter = len10 + len20 + len21;
        if (perimeter > (Real)0)
        {
            Real inv = (Real)1 / perimeter;
            len10 *= inv;
            len20 *= inv;
            len21 *= inv;
            circle.center = len21 * v0 + len20 * v1 + len10 * v2;
            circle.radius = inv * std::fabs(DotPerp(d10, d20));
            return circle.radius > (Real)0;
        }
        return false;
    }
}
