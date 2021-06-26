// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrAlignedBox2Circle2.h>
#include <Mathematics/DistPointOrientedBox.h>

// The find-intersection query is based on the document
// https://www.geometrictools.com/Documentation/IntersectionMovingCircleRectangle.pdf

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox2<Real>, Circle2<Real>>
    {
    public:
        // The intersection query considers the box and circle to be solids;
        // that is, the circle object includes the region inside the circular
        // boundary and the box object includes the region inside the
        // rectangular boundary.  If the circle object and rectangle object
        // overlap, the objects intersect.
        struct Result
        {
            bool intersect;
        };

        Result operator()(OrientedBox2<Real> const& box, Circle2<Real> const& circle)
        {
            DCPQuery<Real, Vector2<Real>, OrientedBox2<Real>> pbQuery;
            auto pbResult = pbQuery(circle.center, box);
            Result result;
            result.intersect = (pbResult.sqrDistance <= circle.radius * circle.radius);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, OrientedBox2<Real>, Circle2<Real>>
        :
        public FIQuery<Real, AlignedBox2<Real>, Circle2<Real>>
    {
    public:
        // See the base class for the definition of 'struct Result'.
        typename FIQuery<Real, AlignedBox2<Real>, Circle2<Real>>::Result
        operator()(OrientedBox2<Real> const& box, Vector2<Real> const& boxVelocity,
            Circle2<Real> const& circle, Vector2<Real> const& circleVelocity)
        {
            // Transform the oriented box to an axis-aligned box centered at
            // the origin and transform the circle accordingly.  Compute the
            // velocity of the circle relative to the box.
            Real const zero(0), one(1), minusOne(-1);
            Vector2<Real> cdiff = circle.center - box.center;
            Vector2<Real> vdiff = circleVelocity - boxVelocity;
            Vector2<Real> C, V;
            for (int i = 0; i < 2; ++i)
            {
                C[i] = Dot(cdiff, box.axis[i]);
                V[i] = Dot(vdiff, box.axis[i]);
            }

            // Change signs on components, if necessary, to transform C to the
            // first quadrant.  Adjust the velocity accordingly.
            Real sign[2];
            for (int i = 0; i < 2; ++i)
            {
                if (C[i] >= zero)
                {
                    sign[i] = one;
                }
                else
                {
                    C[i] = -C[i];
                    V[i] = -V[i];
                    sign[i] = minusOne;
                }
            }

            typename FIQuery<Real, AlignedBox2<Real>, Circle2<Real>>::Result result = { 0, zero, { zero, zero } };
            this->DoQuery(box.extent, C, circle.radius, V, result);

            if (result.intersectionType != 0)
            {
                // Transform back to the original coordinate system.
                result.contactPoint = box.center
                    + (sign[0] * result.contactPoint[0]) * box.axis[0]
                    + (sign[1] * result.contactPoint[1]) * box.axis[1];
            }
            return result;
        }
    };
}
