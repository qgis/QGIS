// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.02.12

#pragma once

#include <Mathematics/BitHacks.h>
#include <Mathematics/Math.h>
#include <Mathematics/IEEEBinary.h>

namespace gte
{
    class IEEEBinary16 : public IEEEBinary<int16_t, uint16_t, 16, 11>
    {
    public:
        // Construction and destruction. The base class destructor is hidden,
        // but this is safe because there are no side effects of the
        // destruction.
        ~IEEEBinary16() = default;

        // The default construction does not initialize the union member of
        // the base class. Use one of the Set*(*) functions to set the
        // member.
        IEEEBinary16()
            :
            IEEEBinary<int16_t, uint16_t, 16, 11>()
        {
            // uninitialized
        }

        IEEEBinary16(IEEEBinary16 const& object)
            :
            IEEEBinary<int16_t, uint16_t, 16, 11>(object)
        {
        }

        IEEEBinary16(float inNumber)
            :
            IEEEBinary<int16_t, uint16_t, 16, 11>()
        {
            union { float n; uint32_t e; } temp = { inNumber };
            encoding = Convert32To16(temp.e);
        }

        IEEEBinary16(double inNumber)
            :
            IEEEBinary<int16_t, uint16_t, 16, 11>()
        {
            union { float n; uint32_t e; } temp;
            temp.n = static_cast<float>(inNumber);
            encoding = Convert32To16(temp.e);
        }

        IEEEBinary16(uint16_t inEncoding)
            :
            IEEEBinary<int16_t, uint16_t, 16, 11>(inEncoding)
        {
        }

        // Implicit conversions.
        operator float() const
        {
            union { uint32_t e; float n; } temp = { Convert16To32(encoding) };
            return temp.n;
        }

        operator double() const
        {
            union { uint32_t e; float n; } temp = { Convert16To32(encoding) };
            return static_cast<double>(temp.n);
        }

        // Assignment.
        IEEEBinary16& operator=(IEEEBinary16 const& object)
        {
            IEEEBinary<int16_t, uint16_t, 16, 11>::operator=(object);
            return *this;
        }

        // Comparison.
        bool operator==(IEEEBinary16 const& object) const
        {
            return static_cast<float>(*this) == static_cast<float>(object);
        }

        bool operator!=(IEEEBinary16 const& object) const
        {
            return static_cast<float>(*this) != static_cast<float>(object);
        }

        bool operator< (IEEEBinary16 const& object) const
        {
            return static_cast<float>(*this) < static_cast<float>(object);
        }

        bool operator<=(IEEEBinary16 const& object) const
        {
            return static_cast<float>(*this) <= static_cast<float>(object);
        }

        bool operator> (IEEEBinary16 const& object) const
        {
            return static_cast<float>(*this) > static_cast<float>(object);
        }

        bool operator>=(IEEEBinary16 const& object) const
        {
            return static_cast<float>(*this) >= static_cast<float>(object);
        }

    private:
        // Members from the base class IEEEBinary<int16_t, uint16_t, 16, 11>.
        //
        // NUM_ENCODING_BITS = 16
        // NUM_EXPONENT_BITS = 5
        // NUM_SIGNIFICAND_BITS = 11
        // NUM_TRAILING_BITS = 10
        // EXPONENT_BIAS = 15
        // MAX_BIASED_EXPONENT = 31
        // MIN_SUB_EXPONENT = -14
        // MIN_EXPONENT = -24
        // SIGN_SHIFT = 15
        // SIGN_MASK =          0x8000
        // NOT_SIGN_MASK =      0x7FFF
        // TRAILING_MASK =      0x03FF
        // EXPONENT_MASK =      0x7C00
        // NAN_QUIET_MASK =     0x0200
        // NAN_PAYLOAD_MASK =   0x01FF
        // MAX_TRAILING =       0x03FF
        // SUP_TRAILING =       0x0400
        // POS_ZERO =           0x0000
        // NEG_ZERO =           0x8000
        // MIN_SUBNORMAL =      0x0001
        // MAX_SUBNORMAL =      0x03FF
        // MIN_NORMAL =         0x0400
        // MAX_NORMAL =         0x7BFF
        // POS_INFINITY =       0x7C00
        // NEG_INFINITY =       0xFC00

