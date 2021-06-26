// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.3.2020.11.06

#pragma once

#include <Mathematics/Vector2.h>
#include <cstdint>
#include <limits>
#include <memory>
#include <stack>
#include <vector>

namespace gte
{
    // These classes are used by class TriangulateEC (triangulation based on
    // ear clipping) and class TriangulateCDT (triangulation based on
    // Constrained Delaunay triangulation). The PolygonTree class used to be
    // the nested class Tree in those classes, but it has been factored out to
    // allow applications to use either triangulator without having to
    // duplicate the trees.
    //
    // NOTE: The polygon member does not duplicate endpoints. For example,
    // if P[] are the point locations and the polygon is a triangle with
    // counterclockwise ordering, <P[i0],P[i1],P[i2]>, then
    // polygon = {i0,i1,i2}. The implication is that there are 3 directed
    // edges: {P[i0],P[i1]}, {P[i1],P[i2]} and {P[i2],P[i0].
    //
    // Eventually, the PolygonTreeEx struct will replace PolygonTree because
    //   1. The algorithms can be rewritten not to depend on the alternating
    //      winding order between parent and child.
    //   2. The triangulation is explicitly stored in the tree nodes and can
    //      support point-in-polygon tree queries (In the tree? Which polygon
    //      contains the point?).
    //   3. The polygon trees can be built not to use std::shared_ptr, making
    //      the trees more compact by using std::vector<PolygonTree> vpt. The
    //      ordering of the tree nodes must be that implied by a breadth-first
    //      search.

    // A tree of nested polygons. The root node corresponds to an outer
    // polygon. The children of the root correspond to inner polygons,
    // which polygons strictly contained in the outer polygon. Each inner
    // polygon may itself contain an outer polygon which in turn can
    // contain inner polygons, thus leading to a hierarchy of polygons.
    // The outer polygons have vertices listed in counterclockwise order.
    // The inner polygons have vertices listed in clockwise order.
    class PolygonTree
    {
    public:
        PolygonTree()
            :
            polygon{},
            child{}
        {
        }

        std::vector<int> polygon;
        std::vector<std::shared_ptr<PolygonTree>> child;
    };

    // A tree of nested polygons with extra information about the polygon.
    // The tree can be stored as: std::vector<PolygonTree> tree(numNodes).
    // The point locations are specified separately to the triangulators.
    //
    // The chirality (winding ordering of the polygon) is set to +1 for a
    // counterclockwise-ordered polygon or -1 for a clockwise-oriented
    // polygon.
    //
    // The triangulation is computed by the triangulators and explicitly
    // stored per tree node.
    //
    // The element node[0] is the root of the tree with node[0].parent = -1.
    // If node[0] has C children, then node[0].minChild = 1 and
    // node[0].supChild = 1 + C. Generally, node[i] is a node with parent
    // node[p], where p = node[i].parent, and children node[c], where
    // node[i].minChild <= c < node[i].supChild. If node[i].minChild >=
    // node[i].supChild, the node has no children.
    class PolygonTreeEx
    {
    public:
        class Node
        {
        public:
            Node()
                :
                polygon{},
                chirality(0),
                triangulation{},
                self(0),
                parent(0),
                minChild(0),
                supChild(0)
            {
            }

            std::vector<int> polygon;
            int64_t chirality;
            std::vector<std::array<int, 3>> triangulation;
            size_t self, parent, minChild, supChild;
        };


        // The nodes of the polygon tree, organized based on a breadth-first
        // traversal of the tree.
        std::vector<Node> nodes;

        // These members support TriangulateCDT. The *NodeIndices members
        // store the indices into 'nodes[]' for the triangles in the
        // *Triangles members. For example, the triangle interiorTriangles[t]
        // comes from nodes[interiorNodes[t]].

        // The triangles in the polygon tree that cover each region bounded
        // by an outer polygon and its contained inner polygons. This set
        // is the equivalent of the output of TriangulateEC that uses ear
        // clipping.
        std::vector<std::array<int, 3>> interiorTriangles;
        std::vector<size_t> interiorNodeIndices;

        // The triangles in the polygon tree that cover each region bounded
        // by an inner polygon and its contained outer polygons.
        std::vector<std::array<int, 3>> exteriorTriangles;
        std::vector<size_t> exteriorNodeIndices;

        // The triangles inside the polygon tree.
        //   insideTriangles = interiorTriangle + exteriorTriangles
        std::vector<std::array<int, 3>> insideTriangles;
        std::vector<size_t> insideNodeIndices;

        // The triangles inside the convex hull of the Delaunay triangles but
        // outside the polygon tree. These triangles are not associated with
        // any 'nodes[]' element.
        std::vector<std::array<int, 3>> outsideTriangles;

        // All the triangles:
        //   allTriangles = insideTriangles + outsideTriangles.
        std::vector<std::array<int, 3>> allTriangles;

    public:
        // Point-containment queries.

