// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/SurfaceExtractor.h>

// The level set extraction algorithm implemented here is described
// in Section 3.2 of the document
// https://www.geometrictools.com/Documentation/LevelSetExtraction.pdf

namespace gte
{
    // The image type T must be one of the integer types:  int8_t, int16_t,
    // int32_t, uint8_t, uint16_t or uint32_t.  Internal integer computations
    // are performed using int64_t.  The type Real is for extraction to
    // floating-point vertices.
    template <typename T, typename Real>
    class SurfaceExtractorCubes : public SurfaceExtractor<T, Real>
    {
    public:
        // Convenience type definitions.
        typedef typename SurfaceExtractor<T, Real>::Vertex Vertex;
        typedef typename SurfaceExtractor<T, Real>::Triangle Triangle;

        // The input is a 3D image with lexicographically ordered voxels
        // (x,y,z) stored in a linear array.  Voxel (x,y,z) is stored in the
        // array at location index = x + xBound * (y + yBound * z).  The
        // inputs xBound, yBound and zBound must each be 2 or larger so that
        // there is at least one image cube to process.  The inputVoxels must
        // be nonnull and point to contiguous storage that contains at least
        // xBound * yBound * zBound elements.
        SurfaceExtractorCubes(int xBound, int yBound, int zBound, T const* inputVoxels)
            :
            SurfaceExtractor<T, Real>(xBound, yBound, zBound, inputVoxels)
        {
        }

        // Extract level surfaces and return rational vertices.  Use the
        // base-class Extract if you want real-valued vertices.
        virtual void Extract(T level, std::vector<Vertex>& vertices,
            std::vector<Triangle>& triangles) override
        {
            // Adjust the image so that the level set is F(x,y,z) = 0.  The
            // precondition for 'level' is that it is not exactly a voxel
            // value.  However, T is an integer type, so we cannot pass in
            // a 'level' that has a fractional value.  To circumvent this,
            // the voxel values are doubled so that they are even integers.
            // The level value is doubled and 1 added to obtain an odd
            // integer, guaranteeing 'level' is not a voxel value.
            int64_t levelI64 = 2 * static_cast<int64_t>(level) + 1;
            for (size_t i = 0; i < this->mVoxels.size(); ++i)
            {
                int64_t inputI64 = 2 * static_cast<int64_t>(this->mInputVoxels[i]);
                this->mVoxels[i] = inputI64 - levelI64;
            }

            vertices.clear();
            triangles.clear();
            for (int z = 0; z < this->mZBound - 1; ++z)
            {
                for (int y = 0; y < this->mYBound - 1; ++y)
                {
                    for (int x = 0; x < this->mXBound - 1; ++x)
                    {
                        // Get vertices on edges of box (if any).
                        VETable table;
                        int type = GetVertices(x, y, z, table);
                        if (type != 0)
                        {
                            // Get edges on faces of box.
                            GetXMinEdges(x, y, z, type, table);
                            GetXMaxEdges(x, y, z, type, table);
                            GetYMinEdges(x, y, z, type, table);
                            GetYMaxEdges(x, y, z, type, table);
                            GetZMinEdges(x, y, z, type, table);
                            GetZMaxEdges(x, y, z, type, table);

                            // Ear-clip the wireframe mesh.
                            table.RemoveTriangles(vertices, triangles);
                        }
                    }
                }
            }
        }

    protected:
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

        // Vertex-edge-triangle table to support mesh topology.
        class VETable
        {
        public:
            VETable() = default;

            inline bool IsValidVertex(int i) const
            {
                return mVertex[i].valid;
            }

            inline int64_t GetXN(int i) const
            {
                return mVertex[i].pos.xNumer;
            }

            inline int64_t GetXD(int i) const
            {
                return mVertex[i].pos.xDenom;
            }

            inline int64_t GetYN(int i) const
            {
                return mVertex[i].pos.yNumer;
            }

            inline int64_t GetYD(int i) const
            {
                return mVertex[i].pos.yDenom;
            }

            inline int64_t GetZN(int i) const
            {
                return mVertex[i].pos.zNumer;
            }

            inline int64_t GetZD(int i) const
            {
                return mVertex[i].pos.zDenom;
            }

