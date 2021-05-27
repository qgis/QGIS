// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.22

#pragma once

// Remove includes of <Mathematics/PrimalQuery2.h> and <set> once
// Delaunay2<InputType, ComputeType> is removed.
#include <Mathematics/Logger.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/FPInterval.h>
#include <Mathematics/HashCombine.h>
#include <Mathematics/Line.h>
#include <Mathematics/PrimalQuery2.h>
#include <Mathematics/SWInterval.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/VETManifoldMesh.h>
#include <numeric>

// Delaunay triangulation of points (intrinsic dimensionality 2).
//   VQ = number of vertices
//   V  = array of vertices
//   TQ = number of triangles
//   I  = Array of 3-tuples of indices into V that represent the triangles
//        (3*TQ total elements).  Access via GetIndices(*).
//   A  = Array of 3-tuples of indices into I that represent the adjacent
//        triangles (3*TQ total elements). Access via GetAdjacencies(*).
// The i-th triangle has vertices
//   vertex[0] = V[I[3*i+0]]
//   vertex[1] = V[I[3*i+1]]
//   vertex[2] = V[I[3*i+2]]
// and edge index pairs
//   edge[0] = <I[3*i+0],I[3*i+1]>
//   edge[1] = <I[3*i+1],I[3*i+2]>
//   edge[2] = <I[3*i+2],I[3*i+0]>
// The triangles adjacent to these edges have indices
//   adjacent[0] = A[3*i+0] is the triangle sharing edge[0]
//   adjacent[1] = A[3*i+1] is the triangle sharing edge[1]
//   adjacent[2] = A[3*i+2] is the triangle sharing edge[2]
// If there is no adjacent triangle, the A[*] value is set to -1. The
// triangle adjacent to edge[j] has vertices
//   adjvertex[0] = V[I[3*adjacent[j]+0]]
//   adjvertex[1] = V[I[3*adjacent[j]+1]]
//   adjvertex[2] = V[I[3*adjacent[j]+2]]
// The only way to ensure a correct result for the input vertices (assumed to
// be exact) is to choose ComputeType for exact rational arithmetic. You may
// use BSNumber. No divisions are performed in this computation, so you do
// not have to use BSRational.

namespace gte
{
    // The variadic template declaration supports the class
    // Delaunay2<InputType, ComputeType>, which is deprecated and will be
    // removed in a future release. The declaration also supports the
    // replacement class Delaunay2<InputType>. The new class uses a blend of
    // interval arithmetic and rational arithmetic. It also uses unordered
    // sets (hash tables). The replacement performs much better than the
    // deprecated class.
    template <typename T, typename...>
    class Delaunay2 {};
}

namespace gte
{
    // This class requires you to specify the ComputeType yourself. If it
    // is BSNumber<> or BSRational<>, the worst-case choices of N for the
    // chosen InputType are listed in the next table. The numerical
    // computations are encapsulated in PrimalQuery2<ComputeType>::ToLine and
    // PrimalQuery2<ComputeType>::ToCircumcircle, the latter query the
    // dominant one in/ determining N. We recommend using only BSNumber,
    // because no divisions are performed in the triangulation computations.
    //
    //    input type | compute type | N
    //    -----------+--------------+------
    //    float      | BSNumber     |    35
    //    double     | BSNumber     |   263
    //    float      | BSRational   |   573
    //    double     | BSRational   |  4329

