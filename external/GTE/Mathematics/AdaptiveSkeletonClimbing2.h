// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <vector>

// Extract level surfaces using an adaptive approach to reduce the triangle
// count.  The implementation is for the algorithm described in the paper
//   Multiresolution Isosurface Extraction with Adaptive Skeleton Climbing
//   Tim Poston, Tien-Tsin Wong and Pheng-Ann Heng
//   Computer Graphics forum, volume 17, issue 3, September 1998
//   pages 137-147
// https://onlinelibrary.wiley.com/doi/abs/10.1111/1467-8659.00261

namespace gte
{
    // The image type T must be one of the integer types:  int8_t, int16_t,
    // int32_t, uint8_t, uint16_t or uint32_t.  Internal integer computations
    // are performed using int64_t.  The type Real is for extraction to
    // floating-point vertices.
    template <typename T, typename Real>
    class AdaptiveSkeletonClimbing2
    {
    public:
        // Construction and destruction.  The input image is assumed to
        // contain (2^N+1)-by-(2^N+1) elements where N >= 0.  The organization
        // is row-major order for (x,y).
        AdaptiveSkeletonClimbing2(int N, T const* inputPixels)
            :
            mTwoPowerN(1 << N),
            mSize(mTwoPowerN + 1),
            mInputPixels(inputPixels),
            mXMerge(mSize),
            mYMerge(mSize)
        {
            static_assert(std::is_integral<T>::value && sizeof(T) <= 4,
                "Type T must be int{8,16,32}_t or uint{8,16,32}_t.");
            if (N <= 0 || mInputPixels == nullptr)
            {
                LogError("Invalid input.");
            }

            for (int i = 0; i < mSize; ++i)
            {
                mXMerge[i] = std::make_shared<LinearMergeTree>(N);
                mYMerge[i] = std::make_shared<LinearMergeTree>(N);
            }

            mXYMerge = std::make_unique<AreaMergeTree>(N, mXMerge, mYMerge);
        }

        // TODO: Refactor this class to have base class CurveExtractor.
        typedef std::array<Real, 2> Vertex;
        typedef std::array<int, 2> Edge;

        void Extract(Real level, int depth,
            std::vector<Vertex>& vertices, std::vector<Edge>& edges)
        {
            std::vector<Rectangle> rectangles;
            std::vector<Vertex> localVertices;
            std::vector<Edge> localEdges;

            SetLevel(level, depth);
            GetRectangles(rectangles);
            for (auto& rectangle : rectangles)
            {
                if (rectangle.type > 0)
                {
                    GetComponents(level, rectangle, localVertices, localEdges);
                }
            }

            vertices = std::move(localVertices);
            edges = std::move(localEdges);
        }

        void MakeUnique(std::vector<Vertex>& vertices, std::vector<Edge>& edges)
        {
            size_t numVertices = vertices.size();
            size_t numEdges = edges.size();
            if (numVertices == 0 || numEdges == 0)
            {
                return;
            }

            // Compute the map of unique vertices and assign to them new and
            // unique indices.
            std::map<Vertex, int> vmap;
            int nextVertex = 0;
            for (size_t v = 0; v < numVertices; ++v)
            {
                // Keep only unique vertices.
                auto result = vmap.insert(std::make_pair(vertices[v], nextVertex));
                if (result.second)
                {
                    ++nextVertex;
                }
            }

            // Compute the map of unique edges and assign to them new and
            // unique indices.
            std::map<Edge, int> emap;
            int nextEdge = 0;
            for (size_t e = 0; e < numEdges; ++e)
            {
                // Replace old vertex indices by new vertex indices.
                Edge& edge = edges[e];
                for (int i = 0; i < 2; ++i)
                {
                    auto iter = vmap.find(vertices[edge[i]]);
                    LogAssert(iter != vmap.end(), "Expecting the vertex to be in the vmap.");
                    edge[i] = iter->second;
                }

                // Keep only unique edges.
                auto result = emap.insert(std::make_pair(edge, nextEdge));
                if (result.second)
                {
                    ++nextEdge;
                }
            }

            // Pack the vertices into an array.
            vertices.resize(vmap.size());
            for (auto const& element : vmap)
            {
                vertices[element.second] = element.first;
            }

            // Pack the edges into an array.
            edges.resize(emap.size());
            for (auto const& element : emap)
            {
                edges[element.second] = element.first;
            }
        }