        // Support for conversions between 16-bit and 32-bit numbers.
        using F16 = IEEEBinary16;
        using F32 = IEEEBinary32;

        // Encodings of special numbers on the "continuous 16-bit
        // number line" as 32-bit float numbers.
        static uint32_t const F16_AVR_MIN_SUB_ZER = 0x33000000;  // 2^{-25}
        static uint32_t const F16_MIN_SUB = 0x33800000;          // 2^{-24}
        static uint32_t const F16_MIN_NOR = 0x38800000;          // 2^{-14}
        static uint32_t const F16_MAX_NOR = 0x477FE000;          // 2^{16)*(1-2^{-11})
        static uint32_t const F16_AVR_MAX_NOR_INF = 0x477FF000;  // 2^{16)*(1-2^{-12})

        // The amount to shift when converting between signs of 16-bit and
        // 32-bit numbers.
        static uint32_t const CONVERSION_SIGN_SHIFT
            = F32::NUM_ENCODING_BITS - F16::NUM_ENCODING_BITS;

        // The amount to shift when converting between trailing significands
        // of 16-bit and 32-bit numbers.
        static uint32_t const CONVERSION_TRAILING_SHIFT
            = F32::NUM_SIGNIFICAND_BITS - F16::NUM_SIGNIFICAND_BITS;

        // The half value for round-to-nearest-ties-to-even. The fractional
        // part in the rounding is shifted left so that the leading bit is
        // the high-order bit of a 32-bit unsigned integer.
        static uint32_t const FRACTION_HALF = F32::SIGN_MASK;

