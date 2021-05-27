// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.9.2021.04.22

#pragma once
#include <Mathematics/Logger.h>
#include <Mathematics/ConvexHull3.h>
#include <Mathematics/MinimumAreaBox2.h>
#include <Mathematics/VETManifoldMesh.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/UniqueVerticesSimplices.h>
#include <cstring>

// Compute a minimum-volume oriented box containing the specified points. The
// algorithm is really about computing the minimum-volume box containing the
// convex hull of the points, so you must compute the convex hull or pass an
// an already built hull to the code. The convex hull is, of course, a convex
// polyhedron.
//
// According to
//   J.O'Rourke, "Finding minimal enclosing boxes",
//   Internat. J. Comput. Inform. Sci., 14:183-199, 1985.
// the minimum-volume oriented box must have at least two adjacent faces
// flush with edges of the convex polyhedron. The implementation processes
// all pairs of edges, determining for each pair the relevant box-face
// normals that are candidates for the minimum-volume box. I use an approach
// different from that of the details of proof in the paper; see
//   https://www.geometrictools.com/Documentation/MinimumVolumeBox.pdf
// The computations involve an iterative minimizer, so you cannot expect
// to obtain the exact minimum-volume box, but you will obtain a good
// approximation to it based on how many samples the minimizer uses in its
// search. You can also derive from a class and override the virtual
// functions that are used for minimization in order to provided your own
// minimizer algorithm.

namespace gte
{
    // The InputType is 'float' or 'double'. The ComputeType is 'double' when
    // computeDouble is 'true' or it is the appropriate fixed-precision
    // BSNumber<> class when computeDouble is 'false'. If you use rational
    // arithmetic for the computations, you must increase the default program
    // stack size significantly. In Microsoft Visual Studio, I set the Stack
    // Reserve Size to 1 GB (which is 1073741824 bytes and is probably much
    // more than required.).
    template <typename InputType, bool computeDouble>
    class MinimumVolumeBox3
    {
    public:
        // Supporting constants and types for numerical computing.
        static int constexpr NumWords = std::is_same<InputType, float>::value ? 342 : 2561;
        using UIntegerType = UIntegerFP32<NumWords>;
        using ComputeType = typename std::conditional<computeDouble, double, BSNumber<UIntegerType>>::type;
        using RationalType = typename std::conditional<computeDouble, double, BSRational<UIntegerType>>::type;
        using VCompute3 = Vector3<ComputeType>;
        using RVCompute3 = Vector3<RationalType>;

        // Supporting constants types for a compact vertex-edge-triangle mesh
        // that represents the convex polyhedron input.
        static size_t constexpr invalidIndex = std::numeric_limits<size_t>::max();

        struct Edge
        {
            Edge()
                :
                V{ invalidIndex, invalidIndex },
                T{ invalidIndex, invalidIndex }
            {
            }
            std::array<size_t, 2> V;
            std::array<size_t, 2> T;
        };

        struct Triangle
        {
            Triangle()
                :
                V{ invalidIndex, invalidIndex },
                E{ invalidIndex, invalidIndex },
                T{ invalidIndex, invalidIndex }
            {
            }

            std::array<size_t, 3> V;
            std::array<size_t, 3> E;
            std::array<size_t, 3> T;
        };

        // Information about candidates for the minimum-volume box and about
        // that box itself.
        struct Candidate
        {
            Candidate()
                :
                edgeIndex{ invalidIndex, invalidIndex },
                edge{},
                N{ VCompute3::Zero(), VCompute3::Zero() },
                M{ VCompute3::Zero(), VCompute3::Zero() },
                f00(static_cast<ComputeType>(0)),
                f10(static_cast<ComputeType>(0)),
                f01(static_cast<ComputeType>(0)),
                f11(static_cast<ComputeType>(0)),
                levelCurveProcessorIndex(invalidIndex),
                axis{ VCompute3::Unit(0), VCompute3::Unit(1), VCompute3::Unit(2) },
                minSupportIndex{ invalidIndex, invalidIndex, invalidIndex },
                maxSupportIndex{ invalidIndex, invalidIndex, invalidIndex },
                volume(static_cast<ComputeType>(0))
            {
            }

            // Set by ProcessEdgePair.
            std::array<size_t, 2> edgeIndex;
            std::array<Edge, 2> edge;
            std::array<VCompute3, 2> N, M;
            ComputeType f00, f10, f01, f11;
            size_t levelCurveProcessorIndex;

            // Set by Pair, MinimizerConstantT, MinimizerConstantS,
            // MinimizerVariableS and MinimizerVariableT. The axis[0] and
            // axis[1] are set by the aforementioned functions. The axis[2]
            // is computed by ComputeVolume.
            std::array<VCompute3, 3> axis;

            // Set by ComputeVolume.
            std::array<size_t, 3> minSupportIndex;
            std::array<size_t, 3> maxSupportIndex;
            RationalType volume;
        };

        // The rational representation of the minimum-volume box. The axis[]
        // vectors are generally not unit length. To obtain a unit-length
        // vector, use axis[i]/std::sqrt(sqrLengthAxis[i]).
        struct RBox
        {
            RBox()
                :
                center(RVCompute3::Zero()),
                axis{ RVCompute3::Zero(), RVCompute3::Zero(), RVCompute3::Zero() },
                sqrLengthAxis(RVCompute3::Zero()),
                volume(static_cast<RationalType>(0))
            {
            }

            RVCompute3 center;
            std::array<RVCompute3, 3> axis;
            RVCompute3 sqrLengthAxis;
            RVCompute3 scaledExtent;
            RationalType volume;
        };

    public:
        // Construction and destruction. To execute in the main thread, set
        // numThreads to 0. To run multithreaded on the CPU, set numThreads
        // to a positive number.
        MinimumVolumeBox3(size_t numThreads = 0)
            :
            mNumThreads(numThreads),
            mDomainIndex{},
            mZero(static_cast<ComputeType>(0)),
            mOne(static_cast<ComputeType>(1)),
            mHalf(static_cast<ComputeType>(0.5)),
            mNumVertices(0),
            mNumTriangles(0),
            mAdjacentPool{},
            mVertexAdjacent{},
            mEdges{},
            mTriangles{},
            mEdgeIndices{},
            mOrigin{},
            mVertices{},
            mNormals{},
            mAlignedCandidate{},
            mMinimumVolumeObject{},
            mLevelCurveProcessor{}
        {
            static_assert(std::is_floating_point<InputType>::value,
                "The input type must be 'float' or 'double'.");

            InitializeLevelCurveProcessors();
        }

        virtual ~MinimumVolumeBox3() = default;

        // The class is a wrapper for operator()(*), so there is no need for
        // copy semantics.
        MinimumVolumeBox3(MinimumVolumeBox3 const&) = delete;
        MinimumVolumeBox3& operator=(MinimumVolumeBox3 const&) = delete;

