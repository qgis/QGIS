/******************************************************************************
 * $Id$
 *
 * Project:  GDAL Core
 * Purpose:  Inline C++ templates
 * Author:   Phil Vachon, <philippe at cowpig.ca>
 *
 ******************************************************************************
 * Copyright (c) 2009, Phil Vachon, <philippe at cowpig.ca>
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#ifndef GDAL_PRIV_TEMPLATES_HPP_INCLUDED
#define GDAL_PRIV_TEMPLATES_HPP_INCLUDED

#include "cpl_port.h"

#include <cmath>
#include <cstdint>
#include <limits>

/************************************************************************/
/*                        GDALGetDataLimits()                           */
/************************************************************************/
/**
 * Compute the limits of values that can be placed in Tout in terms of
 * Tin. Usually used for output clamping, when the output data type's
 * limits are stable relative to the input type (i.e. no roundoff error).
 *
 * @param tMaxValue the returned maximum value
 * @param tMinValue the returned minimum value
 */

template <class Tin, class Tout>
inline void GDALGetDataLimits(Tin &tMaxValue, Tin &tMinValue)
{
    tMaxValue = std::numeric_limits<Tin>::max();
    tMinValue = std::numeric_limits<Tin>::min();

    // Compute the actual minimum value of Tout in terms of Tin.
    if constexpr (std::numeric_limits<Tout>::is_signed &&
                  std::numeric_limits<Tout>::is_integer)
    {
        // the minimum value is less than zero
        if constexpr (std::numeric_limits<Tout>::digits <
                          std::numeric_limits<Tin>::digits ||
                      !std::numeric_limits<Tin>::is_integer)
        {
            // Tout is smaller than Tin, so we need to clamp values in input
            // to the range of Tout's min/max values
            if (std::numeric_limits<Tin>::is_signed)
            {
                tMinValue = static_cast<Tin>(std::numeric_limits<Tout>::min());
            }
            tMaxValue = static_cast<Tin>(std::numeric_limits<Tout>::max());
        }
    }
    else if constexpr (std::numeric_limits<Tout>::is_integer)
    {
        // the output is unsigned, so we just need to determine the max
        /* coverity[same_on_both_sides] */
        if constexpr (std::numeric_limits<Tout>::digits <=
                      std::numeric_limits<Tin>::digits)
        {
            // Tout is smaller than Tin, so we need to clamp the input values
            // to the range of Tout's max
            tMaxValue = static_cast<Tin>(std::numeric_limits<Tout>::max());
        }
        tMinValue = 0;
    }
}

/************************************************************************/
/*                          GDALClampValue()                            */
/************************************************************************/
/**
 * Clamp values of type T to a specified range
 *
 * @param tValue the value
 * @param tMax the max value
 * @param tMin the min value
 */
template <class T>
inline T GDALClampValue(const T tValue, const T tMax, const T tMin)
{
    return tValue > tMax ? tMax : tValue < tMin ? tMin : tValue;
}

/************************************************************************/
/*                          GDALClampDoubleValue()                            */
/************************************************************************/
/**
 * Clamp double values to a specified range, this uses the same
 * argument ordering as std::clamp, returns TRUE if the value was clamped.
 *
 * @param tValue the value
 * @param tMin the min value
 * @param tMax the max value
 *
 */
template <class T2, class T3>
inline bool GDALClampDoubleValue(double &tValue, const T2 tMin, const T3 tMax)
{
    const double tMin2{static_cast<double>(tMin)};
    const double tMax2{static_cast<double>(tMax)};
    if (tValue > tMax2 || tValue < tMin2)
    {
        tValue = tValue > tMax2 ? tMax2 : tValue < tMin2 ? tMin2 : tValue;
        return true;
    }
    else
    {
        return false;
    }
}

/************************************************************************/
/*                         GDALIsValueInRange()                         */
/************************************************************************/
/**
 * Returns whether a value is in the type range.
 * NaN is considered not to be in type range.
 *
 * @param dfValue the value
 * @return whether the value is in the type range.
 */
