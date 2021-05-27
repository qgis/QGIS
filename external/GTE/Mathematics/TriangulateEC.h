// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.10.26

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/PolygonTree.h>
#include <Mathematics/PrimalQuery2.h>
#include <memory>
#include <map>
#include <queue>
#include <vector>

// Triangulate polygons using ear clipping.  The algorithm is described in
// https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
// The algorithm for processing nested polygons involves a division, so the
// ComputeType must be rational-based, say, BSRational.  If you process only
// triangles that are simple, you may use BSNumber for the ComputeType.

namespace gte
{
    template <typename InputType, typename ComputeType>
    class TriangulateEC
    {
    public:
        // The fundamental problem is to compute the triangulation of a
        // polygon tree. The outer polygons have counterclockwise ordered
        // vertices. The inner polygons have clockwise ordered vertices.
        typedef std::vector<int> Polygon;

        // The class is a functor to support triangulating multiple polygons
        // that share vertices in a collection of points.  The interpretation
        // of 'numPoints' and 'points' is described before each operator()
        // function.  Preconditions are numPoints >= 3 and points is a nonnull
        // pointer to an array of at least numPoints elements.  If the
        // preconditions are satisfied, then operator() functions will return
        // 'true'; otherwise, they return 'false'.
        TriangulateEC(int numPoints, Vector2<InputType> const* points)
            :
            mNumPoints(numPoints),
            mPoints(points),
            mCFirst(-1),
            mCLast(-1),
            mRFirst(-1),
            mRLast(-1),
            mEFirst(-1),
            mELast(-1)
        {
            LogAssert(numPoints >= 3 && points != nullptr, "Invalid input.");
            mComputePoints.resize(mNumPoints);
            mIsConverted.resize(mNumPoints);
            std::fill(mIsConverted.begin(), mIsConverted.end(), false);
            mQuery.Set(mNumPoints, &mComputePoints[0]);
        }

        TriangulateEC(std::vector<Vector2<InputType>> const& points)
            :
            mNumPoints(static_cast<int>(points.size())),
            mPoints(points.data()),
            mCFirst(-1),
            mCLast(-1),
            mRFirst(-1),
            mRLast(-1),
            mEFirst(-1),
            mELast(-1)
        {
            LogAssert(mNumPoints >= 3 && mPoints != nullptr, "Invalid input.");
            mComputePoints.resize(mNumPoints);
            mIsConverted.resize(mNumPoints);
            std::fill(mIsConverted.begin(), mIsConverted.end(), false);
            mQuery.Set(mNumPoints, &mComputePoints[0]);
        }

        // Access the triangulation after each operator() call.
        inline std::vector<std::array<int, 3>> const& GetTriangles() const
        {
            return mTriangles;
        }

        // The input 'points' represents an array of vertices for a simple
        // polygon. The vertices are points[0] through points[numPoints-1] and
        // are listed in counterclockwise order.
        bool operator()()
        {
            mTriangles.clear();
            if (mPoints)
            {
                // Compute the points for the queries.
                for (int i = 0; i < mNumPoints; ++i)
                {
                    if (!mIsConverted[i])
                    {
                        mIsConverted[i] = true;
                        for (int j = 0; j < 2; ++j)
                        {
                            mComputePoints[i][j] = mPoints[i][j];
                        }
                    }
                }

                // Triangulate the unindexed polygon.
                InitializeVertices(mNumPoints, nullptr);
                DoEarClipping(mNumPoints, nullptr);
                return true;
            }
            else
            {
                return false;
            }
        }

        // The input 'points' represents an array of vertices that contains
        // the vertices of a simple polygon.
        bool operator()(Polygon const& polygon)
        {
            mTriangles.clear();
            if (mPoints)
            {
                // Compute the points for the queries.
                int const numIndices = static_cast<int>(polygon.size());
                int const* indices = polygon.data();
                for (int i = 0; i < numIndices; ++i)
                {
                    int index = indices[i];
                    if (!mIsConverted[index])
                    {
                        mIsConverted[index] = true;
                        for (int j = 0; j < 2; ++j)
                        {
                            mComputePoints[index][j] = mPoints[index][j];
                        }
                    }
                }

                // Triangulate the indexed polygon.
                InitializeVertices(numIndices, indices);
                DoEarClipping(numIndices, indices);
                return true;
            }
            else
            {
                return false;
            }
        }