        // The convex hull of the input points is computed. The output
        // box is determined by the dimension of the hull.
        //   0D: The hull is a single point. The box has center at that
        //       point, the axes are the standard Euclidean basis, and the
        //       extents are all 0.
        //   1D: The hull is a line segment. The box has center at the
        //       midpoint of the segment, axis[0] is the segment direction,
        //       axis[1] and axis[2] are chosen so that the three axes form
        //       a right-handed orthonormal set, extent[0] is the half-length
        //       of the segment, and extent[1] and extent[2] are 0.
        //   2D: The hull is a planar polygon. The box is the minimum-area
        //       box containing that polygon. The axis[0] and axis[1] are
        //       the axis directions for the planar box, axis[2] is normal
        //       to the plane of the polygon, extent[0] and extent[1] are
        //       the extents of the planar box, and extent[2] is 0.
        //   3D: The hull is a convex polyhedron. The other operator()(*)
        //       function is called to compute the minimum-volume box of the
        //       polyhedron.
        // If the dimension is 0, 1 or 2, the objects returned by the
        // GetMinimumVolumeObject() and GetRationalBox() are invalid for this
        // operator()(*).
        int operator()(
            int numPoints,
            Vector3<InputType> const* points,
            size_t lgMaxSample,
            OrientedBox3<InputType>& box,
            InputType& volume)
        {
            LogAssert(numPoints > 0 && points != nullptr && lgMaxSample >= 2,
                "Invalid argument.");

            InputType const zero = static_cast<InputType>(0);
            InputType const one = static_cast<InputType>(1);
            InputType const half = static_cast<InputType>(0.5);

            ConvexHull3<InputType> ch3;
            ch3(static_cast<size_t>(numPoints), points, 0);
            size_t dimension = ch3.GetDimension();
            auto const& hull = ch3.GetHull();

            if (dimension == 0)
            {
                // The points are all the same.
                box.center = points[hull[0]];
                box.axis[0] = { one, zero, zero };
                box.axis[1] = { zero, one, zero };
                box.axis[2] = { zero, zero, one };
                box.extent[0] = zero;
                box.extent[1] = zero;
                box.extent[2] = zero;
                volume = zero;
                return 0;
            }

            if (dimension == 1)
            {
                // The points lie on a line.
                Vector3<InputType> direction = points[hull[1]] - points[hull[0]];
                box.center = half * (points[hull[0]] + points[hull[1]]);
                box.extent[0] = half * Normalize(direction);
                box.extent[1] = zero;
                box.extent[2] = zero;
                box.axis[0] = direction;
                ComputeOrthogonalComplement(1, &box.axis[0]);
                volume = zero;
                return 1;
            }

            if (dimension == 2)
            {
                // The points line on a plane. Get a coordinate system
                // relative to the plane of the points. Choose the origin
                // to be any of the input points.
                Vector3<InputType> origin = points[hull[0]];
                Vector3<InputType> normal = Vector3<InputType>::Zero();
                size_t numHull = hull.size();
                for (size_t i0 = numHull - 1, i1 = 1; i1 < numHull; i0 = i1++)
                {
                    auto const& P0 = points[hull[i0]];
                    auto const& P1 = points[hull[i1]];
                    normal += Cross(P0, P1);
                }

                Vector3<InputType> basis[3];
                basis[0] = normal;
                ComputeOrthogonalComplement(1, basis);

                // Project the input points onto the plane.
                std::vector<Vector2<InputType>> projection(numPoints);
                for (int i = 0; i < numPoints; ++i)
                {
                    Vector3<InputType> diff = points[i] - origin;
                    projection[i][0] = Dot(basis[1], diff);
                    projection[i][1] = Dot(basis[2], diff);
                }

                // Compute the minimum area box in 2D.
                MinimumAreaBox2<InputType, BSRational<UIntegerAP32>> mab2;
                OrientedBox2<InputType> rectangle = mab2(numPoints, &projection[0]);

                // Lift the values into 3D.
                box.center = origin + rectangle.center[0] * basis[1] + rectangle.center[1] * basis[2];
                box.axis[0] = rectangle.axis[0][0] * basis[1] + rectangle.axis[0][1] * basis[2];
                box.axis[1] = rectangle.axis[1][0] * basis[1] + rectangle.axis[1][1] * basis[2];
                box.axis[2] = basis[0];
                box.extent[0] = rectangle.extent[0];
                box.extent[1] = rectangle.extent[1];
                box.extent[2] = zero;
                volume = zero;
                return 2;
            }

            // Remove duplicated vertices and reindex them for the convex
            // polyhedron.
            std::vector<Vector3<InputType>> inVertices(numPoints);
            std::memcpy(inVertices.data(), points, inVertices.size() * sizeof(Vector3<InputType>));
            auto const& triangles = ch3.GetHull();
            std::vector<int> inIndices(triangles.size());
            size_t current = 0;
            for (auto index : triangles)
            {
                inIndices[current++] = static_cast<int>(index);
            }

            UniqueVerticesSimplices<Vector3<InputType>, int, 3> uvt;
            std::vector<Vector3<InputType>> outVertices;
            std::vector<int> outIndices;
            uvt.RemoveDuplicateAndUnusedVertices(inVertices, inIndices,
                outVertices, outIndices);

            operator()(static_cast<int>(outVertices.size()), outVertices.data(),
                static_cast<int>(outIndices.size()), outIndices.data(),
                lgMaxSample, box, volume);

            return 3;
        }

        // The points form a nondegenerate convex polyhedron. The inputs
        // 'vertices' and 'indices' must be nonempty and the 'vertices' must
        // have no duplicates. The triangle faces are triples of the indices;
        // there are indices.size()/3 triangles. Also, 0 <= indices[i] <
        // vertices.size() for all 0 <= i < indices.size(). The logarithm base
        // 2 of maximum sample index must satisfy the condition
        // lgMaxSample >= 2, so there are at least 4 samples. Do not choose it
        // to be too large when using rational computation because the
        // computational costs are excessive. You can override the minimizer
        // functions to use your own minimization algorithm; see the comments
        // before MinimizerConstantT.
        void operator()(
            int numVertices,
            Vector3<InputType> const* inVertices,
            int numIndices,
            int const* inIndices,
            size_t lgMaxSample,
            OrientedBox3<InputType>& box,
            InputType& volume)
        {
            LogAssert(
                numVertices > 0 && inVertices != nullptr &&
                numIndices > 0 && inIndices != nullptr &&
                (numIndices % 3) == 0 && lgMaxSample >= 2,
                "Invalid argument.");
                for (int i = 0; i < numIndices; ++i)
                {
                    LogAssert(0 <= inIndices[i] && inIndices[i] < numVertices,
                        "Invalid index.");
                }

                std::vector<Vector3<InputType>> vertices(numVertices);
                std::memcpy(vertices.data(), inVertices,
                    vertices.size() * sizeof(Vector3<InputType>));
                std::vector<int> indices(numIndices);
                std::memcpy(indices.data(), inIndices,
                    indices.size() * sizeof(int));

                GenerateSubdivision(lgMaxSample);
                CreateCompactMesh(vertices, indices);
                PrepareVerticesAndNormals(vertices);
                ComputeAlignedCandidate();
                GetMinimumVolumeCandidate();
                GetMinimumVolumeBox(box, volume);
        }

        // For more information about the minimum-volume box, access the
        // candidate that stores it.
        inline Candidate const& GetMinimumVolumeObject() const
        {
            return mMinimumVolumeObject;
        }

        // The operator()(*) function returns a floating-point box and volume.
        // If you computed using rational arithmetic, the rational box is
        // accessed by this member function.
        inline void GetRationalBox(RBox& rbox) const
        {
            rbox = mRBox;
        }

    protected:
        void CreateDomainIndex(size_t& current, size_t end0, size_t end1)
        {
            size_t mid = (end0 + end1) / 2;
            if (mid != end0 && mid != end1)
            {
                mDomainIndex[current++] = { mid, end0, end1 };
                CreateDomainIndex(current, end0, mid);
                CreateDomainIndex(current, mid, end1);
            }
        }

        void GenerateSubdivision(size_t lgMaxSample)
        {
            mMaxSample = (static_cast<size_t>(1) << lgMaxSample);
            mDomainIndex.resize(mMaxSample - 1);
            size_t current = 0;
            CreateDomainIndex(current, 0, mMaxSample);
        }

