// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistPointAlignedBox.h>
#include <Mathematics/OrientedBox.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Vector<N, Real>, OrientedBox<N, Real>>
        :
        public DCPQuery<Real, Vector<N, Real>, AlignedBox<N, Real>>
    {
    public:
        struct Result
            :
            public DCPQuery<Real, Vector<N, Real>, AlignedBox<N, Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Vector<N, Real> const& point, OrientedBox<N, Real> const& box)
        {
            // Translate the point to the coordinate system of the box.  In
            // this system, the box is axis-aligned with center at the origin.
            Vector<N, Real> diff = point - box.center;
            Vector<N, Real> closest;
            for (int i = 0; i < N; ++i)
            {
                closest[i] = Dot(diff, box.axis[i]);
            }

            Result result;
            this->DoQuery(closest, box.extent, result);

            // Compute the closest point on the box.
            result.boxClosest = box.center;
            for (int i = 0; i < N; ++i)
            {
                result.boxClosest += closest[i] * box.axis[i];
            }
            return result;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPPointOrientedBox = DCPQuery<Real, Vector<N, Real>, AlignedBox<N, Real>>;

    template <typename Real>
    using DCPPoint2OrientedBox2 = DCPPointOrientedBox<2, Real>;

    template <typename Real>
    using DCPPoint3OrientedBox3 = DCPPointOrientedBox<3, Real>;
}