        static uint16_t Convert32To16(uint32_t inEncoding)
        {
            // In the comments of this function, x refers to the 32-bit
            // floating-point number corresponding to inEncoding and y refers
            // to the 16-bit floating-point number that x is converted to.

            // Extract the channels for x.
            uint32_t sign32 = (inEncoding & F32::SIGN_MASK);
            uint32_t biased32 = ((inEncoding & F32::EXPONENT_MASK) >> F32::NUM_TRAILING_BITS);
            uint32_t trailing32 = (inEncoding & F32::TRAILING_MASK);
            uint32_t nonneg32 = (inEncoding & F32::NOT_SIGN_MASK);

            // Generate the channels for y.
            uint16_t sign16 = static_cast<uint16_t>(sign32 >> CONVERSION_SIGN_SHIFT);
            uint16_t biased16, trailing16;
            uint32_t frcpart;

            if (biased32 == 0)
            {
                // x is zero or 32-subnormal, the nearest y is zero.
                return sign16;
            }

            if (biased32 < F32::MAX_BIASED_EXPONENT)
            {
                // x is 32-normal.
                if (nonneg32 <= F16_AVR_MIN_SUB_ZER)
                {
                    // x <= 2^{-25}, the nearest y is zero.
                    return sign16;
                }

                if (nonneg32 <= F16_MIN_SUB)
                {
                    // 2^{-25} < x <= 2^{-24}, the nearest y is
                    // 16-min-subnormal.
                    return sign16 | F16::MIN_SUBNORMAL;
                }

                if (nonneg32 < F16_MIN_NOR)
                {
                    // 2^{-24} < x < 2^{-14}, compute nearest 16-bit subnormal
                    // y using round-to-nearest-ties-to-even.
                    //
                    // y = 0.s9 ... s0 * 2^{14}
                    // x = 1.t22 ... t0 * 2^e, where -24 <= e <= -15
                    //   = (0.1 t22 ... t0 * 2^{e+15}) * 2^{-14}
                    //   = (0.1 t22 ... t0 * 2^p) * 2^{-14}
                    // where p = e+15 with -9 = p <= 0. The term
                    // (0.1 t22 ... t0 * 2^p) must be rounded to
                    // 0.s9 ... s0 * 2^{-14}.
                    int32_t p = static_cast<int32_t>(biased32) - F32::EXPONENT_BIAS
                        + F16::EXPONENT_BIAS;

                    // x is 32-normal, so there is an implied 1-bit that must
                    // first be appended to the 32-trailing significand to
                    // obtain all the bits necessary for the 16-trailing
                    // significand for the 16-subnormal y. The resulting number
                    // is 000000001 t22 ... t0.
                    trailing32 |= F32::SUP_TRAILING;

                    // Get the integer part.
                    uint32_t rshift = static_cast<uint32_t>(
                        -F16::MIN_SUB_EXPONENT - p);
                    trailing16 = static_cast<uint16_t>(trailing32 >> rshift);

                    // Get the fractional part.
                    uint32_t lshift = static_cast<uint32_t>(
                        F32::NUM_ENCODING_BITS + F16::MIN_SUB_EXPONENT + p);
                    frcpart = (trailing32 << lshift);

                    // Round to nearest with ties to even.
                    if (frcpart > FRACTION_HALF
                        || (frcpart == FRACTION_HALF && (trailing16 & 1)))
                    {
                        // If there is a carry into the exponent, the nearest
                        // is actually 16-min-normal 1.0*2^{-14}, so the
                        // high-order bit of trailing16 makes biased16 equal
                        // to 1 and the result is correct.
                        ++trailing16;
                    }
                    return sign16 | trailing16;
                }

                if (nonneg32 <= F16_MAX_NOR)
                {
                    // 2^{-14} <= x <= 1.1111111111*2^{15}, compute nearest
                    // 16-bit subnormal y using round-to-nearest-ties-to-even.

                    // The exponents of x and y are the same, although the
                    // biased exponents are different because of different
                    // exponent-bias parameters.
                    int32_t e = static_cast<int32_t>(biased32) - F32::EXPONENT_BIAS;
                    biased16 = static_cast<uint16_t>(e + F16::EXPONENT_BIAS);
                    biased16 = (biased16 << F16::NUM_TRAILING_BITS);

                    // Let x = 1.t22...t0 * 2^e and y = 1.s9...s0 * 2^e. Both
                    // x and y have an implied leading 1-bit (both are normal),
                    // so we can ignore it. The number 0.t22...t0 must be
                    // rounded to the number 0.s9...s0.

                    // Get the integer part.
                    trailing16 = static_cast<uint16_t>(
                        trailing32 >> CONVERSION_TRAILING_SHIFT);

                    // Get the fractional part.
                    uint32_t lshift = static_cast<uint32_t>(
                        F32::NUM_ENCODING_BITS + F16::MIN_SUB_EXPONENT + 1);
                    frcpart = (trailing32 << lshift);

                    // Round to nearest with ties to even.
                    if (frcpart > FRACTION_HALF
                        || (frcpart == FRACTION_HALF && (trailing16 & 1)))
                    {
                        // If there is a carry into the exponent, the addition
                        // of trailing16 to biased16 (rather than or-ing)
                        // produces the correct result.
                        ++trailing16;
                    }
                    return sign16 | (biased16 + trailing16);
                }

                if (nonneg32 < F16_AVR_MAX_NOR_INF)
                {
                    // 1.1111111111*2^{15} < x < (MAX_NORMAL+INFINITY)/2,
                    // so the number is closest to 16-max-normal.
                    return sign16 | F16::MAX_NORMAL;
                }

                // nonneg32 >= (MAX_NORMAL+INFINITY)/2, so convert to
                // 16-infinite.
                return sign16 | F16::POS_INFINITY;
            }

            if (trailing32 == 0)
            {
                // The number is 32-infinite. Convert to 16-infinite.
                return sign16 | F16::POS_INFINITY;
            }

            // The number is 32-NaN. Convert to 16-NaN with 16-payload the
            // high-order 9 bits of the 32-payload. The 32-quiet-NaN mask
            // bit is copied in the conversion.
            uint16_t maskPayload = static_cast<uint16_t>(
                trailing32 >> CONVERSION_TRAILING_SHIFT);
            return sign16 | F16::EXPONENT_MASK | maskPayload;
        }

