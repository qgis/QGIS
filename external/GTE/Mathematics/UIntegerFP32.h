// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.11.16

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/UIntegerALU32.h>
#include <array>
#include <istream>
#include <ostream>

// Class UIntegerFP32 is designed to support fixed precision arithmetic
// using BSNumber and BSRational.  It is not a general-purpose class for
// arithmetic of unsigned integers.  The template parameter N is the
// number of 32-bit words required to store the precision for the desired
// computations (maximum number of bits is 32*N).

// Uncomment this to collect statistics on how large the UIntegerFP32 storage
// becomes when using it for the UInteger of BSNumber.  If you use this
// feature, you must define gsUIntegerFP32MaxSize somewhere in your code.
//
//#define GTE_COLLECT_UINTEGERFP32_STATISTICS
#if defined(GTE_COLLECT_UINTEGERFP32_STATISTICS)
#include <Mathematics/AtomicMinMax.h>
namespace gte
{
    extern std::atomic<int32_t> gsUIntegerFP32MaxSize;
}
#endif

namespace gte
{
    template <int N>
    class UIntegerFP32 : public UIntegerALU32<UIntegerFP32<N>>
    {
    public:
        // Construction.
        UIntegerFP32()
            :
            mNumBits(0),
            mSize(0)
        {
            static_assert(N >= 1, "Invalid size N.");
        }

        UIntegerFP32(UIntegerFP32 const& number)
        {
            static_assert(N >= 1, "Invalid size N.");

            *this = number;
        }

        UIntegerFP32(uint32_t number)
        {
            static_assert(N >= 1, "Invalid size N.");

            if (number > 0)
            {
                int32_t first = BitHacks::GetLeadingBit(number);
                int32_t last = BitHacks::GetTrailingBit(number);
                mNumBits = first - last + 1;
                mSize = 1;
                mBits[0] = (number >> last);
            }
            else
            {
                mNumBits = 0;
                mSize = 0;
            }

#if defined(GTE_COLLECT_UINTEGERFP32_STATISTICS)
            AtomicMax(gsUIntegerFP32MaxSize, mSize);
#endif
        }

        UIntegerFP32(uint64_t number)
        {
            static_assert(N >= 2, "N not large enough to store 64-bit integers.");

            if (number > 0)
            {
                int32_t first = BitHacks::GetLeadingBit(number);
                int32_t last = BitHacks::GetTrailingBit(number);
                number >>= last;
                mNumBits = first - last + 1;
                mSize = 1 + (mNumBits - 1) / 32;
                mBits[0] = (uint32_t)(number & 0x00000000FFFFFFFFull);
                if (mSize > 1)
                {
                    mBits[1] = (uint32_t)((number >> 32) & 0x00000000FFFFFFFFull);
                }
            }
            else
            {
                mNumBits = 0;
                mSize = 0;
            }

#if defined(GTE_COLLECT_UINTEGERFP32_STATISTICS)
            AtomicMax(gsUIntegerFP32MaxSize, mSize);
#endif
        }

        // Assignment.  Only mSize elements are copied.
        UIntegerFP32& operator=(UIntegerFP32 const& number)
        {
            static_assert(N >= 1, "Invalid size N.");

            mNumBits = number.mNumBits;
            mSize = number.mSize;
            std::copy(number.mBits.begin(), number.mBits.begin() + mSize, mBits.begin());
            return *this;
        }

        // Support for std::move.  The interface is required by BSNumber, but
        // the std::move of std::array is a copy (no pointer stealing).
        // Moreover, a std::array object in this class typically uses smaller
        // than N elements, the actual size stored in mSize, so we do not want
        // to move everything.  Therefore, the move operator only copies the
        // bits BUT 'number' is modified as if you have stolen the data
        // (mNumBits and mSize set to zero).
        UIntegerFP32(UIntegerFP32&& number) noexcept
        {
            *this = std::move(number);
        }

        UIntegerFP32& operator=(UIntegerFP32&& number) noexcept
        {
            mNumBits = number.mNumBits;
            mSize = number.mSize;
            std::copy(number.mBits.begin(), number.mBits.begin() + mSize,
                mBits.begin());
            number.mNumBits = 0;
            number.mSize = 0;
            return *this;
        }

        // Member access.
        void SetNumBits(int32_t numBits)
        {
            if (numBits > 0)
            {
                mNumBits = numBits;
                mSize = 1 + (numBits - 1) / 32;
            }
            else if (numBits == 0)
            {
                mNumBits = 0;
                mSize = 0;
            }
            else
            {
                LogError("The number of bits must be nonnegative.");
            }

#if defined(GTE_COLLECT_UINTEGERFP32_STATISTICS)
            AtomicMax(gsUIntegerFP32MaxSize, mSize);
#endif
            LogAssert(mSize <= N, "N not large enough to store number of bits.");
        }

        inline int32_t GetNumBits() const
        {
            return mNumBits;
        }

        inline std::array<uint32_t, N> const& GetBits() const
        {
            return mBits;
        }

        inline std::array<uint32_t, N>& GetBits()
        {
            return mBits;
        }

        inline void SetBack(uint32_t value)
        {
            mBits[mSize - 1] = value;
        }

        inline uint32_t GetBack() const
        {
            return mBits[mSize - 1];
        }

        inline int32_t GetSize() const
        {
            return mSize;
        }

        inline static int32_t GetMaxSize()
        {
            return N;
        }

        inline void SetAllBitsToZero()
        {
            std::fill(mBits.begin(), mBits.end(), 0u);
        }

        // Copy from UIntegerFP32<NSource> to UIntegerFP32<N> as long as
        // NSource <= N.
        template <int NSource>
        void CopyFrom(UIntegerFP32<NSource> const& source)
        {
            static_assert(NSource <= N,
                "The source dimension cannot exceed the target dimension.");

            mNumBits = source.GetNumBits();
            mSize = source.GetSize();
            auto const& srcBits = source.GetBits();
            std::copy(srcBits.begin(), srcBits.end(), mBits.begin());
        }

        // Disk input/output.  The fstream objects should be created using
        // std::ios::binary.  The return value is 'true' iff the operation
        // was successful.
        bool Write(std::ostream& output) const
        {
            if (output.write((char const*)& mNumBits, sizeof(mNumBits)).bad())
            {
                return false;
            }

            if (output.write((char const*)& mSize, sizeof(mSize)).bad())
            {
                return false;
            }

            return output.write((char const*)& mBits[0], mSize * sizeof(mBits[0])).good();
        }

        bool Read(std::istream& input)
        {
            if (input.read((char*)& mNumBits, sizeof(mNumBits)).bad())
            {
                return false;
            }

            if (input.read((char*)& mSize, sizeof(mSize)).bad())
            {
                return false;
            }

            return input.read((char*)& mBits[0], mSize * sizeof(mBits[0])).good();
        }

    private:
        int32_t mNumBits, mSize;
        std::array<uint32_t, N> mBits;
    };
}
