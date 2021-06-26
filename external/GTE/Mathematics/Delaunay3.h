// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/PrimalQuery3.h>
#include <Mathematics/TSManifoldMesh.h>
#include <Mathematics/Line.h>
#include <Mathematics/Hyperplane.h>
#include <set>
#include <vector>

// Delaunay tetrahedralization of points (intrinsic dimensionality 3).
//   VQ = number of vertices
//   V  = array of vertices
//   TQ = number of tetrahedra
//   I  = Array of 4-tuples of indices into V that represent the tetrahedra
//        (4*TQ total elements).  Access via GetIndices(*).
//   A  = Array of 4-tuples of indices into I that represent the adjacent
//        tetrahedra (4*TQ total elements).  Access via GetAdjacencies(*).
// The i-th tetrahedron has vertices
//   vertex[0] = V[I[4*i+0]]
//   vertex[1] = V[I[4*i+1]]
//   vertex[2] = V[I[4*i+2]]
//   vertex[3] = V[I[4*i+3]]
// and face index triples listed below.  The face vertex ordering when
// viewed from outside the tetrahedron is counterclockwise.
//   face[0] = <I[4*i+1],I[4*i+2],I[4*i+3]>
//   face[1] = <I[4*i+0],I[4*i+3],I[4*i+2]>
//   face[2] = <I[4*i+0],I[4*i+1],I[4*i+3]>
//   face[3] = <I[4*i+0],I[4*i+2],I[4*i+1]>
// The tetrahedra adjacent to these faces have indices
//   adjacent[0] = A[4*i+0] is the tetrahedron opposite vertex[0], so it
//                 is the tetrahedron sharing face[0].
//   adjacent[1] = A[4*i+1] is the tetrahedron opposite vertex[1], so it
//                 is the tetrahedron sharing face[1].
//   adjacent[2] = A[4*i+2] is the tetrahedron opposite vertex[2], so it
//                 is the tetrahedron sharing face[2].
//   adjacent[3] = A[4*i+3] is the tetrahedron opposite vertex[3], so it
//                 is the tetrahedron sharing face[3].
// If there is no adjacent tetrahedron, the A[*] value is set to -1.  The
// tetrahedron adjacent to face[j] has vertices
//   adjvertex[0] = V[I[4*adjacent[j]+0]]
//   adjvertex[1] = V[I[4*adjacent[j]+1]]
//   adjvertex[2] = V[I[4*adjacent[j]+2]]
//   adjvertex[3] = V[I[4*adjacent[j]+3]]
// The only way to ensure a correct result for the input vertices (assumed to
// be exact) is to choose ComputeType for exact rational arithmetic.  You may
// use BSNumber.  No divisions are performed in this computation, so you do
// not have to use BSRational.
//
// The worst-case choices of N for Real of type BSNumber or BSRational with
// integer storage UIntegerFP32<N> are listed in the next table.  The numerical
// computations are encapsulated in PrimalQuery3<Real>::ToPlane and
// PrimalQuery3<Real>::ToCircumsphere, the latter query the dominant one in
// determining N.  We recommend using only BSNumber, because no divisions are
// performed in the convex-hull computations.
//
//    input type | compute type | N
//    -----------+--------------+--------
//    float      | BSNumber     |      44
//    float      | BSRational   |     329
//    double     | BSNumber     |  298037
//    double     | BSRational   | 2254442

namespace gte
{
    template <typename InputType, typename ComputeType>
    class Delaunay3
    {
    public:
        // The class is a functor to support computing the Delaunay
        // tetrahedralization of multiple data sets using the same class
        // object.
        Delaunay3()
            :
            mEpsilon((InputType)0),
            mDimension(0),
            mLine(Vector3<InputType>::Zero(), Vector3<InputType>::Zero()),
            mPlane(Vector3<InputType>::Zero(), (InputType)0),
            mNumVertices(0),
            mNumUniqueVertices(0),
            mNumTetrahedra(0),
            mVertices(nullptr)
        {
        }