template <class T> inline bool GDALIsValueInRange(double dfValue)
{
    return dfValue >= static_cast<double>(std::numeric_limits<T>::lowest()) &&
           dfValue <= static_cast<double>(std::numeric_limits<T>::max());
}

template <> inline bool GDALIsValueInRange<double>(double dfValue)
{
    return !std::isnan(dfValue);
}

template <> inline bool GDALIsValueInRange<float>(double dfValue)
{
    return std::isinf(dfValue) ||
           (dfValue >= -std::numeric_limits<float>::max() &&
            dfValue <= std::numeric_limits<float>::max());
}

template <> inline bool GDALIsValueInRange<int64_t>(double dfValue)
{
    // Values in the range [INT64_MAX - 1023, INT64_MAX - 1]
    // get converted to a double that once cast to int64_t is
    // INT64_MAX + 1, hence the < strict comparison.
    return dfValue >=
               static_cast<double>(std::numeric_limits<int64_t>::min()) &&
           dfValue < static_cast<double>(std::numeric_limits<int64_t>::max());
}

template <> inline bool GDALIsValueInRange<uint64_t>(double dfValue)
{
    // Values in the range [UINT64_MAX - 2047, UINT64_MAX - 1]
    // get converted to a double that once cast to uint64_t is
    // UINT64_MAX + 1, hence the < strict comparison.
    return dfValue >= 0 &&
           dfValue < static_cast<double>(std::numeric_limits<uint64_t>::max());
}

/************************************************************************/
/*                         GDALIsValueExactAs()                         */
/************************************************************************/
/**
 * Returns whether a value can be exactly represented on type T.
 *
 * That is static_cast\<double\>(static_cast\<T\>(dfValue)) is legal and is
 * equal to dfValue.
 *
 * Note: for T=float or double, a NaN input leads to true
 *
 * @param dfValue the value
 * @return whether the value can be exactly represented on type T.
 */
template <class T> inline bool GDALIsValueExactAs(double dfValue)
{
    return GDALIsValueInRange<T>(dfValue) &&
           static_cast<double>(static_cast<T>(dfValue)) == dfValue;
}

template <> inline bool GDALIsValueExactAs<float>(double dfValue)
{
    return std::isnan(dfValue) ||
           (GDALIsValueInRange<float>(dfValue) &&
            static_cast<double>(static_cast<float>(dfValue)) == dfValue);
}

template <> inline bool GDALIsValueExactAs<double>(double)
{
    return true;
}

/************************************************************************/
/*                          GDALCopyWord()                              */
/************************************************************************/

template <class Tin, class Tout> struct sGDALCopyWord
{
    static inline void f(const Tin tValueIn, Tout &tValueOut)
    {
        Tin tMaxVal, tMinVal;
        GDALGetDataLimits<Tin, Tout>(tMaxVal, tMinVal);
        tValueOut =
            static_cast<Tout>(GDALClampValue(tValueIn, tMaxVal, tMinVal));
    }
};

template <class Tin> struct sGDALCopyWord<Tin, float>
{
    static inline void f(const Tin tValueIn, float &fValueOut)
    {
        fValueOut = static_cast<float>(tValueIn);
    }
};

template <class Tin> struct sGDALCopyWord<Tin, double>
{
    static inline void f(const Tin tValueIn, double &dfValueOut)
    {
        dfValueOut = static_cast<double>(tValueIn);
    }
};

template <> struct sGDALCopyWord<double, double>
{
    static inline void f(const double dfValueIn, double &dfValueOut)
    {
        dfValueOut = dfValueIn;
    }
};

template <> struct sGDALCopyWord<float, float>
{
    static inline void f(const float fValueIn, float &fValueOut)
    {
        fValueOut = fValueIn;
    }
};

template <> struct sGDALCopyWord<float, double>
{
    static inline void f(const float fValueIn, double &dfValueOut)
    {
        dfValueOut = fValueIn;
    }
};

template <> struct sGDALCopyWord<double, float>
{
    static inline void f(const double dfValueIn, float &fValueOut)
    {
        if (dfValueIn > std::numeric_limits<float>::max())
        {
            fValueOut = std::numeric_limits<float>::infinity();
            return;
        }
        if (dfValueIn < -std::numeric_limits<float>::max())
        {
            fValueOut = -std::numeric_limits<float>::infinity();
            return;
        }

        fValueOut = static_cast<float>(dfValueIn);
    }
};

