// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <vector>

// Test whether an undirected graph is planar.  The input positions must be
// unique and the input edges must be unique, so the number of positions is
// at least 2 and the number of edges is at least one.  The elements of the
// edges array must be indices in {0,..,positions.size()-1}.
// 
// A sort-and-sweep algorithm is used to determine edge-edge intersections.
// If none of the intersections occur at edge-interior points, the graph is
// planar.  See Game Physics (2nd edition), Section 6.2.2: Culling with
// Axis-Aligned Bounding Boxes for such an algorithm.  The operator()
// returns 'true' when the graph is planar.  If it returns 'false', the
// 'invalidIntersections' set contains pairs of edges that intersect at an
// edge-interior point (that by definition is not a graph vertex).  Each set
// element is a pair of indices into the 'edges' array; the indices are
// ordered so that element[0] < element[1].  The Real type must be chosen
// to guarantee exact computation of edge-edge intersections.

namespace gte
{
    template <typename Real>
    class IsPlanarGraph
    {
    public:
        IsPlanarGraph()
            :
            mZero(0),
            mOne(1)
        {
        }

        enum
        {
            IPG_IS_PLANAR_GRAPH = 0,
            IPG_INVALID_INPUT_SIZES = 1,
            IPG_DUPLICATED_POSITIONS = 2,
            IPG_DUPLICATED_EDGES = 4,
            IPG_DEGENERATE_EDGES = 8,
            IPG_EDGES_WITH_INVALID_VERTICES = 16,
            IPG_INVALID_INTERSECTIONS = 32
        };

        // The function returns a combination of the IPG_* flags listed in the
        // previous enumeration.  A combined value of 0 indicates the input
        // forms a planar graph.  If the combined value is not zero, you may
        // examine the flags for the failure conditions and use the Get*
        // member accessors to obtain specific information about the failure.
        // If the positions.size() < 2 or edges.size() == 0, the
        // IPG_INVALID_INPUT_SIZES flag is set.

        int operator()(std::vector<std::array<Real, 2>> const& positions,
            std::vector<std::array<int, 2>> const& edges)
        {
            mDuplicatedPositions.clear();
            mDuplicatedEdges.clear();
            mDegenerateEdges.clear();
            mEdgesWithInvalidVertices.clear();
            mInvalidIntersections.clear();

            int flags = IsValidTopology(positions, edges);
            if (flags == IPG_INVALID_INPUT_SIZES)
            {
                return flags;
            }

            std::set<OrderedEdge> overlappingRectangles;
            ComputeOverlappingRectangles(positions, edges, overlappingRectangles);
            for (auto key : overlappingRectangles)
            {
                // Get the endpoints of the line segments for the edges whose
                // bounding rectangles overlapped.  Determine whether the line
                // segments intersect.  If they do, determine how they
                // intersect.
                std::array<int, 2> e0 = edges[key.V[0]];
                std::array<int, 2> e1 = edges[key.V[1]];
                std::array<Real, 2> const& p0 = positions[e0[0]];
                std::array<Real, 2> const& p1 = positions[e0[1]];
                std::array<Real, 2> const& q0 = positions[e1[0]];
                std::array<Real, 2> const& q1 = positions[e1[1]];
                if (InvalidSegmentIntersection(p0, p1, q0, q1))
                {
                    mInvalidIntersections.push_back(key);
                }
            }

            if (mInvalidIntersections.size() > 0)
            {
                flags |= IPG_INVALID_INTERSECTIONS;
            }

            return flags;
        }

        // A pair of indices (v0,v1) into the positions array is stored as
        // (min(v0,v1), max(v0,v1)).  This supports sorted containers of
        // edges.
        struct OrderedEdge
        {
            OrderedEdge(int v0 = -1, int v1 = -1)
            {
                if (v0 < v1)
                {
                    // v0 is minimum
                    V[0] = v0;
                    V[1] = v1;
                }
                else
                {
                    // v1 is minimum
                    V[0] = v1;
                    V[1] = v0;
                }
            }

