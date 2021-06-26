// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Segment.h>
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
    class FIQuery<Real, Segment3<Real>, Cone3<Real>>
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

        Result operator()(Segment3<Real> const& segment, Cone3<Real> const& cone)
        {
            // Execute the line-cone query.
            Result result;
            Vector3<Real> segOrigin = segment.p[0];
            Vector3<Real> segDirection = segment.p[1] - segment.p[0];
            this->DoQuery(segOrigin, segDirection, cone, result);

            // Adjust the t-interval depending on whether the line-cone
            // t-interval overlaps the segment interval [0,1].  The block
            // numbers are a continuation of those in IntrRay3Cone3.h, which
            // themselves are a continuation of those in IntrLine3Cone3.h.
            if (result.type != Result::isEmpty)
            {
                using QFN1 = typename FIQuery<Real, Line3<Real>, Cone3<Real>>::QFN1;
                QFN1 zero(0, 0, result.t[0].d), one(1, 0, result.t[0].d);

                if (result.type == Result::isPoint)
                {
                    if (result.t[0] < zero || result.t[0] > one)
                    {
                        // Block 21.
                        this->SetEmpty(result);
                    }
                    // else: Block 22.
                }
                else if (result.type == Result::isSegment)
                {
                    if (result.t[1] < zero || result.t[0] > one)
                    {
                        // Block 23.
                        this->SetEmpty(result);
                    }
                    else
                    {
                        auto t0 = std::max(zero, result.t[0]);
                        auto t1 = std::min(one, result.t[1]);
                        if (t0 < t1)
                        {
                            // Block 24.
                            this->SetSegment(t0, t1, result);
                        }
                        else
                        {
                            // Block 25.
                            this->SetPoint(t0, result);
                        }
                    }
                }
                else if (result.type == Result::isRayPositive)
                {
                    if (one < result.t[0])
                    {
                        // Block 26.
                        this->SetEmpty(result);
                    }
                    else if (one > result.t[0])
                    {
                        // Block 27.
                        this->SetSegment(std::max(zero, result.t[0]), one, result);
                    }
                    else
                    {
                        // Block 28.
                        this->SetPoint(one, result);
                    }
                }
                else  // result.type == Result::isRayNegative
                {
                    if (zero > result.t[1])
                    {
                        // Block 29.
                        this->SetEmpty(result);
                    }
                    else if (zero < result.t[1])
                    {
                        // Block 30.
                        this->SetSegment(zero, std::min(one, result.t[1]), result);
                    }
                    else
                    {
                        // Block 31.
                        this->SetPoint(zero, result);
                    }
                }
            }

            result.ComputePoints(segment.p[0], segDirection);
            result.intersect = (result.type != Result::isEmpty);
            return result;
        }
    };
}
