// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.6.2020.09.18

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/ArbitraryPrecision.h>
#include "ConvexMesh3.h"
#include <Mathematics/EdgeKey.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/UniqueVerticesSimplices.h>

namespace gte
{
    template <typename Real>
    class FIQuery<Real, ConvexMesh3<Real>, Plane3<Real>>
    {
    public:
        // Convenient type definitions.
        using CM = ConvexMesh3<Real>;
        using Vertex = typename CM::Vertex;
        using Triangle = typename CM::Triangle;

        // The configuration describes geometrically how the input convex
        // polyhedron and the plane intersect.
        static int constexpr CFG_EMPTY = 0x00000000;
        static int constexpr CFG_POS_SIDE = 0x00000010;
        static int constexpr CFG_NEG_SIDE = 0x00000020;

        // The plane intersects the convex polyhedron transversely. The set of
        // intersection is a convex polygon. The convex polyhedron is split
        // into two convex polyhedra, one on the positive side of the plane
        // and one on the negative side of the plane, both polyhedra sharing
        // the convex polygon of intersection.
        static int constexpr CFG_SPLIT = CFG_POS_SIDE | CFG_NEG_SIDE;  // 48

        // The convex polyhedron is strictly on the positive side of the
        // plane.
        static int constexpr CFG_POS_SIDE_STRICT = CFG_POS_SIDE;  // 16

        // The convex polyhedron is on the positive side of the plane with one
        // vertex in the plane.
        static int constexpr CFG_POS_SIDE_VERTEX = CFG_POS_SIDE | 1;  // 17

        // The convex polyhedron is on the positive side of the plane with one
        // edge in the plane.
        static int constexpr CFG_POS_SIDE_EDGE = CFG_POS_SIDE | 2;  // 18

        // The convex polyhedron is on the positive side of the plane with a
        // polygonal face in the plane. The face can consist of multiple
        // triangles.
        static int constexpr CFG_POS_SIDE_POLYGON = CFG_POS_SIDE | 4;  // 20

        // Flags for any of the tangential cases (vertex touching, edge
        // touching, face touching).
        static int constexpr CFG_POS_SIDE_TANGENT = CFG_POS_SIDE | 7;  // 23

        // The convex polyhedron is strictly on the negative side of the
        // plane.
        static int constexpr CFG_NEG_SIDE_STRICT = CFG_NEG_SIDE;  // 32

        // The convex polyhedron is on the negative side of the plane with one
        // vertex in the plane.
        static int constexpr CFG_NEG_SIDE_VERTEX = CFG_NEG_SIDE | 1;  // 33

        // The convex polyhedron is on the negative side of the plane with one
        // edge in the plane.
        static int constexpr CFG_NEG_SIDE_EDGE = CFG_NEG_SIDE | 2;  // 34

        // The convex polyhedron is on the negative side of the plane with a
        // polygonal face in the plane. The face can consist of multiple
        // triangles.
        static int constexpr CFG_NEG_SIDE_POLYGON = CFG_NEG_SIDE | 4;  // 36

        // Flags for any of the tangential cases (vertex touching, edge
        // touching, face touching).
        static int constexpr CFG_NEG_SIDE_TANGENT = CFG_NEG_SIDE | 7;  // 39

        // Requested information for the query to compute.
        static int constexpr REQ_CONFIGURATION_ONLY = 0x00000000;
        static int constexpr REQ_INTR_MESH = 0x00000001;
        static int constexpr REQ_INTR_POLYGON = 0x00000002;
        static int constexpr REQ_INTR_BOTH = REQ_INTR_MESH | REQ_INTR_POLYGON;
        static int constexpr REQ_POLYHEDRON_POS = 0x00000004;
        static int constexpr REQ_POLYHEDRON_NEG = 0x00000008;
        static int constexpr REQ_POLYHEDRON_BOTH = REQ_POLYHEDRON_POS | REQ_POLYHEDRON_NEG;
        static int constexpr REQ_ALL = 0x0000000F;

