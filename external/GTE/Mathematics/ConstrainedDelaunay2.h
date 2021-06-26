// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.22

#pragma once

#include <Mathematics/Delaunay2.h>

// Compute the Delaunay triangulation of the input point and then insert
// edges that are constrained to be in the triangulation. For each such
// edge, a retriangulation of the triangle strip containing the edge is
// required. NOTE: If two constrained edges overlap at a point that is
// an interior point of each edge, the second insertion will interfere
// with the retriangulation of the first edge. Although the code here
// will do what is requested, a pair of such edges usually indicates the
// upstream process that generated the edges is not doing what it should.

namespace gte
{
    // The variadic template declaration supports the class
    // ConstrainedDelaunay2<InputType, ComputeType>, which is deprecated and
    // will be removed in a future release. The declaration also supports the
    // replacement class ConstrainedDelaunay2<InputType>. The new class uses
    // a blend of interval arithmetic and rational arithmetic. It also uses
    // unordered sets (hash tables). The replacement performs much better than
    // the deprecated class.
    template <typename T, typename...>
    class ConstrainedDelaunay2 {};
}

namespace gte
{
    // The code has LogAssert statements that throw exceptions when triggered.
    // For a correct algorithm using exact arithmetic, these should not occur.
    // When using floating-point arithmetic, it is possible that rounding errors
    // lead to a malformed triangulation. It is strongly recommended that you
    // choose ComputeType to be a rational type. No divisions are required,
    // either in Delaunay2 or ConstrainedDelaunay2, so you may choose the type
    // to be BSNumber<UInteger> rather than BSRational<UInteger>. However, if
    // you choose ComputeType to be 'float' or 'double', you should call the
    // Insert(...) function in a try-catch block and take appropriate action.
    //
    // The worst-case choices of N for ComputeType of type BSNumber or BSRational
    // with integer storage UIntegerFP32<N> are listed in the next table. The
    // expression requiring the most bits is in the last else-block of ComputePSD.
    // We recommend using only BSNumber, because no divisions are performed in the
    // insertion computations.
    //
    //    input type | compute type | ComputePSD | Delaunay |    N
    //    -----------+--------------+------------+----------+------
    //    float      | BSNumber     |         70 |       35 |   70
    //    double     | BSNumber     |        525 |      263 |  525
    //    float      | BSRational   |        555 |      573 |  573
    //    double     | BSRational   |       4197 |     4329 | 4329
    //
    // The recommended ComputeType is BSNumber<UIntegerFP32<70>> for the
    // InputType 'float' and BSNumber<UIntegerFP32<526>> for the InputType
    // 'double' (525 rounded up to 526 for the bits array size to be a
    // multiple of 8 bytes.)

