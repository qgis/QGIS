// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/SurfaceExtractor.h>
#include <set>

// The level set extraction algorithm implemented here is described
// in the document
// https://www.geometrictools.com/Documentation/ExtractLevelSurfaces.pdf

namespace gte
{
    // The image type T must be one of the integer types:  int8_t, int16_t,
    // int32_t, uint8_t, uint16_t or uint32_t.  Internal integer computations
    // are performed using int64_t.  The type Real is for extraction to
    // floating-point vertices.
    template <typename T, typename Real>
    class SurfaceExtractorTetrahedra : public SurfaceExtractor<T, Real>
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
        SurfaceExtractorTetrahedra(int xBound, int yBound, int zBound, T const* inputVoxels)
            :
            SurfaceExtractor<T, Real>(xBound, yBound, zBound, inputVoxels),
            mNextVertex(0)
        {
        }

        // Extract level surfaces and return rational vertices.  Use the
        // base-class Extract if you want real-valued vertices.
        virtual void Extract(T level, std::vector<Vertex>& vertices,
            std::vector<Triangle>& triangles) override
        {
            // Adjust the image so that the level set is F(x,y,z) = 0.
            int64_t levelI64 = static_cast<int64_t>(level);
            for (size_t i = 0; i < this->mVoxels.size(); ++i)
            {
                int64_t inputI64 = static_cast<int64_t>(this->mInputVoxels[i]);
                this->mVoxels[i] = inputI64 - levelI64;
            }

            mVMap.clear();
            mESet.clear();
            mTSet.clear();
            mNextVertex = 0;
            vertices.clear();
            triangles.clear();
            for (int z = 0, zp = 1; zp < this->mZBound; ++z, ++zp)
            {
                int zParity = (z & 1);
                for (int y = 0, yp = 1; yp < this->mYBound; ++y, ++yp)
                {
                    int yParity = (y & 1);
                    for (int x = 0, xp = 1; xp < this->mXBound; ++x, ++xp)
                    {
                        int xParity = (x & 1);

                        int i000 = x + this->mXBound * (y + this->mYBound * z);
                        int i100 = i000 + 1;
                        int i010 = i000 + this->mXBound;
                        int i110 = i010 + 1;
                        int i001 = i000 + this->mXYBound;
                        int i101 = i001 + 1;
                        int i011 = i001 + this->mXBound;
                        int i111 = i011 + 1;
                        int64_t f000 = static_cast<int64_t>(this->mVoxels[i000]);
                        int64_t f100 = static_cast<int64_t>(this->mVoxels[i100]);
                        int64_t f010 = static_cast<int64_t>(this->mVoxels[i010]);
                        int64_t f110 = static_cast<int64_t>(this->mVoxels[i110]);
                        int64_t f001 = static_cast<int64_t>(this->mVoxels[i001]);
                        int64_t f101 = static_cast<int64_t>(this->mVoxels[i101]);
                        int64_t f011 = static_cast<int64_t>(this->mVoxels[i011]);
                        int64_t f111 = static_cast<int64_t>(this->mVoxels[i111]);

                        if (xParity ^ yParity ^ zParity)
                        {
                            // 1205
                            ProcessTetrahedron(
                                xp, y, z, f100,
                                xp, yp, z, f110,
                                x, y, z, f000,
                                xp, y, zp, f101);

                            // 3027
                            ProcessTetrahedron(
                                x, yp, z, f010,
                                x, y, z, f000,
                                xp, yp, z, f110,
                                x, yp, zp, f011);

                            // 4750
                            ProcessTetrahedron(
                                x, y, zp, f001,
                                x, yp, zp, f011,
                                xp, y, zp, f101,
                                x, y, z, f000);

                            // 6572
                            ProcessTetrahedron(
                                xp, yp, zp, f111,
                                xp, y, zp, f101,
                                x, yp, zp, f011,
                                xp, yp, z, f110);

                            // 0752
                            ProcessTetrahedron(
                                x, y, z, f000,
                                x, yp, zp, f011,
                                xp, y, zp, f101,
                                xp, yp, z, f110);
                        }
                        else
                        {
                            // 0134
                            ProcessTetrahedron(
                                x, y, z, f000,
                                xp, y, z, f100,
                                x, yp, z, f010,
                                x, y, zp, f001);

                            // 2316
                            ProcessTetrahedron(
                                xp, yp, z, f110,
                                x, yp, z, f010,
                                xp, y, z, f100,
                                xp, yp, zp, f111);

                            // 5461
                            ProcessTetrahedron(
                                xp, y, zp, f101,
                                x, y, zp, f001,
                                xp, yp, zp, f111,
                                xp, y, z, f100);

                            // 7643
                            ProcessTetrahedron(
                                x, yp, zp, f011,
                                xp, yp, zp, f111,
                                x, y, zp, f001,
                                x, yp, z, f010);

                            // 6314
                            ProcessTetrahedron(
                                xp, yp, zp, f111,
                                x, yp, z, f010,
                                xp, y, z, f100,
                                x, y, zp, f001);
                        }
                    }
                }
            }

            // Pack vertices into an array.
            vertices.resize(mVMap.size());
            for (auto const& element : mVMap)
            {
                vertices[element.second] = element.first;
            }

            // Pack edges into an array (computed, but not reported to
            // caller).
            std::vector<Edge> edges(mESet.size());
            size_t i = 0;
            for (auto const& element : mESet)
            {
                edges[i++] = element;
            }

            // Pack triangles into an array.
            triangles.resize(mTSet.size());
            i = 0;
            for (auto const& element : mTSet)
            {
                triangles[i++] = element;
            }
        }

