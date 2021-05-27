// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/WeakPtrCompare.h>
#include <Mathematics/EdgeKey.h>
#include <Mathematics/TriangleKey.h>
#include <map>
#include <vector>

namespace gte
{
    class ETNonmanifoldMesh
    {
    public:
        // Edge data types.
        class Edge;
        typedef std::shared_ptr<Edge>(*ECreator)(int, int);
        typedef std::map<EdgeKey<false>, std::shared_ptr<Edge>> EMap;

        // Triangle data types.
        class Triangle;
        typedef std::shared_ptr<Triangle>(*TCreator)(int, int, int);
        typedef std::map<TriangleKey<true>, std::shared_ptr<Triangle>> TMap;

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

            bool operator<(Edge const& other) const
            {
                return EdgeKey<false>(V[0], V[1]) < EdgeKey<false>(other.V[0], other.V[1]);
            }

            // Vertices of the edge.
            std::array<int, 2> V;

            // Triangles sharing the edge.
            std::set<std::weak_ptr<Triangle>, WeakPtrLT<Triangle>> T;
        };

        // Triangle object.
        class Triangle
        {
        public:
            virtual ~Triangle() = default;

            Triangle(int v0, int v1, int v2)
                :
                V{ v0, v1, v2 }
            {
            }

            bool operator<(Triangle const& other) const
            {
                return TriangleKey<true>(V[0], V[1], V[2]) < TriangleKey<true>(other.V[0], other.V[1], other.V[2]);
            }

            // Vertices listed in counterclockwise order (V[0],V[1],V[2]).
            std::array<int, 3> V;

            // Adjacent edges.  E[i] points to edge (V[i],V[(i+1)%3]).
            std::array<std::weak_ptr<Edge>, 3> E;
        };


        // Construction and destruction.
        virtual ~ETNonmanifoldMesh() = default;

        ETNonmanifoldMesh(ECreator eCreator = nullptr, TCreator tCreator = nullptr)
            :
            mECreator(eCreator ? eCreator : CreateEdge),
            mTCreator(tCreator ? tCreator : CreateTriangle)
        {
        }

        // Support for a deep copy of the mesh.  The mEMap and mTMap objects
        // have dynamically allocated memory for edges and triangles.  A
        // shallow copy of the pointers to this memory is problematic.
        // Allowing sharing, say, via std::shared_ptr, is an option but not
        // really the intent of copying the mesh graph.
        ETNonmanifoldMesh(ETNonmanifoldMesh const& mesh)
        {
            *this = mesh;
        }

        ETNonmanifoldMesh& operator=(ETNonmanifoldMesh const& mesh)
        {
            Clear();

            mECreator = mesh.mECreator;
            mTCreator = mesh.mTCreator;
            for (auto const& element : mesh.mTMap)
            {
                Insert(element.first.V[0], element.first.V[1], element.first.V[2]);
            }

            return *this;
        }

        // Member access.
        inline EMap const& GetEdges() const
        {
            return mEMap;
        }

        inline TMap const& GetTriangles() const
        {
            return mTMap;
        }

        // If <v0,v1,v2> is not in the mesh, a Triangle object is created and
        // returned; otherwise, <v0,v1,v2> is in the mesh and nullptr is
        // returned.
        virtual std::shared_ptr<Triangle> Insert(int v0, int v1, int v2)
        {
            TriangleKey<true> tkey(v0, v1, v2);
            if (mTMap.find(tkey) != mTMap.end())
            {
                // The triangle already exists.  Return a null pointer as a
                // signal to the caller that the insertion failed.
                return nullptr;
            }

            // Create the new triangle.  It will be added to mTMap at the end
            // of the function so that if an assertion is triggered and the
            // function returns early, the (bad) triangle will not be part of
            // the mesh.
            std::shared_ptr<Triangle> tri = mTCreator(v0, v1, v2);

            // Add the edges to the mesh if they do not already exist.
            for (int i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                EdgeKey<false> ekey(tri->V[i0], tri->V[i1]);
                std::shared_ptr<Edge> edge;
                auto eiter = mEMap.find(ekey);
                if (eiter == mEMap.end())
                {
                    // This is the first time the edge is encountered.
                    edge = mECreator(tri->V[i0], tri->V[i1]);
                    mEMap[ekey] = edge;
                }
                else
                {
                    // The edge was previously encountered and created.
                    edge = eiter->second;
                    LogAssert(edge != nullptr, "Unexpected condition.");
                }

                // Associate the edge with the triangle.
                tri->E[i0] = edge;

                // Update the adjacent set of triangles for the edge.
                edge->T.insert(tri);
            }

            mTMap[tkey] = tri;
            return tri;
        }

        // If <v0,v1,v2> is in the mesh, it is removed and 'true' is returned;
        // otherwise, <v0,v1,v2> is not in the mesh and 'false' is returned.
        virtual bool Remove(int v0, int v1, int v2)
        {
            TriangleKey<true> tkey(v0, v1, v2);
            auto titer = mTMap.find(tkey);
            if (titer == mTMap.end())
            {
                // The triangle does not exist.
                return false;
            }

            // Get the triangle.
            std::shared_ptr<Triangle> tri = titer->second;

            // Remove the edges and update adjacent triangles if necessary.
            for (int i = 0; i < 3; ++i)
            {
                // Inform the edges the triangle is being deleted.
                auto edge = tri->E[i].lock();
                LogAssert(edge != nullptr, "Unexpected condition.");

                // Remove the triangle from the edge's set of adjacent
                // triangles.
                size_t numRemoved = edge->T.erase(tri);
                LogAssert(numRemoved > 0, "Unexpected condition.");

                // Remove the edge if you have the last reference to it.
                if (edge->T.size() == 0)
                {
                    EdgeKey<false> ekey(edge->V[0], edge->V[1]);
                    mEMap.erase(ekey);
                }
            }

            // Remove the triangle from the graph.
            mTMap.erase(tkey);
            return true;
        }

