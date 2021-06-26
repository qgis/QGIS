// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/SharedPtrCompare.h>
#include <Mathematics/ETNonmanifoldMesh.h>

// The VETNonmanifoldMesh class represents an edge-triangle nonmanifold mesh
// but additionally stores vertex adjacency information.

namespace gte
{
    class VETNonmanifoldMesh : public ETNonmanifoldMesh
    {
    public:
        // Vertex data types.
        class Vertex;
        typedef std::shared_ptr<Vertex>(*VCreator)(int);
        typedef std::map<int, std::shared_ptr<Vertex>> VMap;

        // Vertex object.
        class Vertex
        {
        public:
            virtual ~Vertex() = default;

            Vertex(int vIndex)
                :
                V(vIndex)
            {
            }

            // The index into the vertex pool of the mesh.
            int V;

            bool operator<(Vertex const& other) const
            {
                return V < other.V;
            }

            // Adjacent objects.
            std::set<int> VAdjacent;
            std::set<std::shared_ptr<Edge>, SharedPtrLT<Edge>> EAdjacent;
            std::set<std::shared_ptr<Triangle>, SharedPtrLT<Triangle>> TAdjacent;
        };


        // Construction and destruction.
        virtual ~VETNonmanifoldMesh() = default;

        VETNonmanifoldMesh(VCreator vCreator = nullptr, ECreator eCreator = nullptr, TCreator tCreator = nullptr)
            :
            ETNonmanifoldMesh(eCreator, tCreator),
            mVCreator(vCreator ? vCreator : CreateVertex)
        {
        }

        // Support for a deep copy of the mesh.  The mVMap, mEMap, and mTMap
        // objects have dynamically allocated memory for vertices, edges, and
        // triangles.  A shallow copy of the pointers to this memory is
        // problematic.  Allowing sharing, say, via std::shared_ptr, is an
        // option but not really the intent of copying the mesh graph.
        VETNonmanifoldMesh(VETNonmanifoldMesh const& mesh)
        {
            *this = mesh;
        }

        VETNonmanifoldMesh& operator=(VETNonmanifoldMesh const& mesh)
        {
            Clear();
            mVCreator = mesh.mVCreator;
            ETNonmanifoldMesh::operator=(mesh);
            return *this;
        }

        // Member access.
        inline VMap const& GetVertices() const
        {
            return mVMap;
        }

        // If <v0,v1,v2> is not in the mesh, a Triangle object is created and
        // returned; otherwise, <v0,v1,v2> is in the mesh and nullptr is
        // returned.
        virtual std::shared_ptr<Triangle> Insert(int v0, int v1, int v2) override
        {
            std::shared_ptr<Triangle> tri = ETNonmanifoldMesh::Insert(v0, v1, v2);
            if (!tri)
            {
                return nullptr;
            }

            for (int i = 0; i < 3; ++i)
            {
                int vIndex = tri->V[i];
                auto vItem = mVMap.find(vIndex);
                std::shared_ptr<Vertex> vertex;
                if (vItem == mVMap.end())
                {
                    vertex = mVCreator(vIndex);
                    mVMap[vIndex] = vertex;
                }
                else
                {
                    vertex = vItem->second;
                }

                vertex->TAdjacent.insert(tri);

                for (int j = 0; j < 3; ++j)
                {
                    auto edge = tri->E[j].lock();
                    LogAssert(edge != nullptr, "Unexpected condition.");
                    if (edge->V[0] == vIndex)
                    {
                        vertex->VAdjacent.insert(edge->V[1]);
                        vertex->EAdjacent.insert(edge);
                    }
                    else if (edge->V[1] == vIndex)
                    {
                        vertex->VAdjacent.insert(edge->V[0]);
                        vertex->EAdjacent.insert(edge);
                    }
                }
            }

            return tri;
        }

        // If <v0,v1,v2> is in the mesh, it is removed and 'true' is returned;
        // otherwise, <v0,v1,v2> is not in the mesh and 'false' is returned.
        virtual bool Remove(int v0, int v1, int v2) override
        {
            auto tItem = mTMap.find(TriangleKey<true>(v0, v1, v2));
            if (tItem == mTMap.end())
            {
                return false;
            }

            std::shared_ptr<Triangle> tri = tItem->second;
            for (int i = 0; i < 3; ++i)
            {
                int vIndex = tri->V[i];
                auto vItem = mVMap.find(vIndex);
                LogAssert(vItem != mVMap.end(), "Unexpected condition.");
                std::shared_ptr<Vertex> vertex = vItem->second;
                for (int j = 0; j < 3; ++j)
                {
                    auto edge = tri->E[j].lock();
                    LogAssert(edge != nullptr, "Unexpected condition.");

                    // If the edge will be removed by
                    // ETNonmanifoldMesh::Remove, remove the vertex
                    // references to it.
                    if (edge->T.size() == 1)
                    {
                        for (auto const& adjw : edge->T)
                        {
                            auto adj = adjw.lock();
                            LogAssert(adj != nullptr, "Unexpected condition.");
                            if (edge->V[0] == vIndex)
                            {
                                vertex->VAdjacent.erase(edge->V[1]);
                                vertex->EAdjacent.erase(edge);
                            }
                            else if (edge->V[1] == vIndex)
                            {
                                vertex->VAdjacent.erase(edge->V[0]);
                                vertex->EAdjacent.erase(edge);
                            }
                        }
                    }
                }

                vertex->TAdjacent.erase(tri);

                // If the vertex is no longer shared by any triangle,
                // remove it.
                if (vertex->TAdjacent.size() == 0)
                {
                    LogAssert(vertex->VAdjacent.size() != 0 || vertex->EAdjacent.size() != 0,
                        "Malformed mesh.");

                    mVMap.erase(vItem);
                }
            }

            return ETNonmanifoldMesh::Remove(v0, v1, v2);
        }

        // Destroy the vertices, edges, and triangles to obtain an empty mesh.
        virtual void Clear() override
        {
            mVMap.clear();
            ETNonmanifoldMesh::Clear();
        }

    protected:
        // The vertex data and default vertex creation.
        static std::shared_ptr<Vertex> CreateVertex(int vIndex)
        {
            return std::make_shared<Vertex>(vIndex);
        }

        VCreator mVCreator;
        VMap mVMap;
    };
}