    template <typename InputType, typename ComputeType>
    class // [[deprecated("Use ConstrainedDelaunay2<InputType> instead.")]]
        ConstrainedDelaunay2<InputType, ComputeType> : public Delaunay2<InputType, ComputeType>
    {
    public:
        // The class is a functor to support computing the constrained
        // Delaunay triangulation of multiple data sets using the same class
        // object.
        virtual ~ConstrainedDelaunay2() = default;

        ConstrainedDelaunay2()
            :
            Delaunay2<InputType, ComputeType>()
        {
        }

        // This operator computes the Delaunay triangulation only. Read the
        // Delaunay2 constructor comments about 'vertices' and 'epsilon'. For
        // ComputeType chosen to be a rational type, pass zero for epsilon.
        bool operator()(int numVertices, Vector2<InputType> const* vertices, InputType epsilon)
        {
            return Delaunay2<InputType, ComputeType>::operator()(numVertices, vertices, epsilon);
        }

        // The 'edge' is the constrained edge to be inserted into the
        // triangulation. If that edge is already in the triangulation, the
        // function returns without any retriangulation and 'partitionedEdge'
        // contains the input 'edge'. If 'edge' is coincident with 1 or more
        // edges already in the triangulation, 'edge' is partitioned into
        // subedges which are then inserted. It is also possible that 'edge'
        // does not overlap already existing edges in the triangulation but
        // has interior points that are vertices in the triangulation; in
        // this case, 'edge' is partitioned and the subedges are inserted.
        // In either case, 'partitionEdge' is an ordered list of indices
        // into the triangulation vertices that are on the edge. It is
        // guaranteed that partitionedEdge.front() = edge[0] and
        // partitionedEdge.back() = edge[1].
        void Insert(std::array<int, 2> edge, std::vector<int>& partitionedEdge)
        {
            LogAssert(
                edge[0] != edge[1] &&
                0 <= edge[0] && edge[0] < this->mNumVertices &&
                0 <= edge[1] && edge[1] < this->mNumVertices,
                "Invalid edge.");

            // The partitionedEdge vector stores the endpoints of the incoming
            // edge if that edge does not contain interior points that are
            // vertices of the Delaunay triangulation. If the edge contains
            // one or more vertices in its interior, the edge is partitioned
            // into subedges, each subedge having vertex endpoints but no
            // interior point is a vertex. The partition is stored in the
            // partitionedEdge vector.
            std::vector<std::array<int, 2>> partition;

            // When using exact arithmetic, a while(!edgeConsumed) loop
            // suffices. When using floating-point arithmetic (which you
            // should not do for CDT), guard against an infinite loop.
            bool edgeConsumed = false;
            size_t const numTriangles = this->mGraph.GetTriangles().size();
            size_t t;
            for (t = 0; t < numTriangles && !edgeConsumed; ++t)
            {
                EdgeKey<false> ekey(edge[0], edge[1]);
                if (this->mGraph.GetEdges().find(ekey) != this->mGraph.GetEdges().end())
                {
                    // The edge already exists in the triangulation.
                    mInsertedEdges.insert(ekey);
                    partition.push_back(edge);
                    break;
                }

                // Get the link edges for the vertex edge[0]. These edges are
                // opposite the link vertex.
                std::vector<std::array<int, 2>> linkEdges;
                GetLinkEdges(edge[0], linkEdges);

                // Determine which link triangle contains the to-be-inserted
                // edge.
                for (auto const& linkEdge : linkEdges)
                {
                    // Compute on which side of the to-be-inserted edge the
                    // link vertices live. The triangles are not degenerate,
                    // so it is not possible for sign0 = sign1 = 0.
                    int v0 = linkEdge[0];
                    int v1 = linkEdge[1];
                    int sign0 = this->mQuery.ToLine(v0, edge[0], edge[1]);
                    int sign1 = this->mQuery.ToLine(v1, edge[0], edge[1]);
                    if (sign0 >= 0 && sign1 <= 0)
                    {
                        if (sign0 > 0)
                        {
                            if (sign1 < 0)
                            {
                                // The triangle <edge[0], v0, v1> strictly
                                // contains the to-be-inserted edge. Gather
                                // the triangles in the triangle strip
                                // containing the edge.
                                edgeConsumed = ProcessTriangleStrip(edge, v0, v1, partition);
                            }
                            else  // sign1 == 0 && sign0 > 0
                            {
                                // The to-be-inserted edge is coincident with
                                // the triangle edge <edge[0], v1>, and it is
                                // guaranteed that the vertex at v1 is an
                                // interior point of <edge[0],edge[1]> because
                                // we previously tested whether edge[] is in
                                // the triangulation.
                                edgeConsumed = ProcessCoincidentEdge(edge, v1, partition);
                            }
                        }
                        else  // sign0 == 0 && sign1 < 0
                        {
                            // The to-be-inserted edge is coincident with
                            // the triangle edge <edge[0], v0>, and it is
                            // guaranteed that the vertex at v0 is an
                            // interior point of <edge[0],edge[1]> because
                            // we previously tested whether edge[] is in
                            // the triangulation.
                            edgeConsumed = ProcessCoincidentEdge(edge, v0, partition);
                        }
                        break;
                    }
                }
            }

            // If the following assertion is triggered, ComputeType was chosen
            // to be 'float' or 'double'. Floating-point rounding errors led to
            // misclassification of signs. The linkEdges-loop exited without
            // ever calling the ProcessTriangleStrip or ProcessCoincidentEdge
            // functions.
            LogAssert(partition.size() > 0, CDTMessage());

            partitionedEdge.resize(partition.size() + 1);
            for (size_t i = 0; i < partition.size(); ++i)
            {
                partitionedEdge[i] = partition[i][0];
            }
            partitionedEdge.back() = partition.back()[1];
        }

        // All edges inserted via the Insert(...) call are stored for use
        // by the caller. If any edge passed to Insert(...) is partitioned
        // into subedges, the subedges are stored but not the original edge.
        std::set<EdgeKey<false>> const& GetInsertedEdges() const
        {
            return mInsertedEdges;
        }

        // The interface functions to the base class Delaunay2 are valid, so
        // access to any Delaunay information is allowed. Perhaps the most
        // important member function is GetGraph() that returns a reference
        // to the ETManifoldMesh that represents the constrained Delaunay
        // triangulation. NOTE: If you want access to the compact arrays
        // via GetIndices(t, indices[]) or GetAdjacencies(t, adjacents[]),
        // you must first call UpdateIndicesAdjacencies() to ensure that the
        // compact arrays are synchonized with the Delaunay graph.

    private:
        using Vertex = VETManifoldMesh::Vertex;
        using Edge = VETManifoldMesh::Edge;
        using Triangle = VETManifoldMesh::Triangle;

        // For a vertex at index v, return the edges of the adjacent triangles,
        // each triangle having v as a vertex and the returned edge is
        // opposite v.
        void GetLinkEdges(int v, std::vector<std::array<int, 2>>& linkEdges)
        {
            auto const& vmap = this->mGraph.GetVertices();
            auto viter = vmap.find(v);
            LogAssert(viter != vmap.end(), "Failed to find vertex in graph.");
            auto vertex = viter->second.get();
            LogAssert(vertex != nullptr, "Unexpected condition.");

            for (auto const& linkTri : vertex->TAdjacent)
            {
                size_t j;
                for (j = 0; j < 3; ++j)
                {
                    if (linkTri->V[j] == vertex->V)
                    {
                        linkEdges.push_back({
                            linkTri->V[(j + 1) % 3], linkTri->V[(j + 2) % 3] });
                        break;
                    }
                }
                LogAssert(j < 3, "Unexpected condition.");
            }
        }

        // The return value is 'true' if the edge did not have to be
        // subdivided because it has an interior point that is a vertex.
        // The return value is 'false' if it does have such a point, in
        // which case edge[0] is updated to the index of that vertex. The
        // caller must process the new edge.
        bool ProcessTriangleStrip(std::array<int, 2>& edge, int v0, int v1,
            std::vector<std::array<int, 2>>& partitionedEdge)
        {
            bool edgeConsumed = true;
            std::array<int, 2> localEdge = edge;

            // Locate and store the triangles in the triangle strip containing
            // the edge.
            std::vector<TriangleKey<true>> tristrip;
            tristrip.emplace_back(localEdge[0], v0, v1);

            auto const& tmap = this->mGraph.GetTriangles();
            auto titer = tmap.find(TriangleKey<true>(localEdge[0], v0, v1));
            LogAssert(titer != tmap.end(), CDTMessage());
            auto tri = titer->second.get();
            LogAssert(tri, CDTMessage());

            // Keep track of the right and left polylines that bound the
            // triangle strip. These polylines can have coincident edges.
            // In particular, this happens when the current triangle in the
            // strip shares an edge with a previous triangle in the strip
            // and the previous triangle is not the immediate predecessor
            // to the current triangle.
            std::vector<int> rightPolygon, leftPolygon;
            rightPolygon.push_back(localEdge[0]);
            rightPolygon.push_back(v0);
            leftPolygon.push_back(localEdge[0]);
            leftPolygon.push_back(v1);

            // When using exact arithmetic, a for(;;) loop suffices. When
            // using floating-point arithmetic (which you should really not
            // do for CDT), guard against an infinite loop.
            size_t const numTriangles = tmap.size();
            size_t t;
            for (t = 0; t < numTriangles; ++t)
            {
                // The current triangle is tri and has edge <v0,v1>. Get
                // the triangle adj that is adjacent to tri via this edge.
                auto adj = tri->GetAdjacentOfEdge(v0, v1);
                LogAssert(adj, CDTMessage());
                tristrip.emplace_back(adj->V[0], adj->V[1], adj->V[2]);

                // Get the vertex of adj that is opposite edge <v0,v1>.
                int vOpposite = 0;
                bool found = adj->GetOppositeVertexOfEdge(v0, v1, vOpposite);
                LogAssert(found, CDTMessage());
                if (vOpposite == localEdge[1])
                {
                    // The triangle strip containing the edge is complete.
                    break;
                }

                // The next triangle in the strip depends on whether the
                // opposite vertex is left-of the edge, right-of the edge
                // or on the edge.
                int querySign = this->mQuery.ToLine(vOpposite, localEdge[0], localEdge[1]);
                if (querySign > 0)
                {
                    tri = adj;
                    v0 = vOpposite;
                    rightPolygon.push_back(v0);
                }
                else if (querySign < 0)
                {
                    tri = adj;
                    v1 = vOpposite;
                    leftPolygon.push_back(v1);
                }
                else
                {
                    // The to-be-inserted edge contains an interior point that
                    // is also a vertex in the triangulation. The edge must be
                    // subdivided. The first subedge is in a triangle strip
                    // that is processed by code below that is outside the
                    // loop. The second subedge must be processed by the
                    // caller.
                    localEdge[1] = vOpposite;
                    edge[0] = vOpposite;
                    edgeConsumed = false;
                    break;
                }
            }
            LogAssert(t < numTriangles, CDTMessage());

            // Insert the final endpoint of the to-be-inserted edge.
            rightPolygon.push_back(localEdge[1]);
            leftPolygon.push_back(localEdge[1]);

            // The retriangulation depends on counterclockwise ordering of
            // the boundary right and left polygons. The right polygon is
            // already counterclockwise ordered. The left polygon is
            // clockwise ordered, so reverse it.
            std::reverse(leftPolygon.begin(), leftPolygon.end());

            // Update the inserted edges.
            mInsertedEdges.insert(EdgeKey<false>(localEdge[0], localEdge[1]));
            partitionedEdge.push_back(localEdge);

            // Remove the triangle strip from the full triangulation. This
            // must occur before the retriangulation which inserts new
            // triangles into the full triangulation.
            for (auto const& tkey : tristrip)
            {
                this->mGraph.Remove(tkey.V[0], tkey.V[1], tkey.V[2]);
            }

            // Retriangulate the tristrip region.
            Retriangulate(leftPolygon);
            Retriangulate(rightPolygon);

            return edgeConsumed;
        }

        // Process a to-be-inserted edge that is coincident with an already
        // existing triangulation edge.
        bool ProcessCoincidentEdge(std::array<int, 2>& edge, int v,
            std::vector<std::array<int, 2>>& partitionedEdge)
        {
            mInsertedEdges.insert(EdgeKey<false>(edge[0], v));
            partitionedEdge.push_back({ edge[0], v });
            edge[0] = v;
            bool edgeConsumed = (v == edge[1]);
            return edgeConsumed;
        }

        // Retriangulate the polygon via a bisection-like method that finds
        // vertices closest to the current polygon base edge. The function
        // is naturally recursive, but simulated recursion is used to avoid
        // a large program stack by instead using the heap.
        void Retriangulate(std::vector<int> const& polygon)
        {
            std::vector<std::array<size_t, 2>> stack(polygon.size());
            int top = -1;
            stack[++top] = { 0, polygon.size() - 1 };
            while (top != -1)
            {
                std::array<size_t, 2> i = stack[top--];
                if (i[1] > i[0] + 1)
                {
                    // Get the vertex indices for the specified i-values.
                    int v0 = polygon[i[0]];
                    int v1 = polygon[i[1]];

                    // Select isplit in the index range [i[0]+1,i[1]-1] so
                    // that the vertex at index polygon[isplit] attains the
                    // minimum distance to the edge with vertices at the
                    // indices polygon[i[0]] and polygon[i[1]].
                    size_t isplit = SelectSplit(polygon, i[0], i[1]);
                    int vsplit = polygon[isplit];

                    // Insert the triangle into the Delaunay graph.
                    this->mGraph.Insert(v0, vsplit, v1);

                    stack[++top] = { i[0], isplit };
                    stack[++top] = { isplit, i[1] };
                }
            }
        }

        // Determine the polygon vertex with index strictly between i0 and i1
        // that minimizes the pseudosquared distance from that vertex to the
        // line segment whose endpoints are at indices i0 and i1.
        size_t SelectSplit(std::vector<int> const& polygon, size_t i0, size_t i1)
        {
            size_t i2;
            if (i1 == i0 + 2)
            {
                // This is the only candidate.
                i2 = i0 + 1;
            }
            else  // i1 - i0 > 2
            {
                // Select the index i2 in [i0+1,i1-1] for which the distance
                // from the vertex v2 at i2 to the edge <v0,v1> is minimized.
                // To allow exact arithmetic, use a pseudosquared distance
                // that avoids divisions and square roots.
                i2 = i0 + 1;
                int v0 = polygon[i0];
                int v1 = polygon[i1];
                int v2 = polygon[i2];

                // Precompute some common values that are used in all calls
                // to ComputePSD.
                Vector2<ComputeType> const& ctv0 = this->mComputeVertices[v0];
                Vector2<ComputeType> const& ctv1 = this->mComputeVertices[v1];
                Vector2<ComputeType> V1mV0 = ctv1 - ctv0;
                ComputeType sqrlen10 = Dot(V1mV0, V1mV0);

                // Locate the minimum pseudosquared distance.
                ComputeType minpsd = ComputePSD(v0, v1, v2, V1mV0, sqrlen10);
                for (size_t i = i2 + 1; i < i1; ++i)
                {
                    v2 = polygon[i];
                    ComputeType psd = ComputePSD(v0, v1, v2, V1mV0, sqrlen10);
                    if (psd < minpsd)
                    {
                        minpsd = psd;
                        i2 = i;
                    }
                }
            }
            return i2;
        }

        // Compute a pseudosquared distance from the vertex at v2 to the edge
        // <v0,v1>. The result is exact for rational arithmetic and does not
        // involve division. This allows ComputeType to be BSNumber<UInteger>
        // rather than BSRational<UInteger>, which leads to better
        // performance.
        ComputeType ComputePSD(int v0, int v1, int v2,
            Vector2<ComputeType> const& V1mV0, ComputeType const& sqrlen10)
        {
            ComputeType const zero = static_cast<ComputeType>(0);
            Vector2<ComputeType> const& ctv0 = this->mComputeVertices[v0];
            Vector2<ComputeType> const& ctv1 = this->mComputeVertices[v1];
            Vector2<ComputeType> const& ctv2 = this->mComputeVertices[v2];
            Vector2<ComputeType> V2mV0 = ctv2 - ctv0;
            ComputeType dot1020 = Dot(V1mV0, V2mV0);
            ComputeType psd;

            if (dot1020 <= zero)
            {
                ComputeType sqrlen20 = Dot(V2mV0, V2mV0);
                psd = sqrlen10 * sqrlen20;
            }
            else
            {
                Vector2<ComputeType> V2mV1 = ctv2 - ctv1;
                ComputeType dot1021 = Dot(V1mV0, V2mV1);
                if (dot1021 >= zero)
                {
                    ComputeType sqrlen21 = Dot(V2mV1, V2mV1);
                    psd = sqrlen10 * sqrlen21;
                }
                else
                {
                    ComputeType sqrlen20 = Dot(V2mV0, V2mV0);
                    psd = sqrlen10 * sqrlen20 - dot1020 * dot1020;
                }
            }

            return psd;
        }

        // All edges inserted via the Insert(...) call are stored for use
        // by the caller. If any edge passed to Insert(...) is partitioned
        // into subedges, the subedges are inserted into this member.
        std::set<EdgeKey<false>> mInsertedEdges;

    private:
        // All LogAssert statements other than the first one in the Insert
        // call possibly can occur when ComputeType is chosen to be 'float'
        // or 'double' rather than an arbitrary-precision type. This function
        // encapsulates a message that is included in the logging and in the
        // thrown exception explaining that floating-point rounding errors
        // are most likely the problem and that you should consider using
        // arbitrary precision for the ComputeType.
        template <typename Dummy = ComputeType>
        static typename std::enable_if<!is_arbitrary_precision<Dummy>::value, std::string>::type
            CDTMessage()
        {
return R"(
ComputeType is a floating-point type. The assertions can be triggered because
of rounding errors. Repeat the call to operator() using ComputeType the type
BSNumber<UIntegerAP32>. If no assertion is triggered, the problem was most
likely due to rounding errors. If an assertion is triggered, please file a
bug report and provide the input dataset to the operator()(...) function.
)";
        }

