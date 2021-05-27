// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.25

#pragma once

#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/ETManifoldMesh.h>
#include <cstdint>
#include <cstring>
#include <functional>
#include <thread>

// This class is an implementation of the barycentric mapping algorithm
// described in Section 5.3 of the book
//     Polygon Mesh Processing
//     Mario Botsch, Leif Kobbelt, Mark Pauly, Pierre Alliez, Bruno Levy
//     AK Peters, Ltd., Natick MA, 2010
// It uses the mean value weights described in Section 5.3.1 to allow the mesh
// geometry to influence the texture coordinate generation, and it uses
// Gauss-Seidel iteration to solve the sparse linear system.  The authors'
// advice is that the Gauss-Seidel approach works well for at most about 5000
// vertices, presumably the convergence rate degrading as the number of
// vertices increases.
//
// The algorithm implemented here has an additional preprocessing step that
// computes a topological distance transform of the vertices.  The boundary
// texture coordinates are propagated inward by updating the vertices in
// topological distance order, leading to fast convergence for large numbers
// of vertices.

namespace gte
{
    template <typename Real>
    class GenerateMeshUV
    {
    public:
        // Construction and destruction.  Set the number of threads to 0 when
        // you want the code to run in the main thread of the applications.
        // Set the number of threads to a positive number when you want the
        // code to run multithreaded on the CPU.  Derived classes that use the
        // GPU ignore the number of threads, setting the constructor input to
        // std::numeric_limits<uint32_t>::max().  Provide a callback when you
        // want to monitor each iteration of the uv-solver.  The input to the
        // progress callback is the current iteration; it starts at 1 and
        // increases to the numIterations input to the operator() member
        // function.
        GenerateMeshUV(uint32_t numThreads,
            std::function<void(uint32_t)> const* progress = nullptr)
            :
            mNumThreads(numThreads),
            mProgress(progress),
            mNumVertices(0),
            mVertices(nullptr),
            mTCoords(nullptr),
            mNumBoundaryEdges(0),
            mBoundaryStart(0)
        {
        }

        virtual ~GenerateMeshUV() = default;

        // The incoming mesh must be edge-triangle manifold and have rectangle
        // topology (simply connected, closed polyline boundary).  The arrays
        // 'vertices' and 'tcoords' must both have 'numVertices' elements.
        // Set 'useSquareTopology' to true for the generated coordinates to
        // live in the uv-square [0,1]^2.  Set it to false for the generated
        // coordinates to live in a convex polygon that inscribes the uv-disk
        // of center (1/2,1/2) and radius 1/2.
        void operator()(uint32_t numIterations, bool useSquareTopology,
            int numVertices, Vector3<Real> const* vertices, int numIndices,
            int const* indices, Vector2<Real>* tcoords)
        {
            // Ensure that numIterations is even, which avoids having a memory
            // copy from the temporary ping-pong buffer to 'tcoords'.
            if (numIterations & 1)
            {
                ++numIterations;
            }

            mNumVertices = numVertices;
            mVertices = vertices;
            mTCoords = tcoords;

            // The linear system solver has a first pass to initialize the
            // texture coordinates to ensure the Gauss-Seidel iteration
            // converges rapidly.  This requires the texture coordinates all
            // start as (-1,-1).
            for (int i = 0; i < numVertices; ++i)
            {
                mTCoords[i][0] = (Real)-1;
                mTCoords[i][1] = (Real)-1;
            }

            // Create the manifold mesh data structure.
            mGraph.Clear();
            int const numTriangles = numIndices / 3;
            for (int t = 0; t < numTriangles; ++t)
            {
                int v0 = *indices++;
                int v1 = *indices++;
                int v2 = *indices++;
                mGraph.Insert(v0, v1, v2);
            }

            TopologicalVertexDistanceTransform();

            if (useSquareTopology)
            {
                AssignBoundaryTextureCoordinatesSquare();
            }
            else
            {
                AssignBoundaryTextureCoordinatesDisk();
            }

            ComputeMeanValueWeights();
            SolveSystem(numIterations);
        }