    private:
        // Helper classes for the skeleton climbing.
        struct QuadRectangle
        {
            QuadRectangle()
                :
                xOrigin(0),
                yOrigin(0),
                xStride(0),
                yStride(0),
                valid(false)
            {
            }

            QuadRectangle(int inXOrigin, int inYOrigin, int inXStride, int inYStride)
            {
                Initialize(inXOrigin, inYOrigin, inXStride, inYStride);
            }

            void Initialize(int inXOrigin, int inYOrigin, int inXStride, int inYStride)
            {
                xOrigin = inXOrigin;
                yOrigin = inYOrigin;
                xStride = inXStride;
                yStride = inYStride;
                valid = true;
            }

            int xOrigin, yOrigin, xStride, yStride;
            bool valid;
        };

        struct QuadNode
        {
            QuadNode()
            {
                // The members are uninitialized.
            }

            QuadNode(int xOrigin, int yOrigin, int xNext, int yNext, int stride)
                :
                r00(xOrigin, yOrigin, stride, stride),
                r10(xNext, yOrigin, stride, stride),
                r01(xOrigin, yNext, stride, stride),
                r11(xNext, yNext, stride, stride)
            {

            }

            void Initialize(int xOrigin, int yOrigin, int xNext, int yNext, int stride)
            {
                r00.Initialize(xOrigin, yOrigin, stride, stride);
                r10.Initialize(xNext, yOrigin, stride, stride);
                r01.Initialize(xOrigin, yNext, stride, stride);
                r11.Initialize(xNext, yNext, stride, stride);
            }

            bool IsMono() const
            {
                return !r10.valid && !r01.valid && !r11.valid;
            }

            int GetQuantity() const
            {
                int quantity = 0;

                if (r00.valid)
                {
                    ++quantity;
                }

                if (r10.valid)
                {
                    ++quantity;
                }

                if (r01.valid)
                {
                    ++quantity;
                }

                if (r11.valid)
                {
                    ++quantity;
                }

                return quantity;
            }

            QuadRectangle r00, r10, r01, r11;
        };

        class LinearMergeTree
        {
        public:
            LinearMergeTree(int N)
                :
                mTwoPowerN(1 << N),
                mNodes(2 * mTwoPowerN - 1)
            {
            }

            enum
            {
                CFG_NONE,
                CFG_INCR,
                CFG_DECR,
                CFG_MULT
            };

            // Member access.
            int GetQuantity() const
            {
                return 2 * mTwoPowerN - 1;
            }

            int GetNode(int i) const
            {
                return mNodes[i];
            }

            int GetEdge(int i) const
            {
                // assert: mNodes[i] == CFG_INCR || mNodes[i] == CFG_DECR

                // Traverse binary tree looking for incr or decr leaf node.
                int const firstLeaf = mTwoPowerN - 1;
                while (i < firstLeaf)
                {
                    i = 2 * i + 1;
                    if (mNodes[i] == CFG_NONE)
                    {
                        ++i;
                    }
                }

                return i - firstLeaf;
            }

            void SetLevel(Real level, T const* data, int offset, int stride)
            {
                // Assert:  The 'level' is not an image value.  Because T is
                // an integer type, choose 'level' to be a Real-valued number
                // that does not represent an integer.

                // Determine the sign changes between pairs of consecutive
                // samples.
                int const firstLeaf = mTwoPowerN - 1;
                for (int i = 0, leaf = firstLeaf; i < mTwoPowerN; ++i, ++leaf)
                {
                    int base = offset + stride * i;
                    Real value0 = static_cast<Real>(data[base]);
                    Real value1 = static_cast<Real>(data[base + stride]);

                    if (value0 > level)
                    {
                        if (value1 > level)
                        {
                            mNodes[leaf] = CFG_NONE;
                        }
                        else
                        {
                            mNodes[leaf] = CFG_DECR;
                        }
                    }
                    else // value0 < level
                    {
                        if (value1 > level)
                        {
                            mNodes[leaf] = CFG_INCR;
                        }
                        else
                        {
                            mNodes[leaf] = CFG_NONE;
                        }
                    }
                }

                // Propagate the sign change information up the binary tree.
                for (int i = firstLeaf - 1; i >= 0; --i)
                {
                    int twoIp1 = 2 * i + 1;
                    int child0 = mNodes[twoIp1];
                    int child1 = mNodes[twoIp1 + 1];
                    mNodes[i] = (child0 | child1);
                }
            }

