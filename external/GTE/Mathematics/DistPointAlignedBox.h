// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/AlignedBox.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Vector<N, Real>, AlignedBox<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Vector<N, Real> boxClosest;
        };

        Result operator()(Vector<N, Real> const& point, AlignedBox<N, Real> const& box)
        {
            // Translate the point and box so that the box has center at the
            // origin.
            Vector<N, Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);
            Vector<N, Real> closest = point - boxCenter;

            Result result;
            DoQuery(closest, boxExtent, result);

            // Compute the closest point on the box.
            result.boxClosest = boxCenter + closest;
            return result;
        }

    protected:
        // On input, 'point' is the difference of the query point and the box
        // center.  On output, 'point' is the point on the box closest to the
        // query point.
        void DoQuery(Vector<N, Real>& point, Vector<N, Real> const& boxExtent,
            Result& result)
        {
            result.sqrDistance = (Real)0;
            for (int i = 0; i < N; ++i)
            {
                if (point[i] < -boxExtent[i])
                {
                    Real delta = point[i] + boxExtent[i];
                    result.sqrDistance += delta * delta;
                    point[i] = -boxExtent[i];
                }
                else if (point[i] > boxExtent[i])
                {
                    Real delta = point[i] - boxExtent[i];
                    result.sqrDistance += delta * delta;
                    point[i] = boxExtent[i];
                }
            }
            result.distance = std::sqrt(result.sqrDistance);
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPPointAlignedBox = DCPQuery<Real, Vector<N, Real>, AlignedBox<N, Real>>;

    template <typename Real>
    using DCPPoint2AlignedBox2 = DCPPointAlignedBox<2, Real>;

    template <typename Real>
    using DCPPoint3AlignedBox3 = DCPPointAlignedBox<3, Real>;
}