        // The input 'points' is a shared array of vertices that contains the
        // vertices for two simple polygons, an outer polygon and an inner
        // polygon.  The inner polygon must be strictly inside the outer
        // polygon.
        bool operator()(Polygon const& outer, Polygon const& inner)
        {
            mTriangles.clear();
            if (mPoints)
            {
                // Two extra elements are needed to duplicate the endpoints of
                // the edge introduced to combine outer and inner polygons.
                int numPointsPlusExtras = mNumPoints + 2;
                if (numPointsPlusExtras > static_cast<int>(mComputePoints.size()))
                {
                    mComputePoints.resize(numPointsPlusExtras);
                    mIsConverted.resize(numPointsPlusExtras);
                    mIsConverted[mNumPoints] = false;
                    mIsConverted[mNumPoints + 1] = false;
                    mQuery.Set(numPointsPlusExtras, &mComputePoints[0]);
                }

                // Convert any points that have not been encountered in other
                // triangulation calls.
                int const numOuterIndices = static_cast<int>(outer.size());
                int const* outerIndices = outer.data();
                for (int i = 0; i < numOuterIndices; ++i)
                {
                    int index = outerIndices[i];
                    if (!mIsConverted[index])
                    {
                        mIsConverted[index] = true;
                        for (int j = 0; j < 2; ++j)
                        {
                            mComputePoints[index][j] = mPoints[index][j];
                        }
                    }
                }

                int const numInnerIndices = static_cast<int>(inner.size());
                int const* innerIndices = inner.data();
                for (int i = 0; i < numInnerIndices; ++i)
                {
                    int index = innerIndices[i];
                    if (!mIsConverted[index])
                    {
                        mIsConverted[index] = true;
                        for (int j = 0; j < 2; ++j)
                        {
                            mComputePoints[index][j] = mPoints[index][j];
                        }
                    }
                }

                // Combine the outer polygon and the inner polygon into a
                // simple polygon by inserting two edges connecting mutually
                // visible vertices, one from the outer polygon and one from
                // the inner polygon.
                int nextElement = mNumPoints;  // The next available element.
                std::map<int, int> indexMap;
                std::vector<int> combined;
                if (!CombinePolygons(nextElement, outer, inner, indexMap, combined))
                {
                    // An unexpected condition was encountered.
                    return false;
                }

                // The combined polygon is now in the format of a simple
                // polygon, albeit one with coincident edges.
                int numVertices = static_cast<int>(combined.size());
                int* const indices = &combined[0];
                InitializeVertices(numVertices, indices);
                DoEarClipping(numVertices, indices);

                // Map the duplicate indices back to the original indices.
                RemapIndices(indexMap);
                return true;
            }
            else
            {
                return false;
            }
        }

        // The input 'points' is a shared array of vertices that contains the
        // vertices for multiple simple polygons, an outer polygon and one or
        // more inner polygons.  The inner polygons must be nonoverlapping and
        // strictly inside the outer polygon.
        bool operator()(Polygon const& outer, std::vector<Polygon> const& inners)
        {
            mTriangles.clear();
            if (mPoints)
            {
                // Two extra elements per inner polygon are needed to
                // duplicate the endpoints of the edges introduced to combine
                // outer and inner polygons.
                int numPointsPlusExtras = mNumPoints + 2 * (int)inners.size();
                if (numPointsPlusExtras > static_cast<int>(mComputePoints.size()))
                {
                    mComputePoints.resize(numPointsPlusExtras);
                    mIsConverted.resize(numPointsPlusExtras);
                    for (int i = mNumPoints; i < numPointsPlusExtras; ++i)
                    {
                        mIsConverted[i] = false;
                    }
                    mQuery.Set(numPointsPlusExtras, &mComputePoints[0]);
                }

                // Convert any points that have not been encountered in other
                // triangulation calls.
                int const numOuterIndices = static_cast<int>(outer.size());
                int const* outerIndices = outer.data();
                for (int i = 0; i < numOuterIndices; ++i)
                {
                    int index = outerIndices[i];
                    if (!mIsConverted[index])
                    {
                        mIsConverted[index] = true;
                        for (int j = 0; j < 2; ++j)
                        {
                            mComputePoints[index][j] = mPoints[index][j];
                        }
                    }
                }

                for (auto const& inner : inners)
                {
                    int const numInnerIndices = static_cast<int>(inner.size());
                    int const* innerIndices = inner.data();
                    for (int i = 0; i < numInnerIndices; ++i)
                    {
                        int index = innerIndices[i];
                        if (!mIsConverted[index])
                        {
                            mIsConverted[index] = true;
                            for (int j = 0; j < 2; ++j)
                            {
                                mComputePoints[index][j] = mPoints[index][j];
                            }
                        }
                    }
                }

                // Combine the outer polygon and the inner polygons into a
                // simple polygon by inserting two edges per inner polygon
                // connecting mutually visible vertices.
                int nextElement = mNumPoints;  // The next available element.
                std::map<int, int> indexMap;
                std::vector<int> combined;
                if (!ProcessOuterAndInners(nextElement, outer, inners, indexMap, combined))
                {
                    // An unexpected condition was encountered.
                    return false;
                }

                // The combined polygon is now in the format of a simple
                // polygon, albeit with coincident edges.
                int numVertices = static_cast<int>(combined.size());
                int* const indices = &combined[0];
                InitializeVertices(numVertices, indices);
                DoEarClipping(numVertices, indices);

                // Map the duplicate indices back to the original indices.
                RemapIndices(indexMap);
                return true;
            }
            else
            {
                return false;
            }
        }