    protected:
        struct Edge
        {
            Edge() = default;

            Edge(int v0, int v1)
            {
                if (v0 < v1)
                {
                    v[0] = v0;
                    v[1] = v1;
                }
                else
                {
                    v[0] = v1;
                    v[1] = v0;
                }
            }

            bool operator==(Edge const& other) const
            {
                return v[0] == other.v[0] && v[1] == other.v[1];
            }

            bool operator<(Edge const& other) const
            {
                for (int i = 0; i < 2; ++i)
                {
                    if (v[i] < other.v[i])
                    {
                        return true;
                    }
                    if (v[i] > other.v[i])
                    {
                        return false;
                    }
                }
                return false;
            }

            std::array<int, 2> v;
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

            std::array<Real, 3> grad;

            if ((x & 1) ^ (y & 1) ^ (z & 1))
            {
                if (dx - dy - dz >= (Real)0)
                {
                    // 1205
                    grad[0] = +f100 - f000;
                    grad[1] = -f100 + f110;
                    grad[2] = -f100 + f101;
                }
                else if (dx - dy + dz <= (Real)0)
                {
                    // 3027
                    grad[0] = -f010 + f110;
                    grad[1] = +f010 - f000;
                    grad[2] = -f010 + f011;
                }
                else if (dx + dy - dz <= (Real)0)
                {
                    // 4750
                    grad[0] = -f001 + f101;
                    grad[1] = -f001 + f011;
                    grad[2] = +f001 - f000;
                }
                else if (dx + dy + dz >= (Real)0)
                {
                    // 6572
                    grad[0] = +f111 - f011;
                    grad[1] = +f111 - f101;
                    grad[2] = +f111 - f110;
                }
                else
                {
                    // 0752
                    grad[0] = (Real)0.5 * (-f000 - f011 + f101 + f110);
                    grad[1] = (Real)0.5 * (-f000 + f011 - f101 + f110);
                    grad[2] = (Real)0.5 * (-f000 + f011 + f101 - f110);
                }
            }
            else
            {
                if (dx + dy + dz <= (Real)1)
                {
                    // 0134
                    grad[0] = -f000 + f100;
                    grad[1] = -f000 + f010;
                    grad[2] = -f000 + f001;
                }
                else if (dx + dy - dz >= (Real)1)
                {
                    // 2316
                    grad[0] = +f110 - f010;
                    grad[1] = +f110 - f100;
                    grad[2] = -f110 + f111;
                }
                else if (dx - dy + dz >= (Real)1)
                {
                    // 5461
                    grad[0] = +f101 - f001;
                    grad[1] = -f101 + f111;
                    grad[2] = +f101 - f100;
                }
                else if (-dx + dy + dz >= (Real)1)
                {
                    // 7643
                    grad[0] = -f011 + f111;
                    grad[1] = +f011 - f001;
                    grad[2] = +f011 - f010;
                }
                else
                {
                    // 6314
                    grad[0] = (Real)0.5 * (f111 - f010 + f100 - f001);
                    grad[1] = (Real)0.5 * (f111 + f010 - f100 - f001);
                    grad[2] = (Real)0.5 * (f111 - f010 - f100 + f001);
                }
            }

