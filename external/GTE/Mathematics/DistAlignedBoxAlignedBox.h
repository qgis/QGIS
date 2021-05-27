// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/IntrIntervals.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, AlignedBox<N, Real>, AlignedBox<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;

            // To compute a single closest point on each box, use
            // Vector<N, Real> closest0 =
            //   (closestPoints[0].min + closestPoints[0].max)/2;
            // Vector<N, Real> closest1 =
            //   (closestPoints[1].min + closestPoints[1].max)/2;
            AlignedBox<N, Real> closestPoints[2];
        };

        Result operator()(AlignedBox<N, Real> const& box0, AlignedBox<N, Real> const& box1)
        {
            Result result;
            result.sqrDistance = (Real)0;
            for (int i = 0; i < N; ++i)
            {
                if (box0.min[i] >= box1.max[i])
                {
                    Real delta = box0.min[i] - box1.min[i];
                    result.sqrDistance += delta * delta;
                    result.closestPoints[0].min[i] = box0.min[i];
                    result.closestPoints[0].max[i] = box0.min[i];
                    result.closestPoints[1].min[i] = box1.max[i];
                    result.closestPoints[1].max[i] = box1.max[i];
                }
                else if (box1.min[i] >= box0.max[i])
                {
                    Real delta = box1.min[i] - box0.max[i];
                    result.sqrDistance += delta * delta;
                    result.closestPoints[0].min[i] = box0.max[i];
                    result.closestPoints[0].max[i] = box0.max[i];
                    result.closestPoints[1].min[i] = box1.min[i];
                    result.closestPoints[1].max[i] = box1.min[i];
                }
                else
                {
                    std::array<Real, 2> intr0 = { box0.min[i], box0.max[i] };
                    std::array<Real, 2> intr1 = { box1.min[i], box1.max[i] };
                    FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> query;
                    auto iiResult = query(intr0, intr1);
                    for (int j = 0; j < 2; ++j)
                    {
                        result.closestPoints[j].min[i] = iiResult.overlap[0];
                        result.closestPoints[j].max[i] = iiResult.overlap[1];
                    }
                }
            }
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };
}