        // The vertices are stored in a vertex-edge-triangle manifold mesh.
        // Each vertex as a set of adjacent vertices, a set of adjacent
        // edges and a set of adjacent triangles. The adjacent vertices are
        // repackaged into mVertexAdjacent[] and mAdjacentPool[]. For
        // vertex v with n adjacent vertices, mVertexAdjacent[v] is the
        // index into mAdjacentPool[] whre the n adjacent vertices are
        // stored. If the adjacent vertices are a[0] through a[n-1], then
        // mAdjacentPool[mVertexAdjacent[v] + i] is a[i].
        void CreateCompactMesh(
            std::vector<Vector3<InputType>> const& vertices,
            std::vector<int> const& indices)
        {
            mNumVertices = vertices.size();
            mNumTriangles = indices.size() / 3;

            VETManifoldMesh mesh;
            int const* current = indices.data();
            for (size_t t = 0; t < mNumTriangles; ++t)
            {
                int v0 = *current++;
                int v1 = *current++;
                int v2 = *current++;
                mesh.Insert(v0, v1, v2);
            }

            // It is implicit in the construction of mVertexAdjacent that
            //   (1) the vertex indices v satisfy 0 <= v < N for a mesh of
            //       N vertices and
            //   (2) the vertex map itself is ordered as <0,vertex0>,
            //       <1,vertex1>, ..., <N-1,vertexNm1>.
            // Condition (1) is guaranteed because the input to the MVB3
            // constructor uses the contiguous indices of the position array.
            // Condition (2) is not guaranteed because VETManifoldMesh::VMap
            // is a std::unordered_map. The vertices must be sorted here to
            // satisfy condition2.
            auto const& vmap = mesh.GetVertices();
            std::map<int, VETManifoldMesh::Vertex*> sortedVMap;
            for (auto const& element : vmap)
            {
                sortedVMap.emplace(element.first, element.second.get());
            }

            size_t numAdjacentPool = 0;
            for (auto const& element : sortedVMap)
            {
                numAdjacentPool += element.second->VAdjacent.size() + 1;
            }
            mAdjacentPool.resize(numAdjacentPool);
            mVertexAdjacent.resize(sortedVMap.size());
            size_t apIndex = 0, vaIndex = 0;
            for (auto const& element : sortedVMap)
            {
                auto const& adjacent = element.second->VAdjacent;
                mVertexAdjacent[vaIndex++] = apIndex;
                mAdjacentPool[apIndex++] = adjacent.size();
                for (auto v : adjacent)
                {
                    mAdjacentPool[apIndex++] = static_cast<size_t>(v);
                }
            }

            auto const& emap = mesh.GetEdges();
            auto const& tmap = mesh.GetTriangles();
            mEdges.resize(emap.size());
            mTriangles.resize(tmap.size());

            std::map<ETManifoldMesh::Edge*, size_t> edgeIndexMap;
            size_t index = 0;
            for (auto const& element : emap)
            {
                edgeIndexMap.emplace(element.second.get(), index);
                for (size_t j = 0; j < 2; ++j)
                {
                    mEdges[index].V[j] = (size_t)element.second->V[j];
                }
                ++index;
            }

            std::map<ETManifoldMesh::Triangle*, size_t> triangleIndexMap;
            index = 0;
            for (auto const& element : tmap)
            {
                triangleIndexMap.emplace(element.second.get(), index);
                for (size_t j = 0; j < 3; ++j)
                {
                    mTriangles[index].V[j] = (size_t)element.second->V[j];
                }
                ++index;
            }

            index = 0;
            for (auto const& element : emap)
            {
                for (size_t j = 0; j < 2; ++j)
                {
                    auto tri = element.second->T[j];
                    auto titer = triangleIndexMap.find(tri);
                    mEdges[index].T[j] = titer->second;
                }
                ++index;
            }

            index = 0;
            for (auto const& element : tmap)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    auto edg = element.second->E[j];
                    auto eiter = edgeIndexMap.find(edg);
                    mTriangles[index].E[j] = eiter->second;
                }
                for (size_t j = 0; j < 3; ++j)
                {
                    auto tri = element.second->T[j];
                    auto titer = triangleIndexMap.find(tri);
                    mTriangles[index].T[j] = titer->second;
                }
                ++index;
            }