            bool operator<(OrderedEdge const& edge) const
            {
                // Lexicographical ordering used by std::array<int,2>.
                return V < edge.V;
            }

            std::array<int, 2> V;
        };

        inline std::vector<std::vector<int>> const& GetDuplicatedPositions() const
        {
            return mDuplicatedPositions;
        }

        inline std::vector<std::vector<int>> const& GetDuplicatedEdges() const
        {
            return mDuplicatedEdges;
        }

        inline std::vector<int> const& GetDegenerateEdges() const
        {
            return mDegenerateEdges;
        }

        inline std::vector<int> const& GetEdgesWithInvalidVertices() const
        {
            return mEdgesWithInvalidVertices;
        }

        inline std::vector<typename IsPlanarGraph<Real>::OrderedEdge> const&
        GetInvalidIntersections() const
        {
            return mInvalidIntersections;
        }

    private:
        class Endpoint
        {
        public:
            Real value;     // endpoint value
            int type;       // '0' if interval min, '1' if interval max.
            int index;      // index of interval containing this endpoint

            // Comparison operator for sorting.
            bool operator<(Endpoint const& endpoint) const
            {
                if (value < endpoint.value)
                {
                    return true;
                }
                if (value > endpoint.value)
                {
                    return false;
                }
                return type < endpoint.type;
            }
        };

        int IsValidTopology(std::vector<std::array<Real, 2>> const& positions,
            std::vector<std::array<int, 2>> const& edges)
        {
            int const numPositions = static_cast<int>(positions.size());
            int const numEdges = static_cast<int>(edges.size());
            if (numPositions < 2 || numEdges == 0)
            {
                // The graph must have at least one edge.
                return IPG_INVALID_INPUT_SIZES;
            }

            // The positions must be unique.
            int flags = IPG_IS_PLANAR_GRAPH;
            std::map<std::array<Real, 2>, std::vector<int>> uniquePositions;
            for (int i = 0; i < numPositions; ++i)
            {
                std::array<Real, 2> p = positions[i];
                auto iter = uniquePositions.find(p);
                if (iter == uniquePositions.end())
                {
                    std::vector<int> indices;
                    indices.push_back(i);
                    uniquePositions.insert(std::make_pair(p, indices));
                }
                else
                {
                    iter->second.push_back(i);
                }
            }
            if (uniquePositions.size() < positions.size())
            {
                // At least two positions are duplicated.
                for (auto const& element : uniquePositions)
                {
                    if (element.second.size() > 1)
                    {
                        mDuplicatedPositions.push_back(element.second);
                    }
                }
                flags |= IPG_DUPLICATED_POSITIONS;
            }

            // The edges must be unique.
            std::map<OrderedEdge, std::vector<int>> uniqueEdges;
            for (int i = 0; i < numEdges; ++i)
            {
                OrderedEdge key(edges[i][0], edges[i][1]);
                auto iter = uniqueEdges.find(key);
                if (iter == uniqueEdges.end())
                {
                    std::vector<int> indices;
                    indices.push_back(i);
                    uniqueEdges.insert(std::make_pair(key, indices));
                }
                else
                {
                    iter->second.push_back(i);
                }
            }
            if (uniqueEdges.size() < edges.size())
            {
                // At least two edges are duplicated, possibly even a pair of
                // edges (v0,v1) and (v1,v0) which is not allowed because the
                // graph is undirected.
                for (auto const& element : uniqueEdges)
                {
                    if (element.second.size() > 1)
                    {
                        mDuplicatedEdges.push_back(element.second);
                    }
                }
                flags |= IPG_DUPLICATED_EDGES;
            }

            // The edges are represented as pairs of indices into the
            // 'positions' array.  The indices for a single edge must be
            // different (no edges allowed from a vertex to itself) and all
            // indices must be valid.  At the same time, keep track of unique
            // edges.
            for (int i = 0; i < numEdges; ++i)
            {
                std::array<int, 2> e = edges[i];
                if (e[0] == e[1])
                {
                    // The edge is degenerate, originating and terminating at
                    // the same vertex.
                    mDegenerateEdges.push_back(i);
                    flags |= IPG_DEGENERATE_EDGES;
                }

                if (e[0] < 0 || e[0] >= numPositions || e[1] < 0 || e[1] >= numPositions)
                {
                    // The edge has an index that references a nonexistent
                    // vertex.
                    mEdgesWithInvalidVertices.push_back(i);
                    flags |= IPG_EDGES_WITH_INVALID_VERTICES;
                }
            }

            return flags;
        }

