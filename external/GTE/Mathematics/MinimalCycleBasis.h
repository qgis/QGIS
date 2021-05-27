// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/MinHeap.h>
#include <array>
#include <map>
#include <memory>
#include <set>
#include <stack>

// Extract the minimal cycle basis for a planar graph.  The input vertices and
// edges must form a graph for which edges intersect only at vertices; that is,
// no two edges must intersect at an interior point of one of the edges.  The
// algorithm is described in 
//   https://www.geometrictools.com/Documentation/MinimalCycleBasis.pdf
// The graph might have filaments, which are polylines in the graph that are
// not shared by a cycle.  These are also extracted by the implementation.
// Because the inputs to the constructor are vertices and edges of the graph,
// isolated vertices are ignored.
//
// The computations that determine which adjacent vertex to visit next during
// a filament or cycle traversal do not require division, so the exact
// arithmetic type BSNumber<UIntegerAP32> suffices for ComputeType when you
// want to ensure a correct output.  (Floating-point rounding errors
// potentially can lead to an incorrect output.)

namespace gte
{
    template <typename Real>
    class MinimalCycleBasis
    {
    public:
        struct Tree
        {
            std::vector<int> cycle;
            std::vector<std::shared_ptr<Tree>> children;
        };

        // The input positions and edges must form a planar graph for which
        // edges intersect only at vertices; that is, no two edges must
        // intersect at an interior point of one of the edges.
        MinimalCycleBasis(
            std::vector<std::array<Real, 2>> const& positions,
            std::vector<std::array<int, 2>> const& edges,
            std::vector<std::shared_ptr<Tree>>& forest)
        {
            forest.clear();
            if (positions.size() == 0 || edges.size() == 0)
            {
                // The graph is empty, so there are no filaments or cycles.
                return;
            }

            // Determine the unique positions referenced by the edges.
            std::map<int, std::shared_ptr<Vertex>> unique;
            for (auto const& edge : edges)
            {
                for (int i = 0; i < 2; ++i)
                {
                    int name = edge[i];
                    if (unique.find(name) == unique.end())
                    {
                        auto vertex = std::make_shared<Vertex>(name, &positions[name]);
                        unique.insert(std::make_pair(name, vertex));

                    }
                }
            }

            // Assign responsibility for ownership of the Vertex objects.
            std::vector<Vertex*> vertices;
            mVertexStorage.reserve(unique.size());
            vertices.reserve(unique.size());
            for (auto const& element : unique)
            {
                mVertexStorage.push_back(element.second);
                vertices.push_back(element.second.get());
            }

            // Determine the adjacencies from the edge information.
            for (auto const& edge : edges)
            {
                auto iter0 = unique.find(edge[0]);
                auto iter1 = unique.find(edge[1]);
                iter0->second->adjacent.insert(iter1->second.get());
                iter1->second->adjacent.insert(iter0->second.get());
            }

            // Get the connected components of the graph.  The 'visited' flags
            // are 0 (unvisited), 1 (discovered), 2 (finished).  The Vertex
            // constructor sets all 'visited' flags to 0.
            std::vector<std::vector<Vertex*>> components;
            for (auto vInitial : mVertexStorage)
            {
                if (vInitial->visited == 0)
                {
                    components.push_back(std::vector<Vertex*>());
                    DepthFirstSearch(vInitial.get(), components.back());
                }
            }

            // The depth-first search is used later for collecting vertices
            // for subgraphs that are detached from the main graph, so the
            // 'visited' flags must be reset to zero after component finding.
            for (auto vertex : mVertexStorage)
            {
                vertex->visited = 0;
            }

            // Get the primitives for the components.
            for (auto& component : components)
            {
                forest.push_back(ExtractBasis(component));
            }
        }

        // No copy or assignment allowed.
        MinimalCycleBasis(MinimalCycleBasis const&) = delete;
        MinimalCycleBasis& operator=(MinimalCycleBasis const&) = delete;

    private:
        struct Vertex
        {
            Vertex(int inName, std::array<Real, 2> const* inPosition)
                :
                name(inName),
                position(inPosition),
                visited(0)
            {
            }

            bool operator< (Vertex const& vertex) const
            {
                return name < vertex.name;
            }