        struct Result
        {
            // The configuration describes geometrically how the input convex
            // polyhedron and the plane intersect.
            int configuration = CFG_EMPTY;

            // You can specify the information you want from the query.
            int requested = REQ_CONFIGURATION_ONLY;

            // The intersection of the convex polyhedron and the plane is
            // either empty, a single vertex, a single edge or a convex
            // polygon. The intersection members have the properties:
            //   empty:    mVertices has 0 elements, mMesh is empty
            //   vertex:   mVertices has 1 element, mMesh is empty
            //   edge:     mVertices has 2 elements, mMesh is empty
            //   polygon:  mVertices has 3 or more elements, mMesh is a
            //             triangulation of the convex polygon
            // The convex polygon vertices are indexed by 'polygon' in the
            // order consistent with that of the positive polyhedron
            // triangles. The indices are into intersection.vertices.
            ConvexMesh3<Real> intersectionMesh;
            std::vector<Vertex> intersectionPolygon;

            // If the configuration is POSITIVE_* or SPLIT, this convex
            // polyhedron is the portion of the input convex polyhedron on the
            // positive side of the plane with possibly a vertex or edge on
            // the plane.
            ConvexMesh3<Real> positivePolyhedron;

            // If the configuration is NEGATIVE_* or SPLIT, this convex
            // polyhedron is the portion of the input convex polyhedron on the
            // negative side of the plane with possibly a vertex or edge on
            // the plane.
            ConvexMesh3<Real> negativePolyhedron;
        };

        Result operator() (ConvexMesh3<Real> const& polyhedron,
            Plane3<Real> const& plane, int requested)
        {
            static_assert(is_arbitrary_precision<Real>::value, "Real must be arbitrary precision.");
            static_assert(has_division_operator<Real>::value, "Real must support division.");

            Result result;
            result.requested = requested;

            // Storage for (Dot(N,X) - c) for each vertex X, where N is a
            // plane normal (not necessarily unit length) and c is the
            // corresponding plane constant.
            int numPositive = 0, numNegative = 0, numZero = 0;
            size_t const numVertices = polyhedron.vertices.size();
            std::vector<Real> dot(numVertices);
            std::vector<int> sign(numVertices);
            for (size_t i = 0; i < numVertices; ++i)
            {
                dot[i] = Dot(plane.normal, polyhedron.vertices[i]) - plane.constant;
                if (dot[i] > (Real)0)
                {
                    sign[i] = +1;
                    ++numPositive;
                }
                else if (dot[i] < (Real)0)
                {
                    sign[i] = -1;
                    ++numNegative;
                }
                else
                {
                    sign[i] = 0;
                    ++numZero;
                }
            }

            if (numPositive == 0)
            {
                result.configuration = CFG_NEG_SIDE | (numZero < 3 ? numZero : 4);

                if ((requested & REQ_POLYHEDRON_NEG) != 0)
                {
                    result.negativePolyhedron = polyhedron;
                }

                if (numZero > 0 && ((requested & REQ_INTR_BOTH) != 0))
                {
                    GetIntersection(polyhedron, numZero, sign, result);
                }
            }
            else if (numNegative == 0)
            {
                result.configuration = CFG_POS_SIDE | (numZero < 3 ? numZero : 4);

                if ((requested & REQ_POLYHEDRON_POS) != 0)
                {
                    result.positivePolyhedron = polyhedron;
                }

                if (numZero > 0 && ((requested & REQ_INTR_BOTH) != 0))
                {
                    GetIntersection(polyhedron, numZero, sign, result);
                }
            }
            else
            {
                result.configuration = CFG_SPLIT;

                if (requested != REQ_CONFIGURATION_ONLY)
                {
                    SplitPolyhedron(polyhedron, dot, sign, result);
                }
            }
            return result;
        }

