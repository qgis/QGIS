// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.1.2020.09.18

#pragma once

// UniqueVerticesSimplices allows mesh generation and elimination of duplicate
// and/or unused vertices. The vertices have type VertexType, which must have
// a less-than comparison predicate because it is used as the key type in
// std::map. The IndexType can be any signed or unsigned integer type, not
// including 1-byte types or bool. The mesh can be in any dimension D >= 2. In
// 2 dimensions, the mesh is a collection of edges. In 3 dimensions, the mesh
// is a collection of triangles. Generally, the mesh is a collection of
// D-dimensional simplices. The following operations are supported, either for
// a mesh topology consisting of indices or of arrays, each array representing
// a simplex.
//
//   1. Generate an indexed simplex representation from an array of simplices,
//      each simplex represented by D contiguous vertices. Presumably, the
//      simplices share vertices. The output is an array of unique simplices
//      (a vertex pool) and an array of D-element arrays of indices into the
//      pool, each such array representing a simplex.
//
//   2. Remove duplicate vertices from a vertex pool used by an indexed
//      simplex representation. A new vertex pool of unique vertices is
//      generated and the indexed simplices are modified to be indices into
//      this vertex pool.
//
//   3. Remove unused vertices from a vertex pool used by an indexed simplex
//      representation. A new vertex pool of unique vertices is generated and
//      the indexed simplices are modified to be indices into the new vertex
//      pool.
//
//   4. Remove duplicate and unused vertices from a vertex pool, a combination
//      of the operations in #2 and #3.
//
// In the Geometric Tools distribution, the class is used for polygon Boolean
// operations (D = 2) and for compactifying triangle meshes (D = 3).

#include <Mathematics/Logger.h>
#include <array>
#include <map>
#include <type_traits>
#include <vector>

namespace gte
{
    template <typename VertexType, typename IndexType, size_t Dimension>
    class UniqueVerticesSimplices
    {
    public:
        UniqueVerticesSimplices()
        {
            // The index type must be an integral type that does not include
            // bool. MSVS 2019 16.7.3 does not trigger this static assertion
            // when IndexType is bool. Instead, it complains about using
            // .data() for the std::vector<bool> objects and about comparing
            // bool values to 0 in the LogAssert statements, after which the
            // compilation terminates.
            static_assert(
                std::is_integral<IndexType>::value &&
                !std::is_same<IndexType, bool>::value,
                "Invalid index type.");
        }

        // See #1 in the comments at the beginning of this file. The
        // preconditions are
        //   1. inVertices.size() is a positive multiple of D
        // The postconditions are
        //   1. outVertices has unique vertices
        //   2. outIndices.size() = inVertices.size()
        //   3. 0 <= outIndices[i] < outVertices.size()
        void GenerateIndexedSimplices(
            std::vector<VertexType> const& inVertices,
            std::vector<VertexType>& outVertices,
            std::vector<IndexType>& outIndices)
        {
            LogAssert(
                inVertices.size() > 0 &&
                inVertices.size() % Dimension == 0,
                "Invalid number of vertices.");

            outIndices.resize(inVertices.size());
            RemoveDuplicates(inVertices, outVertices, outIndices.data());
        }

        // See #1 in the comments at the beginning of this file. The
        // preconditions are
        //   1. inVertices.size() is a positive multiple of Dimension
        // The postconditions are
        //   1. outVertices has unique vertices
        //   2. outSimplices.size() = inVertices.size() / Dimension
        //   3. 0 <= outSimplices[s][d] < outVertices.size()
        void GenerateIndexedSimplices(
            std::vector<VertexType> const& inVertices,
            std::vector<VertexType>& outVertices,
            std::vector<std::array<IndexType, Dimension>>& outSimplices)
        {
            LogAssert(
                inVertices.size() > 0 &&
                inVertices.size() % Dimension == 0,
                "Invalid number of vertices.");

            outSimplices.resize(inVertices.size() / Dimension);
            IndexType* outIndices = reinterpret_cast<IndexType*>(outSimplices.data());
            RemoveDuplicates(inVertices, outVertices, outIndices);
        }