            // The index into the 'positions' input provided to the call to
            // operator().  The index is used when reporting cycles to the
            // caller of the constructor for MinimalCycleBasis.
            int name;

            // Multiple vertices can share a position during processing of
            // graph components.
            std::array<Real, 2> const* position;

            // The mVertexStorage member owns the Vertex objects and maintains
            // the reference counts on those objects.  The adjacent pointers
            // are considered to be weak pointers; neither object ownership
            // nor reference counting is required by 'adjacent'.
            std::set<Vertex*> adjacent;

            // Support for depth-first traversal of a graph.
            int visited;
        };

        // The constructor uses GetComponents(...) and DepthFirstSearch(...)
        // to get the connected components of the graph implied by the input
        // 'edges'.  Recursive processing uses only DepthFirstSearch(...) to
        // collect vertices of the subgraphs of the original graph.
        static void DepthFirstSearch(Vertex* vInitial, std::vector<Vertex*>& component)
        {
            std::stack<Vertex*> vStack;
            vStack.push(vInitial);
            while (vStack.size() > 0)
            {
                Vertex* vertex = vStack.top();
                vertex->visited = 1;
                size_t i = 0;
                for (auto adjacent : vertex->adjacent)
                {
                    if (adjacent && adjacent->visited == 0)
                    {
                        vStack.push(adjacent);
                        break;
                    }
                    ++i;
                }

                if (i == vertex->adjacent.size())
                {
                    vertex->visited = 2;
                    component.push_back(vertex);
                    vStack.pop();
                }
            }
        }

        // Support for traversing a simply connected component of the graph.
        std::shared_ptr<Tree> ExtractBasis(std::vector<Vertex*>& component)
        {
            // The root will not have its 'cycle' member set.  The children
            // are the cycle trees extracted from the component.
            auto tree = std::make_shared<Tree>();
            while (component.size() > 0)
            {
                RemoveFilaments(component);
                if (component.size() > 0)
                {
                    tree->children.push_back(ExtractCycleFromComponent(component));
                }
            }

            if (tree->cycle.size() == 0 && tree->children.size() == 1)
            {
                // Replace the parent by the child to avoid having two empty
                // cycles in parent/child.
                auto child = tree->children.back();
                tree->cycle = std::move(child->cycle);
                tree->children = std::move(child->children);
            }
            return tree;
        }

        void RemoveFilaments(std::vector<Vertex*>& component)
        {
            // Locate all filament endpoints, which are vertices, each having
            // exactly one adjacent vertex.
            std::vector<Vertex*> endpoints;
            for (auto vertex : component)
            {
                if (vertex->adjacent.size() == 1)
                {
                    endpoints.push_back(vertex);
                }
            }

            if (endpoints.size() > 0)
            {
                // Remove the filaments from the component.  If a filament has
                // two endpoints, each having one adjacent vertex, the
                // adjacency set of the final visited vertex become empty.
                // We must test for that condition before starting a new
                // filament removal.
                for (auto vertex : endpoints)
                {
                    if (vertex->adjacent.size() == 1)
                    {
                        // Traverse the filament and remove the vertices.
                        while (vertex->adjacent.size() == 1)
                        {
                            // Break the connection between the two vertices.
                            Vertex* adjacent = *vertex->adjacent.begin();
                            adjacent->adjacent.erase(vertex);
                            vertex->adjacent.erase(adjacent);

                            // Traverse to the adjacent vertex.
                            vertex = adjacent;
                        }
                    }
                }

                // At this time the component is either empty (it was a union
                // of polylines) or it has no filaments and at least one
                // cycle.  Remove the isolated vertices generated by filament
                // extraction.
                std::vector<Vertex*> remaining;
                remaining.reserve(component.size());
                for (auto vertex : component)
                {
                    if (vertex->adjacent.size() > 0)
                    {
                        remaining.push_back(vertex);
                    }
                }
                component = std::move(remaining);
            }
        }

