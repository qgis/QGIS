// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <vector>

namespace gte
{
    // The image type T must be one of the integer types:  int8_t, int16_t,
    // int32_t, uint8_t, uint16_t or uint32_t.  Internal integer computations
    // are performed using int64_t.  The type Real is for extraction to
    // floating-point vertices.
    template <typename T, typename Real>
    class SurfaceExtractor
    {
    public:
        // Abstract base class.
        virtual ~SurfaceExtractor() = default;

        // The level surfaces form a graph of vertices, edges and triangles.
        // The vertices are computed as triples of nonnegative rational
        // numbers.  Vertex represents the rational triple
        //   (xNumer/xDenom, yNumer/yDenom, zNumer/zDenom)
        // as
        //   (xNumer, xDenom, yNumer, yDenom, zNumer, zDenom)
        // where all components are nonnegative.  The edges connect pairs of
        // vertices and the triangles connect triples of vertices, forming
        // a graph that represents the level set.
        struct Vertex
        {
            Vertex() = default;

            Vertex(int64_t inXNumer, int64_t inXDenom, int64_t inYNumer, int64_t inYDenom,
                int64_t inZNumer, int64_t inZDenom)
            {
                // The vertex generation leads to the numerator and
                // denominator having the same sign.  This constructor changes
                // sign to ensure the numerator and denominator are both
                // positive.
                if (inXDenom > 0)
                {
                    xNumer = inXNumer;
                    xDenom = inXDenom;
                }
                else
                {
                    xNumer = -inXNumer;
                    xDenom = -inXDenom;
                }

                if (inYDenom > 0)
                {
                    yNumer = inYNumer;
                    yDenom = inYDenom;
                }
                else
                {
                    yNumer = -inYNumer;
                    yDenom = -inYDenom;
                }

                if (inZDenom > 0)
                {
                    zNumer = inZNumer;
                    zDenom = inZDenom;
                }
                else
                {
                    zNumer = -inZNumer;
                    zDenom = -inZDenom;
                }
            }

            // The non-default constructor guarantees that xDenom > 0,
            // yDenom > 0 and zDenom > 0.  The following comparison operators
            // assume that the denominators are positive.
            bool operator==(Vertex const& other) const
            {
                return
                    // xn0/xd0 == xn1/xd1
                    xNumer * other.xDenom == other.xNumer * xDenom
                    &&
                    // yn0/yd0 == yn1/yd1
                    yNumer * other.yDenom == other.yNumer * yDenom
                    &&
                    // zn0/zd0 == zn1/zd1
                    zNumer * other.zDenom == other.zNumer * zDenom;
            }

            bool operator<(Vertex const& other) const
            {
                int64_t xn0txd1 = xNumer * other.xDenom;
                int64_t xn1txd0 = other.xNumer * xDenom;
                if (xn0txd1 < xn1txd0)
                {
                    // xn0/xd0 < xn1/xd1
                    return true;
                }
                if (xn0txd1 > xn1txd0)
                {
                    // xn0/xd0 > xn1/xd1
                    return false;
                }

                int64_t yn0tyd1 = yNumer * other.yDenom;
                int64_t yn1tyd0 = other.yNumer * yDenom;
                if (yn0tyd1 < yn1tyd0)
                {
                    // yn0/yd0 < yn1/yd1
                    return true;
                }
                if (yn0tyd1 > yn1tyd0)
                {
                    // yn0/yd0 > yn1/yd1
                    return false;
                }

                int64_t zn0tzd1 = zNumer * other.zDenom;
                int64_t zn1tzd0 = other.zNumer * zDenom;
                // zn0/zd0 < zn1/zd1
                return zn0tzd1 < zn1tzd0;
            }

            int64_t xNumer, xDenom, yNumer, yDenom, zNumer, zDenom;
        };

        struct Triangle
        {
            Triangle() = default;

            Triangle(int v0, int v1, int v2)
            {
                // After the code is executed, (v[0],v[1],v[2]) is a cyclic
                // permutation of (v0,v1,v2) with v[0] = min{v0,v1,v2}.
                if (v0 < v1)
                {
                    if (v0 < v2)
                    {
                        v[0] = v0;
                        v[1] = v1;
                        v[2] = v2;
                    }
                    else
                    {
                        v[0] = v2;
                        v[1] = v0;
                        v[2] = v1;
                    }
                }
                else
                {
                    if (v1 < v2)
                    {
                        v[0] = v1;
                        v[1] = v2;
                        v[2] = v0;
                    }
                    else
                    {
                        v[0] = v2;
                        v[1] = v0;
                        v[2] = v1;
                    }
                }
            }

            bool operator==(Triangle const& other) const
            {
                return v[0] == other.v[0] && v[1] == other.v[1] && v[2] == other.v[2];
            }

            bool operator<(Triangle const& other) const
            {
                for (int i = 0; i < 3; ++i)
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

            std::array<int, 3> v;
        };

        // Extract level surfaces and return rational vertices.
        virtual void Extract(T level, std::vector<Vertex>& vertices,
            std::vector<Triangle>& triangles) = 0;