        // See #2 in the comments at the beginning of the file. The
        // preconditions are
        //   1. inVertices.size() is positive
        //   2. inIndices.size() is a positive multiple of Dimension
        //   3. 0 <= inIndices[i] < inVertices.size()
        // The postconditions are
        //   1. outVertices has unique vertices
        //   2. outIndices.size() = inIndices.size()
        //   3. 0 <= outIndices[i] < outVertices.size()
        void RemoveDuplicateVertices(
            std::vector<VertexType> const& inVertices,
            std::vector<IndexType> const& inIndices,
            std::vector<VertexType>& outVertices,
            std::vector<IndexType>& outIndices)
        {
            LogAssert(
                inVertices.size() > 0,
                "Invalid number of vertices.");
            LogAssert(
                inIndices.size() > 0 &&
                inIndices.size() % Dimension == 0,
                "Invalid number of indices.");
            IndexType const numVertices = static_cast<IndexType>(inVertices.size());
            for (auto index : inIndices)
            {
                LogAssert(
                    0 <= index && index < numVertices,
                    "Invalid index.");
            }

            std::vector<IndexType> inToOutMapping(inVertices.size());
            RemoveDuplicates(inVertices, outVertices, inToOutMapping.data());

            outIndices.resize(inIndices.size());
            for (size_t i = 0; i < inIndices.size(); ++i)
            {
                outIndices[i] = inToOutMapping[inIndices[i]];
            }
        }

        // See #2 in the comments at the beginning of the file. The
        // preconditions are
        //   1. inVertices.size() is positive
        //   2. inSimplices.size() is positive
        //   3. 0 <= inSimplices[s][d] < inVertices.size()
        // The postconditions are
        //   1. outVertices has unique vertices
        //   2. outSimplices.size() = inSimplices.size()
        //   3. 0 <= outSimplices[s][d] < outVertices.size()
        void RemoveDuplicateVertices(
            std::vector<VertexType> const& inVertices,
            std::vector<std::array<IndexType, Dimension>> const& inSimplices,
            std::vector<VertexType>& outVertices,
            std::vector<std::array<IndexType, Dimension>>& outSimplices)
        {
            LogAssert(
                inVertices.size() > 0,
                "Invalid number of vertices.");
            LogAssert(inSimplices.size() > 0,
                "Invalid number of simplices.");
            IndexType const numVertices = static_cast<IndexType>(inVertices.size());
            for (auto const& simplex : inSimplices)
            {
                for (size_t d = 0; d < Dimension; ++d)
                {
                    LogAssert(
                        0 <= simplex[d] && simplex[d] < numVertices,
                        "Invalid index.");
                }
            }

            std::vector<IndexType> inToOutMapping(inVertices.size());
            RemoveDuplicates(inVertices, outVertices, inToOutMapping.data());

            size_t const numSimplices = inSimplices.size();
            outSimplices.resize(numSimplices);
            for (size_t s = 0; s < numSimplices; ++s)
            {
                for (size_t d = 0; d < Dimension; ++d)
                {
                    outSimplices[s][d] = inToOutMapping[inSimplices[s][d]];
                }
            }
        }

        // See #3 in the comments at the beginning of the file. The
        // preconditions are
        //   1. inVertices.size() is positive
        //   2. inIndices.size() is a positive multiple of Dimension
        //   3. 0 <= inIndices[i] < inVertices.size()
        // The postconditions are
        //   1. outVertices.size() is positive
        //   2. outIndices.size() = inIndices.size()
        //   3. 0 <= outIndices[i] < outVertices.size()
        //   4. each outVertices[v] occurs at least once in outIndices[]
        void RemoveUnusedVertices(
            std::vector<VertexType> const& inVertices,
            std::vector<IndexType> const& inIndices,
            std::vector<VertexType>& outVertices,
            std::vector<IndexType>& outIndices)
        {
            LogAssert(inVertices.size() > 0,
                "Invalid number of vertices.");
            LogAssert(
                inIndices.size() > 0 &&
                inIndices.size() % Dimension == 0,
                "Invalid number of indices.");
            IndexType const numVertices = static_cast<IndexType>(inVertices.size());
            for (auto index : inIndices)
            {
                LogAssert(
                    0 <= index && index < numVertices,
                    "Invalid index.");
            }

            outIndices.resize(inIndices.size());
            RemoveUnused(inVertices, inIndices.size(), inIndices.data(),
                outVertices, outIndices.data());
        }

