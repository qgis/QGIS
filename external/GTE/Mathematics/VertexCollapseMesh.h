// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.22

#pragma once

#include <Mathematics/MinHeap.h>
#include <Mathematics/Polygon2.h>
#include <Mathematics/TriangulateEC.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/VETManifoldMesh.h>
#include <set>

namespace gte
{
    template <typename Real>
    class VertexCollapseMesh
    {
    public:
        // Construction.
        VertexCollapseMesh(int numPositions, Vector3<Real> const* positions,
            int numIndices, int const* indices)
            :
            mNumPositions(numPositions),
            mPositions(positions),
            mMesh(VCVertex::Create)
        {
            if (numPositions <= 0 || !positions || numIndices < 3 || !indices)
            {
                mNumPositions = 0;
                mPositions = nullptr;
                return;
            }

            // Build the manifold mesh from the inputs.
            int numTriangles = numIndices / 3;
            int const* current = indices;
            for (int t = 0; t < numTriangles; ++t)
            {
                int v0 = *current++;
                int v1 = *current++;
                int v2 = *current++;
                mMesh.Insert(v0, v1, v2);
            }

            // Locate the vertices (if any) on the mesh boundary.
            auto const& vmap = mMesh.GetVertices();
            for (auto const& eelement : mMesh.GetEdges())
            {
                auto edge = eelement.second.get();
                if (!edge->T[1])
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        auto velement = vmap.find(edge->V[i]);
                        auto vertex = static_cast<VCVertex*>(velement->second.get());
                        vertex->isBoundary = true;
                    }
                }
            }

