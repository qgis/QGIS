// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrLine2AlignedBox2.h>
#include <Mathematics/OrientedBox.h>

// The queries consider the box to be a solid.
//
// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the four
// edges of the box.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line2<Real>, OrientedBox2<Real>>
        :
        public TIQuery<Real, Line2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public TIQuery<Real, Line2<Real>, AlignedBox2<Real>>::Result
        {
            // No additional relevant information to compute.
        };

        Result operator()(Line2<Real> const& line, OrientedBox2<Real> const& box)
        {
            // Transform the line to the oriented-box coordinate system.
            Vector2<Real> diff = line.origin - box.center;
            Vector2<Real> lineOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<Real> lineDirection
            {
                Dot(line.direction, box.axis[0]),
                Dot(line.direction, box.axis[1])
            };

            Result result;
            this->DoQuery(lineOrigin, lineDirection, box.extent, result);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line2<Real>, OrientedBox2<Real>>
        :
        public FIQuery<Real, Line2<Real>, AlignedBox2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line2<Real>, AlignedBox2<Real>>::Result
        {
            // No additional relevant information to compute.
        };

        Result operator()(Line2<Real> const& line, OrientedBox2<Real> const& box)
        {
            // Transform the line to the oriented-box coordinate system.
            Vector2<Real> diff = line.origin - box.center;
            Vector2<Real> lineOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<Real> lineDirection
            {
                Dot(line.direction, box.axis[0]),
                Dot(line.direction, box.axis[1])
            };

            Result result;
            this->DoQuery(lineOrigin, lineDirection, box.extent, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }
    };
}
