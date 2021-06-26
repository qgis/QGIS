// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <array>
#include <map>
#include <memory>

namespace gte
{
    class VEManifoldMesh
    {
    public:
        // Vertex data types.
        class Vertex;
        typedef std::shared_ptr<Vertex>(*VCreator)(int);
        typedef std::map<int, std::shared_ptr<Vertex>> VMap;

        // Edge data types.
        class Edge;
        typedef std::shared_ptr<Edge>(*ECreator)(int, int);
        typedef std::map<std::pair<int, int>, std::shared_ptr<Edge>> EMap;

        // Vertex object.
        class Vertex
        {
        public:
            virtual ~Vertex() = default;

            Vertex(int v)
                :
                V(v)
            {
            }

            // The unique vertex index.
            int V;

            // The edges (if any) sharing the vertex.
            std::array<std::weak_ptr<Edge>, 2> E;
        };

        // Edge object.
        class Edge
        {
        public:
            virtual ~Edge() = default;

            Edge(int v0, int v1)
                :
                V{ v0, v1 }
            {
            }

            // Vertices, listed as a directed edge <V[0],V[1]>.
            std::array<int, 2> V;

            // Adjacent edges.  E[i] points to edge sharing V[i].
            std::array<std::weak_ptr<Edge>, 2> E;
        };


        // Construction and destruction.
        virtual ~VEManifoldMesh() = default;

        VEManifoldMesh(VCreator vCreator = nullptr, ECreator eCreator = nullptr)
            :
            mVCreator(vCreator ? vCreator : CreateVertex),
            mECreator(eCreator ? eCreator : CreateEdge),
            mThrowOnNonmanifoldInsertion(true)
        {
        }

        // Member access.
        inline VMap const& GetVertices() const
        {
            return mVMap;
        }

        inline EMap const& GetEdges() const
        {
            return mEMap;
        }

        // If the insertion of an edge fails because the mesh would become
        // nonmanifold, the default behavior is to throw an exception.  You
        // can disable this behavior and continue gracefully without an
        // exception.
        void ThrowOnNonmanifoldInsertion(bool doException)
        {
            mThrowOnNonmanifoldInsertion = doException;
        }

        // If <v0,v1> is not in the mesh, an Edge object is created and
        // returned; otherwise, <v0,v1> is in the mesh and nullptr is
        // returned.  If the insertion leads to a nonmanifold mesh, the
        // call fails with a nullptr returned.
        std::shared_ptr<Edge> Insert(int v0, int v1)
        {
            std::pair<int, int> ekey(v0, v1);
            if (mEMap.find(ekey) != mEMap.end())
            {
                // The edge already exists.  Return a null pointer as a
                // signal to the caller that the insertion failed.
                return nullptr;
            }

            // Add the new edge.
            std::shared_ptr<Edge> edge = mECreator(v0, v1);
            mEMap[ekey] = edge;

            // Add the vertices if they do not already exist.
            for (int i = 0; i < 2; ++i)
            {
                int v = edge->V[i];
                std::shared_ptr<Vertex> vertex;
                auto viter = mVMap.find(v);
                if (viter == mVMap.end())
                {
                    // This is the first time the vertex is encountered.
                    vertex = mVCreator(v);
                    mVMap[v] = vertex;

                    // Update the vertex.
                    vertex->E[0] = edge;
                }
                else
                {
                    // This is the second time the vertex is encountered.
                    vertex = viter->second;
                    LogAssert(vertex != nullptr, "Unexpected condition.");

                    // Update the vertex.
                    if (vertex->E[1].lock())
                    {
                        if (mThrowOnNonmanifoldInsertion)
                        {
                            LogError("The mesh must be manifold.");
                        }
                        else
                        {
                            return nullptr;
                        }
                    }
                    vertex->E[1] = edge;

                    // Update the adjacent edge.
                    auto adjacent = vertex->E[0].lock();
                    LogAssert(adjacent != nullptr, "Unexpected condition.");
                    for (int j = 0; j < 2; ++j)
                    {
                        if (adjacent->V[j] == v)
                        {
                            adjacent->E[j] = edge;
                            break;
                        }
                    }

                    // Update the edge.
                    edge->E[i] = adjacent;
                }
            }

            return edge;
        }

        // If <v0,v1> is in the mesh, it is removed and 'true' is returned;
        // otherwise, <v0,v1> is not in the mesh and 'false' is returned.
        bool Remove(int v0, int v1)
        {
            std::pair<int, int> ekey(v0, v1);
            auto eiter = mEMap.find(ekey);
            if (eiter == mEMap.end())
            {
                // The edge does not exist.
                return false;
            }

            // Get the edge.
            std::shared_ptr<Edge> edge = eiter->second;

            // Remove the vertices if necessary (when they are not shared).
            for (int i = 0; i < 2; ++i)
            {
                // Inform the vertices the edge is being deleted.
                auto viter = mVMap.find(edge->V[i]);
                LogAssert(viter != mVMap.end(), "Unexpected condition.");

                std::shared_ptr<Vertex> vertex = viter->second;
                LogAssert(vertex != nullptr, "Unexpected condition.");
                if (vertex->E[0].lock() == edge)
                {
                    // One-edge vertices always have pointer at index zero.
                    vertex->E[0] = vertex->E[1];
                    vertex->E[1].reset();
                }
                else if (vertex->E[1].lock() == edge)
                {
                    vertex->E[1].reset();
                }
                else
                {
                    LogError("Unexpected condition.");
                }

                // Remove the vertex if you have the last reference to it.
                if (!vertex->E[0].lock() && !vertex->E[1].lock())
                {
                    mVMap.erase(vertex->V);
                }

                // Inform adjacent edges the edge is being deleted.
                auto adjacent = edge->E[i].lock();
                if (adjacent)
                {
                    for (int j = 0; j < 2; ++j)
                    {
                        if (adjacent->E[j].lock() == edge)
                        {
                            adjacent->E[j].reset();
                            break;
                        }
                    }
                }
            }

            mEMap.erase(ekey);
            return true;
        }

        // A manifold mesh is closed if each vertex is shared twice.
        bool IsClosed() const
        {
            for (auto const& element : mVMap)
            {
                auto vertex = element.second;
                if (!vertex->E[0].lock() || !vertex->E[1].lock())
                {
                    return false;
                }
            }
            return true;
        }

    protected:
        // The vertex data and default vertex creation.
        static std::shared_ptr<Vertex> CreateVertex(int v0)
        {
            return std::make_shared<Vertex>(v0);
        }

        VCreator mVCreator;
        VMap mVMap;

        // The edge data and default edge creation.
        static std::shared_ptr<Edge> CreateEdge(int v0, int v1)
        {
            return std::make_shared<Edge>(v0, v1);
        }

        ECreator mECreator;
        EMap mEMap;
        bool mThrowOnNonmanifoldInsertion;  // default: true
    };
}
