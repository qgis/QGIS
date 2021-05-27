// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrSegment2Segment2.h>
#include <set>
#include <vector>

// The Polygon2 object represents a simple polygon:  No duplicate vertices,
// closed (each vertex is shared by exactly two edges), and no
// self-intersections at interior edge points.  The 'vertexPool' array can
// contain more points than needed to define the polygon, which allows the
// vertex pool to have multiple polygons associated with it.  Thus, the
// programmer must ensure that the vertex pool persists as long as any
// Polygon2 objects exist that depend on the pool.  The number of polygon
// vertices is 'numIndices' and must be 3 or larger.  The 'indices' array
// refers to the points in 'vertexPool' that are part of the polygon and must
// have 'numIndices' unique elements.  The edges of the polygon are pairs of
// indices into 'vertexPool',
//   edge[0] = (indices[0], indices[1])
//   :
//   edge[numIndices-2] = (indices[numIndices-2], indices[numIndices-1])
//   edge[numIndices-1] = (indices[numIndices-1], indices[0])
// The programmer should ensure the polygon is simple.  The geometric
// queries are valid regardless of whether the polygon is oriented clockwise
// or counterclockwise.
//
// NOTE: Comparison operators are not provided.  The semantics of equal
// polygons is complicated and (at the moment) not useful.  The vertex pools
// can be different and indices do not match, but the vertices they reference
// can match.  Even with a shared vertex pool, the indices can be "rotated",
// leading to the same polygon abstractly but the data structures do not
// match.

namespace gte
{
    template <typename Real>
    class Polygon2
    {
    public:
        // Construction.  The constructor succeeds when 'numIndices' >= 3 and
        // 'vertexPool' and 'indices' are not null; we cannot test whether you
        // have a valid number of elements in the input arrays.  A copy is
        // made of 'indices', but the 'vertexPool' is not copied.  If the
        // constructor fails, the internal vertex pointer is set to null, the
        // index array has no elements, and the orientation is set to
        // clockwise.
        Polygon2(Vector2<Real> const* vertexPool, int numIndices,
            int const* indices, bool counterClockwise)
            :
            mVertexPool(vertexPool),
            mCounterClockwise(counterClockwise)
        {
            if (numIndices >= 3 && vertexPool && indices)
            {
                for (int i = 0; i < numIndices; ++i)
                {
                    mVertices.insert(indices[i]);
                }

                if (numIndices == static_cast<int>(mVertices.size()))
                {
                    mIndices.resize(numIndices);
                    std::copy(indices, indices + numIndices, mIndices.begin());
                    return;
                }

                // At least one duplicated vertex was encountered, so the
                // polygon is not simple.  Fail the constructor call.
                mVertices.clear();
            }

            // Invalid input to the Polygon2 constructor.
            mVertexPool = nullptr;
            mCounterClockwise = false;
        }

        // To validate construction, create an object as shown:
        //     Polygon2<Real> polygon(parameters);
        //     if (!polygon) { <constructor failed, handle accordingly>; }
        inline operator bool() const
        {
            return mVertexPool != nullptr;
        }

        // Member access.
        inline Vector2<Real> const* GetVertexPool() const
        {
            return mVertexPool;
        }

        inline std::set<int> const& GetVertices() const
        {
            return mVertices;
        }

        inline std::vector<int> const& GetIndices() const
        {
            return mIndices;
        }

        inline bool CounterClockwise() const
        {
            return mCounterClockwise;
        }

        // Geometric queries.
        Vector2<Real> ComputeVertexAverage() const
        {
            Vector2<Real> average = Vector2<Real>::Zero();
            if (mVertexPool)
            {
                for (int index : mVertices)
                {
                    average += mVertexPool[index];
                }
                average /= static_cast<Real>(mVertices.size());
            }
            return average;
        }

        Real ComputePerimeterLength() const
        {
            Real length(0);
            if (mVertexPool)
            {
                Vector2<Real> v0 = mVertexPool[mIndices.back()];
                for (int index : mIndices)
                {
                    Vector2<Real> v1 = mVertexPool[index];
                    length += Length(v1 - v0);
                    v0 = v1;
                }
            }
            return length;
        }

