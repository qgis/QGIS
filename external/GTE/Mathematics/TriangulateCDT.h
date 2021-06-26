// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.22

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/PolygonTree.h>
#include <Mathematics/ConstrainedDelaunay2.h>
#include <numeric>

// The fundamental problem is to compute the triangulation of a polygon tree.
// The outer polygons have counterclockwise ordered vertices. The inner
// polygons have clockwise ordered vertices. The algorithm uses Constrained
// Delaunay Triangulation and the implementation allows polygons to share
// vertices and edges.
//
// The polygons are not required to be simple in the sense that a vertex can
// be shared by an even number of edges, where the number is larger than 2.
// The input points can have duplicates, which the triangulator handles
// correctly. The algorithm supports coincident vertex-edge and coincident
// edge-edge configurations. See the document
//   https://www.geometrictools.com/Documentation/TriangulationByCDT.pdf
// for examples.
//
// If two edges intersect at edge-interior points, the current implementation
// cannot handle this. A pair of such edges cannot simultaneously be inserted
// into the constrained triangulation without affecting each other's local
// re-triangulation. A pending work item is to add validation code and fix
// an incoming malformed polygon tree during the triangulation.
//
// The input points are a vertex pool. The input tree is a PolygonTree object,
// defined in PolygonTree.h. Any outer polygon has vertices points[outer[0]]
// through points[outer[outer.size()-1]] listed in counterclockwise order. Any
// inner polygon has vertices points[inner[0]] through
// points[inner[inner.size()-1]] listed in clockwise order. The output tree
// contains the triangulation of the polygon tree on a per-node basis. If
// coincident vertex-edge or coincident edge-edge configurations exist in
// the polygon tree, the corresponding output polygons differ from the input
// polygons in that they have more vertices due to edge splits. The triangle
// chirality (winding order) is the same as the containing polygon.
//
// TriangulateCDT uses the ComputeType only for the ConstrainedDelaunay2
// object. To avoid the heap management costs of BSNumber<UIntegerAP32> when
// the input datasets are large, use BSNumber<UIntegerFP32<N>>. The worst-case
// choices of N for ComputeType are listed in the next table.
//
//    input type | compute type |    N
//    -----------+--------------+------
//    float      | BSNumber     |   70
//    double     | BSNumber     |  525
//    float      | BSRational   |  573
//    double     | BSRational   | 4329

namespace gte
{
    // The variadic template declaration supports the class
    // TriangulateCDT<InputType, ComputeType>, which is deprecated and will
    // be removed in a future release. The declaration also supports the
    // replacement class TriangulateCDT<InputType>.
    template <typename T, typename...>
    class TriangulateCDT {};
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
    // 'double' (525 rounded up to 526 for the bits array to be a multiple
    // of 8 bytes.)