        static uint32_t Convert16To32(uint16_t inEncoding)
        {
            // Extract the channels for the IEEEBinary16 number.
            uint16_t sign16 = (inEncoding & F16::SIGN_MASK);
            uint16_t biased16 = ((inEncoding & F16::EXPONENT_MASK) >> F16::NUM_TRAILING_BITS);
            uint16_t trailing16 = (inEncoding & F16::TRAILING_MASK);

            // Generate the channels for the binary32 number.
            uint32_t sign32 = static_cast<uint32_t>(sign16 << CONVERSION_SIGN_SHIFT);
            uint32_t biased32, trailing32;

            if (biased16 == 0)
            {
                if (trailing16 == 0)
                {
                    // The number is 16-zero. Convert to 32-zero.
                    return sign32;
                }
                else
                {
                    // The number is 16-subnormal.  Convert to 32-normal.
                    trailing32 = static_cast<uint32_t>(trailing16);
                    int32_t leading = BitHacks::GetLeadingBit(trailing32);
                    int32_t shift = 23 - leading;
                    biased32 = static_cast<uint32_t>(F32::EXPONENT_BIAS - 1 - shift);
                    trailing32 = (trailing32 << shift) & F32::TRAILING_MASK;
                    return sign32 | (biased32 << F32::NUM_TRAILING_BITS) | trailing32;
                }
            }

            if (biased16 < F16::MAX_BIASED_EXPONENT)
            {
                // The number is 16-normal. Convert to 32-normal.
                biased32 = static_cast<uint32_t>(biased16 - F16::EXPONENT_BIAS + F32::EXPONENT_BIAS);
                trailing32 = (static_cast<uint32_t>(trailing16) << CONVERSION_TRAILING_SHIFT);
                return sign32 | (biased32 << F32::NUM_TRAILING_BITS) | trailing32;
            }

            if (trailing16 == 0)
            {
                // The number is 16-infinite. Convert to 32-infinite.
                return sign32 | F32::EXPONENT_MASK;
            }

            // The number is 16-NaN. Convert to 32-NaN with 32-payload
            // whose high-order 9 bits are from the 16-payload. The
            // 16-quiet-NaN mask bit is copied in the conversion.
            uint32_t maskPayload = (static_cast<uint32_t>(trailing16) << CONVERSION_TRAILING_SHIFT);
            return sign32 | F32::EXPONENT_MASK | maskPayload;
        }
    };

    // Arithmetic operations (high-precision).
    inline IEEEBinary16 operator-(IEEEBinary16 x)
    {
        uint16_t result = static_cast<uint16_t>(x) ^ IEEEBinary16::SIGN_MASK;
        return result;
    }

    inline float operator+(IEEEBinary16 x, IEEEBinary16 y)
    {
        return static_cast<float>(x) + static_cast<float>(y);
    }

    inline float operator-(IEEEBinary16 x, IEEEBinary16 y)
    {
        return static_cast<float>(x) - static_cast<float>(y);
    }

    inline float operator*(IEEEBinary16 x, IEEEBinary16 y)
    {
        return static_cast<float>(x)* static_cast<float>(y);
    }

    inline float operator/(IEEEBinary16 x, IEEEBinary16 y)
    {
        return static_cast<float>(x) / static_cast<float>(y);
    }

    inline float operator+(IEEEBinary16 x, float y)
    {
        return static_cast<float>(x) + y;
    }

    inline float operator-(IEEEBinary16 x, float y)
    {
        return static_cast<float>(x) - y;
    }

    inline float operator*(IEEEBinary16 x, float y)
    {
        return static_cast<float>(x)* y;
    }

    inline float operator/(IEEEBinary16 x, float y)
    {
        return static_cast<float>(x) / y;
    }

    inline float operator+(float x, IEEEBinary16 y)
    {
        return x + static_cast<float>(y);
    }

    inline float operator-(float x, IEEEBinary16 y)
    {
        return x - static_cast<float>(y);
    }

    inline float operator*(float x, IEEEBinary16 y)
    {
        return x * static_cast<float>(y);
    }

    inline float operator/(float x, IEEEBinary16 y)
    {
        return x / static_cast<float>(y);
    }

    // Arithmetic updates.
    inline IEEEBinary16& operator+=(IEEEBinary16& x, IEEEBinary16 y)
    {
        x = static_cast<float>(x) + static_cast<float>(y);
        return x;
    }

    inline IEEEBinary16& operator-=(IEEEBinary16& x, IEEEBinary16 y)
    {
        x = static_cast<float>(x) - static_cast<float>(y);
        return x;
    }