    protected:
        // A CPU-based implementation is provided by this class.  The derived
        // classes using the GPU override this function.
        virtual void SolveSystemInternal(uint32_t numIterations)
        {
            if (mNumThreads > 1)
            {
                SolveSystemCPUMultiple(numIterations);
            }
            else
            {
                SolveSystemCPUSingle(numIterations);
            }
        }

        // Constructor inputs.
        uint32_t mNumThreads;
        std::function<void(uint32_t)> const* mProgress;

        // Convenience members that store the input parameters to operator().
        int mNumVertices;
        Vector3<Real> const* mVertices;
        Vector2<Real>* mTCoords;

        // The edge-triangle manifold graph, where each edge is shared by at
        // most two triangles.
        ETManifoldMesh mGraph;

        // The mVertexInfo array stores -1 for the interior vertices.  For a
        // boundary edge <v0,v1> that is counterclockwise,
        // mVertexInfo[v0] = v1, which gives us an orded boundary polyline.
        enum { INTERIOR_VERTEX = -1 };
        std::vector<int> mVertexInfo;
        int mNumBoundaryEdges, mBoundaryStart;
        typedef ETManifoldMesh::Edge Edge;
        std::set<Edge*> mInteriorEdges;

        // The vertex graph required to set up a sparse linear system of
        // equations to determine the texture coordinates.
        struct Vertex
        {
            // The topological distance from the boundary of the mesh.
            int distance;

            // The value range0 is the index into mVertexGraphData for the
            // first adjacent vertex.  The value range1 is the number of
            // adjacent vertices.
            int range0, range1;

            // Unused on the CPU. The padding is necessary for the HLSL and
            // GLSL programs in GPUGenerateMeshUV.h.
            int padding;
        };

        std::vector<Vertex> mVertexGraph;
        std::vector<std::pair<int, Real>> mVertexGraphData;

        // The vertices are listed in the order determined by a topological
        // distance transform.  Boundary vertices have 'distance' 0.  Any
        // vertices that are not boundary vertices but are edge-adjacent to
        // boundary vertices have 'distance' 1.  Neighbors of those have
        // distance '2', and so on.  The mOrderedVertices array stores
        // distance-0 vertices first, distance-1 vertices second, and so on.
        std::vector<int> mOrderedVertices;

