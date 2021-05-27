// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.04

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/UIntegerALU32.h>
#include <limits>
#include <istream>
#include <ostream>
#include <vector>

// Class UIntegerAP32 is designed to support arbitrary precision arithmetic
// using BSNumber and BSRational.  It is not a general-purpose class for
// arithmetic of unsigned integers.

// Uncomment this to collect statistics on how large the UIntegerAP32 storage
// becomes when using it for the UInteger of BSNumber.  If you use this
// feature, you must define gsUIntegerAP32MaxSize somewhere in your code.
// After a sequence of BSNumber operations, look at gsUIntegerAP32MaxSize in
// the debugger watch window.  If the number is not too large, you might be
// safe in replacing UIntegerAP32 by UIntegerFP32<N>, where N is the value of
// gsUIntegerAP32MaxSize.  This leads to much faster code because you no
// longer have dynamic memory allocations and deallocations that occur
// regularly with std::vector<uint32_t> during BSNumber operations.  A safer
// choice is to argue mathematically that the maximum size is bounded by N.
// This requires an analysis of how many bits of precision you need for the
// types of computation you perform.  See class BSPrecision for code that
// allows you to compute maximum N.
//
//#define GTE_COLLECT_UINTEGERAP32_STATISTICS

#if defined(GTE_COLLECT_UINTEGERAP32_STATISTICS)
#include <Mathematics/AtomicMinMax.h>
namespace gte
{
    extern std::atomic<size_t> gsUIntegerAP32MaxSize;
}
#endif

namespace gte
{
    class UIntegerAP32 : public UIntegerALU32<UIntegerAP32>
    {
    public:
        // Construction.
        UIntegerAP32()
            :
            mNumBits(0)
        {
        }

        UIntegerAP32(UIntegerAP32 const& number)
        {
            *this = number;
        }

        UIntegerAP32(uint32_t number)
        {
            if (number > 0)
            {
                int32_t first = BitHacks::GetLeadingBit(number);
                int32_t last = BitHacks::GetTrailingBit(number);
                mNumBits = first - last + 1;
                mBits.resize(1);
                mBits[0] = (number >> last);
            }
            else
            {
                mNumBits = 0;
            }

#if defined(GTE_COLLECT_UINTEGERAP32_STATISTICS)
            AtomicMax(gsUIntegerAP32MaxSize, mBits.size());
#endif
        }

        UIntegerAP32(uint64_t number)
        {
            if (number > 0)
            {
                int32_t first = BitHacks::GetLeadingBit(number);
                int32_t last = BitHacks::GetTrailingBit(number);
                number >>= last;
                mNumBits = first - last + 1;
                mBits.resize(1 + (mNumBits - 1) / 32);
                mBits[0] = (uint32_t)(number & 0x00000000FFFFFFFFull);
                if (mBits.size() > 1)
                {
                    mBits[1] = (uint32_t)((number >> 32) & 0x00000000FFFFFFFFull);
                }
            }
            else
            {
                mNumBits = 0;
            }

#if defined(GTE_COLLECT_UINTEGERAP32_STATISTICS)
            AtomicMax(gsUIntegerAP32MaxSize, mBits.size());
#endif
        }

        // Assignment.
        UIntegerAP32& operator=(UIntegerAP32 const& number)
        {
            mNumBits = number.mNumBits;
            mBits = number.mBits;
            return *this;
        }

        // Support for std::move.
        UIntegerAP32(UIntegerAP32&& number) noexcept
        {
            *this = std::move(number);
        }

        UIntegerAP32& operator=(UIntegerAP32&& number) noexcept
        {
            mNumBits = number.mNumBits;
            mBits = std::move(number.mBits);
            number.mNumBits = 0;
            return *this;
        }

        // Member access.
        void SetNumBits(int32_t numBits)
        {
            if (numBits > 0)
            {
                mNumBits = numBits;
                mBits.resize(1 + (numBits - 1) / 32);
            }
            else if (numBits == 0)
            {
                mNumBits = 0;
                mBits.clear();
            }
            else
            {
                LogError("The number of bits must be nonnegative.");
            }

#if defined(GTE_COLLECT_UINTEGERAP32_STATISTICS)
            AtomicMax(gsUIntegerAP32MaxSize, mBits.size());
#endif
        }

        inline int32_t GetNumBits() const
        {
            return mNumBits;
        }

        inline std::vector<uint32_t> const& GetBits() const
        {
            return mBits;
        }

        inline std::vector<uint32_t>& GetBits()
        {
            return mBits;
        }

        inline void SetBack(uint32_t value)
        {
            mBits.back() = value;
        }

        inline uint32_t GetBack() const
        {
            return mBits.back();
        }

        inline int32_t GetSize() const
        {
            return static_cast<int32_t>(mBits.size());
        }

        inline static int32_t GetMaxSize()
        {
            return std::numeric_limits<int32_t>::max();
        }

        inline void SetAllBitsToZero()
        {
            std::fill(mBits.begin(), mBits.end(), 0u);
        }

        // Disk input/output.  The return value is 'true' iff the operation
        // was successful.
        bool Write(std::ostream& output) const
        {
            if (output.write((char const*)& mNumBits, sizeof(mNumBits)).bad())
            {
                return false;
            }

            std::size_t size = mBits.size();
            if (output.write((char const*)& size, sizeof(size)).bad())
            {
                return false;
            }

            return output.write((char const*)& mBits[0], size * sizeof(mBits[0])).good();
        }

        bool Read(std::istream& input)
        {
            if (input.read((char*)& mNumBits, sizeof(mNumBits)).bad())
            {
                return false;
            }

            std::size_t size;
            if (input.read((char*)& size, sizeof(size)).bad())
            {
                return false;
            }

            mBits.resize(size);
            return input.read((char*)& mBits[0], size * sizeof(mBits[0])).good();
        }

    private:
        int32_t mNumBits;
        std::vector<uint32_t> mBits;
    };
}