    inline IEEEBinary16& operator*=(IEEEBinary16& x, IEEEBinary16 y)
    {
        x = static_cast<float>(x)* static_cast<float>(y);
        return x;
    }

    inline IEEEBinary16& operator/=(IEEEBinary16& x, IEEEBinary16 y)
    {
        x = static_cast<float>(x) / static_cast<float>(y);
        return x;
    }

    inline IEEEBinary16& operator+=(IEEEBinary16& x, float y)
    {
        x = static_cast<float>(x) + y;
        return x;
    }

    inline IEEEBinary16& operator-=(IEEEBinary16& x, float y)
    {
        x = static_cast<float>(x) - y;
        return x;
    }

    inline IEEEBinary16& operator*=(IEEEBinary16& x, float y)
    {
        x = static_cast<float>(x)* y;
        return x;
    }

    inline IEEEBinary16& operator/=(IEEEBinary16& x, float y)
    {
        x = static_cast<float>(x) / y;
        return x;
    }
}

namespace std
{
    inline gte::IEEEBinary16 acos(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::acos(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 acosh(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::acosh(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 asin(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::asin(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 asinh(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::asinh(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 atan(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::atan(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 atanh(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::atanh(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 atan2(gte::IEEEBinary16 y, gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::atan2(static_cast<float>(y), static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 ceil(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::ceil(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 cos(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::cos(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 cosh(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::cosh(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 exp(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::exp(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 exp2(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::exp2(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 fabs(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::fabs(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 floor(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::floor(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 fmod(gte::IEEEBinary16 x, gte::IEEEBinary16 y)
    {
        return static_cast<gte::IEEEBinary16>(std::fmod(static_cast<float>(x), static_cast<float>(y)));
    }

    inline gte::IEEEBinary16 frexp(gte::IEEEBinary16 x, int* exponent)
    {
        return static_cast<gte::IEEEBinary16>(std::frexp(static_cast<float>(x), exponent));
    }

    inline gte::IEEEBinary16 ldexp(gte::IEEEBinary16 x, int exponent)
    {
        return static_cast<gte::IEEEBinary16>(std::ldexp(static_cast<float>(x), exponent));
    }

    inline gte::IEEEBinary16 log(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::log(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 log2(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::log2(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 log10(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::log10(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 pow(gte::IEEEBinary16 x, gte::IEEEBinary16 y)
    {
        return static_cast<gte::IEEEBinary16>(std::pow(static_cast<float>(x), static_cast<float>(y)));
    }

    inline gte::IEEEBinary16 sin(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::sin(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 sinh(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::sinh(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 sqrt(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::sqrt(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 tan(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::tan(static_cast<float>(x)));
    }

    inline gte::IEEEBinary16 tanh(gte::IEEEBinary16 x)
    {
        return static_cast<gte::IEEEBinary16>(std::tanh(static_cast<float>(x)));
    }
}

namespace gte
{
    inline IEEEBinary16 atandivpi(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(atandivpi(static_cast<float>(x)));
    }

    inline IEEEBinary16 atan2divpi(IEEEBinary16 y, IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(atan2divpi(static_cast<float>(y), static_cast<float>(x)));
    }

    inline IEEEBinary16 clamp(IEEEBinary16 x, IEEEBinary16 xmin, IEEEBinary16 xmax)
    {
        return static_cast<IEEEBinary16>(clamp(static_cast<float>(x), static_cast<float>(xmin), static_cast<float>(xmax)));
    }

    inline IEEEBinary16 cospi(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(cospi(static_cast<float>(x)));
    }

    inline IEEEBinary16 exp10(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(exp10(static_cast<float>(x)));
    }

    inline IEEEBinary16 invsqrt(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(invsqrt(static_cast<float>(x)));
    }

    inline int isign(IEEEBinary16 x)
    {
        return isign(static_cast<float>(x));
    }

    inline IEEEBinary16 saturate(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(saturate(static_cast<float>(x)));
    }

    inline IEEEBinary16 sign(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(sign(static_cast<float>(x)));
    }

    inline IEEEBinary16 sinpi(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(sinpi(static_cast<float>(x)));
    }

    inline IEEEBinary16 sqr(IEEEBinary16 x)
    {
        return static_cast<IEEEBinary16>(sqr(static_cast<float>(x)));
    }
}
