// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.5.2020.12.18

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <vector>

namespace gte
{
    template <typename T>
    class TetrahedraRasterizer
    {
    public:
        // The tetrahedra are stored as indexed primitives where the indices
        // are relative to the vertices[] array. The vertices of the
        // tetrahedra are vertices[tetrahedra[t][i]] for 0 <= i < 4. If
        // v0, v1, v2 and v3 are those vertices, the triangle faces have
        // vertices {v0,v2,v1}, {v0,v1,v3}, {v0,v3,v2} and {v1,v2,v3}. The
        // faces are counterclockwise ordered when viewed by an observer
        // outside the tetrahedron. The canonical tetrahedron is
        // {v0,v1,v2,v3} where v0 = (0,0,0), v1 = (1,0,0), v2 = (0,1,0) and
        // v3 = (0,0,1).
        TetrahedraRasterizer(size_t numVertices, std::array<T, 3> const* vertices,
            size_t numTetrahedra, std::array<size_t, 4> const* tetrahedra)
            :
            mNumVertices(numVertices),
            mVertices(vertices),
            mNumTetrahedra(numTetrahedra),
            mTetrahedra(tetrahedra),
            mTetraMin(numTetrahedra),
            mTetraMax(numTetrahedra),
            mValid(numTetrahedra),
            mGridVertices(numVertices),
            mGridTetraMin(numTetrahedra),
            mGridTetraMax(numTetrahedra)
        {
            if (numVertices == 0 || !vertices || numTetrahedra == 0 || !tetrahedra)
            {
                throw std::invalid_argument("Invalid argument.");
            }

            ComputeTetrahedraAABBs();
        }

        // Rasterize the tetrahedra into a 3D grid. The input region is a
        // box in the coordinate system of the vertices. The box is associated
        // with a 3D grid of specified bounds. A grid point is a 3-tuple
        // (x,y,z) with integer coordinates satisfying 0 <= x < xBound,
        // 0 <= y < yBound and 0 <= z < zBound. The point is classified based
        // on whether or not it is contained by a tetrahedron, and the
        // classification is stored as an integer in grid[i] where the index
        // is i = x + bound[0] * (y + bound[1] * z). If the point is not
        // contained by a tetrahedron, grid[i] is set to -1. If the point is
        // contained by a tetrahedron, grid[i] is set to the tetrahedron
        // index t where 0 <= t < numTetrahedra.
        //
        // To run in the main thread only, choose numThreads to be 0. For
        // multithreading, choose numThreads > 0. The number of available
        // hardware threads is std::thread::hardware_concurrency(). A
        // reasonable choice for numThreads will not exceed the number of
        // available hardware threads. You might want to keep 1 or 2 threads
        // available for the operating system and other applications running
        // on the machine.
        void operator()(size_t numThreads, std::array<T, 3> const& regionMin,
            std::array<T, 3> const& regionMax, std::array<size_t, 3> const& bound,
            std::vector<int32_t>& grid)
        {
            if (bound[0] < 2 || bound[1] < 2 || bound[2] < 2)
            {
                throw std::invalid_argument("Invalid argument.");
            }

            // Initialize the grid values to -1. When a grid cell is contained
            // in a tetrahedron, the index of that tetrahedron is stored in
            // grid[i]. All such contained grid[] values are nonnegative.
            grid.resize(bound[0] * bound[1] * bound[2]);
            std::fill(grid.begin(), grid.end(), -1);

            // Clip-cull the tetrahedra bounding boxes against the region.
            ClipCullAABBs(regionMin, regionMax);

            // Transform the vertices and tetrahedra bounding boxes to
            // grid coordinates.
            TransformToGridCoordinates(regionMin, regionMax, bound);

            if (numThreads > 0)
            {
                MultiThreadedRasterizer(numThreads, bound, grid);
            }
            else
            {
                SingleThreadedRasterizer(bound, grid);
            }
        }

    private:
        // Compute the axis-aligned bounding boxes of the tetrahedra.
        void ComputeTetrahedraAABBs()
        {
            for (size_t t = 0; t < mNumTetrahedra; ++t)
            {
                auto& tetraMin = mTetraMin[t];
                auto& tetraMax = mTetraMax[t];
                tetraMin = mVertices[mTetrahedra[t][0]];
                tetraMax = tetraMin;
                for (size_t j = 1; j < 4; ++j)
                {
                    auto const& vertex = mVertices[mTetrahedra[t][j]];
                    for (size_t i = 0; i < 3; ++i)
                    {
                        if (vertex[i] < tetraMin[i])
                        {
                            tetraMin[i] = vertex[i];
                        }
                        else if (vertex[i] > tetraMax[i])
                        {
                            tetraMax[i] = vertex[i];
                        }
                    }
                }
            }
        }