        // The input 'positions' is a shared array of vertices that contains
        // the vertices for multiple simple polygons in a tree of polygons.
        bool operator()(std::shared_ptr<PolygonTree> const& tree)
        {
            mTriangles.clear();
            if (mPoints)
            {
                // Two extra elements per inner polygon are needed to
                // duplicate the endpoints of the edges introduced to combine
                // outer and inner polygons.
                int numPointsPlusExtras = mNumPoints + InitializeFromTree(tree);
                if (numPointsPlusExtras > static_cast<int>(mComputePoints.size()))
                {
                    mComputePoints.resize(numPointsPlusExtras);
                    mIsConverted.resize(numPointsPlusExtras);
                    for (int i = mNumPoints; i < numPointsPlusExtras; ++i)
                    {
                        mIsConverted[i] = false;
                    }
                    mQuery.Set(numPointsPlusExtras, &mComputePoints[0]);
                }

                int nextElement = mNumPoints;
                std::map<int, int> indexMap;

                std::queue<std::shared_ptr<PolygonTree>> treeQueue;
                treeQueue.push(tree);
                while (treeQueue.size() > 0)
                {
                    std::shared_ptr<PolygonTree> outer = treeQueue.front();
                    treeQueue.pop();

                    int numChildren = static_cast<int>(outer->child.size());
                    int numVertices;
                    int const* indices;

                    if (numChildren == 0)
                    {
                        // The outer polygon is a simple polygon (no nested
                        // inner polygons).  Triangulate the simple polygon.
                        numVertices = static_cast<int>(outer->polygon.size());
                        indices = outer->polygon.data();
                        InitializeVertices(numVertices, indices);
                        DoEarClipping(numVertices, indices);
                    }
                    else
                    {
                        // Place the next level of outer polygon nodes on the
                        // queue for triangulation.
                        std::vector<Polygon> inners(numChildren);
                        for (int c = 0; c < numChildren; ++c)
                        {
                            std::shared_ptr<PolygonTree> inner = outer->child[c];
                            inners[c] = inner->polygon;
                            int numGrandChildren = static_cast<int>(inner->child.size());
                            for (int g = 0; g < numGrandChildren; ++g)
                            {
                                treeQueue.push(inner->child[g]);
                            }
                        }

                        // Combine the outer polygon and the inner polygons
                        // into a simple polygon by inserting two edges per
                        // inner polygon connecting mutually visible vertices.
                        std::vector<int> combined;
                        ProcessOuterAndInners(nextElement, outer->polygon, inners, indexMap, combined);

                        // The combined polygon is now in the format of a
                        // simple polygon, albeit with coincident edges.
                        numVertices = static_cast<int>(combined.size());
                        indices = &combined[0];
                        InitializeVertices(numVertices, indices);
                        DoEarClipping(numVertices, indices);
                    }
                }

                // Map the duplicate indices back to the original indices.
                RemapIndices(indexMap);
                return true;
            }
            else
            {
                return false;
            }
        }

    private:
        // Create the vertex objects that store the various lists required by
        // the ear-clipping algorithm.
        void InitializeVertices(int numVertices, int const* indices)
        {
            mVertices.clear();
            mVertices.resize(numVertices);
            mCFirst = -1;
            mCLast = -1;
            mRFirst = -1;
            mRLast = -1;
            mEFirst = -1;
            mELast = -1;

            // Create a circular list of the polygon vertices for dynamic
            // removal of vertices.
            int numVerticesM1 = numVertices - 1;
            for (int i = 0; i <= numVerticesM1; ++i)
            {
                Vertex& vertex = V(i);
                vertex.index = (indices ? indices[i] : i);
                vertex.vPrev = (i > 0 ? i - 1 : numVerticesM1);
                vertex.vNext = (i < numVerticesM1 ? i + 1 : 0);
            }

            // Create a circular list of the polygon vertices for dynamic
            // removal of vertices.  Keep track of two linear sublists, one
            // for the convex vertices and one for the reflex vertices.
            // This is an O(N) process where N is the number of polygon
            // vertices.
            for (int i = 0; i <= numVerticesM1; ++i)
            {
                if (IsConvex(i))
                {
                    InsertAfterC(i);
                }
                else
                {
                    InsertAfterR(i);
                }
            }
        }