        // Search the polygon tree for the triangle that contains 'test'. If
        // there is such a triangle, the returned pair (nIndex,tIndex) states
        // that the triangle is nodes[nIndex].triangulation[tIndex]. If there
        // is no such triangle, the returned pair is (smax,smax) where
        // smax = std::numeric_limits<size_t>::max(). The function is
        // naturally recursive, but simulated recursion is used to avoid a
        // large program stack by instead using the heap. A typical call is
        //   PolygonTreeEx tree = <some tree>;
        //   std::vector<Vector2<T>> points = <some vector of points>;
        //   Vector2<T> test = <some point>;
        //   std::pair<size_t, size_t> result;
        //   result = tree.GetContainingTriangle(test, points);
        template <typename T>
        std::pair<size_t, size_t> GetContainingTriangle(Vector2<T> const& test,
            Vector2<T> const* points)
        {
            size_t constexpr smax = std::numeric_limits<size_t>::max();
            std::pair<size_t, size_t> result = std::make_pair(smax, smax);

            std::stack<size_t> stack;
            stack.push(0);
            while (stack.size() > 0 && result.first == smax)
            {
                auto nIndex = stack.top();
                stack.pop();
                auto const& node = nodes[nIndex];
                for (size_t c = node.minChild; c < node.supChild; ++c)
                {
                    stack.push(c);
                }

                for (size_t tIndex = 0; tIndex < node.triangulation.size(); ++tIndex)
                {
                    if (PointInTriangle(test, node.chirality, node.triangulation[tIndex], points))
                    {
                        result = std::make_pair(nIndex, tIndex);
                        break;
                    }
                }
            }

            return result;
        }

        // Search the triangles for the triangle that contains 'test'. If
        // there is such a triangle, the returned pair (nIndex,tIndex) states
        // that the triangle is nodes[nIndex].triangulation[tIndex]. If there
        // is no such triangle, the returned pair is (smax,smax) where
        // smax = std::numeric_limits<size_t>::max(). The function uses a
        // linear search of the input triangles. Some typical calls are
        //   PolygonTreeEx tree = <some tree>;
        //   std::vector<Vector2<T>> points = <some vector of points>;
        //   Vector2<T> test = <some point>;
        //   std::pair<size_t, size_t> result;
        //   result = tree.GetContainingTriangle(test, tree.insideTriangles,
        //       tree.insideNodeIndices, points);
        //   result = tree.GetContainingTriangle(test, tree.interiorTriangles,
        //       tree.interiorNodeIndices, points);
        //   result = tree.GetContainingTriangle(test, tree.exteriorTriangles,
        //       tree.exteriorIndices, points);
        template <typename T>
        std::pair<size_t, size_t> GetContainingTriangle(Vector2<T> const& test,
            std::vector<std::array<int, 3>> const& triangles,
            std::vector<size_t> const& nodeIndices,
            Vector2<T> const* points)
        {
            LogAssert(triangles.size() == nodeIndices.size(), "Invalid argument.");

            size_t constexpr smax = std::numeric_limits<size_t>::max();
            std::pair<size_t, size_t> result = std::make_pair(smax, smax);
            for (size_t tIndex = 0; tIndex < triangles.size(); ++tIndex)
            {
                size_t const nIndex = nodeIndices[tIndex];
                auto const& node = nodes[nIndex];
                if (PointInTriangle(test, node.chirality, triangles[tIndex], points))
                {
                    result = std::make_pair(nIndex, tIndex);
                    break;
                }
            }
            return result;
        }

        // Search the triangles for the triangle that contains 'test'. If
        // there is such a triangle, the returned t-value is in the range
        // 0 <= t < triangles.size(); otherwise, smax is returned where
        // smax = std::numeric_limits<size_t>::max(). The function uses a
        // linear search of the input triangles. No information is available
        // about the 'nodes[]' element corresponding to the containing
        // triangle of the test point. Typical calls are
        //   PolygonTreeEx tree = <some tree>;
        //   std::vector<Vector2<T>> points = <some vector of points>;
        //   Vector2<T> test = <some point>;
        //   size_t resultInt, resultExt, resultOut;
        //   resultInt = PolygonTreeEx::GetContainingTriangle(test,
        //       tree.interiorTriangles, +1, points);
        //   resultExt = PolygonTreeEx::GetContainingTriangle(test,
        //       tree.exteriorTriangles, -1, points);
        //   resultOut = PolygonTreeEx::GetContainingTriangle(test,
        //       tree.outsideTriangles, +1, points);
        template <typename T>
        size_t GetContainingTriangle(Vector2<T> const& test,
            std::vector<std::array<int, 3>> const& triangles, int64_t chirality,
            Vector2<T> const* points)
        {
            size_t result = std::numeric_limits<size_t>::max();
            for (size_t tIndex = 0; tIndex < triangles.size(); ++tIndex)
            {
                if (PointInTriangle(test, chirality, triangles[tIndex], points))
                {
                    result = tIndex;
                    break;
                }
            }
            return result;
        }

    private:
        // Determine whether 'test' is inside the triangle whose vertices
        // are points[triangle[0]], points[triangle[1]], points[triangle[2].
        // If the points are counterclockwise ordered, set 'chirality' to +1.
        // If the points are clockwise ordered, set 'chirality' to -1.
        template <typename T>
        static bool PointInTriangle(Vector2<T> const& test, int64_t chirality,
            std::array<int, 3> const& triangle, Vector2<T> const* points)
        {
            T const zero = static_cast<T>(0);
            T const sign = static_cast<T>(chirality);
            for (int i1 = 0, i0 = 2; i1 < 3; i0 = i1++)
            {
                T nx = points[triangle[i1]][1] - points[triangle[i0]][1];
                T ny = points[triangle[i0]][0] - points[triangle[i1]][0];
                T dx = test[0] - points[triangle[i0]][0];
                T dy = test[1] - points[triangle[i0]][1];
                T sdot = sign * (nx * dx + ny * dy);
                if (sdot > zero)
                {
                    return false;
                }
            }
            return true;
        }
    };
}