    private:
        static void GetIntersection(CM const& polyhedron, int numZero,
            std::vector<int> const& sign, Result& result)
        {
            bool const wantIntrMesh = (result.requested & REQ_INTR_MESH) != 0;
            bool const wantIntrPolygon = (result.requested & REQ_INTR_POLYGON) != 0;

            if (numZero == 1)
            {
                GetIntersectionVertex(polyhedron, sign, wantIntrMesh,
                    wantIntrPolygon, result);
            }
            else if (numZero == 2)
            {
                GetIntersectionEdge(polyhedron, sign, wantIntrMesh,
                    wantIntrPolygon, result);
            }
            else  // numZero >= 3
            {
                GetIntersectionPolygon(polyhedron, sign, wantIntrMesh,
                    wantIntrPolygon, result);
            }
        }

        static void GetIntersectionVertex(CM const& polyhedron,
            std::vector<int> const& sign, bool wantIntrMesh, bool wantIntrPolygon,
            Result& result)
        {
            result.intersectionMesh.configuration = ConvexMesh3<Real>::CFG_POINT;
            if (wantIntrMesh)
            {
                result.intersectionMesh.vertices.resize(1);
            }
            if (wantIntrPolygon)
            {
                result.intersectionPolygon.resize(1);
            }

            size_t const numVertices = polyhedron.vertices.size();
            for (size_t i = 0; i < numVertices; ++i)
            {
                if (sign[i] == 0)
                {
                    if (wantIntrMesh)
                    {
                        result.intersectionMesh.vertices[0] = polyhedron.vertices[i];
                    }
                    if (wantIntrPolygon)
                    {
                        result.intersectionPolygon[0] = polyhedron.vertices[i];
                    }
                    return;
                }
            }
        }

        static void GetIntersectionEdge(CM const& polyhedron,
            std::vector<int> const& sign, bool wantIntrMesh, bool wantIntrPolygon,
            Result& result)
        {
            result.intersectionMesh.configuration = ConvexMesh3<Real>::CFG_SEGMENT;
            if (wantIntrMesh)
            {
                result.intersectionMesh.vertices.resize(2);
            }
            if (wantIntrPolygon)
            {
                result.intersectionPolygon.resize(2);
            }

            size_t const numVertices = polyhedron.vertices.size();
            for (size_t i = 0, numFound = 0; i < numVertices; ++i)
            {
                if (sign[i] == 0)
                {
                    if (wantIntrMesh)
                    {
                        result.intersectionMesh.vertices[numFound] = polyhedron.vertices[i];
                    }
                    if (wantIntrPolygon)
                    {
                        result.intersectionPolygon[numFound] = polyhedron.vertices[i];
                    }
                    if (++numFound == 2)
                    {
                        return;
                    }
                }
            }
        }