            void Insert(int i, Vertex const& pos)
            {
                TVertex& vertex = mVertex[i];
                vertex.pos = pos;
                vertex.valid = true;
            }

            void Insert(int i0, int i1)
            {
                TVertex& vertex0 = mVertex[i0];
                TVertex& vertex1 = mVertex[i1];
                vertex0.adj[vertex0.numAdjacents++] = i1;
                vertex1.adj[vertex1.numAdjacents++] = i0;
            }

            void RemoveTriangles(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles)
            {
                // Ear-clip the wireframe to get the triangles.
                Triangle triangle;
                while (Remove(triangle))
                {
                    int v0 = static_cast<int>(vertices.size());
                    int v1 = v0 + 1;
                    int v2 = v1 + 1;
                    triangles.push_back(Triangle(v0, v1, v2));
                    vertices.push_back(mVertex[triangle.v[0]].pos);
                    vertices.push_back(mVertex[triangle.v[1]].pos);
                    vertices.push_back(mVertex[triangle.v[2]].pos);
                }
            }

        protected:
            void RemoveVertex(int i)
            {
                TVertex& vertex0 = mVertex[i];
                // assert:  vertex0.numAdjacents == 2
                int a0 = vertex0.adj[0];
                int a1 = vertex0.adj[1];
                TVertex& adjVertex0 = mVertex[a0];
                TVertex& adjVertex1 = mVertex[a1];

                int j;
                for (j = 0; j < adjVertex0.numAdjacents; ++j)
                {
                    if (adjVertex0.adj[j] == i)
                    {
                        adjVertex0.adj[j] = a1;
                        break;
                    }
                }
                // assert: j != adjVertex0.numAdjacents

                for (j = 0; j < adjVertex1.numAdjacents; j++)
                {
                    if (adjVertex1.adj[j] == i)
                    {
                        adjVertex1.adj[j] = a0;
                        break;
                    }
                }
                // assert: j != adjVertex1.numAdjacents

                vertex0.valid = false;

                if (adjVertex0.numAdjacents == 2)
                {
                    if (adjVertex0.adj[0] == adjVertex0.adj[1])
                    {
                        adjVertex0.valid = false;
                    }
                }

                if (adjVertex1.numAdjacents == 2)
                {
                    if (adjVertex1.adj[0] == adjVertex1.adj[1])
                    {
                        adjVertex1.valid = false;
                    }
                }
            }