        std::shared_ptr<Tree> ExtractCycleFromComponent(std::vector<Vertex*>& component)
        {
            // Search for the left-most vertex of the component.  If two or
            // more vertices attain minimum x-value, select the one that has
            // minimum y-value.
            Vertex* minVertex = component[0];
            for (auto vertex : component)
            {
                if (*vertex->position < *minVertex->position)
                {
                    minVertex = vertex;
                }
            }

            // Traverse the closed walk, duplicating the starting vertex as
            // the last vertex.
            std::vector<Vertex*> closedWalk;
            Vertex* vCurr = minVertex;
            Vertex* vStart = vCurr;
            closedWalk.push_back(vStart);
            Vertex* vAdj = GetClockwiseMost(nullptr, vStart);
            while (vAdj != vStart)
            {
                closedWalk.push_back(vAdj);
                Vertex* vNext = GetCounterclockwiseMost(vCurr, vAdj);
                vCurr = vAdj;
                vAdj = vNext;
            }
            closedWalk.push_back(vStart);

            // Recursively process the closed walk to extract cycles.
            auto tree = ExtractCycleFromClosedWalk(closedWalk);

            // The isolated vertices generated by cycle removal are also
            // removed from the component.
            std::vector<Vertex*> remaining;
            remaining.reserve(component.size());
            for (auto vertex : component)
            {
                if (vertex->adjacent.size() > 0)
                {
                    remaining.push_back(vertex);
                }
            }
            component = std::move(remaining);

            return tree;
        }

        std::shared_ptr<Tree> ExtractCycleFromClosedWalk(std::vector<Vertex*>& closedWalk)
        {
            auto tree = std::make_shared<Tree>();

            std::map<Vertex*, int> duplicates;
            std::set<int> detachments;
            int numClosedWalk = static_cast<int>(closedWalk.size());
            for (int i = 1; i < numClosedWalk - 1; ++i)
            {
                auto diter = duplicates.find(closedWalk[i]);
                if (diter == duplicates.end())
                {
                    // We have not yet visited this vertex.
                    duplicates.insert(std::make_pair(closedWalk[i], i));
                    continue;
                }

                // The vertex has been visited previously.  Collapse the
                // closed walk by removing the subwalk sharing this vertex.
                // Note that the vertex is pointed to by
                // closedWalk[diter->second] and closedWalk[i].
                int iMin = diter->second, iMax = i;
                detachments.insert(iMin);
                for (int j = iMin + 1; j < iMax; ++j)
                {
                    Vertex* vertex = closedWalk[j];
                    duplicates.erase(vertex);
                    detachments.erase(j);
                }
                closedWalk.erase(closedWalk.begin() + iMin + 1, closedWalk.begin() + iMax + 1);
                numClosedWalk = static_cast<int>(closedWalk.size());
                i = iMin;
            }

            if (numClosedWalk > 3)
            {
                // We do not know whether closedWalk[0] is a detachment point.
                // To determine this, we must test for any edges strictly
                // contained by the wedge formed by the edges
                // <closedWalk[0],closedWalk[N-1]> and
                // <closedWalk[0],closedWalk[1]>.  However, we must execute
                // this test even for the known detachment points.  The
                // ensuing logic is designed to handle this and reduce the
                // amount of code, so we insert closedWalk[0] into the
                // detachment set and will ignore it later if it actually
                // is not.
                detachments.insert(0);

                // Detach subgraphs from the vertices of the cycle.
                for (auto i : detachments)
                {
                    Vertex* original = closedWalk[i];
                    Vertex* maxVertex = closedWalk[i + 1];
                    Vertex* minVertex = (i > 0 ? closedWalk[i - 1] : closedWalk[numClosedWalk - 2]);

                    std::array<Real, 2> dMin, dMax;
                    for (int j = 0; j < 2; ++j)
                    {
                        dMin[j] = (*minVertex->position)[j] - (*original->position)[j];
                        dMax[j] = (*maxVertex->position)[j] - (*original->position)[j];
                    }

                    // For debugging.
                    bool isConvex = (dMax[0] * dMin[1] >= dMax[1] * dMin[0]);
                    (void)isConvex;

                    std::set<Vertex*> inWedge;
                    std::set<Vertex*> adjacent = original->adjacent;
                    for (auto vertex : adjacent)
                    {
                        if (vertex->name == minVertex->name || vertex->name == maxVertex->name)
                        {
                            continue;
                        }

                        std::array<Real, 2> dVer;
                        for (int j = 0; j < 2; ++j)
                        {
                            dVer[j] = (*vertex->position)[j] - (*original->position)[j];
                        }

                        bool containsVertex;
                        if (isConvex)
                        {
                            containsVertex =
                                dVer[0] * dMin[1] > dVer[1] * dMin[0] &&
                                dVer[0] * dMax[1] < dVer[1] * dMax[0];
                        }
                        else
                        {
                            containsVertex =
                                (dVer[0] * dMin[1] > dVer[1] * dMin[0]) ||
                                (dVer[0] * dMax[1] < dVer[1] * dMax[0]);
                        }
                        if (containsVertex)
                        {
                            inWedge.insert(vertex);
                        }
                    }

                    if (inWedge.size() > 0)
                    {
                        // The clone will manage the adjacents for 'original'
                        // that lie inside the wedge defined by the first and
                        // last edges of the subgraph rooted at 'original'.
                        // The sorting is in the clockwise direction.
                        auto clone = std::make_shared<Vertex>(original->name, original->position);
                        mVertexStorage.push_back(clone);

                        // Detach the edges inside the wedge.
                        for (auto vertex : inWedge)
                        {
                            original->adjacent.erase(vertex);
                            vertex->adjacent.erase(original);
                            clone->adjacent.insert(vertex);
                            vertex->adjacent.insert(clone.get());
                        }

                        // Get the subgraph (it is a single connected
                        // component).
                        std::vector<Vertex*> component;
                        DepthFirstSearch(clone.get(), component);

                        // Extract the cycles of the subgraph.
                        tree->children.push_back(ExtractBasis(component));
                    }
                    // else the candidate was closedWalk[0] and it has no
                    // subgraph to detach.
                }

                tree->cycle = std::move(ExtractCycle(closedWalk));
            }
            else
            {
                // Detach the subgraph from vertex closedWalk[0]; the subgraph
                // is attached via a filament.
                Vertex* original = closedWalk[0];
                Vertex* adjacent = closedWalk[1];

                auto clone = std::make_shared<Vertex>(original->name, original->position);
                mVertexStorage.push_back(clone);

                original->adjacent.erase(adjacent);
                adjacent->adjacent.erase(original);
                clone->adjacent.insert(adjacent);
                adjacent->adjacent.insert(clone.get());

                // Get the subgraph (it is a single connected component).
                std::vector<Vertex*> component;
                DepthFirstSearch(clone.get(), component);

                // Extract the cycles of the subgraph.
                tree->children.push_back(ExtractBasis(component));
                if (tree->cycle.size() == 0 && tree->children.size() == 1)
                {
                    // Replace the parent by the child to avoid having two
                    // empty cycles in parent/child.
                    auto child = tree->children.back();
                    tree->cycle = std::move(child->cycle);
                    tree->children = std::move(child->children);
                }
            }

            return tree;
        }