    private:
        void TopologicalVertexDistanceTransform()
        {
            // Initialize the graph information.
            mVertexInfo.resize(mNumVertices);
            std::fill(mVertexInfo.begin(), mVertexInfo.end(), INTERIOR_VERTEX);
            mVertexGraph.resize(mNumVertices);
            mVertexGraphData.resize(2 * mGraph.GetEdges().size());
            std::pair<int, Real> initialData = std::make_pair(-1, (Real)-1);
            std::fill(mVertexGraphData.begin(), mVertexGraphData.end(), initialData);
            mOrderedVertices.resize(mNumVertices);
            mInteriorEdges.clear();
            mNumBoundaryEdges = 0;
            mBoundaryStart = std::numeric_limits<int>::max();

            // Count the number of adjacent vertices for each vertex.  For
            // data sets with a large number of vertices, this is a
            // preprocessing step to avoid a dynamic data structure that has
            // a large number of std:map objects that take a very long time
            // to destroy when a debugger is attached to the executable.
            // Instead, we allocate a single array that stores all the
            // adjacency information.  It is also necessary to bundle the
            // data this way for a GPU version of the algorithm.
            std::vector<int> numAdjacencies(mNumVertices);
            std::fill(numAdjacencies.begin(), numAdjacencies.end(), 0);

            for (auto const& element : mGraph.GetEdges())
            {
                ++numAdjacencies[element.first.V[0]];
                ++numAdjacencies[element.first.V[1]];

                if (element.second->T[1])
                {
                    // This is an interior edge.
                    mInteriorEdges.insert(element.second.get());
                }
                else
                {
                    // This is a boundary edge.  Determine the ordering of the
                    // vertex indices to make the edge counterclockwise.
                    ++mNumBoundaryEdges;
                    int v0 = element.second->V[0], v1 = element.second->V[1];
                    auto tri = element.second->T[0];
                    int i;
                    for (i = 0; i < 3; ++i)
                    {
                        int v2 = tri->V[i];
                        if (v2 != v0 && v2 != v1)
                        {
                            // The vertex is opposite the boundary edge.
                            v0 = tri->V[(i + 1) % 3];
                            v1 = tri->V[(i + 2) % 3];
                            mVertexInfo[v0] = v1;
                            mBoundaryStart = std::min(mBoundaryStart, v0);
                            break;
                        }
                    }
                }
            }

            // Set the range data for each vertex.
            for (int vIndex = 0, aIndex = 0; vIndex < mNumVertices; ++vIndex)
            {
                int numAdjacent = numAdjacencies[vIndex];
                mVertexGraph[vIndex].range0 = aIndex;
                mVertexGraph[vIndex].range1 = numAdjacent;
                aIndex += numAdjacent;
                mVertexGraph[vIndex].padding = 0;
            }

            // Compute a topological distance transform of the vertices.
            std::set<int> currFront;
            for (auto const& element : mGraph.GetEdges())
            {
                int v0 = element.second->V[0], v1 = element.second->V[1];
                for (int i = 0; i < 2; ++i)
                {
                    if (mVertexInfo[v0] == INTERIOR_VERTEX)
                    {
                        mVertexGraph[v0].distance = -1;
                    }
                    else
                    {
                        mVertexGraph[v0].distance = 0;
                        currFront.insert(v0);
                    }

                    // Insert v1 into the first available slot of the
                    // adjacency array.
                    int range0 = mVertexGraph[v0].range0;
                    int range1 = mVertexGraph[v0].range1;
                    for (int j = 0; j < range1; ++j)
                    {
                        std::pair<int, Real>& data = mVertexGraphData[range0 + j];
                        if (data.second == (Real)-1)
                        {
                            data.first = v1;
                            data.second = (Real)0;
                            break;
                        }
                    }

                    std::swap(v0, v1);
                }
            }

            // Use a breadth-first search to propagate the distance
            // information.
            int nextDistance = 1;
            size_t numFrontVertices = currFront.size();
            std::copy(currFront.begin(), currFront.end(), mOrderedVertices.begin());
            while (currFront.size() > 0)
            {
                std::set<int> nextFront;
                for (auto v : currFront)
                {
                    int range0 = mVertexGraph[v].range0;
                    int range1 = mVertexGraph[v].range1;
                    auto* current = &mVertexGraphData[range0];
                    for (int j = 0; j < range1; ++j, ++current)
                    {
                        int a = current->first;
                        if (mVertexGraph[a].distance == -1)
                        {
                            mVertexGraph[a].distance = nextDistance;
                            nextFront.insert(a);
                        }
                    }
                }
                std::copy(nextFront.begin(), nextFront.end(), mOrderedVertices.begin() + numFrontVertices);
                numFrontVertices += nextFront.size();
                currFront = std::move(nextFront);
                ++nextDistance;
            }
        }