        // Clip-cull the tetrahedra bounding boxes against the region.
        // The mValid[t] is true whenever the mAABB[t] intersects the
        // region.
        void ClipCullAABBs(std::array<T, 3> const& regionMin,
            std::array<T, 3> const& regionMax)
        {
            for (size_t t = 0; t < mNumTetrahedra; ++t)
            {
                auto& tetraMin = mTetraMin[t];
                auto& tetraMax = mTetraMax[t];
                mValid[t] = true;
                for (size_t i = 0; i < 3; ++i)
                {
                    tetraMin[i] = std::max(tetraMin[i], regionMin[i]);
                    tetraMax[i] = std::min(tetraMax[i], regionMax[i]);
                    if (tetraMin[i] > tetraMax[i])
                    {
                        mValid[t] = false;
                    }
                }
            }
        }

        void TransformToGridCoordinates(std::array<T, 3> const& regionMin,
            std::array<T, 3> const& regionMax, std::array<size_t, 3> const& bound)
        {
            std::array<T, 3> multiplier{};
            for (size_t i = 0; i < 3; ++i)
            {
                multiplier[i] = static_cast<T>(bound[i] - 1) / (regionMax[i] - regionMin[i]);
            }

            for (size_t v = 0; v < mNumVertices; ++v)
            {
                auto const& vertex = mVertices[v];
                auto& gridVertex = mGridVertices[v];
                for (size_t i = 0; i < 3; ++i)
                {
                    gridVertex[i] = multiplier[i] * (vertex[i] - regionMin[i]);
                }
            }

            for (size_t t = 0; t < mNumTetrahedra; ++t)
            {
                auto const& tetraMin = mTetraMin[t];
                auto const& tetraMax = mTetraMax[t];
                auto& gridTetraMin = mGridTetraMin[t];
                auto& gridTetraMax = mGridTetraMax[t];
                for (size_t i = 0; i < 3; ++i)
                {
                    gridTetraMin[i] = static_cast<size_t>(std::ceil(
                        multiplier[i] * (tetraMin[i] - regionMin[i])));

                    gridTetraMax[i] = static_cast<size_t>(std::floor(
                        multiplier[i] * (tetraMax[i] - regionMin[i])));
                }
            }
        }

        void SingleThreadedRasterizer(std::array<size_t, 3> const& bound,
            std::vector<int32_t>& grid)
        {
            for (size_t t = 0; t < mNumTetrahedra; ++t)
            {
                if (mValid[t])
                {
                    Rasterize(t, bound, grid);
                }
            }
        }

        void MultiThreadedRasterizer(size_t numThreads,
            std::array<size_t, 3> const& bound, std::vector<int32_t>& grid)
        {
            // Partition the data for multiple threads.
            size_t const numTetrahedraPerThread = mNumTetrahedra / numThreads;
            std::vector<size_t> nmin(numThreads), nsup(numThreads);
            for (size_t k = 0; k < numThreads; ++k)
            {
                nmin[k] = k * numTetrahedraPerThread;
                nsup[k] = (k + 1) * numTetrahedraPerThread;
            }
            nsup[numThreads - 1] = mNumTetrahedra;

            std::vector<std::thread> process(numThreads);
            for (size_t k = 0; k < numThreads; ++k)
            {
                process[k] = std::thread([this, k, &nmin, &nsup, &bound, &grid]()
                {
                    for (size_t t = nmin[k]; t < nsup[k]; ++t)
                    {
                        if (mValid[t])
                        {
                            Rasterize(t, bound, grid);
                        }
                    }
                });
            }

            for (size_t t = 0; t < numThreads; ++t)
            {
                process[t].join();
            }
        }