template <class Tout> struct sGDALCopyWord<float, Tout>
{
    static inline void f(const float fValueIn, Tout &tValueOut)
    {
        if (std::isnan(fValueIn))
        {
            tValueOut = 0;
            return;
        }
        float fMaxVal, fMinVal;
        GDALGetDataLimits<float, Tout>(fMaxVal, fMinVal);
        tValueOut = static_cast<Tout>(
            GDALClampValue(fValueIn + 0.5f, fMaxVal, fMinVal));
    }
};

template <> struct sGDALCopyWord<float, short>
{
    static inline void f(const float fValueIn, short &nValueOut)
    {
        if (std::isnan(fValueIn))
        {
            nValueOut = 0;
            return;
        }
        float fMaxVal, fMinVal;
        GDALGetDataLimits<float, short>(fMaxVal, fMinVal);
        float fValue = fValueIn >= 0.0f ? fValueIn + 0.5f : fValueIn - 0.5f;
        nValueOut =
            static_cast<short>(GDALClampValue(fValue, fMaxVal, fMinVal));
    }
};

template <> struct sGDALCopyWord<float, signed char>
{
    static inline void f(const float fValueIn, signed char &nValueOut)
    {
        if (std::isnan(fValueIn))
        {
            nValueOut = 0;
            return;
        }
        float fMaxVal, fMinVal;
        GDALGetDataLimits<float, signed char>(fMaxVal, fMinVal);
        float fValue = fValueIn >= 0.0f ? fValueIn + 0.5f : fValueIn - 0.5f;
        nValueOut =
            static_cast<signed char>(GDALClampValue(fValue, fMaxVal, fMinVal));
    }
};

template <class Tout> struct sGDALCopyWord<double, Tout>
{
    static inline void f(const double dfValueIn, Tout &tValueOut)
    {
        if (std::isnan(dfValueIn))
        {
            tValueOut = 0;
            return;
        }
        double dfMaxVal, dfMinVal;
        GDALGetDataLimits<double, Tout>(dfMaxVal, dfMinVal);
        tValueOut = static_cast<Tout>(
            GDALClampValue(dfValueIn + 0.5, dfMaxVal, dfMinVal));
    }
};

template <> struct sGDALCopyWord<double, int>
{
    static inline void f(const double dfValueIn, int &nValueOut)
    {
        if (std::isnan(dfValueIn))
        {
            nValueOut = 0;
            return;
        }
        double dfMaxVal, dfMinVal;
        GDALGetDataLimits<double, int>(dfMaxVal, dfMinVal);
        double dfValue = dfValueIn >= 0.0 ? dfValueIn + 0.5 : dfValueIn - 0.5;
        nValueOut =
            static_cast<int>(GDALClampValue(dfValue, dfMaxVal, dfMinVal));
    }
};

template <> struct sGDALCopyWord<double, std::int64_t>
{
    static inline void f(const double dfValueIn, std::int64_t &nValueOut)
    {
        if (std::isnan(dfValueIn))
        {
            nValueOut = 0;
        }
        else if (dfValueIn >=
                 static_cast<double>(std::numeric_limits<std::int64_t>::max()))
        {
            nValueOut = std::numeric_limits<std::int64_t>::max();
        }
        else if (dfValueIn <=
                 static_cast<double>(std::numeric_limits<std::int64_t>::min()))
        {
            nValueOut = std::numeric_limits<std::int64_t>::min();
        }
        else
        {
            nValueOut = static_cast<std::int64_t>(
                dfValueIn > 0.0f ? dfValueIn + 0.5f : dfValueIn - 0.5f);
        }
    }
};

template <> struct sGDALCopyWord<double, std::uint64_t>
{
    static inline void f(const double dfValueIn, std::uint64_t &nValueOut)
    {
        if (!(dfValueIn > 0))
        {
            nValueOut = 0;
        }
        else if (dfValueIn >
                 static_cast<double>(std::numeric_limits<uint64_t>::max()))
        {
            nValueOut = std::numeric_limits<uint64_t>::max();
        }
        else
        {
            nValueOut = static_cast<std::uint64_t>(dfValueIn + 0.5);
        }
    }
};