        // Apply ear clipping to the input polygon.  Polygons with holes are
        // preprocessed to obtain an index array that is nearly a simple
        // polygon.  This outer polygon has a pair of coincident edges per
        // inner polygon.
        void DoEarClipping(int numVertices, int const* indices)
        {
            // If the polygon is convex, just create a triangle fan.
            if (mRFirst == -1)
            {
                int numVerticesM1 = numVertices - 1;
                if (indices)
                {
                    for (int i = 1; i < numVerticesM1; ++i)
                    {
                        mTriangles.push_back( { indices[0], indices[i], indices[i + 1] } );
                    }
                }
                else
                {
                    for (int i = 1; i < numVerticesM1; ++i)
                    {
                        mTriangles.push_back( { 0, i, i + 1 } );
                    }
                }
                return;
            }

            // Identify the ears and build a circular list of them.  Let V0,
            // V1, and V2 be consecutive vertices forming a triangle T.  The
            // vertex V1 is an ear if no other vertices of the polygon lie
            // inside T.  Although it is enough to show that V1 is not an ear
            // by finding at least one other vertex inside T, it is sufficient
            // to search only the reflex vertices.  This is an O(C*R) process,
            // where C is the number of convex vertices and R is the number of
            // reflex vertices with N = C+R.  The order is O(N^2), for example
            // when C = R = N/2.
            for (int i = mCFirst; i != -1; i = V(i).sNext)
            {
                if (IsEar(i))
                {
                    InsertEndE(i);
                }
            }
            V(mEFirst).ePrev = mELast;
            V(mELast).eNext = mEFirst;

            // Remove the ears, one at a time.
            bool bRemoveAnEar = true;
            while (bRemoveAnEar)
            {
                // Add the triangle with the ear to the output list of
                // triangles.
                int iVPrev = V(mEFirst).vPrev;
                int iVNext = V(mEFirst).vNext;
                mTriangles.push_back( { V(iVPrev).index, V(mEFirst).index, V(iVNext).index } );

                // Remove the vertex corresponding to the ear.
                RemoveV(mEFirst);
                if (--numVertices == 3)
                {
                    // Only one triangle remains, just remove the ear and
                    // copy it.
                    mEFirst = RemoveE(mEFirst);
                    iVPrev = V(mEFirst).vPrev;
                    iVNext = V(mEFirst).vNext;
                    mTriangles.push_back( { V(iVPrev).index, V(mEFirst).index, V(iVNext).index } );
                    bRemoveAnEar = false;
                    continue;
                }

                // Removal of the ear can cause an adjacent vertex to become
                // an ear or to stop being an ear.
                Vertex& vPrev = V(iVPrev);
                if (vPrev.isEar)
                {
                    if (!IsEar(iVPrev))
                    {
                        RemoveE(iVPrev);
                    }
                }
                else
                {
                    bool wasReflex = !vPrev.isConvex;
                    if (IsConvex(iVPrev))
                    {
                        if (wasReflex)
                        {
                            RemoveR(iVPrev);
                        }

                        if (IsEar(iVPrev))
                        {
                            InsertBeforeE(iVPrev);
                        }
                    }
                }

                Vertex& vNext = V(iVNext);
                if (vNext.isEar)
                {
                    if (!IsEar(iVNext))
                    {
                        RemoveE(iVNext);
                    }
                }
                else
                {
                    bool wasReflex = !vNext.isConvex;
                    if (IsConvex(iVNext))
                    {
                        if (wasReflex)
                        {
                            RemoveR(iVNext);
                        }

                        if (IsEar(iVNext))
                        {
                            InsertAfterE(iVNext);
                        }
                    }
                }

                // Remove the ear.
                mEFirst = RemoveE(mEFirst);
            }
        }