        std::vector<int> ExtractCycle(std::vector<Vertex*>& closedWalk)
        {
            // TODO:  This logic was designed not to remove filaments after
            // the cycle deletion is complete.  Modify this to allow filament
            // removal.

            // The closed walk is a cycle.
            int const numVertices = static_cast<int>(closedWalk.size());
            std::vector<int> cycle(numVertices);
            for (int i = 0; i < numVertices; ++i)
            {
                cycle[i] = closedWalk[i]->name;
            }

            // The clockwise-most edge is always removable.
            Vertex* v0 = closedWalk[0];
            Vertex* v1 = closedWalk[1];
            Vertex* vBranch = (v0->adjacent.size() > 2 ? v0 : nullptr);
            v0->adjacent.erase(v1);
            v1->adjacent.erase(v0);

            // Remove edges while traversing counterclockwise.
            while (v1 != vBranch && v1->adjacent.size() == 1)
            {
                Vertex* adj = *v1->adjacent.begin();
                v1->adjacent.erase(adj);
                adj->adjacent.erase(v1);
                v1 = adj;
            }

            if (v1 != v0)
            {
                // If v1 had exactly 3 adjacent vertices, removal of the CCW
                // edge that shared v1 leads to v1 having 2 adjacent vertices.
                // When the CW removal occurs and we reach v1, the edge
                // deletion will lead to v1 having 1 adjacent vertex, making
                // it a filament endpoint.  We must ensure we do not delete v1
                // in this case, allowing the recursive algorithm to handle
                // the filament later.
                vBranch = v1;

                // Remove edges while traversing clockwise.
                while (v0 != vBranch && v0->adjacent.size() == 1)
                {
                    v1 = *v0->adjacent.begin();
                    v0->adjacent.erase(v1);
                    v1->adjacent.erase(v0);
                    v0 = v1;
                }
            }
            // else the cycle is its own connected component.

            return cycle;
        }