template <> struct sGDALCopyWord<double, short>
{
    static inline void f(const double dfValueIn, short &nValueOut)
    {
        if (std::isnan(dfValueIn))
        {
            nValueOut = 0;
            return;
        }
        double dfMaxVal, dfMinVal;
        GDALGetDataLimits<double, short>(dfMaxVal, dfMinVal);
        double dfValue = dfValueIn > 0.0 ? dfValueIn + 0.5 : dfValueIn - 0.5;
        nValueOut =
            static_cast<short>(GDALClampValue(dfValue, dfMaxVal, dfMinVal));
    }
};

template <> struct sGDALCopyWord<double, signed char>
{
    static inline void f(const double dfValueIn, signed char &nValueOut)
    {
        if (std::isnan(dfValueIn))
        {
            nValueOut = 0;
            return;
        }
        double dfMaxVal, dfMinVal;
        GDALGetDataLimits<double, signed char>(dfMaxVal, dfMinVal);
        double dfValue = dfValueIn > 0.0 ? dfValueIn + 0.5 : dfValueIn - 0.5;
        nValueOut = static_cast<signed char>(
            GDALClampValue(dfValue, dfMaxVal, dfMinVal));
    }
};

// Roundoff occurs for Float32 -> int32 for max/min. Overload GDALCopyWord
// specifically for this case.
template <> struct sGDALCopyWord<float, int>
{
    static inline void f(const float fValueIn, int &nValueOut)
    {
        if (std::isnan(fValueIn))
        {
            nValueOut = 0;
        }
        else if (fValueIn >=
                 static_cast<float>(std::numeric_limits<int>::max()))
        {
            nValueOut = std::numeric_limits<int>::max();
        }
        else if (fValueIn <=
                 static_cast<float>(std::numeric_limits<int>::min()))
        {
            nValueOut = std::numeric_limits<int>::min();
        }
        else
        {
            nValueOut = static_cast<int>(fValueIn > 0.0f ? fValueIn + 0.5f
                                                         : fValueIn - 0.5f);
        }
    }
};

// Roundoff occurs for Float32 -> uint32 for max. Overload GDALCopyWord
// specifically for this case.
template <> struct sGDALCopyWord<float, unsigned int>
{
    static inline void f(const float fValueIn, unsigned int &nValueOut)
    {
        if (!(fValueIn > 0))
        {
            nValueOut = 0;
        }
        else if (fValueIn >=
                 static_cast<float>(std::numeric_limits<unsigned int>::max()))
        {
            nValueOut = std::numeric_limits<unsigned int>::max();
        }
        else
        {
            nValueOut = static_cast<unsigned int>(fValueIn + 0.5f);
        }
    }
};

// Roundoff occurs for Float32 -> std::int64_t for max/min. Overload
// GDALCopyWord specifically for this case.
template <> struct sGDALCopyWord<float, std::int64_t>
{
    static inline void f(const float fValueIn, std::int64_t &nValueOut)
    {
        if (std::isnan(fValueIn))
        {
            nValueOut = 0;
        }
        else if (fValueIn >=
                 static_cast<float>(std::numeric_limits<std::int64_t>::max()))
        {
            nValueOut = std::numeric_limits<std::int64_t>::max();
        }
        else if (fValueIn <=
                 static_cast<float>(std::numeric_limits<std::int64_t>::min()))
        {
            nValueOut = std::numeric_limits<std::int64_t>::min();
        }
        else
        {
            nValueOut = static_cast<std::int64_t>(
                fValueIn > 0.0f ? fValueIn + 0.5f : fValueIn - 0.5f);
        }
    }
};

