// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

// HLSL Shader Compiler 6.3.9600.16384 has poorly written intrinsics for
// frexp and ldexp.  frexp ignores the sign bit and works only on normal
// floats.  It does not work correctly for subnormals, infinities, or NaNs.
// ldexp computes y = x*2^p by computing 2^p (with the 'exp' instruction) and
// then multiplies by x.  The problem is that 2^p can overflow or underflow,
// but x*2^p is a positive finite floating-point number.  The implementations
// provided here match the CPU versions, verified by unit tests.

//----------------------------------------------------------------------------
float frexp_f(float x, out int power)
{
    // Extract the sign, biased exponent, and trailing significand.
    int ix = asint(x);
    int sgn = ix & 0x80000000;
    int biased = (ix & 0x7F800000) >> 23;
    int trailing = ix & 0x007FFFFF;

    // 1/2 combined with sign(x).
    int signedHalf = sgn | 0x3F000000;

    int iy;

    if (biased == 0)
    {
        if (trailing == 0)
        {
            // x is a zero, but the sign bit is discarded
            power = 0;
            iy = 0;
        }
        else
        {
            // x is a subnormal
            int leading = firstbithigh(trailing);
            power = leading - 148;
            trailing = (trailing << (23 - leading)) & ~0x00800000;
            iy = signedHalf | trailing;
        }
    }
    else
    {
        if (biased < 255)
        {
            // x is a normal
            power = biased - 126;
            iy = signedHalf | trailing;
        }
        else
        {
            power = -1;
            if (trailing == 0)
            {
                // x is an infinity
                iy = 0xFFC00000;
            }
            else
            {
                // payload = trailing & 0x003FFFFF
                if (trailing & 0x00400000)
                {
                    // x is a quiet NaN, preserve payload
                    iy = ix;
                }
                else
                {
                    // x is a signaling NaN, make it quiet, preserve payload
                    iy = ix | 0x00400000;
                }
            }
        }
    }

    return asfloat(iy);
}
//----------------------------------------------------------------------------
float ldexp_f(float x, int power)
{
    // Extract the sign, biased exponent, and trailing significand.
    int ix = asint(x);
    int sgn = ix & 0x80000000;
    int biased = (ix & 0x7F800000) >> 23;
    int trailing = ix & 0x007FFFFF;

    // The rounding that occurs in several of the cases is the default IEEE
    // rounding mode: round-to-nearest with ties-to-even.
    int iy;

    if (biased == 0)
    {
        if (trailing == 0)
        {
            // x is a zero
            iy = 0;
        }
        else
        {
            // x is a subnormal, convert to 1.u*2^{expn} and analyze exponent.
            uint leading = firstbithigh(trailing);
            int expn = power + leading - 149;
            if (expn < -149)
            {
                if (expn < -150 || leading == firstbitlow(trailing))
                {
                    // y <= 2^{-150}, so round down to zero.
                    iy = 0;
                }
                else
                {
                    // Round up to min subnormal 2^{-149}.
                    iy = 1;
                }
            }
            else if (expn < -126)
            {
                // The result is subnormal but we need to shift and round
                // properly.
                if (power >= 0)
                {
                    // No rounding is needed.
                    trailing <<= power;
                    iy = trailing;
                }
                else
                {
                    // Rounding test is necessary.  The test involves looking
                    // at the three bits starting with the ULP bit and the
                    // two lower-significant bits after it, plus remainder
                    // analysis which is where the comparisons come in.  The
                    // 'test' value is a shift of 'trailing' to get those
                    // three bits starting in bit 23.
                    int test = trailing << (23 + power);
                    int round = ((test & 0x00FFFFFF) > 0x00400000 &&
                        (test & 0x00400000) > 0 ? 1 : 0);
                    iy = (trailing >> -power) + round;
                }
            }
            else if (expn < 128)
            {
                // The result is normal.  No rounding is needed.  Clear out
                // bit 23 because that 1 is implicit in normal representation.
                trailing = (trailing << (23 - leading)) & ~0x00800000;
                iy = ((expn + 127) << 23) | trailing;
            }
            else
            {
                // The result is infinite.
                iy = 0x7F800000;
            }
        }
    }
    else if (biased < 255)
    {
        // x is a normal
        int expn = power + biased - 127;
        if (expn < -149)
        {
            if (expn < -150
                || firstbithigh(trailing) == firstbitlow(trailing))
            {
                // y <= 2^{-150}, so round down to zero.
                iy = 0;
            }
            else
            {
                // Round up to min subnormal 2^{-149}.
                iy = 1;
            }
        }
        else if (expn < -126)
        {
            // The result is subnormal but we need to shift and round
            // properly.  See the comments previously about 'test'.
            int shift = -126 - expn;  // in {1..23}
            trailing |= 0x00800000;  // Prepend the implicit 1-bit.
            int test = trailing << (23 - shift);
            int round = ((test & 0x00FFFFFF) > 0x00400000 &&
                (test & 0x00400000) > 0 ? 1 : 0);
            iy = (trailing >> shift) + round;
        }
        else if (expn < 128)
        {
            // The result is normal.  No rounding is needed.  The trailing
            // trailing value is used as-is, because the implicit 1-bit
            // already exists.
            iy = ((expn + 127) << 23) | trailing;
        }
        else
        {
            // The result is infinite.
            iy = 0x7F800000;
        }
    }
    else
    {
        if (trailing == 0)
        {
            // x is an infinity
            iy = 0x7F800000;
        }
        else
        {
            // payload = trailing & 0x003FFFFF
            if (trailing & 0x00400000)
            {
                // x is a quiet NaN, preserve payload
                iy = ix;
            }
            else
            {
                // x is a signaling NaN, make it quiet, preserve payload
                iy = ix | 0x00400000;
            }
        }
    }

    iy |= sgn;
    return asfloat(iy);
}
//----------------------------------------------------------------------------