        // Given an outer polygon that contains an inner polygon, this
        // function determines a pair of visible vertices and inserts two
        // coincident edges to generate a nearly simple polygon.
        bool CombinePolygons(int nextElement, Polygon const& outer,
            Polygon const& inner, std::map<int, int>& indexMap,
            std::vector<int>& combined)
        {
            int const numOuterIndices = static_cast<int>(outer.size());
            int const* outerIndices = outer.data();
            int const numInnerIndices = static_cast<int>(inner.size());
            int const* innerIndices = inner.data();

            // Locate the inner-polygon vertex of maximum x-value, call this
            // vertex M.
            ComputeType xmax = mComputePoints[innerIndices[0]][0];
            int xmaxIndex = 0;
            for (int i = 1; i < numInnerIndices; ++i)
            {
                ComputeType x = mComputePoints[innerIndices[i]][0];
                if (x > xmax)
                {
                    xmax = x;
                    xmaxIndex = i;
                }
            }
            Vector2<ComputeType> M = mComputePoints[innerIndices[xmaxIndex]];

            // Find the edge whose intersection Intr with the ray M+t*(1,0)
            // minimizes
            // the ray parameter t >= 0.
            ComputeType const cmax = static_cast<ComputeType>(std::numeric_limits<InputType>::max());
            ComputeType const zero = static_cast<ComputeType>(0);
            Vector2<ComputeType> intr{ cmax, M[1] };
            int v0min = -1, v1min = -1, endMin = -1;
            int i0, i1;
            ComputeType s = cmax;
            ComputeType t = cmax;
            for (i0 = numOuterIndices - 1, i1 = 0; i1 < numOuterIndices; i0 = i1++)
            {
                // Consider only edges for which the first vertex is below
                // (or on) the ray and the second vertex is above (or on)
                // the ray.
                Vector2<ComputeType> diff0 = mComputePoints[outerIndices[i0]] - M;
                if (diff0[1] > zero)
                {
                    continue;
                }
                Vector2<ComputeType> diff1 = mComputePoints[outerIndices[i1]] - M;
                if (diff1[1] < zero)
                {
                    continue;
                }

                // At this time, diff0.y <= 0 and diff1.y >= 0.
                int currentEndMin = -1;
                if (diff0[1] < zero)
                {
                    if (diff1[1] > zero)
                    {
                        // The intersection of the edge and ray occurs at an
                        // interior edge point.
                        s = diff0[1] / (diff0[1] - diff1[1]);
                        t = diff0[0] + s * (diff1[0] - diff0[0]);
                    }
                    else  // diff1.y == 0
                    {
                        // The vertex Outer[i1] is the intersection of the
                        // edge and the ray.
                        t = diff1[0];
                        currentEndMin = i1;
                    }
                }
                else  // diff0.y == 0
                {
                    if (diff1[1] > zero)
                    {
                        // The vertex Outer[i0] is the intersection of the
                        // edge and the ray;
                        t = diff0[0];
                        currentEndMin = i0;
                    }
                    else  // diff1.y == 0
                    {
                        if (diff0[0] < diff1[0])
                        {
                            t = diff0[0];
                            currentEndMin = i0;
                        }
                        else
                        {
                            t = diff1[0];
                            currentEndMin = i1;
                        }
                    }
                }

                if (zero <= t && t < intr[0])
                {
                    intr[0] = t;
                    v0min = i0;
                    v1min = i1;
                    if (currentEndMin == -1)
                    {
                        // The current closest point is an edge-interior
                        // point.
                        endMin = -1;
                    }
                    else
                    {
                        // The current closest point is a vertex.
                        endMin = currentEndMin;
                    }
                }
                else if (t == intr[0])
                {
                    // The current closest point is a vertex shared by
                    // multiple edges; thus, endMin and currentMin refer to
                    // the same point.
                    LogAssert(endMin != -1 && currentEndMin != -1, "Unexpected condition.");

                    // We need to select the edge closest to M.  The previous
                    // closest edge is <outer[v0min],outer[v1min]>.  The
                    // current candidate is <outer[i0],outer[i1]>.
                    Vector2<ComputeType> shared = mComputePoints[outerIndices[i1]];

                    // For the previous closest edge, endMin refers to a
                    // vertex of the edge.  Get the index of the other vertex.
                    int other = (endMin == v0min ? v1min : v0min);

                    // The new edge is closer if the other vertex of the old
                    // edge is left-of the new edge.
                    diff0 = mComputePoints[outerIndices[i0]] - shared;
                    diff1 = mComputePoints[outerIndices[other]] - shared;
                    ComputeType dotperp = DotPerp(diff0, diff1);
                    if (dotperp > zero)
                    {
                        // The new edge is closer to M.
                        v0min = i0;
                        v1min = i1;
                        endMin = currentEndMin;
                    }
                }
            }

            // The intersection intr[0] stored only the t-value of the ray.
            // The actual point is (mx,my)+t*(1,0), so intr[0] must be
            // adjusted.
            intr[0] += M[0];

            int maxCosIndex;
            if (endMin == -1)
            {
                // If you reach this assert, there is a good chance that you
                // have two inner polygons that share a vertex or an edge.
                LogAssert(v0min >= 0 && v1min >= 0, "Is this an invalid nested polygon?");

                // Select one of Outer[v0min] and Outer[v1min] that has an
                // x-value larger than M.x, call this vertex P.  The triangle
                // <M,I,P> must contain an outer-polygon vertex that is
                // visible to M, which is possibly P itself.
                Vector2<ComputeType> sTriangle[3];  // <P,M,I> or <P,I,M>
                int pIndex;
                if (mComputePoints[outerIndices[v0min]][0] > mComputePoints[outerIndices[v1min]][0])
                {
                    sTriangle[0] = mComputePoints[outerIndices[v0min]];
                    sTriangle[1] = intr;
                    sTriangle[2] = M;
                    pIndex = v0min;
                }
                else
                {
                    sTriangle[0] = mComputePoints[outerIndices[v1min]];
                    sTriangle[1] = M;
                    sTriangle[2] = intr;
                    pIndex = v1min;
                }

                // If any outer-polygon vertices other than P are inside the
                // triangle <M,I,P>, then at least one of these vertices must
                // be a reflex vertex.  It is sufficient to locate the reflex
                // vertex R (if any) in <M,I,P> that minimizes the angle
                // between R-M and (1,0).  The data member mQuery is used for
                // the reflex query.
                Vector2<ComputeType> diff = sTriangle[0] - M;
                ComputeType maxSqrLen = Dot(diff, diff);
                ComputeType maxCos = diff[0] * diff[0] / maxSqrLen;
                PrimalQuery2<ComputeType> localQuery(3, sTriangle);
                maxCosIndex = pIndex;
                for (int i = 0; i < numOuterIndices; ++i)
                {
                    if (i == pIndex)
                    {
                        continue;
                    }

                    int curr = outerIndices[i];
                    int prev = outerIndices[(i + numOuterIndices - 1) % numOuterIndices];
                    int next = outerIndices[(i + 1) % numOuterIndices];
                    if (mQuery.ToLine(curr, prev, next) <= 0
                        && localQuery.ToTriangle(mComputePoints[curr], 0, 1, 2) <= 0)
                    {
                        // The vertex is reflex and inside the <M,I,P>
                        // triangle.
                        diff = mComputePoints[curr] - M;
                        ComputeType sqrLen = Dot(diff, diff);
                        ComputeType cs = diff[0] * diff[0] / sqrLen;
                        if (cs > maxCos)
                        {
                            // The reflex vertex forms a smaller angle with
                            // the positive x-axis, so it becomes the new
                            // visible candidate.
                            maxSqrLen = sqrLen;
                            maxCos = cs;
                            maxCosIndex = i;
                        }
                        else if (cs == maxCos && sqrLen < maxSqrLen)
                        {
                            // The reflex vertex has angle equal to the
                            // current minimum but the length is smaller, so
                            // it becomes the new visible candidate.
                            maxSqrLen = sqrLen;
                            maxCosIndex = i;
                        }
                    }
                }
            }
            else
            {
                maxCosIndex = endMin;
            }

            // The visible vertices are Position[Inner[xmaxIndex]] and
            // Position[Outer[maxCosIndex]].  Two coincident edges with
            // these endpoints are inserted to connect the outer and inner
            // polygons into a simple polygon.  Each of the two Position[]
            // values must be duplicated, because the original might be
            // convex (or reflex) and the duplicate is reflex (or convex).
            // The ear-clipping algorithm needs to distinguish between them.
            combined.resize(numOuterIndices + numInnerIndices + 2);
            int cIndex = 0;
            for (int i = 0; i <= maxCosIndex; ++i, ++cIndex)
            {
                combined[cIndex] = outerIndices[i];
            }

            for (int i = 0; i < numInnerIndices; ++i, ++cIndex)
            {
                int j = (xmaxIndex + i) % numInnerIndices;
                combined[cIndex] = innerIndices[j];
            }

            int innerIndex = innerIndices[xmaxIndex];
            mComputePoints[nextElement] = mComputePoints[innerIndex];
            combined[cIndex] = nextElement;
            auto iter = indexMap.find(innerIndex);
            if (iter != indexMap.end())
            {
                innerIndex = iter->second;
            }
            indexMap[nextElement] = innerIndex;
            ++cIndex;
            ++nextElement;

            int outerIndex = outerIndices[maxCosIndex];
            mComputePoints[nextElement] = mComputePoints[outerIndex];
            combined[cIndex] = nextElement;
            iter = indexMap.find(outerIndex);
            if (iter != indexMap.end())
            {
                outerIndex = iter->second;
            }
            indexMap[nextElement] = outerIndex;
            ++cIndex;
            ++nextElement;

            for (int i = maxCosIndex + 1; i < numOuterIndices; ++i, ++cIndex)
            {
                combined[cIndex] = outerIndices[i];
            }
            return true;
        }

