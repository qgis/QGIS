// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrCircle2Circle2.h>
#include <Mathematics/Arc2.h>

namespace gte
{
    template <typename Real>
    class FIQuery<Real, Arc2<Real>, Arc2<Real>>
    {
    public:
        // The possible 'configuration' in Result are listed as an
        // enumeration.  The valid array elements are listed in the comments.
        enum
        {
            NO_INTERSECTION,
            NONCOCIRCULAR_ONE_POINT,        // point[0]
            NONCOCIRCULAR_TWO_POINTS,       // point[0], point[1]
            COCIRCULAR_ONE_POINT,           // point[0]
            COCIRCULAR_TWO_POINTS,          // point[0], point[1]
            COCIRCULAR_ONE_POINT_ONE_ARC,   // point[0], arc[0]
            COCIRCULAR_ONE_ARC,             // arc[0]
            COCIRCULAR_TWO_ARCS             // arc[0], arc[1]
        };

        struct Result
        {
            // 'true' iff configuration != NO_INTERSECTION
            bool intersect;

            // one of the enumerations listed previously
            int configuration;

            Vector2<Real> point[2];
            Arc2<Real> arc[2];
        };

        Result operator()(Arc2<Real> const& arc0, Arc2<Real> const& arc1)
        {
            // Assume initially there are no intersections.  If we find at
            // least one intersection, we will set result.intersect to 'true'.
            Result result;
            result.intersect = false;
            result.configuration = NO_INTERSECTION;
            result.point[0] = { (Real)0, (Real)0 };
            result.point[1] = { (Real)0, (Real)0 };

            Circle2<Real> circle0(arc0.center, arc0.radius);
            Circle2<Real> circle1(arc1.center, arc1.radius);
            FIQuery<Real, Circle2<Real>, Circle2<Real>> ccQuery;
            auto ccResult = ccQuery(circle0, circle1);
            if (!ccResult.intersect)
            {
                // The arcs do not intersect.
                result.configuration = NO_INTERSECTION;
                return result;
            }

            if (ccResult.numIntersections == std::numeric_limits<int>::max())
            {
                // The arcs are cocircular.  Determine whether they overlap.
                // Let arc0 be <A0,A1> and arc1 be <B0,B1>.  The points are
                // ordered counterclockwise around the circle of the arc.
                if (arc1.Contains(arc0.end[0]))
                {
                    result.intersect = true;
                    if (arc1.Contains(arc0.end[1]))
                    {
                        if (arc0.Contains(arc1.end[0]) && arc0.Contains(arc1.end[1]))
                        {
                            if (arc0.end[0] == arc1.end[0] && arc0.end[1] == arc1.end[1])
                            {
                                // The arcs are the same.
                                result.configuration = COCIRCULAR_ONE_ARC;
                                result.arc[0] = arc0;
                            }
                            else
                            {
                                // arc0 and arc1 overlap in two disjoint
                                // subsets.
                                if (arc0.end[0] != arc1.end[1])
                                {
                                    if (arc1.end[0] != arc0.end[1])
                                    {
                                        // The arcs overlap in two disjoint
                                        // subarcs, each of positive subtended
                                        // angle: <A0,B1>, <A1,B0>
                                        result.configuration = COCIRCULAR_TWO_ARCS;
                                        result.arc[0] = Arc2<Real>(arc0.center, arc0.radius, arc0.end[0], arc1.end[1]);
                                        result.arc[1] = Arc2<Real>(arc0.center, arc0.radius, arc1.end[0], arc0.end[1]);
                                    }
                                    else  // B0 = A1
                                    {
                                        // The intersection is a point {A1}
                                        // and an arc <A0,B1>.
                                        result.configuration = COCIRCULAR_ONE_POINT_ONE_ARC;
                                        result.point[0] = arc0.end[1];
                                        result.arc[0] = Arc2<Real>(arc0.center, arc0.radius, arc0.end[0], arc1.end[1]);
                                    }
                                }
                                else  // A0 = B1
                                {
                                    if (arc1.end[0] != arc0.end[1])
                                    {
                                        // The intersection is a point {A0}
                                        // and an arc <A1,B0>.
                                        result.configuration = COCIRCULAR_ONE_POINT_ONE_ARC;
                                        result.point[0] = arc0.end[0];
                                        result.arc[0] = Arc2<Real>(arc0.center, arc0.radius, arc1.end[0], arc0.end[1]);
                                    }
                                    else
                                    {
                                        // The arcs shared endpoints, so the
                                        // union is a circle.
                                        result.configuration = COCIRCULAR_TWO_POINTS;
                                        result.point[0] = arc0.end[0];
                                        result.point[1] = arc0.end[1];
                                    }
                                }
                            }
                        }
                        else
                        {
                            // Arc0 inside arc1, <B0,A0,A1,B1>.
                            result.configuration = COCIRCULAR_ONE_ARC;
                            result.arc[0] = arc0;
                        }
                    }
                    else
                    {
                        if (arc0.end[0] != arc1.end[1])
                        {
                            // Arc0 and arc1 overlap, <B0,A0,B1,A1>.
                            result.configuration = COCIRCULAR_ONE_ARC;
                            result.arc[0] = Arc2<Real>(arc0.center, arc0.radius, arc0.end[0], arc1.end[1]);
                        }
                        else
                        {
                            // Arc0 and arc1 share endpoint, <B0,A0,B1,A1>
                            // with A0 = B1.
                            result.configuration = COCIRCULAR_ONE_POINT;
                            result.point[0] = arc0.end[0];
                        }
                    }
                    return result;
                }

                if (arc1.Contains(arc0.end[1]))
                {
                    result.intersect = true;
                    if (arc0.end[1] != arc1.end[0])
                    {
                        // Arc0 and arc1 overlap in a single arc,
                        // <A0,B0,A1,B1>.
                        result.configuration = COCIRCULAR_ONE_ARC;
                        result.arc[0] = Arc2<Real>(arc0.center, arc0.radius, arc1.end[0], arc0.end[1]);
                    }
                    else
                    {
                        // Arc0 and arc1 share endpoint, <A0,B0,A1,B1>
                        // with B0 = A1.
                        result.configuration = COCIRCULAR_ONE_POINT;
                        result.point[0] = arc1.end[0];
                    }
                    return result;
                }

                if (arc0.Contains(arc1.end[0]))
                {
                    // Arc1 inside arc0, <A0,B0,B1,A1>.
                    result.intersect = true;
                    result.configuration = COCIRCULAR_ONE_ARC;
                    result.arc[0] = arc1;
                }
                else
                {
                    // Arcs do not overlap, <A0,A1,B0,B1>.
                    result.configuration = NO_INTERSECTION;
                }
                return result;
            }

            // Test whether circle-circle intersection points are on the arcs.
            int numIntersections = 0;
            for (int i = 0; i < ccResult.numIntersections; ++i)
            {
                if (arc0.Contains(ccResult.point[i]) && arc1.Contains(ccResult.point[i]))
                {
                    result.point[numIntersections++] = ccResult.point[i];
                    result.intersect = true;
                }
            }

            if (numIntersections == 2)
            {
                result.configuration = NONCOCIRCULAR_TWO_POINTS;
            }
            else if (numIntersections == 1)
            {
                result.configuration = NONCOCIRCULAR_ONE_POINT;
            }
            else
            {
                result.configuration = NO_INTERSECTION;
            }

            return result;
        }
    };
}
