// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.14

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Array2.h>
#include <Mathematics/Math.h>
#include <Mathematics/TriangleKey.h>
#include <map>
#include <memory>
#include <ostream>

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
    class AdaptiveSkeletonClimbing3
    {
    public:
        // Construction and destruction.  The input image is assumed to
        // contain (2^N+1)-by-(2^N+1)-by-(2^N+1) elements where N >= 0.
        // The organization is lexicographic order for (x,y,z).  When
        // 'fixBoundary' is set to 'true', image boundary voxels are not
        // allowed to merge with any other voxels.  This forces highest
        // level of detail on the boundary.  The idea is that an image too
        // large to process by itself can be partitioned into smaller
        // subimages and the adaptive skeleton climbing applied to each
        // subimage.  By forcing highest resolution on the boundary,
        // adjacent subimages will not have any cracking problems.
        AdaptiveSkeletonClimbing3(int N, T const* inputVoxels,
            bool fixBoundary = false)
            :
            mTwoPowerN(1 << N),
            mSize(mTwoPowerN + 1),
            mSizeSqr(mSize * mSize),
            mInputVoxels(inputVoxels),
            mLevel((Real)0),
            mFixBoundary(fixBoundary),
            mXMerge(mSize, mSize),
            mYMerge(mSize, mSize),
            mZMerge(mSize, mSize)
        {
            static_assert(std::is_integral<T>::value && sizeof(T) <= 4,
                "Type T must be int{8,16,32}_t or uint{8,16,32}_t.");
            if (N <= 0 || mInputVoxels == nullptr)
            {
                LogError("Invalid input.");
            }

            for (int i = 0; i < mSize; ++i)
            {
                for (int j = 0; j < mSize; ++j)
                {
                    mXMerge[i][j] = std::make_shared<LinearMergeTree>(N);
                    mYMerge[i][j] = std::make_shared<LinearMergeTree>(N);
                    mZMerge[i][j] = std::make_shared<LinearMergeTree>(N);
                }
            }
        }

        // TODO: Refactor this class to have base class SurfaceExtractor.
        typedef std::array<Real, 3> Vertex;
        typedef std::array<int, 2> Edge;
        typedef TriangleKey<true> Triangle;

        void Extract(Real level, int depth,
            std::vector<Vertex>& vertices, std::vector<Triangle>& triangles)
        {
            std::vector<Vertex> localVertices;
            std::vector<Triangle> localTriangles;
            mBoxes.clear();

            mLevel = level;
            Merge(depth);
            Tessellate(localVertices, localTriangles);

            vertices = std::move(localVertices);
            triangles = std::move(localTriangles);
        }

        void MakeUnique(std::vector<Vertex>& vertices,
            std::vector<Triangle>& triangles)
        {
            size_t numVertices = vertices.size();
            size_t numTriangles = triangles.size();
            if (numVertices == 0 || numTriangles == 0)
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

            // Compute the map of unique triangles and assign to them new and
            // unique indices.
            std::map<Triangle, int> tmap;
            int nextTriangle = 0;
            for (size_t t = 0; t < numTriangles; ++t)
            {
                Triangle& triangle = triangles[t];
                for (int i = 0; i < 3; ++i)
                {
                    auto iter = vmap.find(vertices[triangle.V[i]]);
                    LogAssert(iter != vmap.end(), "Expecting the vertex to be in the vmap.");
                    triangle.V[i] = iter->second;
                }

                // Keep only unique triangles.
                auto result = tmap.insert(std::make_pair(triangle, nextTriangle));
                if (result.second)
                {
                    ++nextTriangle;
                }
            }

            // Pack the vertices into an array.
            vertices.resize(vmap.size());
            for (auto const& element : vmap)
            {
                vertices[element.second] = element.first;
            }

            // Pack the triangles into an array.
            triangles.resize(tmap.size());
            for (auto const& element : tmap)
            {
                triangles[element.second] = element.first;
            }
        }

        void OrientTriangles(std::vector<Vertex>& vertices,
            std::vector<Triangle>& triangles, bool sameDir)
        {
            for (auto& triangle : triangles)
            {
                // Get the triangle vertices.
                std::array<Real, 3> v0 = vertices[triangle.V[0]];
                std::array<Real, 3> v1 = vertices[triangle.V[1]];
                std::array<Real, 3> v2 = vertices[triangle.V[2]];

                // Construct the triangle normal based on the current
                // orientation.
                std::array<Real, 3> edge1, edge2, normal;
                for (int i = 0; i < 3; ++i)
                {
                    edge1[i] = v1[i] - v0[i];
                    edge2[i] = v2[i] - v0[i];
                }
                normal[0] = edge1[1] * edge2[2] - edge1[2] * edge2[1];
                normal[1] = edge1[2] * edge2[0] - edge1[0] * edge2[2];
                normal[2] = edge1[0] * edge2[1] - edge1[1] * edge2[0];

                // Get the image gradient at the vertices.
                std::array<Real, 3> grad0 = GetGradient(v0);
                std::array<Real, 3> grad1 = GetGradient(v1);
                std::array<Real, 3> grad2 = GetGradient(v2);

                // Compute the average gradient.
                std::array<Real, 3> gradAvr;
                for (int i = 0; i < 3; ++i)
                {
                    gradAvr[i] = (grad0[i] + grad1[i] + grad2[i]) / (Real)3;
                }

                // Compute the dot product of normal and average gradient.
                Real dot = gradAvr[0] * normal[0] + gradAvr[1] * normal[1] + gradAvr[2] * normal[2];

                // Choose triangle orientation based on gradient direction.
                if (sameDir)
                {
                    if (dot < (Real)0)
                    {
                        // Wrong orientation, reorder it.
                        std::swap(triangle.V[1], triangle.V[2]);
                    }
                }
                else
                {
                    if (dot > (Real)0)
                    {
                        // Wrong orientation, reorder it.
                        std::swap(triangle.V[1], triangle.V[2]);
                    }
                }
            }
        }

        void ComputeNormals(std::vector<Vertex> const& vertices,
            std::vector<Triangle> const& triangles,
            std::vector<std::array<Real, 3>>& normals)
        {
            // Compute a vertex normal to be area-weighted sums of the normals
            // to the triangles that share that vertex.
            std::array<Real, 3> const zero{ (Real)0, (Real)0, (Real)0 };
            normals.resize(vertices.size());
            std::fill(normals.begin(), normals.end(), zero);

            for (auto const& triangle : triangles)
            {
                // Get the triangle vertices.
                std::array<Real, 3> v0 = vertices[triangle.V[0]];
                std::array<Real, 3> v1 = vertices[triangle.V[1]];
                std::array<Real, 3> v2 = vertices[triangle.V[2]];

                // Construct the triangle normal.
                std::array<Real, 3> edge1, edge2, normal;
                for (int i = 0; i < 3; ++i)
                {
                    edge1[i] = v1[i] - v0[i];
                    edge2[i] = v2[i] - v0[i];
                }
                normal[0] = edge1[1] * edge2[2] - edge1[2] * edge2[1];
                normal[1] = edge1[2] * edge2[0] - edge1[0] * edge2[2];
                normal[2] = edge1[0] * edge2[1] - edge1[1] * edge2[0];

                // Maintain the sum of normals at each vertex.
                for (int i = 0; i < 3; ++i)
                {
                    for (int j = 0; j < 3; ++j)
                    {
                        normals[triangle.V[i]][j] += normal[j];
                    }
                }
            }

            // The normal vector storage was used to accumulate the sum of
            // triangle normals.  Now these vectors must be rescaled to be
            // unit length.
            for (auto& normal : normals)
            {
                Real sqrLength = normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2];
                Real length = std::sqrt(sqrLength);
                if (length > (Real)0)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        normal[i] /= length;
                    }
                }
                else
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        normal[i] = (Real)0;
                    }
                }
            }
        }

        // Support for debugging.
        void PrintBoxes(std::ostream& output)
        {
            output << mBoxes.size() << std::endl;
            for (size_t i = 0; i < mBoxes.size(); ++i)
            {
                OctBox const& box = mBoxes[i];
                output << "box " << i << ": ";
                output << "x0 = " << box.x0 << ", ";
                output << "y0 = " << box.y0 << ", ";
                output << "z0 = " << box.z0 << ", ";
                output << "dx = " << box.dx << ", ";
                output << "dy = " << box.dy << ", ";
                output << "dz = " << box.dz << std::endl;
            }
        }

    private:
        // Helper classes for the skeleton climbing.
        struct OctBox
        {
            OctBox(int inX0, int inY0, int inZ0, int inDX, int inDY, int inDZ,
                int inLX, int inLY, int inLZ)
                :
                x0(inX0), y0(inY0), z0(inZ0),
                x1(inX0 + inDX), y1(inY0 + inDY), z1(inZ0 + inDZ),
                dx(inDX), dy(inDY), dz(inDZ),
                LX(inLX), LY(inLY), LZ(inLZ)
            {
            }

            int x0, y0, z0, x1, y1, z1, dx, dy, dz, LX, LY, LZ;
        };

        struct MergeBox
        {
            MergeBox(int stride)
                :
                xStride(stride), yStride(stride), zStride(stride),
                valid(true)
            {
            }

            int xStride, yStride, zStride;
            bool valid;
        };

        class LinearMergeTree
        {
        public:
            LinearMergeTree(int N)
                :
                mTwoPowerN(1 << N),
                mNodes(2 * mTwoPowerN - 1),
                mZeroBases(2 * mTwoPowerN - 1)
            {
            }

            enum
            {
                CFG_NONE = 0,
                CFG_INCR = 1,
                CFG_DECR = 2,
                CFG_MULT = 3,
                CFG_ROOT_MASK = 3,
                CFG_EDGE = 4,
                CFG_ZERO_SUBEDGE = 8
            };

            bool IsNone(int i) const
            {
                return (mNodes[i] & CFG_ROOT_MASK) == CFG_NONE;
            }

            int GetRootType(int i) const
            {
                return mNodes[i] & CFG_ROOT_MASK;
            }

            int GetZeroBase(int i) const
            {
                return mZeroBases[i];
            }

            void SetEdge(int i)
            {
                mNodes[i] |= CFG_EDGE;

                // Inform all predecessors whether they have a zero subedge.
                if (mZeroBases[i] >= 0)
                {
                    while (i > 0)
                    {
                        i = (i - 1) / 2;
                        mNodes[i] |= CFG_ZERO_SUBEDGE;
                    }
                }
            }

            bool IsZeroEdge(int i) const
            {
                return mNodes[i] == (CFG_EDGE | CFG_INCR)
                    || mNodes[i] == (CFG_EDGE | CFG_DECR);
            }

            bool HasZeroSubedge(int i) const
            {
                return (mNodes[i] & CFG_ZERO_SUBEDGE) != 0;
            }

            void SetLevel(Real level, T const* data, int offset, int stride)
            {
                // Assert:  The 'level' is not an image value.  Because T is
                // an integer type, choose 'level' to be a Real-valued number
                // that does not represent an integer.

                // Determine the sign changes between pairs of consecutive
                // samples.
                int firstLeaf = mTwoPowerN - 1;
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
                            mZeroBases[leaf] = -1;
                        }
                        else
                        {
                            mNodes[leaf] = CFG_DECR;
                            mZeroBases[leaf] = i;
                        }
                    }
                    else  // value0 < level
                    {
                        if (value1 > level)
                        {
                            mNodes[leaf] = CFG_INCR;
                            mZeroBases[leaf] = i;
                        }
                        else
                        {
                            mNodes[leaf] = CFG_NONE;
                            mZeroBases[leaf] = -1;
                        }
                    }
                }

                // Propagate the sign change information up the binary tree.
                for (int i = firstLeaf - 1; i >= 0; --i)
                {
                    int twoIp1 = 2 * i + 1, twoIp2 = twoIp1 + 1;
                    int value0 = mNodes[twoIp1];
                    int value1 = mNodes[twoIp2];
                    int combine = (value0 | value1);
                    mNodes[i] = combine;
                    if (combine == CFG_INCR)
                    {
                        if (value0 == CFG_INCR)
                        {
                            mZeroBases[i] = mZeroBases[twoIp1];
                        }
                        else
                        {
                            mZeroBases[i] = mZeroBases[twoIp2];
                        }
                    }
                    else if (combine == CFG_DECR)
                    {
                        if (value0 == CFG_DECR)
                        {
                            mZeroBases[i] = mZeroBases[twoIp1];
                        }
                        else
                        {
                            mZeroBases[i] = mZeroBases[twoIp2];
                        }
                    }
                    else
                    {
                        mZeroBases[i] = -1;
                    }
                }
            }

        private:
            int mTwoPowerN;
            std::vector<int> mNodes;
            std::vector<int> mZeroBases;
        };

        class VETable
        {
        public:
            VETable()
                :
                mVertices(18)
            {
            }

            bool IsValidVertex(int i) const
            {
                return mVertices[i].valid;
            }

            int GetNumVertices() const
            {
                return static_cast<int>(mVertices.size());
            }

            Vertex const& GetVertex(int i) const
            {
                return mVertices[i].position;
            }

            void Insert(int i, Real x, Real y, Real z)
            {
                TVertex& vertex = mVertices[i];
                vertex.position = Vertex{ x, y, z };
                vertex.valid = true;
            }

            void Insert(Vertex const& position)
            {
                mVertices.push_back(TVertex(position));
            }

            void InsertEdge(int v0, int v1)
            {
                TVertex& vertex0 = mVertices[v0];
                TVertex& vertex1 = mVertices[v1];
                vertex0.adjacent[vertex0.adjQuantity] = v1;
                ++vertex0.adjQuantity;
                vertex1.adjacent[vertex1.adjQuantity] = v0;
                ++vertex1.adjQuantity;
            }

            void RemoveTrianglesEC(std::vector<Vertex>& positions,
                std::vector<Triangle>& triangles)
            {
                // Ear-clip the wireframe to get the triangles.
                Triangle triangle;
                while (RemoveEC(triangle))
                {
                    int v0 = static_cast<int>(positions.size());
                    int v1 = v0 + 1;
                    int v2 = v1 + 1;
                    // Bypassing the constructor to avoid a warning in the
                    // release build by gcc 7.5.0 on Ubuntu 18.04.5 LTS:
                    // "assuming signed overflow does not occur when assuming
                    // that (X + c) >= X is always true." The correct
                    // constructor case is used because v0 < v1 < v2.
                    Triangle tkey;
                    tkey.V[0] = v0;
                    tkey.V[1] = v1;
                    tkey.V[2] = v2;
                    triangles.push_back(tkey);
                    positions.push_back(mVertices[triangle.V[0]].position);
                    positions.push_back(mVertices[triangle.V[1]].position);
                    positions.push_back(mVertices[triangle.V[2]].position);
                }
            }

            void RemoveTrianglesSE(std::vector<Vertex>& positions,
                std::vector<Triangle>& triangles)
            {
                // Compute centroid of vertices.
                Vertex centroid = { (Real)0, (Real)0, (Real)0 };
                int const vmax = static_cast<int>(mVertices.size());
                int i, j, quantity = 0;
                for (i = 0; i < vmax; i++)
                {
                    TVertex const& vertex = mVertices[i];
                    if (vertex.valid)
                    {
                        for (j = 0; j < 3; ++j)
                        {
                            centroid[j] += vertex.position[j];
                        }
                        ++quantity;
                    }
                }
                for (j = 0; j < 3; ++j)
                {
                    centroid[j] /= static_cast<Real>(quantity);
                }

                int v0 = static_cast<int>(positions.size());
                positions.push_back(centroid);

                int i1 = 18;
                int v1 = v0 + 1;
                positions.push_back(mVertices[i1].position);

                int i2 = mVertices[i1].adjacent[1], v2;
                for (i = 0; i < quantity - 1; ++i)
                {
                    v2 = v1 + 1;
                    positions.push_back(mVertices[i2].position);
                    // Bypassing the constructor to avoid a warning in the
                    // release build by gcc 7.5.0 on Ubuntu 18.04.5 LTS:
                    // "assuming signed overflow does not occur when assuming
                    // that (X + c) >= X is always true." The correct
                    // constructor case is used because v0 < v1 < v2.
                    Triangle tkey;
                    tkey.V[0] = v0;
                    tkey.V[1] = v1;
                    tkey.V[2] = v2;
                    triangles.push_back(tkey);
                    if (mVertices[i2].adjacent[1] != i1)
                    {
                        i1 = i2;
                        i2 = mVertices[i2].adjacent[1];
                    }
                    else
                    {
                        i1 = i2;
                        i2 = mVertices[i2].adjacent[0];
                    }
                    v1 = v2;
                }

                v2 = v0 + 1;
                // Unlike the previous two constructor calls, it is not
                // guaranteed that v0 < v1.
                triangles.push_back(Triangle(v0, v1, v2));
            }

        protected:
            void RemoveVertex(int i)
            {
                TVertex& vertex0 = mVertices[i];
                int a0 = vertex0.adjacent[0];
                int a1 = vertex0.adjacent[1];
                TVertex& adjVertex0 = mVertices[a0];
                TVertex& adjVertex1 = mVertices[a1];

                int j;
                for (j = 0; j < adjVertex0.adjQuantity; j++)
                {
                    if (adjVertex0.adjacent[j] == i)
                    {
                        adjVertex0.adjacent[j] = a1;
                        break;
                    }
                }

                for (j = 0; j < adjVertex1.adjQuantity; j++)
                {
                    if (adjVertex1.adjacent[j] == i)
                    {
                        adjVertex1.adjacent[j] = a0;
                        break;
                    }
                }

                vertex0.valid = false;

                if (adjVertex0.adjQuantity == 2)
                {
                    if (adjVertex0.adjacent[0] == adjVertex0.adjacent[1])
                    {
                        adjVertex0.valid = false;
                    }
                }

                if (adjVertex1.adjQuantity == 2)
                {
                    if (adjVertex1.adjacent[0] == adjVertex1.adjacent[1])
                    {
                        adjVertex1.valid = false;
                    }
                }
            }

            // ear clipping
            bool RemoveEC(Triangle& triangle)
            {
                int numVertices = static_cast<int>(mVertices.size());
                for (int i = 0; i < numVertices; ++i)
                {
                    TVertex const& vertex = mVertices[i];
                    if (vertex.valid && vertex.adjQuantity == 2)
                    {
                        triangle.V[0] = i;
                        triangle.V[1] = vertex.adjacent[0];
                        triangle.V[2] = vertex.adjacent[1];
                        RemoveVertex(i);
                        return true;
                    }
                }

                return false;
            }

            class TVertex
            {
            public:
                TVertex()
                    :
                    adjQuantity(0),
                    valid(false)
                {
                }

                TVertex(Vertex const& inPosition)
                    :
                    position(inPosition),
                    adjQuantity(0),
                    valid(true)
                {

                }

                Vertex position;
                int adjQuantity;
                std::array<int, 4> adjacent;
                bool valid;
            };

            std::vector<TVertex> mVertices;
        };

    private:
        // Support for merging monoboxes.
        void Merge(int depth)
        {
            int x, y, z, offset, stride;

            for (y = 0; y < mSize; ++y)
            {
                for (z = 0; z < mSize; ++z)
                {
                    offset = mSize * (y + mSize * z);
                    stride = 1;
                    mXMerge[y][z]->SetLevel(mLevel, mInputVoxels, offset, stride);
                }
            }

            for (x = 0; x < mSize; ++x)
            {
                for (z = 0; z < mSize; ++z)
                {
                    offset = x + mSizeSqr * z;
                    stride = mSize;
                    mYMerge[x][z]->SetLevel(mLevel, mInputVoxels, offset, stride);
                }
            }

            for (x = 0; x < mSize; ++x)
            {
                for (y = 0; y < mSize; ++y)
                {
                    offset = x + mSize * y;
                    stride = mSizeSqr;
                    mZMerge[x][y]->SetLevel(mLevel, mInputVoxels, offset, stride);
                }
            }

            Merge(0, 0, 0, 0, 0, 0, 0, mTwoPowerN, depth);
        }

        bool Merge(int v, int LX, int LY, int LZ, int x0, int y0, int z0, int stride, int depth)
        {
            if (stride > 1)  // internal nodes
            {
                int hStride = stride / 2;
                int vBase = 8 * v;
                int v000 = vBase + 1;
                int v100 = vBase + 2;
                int v010 = vBase + 3;
                int v110 = vBase + 4;
                int v001 = vBase + 5;
                int v101 = vBase + 6;
                int v011 = vBase + 7;
                int v111 = vBase + 8;
                int LX0 = 2 * LX + 1, LX1 = LX0 + 1;
                int LY0 = 2 * LY + 1, LY1 = LY0 + 1;
                int LZ0 = 2 * LZ + 1, LZ1 = LZ0 + 1;
                int x1 = x0 + hStride, y1 = y0 + hStride, z1 = z0 + hStride;

                int dm1 = depth - 1;
                bool m000 = Merge(v000, LX0, LY0, LZ0, x0, y0, z0, hStride, dm1);
                bool m100 = Merge(v100, LX1, LY0, LZ0, x1, y0, z0, hStride, dm1);
                bool m010 = Merge(v010, LX0, LY1, LZ0, x0, y1, z0, hStride, dm1);
                bool m110 = Merge(v110, LX1, LY1, LZ0, x1, y1, z0, hStride, dm1);
                bool m001 = Merge(v001, LX0, LY0, LZ1, x0, y0, z1, hStride, dm1);
                bool m101 = Merge(v101, LX1, LY0, LZ1, x1, y0, z1, hStride, dm1);
                bool m011 = Merge(v011, LX0, LY1, LZ1, x0, y1, z1, hStride, dm1);
                bool m111 = Merge(v111, LX1, LY1, LZ1, x1, y1, z1, hStride, dm1);

                MergeBox r000(hStride), r100(hStride), r010(hStride), r110(hStride);
                MergeBox r001(hStride), r101(hStride), r011(hStride), r111(hStride);

                if (depth <= 0)
                {
                    if (m000 && m001)
                    {
                        DoZMerge(r000, r001, x0, y0, LZ);
                    }

                    if (m100 && m101)
                    {
                        DoZMerge(r100, r101, x1, y0, LZ);
                    }

                    if (m010 && m011)
                    {
                        DoZMerge(r010, r011, x0, y1, LZ);
                    }

                    if (m110 && m111)
                    {
                        DoZMerge(r110, r111, x1, y1, LZ);
                    }

                    if (m000 && m010)
                    {
                        DoYMerge(r000, r010, x0, LY, z0);
                    }

                    if (m100 && m110)
                    {
                        DoYMerge(r100, r110, x1, LY, z0);
                    }

                    if (m001 && m011)
                    {
                        DoYMerge(r001, r011, x0, LY, z1);
                    }

                    if (m101 && m111)
                    {
                        DoYMerge(r101, r111, x1, LY, z1);
                    }

                    if (m000 && m100)
                    {
                        DoXMerge(r000, r100, LX, y0, z0);
                    }

                    if (m010 && m110)
                    {
                        DoXMerge(r010, r110, LX, y1, z0);
                    }

                    if (m001 && m101)
                    {
                        DoXMerge(r001, r101, LX, y0, z1);
                    }

                    if (m011 && m111)
                    {
                        DoXMerge(r011, r111, LX, y1, z1);
                    }
                }

                if (depth <= 1)
                {
                    if (r000.valid)
                    {
                        if (r000.xStride == stride)
                        {
                            if (r000.yStride == stride)
                            {
                                if (r000.zStride == stride)
                                {
                                    return true;
                                }
                                else
                                {
                                    AddBox(x0, y0, z0, stride, stride, hStride, LX, LY, LZ0);
                                }
                            }
                            else
                            {
                                if (r000.zStride == stride)
                                {
                                    AddBox(x0, y0, z0, stride, hStride, stride, LX, LY0, LZ);
                                }
                                else
                                {
                                    AddBox(x0, y0, z0, stride, hStride, hStride, LX, LY0, LZ0);
                                }
                            }
                        }
                        else
                        {
                            if (r000.yStride == stride)
                            {
                                if (r000.zStride == stride)
                                {
                                    AddBox(x0, y0, z0, hStride, stride, stride, LX0, LY, LZ);
                                }
                                else
                                {
                                    AddBox(x0, y0, z0, hStride, stride, hStride, LX0, LY, LZ0);
                                }
                            }
                            else
                            {
                                if (r000.zStride == stride)
                                {
                                    AddBox(x0, y0, z0, hStride, hStride, stride, LX0, LY0, LZ);
                                }
                                else if (m000)
                                {
                                    AddBox(x0, y0, z0, hStride, hStride, hStride, LX0, LY0, LZ0);
                                }
                            }
                        }
                    }

                    if (r100.valid)
                    {
                        if (r100.yStride == stride)
                        {
                            if (r100.zStride == stride)
                            {
                                AddBox(x1, y0, z0, hStride, stride, stride, LX1, LY, LZ);
                            }
                            else
                            {
                                AddBox(x1, y0, z0, hStride, stride, hStride, LX1, LY, LZ0);
                            }
                        }
                        else
                        {
                            if (r100.zStride == stride)
                            {
                                AddBox(x1, y0, z0, hStride, hStride, stride, LX1, LY0, LZ);
                            }
                            else if (m100)
                            {
                                AddBox(x1, y0, z0, hStride, hStride, hStride, LX1, LY0, LZ0);
                            }
                        }
                    }

                    if (r010.valid)
                    {
                        if (r010.xStride == stride)
                        {
                            if (r010.zStride == stride)
                            {
                                AddBox(x0, y1, z0, stride, hStride, stride, LX, LY1, LZ);
                            }
                            else
                            {
                                AddBox(x0, y1, z0, stride, hStride, hStride, LX, LY1, LZ0);
                            }
                        }
                        else
                        {
                            if (r010.zStride == stride)
                            {
                                AddBox(x0, y1, z0, hStride, hStride, stride, LX0, LY1, LZ);
                            }
                            else if (m010)
                            {
                                AddBox(x0, y1, z0, hStride, hStride, hStride, LX0, LY1, LZ0);
                            }
                        }
                    }

                    if (r001.valid)
                    {
                        if (r001.xStride == stride)
                        {
                            if (r001.yStride == stride)
                            {
                                AddBox(x0, y0, z1, stride, stride, hStride, LX, LY, LZ1);
                            }
                            else
                            {
                                AddBox(x0, y0, z1, stride, hStride, hStride, LX, LY0, LZ1);
                            }
                        }
                        else
                        {
                            if (r001.yStride == stride)
                            {
                                AddBox(x0, y0, z1, hStride, stride, hStride, LX0, LY, LZ1);
                            }
                            else if (m001)
                            {
                                AddBox(x0, y0, z1, hStride, hStride, hStride, LX0, LY0, LZ1);
                            }
                        }
                    }

                    if (r110.valid)
                    {
                        if (r110.zStride == stride)
                        {
                            AddBox(x1, y1, z0, hStride, hStride, stride, LX1, LY1, LZ);
                        }
                        else if (m110)
                        {
                            AddBox(x1, y1, z0, hStride, hStride, hStride, LX1, LY1, LZ0);
                        }
                    }

                    if (r101.valid)
                    {
                        if (r101.yStride == stride)
                        {
                            AddBox(x1, y0, z1, hStride, stride, hStride, LX1, LY, LZ1);
                        }
                        else if (m101)
                        {
                            AddBox(x1, y0, z1, hStride, hStride, hStride, LX1, LY0, LZ1);
                        }
                    }

                    if (r011.valid)
                    {
                        if (r011.xStride == stride)
                        {
                            AddBox(x0, y1, z1, stride, hStride, hStride, LX, LY1, LZ1);
                        }
                        else if (m011)
                        {
                            AddBox(x0, y1, z1, hStride, hStride, hStride, LX0, LY1, LZ1);
                        }
                    }

                    if (r111.valid && m111)
                    {
                        AddBox(x1, y1, z1, hStride, hStride, hStride, LX1, LY1, LZ1);
                    }
                }
                return false;
            }
            else  // leaf nodes
            {
                if (mFixBoundary)
                {
                    // Do not allow boundary voxels to merge with any other
                    // voxels.
                    AddBox(x0, y0, z0, 1, 1, 1, LX, LY, LZ);
                    return false;
                }

                // A leaf box is mergeable with neighbors as long as all its
                // faces have 0 or 2 sign changes on the edges.  That is, a
                // face may not have sign changes on all four edges.  If it
                // does, the resulting box for tessellating is 1x1x1 and is
                // handled separately from boxes of larger dimensions.

                // xmin face
                int z1 = z0 + 1;
                int rt0 = mYMerge[x0][z0]->GetRootType(LY);
                int rt1 = mYMerge[x0][z1]->GetRootType(LY);
                if ((rt0 | rt1) == LinearMergeTree::CFG_MULT)
                {
                    AddBox(x0, y0, z0, 1, 1, 1, LX, LY, LZ);
                    return false;
                }

                // xmax face
                int x1 = x0 + 1;
                rt0 = mYMerge[x1][z0]->GetRootType(LY);
                rt1 = mYMerge[x1][z1]->GetRootType(LY);
                if ((rt0 | rt1) == LinearMergeTree::CFG_MULT)
                {
                    AddBox(x0, y0, z0, 1, 1, 1, LX, LY, LZ);
                    return false;
                }

                // ymin face
                rt0 = mZMerge[x0][y0]->GetRootType(LZ);
                rt1 = mZMerge[x1][y0]->GetRootType(LZ);
                if ((rt0 | rt1) == LinearMergeTree::CFG_MULT)
                {
                    AddBox(x0, y0, z0, 1, 1, 1, LX, LY, LZ);
                    return false;
                }

                // ymax face
                int y1 = y0 + 1;
                rt0 = mZMerge[x0][y1]->GetRootType(LZ);
                rt1 = mZMerge[x1][y1]->GetRootType(LZ);
                if ((rt0 | rt1) == LinearMergeTree::CFG_MULT)
                {
                    AddBox(x0, y0, z0, 1, 1, 1, LX, LY, LZ);
                    return false;
                }

                // zmin face
                rt0 = mXMerge[y0][z0]->GetRootType(LX);
                rt1 = mXMerge[y1][z0]->GetRootType(LX);
                if ((rt0 | rt1) == LinearMergeTree::CFG_MULT)
                {
                    AddBox(x0, y0, z0, 1, 1, 1, LX, LY, LZ);
                    return false;
                }

                // zmax face
                rt0 = mXMerge[y0][z1]->GetRootType(LX);
                rt1 = mXMerge[y1][z1]->GetRootType(LX);
                if ((rt0 | rt1) == LinearMergeTree::CFG_MULT)
                {
                    AddBox(x0, y0, z0, 1, 1, 1, LX, LY, LZ);
                    return false;
                }

                return true;
            }
        }

        bool DoXMerge(MergeBox& r0, MergeBox& r1, int LX, int y0, int z0)
        {
            if (!r0.valid || !r1.valid || r0.yStride != r1.yStride || r0.zStride != r1.zStride)
            {
                return false;
            }

            // Boxes are potentially x-mergeable.
            int y1 = y0 + r0.yStride, z1 = z0 + r0.zStride;
            int incr = 0, decr = 0;
            for (int y = y0; y <= y1; ++y)
            {
                for (int z = z0; z <= z1; ++z)
                {
                    switch (mXMerge[y][z]->GetRootType(LX))
                    {
                    case LinearMergeTree::CFG_MULT:
                        return false;
                    case LinearMergeTree::CFG_INCR:
                        ++incr;
                        break;
                    case LinearMergeTree::CFG_DECR:
                        ++decr;
                        break;
                    }
                }
            }

            if (incr != 0 && decr != 0)
            {
                return false;
            }

            // Strongly mono, x-merge the boxes.
            r0.xStride *= 2;
            r1.valid = false;
            return true;
        }

        bool DoYMerge(MergeBox& r0, MergeBox& r1, int x0, int LY, int z0)
        {
            if (!r0.valid || !r1.valid || r0.xStride != r1.xStride || r0.zStride != r1.zStride)
            {
                return false;
            }

            // Boxes are potentially y-mergeable.
            int x1 = x0 + r0.xStride, z1 = z0 + r0.zStride;
            int incr = 0, decr = 0;
            for (int x = x0; x <= x1; ++x)
            {
                for (int z = z0; z <= z1; ++z)
                {
                    switch (mYMerge[x][z]->GetRootType(LY))
                    {
                    case LinearMergeTree::CFG_MULT:
                        return false;
                    case LinearMergeTree::CFG_INCR:
                        ++incr;
                        break;
                    case LinearMergeTree::CFG_DECR:
                        ++decr;
                        break;
                    }
                }
            }

            if (incr != 0 && decr != 0)
            {
                return false;
            }

            // Strongly mono, y-merge the boxes.
            r0.yStride *= 2;
            r1.valid = false;
            return true;
        }

        bool DoZMerge(MergeBox& r0, MergeBox& r1, int x0, int y0, int LZ)
        {
            if (!r0.valid || !r1.valid || r0.xStride != r1.xStride || r0.yStride != r1.yStride)
            {
                return false;
            }

            // Boxes are potentially z-mergeable.
            int x1 = x0 + r0.xStride, y1 = y0 + r0.yStride;
            int incr = 0, decr = 0;
            for (int x = x0; x <= x1; ++x)
            {
                for (int y = y0; y <= y1; ++y)
                {
                    switch (mZMerge[x][y]->GetRootType(LZ))
                    {
                    case LinearMergeTree::CFG_MULT:
                        return false;
                    case LinearMergeTree::CFG_INCR:
                        ++incr;
                        break;
                    case LinearMergeTree::CFG_DECR:
                        ++decr;
                        break;
                    }
                }
            }

            if (incr != 0 && decr != 0)
            {
                return false;
            }

            // Strongly mono, z-merge the boxes.
            r0.zStride *= 2;
            r1.valid = false;
            return true;
        }

        void AddBox(int x0, int y0, int z0, int dx, int dy, int dz, int LX, int LY, int LZ)
        {
            OctBox box(x0, y0, z0, dx, dy, dz, LX, LY, LZ);
            mBoxes.push_back(box);

            // Mark box edges in linear merge trees.  This information will be
            // used later for extraction.
            mXMerge[box.y0][box.z0]->SetEdge(box.LX);
            mXMerge[box.y0][box.z1]->SetEdge(box.LX);
            mXMerge[box.y1][box.z0]->SetEdge(box.LX);
            mXMerge[box.y1][box.z1]->SetEdge(box.LX);
            mYMerge[box.x0][box.z0]->SetEdge(box.LY);
            mYMerge[box.x0][box.z1]->SetEdge(box.LY);
            mYMerge[box.x1][box.z0]->SetEdge(box.LY);
            mYMerge[box.x1][box.z1]->SetEdge(box.LY);
            mZMerge[box.x0][box.y0]->SetEdge(box.LZ);
            mZMerge[box.x0][box.y1]->SetEdge(box.LZ);
            mZMerge[box.x1][box.y0]->SetEdge(box.LZ);
            mZMerge[box.x1][box.y1]->SetEdge(box.LZ);
        }

        // Support for tessellating monoboxes.
        void Tessellate(std::vector<Vertex>& positions, std::vector<Triangle>& triangles)
        {
            for (size_t i = 0; i < mBoxes.size(); ++i)
            {
                OctBox const& box = mBoxes[i];

                // Get vertices on edges of box.
                VETable table;
                unsigned int type;
                GetVertices(box, type, table);
                if (type == 0)
                {
                    continue;
                }

                // Add wireframe edges to table, add face-vertices if
                // necessary.
                if (box.dx > 1 || box.dy > 1 || box.dz > 1)
                {
                    // Box is larger than voxel, each face has at most one
                    // edge.
                    GetXMinEdgesM(box, type, table);
                    GetXMaxEdgesM(box, type, table);
                    GetYMinEdgesM(box, type, table);
                    GetYMaxEdgesM(box, type, table);
                    GetZMinEdgesM(box, type, table);
                    GetZMaxEdgesM(box, type, table);

                    if (table.GetNumVertices() > 18)
                    {
                        table.RemoveTrianglesSE(positions, triangles);
                    }
                    else
                    {
                        table.RemoveTrianglesEC(positions, triangles);
                    }
                }
                else
                {
                    // The box is a 1x1x1 voxel.  Do the full edge analysis
                    // but no splitting is required.
                    GetXMinEdgesS(box, type, table);
                    GetXMaxEdgesS(box, type, table);
                    GetYMinEdgesS(box, type, table);
                    GetYMaxEdgesS(box, type, table);
                    GetZMinEdgesS(box, type, table);
                    GetZMaxEdgesS(box, type, table);
                    table.RemoveTrianglesEC(positions, triangles);
                }
            }
        }

        Real GetXInterp(int x, int y, int z) const
        {
            int index = x + mSize * (y + mSize * z);
            Real f0 = static_cast<Real>(mInputVoxels[index]);
            ++index;
            Real f1 = static_cast<Real>(mInputVoxels[index]);
            return static_cast<Real>(x) + (mLevel - f0) / (f1 - f0);
        }

        Real GetYInterp(int x, int y, int z) const
        {
            int index = x + mSize * (y + mSize * z);
            Real f0 = static_cast<Real>(mInputVoxels[index]);
            index += mSize;
            Real f1 = static_cast<Real>(mInputVoxels[index]);
            return static_cast<Real>(y) + (mLevel - f0) / (f1 - f0);
        }

        Real GetZInterp(int x, int y, int z) const
        {
            int index = x + mSize * (y + mSize * z);
            Real f0 = static_cast<Real>(mInputVoxels[index]);
            index += mSizeSqr;
            Real f1 = static_cast<Real>(mInputVoxels[index]);
            return static_cast<Real>(z) + (mLevel - f0) / (f1 - f0);
        }

        void GetVertices(OctBox const& box, unsigned int& type, VETable& table)
        {
            int root;
            type = 0;

            // xmin-ymin edge
            root = mZMerge[box.x0][box.y0]->GetZeroBase(box.LZ);
            if (root != -1)
            {
                type |= EB_XMIN_YMIN;
                table.Insert(EI_XMIN_YMIN,
                    static_cast<float>(box.x0),
                    static_cast<float>(box.y0),
                    GetZInterp(box.x0, box.y0, root));
            }

            // xmin-ymax edge
            root = mZMerge[box.x0][box.y1]->GetZeroBase(box.LZ);
            if (root != -1)
            {
                type |= EB_XMIN_YMAX;
                table.Insert(EI_XMIN_YMAX,
                    static_cast<float>(box.x0),
                    static_cast<float>(box.y1),
                    GetZInterp(box.x0, box.y1, root));
            }

            // xmax-ymin edge
            root = mZMerge[box.x1][box.y0]->GetZeroBase(box.LZ);
            if (root != -1)
            {
                type |= EB_XMAX_YMIN;
                table.Insert(EI_XMAX_YMIN,
                    static_cast<float>(box.x1),
                    static_cast<float>(box.y0),
                    GetZInterp(box.x1, box.y0, root));
            }

            // xmax-ymax edge
            root = mZMerge[box.x1][box.y1]->GetZeroBase(box.LZ);
            if (root != -1)
            {
                type |= EB_XMAX_YMAX;
                table.Insert(EI_XMAX_YMAX,
                    static_cast<float>(box.x1),
                    static_cast<float>(box.y1),
                    GetZInterp(box.x1, box.y1, root));
            }

            // xmin-zmin edge
            root = mYMerge[box.x0][box.z0]->GetZeroBase(box.LY);
            if (root != -1)
            {
                type |= EB_XMIN_ZMIN;
                table.Insert(EI_XMIN_ZMIN,
                    static_cast<float>(box.x0),
                    GetYInterp(box.x0, root, box.z0),
                    static_cast<float>(box.z0));
            }

            // xmin-zmax edge
            root = mYMerge[box.x0][box.z1]->GetZeroBase(box.LY);
            if (root != -1)
            {
                type |= EB_XMIN_ZMAX;
                table.Insert(EI_XMIN_ZMAX,
                    static_cast<float>(box.x0),
                    GetYInterp(box.x0, root, box.z1),
                    static_cast<float>(box.z1));
            }

            // xmax-zmin edge
            root = mYMerge[box.x1][box.z0]->GetZeroBase(box.LY);
            if (root != -1)
            {
                type |= EB_XMAX_ZMIN;
                table.Insert(EI_XMAX_ZMIN,
                    static_cast<float>(box.x1),
                    GetYInterp(box.x1, root, box.z0),
                    static_cast<float>(box.z0));
            }

            // xmax-zmax edge
            root = mYMerge[box.x1][box.z1]->GetZeroBase(box.LY);
            if (root != -1)
            {
                type |= EB_XMAX_ZMAX;
                table.Insert(EI_XMAX_ZMAX,
                    static_cast<float>(box.x1),
                    GetYInterp(box.x1, root, box.z1),
                    static_cast<float>(box.z1));
            }

            // ymin-zmin edge
            root = mXMerge[box.y0][box.z0]->GetZeroBase(box.LX);
            if (root != -1)
            {
                type |= EB_YMIN_ZMIN;
                table.Insert(EI_YMIN_ZMIN,
                    GetXInterp(root, box.y0, box.z0),
                    static_cast<float>(box.y0),
                    static_cast<float>(box.z0));
            }

            // ymin-zmax edge
            root = mXMerge[box.y0][box.z1]->GetZeroBase(box.LX);
            if (root != -1)
            {
                type |= EB_YMIN_ZMAX;
                table.Insert(EI_YMIN_ZMAX,
                    GetXInterp(root, box.y0, box.z1),
                    static_cast<float>(box.y0),
                    static_cast<float>(box.z1));
            }

            // ymax-zmin edge
            root = mXMerge[box.y1][box.z0]->GetZeroBase(box.LX);
            if (root != -1)
            {
                type |= EB_YMAX_ZMIN;
                table.Insert(EI_YMAX_ZMIN,
                    GetXInterp(root, box.y1, box.z0),
                    static_cast<float>(box.y1),
                    static_cast<float>(box.z0));
            }

            // ymax-zmax edge
            root = mXMerge[box.y1][box.z1]->GetZeroBase(box.LX);
            if (root != -1)
            {
                type |= EB_YMAX_ZMAX;
                table.Insert(EI_YMAX_ZMAX,
                    GetXInterp(root, box.y1, box.z1),
                    static_cast<float>(box.y1),
                    static_cast<float>(box.z1));
            }
        }

        // Edge extraction for single boxes (1x1x1).
        void GetXMinEdgesS(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_YMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMIN_YMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_XMIN_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_XMIN_ZMAX)
            {
                faceType |= 0x08;
            }

            switch (faceType)
            {
            case 0:
                return;
            case 3:
                table.InsertEdge(EI_XMIN_YMIN, EI_XMIN_YMAX);
                break;
            case 5:
                table.InsertEdge(EI_XMIN_YMIN, EI_XMIN_ZMIN);
                break;
            case 6:
                table.InsertEdge(EI_XMIN_YMAX, EI_XMIN_ZMIN);
                break;
            case 9:
                table.InsertEdge(EI_XMIN_YMIN, EI_XMIN_ZMAX);
                break;
            case 10:
                table.InsertEdge(EI_XMIN_YMAX, EI_XMIN_ZMAX);
                break;
            case 12:
                table.InsertEdge(EI_XMIN_ZMIN, EI_XMIN_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = box.x0 + mSize * (box.y0 + mSize * box.z0);
                // F(x,y,z)
                int64_t f00 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSize;
                // F(x,y+1,z)
                int64_t f10 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSizeSqr;
                // F(x,y+1,z+1)
                int64_t f11 = static_cast<int64_t>(mInputVoxels[i]);
                i -= mSize;
                // F(x,y,z+1)
                int64_t f01 = static_cast<int64_t>(mInputVoxels[i]);
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.InsertEdge(EI_XMIN_YMIN, EI_XMIN_ZMIN);
                    table.InsertEdge(EI_XMIN_YMAX, EI_XMIN_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.InsertEdge(EI_XMIN_YMIN, EI_XMIN_ZMAX);
                    table.InsertEdge(EI_XMIN_YMAX, EI_XMIN_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to
                    // tessellation.
                    table.Insert(FI_XMIN,
                        table.GetVertex(EI_XMIN_ZMIN)[0],
                        table.GetVertex(EI_XMIN_ZMIN)[1],
                        table.GetVertex(EI_XMIN_YMIN)[2]);

                    // Add edges sharing the branch point.
                    table.InsertEdge(EI_XMIN_YMIN, FI_XMIN);
                    table.InsertEdge(EI_XMIN_YMAX, FI_XMIN);
                    table.InsertEdge(EI_XMIN_ZMIN, FI_XMIN);
                    table.InsertEdge(EI_XMIN_ZMAX, FI_XMIN);
                }
                break;
            }
            default:
                LogError("The level value cannot be an exact integer.");
            }
        }

        void GetXMaxEdgesS(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMAX_YMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_YMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_XMAX_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_XMAX_ZMAX)
            {
                faceType |= 0x08;
            }

            switch (faceType)
            {
            case 0:
                return;
            case 3:
                table.InsertEdge(EI_XMAX_YMIN, EI_XMAX_YMAX);
                break;
            case 5:
                table.InsertEdge(EI_XMAX_YMIN, EI_XMAX_ZMIN);
                break;
            case 6:
                table.InsertEdge(EI_XMAX_YMAX, EI_XMAX_ZMIN);
                break;
            case 9:
                table.InsertEdge(EI_XMAX_YMIN, EI_XMAX_ZMAX);
                break;
            case 10:
                table.InsertEdge(EI_XMAX_YMAX, EI_XMAX_ZMAX);
                break;
            case 12:
                table.InsertEdge(EI_XMAX_ZMIN, EI_XMAX_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = box.x1 + mSize * (box.y0 + mSize * box.z0);
                // F(x,y,z)
                int64_t f00 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSize;
                // F(x,y+1,z)
                int64_t f10 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSizeSqr;
                // F(x,y+1,z+1)
                int64_t f11 = static_cast<int64_t>(mInputVoxels[i]);
                i -= mSize;
                // F(x,y,z+1)
                int64_t f01 = static_cast<int64_t>(mInputVoxels[i]);
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.InsertEdge(EI_XMAX_YMIN, EI_XMAX_ZMIN);
                    table.InsertEdge(EI_XMAX_YMAX, EI_XMAX_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.InsertEdge(EI_XMAX_YMIN, EI_XMAX_ZMAX);
                    table.InsertEdge(EI_XMAX_YMAX, EI_XMAX_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to
                    // tessellation.
                    table.Insert(FI_XMAX,
                        table.GetVertex(EI_XMAX_ZMIN)[0],
                        table.GetVertex(EI_XMAX_ZMIN)[1],
                        table.GetVertex(EI_XMAX_YMIN)[2]);

                    // Add edges sharing the branch point.
                    table.InsertEdge(EI_XMAX_YMIN, FI_XMAX);
                    table.InsertEdge(EI_XMAX_YMAX, FI_XMAX);
                    table.InsertEdge(EI_XMAX_ZMIN, FI_XMAX);
                    table.InsertEdge(EI_XMAX_ZMAX, FI_XMAX);
                }
                break;
            }
            default:
                LogError("The level value cannot be an exact integer.");
            }
        }

        void GetYMinEdgesS(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_YMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_YMIN)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMIN_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMIN_ZMAX)
            {
                faceType |= 0x08;
            }

            switch (faceType)
            {
            case 0:
                return;
            case 3:
                table.InsertEdge(EI_XMIN_YMIN, EI_XMAX_YMIN);
                break;
            case 5:
                table.InsertEdge(EI_XMIN_YMIN, EI_YMIN_ZMIN);
                break;
            case 6:
                table.InsertEdge(EI_XMAX_YMIN, EI_YMIN_ZMIN);
                break;
            case 9:
                table.InsertEdge(EI_XMIN_YMIN, EI_YMIN_ZMAX);
                break;
            case 10:
                table.InsertEdge(EI_XMAX_YMIN, EI_YMIN_ZMAX);
                break;
            case 12:
                table.InsertEdge(EI_YMIN_ZMIN, EI_YMIN_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = box.x0 + mSize * (box.y0 + mSize * box.z0);
                // F(x,y,z)
                int64_t f00 = static_cast<int64_t>(mInputVoxels[i]);
                ++i;
                // F(x+1,y,z)
                int64_t f10 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSizeSqr;
                // F(x+1,y,z+1)
                int64_t f11 = static_cast<int64_t>(mInputVoxels[i]);
                --i;
                // F(x,y,z+1)
                int64_t f01 = static_cast<int64_t>(mInputVoxels[i]);
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.InsertEdge(EI_XMIN_YMIN, EI_YMIN_ZMIN);
                    table.InsertEdge(EI_XMAX_YMIN, EI_YMIN_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.InsertEdge(EI_XMIN_YMIN, EI_YMIN_ZMAX);
                    table.InsertEdge(EI_XMAX_YMIN, EI_YMIN_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to
                    // tessellation.
                    table.Insert(FI_YMIN,
                        table.GetVertex(EI_YMIN_ZMIN)[0],
                        table.GetVertex(EI_XMIN_YMIN)[1],
                        table.GetVertex(EI_XMIN_YMIN)[2]);

                    // Add edges sharing the branch point.
                    table.InsertEdge(EI_XMIN_YMIN, FI_YMIN);
                    table.InsertEdge(EI_XMAX_YMIN, FI_YMIN);
                    table.InsertEdge(EI_YMIN_ZMIN, FI_YMIN);
                    table.InsertEdge(EI_YMIN_ZMAX, FI_YMIN);
                }
                break;
            }
            default:
                LogError("The level value cannot be an exact integer.");
            }
        }

        void GetYMaxEdgesS(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_YMAX)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_YMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMAX_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMAX_ZMAX)
            {
                faceType |= 0x08;
            }

            switch (faceType)
            {
            case 0:
                return;
            case 3:
                table.InsertEdge(EI_XMIN_YMAX, EI_XMAX_YMAX);
                break;
            case 5:
                table.InsertEdge(EI_XMIN_YMAX, EI_YMAX_ZMIN);
                break;
            case 6:
                table.InsertEdge(EI_XMAX_YMAX, EI_YMAX_ZMIN);
                break;
            case 9:
                table.InsertEdge(EI_XMIN_YMAX, EI_YMAX_ZMAX);
                break;
            case 10:
                table.InsertEdge(EI_XMAX_YMAX, EI_YMAX_ZMAX);
                break;
            case 12:
                table.InsertEdge(EI_YMAX_ZMIN, EI_YMAX_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = box.x0 + mSize * (box.y1 + mSize * box.z0);
                // F(x,y,z)
                int64_t f00 = static_cast<int64_t>(mInputVoxels[i]);
                ++i;
                // F(x+1,y,z)
                int64_t f10 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSizeSqr;
                // F(x+1,y,z+1)
                int64_t f11 = static_cast<int64_t>(mInputVoxels[i]);
                --i;
                // F(x,y,z+1)
                int64_t f01 = static_cast<int64_t>(mInputVoxels[i]);
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.InsertEdge(EI_XMIN_YMAX, EI_YMAX_ZMIN);
                    table.InsertEdge(EI_XMAX_YMAX, EI_YMAX_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.InsertEdge(EI_XMIN_YMAX, EI_YMAX_ZMAX);
                    table.InsertEdge(EI_XMAX_YMAX, EI_YMAX_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to
                    // tessellation.
                    table.Insert(FI_YMAX,
                        table.GetVertex(EI_YMAX_ZMIN)[0],
                        table.GetVertex(EI_XMIN_YMAX)[1],
                        table.GetVertex(EI_XMIN_YMAX)[2]);

                    // Add edges sharing the branch point.
                    table.InsertEdge(EI_XMIN_YMAX, FI_YMAX);
                    table.InsertEdge(EI_XMAX_YMAX, FI_YMAX);
                    table.InsertEdge(EI_YMAX_ZMIN, FI_YMAX);
                    table.InsertEdge(EI_YMAX_ZMAX, FI_YMAX);
                }
                break;
            }
            default:
                LogError("The level value cannot be an exact integer.");
            }
        }

        void GetZMinEdgesS(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_ZMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_ZMIN)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMIN_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMAX_ZMIN)
            {
                faceType |= 0x08;
            }

            switch (faceType)
            {
            case 0:
                return;
            case 3:
                table.InsertEdge(EI_XMIN_ZMIN, EI_XMAX_ZMIN);
                break;
            case 5:
                table.InsertEdge(EI_XMIN_ZMIN, EI_YMIN_ZMIN);
                break;
            case 6:
                table.InsertEdge(EI_XMAX_ZMIN, EI_YMIN_ZMIN);
                break;
            case 9:
                table.InsertEdge(EI_XMIN_ZMIN, EI_YMAX_ZMIN);
                break;
            case 10:
                table.InsertEdge(EI_XMAX_ZMIN, EI_YMAX_ZMIN);
                break;
            case 12:
                table.InsertEdge(EI_YMIN_ZMIN, EI_YMAX_ZMIN);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = box.x0 + mSize * (box.y0 + mSize * box.z0);
                // F(x,y,z)
                int64_t f00 = static_cast<int64_t>(mInputVoxels[i]);
                ++i;
                // F(x+1,y,z)
                int64_t f10 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSize;
                // F(x+1,y+1,z)
                int64_t f11 = static_cast<int64_t>(mInputVoxels[i]);
                --i;
                // F(x,y+1,z)
                int64_t f01 = static_cast<int64_t>(mInputVoxels[i]);
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.InsertEdge(EI_XMIN_ZMIN, EI_YMIN_ZMIN);
                    table.InsertEdge(EI_XMAX_ZMIN, EI_YMAX_ZMIN);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.InsertEdge(EI_XMIN_ZMIN, EI_YMAX_ZMIN);
                    table.InsertEdge(EI_XMAX_ZMIN, EI_YMIN_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to
                    // tessellation.
                    table.Insert(FI_ZMIN,
                        table.GetVertex(EI_YMIN_ZMIN)[0],
                        table.GetVertex(EI_XMIN_ZMIN)[1],
                        table.GetVertex(EI_XMIN_ZMIN)[2]);

                    // Add edges sharing the branch point.
                    table.InsertEdge(EI_XMIN_ZMIN, FI_ZMIN);
                    table.InsertEdge(EI_XMAX_ZMIN, FI_ZMIN);
                    table.InsertEdge(EI_YMIN_ZMIN, FI_ZMIN);
                    table.InsertEdge(EI_YMAX_ZMIN, FI_ZMIN);
                }
                break;
            }
            default:
                LogError("The level value cannot be an exact integer.");
            }
        }

        void GetZMaxEdgesS(const OctBox& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_ZMAX)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_ZMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMIN_ZMAX)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMAX_ZMAX)
            {
                faceType |= 0x08;
            }

            switch (faceType)
            {
            case 0:
                return;
            case 3:
                table.InsertEdge(EI_XMIN_ZMAX, EI_XMAX_ZMAX);
                break;
            case 5:
                table.InsertEdge(EI_XMIN_ZMAX, EI_YMIN_ZMAX);
                break;
            case 6:
                table.InsertEdge(EI_XMAX_ZMAX, EI_YMIN_ZMAX);
                break;
            case 9:
                table.InsertEdge(EI_XMIN_ZMAX, EI_YMAX_ZMAX);
                break;
            case 10:
                table.InsertEdge(EI_XMAX_ZMAX, EI_YMAX_ZMAX);
                break;
            case 12:
                table.InsertEdge(EI_YMIN_ZMAX, EI_YMAX_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = box.x0 + mSize * (box.y0 + mSize * box.z1);
                // F(x,y,z)
                int64_t f00 = static_cast<int64_t>(mInputVoxels[i]);
                i++;
                // F(x+1,y,z)
                int64_t f10 = static_cast<int64_t>(mInputVoxels[i]);
                i += mSize;
                // F(x+1,y+1,z)
                int64_t f11 = static_cast<int64_t>(mInputVoxels[i]);
                i--;
                // F(x,y+1,z)
                int64_t f01 = static_cast<int64_t>(mInputVoxels[i]);
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.InsertEdge(EI_XMIN_ZMAX, EI_YMIN_ZMAX);
                    table.InsertEdge(EI_XMAX_ZMAX, EI_YMAX_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.InsertEdge(EI_XMIN_ZMAX, EI_YMAX_ZMAX);
                    table.InsertEdge(EI_XMAX_ZMAX, EI_YMIN_ZMAX);
                }
                else
                {
                    // Plus-sign configuration, add branch point to
                    // tessellation.
                    table.Insert(FI_ZMAX,
                        table.GetVertex(EI_YMIN_ZMAX)[0],
                        table.GetVertex(EI_XMIN_ZMAX)[1],
                        table.GetVertex(EI_XMIN_ZMAX)[2]);

                    // Add edges sharing the branch point.
                    table.InsertEdge(EI_XMIN_ZMAX, FI_ZMAX);
                    table.InsertEdge(EI_XMAX_ZMAX, FI_ZMAX);
                    table.InsertEdge(EI_YMIN_ZMAX, FI_ZMAX);
                    table.InsertEdge(EI_YMAX_ZMAX, FI_ZMAX);
                }
                break;
            }
            default:
                LogError("The level value cannot be an exact integer.");
            }
        }

        // Edge extraction for merged boxes.
        class Sort0
        {
        public:
            bool operator()(Vertex const& arg0, Vertex const& arg1) const
            {
                if (arg0[0] < arg1[0])
                {
                    return true;
                }
                if (arg0[0] > arg1[0])
                {
                    return false;
                }
                if (arg0[1] < arg1[1])
                {
                    return true;
                }
                if (arg0[1] > arg1[1])
                {
                    return false;
                }
                return arg0[2] < arg1[2];
            }
        };

        class Sort1
        {
        public:
            bool operator()(Vertex const& arg0, Vertex const& arg1) const
            {
                if (arg0[2] < arg1[2])
                {
                    return true;
                }
                if (arg0[2] > arg1[2])
                {
                    return false;
                }
                if (arg0[0] < arg1[0])
                {
                    return true;
                }
                if (arg0[0] > arg1[0])
                {
                    return false;
                }
                return arg0[1] < arg1[1];
            }
        };

        class Sort2
        {
        public:
            bool operator()(Vertex const& arg0, Vertex const& arg1) const
            {
                if (arg0[1] < arg1[1])
                {
                    return true;
                }
                if (arg0[1] > arg1[1])
                {
                    return false;
                }
                if (arg0[2] < arg1[2])
                {
                    return true;
                }
                if (arg0[2] > arg1[2])
                {
                    return false;
                }
                return arg0[0] < arg1[0];
            }
        };

        void GetZMinEdgesM(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_ZMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_ZMIN)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMIN_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMAX_ZMIN)
            {
                faceType |= 0x08;
            }

            int end0 = 0, end1 = 0;
            switch (faceType)
            {
            case 0:
                return;
            case 3:
                end0 = EI_XMIN_ZMIN;
                end1 = EI_XMAX_ZMIN;
                break;
            case 5:
                end0 = EI_XMIN_ZMIN;
                end1 = EI_YMIN_ZMIN;
                break;
            case 6:
                end0 = EI_YMIN_ZMIN;
                end1 = EI_XMAX_ZMIN;
                break;
            case 9:
                end0 = EI_XMIN_ZMIN;
                end1 = EI_YMAX_ZMIN;
                break;
            case 10:
                end0 = EI_YMAX_ZMIN;
                end1 = EI_XMAX_ZMIN;
                break;
            case 12:
                end0 = EI_YMIN_ZMIN;
                end1 = EI_YMAX_ZMIN;
                break;
            default:
                LogError("The level value cannot be an exact integer.");
            }

            std::set<Vertex, Sort0> vSet;

            for (int x = box.x0 + 1; x < box.x1; ++x)
            {
                auto const& merge = mYMerge[x][box.z0];
                if (merge->IsZeroEdge(box.LY) || merge->HasZeroSubedge(box.LY))
                {
                    int root = merge->GetZeroBase(box.LY);
                    vSet.insert(Vertex{
                        static_cast<Real>(x),
                        GetYInterp(x, root, box.z0),
                        static_cast<Real>(box.z0) });
                }
            }

            for (int y = box.y0 + 1; y < box.y1; ++y)
            {
                auto const& merge = mXMerge[y][box.z0];
                if (merge->IsZeroEdge(box.LX) || merge->HasZeroSubedge(box.LX))
                {
                    int root = merge->GetZeroBase(box.LX);
                    vSet.insert(Vertex{
                        GetXInterp(root, y, box.z0),
                        static_cast<Real>(y),
                        static_cast<Real>(box.z0) });
                }
            }

            // Add subdivision.
            if (vSet.size() == 0)
            {
                table.InsertEdge(end0, end1);
                return;
            }

            Real v0x = std::floor((*vSet.begin())[0]);
            Real v1x = std::floor((*vSet.rbegin())[0]);
            Real e0x = std::floor(table.GetVertex(end0)[0]);
            Real e1x = std::floor(table.GetVertex(end1)[0]);
            if (e1x <= v0x && v1x <= e0x)
            {
                std::swap(end0, end1);
            }

            // Add vertices.
            int v0 = table.GetNumVertices(), v1 = v0;
            for (auto const& position : vSet)
            {
                table.Insert(position);
            }

            // Add edges.
            table.InsertEdge(end0, v1);
            ++v1;
            int const imax = static_cast<int>(vSet.size());
            for (int i = 1; i < imax; ++i, ++v0, ++v1)
            {
                table.InsertEdge(v0, v1);
            }
            table.InsertEdge(v0, end1);
        }

        void GetZMaxEdgesM(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_ZMAX)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_ZMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMIN_ZMAX)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMAX_ZMAX)
            {
                faceType |= 0x08;
            }

            int end0 = 0, end1 = 0;
            switch (faceType)
            {
            case 0:
                return;
            case 3:
                end0 = EI_XMIN_ZMAX;
                end1 = EI_XMAX_ZMAX;
                break;
            case 5:
                end0 = EI_XMIN_ZMAX;
                end1 = EI_YMIN_ZMAX;
                break;
            case 6:
                end0 = EI_YMIN_ZMAX;
                end1 = EI_XMAX_ZMAX;
                break;
            case 9:
                end0 = EI_XMIN_ZMAX;
                end1 = EI_YMAX_ZMAX;
                break;
            case 10:
                end0 = EI_YMAX_ZMAX;
                end1 = EI_XMAX_ZMAX;
                break;
            case 12:
                end0 = EI_YMIN_ZMAX;
                end1 = EI_YMAX_ZMAX;
                break;
            default:
                LogError("The level value cannot be an exact integer.");
            }

            std::set<Vertex, Sort0> vSet;

            for (int x = box.x0 + 1; x < box.x1; ++x)
            {
                auto const& merge = mYMerge[x][box.z1];
                if (merge->IsZeroEdge(box.LY) || merge->HasZeroSubedge(box.LY))
                {
                    int root = merge->GetZeroBase(box.LY);
                    vSet.insert(Vertex{
                        static_cast<Real>(x),
                        GetYInterp(x, root, box.z1),
                        static_cast<Real>(box.z1) });
                }
            }

            for (int y = box.y0 + 1; y < box.y1; ++y)
            {
                auto const& merge = mXMerge[y][box.z1];
                if (merge->IsZeroEdge(box.LX) || merge->HasZeroSubedge(box.LX))
                {
                    int root = merge->GetZeroBase(box.LX);
                    vSet.insert(Vertex{
                        GetXInterp(root, y, box.z1),
                        static_cast<Real>(y),
                        static_cast<Real>(box.z1) });
                }
            }

            // Add subdivision.
            int v0 = table.GetNumVertices(), v1 = v0;
            if (vSet.size() == 0)
            {
                table.InsertEdge(end0, end1);
                return;
            }

            Real v0x = std::floor((*vSet.begin())[0]);
            Real v1x = std::floor((*vSet.rbegin())[0]);
            Real e0x = std::floor(table.GetVertex(end0)[0]);
            Real e1x = std::floor(table.GetVertex(end1)[0]);
            if (e1x <= v0x && v1x <= e0x)
            {
                std::swap(end0, end1);
            }

            // Add vertices.
            for (auto const& position : vSet)
            {
                table.Insert(position);
            }

            // Add edges.
            table.InsertEdge(end0, v1);
            ++v1;
            int const imax = static_cast<int>(vSet.size());
            for (int i = 1; i < imax; ++i, ++v0, ++v1)
            {
                table.InsertEdge(v0, v1);
            }
            table.InsertEdge(v0, end1);
        }

        void GetYMinEdgesM(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_YMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_YMIN)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMIN_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMIN_ZMAX)
            {
                faceType |= 0x08;
            }

            int end0 = 0, end1 = 0;
            switch (faceType)
            {
            case 0:
                return;
            case 3:
                end0 = EI_XMIN_YMIN;
                end1 = EI_XMAX_YMIN;
                break;
            case 5:
                end0 = EI_XMIN_YMIN;
                end1 = EI_YMIN_ZMIN;
                break;
            case 6:
                end0 = EI_YMIN_ZMIN;
                end1 = EI_XMAX_YMIN;
                break;
            case 9:
                end0 = EI_XMIN_YMIN;
                end1 = EI_YMIN_ZMAX;
                break;
            case 10:
                end0 = EI_YMIN_ZMAX;
                end1 = EI_XMAX_YMIN;
                break;
            case 12:
                end0 = EI_YMIN_ZMIN;
                end1 = EI_YMIN_ZMAX;
                break;
            default:
                LogError("The level value cannot be an exact integer.");
            }

            std::set<Vertex, Sort1> vSet;

            for (int x = box.x0 + 1; x < box.x1; ++x)
            {
                auto const& merge = mZMerge[x][box.y0];
                if (merge->IsZeroEdge(box.LZ) || merge->HasZeroSubedge(box.LZ))
                {
                    int root = merge->GetZeroBase(box.LZ);
                    vSet.insert(Vertex{
                        static_cast<Real>(x),
                        static_cast<Real>(box.y0),
                        GetZInterp(x, box.y0, root) });
                }
            }

            for (int z = box.z0 + 1; z < box.z1; ++z)
            {
                auto const& merge = mXMerge[box.y0][z];
                if (merge->IsZeroEdge(box.LX) || merge->HasZeroSubedge(box.LX))
                {
                    int root = merge->GetZeroBase(box.LX);
                    vSet.insert(Vertex{
                        GetXInterp(root, box.y0, z),
                        static_cast<Real>(box.y0),
                        static_cast<Real>(z) });
                }
            }

            // Add subdivision.
            int v0 = table.GetNumVertices(), v1 = v0;
            if (vSet.size() == 0)
            {
                table.InsertEdge(end0, end1);
                return;
            }

            Real v0z = std::floor((*vSet.begin())[2]);
            Real v1z = std::floor((*vSet.rbegin())[2]);
            Real e0z = std::floor(table.GetVertex(end0)[2]);
            Real e1z = std::floor(table.GetVertex(end1)[2]);
            if (e1z <= v0z && v1z <= e0z)
            {
                std::swap(end0, end1);
            }

            // Add vertices.
            for (auto const& position : vSet)
            {
                table.Insert(position);
            }

            // Add edges.
            table.InsertEdge(end0, v1);
            ++v1;
            int const imax = static_cast<int>(vSet.size());
            for (int i = 1; i < imax; ++i, ++v0, ++v1)
            {
                table.InsertEdge(v0, v1);
            }
            table.InsertEdge(v0, end1);
        }

        void GetYMaxEdgesM(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_YMAX)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_YMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_YMAX_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_YMAX_ZMAX)
            {
                faceType |= 0x08;
            }

            int end0 = 0, end1 = 0;
            switch (faceType)
            {
            case 0:
                return;
            case 3:
                end0 = EI_XMIN_YMAX;
                end1 = EI_XMAX_YMAX;
                break;
            case 5:
                end0 = EI_XMIN_YMAX;
                end1 = EI_YMAX_ZMIN;
                break;
            case 6:
                end0 = EI_YMAX_ZMIN;
                end1 = EI_XMAX_YMAX;
                break;
            case 9:
                end0 = EI_XMIN_YMAX;
                end1 = EI_YMAX_ZMAX;
                break;
            case 10:
                end0 = EI_YMAX_ZMAX;
                end1 = EI_XMAX_YMAX;
                break;
            case 12:
                end0 = EI_YMAX_ZMIN;
                end1 = EI_YMAX_ZMAX;
                break;
            default:
                LogError("The level value cannot be an exact integer.");
            }

            std::set<Vertex, Sort1> vSet;

            for (int x = box.x0 + 1; x < box.x1; ++x)
            {
                auto const& merge = mZMerge[x][box.y1];
                if (merge->IsZeroEdge(box.LZ) || merge->HasZeroSubedge(box.LZ))
                {
                    int root = merge->GetZeroBase(box.LZ);
                    vSet.insert(Vertex{
                        static_cast<Real>(x),
                        static_cast<Real>(box.y1),
                        GetZInterp(x, box.y1, root) });
                }
            }

            for (int z = box.z0 + 1; z < box.z1; ++z)
            {
                auto const& merge = mXMerge[box.y1][z];
                if (merge->IsZeroEdge(box.LX) || merge->HasZeroSubedge(box.LX))
                {
                    int root = merge->GetZeroBase(box.LX);
                    vSet.insert(Vertex{
                        GetXInterp(root, box.y1, z),
                        static_cast<Real>(box.y1),
                        static_cast<Real>(z) });
                }
            }

            // Add subdivision.
            if (vSet.size() == 0)
            {
                table.InsertEdge(end0, end1);
                return;
            }

            Real v0z = std::floor((*vSet.begin())[2]);
            Real v1z = std::floor((*vSet.rbegin())[2]);
            Real e0z = std::floor(table.GetVertex(end0)[2]);
            Real e1z = std::floor(table.GetVertex(end1)[2]);
            if (e1z <= v0z && v1z <= e0z)
            {
                std::swap(end0, end1);
            }

            // Add vertices.
            int v0 = table.GetNumVertices(), v1 = v0;
            for (auto const& position : vSet)
            {
                table.Insert(position);
            }

            // Add edges.
            table.InsertEdge(end0, v1);
            ++v1;
            int const imax = static_cast<int>(vSet.size());
            for (int i = 1; i < imax; ++i, ++v0, ++v1)
            {
                table.InsertEdge(v0, v1);
            }
            table.InsertEdge(v0, end1);
        }

        void GetXMinEdgesM(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMIN_YMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMIN_YMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_XMIN_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_XMIN_ZMAX)
            {
                faceType |= 0x08;
            }

            int end0 = 0, end1 = 0;
            switch (faceType)
            {
            case 0:
                return;
            case 3:
                end0 = EI_XMIN_YMIN;
                end1 = EI_XMIN_YMAX;
                break;
            case 5:
                end0 = EI_XMIN_YMIN;
                end1 = EI_XMIN_ZMIN;
                break;
            case 6:
                end0 = EI_XMIN_ZMIN;
                end1 = EI_XMIN_YMAX;
                break;
            case 9:
                end0 = EI_XMIN_YMIN;
                end1 = EI_XMIN_ZMAX;
                break;
            case 10:
                end0 = EI_XMIN_ZMAX;
                end1 = EI_XMIN_YMAX;
                break;
            case 12:
                end0 = EI_XMIN_ZMIN;
                end1 = EI_XMIN_ZMAX;
                break;
            default:
                LogError("The level value cannot be an exact integer.");
            }

            std::set<Vertex, Sort2> vSet;

            for (int z = box.z0 + 1; z < box.z1; ++z)
            {
                auto const& merge = mYMerge[box.x0][z];
                if (merge->IsZeroEdge(box.LY) || merge->HasZeroSubedge(box.LY))
                {
                    int root = merge->GetZeroBase(box.LY);
                    vSet.insert(Vertex{
                        static_cast<Real>(box.x0),
                        GetYInterp(box.x0, root, z),
                        static_cast<Real>(z) });
                }
            }

            for (int y = box.y0 + 1; y < box.y1; ++y)
            {
                auto const& merge = mZMerge[box.x0][y];
                if (merge->IsZeroEdge(box.LZ) || merge->HasZeroSubedge(box.LZ))
                {
                    int root = merge->GetZeroBase(box.LZ);
                    vSet.insert(Vertex{
                        static_cast<Real>(box.x0),
                        static_cast<Real>(y),
                        GetZInterp(box.x0, y, root) });
                }
            }

            // Add subdivision.
            int v0 = table.GetNumVertices(), v1 = v0;
            if (vSet.size() == 0)
            {
                table.InsertEdge(end0, end1);
                return;
            }

            Real v0y = std::floor((*vSet.begin())[1]);
            Real v1y = std::floor((*vSet.rbegin())[1]);
            Real e0y = std::floor(table.GetVertex(end0)[1]);
            Real e1y = std::floor(table.GetVertex(end1)[1]);
            if (e1y <= v0y && v1y <= e0y)
            {
                std::swap(end0, end1);
            }

            // Add vertices.
            for (auto const& position : vSet)
            {
                table.Insert(position);
            }

            // Add edges.
            table.InsertEdge(end0, v1);
            ++v1;
            int const imax = static_cast<int>(vSet.size());
            for (int i = 1; i < imax; ++i, ++v0, ++v1)
            {
                table.InsertEdge(v0, v1);
            }
            table.InsertEdge(v0, end1);
        }

        void GetXMaxEdgesM(OctBox const& box, unsigned int type, VETable& table)
        {
            unsigned int faceType = 0;
            if (type & EB_XMAX_YMIN)
            {
                faceType |= 0x01;
            }
            if (type & EB_XMAX_YMAX)
            {
                faceType |= 0x02;
            }
            if (type & EB_XMAX_ZMIN)
            {
                faceType |= 0x04;
            }
            if (type & EB_XMAX_ZMAX)
            {
                faceType |= 0x08;
            }

            int end0 = 0, end1 = 0;
            switch (faceType)
            {
            case 0:
                return;
            case 3:
                end0 = EI_XMAX_YMIN;
                end1 = EI_XMAX_YMAX;
                break;
            case 5:
                end0 = EI_XMAX_YMIN;
                end1 = EI_XMAX_ZMIN;
                break;
            case 6:
                end0 = EI_XMAX_ZMIN;
                end1 = EI_XMAX_YMAX;
                break;
            case 9:
                end0 = EI_XMAX_YMIN;
                end1 = EI_XMAX_ZMAX;
                break;
            case 10:
                end0 = EI_XMAX_ZMAX;
                end1 = EI_XMAX_YMAX;
                break;
            case 12:
                end0 = EI_XMAX_ZMIN;
                end1 = EI_XMAX_ZMAX;
                break;
            default:
                LogError("The level value cannot be an exact integer.");
            }

            std::set<Vertex, Sort2> vSet;

            for (int z = box.z0 + 1; z < box.z1; ++z)
            {
                auto const& merge = mYMerge[box.x1][z];
                if (merge->IsZeroEdge(box.LY) || merge->HasZeroSubedge(box.LY))
                {
                    int root = merge->GetZeroBase(box.LY);
                    vSet.insert(Vertex{
                        static_cast<Real>(box.x1),
                        GetYInterp(box.x1, root, z),
                        static_cast<Real>(z) });
                }
            }

            for (int y = box.y0 + 1; y < box.y1; y++)
            {
                auto const& merge = mZMerge[box.x1][y];
                if (merge->IsZeroEdge(box.LZ) || merge->HasZeroSubedge(box.LZ))
                {
                    int root = merge->GetZeroBase(box.LZ);
                    vSet.insert(Vertex{
                        static_cast<Real>(box.x1),
                        static_cast<Real>(y),
                        GetZInterp(box.x1, y, root) });
                }
            }

            // Add subdivision.
            if (vSet.size() == 0)
            {
                table.InsertEdge(end0, end1);
                return;
            }

            Real v0y = std::floor((*vSet.begin())[1]);
            Real v1y = std::floor((*vSet.rbegin())[1]);
            Real e0y = std::floor(table.GetVertex(end0)[1]);
            Real e1y = std::floor(table.GetVertex(end1)[1]);
            if (e1y <= v0y && v1y <= e0y)
            {
                std::swap(end0, end1);
            }

            // Add vertices.
            int v0 = table.GetNumVertices(), v1 = v0;
            for (auto const& position : vSet)
            {
                table.Insert(position);
            }

            // Add edges.
            table.InsertEdge(end0, v1);
            ++v1;
            int const imax = static_cast<int>(vSet.size());
            for (int i = 1; i < imax; ++i, ++v0, ++v1)
            {
                table.InsertEdge(v0, v1);
            }
            table.InsertEdge(v0, end1);
        }

        // Support for normal vector calculations.
        Vertex GetGradient(Vertex const& position) const
        {
            Vertex vzero = { (Real)0, (Real)0, (Real)0 };
            int x = static_cast<int>(position[0]);
            if (x < 0 || x >= mTwoPowerN)
            {
                return vzero;
            }

            int y = static_cast<int>(position[1]);
            if (y < 0 || y >= mTwoPowerN)
            {
                return vzero;
            }

            int z = static_cast<int>(position[2]);
            if (z < 0 || z >= mTwoPowerN)
            {
                return vzero;
            }

            int i000 = x + mSize * (y + mSize * z);
            int i100 = i000 + 1;
            int i010 = i000 + mSize;
            int i110 = i100 + mSize;
            int i001 = i000 + mSizeSqr;
            int i101 = i100 + mSizeSqr;
            int i011 = i010 + mSizeSqr;
            int i111 = i110 + mSizeSqr;
            Real f000 = static_cast<Real>(mInputVoxels[i000]);
            Real f100 = static_cast<Real>(mInputVoxels[i100]);
            Real f010 = static_cast<Real>(mInputVoxels[i010]);
            Real f110 = static_cast<Real>(mInputVoxels[i110]);
            Real f001 = static_cast<Real>(mInputVoxels[i001]);
            Real f101 = static_cast<Real>(mInputVoxels[i101]);
            Real f011 = static_cast<Real>(mInputVoxels[i011]);
            Real f111 = static_cast<Real>(mInputVoxels[i111]);

            Real fx = position[0] - static_cast<Real>(x);
            Real fy = position[1] - static_cast<Real>(y);
            Real fz = position[2] - static_cast<Real>(z);
            Real oneMinusX = (Real)1 - fx;
            Real oneMinusY = (Real)1 - fy;
            Real oneMinusZ = (Real)1 - fz;
            Real tmp0, tmp1;
            Vertex gradient;

            tmp0 = oneMinusY * (f100 - f000) + fy * (f110 - f010);
            tmp1 = oneMinusY * (f101 - f001) + fy * (f111 - f011);
            gradient[0] = oneMinusZ * tmp0 + fz * tmp1;

            tmp0 = oneMinusX * (f010 - f000) + fx * (f110 - f100);
            tmp1 = oneMinusX * (f011 - f001) + fx * (f111 - f101);
            gradient[1] = oneMinusZ * tmp0 + fz * tmp1;

            tmp0 = oneMinusX * (f001 - f000) + fx * (f101 - f100);
            tmp1 = oneMinusX * (f011 - f010) + fx * (f111 - f110);
            gradient[2] = oneMinusY * tmp0 + oneMinusY * tmp1;

            return gradient;
        }

        enum
        {
            EI_XMIN_YMIN = 0,
            EI_XMIN_YMAX = 1,
            EI_XMAX_YMIN = 2,
            EI_XMAX_YMAX = 3,
            EI_XMIN_ZMIN = 4,
            EI_XMIN_ZMAX = 5,
            EI_XMAX_ZMIN = 6,
            EI_XMAX_ZMAX = 7,
            EI_YMIN_ZMIN = 8,
            EI_YMIN_ZMAX = 9,
            EI_YMAX_ZMIN = 10,
            EI_YMAX_ZMAX = 11,
            FI_XMIN = 12,
            FI_XMAX = 13,
            FI_YMIN = 14,
            FI_YMAX = 15,
            FI_ZMIN = 16,
            FI_ZMAX = 17,
            I_QUANTITY = 18,

            EB_XMIN_YMIN = 1 << EI_XMIN_YMIN,
            EB_XMIN_YMAX = 1 << EI_XMIN_YMAX,
            EB_XMAX_YMIN = 1 << EI_XMAX_YMIN,
            EB_XMAX_YMAX = 1 << EI_XMAX_YMAX,
            EB_XMIN_ZMIN = 1 << EI_XMIN_ZMIN,
            EB_XMIN_ZMAX = 1 << EI_XMIN_ZMAX,
            EB_XMAX_ZMIN = 1 << EI_XMAX_ZMIN,
            EB_XMAX_ZMAX = 1 << EI_XMAX_ZMAX,
            EB_YMIN_ZMIN = 1 << EI_YMIN_ZMIN,
            EB_YMIN_ZMAX = 1 << EI_YMIN_ZMAX,
            EB_YMAX_ZMIN = 1 << EI_YMAX_ZMIN,
            EB_YMAX_ZMAX = 1 << EI_YMAX_ZMAX,
            FB_XMIN = 1 << FI_XMIN,
            FB_XMAX = 1 << FI_XMAX,
            FB_YMIN = 1 << FI_YMIN,
            FB_YMAX = 1 << FI_YMAX,
            FB_ZMIN = 1 << FI_ZMIN,
            FB_ZMAX = 1 << FI_ZMAX
        };

        // image data
        int mTwoPowerN, mSize, mSizeSqr;
        T const* mInputVoxels;
        Real mLevel;

        bool mFixBoundary;

        // Trees for linear merging.
        Array2<std::shared_ptr<LinearMergeTree>> mXMerge, mYMerge, mZMerge;

        // monoboxes
        std::vector<OctBox> mBoxes;
    };
}
