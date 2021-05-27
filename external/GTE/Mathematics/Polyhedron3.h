// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector3.h>
#include <memory>
#include <set>
#include <vector>

// The Polyhedron3 object represents a simple polyhedron.  The 'vertexPool'
// array can contain more points than needed to define the polyhedron, which
// allows the vertex pool to have multiple polyhedra associated with it.
// Thus, the programmer must ensure that the vertex pool persists as long as
// any Polyhedron3 objects exist that depend on the pool.  The number of
// polyhedron indices is 'numIndices' and must be 6 or larger  The 'indices'
// array refers to the points in 'vertexPool' that form the triangle faces,
// so 'numIndices' must be a multiple of 3.  The number of vertices is
// the number of unique elements in 'indices' and is determined during
// construction.  The programmer should ensure the polyhedron is simple.  The
// geometric queries are valid regardless of whether the polyhedron triangles
// are oriented clockwise or counterclockwise.
//
// NOTE:  Comparison operators are not provided.  The semantics of equal
// polyhedra is complicated and (at the moment) not useful.  The vertex pools
// can be different and indices do not match, but the vertices they reference
// can match.  Even with a shared vertex pool, the indices can be permuted,
// leading to the same polyhedron abstractly but the data structures do not
// match.

namespace gte
{
    template <typename Real>
    class Polyhedron3
    {
    public:
        // Construction.  The constructor succeeds when 'numIndices >= 12' (at
        // least 4 triangles), and 'vertexPool' and 'indices' are not null; we
        // cannot test whether you have a valid number of elements in the
        // input arrays.  A copy is made of 'indices', but the 'vertexPool' is
        // not copied.  If the constructor fails, the internal vertex pointer
        // is set to null, the number of vertices is set to zero, the index
        // array has no elements, and the triangle face orientation is set to
        // clockwise.
        Polyhedron3(std::shared_ptr<std::vector<Vector3<Real>>> const& vertexPool,
            int numIndices, int const* indices, bool counterClockwise)
            :
            mVertexPool(vertexPool),
            mCounterClockwise(counterClockwise)
        {
            if (vertexPool && indices && numIndices >= 12 && (numIndices % 3) == 0)
            {
                for (int i = 0; i < numIndices; ++i)
                {
                    mUniqueIndices.insert(indices[i]);
                }

                mIndices.resize(numIndices);
                std::copy(indices, indices + numIndices, mIndices.begin());
            }
            else
            {
                // Encountered an invalid input.
                mVertexPool = nullptr;
                mCounterClockwise = false;
            }
        }

        // To validate construction, create an object as shown:
        //     Polyhedron3<Real> polyhedron(parameters);
        //     if (!polyhedron) { <constructor failed, handle accordingly>; }
        inline operator bool() const
        {
            return mVertexPool != nullptr;
        }

        // Member access.
        inline std::shared_ptr<std::vector<Vector3<Real>>> const& GetVertexPool() const
        {
            return mVertexPool;
        }

        inline std::vector<Vector3<Real>> const& GetVertices() const
        {
            return *mVertexPool.get();
        }

        inline std::set<int> const& GetUniqueIndices() const
        {
            return mUniqueIndices;
        }

        inline std::vector<int> const& GetIndices() const
        {
            return mIndices;
        }

        inline bool CounterClockwise() const
        {
            return mCounterClockwise;
        }

        // Geometric queries.
        Vector3<Real> ComputeVertexAverage() const
        {
            Vector3<Real> average = Vector3<Real>::Zero();
            if (mVertexPool)
            {
                auto vertexPool = GetVertices();
                for (int index : mUniqueIndices)
                {
                    average += vertexPool[index];
                }
                average /= static_cast<Real>(mUniqueIndices.size());
            }
            return average;
        }

        Real ComputeSurfaceArea() const
        {
            Real surfaceArea(0);
            if (mVertexPool)
            {
                auto vertexPool = GetVertices();
                int const numTriangles = static_cast<int>(mIndices.size()) / 3;
                int const* indices = mIndices.data();
                for (int t = 0; t < numTriangles; ++t)
                {
                    int v0 = *indices++;
                    int v1 = *indices++;
                    int v2 = *indices++;
                    Vector3<Real> edge0 = vertexPool[v1] - vertexPool[v0];
                    Vector3<Real> edge1 = vertexPool[v2] - vertexPool[v0];
                    Vector3<Real> cross = Cross(edge0, edge1);
                    surfaceArea += Length(cross);
                }
                surfaceArea *= (Real)0.5;
            }
            return surfaceArea;
        }

        Real ComputeVolume() const
        {
            Real volume(0);
            if (mVertexPool)
            {
                auto vertexPool = GetVertices();
                int const numTriangles = static_cast<int>(mIndices.size()) / 3;
                int const* indices = mIndices.data();
                for (int t = 0; t < numTriangles; ++t)
                {
                    int v0 = *indices++;
                    int v1 = *indices++;
                    int v2 = *indices++;
                    volume += DotCross(vertexPool[v0], vertexPool[v1], vertexPool[v2]);
                }
                volume /= (Real)6;
            }
            return std::fabs(volume);
        }

    private:
        std::shared_ptr<std::vector<Vector3<Real>>> mVertexPool;
        std::set<int> mUniqueIndices;
        std::vector<int> mIndices;
        bool mCounterClockwise;
    };
}
