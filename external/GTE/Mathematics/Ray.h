// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector.h>

// The ray is represented as P+t*D, where P is the ray origin, D is a
// unit-length direction vector, and t >= 0.  The user must ensure that D is
// unit length.

namespace gte
{
    template <int N, typename Real>
    class Ray
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // origin to (0,...,0) and the ray direction to (1,0,...,0).
        Ray()
        {
            origin.MakeZero();
            direction.MakeUnit(0);
        }

        Ray(Vector<N, Real> const& inOrigin, Vector<N, Real> const& inDirection)
            :
            origin(inOrigin),
            direction(inDirection)
        {
        }

        // Public member access.  The direction must be unit length.
        Vector<N, Real> origin, direction;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Ray const& ray) const
        {
            return origin == ray.origin && direction == ray.direction;
        }

        bool operator!=(Ray const& ray) const
        {
            return !operator==(ray);
        }

        bool operator< (Ray const& ray) const
        {
            if (origin < ray.origin)
            {
                return true;
            }

            if (origin > ray.origin)
            {
                return false;
            }

            return direction < ray.direction;
        }

        bool operator<=(Ray const& ray) const
        {
            return !ray.operator<(*this);
        }

        bool operator> (Ray const& ray) const
        {
            return ray.operator<(*this);
        }

        bool operator>=(Ray const& ray) const
        {
            return !operator<(ray);
        }
    };

    // Template aliases for convenience.
    template <typename Real>
    using Ray2 = Ray<2, Real>;

    template <typename Real>
    using Ray3 = Ray<3, Real>;
}