// Roundoff occurs for Float32 -> std::uint64_t for max. Overload GDALCopyWord
// specifically for this case.
template <> struct sGDALCopyWord<float, std::uint64_t>
{
    static inline void f(const float fValueIn, std::uint64_t &nValueOut)
    {
        if (!(fValueIn > 0))
        {
            nValueOut = 0;
        }
        else if (fValueIn >=
                 static_cast<float>(std::numeric_limits<std::uint64_t>::max()))
        {
            nValueOut = std::numeric_limits<std::uint64_t>::max();
        }
        else
        {
            nValueOut = static_cast<std::uint64_t>(fValueIn + 0.5f);
        }
    }
};

/**
 * Copy a single word, optionally rounding if appropriate (i.e. going
 * from the float to the integer case). Note that this is the function
 * you should specialize if you're adding a new data type.
 *
 * @param tValueIn value of type Tin; the input value to be converted
 * @param tValueOut value of type Tout; the output value
 */

template <class Tin, class Tout>
inline void GDALCopyWord(const Tin tValueIn, Tout &tValueOut)
{
    sGDALCopyWord<Tin, Tout>::f(tValueIn, tValueOut);
}

/************************************************************************/
/*                         GDALCopy4Words()                             */
/************************************************************************/
/**
 * Copy 4 packed words to 4 packed words, optionally rounding if appropriate
 * (i.e. going from the float to the integer case).
 *
 * @param pValueIn pointer to 4 input values of type Tin.
 * @param pValueOut pointer to 4 output values of type Tout.
 */

template <class Tin, class Tout>
inline void GDALCopy4Words(const Tin *pValueIn, Tout *const pValueOut)
{
    GDALCopyWord(pValueIn[0], pValueOut[0]);
    GDALCopyWord(pValueIn[1], pValueOut[1]);
    GDALCopyWord(pValueIn[2], pValueOut[2]);
    GDALCopyWord(pValueIn[3], pValueOut[3]);
}

/************************************************************************/
/*                         GDALCopy8Words()                             */
/************************************************************************/
/**
 * Copy 8 packed words to 8 packed words, optionally rounding if appropriate
 * (i.e. going from the float to the integer case).
 *
 * @param pValueIn pointer to 8 input values of type Tin.
 * @param pValueOut pointer to 8 output values of type Tout.
 */

template <class Tin, class Tout>
inline void GDALCopy8Words(const Tin *pValueIn, Tout *const pValueOut)
{
    GDALCopy4Words(pValueIn, pValueOut);
    GDALCopy4Words(pValueIn + 4, pValueOut + 4);
}

// Needs SSE2
#if defined(__x86_64) || defined(_M_X64) || defined(USE_SSE2)

#include <emmintrin.h>

static inline void GDALCopyXMMToInt32(const __m128i xmm, void *pDest)
{
    int n32 = _mm_cvtsi128_si32(xmm);  // Extract lower 32 bit word
    memcpy(pDest, &n32, sizeof(n32));
}

static inline void GDALCopyXMMToInt64(const __m128i xmm, void *pDest)
{
    _mm_storel_epi64(reinterpret_cast<__m128i *>(pDest), xmm);
}

#if __SSSE3__
#include <tmmintrin.h>
#endif

#if __SSE4_1__
#include <smmintrin.h>
#endif

template <>
inline void GDALCopy4Words(const float *pValueIn, GByte *const pValueOut)
{
    __m128 xmm = _mm_loadu_ps(pValueIn);

    // The following clamping would be useless due to the final saturating
    // packing if we could guarantee the input range in [INT_MIN,INT_MAX]
    const __m128 p0d5 = _mm_set1_ps(0.5f);
    const __m128 xmm_max = _mm_set1_ps(255);
    xmm = _mm_add_ps(xmm, p0d5);
    xmm = _mm_min_ps(_mm_max_ps(xmm, p0d5), xmm_max);

    __m128i xmm_i = _mm_cvttps_epi32(xmm);

#if __SSSE3__
    xmm_i = _mm_shuffle_epi8(
        xmm_i, _mm_cvtsi32_si128(0 | (4 << 8) | (8 << 16) | (12 << 24)));
#else
    xmm_i = _mm_packs_epi32(xmm_i, xmm_i);   // Pack int32 to int16
    xmm_i = _mm_packus_epi16(xmm_i, xmm_i);  // Pack int16 to uint8
#endif
    GDALCopyXMMToInt32(xmm_i, pValueOut);
}

