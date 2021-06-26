// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Rectangle.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    // A lozenge is the set of points that are equidistant from a rectangle,
    // the common distance called the radius.
    template <typename Real>
    class Lozenge3
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // rectangle to have origin (0,0,0), axes (1,0,0) and (0,1,0), and
        // both extents 1.  The default radius is 1.
        Lozenge3()
            :
            radius((Real)1)
        {
        }

        Lozenge3(Rectangle<3, Real> const& inRectangle, Real inRadius)
            :
            rectangle(inRectangle),
            radius(inRadius)
        {
        }

        // Public member access.
        Rectangle<3, Real> rectangle;
        Real radius;

        // Comparisons to support sorted containers.
        bool operator==(Lozenge3 const& other) const
        {
            return rectangle == other.rectangle && radius == other.radius;
        }

        bool operator!=(Lozenge3 const& other) const
        {
            return !operator==(other);
        }

        bool operator< (Lozenge3 const& other) const
        {
            if (rectangle < other.rectangle)
            {
                return true;
            }

            if (rectangle > other.rectangle)
            {
                return false;
            }

            return radius < other.radius;
        }

        bool operator<=(Lozenge3 const& other) const
        {
            return !other.operator<(*this);
        }

        bool operator> (Lozenge3 const& other) const
        {
            return other.operator<(*this);
        }

        bool operator>=(Lozenge3 const& other) const
        {
            return !operator<(other);
        }
    };
}