        private:
            int mTwoPowerN;
            std::vector<int> mNodes;
        };

        struct Rectangle
        {
            Rectangle(int inXOrigin, int inYOrigin, int inXStride, int inYStride)
                :
                xOrigin(inXOrigin),
                yOrigin(inYOrigin),
                xStride(inXStride),
                yStride(inYStride),
                yOfXMin(-1),
                yOfXMax(-1),
                xOfYMin(-1),
                xOfYMax(-1),
                type(0)
            {

            }

            int xOrigin, yOrigin, xStride, yStride;
            int yOfXMin, yOfXMax, xOfYMin, xOfYMax;

            // A 4-bit flag for how the level set intersects the rectangle
            // boundary.
            //   bit 0 = xmin edge
            //   bit 1 = xmax edge
            //   bit 2 = ymin edge
            //   bit 3 = ymax edge
            // A bit is set if the corresponding edge is intersected by the
            // level set.  This information is known from the CFG flags for
            // LinearMergeTree.  Intersection occurs whenever the flag is
            // CFG_INCR or CFG_DECR.
            unsigned int type;
        };

        class AreaMergeTree
        {
        public:
            AreaMergeTree(int N,
                std::vector<std::shared_ptr<LinearMergeTree>> const& xMerge,
                std::vector<std::shared_ptr<LinearMergeTree>> const& yMerge)
                :
                mXMerge(xMerge),
                mYMerge(yMerge),
                mNodes(((1 << 2 * (N + 1)) - 1) / 3)
            {
            }

            void ConstructMono(int A, int LX, int LY, int xOrigin, int yOrigin,
                int stride, int depth)
            {
                if (stride > 1)  // internal nodes
                {
                    int hStride = stride / 2;

                    int ABase = 4 * A;
                    int A00 = ++ABase;
                    int A10 = ++ABase;
                    int A01 = ++ABase;
                    int A11 = ++ABase;

                    int LXBase = 2 * LX;
                    int LX0 = ++LXBase;
                    int LX1 = ++LXBase;

                    int LYBase = 2 * LY;
                    int LY0 = ++LYBase;
                    int LY1 = ++LYBase;

                    int xNext = xOrigin + hStride;
                    int yNext = yOrigin + hStride;

                    int depthM1 = depth - 1;
                    ConstructMono(A00, LX0, LY0, xOrigin, yOrigin, hStride, depthM1);
                    ConstructMono(A10, LX1, LY0, xNext, yOrigin, hStride, depthM1);
                    ConstructMono(A01, LX0, LY1, xOrigin, yNext, hStride, depthM1);
                    ConstructMono(A11, LX1, LY1, xNext, yNext, hStride, depthM1);

                    if (depth >= 0)
                    {
                        // Merging is prevented above the specified depth in
                        // the tree.  This allows a single object to produce
                        // any resolution isocontour rather than using
                        // multiple objects to do so.
                        mNodes[A].Initialize(xOrigin, yOrigin, xNext, yNext, hStride);
                        return;
                    }

                    bool mono00 = mNodes[A00].IsMono();
                    bool mono10 = mNodes[A10].IsMono();
                    bool mono01 = mNodes[A01].IsMono();
                    bool mono11 = mNodes[A11].IsMono();

                    QuadNode node0(xOrigin, yOrigin, xNext, yNext, hStride);
                    QuadNode node1 = node0;

                    // Merge x first, y second.
                    if (mono00 && mono10)
                    {
                        DoXMerge(node0.r00, node0.r10, LX, yOrigin);
                    }
                    if (mono01 && mono11)
                    {
                        DoXMerge(node0.r01, node0.r11, LX, yNext);
                    }
                    if (mono00 && mono01)
                    {
                        DoYMerge(node0.r00, node0.r01, xOrigin, LY);
                    }
                    if (mono10 && mono11)
                    {
                        DoYMerge(node0.r10, node0.r11, xNext, LY);
                    }

                    // Merge y first, x second.
                    if (mono00 && mono01)
                    {
                        DoYMerge(node1.r00, node1.r01, xOrigin, LY);
                    }
                    if (mono10 && mono11)
                    {
                        DoYMerge(node1.r10, node1.r11, xNext, LY);
                    }
                    if (mono00 && mono10)
                    {
                        DoXMerge(node1.r00, node1.r10, LX, yOrigin);
                    }
                    if (mono01 && mono11)
                    {
                        DoXMerge(node1.r01, node1.r11, LX, yNext);
                    }

                    // Choose the merge that produced the smallest number of
                    // rectangles.
                    if (node0.GetQuantity() <= node1.GetQuantity())
                    {
                        mNodes[A] = node0;
                    }
                    else
                    {
                        mNodes[A] = node1;
                    }
                }
                else  // leaf nodes
                {
                    mNodes[A].r00.Initialize(xOrigin, yOrigin, 1, 1);
                }
            }