        Real ComputeArea() const
        {
            Real area(0);
            if (mVertexPool)
            {
                int const numIndices = static_cast<int>(mIndices.size());
                Vector2<Real> v0 = mVertexPool[mIndices[numIndices - 2]];
                Vector2<Real> v1 = mVertexPool[mIndices[numIndices - 1]];
                for (int index : mIndices)
                {
                    Vector2<Real> v2 = mVertexPool[index];
                    area += v1[0] * (v2[1] - v0[1]);
                    v0 = v1;
                    v1 = v2;
                }
                area *= (Real)0.5;
            }
            return std::fabs(area);
        }

        // Simple polygons have no self-intersections at interior points
        // of edges.  The test is an exhaustive all-pairs intersection
        // test for edges, which is inefficient for polygons with a large
        // number of vertices.  TODO: Provide an efficient algorithm that
        // uses the algorithm of class RectangleManager.h.
        bool IsSimple() const
        {
            if (!mVertexPool)
            {
                return false;
            }

            // For mVertexPool to be nonnull, the number of indices is
            // guaranteed to be at least 3.
            int const numIndices = static_cast<int>(mIndices.size());
            if (numIndices == 3)
            {
                // The polygon is a triangle.
                return true;
            }

            return IsSimpleInternal();
        }

        // Convex polygons are simple polygons where the angles between
        // consecutive edges are less than or equal to pi radians.
        bool IsConvex() const
        {
            if (!mVertexPool)
            {
                return false;
            }

            // For mVertexPool to be nonnull, the number of indices is
            // guaranteed to be at least 3.
            int const numIndices = static_cast<int>(mIndices.size());
            if (numIndices == 3)
            {
                // The polygon is a triangle.
                return true;
            }

            return IsSimpleInternal() && IsConvexInternal();
        }

    private:
        // These calls have preconditions that mVertexPool is not null and
        // mIndices.size() > 3.  The heart of the algorithms are implemented
        // here.
        bool IsSimpleInternal() const
        {
            Segment2<Real> seg0, seg1;
            TIQuery<Real, Segment2<Real>, Segment2<Real>> query;
            typename TIQuery<Real, Segment2<Real>, Segment2<Real>>::Result result;

            int const numIndices = static_cast<int>(mIndices.size());
            for (int i0 = 0; i0 < numIndices; ++i0)
            {
                int i0p1 = (i0 + 1) % numIndices;
                seg0.p[0] = mVertexPool[mIndices[i0]];
                seg0.p[1] = mVertexPool[mIndices[i0p1]];

                int i1min = (i0 + 2) % numIndices;
                int i1max = (i0 - 2 + numIndices) % numIndices;
                for (int i1 = i1min; i1 <= i1max; ++i1)
                {
                    int i1p1 = (i1 + 1) % numIndices;
                    seg1.p[0] = mVertexPool[mIndices[i1]];
                    seg1.p[1] = mVertexPool[mIndices[i1p1]];

                    result = query(seg0, seg1);
                    if (result.intersect)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        bool IsConvexInternal() const
        {
            Real sign = (mCounterClockwise ? (Real)1 : (Real)-1);
            int const numIndices = static_cast<int>(mIndices.size());
            for (int i = 0; i < numIndices; ++i)
            {
                int iPrev = (i + numIndices - 1) % numIndices;
                int iNext = (i + 1) % numIndices;
                Vector2<Real> vPrev = mVertexPool[mIndices[iPrev]];
                Vector2<Real> vCurr = mVertexPool[mIndices[i]];
                Vector2<Real> vNext = mVertexPool[mIndices[iNext]];
                Vector2<Real> edge0 = vCurr - vPrev;
                Vector2<Real> edge1 = vNext - vCurr;
                Real test = sign * DotPerp(edge0, edge1);
                if (test < (Real)0)
                {
                    return false;
                }
            }
            return true;
        }

        Vector2<Real> const* mVertexPool;
        std::set<int> mVertices;
        std::vector<int> mIndices;
        bool mCounterClockwise;
    };
}
