// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector.h>

// The box is aligned with the standard coordinate axes, which allows us to
// represent it using minimum and maximum values along each axis.  Some
// algorithms prefer the centered representation that is used for oriented
// boxes.  The center is C and the extents are the half-lengths in each
// coordinate-axis direction.

namespace gte
{
    template <int N, typename Real>
    class AlignedBox
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // minimum values to -1 and the maximum values to +1.
        AlignedBox()
        {
            for (int i = 0; i < N; ++i)
            {
                min[i] = (Real)-1;
                max[i] = (Real)+1;
            }
        }

        // Please ensure that inMin[i] <= inMax[i] for all i.
        AlignedBox(Vector<N, Real> const& inMin, Vector<N, Real> const& inMax)
        {
            for (int i = 0; i < N; ++i)
            {
                min[i] = inMin[i];
                max[i] = inMax[i];
            }
        }

        // Compute the centered representation.  NOTE:  If you set the minimum
        // and maximum values, compute C and extents, and then recompute the
        // minimum and maximum values, the numerical round-off errors can lead
        // to results different from what you started with.
        void GetCenteredForm(Vector<N, Real>& center, Vector<N, Real>& extent) const
        {
            center = (max + min) * (Real)0.5;
            extent = (max - min) * (Real)0.5;
        }

        // Public member access.  It is required that min[i] <= max[i].
        Vector<N, Real> min, max;

    public:
        // Comparisons to support sorted containers.
        bool operator==(AlignedBox const& box) const
        {
            return min == box.min && max == box.max;
        }

        bool operator!=(AlignedBox const& box) const
        {
            return !operator==(box);
        }

        bool operator< (AlignedBox const& box) const
        {
            if (min < box.min)
            {
                return true;
            }

            if (min > box.min)
            {
                return false;
            }

            return max < box.max;
        }

        bool operator<=(AlignedBox const& box) const
        {
            return !box.operator<(*this);
        }

        bool operator> (AlignedBox const& box) const
        {
            return box.operator<(*this);
        }

        bool operator>=(AlignedBox const& box) const
        {
            return !operator<(box);
        }
    };

    // Template aliases for convenience.
    template <typename Real>
    using AlignedBox2 = AlignedBox<2, Real>;

    template <typename Real>
    using AlignedBox3 = AlignedBox<3, Real>;
}