        void AssignBoundaryTextureCoordinatesSquare()
        {
            // Map the boundary of the mesh to the unit square [0,1]^2.  The
            // selection of square vertices is such that the relative
            // distances between boundary vertices and the relative distances
            // between polygon vertices is preserved, except that the four
            // corners of the square are required to have boundary points
            // mapped to them.  The first boundary point has an implied
            // distance of zero.  The value distance[i] is the length of the
            // boundary polyline from vertex 0 to vertex i+1.
            std::vector<Real> distance(mNumBoundaryEdges);
            Real total = (Real)0;
            int v0 = mBoundaryStart, v1, i;
            for (i = 0; i < mNumBoundaryEdges; ++i)
            {
                v1 = mVertexInfo[v0];
                total += Length(mVertices[v1] - mVertices[v0]);
                distance[i] = total;
                v0 = v1;
            }

            Real invTotal = (Real)1 / total;
            for (auto& d : distance)
            {
                d *= invTotal;
            }

            auto begin = distance.begin(), end = distance.end();
            int endYMin = (int)(std::lower_bound(begin, end, (Real)0.25) - begin);
            int endXMax = (int)(std::lower_bound(begin, end, (Real)0.50) - begin);
            int endYMax = (int)(std::lower_bound(begin, end, (Real)0.75) - begin);
            int endXMin = (int)distance.size() - 1;

            // The first polygon vertex is (0,0).  The remaining vertices are
            // chosen counterclockwise around the square.
            v0 = mBoundaryStart;
            mTCoords[v0][0] = (Real)0;
            mTCoords[v0][1] = (Real)0;
            for (i = 0; i < endYMin; ++i)
            {
                v1 = mVertexInfo[v0];
                mTCoords[v1][0] = distance[i] * (Real)4;
                mTCoords[v1][1] = (Real)0;
                v0 = v1;
            }

            v1 = mVertexInfo[v0];
            mTCoords[v1][0] = (Real)1;
            mTCoords[v1][1] = (Real)0;
            v0 = v1;
            for (++i; i < endXMax; ++i)
            {
                v1 = mVertexInfo[v0];
                mTCoords[v1][0] = (Real)1;
                mTCoords[v1][1] = distance[i] * (Real)4 - (Real)1;
                v0 = v1;
            }

            v1 = mVertexInfo[v0];
            mTCoords[v1][0] = (Real)1;
            mTCoords[v1][1] = (Real)1;
            v0 = v1;
            for (++i; i < endYMax; ++i)
            {
                v1 = mVertexInfo[v0];
                mTCoords[v1][0] = (Real)3 - distance[i] * (Real)4;
                mTCoords[v1][1] = (Real)1;
                v0 = v1;
            }

            v1 = mVertexInfo[v0];
            mTCoords[v1][0] = (Real)0;
            mTCoords[v1][1] = (Real)1;
            v0 = v1;
            for (++i; i < endXMin; ++i)
            {
                v1 = mVertexInfo[v0];
                mTCoords[v1][0] = (Real)0;
                mTCoords[v1][1] = (Real)4 - distance[i] * (Real)4;
                v0 = v1;
            }
        }

        void AssignBoundaryTextureCoordinatesDisk()
        {
            // Map the boundary of the mesh to a convex polygon. The selection
            // of convex polygon vertices is such that the relative distances
            // between boundary vertices and the relative distances between
            // polygon vertices is preserved.  The first boundary point has an
            // implied distance of zero.  The value distance[i] is the length
            // of the boundary polyline from vertex 0 to vertex i+1.
            std::vector<Real> distance(mNumBoundaryEdges);
            Real total = (Real)0;
            int v0 = mBoundaryStart;
            for (int i = 0; i < mNumBoundaryEdges; ++i)
            {
                int v1 = mVertexInfo[v0];
                total += Length(mVertices[v1] - mVertices[v0]);
                distance[i] = total;
                v0 = v1;
            }

            // The convex polygon lives in [0,1]^2 and inscribes a circle with
            // center (1/2,1/2) and radius 1/2.  The polygon center is not
            // necessarily the circle center!  This is the case when a
            // boundary edge has length larger than half the total length of
            // the boundary polyline; we do not expect such data for our
            // meshes.  The first polygon vertex is (1/2,0).  The remaining
            // vertices are chosen counterclockwise around the polygon.
            Real multiplier = (Real)GTE_C_TWO_PI / total;
            v0 = mBoundaryStart;
            mTCoords[v0][0] = (Real)1;
            mTCoords[v0][1] = (Real)0.5;
            for (int i = 1; i < mNumBoundaryEdges; ++i)
            {
                int v1 = mVertexInfo[v0];
                Real angle = multiplier * distance[i - 1];
                mTCoords[v1][0] = (std::cos(angle) + (Real)1) * (Real)0.5;
                mTCoords[v1][1] = (std::sin(angle) + (Real)1) * (Real)0.5;
                v0 = v1;
            }
        }