            return grad;
        }

        int AddVertex(Vertex const& v)
        {
            auto iter = mVMap.find(v);
            if (iter != mVMap.end())
            {
                // Vertex already in map, just return its unique index.
                return iter->second;
            }
            else
            {
                // Vertex not in map, insert it and assign it a unique index.
                int i = mNextVertex++;
                mVMap.insert(std::make_pair(v, i));
                return i;
            }
        }

        void AddEdge(Vertex const& v0, Vertex const& v1)
        {
            int i0 = AddVertex(v0);
            int i1 = AddVertex(v1);
            mESet.insert(Edge(i0, i1));
        }

        void AddTriangle(Vertex const& v0, Vertex const& v1, Vertex const& v2)
        {
            int i0 = AddVertex(v0);
            int i1 = AddVertex(v1);
            int i2 = AddVertex(v2);

            // Nothing to do if triangle already exists.
            Triangle triangle(i0, i1, i2);
            if (mTSet.find(triangle) != mTSet.end())
            {
                return;
            }

            // Prevent double-sided triangles.
            std::swap(triangle.v[1], triangle.v[2]);
            if (mTSet.find(triangle) != mTSet.end())
            {
                return;
            }

            mESet.insert(Edge(i0, i1));
            mESet.insert(Edge(i1, i2));
            mESet.insert(Edge(i2, i0));

            mTSet.insert(triangle);
        }

