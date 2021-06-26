// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.22

#pragma once

#include <Mathematics/ContPointInPolygon2.h>
#include <Mathematics/ETManifoldMesh.h>
#include <Mathematics/PrimalQuery2.h>

// The planar mesh class is convenient for many applications involving
// searches for triangles containing a specified point.  A couple of
// issues can show up in practice when the input data to the constructors
// is very large (number of triangles on the order of 10^5 or larger).
//
// The first constructor builds an ETManifoldMesh mesh that contains
// std::map objects.  When such maps are large, the amount of time it
// takes to delete them is enormous.  Although you can control the level
// of debug support in MSVS 2013 (see _ITERATOR_DEBUG_LEVEL), turning off
// checking might very well affect other classes for which you want
// iterator checking to be on.  An alternative to reduce debugging time
// is to dynamically allocate the PlanarMesh object in the main thread but
// then launch another thread to delete the object and avoid stalling
// the main thread.  For example,
//
//    PlanarMesh<IT,CT,RT>* pmesh =
//        new PlanarMesh<IT,CT,RT>(numV, vertices, numT, indices);
//    <make various calls to pmesh>;
//    std::thread deleter = [pmesh](){ delete pmesh; };
//    deleter.detach();  // Do not wait for the thread to finish.
//
// The second constructor has the mesh passed in, but mTriIndexMap is used
// in both constructors and can take time to delete.
//
// The input mesh should be consistently oriented, say, the triangles are
// counterclockwise ordered.  The vertices should be consistent with this
// ordering.  However, floating-point rounding errors in generating the
// vertices can cause apparent fold-over of the mesh; that is, theoretically
// the vertex geometry supports counterclockwise geometry but numerical
// errors cause an inconsistency.  This can manifest in the mQuery.ToLine
// tests whereby cycles of triangles occur in the linear walk.  When cycles
// occur, GetContainingTriangle(P,startTriangle) will iterate numTriangle
// times before reporting that the triangle cannot be found, which is a
// very slow process (in debug or release builds).  The function
// GetContainingTriangle(P,startTriangle,visited) is provided to avoid the
// performance loss, trapping a cycle the first time and exiting, but
// again reporting that the triangle cannot be found.  If you know that the
// query should be (theoretically) successful, use the second version of
// GetContainingTriangle.  If it fails by returning -1, then perform an
// exhaustive search over the triangles.  For example,
//
//    int triangle = pmesh->GetContainingTriangle(P,startTriangle,visited);
//    if (triangle >= 0)
//    {
//        <take action; for example, compute barycenteric coordinates>;
//    }
//    else
//    {
//        int numTriangles = pmesh->GetNumTriangles();
//        for (triangle = 0; triangle < numTriangles; ++triangle)
//        {
//            if (pmesh->Contains(triangle, P))
//            {
//                <take action>;
//                break;
//            }
//        }
//        if (triangle == numTriangles)
//        {
//            <Triangle still not found, take appropriate action>;
//        }
//    }
//
// The PlanarMesh<*>::Contains function does not require the triangles to
// be ordered.

namespace gte
{
    template <typename InputType, typename ComputeType, typename RationalType>
    class PlanarMesh
    {
    public:
        // Construction.  The inputs must represent a manifold mesh of
        // triangles in the plane.  The index array must have 3*numTriangles
        // elements, each triple of indices representing a triangle in the
        // mesh.  Each index is into the 'vertices' array.
        PlanarMesh(int numVertices, Vector2<InputType> const* vertices, int numTriangles, int const* indices)
            :
            mNumVertices(0),
            mVertices(nullptr),
            mNumTriangles(0)
        {
            LogAssert(numVertices >= 3 && vertices != nullptr && numTriangles >= 1
                && indices != nullptr, "Invalid input.");

            // Create a mesh in order to get adjacency information.
            int const* current = indices;
            for (int t = 0; t < numTriangles; ++t)
            {
                int v0 = *current++;
                int v1 = *current++;
                int v2 = *current++;
                if (!mMesh.Insert(v0, v1, v2))
                {
                    // TODO: Fix this comment once the exception handling is
                    // tested.
                    //
                    // The 'mesh' object will throw on nonmanifold inputs.
                    return;
                }
            }

            // We have a valid mesh.
            CreateVertices(numVertices, vertices);

            // Build the adjacency graph using the triangle ordering implied
            // by the indices, not the mesh triangle map, to preserve the
            // triangle ordering of the input indices.
            mNumTriangles = numTriangles;
            int const numIndices = 3 * numTriangles;
            mIndices.resize(numIndices);

            std::copy(indices, indices + numIndices, mIndices.begin());
            for (int t = 0, vIndex = 0; t < numTriangles; ++t)
            {
                int v0 = indices[vIndex++];
                int v1 = indices[vIndex++];
                int v2 = indices[vIndex++];
                TriangleKey<true> key(v0, v1, v2);
                mTriIndexMap.insert(std::make_pair(key, t));
            }

            mAdjacencies.resize(numIndices);
            auto const& tmap = mMesh.GetTriangles();
            for (int t = 0, base = 0; t < numTriangles; ++t, base += 3)
            {
                int v0 = indices[base];
                int v1 = indices[base + 1];
                int v2 = indices[base + 2];
                TriangleKey<true> key(v0, v1, v2);
                auto element = tmap.find(key);
                for (int i = 0; i < 3; ++i)
                {
                    auto adj = element->second->T[i];
                    if (adj)
                    {
                        key = TriangleKey<true>(adj->V[0], adj->V[1], adj->V[2]);
                        mAdjacencies[base + i] = mTriIndexMap.find(key)->second;
                    }
                    else
                    {
                        mAdjacencies[base + i] = -1;
                    }
                }
            }
        }