        static void GetIntersectionPolygon(CM const& polyhedron,
            std::vector<int> const& sign, bool wantIntrMesh, bool wantIntrPolygon,
            Result& result)
        {
            result.intersectionMesh.configuration = ConvexMesh3<Real>::CFG_POLYGON;

            std::vector<Triangle> intersectionMeshTriangles;
            intersectionMeshTriangles.reserve(polyhedron.triangles.size());
            for (auto const& triangle : polyhedron.triangles)
            {
                if (sign[triangle[0]] == 0 && sign[triangle[1]] == 0 && sign[triangle[2]] == 0)
                {
                    intersectionMeshTriangles.push_back(triangle);
                }
            }

            std::vector<Vertex> outVertices;
            std::vector<Triangle> outTriangles;
            UniqueVerticesSimplices<Vertex, int, 3> uvt;
            uvt.RemoveDuplicateAndUnusedVertices(polyhedron.vertices,
                intersectionMeshTriangles, outVertices, outTriangles);

            if (wantIntrPolygon)
            {
                // Get the boundary edges with ordering consistent with the
                // triangle face chirality.
                std::map<EdgeKey<false>, std::array<int, 2>> edgeMap;
                for (auto const& triangle : outTriangles)
                {
                    for (size_t j0 = 2, j1 = 0; j1 < 3; j0 = j1++)
                    {
                        EdgeKey<false> edge(triangle[j0], triangle[j1]);
                        auto iter = edgeMap.find(edge);
                        if (iter != edgeMap.end())
                        {
                            // The edge is now visited twice, so it cannot be a
                            // boundary edge.
                            edgeMap.erase(iter);
                        }
                        else
                        {
                            // The edge is visited the first time, so it might be
                            // a boundary edge.
                            std::array<int, 2> value = { triangle[j1], triangle[j0] };
                            edgeMap.insert(std::make_pair(edge, value));
                        }
                    }
                }

                // Construct the boundary polygon.
                std::vector<int> polygonIndices(edgeMap.size(), -1);
                for (auto const& element : edgeMap)
                {
                    polygonIndices[element.second[0]] = element.second[1];
                }

                result.intersectionPolygon.resize(edgeMap.size());
                for (size_t i = 0; i < result.intersectionPolygon.size(); ++i)
                {
                    result.intersectionPolygon[i] = outVertices[polygonIndices[i]];
                }
            }

            if (wantIntrMesh)
            {
                result.intersectionMesh.vertices = std::move(outVertices);
                result.intersectionMesh.triangles = std::move(outTriangles);
            }
        }

        static void SplitPolyhedron(CM const& polyhedron, std::vector<Real> const& dot,
            std::vector<int> const& sign, Result& result)
        {
            bool const wantPosMesh = (result.requested & REQ_POLYHEDRON_POS) != 0;
            bool const wantNegMesh = (result.requested & REQ_POLYHEDRON_NEG) != 0;
            bool const wantIntrMesh = (result.requested & REQ_INTR_MESH) != 0;
            bool const wantIntrPolygon = (result.requested & REQ_INTR_POLYGON) != 0;

            // The split polyhedra use the input polyhedron's vertices and any
            // edge-interior intersections between the plane and the mesh
            // edges. The center point of the polygon of intersection (if any)
            // is also used as a vertex.
            std::vector<Vertex> splitVertices;
            std::map<EdgeKey<false>, int> eiVMap;
            GetVertexCandidates(polyhedron, dot, sign, splitVertices, eiVMap);

            // Split each triangle face of the polyhedron by the plane.
            std::vector<Triangle> posMesh, negMesh;
            std::map<int, int> posIntersection;
            DoSplit(polyhedron, sign, eiVMap, wantPosMesh, posMesh,
                wantNegMesh, negMesh, posIntersection);

            // Get the polygon of intersection. This is used by all of the
            // requested features.
            std::vector<int> polygon;
            GetIntersectionPolygon(posIntersection, splitVertices,
                wantIntrPolygon, polygon, result);

            if (wantPosMesh || wantNegMesh || wantIntrMesh)
            {
                // Get the polyhedra split by the plane. The polygon of
                // intersection is also computed and used to close the
                // polyhedra.
                GetSplitPolyhedra(splitVertices, polygon, wantIntrMesh,
                    wantPosMesh, posMesh, wantNegMesh, negMesh, result);
            }
        }