template <>
inline void GDALCopy4Words(const float *pValueIn, GInt16 *const pValueOut)
{
    __m128 xmm = _mm_loadu_ps(pValueIn);

    const __m128 xmm_min = _mm_set1_ps(-32768);
    const __m128 xmm_max = _mm_set1_ps(32767);
    xmm = _mm_min_ps(_mm_max_ps(xmm, xmm_min), xmm_max);

    const __m128 p0d5 = _mm_set1_ps(0.5f);
    const __m128 m0d5 = _mm_set1_ps(-0.5f);
    const __m128 mask = _mm_cmpge_ps(xmm, p0d5);
    // f >= 0.5f ? f + 0.5f : f - 0.5f
    xmm = _mm_add_ps(
        xmm, _mm_or_ps(_mm_and_ps(mask, p0d5), _mm_andnot_ps(mask, m0d5)));

    __m128i xmm_i = _mm_cvttps_epi32(xmm);

    xmm_i = _mm_packs_epi32(xmm_i, xmm_i);  // Pack int32 to int16
    GDALCopyXMMToInt64(xmm_i, pValueOut);
}

template <>
inline void GDALCopy4Words(const float *pValueIn, GUInt16 *const pValueOut)
{
    __m128 xmm = _mm_loadu_ps(pValueIn);

    const __m128 p0d5 = _mm_set1_ps(0.5f);
    const __m128 xmm_max = _mm_set1_ps(65535);
    xmm = _mm_add_ps(xmm, p0d5);
    xmm = _mm_min_ps(_mm_max_ps(xmm, p0d5), xmm_max);

    __m128i xmm_i = _mm_cvttps_epi32(xmm);

#if __SSE4_1__
    xmm_i = _mm_packus_epi32(xmm_i, xmm_i);  // Pack int32 to uint16
#else
    // Translate to int16 range because _mm_packus_epi32 is SSE4.1 only
    xmm_i = _mm_add_epi32(xmm_i, _mm_set1_epi32(-32768));
    xmm_i = _mm_packs_epi32(xmm_i, xmm_i);  // Pack int32 to int16
    // Translate back to uint16 range (actually -32768==32768 in int16)
    xmm_i = _mm_add_epi16(xmm_i, _mm_set1_epi16(-32768));
#endif
    GDALCopyXMMToInt64(xmm_i, pValueOut);
}

#ifdef __AVX2__

#include <immintrin.h>

template <>
inline void GDALCopy8Words(const float *pValueIn, GByte *const pValueOut)
{
    __m256 ymm = _mm256_loadu_ps(pValueIn);

    const __m256 p0d5 = _mm256_set1_ps(0.5f);
    const __m256 ymm_max = _mm256_set1_ps(255);
    ymm = _mm256_add_ps(ymm, p0d5);
    ymm = _mm256_min_ps(_mm256_max_ps(ymm, p0d5), ymm_max);

    __m256i ymm_i = _mm256_cvttps_epi32(ymm);

    ymm_i = _mm256_packus_epi32(ymm_i, ymm_i);  // Pack int32 to uint16
    ymm_i = _mm256_permute4x64_epi64(ymm_i, 0 | (2 << 2));  // AVX2

    __m128i xmm_i = _mm256_castsi256_si128(ymm_i);
    xmm_i = _mm_packus_epi16(xmm_i, xmm_i);
    GDALCopyXMMToInt64(xmm_i, pValueOut);
}