        // The input is the array of vertices whose Delaunay
        // tetrahedralization is required.  The epsilon value is used to
        // determine the intrinsic dimensionality of the vertices
        // (d = 0, 1, 2, or 3).  When epsilon is positive, the determination
        // is fuzzy--vertices approximately the same point, approximately on
        // a line, approximately planar, or volumetric.
        bool operator()(int numVertices, Vector3<InputType> const* vertices, InputType epsilon)
        {
            mEpsilon = std::max(epsilon, (InputType)0);
            mDimension = 0;
            mLine.origin = Vector3<InputType>::Zero();
            mLine.direction = Vector3<InputType>::Zero();
            mPlane.normal = Vector3<InputType>::Zero();
            mPlane.constant = (InputType)0;
            mNumVertices = numVertices;
            mNumUniqueVertices = 0;
            mNumTetrahedra = 0;
            mVertices = vertices;
            mGraph = TSManifoldMesh();
            mIndices.clear();
            mAdjacencies.clear();

            int i, j;
            if (mNumVertices < 4)
            {
                // Delaunay3 should be called with at least four points.
                return false;
            }

            IntrinsicsVector3<InputType> info(mNumVertices, vertices, mEpsilon);
            if (info.dimension == 0)
            {
                // mDimension is 0; mGraph, mIndices, and mAdjacencies are
                // empty
                return false;
            }

            if (info.dimension == 1)
            {
                // The set is (nearly) collinear.
                mDimension = 1;
                mLine = Line3<InputType>(info.origin, info.direction[0]);
                return false;
            }

            if (info.dimension == 2)
            {
                // The set is (nearly) coplanar.
                mDimension = 2;
                mPlane = Plane3<InputType>(UnitCross(info.direction[0],
                    info.direction[1]), info.origin);
                return false;
            }

            mDimension = 3;

            // Compute the vertices for the queries.
            mComputeVertices.resize(mNumVertices);
            mQuery.Set(mNumVertices, &mComputeVertices[0]);
            for (i = 0; i < mNumVertices; ++i)
            {
                for (j = 0; j < 3; ++j)
                {
                    mComputeVertices[i][j] = vertices[i][j];
                }
            }

            // Insert the (nondegenerate) tetrahedron constructed by the call
            // to GetInformation. This is necessary for the circumsphere
            // visibility algorithm to work correctly.
            if (!info.extremeCCW)
            {
                std::swap(info.extreme[2], info.extreme[3]);
            }
            if (!mGraph.Insert(info.extreme[0], info.extreme[1], info.extreme[2], info.extreme[3]))
            {
                return false;
            }

            // Incrementally update the tetrahedralization.  The set of
            // processed points is maintained to eliminate duplicates, either
            // in the original input points or in the points obtained by snap
            // rounding.
            std::set<Vector3<InputType>> processed;
            for (i = 0; i < 4; ++i)
            {
                processed.insert(vertices[info.extreme[i]]);
            }
            for (i = 0; i < mNumVertices; ++i)
            {
                if (processed.find(vertices[i]) == processed.end())
                {
                    if (!Update(i))
                    {
                        // A failure can occur if ComputeType is not an exact
                        // arithmetic type.
                        return false;
                    }
                    processed.insert(vertices[i]);
                }
            }
            mNumUniqueVertices = static_cast<int>(processed.size());

            // Assign integer values to the tetrahedra for use by the caller.
            std::map<std::shared_ptr<Tetrahedron>, int> permute;
            i = -1;
            permute[nullptr] = i++;
            for (auto const& element : mGraph.GetTetrahedra())
            {
                permute[element.second] = i++;
            }

            // Put Delaunay tetrahedra into an array (vertices and adjacency
            // info).
            mNumTetrahedra = static_cast<int>(mGraph.GetTetrahedra().size());
            int numIndices = 4 * mNumTetrahedra;
            if (mNumTetrahedra > 0)
            {
                mIndices.resize(numIndices);
                mAdjacencies.resize(numIndices);
                i = 0;
                for (auto const& element : mGraph.GetTetrahedra())
                {
                    std::shared_ptr<Tetrahedron> tetra = element.second;
                    for (j = 0; j < 4; ++j, ++i)
                    {
                        mIndices[i] = tetra->V[j];
                        mAdjacencies[i] = permute[tetra->S[j].lock()];
                    }
                }
            }

            return true;
        }

        // Dimensional information.  If GetDimension() returns 1, the points
        // lie on a line P+t*D (fuzzy comparison when epsilon > 0).  You can
        // sort these if you need a polyline output by projecting onto the
        // line each vertex X = P+t*D, where t = Dot(D,X-P).  If
        // GetDimension() returns 2, the points line on a plane P+s*U+t*V
        // (fuzzy comparison when epsilon > 0).  You can project each vertex
        // X = P+s*U+t*V, where s = Dot(U,X-P) and t = Dot(V,X-P), then apply
        // Delaunay2 to the (s,t) tuples.
        inline InputType GetEpsilon() const
        {
            return mEpsilon;
        }