        void ComputeMeanValueWeights()
        {
            for (auto const& edge : mInteriorEdges)
            {
                int v0 = edge->V[0], v1 = edge->V[1];
                for (int i = 0; i < 2; ++i)
                {
                    // Compute the direction from X0 to X1 and compute the
                    // length of the edge (X0,X1).
                    Vector3<Real> X0 = mVertices[v0];
                    Vector3<Real> X1 = mVertices[v1];
                    Vector3<Real> X1mX0 = X1 - X0;
                    Real x1mx0length = Normalize(X1mX0);
                    Real weight;
                    if (x1mx0length > (Real)0)
                    {
                        // Compute the weight for X0 associated with X1.
                        weight = (Real)0;
                        for (int j = 0; j < 2; ++j)
                        {
                            // Find the vertex of triangle T[j] opposite edge
                            // <X0,X1>.
                            auto tri = edge->T[j];
                            int k;
                            for (k = 0; k < 3; ++k)
                            {
                                int v2 = tri->V[k];
                                if (v2 != v0 && v2 != v1)
                                {
                                    Vector3<Real> X2 = mVertices[v2];
                                    Vector3<Real> X2mX0 = X2 - X0;
                                    Real x2mx0Length = Normalize(X2mX0);
                                    if (x2mx0Length > (Real)0)
                                    {
                                        Real dot = Dot(X2mX0, X1mX0);
                                        Real cs = std::min(std::max(dot, (Real)-1), (Real)1);
                                        Real angle = std::acos(cs);
                                        weight += std::tan(angle * (Real)0.5);
                                    }
                                    else
                                    {
                                        weight += (Real)1;
                                    }
                                    break;
                                }
                            }
                        }
                        weight /= x1mx0length;
                    }
                    else
                    {
                        weight = (Real)1;
                    }

                    int range0 = mVertexGraph[v0].range0;
                    int range1 = mVertexGraph[v0].range1;
                    for (int j = 0; j < range1; ++j)
                    {
                        std::pair<int, Real>& data = mVertexGraphData[range0 + j];
                        if (data.first == v1)
                        {
                            data.second = weight;
                        }
                    }

                    std::swap(v0, v1);
                }
            }
        }

        void SolveSystem(uint32_t numIterations)
        {
            // On the first pass, average only neighbors whose texture
            // coordinates have been computed.  This is a good initial guess
            // for the linear system and leads to relatively fast convergence
            // of the Gauss-Seidel iterates.
            Real zero = (Real)0;
            for (int i = mNumBoundaryEdges; i < mNumVertices; ++i)
            {
                int v0 = mOrderedVertices[i];
                int range0 = mVertexGraph[v0].range0;
                int range1 = mVertexGraph[v0].range1;
                auto const* current = &mVertexGraphData[range0];
                Vector2<Real> tcoord{ zero, zero };
                Real weight, weightSum = zero;
                for (int j = 0; j < range1; ++j, ++current)
                {
                    int v1 = current->first;
                    if (mTCoords[v1][0] != -1.0f)
                    {
                        weight = current->second;
                        weightSum += weight;
                        tcoord += weight * mTCoords[v1];
                    }
                }
                tcoord /= weightSum;
                mTCoords[v0] = tcoord;
            }

            SolveSystemInternal(numIterations);
        }