        template <typename Dummy = ComputeType>
        static typename std::enable_if<is_arbitrary_precision<Dummy>::value, std::string>::type
            CDTMessage()
        {
return R"(
The failed assertion is unexpected when using arbitrary-precision arithmetic.
Please file a bug report and provide the input dataset to the operator()(...)
function.
)";
        }
    };
}

namespace gte
{
    // The input type must be 'float' or 'double'. The user no longer has
    // the responsibility to specify the compute type.

    template <typename T>
    class ConstrainedDelaunay2<T> : public Delaunay2<T>
    {
    public:
        // The class is a functor to support computing the constrained
        // Delaunay triangulation of multiple data sets using the same class
        // object.
        virtual ~ConstrainedDelaunay2() = default;

        ConstrainedDelaunay2()
            :
            Delaunay2<T>(),
            mInsertedEdges{},
            mCRPool(maxNumCRPool)
        {
        }

        // This operator computes the Delaunay triangulation only. Edges are
        // inserted later.
        bool operator()(std::vector<Vector2<T>> const& vertices)
        {
            return Delaunay2<T>::operator()(vertices);
        }

        bool operator()(size_t numVertices, Vector2<T> const* vertices)
        {
            return Delaunay2<T>::operator()(numVertices, vertices);
        }

        // The 'edge' is the constrained edge to be inserted into the
        // triangulation. If that edge is already in the triangulation, the
        // function returns without any retriangulation and 'partitionedEdge'
        // contains the input 'edge'. If 'edge' is coincident with 1 or more
        // edges already in the triangulation, 'edge' is partitioned into
        // subedges which are then inserted. It is also possible that 'edge'
        // does not overlap already existing edges in the triangulation but
        // has interior points that are vertices in the triangulation; in
        // this case, 'edge' is partitioned and the subedges are inserted.
        // In either case, 'partitionEdge' is an ordered list of indices
        // into the triangulation vertices that are on the edge. It is
        // guaranteed that partitionedEdge.front() = edge[0] and
        // partitionedEdge.back() = edge[1].
        void Insert(std::array<int32_t, 2> edge, std::vector<int32_t>& partitionedEdge)
        {
            LogAssert(
                edge[0] != edge[1] &&
                0 <= edge[0] && edge[0] < static_cast<int32_t>(this->GetNumVertices()) &&
                0 <= edge[1] && edge[1] < static_cast<int32_t>(this->GetNumVertices()),
                "Invalid edge.");

            // The partitionedEdge vector stores the endpoints of the incoming
            // edge if that edge does not contain interior points that are
            // vertices of the Delaunay triangulation. If the edge contains
            // one or more vertices in its interior, the edge is partitioned
            // into subedges, each subedge having vertex endpoints but no
            // interior point is a vertex. The partition is stored in the
            // partitionedEdge vector.
            std::vector<std::array<int32_t, 2>> partition;

            // When using exact arithmetic, a while(!edgeConsumed) loop
            // suffices. Just in case the code has a bug, guard against an
            // infinite loop.
            bool edgeConsumed = false;
            size_t const numTriangles = this->mGraph.GetTriangles().size();
            size_t t;
            for (t = 0; t < numTriangles && !edgeConsumed; ++t)
            {
                EdgeKey<false> ekey(edge[0], edge[1]);
                if (this->mGraph.GetEdges().find(ekey) != this->mGraph.GetEdges().end())
                {
                    // The edge already exists in the triangulation.
                    mInsertedEdges.insert(ekey);
                    partition.push_back(edge);
                    break;
                }

                // Get the link edges for the vertex edge[0]. These edges are
                // opposite the link vertex.
                std::vector<std::array<int32_t, 2>> linkEdges;
                GetLinkEdges(edge[0], linkEdges);

                // Determine which link triangle contains the to-be-inserted
                // edge.
                for (auto const& linkEdge : linkEdges)
                {
                    // Compute on which side of the to-be-inserted edge the
                    // link vertices live. The triangles are not degenerate,
                    // so it is not possible for sign0 = sign1 = 0.
                    size_t e0Index = static_cast<size_t>(edge[0]);
                    size_t e1Index = static_cast<size_t>(edge[1]);
                    size_t v0Index = static_cast<size_t>(linkEdge[0]);
                    size_t v1Index = static_cast<size_t>(linkEdge[1]);
                    int32_t sign0 = ToLine(v0Index, e0Index, e1Index);
                    int32_t sign1 = ToLine(v1Index, e0Index, e1Index);
                    if (sign0 >= 0 && sign1 <= 0)
                    {
                        if (sign0 > 0)
                        {
                            if (sign1 < 0)
                            {
                                // The triangle <edge[0], v0, v1> strictly
                                // contains the to-be-inserted edge. Gather
                                // the triangles in the triangle strip
                                // containing the edge.
                                edgeConsumed = ProcessTriangleStrip(edge, v0Index, v1Index, partition);
                            }
                            else  // sign1 == 0 && sign0 > 0
                            {
                                // The to-be-inserted edge is coincident with
                                // the triangle edge <edge[0], v1>, and it is
                                // guaranteed that the vertex at v1 is an
                                // interior point of <edge[0],edge[1]> because
                                // we previously tested whether edge[] is in
                                // the triangulation.
                                edgeConsumed = ProcessCoincidentEdge(edge, v1Index, partition);
                            }
                        }
                        else  // sign0 == 0 && sign1 < 0
                        {
                            // The to-be-inserted edge is coincident with
                            // the triangle edge <edge[0], v0>, and it is
                            // guaranteed that the vertex at v0 is an
                            // interior point of <edge[0],edge[1]> because
                            // we previously tested whether edge[] is in
                            // the triangulation.
                            edgeConsumed = ProcessCoincidentEdge(edge, v0Index, partition);
                        }
                        break;
                    }
                }
            }

            // If the following assertion is triggered, ComputeType was chosen
            // to be 'float' or 'double'. Floating-point rounding errors led to
            // misclassification of signs. The linkEdges-loop exited without
            // ever calling the ProcessTriangleStrip or ProcessCoincidentEdge
            // functions.
            LogAssert(partition.size() > 0, CDTMessage());

            partitionedEdge.resize(partition.size() + 1);
            for (size_t i = 0; i < partition.size(); ++i)
            {
                partitionedEdge[i] = partition[i][0];
            }
            partitionedEdge.back() = partition.back()[1];
        }