        // Support for extraction with linear interpolation.
        void ProcessTetrahedron(
            int64_t x0, int64_t y0, int64_t z0, int64_t f0,
            int64_t x1, int64_t y1, int64_t z1, int64_t f1,
            int64_t x2, int64_t y2, int64_t z2, int64_t f2,
            int64_t x3, int64_t y3, int64_t z3, int64_t f3)
        {
            int64_t xn0, yn0, zn0, d0;
            int64_t xn1, yn1, zn1, d1;
            int64_t xn2, yn2, zn2, d2;
            int64_t xn3, yn3, zn3, d3;
            if (f0 != 0)
            {
                // convert to case +***
                if (f0 < 0)
                {
                    f0 = -f0;
                    f1 = -f1;
                    f2 = -f2;
                    f3 = -f3;
                }

                if (f1 > 0)
                {
                    if (f2 > 0)
                    {
                        if (f3 > 0)
                        {
                            // ++++
                            return;
                        }
                        else if (f3 < 0)
                        {
                            // +++-
                            d0 = f0 - f3;
                            xn0 = f0 * x3 - f3 * x0;
                            yn0 = f0 * y3 - f3 * y0;
                            zn0 = f0 * z3 - f3 * z0;
                            d1 = f1 - f3;
                            xn1 = f1 * x3 - f3 * x1;
                            yn1 = f1 * y3 - f3 * y1;
                            zn1 = f1 * z3 - f3 * z1;
                            d2 = f2 - f3;
                            xn2 = f2 * x3 - f3 * x2;
                            yn2 = f2 * y3 - f3 * y2;
                            zn2 = f2 * z3 - f3 * z2;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                        }
                        else
                        {
                            // +++0
                            AddVertex(
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                    else if (f2 < 0)
                    {
                        d0 = f0 - f2;
                        xn0 = f0 * x2 - f2 * x0;
                        yn0 = f0 * y2 - f2 * y0;
                        zn0 = f0 * z2 - f2 * z0;
                        d1 = f1 - f2;
                        xn1 = f1 * x2 - f2 * x1;
                        yn1 = f1 * y2 - f2 * y1;
                        zn1 = f1 * z2 - f2 * z1;

                        if (f3 > 0)
                        {
                            // ++-+
                            d2 = f3 - f2;
                            xn2 = f3 * x2 - f2 * x3;
                            yn2 = f3 * y2 - f2 * y3;
                            zn2 = f3 * z2 - f2 * z3;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                        }
                        else if (f3 < 0)
                        {
                            // ++--
                            d2 = f0 - f3;
                            xn2 = f0 * x3 - f3 * x0;
                            yn2 = f0 * y3 - f3 * y0;
                            zn2 = f0 * z3 - f3 * z0;
                            d3 = f1 - f3;
                            xn3 = f1 * x3 - f3 * x1;
                            yn3 = f1 * y3 - f3 * y1;
                            zn3 = f1 * z3 - f3 * z1;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                            AddTriangle(
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn3, d3, yn3, d3, zn3, d3),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                        }
                        else
                        {
                            // ++-0
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                    else
                    {
                        if (f3 > 0)
                        {
                            // ++0+
                            AddVertex(
                                Vertex(x2, 1, y2, 1, z2, 1));
                        }
                        else if (f3 < 0)
                        {
                            // ++0-
                            d0 = f0 - f3;
                            xn0 = f0 * x3 - f3 * x0;
                            yn0 = f0 * y3 - f3 * y0;
                            zn0 = f0 * z3 - f3 * z0;
                            d1 = f1 - f3;
                            xn1 = f1 * x3 - f3 * x1;
                            yn1 = f1 * y3 - f3 * y1;
                            zn1 = f1 * z3 - f3 * z1;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x2, 1, y2, 1, z2, 1));
                        }
                        else
                        {
                            // ++00
                            AddEdge(
                                Vertex(x2, 1, y2, 1, z2, 1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                }
                else if (f1 < 0)
                {
                    if (f2 > 0)
                    {
                        d0 = f0 - f1;
                        xn0 = f0 * x1 - f1 * x0;
                        yn0 = f0 * y1 - f1 * y0;
                        zn0 = f0 * z1 - f1 * z0;
                        d1 = f2 - f1;
                        xn1 = f2 * x1 - f1 * x2;
                        yn1 = f2 * y1 - f1 * y2;
                        zn1 = f2 * z1 - f1 * z2;

                        if (f3 > 0)
                        {
                            // +-++
                            d2 = f3 - f1;
                            xn2 = f3 * x1 - f1 * x3;
                            yn2 = f3 * y1 - f1 * y3;
                            zn2 = f3 * z1 - f1 * z3;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                        }
                        else if (f3 < 0)
                        {
                            // +-+-
                            d2 = f0 - f3;
                            xn2 = f0 * x3 - f3 * x0;
                            yn2 = f0 * y3 - f3 * y0;
                            zn2 = f0 * z3 - f3 * z0;
                            d3 = f2 - f3;
                            xn3 = f2 * x3 - f3 * x2;
                            yn3 = f2 * y3 - f3 * y2;
                            zn3 = f2 * z3 - f3 * z2;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                            AddTriangle(
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn3, d3, yn3, d3, zn3, d3),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                        }
                        else
                        {
                            // +-+0
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                    else if (f2 < 0)
                    {
                        d0 = f1 - f0;
                        xn0 = f1 * x0 - f0 * x1;
                        yn0 = f1 * y0 - f0 * y1;
                        zn0 = f1 * z0 - f0 * z1;
                        d1 = f2 - f0;
                        xn1 = f2 * x0 - f0 * x2;
                        yn1 = f2 * y0 - f0 * y2;
                        zn1 = f2 * z0 - f0 * z2;

                        if (f3 > 0)
                        {
                            // +--+
                            d2 = f1 - f3;
                            xn2 = f1 * x3 - f3 * x1;
                            yn2 = f1 * y3 - f3 * y1;
                            zn2 = f1 * z3 - f3 * z1;
                            d3 = f2 - f3;
                            xn3 = f2 * x3 - f3 * x2;
                            yn3 = f2 * y3 - f3 * y2;
                            zn3 = f2 * z3 - f3 * z2;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                            AddTriangle(
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn3, d3, yn3, d3, zn3, d3),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                        }
                        else if (f3 < 0)
                        {
                            // +---
                            d2 = f3 - f0;
                            xn2 = f3 * x0 - f0 * x3;
                            yn2 = f3 * y0 - f0 * y3;
                            zn2 = f3 * z0 - f0 * z3;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(xn2, d2, yn2, d2, zn2, d2));
                        }
                        else
                        {
                            // +--0
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                    else
                    {
                        d0 = f1 - f0;
                        xn0 = f1 * x0 - f0 * x1;
                        yn0 = f1 * y0 - f0 * y1;
                        zn0 = f1 * z0 - f0 * z1;

                        if (f3 > 0)
                        {
                            // +-0+
                            d1 = f1 - f3;
                            xn1 = f1 * x3 - f3 * x1;
                            yn1 = f1 * y3 - f3 * y1;
                            zn1 = f1 * z3 - f3 * z1;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x2, 1, y2, 1, z2, 1));
                        }
                        else if (f3 < 0)
                        {
                            // +-0-
                            d1 = f3 - f0;
                            xn1 = f3 * x0 - f0 * x3;
                            yn1 = f3 * y0 - f0 * y3;
                            zn1 = f3 * z0 - f0 * z3;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x2, 1, y2, 1, z2, 1));
                        }
                        else
                        {
                            // +-00
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(x2, 1, y2, 1, z2, 1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                }
                else
                {
                    if (f2 > 0)
                    {
                        if (f3 > 0)
                        {
                            // +0++
                            AddVertex(
                                Vertex(x1, 1, y1, 1, z1, 1));
                        }
                        else if (f3 < 0)
                        {
                            // +0+-
                            d0 = f0 - f3;
                            xn0 = f0 * x3 - f3 * x0;
                            yn0 = f0 * y3 - f3 * y0;
                            zn0 = f0 * z3 - f3 * z0;
                            d1 = f2 - f3;
                            xn1 = f2 * x3 - f3 * x2;
                            yn1 = f2 * y3 - f3 * y2;
                            zn1 = f2 * z3 - f3 * z2;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x1, 1, y1, 1, z1, 1));
                        }
                        else
                        {
                            // +0+0
                            AddEdge(
                                Vertex(x1, 1, y1, 1, z1, 1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                    else if (f2 < 0)
                    {
                        d0 = f2 - f0;
                        xn0 = f2 * x0 - f0 * x2;
                        yn0 = f2 * y0 - f0 * y2;
                        zn0 = f2 * z0 - f0 * z2;

                        if (f3 > 0)
                        {
                            // +0-+
                            d1 = f2 - f3;
                            xn1 = f2 * x3 - f3 * x2;
                            yn1 = f2 * y3 - f3 * y2;
                            zn1 = f2 * z3 - f3 * z2;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x1, 1, y1, 1, z1, 1));
                        }
                        else if (f3 < 0)
                        {
                            // +0--
                            d1 = f0 - f3;
                            xn1 = f0 * x3 - f3 * x0;
                            yn1 = f0 * y3 - f3 * y0;
                            zn1 = f0 * z3 - f3 * z0;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(xn1, d1, yn1, d1, zn1, d1),
                                Vertex(x1, 1, y1, 1, z1, 1));
                        }
                        else
                        {
                            // +0-0
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(x1, 1, y1, 1, z1, 1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                    else
                    {
                        if (f3 > 0)
                        {
                            // +00+
                            AddEdge(
                                Vertex(x1, 1, y1, 1, z1, 1),
                                Vertex(x2, 1, y2, 1, z2, 1));
                        }
                        else if (f3 < 0)
                        {
                            // +00-
                            d0 = f0 - f3;
                            xn0 = f0 * x3 - f3 * x0;
                            yn0 = f0 * y3 - f3 * y0;
                            zn0 = f0 * z3 - f3 * z0;
                            AddTriangle(
                                Vertex(xn0, d0, yn0, d0, zn0, d0),
                                Vertex(x1, 1, y1, 1, z1, 1),
                                Vertex(x2, 1, y2, 1, z2, 1));
                        }
                        else
                        {
                            // +000
                            AddTriangle(
                                Vertex(x1, 1, y1, 1, z1, 1),
                                Vertex(x2, 1, y2, 1, z2, 1),
                                Vertex(x3, 1, y3, 1, z3, 1));
                        }
                    }
                }
            }
            else if (f1 != 0)
            {
                // convert to case 0+**
                if (f1 < 0)
                {
                    f1 = -f1;
                    f2 = -f2;
                    f3 = -f3;
                }

                if (f2 > 0)
                {
                    if (f3 > 0)
                    {
                        // 0+++
                        AddVertex(
                            Vertex(x0, 1, y0, 1, z0, 1));
                    }
                    else if (f3 < 0)
                    {
                        // 0++-
                        d0 = f2 - f3;
                        xn0 = f2 * x3 - f3 * x2;
                        yn0 = f2 * y3 - f3 * y2;
                        zn0 = f2 * z3 - f3 * z2;
                        d1 = f1 - f3;
                        xn1 = f1 * x3 - f3 * x1;
                        yn1 = f1 * y3 - f3 * y1;
                        zn1 = f1 * z3 - f3 * z1;
                        AddTriangle(
                            Vertex(xn0, d0, yn0, d0, zn0, d0),
                            Vertex(xn1, d1, yn1, d1, zn1, d1),
                            Vertex(x0, 1, y0, 1, z0, 1));
                    }
                    else
                    {
                        // 0++0
                        AddEdge(
                            Vertex(x0, 1, y0, 1, z0, 1),
                            Vertex(x3, 1, y3, 1, z3, 1));
                    }
                }
                else if (f2 < 0)
                {
                    d0 = f2 - f1;
                    xn0 = f2 * x1 - f1 * x2;
                    yn0 = f2 * y1 - f1 * y2;
                    zn0 = f2 * z1 - f1 * z2;

                    if (f3 > 0)
                    {
                        // 0+-+
                        d1 = f2 - f3;
                        xn1 = f2 * x3 - f3 * x2;
                        yn1 = f2 * y3 - f3 * y2;
                        zn1 = f2 * z3 - f3 * z2;
                        AddTriangle(
                            Vertex(xn0, d0, yn0, d0, zn0, d0),
                            Vertex(xn1, d1, yn1, d1, zn1, d1),
                            Vertex(x0, 1, y0, 1, z0, 1));
                    }
                    else if (f3 < 0)
                    {
                        // 0+--
                        d1 = f1 - f3;
                        xn1 = f1 * x3 - f3 * x1;
                        yn1 = f1 * y3 - f3 * y1;
                        zn1 = f1 * z3 - f3 * z1;
                        AddTriangle(
                            Vertex(xn0, d0, yn0, d0, zn0, d0),
                            Vertex(xn1, d1, yn1, d1, zn1, d1),
                            Vertex(x0, 1, y0, 1, z0, 1));
                    }
                    else
                    {
                        // 0+-0
                        AddTriangle(
                            Vertex(xn0, d0, yn0, d0, zn0, d0),
                            Vertex(x0, 1, y0, 1, z0, 1),
                            Vertex(x3, 1, y3, 1, z3, 1));
                    }
                }
                else
                {
                    if (f3 > 0)
                    {
                        // 0+0+
                        AddEdge(
                            Vertex(x0, 1, y0, 1, z0, 1),
                            Vertex(x2, 1, y2, 1, z2, 1));
                    }
                    else if (f3 < 0)
                    {
                        // 0+0-
                        d0 = f1 - f3;
                        xn0 = f1 * x3 - f3 * x1;
                        yn0 = f1 * y3 - f3 * y1;
                        zn0 = f1 * z3 - f3 * z1;
                        AddTriangle(
                            Vertex(xn0, d0, yn0, d0, zn0, d0),
                            Vertex(x0, 1, y0, 1, z0, 1),
                            Vertex(x2, 1, y2, 1, z2, 1));
                    }
                    else
                    {
                        // 0+00
                        AddTriangle(
                            Vertex(x0, 1, y0, 1, z0, 1),
                            Vertex(x2, 1, y2, 1, z2, 1),
                            Vertex(x3, 1, y3, 1, z3, 1));
                    }
                }
            }
            else if (f2 != 0)
            {
                // convert to case 00+*
                if (f2 < 0)
                {
                    f2 = -f2;
                    f3 = -f3;
                }

                if (f3 > 0)
                {
                    // 00++
                    AddEdge(
                        Vertex(x0, 1, y0, 1, z0, 1),
                        Vertex(x1, 1, y1, 1, z1, 1));
                }
                else if (f3 < 0)
                {
                    // 00+-
                    d0 = f2 - f3;
                    xn0 = f2 * x3 - f3 * x2;
                    yn0 = f2 * y3 - f3 * y2;
                    zn0 = f2 * z3 - f3 * z2;
                    AddTriangle(
                        Vertex(xn0, d0, yn0, d0, zn0, d0),
                        Vertex(x0, 1, y0, 1, z0, 1),
                        Vertex(x1, 1, y1, 1, z1, 1));
                }
                else
                {
                    // 00+0
                    AddTriangle(
                        Vertex(x0, 1, y0, 1, z0, 1),
                        Vertex(x1, 1, y1, 1, z1, 1),
                        Vertex(x3, 1, y3, 1, z3, 1));
                }
            }
            else if (f3 != 0)
            {
                // cases 000+ or 000-
                AddTriangle(
                    Vertex(x0, 1, y0, 1, z0, 1),
                    Vertex(x1, 1, y1, 1, z1, 1),
                    Vertex(x2, 1, y2, 1, z2, 1));
            }
            else
            {
                // case 0000
                AddTriangle(
                    Vertex(x0, 1, y0, 1, z0, 1),
                    Vertex(x1, 1, y1, 1, z1, 1),
                    Vertex(x2, 1, y2, 1, z2, 1));
                AddTriangle(
                    Vertex(x0, 1, y0, 1, z0, 1),
                    Vertex(x1, 1, y1, 1, z1, 1),
                    Vertex(x3, 1, y3, 1, z3, 1));
                AddTriangle(
                    Vertex(x0, 1, y0, 1, z0, 1),
                    Vertex(x2, 1, y2, 1, z2, 1),
                    Vertex(x3, 1, y3, 1, z3, 1));
                AddTriangle(
                    Vertex(x1, 1, y1, 1, z1, 1),
                    Vertex(x2, 1, y2, 1, z2, 1),
                    Vertex(x3, 1, y3, 1, z3, 1));
            }
        }

        std::map<Vertex, int> mVMap;
        std::set<Edge> mESet;
        std::set<Triangle> mTSet;
        int mNextVertex;
    };
}
