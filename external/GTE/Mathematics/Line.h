// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector.h>

// The line is represented by P+t*D, where P is an origin point, D is a
// unit-length direction vector, and t is any real number.  The user must
// ensure that D is unit length.

namespace gte
{
    template <int N, typename Real>
    class Line
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // origin to (0,...,0) and the line direction to (1,0,...,0).
        Line()
        {
            origin.MakeZero();
            direction.MakeUnit(0);
        }

        Line(Vector<N, Real> const& inOrigin, Vector<N, Real> const& inDirection)
            :
            origin(inOrigin),
            direction(inDirection)
        {
        }

        // Public member access.  The direction must be unit length.
        Vector<N, Real> origin, direction;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Line const& line) const
        {
            return origin == line.origin && direction == line.direction;
        }

        bool operator!=(Line const& line) const
        {
            return !operator==(line);
        }

        bool operator< (Line const& line) const
        {
            if (origin < line.origin)
            {
                return true;
            }

            if (origin > line.origin)
            {
                return false;
            }

            return direction < line.direction;
        }

        bool operator<=(Line const& line) const
        {
            return !line.operator<(*this);
        }

        bool operator> (Line const& line) const
        {
            return line.operator<(*this);
        }

        bool operator>=(Line const& line) const
        {
            return !operator<(line);
        }
    };

    // Template aliases for convenience.
    template <typename Real>
    using Line2 = Line<2, Real>;

    template <typename Real>
    using Line3 = Line<3, Real>;
}