        void Rasterize(size_t t, std::array<size_t, 3> const& bound,
            std::vector<int32_t>& grid)
        {
            auto const& imin = mGridTetraMin[t];
            auto const& imax = mGridTetraMax[t];
            std::array<T, 3> gridP{};

            for (size_t i2 = imin[2]; i2 <= imax[2]; ++i2)
            {
                gridP[2] = static_cast<T>(i2);
                for (size_t i1 = imin[1]; i1 <= imax[1]; ++i1)
                {
                    gridP[1] = static_cast<T>(i1);

                    size_t i0min;
                    for (i0min = imin[0]; i0min <= imax[0]; ++i0min)
                    {
                        gridP[0] = static_cast<T>(i0min);

                        if (PointInTetrahedron(gridP,
                            mGridVertices[mTetrahedra[t][0]],
                            mGridVertices[mTetrahedra[t][1]],
                            mGridVertices[mTetrahedra[t][2]],
                            mGridVertices[mTetrahedra[t][3]]))
                        {
                            break;
                        }
                    }
                    if (i0min > imax[0])
                    {
                        continue;
                    }

                    size_t i0max;
                    for (i0max = imax[0]; i0max >= i0min; --i0max)
                    {
                        gridP[0] = static_cast<T>(i0max);

                        if (PointInTetrahedron(gridP,
                            mGridVertices[mTetrahedra[t][0]],
                            mGridVertices[mTetrahedra[t][1]],
                            mGridVertices[mTetrahedra[t][2]],
                            mGridVertices[mTetrahedra[t][3]]))
                        {
                            break;
                        }
                    }

                    size_t const base = bound[0] * (i1 + bound[1] * i2);
                    int32_t const tetrahedronIndex = static_cast<int32_t>(t);
                    for (size_t i0 = i0min, j = i0 + base; i0 <= i0max; ++i0, ++j)
                    {
                        grid[j] = tetrahedronIndex;
                    }
                }
            }
        }

        static bool PointInTetrahedron(std::array<T, 3> const& P,
            std::array<T, 3> const& V0, std::array<T, 3> const& V1,
            std::array<T, 3> const& V2, std::array<T, 3> const& V3)
        {
            T const zero = static_cast<T>(0);

            std::array<T, 3> PmV0 = Sub(P, V0);
            std::array<T, 3> V1mV0 = Sub(V1, V0);
            std::array<T, 3> V2mV0 = Sub(V2, V0);
            if (DotCross(PmV0, V2mV0, V1mV0) > zero)
            {
                return false;
            }

            std::array<T, 3> V3mV0 = Sub(V3, V0);
            if (DotCross(PmV0, V1mV0, V3mV0) > zero)
            {
                return false;
            }

            if (DotCross(PmV0, V3mV0, V2mV0) > zero)
            {
                return false;
            }

            std::array<T, 3> PmV1 = Sub(P, V1);
            std::array<T, 3> V2mV1 = Sub(V2, V1);
            std::array<T, 3> V3mV1 = Sub(V3, V1);
            if (DotCross(PmV1, V2mV1, V3mV1) > zero)
            {
                return false;
            }

            return true;
        }

        inline static std::array<T, 3> Sub(std::array<T, 3> const& U,
            std::array<T, 3> const& V)
        {
            std::array<T, 3> sub = { U[0] - V[0], U[1] - V[1], U[2] - V[2] };
            return sub;
        }

        inline static T Dot(std::array<T, 3> const& U, std::array<T, 3> const& V)
        {
            T dot = U[0] * V[0] + U[1] * V[1] + U[2] * V[2];
            return dot;
        }

        inline static std::array<T, 3> Cross(std::array<T, 3> const& U,
            std::array<T, 3> const& V)
        {
            std::array<T, 3> cross =
            {
                U[1] * V[2] - U[2] * V[1],
                U[2] * V[0] - U[0] * V[2],
                U[0] * V[1] - U[1] * V[0]
            };
            return cross;
        }

        inline static T DotCross(std::array<T, 3> const& U, std::array<T, 3> const& V,
            std::array<T, 3> const& W)
        {
            return Dot(U, Cross(V, W));
        }

        // Constructor arguments.
        size_t mNumVertices;
        std::array<T, 3> const* mVertices;
        size_t mNumTetrahedra;
        std::array<size_t, 4> const* mTetrahedra;

        // Axis-aligned bounding boxes for the tetrahedra.
        std::vector<std::array<T, 3>> mTetraMin;
        std::vector<std::array<T, 3>> mTetraMax;
        std::vector<bool> mValid;

        // Vertices and axis-aligned bounding boxes in grid coordinates.
        std::vector<std::array<T, 3>> mGridVertices;
        std::vector<std::array<size_t, 3>> mGridTetraMin;
        std::vector<std::array<size_t, 3>> mGridTetraMax;
    };
}