            void GetRectangles(int A, int LX, int LY, int xOrigin, int yOrigin,
                int stride, std::vector<Rectangle>& rectangles)
            {
                int hStride = stride / 2;
                int ABase = 4 * A;
                int A00 = ++ABase;
                int A10 = ++ABase;
                int A01 = ++ABase;
                int A11 = ++ABase;
                int LXBase = 2 * LX;
                int LX0 = ++LXBase;
                int LX1 = ++LXBase;
                int LYBase = 2 * LY;
                int LY0 = ++LYBase;
                int LY1 = ++LYBase;
                int xNext = xOrigin + hStride;
                int yNext = yOrigin + hStride;

                QuadRectangle const& r00 = mNodes[A].r00;
                if (r00.valid)
                {
                    if (r00.xStride == stride)
                    {
                        if (r00.yStride == stride)
                        {
                            rectangles.push_back(GetRectangle(r00, LX, LY));
                        }
                        else
                        {
                            rectangles.push_back(GetRectangle(r00, LX, LY0));
                        }
                    }
                    else
                    {
                        if (r00.yStride == stride)
                        {
                            rectangles.push_back(GetRectangle(r00, LX0, LY));
                        }
                        else
                        {
                            GetRectangles(A00, LX0, LY0, xOrigin, yOrigin, hStride, rectangles);
                        }
                    }
                }

                QuadRectangle const& r10 = mNodes[A].r10;
                if (r10.valid)
                {
                    if (r10.yStride == stride)
                    {
                        rectangles.push_back(GetRectangle(r10, LX1, LY));
                    }
                    else
                    {
                        GetRectangles(A10, LX1, LY0, xNext, yOrigin, hStride, rectangles);
                    }
                }

                QuadRectangle const& r01 = mNodes[A].r01;
                if (r01.valid)
                {
                    if (r01.xStride == stride)
                    {
                        rectangles.push_back(GetRectangle(r01, LX, LY1));
                    }
                    else
                    {
                        GetRectangles(A01, LX0, LY1, xOrigin, yNext, hStride, rectangles);
                    }
                }

                QuadRectangle const& r11 = mNodes[A].r11;
                if (r11.valid)
                {
                    GetRectangles(A11, LX1, LY1, xNext, yNext, hStride, rectangles);
                }
            }

        private:
            void DoXMerge(QuadRectangle& r0, QuadRectangle& r1, int LX, int yOrigin)
            {
                if (r0.valid && r1.valid && r0.yStride == r1.yStride)
                {
                    // Rectangles are x-mergeable.
                    int incr = 0, decr = 0;
                    for (int y = 0; y <= r0.yStride; ++y)
                    {
                        switch (mXMerge[yOrigin + y]->GetNode(LX))
                        {
                        case LinearMergeTree::CFG_MULT:
                            return;
                        case LinearMergeTree::CFG_INCR:
                            ++incr;
                            break;
                        case LinearMergeTree::CFG_DECR:
                            ++decr;
                            break;
                        }
                    }

                    if (incr == 0 || decr == 0)
                    {
                        // Strongly mono, x-merge the rectangles.
                        r0.xStride *= 2;
                        r1.valid = false;
                    }
                }
            }