        // See #3 in the comments at the beginning of the file. The
        // preconditions are
        //   1. inVertices.size() is positive
        //   2. inSimplices.size() is positive
        //   3. 0 <= inSimplices[s][d] < inVertices.size()
        // The postconditions are
        //   1. outVertices.size() is positive
        //   2. outSimplices.size() = inSimplices.size()
        //   3. 0 <= outSimplices[s][d] < outVertices.size()
        //   4. each outVertices[v] occurs at least once in outSimplices[][]
        void RemoveUnusedVertices(
            std::vector<VertexType> const& inVertices,
            std::vector<std::array<IndexType, Dimension>> const& inSimplices,
            std::vector<VertexType>& outVertices,
            std::vector<std::array<IndexType, Dimension>>& outSimplices)
        {
            LogAssert(
                inVertices.size() > 0,
                "Invalid number of vertices.");
            LogAssert(
                inSimplices.size() > 0,
                "Invalid number of simplices.");
            IndexType const numVertices = static_cast<IndexType>(inVertices.size());
            for (auto const& simplex : inSimplices)
            {
                for (size_t d = 0; d < Dimension; ++d)
                {
                    LogAssert(
                        0 <= simplex[d] && simplex[d] < numVertices,
                        "Invalid index.");
                }
            }

            outSimplices.resize(inSimplices.size());
            size_t const numInIndices = Dimension * inSimplices.size();
            IndexType const* inIndices = reinterpret_cast<IndexType const*>(inSimplices.data());
            IndexType* outIndices = reinterpret_cast<IndexType*>(outSimplices.data());
            RemoveUnused(inVertices, numInIndices, inIndices, outVertices, outIndices);
        }

        // See #4 and the preconditions for RemoveDuplicateVertices and for
        // RemoveUnusedVertices.
        void RemoveDuplicateAndUnusedVertices(
            std::vector<VertexType> const& inVertices,
            std::vector<IndexType> const& inIndices,
            std::vector<VertexType>& outVertices,
            std::vector<IndexType>& outIndices)
        {
            std::vector<VertexType> tempVertices;
            std::vector<IndexType> tempIndices;
            RemoveDuplicateVertices(inVertices, inIndices, tempVertices, tempIndices);
            RemoveUnusedVertices(tempVertices, tempIndices, outVertices, outIndices);
        }

        // See #4 and the preconditions for RemoveDuplicateVertices and for
        // RemoveUnusedVertices.
        void RemoveDuplicateAndUnusedVertices(
            std::vector<VertexType> const& inVertices,
            std::vector<std::array<IndexType, Dimension>> const& inSimplices,
            std::vector<VertexType>& outVertices,
            std::vector<std::array<IndexType, Dimension>>& outSimplices)
        {
            std::vector<VertexType> tempVertices;
            std::vector<std::array<IndexType, Dimension>> tempSimplices;
            RemoveDuplicateVertices(inVertices, inSimplices, tempVertices, tempSimplices);
            RemoveUnusedVertices(tempVertices, tempSimplices, outVertices, outSimplices);
        }

    private:
        void RemoveDuplicates(
            std::vector<VertexType> const& inVertices,
            std::vector<VertexType>& outVertices,
            IndexType* inToOutMapping)
        {
            // Construct the unique vertices.
            size_t const numInVertices = inVertices.size();
            size_t numOutVertices = 0;
            std::map<VertexType, IndexType> vmap;
            for (size_t i = 0; i < numInVertices; ++i)
            {
                auto const iter = vmap.find(inVertices[i]);
                if (iter != vmap.end())
                {
                    // The vertex is a duplicate of one inserted earlier into
                    // the map. Its index i will be modified to that of the
                    // first-found vertex.
                    inToOutMapping[i] = iter->second;
                }
                else
                {
                    // The vertex occurs for the first time.
                    vmap.insert(std::make_pair(inVertices[i],
                        static_cast<IndexType>(numOutVertices)));
                    inToOutMapping[i] = static_cast<IndexType>(numOutVertices);
                    ++numOutVertices;
                }
            }

            // Pack the unique vertices into an array.
            outVertices.resize(numOutVertices);
            for (auto const& element : vmap)
            {
                outVertices[element.second] = element.first;
            }
        }

        void RemoveUnused(
            std::vector<VertexType> const& inVertices,
            size_t const numInIndices,
            IndexType const* inIndices,
            std::vector<VertexType>& outVertices,
            IndexType* outIndices)
        {
            // Get the unique set of used indices.
            std::set<IndexType> usedIndices;
            for (size_t i = 0; i < numInIndices; ++i)
            {
                usedIndices.insert(inIndices[i]);
            }

            // Locate the used vertices and pack them into an array.
            outVertices.resize(usedIndices.size());
            size_t numOutVertices = 0;
            std::map<IndexType, IndexType> vmap;
            for (auto oldIndex : usedIndices)
            {
                outVertices[numOutVertices] = inVertices[oldIndex];
                vmap.insert(std::make_pair(oldIndex, static_cast<IndexType>(numOutVertices)));
                ++numOutVertices;
            }

            // Reassign the old indices to the new indices.
            for (size_t i = 0; i < numInIndices; ++i)
            {
                outIndices[i] = vmap.find(inIndices[i])->second;
            }
        }
    };
}
