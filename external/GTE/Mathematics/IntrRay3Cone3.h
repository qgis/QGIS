// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Ray.h>
#include <Mathematics/IntrLine3Cone3.h>

// The queries consider the cone to be single sided and solid.  The
// cone height range is [hmin,hmax].  The cone can be infinite where
// hmin = 0 and hmax = +infinity, infinite truncated where hmin > 0
// and hmax = +infinity, finite where hmin = 0 and hmax < +infinity,
// or a cone frustum where hmin > 0 and hmax < +infinity.  The
// algorithm details are found in
// https://www.geometrictools.com/Documentation/IntersectionLineCone.pdf

namespace gte
{
    template <typename Real>
    class FIQuery<Real, Ray3<Real>, Cone3<Real>>
        :
        public FIQuery<Real, Line3<Real>, Cone3<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line3<Real>, Cone3<Real>>::Result
        {
            // No additional information to compute.
        };

        Result operator()(Ray3<Real> const& ray, Cone3<Real> const& cone)
        {
            // Execute the line-cone query.
            Result result;
            this->DoQuery(ray.origin, ray.direction, cone, result);

            // Adjust the t-interval depending on whether the line-cone
            // t-interval overlaps the ray interval [0,+infinity).  The block
            // numbers are a continuation of those in IntrLine3Cone3.h.
            if (result.type != Result::isEmpty)
            {
                using QFN1 = typename FIQuery<Real, Line3<Real>, Cone3<Real>>::QFN1;
                QFN1 zero(0, 0, result.t[0].d);

                if (result.type == Result::isPoint)
                {
                    if (result.t[0] < zero)
                    {
                        // Block 12.
                        this->SetEmpty(result);
                    }
                    // else: Block 13.
                }
                else if (result.type == Result::isSegment)
                {
                    if (result.t[1] > zero)
                    {
                        // Block 14.
                        this->SetSegment(std::max(result.t[0], zero), result.t[1], result);
                    }
                    else if (result.t[1] < zero)
                    {
                        // Block 15.
                        this->SetEmpty(result);
                    }
                    else  // result.t[1] == zero
                    {
                        // Block 16.
                        this->SetPoint(zero, result);
                    }
                }
                else if (result.type == Result::isRayPositive)
                {
                    // Block 17.
                    this->SetRayPositive(std::max(result.t[0], zero), result);
                }
                else  // result.type == Result::isRayNegative
                {
                    if (result.t[1] > zero)
                    {
                        // Block 18.
                        this->SetSegment(zero, result.t[1], result);
                    }
                    else if (result.t[1] < zero)
                    {
                        // Block 19.
                        this->SetEmpty(result);
                    }
                    else  // result.t[1] == zero
                    {
                        // Block 20.
                        this->SetPoint(zero, result);
                    }
                }
            }

            result.ComputePoints(ray.origin, ray.direction);
            result.intersect = (result.type != Result::isEmpty);
            return result;
        }
    };
}