        // All edges inserted via the Insert(...) call are stored for use
        // by the caller. If any edge passed to Insert(...) is partitioned
        // into subedges, the subedges are stored but not the original edge.
        using EdgeKeySet = std::unordered_set<EdgeKey<false>,
            EdgeKey<false>, EdgeKey<false>>;

        EdgeKeySet const& GetInsertedEdges() const
        {
            return mInsertedEdges;
        }

        // The interface functions to the base class Delaunay2 are valid, so
        // access to any Delaunay information is allowed. Perhaps the most
        // important member function is GetGraph() that returns a reference
        // to the ETManifoldMesh that represents the constrained Delaunay
        // triangulation. NOTE: If you want access to the compact arrays
        // via GetIndices(t, indices[]) or GetAdjacencies(t, adjacents[]),
        // you must first call UpdateIndicesAdjacencies() to ensure that the
        // compact arrays are synchonized with the Delaunay graph.

    private:
        // The type of the read-only input vertices[] when converted for
        // rational arithmetic.
        using InputRational = typename Delaunay2<T>::InputRational;

        // The compute type used for exact sign classification.
        static int32_t constexpr ComputeNumWords = std::is_same<T, float>::value ? 70 : 526;
        using ComputeRational = BSNumber<UIntegerFP32<ComputeNumWords>>;

