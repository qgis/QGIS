// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.18

#pragma once

#include <Mathematics/MarchingCubes.h>
#include <Mathematics/Image3.h>
#include <Mathematics/UniqueVerticesSimplices.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    template <typename Real>
    class SurfaceExtractorMC : public MarchingCubes
    {
    public:
        // Construction and destruction.
        virtual ~SurfaceExtractorMC()
        {
        }

        SurfaceExtractorMC(Image3<Real> const& image)
            :
            mImage(image)
        {
        }

        // Object copies are not allowed.
        SurfaceExtractorMC() = delete;
        SurfaceExtractorMC(SurfaceExtractorMC const&) = delete;
        SurfaceExtractorMC const& operator=(SurfaceExtractorMC const&) = delete;

        struct Mesh
        {
            // All members are set to zeros.
            Mesh()
            {
                std::array<Real, 3> zero = { (Real)0, (Real)0, (Real)0 };
                std::fill(vertices.begin(), vertices.end(), zero);
            }

            Topology topology;
            std::array<Vector3<Real>, MAX_VERTICES> vertices;
        };

        // Extract the triangle mesh approximating F = 0 for a single voxel.
        // The input function values must be stored as
        //  F[0] = function(0,0,0), F[4] = function(0,0,1),
        //  F[1] = function(1,0,0), F[5] = function(1,0,1),
        //  F[2] = function(0,1,0), F[6] = function(0,1,1),
        //  F[3] = function(1,1,0), F[7] = function(1,1,1).
        // Thus, F[k] = function(k & 1, (k & 2) >> 1, (k & 4) >> 2).
        // The return value is 'true' iff the F[] values are all nonzero.
        // If they are not, the returned 'mesh' has no vertices and no
        // triangles--as if F[] had all positive or all negative values.
        bool Extract(std::array<Real, 8> const& F, Mesh& mesh) const
        {
            int entry = 0;
            for (int i = 0, mask = 1; i < 8; ++i, mask <<= 1)
            {
                if (F[i] < (Real)0)
                {
                    entry |= mask;
                }
                else if (F[i] == (Real)0)
                {
                    return false;
                }
            }

            mesh.topology = GetTable(entry);

            for (int i = 0; i < mesh.topology.numVertices; ++i)
            {
                int j0 = mesh.topology.vpair[i][0];
                int j1 = mesh.topology.vpair[i][1];

                Real corner0[3];
                corner0[0] = static_cast<Real>(j0 & 1);
                corner0[1] = static_cast<Real>((j0 & 2) >> 1);
                corner0[2] = static_cast<Real>((j0 & 4) >> 2);

                Real corner1[3];
                corner1[0] = static_cast<Real>(j1 & 1);
                corner1[1] = static_cast<Real>((j1 & 2) >> 1);
                corner1[2] = static_cast<Real>((j1 & 4) >> 2);

                Real invDenom = ((Real)1) / (F[j0] - F[j1]);
                for (int k = 0; k < 3; ++k)
                {
                    Real numer = F[j0] * corner1[k] - F[j1] * corner0[k];
                    mesh.vertices[i][k] = numer * invDenom;
                }
            }
            return true;
        }

        // Extract the triangle mesh approximating F = 0 for all the voxels in
        // a 3D image.  The input image must be stored in a 1-dimensional
        // array with lexicographical order; that is, image[i] corresponds to
        // voxel location (x,y,z) where i = x + bound0 * (y + bound1 * z).
        // The output 'indices' consists indices.size()/3 triangles, each a
        // triple of indices into 'vertices'
        bool Extract(Real level, std::vector<Vector3<Real>>& vertices, std::vector<int>& indices) const
        {
            vertices.clear();
            indices.clear();

            for (int z = 0; z + 1 < mImage.GetDimension(2); ++z)
            {
                for (int y = 0; y + 1 < mImage.GetDimension(1); ++y)
                {
                    for (int x = 0; x + 1 < mImage.GetDimension(0); ++x)
                    {
                        std::array<size_t, 8> corners;
                        mImage.GetCorners(x, y, z, corners);

                        std::array<Real, 8> F;
                        for (int k = 0; k < 8; ++k)
                        {
                            F[k] = mImage[corners[k]] - level;
                        }

                        Mesh mesh;

                        if (Extract(F, mesh))
                        {
                            int vbase = static_cast<int>(vertices.size());
                            for (int i = 0; i < mesh.topology.numVertices; ++i)
                            {
                                Vector3<float> position = mesh.vertices[i];
                                position[0] += static_cast<Real>(x);
                                position[1] += static_cast<Real>(y);
                                position[2] += static_cast<Real>(z);
                                vertices.push_back(position);
                            }

                            for (int i = 0; i < mesh.topology.numTriangles; ++i)
                            {
                                for (int j = 0; j < 3; ++j)
                                {
                                    indices.push_back(vbase + mesh.topology.itriple[i][j]);
                                }
                            }
                        }
                        else
                        {
                            vertices.clear();
                            indices.clear();
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        // The extraction has duplicate vertices on edges shared by voxels.
        // This function will eliminate the duplication.
        void MakeUnique(std::vector<Vector3<Real>>& vertices, std::vector<int>& indices) const
        {
            std::vector<Vector3<Real>> outVertices;
            std::vector<int> outIndices;
            UniqueVerticesSimplices<Vector3<Real>, int, 3> uvt;
            uvt.RemoveDuplicateVertices(vertices, indices, outVertices, outIndices);
            vertices = std::move(outVertices);
            indices = std::move(outIndices);
        }

        // The extraction does not use any topological information about the
        // level surface.  The triangles can be a mixture of clockwise-ordered
        // and counterclockwise-ordered.  This function is an attempt to give
        // the triangles a consistent ordering by selecting a normal in
        // approximately the same direction as the average gradient at the
        // vertices (when sameDir is true), or in the opposite direction (when
        // sameDir is false).  This might not always produce a consistent
        // order, but is fast.  A consistent order can be computed if you
        // build a table of vertex, edge, and face adjacencies, but the
        // resulting data structure is somewhat expensive to process to
        // reorient triangles.
        void OrientTriangles(std::vector<Vector3<Real>> const& vertices, std::vector<int>& indices, bool sameDir) const
        {
            int const numTriangles = static_cast<int>(indices.size() / 3);
            int* triangle = indices.data();
            for (int t = 0; t < numTriangles; ++t, triangle += 3)
            {
                // Get triangle vertices.
                Vector3<Real> v0 = vertices[triangle[0]];
                Vector3<Real> v1 = vertices[triangle[1]];
                Vector3<Real> v2 = vertices[triangle[2]];

                // Construct triangle normal based on current orientation.
                Vector3<Real> edge1 = v1 - v0;
                Vector3<Real> edge2 = v2 - v0;
                Vector3<Real> normal = Cross(edge1, edge2);

                // Get the image gradient at the vertices.
                Vector3<Real> gradient0 = GetGradient(v0);
                Vector3<Real> gradient1 = GetGradient(v1);
                Vector3<Real> gradient2 = GetGradient(v2);

                // Compute the average gradient.
                Vector3<Real> gradientAvr = (gradient0 + gradient1 + gradient2) / (Real)3;

                // Compute the dot product of normal and average gradient.
                Real dot = Dot(gradientAvr, normal);

                // Choose triangle orientation based on gradient direction.
                if (sameDir)
                {
                    if (dot < (Real)0)
                    {
                        // Wrong orientation, reorder it.
                        std::swap(triangle[1], triangle[2]);
                    }
                }
                else
                {
                    if (dot > (Real)0)
                    {
                        // Wrong orientation, reorder it.
                        std::swap(triangle[1], triangle[2]);
                    }
                }
            }
        }

        // Compute vertex normals for the mesh.
        void ComputeNormals(std::vector<Vector3<Real>> const& vertices, std::vector<int> const& indices,
            std::vector<Vector3<Real>>& normals) const
        {
            // Maintain a running sum of triangle normals at each vertex.
            int const numVertices = static_cast<int>(vertices.size());
            normals.resize(numVertices);
            Vector3<Real> zero = Vector3<Real>::Zero();
            std::fill(normals.begin(), normals.end(), zero);

            int const numTriangles = static_cast<int>(indices.size() / 3);
            int const* current = indices.data();
            for (int i = 0; i < numTriangles; ++i)
            {
                int i0 = *current++;
                int i1 = *current++;
                int i2 = *current++;
                Vector3<Real> v0 = vertices[i0];
                Vector3<Real> v1 = vertices[i1];
                Vector3<Real> v2 = vertices[i2];

                // Construct triangle normal.
                Vector3<Real> edge1 = v1 - v0;
                Vector3<Real> edge2 = v2 - v0;
                Vector3<Real> normal = Cross(edge1, edge2);

                // Maintain the sum of normals at each vertex.
                normals[i0] += normal;
                normals[i1] += normal;
                normals[i2] += normal;
            }

            // The normal vector storage was used to accumulate the sum of
            // triangle normals.  Now these vectors must be rescaled to be
            // unit length.
            for (auto& normal : normals)
            {
                Normalize(normal);
            }
        }

    protected:
        Vector3<Real> GetGradient(Vector3<Real> position) const
        {
            int x = static_cast<int>(std::floor(position[0]));
            if (x < 0 || x >= mImage.GetDimension(0) - 1)
            {
                return Vector3<Real>::Zero();
            }

            int y = static_cast<int>(std::floor(position[1]));
            if (y < 0 || y >= mImage.GetDimension(1) - 1)
            {
                return Vector3<Real>::Zero();
            }

            int z = static_cast<int>(std::floor(position[2]));
            if (z < 0 || z >= mImage.GetDimension(2) - 1)
            {
                return Vector3<Real>::Zero();
            }

            position[0] -= static_cast<Real>(x);
            position[1] -= static_cast<Real>(y);
            position[2] -= static_cast<Real>(z);
            Real oneMX = (Real)1 - position[0];
            Real oneMY = (Real)1 - position[1];
            Real oneMZ = (Real)1 - position[2];

            // Get image values at corners of voxel.
            std::array<size_t, 8> corners;
            mImage.GetCorners(x, y, z, corners);
            Real f000 = mImage[corners[0]];
            Real f100 = mImage[corners[1]];
            Real f010 = mImage[corners[2]];
            Real f110 = mImage[corners[3]];
            Real f001 = mImage[corners[4]];
            Real f101 = mImage[corners[5]];
            Real f011 = mImage[corners[6]];
            Real f111 = mImage[corners[7]];

            Vector3<Real> gradient;

            Real tmp0 = oneMY * (f100 - f000) + position[1] * (f110 - f010);
            Real tmp1 = oneMY * (f101 - f001) + position[1] * (f111 - f011);
            gradient[0] = oneMZ * tmp0 + position[2] * tmp1;

            tmp0 = oneMX * (f010 - f000) + position[0] * (f110 - f100);
            tmp1 = oneMX * (f011 - f001) + position[0] * (f111 - f101);
            gradient[1] = oneMZ * tmp0 + position[2] * tmp1;

            tmp0 = oneMX * (f001 - f000) + position[0] * (f101 - f100);
            tmp1 = oneMX * (f011 - f010) + position[0] * (f111 - f110);
            gradient[2] = oneMY * tmp0 + position[1] * tmp1;

            return gradient;
        }

        Image3<Real> const& mImage;
    };
}
