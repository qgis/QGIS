// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.02.12

#pragma once

#include <cstdint>

namespace gte
{
    template <typename Float, typename UInt, int NumBits, int Precision>
    class IEEEBinary
    {
    public:
        // For generic access of the template types.
        using FloatType = Float;
        using UIntType = UInt;

        // The standard constructors, copy semantics and move semantics are
        // all default behavior. The default constructor does not initialize
        // its union member.
        IEEEBinary() = default;
        ~IEEEBinary() = default;
        IEEEBinary(IEEEBinary const&) = default;
        IEEEBinary(IEEEBinary&&) = default;
        IEEEBinary& operator=(IEEEBinary const&) = default;
        IEEEBinary& operator=(IEEEBinary&&) = default;

        // Construction from primitive elements.
        IEEEBinary(UInt inEncoding)
            :
            encoding(inEncoding)
        {
        }

        IEEEBinary(UInt inSign, UInt inBiased, UInt inTrailing)
        {
            SetEncoding(inSign, inBiased, inTrailing);
        }

        IEEEBinary(Float inNumber)
            :
            number(inNumber)
        {
        }

        // Implicit conversions to integer type or floating-point type.
        inline operator UInt () const
        {
            return encoding;
        }

        inline operator Float () const
        {
            return number;
        }

        // Special constants.
        static int const NUM_ENCODING_BITS = NumBits;
        static int const NUM_EXPONENT_BITS = NumBits - Precision;
        static int const NUM_SIGNIFICAND_BITS = Precision;
        static int const NUM_TRAILING_BITS = Precision - 1;
        static int const EXPONENT_BIAS = (1 << (NUM_EXPONENT_BITS - 1)) - 1;
        static int const MAX_BIASED_EXPONENT = (1 << NUM_EXPONENT_BITS) - 1;
        static int const MIN_SUB_EXPONENT = 1 - EXPONENT_BIAS;
        static int const MIN_EXPONENT = MIN_SUB_EXPONENT - NUM_TRAILING_BITS;
        static int const SIGN_SHIFT = NumBits - 1;

        static UInt const SIGN_MASK = (UInt(1) << (NumBits - 1));
        static UInt const NOT_SIGN_MASK = UInt(~SIGN_MASK);
        static UInt const TRAILING_MASK = (UInt(1) << NUM_TRAILING_BITS) - 1;
        static UInt const EXPONENT_MASK = NOT_SIGN_MASK & ~TRAILING_MASK;
        static UInt const NAN_QUIET_MASK = (UInt(1) << (NUM_TRAILING_BITS - 1));
        static UInt const NAN_PAYLOAD_MASK = (TRAILING_MASK >> 1);
        static UInt const MAX_TRAILING = TRAILING_MASK;
        static UInt const SUP_TRAILING = (UInt(1) << NUM_TRAILING_BITS);
        static UInt const POS_ZERO = UInt(0);
        static UInt const NEG_ZERO = SIGN_MASK;
        static UInt const MIN_SUBNORMAL = UInt(1);
        static UInt const MAX_SUBNORMAL = TRAILING_MASK;
        static UInt const MIN_NORMAL = SUP_TRAILING;
        static UInt const MAX_NORMAL = NOT_SIGN_MASK & ~SUP_TRAILING;
        static UInt const POS_INFINITY = EXPONENT_MASK;
        static UInt const NEG_INFINITY = SIGN_MASK | EXPONENT_MASK;

        // The types of numbers.
        enum class Classification
        {
            NEG_INFINITY,
            NEG_SUBNORMAL,
            NEG_NORMAL,
            NEG_ZERO,
            POS_ZERO,
            POS_SUBNORMAL,
            POS_NORMAL,
            POS_INFINITY,
            QUIET_NAN,
            SIGNALING_NAN
        };

        Classification GetClassification() const
        {
            UInt sign, biased, trailing;
            GetEncoding(sign, biased, trailing);

            if (biased == 0)
            {
                if (trailing == 0)
                {
                    if (sign != 0)
                    {
                        return Classification::NEG_ZERO;
                    }
                    else
                    {
                        return Classification::POS_ZERO;
                    }
                }
                else
                {
                    if (sign != 0)
                    {
                        return Classification::NEG_SUBNORMAL;
                    }
                    else
                    {
                        return Classification::POS_SUBNORMAL;
                    }
                }
            }
            else if (biased < MAX_BIASED_EXPONENT)
            {
                if (sign != 0)
                {
                    return Classification::NEG_NORMAL;
                }
                else
                {
                    return Classification::POS_NORMAL;
                }
            }
            else if (trailing == 0)
            {
                if (sign != 0)
                {
                    return Classification::NEG_INFINITY;
                }
                else
                {
                    return Classification::POS_INFINITY;
                }
            }
            else if (trailing & NAN_QUIET_MASK)
            {
                return Classification::QUIET_NAN;
            }
            else
            {
                return Classification::SIGNALING_NAN;
            }
        }