    template <typename InputType, typename ComputeType>
    class // [[deprecated("Use Delaunay2<InputType> instead.")]]
        Delaunay2<InputType, ComputeType>
    {
    public:
        // The class is a functor to support computing the Delaunay
        // triangulation of multiple data sets using the same class object.
        virtual ~Delaunay2() = default;

        Delaunay2()
            :
            mEpsilon((InputType)0),
            mDimension(0),
            mLine(Vector2<InputType>::Zero(), Vector2<InputType>::Zero()),
            mNumVertices(0),
            mNumUniqueVertices(0),
            mNumTriangles(0),
            mVertices(nullptr),
            mIndex{ { { 0, 1 }, { 1, 2 }, { 2, 0 } } }
        {
        }

        // The input is the array of vertices whose Delaunay triangulation is
        // required.  The epsilon value is used to determine the intrinsic
        // dimensionality of the vertices (d = 0, 1, or 2).  When epsilon is
        // positive, the determination is fuzzy--vertices approximately the
        // same point, approximately on a line, or planar.  The return value
        // is 'true' if and only if the hull construction is successful.
        bool operator()(int numVertices, Vector2<InputType> const* vertices, InputType epsilon)
        {
            mEpsilon = std::max(epsilon, (InputType)0);
            mDimension = 0;
            mLine.origin = Vector2<InputType>::Zero();
            mLine.direction = Vector2<InputType>::Zero();
            mNumVertices = numVertices;
            mNumUniqueVertices = 0;
            mNumTriangles = 0;
            mVertices = vertices;
            mGraph.Clear();
            mIndices.clear();
            mAdjacencies.clear();
            mDuplicates.resize(std::max(numVertices, 3));

            int i, j;
            if (mNumVertices < 3)
            {
                // Delaunay2 should be called with at least three points.
                return false;
            }

            IntrinsicsVector2<InputType> info(mNumVertices, vertices, mEpsilon);
            if (info.dimension == 0)
            {
                // mDimension is 0; mGraph, mIndices, and mAdjacencies are empty
                return false;
            }

            if (info.dimension == 1)
            {
                // The set is (nearly) collinear.
                mDimension = 1;
                mLine = Line2<InputType>(info.origin, info.direction[0]);
                return false;
            }

            mDimension = 2;

            // Compute the vertices for the queries.
            mComputeVertices.resize(mNumVertices);
            mQuery.Set(mNumVertices, &mComputeVertices[0]);
            for (i = 0; i < mNumVertices; ++i)
            {
                for (j = 0; j < 2; ++j)
                {
                    mComputeVertices[i][j] = vertices[i][j];
                }
            }

            // Insert the (nondegenerate) triangle constructed by the call to
            // GetInformation.  This is necessary for the circumcircle-visibility
            // algorithm to work correctly.
            if (!info.extremeCCW)
            {
                std::swap(info.extreme[1], info.extreme[2]);
            }
            if (!mGraph.Insert(info.extreme[0], info.extreme[1], info.extreme[2]))
            {
                return false;
            }

            // Incrementally update the triangulation.  The set of processed
            // points is maintained to eliminate duplicates, either in the
            // original input points or in the points obtained by snap rounding.
            std::set<ProcessedVertex> processed;
            for (i = 0; i < 3; ++i)
            {
                j = info.extreme[i];
                processed.insert(ProcessedVertex(vertices[j], j));
                mDuplicates[j] = j;
            }
            for (i = 0; i < mNumVertices; ++i)
            {
                ProcessedVertex v(vertices[i], i);
                auto iter = processed.find(v);
                if (iter == processed.end())
                {
                    if (!Update(i))
                    {
                        // A failure can occur if ComputeType is not an exact
                        // arithmetic type.
                        return false;
                    }
                    processed.insert(v);
                    mDuplicates[i] = i;
                }
                else
                {
                    mDuplicates[i] = iter->location;
                }
            }
            mNumUniqueVertices = static_cast<int>(processed.size());

            // Assign integer values to the triangles for use by the caller
            // and copy the triangle information to compact arrays mIndices
            // and mAdjacencies.
            UpdateIndicesAdjacencies();

            return true;
        }

        // Dimensional information.  If GetDimension() returns 1, the points
        // lie on a line P+t*D (fuzzy comparison when epsilon > 0).  You can
        // sort these if you need a polyline output by projecting onto the
        // line each vertex X = P+t*D, where t = Dot(D,X-P).
        inline InputType GetEpsilon() const
        {
            return mEpsilon;
        }

        inline int GetDimension() const
        {
            return mDimension;
        }

        inline Line2<InputType> const& GetLine() const
        {
            return mLine;
        }

        // Member access.
        inline int GetNumVertices() const
        {
            return mNumVertices;
        }

        inline int GetNumUniqueVertices() const
        {
            return mNumUniqueVertices;
        }

        inline int GetNumTriangles() const
        {
            return mNumTriangles;
        }

        inline Vector2<InputType> const* GetVertices() const
        {
            return mVertices;
        }

        inline PrimalQuery2<ComputeType> const& GetQuery() const
        {
            return mQuery;
        }

        inline ETManifoldMesh const& GetGraph() const
        {
            return mGraph;
        }

        inline std::vector<int> const& GetIndices() const
        {
            return mIndices;
        }

        inline std::vector<int> const& GetAdjacencies() const
        {
            return mAdjacencies;
        }

        // If 'vertices' has no duplicates, GetDuplicates()[i] = i for all i.
        // If vertices[i] is the first occurrence of a vertex and if
        // vertices[j] is found later, then GetDuplicates()[j] = i.
        inline std::vector<int> const& GetDuplicates() const
        {
            return mDuplicates;
        }

        // Locate those triangle edges that do not share other triangles.  The
        // returned array has hull.size() = 2*numEdges, each pair representing
        // an edge.  The edges are not ordered, but the pair of vertices for
        // an edge is ordered so that they conform to a counterclockwise
        // traversal of the hull.  The return value is 'true' if and only if
        // the dimension is 2.
        bool GetHull(std::vector<int>& hull) const
        {
            if (mDimension == 2)
            {
                // Count the number of edges that are not shared by two
                // triangles.
                int numEdges = 0;
                for (auto adj : mAdjacencies)
                {
                    if (adj == -1)
                    {
                        ++numEdges;
                    }
                }

                if (numEdges > 0)
                {
                    // Enumerate the edges.
                    hull.resize(2 * numEdges);
                    int current = 0, i = 0;
                    for (auto adj : mAdjacencies)
                    {
                        if (adj == -1)
                        {
                            int tri = i / 3, j = i % 3;
                            hull[current++] = mIndices[3 * tri + j];
                            hull[current++] = mIndices[3 * tri + ((j + 1) % 3)];
                        }
                        ++i;
                    }
                    return true;
                }
                else
                {
                    LogError("Unexpected. There must be at least one triangle.");
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
        }

        // Copy Delaunay triangles to compact arrays mIndices and
        // mAdjacencies. The array information is accessible via the
        // functions GetIndices(int, std::array<int, 3>&) and
        // GetAdjacencies(int, std::array<int, 3>&).
        void UpdateIndicesAdjacencies()
        {
            // Assign integer values to the triangles.
            auto const& tmap = mGraph.GetTriangles();
            std::map<Triangle*, int> permute;
            int i = -1;
            permute[nullptr] = i++;
            for (auto const& element : tmap)
            {
                permute[element.second.get()] = i++;
            }

            mNumTriangles = static_cast<int>(tmap.size());
            int numindices = 3 * mNumTriangles;
            if (numindices > 0)
            {
                mIndices.resize(numindices);
                mAdjacencies.resize(numindices);
                i = 0;
                for (auto const& element : tmap)
                {
                    Triangle* tri = element.second.get();
                    for (size_t j = 0; j < 3; ++j, ++i)
                    {
                        mIndices[i] = tri->V[j];
                        mAdjacencies[i] = permute[tri->T[j]];
                    }
                }
            }
        }

        // Get the vertex indices for triangle i.  The function returns 'true'
        // when the dimension is 2 and i is a valid triangle index, in which
        // case the vertices are valid; otherwise, the function returns
        // 'false' and the vertices are invalid.
        bool GetIndices(int i, std::array<int, 3>& indices) const
        {
            if (mDimension == 2)
            {
                int numTriangles = static_cast<int>(mIndices.size() / 3);
                if (0 <= i && i < numTriangles)
                {
                    indices[0] = mIndices[3 * i];
                    indices[1] = mIndices[3 * i + 1];
                    indices[2] = mIndices[3 * i + 2];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
            return false;
        }

        // Get the indices for triangles adjacent to triangle i.  The function
        // returns 'true' when the dimension is 2 and if i is a valid triangle
        // index, in which case the adjacencies are valid; otherwise, the
        // function returns 'false' and the adjacencies are invalid.
        bool GetAdjacencies(int i, std::array<int, 3>& adjacencies) const
        {
            if (mDimension == 2)
            {
                int numTriangles = static_cast<int>(mIndices.size() / 3);
                if (0 <= i && i < numTriangles)
                {
                    adjacencies[0] = mAdjacencies[3 * i];
                    adjacencies[1] = mAdjacencies[3 * i + 1];
                    adjacencies[2] = mAdjacencies[3 * i + 2];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
            return false;
        }

        // Support for searching the triangulation for a triangle that
        // contains a point.  If there is a containing triangle, the returned
        // value is a triangle index i with 0 <= i < GetNumTriangles().  If
        // there is not a containing triangle, -1 is returned.  The
        // computations are performed using exact rational arithmetic.
        //
        // The SearchInfo input stores information about the triangle search
        // when looking for the triangle (if any) that contains p.  The first
        // triangle searched is 'initialTriangle'.  On return 'path' stores
        // those (ordered) triangle indices visited during the search.  The
        // last visited triangle has index 'finalTriangle and vertex indices
        // 'finalV[0,1,2]', stored in counterclockwise order.  The last edge
        // of the search is <finalV[0],finalV[1]>.  For spatially coherent
        // inputs p for numerous calls to this function, you will want to
        // specify 'finalTriangle' from the previous call as 'initialTriangle'
        // for the next call, which should reduce search times.

        static int constexpr negOne = -1;

        struct SearchInfo
        {
            SearchInfo()
                :
                initialTriangle(negOne),
                numPath(0),
                path{},
                finalTriangle(0),
                finalV{ 0, 0, 0 }
            {
            }

            int initialTriangle;
            int numPath;
            std::vector<int> path;
            int finalTriangle;
            std::array<int, 3> finalV;
        };

        int GetContainingTriangle(Vector2<InputType> const& p, SearchInfo& info) const
        {
            if (mDimension == 2)
            {
                Vector2<ComputeType> test{ p[0], p[1] };

                int numTriangles = static_cast<int>(mIndices.size() / 3);
                info.path.resize(numTriangles);
                info.numPath = 0;
                int triangle;
                if (0 <= info.initialTriangle && info.initialTriangle < numTriangles)
                {
                    triangle = info.initialTriangle;
                }
                else
                {
                    info.initialTriangle = 0;
                    triangle = 0;
                }

                // Use triangle edges as binary separating lines.
                for (int i = 0; i < numTriangles; ++i)
                {
                    int ibase = 3 * triangle;
                    int const* v = &mIndices[ibase];

                    info.path[info.numPath++] = triangle;
                    info.finalTriangle = triangle;
                    info.finalV[0] = v[0];
                    info.finalV[1] = v[1];
                    info.finalV[2] = v[2];

                    if (mQuery.ToLine(test, v[0], v[1]) > 0)
                    {
                        triangle = mAdjacencies[ibase];
                        if (triangle == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[1];
                            info.finalV[2] = v[2];
                            return -1;
                        }
                        continue;
                    }

                    if (mQuery.ToLine(test, v[1], v[2]) > 0)
                    {
                        triangle = mAdjacencies[ibase + 1];
                        if (triangle == -1)
                        {
                            info.finalV[0] = v[1];
                            info.finalV[1] = v[2];
                            info.finalV[2] = v[0];
                            return -1;
                        }
                        continue;
                    }

                    if (mQuery.ToLine(test, v[2], v[0]) > 0)
                    {
                        triangle = mAdjacencies[ibase + 2];
                        if (triangle == -1)
                        {
                            info.finalV[0] = v[2];
                            info.finalV[1] = v[0];
                            info.finalV[2] = v[1];
                            return -1;
                        }
                        continue;
                    }

                    return triangle;
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
            return -1;
        }

    protected:
        // Support for incremental Delaunay triangulation.
        typedef ETManifoldMesh::Triangle Triangle;

        bool GetContainingTriangle(int i, Triangle*& tri) const
        {
            int numTriangles = static_cast<int>(mGraph.GetTriangles().size());
            for (int t = 0; t < numTriangles; ++t)
            {
                int j;
                for (j = 0; j < 3; ++j)
                {
                    int v0 = tri->V[mIndex[j][0]];
                    int v1 = tri->V[mIndex[j][1]];
                    if (mQuery.ToLine(i, v0, v1) > 0)
                    {
                        // Point i sees edge <v0,v1> from outside the triangle.
                        auto adjTri = tri->T[j];
                        if (adjTri)
                        {
                            // Traverse to the triangle sharing the face.
                            tri = adjTri;
                            break;
                        }
                        else
                        {
                            // We reached a hull edge, so the point is outside
                            // the hull.
                            return false;
                        }
                    }

                }

                if (j == 3)
                {
                    // The point is inside all four edges, so the point is inside
                    // a triangle.
                    return true;
                }
            }

            LogError("Unexpected termination of loop.");
        }

        bool GetAndRemoveInsertionPolygon(int i, std::set<Triangle*>& candidates,
            std::set<EdgeKey<true>>& boundary)
        {
            // Locate the triangles that make up the insertion polygon.
            ETManifoldMesh polygon;
            while (candidates.size() > 0)
            {
                Triangle* tri = *candidates.begin();
                candidates.erase(candidates.begin());

                for (int j = 0; j < 3; ++j)
                {
                    auto adj = tri->T[j];
                    if (adj && candidates.find(adj) == candidates.end())
                    {
                        int a0 = adj->V[0];
                        int a1 = adj->V[1];
                        int a2 = adj->V[2];
                        if (mQuery.ToCircumcircle(i, a0, a1, a2) <= 0)
                        {
                            // Point i is in the circumcircle.
                            candidates.insert(adj);
                        }
                    }
                }

                if (!polygon.Insert(tri->V[0], tri->V[1], tri->V[2]))
                {
                    return false;
                }
                if (!mGraph.Remove(tri->V[0], tri->V[1], tri->V[2]))
                {
                    return false;
                }
            }

            // Get the boundary edges of the insertion polygon.
            for (auto const& element : polygon.GetTriangles())
            {
                Triangle* tri = element.second.get();
                for (int j = 0; j < 3; ++j)
                {
                    if (!tri->T[j])
                    {
                        boundary.insert(EdgeKey<true>(tri->V[mIndex[j][0]], tri->V[mIndex[j][1]]));
                    }
                }
            }
            return true;
        }

        bool Update(int i)
        {
            // The return value of mGraph.Insert(...) is nullptr if there was
            // a failure to insert.  The Update function will return 'false'
            // when the insertion fails.

            auto const& tmap = mGraph.GetTriangles();
            Triangle* tri = tmap.begin()->second.get();
            if (GetContainingTriangle(i, tri))
            {
                // The point is inside the convex hull.  The insertion polygon
                // contains only triangles in the current triangulation; the
                // hull does not change.

                // Use a depth-first search for those triangles whose
                // circumcircles contain point i.
                std::set<Triangle*> candidates;
                candidates.insert(tri);

                // Get the boundary of the insertion polygon C that contains
                // the triangles whose circumcircles contain point i.  Polygon
                // C contains the point i.
                std::set<EdgeKey<true>> boundary;
                if (!GetAndRemoveInsertionPolygon(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polygon consists of the triangles formed by
                // point i and the faces of C.
                for (auto const& key : boundary)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    if (mQuery.ToLine(i, v0, v1) < 0)
                    {
                        if (!mGraph.Insert(i, v0, v1))
                        {
                            return false;
                        }
                    }
                    // else:  Point i is on an edge of 'tri', so the
                    // subdivision has degenerate triangles.  Ignore these.
                }
            }
            else
            {
                // The point is outside the convex hull.  The insertion
                // polygon is formed by point i and any triangles in the
                // current triangulation whose circumcircles contain point i.

                // Locate the convex hull of the triangles.
                std::set<EdgeKey<true>> hull;
                for (auto const& element : tmap)
                {
                    Triangle* t = element.second.get();
                    for (int j = 0; j < 3; ++j)
                    {
                        if (!t->T[j])
                        {
                            hull.insert(EdgeKey<true>(t->V[mIndex[j][0]], t->V[mIndex[j][1]]));
                        }
                    }
                }

                // Iterate over all the hull edges and use the ones visible to
                // point i to locate the insertion polygon.
                auto const& emap = mGraph.GetEdges();
                std::set<Triangle*> candidates;
                std::set<EdgeKey<true>> visible;
                for (auto const& key : hull)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    if (mQuery.ToLine(i, v0, v1) > 0)
                    {
                        auto iter = emap.find(EdgeKey<false>(v0, v1));
                        if (iter != emap.end() && iter->second->T[1] == nullptr)
                        {
                            auto adj = iter->second->T[0];
                            if (adj && candidates.find(adj) == candidates.end())
                            {
                                int a0 = adj->V[0];
                                int a1 = adj->V[1];
                                int a2 = adj->V[2];
                                if (mQuery.ToCircumcircle(i, a0, a1, a2) <= 0)
                                {
                                    // Point i is in the circumcircle.
                                    candidates.insert(adj);
                                }
                                else
                                {
                                    // Point i is not in the circumcircle but
                                    // the hull edge is visible.
                                    visible.insert(key);
                                }
                            }
                        }
                        else
                        {
                            // This should be exposed, but because the class is
                            // deprecated, it is not exposed to preserve current
                            // behavior in client applications.
                            // LogError("Unexpected condition (ComputeType not exact?)");
                            return false;
                        }
                    }
                }

                // Get the boundary of the insertion subpolygon C that
                // contains the triangles whose circumcircles contain point i.
                std::set<EdgeKey<true>> boundary;
                if (!GetAndRemoveInsertionPolygon(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polygon P consists of the triangles formed by
                // point i and the back edges of C *and* the visible edges of
                // mGraph-C.
                for (auto const& key : boundary)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    if (mQuery.ToLine(i, v0, v1) < 0)
                    {
                        // This is a back edge of the boundary.
                        if (!mGraph.Insert(i, v0, v1))
                        {
                            return false;
                        }
                    }
                }
                for (auto const& key : visible)
                {
                    if (!mGraph.Insert(i, key.V[1], key.V[0]))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        // The epsilon value is used for fuzzy determination of intrinsic
        // dimensionality.  If the dimension is 0 or 1, the constructor
        // returns early.  The caller is responsible for retrieving the
        // dimension and taking an alternate path should the dimension be
        // smaller than 2.  If the dimension is 0, the caller may as well
        // treat all vertices[] as a single point, say, vertices[0].  If the
        // dimension is 1, the caller can query for the approximating line and
        // project vertices[] onto it for further processing.
        InputType mEpsilon;
        int mDimension;
        Line2<InputType> mLine;

        // The array of vertices used for geometric queries.  If you want to
        // be certain of a correct result, choose ComputeType to be BSNumber.
        std::vector<Vector2<ComputeType>> mComputeVertices;
        PrimalQuery2<ComputeType> mQuery;

        // The graph information.
        int mNumVertices;
        int mNumUniqueVertices;
        int mNumTriangles;
        Vector2<InputType> const* mVertices;
        VETManifoldMesh mGraph;
        std::vector<int> mIndices;
        std::vector<int> mAdjacencies;

        // If a vertex occurs multiple times in the 'vertices' input to the
        // constructor, the first processed occurrence of that vertex has an
        // index stored in this array.  If there are no duplicates, then
        // mDuplicates[i] = i for all i.

        struct ProcessedVertex
        {
            ProcessedVertex() = default;

            ProcessedVertex(Vector2<InputType> const& inVertex, int inLocation)
                :
                vertex(inVertex),
                location(inLocation)
            {
            }

            bool operator<(ProcessedVertex const& v) const
            {
                return vertex < v.vertex;
            }

            Vector2<InputType> vertex;
            int location;
        };

        std::vector<int> mDuplicates;

        // Indexing for the vertices of the triangle adjacent to a vertex.
        // The edge adjacent to vertex j is <mIndex[j][0], mIndex[j][1]> and
        // is listed so that the triangle interior is to your left as you walk
        // around the edges.
        std::array<std::array<int, 2>, 3> mIndex;
    };
}

namespace gte
{
    // The input type must be 'float' or 'double'. The user no longer has
    // the responsibility to specify the compute type.

    template <typename T>
    class Delaunay2<T>
    {
    public:
        // The class is a functor to support computing the Delaunay
        // triangulation of multiple data sets using the same class object.
        virtual ~Delaunay2() = default;

        Delaunay2()
            :
            mNumVertices(0),
            mVertices(nullptr),
            mIRVertices{},
            mGraph(),
            mDuplicates{},
            mNumUniqueVertices(0),
            mDimension(0),
            mLine(Vector2<T>::Zero(), Vector2<T>::Zero()),
            mNumTriangles(0),
            mIndices{},
            mAdjacencies{},
            mIndex{ { { 0, 1 }, { 1, 2 }, { 2, 0 } } },
            mQueryPoint(Vector2<T>::Zero()),
            mIRQueryPoint(Vector2<InputRational>::Zero()),
            mCRPool(maxNumCRPool)
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be float or double.");
        }

        // The input is the array of vertices whose Delaunay triangulation is
        // required. The return value is 'true' if and only if the intrinsic
        // dimension of the points is 2. If the intrinsic dimension is 1, the
        // points lie exactly on a line which is then accessible via the
        // accessor GetLine(). If the intrinsic dimension is 0, the points are
        // all the same point.
        bool operator()(std::vector<Vector2<T>> const& vertices)
        {
            return operator()(vertices.size(), vertices.data());
        }

        bool operator()(size_t numVertices, Vector2<T> const* vertices)
        {
            // Initialize values in case they were set by a previous call
            // to operator()(...).
            LogAssert(numVertices > 0 && vertices != nullptr, "Invalid argument.");

            mNumVertices = numVertices;
            mVertices = vertices;
            mIRVertices.clear();
            mDuplicates.clear();
            mLine.origin = Vector2<T>::Zero();
            mLine.direction = Vector2<T>::Zero();
            mNumUniqueVertices = 0;
            mNumTriangles = 0;
            mGraph.Clear();
            mIndices.clear();
            mAdjacencies.clear();
            mQueryPoint = Vector2<T>::Zero();
            mIRQueryPoint = Vector2<InputRational>::Zero();

            // Compute the intrinsic dimension and return early if that
            // dimension is 0 or 1.
            IntrinsicsVector2<T> info(static_cast<int>(mNumVertices), mVertices, static_cast<T>(0));
            if (info.dimension == 0)
            {
                // The vertices are the same point.
                mDimension = 0;
                mLine.origin = info.origin;
                return false;
            }

            if (info.dimension == 1)
            {
                // The vertices are collinear.
                mDimension = 1;
                mLine.origin = info.origin;
                mLine.direction = info.direction[0];
                return false;
            }

            // The vertices necessarily will have a triangulation.
            mDimension = 2;

            // Convert the floating-point inputs to rational type.
            mIRVertices.resize(mNumVertices);
            for (size_t i = 0; i < mNumVertices; ++i)
            {
                mIRVertices[i][0] = mVertices[i][0];
                mIRVertices[i][1] = mVertices[i][1];
            }

            // Assume initially the vertices are unique. If duplicates are
            // found during the Delaunay update, mDuplicates[] will be
            // modified accordingly.
            mDuplicates.resize(mNumVertices);
            std::iota(mDuplicates.begin(), mDuplicates.end(), 0);

            // Insert the nondegenerate triangle constructed by the call to
            // GetInformation. This is necessary for the circumcircle
            // visibility algorithm to work correctly.
            if (!info.extremeCCW)
            {
                std::swap(info.extreme[1], info.extreme[2]);
            }

            auto inserted = mGraph.Insert(info.extreme[0], info.extreme[1], info.extreme[2]);
            LogAssert(inserted != nullptr, "The triangle should not be degenerate.");

            // Incrementally update the triangulation. The set of processed
            // points is maintained to eliminate duplicates.
            ProcessedVertexSet processed;
            for (size_t i = 0; i < 3; ++i)
            {
                int32_t j = info.extreme[i];
                processed.insert(ProcessedVertex(mVertices[j], j));
                mDuplicates[j] = j;
            }
            for (size_t i = 0; i < mNumVertices; ++i)
            {
                ProcessedVertex v(mVertices[i], i);
                auto iter = processed.find(v);
                if (iter == processed.end())
                {
                    Update(i);
                    processed.insert(v);
                    mDuplicates[i] = i;
                }
                else
                {
                    mDuplicates[i] = iter->location;
                }
            }
            mNumUniqueVertices = processed.size();

            // Assign integer values to the triangles for use by the caller
            // and copy the triangle information to compact arrays mIndices
            // and mAdjacencies.
            UpdateIndicesAdjacencies();

            return true;
        }

        // Dimensional information. If GetDimension() returns 1, the points
        // lie on a line P+t*D. You can sort these if you need a polyline
        // output by projecting onto the line each vertex X = P+t*D, where
        // t = Dot(D,X-P).
        inline size_t GetDimension() const
        {
            return mDimension;
        }

        inline Line2<T> const& GetLine() const
        {
            return mLine;
        }

        // Member access.
        inline size_t GetNumVertices() const
        {
            return mIRVertices.size();
        }

        inline Vector2<T> const* GetVertices() const
        {
            return mVertices;
        }

        inline size_t GetNumUniqueVertices() const
        {
            return mNumUniqueVertices;
        }

        // If 'vertices' has no duplicates, GetDuplicates()[i] = i for all i.
        // If vertices[i] is the first occurrence of a vertex and if
        // vertices[j] is found later, then GetDuplicates()[j] = i.
        inline std::vector<size_t> const& GetDuplicates() const
        {
            return mDuplicates;
        }

        inline size_t GetNumTriangles() const
        {
            return mNumTriangles;
        }

        inline ETManifoldMesh const& GetGraph() const
        {
            return mGraph;
        }

        inline std::vector<int32_t> const& GetIndices() const
        {
            return mIndices;
        }

        inline std::vector<int32_t> const& GetAdjacencies() const
        {
            return mAdjacencies;
        }

        // Locate those triangle edges that do not share other triangles. The
        // returned array has hull.size() = 2*numEdges, each pair representing
        // an edge. The edges are not ordered, but the pair of vertices for
        // an edge is ordered so that they conform to a counterclockwise
        // traversal of the hull. The return value is 'true' if and only if
        // the dimension is 2.
        bool GetHull(std::vector<size_t>& hull) const
        {
            if (mDimension == 2)
            {
                // Count the number of edges that are not shared by two
                // triangles.
                size_t numEdges = 0;
                for (auto adj : mAdjacencies)
                {
                    if (adj == -1)
                    {
                        ++numEdges;
                    }
                }

                if (numEdges > 0)
                {
                    // Enumerate the edges.
                    hull.resize(2 * numEdges);
                    size_t current = 0, i = 0;
                    for (auto adj : mAdjacencies)
                    {
                        if (adj == -1)
                        {
                            size_t tri = i / 3, j = i % 3;
                            hull[current++] = mIndices[3 * tri + j];
                            hull[current++] = mIndices[3 * tri + ((j + 1) % 3)];
                        }
                        ++i;
                    }
                    return true;
                }
                else
                {
                    LogError("Unexpected condition. There must be at least one triangle.");
                }
            }
            else
            {
                LogError("The dimension must be 2.");
            }
        }

        // Copy Delaunay triangles to compact arrays mIndices and
        // mAdjacencies. The array information is accessible via the
        // functions GetIndices(int32_t, std::array<int32_t, 3>&) and
        // GetAdjacencies(int32_t, std::array<int32_t, 3>&).
        void UpdateIndicesAdjacencies()
        {
            // Assign integer values to the triangles.
            auto const& tmap = mGraph.GetTriangles();
            std::unordered_map<Triangle*, int32_t> permute;
            int32_t i = -1;
            permute[nullptr] = i++;
            for (auto const& element : tmap)
            {
                permute[element.second.get()] = i++;
            }

            mNumTriangles = tmap.size();
            size_t numindices = 3 * mNumTriangles;
            if (numindices > 0)
            {
                mIndices.resize(numindices);
                mAdjacencies.resize(numindices);
                i = 0;
                for (auto const& element : tmap)
                {
                    Triangle* tri = element.second.get();
                    for (size_t j = 0; j < 3; ++j, ++i)
                    {
                        mIndices[i] = tri->V[j];
                        mAdjacencies[i] = permute[tri->T[j]];
                    }
                }
            }
        }

        // Get the vertex indices for triangle t. The function returns 'true'
        // when the dimension is 2 and t is a valid triangle index, in which
        // case the vertices are valid; otherwise, the function returns
        // 'false' and the vertices are invalid.
        bool GetIndices(size_t t, std::array<int32_t, 3>& indices) const
        {
            if (mDimension == 2)
            {
                size_t const numTriangles = mIndices.size() / 3;
                if (t < numTriangles)
                {
                    indices[0] = mIndices[3 * t];
                    indices[1] = mIndices[3 * t + 1];
                    indices[2] = mIndices[3 * t + 2];
                    return true;
                }
            }
            return false;
        }

        // Get the indices for triangles adjacent to triangle t. The function
        // returns 'true' when the dimension is 2 and if t is a valid triangle
        // index, in which case the adjacencies are valid; otherwise, the
        // function returns 'false' and the adjacencies are invalid.
        bool GetAdjacencies(size_t t, std::array<int32_t, 3>& adjacencies) const
        {
            if (mDimension == 2)
            {
                size_t const numTriangles = mIndices.size() / 3;
                if (t < numTriangles)
                {
                    adjacencies[0] = mAdjacencies[3 * t];
                    adjacencies[1] = mAdjacencies[3 * t + 1];
                    adjacencies[2] = mAdjacencies[3 * t + 2];
                    return true;
                }
            }
            return false;
        }

        // Support for searching the triangulation for a triangle that
        // contains a point. If there is a containing triangle, the returned
        // value is a triangle index t with 0 <= t < GetNumTriangles(). If
        // there is not a containing triangle, -1 is returned. The
        // computations are performed using exact rational arithmetic.
        //
        // The SearchInfo input stores information about the triangle search
        // when looking for the triangle (if any) that contains p.  The first
        // triangle searched is 'initialTriangle'.  On return 'path' stores
        // those (ordered) triangle indices visited during the search.  The
        // last visited triangle has index 'finalTriangle and vertex indices
        // 'finalV[0,1,2]', stored in counterclockwise order.  The last edge
        // of the search is <finalV[0],finalV[1]>.  For spatially coherent
        // inputs p for numerous calls to this function, you will want to
        // specify 'finalTriangle' from the previous call as 'initialTriangle'
        // for the next call, which should reduce search times.

        static size_t constexpr negOne = std::numeric_limits<size_t>::max();

        struct SearchInfo
        {
            SearchInfo()
                :
                initialTriangle(negOne),
                numPath(0),
                finalTriangle(0),
                finalV{ 0, 0, 0 },
                path{}
            {
            }

            size_t initialTriangle;
            size_t numPath;
            size_t finalTriangle;
            std::array<int32_t, 3> finalV;
            std::vector<size_t> path;
        };

        // If the point is in a triangle, the return value is the index of the
        // triangle. If the point is not in a triangle, the return value is
        // std::numeric_limits<size_t>::max().
        size_t GetContainingTriangle(Vector2<T> const& inP, SearchInfo& info) const
        {
            LogAssert(mDimension == 2, "Invalid dimension for triangle search.");

            mQueryPoint = inP;
            mIRQueryPoint = { inP[0], inP[1] };

            size_t const numTriangles = mIndices.size() / 3;
            info.path.resize(numTriangles);
            info.numPath = 0;
            size_t triangle;
            if (info.initialTriangle < numTriangles)
            {
                triangle = info.initialTriangle;
            }
            else
            {
                info.initialTriangle = 0;
                triangle = 0;
            }

            // Use triangle edges as binary separating lines.
            int32_t adjacent;
            for (size_t i = 0; i < numTriangles; ++i)
            {
                size_t ibase = 3 * triangle;
                int32_t const* v = &mIndices[ibase];

                info.path[info.numPath++] = triangle;
                info.finalTriangle = triangle;
                info.finalV[0] = v[0];
                info.finalV[1] = v[1];
                info.finalV[2] = v[2];

                if (ToLine(negOne, v[0], v[1]) > 0)
                {
                    adjacent = mAdjacencies[ibase];
                    if (adjacent == -1)
                    {
                        info.finalV[0] = v[0];
                        info.finalV[1] = v[1];
                        info.finalV[2] = v[2];
                        return negOne;
                    }
                    triangle = static_cast<size_t>(adjacent);
                    continue;
                }

                if (ToLine(negOne, v[1], v[2]) > 0)
                {
                    adjacent = mAdjacencies[ibase + 1];
                    if (adjacent == -1)
                    {
                        info.finalV[0] = v[1];
                        info.finalV[1] = v[2];
                        info.finalV[2] = v[0];
                        return negOne;
                    }
                    triangle = static_cast<size_t>(adjacent);
                    continue;
                }

                if (ToLine(negOne, v[2], v[0]) > 0)
                {
                    adjacent = mAdjacencies[ibase + 2];
                    if (adjacent == -1)
                    {
                        info.finalV[0] = v[2];
                        info.finalV[1] = v[0];
                        info.finalV[2] = v[1];
                        return negOne;
                    }
                    triangle = static_cast<size_t>(adjacent);
                    continue;
                }

                return triangle;
            }

            LogError("Unexpected termination of loop while searching for a triangle.");
        }

    protected:
        // The type of the read-only input vertices[] when converted for
        // rational arithmetic.
        static int32_t constexpr InputNumWords = std::is_same<T, float>::value ? 2 : 4;
        using InputRational = BSNumber<UIntegerFP32<InputNumWords>>;

        // The vector of vertices used for geometric queries. The input
        // vertices are read-only, so we can represent them by the type
        // InputRational.
        size_t mNumVertices;
        Vector2<T> const* mVertices;
        std::vector<Vector2<InputRational>> mIRVertices;

        VETManifoldMesh mGraph;

    private:
        // The compute type used for exact sign classification.
        static int32_t constexpr ComputeNumWords = std::is_same<T, float>::value ? 36 : 264;
        using ComputeRational = BSNumber<UIntegerFP32<ComputeNumWords>>;

        // Convenient renaming.
        using Triangle = ETManifoldMesh::Triangle;

        struct ProcessedVertex
        {
            ProcessedVertex() = default;

            ProcessedVertex(Vector2<T> const& inVertex, size_t inLocation)
                :
                vertex(inVertex),
                location(inLocation)
            {
            }

            // Support for hashing in std::unordered_set<>. The first
            // operator() is the hash function. The second operator() is
            // the equality comparison used for elements in the same bucket.
            std::size_t operator()(ProcessedVertex const& v) const
            {
                return HashValue(v.vertex[0], v.vertex[1], v.location);
            }

            bool operator()(ProcessedVertex const& v0, ProcessedVertex const& v1) const
            {
                return v0.vertex == v1.vertex && v0.location == v1.location;
            }

            Vector2<T> vertex;
            size_t location;
        };

        using ProcessedVertexSet = std::unordered_set<
            ProcessedVertex, ProcessedVertex, ProcessedVertex>;

        using DirectedEdgeKeySet = std::unordered_set<
            EdgeKey<true>, EdgeKey<true>, EdgeKey<true>>;

        using TrianglePtrSet = std::unordered_set<Triangle*>;

        static ComputeRational const& Copy(InputRational const& source,
            ComputeRational& target)
        {
            target.SetSign(source.GetSign());
            target.SetBiasedExponent(source.GetBiasedExponent());
            target.GetUInteger().CopyFrom(source.GetUInteger());
            return target;
        }

        // Given a line with origin V0 and direction <V0,V1> and a query
        // point P, ToLine returns
        //   +1, P on right of line
        //   -1, P on left of line
        //    0, P on the line
        int32_t ToLine(size_t pIndex, size_t v0Index, size_t v1Index) const
        {
            // The expression tree has 13 nodes consisting of 6 input
            // leaves and 7 compute nodes.

            // Use interval arithmetic to determine the sign if possible.
            auto const& inP = (pIndex != negOne ? mVertices[pIndex] : mQueryPoint);
            Vector2<T> const& inV0 = mVertices[v0Index];
            Vector2<T> const& inV1 = mVertices[v1Index];

            auto x0 = SWInterval<T>::Sub(inP[0], inV0[0]);
            auto y0 = SWInterval<T>::Sub(inP[1], inV0[1]);
            auto x1 = SWInterval<T>::Sub(inV1[0], inV0[0]);
            auto y1 = SWInterval<T>::Sub(inV1[1], inV0[1]);
            auto x0y1 = x0 * y1;
            auto x1y0 = x1 * y0;
            auto det = x0y1 - x1y0;

            T constexpr zero = 0;
            if (det[0] > zero)
            {
                return +1;
            }
            else if (det[1] < zero)
            {
                return -1;
            }

            // The exact sign of the determinant is not known, so compute
            // the determinant using rational arithmetic.

            // Name the nodes of the expression tree.
            auto const& irP = (pIndex != negOne ? mIRVertices[pIndex] : mIRQueryPoint);
            Vector2<InputRational> const& irV0 = mIRVertices[v0Index];
            Vector2<InputRational> const& irV1 = mIRVertices[v1Index];

            auto const& crP0 = Copy(irP[0], mCRPool[0]);
            auto const& crP1 = Copy(irP[1], mCRPool[1]);
            auto const& crV00 = Copy(irV0[0], mCRPool[2]);
            auto const& crV01 = Copy(irV0[1], mCRPool[3]);
            auto const& crV10 = Copy(irV1[0], mCRPool[4]);
            auto const& crV11 = Copy(irV1[1], mCRPool[5]);
            auto& crX0 = mCRPool[6];
            auto& crY0 = mCRPool[7];
            auto& crX1 = mCRPool[8];
            auto& crY1 = mCRPool[9];
            auto& crX0Y1 = mCRPool[10];
            auto& crX1Y0 = mCRPool[11];
            auto& crDet = mCRPool[12];

            // Evaluate the expression tree.
            crX0 = crP0 - crV00;
            crY0 = crP1 - crV01;
            crX1 = crV10 - crV00;
            crY1 = crV11 - crV01;
            crX0Y1 = crX0 * crY1;
            crX1Y0 = crX1 * crY0;
            crDet = crX0Y1 - crX1Y0;
            return crDet.GetSign();
        }

        // For a triangle with counterclockwise vertices V0, V1 and V2 and a
        // query point P, ToCircumcircle returns
        //   +1, P outside circumcircle of triangle
        //   -1, P inside circumcircle of triangle
        //    0, P on circumcircle of triangle
        int32_t ToCircumcircle(size_t pIndex, size_t v0Index, size_t v1Index, size_t v2Index) const
        {
            // The expression tree has 43 nodes consisting of 8 input
            // leaves and 35 compute nodes.

            // Use interval arithmetic to determine the sign if possible.
            auto const& inP = (pIndex != negOne ? mVertices[pIndex] : mQueryPoint);
            Vector2<T> const& inV0 = mVertices[v0Index];
            Vector2<T> const& inV1 = mVertices[v1Index];
            Vector2<T> const& inV2 = mVertices[v2Index];

            auto x0 = SWInterval<T>::Sub(inV0[0], inP[0]);
            auto y0 = SWInterval<T>::Sub(inV0[1], inP[1]);
            auto s00 = SWInterval<T>::Add(inV0[0], inP[0]);
            auto s01 = SWInterval<T>::Add(inV0[1], inP[1]);
            auto x1 = SWInterval<T>::Sub(inV1[0], inP[0]);
            auto y1 = SWInterval<T>::Sub(inV1[1], inP[1]);
            auto s10 = SWInterval<T>::Add(inV1[0], inP[0]);
            auto s11 = SWInterval<T>::Add(inV1[1], inP[1]);
            auto x2 = SWInterval<T>::Sub(inV2[0], inP[0]);
            auto y2 = SWInterval<T>::Sub(inV2[1], inP[1]);
            auto s20 = SWInterval<T>::Add(inV2[0], inP[0]);
            auto s21 = SWInterval<T>::Add(inV2[1], inP[1]);
            auto t00 = s00 * x0;
            auto t01 = s01 * y0;
            auto t10 = s10 * x1;
            auto t11 = s11 * y1;
            auto t20 = s20 * x2;
            auto t21 = s21 * y2;
            auto z0 = t00 + t01;
            auto z1 = t10 + t11;
            auto z2 = t20 + t21;
            auto y0z1 = y0 * z1;
            auto y0z2 = y0 * z2;
            auto y1z0 = y1 * z0;
            auto y1z2 = y1 * z2;
            auto y2z0 = y2 * z0;
            auto y2z1 = y2 * z1;
            auto c0 = y1z2 - y2z1;
            auto c1 = y2z0 - y0z2;
            auto c2 = y0z1 - y1z0;
            auto x0c0 = x0 * c0;
            auto x1c1 = x1 * c1;
            auto x2c2 = x2 * c2;
            auto det = x0c0 + x1c1 + x2c2;

            T constexpr zero = 0;
            if (det[0] > zero)
            {
                return -1;
            }
            else if (det[1] < zero)
            {
                return +1;
            }

            // The exact sign of the determinant is not known, so compute
            // the determinant using rational arithmetic.

            // Name the nodes of the expression tree.
            auto const& irP = (pIndex != negOne ? mIRVertices[pIndex] : mIRQueryPoint);
            Vector2<InputRational> const& irV0 = mIRVertices[v0Index];
            Vector2<InputRational> const& irV1 = mIRVertices[v1Index];
            Vector2<InputRational> const& irV2 = mIRVertices[v2Index];

            auto const& crP0 = Copy(irP[0], mCRPool[0]);
            auto const& crP1 = Copy(irP[1], mCRPool[1]);
            auto const& crV00 = Copy(irV0[0], mCRPool[2]);
            auto const& crV01 = Copy(irV0[1], mCRPool[3]);
            auto const& crV10 = Copy(irV1[0], mCRPool[4]);
            auto const& crV11 = Copy(irV1[1], mCRPool[5]);
            auto const& crV20 = Copy(irV2[0], mCRPool[6]);
            auto const& crV21 = Copy(irV2[1], mCRPool[7]);

            auto& crX0 = mCRPool[8];
            auto& crY0 = mCRPool[9];
            auto& crS00 = mCRPool[10];
            auto& crS01 = mCRPool[11];
            auto& crT00 = mCRPool[12];
            auto& crT01 = mCRPool[13];
            auto& crZ0 = mCRPool[14];

            auto& crX1 = mCRPool[15];
            auto& crY1 = mCRPool[16];
            auto& crS10 = mCRPool[17];
            auto& crS11 = mCRPool[18];
            auto& crT10 = mCRPool[19];
            auto& crT11 = mCRPool[20];
            auto& crZ1 = mCRPool[21];

            auto& crX2 = mCRPool[22];
            auto& crY2 = mCRPool[23];
            auto& crS20 = mCRPool[24];
            auto& crS21 = mCRPool[25];
            auto& crT20 = mCRPool[26];
            auto& crT21 = mCRPool[27];
            auto& crZ2 = mCRPool[28];

            auto& crY0Z1 = mCRPool[29];
            auto& crY0Z2 = mCRPool[30];
            auto& crY1Z0 = mCRPool[31];
            auto& crY1Z2 = mCRPool[32];
            auto& crY2Z0 = mCRPool[33];
            auto& crY2Z1 = mCRPool[34];

            auto& crC0 = mCRPool[35];
            auto& crC1 = mCRPool[36];
            auto& crC2 = mCRPool[37];
            auto& crX0C0 = mCRPool[38];
            auto& crX1C1 = mCRPool[39];
            auto& crX2C2 = mCRPool[40];
            auto& crTerm = mCRPool[41];
            auto& crDet = mCRPool[42];

            // Evaluate the expression tree.
            crX0 = crV00 - crP0;
            crY0 = crV01 - crP1;
            crS00 = crV00 + crP0;
            crS01 = crV01 + crP1;
            crT00 = crS00 * crX0;
            crT01 = crS01 * crY0;
            crZ0 = crT00 + crT01;

            crX1 = crV10 - crP0;
            crY1 = crV11 - crP1;
            crS10 = crV10 + crP0;
            crS11 = crV11 + crP1;
            crT10 = crS10 * crX1;
            crT11 = crS11 * crY1;
            crZ1 = crT10 + crT11;

            crX2 = crV20 - crP0;
            crY2 = crV21 - crP1;
            crS20 = crV20 + crP0;
            crS21 = crV21 + crP1;
            crT20 = crS20 * crX2;
            crT21 = crS21 * crY2;
            crZ2 = crT20 + crT21;

            crY0Z1 = crY0 * crZ1;
            crY0Z2 = crY0 * crZ2;
            crY1Z0 = crY1 * crZ0;
            crY1Z2 = crY1 * crZ2;
            crY2Z0 = crY2 * crZ0;
            crY2Z1 = crY2 * crZ1;

            crC0 = crY1Z2 - crY2Z1;
            crC1 = crY2Z0 - crY0Z2;
            crC2 = crY0Z1 - crY1Z0;
            crX0C0 = crX0 * crC0;
            crX1C1 = crX1 * crC1;
            crX2C2 = crX2 * crC2;
            crTerm = crX0C0 + crX1C1;
            crDet = crTerm + crX2C2;
            return -crDet.GetSign();
        }

        bool GetContainingTriangle(size_t pIndex, Triangle*& tri) const
        {
            size_t const numTriangles = mGraph.GetTriangles().size();
            for (size_t t = 0; t < numTriangles; ++t)
            {
                size_t j;
                for (j = 0; j < 3; ++j)
                {
                    size_t v0Index = static_cast<size_t>(tri->V[mIndex[j][0]]);
                    size_t v1Index = static_cast<size_t>(tri->V[mIndex[j][1]]);
                    if (ToLine(pIndex, v0Index, v1Index) > 0)
                    {
                        // Point i sees edge <v0,v1> from outside the triangle.
                        auto adjTri = tri->T[j];
                        if (adjTri)
                        {
                            // Traverse to the triangle sharing the face.
                            tri = adjTri;
                            break;
                        }
                        else
                        {
                            // We reached a hull edge, so the point is outside
                            // the hull.
                            return false;
                        }
                    }

                }

                if (j == 3)
                {
                    // The point is inside all four edges, so the point is
                    // inside a triangle.
                    return true;
                }
            }

            LogError("Unexpected termination of loop while searching for a triangle.");
        }

        void GetAndRemoveInsertionPolygon(size_t pIndex,
            TrianglePtrSet& candidates, DirectedEdgeKeySet& boundary)
        {
            // Locate the triangles that make up the insertion polygon.
            ETManifoldMesh polygon;
            while (candidates.size() > 0)
            {
                Triangle* tri = *candidates.begin();
                candidates.erase(candidates.begin());

                for (size_t j = 0; j < 3; ++j)
                {
                    auto adj = tri->T[j];
                    if (adj && candidates.find(adj) == candidates.end())
                    {
                        size_t v0Index = adj->V[0];
                        size_t v1Index = adj->V[1];
                        size_t v2Index = adj->V[2];
                        if (ToCircumcircle(pIndex, v0Index, v1Index, v2Index) <= 0)
                        {
                            // Point P is in the circumcircle.
                            candidates.insert(adj);
                        }
                    }
                }

                auto inserted = polygon.Insert(tri->V[0], tri->V[1], tri->V[2]);
                LogAssert(inserted != nullptr, "Unexpected insertion failure.");
                auto removed = mGraph.Remove(tri->V[0], tri->V[1], tri->V[2]);
                LogAssert(removed, "Unexpected removal failure.");
            }

            // Get the boundary edges of the insertion polygon.
            for (auto const& element : polygon.GetTriangles())
            {
                Triangle* tri = element.second.get();
                for (size_t j = 0; j < 3; ++j)
                {
                    if (!tri->T[j])
                    {
                        EdgeKey<true> ekey(tri->V[mIndex[j][0]], tri->V[mIndex[j][1]]);
                        boundary.insert(ekey);
                    }
                }
            }
        }

        void Update(size_t pIndex)
        {
            auto const& tmap = mGraph.GetTriangles();
            Triangle* tri = tmap.begin()->second.get();
            if (GetContainingTriangle(pIndex, tri))
            {
                // The point is inside the convex hull. The insertion polygon
                // contains only triangles in the current triangulation; the
                // hull does not change.

                // Use a depth-first search for those triangles whose
                // circumcircles contain point P.
                TrianglePtrSet candidates;
                candidates.insert(tri);

                // Get the boundary of the insertion polygon C that contains
                // the triangles whose circumcircles contain point P. Polygon
                // Polygon C contains this point.
                DirectedEdgeKeySet boundary;
                GetAndRemoveInsertionPolygon(pIndex, candidates, boundary);

                // The insertion polygon consists of the triangles formed by
                // point P and the faces of C.
                for (auto const& key : boundary)
                {
                    size_t v0Index = static_cast<size_t>(key.V[0]);
                    size_t v1Index = static_cast<size_t>(key.V[1]);
                    if (ToLine(pIndex, v0Index, v1Index) < 0)
                    {
                        auto inserted = mGraph.Insert(static_cast<int32_t>(pIndex),
                            key.V[0], key.V[1]);
                        LogAssert(inserted != nullptr, "Unexpected insertion failure.");
                    }
                }
            }
            else
            {
                // The point is outside the convex hull. The insertion
                // polygon is formed by point P and any triangles in the
                // current triangulation whose circumcircles contain point P.

                // Locate the convex hull of the triangles.
                DirectedEdgeKeySet hull;
                for (auto const& element : tmap)
                {
                    Triangle* t = element.second.get();
                    for (size_t j = 0; j < 3; ++j)
                    {
                        if (!t->T[j])
                        {
                            hull.insert(EdgeKey<true>(t->V[mIndex[j][0]], t->V[mIndex[j][1]]));
                        }
                    }
                }

                // Iterate over all the hull edges and use the ones visible to
                // point P to locate the insertion polygon.
                auto const& emap = mGraph.GetEdges();
                TrianglePtrSet candidates;
                DirectedEdgeKeySet visible;
                for (auto const& key : hull)
                {
                    size_t v0Index = static_cast<size_t>(key.V[0]);
                    size_t v1Index = static_cast<size_t>(key.V[1]);
                    if (ToLine(pIndex, v0Index, v1Index) > 0)
                    {
                        auto iter = emap.find(EdgeKey<false>(key.V[0], key.V[1]));
                        if (iter != emap.end() && iter->second->T[1] == nullptr)
                        {
                            auto adj = iter->second->T[0];
                            if (adj && candidates.find(adj) == candidates.end())
                            {
                                size_t a0Index = static_cast<size_t>(adj->V[0]);
                                size_t a1Index = static_cast<size_t>(adj->V[1]);
                                size_t a2Index = static_cast<size_t>(adj->V[2]);
                                if (ToCircumcircle(pIndex, a0Index, a1Index, a2Index) <= 0)
                                {
                                    // Point P is in the circumcircle.
                                    candidates.insert(adj);
                                }
                                else
                                {
                                    // Point P is not in the circumcircle but
                                    // the hull edge is visible.
                                    visible.insert(key);
                                }
                            }
                        }
                        else
                        {
                            LogError("This condition should not occur for rational arithmetic.");
                        }
                    }
                }

                // Get the boundary of the insertion subpolygon C that
                // contains the triangles whose circumcircles contain point P.
                DirectedEdgeKeySet boundary;
                GetAndRemoveInsertionPolygon(pIndex, candidates, boundary);

                // The insertion polygon P consists of the triangles formed by
                // point i and the back edges of C and by the visible edges of
                // mGraph-C.
                for (auto const& key : boundary)
                {
                    size_t v0Index = static_cast<size_t>(key.V[0]);
                    size_t v1Index = static_cast<size_t>(key.V[1]);
                    if (ToLine(pIndex, v0Index, v1Index) < 0)
                    {
                        // This is a back edge of the boundary.
                        auto inserted = mGraph.Insert(static_cast<int32_t>(pIndex),
                            key.V[0], key.V[1]);
                        LogAssert(inserted != nullptr, "Unexpected insertion failure.");
                    }
                }
                for (auto const& key : visible)
                {
                    auto inserted = mGraph.Insert(static_cast<int32_t>(pIndex),
                        key.V[1], key.V[0]);
                    LogAssert(inserted != nullptr, "Unexpected insertion failure.");
                }
            }
        }

        // If a vertex occurs multiple times in the 'vertices' input to the
        // constructor, the first processed occurrence of that vertex has an
        // index stored in this array.  If there are no duplicates, then
        // mDuplicates[i] = i for all i.
        std::vector<size_t> mDuplicates;
        size_t mNumUniqueVertices;

        // If the intrinsic dimension of the input vertices is 0 or 1, the
        // constructor returns early. The caller is responsible for retrieving
        // the dimension and taking an alternate path should the dimension be
        // smaller than 2. If the dimension is 0, all vertices are the same.
        // If the dimension is 1, the vertices lie on a line, in which case
        // the caller can project vertices[] onto the line for further
        // processing.
        size_t mDimension;
        Line2<T> mLine;

        // These are computed by UpdateIndicesAdjacencies(). They are used
        // for point-containment queries in the triangle mesh.
        size_t mNumTriangles;
        std::vector<int32_t> mIndices;
        std::vector<int32_t> mAdjacencies;

    private:
        // Indexing for the vertices of the triangle adjacent to a vertex.
        // The edge adjacent to vertex j is <mIndex[j][0], mIndex[j][1]> and
        // is listed so that the triangle interior is to your left as you walk
        // around the edges.
        std::array<std::array<size_t, 2>, 3> const mIndex;

        // The query point for Update, GetContainingTriangle and
        // GetAndRemoveInsertionPolygon when the point is not an input vertex
        // to the constructor. ToLine and ToCircumcircle are passed indices
        // into the vertex array. When the vertex is valid, mVertices[] and
        // mCRVertices[] are used for lookups. When the vertex is 'negOne', the
        // query point is used for lookups.
        mutable Vector2<T> mQueryPoint;
        mutable Vector2<InputRational> mIRQueryPoint;

        // Sufficient storage for the expression trees related to computing
        // the exact signs in ToLine(...) and ToCircumcircle(...).
        static size_t constexpr maxNumCRPool = 43;
        mutable std::vector<ComputeRational> mCRPool;
    };
}