        inline int GetDimension() const
        {
            return mDimension;
        }

        inline Line3<InputType> const& GetLine() const
        {
            return mLine;
        }

        inline Plane3<InputType> const& GetPlane() const
        {
            return mPlane;
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

        inline int GetNumTetrahedra() const
        {
            return mNumTetrahedra;
        }

        inline Vector3<InputType> const* GetVertices() const
        {
            return mVertices;
        }

        inline PrimalQuery3<ComputeType> const& GetQuery() const
        {
            return mQuery;
        }

        inline TSManifoldMesh const& GetGraph() const
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

        // Locate those tetrahedra faces that do not share other tetrahedra.
        // The returned array has hull.size() = 3*numFaces indices, each
        // triple representing a triangle.  The triangles are counterclockwise
        // ordered when viewed from outside the hull.  The return value is
        // 'true' iff the dimension is 3.
        bool GetHull(std::vector<int>& hull) const
        {
            if (mDimension == 3)
            {
                // Count the number of triangles that are not shared by two
                // tetrahedra.
                int numTriangles = 0;
                for (auto adj : mAdjacencies)
                {
                    if (adj == -1)
                    {
                        ++numTriangles;
                    }
                }

                if (numTriangles > 0)
                {
                    // Enumerate the triangles.  The prototypical case is the
                    // single tetrahedron V[0] = (0,0,0), V[1] = (1,0,0),
                    // V[2] = (0,1,0) and V[3] = (0,0,1) with no adjacent
                    // tetrahedra.  The mIndices[] array is <0,1,2,3>.
                    //   i = 0, face = 0:
                    //    skip index 0, <x,1,2,3>, no swap, triangle = <1,2,3>
                    //   i = 1, face = 1:
                    //    skip index 1, <0,x,2,3>, swap,    triangle = <0,3,2>
                    //   i = 2, face = 2:
                    //    skip index 2, <0,1,x,3>, no swap, triangle = <0,1,3>
                    //   i = 3, face = 3:
                    //    skip index 3, <0,1,2,x>, swap,    triangle = <0,2,1>
                    // To guarantee counterclockwise order of triangles when
                    // viewed outside the tetrahedron, the swap of the last
                    // two indices occurs when face is an odd number;
                    // (face % 2) != 0.
                    hull.resize(3 * numTriangles);
                    int current = 0, i = 0;
                    for (auto adj : mAdjacencies)
                    {
                        if (adj == -1)
                        {
                            int tetra = i / 4, face = i % 4;
                            for (int j = 0; j < 4; ++j)
                            {
                                if (j != face)
                                {
                                    hull[current++] = mIndices[4 * tetra + j];
                                }
                            }
                            if ((face % 2) != 0)
                            {
                                std::swap(hull[current - 1], hull[current - 2]);
                            }
                        }
                        ++i;
                    }
                    return true;
                }
                else
                {
                    LogError("Unexpected. There must be at least one tetrahedron.");
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
        }

        // Get the vertex indices for tetrahedron i.  The function returns
        // 'true' when the dimension is 3 and i is a valid tetrahedron index,
        // in which case the vertices are valid; otherwise, the function
        // returns 'false' and the vertices are invalid.
        bool GetIndices(int i, std::array<int, 4>& indices) const
        {
            if (mDimension == 3)
            {
                int numTetrahedra = static_cast<int>(mIndices.size() / 4);
                if (0 <= i && i < numTetrahedra)
                {
                    indices[0] = mIndices[4 * i];
                    indices[1] = mIndices[4 * i + 1];
                    indices[2] = mIndices[4 * i + 2];
                    indices[3] = mIndices[4 * i + 3];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
            return false;
        }

        // Get the indices for tetrahedra adjacent to tetrahedron i.  The
        // function returns 'true' when the dimension is 3 and i is a valid
        // tetrahedron index, in which case the adjacencies are valid;
        // otherwise, the function returns 'false' and the adjacencies are
        // invalid.
        bool GetAdjacencies(int i, std::array<int, 4>& adjacencies) const
        {
            if (mDimension == 3)
            {
                int numTetrahedra = static_cast<int>(mIndices.size() / 4);
                if (0 <= i && i < numTetrahedra)
                {
                    adjacencies[0] = mAdjacencies[4 * i];
                    adjacencies[1] = mAdjacencies[4 * i + 1];
                    adjacencies[2] = mAdjacencies[4 * i + 2];
                    adjacencies[3] = mAdjacencies[4 * i + 3];
                    return true;
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
            return false;
        }

        // Support for searching the tetrahedralization for a tetrahedron
        // that contains a point.  If there is a containing tetrahedron, the
        // returned value is a tetrahedron index i with
        // 0 <= i < GetNumTetrahedra().  If there is not a containing
        // tetrahedron, -1 is returned.  The computations are performed using
        // exact rational arithmetic.
        //
        // The SearchInfo input stores information about the tetrahedron
        // search when looking for the tetrahedron (if any) that contains p.
        // The first tetrahedron searched is 'initialTetrahedron'.  On return
        // 'path' stores those (ordered) tetrahedron indices visited during
        // the search.  The last visited tetrahedron has index
        // 'finalTetrahedron' and vertex indices 'finalV[0,1,2,3]', stored in
        // volumetric counterclockwise order.  The last face of the search is
        // <finalV[0],finalV[1],finalV[2]>.  For spatially coherent inputs p
        // for numerous calls to this function, you will want to specify
        // 'finalTetrahedron' from the previous call as 'initialTetrahedron'
        // for the next call, which should reduce search times.
        struct SearchInfo
        {
            int initialTetrahedron;
            int numPath;
            std::vector<int> path;
            int finalTetrahedron;
            std::array<int, 4> finalV;
        };
        
        int GetContainingTetrahedron(Vector3<InputType> const& p, SearchInfo& info) const
        {
            if (mDimension == 3)
            {
                Vector3<ComputeType> test{ p[0], p[1], p[2] };

                int numTetrahedra = static_cast<int>(mIndices.size() / 4);
                info.path.resize(numTetrahedra);
                info.numPath = 0;
                int tetrahedron;
                if (0 <= info.initialTetrahedron
                    && info.initialTetrahedron < numTetrahedra)
                {
                    tetrahedron = info.initialTetrahedron;
                }
                else
                {
                    info.initialTetrahedron = 0;
                    tetrahedron = 0;
                }

                // Use tetrahedron faces as binary separating planes.
                for (int i = 0; i < numTetrahedra; ++i)
                {
                    int ibase = 4 * tetrahedron;
                    int const* v = &mIndices[ibase];

                    info.path[info.numPath++] = tetrahedron;
                    info.finalTetrahedron = tetrahedron;
                    info.finalV[0] = v[0];
                    info.finalV[1] = v[1];
                    info.finalV[2] = v[2];
                    info.finalV[3] = v[3];

                    // <V1,V2,V3> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[1], v[2], v[3]) > 0)
                    {
                        tetrahedron = mAdjacencies[ibase];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[1];
                            info.finalV[1] = v[2];
                            info.finalV[2] = v[3];
                            info.finalV[3] = v[0];
                            return -1;
                        }
                        continue;
                    }

                    // <V0,V3,V2> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[0], v[2], v[3]) < 0)
                    {
                        tetrahedron = mAdjacencies[ibase + 1];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[2];
                            info.finalV[2] = v[3];
                            info.finalV[3] = v[1];
                            return -1;
                        }
                        continue;
                    }

                    // <V0,V1,V3> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[0], v[1], v[3]) > 0)
                    {
                        tetrahedron = mAdjacencies[ibase + 2];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[1];
                            info.finalV[2] = v[3];
                            info.finalV[3] = v[2];
                            return -1;
                        }
                        continue;
                    }

                    // <V0,V2,V1> counterclockwise when viewed outside
                    // tetrahedron.
                    if (mQuery.ToPlane(test, v[0], v[1], v[2]) < 0)
                    {
                        tetrahedron = mAdjacencies[ibase + 3];
                        if (tetrahedron == -1)
                        {
                            info.finalV[0] = v[0];
                            info.finalV[1] = v[1];
                            info.finalV[2] = v[2];
                            info.finalV[3] = v[3];
                            return -1;
                        }
                        continue;
                    }

                    return tetrahedron;
                }
            }
            else
            {
                LogError("The dimension must be 3.");
            }
            return -1;
        }

    private:
        // Support for incremental Delaunay tetrahedralization.
        typedef TSManifoldMesh::Tetrahedron Tetrahedron;

        bool GetContainingTetrahedron(int i, std::shared_ptr<Tetrahedron>& tetra) const
        {
            int numTetrahedra = static_cast<int>(mGraph.GetTetrahedra().size());
            for (int t = 0; t < numTetrahedra; ++t)
            {
                int j;
                for (j = 0; j < 4; ++j)
                {
                    auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                    int v0 = tetra->V[opposite[j][0]];
                    int v1 = tetra->V[opposite[j][1]];
                    int v2 = tetra->V[opposite[j][2]];
                    if (mQuery.ToPlane(i, v0, v1, v2) > 0)
                    {
                        // Point i sees face <v0,v1,v2> from outside the
                        // tetrahedron.
                        auto adjTetra = tetra->S[j].lock();
                        if (adjTetra)
                        {
                            // Traverse to the tetrahedron sharing the face.
                            tetra = adjTetra;
                            break;
                        }
                        else
                        {
                            // We reached a hull face, so the point is outside
                            // the hull.  TODO:  Once a hull data structure is
                            // in place, return tetra->S[j] as the candidate
                            // for starting a search for visible hull faces.
                            return false;
                        }
                    }

                }

                if (j == 4)
                {
                    // The point is inside all four faces, so the point is inside
                    // a tetrahedron.
                    return true;
                }
            }

            LogError("Unexpected termination of loop.");
        }

        bool GetAndRemoveInsertionPolyhedron(int i, std::set<std::shared_ptr<Tetrahedron>>& candidates,
            std::set<TriangleKey<true>>& boundary)
        {
            // Locate the tetrahedra that make up the insertion polyhedron.
            TSManifoldMesh polyhedron;
            while (candidates.size() > 0)
            {
                std::shared_ptr<Tetrahedron> tetra = *candidates.begin();
                candidates.erase(candidates.begin());

                for (int j = 0; j < 4; ++j)
                {
                    auto adj = tetra->S[j].lock();
                    if (adj && candidates.find(adj) == candidates.end())
                    {
                        int a0 = adj->V[0];
                        int a1 = adj->V[1];
                        int a2 = adj->V[2];
                        int a3 = adj->V[3];
                        if (mQuery.ToCircumsphere(i, a0, a1, a2, a3) <= 0)
                        {
                            // Point i is in the circumsphere.
                            candidates.insert(adj);
                        }
                    }
                }

                int v0 = tetra->V[0];
                int v1 = tetra->V[1];
                int v2 = tetra->V[2];
                int v3 = tetra->V[3];
                if (!polyhedron.Insert(v0, v1, v2, v3))
                {
                    return false;
                }
                if (!mGraph.Remove(v0, v1, v2, v3))
                {
                    return false;
                }
            }

            // Get the boundary triangles of the insertion polyhedron.
            for (auto const& element : polyhedron.GetTetrahedra())
            {
                std::shared_ptr<Tetrahedron> tetra = element.second;
                for (int j = 0; j < 4; ++j)
                {
                    if (!tetra->S[j].lock())
                    {
                        auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                        int v0 = tetra->V[opposite[j][0]];
                        int v1 = tetra->V[opposite[j][1]];
                        int v2 = tetra->V[opposite[j][2]];
                        boundary.insert(TriangleKey<true>(v0, v1, v2));
                    }
                }
            }
            return true;
        }

        bool Update(int i)
        {
            auto const& smap = mGraph.GetTetrahedra();
            std::shared_ptr<Tetrahedron> tetra = smap.begin()->second;
            if (GetContainingTetrahedron(i, tetra))
            {
                // The point is inside the convex hull.  The insertion
                // polyhedron contains only tetrahedra in the current
                // tetrahedralization; the hull does not change.

                // Use a depth-first search for those tetrahedra whose
                // circumspheres contain point i.
                std::set<std::shared_ptr<Tetrahedron>> candidates;
                candidates.insert(tetra);

                // Get the boundary of the insertion polyhedron C that
                // contains the tetrahedra whose circumspheres contain point
                // i.  Polyhedron C contains the point i.
                std::set<TriangleKey<true>> boundary;
                if (!GetAndRemoveInsertionPolyhedron(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polyhedron consists of the tetrahedra formed
                // by point i and the faces of C.
                for (auto const& key : boundary)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    int v2 = key.V[2];
                    if (mQuery.ToPlane(i, v0, v1, v2) < 0)
                    {
                        if (!mGraph.Insert(i, v0, v1, v2))
                        {
                            return false;
                        }
                    }
                    // else:  Point i is on an edge or face of 'tetra', so the
                    // subdivision has degenerate tetrahedra.  Ignore these.
                }
            }
            else
            {
                // The point is outside the convex hull.  The insertion
                // polyhedron is formed by point i and any tetrahedra in the
                // current tetrahedralization whose circumspheres contain
                // point i.

                // Locate the convex hull of the tetrahedra.  TODO:  Maintain
                // a hull data structure that is updated incrementally.
                std::set<TriangleKey<true>> hull;
                for (auto const& element : smap)
                {
                    std::shared_ptr<Tetrahedron> t = element.second;
                    for (int j = 0; j < 4; ++j)
                    {
                        if (!t->S[j].lock())
                        {
                            auto const& opposite = TetrahedronKey<true>::GetOppositeFace();
                            int v0 = t->V[opposite[j][0]];
                            int v1 = t->V[opposite[j][1]];
                            int v2 = t->V[opposite[j][2]];
                            hull.insert(TriangleKey<true>(v0, v1, v2));
                        }
                    }
                }

                // TODO:  Until the hull change, for now just iterate over all
                // the hull faces and use the ones visible to point i to
                // locate the insertion polyhedron.
                auto const& tmap = mGraph.GetTriangles();
                std::set<std::shared_ptr<Tetrahedron>> candidates;
                std::set<TriangleKey<true>> visible;
                for (auto const& key : hull)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    int v2 = key.V[2];
                    if (mQuery.ToPlane(i, v0, v1, v2) > 0)
                    {
                        auto iter = tmap.find(TriangleKey<false>(v0, v1, v2));
                        if (iter != tmap.end() && iter->second->T[1].lock() == nullptr)
                        {
                            auto adj = iter->second->T[0].lock();
                            if (adj && candidates.find(adj) == candidates.end())
                            {
                                int a0 = adj->V[0];
                                int a1 = adj->V[1];
                                int a2 = adj->V[2];
                                int a3 = adj->V[3];
                                if (mQuery.ToCircumsphere(i, a0, a1, a2, a3) <= 0)
                                {
                                    // Point i is in the circumsphere.
                                    candidates.insert(adj);
                                }
                                else
                                {
                                    // Point i is not in the circumsphere but
                                    // the hull face is visible.
                                    visible.insert(key);
                                }
                            }
                        }
                        else
                        {
                            // TODO:  Add a preprocessor symbol to this file
                            // to allow throwing an exception.  Currently, the
                            // code exits gracefully when floating-point
                            // rounding causes problems.
                            //
                            // LogError("Unexpected condition (ComputeType not exact?)");
                            return false;
                        }
                    }
                }

                // Get the boundary of the insertion subpolyhedron C that
                // contains the tetrahedra whose circumspheres contain
                // point i.
                std::set<TriangleKey<true>> boundary;
                if (!GetAndRemoveInsertionPolyhedron(i, candidates, boundary))
                {
                    return false;
                }

                // The insertion polyhedron P consists of the tetrahedra
                // formed by point i and the back faces of C *and* the visible
                // faces of mGraph-C.
                for (auto const& key : boundary)
                {
                    int v0 = key.V[0];
                    int v1 = key.V[1];
                    int v2 = key.V[2];
                    if (mQuery.ToPlane(i, v0, v1, v2) < 0)
                    {
                        // This is a back face of the boundary.
                        if (!mGraph.Insert(i, v0, v1, v2))
                        {
                            return false;
                        }
                    }
                }
                for (auto const& key : visible)
                {
                    if (!mGraph.Insert(i, key.V[0], key.V[2], key.V[1]))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        // The epsilon value is used for fuzzy determination of intrinsic
        // dimensionality.  If the dimension is 0, 1, or 2, the constructor
        // returns early.  The caller is responsible for retrieving the
        // dimension and taking an alternate path should the dimension be
        // smaller than 3.  If the dimension is 0, the caller may as well
        // treat all vertices[] as a single point, say, vertices[0].  If the
        // dimension is 1, the caller can query for the approximating line
        // and project vertices[] onto it for further processing.  If the
        // dimension is 2, the caller can query for the approximating plane
        // and project vertices[] onto it for further processing.
        InputType mEpsilon;
        int mDimension;
        Line3<InputType> mLine;
        Plane3<InputType> mPlane;

        // The array of vertices used for geometric queries.  If you want to
        // be certain of a correct result, choose ComputeType to be BSNumber.
        std::vector<Vector3<ComputeType>> mComputeVertices;
        PrimalQuery3<ComputeType> mQuery;

        // The graph information.
        int mNumVertices;
        int mNumUniqueVertices;
        int mNumTetrahedra;
        Vector3<InputType> const* mVertices;
        TSManifoldMesh mGraph;
        std::vector<int> mIndices;
        std::vector<int> mAdjacencies;
    };
}