    template <typename InputType, typename ComputeType>
    class // [[deprecated("Use TriangulateCDT<InputType> instead.")]]
        TriangulateCDT<InputType, ComputeType>
    {
    public:
        void operator()(
            std::vector<Vector2<InputType>> const& inputPoints,
            std::shared_ptr<PolygonTree> const& inputTree,
            PolygonTreeEx& outputTree)
        {
            operator()(static_cast<int>(inputPoints.size()), inputPoints.data(),
                inputTree, outputTree);
        }

        void operator()(int numInputPoints, Vector2<InputType> const* inputPoints,
            std::shared_ptr<PolygonTree> const& inputTree,
            PolygonTreeEx& outputTree)
        {
            LogAssert(numInputPoints >= 3 && inputPoints != nullptr && inputTree,
                "Invalid argument.");

            CopyAndCompactify(inputTree, outputTree);
            Triangulate(numInputPoints, inputPoints, outputTree);
        }

    private:
        void CopyAndCompactify(std::shared_ptr<PolygonTree> const& input,
            PolygonTreeEx& output)
        {
            output.nodes.clear();
            output.interiorTriangles.clear();
            output.interiorNodeIndices.clear();
            output.exteriorTriangles.clear();
            output.exteriorNodeIndices.clear();
            output.insideTriangles.clear();
            output.insideNodeIndices.clear();
            output.outsideTriangles.clear();
            output.allTriangles.clear();

            // Count the number of nodes in the tree.
            size_t numNodes = 1;  // the root node
            std::queue<std::shared_ptr<PolygonTree>> queue;
            queue.push(input);
            while (queue.size() > 0)
            {
                std::shared_ptr<PolygonTree> node = queue.front();
                queue.pop();
                numNodes += node->child.size();
                for (auto const& child : node->child)
                {
                    queue.push(child);
                }
            }

            // Create the PolygonTreeEx nodes.
            output.nodes.resize(numNodes);
            for (size_t i = 0; i < numNodes; ++i)
            {
                output.nodes[i].self = i;
            }
            output.nodes[0].chirality = +1;
            output.nodes[0].parent = std::numeric_limits<size_t>::max();

            size_t current = 0, last = 0, minChild = 1;
            queue.push(input);
            while (queue.size() > 0)
            {
                std::shared_ptr<PolygonTree> node = queue.front();
                queue.pop();
                auto& exnode = output.nodes[current++];
                exnode.polygon = node->polygon;
                exnode.minChild = minChild;
                exnode.supChild = minChild + node->child.size();
                minChild = exnode.supChild;
                for (auto const& child : node->child)
                {
                    auto& exchild = output.nodes[++last];
                    exchild.chirality = -exnode.chirality;
                    exchild.parent = exnode.self;
                    queue.push(child);
                }
            }
        }

        void Triangulate(int numInputPoints, Vector2<InputType> const* inputPoints,
            PolygonTreeEx& tree)
        {
            // The constrained Delaunay triangulator will be given the unique
            // points referenced by the polygons in the tree. The tree
            // 'polygon' indices are relative to inputPoints[], but they are
            // temporarily mapped to indices relative to 'points'. Once the
            // triangulation is complete, the indices are restored to those
            // relative to inputPoints[].
            std::vector<Vector2<InputType>> points;
            std::vector<int> remapping;
            RemapPolygonTree(numInputPoints, inputPoints, tree, points, remapping);
            LogAssert(points.size() >= 3, "Invalid polygon tree.");

            ETManifoldMesh graph;
            std::set<EdgeKey<false>> edges;
            ConstrainedTriangulate(tree, points, graph, edges);
            ClassifyTriangles(tree, graph, edges);

            RestorePolygonTree(tree, remapping);
        }

        // On return, 'points' are the unique inputPoints[] values referenced by
        // the tree. The tree 'polygon' members are modified to be indices
        // into 'points' rather than inputPoints[]. The 'remapping' allows us to
        // restore the tree 'polygon' members to be indices into inputPoints[]
        // after the triangulation is computed. The 'edges' stores all the
        // polygon edges that must be in the triangulation.
        void RemapPolygonTree(
            int numInputPoints,
            Vector2<InputType> const* inputPoints,
            PolygonTreeEx& tree,
            std::vector<Vector2<InputType>>& points,
            std::vector<int>& remapping)
        {
            std::map<Vector2<InputType>, int> pointMap;
            points.reserve(numInputPoints);
            int currentIndex = 0;

            // The remapping is initially the identity, remapping[j] = j.
            remapping.resize(numInputPoints);
            std::iota(remapping.begin(), remapping.end(), 0);

            std::queue<size_t> queue;
            queue.push(0);
            while (queue.size() > 0)
            {
                PolygonTreeEx::Node& node = tree.nodes[queue.front()];
                queue.pop();
                size_t const numIndices = node.polygon.size();
                for (size_t i = 0; i < numIndices; ++i)
                {
                    auto const& point = inputPoints[node.polygon[i]];
                    auto iter = pointMap.find(point);
                    if (iter == pointMap.end())
                    {
                        // The point is encountered the first time.
                        pointMap.insert(std::make_pair(point, currentIndex));
                        remapping[currentIndex] = node.polygon[i];
                        node.polygon[i] = currentIndex;
                        points.push_back(point);
                        ++currentIndex;
                    }
                    else
                    {
                        // The point is a duplicate. On the remapping, the
                        // polygon[] value is set to the index for the
                        // first occurrence of the duplicate.
                        remapping[iter->second] = node.polygon[i];
                        node.polygon[i] = iter->second;
                    }

                }

                for (size_t c = node.minChild; c < node.supChild; ++c)
                {
                    queue.push(c);
                }
            }
        }

        void RestorePolygonTree(PolygonTreeEx& tree, std::vector<int> const& remapping)
        {
            std::queue<size_t> queue;
            queue.push(0);
            while (queue.size() > 0)
            {
                auto& node = tree.nodes[queue.front()];
                queue.pop();

                for (auto& index : node.polygon)
                {
                    index = remapping[index];
                }
                for (auto& tri : node.triangulation)
                {
                    for (size_t j = 0; j < 3; ++j)
                    {
                        tri[j] = remapping[tri[j]];
                    }
                }

                for (size_t c = node.minChild; c < node.supChild; ++c)
                {
                    queue.push(c);
                }
            }

            for (auto& tri : tree.interiorTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.exteriorTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.insideTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.outsideTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.allTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }
        }

        void ConstrainedTriangulate(
            PolygonTreeEx& tree,
            std::vector<Vector2<InputType>> const& points,
            ETManifoldMesh& graph,
            std::set<EdgeKey<false>>& edges)
        {
            // Use constrained Delaunay triangulation.
            ConstrainedDelaunay2<InputType, ComputeType> cdt;
            int const numPoints = static_cast<int>(points.size());
            cdt(numPoints, points.data(), static_cast<InputType>(0));
            std::vector<int> outEdge;

            std::queue<size_t> queue;
            queue.push(0);
            while (queue.size() > 0)
            {
                auto& node = tree.nodes[queue.front()];
                queue.pop();

                std::vector<int> replacement;
                size_t numIndices = node.polygon.size();
                for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
                {
                    // Insert the polygon edge into the constrained Delaunay
                    // triangulation.
                    std::array<int, 2> edge = { node.polygon[i0], node.polygon[i1] };
                    outEdge.clear();
                    (void)cdt.Insert(edge, outEdge);
                    if (outEdge.size() > 2)
                    {
                        // The polygon edge intersects additional vertices in
                        // the triangulation. The outEdge[] edge values are
                        // { edge[0], other_vertices, edge[1] } which are
                        // ordered along the line segment.
                        replacement.insert(replacement.end(), outEdge.begin() + 1, outEdge.end());
                    }
                    else
                    {
                        replacement.push_back(node.polygon[i1]);
                    }
                }
                if (replacement.size() > node.polygon.size())
                {
                    node.polygon = std::move(replacement);
                }

                numIndices = node.polygon.size();
                for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
                {
                    edges.insert(EdgeKey<false>(node.polygon[i0], node.polygon[i1]));
                }

                for (size_t c = node.minChild; c < node.supChild; ++c)
                {
                    queue.push(c);
                }
            }

            // Copy the graph to the compact arrays mIndices and
            // mAdjacencies for use by the caller.
            cdt.UpdateIndicesAdjacencies();

            // Duplicate the graph, which will be modified during triangle
            // extration. The original is kept intact for use by the caller.
            graph = cdt.GetGraph();

            // Store the triangles in allTriangles for potential use by the
            // caller.
            int const numTriangles = cdt.GetNumTriangles();
            int const* indices = cdt.GetIndices().data();
            tree.allTriangles.resize(numTriangles);
            for (int t = 0; t < numTriangles; ++t)
            {
                int v0 = *indices++;
                int v1 = *indices++;
                int v2 = *indices++;
                graph.Insert(v0, v1, v2);
                tree.allTriangles[t] = { v0, v1, v2 };
            }
        }

        void ClassifyTriangles(PolygonTreeEx& tree, ETManifoldMesh& graph,
            std::set<EdgeKey<false>>& edges)
        {
            ClassifyDFS(tree, 0, graph, edges);
            LogAssert(edges.size() == 0, "The edges should be empty for a correct implementation.");
            GetOutsideTriangles(tree, graph);
            GetInsideTriangles(tree);
        }

        void ClassifyDFS(PolygonTreeEx& tree, size_t index, ETManifoldMesh& graph,
            std::set<EdgeKey<false>>& edges)
        {
            auto& node = tree.nodes[index];
            for (size_t c = node.minChild; c < node.supChild; ++c)
            {
                ClassifyDFS(tree, c, graph, edges);
            }

            auto const& emap = graph.GetEdges();
            std::set<TriangleKey<true>> region;
            size_t const numIndices = node.polygon.size();
            for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
            {
                int v0 = node.polygon[i0];
                int v1 = node.polygon[i1];
                EdgeKey<false> ekey(v0, v1);
                auto eiter = emap.find(ekey);
                LogAssert(eiter != emap.end(), "Unexpected condition.");
                auto const& edge = eiter->second;
                LogAssert(edge, "Unexpected condition.");
                auto tri0 = edge->T[0];
                LogAssert(tri0, "Unexpected condition.");
                if (tri0->WhichSideOfEdge(v0, v1) == node.chirality)
                {
                    region.insert(TriangleKey<true>(tri0->V[0], tri0->V[1], tri0->V[2]));
                }
                else
                {
                    auto tri1 = edge->T[1];
                    if (tri1)
                    {
                        region.insert(TriangleKey<true>(tri1->V[0], tri1->V[1], tri1->V[2]));
                    }
                }
            }

            FillRegion(graph, edges, region);
            ExtractTriangles(graph, region, node);
            for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
            {
                edges.erase(EdgeKey<false>(node.polygon[i0], node.polygon[i1]));
            }
        }

        // On input, the set has the initial seeds for the desired region. A
        // breadth-first search is performed to find the connected component
        // of the seeds. The component is bounded by an outer polygon and the
        // inner polygons of its children.
        void FillRegion(ETManifoldMesh& graph, std::set<EdgeKey<false>> const& edges,
            std::set<TriangleKey<true>>& region)
        {
            std::queue<TriangleKey<true>> regionQueue;
            for (auto const& tkey : region)
            {
                regionQueue.push(tkey);
            }

            auto const& tmap = graph.GetTriangles();
            while (regionQueue.size() > 0)
            {
                auto const& tkey = regionQueue.front();
                regionQueue.pop();
                auto titer = tmap.find(tkey);
                LogAssert(titer != tmap.end(), "Unexpected condition.");
                auto const& tri = titer->second;
                LogAssert(tri, "Unexpected condition.");
                for (size_t j = 0; j < 3; ++j)
                {
                    auto edge = tri->E[j];
                    if (edge)
                    {
                        auto siter = edges.find(EdgeKey<false>(edge->V[0], edge->V[1]));
                        if (siter == edges.end())
                        {
                            // The edge is not constrained, so it allows the
                            // search to continue.
                            auto adj = tri->T[j];
                            if (adj)
                            {
                                TriangleKey<true> akey(adj->V[0], adj->V[1], adj->V[2]);
                                auto riter = region.find(akey);
                                if (riter == region.end())
                                {
                                    // The adjacent triangle has not yet been
                                    // visited, so place it in the queue to
                                    // continue the search.
                                    region.insert(akey);
                                    regionQueue.push(akey);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Store the region triangles in a triangulation object and remove
        // those triangles from the graph in preparation for processing the
        // next layer of triangles.
        void ExtractTriangles(ETManifoldMesh& graph,
            std::set<TriangleKey<true>> const& region,
            PolygonTreeEx::Node& node)
        {
            node.triangulation.reserve(region.size());
            if (node.chirality > 0)
            {
                for (auto const& tri : region)
                {
                    node.triangulation.push_back({ tri.V[0], tri.V[1], tri.V[2] });
                    graph.Remove(tri.V[0], tri.V[1], tri.V[2]);
                }
            }
            else  // node.chirality < 0
            {
                for (auto const& tri : region)
                {
                    node.triangulation.push_back({ tri.V[0], tri.V[2], tri.V[1] });
                    graph.Remove(tri.V[0], tri.V[1], tri.V[2]);
                }
            }
        }

        void GetOutsideTriangles(PolygonTreeEx& tree, ETManifoldMesh& graph)
        {
            auto const& tmap = graph.GetTriangles();
            tree.outsideTriangles.resize(tmap.size());
            size_t t = 0;
            for (auto const& tri : tmap)
            {
                auto const& tkey = tri.first;
                tree.outsideTriangles[t++] = { tkey.V[0], tkey.V[1], tkey.V[2] };
            }
            graph.Clear();
        }

        // Get the triangles in the polygon tree, classifying each as
        // interior (in region bounded by outer polygon and its contained
        // inner polygons) or exterior (in region bounded by inner polygon
        // and its contained outer polygons). The inside triangles are the
        // union of the interior and exterior triangles.
        void GetInsideTriangles(PolygonTreeEx& tree)
        {
            size_t const numTriangles = tree.allTriangles.size();
            size_t const numOutside = tree.outsideTriangles.size();
            size_t const numInside = numTriangles - numOutside;
            tree.interiorTriangles.reserve(numTriangles);
            tree.interiorNodeIndices.reserve(numTriangles);
            tree.exteriorTriangles.reserve(numTriangles);
            tree.exteriorNodeIndices.reserve(numTriangles);
            tree.insideTriangles.reserve(numInside);
            tree.insideNodeIndices.reserve(numInside);

            for (size_t nIndex = 0; nIndex < tree.nodes.size(); ++nIndex)
            {
                auto const& node = tree.nodes[nIndex];
                for (auto& tri : node.triangulation)
                {
                    if (node.chirality > 0)
                    {
                        tree.interiorTriangles.push_back(tri);
                        tree.interiorNodeIndices.push_back(nIndex);
                    }
                    else
                    {
                        tree.exteriorTriangles.push_back(tri);
                        tree.exteriorNodeIndices.push_back(nIndex);
                    }

                    tree.insideTriangles.push_back(tri);
                    tree.insideNodeIndices.push_back(nIndex);
                }
            }
        }
    };
}

namespace gte
{
    // The input type must be 'float' or 'double'. The user no longer has
    // the responsibility to specify the compute type.

    template <typename T>
    class TriangulateCDT<T>
    {
    public:
        void operator()(
            std::vector<Vector2<T>> const& inputPoints,
            std::shared_ptr<PolygonTree> const& inputTree,
            PolygonTreeEx& outputTree)
        {
            LogAssert(inputPoints.size() >= 3 && inputTree, "Invalid argument.");

            CopyAndCompactify(inputTree, outputTree);
            Triangulate(inputPoints.size(), inputPoints.data(), outputTree);
        }

        void operator()(
            size_t numInputPoints,
            Vector2<T> const* inputPoints,
            std::shared_ptr<PolygonTree> const& inputTree,
            PolygonTreeEx& outputTree)
        {
            LogAssert(numInputPoints >= 3 && inputPoints != nullptr && inputTree, "Invalid argument.");

            CopyAndCompactify(inputTree, outputTree);
            Triangulate(numInputPoints, inputPoints, outputTree);
        }

    private:
        void CopyAndCompactify(std::shared_ptr<PolygonTree> const& input,
            PolygonTreeEx& output)
        {
            output.nodes.clear();
            output.interiorTriangles.clear();
            output.interiorNodeIndices.clear();
            output.exteriorTriangles.clear();
            output.exteriorNodeIndices.clear();
            output.insideTriangles.clear();
            output.insideNodeIndices.clear();
            output.outsideTriangles.clear();
            output.allTriangles.clear();

            // Count the number of nodes in the tree.
            size_t numNodes = 1;  // the root node
            std::queue<std::shared_ptr<PolygonTree>> queue;
            queue.push(input);
            while (queue.size() > 0)
            {
                auto const& node = queue.front();
                queue.pop();
                numNodes += node->child.size();
                for (auto const& child : node->child)
                {
                    queue.push(child);
                }
            }

            // Create the PolygonTreeEx nodes.
            output.nodes.resize(numNodes);
            for (size_t i = 0; i < numNodes; ++i)
            {
                output.nodes[i].self = i;
            }
            output.nodes[0].chirality = +1;
            output.nodes[0].parent = std::numeric_limits<size_t>::max();

            size_t current = 0, last = 0, minChild = 1;
            queue.push(input);
            while (queue.size() > 0)
            {
                auto const& node = queue.front();
                queue.pop();
                auto& exnode = output.nodes[current++];
                exnode.polygon = node->polygon;
                exnode.minChild = minChild;
                exnode.supChild = minChild + node->child.size();
                minChild = exnode.supChild;
                for (auto const& child : node->child)
                {
                    auto& exchild = output.nodes[++last];
                    exchild.chirality = -exnode.chirality;
                    exchild.parent = exnode.self;
                    queue.push(child);
                }
            }
        }

        void Triangulate(size_t numInputPoints, Vector2<T> const* inputPoints,
            PolygonTreeEx& tree)
        {
            // The constrained Delaunay triangulator will be given the unique
            // points referenced by the polygons in the tree. The tree
            // 'polygon' indices are relative to inputPoints[], but they are
            // temporarily mapped to indices relative to 'points'. Once the
            // triangulation is complete, the indices are restored to those
            // relative to inputPoints[].
            std::vector<Vector2<T>> points;
            std::vector<int32_t> remapping;
            RemapPolygonTree(numInputPoints, inputPoints, tree, points, remapping);
            LogAssert(points.size() >= 3, "Invalid polygon tree.");

            ETManifoldMesh graph;
            std::set<EdgeKey<false>> edges;
            ConstrainedTriangulate(tree, points, graph, edges);
            ClassifyTriangles(tree, graph, edges);

            RestorePolygonTree(tree, remapping);
        }

        // On return, 'points' are the unique inputPoints[] values referenced by
        // the tree. The tree 'polygon' members are modified to be indices
        // into 'points' rather than inputPoints[]. The 'remapping' allows us to
        // restore the tree 'polygon' members to be indices into inputPoints[]
        // after the triangulation is computed. The 'edges' stores all the
        // polygon edges that must be in the triangulation.
        void RemapPolygonTree(
            size_t numInputPoints,
            Vector2<T> const* inputPoints,
            PolygonTreeEx& tree,
            std::vector<Vector2<T>>& points,
            std::vector<int32_t>& remapping)
        {
            std::map<Vector2<T>, int32_t> pointMap;
            points.reserve(numInputPoints);
            int32_t currentIndex = 0;

            // The remapping is initially the identity, remapping[j] = j.
            remapping.resize(numInputPoints);
            std::iota(remapping.begin(), remapping.end(), 0);

            std::queue<size_t> queue;
            queue.push(0);
            while (queue.size() > 0)
            {
                PolygonTreeEx::Node& node = tree.nodes[queue.front()];
                queue.pop();
                size_t const numIndices = node.polygon.size();
                for (size_t i = 0; i < numIndices; ++i)
                {
                    auto const& point = inputPoints[node.polygon[i]];
                    auto iter = pointMap.find(point);
                    if (iter == pointMap.end())
                    {
                        // The point is encountered the first time.
                        pointMap.insert(std::make_pair(point, currentIndex));
                        remapping[currentIndex] = node.polygon[i];
                        node.polygon[i] = currentIndex;
                        points.push_back(point);
                        ++currentIndex;
                    }
                    else
                    {
                        // The point is a duplicate. On the remapping, the
                        // polygon[] value is set to the index for the
                        // first occurrence of the duplicate.
                        remapping[iter->second] = node.polygon[i];
                        node.polygon[i] = iter->second;
                    }

                }

                for (size_t c = node.minChild; c < node.supChild; ++c)
                {
                    queue.push(c);
                }
            }
        }

        void RestorePolygonTree(PolygonTreeEx& tree, std::vector<int32_t> const& remapping)
        {
            std::queue<size_t> queue;
            queue.push(0);
            while (queue.size() > 0)
            {
                auto& node = tree.nodes[queue.front()];
                queue.pop();

                for (auto& index : node.polygon)
                {
                    index = remapping[index];
                }
                for (auto& tri : node.triangulation)
                {
                    for (size_t j = 0; j < 3; ++j)
                    {
                        tri[j] = remapping[tri[j]];
                    }
                }

                for (size_t c = node.minChild; c < node.supChild; ++c)
                {
                    queue.push(c);
                }
            }

            for (auto& tri : tree.interiorTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.exteriorTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.insideTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.outsideTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }

            for (auto& tri : tree.allTriangles)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = remapping[tri[j]];
                }
            }
        }

        void ConstrainedTriangulate(
            PolygonTreeEx& tree,
            std::vector<Vector2<T>> const& points,
            ETManifoldMesh& graph,
            std::set<EdgeKey<false>>& edges)
        {
            // Use constrained Delaunay triangulation.
            ConstrainedDelaunay2<T> cdt;
            cdt(points);

            size_t counter = 0;

            std::vector<int32_t> outEdge;
            std::queue<size_t> queue;
            queue.push(0);
            while (queue.size() > 0)
            {
                auto& node = tree.nodes[queue.front()];
                queue.pop();

                std::vector<int32_t> replacement;
                size_t numIndices = node.polygon.size();
                for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
                {
                    // Insert the polygon edge into the constrained Delaunay
                    // triangulation.
                    std::array<int32_t, 2> edge = { node.polygon[i0], node.polygon[i1] };
                    outEdge.clear();
                    (void)cdt.Insert(edge, outEdge);
                    ++counter;
                    if (outEdge.size() > 2)
                    {
                        // The polygon edge intersects additional vertices in
                        // the triangulation. The outEdge[] edge values are
                        // { edge[0], other_vertices, edge[1] } which are
                        // ordered along the line segment.
                        replacement.insert(replacement.end(), outEdge.begin() + 1, outEdge.end());
                    }
                    else
                    {
                        replacement.push_back(node.polygon[i1]);
                    }
                }
                if (replacement.size() > node.polygon.size())
                {
                    node.polygon = std::move(replacement);
                }

                numIndices = node.polygon.size();
                for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
                {
                    edges.insert(EdgeKey<false>(node.polygon[i0], node.polygon[i1]));
                }

                for (size_t c = node.minChild; c < node.supChild; ++c)
                {
                    queue.push(c);
                }
            }

            // Copy the graph to the compact arrays mIndices and
            // mAdjacencies for use by the caller.
            cdt.UpdateIndicesAdjacencies();

            // Duplicate the graph, which will be modified during triangle
            // extration. The original is kept intact for use by the caller.
            graph = cdt.GetGraph();

            // Store the triangles in allTriangles for potential use by the
            // caller.
            size_t const numTriangles = cdt.GetNumTriangles();
            int32_t const* indices = cdt.GetIndices().data();
            tree.allTriangles.resize(numTriangles);
            for (size_t t = 0; t < numTriangles; ++t)
            {
                int32_t v0 = *indices++;
                int32_t v1 = *indices++;
                int32_t v2 = *indices++;
                graph.Insert(v0, v1, v2);
                tree.allTriangles[t] = { v0, v1, v2 };
            }
        }

        void ClassifyTriangles(PolygonTreeEx& tree, ETManifoldMesh& graph,
            std::set<EdgeKey<false>>& edges)
        {
            ClassifyDFS(tree, 0, graph, edges);
            LogAssert(edges.size() == 0, "The edges should be empty for a correct implementation.");
            GetOutsideTriangles(tree, graph);
            GetInsideTriangles(tree);
        }

        void ClassifyDFS(PolygonTreeEx& tree, size_t index, ETManifoldMesh& graph,
            std::set<EdgeKey<false>>& edges)
        {
            auto& node = tree.nodes[index];
            for (size_t c = node.minChild; c < node.supChild; ++c)
            {
                ClassifyDFS(tree, c, graph, edges);
            }

            auto const& emap = graph.GetEdges();
            std::set<TriangleKey<true>> region;
            size_t const numIndices = node.polygon.size();
            for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
            {
                int32_t v0 = node.polygon[i0];
                int32_t v1 = node.polygon[i1];
                EdgeKey<false> ekey(v0, v1);
                auto eiter = emap.find(ekey);
                LogAssert(eiter != emap.end(), "Unexpected condition.");
                auto const& edge = eiter->second;
                LogAssert(edge, "Unexpected condition.");
                auto tri0 = edge->T[0];
                LogAssert(tri0, "Unexpected condition.");
                if (tri0->WhichSideOfEdge(v0, v1) == node.chirality)
                {
                    region.insert(TriangleKey<true>(tri0->V[0], tri0->V[1], tri0->V[2]));
                }
                else
                {
                    auto tri1 = edge->T[1];
                    if (tri1)
                    {
                        region.insert(TriangleKey<true>(tri1->V[0], tri1->V[1], tri1->V[2]));
                    }
                }
            }

            FillRegion(graph, edges, region);
            ExtractTriangles(graph, region, node);
            for (size_t i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
            {
                edges.erase(EdgeKey<false>(node.polygon[i0], node.polygon[i1]));
            }
        }

        // On input, the set has the initial seeds for the desired region. A
        // breadth-first search is performed to find the connected component
        // of the seeds. The component is bounded by an outer polygon and the
        // inner polygons of its children.
        void FillRegion(ETManifoldMesh& graph, std::set<EdgeKey<false>> const& edges,
            std::set<TriangleKey<true>>& region)
        {
            std::queue<TriangleKey<true>> regionQueue;
            for (auto const& tkey : region)
            {
                regionQueue.push(tkey);
            }

            auto const& tmap = graph.GetTriangles();
            while (regionQueue.size() > 0)
            {
                auto const& tkey = regionQueue.front();
                regionQueue.pop();
                auto titer = tmap.find(tkey);
                LogAssert(titer != tmap.end(), "Unexpected condition.");
                auto const& tri = titer->second;
                LogAssert(tri, "Unexpected condition.");
                for (size_t j = 0; j < 3; ++j)
                {
                    auto edge = tri->E[j];
                    if (edge)
                    {
                        auto siter = edges.find(EdgeKey<false>(edge->V[0], edge->V[1]));
                        if (siter == edges.end())
                        {
                            // The edge is not constrained, so it allows the
                            // search to continue.
                            auto adj = tri->T[j];
                            if (adj)
                            {
                                TriangleKey<true> akey(adj->V[0], adj->V[1], adj->V[2]);
                                auto riter = region.find(akey);
                                if (riter == region.end())
                                {
                                    // The adjacent triangle has not yet been
                                    // visited, so place it in the queue to
                                    // continue the search.
                                    region.insert(akey);
                                    regionQueue.push(akey);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Store the region triangles in a triangulation object and remove
        // those triangles from the graph in preparation for processing the
        // next layer of triangles.
        void ExtractTriangles(ETManifoldMesh& graph,
            std::set<TriangleKey<true>> const& region,
            PolygonTreeEx::Node& node)
        {
            node.triangulation.reserve(region.size());
            if (node.chirality > 0)
            {
                for (auto const& tri : region)
                {
                    node.triangulation.push_back({ tri.V[0], tri.V[1], tri.V[2] });
                    graph.Remove(tri.V[0], tri.V[1], tri.V[2]);
                }
            }
            else  // node.chirality < 0
            {
                for (auto const& tri : region)
                {
                    node.triangulation.push_back({ tri.V[0], tri.V[2], tri.V[1] });
                    graph.Remove(tri.V[0], tri.V[1], tri.V[2]);
                }
            }
        }

        void GetOutsideTriangles(PolygonTreeEx& tree, ETManifoldMesh& graph)
        {
            auto const& tmap = graph.GetTriangles();
            tree.outsideTriangles.resize(tmap.size());
            size_t t = 0;
            for (auto const& tri : tmap)
            {
                auto const& tkey = tri.first;
                tree.outsideTriangles[t++] = { tkey.V[0], tkey.V[1], tkey.V[2] };
            }
            graph.Clear();
        }

        // Get the triangles in the polygon tree, classifying each as
        // interior (in region bounded by outer polygon and its contained
        // inner polygons) or exterior (in region bounded by inner polygon
        // and its contained outer polygons). The inside triangles are the
        // union of the interior and exterior triangles.
        void GetInsideTriangles(PolygonTreeEx& tree)
        {
            size_t const numTriangles = tree.allTriangles.size();
            size_t const numOutside = tree.outsideTriangles.size();
            size_t const numInside = numTriangles - numOutside;
            tree.interiorTriangles.reserve(numTriangles);
            tree.interiorNodeIndices.reserve(numTriangles);
            tree.exteriorTriangles.reserve(numTriangles);
            tree.exteriorNodeIndices.reserve(numTriangles);
            tree.insideTriangles.reserve(numInside);
            tree.insideNodeIndices.reserve(numInside);

            for (size_t nIndex = 0; nIndex < tree.nodes.size(); ++nIndex)
            {
                auto const& node = tree.nodes[nIndex];
                for (auto& tri : node.triangulation)
                {
                    if (node.chirality > 0)
                    {
                        tree.interiorTriangles.push_back(tri);
                        tree.interiorNodeIndices.push_back(nIndex);
                    }
                    else
                    {
                        tree.exteriorTriangles.push_back(tri);
                        tree.exteriorNodeIndices.push_back(nIndex);
                    }

                    tree.insideTriangles.push_back(tri);
                    tree.insideNodeIndices.push_back(nIndex);
                }
            }
        }
    };
}