        void ComputeOverlappingRectangles(std::vector<std::array<Real, 2>> const& positions,
            std::vector<std::array<int, 2>> const& edges,
            std::set<OrderedEdge>& overlappingRectangles) const
        {
            // Compute axis-aligned bounding rectangles for the edges.
            int const numEdges = static_cast<int>(edges.size());
            std::vector<std::array<Real, 2>> emin(numEdges);
            std::vector<std::array<Real, 2>> emax(numEdges);
            for (int i = 0; i < numEdges; ++i)
            {
                std::array<int, 2> e = edges[i];
                std::array<Real, 2> const& p0 = positions[e[0]];
                std::array<Real, 2> const& p1 = positions[e[1]];

                for (int j = 0; j < 2; ++j)
                {
                    if (p0[j] < p1[j])
                    {
                        emin[i][j] = p0[j];
                        emax[i][j] = p1[j];
                    }
                    else
                    {
                        emin[i][j] = p1[j];
                        emax[i][j] = p0[j];
                    }
                }
            }

            // Get the rectangle endpoints.
            int const numEndpoints = 2 * numEdges;
            std::vector<Endpoint> xEndpoints(numEndpoints);
            std::vector<Endpoint> yEndpoints(numEndpoints);
            for (int i = 0, j = 0; i < numEdges; ++i)
            {
                xEndpoints[j].type = 0;
                xEndpoints[j].value = emin[i][0];
                xEndpoints[j].index = i;
                yEndpoints[j].type = 0;
                yEndpoints[j].value = emin[i][1];
                yEndpoints[j].index = i;
                ++j;

                xEndpoints[j].type = 1;
                xEndpoints[j].value = emax[i][0];
                xEndpoints[j].index = i;
                yEndpoints[j].type = 1;
                yEndpoints[j].value = emax[i][1];
                yEndpoints[j].index = i;
                ++j;
            }

            // Sort the rectangle endpoints.
            std::sort(xEndpoints.begin(), xEndpoints.end());
            std::sort(yEndpoints.begin(), yEndpoints.end());

            // Sweep through the endpoints to determine overlapping
            // x-intervals.  Use an active set of rectangles to reduce the
            // complexity of the algorithm.
            std::set<int> active;
            for (int i = 0; i < numEndpoints; ++i)
            {
                Endpoint const& endpoint = xEndpoints[i];
                int index = endpoint.index;
                if (endpoint.type == 0)  // an interval 'begin' value
                {
                    // In the 1D problem, the current interval overlaps with
                    // all the active intervals.  In 2D this we also need to
                    // check for y-overlap.
                    for (auto activeIndex : active)
                    {
                        // Rectangles activeIndex and index overlap in the
                        // x-dimension.  Test for overlap in the y-dimension.
                        std::array<Real, 2> const& r0min = emin[activeIndex];
                        std::array<Real, 2> const& r0max = emax[activeIndex];
                        std::array<Real, 2> const& r1min = emin[index];
                        std::array<Real, 2> const& r1max = emax[index];
                        if (r0max[1] >= r1min[1] && r0min[1] <= r1max[1])
                        {
                            if (activeIndex < index)
                            {
                                overlappingRectangles.insert(OrderedEdge(activeIndex, index));
                            }
                            else
                            {
                                overlappingRectangles.insert(OrderedEdge(index, activeIndex));
                            }
                        }
                    }
                    active.insert(index);
                }
                else  // an interval 'end' value
                {
                    active.erase(index);
                }
            }
        }