        Vertex* GetClockwiseMost(Vertex* vPrev, Vertex* vCurr) const
        {
            Vertex* vNext = nullptr;
            bool vCurrConvex = false;
            std::array<Real, 2> dCurr, dNext;
            if (vPrev)
            {
                dCurr[0] = (*vCurr->position)[0] - (*vPrev->position)[0];
                dCurr[1] = (*vCurr->position)[1] - (*vPrev->position)[1];
            }
            else
            {
                dCurr[0] = static_cast<Real>(0);
                dCurr[1] = static_cast<Real>(-1);
            }

            for (auto vAdj : vCurr->adjacent)
            {
                // vAdj is a vertex adjacent to vCurr.  No backtracking is
                // allowed.
                if (vAdj == vPrev)
                {
                    continue;
                }

                // Compute the potential direction to move in.
                std::array<Real, 2> dAdj;
                dAdj[0] = (*vAdj->position)[0] - (*vCurr->position)[0];
                dAdj[1] = (*vAdj->position)[1] - (*vCurr->position)[1];

                // Select the first candidate.
                if (!vNext)
                {
                    vNext = vAdj;
                    dNext = dAdj;
                    vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    continue;
                }

                // Update if the next candidate is clockwise of the current
                // clockwise-most vertex.
                if (vCurrConvex)
                {
                    if (dCurr[0] * dAdj[1] < dCurr[1] * dAdj[0]
                        || dNext[0] * dAdj[1] < dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    }
                }
                else
                {
                    if (dCurr[0] * dAdj[1] < dCurr[1] * dAdj[0]
                        && dNext[0] * dAdj[1] < dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] < dNext[1] * dCurr[0]);
                    }
                }
            }

            return vNext;
        }

        Vertex* GetCounterclockwiseMost(Vertex* vPrev, Vertex* vCurr) const
        {
            Vertex* vNext = nullptr;
            bool vCurrConvex = false;
            std::array<Real, 2> dCurr, dNext;
            if (vPrev)
            {
                dCurr[0] = (*vCurr->position)[0] - (*vPrev->position)[0];
                dCurr[1] = (*vCurr->position)[1] - (*vPrev->position)[1];
            }
            else
            {
                dCurr[0] = static_cast<Real>(0);
                dCurr[1] = static_cast<Real>(-1);
            }

            for (auto vAdj : vCurr->adjacent)
            {
                // vAdj is a vertex adjacent to vCurr.  No backtracking is
                // allowed.
                if (vAdj == vPrev)
                {
                    continue;
                }

                // Compute the potential direction to move in.
                std::array<Real, 2> dAdj;
                dAdj[0] = (*vAdj->position)[0] - (*vCurr->position)[0];
                dAdj[1] = (*vAdj->position)[1] - (*vCurr->position)[1];

                // Select the first candidate.
                if (!vNext)
                {
                    vNext = vAdj;
                    dNext = dAdj;
                    vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    continue;
                }

                // Select the next candidate if it is counterclockwise of the
                // current counterclockwise-most vertex.
                if (vCurrConvex)
                {
                    if (dCurr[0] * dAdj[1] > dCurr[1] * dAdj[0]
                        && dNext[0] * dAdj[1] > dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    }
                }
                else
                {
                    if (dCurr[0] * dAdj[1] > dCurr[1] * dAdj[0]
                        || dNext[0] * dAdj[1] > dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    }
                }
            }

            return vNext;
        }

        // Storage for referenced vertices of the original graph and for new
        // vertices added during graph traversal.
        std::vector<std::shared_ptr<Vertex>> mVertexStorage;
    };
}