            bool Remove(Triangle& triangle)
            {
                for (int i = 0; i < 18; ++i)
                {
                    TVertex& vertex = mVertex[i];
                    if (vertex.valid && vertex.numAdjacents == 2)
                    {
                        triangle.v[0] = i;
                        triangle.v[1] = vertex.adj[0];
                        triangle.v[2] = vertex.adj[1];
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
                    numAdjacents(0),
                    valid(false)
                {
                }

                Vertex pos;
                int numAdjacents;
                std::array<int, 4> adj;
                bool valid;
            };

            std::array<TVertex, 18> mVertex;
        };

        virtual std::array<Real, 3> GetGradient(std::array<Real, 3> const& pos) override
        {
            std::array<Real, 3> const zero{ (Real)0, (Real)0, (Real)0 };

            int x = static_cast<int>(pos[0]);
            if (x < 0 || x + 1 >= this->mXBound)
            {
                return zero;
            }

            int y = static_cast<int>(pos[1]);
            if (y < 0 || y + 1 >= this->mYBound)
            {
                return zero;
            }

            int z = static_cast<int>(pos[2]);
            if (z < 0 || z + 1 >= this->mZBound)
            {
                return zero;
            }

            // Get image values at corners of voxel.
            int i000 = x + this->mXBound * (y + this->mYBound * z);
            int i100 = i000 + 1;
            int i010 = i000 + this->mXBound;
            int i110 = i010 + 1;
            int i001 = i000 + this->mXYBound;
            int i101 = i001 + 1;
            int i011 = i001 + this->mXBound;
            int i111 = i011 + 1;
            Real f000 = static_cast<Real>(this->mVoxels[i000]);
            Real f100 = static_cast<Real>(this->mVoxels[i100]);
            Real f010 = static_cast<Real>(this->mVoxels[i010]);
            Real f110 = static_cast<Real>(this->mVoxels[i110]);
            Real f001 = static_cast<Real>(this->mVoxels[i001]);
            Real f101 = static_cast<Real>(this->mVoxels[i101]);
            Real f011 = static_cast<Real>(this->mVoxels[i011]);
            Real f111 = static_cast<Real>(this->mVoxels[i111]);

            Real dx = pos[0] - static_cast<Real>(x);
            Real dy = pos[1] - static_cast<Real>(y);
            Real dz = pos[2] - static_cast<Real>(z);
            Real oneMX = 1.0f - dx;
            Real oneMY = 1.0f - dy;
            Real oneMZ = 1.0f - dz;

            std::array<Real, 3> grad;

            Real tmp0 = oneMY * (f100 - f000) + dy * (f110 - f010);
            Real tmp1 = oneMY * (f101 - f001) + dy * (f111 - f011);
            grad[0] = oneMZ * tmp0 + dz * tmp1;

            tmp0 = oneMX * (f010 - f000) + dx * (f110 - f100);
            tmp1 = oneMX * (f011 - f001) + dx * (f111 - f101);
            grad[1] = oneMZ * tmp0 + dz * tmp1;

            tmp0 = oneMX * (f001 - f000) + dx * (f101 - f100);
            tmp1 = oneMX * (f011 - f010) + dx * (f111 - f110);
            grad[2] = oneMY * tmp0 + dy * tmp1;

            return grad;
        }

        int GetVertices(int x, int y, int z, VETable& table)
        {
            int type = 0;

            // Get the image values at the corners of the voxel.
            int i000 = x + this->mXBound * (y + this->mYBound * z);
            int i100 = i000 + 1;
            int i010 = i000 + this->mXBound;
            int i110 = i010 + 1;
            int i001 = i000 + this->mXYBound;
            int i101 = i001 + 1;
            int i011 = i001 + this->mXBound;
            int i111 = i011 + 1;
            int64_t f000 = this->mVoxels[i000];
            int64_t f100 = this->mVoxels[i100];
            int64_t f010 = this->mVoxels[i010];
            int64_t f110 = this->mVoxels[i110];
            int64_t f001 = this->mVoxels[i001];
            int64_t f101 = this->mVoxels[i101];
            int64_t f011 = this->mVoxels[i011];
            int64_t f111 = this->mVoxels[i111];

            int64_t x0 = x, x1 = x + 1, y0 = y, y1 = y + 1, z0 = z, z1 = z + 1;
            int64_t d;

            // xmin-ymin edge
            if (f000 * f001 < 0)
            {
                type |= EB_XMIN_YMIN;
                d = f001 - f000;
                table.Insert(EI_XMIN_YMIN, Vertex(x0, 1, y0, 1, z0 * d - f000, d));
            }

            // xmin-ymax edge
            if (f010 * f011 < 0)
            {
                type |= EB_XMIN_YMAX;
                d = f011 - f010;
                table.Insert(EI_XMIN_YMAX, Vertex(x0, 1, y1, 1, z0 * d - f010, d));
            }

            // xmax-ymin edge
            if (f100 * f101 < 0)
            {
                type |= EB_XMAX_YMIN;
                d = f101 - f100;
                table.Insert(EI_XMAX_YMIN, Vertex(x1, 1, y0, 1, z0 * d - f100, d));
            }

            // xmax-ymax edge
            if (f110 * f111 < 0)
            {
                type |= EB_XMAX_YMAX;
                d = f111 - f110;
                table.Insert(EI_XMAX_YMAX, Vertex(x1, 1, y1, 1, z0 * d - f110, d));
            }

            // xmin-zmin edge
            if (f000 * f010 < 0)
            {
                type |= EB_XMIN_ZMIN;
                d = f010 - f000;
                table.Insert(EI_XMIN_ZMIN, Vertex(x0, 1, y0 * d - f000, d, z0, 1));
            }

            // xmin-zmax edge
            if (f001 * f011 < 0)
            {
                type |= EB_XMIN_ZMAX;
                d = f011 - f001;
                table.Insert(EI_XMIN_ZMAX, Vertex(x0, 1, y0 * d - f001, d, z1, 1));
            }

            // xmax-zmin edge
            if (f100 * f110 < 0)
            {
                type |= EB_XMAX_ZMIN;
                d = f110 - f100;
                table.Insert(EI_XMAX_ZMIN, Vertex(x1, 1, y0 * d - f100, d, z0, 1));
            }

            // xmax-zmax edge
            if (f101 * f111 < 0)
            {
                type |= EB_XMAX_ZMAX;
                d = f111 - f101;
                table.Insert(EI_XMAX_ZMAX, Vertex(x1, 1, y0 * d - f101, d, z1, 1));
            }

            // ymin-zmin edge
            if (f000 * f100 < 0)
            {
                type |= EB_YMIN_ZMIN;
                d = f100 - f000;
                table.Insert(EI_YMIN_ZMIN, Vertex(x0 * d - f000, d, y0, 1, z0, 1));
            }

            // ymin-zmax edge
            if (f001 * f101 < 0)
            {
                type |= EB_YMIN_ZMAX;
                d = f101 - f001;
                table.Insert(EI_YMIN_ZMAX, Vertex(x0 * d - f001, d, y0, 1, z1, 1));
            }

            // ymax-zmin edge
            if (f010 * f110 < 0)
            {
                type |= EB_YMAX_ZMIN;
                d = f110 - f010;
                table.Insert(EI_YMAX_ZMIN, Vertex(x0 * d - f010, d, y1, 1, z0, 1));
            }

            // ymax-zmax edge
            if (f011 * f111 < 0)
            {
                type |= EB_YMAX_ZMAX;
                d = f111 - f011;
                table.Insert(EI_YMAX_ZMAX, Vertex(x0 * d - f011, d, y1, 1, z1, 1));
            }

            return type;
        }

        void GetXMinEdges(int x, int y, int z, int type, VETable& table)
        {
            int faceType = 0;
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
                break;
            case 3:
                table.Insert(EI_XMIN_YMIN, EI_XMIN_YMAX);
                break;
            case 5:
                table.Insert(EI_XMIN_YMIN, EI_XMIN_ZMIN);
                break;
            case 6:
                table.Insert(EI_XMIN_YMAX, EI_XMIN_ZMIN);
                break;
            case 9:
                table.Insert(EI_XMIN_YMIN, EI_XMIN_ZMAX);
                break;
            case 10:
                table.Insert(EI_XMIN_YMAX, EI_XMIN_ZMAX);
                break;
            case 12:
                table.Insert(EI_XMIN_ZMIN, EI_XMIN_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = x + this->mXBound * (y + this->mYBound * z);
                // F(x,y,z)
                int64_t f00 = this->mVoxels[i];
                i += this->mXBound;
                // F(x,y+1,z)
                int64_t f10 = this->mVoxels[i];
                i += this->mXYBound;
                // F(x,y+1,z+1)
                int64_t f11 = this->mVoxels[i];
                i -= this->mXBound;
                // F(x,y,z+1)
                int64_t f01 = this->mVoxels[i];
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.Insert(EI_XMIN_YMIN, EI_XMIN_ZMIN);
                    table.Insert(EI_XMIN_YMAX, EI_XMIN_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.Insert(EI_XMIN_YMIN, EI_XMIN_ZMAX);
                    table.Insert(EI_XMIN_YMAX, EI_XMIN_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to tessellation.
                    table.Insert(FI_XMIN, Vertex(
                        table.GetXN(EI_XMIN_ZMIN), table.GetXD(EI_XMIN_ZMIN),
                        table.GetYN(EI_XMIN_ZMIN), table.GetYD(EI_XMIN_ZMIN),
                        table.GetZN(EI_XMIN_YMIN), table.GetZD(EI_XMIN_YMIN)));

                    // Add edges sharing the branch point.
                    table.Insert(EI_XMIN_YMIN, FI_XMIN);
                    table.Insert(EI_XMIN_YMAX, FI_XMIN);
                    table.Insert(EI_XMIN_ZMIN, FI_XMIN);
                    table.Insert(EI_XMIN_ZMAX, FI_XMIN);
                }
                break;
            }
            default:
                LogError("Unexpected condition.");
            }
        }

        void GetXMaxEdges(int x, int y, int z, int type, VETable& table)
        {
            int faceType = 0;
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
                break;
            case 3:
                table.Insert(EI_XMAX_YMIN, EI_XMAX_YMAX);
                break;
            case 5:
                table.Insert(EI_XMAX_YMIN, EI_XMAX_ZMIN);
                break;
            case 6:
                table.Insert(EI_XMAX_YMAX, EI_XMAX_ZMIN);
                break;
            case 9:
                table.Insert(EI_XMAX_YMIN, EI_XMAX_ZMAX);
                break;
            case 10:
                table.Insert(EI_XMAX_YMAX, EI_XMAX_ZMAX);
                break;
            case 12:
                table.Insert(EI_XMAX_ZMIN, EI_XMAX_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = (x + 1) + this->mXBound * (y + this->mYBound * z);
                // F(x,y,z)
                int64_t f00 = this->mVoxels[i];
                i += this->mXBound;
                // F(x,y+1,z)
                int64_t f10 = this->mVoxels[i];
                i += this->mXYBound;
                // F(x,y+1,z+1)
                int64_t f11 = this->mVoxels[i];
                i -= this->mXBound;
                // F(x,y,z+1)
                int64_t f01 = this->mVoxels[i];
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.Insert(EI_XMAX_YMIN, EI_XMAX_ZMIN);
                    table.Insert(EI_XMAX_YMAX, EI_XMAX_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.Insert(EI_XMAX_YMIN, EI_XMAX_ZMAX);
                    table.Insert(EI_XMAX_YMAX, EI_XMAX_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to tessellation.
                    table.Insert(FI_XMAX, Vertex(
                        table.GetXN(EI_XMAX_ZMIN), table.GetXD(EI_XMAX_ZMIN),
                        table.GetYN(EI_XMAX_ZMIN), table.GetYD(EI_XMAX_ZMIN),
                        table.GetZN(EI_XMAX_YMIN), table.GetZD(EI_XMAX_YMIN)));

                    // Add edges sharing the branch point.
                    table.Insert(EI_XMAX_YMIN, FI_XMAX);
                    table.Insert(EI_XMAX_YMAX, FI_XMAX);
                    table.Insert(EI_XMAX_ZMIN, FI_XMAX);
                    table.Insert(EI_XMAX_ZMAX, FI_XMAX);
                }
                break;
            }
            default:
                LogError("Unexpected condition.");
            }
        }

        void GetYMinEdges(int x, int y, int z, int type, VETable& table)
        {
            int faceType = 0;
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
                break;
            case 3:
                table.Insert(EI_XMIN_YMIN, EI_XMAX_YMIN);
                break;
            case 5:
                table.Insert(EI_XMIN_YMIN, EI_YMIN_ZMIN);
                break;
            case 6:
                table.Insert(EI_XMAX_YMIN, EI_YMIN_ZMIN);
                break;
            case 9:
                table.Insert(EI_XMIN_YMIN, EI_YMIN_ZMAX);
                break;
            case 10:
                table.Insert(EI_XMAX_YMIN, EI_YMIN_ZMAX);
                break;
            case 12:
                table.Insert(EI_YMIN_ZMIN, EI_YMIN_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = x + this->mXBound * (y + this->mYBound * z);
                // F(x,y,z)
                int64_t f00 = this->mVoxels[i];
                ++i;
                // F(x+1,y,z)
                int64_t f10 = this->mVoxels[i];
                i += this->mXYBound;
                // F(x+1,y,z+1)
                int64_t f11 = this->mVoxels[i];
                --i;
                // F(x,y,z+1)
                int64_t f01 = this->mVoxels[i];
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.Insert(EI_XMIN_YMIN, EI_YMIN_ZMIN);
                    table.Insert(EI_XMAX_YMIN, EI_YMIN_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.Insert(EI_XMIN_YMIN, EI_YMIN_ZMAX);
                    table.Insert(EI_XMAX_YMIN, EI_YMIN_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to tessellation.
                    table.Insert(FI_YMIN, Vertex(
                        table.GetXN(EI_YMIN_ZMIN), table.GetXD(EI_YMIN_ZMIN),
                        table.GetYN(EI_XMIN_YMIN), table.GetYD(EI_XMIN_YMIN),
                        table.GetZN(EI_XMIN_YMIN), table.GetZD(EI_XMIN_YMIN)));

                    // Add edges sharing the branch point.
                    table.Insert(EI_XMIN_YMIN, FI_YMIN);
                    table.Insert(EI_XMAX_YMIN, FI_YMIN);
                    table.Insert(EI_YMIN_ZMIN, FI_YMIN);
                    table.Insert(EI_YMIN_ZMAX, FI_YMIN);
                }
                break;
            }
            default:
                LogError("Unexpected condition.");
            }
        }

        void GetYMaxEdges(int x, int y, int z, int type, VETable& table)
        {
            int faceType = 0;
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
                break;
            case 3:
                table.Insert(EI_XMIN_YMAX, EI_XMAX_YMAX);
                break;
            case 5:
                table.Insert(EI_XMIN_YMAX, EI_YMAX_ZMIN);
                break;
            case 6:
                table.Insert(EI_XMAX_YMAX, EI_YMAX_ZMIN);
                break;
            case 9:
                table.Insert(EI_XMIN_YMAX, EI_YMAX_ZMAX);
                break;
            case 10:
                table.Insert(EI_XMAX_YMAX, EI_YMAX_ZMAX);
                break;
            case 12:
                table.Insert(EI_YMAX_ZMIN, EI_YMAX_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = x + this->mXBound * ((y + 1) + this->mYBound * z);
                // F(x,y,z)
                int64_t f00 = this->mVoxels[i];
                ++i;
                // F(x+1,y,z)
                int64_t f10 = this->mVoxels[i];
                i += this->mXYBound;
                // F(x+1,y,z+1)
                int64_t f11 = this->mVoxels[i];
                --i;
                // F(x,y,z+1)
                int64_t f01 = this->mVoxels[i];
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.Insert(EI_XMIN_YMAX, EI_YMAX_ZMIN);
                    table.Insert(EI_XMAX_YMAX, EI_YMAX_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.Insert(EI_XMIN_YMAX, EI_YMAX_ZMAX);
                    table.Insert(EI_XMAX_YMAX, EI_YMAX_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to tessellation.
                    table.Insert(FI_YMAX, Vertex(
                        table.GetXN(EI_YMAX_ZMIN), table.GetXD(EI_YMAX_ZMIN),
                        table.GetYN(EI_XMIN_YMAX), table.GetYD(EI_XMIN_YMAX),
                        table.GetZN(EI_XMIN_YMAX), table.GetZD(EI_XMIN_YMAX)));

                    // Add edges sharing the branch point.
                    table.Insert(EI_XMIN_YMAX, FI_YMAX);
                    table.Insert(EI_XMAX_YMAX, FI_YMAX);
                    table.Insert(EI_YMAX_ZMIN, FI_YMAX);
                    table.Insert(EI_YMAX_ZMAX, FI_YMAX);
                }
                break;
            }
            default:
                LogError("Unexpected condition.");
            }
        }

        void GetZMinEdges(int x, int y, int z, int type, VETable& table)
        {
            int faceType = 0;
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
                break;
            case 3:
                table.Insert(EI_XMIN_ZMIN, EI_XMAX_ZMIN);
                break;
            case 5:
                table.Insert(EI_XMIN_ZMIN, EI_YMIN_ZMIN);
                break;
            case 6:
                table.Insert(EI_XMAX_ZMIN, EI_YMIN_ZMIN);
                break;
            case 9:
                table.Insert(EI_XMIN_ZMIN, EI_YMAX_ZMIN);
                break;
            case 10:
                table.Insert(EI_XMAX_ZMIN, EI_YMAX_ZMIN);
                break;
            case 12:
                table.Insert(EI_YMIN_ZMIN, EI_YMAX_ZMIN);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = x + this->mXBound * (y + this->mYBound * z);
                // F(x,y,z)
                int64_t f00 = this->mVoxels[i];
                ++i;
                // F(x+1,y,z)
                int64_t f10 = this->mVoxels[i];
                i += this->mXBound;
                // F(x+1,y+1,z)
                int64_t f11 = this->mVoxels[i];
                --i;
                // F(x,y+1,z)
                int64_t f01 = this->mVoxels[i];
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.Insert(EI_XMIN_ZMIN, EI_YMIN_ZMIN);
                    table.Insert(EI_XMAX_ZMIN, EI_YMAX_ZMIN);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.Insert(EI_XMIN_ZMIN, EI_YMAX_ZMIN);
                    table.Insert(EI_XMAX_ZMIN, EI_YMIN_ZMIN);
                }
                else
                {
                    // Plus-sign configuration, add branch point to tessellation.
                    table.Insert(FI_ZMIN, Vertex(
                        table.GetXN(EI_YMIN_ZMIN), table.GetXD(EI_YMIN_ZMIN),
                        table.GetYN(EI_XMIN_ZMIN), table.GetYD(EI_XMIN_ZMIN),
                        table.GetZN(EI_XMIN_ZMIN), table.GetZD(EI_XMIN_ZMIN)));

                    // Add edges sharing the branch point.
                    table.Insert(EI_XMIN_ZMIN, FI_ZMIN);
                    table.Insert(EI_XMAX_ZMIN, FI_ZMIN);
                    table.Insert(EI_YMIN_ZMIN, FI_ZMIN);
                    table.Insert(EI_YMAX_ZMIN, FI_ZMIN);
                }
                break;
            }
            default:
                LogError("Unexpected condition.");
            }
        }