        void SolveSystemCPUSingle(uint32_t numIterations)
        {
            // Use ping-pong buffers for the texture coordinates.
            std::vector<Vector2<Real>> tcoords(mNumVertices);
            size_t numBytes = mNumVertices * sizeof(Vector2<Real>);
            std::memcpy(&tcoords[0], mTCoords, numBytes);
            Vector2<Real>* inTCoords = mTCoords;
            Vector2<Real>* outTCoords = &tcoords[0];

            // The value numIterations is even, so we always swap an even
            // number of times.  This ensures that on exit from the loop,
            // outTCoords is tcoords.
            for (uint32_t i = 1; i <= numIterations; ++i)
            {
                if (mProgress)
                {
                    (*mProgress)(i);
                }

                for (int j = mNumBoundaryEdges; j < mNumVertices; ++j)
                {
                    int v0 = mOrderedVertices[j];
                    int range0 = mVertexGraph[v0].range0;
                    int range1 = mVertexGraph[v0].range1;
                    auto const* current = &mVertexGraphData[range0];
                    Vector2<Real> tcoord{ (Real)0, (Real)0 };
                    Real weight, weightSum = (Real)0;
                    for (int k = 0; k < range1; ++k, ++current)
                    {
                        int v1 = current->first;
                        weight = current->second;
                        weightSum += weight;
                        tcoord += weight * inTCoords[v1];
                    }
                    tcoord /= weightSum;
                    outTCoords[v0] = tcoord;
                }

                std::swap(inTCoords, outTCoords);
            }
        }

        void SolveSystemCPUMultiple(uint32_t numIterations)
        {
            // Use ping-pong buffers for the texture coordinates.
            std::vector<Vector2<Real>> tcoords(mNumVertices);
            size_t numBytes = mNumVertices * sizeof(Vector2<Real>);
            std::memcpy(&tcoords[0], mTCoords, numBytes);
            Vector2<Real>* inTCoords = mTCoords;
            Vector2<Real>* outTCoords = &tcoords[0];

            // Partition the data for multiple threads.
            int numV = mNumVertices - mNumBoundaryEdges;
            int numVPerThread = numV / mNumThreads;
            std::vector<int> vmin(mNumThreads), vmax(mNumThreads);
            for (uint32_t t = 0; t < mNumThreads; ++t)
            {
                vmin[t] = mNumBoundaryEdges + t * numVPerThread;
                vmax[t] = vmin[t] + numVPerThread - 1;
            }
            vmax[mNumThreads - 1] = mNumVertices - 1;

            // The value numIterations is even, so we always swap an even
            // number of times.  This ensures that on exit from the loop,
            // outTCoords is tcoords.
            for (uint32_t i = 1; i <= numIterations; ++i)
            {
                if (mProgress)
                {
                    (*mProgress)(i);
                }

                // Execute Gauss-Seidel iterations in multiple threads.
                std::vector<std::thread> process(mNumThreads);
                for (uint32_t t = 0; t < mNumThreads; ++t)
                {
                    process[t] = std::thread([this, t, &vmin, &vmax, inTCoords,
                        outTCoords]()
                        {
                            for (int j = vmin[t]; j <= vmax[t]; ++j)
                            {
                                int v0 = mOrderedVertices[j];
                                int range0 = mVertexGraph[v0].range0;
                                int range1 = mVertexGraph[v0].range1;
                                auto const* current = &mVertexGraphData[range0];
                                Vector2<Real> tcoord{ (Real)0, (Real)0 };
                                Real weight, weightSum = (Real)0;
                                for (int k = 0; k < range1; ++k, ++current)
                                {
                                    int v1 = current->first;
                                    weight = current->second;
                                    weightSum += weight;
                                    tcoord += weight * inTCoords[v1];
                                }
                                tcoord /= weightSum;
                                outTCoords[v0] = tcoord;
                            }
                        });
                }

                // Wait for all threads to finish.
                for (uint32_t t = 0; t < mNumThreads; ++t)
                {
                    process[t].join();
                }

                std::swap(inTCoords, outTCoords);
            }
        }
    };
}