        // Convenient renaming.
        using Vertex = VETManifoldMesh::Vertex;
        using Edge = VETManifoldMesh::Edge;
        using Triangle = VETManifoldMesh::Triangle;

        // For a vertex at index v, return the edges of the adjacent triangles,
        // each triangle having v as a vertex and the returned edge is
        // opposite v.
        void GetLinkEdges(int32_t v, std::vector<std::array<int32_t, 2>>& linkEdges)
        {
            auto const& vmap = this->mGraph.GetVertices();
            auto viter = vmap.find(v);
            LogAssert(viter != vmap.end(), "Failed to find vertex in graph.");
            auto vertex = viter->second.get();
            LogAssert(vertex != nullptr, "Unexpected condition.");

            for (auto const& linkTri : vertex->TAdjacent)
            {
                size_t j;
                for (j = 0; j < 3; ++j)
                {
                    if (linkTri->V[j] == vertex->V)
                    {
                        linkEdges.push_back({
                            linkTri->V[(j + 1) % 3], linkTri->V[(j + 2) % 3] });
                        break;
                    }
                }
                LogAssert(j < 3, "Unexpected condition.");
            }
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
            auto const& inP = this->mVertices[pIndex];
            Vector2<T> const& inV0 = this->mVertices[v0Index];
            Vector2<T> const& inV1 = this->mVertices[v1Index];

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
            auto const& irP = this->mIRVertices[pIndex];
            auto const& irV0 = this->mIRVertices[v0Index];
            auto const& irV1 = this->mIRVertices[v1Index];

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

        // The return value is 'true' if the edge did not have to be
        // subdivided because it has an interior point that is a vertex.
        // The return value is 'false' if it does have such a point, in
        // which case edge[0] is updated to the index of that vertex. The
        // caller must process the new edge.
        bool ProcessTriangleStrip(std::array<int32_t, 2>& edge, size_t v0Index,
            size_t v1Index, std::vector<std::array<int32_t, 2>>& partitionedEdge)
        {
            int32_t v0 = static_cast<int32_t>(v0Index);
            int32_t v1 = static_cast<int32_t>(v1Index);
            bool edgeConsumed = true;
            std::array<int32_t, 2> localEdge = edge;

            // Locate and store the triangles in the triangle strip containing
            // the edge.
            ETManifoldMesh tristrip;
            tristrip.Insert(localEdge[0], v0, v1);

            auto const& tmap = this->mGraph.GetTriangles();
            auto titer = tmap.find(TriangleKey<true>(localEdge[0], v0, v1));
            LogAssert(titer != tmap.end(), CDTMessage());
            auto tri = titer->second.get();
            LogAssert(tri, CDTMessage());

            // Keep track of the right and left polylines that bound the
            // triangle strip. These polylines can have coincident edges.
            // In particular, this happens when the current triangle in the
            // strip shares an edge with a previous triangle in the strip
            // and the previous triangle is not the immediate predecessor
            // to the current triangle.
            std::vector<int32_t> rightPolygon, leftPolygon;
            rightPolygon.push_back(localEdge[0]);
            rightPolygon.push_back(v0);
            leftPolygon.push_back(localEdge[0]);
            leftPolygon.push_back(v1);

            // When using exact arithmetic, a for(;;) loop suffices. When
            // using floating-point arithmetic (which you should really not
            // do for CDT), guard against an infinite loop.
            size_t const numTriangles = tmap.size();
            size_t t;
            for (t = 0; t < numTriangles; ++t)
            {
                // The current triangle is tri and has edge <v0,v1>. Get
                // the triangle adj that is adjacent to tri via this edge.
                auto adj = tri->GetAdjacentOfEdge(v0, v1);
                LogAssert(adj, CDTMessage());
                tristrip.Insert(adj->V[0], adj->V[1], adj->V[2]);

                // Get the vertex of adj that is opposite edge <v0,v1>.
                int32_t vOpposite = 0;
                bool found = adj->GetOppositeVertexOfEdge(v0, v1, vOpposite);
                LogAssert(found, CDTMessage());
                if (vOpposite == localEdge[1])
                {
                    // The triangle strip containing the edge is complete.
                    break;
                }

                // The next triangle in the strip depends on whether the
                // opposite vertex is left-of the edge, right-of the edge
                // or on the edge.
                int32_t querySign = ToLine(static_cast<size_t>(vOpposite),
                    static_cast<size_t>(localEdge[0]), static_cast<size_t>(localEdge[1]));
                if (querySign > 0)
                {
                    tri = adj;
                    v0 = vOpposite;
                    rightPolygon.push_back(v0);
                }
                else if (querySign < 0)
                {
                    tri = adj;
                    v1 = vOpposite;
                    leftPolygon.push_back(v1);
                }
                else
                {
                    // The to-be-inserted edge contains an interior point that
                    // is also a vertex in the triangulation. The edge must be
                    // subdivided. The first subedge is in a triangle strip
                    // that is processed by code below that is outside the
                    // loop. The second subedge must be processed by the
                    // caller.
                    localEdge[1] = vOpposite;
                    edge[0] = vOpposite;
                    edgeConsumed = false;
                    break;
                }
            }
            LogAssert(t < numTriangles, CDTMessage());

            // Insert the final endpoint of the to-be-inserted edge.
            rightPolygon.push_back(localEdge[1]);
            leftPolygon.push_back(localEdge[1]);

            // The retriangulation depends on counterclockwise ordering of
            // the boundary right and left polygons. The right polygon is
            // already counterclockwise ordered. The left polygon is
            // clockwise ordered, so reverse it.
            std::reverse(leftPolygon.begin(), leftPolygon.end());

            // Update the inserted edges.
            mInsertedEdges.insert(EdgeKey<false>(localEdge[0], localEdge[1]));
            partitionedEdge.push_back(localEdge);

            // Remove the triangle strip from the full triangulation. This
            // must occur before the retriangulation which inserts new
            // triangles into the full triangulation.
            for (auto const& element : tristrip.GetTriangles())
            {
                auto const& tkey = element.first;
                this->mGraph.Remove(tkey.V[0], tkey.V[1], tkey.V[2]);
            }

            // Retriangulate the tristrip region.
            Retriangulate(leftPolygon);
            Retriangulate(rightPolygon);

            return edgeConsumed;
        }

        // Process a to-be-inserted edge that is coincident with an already
        // existing triangulation edge.
        bool ProcessCoincidentEdge(std::array<int32_t, 2>& edge, size_t vIndex,
            std::vector<std::array<int32_t, 2>>& partitionedEdge)
        {
            int32_t v = static_cast<int32_t>(vIndex);
            mInsertedEdges.insert(EdgeKey<false>(edge[0], v));
            partitionedEdge.push_back({ edge[0], v });
            edge[0] = v;
            bool edgeConsumed = (v == edge[1]);
            return edgeConsumed;
        }

        // Retriangulate the polygon via a bisection-like method that finds
        // vertices closest to the current polygon base edge. The function
        // is naturally recursive, but simulated recursion is used to avoid
        // a large program stack by instead using the heap.
        void Retriangulate(std::vector<int32_t> const& polygon)
        {
            std::vector<std::array<size_t, 2>> stack(polygon.size());
            size_t top = std::numeric_limits<size_t>::max();
            stack[++top] = { 0, polygon.size() - 1 };
            while (top != std::numeric_limits<size_t>::max())
            {
                std::array<size_t, 2> i = stack[top--];
                if (i[1] > i[0] + 1)
                {
                    // Get the vertex indices for the specified i-values.
                    int32_t v0 = polygon[i[0]];
                    int32_t v1 = polygon[i[1]];

                    // Select isplit in the index range [i[0]+1,i[1]-1] so
                    // that the vertex at index polygon[isplit] attains the
                    // minimum distance to the edge with vertices at the
                    // indices polygon[i[0]] and polygon[i[1]].
                    size_t isplit = SelectSplit(polygon, i[0], i[1]);
                    int32_t vsplit = polygon[isplit];

                    // Insert the triangle into the Delaunay graph.
                    this->mGraph.Insert(v0, vsplit, v1);

                    stack[++top] = { i[0], isplit };
                    stack[++top] = { isplit, i[1] };
                }
            }
        }

        static ComputeRational const& Copy(InputRational const& source,
            ComputeRational& target)
        {
            target.SetSign(source.GetSign());
            target.SetBiasedExponent(source.GetBiasedExponent());
            target.GetUInteger().CopyFrom(source.GetUInteger());
            return target;
        }

        // Determine the polygon vertex with index strictly between i0 and i1
        // that minimizes the pseudosquared distance from that vertex to the
        // line segment whose endpoints are at indices i0 and i1.
        size_t SelectSplit(std::vector<int32_t> const& polygon, size_t i0, size_t i1)
        {
            size_t i2;
            if (i1 == i0 + 2)
            {
                // This is the only candidate.
                i2 = i0 + 1;
            }
            else  // i1 - i0 > 2
            {
                // Select the index i2 in [i0+1,i1-1] for which the distance
                // from the vertex v2 at i2 to the edge <v0,v1> is minimized.
                // To allow exact arithmetic, use a pseudosquared distance
                // that avoids divisions and square roots.
                i2 = i0 + 1;
                size_t v0 = static_cast<size_t>(polygon[i0]);
                size_t v1 = static_cast<size_t>(polygon[i1]);
                size_t v2 = static_cast<size_t>(polygon[i2]);

                // Precompute some common values that are used in all calls
                // to ComputePSD.
                auto const& irV0 = this->mIRVertices[v0];
                auto const& irV1 = this->mIRVertices[v1];
                auto const& irV2 = this->mIRVertices[v2];
                auto const& crV0x = Copy(irV0[0], mCRPool[0]);
                auto const& crV0y = Copy(irV0[1], mCRPool[1]);
                auto const& crV1x = Copy(irV1[0], mCRPool[2]);
                auto const& crV1y = Copy(irV1[1], mCRPool[3]);
                auto const& crV2x = Copy(irV2[0], mCRPool[4]);
                auto const& crV2y = Copy(irV2[1], mCRPool[5]);
                auto& crV1mV0x = mCRPool[6];
                auto& crV1mV0y = mCRPool[7];
                auto& crSqrLen10 = mCRPool[8];
                auto& crPSD = mCRPool[9];
                auto& crMinPSD = mCRPool[10];

                crV1mV0x = crV1x - crV0x;
                crV1mV0y = crV1y - crV0y;
                crSqrLen10 = crV1mV0x * crV1mV0x + crV1mV0y * crV1mV0y;

                // Locate the minimum pseudosquared distance.
                ComputePSD(crV0x, crV0y, crV1x, crV1y, crV2x, crV2y,
                    crV1mV0x, crV1mV0y, crSqrLen10, crMinPSD);
                for (size_t i = i2 + 1; i < i1; ++i)
                {
                    v2 = polygon[i];
                    auto const& irNextV2 = this->mIRVertices[v2];
                    this->Copy(irNextV2[0], mCRPool[4]);
                    this->Copy(irNextV2[1], mCRPool[5]);
                    ComputePSD(crV0x, crV0y, crV1x, crV1y, crV2x, crV2y,
                        crV1mV0x, crV1mV0y, crSqrLen10, crPSD);
                    if (crPSD < crMinPSD)
                    {
                        crMinPSD = crPSD;
                        i2 = i;
                    }
                }
            }
            return i2;
        }

        // Compute a pseudosquared distance from the vertex at v2 to the edge
        // <v0,v1>. The result is exact for rational arithmetic and does not
        // involve division. This allows ComputeType to be BSNumber<UInteger>
        // rather than BSRational<UInteger>, which leads to better
        // performance.
        void ComputePSD(
            ComputeRational const& crV0x,
            ComputeRational const& crV0y,
            ComputeRational const& crV1x,
            ComputeRational const& crV1y,
            ComputeRational const& crV2x,
            ComputeRational const& crV2y,
            ComputeRational const& crV1mV0x,
            ComputeRational const& crV1mV0y,
            ComputeRational const& crSqrLen10,
            ComputeRational& crPSD)
        {
            auto& crV2mV0x = mCRPool[11];
            auto& crV2mV0y = mCRPool[12];
            auto& crV2mV1x = mCRPool[13];
            auto& crV2mV1y = mCRPool[14];
            auto& crDot1020 = mCRPool[15];
            auto& crSqrLen20 = mCRPool[16];
            auto& crDot1021 = mCRPool[17];
            auto& crSqrLen21 = mCRPool[18];

            crV2mV0x = crV2x - crV0x;
            crV2mV0y = crV2y - crV0y;
            crDot1020 = crV1mV0x * crV2mV0x + crV1mV0y * crV2mV0y;

            if (crDot1020.GetSign() <= 0)
            {
                crSqrLen20 = crV2mV0x * crV2mV0x + crV2mV0y * crV2mV0y;
                crPSD = crSqrLen10 * crSqrLen20;
            }
            else
            {
                crV2mV1x = crV2x - crV1x;
                crV2mV1y = crV2y - crV1y;
                crDot1021 = crV1mV0x * crV2mV1x + crV1mV0y * crV2mV1y;
                if (crDot1021.GetSign() >= 0)
                {
                    crSqrLen21 = crV2mV1x * crV2mV1x + crV2mV1y * crV2mV1y;
                    crPSD = crSqrLen10 * crSqrLen21;
                }
                else
                {
                    crSqrLen20 = crV2mV0x * crV2mV0x + crV2mV0y * crV2mV0y;
                    crPSD = crSqrLen10 * crSqrLen20 - crDot1020 * crDot1020;
                }
            }
        }

        static std::string CDTMessage()
        {
            return R"(
The failed assertion is unexpected when using arbitrary-precision arithmetic.
Please file a bug report and provide the input dataset to the operator()(...)
function.
)";
        }

        // All edges inserted via the Insert(...) call are stored for use
        // by the caller. If any edge passed to Insert(...) is partitioned
        // into subedges, the subedges are inserted into this member.
        EdgeKeySet mInsertedEdges;

        // Sufficient storage for the expression trees related to computing
        // the exact pseudosquared distances in SelectSplit and ComputePSD.
        static size_t constexpr maxNumCRPool = 19;
        mutable std::vector<ComputeRational> mCRPool;
    };
}