        PlanarMesh(int numVertices, Vector2<InputType> const* vertices, ETManifoldMesh const& mesh)
            :
            mNumVertices(0),
            mVertices(nullptr),
            mNumTriangles(0)
        {
            if (numVertices < 3 || !vertices || mesh.GetTriangles().size() < 1)
            {
                throw std::invalid_argument("Invalid input in PlanarMesh constructor.");
            }

            // We have a valid mesh.
            CreateVertices(numVertices, vertices);

            // Build the adjacency graph using the triangle ordering implied
            // by the mesh triangle map.
            auto const& tmap = mesh.GetTriangles();
            mNumTriangles = static_cast<int>(tmap.size());
            mIndices.resize(3 * mNumTriangles);

            int tIndex = 0, vIndex = 0;
            for (auto const& element : tmap)
            {
                mTriIndexMap.insert(std::make_pair(element.first, tIndex++));
                for (int i = 0; i < 3; ++i, ++vIndex)
                {
                    mIndices[vIndex] = element.second->V[i];
                }
            }

            mAdjacencies.resize(3 * mNumTriangles);
            vIndex = 0;
            for (auto const& element : tmap)
            {
                for (int i = 0; i < 3; ++i, ++vIndex)
                {
                    auto adj = element.second->T[i];
                    if (adj)
                    {
                        TriangleKey<true> key(adj->V[0], adj->V[1], adj->V[2]);
                        mAdjacencies[vIndex] = mTriIndexMap.find(key)->second;
                    }
                    else
                    {
                        mAdjacencies[vIndex] = -1;
                    }
                }
            }
        }

        // Mesh information.
        inline int GetNumVertices() const
        {
            return mNumVertices;
        }

        inline int GetNumTriangles() const
        {
            return mNumTriangles;
        }

        inline Vector2<InputType> const* GetVertices() const
        {
            return mVertices;
        }

        inline int const* GetIndices() const
        {
            return mIndices.data();
        }

        inline int const* GetAdjacencies() const
        {
            return mAdjacencies.data();
        }

        // Containment queries.  The function GetContainingTriangle works
        // correctly when the planar mesh is a convex set.  If the mesh is not
        // convex, it is possible that the linear-walk search algorithm exits
        // the mesh before finding a containing triangle.  For example, a
        // C-shaped mesh can contain a point in the top branch of the "C".
        // A starting point in the bottom branch of the "C" will lead to the
        // search exiting the bottom branch and having no path to walk to the
        // top branch.  If your mesh is not convex and you want a correct
        // containment query, you will have to append "outside" triangles to
        // your mesh to form a convex set.
        int GetContainingTriangle(Vector2<InputType> const& P, int startTriangle = 0) const
        {
            Vector2<ComputeType> test{ P[0], P[1] };

            // Use triangle edges as binary separating lines.
            int triangle = startTriangle;
            for (int i = 0; i < mNumTriangles; ++i)
            {
                int ibase = 3 * triangle;
                int const* v = &mIndices[ibase];

                if (mQuery.ToLine(test, v[0], v[1]) > 0)
                {
                    triangle = mAdjacencies[ibase];
                    if (triangle == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[1], v[2]) > 0)
                {
                    triangle = mAdjacencies[ibase + 1];
                    if (triangle == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[2], v[0]) > 0)
                {
                    triangle = mAdjacencies[ibase + 2];
                    if (triangle == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                return triangle;
            }

            return -1;
        }

        int GetContainingTriangle(Vector2<InputType> const& P, int startTriangle, std::set<int>& visited) const
        {
            Vector2<ComputeType> test{ P[0], P[1] };
            visited.clear();

            // Use triangle edges as binary separating lines.
            int triangle = startTriangle;
            for (int i = 0; i < mNumTriangles; ++i)
            {
                visited.insert(triangle);
                int ibase = 3 * triangle;
                int const* v = &mIndices[ibase];

                if (mQuery.ToLine(test, v[0], v[1]) > 0)
                {
                    triangle = mAdjacencies[ibase];
                    if (triangle == -1 || visited.find(triangle) != visited.end())
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[1], v[2]) > 0)
                {
                    triangle = mAdjacencies[ibase + 1];
                    if (triangle == -1 || visited.find(triangle) != visited.end())
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[2], v[0]) > 0)
                {
                    triangle = mAdjacencies[ibase + 2];
                    if (triangle == -1 || visited.find(triangle) != visited.end())
                    {
                        return -1;
                    }
                    continue;
                }

                return triangle;
            }

            return -1;
        }