            void DoYMerge(QuadRectangle& r0, QuadRectangle& r1, int xOrigin, int LY)
            {
                if (r0.valid && r1.valid && r0.xStride == r1.xStride)
                {
                    // Rectangles are y-mergeable.
                    int incr = 0, decr = 0;
                    for (int x = 0; x <= r0.xStride; ++x)
                    {
                        switch (mYMerge[xOrigin + x]->GetNode(LY))
                        {
                        case LinearMergeTree::CFG_MULT:
                            return;
                        case LinearMergeTree::CFG_INCR:
                            ++incr;
                            break;
                        case LinearMergeTree::CFG_DECR:
                            ++decr;
                            break;
                        }
                    }

                    if (incr == 0 || decr == 0)
                    {
                        // Strongly mono, y-merge the rectangles.
                        r0.yStride *= 2;
                        r1.valid = false;
                    }
                }
            }

            Rectangle GetRectangle(QuadRectangle const& qrect, int LX, int LY)
            {
                Rectangle rect(qrect.xOrigin, qrect.yOrigin, qrect.xStride, qrect.yStride);

                // xmin edge
                auto merge = mYMerge[qrect.xOrigin];
                if (merge->GetNode(LY) != LinearMergeTree::CFG_NONE)
                {
                    rect.yOfXMin = merge->GetEdge(LY);
                    if (rect.yOfXMin != -1)
                    {
                        rect.type |= 0x01;
                    }
                }

                // xmax edge
                merge = mYMerge[qrect.xOrigin + qrect.xStride];
                if (merge->GetNode(LY) != LinearMergeTree::CFG_NONE)
                {
                    rect.yOfXMax = merge->GetEdge(LY);
                    if (rect.yOfXMax != -1)
                    {
                        rect.type |= 0x02;
                    }
                }

                // ymin edge
                merge = mXMerge[qrect.yOrigin];
                if (merge->GetNode(LX) != LinearMergeTree::CFG_NONE)
                {
                    rect.xOfYMin = merge->GetEdge(LX);
                    if (rect.xOfYMin != -1)
                    {
                        rect.type |= 0x04;
                    }
                }

                // ymax edge
                merge = mXMerge[qrect.yOrigin + qrect.yStride];
                if (merge->GetNode(LX) != LinearMergeTree::CFG_NONE)
                {
                    rect.xOfYMax = merge->GetEdge(LX);
                    if (rect.xOfYMax != -1)
                    {
                        rect.type |= 0x08;
                    }
                }

                return rect;
            }

            std::vector<std::shared_ptr<LinearMergeTree>> mXMerge;
            std::vector<std::shared_ptr<LinearMergeTree>> mYMerge;
            std::vector<QuadNode> mNodes;
        };

    private:
        // Support for extraction of level sets.
        Real GetInterp(Real level, int base, int index, int increment)
        {
            Real f0 = static_cast<Real>(mInputPixels[index]);
            index += increment;
            Real f1 = static_cast<Real>(mInputPixels[index]);
            LogAssert((f0 - level) * (f1 - level) < (Real)0, "Unexpected condition.");
            return static_cast<Real>(base) + (level - f0) / (f1 - f0);
        }

        void AddVertex(std::vector<Vertex>& vertices, Real x, Real y)
        {
            Vertex vertex = { x, y };
            vertices.push_back(vertex);
        }

        void AddEdge(std::vector<Vertex>& vertices,
            std::vector<Edge>& edges, Real x0, Real y0, Real x1, Real y1)
        {
            int v0 = static_cast<int>(vertices.size());
            int v1 = v0 + 1;
            Edge edge = { v0, v1 };
            edges.push_back(edge);
            Vertex vertex0 = { x0, y0 };
            Vertex vertex1 = { x1, y1 };
            vertices.push_back(vertex0);
            vertices.push_back(vertex1);
        }

        void SetLevel(Real level, int depth)
        {
            int offset, stride;

            for (int y = 0; y < mSize; ++y)
            {
                offset = mSize * y;
                stride = 1;
                mXMerge[y]->SetLevel(level, mInputPixels, offset, stride);
            }

            for (int x = 0; x < mSize; ++x)
            {
                offset = x;
                stride = mSize;
                mYMerge[x]->SetLevel(level, mInputPixels, offset, stride);
            }

            mXYMerge->ConstructMono(0, 0, 0, 0, 0, mTwoPowerN, depth);
        }

        void GetRectangles(std::vector<Rectangle>& rectangles)
        {
            mXYMerge->GetRectangles(0, 0, 0, 0, 0, mTwoPowerN, rectangles);
        }