        bool InvalidSegmentIntersection(
            std::array<Real, 2> const& p0, std::array<Real, 2> const& p1,
            std::array<Real, 2> const& q0, std::array<Real, 2> const& q1) const
        {
            // We must solve the two linear equations
            //   p0 + t0 * (p1 - p0) = q0 + t1 * (q1 - q0)
            // for the unknown variables t0 and t1.  These may be written as
            //   t0 * (p1 - p0) - t1 * (q1 - q0) = q0 - p0
            // If denom = Dot(p1 - p0, Perp(q1 - q0)) is not zero, then
            //   t0 = Dot(q0 - p0, Perp(q1 - q0)) / denom = numer0 / denom
            //   t1 = Dot(q0 - p0, Perp(p1 - p0)) / denom = numer1 / denom
            // For an invalid intersection, we need (t0,t1) with:
            // ((0 < t0 < 1) and (0 <= t1 <= 1)) or ((0 <= t0 <= 1) and
            // (0 < t1 < 1).

            std::array<Real, 2> p1mp0, q1mq0, q0mp0;
            for (int j = 0; j < 2; ++j)
            {
                p1mp0[j] = p1[j] - p0[j];
                q1mq0[j] = q1[j] - q0[j];
                q0mp0[j] = q0[j] - p0[j];
            }

            Real denom = p1mp0[0] * q1mq0[1] - p1mp0[1] * q1mq0[0];
            Real numer0 = q0mp0[0] * q1mq0[1] - q0mp0[1] * q1mq0[0];
            Real numer1 = q0mp0[0] * p1mp0[1] - q0mp0[1] * p1mp0[0];

            if (denom != mZero)
            {
                // The lines of the segments are not parallel.
                if (denom > mZero)
                {
                    if (mZero <= numer0 && numer0 <= denom && mZero <= numer1 && numer1 <= denom)
                    {
                        // The segments intersect.
                        return (numer0 != mZero && numer0 != denom) || (numer1 != mZero && numer1 != denom);
                    }
                    else
                    {
                        return false;
                    }
                }
                else  // denom < mZero
                {
                    if (mZero >= numer0 && numer0 >= denom && mZero >= numer1 && numer1 >= denom)
                    {
                        // The segments intersect.
                        return (numer0 != mZero && numer0 != denom) || (numer1 != mZero && numer1 != denom);
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                // The lines of the segments are parallel.
                if (numer0 != mZero || numer1 != mZero)
                {
                    // The lines of the segments are separated.
                    return false;
                }
                else
                {
                    // The segments lie on the same line.  Compute the
                    // parameter intervals for the segments in terms of the
                    // t0-parameter and determine their overlap (if any).
                    std::array<Real, 2> q1mp0;
                    for (int j = 0; j < 2; ++j)
                    {
                        q1mp0[j] = q1[j] - p0[j];
                    }
                    Real sqrLenP1mP0 = p1mp0[0] * p1mp0[0] + p1mp0[1] * p1mp0[1];
                    Real value0 = q0mp0[0] * p1mp0[0] + q0mp0[1] * p1mp0[1];
                    Real value1 = q1mp0[0] * p1mp0[0] + q1mp0[1] * p1mp0[1];
                    if ((value0 >= sqrLenP1mP0 && value1 >= sqrLenP1mP0)
                        || (value0 <= mZero && value1 <= mZero))
                    {
                        // If the segments intersect, they must do so at
                        // endpoints of the segments.
                        return false;
                    }
                    else
                    {
                        // The segments overlap in a positive-length interval.
                        return true;
                    }
                }
            }
        }

        std::vector<std::vector<int>> mDuplicatedPositions;
        std::vector<std::vector<int>> mDuplicatedEdges;
        std::vector<int> mDegenerateEdges;
        std::vector<int> mEdgesWithInvalidVertices;
        std::vector<OrderedEdge> mInvalidIntersections;
        Real mZero, mOne;
    };
}