        bool GetVertices(int t, std::array<Vector2<InputType>, 3>& vertices) const
        {
            if (0 <= t && t < mNumTriangles)
            {
                for (int i = 0, vIndex = 3 * t; i < 3; ++i, ++vIndex)
                {
                    vertices[i] = mVertices[mIndices[vIndex]];
                }
                return true;
            }
            return false;
        }

        bool GetIndices(int t, std::array<int, 3>& indices) const
        {
            if (0 <= t && t < mNumTriangles)
            {
                for (int i = 0, vIndex = 3 * t; i < 3; ++i, ++vIndex)
                {
                    indices[i] = mIndices[vIndex];
                }
                return true;
            }
            return false;
        }

        bool GetAdjacencies(int t, std::array<int, 3>& adjacencies) const
        {
            if (0 <= t && t < mNumTriangles)
            {
                for (int i = 0, vIndex = 3 * t; i < 3; ++i, ++vIndex)
                {
                    adjacencies[i] = mAdjacencies[vIndex];
                }
                return true;
            }
            return false;
        }

        bool GetBarycentrics(int t, Vector2<InputType> const& P, std::array<InputType, 3>& bary) const
        {
            std::array<int, 3> indices;
            if (GetIndices(t, indices))
            {
                Vector2<RationalType> rtP{ P[0], P[1] };
                std::array<Vector2<RationalType>, 3> rtV;
                for (int i = 0; i < 3; ++i)
                {
                    Vector2<ComputeType> const& V = mComputeVertices[indices[i]];
                    for (int j = 0; j < 2; ++j)
                    {
                        rtV[i][j] = (RationalType)V[j];
                    }
                };

                RationalType rtBary[3];
                if (ComputeBarycentrics(rtP, rtV[0], rtV[1], rtV[2], rtBary))
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        bary[i] = (InputType)rtBary[i];
                    }
                    return true;
                }
            }
            return false;
        }

        bool Contains(int triangle, Vector2<InputType> const& P) const
        {
            Vector2<ComputeType> test{ P[0], P[1] };
            Vector2<ComputeType> v[3];
            v[0] = mComputeVertices[mIndices[3 * triangle + 0]];
            v[1] = mComputeVertices[mIndices[3 * triangle + 1]];
            v[2] = mComputeVertices[mIndices[3 * triangle + 2]];
            PointInPolygon2<ComputeType> pip(3, v);
            return pip.Contains(test);
        }

    public:
        void CreateVertices(int numVertices, Vector2<InputType> const* vertices)
        {
            mNumVertices = numVertices;
            mVertices = vertices;
            mComputeVertices.resize(mNumVertices);
            for (int i = 0; i < mNumVertices; ++i)
            {
                for (int j = 0; j < 2; ++j)
                {
                    mComputeVertices[i][j] = (ComputeType)mVertices[i][j];
                }
            }
            mQuery.Set(mNumVertices, &mComputeVertices[0]);
        }

        int mNumVertices;
        Vector2<InputType> const* mVertices;
        int mNumTriangles;
        std::vector<int> mIndices;
        ETManifoldMesh mMesh;
        std::map<TriangleKey<true>, int> mTriIndexMap;
        std::vector<int> mAdjacencies;
        std::vector<Vector2<ComputeType>> mComputeVertices;
        PrimalQuery2<ComputeType> mQuery;
    };
}