        bool IsZero() const
        {
            return encoding == POS_ZERO || encoding == NEG_ZERO;
        }

        bool IsSignMinus() const
        {
            return (encoding & SIGN_MASK) != 0;
        }

        bool IsSubnormal() const
        {
            return GetBiased() == 0 && GetTrailing() > 0;
        }

        bool IsNormal() const
        {
            UInt biased = GetBiased();
            return 0 < biased && biased < MAX_BIASED_EXPONENT;
        }

        bool IsFinite() const
        {
            return GetBiased() < MAX_BIASED_EXPONENT;
        }

        bool IsInfinite() const
        {
            return GetBiased() == MAX_BIASED_EXPONENT && GetTrailing() == 0;
        }

        bool IsNaN() const
        {
            return GetBiased() == MAX_BIASED_EXPONENT && GetTrailing() != 0;
        }

        bool IsQuietNaN() const
        {
            UInt trailing = GetTrailing();
            return GetBiased() == MAX_BIASED_EXPONENT
                && (trailing & NAN_QUIET_MASK) != 0
                && (trailing & NAN_PAYLOAD_MASK) != 0;
        }

        bool IsSignalingNaN() const
        {
            UInt trailing = GetTrailing();
            return GetBiased() == MAX_BIASED_EXPONENT
                && (trailing & NAN_QUIET_MASK) == 0
                && (trailing & NAN_PAYLOAD_MASK) != 0;
        }

        // Get neighboring numbers.
        UInt GetNextUp() const
        {
            UInt sign, biased, trailing;
            GetEncoding(sign, biased, trailing);

            if (biased == 0)
            {
                if (trailing == 0)
                {
                    // The next-up for both -0 and +0 is MIN_SUBNORMAL.
                    return MIN_SUBNORMAL;
                }
                else
                {
                    if (sign != 0)
                    {
                        // When trailing is 1, 'this' is -MIN_SUBNORMAL and
                        // next-up is -0.
                        --trailing;
                        return SIGN_MASK | trailing;
                    }
                    else
                    {
                        // When trailing is MAX_TRAILING, 'this' is
                        // MAX_SUBNORMAL and next-up is MIN_NORMAL.
                        ++trailing;
                        return trailing;
                    }
                }
            }
            else if (biased < MAX_BIASED_EXPONENT)
            {
                UInt nonnegative = (encoding & NOT_SIGN_MASK);
                if (sign != 0)
                {
                    --nonnegative;
                    return SIGN_MASK | nonnegative;
                }
                else
                {
                    ++nonnegative;
                    return nonnegative;
                }
            }
            else if (trailing == 0)
            {
                if (sign != 0)
                {
                    // The next-up of -INFINITY is -MAX_NORMAL.
                    return SIGN_MASK | MAX_NORMAL;
                }
                else
                {
                    // The next-up of +INFINITY is +INFINITY.
                    return POS_INFINITY;
                }
            }
            else if (trailing & NAN_QUIET_MASK)
            {
                // The number is a quiet NAN, possibly with payload.
                // Just return the number itself.
                return encoding;
            }
            else
            {
                // The number is a signaling NAN, possibly with payload.
                // Just return the number itself.
                return encoding;
            }
        }

        UInt GetNextDown() const
        {
            UInt sign, biased, trailing;
            GetEncoding(sign, biased, trailing);

            if (biased == 0)
            {
                if (trailing == 0)
                {
                    // The next-down for both -0 and +0 is -MIN_SUBNORMAL.
                    return SIGN_MASK | MIN_SUBNORMAL;
                }
                else
                {
                    if (sign == 0)
                    {
                        // When trailing is 1, 'this' is MIN_SUBNORMAL and
                        // next-down is +0.
                        --trailing;
                        return trailing;
                    }
                    else
                    {
                        // When trailing is MAX_TRAILING, 'this' is
                        // -MAX_SUBNORMAL and next-down is -MIN_NORMAL.
                        ++trailing;
                        return SIGN_MASK | trailing;
                    }
                }
            }
            else if (biased < MAX_BIASED_EXPONENT)
            {
                UInt nonnegative = (encoding & NOT_SIGN_MASK);
                if (sign == 0)
                {
                    --nonnegative;
                    return nonnegative;
                }
                else
                {
                    ++nonnegative;
                    return SIGN_MASK | nonnegative;
                }
            }
            else if (trailing == 0)
            {
                if (sign == 0)
                {
                    // The next-down of +INFINITY is +MAX_NORMAL.
                    return MAX_NORMAL;
                }
                else
                {
                    // The next-down of -INFINITY is -INFINITY.
                    return NEG_INFINITY;
                }
            }
            else if (trailing & NAN_QUIET_MASK)
            {
                // The number is a quiet NAN, possibly with payload.
                // Just return the number itself.
                return encoding;
            }
            else
            {
                // The number is a signaling NAN, possibly with payload.
                // Just return the number itself.
                return encoding;
            }
        }