            size_t const numEdges = mEdges.size();
            mEdgeIndices.reserve(mEdges.size() * mEdges.size());
            for (size_t e0 = 0; e0 < numEdges; ++e0)
            {
                for (size_t e1 = e0 + 1; e1 < numEdges; ++e1)
                {
                    mEdgeIndices.push_back({ e0, e1 });
                }
            }
        }

        template <bool useDouble = computeDouble>
        typename std::enable_if<useDouble, void>::type
            ComputeNormal(VCompute3 const& edge0, VCompute3 const& edge1, VCompute3& normal)
        {
            normal = UnitCross(edge0, edge1);
        }

        template <bool useDouble = computeDouble>
        typename std::enable_if<!useDouble, void>::type
            ComputeNormal(VCompute3 const& edge0, VCompute3 const& edge1, VCompute3& normal)
        {
            normal = Cross(edge0, edge1);
        }

        void PrepareVerticesAndNormals(std::vector<Vector3<InputType>> const& vertices)
        {
            // Convert from floating-point type to the compute type (double or
            // rational). The origin is considered to be inVertices[0]).
            mVertices.resize(mNumVertices);
            for (int32_t j = 0; j < 3; ++j)
            {
                mOrigin[j] = static_cast<ComputeType>(vertices[0][j]);
                mVertices[0][j] = static_cast<ComputeType>(0);
            }
            for (size_t i = 1; i < mNumVertices; ++i)
            {
                for (int32_t j = 0; j < 3; ++j)
                {
                    mVertices[i][j] = static_cast<ComputeType>(vertices[i][j]) - mOrigin[j];
                }
            }

            // Compute inner-pointing normals that are not required to be
            // unit length.
            mNormals.resize(mNumTriangles);
            for (size_t i = 0; i < mNumTriangles; ++i)
            {
                auto const& tri = mTriangles[i];
                size_t v0 = tri.V[0], v1 = tri.V[1], v2 = tri.V[2];
                VCompute3 edge10 = mVertices[v1] - mVertices[v0];
                VCompute3 edge20 = mVertices[v2] - mVertices[v0];
                ComputeNormal(edge20, edge10, mNormals[i]);
            }
        }

        void ComputeAlignedCandidate()
        {
            VCompute3 pmin, pmax;
            for (int32_t j = 0; j < 3; ++j)
            {
                mAlignedCandidate.maxSupportIndex[j] =
                    GetExtreme(mAlignedCandidate.axis[j], pmax[j]);
                mAlignedCandidate.minSupportIndex[j] =
                    GetExtreme(-mAlignedCandidate.axis[j], pmin[j]);
                pmin[j] = -pmin[j];
            }
            VCompute3 diff = pmax - pmin;
            mAlignedCandidate.volume = diff[0] * diff[1] * diff[2];
        }

        size_t GetExtreme(VCompute3 const& direction, ComputeType& dMax)
        {
            size_t vMax = 0;
            dMax = Dot(direction, mVertices[vMax]);
            for (size_t i = 0; i < mNumVertices; ++i)
            {
                size_t vLocalMax = vMax;
                ComputeType dLocalMax = dMax;
                size_t const* adjacent = &mAdjacentPool[mVertexAdjacent[vMax]];
                size_t numAdjacent = *adjacent++;
                for (size_t j = 1; j <= numAdjacent; ++j)
                {
                    size_t vCandidate = *adjacent++;
                    ComputeType dCandidate = Dot(direction, mVertices[vCandidate]);
                    if (dCandidate > dLocalMax)
                    {
                        vLocalMax = vCandidate;
                        dLocalMax = dCandidate;
                    }
                }
                if (vMax != vLocalMax)
                {
                    vMax = vLocalMax;
                    dMax = dLocalMax;
                }
                else
                {
                    break;
                }
            }

            return vMax;
        }

        void ComputeVolume(Candidate& candidate)
        {
            // The last axis is needed only when computing the volume for
            // comparison to the current candidate volume, so compute this
            // axis now.
            candidate.axis[2] = Cross(candidate.axis[0], candidate.axis[1]);

            VCompute3 pmin, pmax;
            candidate.minSupportIndex[0] = mEdges[candidate.edgeIndex[0]].V[0];
            pmin[0] = Dot(candidate.axis[0], mVertices[candidate.minSupportIndex[0]]);
            candidate.maxSupportIndex[0] = GetExtreme(candidate.axis[0], pmax[0]);
            candidate.minSupportIndex[1] = mEdges[candidate.edgeIndex[1]].V[0];
            pmin[1] = Dot(candidate.axis[1], mVertices[candidate.minSupportIndex[1]]);
            candidate.maxSupportIndex[1] = GetExtreme(candidate.axis[1], pmax[1]);
            candidate.axis[2] = Cross(candidate.axis[0], candidate.axis[1]);
            candidate.minSupportIndex[2] = GetExtreme(-candidate.axis[2], pmin[2]);
            pmin[2] = -pmin[2];
            candidate.maxSupportIndex[2] = GetExtreme(candidate.axis[2], pmax[2]);
            VCompute3 diff = pmax - pmin;
            candidate.volume =
                static_cast<RationalType>(diff[0] * diff[1] * diff[2]) /
                static_cast<RationalType>(Dot(candidate.axis[2], candidate.axis[2]));
        }

        void ProcessEdgePair(std::array<size_t, 2> const& edgeIndex, Candidate& mvCandidate)
        {
            // Examine the zero-valued level curves for
            // F(s,t)
            // = Dot((1-s)*edge0.N0 + s*edge0.N1, (1-t)*edge1.N0 + t*edge1.N1)
            // = (1-s)*(1-t)*Dot(edge0.N0,edge1.N0)
            //   + (1-s)*t*Dot(edge0.N0,edge1.N1)
            //   + s*(1-t)*Dot(edge0.N1,edge1.N0)
            //   + s*t*Dot(edge0.N1,edge1.N1)
            // = (1-s)*(1-t)*f00 + (1-s)*t*f01 + s*(1-t)*f10 + s*t*f11
            // = a00 + a10*s + a01*t + a11*s*t
            // = [(a00*a11 - a01*a10) + (a01 + a11*s)*(a10 + a11*t)]/a11
            // where a00 = f00, a10 = f10-f00, a01 = f01-f00 and
            // a11 = f00-f01-f10+f11.  Let d = a00*a11 - a01*a10 =
            // f00*f11 - f01*f10. If d = 0, then the level curves are
            // s = -a01/a11 and t = -a10/a11. If d != 0, then the level curves
            // are hyperbolic curves with asymptotes s = -a01/a11 and
            // t = -a10/a11.

            Candidate candidate = mAlignedCandidate;
            candidate.edgeIndex = edgeIndex;
            Edge const& edge0 = mEdges[candidate.edgeIndex[0]];
            Edge const& edge1 = mEdges[candidate.edgeIndex[1]];
            candidate.edge[0] = edge0;
            candidate.edge[1] = edge1;
            candidate.N[0] = mNormals[edge0.T[0]];
            candidate.N[1] = mNormals[edge0.T[1]];
            candidate.M[0] = mNormals[edge1.T[0]];
            candidate.M[1] = mNormals[edge1.T[1]];
            candidate.f00 = Dot(candidate.N[0], candidate.M[0]);
            candidate.f10 = Dot(candidate.N[1], candidate.M[0]);
            candidate.f01 = Dot(candidate.N[0], candidate.M[1]);
            candidate.f11 = Dot(candidate.N[1], candidate.M[1]);

            uint32_t bits00 = (candidate.f00 > mZero ? 1 : (candidate.f00 < mZero ? 2 : 0));
            uint32_t bits10 = (candidate.f10 > mZero ? 1 : (candidate.f10 < mZero ? 2 : 0));
            uint32_t bits01 = (candidate.f01 > mZero ? 1 : (candidate.f01 < mZero ? 2 : 0));
            uint32_t bits11 = (candidate.f11 > mZero ? 1 : (candidate.f11 < mZero ? 2 : 0));
            uint32_t index = bits00 | (bits10 << 2) | (bits01 << 4) | (bits11 << 6);
            if (index != 0x55 && index != 0xaa)
            {
                candidate.levelCurveProcessorIndex = index;
                (this->*mLevelCurveProcessor[candidate.levelCurveProcessorIndex])(candidate, mvCandidate);
            }
        }

        void GetMinimumVolumeCandidate()
        {
            mMinimumVolumeObject = mAlignedCandidate;

            if (mNumThreads > 0)
            {
                size_t const numPairsPerThread = mEdgeIndices.size() / mNumThreads;
                std::vector<size_t> imin(mNumThreads), imax(mNumThreads);
                for (size_t t = 0; t < mNumThreads; ++t)
                {
                    imin[t] = t * numPairsPerThread;
                    imax[t] = (t + 1) * numPairsPerThread;
                }
                imax.back() = mEdgeIndices.size();

                std::vector<Candidate> candidates(mNumThreads);
                std::vector<std::thread> process(mNumThreads);
                for (size_t t = 0; t < mNumThreads; ++t)
                {
                    process[t] = std::thread(
                        [this, t, &imin, &imax, &candidates]()
                        {
                            candidates[t] = mAlignedCandidate;
                            for (size_t i = imin[t]; i < imax[t]; ++i)
                            {
                                ProcessEdgePair(mEdgeIndices[i], candidates[t]);
                            }
                        });
                }

                for (size_t t = 0; t < mNumThreads; ++t)
                {
                    process[t].join();
                    if (candidates[t].volume < mMinimumVolumeObject.volume)
                    {
                        mMinimumVolumeObject = candidates[t];
                    }
                }
            }
            else
            {
                for (auto const& edgeIndex : mEdgeIndices)
                {
                    ProcessEdgePair(edgeIndex, mMinimumVolumeObject);
                }
            }
        }

        void GetMinimumVolumeBox(OrientedBox3<InputType>& box, InputType& volume)
        {
            Candidate const& mvc = mMinimumVolumeObject;

            // Compute the rational-valued box and volume.
            Vector3<RationalType> pmin, pmax;
            for (int32_t i = 0; i < 3; ++i)
            {
                mRBox.center[i] = mOrigin[i];

                for (int32_t j = 0; j < 3; ++j)
                {
                    mRBox.axis[i][j] = mvc.axis[i][j];
                }
                mRBox.sqrLengthAxis[i] = Dot(mRBox.axis[i], mRBox.axis[i]);

                pmin[i] = static_cast<RationalType>(Dot(mvc.axis[i], mVertices[mvc.minSupportIndex[i]]));
                pmax[i] = static_cast<RationalType>(Dot(mvc.axis[i], mVertices[mvc.maxSupportIndex[i]]));
            }

            RationalType const half(0.5);
            Vector3<RationalType> average = half * (pmax + pmin);
            for (int32_t i = 0; i < 3; ++i)
            {
                for (int32_t j = 0; j < 3; ++j)
                {
                    mRBox.center[j] += (average[i] / mRBox.sqrLengthAxis[i]) * mRBox.axis[i][j];
                }
            }

            Vector3<RationalType> difference = pmax - pmin;
            mRBox.scaledExtent = half * difference;

            mRBox.volume = difference[0] * difference[1] * difference[2] / mRBox.sqrLengthAxis[2];

            // Compute the floating-point-valued box and volume.
            for (int32_t i = 0; i < 3; ++i)
            {
                box.center[i] = static_cast<InputType>(mRBox.center[i]);
                InputType length = static_cast<InputType>(std::sqrt(mRBox.sqrLengthAxis[i]));
                for (int32_t j = 0; j < 3; ++j)
                {
                    box.axis[i][j] = static_cast<InputType>(mRBox.axis[i][j]) / length;
                }
                box.extent[i] = static_cast<InputType>(mRBox.scaledExtent[i]) / length;
            }
            volume = static_cast<InputType>(mRBox.volume);
        }

        // The number of threads to use for computing. If 0, the main thread
        // is used. If positive, std::thread objects are used.
        size_t mNumThreads;

        // The maximum sample index used to search each level curve for
        // non-face-supporting boxes (mMaxSample + 1 values). The samples are
        // visited using subdivision of the domain of the level curve. The
        // subdivision/ information is stored in mDomainIndex(mNumSamples-1).
        size_t mMaxSample;
        std::vector<std::array<size_t, 3>> mDomainIndex;

        // Convenient members to allow construction once when the ComputeType
        // is BSNumber<*>.
        ComputeType const mZero, mOne, mHalf;

        // A mesh representation of the convex polyhedron. The mesh is
        // generated dynamically from the inputs to operator() but then is
        // converted to a pointerless representation. The mAdjacentPool and
        // mVertexAdjacent member are used for fast lookup of adjacent
        // vertices in GetExtreme(*).
        size_t mNumVertices, mNumTriangles;
        std::vector<size_t> mAdjacentPool;
        std::vector<size_t> mVertexAdjacent;
        std::vector<Edge> mEdges;
        std::vector<Triangle> mTriangles;
        std::vector<std::array<size_t, 2>> mEdgeIndices;

        // Storage for translated vertices and normal vectors. If the
        // compute type is 'double', the normals must be normalized to
        // unit length (within floating-point rounding error).
        VCompute3 mOrigin;
        std::vector<VCompute3> mVertices;
        std::vector<VCompute3> mNormals;

        // The axis-aligned bounding box of the vertices is used as the
        // initial candidate for the minimum-volume box.
        Candidate mAlignedCandidate;

        // The information for the minimum-volume bounding box of the
        // vertices.
        Candidate mMinimumVolumeObject;
        RBox mRBox;

        // Each member function A00B10C01D11(*) corresponds to a bilinear
        // function on the domain [0,1]^2. Each corner of the domain has a
        // bilinear function value that is positive, negative or zero,
        // leading to 3^4 = 81 possibilities. The 'A', 'B', 'C' and 'D' are
        // in {'P', 'M', 'Z'} [for Plus, Minus, Zero].
        typedef void (MinimumVolumeBox3::* LevelCurveProcessor)(Candidate&, Candidate&);
        std::array<LevelCurveProcessor, 256> mLevelCurveProcessor;

    protected:
        // Support for the level-curve processing functions.
        void InitializeLevelCurveProcessors()
        {
            // Generate the initialization code for mLevelCurveProcessor.
            // To compile the code, include <fstream>, <strstream> and
            // <iomanip.
            //
            // std::ofstream output("LevelCurveProcessor.txt");
            // std::array<char, 3> signchar = { 'Z', 'P', 'M' };
            // for (uint32_t index = 0; index < 256u; ++index)
            // {
            //     if ((index & 0x00000003u) != 0x00000003u &&
            //         (index & 0x0000000Cu) != 0x0000000Cu &&
            //         (index & 0x00000030u) != 0x00000030u &&
            //         (index & 0x000000C0u) != 0x000000C0u)
            //     {
            //         char s00 = signchar[index & 0x00000003u];
            //         char s10 = signchar[(index & 0x0000000Cu) >> 2];
            //         char s01 = signchar[(index & 0x00000030u) >> 4];
            //         char s11 = signchar[(index & 0x000000C0u) >> 6];
            //         std::strstream ostream;
            //         ostream << std::hex << std::setfill('0') << std::setw(2);
            //         ostream
            //             << "    mLevelCurveProcessor[0x0"
            //             << std::hex << std::setfill('0') << std::setw(2)
            //             << index
            //             << "] = &MinimumVolumeBox3::"
            //             << s00 << "00"
            //             << s10 << "10"
            //             << s01 << "01"
            //             << s11 << "11;"
            //             << std::ends;
            //         output << ostream.str() << std::endl;
            //     }
            // }
            // output.close();

            mLevelCurveProcessor.fill(nullptr);
            mLevelCurveProcessor[0x00] = &MinimumVolumeBox3::Z00Z10Z01Z11;
            mLevelCurveProcessor[0x01] = &MinimumVolumeBox3::P00Z10Z01Z11;
            mLevelCurveProcessor[0x02] = &MinimumVolumeBox3::M00Z10Z01Z11;
            mLevelCurveProcessor[0x04] = &MinimumVolumeBox3::Z00P10Z01Z11;
            mLevelCurveProcessor[0x05] = &MinimumVolumeBox3::P00P10Z01Z11;
            mLevelCurveProcessor[0x06] = &MinimumVolumeBox3::M00P10Z01Z11;
            mLevelCurveProcessor[0x08] = &MinimumVolumeBox3::Z00M10Z01Z11;
            mLevelCurveProcessor[0x09] = &MinimumVolumeBox3::P00M10Z01Z11;
            mLevelCurveProcessor[0x0a] = &MinimumVolumeBox3::M00M10Z01Z11;
            mLevelCurveProcessor[0x10] = &MinimumVolumeBox3::Z00Z10P01Z11;
            mLevelCurveProcessor[0x11] = &MinimumVolumeBox3::P00Z10P01Z11;
            mLevelCurveProcessor[0x12] = &MinimumVolumeBox3::M00Z10P01Z11;
            mLevelCurveProcessor[0x14] = &MinimumVolumeBox3::Z00P10P01Z11;
            mLevelCurveProcessor[0x15] = &MinimumVolumeBox3::P00P10P01Z11;
            mLevelCurveProcessor[0x16] = &MinimumVolumeBox3::M00P10P01Z11;
            mLevelCurveProcessor[0x18] = &MinimumVolumeBox3::Z00M10P01Z11;
            mLevelCurveProcessor[0x19] = &MinimumVolumeBox3::P00M10P01Z11;
            mLevelCurveProcessor[0x1a] = &MinimumVolumeBox3::M00M10P01Z11;
            mLevelCurveProcessor[0x20] = &MinimumVolumeBox3::Z00Z10M01Z11;
            mLevelCurveProcessor[0x21] = &MinimumVolumeBox3::P00Z10M01Z11;
            mLevelCurveProcessor[0x22] = &MinimumVolumeBox3::M00Z10M01Z11;
            mLevelCurveProcessor[0x24] = &MinimumVolumeBox3::Z00P10M01Z11;
            mLevelCurveProcessor[0x25] = &MinimumVolumeBox3::P00P10M01Z11;
            mLevelCurveProcessor[0x26] = &MinimumVolumeBox3::M00P10M01Z11;
            mLevelCurveProcessor[0x28] = &MinimumVolumeBox3::Z00M10M01Z11;
            mLevelCurveProcessor[0x29] = &MinimumVolumeBox3::P00M10M01Z11;
            mLevelCurveProcessor[0x2a] = &MinimumVolumeBox3::M00M10M01Z11;
            mLevelCurveProcessor[0x40] = &MinimumVolumeBox3::Z00Z10Z01P11;
            mLevelCurveProcessor[0x41] = &MinimumVolumeBox3::P00Z10Z01P11;
            mLevelCurveProcessor[0x42] = &MinimumVolumeBox3::M00Z10Z01P11;
            mLevelCurveProcessor[0x44] = &MinimumVolumeBox3::Z00P10Z01P11;
            mLevelCurveProcessor[0x45] = &MinimumVolumeBox3::P00P10Z01P11;
            mLevelCurveProcessor[0x46] = &MinimumVolumeBox3::M00P10Z01P11;
            mLevelCurveProcessor[0x48] = &MinimumVolumeBox3::Z00M10Z01P11;
            mLevelCurveProcessor[0x49] = &MinimumVolumeBox3::P00M10Z01P11;
            mLevelCurveProcessor[0x4a] = &MinimumVolumeBox3::M00M10Z01P11;
            mLevelCurveProcessor[0x50] = &MinimumVolumeBox3::Z00Z10P01P11;
            mLevelCurveProcessor[0x51] = &MinimumVolumeBox3::P00Z10P01P11;
            mLevelCurveProcessor[0x52] = &MinimumVolumeBox3::M00Z10P01P11;
            mLevelCurveProcessor[0x54] = &MinimumVolumeBox3::Z00P10P01P11;
            mLevelCurveProcessor[0x55] = &MinimumVolumeBox3::P00P10P01P11;
            mLevelCurveProcessor[0x56] = &MinimumVolumeBox3::M00P10P01P11;
            mLevelCurveProcessor[0x58] = &MinimumVolumeBox3::Z00M10P01P11;
            mLevelCurveProcessor[0x59] = &MinimumVolumeBox3::P00M10P01P11;
            mLevelCurveProcessor[0x5a] = &MinimumVolumeBox3::M00M10P01P11;
            mLevelCurveProcessor[0x60] = &MinimumVolumeBox3::Z00Z10M01P11;
            mLevelCurveProcessor[0x61] = &MinimumVolumeBox3::P00Z10M01P11;
            mLevelCurveProcessor[0x62] = &MinimumVolumeBox3::M00Z10M01P11;
            mLevelCurveProcessor[0x64] = &MinimumVolumeBox3::Z00P10M01P11;
            mLevelCurveProcessor[0x65] = &MinimumVolumeBox3::P00P10M01P11;
            mLevelCurveProcessor[0x66] = &MinimumVolumeBox3::M00P10M01P11;
            mLevelCurveProcessor[0x68] = &MinimumVolumeBox3::Z00M10M01P11;
            mLevelCurveProcessor[0x69] = &MinimumVolumeBox3::P00M10M01P11;
            mLevelCurveProcessor[0x6a] = &MinimumVolumeBox3::M00M10M01P11;
            mLevelCurveProcessor[0x80] = &MinimumVolumeBox3::Z00Z10Z01M11;
            mLevelCurveProcessor[0x81] = &MinimumVolumeBox3::P00Z10Z01M11;
            mLevelCurveProcessor[0x82] = &MinimumVolumeBox3::M00Z10Z01M11;
            mLevelCurveProcessor[0x84] = &MinimumVolumeBox3::Z00P10Z01M11;
            mLevelCurveProcessor[0x85] = &MinimumVolumeBox3::P00P10Z01M11;
            mLevelCurveProcessor[0x86] = &MinimumVolumeBox3::M00P10Z01M11;
            mLevelCurveProcessor[0x88] = &MinimumVolumeBox3::Z00M10Z01M11;
            mLevelCurveProcessor[0x89] = &MinimumVolumeBox3::P00M10Z01M11;
            mLevelCurveProcessor[0x8a] = &MinimumVolumeBox3::M00M10Z01M11;
            mLevelCurveProcessor[0x90] = &MinimumVolumeBox3::Z00Z10P01M11;
            mLevelCurveProcessor[0x91] = &MinimumVolumeBox3::P00Z10P01M11;
            mLevelCurveProcessor[0x92] = &MinimumVolumeBox3::M00Z10P01M11;
            mLevelCurveProcessor[0x94] = &MinimumVolumeBox3::Z00P10P01M11;
            mLevelCurveProcessor[0x95] = &MinimumVolumeBox3::P00P10P01M11;
            mLevelCurveProcessor[0x96] = &MinimumVolumeBox3::M00P10P01M11;
            mLevelCurveProcessor[0x98] = &MinimumVolumeBox3::Z00M10P01M11;
            mLevelCurveProcessor[0x99] = &MinimumVolumeBox3::P00M10P01M11;
            mLevelCurveProcessor[0x9a] = &MinimumVolumeBox3::M00M10P01M11;
            mLevelCurveProcessor[0xa0] = &MinimumVolumeBox3::Z00Z10M01M11;
            mLevelCurveProcessor[0xa1] = &MinimumVolumeBox3::P00Z10M01M11;
            mLevelCurveProcessor[0xa2] = &MinimumVolumeBox3::M00Z10M01M11;
            mLevelCurveProcessor[0xa4] = &MinimumVolumeBox3::Z00P10M01M11;
            mLevelCurveProcessor[0xa5] = &MinimumVolumeBox3::P00P10M01M11;
            mLevelCurveProcessor[0xa6] = &MinimumVolumeBox3::M00P10M01M11;
            mLevelCurveProcessor[0xa8] = &MinimumVolumeBox3::Z00M10M01M11;
            mLevelCurveProcessor[0xa9] = &MinimumVolumeBox3::P00M10M01M11;
            mLevelCurveProcessor[0xaa] = &MinimumVolumeBox3::M00M10M01M11;
        }

        // The subdivision-based sampling functions.
        template <bool useDouble = computeDouble>
        typename std::enable_if<useDouble, void>::type
            Adjust(VCompute3& normal)
        {
            Normalize(normal);
        }

        template <bool useDouble = computeDouble>
        typename std::enable_if<!useDouble, void>::type
            Adjust(VCompute3&)
        {
            // Nothing to do when the compute type is rational.
        }

        void Pair(Candidate& c, Candidate& mvc)
        {
            ComputeVolume(c);
            if (c.volume < mvc.volume)
            {
                mvc = c;
            }
        }

        // The minimizers for the operator()(maxSample, *) function. The
        // default behavior of MinimumVolumeBox3D is to use the built-in
        // minimizers that sample the level curves as a simple search for a
        // minimum volume. However, you can override the minimizers and
        // provide a more sophisticated algorithm.
        virtual void MinimizerConstantS(Candidate& c, Candidate& mvc)
        {
            std::vector<ComputeType> t(mMaxSample + 1);
            t[0] = mZero;
            t[mMaxSample] = mOne;
            for (auto const& item : mDomainIndex)
            {
                t[item[0]] = mHalf * (t[item[1]] + t[item[2]]);
            }

            Adjust(c.axis[0]);
            for (size_t i = 0, j = mMaxSample; i <= mMaxSample; ++i, --j)
            {
                c.axis[1] = t[j] * c.M[0] + t[i] * c.M[1];
                Adjust(c.axis[1]);
                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        virtual void MinimizerConstantT(Candidate& c, Candidate& mvc)
        {
            std::vector<ComputeType> s(mMaxSample + 1);
            s[0] = mZero;
            s[mMaxSample] = mOne;
            for (auto const& item : mDomainIndex)
            {
                s[item[0]] = mHalf * (s[item[1]] + s[item[2]]);
            }

            Adjust(c.axis[1]);
            for (size_t i = 0, j = mMaxSample; i <= mMaxSample; ++i, --j)
            {
                c.axis[0] = s[j] * c.N[0] + s[i] * c.N[1];
                Adjust(c.axis[0]);
                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        virtual void MinimizerVariableS(ComputeType const& sminNumer,
            ComputeType const& smaxNumer, ComputeType const& sDenom,
            Candidate& c, Candidate& mvc)
        {
            std::vector<ComputeType> s(mMaxSample + 1), oms(mMaxSample + 1);
            s[0] = sminNumer;
            oms[0] = sDenom - sminNumer;
            s[mMaxSample] = smaxNumer;
            oms[mMaxSample] = sDenom - smaxNumer;
            for (auto const& item : mDomainIndex)
            {
                s[item[0]] = mHalf * (s[item[1]] + s[item[2]]);
                oms[item[0]] = mHalf * (oms[item[1]] + oms[item[2]]);
            }

            for (size_t i = 0; i <= mMaxSample; ++i)
            {
                c.axis[0] = oms[i] * c.N[0] + s[i] * c.N[1];
                Adjust(c.axis[0]);

                ComputeType q0 = oms[i] * c.f00 + s[i] * c.f10;
                ComputeType q1 = oms[i] * c.f01 + s[i] * c.f11;
                if (q0 > q1)
                {
                    c.axis[1] = q0 * c.M[1] - q1 * c.M[0];
                }
                else
                {
                    c.axis[1] = q1 * c.M[0] - q0 * c.M[1];
                }
                Adjust(c.axis[1]);

                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        virtual void MinimizerVariableT(ComputeType const& tminNumer,
            ComputeType const& tmaxNumer, ComputeType const& tDenom,
            Candidate& c, Candidate& mvc)
        {
            std::vector<ComputeType> t(mMaxSample + 1), omt(mMaxSample + 1);
            t[0] = tminNumer;
            omt[0] = tDenom - tminNumer;
            t[mMaxSample] = tmaxNumer;
            omt[mMaxSample] = tDenom - tmaxNumer;
            for (auto const& item : mDomainIndex)
            {
                t[item[0]] = mHalf * (t[item[1]] + t[item[2]]);
                omt[item[0]] = mHalf * (omt[item[1]] + omt[item[2]]);
            }

            for (size_t i = 0; i <= mMaxSample; ++i)
            {
                ComputeType p0 = omt[i] * c.f00 + t[i] * c.f01;
                ComputeType p1 = omt[i] * c.f10 + t[i] * c.f11;
                if (p0 > p1)
                {
                    c.axis[0] = p0 * c.N[1] - p1 * c.N[0];
                }
                else
                {
                    c.axis[0] = p1 * c.N[0] - p0 * c.N[1];
                }
                Adjust(c.axis[0]);

                c.axis[1] = omt[i] * c.M[0] + t[i] * c.M[1];
                Adjust(c.axis[1]);

                ComputeVolume(c);
                if (c.volume < mvc.volume)
                {
                    mvc = c;
                }
            }
        }

        void Z00Z10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x00
            // 0 0
            // 0 0
            //
            // This case occurs when each edge is shared by two coplanar
            // faces, so we have only two different normals. The normals
            // are perpendicular.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00Z10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x01
            // 0 0
            // + 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void M00Z10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x02
            // 0 0
            // - 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00P10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x04
            // 0 0
            // 0 +

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void P00P10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x05
            // 0 0
            // + +

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void M00P10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x06
            // 0 0
            // - +

            // tmin = 0, tmax = 1
            // s = -f00 / (f10 - f00), (+)/(+)
            // 1-s = f10 / (f10 - f00), (+)/(+)
            // N = (1-s) * N0 + s * N1, omit denominator
            c.axis[0] = c.f10 * c.N[0] - c.f00 * c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00M10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x08
            // 0 0
            // 0 -

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void P00M10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x09
            // 0 0
            // + -

            // tmin = 0, tmax = 1
            // s = f00 / (f00 - f10), (+)/(+)
            // 1-s = -f10 / (f00 - f10), (+)/(+)
            // N = s * N1 + (1-s) * N0, omit denominator
            c.axis[0] = c.f00 * c.N[1] - c.f10 * c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 0, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void M00M10Z01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x0a
            // 0 0
            // - -

            // smin = 0, smax = 1, t = 1
            c.axis[1] = c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00Z10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x10
            // + 0
            // 0 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x11
            // + 0
            // + 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);
        }

        void M00Z10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x12
            // + 0
            // - 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = -f00 / (f01 - f00), (+)/(+)
            // 1-t = f01 / (f01 - f00), (+)/(+)
            // M = (1-t) * M0 + t * M1, omit denominator
            c.axis[1] = c.f01 * c.M[0] - c.f00 * c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void Z00P10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x14
            // + 0
            // 0 +
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void P00P10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x15
            // + 0
            // + +

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00P10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x16
            // + 0
            // - +

            // smin = 0
            // smax = -f00 / (f10 - f00), (+)/(+)
            ComputeType f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(mZero, -c.f00, f10mf00, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void Z00M10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x18
            // + 0
            // 0 -

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void P00M10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x19
            // + 0
            // + -

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void M00M10P01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x1a
            // + 0
            // - -

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void Z00Z10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x20
            // - 0
            // 0 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x21
            // - 0
            // + 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = f00 / (f00 - f01), (+)/(+)
            // 1-t = -f01 / (f00 - f01), (+)/(+)
            // M = t * M1 + (1-t) * M0, omit denominator
            c.axis[1] = c.f00 * c.M[1] - c.f01 * c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void M00Z10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x22
            // - 0
            // - 0

            // tmin = 0, tmax = 1, s = 1
            c.axis[0] = c.N[1];
            MinimizerConstantS(c, mvc);
        }

        void Z00P10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x24
            // - 0
            // 0 +

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void P00P10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x25
            // - 0
            // + +

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void M00P10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x26
            // - 0
            // - +

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void Z00M10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x28
            // - 0
            // 0 -
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void P00M10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x29
            // - 0
            // + -

            // smin = 0
            // smax = f00 / (f00 - f10), (+)/(+)
            ComputeType f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(mZero, c.f00, f00mf10, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00M10M01Z11(Candidate& c, Candidate& mvc)
        {
            // index = 0x2a
            // - 0
            // - -

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void Z00Z10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x40
            // 0 +
            // 0 0

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smimn = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x41
            // 0 +
            // + 0
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void M00Z10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x42
            // 0 +
            // - 0

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void Z00P10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x44
            // 0 +
            // 0 +

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);
        }

        void P00P10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x45
            // 0 +
            // + +

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00P10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x46
            // 0 +
            // - +

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void Z00M10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x48
            // 0 +
            // 0 -

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = -f10 / (f11 - f10), (+)/(+)
            // 1-t = f11 / (f11 - f10), (+)/(+)
            // M = (1-t) * M0 + t * M1, omit denominator
            c.axis[1] = c.f11 * c.M[0] - c.f10 * c.M[1];
            MinimizerConstantT(c, mvc);
        }

        void P00M10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x49
            // 0 +
            // + -

            // smin = f00 / (f00 - f10), (+)/(+)
            // smax = 1
            ComputeType f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(c.f00, f00mf10, f00mf10, c, mvc);

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void M00M10Z01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x4a
            // 0 +
            // - -

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void Z00Z10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x50
            // + +
            // 0 0

            // smin = 0, smax = 1, t = 0s
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x51
            // + +
            // + 0

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void M00Z10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x52
            // + +
            // - 0

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void Z00P10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x54
            // + +
            // 0 +

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00P10P01P11(Candidate&, Candidate&)
        {
            // index = 0x55
            // + +
            // + +
            // Nothing to do.
        }

        void M00P10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x56
            // + +
            // - +

            // smin = 0
            // smax = -f00 / (f10 - f00), (+)/(+)
            ComputeType f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(mZero, -c.f00, f10mf00, c, mvc);
        }

        void Z00M10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x58
            // + +
            // 0 -

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void P00M10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x59
            // + +
            // + -

            // smin = f00 / (f00 - f10), (+)/(+)
            // smax = 1
            ComputeType f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(c.f00, f00mf10, f00mf10, c, mvc);
        }

        void M00M10P01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x5a
            // + +
            // - -

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void Z00Z10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x60
            // - +
            // 0 0

            // tmin = 0, tmax = 1
            // s = -f01 / (f11 - f01), (+)/(+)
            // 1-s = f11 / (f11 - f01), (+)/(+)
            // N = (1-s) * N0 + s * N1, omit denominator
            c.axis[0] = c.f11 * c.N[0] - c.f01 * c.N[1];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x61
            // - +
            // + 0

            // smin = 0
            // smax = -f01 / (f11 - f01), (+)/(+)
            ComputeType f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(mZero, -c.f01, f11mf01, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void M00Z10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x62
            // - +
            // - 0

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void Z00P10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x64
            // - +
            // 0 +

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void P00P10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x65
            // - +
            // + +

            // smin = 0
            // smax = -f01 / (f11 - f01), (+)/(+)
            ComputeType f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(mZero, -c.f01, f11mf01, c, mvc);
        }

        void M00P10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x66
            // - +
            // - +

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void Z00M10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x68
            // - +
            // 0 -

            // smin = -f01 / (f11 - f01), (+)/(+)
            // smax = 1
            ComputeType f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(-c.f01, f11mf01, f11mf01, c, mvc);

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00M10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x69
            // - +
            // + -
            //
            // The level set F = 0 has two hyperbolic curves, each formed by a
            // pair of endpoints in {(0,t0), (s0,0), (s1,1), (1,t1)}, where
            // s0 = -f00 / (f10 - f00), s1 = -f01 / (f11 - f01),
            // t0 = -f00 / (f01 - f00), t1 = -f10 / (f11 - f10), all quantites
            // in (0,1). The two curves are on opposite sides of the
            // asymptotes
            //   sa = (f01 - f00) / ((f01 - f00) + (f10 - f11))
            //   ta = (f10 - f00) / ((f10 - f00) + (f01 - f11))
            // If s0 < sa, one curve has endpoints {(0,t0),(s0,0)} and the
            // other curve has endpoints {(s1,1),(1,t1)}. If s0 > sa, one
            // curve has endpoints {(0,t0),(s1,1)} and the other curve has
            // endpoints {(s0,0),(1,t1)}. If s0 = sa, then segments of the
            // asymptotes are the two curves for the level set. Define
            // d = f00 * f11 - f10 * f01. It can be shown that
            //   s0 - sa = d / ((f10 - f00)((f10 - f00) + (f01 - f11))
            // The denominator is positive, so sign(s0 - sa) = sign(d). A
            // similar argument applies for the comparison between t0 and ta.

            ComputeType d = c.f00 * c.f11 - c.f10 * c.f01;
            if (d > mZero)
            {
                // endpoints (s0,0) and (1,t1)
                // smin = f00 / (f00 - f10), (+)/(+)
                // smax = 1
                ComputeType f00mf10 = c.f00 - c.f10;
                MinimizerVariableS(c.f00, f00mf10, f00mf10, c, mvc);

                // endpoints (0,t0) and (s1,1)
                // smin = 0
                // smax = -f01 / (f11 - f01), (+)/(+)
                ComputeType f11mf01 = c.f11 - c.f01;
                MinimizerVariableS(mZero, -c.f01, f11mf01, c, mvc);
            }
            else if (d < mZero)
            {
                // endpoints (0,t0) and (s0,0)
                // smin = 0
                // smax = f00 / (f00 - f10), (+)/(+)
                ComputeType f00mf10 = c.f00 - c.f10;
                MinimizerVariableS(mZero, c.f00, f00mf10, c, mvc);

                // endpoints (s1,1) and (1,t1)
                // smin = -f01 / (f11 - f01), (+)/(+)
                // smax = 1
                ComputeType f11mf01 = c.f11 - c.f01;
                MinimizerVariableS(-c.f01, f11mf01, f11mf01, c, mvc);
            }
            else
            {
                // endpoints (sa,0) and (sa,1)
                // sa = (f00 - f01) / ((f00 - f01) + (f11 - f10)), (+)/(+)
                // 1-sa = (f11 - f10) / ((f00 - f01) + (f11 - f10)), (+)/(+)
                // N = (1-sa) * N0 + sa * N1, omit the denominator
                c.axis[0] = (c.f11 - c.f10) * c.N[0] + (c.f00 - c.f01) * c.N[1];
                MinimizerConstantS(c, mvc);

                // endpoints (0,ta) and (1,ta)
                // ta = (f00 - f10) / ((f00 - f10) + (f11 - f01)), (+)/(+)
                // 1-ta = (f11 - f01) / ((f00 - f01) + (f11 - f01)), (+)/(+)
                // M = (1-ta) * M0 + ta * M1, omit the denominator
                c.axis[1] = (c.f11 - c.f01) * c.M[0] + (c.f00 - c.f10) * c.M[1];
                MinimizerConstantT(c, mvc);
            }
        }

        void M00M10M01P11(Candidate& c, Candidate& mvc)
        {
            // index = 0x6a
            // - +
            // - -

            // smin = -f01 / (f11 - f01), (+)/(+)
            // smax = 1
            ComputeType f11mf01 = c.f11 - c.f01;
            MinimizerVariableS(-c.f01, f11mf01, f11mf01, c, mvc);
        }

        void Z00Z10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x80
            // 0 -
            // 0 0

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x81
            // 0 -
            // + 0

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void M00Z10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x82
            // 0 -
            // - 0
            // It is not possible for a level curve to connect the corners.

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void Z00P10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x84
            // 0 -
            // 0 +

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1
            // t = f10 / (f10 - f11), (+)/(+)
            // 1-t = -f11 / (f10 - f11), (+)/(+)
            // M = t * M1 + (1-t) * M0, omit the denominator
            c.axis[1] = c.f10 * c.M[1] - c.f11 * c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00P10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x85
            // 0 -
            // + +

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void M00P10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x86
            // 0 -
            // - +

            // smin = -f00 / (f10 - f00), (+)/(+)
            // smax = 1
            ComputeType f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(-c.f00, f10mf00, f10mf00, c, mvc);
        }

        void Z00M10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x88
            // 0 -
            // 0 -

            // tmin = 0, tmax = 1, s = 0
            c.axis[0] = c.N[0];
            MinimizerConstantS(c, mvc);
        }

        void P00M10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x89
            // 0 -
            // + -

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void M00M10Z01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x8a
            // 0 -
            // - -

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[1];
            Pair(c, mvc);
        }

        void Z00Z10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x90
            // + -
            // 0 0

            // tmin = 0, tmax = 1
            // s = f01 / (f01 - f11), (+)/(+)
            // 1-s = -f11 / (f01 - f11), (+)/(+)
            // N = s * N1 + (1-s) * N0, omit the denominator
            c.axis[0] = c.f01 * c.N[1] - c.f11 * c.N[0];
            MinimizerConstantS(c, mvc);

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x91
            // + -
            // + 0

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void M00Z10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x92
            // + -
            // - 0

            // smin = 0
            // smax = f01 / (f01 - f11), (+)/(+)
            ComputeType f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(mZero, c.f01, f01mf11, c, mvc);

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void Z00P10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x94
            // + -
            // 0 +

            // smin = f01 / (f01 - f11), (+)/(+)
            // smax = 1
            ComputeType f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(c.f01, f01mf11, f01mf11, c, mvc);

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00P10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x95
            // + -
            // + +

            // smin = f01 / (f01 - f11), (+)/(+)
            // smax = 1
            ComputeType f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(c.f01, f01mf11, f01mf11, c, mvc);
        }

        void M00P10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x96
            // + -
            // - +
            //
            // The level set F = 0 has two hyperbolic curves, each formed by a
            // pair of endpoints in {(0,t0), (s0,0), (s1,1), (1,t1)}, where
            // s0 = -f00 / (f10 - f00), s1 = -f01 / (f11 - f01),
            // t0 = -f00 / (f01 - f00), t1 = -f10 / (f11 - f10), all quantites
            // in (0,1). The two curves are on opposite sides of the
            // asymptotes
            //   sa = (f01 - f00) / ((f01 - f00) + (f10 - f11))
            //   ta = (f10 - f00) / ((f10 - f00) + (f01 - f11))
            // If s0 < sa, one curve has endpoints {(0,t0),(s0,0)} and the
            // other curve has endpoints {(s1,1),(1,t1)}. If s0 > sa, one
            // curve has endpoints {(0,t0),(s1,1)} and the other curve has
            // endpoints {(s0,0),(1,t1)}. If s0 = sa, then segments of the
            // asymptotes are the two curves for the level set. Define
            // d = f00 * f11 - f10 * f01. It can be shown that
            //   s0 - sa = d / ((f10 - f00)((f10 - f00) + (f01 - f11))
            // The denominator is positive, so sign(s0 - sa) = sign(d). A
            // similar argument applies for the comparison between t0 and ta.

            ComputeType d = c.f00 * c.f11 - c.f10 * c.f01;
            if (d > mZero)
            {
                // endpoints (s0,0) and (1,t1)
                // smin = -f00 / (f10 - f00), (+)/(+)
                // smax = 1
                ComputeType f10mf00 = c.f10 - c.f00;
                MinimizerVariableS(-c.f00, f10mf00, f10mf00, c, mvc);

                // endpoints (0,t0) and (s1,1)
                // smin = 0
                // smax = f01 / (f01 - f11)
                ComputeType f01mf11 = c.f01 - c.f11;
                MinimizerVariableS(mZero, c.f01, f01mf11, c, mvc);
            }
            else if (d < mZero)
            {
                // endpoints (0,t0) and (s0,0)
                // smin = 0
                // smax = -f00 / (f10- f00), (+)/(+)
                ComputeType f10mf00 = c.f10 - c.f00;
                MinimizerVariableS(mZero, -c.f00, f10mf00, c, mvc);

                // endpoints (s1,1) and (1,t1)
                // smin = f01 / (f01 - f11), (+)/(+)
                // smax = 1
                ComputeType f01mf11 = c.f01 - c.f11;
                MinimizerVariableS(c.f01, f01mf11, f01mf11, c, mvc);
            }
            else
            {
                // endpoints (sa,0) and (sa,1)
                // sa = (f01 - f00) / ((f01 - f00) + (f10 - f11)), (+)/(+)
                // 1-sa = (f10 - f11) / ((f01 - f00) + (f10 - f11)), (+)/(+)
                // N = (1-sa) * N0 + s1* N1
                c.axis[0] = (c.f10 - c.f11) * c.N[0] + (c.f01 - c.f00) * c.N[1];
                MinimizerConstantS(c, mvc);

                // endpoints (0,ta) and (1,ta)
                // ta = (f10 - f00) / ((f10 - f00) + (f01 - f11)), (+)/(+)
                // 1-ta = (f01 - f11) / ((f10 - f00) + (f01 - f11)), (+)/(+)
                // M = (1-ta) * M0 + ta * M1, omit the denominator
                c.axis[1] = (c.f01 - c.f11) * c.M[0] + (c.f10 - c.f00) * c.M[1];
                MinimizerConstantT(c, mvc);
            }
        }

        void Z00M10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x98
            // + -
            // 0 -

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void P00M10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x99
            // + -
            // + -

            // tmin = 0, tmax = 1
            MinimizerVariableT(mZero, mOne, mOne, c, mvc);
        }

        void M00M10P01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0x9a
            // + -
            // - -

            // smin = 0
            // smax = f01 / (f01 - f11), (+)/(+)
            ComputeType f01mf11 = c.f01 - c.f11;
            MinimizerVariableS(mZero, c.f01, f01mf11, c, mvc);
        }

        void Z00Z10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa0
            // - -
            // 0 0

            // smin = 0, smax = 1, t = 0
            c.axis[1] = c.M[0];
            MinimizerConstantT(c, mvc);
        }

        void P00Z10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa1
            // - -
            // + 0

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void M00Z10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa2
            // - -
            // - 0

            c.axis[0] = c.N[1];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void Z00P10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa4
            // - -
            // 0 +

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void P00P10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa5
            // - -
            // + +

            // smin = 0, smax = 1
            MinimizerVariableS(mZero, mOne, mOne, c, mvc);
        }

        void M00P10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa6
            // - -
            // - +

            // smin = -f00 / (f10 - f00), (+)/(+)
            // smax = 1
            ComputeType f10mf00 = c.f10 - c.f00;
            MinimizerVariableS(-c.f00, f10mf00, f10mf00, c, mvc);
        }

        void Z00M10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa8
            // - -
            // 0 -

            c.axis[0] = c.N[0];
            c.axis[1] = c.M[0];
            Pair(c, mvc);
        }

        void P00M10M01M11(Candidate& c, Candidate& mvc)
        {
            // index = 0xa9
            // - -
            // + -

            // smin = 0
            // smax = f00 / (f00 - f10), (+)/(+)
            ComputeType f00mf10 = c.f00 - c.f10;
            MinimizerVariableS(mZero, c.f00, f00mf10, c, mvc);
        }

        void M00M10M01M11(Candidate&, Candidate&)
        {
            // index = 0xaa
            // - -
            // - -
            // Nothing to do.
        }
    };
}