template <>
inline void GDALCopy8Words(const float *pValueIn, GUInt16 *const pValueOut)
{
    __m256 ymm = _mm256_loadu_ps(pValueIn);

    const __m256 p0d5 = _mm256_set1_ps(0.5f);
    const __m256 ymm_max = _mm256_set1_ps(65535);
    ymm = _mm256_add_ps(ymm, p0d5);
    ymm = _mm256_min_ps(_mm256_max_ps(ymm, p0d5), ymm_max);

    __m256i ymm_i = _mm256_cvttps_epi32(ymm);

    ymm_i = _mm256_packus_epi32(ymm_i, ymm_i);  // Pack int32 to uint16
    ymm_i = _mm256_permute4x64_epi64(ymm_i, 0 | (2 << 2));  // AVX2

    _mm_storeu_si128(reinterpret_cast<__m128i *>(pValueOut),
                     _mm256_castsi256_si128(ymm_i));
}
#else
template <>
inline void GDALCopy8Words(const float *pValueIn, GUInt16 *const pValueOut)
{
    __m128 xmm = _mm_loadu_ps(pValueIn);
    __m128 xmm1 = _mm_loadu_ps(pValueIn + 4);

    const __m128 p0d5 = _mm_set1_ps(0.5f);
    const __m128 xmm_max = _mm_set1_ps(65535);
    xmm = _mm_add_ps(xmm, p0d5);
    xmm1 = _mm_add_ps(xmm1, p0d5);
    xmm = _mm_min_ps(_mm_max_ps(xmm, p0d5), xmm_max);
    xmm1 = _mm_min_ps(_mm_max_ps(xmm1, p0d5), xmm_max);

    __m128i xmm_i = _mm_cvttps_epi32(xmm);
    __m128i xmm1_i = _mm_cvttps_epi32(xmm1);

#if __SSE4_1__
    xmm_i = _mm_packus_epi32(xmm_i, xmm1_i);  // Pack int32 to uint16
#else
    // Translate to int16 range because _mm_packus_epi32 is SSE4.1 only
    xmm_i = _mm_add_epi32(xmm_i, _mm_set1_epi32(-32768));
    xmm1_i = _mm_add_epi32(xmm1_i, _mm_set1_epi32(-32768));
    xmm_i = _mm_packs_epi32(xmm_i, xmm1_i);  // Pack int32 to int16
    // Translate back to uint16 range (actually -32768==32768 in int16)
    xmm_i = _mm_add_epi16(xmm_i, _mm_set1_epi16(-32768));
#endif
    _mm_storeu_si128(reinterpret_cast<__m128i *>(pValueOut), xmm_i);
}
#endif

#ifdef notdef_because_slightly_slower_than_default_implementation
template <>
inline void GDALCopy4Words(const double *pValueIn, float *const pValueOut)
{
    __m128d float_posmax = _mm_set1_pd(std::numeric_limits<float>::max());
    __m128d float_negmax = _mm_set1_pd(-std::numeric_limits<float>::max());
    __m128d float_posinf = _mm_set1_pd(std::numeric_limits<float>::infinity());
    __m128d float_neginf = _mm_set1_pd(-std::numeric_limits<float>::infinity());
    __m128d val01 = _mm_loadu_pd(pValueIn);
    __m128d val23 = _mm_loadu_pd(pValueIn + 2);
    __m128d mask_max = _mm_cmpge_pd(val01, float_posmax);
    __m128d mask_max23 = _mm_cmpge_pd(val23, float_posmax);
    val01 = _mm_or_pd(_mm_and_pd(mask_max, float_posinf),
                      _mm_andnot_pd(mask_max, val01));
    val23 = _mm_or_pd(_mm_and_pd(mask_max23, float_posinf),
                      _mm_andnot_pd(mask_max23, val23));
    __m128d mask_min = _mm_cmple_pd(val01, float_negmax);
    __m128d mask_min23 = _mm_cmple_pd(val23, float_negmax);
    val01 = _mm_or_pd(_mm_and_pd(mask_min, float_neginf),
                      _mm_andnot_pd(mask_min, val01));
    val23 = _mm_or_pd(_mm_and_pd(mask_min23, float_neginf),
                      _mm_andnot_pd(mask_min23, val23));
    __m128 val01_s = _mm_cvtpd_ps(val01);
    __m128 val23_s = _mm_cvtpd_ps(val23);
    __m128i val01_i = _mm_castps_si128(val01_s);
    __m128i val23_i = _mm_castps_si128(val23_s);
    GDALCopyXMMToInt64(val01_i, pValueOut);
    GDALCopyXMMToInt64(val23_i, pValueOut + 2);
}
#endif

#endif  //  defined(__x86_64) || defined(_M_X64)

#endif  // GDAL_PRIV_TEMPLATES_HPP_INCLUDED