        void GetComponents(Real level, Rectangle const& rectangle,
            std::vector<Vertex>& vertices, std::vector<Edge>& edges)
        {
            int x, y;
            Real x0, y0, x1, y1;

            switch (rectangle.type)
            {
            case  3:  // two vertices, on xmin and xmax
                LogAssert(rectangle.yOfXMin != -1, "Unexpected condition.");
                x = rectangle.xOrigin;
                y = rectangle.yOfXMin;
                x0 = static_cast<Real>(x);
                y0 = GetInterp(level, y, x + mSize * y, mSize);

                LogAssert(rectangle.yOfXMax != -1, "Unexpected condition.");
                x = rectangle.xOrigin + rectangle.xStride;
                y = rectangle.yOfXMax;
                x1 = static_cast<Real>(x);
                y1 = GetInterp(level, y, x + mSize * y, mSize);

                AddEdge(vertices, edges, x0, y0, x1, y1);
                break;
            case  5:  // two vertices, on xmin and ymin
                LogAssert(rectangle.yOfXMin != -1, "Unexpected condition.");
                x = rectangle.xOrigin;
                y = rectangle.yOfXMin;
                x0 = static_cast<Real>(x);
                y0 = GetInterp(level, y, x + mSize * y, mSize);

                LogAssert(rectangle.xOfYMin != -1, "Unexpected condition.");
                x = rectangle.xOfYMin;
                y = rectangle.yOrigin;
                x1 = GetInterp(level, x, x + mSize * y, 1);
                y1 = static_cast<Real>(y);

                AddEdge(vertices, edges, x0, y0, x1, y1);
                break;
            case  6:  // two vertices, on xmax and ymin
                LogAssert(rectangle.yOfXMax != -1, "Unexpected condition.");
                x = rectangle.xOrigin + rectangle.xStride;
                y = rectangle.yOfXMax;
                x0 = static_cast<Real>(x);
                y0 = GetInterp(level, y, x + mSize * y, mSize);

                LogAssert(rectangle.xOfYMin != -1, "Unexpected condition.");
                x = rectangle.xOfYMin;
                y = rectangle.yOrigin;
                x1 = GetInterp(level, x, x + mSize * y, 1);
                y1 = static_cast<Real>(y);

                AddEdge(vertices, edges, x0, y0, x1, y1);
                break;
            case  9:  // two vertices, on xmin and ymax
                LogAssert(rectangle.yOfXMin != -1, "Unexpected condition.");
                x = rectangle.xOrigin;
                y = rectangle.yOfXMin;
                x0 = static_cast<Real>(x);
                y0 = GetInterp(level, y, x + mSize * y, mSize);

                LogAssert(rectangle.xOfYMax != -1, "Unexpected condition.");
                x = rectangle.xOfYMax;
                y = rectangle.yOrigin + rectangle.yStride;
                x1 = GetInterp(level, x, x + mSize * y, 1);
                y1 = static_cast<Real>(y);

                AddEdge(vertices, edges, x0, y0, x1, y1);
                break;
            case 10:  // two vertices, on xmax and ymax
                LogAssert(rectangle.yOfXMax != -1, "Unexpected condition.");
                x = rectangle.xOrigin + rectangle.xStride;
                y = rectangle.yOfXMax;
                x0 = static_cast<Real>(x);
                y0 = GetInterp(level, y, x + mSize * y, mSize);

                LogAssert(rectangle.xOfYMax != -1, "Unexpected condition.");
                x = rectangle.xOfYMax;
                y = rectangle.yOrigin + rectangle.yStride;
                x1 = GetInterp(level, x, x + mSize * y, 1);
                y1 = static_cast<Real>(y);

                AddEdge(vertices, edges, x0, y0, x1, y1);
                break;
            case 12:  // two vertices, on ymin and ymax
                LogAssert(rectangle.xOfYMin != -1, "Unexpected condition.");
                x = rectangle.xOfYMin;
                y = rectangle.yOrigin;
                x0 = GetInterp(level, x, x + mSize * y, 1);
                y0 = static_cast<Real>(y);

                LogAssert(rectangle.xOfYMax != -1, "Unexpected condition.");
                x = rectangle.xOfYMax;
                y = rectangle.yOrigin + rectangle.yStride;
                x1 = GetInterp(level, x, x + mSize * y, 1);
                y1 = static_cast<Real>(y);

                AddEdge(vertices, edges, x0, y0, x1, y1);
                break;
            case 15:  // four vertices, one per edge, need to disambiguate
            {
                LogAssert(rectangle.xStride == 1 && rectangle.yStride == 1,
                    "Unexpected condition.");

                LogAssert(rectangle.yOfXMin != -1, "Unexpected condition.");
                x = rectangle.xOrigin;
                y = rectangle.yOfXMin;
                x0 = static_cast<Real>(x);
                y0 = GetInterp(level, y, x + mSize * y, mSize);

                LogAssert(rectangle.yOfXMax != -1, "Unexpected condition.");
                x = rectangle.xOrigin + rectangle.xStride;
                y = rectangle.yOfXMax;
                x1 = static_cast<Real>(x);
                y1 = GetInterp(level, y, x + mSize * y, mSize);

                LogAssert(rectangle.xOfYMin != -1, "Unexpected condition.");
                x = rectangle.xOfYMin;
                y = rectangle.yOrigin;
                Real fx2 = GetInterp(level, x, x + mSize * y, 1);
                Real fy2 = static_cast<Real>(y);

                LogAssert(rectangle.xOfYMax != -1, "Unexpected condition.");
                x = rectangle.xOfYMax;
                y = rectangle.yOrigin + rectangle.yStride;
                Real fx3 = GetInterp(level, x, x + mSize * y, 1);
                Real fy3 = static_cast<Real>(y);

                int index = rectangle.xOrigin + mSize * rectangle.yOrigin;
                int64_t i00 = static_cast<int64_t>(mInputPixels[index]);
                ++index;
                int64_t i10 = static_cast<int64_t>(mInputPixels[index]);
                index += mSize;
                int64_t i11 = static_cast<int64_t>(mInputPixels[index]);
                --index;
                int64_t i01 = static_cast<int64_t>(mInputPixels[index]);

                int64_t det = i00 * i11 - i01 * i10;
                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2> and <P1,P3>.
                    AddEdge(vertices, edges, x0, y0, fx2, fy2);
                    AddEdge(vertices, edges, x1, y1, fx3, fy3);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3> and <P1,P2>.
                    AddEdge(vertices, edges, x0, y0, fx3, fy3);
                    AddEdge(vertices, edges, x1, y1, fx2, fy2);
                }
                else
                {
                    // Plus-sign configuration, add branch point to
                    // tessellation.
                    Real fx4 = fx2, fy4 = y0;
                    AddEdge(vertices, edges, x0, y0, fx4, fy4);
                    AddEdge(vertices, edges, x1, y1, fx4, fy4);
                    AddEdge(vertices, edges, fx2, fy2, fx4, fy4);
                    AddEdge(vertices, edges, fx3, fy3, fx4, fy4);
                }
                break;
            }
            default:
                LogError("Unexpected condition.");
            }
        }

