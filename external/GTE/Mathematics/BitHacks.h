// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <array>
#include <cstdint>

// The leadingBit table in GetLeadingBit and the trailingBit table in
// GetTrailingBit are based on De Bruijn sequences.  The leadingBit table
// is taken from
// https://stackoverflow.com/questions/17027878/algorithm-to-find-the-most-significant-bit
// The trailingBit table is taken from
// https://www.dotnetperls.com/trailing-bits

// The int32_t inputs to the bit-hack functions are required to be
// nonnegative.  Expose this if you want exceptions thrown when the
// int32_t inputs are negative.
#define GTE_THROW_ON_BITHACKS_ERROR

namespace gte
{
    class BitHacks
    {
    public:
        static bool IsPowerOfTwo(uint32_t value)
        {
            return (value > 0) && ((value & (value - 1)) == 0);
        }

        static bool IsPowerOfTwo(int32_t value)
        {
#if defined(GTE_THROW_ON_BITHACKS_ERROR)
            LogAssert(value >= 0, "Invalid input.");
#endif
            return IsPowerOfTwo(static_cast<int32_t>(value));
        }

        static uint32_t Log2OfPowerOfTwo(uint32_t powerOfTwo)
        {
            uint32_t log2 = (powerOfTwo & 0xAAAAAAAAu) != 0;
            log2 |= ((powerOfTwo & 0xFFFF0000u) != 0) << 4;
            log2 |= ((powerOfTwo & 0xFF00FF00u) != 0) << 3;
            log2 |= ((powerOfTwo & 0xF0F0F0F0u) != 0) << 2;
            log2 |= ((powerOfTwo & 0xCCCCCCCCu) != 0) << 1;
            return log2;
        }

        static int32_t Log2OfPowerOfTwo(int32_t powerOfTwo)
        {
#if defined(GTE_THROW_ON_BITHACKS_ERROR)
            LogAssert(powerOfTwo >= 0, "Invalid input.");
#endif
            return static_cast<int32_t>(Log2OfPowerOfTwo(static_cast<uint32_t>(powerOfTwo)));
        }

        // The return value of the function is the index into the 32-bit value.
        // For example, GetLeadingBit(10) = 3 and GetTrailingBit(10) = 2.  The
        // value in binary is 0x0000000000001010.  The bit locations start at 0
        // on the right of the pattern and end at 31 on the left of the pattern.
        // If the input value is zero, there is no leading bit and no trailing
        // bit.  However, the functions return 0, which is considered invalid.
        // Try to call these functions only for positive inputs.
        static int32_t GetLeadingBit(uint32_t value)
        {
            static std::array<int32_t, 32> const leadingBitTable =
            {
                 0,  9,  1, 10, 13, 21,  2, 29,
                11, 14, 16, 18, 22, 25,  3, 30,
                 8, 12, 20, 28, 15, 17, 24,  7,
                19, 27, 23,  6, 26,  5,  4, 31
            };

            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            uint32_t key = (value * 0x07C4ACDDu) >> 27;
            return leadingBitTable[key];
        }

        static int32_t GetLeadingBit(int32_t value)
        {
#if defined(GTE_THROW_ON_BITHACKS_ERROR)
            LogAssert(value != 0, "Invalid input.");
#endif
            return GetLeadingBit(static_cast<uint32_t>(value));
        }

        static int32_t GetLeadingBit(uint64_t value)
        {
            uint32_t v1 = static_cast<uint32_t>((value >> 32) & 0x00000000FFFFFFFFull);
            if (v1 != 0)
            {
                return GetLeadingBit(v1) + 32;
            }

            uint32_t v0 = static_cast<uint32_t>(value & 0x00000000FFFFFFFFull);
            return GetLeadingBit(v0);
        }

        static int32_t GetLeadingBit(int64_t value)
        {
#if defined(GTE_THROW_ON_BITHACKS_ERROR)
            LogAssert(value != 0, "Invalid input.");
#endif
            return GetLeadingBit(static_cast<uint64_t>(value));
        }

        static int32_t GetTrailingBit(int32_t value)
        {
            static std::array<int32_t, 32> const trailingBitTable =
            {
                 0,  1, 28,  2, 29, 14, 24,  3,
                30, 22, 20, 15, 25, 17,  4,  8,
                31, 27, 13, 23, 21, 19, 16,  7,
                26, 12, 18,  6, 11,  5, 10,  9
            };

#if defined(GTE_THROW_ON_BITHACKS_ERROR)
            LogAssert(value != 0, "Invalid input.");
#endif

            uint32_t key = (static_cast<uint32_t>((value & -value) * 0x077CB531u)) >> 27;
            return trailingBitTable[key];
        }

        static int32_t GetTrailingBit(uint32_t value)
        {
            // The GetTrailingBit(int32_t) function contains the actual
            // implementation.  If the uint32_t-based function were to be
            // implemented, the (value & -value) statement generates a compiler
            // warning about negating an unsigned integer, which requires
            // additional logic to avoid.
            return GetTrailingBit(static_cast<int32_t>(value));
        }

        static int32_t GetTrailingBit(uint64_t value)
        {
            uint32_t v0 = static_cast<uint32_t>(value & 0x00000000FFFFFFFFull);
            if (v0 != 0)
            {
                return GetTrailingBit(v0);
            }

            uint32_t v1 = static_cast<uint32_t>((value >> 32) & 0x00000000FFFFFFFFull);
            if (v1 != 0)
            {
                return GetTrailingBit(v1) + 32;
            }
            return 0;
        }

        static int32_t GetTrailingBit(int64_t value)
        {
#if defined(GTE_THROW_ON_BITHACKS_ERROR)
            LogAssert(value != 0, "Invalid input.");
#endif
            return GetTrailingBit(static_cast<uint64_t>(value));
        }

        // Round up to a power of two.  If input is zero, the return is 1.  If
        // input is larger than 2^{31}, the return is 2^{32}.
        static uint64_t RoundUpToPowerOfTwo(uint32_t value)
        {
            if (value > 0)
            {
                int32_t leading = GetLeadingBit(value);
                uint32_t mask = (1 << leading);
                if ((value & ~mask) == 0)
                {
                    // value is a power of two
                    return static_cast<uint64_t>(value);
                }
                else
                {
                    // round up to a power of two
                    return (static_cast<uint64_t>(mask) << 1);
                }

            }
            else
            {
                return 1ull;
            }
        }

        // Round down to a power of two.  If input is zero, the return is 0.
        static uint32_t RoundDownToPowerOfTwo(uint32_t value)
        {
            if (value > 0)
            {
                int32_t leading = GetLeadingBit(value);
                uint32_t mask = (1 << leading);
                return mask;
            }
            else
            {
                return 0;
            }
        }
    };
}