            // Build the priority queue of weights for the interior vertices.
            mMinHeap.Reset((int)vmap.size());
            for (auto const& velement : vmap)
            {
                auto vertex = static_cast<VCVertex*>(velement.second.get());

                Real weight;
                if (vertex->isBoundary)
                {
                    weight = std::numeric_limits<Real>::max();
                }
                else
                {
                    weight = vertex->ComputeWeight(mPositions);
                }

                auto record = mMinHeap.Insert(velement.first, weight);
                mHeapRecords.insert(std::make_pair(velement.first, record));
            }
        }

        // Decimate the mesh using vertex collapses
        struct Record
        {
            // The index of the interior vertex that is removed from the mesh.
            // The triangles adjacent to the vertex are 'removed' from the
            // mesh.  The polygon boundary of the adjacent triangles is
            // triangulated and the new triangles are 'inserted' into the
            // mesh.
            int vertex;
            std::vector<TriangleKey<true>> removed;
            std::vector<TriangleKey<true>> inserted;
        };

        // Return 'true' when a vertex collapse occurs.  Once the function
        // returns 'false', no more vertex collapses are allowed so you may
        // then stop calling the function.  The implementation has several
        // consistency tests that should not fail with a theoretically correct
        // implementation.  If a test fails, the function returns 'false' and
        // the record.vertex is set to the invalid integer 0x80000000.  When
        // the Logger system is enabled, the failed tests are reported to any
        // Logger listeners.
        bool DoCollapse(Record& record)
        {
            record.vertex = 0x80000000;
            record.removed.clear();
            record.inserted.clear();

            if (mNumPositions == 0)
            {
                // The constructor failed, so there is nothing to collapse.
                return false;
            }

            while (mMinHeap.GetNumElements() > 0)
            {
                int v = -1;
                Real weight = std::numeric_limits<Real>::max();
                mMinHeap.GetMinimum(v, weight);
                if (weight == std::numeric_limits<Real>::max())
                {
                    // There are no more interior vertices to collapse.
                    return false;
                }

                auto const& vmap = mMesh.GetVertices();
                auto velement = vmap.find(v);
                if (velement == vmap.end())
                {
                    // Unexpected condition.
                    return false;
                }

                auto vertex = static_cast<VCVertex*>(velement->second.get());
                std::vector<TriangleKey<true>> removed, inserted;
                std::vector<int> linkVertices;
                int result = TriangulateLink(vertex, removed, inserted, linkVertices);
                if (result == VCM_UNEXPECTED_ERROR)
                {
                    return false;
                }

                if (result == VCM_ALLOWED)
                {
                    result = Collapsed(removed, inserted, linkVertices);
                    if (result == VCM_UNEXPECTED_ERROR)
                    {
                        return false;
                    }

                    if (result == VCM_ALLOWED)
                    {
                        // Remove the vertex and associated weight.
                        mMinHeap.Remove(v, weight);
                        mHeapRecords.erase(v);

                        // Update the weights of the link vertices.
                        for (auto vlink : linkVertices)
                        {
                            velement = vmap.find(vlink);
                            if (velement == vmap.end())
                            {
                                // Unexpected condition.
                                return false;
                            }

                            vertex = static_cast<VCVertex*>(velement->second.get());
                            if (!vertex->isBoundary)
                            {
                                auto iter = mHeapRecords.find(vlink);
                                if (iter == mHeapRecords.end())
                                {
                                    // Unexpected condition.
                                    return false;
                                }

                                weight = vertex->ComputeWeight(mPositions);
                                mMinHeap.Update(iter->second, weight);
                            }
                        }

                        record.vertex = v;
                        record.removed = std::move(removed);
                        record.inserted = std::move(inserted);
                        return true;
                    }
                    // else:  result == VCM_DEFERRED
                }

                // To get here, result must be VCM_DEFERRED.  The vertex
                // collapse would cause mesh fold-over.  Temporarily set the
                // edge weight to infinity.  After removal of other triangles,
                // the vertex weight will be updated to a finite value and the
                // vertex possibly can be removed at that time.
                auto iter = mHeapRecords.find(v);
                if (iter == mHeapRecords.end())
                {
                    // Unexpected condition.
                    return false;
                }
                mMinHeap.Update(iter->second, std::numeric_limits<Real>::max());
            }

            // We do not expect to reach this line of code, even for a closed
            // mesh.  However, the compiler does not know this, yet requires
            // a return value.
            return false;
        }

        // Access the current state of the mesh, whether the original built
        // in the constructor or a decimated mesh during DoCollapse calls.
        inline ETManifoldMesh const& GetMesh() const
        {
            return mMesh;
        }

    private:
        struct VCVertex : public VETManifoldMesh::Vertex
        {
            VCVertex(int v)
                :
                VETManifoldMesh::Vertex(v),
                normal(Vector3<Real>::Zero()),
                isBoundary(false)
            {
            }

            static std::unique_ptr<Vertex> Create(int v)
            {
                return std::make_unique<VCVertex>(v);
            }

            // The weight depends on the area of the triangles sharing the
            // vertex and the lengths of the projections of the adjacent
            // vertices onto the vertex normal line.  A side effect of the
            // call is that the vertex normal is computed and stored.
            Real ComputeWeight(Vector3<Real> const* positions)
            {
                Real weight = (Real)0;

                normal = { (Real)0, (Real)0, (Real)0 };
                for (auto const& tri : TAdjacent)
                {
                    Vector3<Real> E0 = positions[tri->V[1]] - positions[tri->V[0]];
                    Vector3<Real> E1 = positions[tri->V[2]] - positions[tri->V[0]];
                    Vector3<Real> N = Cross(E0, E1);
                    normal += N;
                    weight += Length(N);
                }
                Normalize(normal);

                for (int index : VAdjacent)
                {
                    Vector3<Real> diff = positions[index] - positions[V];
                    weight += std::fabs(Dot(normal, diff));
                }

                return weight;
            }

            Vector3<Real> normal;
            bool isBoundary;
        };

        // The functions TriangulateLink and Collapsed return one of the
        // enumerates described next.
        //
        // VCM_NO_MORE_ALLOWED:
        //     Either the mesh has no more interior vertices or a collapse
        //     will lead to a mesh fold-over or to a nonmanifold mesh.  The
        //     returned value 'v' is invalid (0x80000000) and 'removed' and
        //     'inserted' are empty.
        //
        // VCM_ALLOWED:
        //     An interior vertex v has been removed.  This is allowed using
        //     the following algorithm.  The vertex normal is the weighted
        //     average of non-unit-length normals of triangles sharing v.  The
        //     weights are the triangle areas.  The adjacent vertices are
        //     projected onto a plane containing v and having normal equal to
        //     the vertex normal.  If the projection is a simple polygon in
        //     the plane, the collapse is allowed.  The triangles sharing v
        //     are 'removed', the polygon is triangulated, and the new
        //     triangles are 'inserted' into the mesh.
        //
        // VCM_DEFERRED:
        //     If the projection polygon described in the previous case is not
        //     simple (at least one pair of edges overlaps at some
        //     edge-interior point), the collapse would produce a fold-over in
        //     the mesh.  We do not collapse in this case.  It is possible
        //     that such a vertex occurs in a later collapse as its neighbors
        //     are adjusted by collapses.  When this case occurs, v is valid
        //     (even though the collapse was not allowed) but 'removed' and
        //     'inserted' are empty.
        //
        // VCM_UNEXPECTED_ERROR:
        //     The code has several tests for conditions that are not expected
        //     to occur for a theoretically correct implementation.  If you
        //     receive this error, file a bug report and provide a data set
        //     that caused the error.
        enum
        {
            VCM_NO_MORE_ALLOWED,
            VCM_ALLOWED,
            VCM_DEFERRED,
            VCM_UNEXPECTED_ERROR
        };

        int TriangulateLink(VCVertex* vertex, std::vector<TriangleKey<true>>& removed,
            std::vector<TriangleKey<true>>& inserted, std::vector<int>& linkVertices) const
        {
            // Create the (CCW) polygon boundary of the link of the vertex.
            // The incoming vertex is interior, so the number of triangles
            // sharing the vertex is equal to the number of vertices of the
            // polygon.  A precondition of the function call is that the
            // vertex normal has already been computed.

            // Get the edges of the link that are opposite the incoming
            // vertex.
            int const numVertices = static_cast<int>(vertex->TAdjacent.size());
            removed.resize(numVertices);
            int j = 0;
            std::map<int, int> edgeMap;
            for (auto tri : vertex->TAdjacent)
            {
                for (int i = 0; i < 3; ++i)
                {
                    if (tri->V[i] == vertex->V)
                    {
                        edgeMap.insert(std::make_pair(tri->V[(i + 1) % 3], tri->V[(i + 2) % 3]));
                        break;
                    }
                }
                removed[j++] = TriangleKey<true>(tri->V[0], tri->V[1], tri->V[2]);
            }
            if (edgeMap.size() != vertex->TAdjacent.size())
            {
                return VCM_UNEXPECTED_ERROR;
            }

            // Connect the edges into a polygon.
            linkVertices.resize(numVertices);
            auto iter = edgeMap.begin();
            for (int i = 0; i < numVertices; ++i)
            {
                linkVertices[i] = iter->first;
                iter = edgeMap.find(iter->second);
                if (iter == edgeMap.end())
                {
                    return VCM_UNEXPECTED_ERROR;
                }
            }
            if (iter->first != linkVertices[0])
            {
                return VCM_UNEXPECTED_ERROR;
            }

            // Project the polygon onto the plane containing the incoming
            // vertex and having the vertex normal.  The projected polygon
            // is computed so that the incoming vertex is projected to (0,0).
            Vector3<Real> center = mPositions[vertex->V];
            Vector3<Real> basis[3];
            basis[0] = vertex->normal;
            ComputeOrthogonalComplement(1, basis);
            std::vector<Vector2<Real>> projected(numVertices);
            std::vector<int> indices(numVertices);
            for (int i = 0; i < numVertices; ++i)
            {
                Vector3<Real> diff = mPositions[linkVertices[i]] - center;
                projected[i][0] = Dot(basis[1], diff);
                projected[i][1] = Dot(basis[2], diff);
                indices[i] = i;
            }

            // The polygon must be simple in order to triangulate it.
            Polygon2<Real> polygon(projected.data(), numVertices, indices.data(), true);
            if (polygon.IsSimple())
            {
                TriangulateEC<Real, Real> triangulator(numVertices, projected.data());
                triangulator();
                auto const& triangles = triangulator.GetTriangles();
                if (triangles.size() == 0)
                {
                    return VCM_UNEXPECTED_ERROR;
                }

                int const numTriangles = static_cast<int>(triangles.size());
                inserted.resize(numTriangles);
                for (int t = 0; t < numTriangles; ++t)
                {
                    inserted[t] = TriangleKey<true>(
                        linkVertices[triangles[t][0]],
                        linkVertices[triangles[t][1]],
                        linkVertices[triangles[t][2]]);
                }
                return VCM_ALLOWED;
            }
            else
            {
                return VCM_DEFERRED;
            }
        }

        int Collapsed(std::vector<TriangleKey<true>> const& removed,
            std::vector<TriangleKey<true>> const& inserted, std::vector<int> const& linkVertices)
        {
            // The triangles that were disconnected from the link edges are
            // guaranteed to allow manifold reconnection to 'inserted'
            // triangles.  On the insertion, each diagonal of the link becomes
            // a mesh edge and shares two (link) triangles.  It is possible
            // that the mesh already contains the (diagonal) edge, which will
            // lead to a nonmanifold connection, which we cannot allow.  The
            // following code traps this condition and restores the mesh to
            // its state before the 'Remove(...)' call.
            bool isCollapsible = true;
            auto const& emap = mMesh.GetEdges();
            std::set<EdgeKey<false>> edges;
            for (auto const& tri : inserted)
            {
                for (int k0 = 2, k1 = 0; k1 < 3; k0 = k1++)
                {
                    EdgeKey<false> edge(tri.V[k0], tri.V[k1]);
                    if (edges.find(edge) == edges.end())
                    {
                        edges.insert(edge);
                    }
                    else
                    {
                        // The edge has been visited twice, so it is a
                        // diagonal of the link.

                        auto eelement = emap.find(edge);
                        if (eelement != emap.end())
                        {
                            if (eelement->second->T[1])
                            {
                                // The edge will not allow a manifold
                                // connection.
                                isCollapsible = false;
                                break;
                            }
                        }

                        edges.erase(edge);
                    }
                };

                if (!isCollapsible)
                {
                    return VCM_DEFERRED;
                }
            }

            // Remove the old triangle neighborhood, which will lead to the
            // vertex itself being removed from the mesh.
            for (auto tri : removed)
            {
                mMesh.Remove(tri.V[0], tri.V[1], tri.V[2]);
            }

            // Insert the new triangulation.
            for (auto const& tri : inserted)
            {
                mMesh.Insert(tri.V[0], tri.V[1], tri.V[2]);
            }

            // If the Remove(...) calls remove a boundary vertex that is in
            // the link vertices, the Insert(...) calls will insert the
            // boundary vertex again.  We must re-tag those boundary
            // vertices.
            auto const& vmap = mMesh.GetVertices();
            size_t const numVertices = linkVertices.size();
            for (size_t i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
            {
                EdgeKey<false> ekey(linkVertices[i0], linkVertices[i1]);
                auto eelement = emap.find(ekey);
                if (eelement == emap.end())
                {
                    return VCM_UNEXPECTED_ERROR;
                }

                auto edge = eelement->second.get();
                if (!edge)
                {
                    return VCM_UNEXPECTED_ERROR;
                }

                if (edge->T[0] && !edge->T[1])
                {
                    for (int k = 0; k < 2; ++k)
                    {
                        auto velement = vmap.find(edge->V[k]);
                        if (velement == vmap.end())
                        {
                            return VCM_UNEXPECTED_ERROR;
                        }

                        auto vertex = static_cast<VCVertex*>(velement->second.get());
                        vertex->isBoundary = true;
                    }
                }
            }

            return VCM_ALLOWED;
        }

        int mNumPositions;
        Vector3<Real> const* mPositions;
        VETManifoldMesh mMesh;

        MinHeap<int, Real> mMinHeap;
        std::map<int, typename MinHeap<int, Real>::Record*> mHeapRecords;
    };
}