        // Support for debugging.
        void PrintRectangles(std::ostream& output, std::vector<Rectangle> const& rectangles)
        {
            for (size_t i = 0; i < rectangles.size(); ++i)
            {
                auto const& rectangle = rectangles[i];
                output << "rectangle " << i << std::endl;
                output << "  x origin = " << rectangle.xOrigin << std::endl;
                output << "  y origin = " << rectangle.yOrigin << std::endl;
                output << "  x stride = " << rectangle.xStride << std::endl;
                output << "  y stride = " << rectangle.yStride << std::endl;
                output << "  flag = " << rectangle.type << std::endl;
                output << "  y of xmin = " << rectangle.yOfXMin << std::endl;
                output << "  y of xmax = " << rectangle.yOfXMax << std::endl;
                output << "  x of ymin = " << rectangle.xOfYMin << std::endl;
                output << "  x of ymax = " << rectangle.xOfYMax << std::endl;
                output << std::endl;
            }
        }

        // Storage of image data.
        int mTwoPowerN, mSize;
        T const* mInputPixels;

        // Trees for linear merging.
        std::vector<std::shared_ptr<LinearMergeTree>> mXMerge, mYMerge;

        // Tree for area merging.
        std::unique_ptr<AreaMergeTree> mXYMerge;
    };
}
