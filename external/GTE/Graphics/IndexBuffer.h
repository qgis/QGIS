// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Buffer.h>
#include <Graphics/IndexFormat.h>
#include <Mathematics/Logger.h>
#include <cstdint>

// Expose this define to test for mismatches of primitive type in {Set,Get}Point,
//  {Set,Get}Segment and {Set,Get}Triangle.
//
//#define GTE_VERIFY_PRIMITIVE_TYPE

namespace gte
{
    class IndexBuffer : public Buffer
    {
    public:
        // Construction.  The first constructor leads to a DrawIndexed call in
        // the graphics engine.
        IndexBuffer(IPType type, uint32_t numPrimitives, size_t indexSize, bool createStorage = true);

        // Use this constructor when you want the indexing to be implied by
        // the ordering of vertices in the vertex buffer.  This leads to a
        // Draw call in the graphics engine.  You must ensure that
        // numPrimitives and numVertices (in the VertexBuffer) are consistent.
        // The usage flag is not applicable, because there is no system-memory
        // resource data.
        IndexBuffer(IPType type, uint32_t numPrimitives);

        // Member access.
        inline IPType GetPrimitiveType() const
        {
            return mPrimitiveType;
        }

        inline uint32_t GetNumPrimitives() const
        {
            return mNumPrimitives;
        }

        inline bool IsIndexed() const
        {
            return mData != nullptr;
        }

        // Specify how many primitives are to be drawn when you do not want
        // them all drawn.  The default value is mNumPrimitives.  The function
        // SetNumActivePrimitives ensures that the input satisfies
        // numActive <= mNumPrimitives.
        void SetNumActivePrimitives(uint32_t numActive);

        inline uint32_t GetNumActivePrimitives() const
        {
            return mNumActivePrimitives;
        }

        uint32_t GetNumActiveIndices() const;

        // Specify the index of the first primitive to be drawn.  The default
        // value is zero.  If you plan to modify both the number of active
        // primitives and the first primitive to be drawn, set the number of
        // of active primitives first.  SetFirstPrimitive ensures that
        // first < mNumPrimitives and first + numActive <= mNumPrimitives.
        void SetFirstPrimitive(uint32_t first);

        inline uint32_t GetFirstPrimitive() const
        {
            return mFirstPrimitive;
        }

        uint32_t GetFirstIndex() const;

        // Support for set/get of primitive indices.  The functions return
        // 'true' when the index i is within range for the primitive.  The
        // caller is responsible for using the correct functions for the
        // primitive type.  If you want to trap mismatches, enable
        // GTE_VERIFY_PRIMITIVE_TYPE.  These functions have per-primitive
        // overhead, namely, various range checks and typecasting, so consider
        // these a convenience.  For optimum speed, you can use
        //   auto ibuffer = std::make_shared<IndexBuffer>(...);
        //   type* indices = ibuffer->GetData<type>();
        //   <set or get indices[i]>;
        // where 'type' is 'uint32_t' or 'uint16_t' depending on how you
        // constructed the index buffer.
        bool SetPoint(uint32_t i, uint32_t v);
        bool GetPoint(uint32_t i, uint32_t& v) const;
        bool SetSegment(uint32_t i, uint32_t v0, uint32_t v1);
        bool GetSegment(uint32_t i, uint32_t& v0, uint32_t& v1) const;
        bool SetTriangle(uint32_t i, uint32_t v0, uint32_t v1, uint32_t v2);
        bool GetTriangle(uint32_t i, uint32_t& v0, uint32_t& v1, uint32_t& v2) const;

        // No member functions exist currently to set the IP_*_ADJ index
        // values.  The layout of the indices is shown in
        // https://msdn.microsoft.com/en-us/library/windows/desktop/bb205124(v=vs.85).aspx

    protected:
        inline bool ValidPrimitiveType(IPType type) const
        {
            if ((mPrimitiveType & type) != 0)
            {
                return true;
            }
            else
            {
#if defined(GTE_VERIFY_PRIMITIVE_TYPE)
                LogError("Invalid primitive type used in Set<Primitive> call.");
#else
                return false;
#endif
            }
        }

        IPType mPrimitiveType;
        uint32_t mNumPrimitives;
        uint32_t mNumActivePrimitives;
        uint32_t mFirstPrimitive;

        typedef uint32_t(*ICFunction)(uint32_t);
        static ICFunction msIndexCounter[IP_NUM_TYPES];

        static uint32_t GetPolypointIndexCount(uint32_t numPrimitives);
        static uint32_t GetPolysegmentDisjointIndexCount(uint32_t numPrimitives);
        static uint32_t GetPolysegmentContiguousIndexCount(uint32_t numPrimitives);
        static uint32_t GetTrimeshIndexCount(uint32_t numPrimitives);
        static uint32_t GetTristripIndexCount(uint32_t numPrimitives);
        static uint32_t GetPolysegmentDisjointAdjIndexCount(uint32_t numPrimitives);
        static uint32_t GetPolysegmentContiguousAdjIndexCount(uint32_t numPrimitives);
        static uint32_t GetTrimeshAdjIndexCount(uint32_t numPrimitives);
        static uint32_t GetTristripAdjIndexCount(uint32_t numPrimitives);
    };
}
