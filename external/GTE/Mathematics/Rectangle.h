// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector.h>

// Points are R(s0,s1) = C + s0*A0 + s1*A1, where C is the center of the
// rectangle and A0 and A1 are unit-length and perpendicular axes.  The
// parameters s0 and s1 are constrained by |s0| <= e0 and |s1| <= e1,
// where e0 > 0 and e1 > 0 are the extents of the rectangle.

namespace gte
{
    template <int N, typename Real>
    class Rectangle
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // origin to (0,...,0), axis A0 to (1,0,...,0), axis A1 to
        // (0,1,0,...0) and both extents to 1.
        Rectangle()
        {
            center.MakeZero();
            for (int i = 0; i < 2; ++i)
            {
                axis[i].MakeUnit(i);
                extent[i] = (Real)1;
            }
        }

        Rectangle(Vector<N, Real> const& inCenter,
            std::array<Vector<N, Real>, 2> const& inAxis,
            Vector<2, Real> const& inExtent)
            :
            center(inCenter),
            axis(inAxis),
            extent(inExtent)
        {
        }

        // Compute the vertices of the rectangle.  If index i has the bit
        // pattern i = b[1]b[0], then
        //   vertex[i] = center + sum_{d=0}^{1} sign[d] * extent[d] * axis[d]
        // where sign[d] = 2*b[d] - 1.
        void GetVertices(std::array<Vector<N, Real>, 4>& vertex) const
        {
            Vector<N, Real> product0 = extent[0] * axis[0];
            Vector<N, Real> product1 = extent[1] * axis[1];
            Vector<N, Real> sum = product0 + product1;
            Vector<N, Real> dif = product0 - product1;

            vertex[0] = center - sum;
            vertex[1] = center + dif;
            vertex[2] = center - dif;
            vertex[3] = center + sum;
        }

        Vector<N, Real> center;
        std::array<Vector<N, Real>, 2> axis;
        Vector<2, Real> extent;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Rectangle const& rectangle) const
        {
            if (center != rectangle.center)
            {
                return false;
            }

            for (int i = 0; i < 2; ++i)
            {
                if (axis[i] != rectangle.axis[i])
                {
                    return false;
                }
            }

            for (int i = 0; i < 2; ++i)
            {
                if (extent[i] != rectangle.extent[i])
                {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(Rectangle const& rectangle) const
        {
            return !operator==(rectangle);
        }

        bool operator< (Rectangle const& rectangle) const
        {
            if (center < rectangle.center)
            {
                return true;
            }

            if (center > rectangle.center)
            {
                return false;
            }

            if (axis < rectangle.axis)
            {
                return true;
            }

            if (axis > rectangle.axis)
            {
                return false;
            }

            return extent < rectangle.extent;
        }

        bool operator<=(Rectangle const& rectangle) const
        {
            return !rectangle.operator<(*this);
        }

        bool operator> (Rectangle const& rectangle) const
        {
            return rectangle.operator<(*this);
        }

        bool operator>=(Rectangle const& rectangle) const
        {
            return !operator<(rectangle);
        }
    };

    // Template alias for convenience.
    template <typename Real>
    using Rectangle3 = Rectangle<3, Real>;
}