        // Destroy the edges and triangles to obtain an empty mesh.
        virtual void Clear()
        {
            mEMap.clear();
            mTMap.clear();
        }

        // A manifold mesh has the property that an edge is shared by at most
        // two triangles sharing.
        bool IsManifold() const
        {
            for (auto const& element : mEMap)
            {
                if (element.second->T.size() > 2)
                {
                    return false;
                }
            }
            return true;
        }

        // A manifold mesh is closed if each edge is shared twice.  A closed
        // mesh is not necessarily oriented.  For example, you could have a
        // mesh with spherical topology. The upper hemisphere has outer-facing
        // normals and the lower hemisphere has inner-facing normals.  The
        // discontinuity in orientation occurs on the circle shared by the
        // hemispheres.
        bool IsClosed() const
        {
            for (auto const& element : mEMap)
            {
                if (element.second->T.size() != 2)
                {
                    return false;
                }
            }
            return true;
        }

        // Compute the connected components of the edge-triangle graph that
        // the mesh represents.  The first function returns pointers into
        // 'this' object's containers, so you must consume the components
        // before clearing or destroying 'this'.  The second function returns
        // triangle keys, which requires three times as much storage as the
        // pointers but allows you to clear or destroy 'this' before consuming
        // the components.
        void GetComponents(std::vector<std::vector<std::shared_ptr<Triangle>>>& components) const
        {
            // visited: 0 (unvisited), 1 (discovered), 2 (finished)
            std::map<std::shared_ptr<Triangle>, int> visited;
            for (auto const& element : mTMap)
            {
                visited.insert(std::make_pair(element.second, 0));
            }

            for (auto& element : mTMap)
            {
                auto tri = element.second;
                if (visited[tri] == 0)
                {
                    std::vector<std::shared_ptr<Triangle>> component;
                    DepthFirstSearch(tri, visited, component);
                    components.push_back(component);
                }
            }
        }

        void GetComponents(std::vector<std::vector<TriangleKey<true>>>& components) const
        {
            // visited: 0 (unvisited), 1 (discovered), 2 (finished)
            std::map<std::shared_ptr<Triangle>, int> visited;
            for (auto const& element : mTMap)
            {
                visited.insert(std::make_pair(element.second, 0));
            }

            for (auto& element : mTMap)
            {
                std::shared_ptr<Triangle> tri = element.second;
                if (visited[tri] == 0)
                {
                    std::vector<std::shared_ptr<Triangle>> component;
                    DepthFirstSearch(tri, visited, component);

                    std::vector<TriangleKey<true>> keyComponent;
                    keyComponent.reserve(component.size());
                    for (auto const& t : component)
                    {
                        keyComponent.push_back(TriangleKey<true>(t->V[0], t->V[1], t->V[2]));
                    }
                    components.push_back(keyComponent);
                }
            }
        }

    protected:
        // The edge data and default edge creation.
        static std::shared_ptr<Edge> CreateEdge(int v0, int v1)
        {
            return std::make_shared<Edge>(v0, v1);
        }

        ECreator mECreator;
        EMap mEMap;

        // The triangle data and default triangle creation.
        static std::shared_ptr<Triangle> CreateTriangle(int v0, int v1, int v2)
        {
            return std::make_shared<Triangle>(v0, v1, v2);
        }

        TCreator mTCreator;
        TMap mTMap;

        // Support for computing connected components.  This is a
        // straightforward depth-first search of the graph but uses a
        // preallocated stack rather than a recursive function that could
        // possibly overflow the call stack.
        void DepthFirstSearch(std::shared_ptr<Triangle> const& tInitial,
            std::map<std::shared_ptr<Triangle>, int>& visited,
            std::vector<std::shared_ptr<Triangle>>& component) const
        {
            // Allocate the maximum-size stack that can occur in the
            // depth-first search.  The stack is empty when the index top
            // is -1.
            std::vector<std::shared_ptr<Triangle>> tStack(mTMap.size());
            int top = -1;
            tStack[++top] = tInitial;
            while (top >= 0)
            {
                std::shared_ptr<Triangle> tri = tStack[top];
                visited[tri] = 1;
                int i;
                for (i = 0; i < 3; ++i)
                {
                    auto edge = tri->E[i].lock();
                    LogAssert(edge != nullptr, "Unexpected condition.");

                    bool foundUnvisited = false;
                    for (auto const& adjw : edge->T)
                    {
                        auto adj = adjw.lock();
                        LogAssert(adj != nullptr, "Unexpected condition.");

                        if (visited[adj] == 0)
                        {
                            tStack[++top] = adj;
                            foundUnvisited = true;
                            break;
                        }
                    }

                    if (foundUnvisited)
                    {
                        break;
                    }
                }
                if (i == 3)
                {
                    visited[tri] = 2;
                    component.push_back(tri);
                    --top;
                }
            }
        }
    };
}