        void GetZMaxEdges(int x, int y, int z, int type, VETable& table)
        {
            int faceType = 0;
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
                break;
            case 3:
                table.Insert(EI_XMIN_ZMAX, EI_XMAX_ZMAX);
                break;
            case 5:
                table.Insert(EI_XMIN_ZMAX, EI_YMIN_ZMAX);
                break;
            case 6:
                table.Insert(EI_XMAX_ZMAX, EI_YMIN_ZMAX);
                break;
            case 9:
                table.Insert(EI_XMIN_ZMAX, EI_YMAX_ZMAX);
                break;
            case 10:
                table.Insert(EI_XMAX_ZMAX, EI_YMAX_ZMAX);
                break;
            case 12:
                table.Insert(EI_YMIN_ZMAX, EI_YMAX_ZMAX);
                break;
            case 15:
            {
                // Four vertices, one per edge, need to disambiguate.
                int i = x + this->mXBound * (y + this->mYBound * (z + 1));
                // F(x,y,z)
                int64_t f00 = this->mVoxels[i];
                ++i;
                // F(x+1,y,z)
                int64_t f10 = this->mVoxels[i];
                i += this->mXBound;
                // F(x+1,y+1,z)
                int64_t f11 = this->mVoxels[i];
                --i;
                // F(x,y+1,z)
                int64_t f01 = this->mVoxels[i];
                int64_t det = f00 * f11 - f01 * f10;

                if (det > 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P2>, <P1,P3>.
                    table.Insert(EI_XMIN_ZMAX, EI_YMIN_ZMAX);
                    table.Insert(EI_XMAX_ZMAX, EI_YMAX_ZMAX);
                }
                else if (det < 0)
                {
                    // Disjoint hyperbolic segments, pair <P0,P3>, <P1,P2>.
                    table.Insert(EI_XMIN_ZMAX, EI_YMAX_ZMAX);
                    table.Insert(EI_XMAX_ZMAX, EI_YMIN_ZMAX);
                }
                else
                {
                    // Plus-sign configuration, add branch point to tessellation.
                    table.Insert(FI_ZMAX, Vertex(
                        table.GetXN(EI_YMIN_ZMAX), table.GetXD(EI_YMIN_ZMAX),
                        table.GetYN(EI_XMIN_ZMAX), table.GetYD(EI_XMIN_ZMAX),
                        table.GetZN(EI_XMIN_ZMAX), table.GetZD(EI_XMIN_ZMAX)));

                    // Add edges sharing the branch point.
                    table.Insert(EI_XMIN_ZMAX, FI_ZMAX);
                    table.Insert(EI_XMAX_ZMAX, FI_ZMAX);
                    table.Insert(EI_YMIN_ZMAX, FI_ZMAX);
                    table.Insert(EI_YMAX_ZMAX, FI_ZMAX);
                }
                break;
            }
            default:
                LogError("Unexpected condition.");
            }
        }
    };
}