        static void GetVertexCandidates(CM const& polyhedron,
            std::vector<Real> const& dot, std::vector<int> const& sign,
            std::vector<Vertex>& splitVertices,
            std::map<EdgeKey<false>, int>& eiVMap)
        {
            // Get the edges of the polyhedron.
            std::set<EdgeKey<false>> edgeMap;
            for (auto const& triangle : polyhedron.triangles)
            {
                edgeMap.insert(EdgeKey<false>(triangle[0], triangle[1]));
                edgeMap.insert(EdgeKey<false>(triangle[1], triangle[2]));
                edgeMap.insert(EdgeKey<false>(triangle[2], triangle[0]));
            }

            // The vertex candidates include the original vertices, any
            // edge-interior intersections between the plane and polyhedron,
            // and the average of the convex-polygon intersection (if there
            // is such an intersection). The number of reserved elements of
            // splitVertices is large enough to avoid resizing of the array
            // later.
            splitVertices.reserve(polyhedron.vertices.size() + edgeMap.size() + 1);
            for (auto const& vertex : polyhedron.vertices)
            {
                splitVertices.push_back(vertex);
            }

            // Compute edge-interior points of intersection between the plane
            // and the mesh edges. The eiVMap container allows accessing the
            // edge-interior vertices when each triangle face of the
            // polyhedron is processed for intersection with the plane.
            for (auto const& element : edgeMap)
            {
                int v0 = element.V[0];
                int v1 = element.V[1];
                if (sign[v0] * sign[v1] < 0)
                {
                    Real denom = dot[v1] - dot[v0];
                    Real w0 = dot[v1] / denom;
                    Real w1 = -dot[v0] / denom;
                    auto eiVertex =
                        w0 * polyhedron.vertices[v0] + w1 * polyhedron.vertices[v1];
                    int const eiIndex = static_cast<int>(splitVertices.size());
                    eiVMap.insert(std::make_pair(EdgeKey<false>(v0, v1), eiIndex));
                    splitVertices.push_back(eiVertex);
                }
            }

            // The average point will be appended to splitVertices later when
            // necessary.
        }

