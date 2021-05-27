// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector.h>

// The halfspace is represented as Dot(N,X) >= c where N is a unit-length
// normal vector, c is the plane constant, and X is any point in space.
// The user must ensure that the normal vector is unit length.

namespace gte
{
    template <int N, typename Real>
    class Halfspace
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // normal to (0,...,0,1) and the constant to zero (halfspace
        // x[N-1] >= 0).
        Halfspace()
            :
            constant((Real)0)
        {
            normal.MakeUnit(N - 1);
        }

        // Specify N and c directly.
        Halfspace(Vector<N, Real> const& inNormal, Real inConstant)
            :
            normal(inNormal),
            constant(inConstant)
        {
        }

        // Public member access.
        Vector<N, Real> normal;
        Real constant;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Halfspace const& halfspace) const
        {
            return normal == halfspace.normal && constant == halfspace.constant;
        }

        bool operator!=(Halfspace const& halfspace) const
        {
            return !operator==(halfspace);
        }

        bool operator< (Halfspace const& halfspace) const
        {
            if (normal < halfspace.normal)
            {
                return true;
            }

            if (normal > halfspace.normal)
            {
                return false;
            }

            return constant < halfspace.constant;
        }

        bool operator<=(Halfspace const& halfspace) const
        {
            return !halfspace.operator<(*this);
        }

        bool operator> (Halfspace const& halfspace) const
        {
            return halfspace.operator<(*this);
        }

        bool operator>=(Halfspace const& halfspace) const
        {
            return !operator<(halfspace);
        }
    };

    // Template alias for convenience.
    template <typename Real>
    using Halfspace3 = Halfspace<3, Real>;
}