        // Given an outer polygon that contains a set of nonoverlapping inner
        // polygons, this function determines pairs of visible vertices and
        // inserts coincident edges to generate a nearly simple polygon.  It
        // repeatedly calls CombinePolygons for each inner polygon of the
        // outer polygon.
        bool ProcessOuterAndInners(int& nextElement, Polygon const& outer,
            std::vector<Polygon> const& inners, std::map<int, int>& indexMap,
            std::vector<int>& combined)
        {
            // Sort the inner polygons based on maximum x-values.
            int numInners = static_cast<int>(inners.size());
            std::vector<std::pair<ComputeType, int>> pairs(numInners);
            for (int p = 0; p < numInners; ++p)
            {
                int numIndices = static_cast<int>(inners[p].size());
                int const* indices = inners[p].data();
                ComputeType xmax = mComputePoints[indices[0]][0];
                for (int j = 1; j < numIndices; ++j)
                {
                    ComputeType x = mComputePoints[indices[j]][0];
                    if (x > xmax)
                    {
                        xmax = x;
                    }
                }
                pairs[p].first = xmax;
                pairs[p].second = p;
            }
            std::sort(pairs.begin(), pairs.end());

            // Merge the inner polygons with the outer polygon.
            Polygon currentPolygon = outer;
            for (int p = numInners - 1; p >= 0; --p)
            {
                Polygon const& polygon = inners[pairs[p].second];
                Polygon currentCombined;
                if (!CombinePolygons(nextElement, currentPolygon, polygon, indexMap, currentCombined))
                {
                    return false;
                }
                currentPolygon = std::move(currentCombined);
                nextElement += 2;
            }

            for (auto index : currentPolygon)
            {
                combined.push_back(index);
            }
            return true;
        }