        // Encode and decode the binary representation. The sign is 0 (number
        // is nonnegative) or 1 (number is negative). The biased exponent is
        // in the range [0,MAX_BIASED_EXPONENT]. The trailing significand is
        // in the range [0,MAX_TRAILING].
        UInt GetSign() const
        {
            return (encoding & SIGN_MASK) >> SIGN_SHIFT;
        }

        UInt GetBiased() const
        {
            return (encoding & EXPONENT_MASK) >> NUM_TRAILING_BITS;
        }

        UInt GetTrailing() const
        {
            return encoding & TRAILING_MASK;
        }

        void SetEncoding(UInt sign, UInt biased, UInt trailing)
        {
            encoding = (sign << SIGN_SHIFT) | (biased << NUM_TRAILING_BITS) | trailing;
        }

        void GetEncoding(UInt& sign, UInt& biased, UInt& trailing) const
        {
            sign = GetSign();
            biased = GetBiased();
            trailing = GetTrailing();
        }

        // Access for direct manipulation of the object.
        union
        {
            UInt encoding;
            Float number;
        };
    };

    using IEEEBinary32 = IEEEBinary<float, uint32_t, 32, 24>;
    // NUM_ENCODING_BITS = 32
    // NUM_EXPONENT_BITS = 8
    // NUM_SIGNIFICAND_BITS = 24
    // NUM_TRAILING_BITS = 23
    // EXPONENT_BIAS = 127
    // MAX_BIASED_EXPONENT = 255
    // MIN_SUB_EXPONENT = -126
    // MIN_EXPONENT = -149
    // SIGN_SHIFT = 31
    // SIGN_MASK =          0x80000000
    // NOT_SIGN_MASK =      0x7FFFFFFF
    // TRAILING_MASK =      0x007FFFFF
    // EXPONENT_MASK =      0x7F800000
    // NAN_QUIET_MASK =     0x00400000
    // NAN_PAYLOAD_MASK =   0x003FFFFF
    // MAX_TRAILING =       0x007FFFFF
    // SUP_TRAILING =       0x00800000
    // POS_ZERO =           0x00000000
    // NEG_ZERO =           0x80000000
    // MIN_SUBNORMAL =      0x00000001
    // MAX_SUBNORMAL =      0x007FFFFF
    // MIN_NORMAL =         0x00800000
    // MAX_NORMAL =         0x7F7FFFFF
    // POS_INFINITY =       0x7F800000
    // NEG_INFINITY =       0xFF800000

    using IEEEBinary64 = IEEEBinary<double, uint64_t, 64, 53>;
    // NUM_ENCODING_BITS = 64
    // NUM_EXPONENT_BITS = 11
    // NUM_SIGNIFICAND_BITS = 53
    // NUM_TRAILING_BITS = 52
    // EXPONENT_BIAS = 1023
    // MAX_BIASED_EXPONENT = 2047
    // MIN_SUB_EXPONENT = -1022
    // MIN_EXPONENT = -1074
    // SIGN_SHIFT = 63
    // SIGN_MASK =          0x8000000000000000
    // NOT_SIGN_MASK =      0x7FFFFFFFFFFFFFFF
    // TRAILING_MASK =      0x000FFFFFFFFFFFFF
    // EXPONENT_MASK =      0x7FF0000000000000
    // NAN_QUIET_MASK =     0x0008000000000000
    // NAN_PAYLOAD_MASK =   0x0007FFFFFFFFFFFF
    // MAX_TRAILING =       0x000FFFFFFFFFFFFF
    // SUP_TRAILING =       0x0010000000000000
    // POS_ZERO =           0x0000000000000000
    // NEG_ZERO =           0x8000000000000000
    // MIN_SUBNORMAL =      0x0000000000000001
    // MAX_SUBNORMAL =      0x000FFFFFFFFFFFFF
    // MIN_NORMAL =         0x0010000000000000
    // MAX_NORMAL =         0x7FEFFFFFFFFFFFFF
    // POS_INFINITY =       0x7FF0000000000000
    // NEG_INFINITY =       0xFFF0000000000000
}