        static void DoSplit(CM const& polyhedron, std::vector<int> const& sign,
            std::map<EdgeKey<false>, int>& eiVMap,
            bool wantPosMesh, std::vector<Triangle>& posMesh,
            bool wantNegMesh, std::vector<Triangle>& negMesh,
            std::map<int, int>& posIntersection)
        {
            for (auto const& triangle : polyhedron.triangles)
            {
                int v0 = triangle[0], v1 = triangle[1], v2 = triangle[2];
                int v01 = -1, v12 = -1, v20 = -1;

                if (sign[v0] > 0)
                {
                    if (sign[v1] > 0)
                    {
                        if (sign[v2] > 0)
                        {
                            // +++
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v1, v2 });
                            }
                        }
                        else if (sign[v2] < 0)
                        {
                            // ++-
                            v12 = eiVMap[EdgeKey<false>(v1, v2)];
                            v20 = eiVMap[EdgeKey<false>(v2, v0)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v12, v20 });
                                posMesh.push_back({ v0, v1, v12 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v2, v20, v12 });
                            }
                            posIntersection.insert(std::make_pair(v20, v12));
                        }
                        else
                        {
                            // ++0
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v1, v2 });
                            }
                        }
                    }
                    else if (sign[v1] < 0)
                    {
                        if (sign[v2] > 0)
                        {
                            // +-+
                            v01 = eiVMap[EdgeKey<false>(v0, v1)];
                            v12 = eiVMap[EdgeKey<false>(v1, v2)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v01, v12 });
                                posMesh.push_back({ v0, v12, v2 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v1, v12, v01 });
                            }
                            posIntersection.insert(std::make_pair(v12, v01));
                        }
                        else if (sign[v2] < 0)
                        {
                            // +--
                            v01 = eiVMap[EdgeKey<false>(v0, v1)];
                            v20 = eiVMap[EdgeKey<false>(v2, v0)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v01, v20 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v1, v20, v01 });
                                negMesh.push_back({ v1, v2, v20 });
                            }
                            posIntersection.insert(std::make_pair(v20, v01));
                        }
                        else
                        {
                            // +-0
                            v01 = eiVMap[EdgeKey<false>(v0, v1)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v2, v0, v01 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v2, v01, v1 });
                            }
                            posIntersection.insert(std::make_pair(v2, v01));
                        }
                    }
                    else
                    {
                        if (sign[v2] > 0)
                        {
                            // +0+
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v1, v2 });
                            }
                        }
                        else if (sign[v2] < 0)
                        {
                            // +0-
                            v20 = eiVMap[EdgeKey<false>(v2, v0)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v1, v20, v0 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v1, v2, v20 });
                            }
                            posIntersection.insert(std::make_pair(v20, v1));
                        }
                        else
                        {
                            // +00
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v1, v2 });
                            }
                            posIntersection.insert(std::make_pair(v2, v1));
                        }
                    }
                }
                else if (sign[v0] < 0)
                {
                    if (sign[v1] > 0)
                    {
                        if (sign[v2] > 0)
                        {
                            // -++
                            v01 = eiVMap[EdgeKey<false>(v0, v1)];
                            v20 = eiVMap[EdgeKey<false>(v2, v0)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v1, v20, v01 });
                                posMesh.push_back({ v1, v2, v20 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v01, v20 });
                            }
                            posIntersection.insert(std::make_pair(v01, v20));
                        }
                        else if (sign[v2] < 0)
                        {
                            // -+-
                            v01 = eiVMap[EdgeKey<false>(v0, v1)];
                            v12 = eiVMap[EdgeKey<false>(v1, v2)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v1, v12, v01 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v01, v12 });
                                negMesh.push_back({ v0, v12, v2 });
                            }
                            posIntersection.insert(std::make_pair(v01, v12));
                        }
                        else
                        {
                            // -+0
                            v01 = eiVMap[EdgeKey<false>(v0, v1)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v1, v2, v01 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v2, v0, v01 });
                            }
                            posIntersection.insert(std::make_pair(v01, v2));
                        }
                    }
                    else if (sign[v1] < 0)
                    {
                        if (sign[v2] > 0)
                        {
                            // --+
                            v12 = eiVMap[EdgeKey<false>(v1, v2)];
                            v20 = eiVMap[EdgeKey<false>(v2, v0)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v2, v20, v12 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v12 });
                                negMesh.push_back({ v0, v12, v20 });
                            }
                            posIntersection.insert(std::make_pair(v12, v20));
                        }
                        else if (sign[v2] < 0)
                        {
                            // ---
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v2 });
                            }
                        }
                        else
                        {
                            // --0
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v2 });
                            }
                        }
                    }
                    else
                    {
                        if (sign[v2] > 0)
                        {
                            // -0+
                            v20 = eiVMap[EdgeKey<false>(v2, v0)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v2, v20, v1 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v20 });
                            }
                            posIntersection.insert(std::make_pair(v1, v20));
                        }
                        else if (sign[v2] < 0)
                        {
                            // -0-
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v2 });
                            }
                        }
                        else
                        {
                            // -00
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v2 });
                            }
                        }
                    }
                }
                else
                {
                    if (sign[v1] > 0)
                    {
                        if (sign[v2] > 0)
                        {
                            // 0++
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v1, v2 });
                            }
                        }
                        else if (sign[v2] < 0)
                        {
                            // 0+-
                            v12 = eiVMap[EdgeKey<false>(v1, v2)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v1, v12, v0 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v2, v0, v12 });
                            }
                            posIntersection.insert(std::make_pair(v0, v12));
                        }
                        else
                        {
                            // 0+0
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v1, v2 });
                            }
                            posIntersection.insert(std::make_pair(v0, v2));
                        }
                    }
                    else if (sign[v1] < 0)
                    {
                        if (sign[v2] > 0)
                        {
                            // 0-+
                            v12 = eiVMap[EdgeKey<false>(v1, v2)];
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v2, v0, v12 });
                            }
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v1, v12, v0 });
                            }
                            posIntersection.insert(std::make_pair(v12, v0));
                        }
                        else if (sign[v2] < 0)
                        {
                            // 0--
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v2 });
                            }
                        }
                        else
                        {
                            // 0-0
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v2 });
                            }
                        }
                    }
                    else
                    {
                        if (sign[v2] > 0)
                        {
                            // 00+
                            if (wantPosMesh)
                            {
                                posMesh.push_back({ v0, v1, v2 });
                            }
                            posIntersection.insert(std::make_pair(v1, v0));
                        }
                        else if (sign[v2] < 0)
                        {
                            // 00-
                            if (wantNegMesh)
                            {
                                negMesh.push_back({ v0, v1, v2 });
                            }
                        }
                        else
                        {
                            // 000
                            // This case cannot occur with exact arithmetic,
                            // because it would have been trapped previously
                            // by tests numPositive == 0 or numNegative == 0.
                            LogError("This case cannot occur with exact arithmetic.");
                        }
                    }
                }
            }
        }

        static void GetIntersectionPolygon(std::map<int, int> const& posIntersection,
            std::vector<Vertex>& splitVertices, bool wantIntrPolygon,
            std::vector<int>& polygon, Result& result)
        {
            size_t const numVertices = posIntersection.size();
            polygon.resize(numVertices);
            auto posIter = posIntersection.begin();
            for (size_t i = 0; i < numVertices; ++i)
            {
                polygon[i] = posIter->first;
                posIter = posIntersection.find(posIter->second);
            }

            if (wantIntrPolygon)
            {
                result.intersectionPolygon.resize(numVertices);
                for (size_t i = 0; i < numVertices; ++i)
                {
                    result.intersectionPolygon[i] = splitVertices[polygon[i]];
                }
            }
        }

        static void GetSplitPolyhedra(std::vector<Vertex>& splitVertices,
            std::vector<int> const& polygon, bool wantIntrMesh,
            bool wantPosMesh, std::vector<Triangle>& posMesh,
            bool wantNegMesh, std::vector<Triangle>& negMesh, Result& result)
        {
            // Triangulate the polygon for use by the positive polyhedron. A
            // triangle fan will not work always work when the polygon has
            // collinear vertices. The average of the polygon vertices is
            // inserted as an extra vertex. The triangulation includes each
            // triangle that is formed by the average point and an edge of the
            // polygon. The negative polyhedron uses the same triangulation
            // but with opposite chirality. NOTE: To avoid biases in the
            // average due to vertex distribution, use the center of mass of
            // the polygon instead.
            Vertex average{ (Real)0, (Real)0, (Real)0 };
            for (auto const& i : polygon)
            {
                average += splitVertices[i];
            }
            int const numVertices = static_cast<int>(polygon.size());
            average /= static_cast<Real>(numVertices);
            int iAvrIndex = static_cast<int>(splitVertices.size());
            splitVertices.push_back(average);

            std::vector<Triangle> intrMesh;
            for (int i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
            {
                if (wantPosMesh)
                {
                    posMesh.push_back({ iAvrIndex, polygon[i0], polygon[i1] });
                }

                if (wantNegMesh)
                {
                    negMesh.push_back({ iAvrIndex, polygon[i1], polygon[i0] });
                }

                if (wantIntrMesh)
                {
                    intrMesh.push_back({ iAvrIndex, polygon[i0], polygon[i1] });
                }
            }

            UniqueVerticesSimplices<Vertex, int, 3> uvt;

            if (wantPosMesh)
            {
                result.positivePolyhedron.configuration = CM::CFG_POLYHEDRON;
                uvt.RemoveDuplicateAndUnusedVertices(splitVertices, posMesh,
                    result.positivePolyhedron.vertices,
                    result.positivePolyhedron.triangles);
            }

            if (wantNegMesh)
            {
                result.negativePolyhedron.configuration = CM::CFG_POLYHEDRON;
                uvt.RemoveDuplicateAndUnusedVertices(splitVertices, negMesh,
                    result.negativePolyhedron.vertices,
                    result.negativePolyhedron.triangles);
            }

            if (wantIntrMesh)
            {
                result.intersectionMesh.configuration = CM::CFG_POLYGON;
                uvt.RemoveDuplicateAndUnusedVertices(splitVertices, intrMesh,
                    result.intersectionMesh.vertices,
                    result.intersectionMesh.triangles);
            }
        }
    };
}