        // The insertion of coincident edges to obtain a nearly simple polygon
        // requires duplication of vertices in order that the ear-clipping
        // algorithm work correctly.  After the triangulation, the indices of
        // the duplicated vertices are converted to the original indices.
        void RemapIndices(std::map<int, int> const& indexMap)
        {
            // The triangulation includes indices to the duplicated outer and
            // inner vertices.  These indices must be mapped back to the
            // original ones.
            for (auto& tri : mTriangles)
            {
                for (int i = 0; i < 3; ++i)
                {
                    auto iter = indexMap.find(tri[i]);
                    if (iter != indexMap.end())
                    {
                        tri[i] = iter->second;
                    }
                }
            }
        }

        // Two extra elements are needed in the position array per
        // outer-inners polygon.  This function computes the total number of
        // extra elements needed for the input tree and it converts InputType
        // vertices to ComputeType values.
        int InitializeFromTree(std::shared_ptr<PolygonTree> const& tree)
        {
            // Use a breadth-first search to process the outer-inners pairs
            // of the tree of nested polygons.
            int numExtraPoints = 0;

            std::queue<std::shared_ptr<PolygonTree>> treeQueue;
            treeQueue.push(tree);
            while (treeQueue.size() > 0)
            {
                // The 'root' is an outer polygon.
                std::shared_ptr<PolygonTree> outer = treeQueue.front();
                treeQueue.pop();

                // Count number of extra points for this outer-inners pair.
                int numChildren = static_cast<int>(outer->child.size());
                numExtraPoints += 2 * numChildren;

                // Convert outer points from InputType to ComputeType.
                int const numOuterIndices = static_cast<int>(outer->polygon.size());
                int const* outerIndices = outer->polygon.data();
                for (int i = 0; i < numOuterIndices; ++i)
                {
                    int index = outerIndices[i];
                    if (!mIsConverted[index])
                    {
                        mIsConverted[index] = true;
                        for (int j = 0; j < 2; ++j)
                        {
                            mComputePoints[index][j] = mPoints[index][j];
                        }
                    }
                }

                // The grandchildren of the outer polygon are also outer
                // polygons.  Insert them into the queue for processing.
                for (int c = 0; c < numChildren; ++c)
                {
                    // The 'child' is an inner polygon.
                    std::shared_ptr<PolygonTree> inner = outer->child[c];

                    // Convert inner points from InputType to ComputeType.
                    int const numInnerIndices = static_cast<int>(inner->polygon.size());
                    int const* innerIndices = inner->polygon.data();
                    for (int i = 0; i < numInnerIndices; ++i)
                    {
                        int index = innerIndices[i];
                        if (!mIsConverted[index])
                        {
                            mIsConverted[index] = true;
                            for (int j = 0; j < 2; ++j)
                            {
                                mComputePoints[index][j] = mPoints[index][j];
                            }
                        }
                    }

                    int numGrandChildren = static_cast<int>(inner->child.size());
                    for (int g = 0; g < numGrandChildren; ++g)
                    {
                        treeQueue.push(inner->child[g]);
                    }
                }
            }

            return numExtraPoints;
        }

        // The input polygon.
        int mNumPoints;
        Vector2<InputType> const* mPoints;

        // The output triangulation.
        std::vector<std::array<int, 3>> mTriangles;

        // The array of points used for geometric queries.  If you want to be
        // certain of a correct result, choose ComputeType to be BSNumber.
        // The InputType points are convertex to ComputeType points on demand;
        // the mIsConverted array keeps track of which input points have been
        // converted.
        std::vector<Vector2<ComputeType>> mComputePoints;
        std::vector<bool> mIsConverted;
        PrimalQuery2<ComputeType> mQuery;

        // Doubly linked lists for storing specially tagged vertices.
        class Vertex
        {
        public:
            Vertex()
                :
                index(-1),
                vPrev(-1),
                vNext(-1),
                sPrev(-1),
                sNext(-1),
                ePrev(-1),
                eNext(-1),
                isConvex(false),
                isEar(false)
            {
            }

            int index;          // index of vertex in position array
            int vPrev, vNext;   // vertex links for polygon
            int sPrev, sNext;   // convex/reflex vertex links (disjoint lists)
            int ePrev, eNext;   // ear links
            bool isConvex, isEar;
        };

        inline Vertex& V(int i)
        {
            LogAssert(0 <= i && i < static_cast<int>(mVertices.size()),
                "Index out of range. Do you have coincident vertex-edge or edge-edge? These violate the assumptions for the algorithm.");
            return mVertices[i];
        }