        void Extract(T level, bool removeDuplicateVertices,
            std::vector<std::array<Real, 3>>& vertices, std::vector<Triangle>& triangles)
        {
            std::vector<Vertex> rationalVertices;
            Extract(level, rationalVertices, triangles);
            if (removeDuplicateVertices)
            {
                MakeUnique(rationalVertices, triangles);
            }
            Convert(rationalVertices, vertices);
        }

        // The extraction has duplicate vertices on edges shared by voxels.
        // This function will eliminate the duplicates.
        void MakeUnique(std::vector<Vertex>& vertices, std::vector<Triangle>& triangles)
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
                    auto iter = vmap.find(vertices[triangle.v[i]]);
                    LogAssert(iter != vmap.end(), "Expecting the vertex to be in the vmap.");
                    triangle.v[i] = iter->second;
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

        // Convert from Vertex to std::array<Real, 3> rationals.
        void Convert(std::vector<Vertex> const& input, std::vector<std::array<Real, 3>>& output)
        {
            output.resize(input.size());
            for (size_t i = 0; i < input.size(); ++i)
            {
                Real rxNumer = static_cast<Real>(input[i].xNumer);
                Real rxDenom = static_cast<Real>(input[i].xDenom);
                Real ryNumer = static_cast<Real>(input[i].yNumer);
                Real ryDenom = static_cast<Real>(input[i].yDenom);
                Real rzNumer = static_cast<Real>(input[i].zNumer);
                Real rzDenom = static_cast<Real>(input[i].zDenom);
                output[i][0] = rxNumer / rxDenom;
                output[i][1] = ryNumer / ryDenom;
                output[i][2] = rzNumer / rzDenom;
            }
        }

        // The extraction does not use any topological information about the
        // level surfaces. The triangles can be a mixture of clockwise-ordered
        // and counterclockwise-ordered.  This function is an attempt to give
        // the triangles a consistent ordering by selecting a normal in
        // approximately the same direction as the average gradient at the
        // vertices (when sameDir is true), or in the opposite direction (when
        // sameDir is false).  This might not always produce a consistent
        // order, but is fast.  A consistent order can be computed by
        // choosing a winding order for each triangle so that any corner of
        // the voxel containing the triangle and that has positive sign sees
        // a counterclockwise order.  Of course, you can also choose that the
        // positive sign corners of the voxel always see the voxel-contained
        // triangles in clockwise order.
        void OrientTriangles(std::vector<std::array<Real, 3>>& vertices,
            std::vector<Triangle>& triangles, bool sameDir)
        {
            for (auto& triangle : triangles)
            {
                // Get the triangle vertices.
                std::array<Real, 3> v0 = vertices[triangle.v[0]];
                std::array<Real, 3> v1 = vertices[triangle.v[1]];
                std::array<Real, 3> v2 = vertices[triangle.v[2]];

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
                        std::swap(triangle.v[1], triangle.v[2]);
                    }
                }
                else
                {
                    if (dot > (Real)0)
                    {
                        // Wrong orientation, reorder it.
                        std::swap(triangle.v[1], triangle.v[2]);
                    }
                }
            }
        }

        // Use this function if you want vertex normals for dynamic lighting
        // of the mesh.
        void ComputeNormals(std::vector<std::array<Real, 3>> const& vertices,
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
                std::array<Real, 3> v0 = vertices[triangle.v[0]];
                std::array<Real, 3> v1 = vertices[triangle.v[1]];
                std::array<Real, 3> v2 = vertices[triangle.v[2]];

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
                        normals[triangle.v[i]][j] += normal[j];
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

    protected:
        // The input is a 3D image with lexicographically ordered voxels
        // (x,y,z) stored in a linear array.  Voxel (x,y,z) is stored in the
        // array at location index = x + xBound * (y + yBound * z).  The
        // inputs xBound, yBound and zBound must each be 2 or larger so that
        // there is at least one image cube to process.  The inputVoxels must
        // be nonnull and point to contiguous storage that contains at least
        // xBound * yBound * zBound elements.
        SurfaceExtractor(int xBound, int yBound, int zBound, T const* inputVoxels)
            :
            mXBound(xBound),
            mYBound(yBound),
            mZBound(zBound),
            mXYBound(xBound * yBound),
            mInputVoxels(inputVoxels)
        {
            static_assert(std::is_integral<T>::value && sizeof(T) <= 4,
                "Type T must be int{8,16,32}_t or uint{8,16,32}_t.");
            LogAssert(xBound > 1 && yBound > 1 && zBound > 1 && mInputVoxels != nullptr,
                "Invalid input.");

            mVoxels.resize(static_cast<size_t>(mXBound * mYBound * mZBound));
        }

        virtual std::array<Real, 3> GetGradient(std::array<Real, 3> const& pos) = 0;

        int mXBound, mYBound, mZBound, mXYBound;
        T const* mInputVoxels;
        std::vector<int64_t> mVoxels;
    };
}