        bool IsConvex(int i)
        {
            Vertex& vertex = V(i);
            int curr = vertex.index;
            int prev = V(vertex.vPrev).index;
            int next = V(vertex.vNext).index;
            vertex.isConvex = (mQuery.ToLine(curr, prev, next) > 0);
            return vertex.isConvex;
        }

        bool IsEar(int i)
        {
            Vertex& vertex = V(i);

            if (mRFirst == -1)
            {
                // The remaining polygon is convex.
                vertex.isEar = true;
                return true;
            }

            // Search the reflex vertices and test if any are in the triangle
            // <V[prev],V[curr],V[next]>.
            int prev = V(vertex.vPrev).index;
            int curr = vertex.index;
            int next = V(vertex.vNext).index;
            vertex.isEar = true;
            for (int j = mRFirst; j != -1; j = V(j).sNext)
            {
                // Check if the test vertex is already one of the triangle
                // vertices.
                if (j == vertex.vPrev || j == i || j == vertex.vNext)
                {
                    continue;
                }

                // V[j] has been ruled out as one of the original vertices of
                // the triangle <V[prev],V[curr],V[next]>.  When triangulating
                // polygons with holes, V[j] might be a duplicated vertex, in
                // which case it does not affect the earness of V[curr].
                int test = V(j).index;
                if (mComputePoints[test] == mComputePoints[prev]
                    || mComputePoints[test] == mComputePoints[curr]
                    || mComputePoints[test] == mComputePoints[next])
                {
                    continue;
                }

                // Test if the vertex is inside or on the triangle.  When it
                // is, it causes V[curr] not to be an ear.
                if (mQuery.ToTriangle(test, prev, curr, next) <= 0)
                {
                    vertex.isEar = false;
                    break;
                }
            }

            return vertex.isEar;
        }

        // insert convex vertex
        void InsertAfterC(int i)
        {
            if (mCFirst == -1)
            {
                // Add the first convex vertex.
                mCFirst = i;
            }
            else
            {
                V(mCLast).sNext = i;
                V(i).sPrev = mCLast;
            }
            mCLast = i;
        }

        // insert reflex vertesx
        void InsertAfterR(int i)
        {
            if (mRFirst == -1)
            {
                // Add the first reflex vertex.
                mRFirst = i;
            }
            else
            {
                V(mRLast).sNext = i;
                V(i).sPrev = mRLast;
            }
            mRLast = i;
        }

        // insert ear at end of list
        void InsertEndE(int i)
        {
            if (mEFirst == -1)
            {
                // Add the first ear.
                mEFirst = i;
                mELast = i;
            }
            V(mELast).eNext = i;
            V(i).ePrev = mELast;
            mELast = i;
        }

        // insert ear after efirst
        void InsertAfterE(int i)
        {
            Vertex& first = V(mEFirst);
            int currENext = first.eNext;
            Vertex& vertex = V(i);
            vertex.ePrev = mEFirst;
            vertex.eNext = currENext;
            first.eNext = i;
            V(currENext).ePrev = i;
        }

        // insert ear before efirst
        void InsertBeforeE(int i)
        {
            Vertex& first = V(mEFirst);
            int currEPrev = first.ePrev;
            Vertex& vertex = V(i);
            vertex.ePrev = currEPrev;
            vertex.eNext = mEFirst;
            first.ePrev = i;
            V(currEPrev).eNext = i;
        }

        // remove vertex
        void RemoveV(int i)
        {
            int currVPrev = V(i).vPrev;
            int currVNext = V(i).vNext;
            V(currVPrev).vNext = currVNext;
            V(currVNext).vPrev = currVPrev;
        }

        // remove ear at i
        int RemoveE(int i)
        {
            int currEPrev = V(i).ePrev;
            int currENext = V(i).eNext;
            V(currEPrev).eNext = currENext;
            V(currENext).ePrev = currEPrev;
            return currENext;
        }

        // remove reflex vertex
        void RemoveR(int i)
        {
            LogAssert(mRFirst != -1 && mRLast != -1, "Reflex vertices must exist.");

            if (i == mRFirst)
            {
                mRFirst = V(i).sNext;
                if (mRFirst != -1)
                {
                    V(mRFirst).sPrev = -1;
                }
                V(i).sNext = -1;
            }
            else if (i == mRLast)
            {
                mRLast = V(i).sPrev;
                if (mRLast != -1)
                {
                    V(mRLast).sNext = -1;
                }
                V(i).sPrev = -1;
            }
            else
            {
                int currSPrev = V(i).sPrev;
                int currSNext = V(i).sNext;
                V(currSPrev).sNext = currSNext;
                V(currSNext).sPrev = currSPrev;
                V(i).sNext = -1;
                V(i).sPrev = -1;
            }
        }

        // The doubly linked list.
        std::vector<Vertex> mVertices;
        int mCFirst, mCLast;  // linear list of convex vertices
        int mRFirst, mRLast;  // linear list of reflex vertices
        int mEFirst, mELast;  // cyclical list of ears
    };
}
