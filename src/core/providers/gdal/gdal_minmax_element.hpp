/******************************************************************************
 * Project:  GDAL Core
 * Purpose:  Utility functions to find minimum and maximum values in a buffer
 * Author:   Even Rouault, <even dot rouault at spatialys.com>
 *
 ******************************************************************************
 * Copyright (c) 2024, Even Rouault <even dot rouault at spatialys.com>
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#ifndef GDAL_MINMAX_ELEMENT_INCLUDED
#define GDAL_MINMAX_ELEMENT_INCLUDED

// NOTE: This header requires C++17

// This file may be vendored by other applications than GDAL
// WARNING: if modifying this file, please also update the upstream GDAL version
// at https://github.com/OSGeo/gdal/blob/master/gcore/gdal_minmax_element.hpp

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "gdal.h"

#ifdef GDAL_COMPILATION
#define GDAL_MINMAXELT_NS gdal
#elif !defined(GDAL_MINMAXELT_NS)
#error "Please define the GDAL_MINMAXELT_NS macro to define the namespace"
#endif

#if defined(__x86_64) || defined(_M_X64)
#define GDAL_MINMAX_ELEMENT_USE_SSE2
#endif

#ifdef GDAL_MINMAX_ELEMENT_USE_SSE2
// SSE2 header
#include <emmintrin.h>
#endif

#include "gdal_priv_templates.hpp"

namespace GDAL_MINMAXELT_NS
{
namespace detail
{

#ifdef GDAL_MINMAX_ELEMENT_USE_SSE2
/************************************************************************/
/*                            compScalar()                              */
/************************************************************************/

template <class T, bool IS_MAX> inline static bool compScalar(T x, T y)
{
    if constexpr (IS_MAX)
        return x > y;
    else
        return x < y;
}

/************************************************************************/
/*                         extremum_element()                           */
/************************************************************************/

template <class T, bool IS_MAX>
size_t extremum_element(const T *v, size_t size, T noDataValue)
{
    static_assert(!(std::is_floating_point_v<T>));
    if (size == 0)
        return 0;
    size_t idx_of_extremum = 0;
    T extremum = v[0];
    bool extremum_is_nodata = extremum == noDataValue;
    size_t i = 1;
    for (; i < size; ++i)
    {
        if (v[i] != noDataValue &&
            (compScalar<T, IS_MAX>(v[i], extremum) || extremum_is_nodata))
        {
            extremum = v[i];
            idx_of_extremum = i;
            extremum_is_nodata = false;
        }
    }
    return idx_of_extremum;
}

/************************************************************************/
/*                         extremum_element()                           */
/************************************************************************/

template <class T, bool IS_MAX> size_t extremum_element(const T *v, size_t size)
{
    static_assert(!(std::is_floating_point_v<T>));
    if (size == 0)
        return 0;
    size_t idx_of_extremum = 0;
    T extremum = v[0];
    size_t i = 1;
    for (; i < size; ++i)
    {
        if (compScalar<T, IS_MAX>(v[i], extremum))
        {
            extremum = v[i];
            idx_of_extremum = i;
        }
    }
    return idx_of_extremum;
}

#ifdef GDAL_MINMAX_ELEMENT_USE_SSE2

/************************************************************************/
/*                   extremum_element_with_nan()                        */
/************************************************************************/

static inline int8_t Shift8(uint8_t x)
{
    return static_cast<int8_t>(x + std::numeric_limits<int8_t>::min());
}

static inline int16_t Shift16(uint16_t x)
{
    return static_cast<int16_t>(x + std::numeric_limits<int16_t>::min());
}

CPL_NOSANITIZE_UNSIGNED_INT_OVERFLOW
    static inline int32_t Shift32(uint32_t x)
{
    x += static_cast<uint32_t>(std::numeric_limits<int32_t>::min());
    int32_t ret;
    memcpy(&ret, &x, sizeof(x));
    return ret;
}

// Return a _mm128[i|d] register with all its elements set to x
template <class T> static inline auto set1(T x)
{
    if constexpr (std::is_same_v<T, uint8_t>)
        return _mm_set1_epi8(Shift8(x));
    else if constexpr (std::is_same_v<T, int8_t>)
        return _mm_set1_epi8(x);
    else if constexpr (std::is_same_v<T, uint16_t>)
        return _mm_set1_epi16(Shift16(x));
    else if constexpr (std::is_same_v<T, int16_t>)
        return _mm_set1_epi16(x);
    else if constexpr (std::is_same_v<T, uint32_t>)
        return _mm_set1_epi32(Shift32(x));
    else if constexpr (std::is_same_v<T, int32_t>)
        return _mm_set1_epi32(x);
    else if constexpr (std::is_same_v<T, float>)
        return _mm_set1_ps(x);
    else
        return _mm_set1_pd(x);
}

// Return a _mm128[i|d] register with all its elements set to x
template <class T> static inline auto set1_unshifted(T x)
{
    if constexpr (std::is_same_v<T, uint8_t>)
    {
        int8_t xSigned;
        memcpy(&xSigned, &x, sizeof(xSigned));
        return _mm_set1_epi8(xSigned);
    }
    else if constexpr (std::is_same_v<T, int8_t>)
        return _mm_set1_epi8(x);
    else if constexpr (std::is_same_v<T, uint16_t>)
    {
        int16_t xSigned;
        memcpy(&xSigned, &x, sizeof(xSigned));
        return _mm_set1_epi16(xSigned);
    }
    else if constexpr (std::is_same_v<T, int16_t>)
        return _mm_set1_epi16(x);
    else if constexpr (std::is_same_v<T, uint32_t>)
    {
        int32_t xSigned;
        memcpy(&xSigned, &x, sizeof(xSigned));
        return _mm_set1_epi32(xSigned);
    }
    else if constexpr (std::is_same_v<T, int32_t>)
        return _mm_set1_epi32(x);
    else if constexpr (std::is_same_v<T, float>)
        return _mm_set1_ps(x);
    else
        return _mm_set1_pd(x);
}

// Load as many values of type T at a _mm128[i|d] register can contain from x
template <class T> static inline auto loadv(const T *x)
{
    if constexpr (std::is_same_v<T, float>)
        return _mm_loadu_ps(x);
    else if constexpr (std::is_same_v<T, double>)
        return _mm_loadu_pd(x);
    else
        return _mm_loadu_si128(reinterpret_cast<const __m128i *>(x));
}

// Return a __m128i register with bits set when x[i] < y[i] when !IS_MAX
// or x[i] > y[i] when IS_MAX
template <class T, bool IS_MAX, class SSE_T>
static inline __m128i comp(SSE_T x, SSE_T y)
{
    if constexpr (IS_MAX)
    {
        if constexpr (std::is_same_v<T, uint8_t>)
            return _mm_cmpgt_epi8(
                _mm_add_epi8(x,
                             _mm_set1_epi8(std::numeric_limits<int8_t>::min())),
                y);
        else if constexpr (std::is_same_v<T, int8_t>)
            return _mm_cmpgt_epi8(x, y);
        else if constexpr (std::is_same_v<T, uint16_t>)
            return _mm_cmpgt_epi16(
                _mm_add_epi16(
                    x, _mm_set1_epi16(std::numeric_limits<int16_t>::min())),
                y);
        else if constexpr (std::is_same_v<T, int16_t>)
            return _mm_cmpgt_epi16(x, y);
        else if constexpr (std::is_same_v<T, uint32_t>)
            return _mm_cmpgt_epi32(
                _mm_add_epi32(
                    x, _mm_set1_epi32(std::numeric_limits<int32_t>::min())),
                y);
        else if constexpr (std::is_same_v<T, int32_t>)
            return _mm_cmpgt_epi32(x, y);
        // We could use _mm_cmpgt_pX() if there was no NaN values
        else if constexpr (std::is_same_v<T, float>)
            return _mm_castps_si128(_mm_cmpnle_ps(x, y));
        else
            return _mm_castpd_si128(_mm_cmpnle_pd(x, y));
    }
    else
    {
        if constexpr (std::is_same_v<T, uint8_t>)
            return _mm_cmplt_epi8(
                _mm_add_epi8(x,
                             _mm_set1_epi8(std::numeric_limits<int8_t>::min())),
                y);
        else if constexpr (std::is_same_v<T, int8_t>)
            return _mm_cmplt_epi8(x, y);
        else if constexpr (std::is_same_v<T, uint16_t>)
            return _mm_cmplt_epi16(
                _mm_add_epi16(
                    x, _mm_set1_epi16(std::numeric_limits<int16_t>::min())),
                y);
        else if constexpr (std::is_same_v<T, int16_t>)
            return _mm_cmplt_epi16(x, y);
        else if constexpr (std::is_same_v<T, uint32_t>)
            return _mm_cmplt_epi32(
                _mm_add_epi32(
                    x, _mm_set1_epi32(std::numeric_limits<int32_t>::min())),
                y);
        else if constexpr (std::is_same_v<T, int32_t>)
            return _mm_cmplt_epi32(x, y);
        // We could use _mm_cmplt_pX() if there was no NaN values
        else if constexpr (std::is_same_v<T, float>)
            return _mm_castps_si128(_mm_cmpnge_ps(x, y));
        else
            return _mm_castpd_si128(_mm_cmpnge_pd(x, y));
    }
}

// Using SSE2
template <class T, bool IS_MAX, bool HAS_NODATA>
inline size_t extremum_element_with_nan(const T *v, size_t size, T noDataValue)
{
    static_assert(std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t> ||
                  std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t> ||
                  std::is_floating_point_v<T>);
    if (size == 0)
        return 0;
    size_t idx_of_extremum = 0;
    T extremum = v[0];
    [[maybe_unused]] bool extremum_is_invalid = false;
    if constexpr (std::is_floating_point_v<T>)
    {
        extremum_is_invalid = std::isnan(extremum);
    }
    if constexpr (HAS_NODATA)
    {
        if (extremum == noDataValue)
            extremum_is_invalid = true;
    }
    size_t i = 1;

    constexpr size_t VALS_PER_REG = sizeof(set1(extremum)) / sizeof(extremum);
    constexpr int LOOP_UNROLLING = 4;
    // If changing the value, then we need to adjust the number of sse_valX
    // loading in the loop.
    static_assert(LOOP_UNROLLING == 4);
    constexpr size_t VALS_PER_ITER = VALS_PER_REG * LOOP_UNROLLING;

    const auto update = [v, noDataValue, &extremum, &idx_of_extremum,
                         &extremum_is_invalid](size_t idx)
    {
        if constexpr (HAS_NODATA)
        {
            if (v[idx] == noDataValue)
                return;
            if (extremum_is_invalid)
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    if (std::isnan(v[idx]))
                        return;
                }
                extremum = v[idx];
                idx_of_extremum = idx;
                extremum_is_invalid = false;
                return;
            }
        }
        else
        {
            CPL_IGNORE_RET_VAL(noDataValue);
        }
        if (compScalar<T, IS_MAX>(v[idx], extremum))
        {
            extremum = v[idx];
            idx_of_extremum = idx;
            extremum_is_invalid = false;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            if (extremum_is_invalid && !std::isnan(v[idx]))
            {
                extremum = v[idx];
                idx_of_extremum = idx;
                extremum_is_invalid = false;
            }
        }
    };

    for (; i < VALS_PER_ITER && i < size; ++i)
    {
        update(i);
    }

    [[maybe_unused]] auto sse_neutral = set1_unshifted(static_cast<T>(0));
    [[maybe_unused]] auto sse_nodata = set1_unshifted(noDataValue);
    if constexpr (HAS_NODATA)
    {
        for (; i < size && extremum_is_invalid; ++i)
        {
            update(i);
        }
        if (!extremum_is_invalid)
        {
            for (; i < size && (i % VALS_PER_ITER) != 0; ++i)
            {
                update(i);
            }
            sse_neutral = set1_unshifted(extremum);
        }
    }

    auto sse_extremum = set1(extremum);

    [[maybe_unused]] size_t hits = 0;
    const auto sse_iter_count = (size / VALS_PER_ITER) * VALS_PER_ITER;
    for (; i < sse_iter_count; i += VALS_PER_ITER)
    {
        // A bit of loop unrolling to save 3/4 of slow movemask operations.
        auto sse_val0 = loadv(v + i + 0 * VALS_PER_REG);
        auto sse_val1 = loadv(v + i + 1 * VALS_PER_REG);
        auto sse_val2 = loadv(v + i + 2 * VALS_PER_REG);
        auto sse_val3 = loadv(v + i + 3 * VALS_PER_REG);

        if constexpr (HAS_NODATA)
        {
            // Replace all components that are at the nodata value by a
            // neutral value (current minimum)
            if constexpr (std::is_same_v<T, uint8_t> ||
                          std::is_same_v<T, int8_t>)
            {
                const auto replaceNoDataByNeutral =
                    [sse_neutral, sse_nodata](auto sse_val)
                {
                    const auto eq_nodata = _mm_cmpeq_epi8(sse_val, sse_nodata);
                    return _mm_or_si128(_mm_and_si128(eq_nodata, sse_neutral),
                                        _mm_andnot_si128(eq_nodata, sse_val));
                };

                sse_val0 = replaceNoDataByNeutral(sse_val0);
                sse_val1 = replaceNoDataByNeutral(sse_val1);
                sse_val2 = replaceNoDataByNeutral(sse_val2);
                sse_val3 = replaceNoDataByNeutral(sse_val3);
            }
            else if constexpr (std::is_same_v<T, uint16_t> ||
                               std::is_same_v<T, int16_t>)
            {
                const auto replaceNoDataByNeutral =
                    [sse_neutral, sse_nodata](auto sse_val)
                {
                    const auto eq_nodata = _mm_cmpeq_epi16(sse_val, sse_nodata);
                    return _mm_or_si128(_mm_and_si128(eq_nodata, sse_neutral),
                                        _mm_andnot_si128(eq_nodata, sse_val));
                };

                sse_val0 = replaceNoDataByNeutral(sse_val0);
                sse_val1 = replaceNoDataByNeutral(sse_val1);
                sse_val2 = replaceNoDataByNeutral(sse_val2);
                sse_val3 = replaceNoDataByNeutral(sse_val3);
            }
            else if constexpr (std::is_same_v<T, uint32_t> ||
                               std::is_same_v<T, int32_t>)
            {
                const auto replaceNoDataByNeutral =
                    [sse_neutral, sse_nodata](auto sse_val)
                {
                    const auto eq_nodata = _mm_cmpeq_epi32(sse_val, sse_nodata);
                    return _mm_or_si128(_mm_and_si128(eq_nodata, sse_neutral),
                                        _mm_andnot_si128(eq_nodata, sse_val));
                };

                sse_val0 = replaceNoDataByNeutral(sse_val0);
                sse_val1 = replaceNoDataByNeutral(sse_val1);
                sse_val2 = replaceNoDataByNeutral(sse_val2);
                sse_val3 = replaceNoDataByNeutral(sse_val3);
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                const auto replaceNoDataByNeutral =
                    [sse_neutral, sse_nodata](auto sse_val)
                {
                    const auto eq_nodata = _mm_cmpeq_ps(sse_val, sse_nodata);
                    return _mm_or_ps(_mm_and_ps(eq_nodata, sse_neutral),
                                     _mm_andnot_ps(eq_nodata, sse_val));
                };

                sse_val0 = replaceNoDataByNeutral(sse_val0);
                sse_val1 = replaceNoDataByNeutral(sse_val1);
                sse_val2 = replaceNoDataByNeutral(sse_val2);
                sse_val3 = replaceNoDataByNeutral(sse_val3);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                const auto replaceNoDataByNeutral =
                    [sse_neutral, sse_nodata](auto sse_val)
                {
                    const auto eq_nodata = _mm_cmpeq_pd(sse_val, sse_nodata);
                    return _mm_or_pd(_mm_and_pd(eq_nodata, sse_neutral),
                                     _mm_andnot_pd(eq_nodata, sse_val));
                };

                sse_val0 = replaceNoDataByNeutral(sse_val0);
                sse_val1 = replaceNoDataByNeutral(sse_val1);
                sse_val2 = replaceNoDataByNeutral(sse_val2);
                sse_val3 = replaceNoDataByNeutral(sse_val3);
            }
            else
            {
                static_assert(
                    std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t> ||
                    std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t> ||
                    std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t> ||
                    std::is_same_v<T, float> || std::is_same_v<T, double>);
            }
        }

        if (_mm_movemask_epi8(_mm_or_si128(
                _mm_or_si128(comp<T, IS_MAX>(sse_val0, sse_extremum),
                             comp<T, IS_MAX>(sse_val1, sse_extremum)),
                _mm_or_si128(comp<T, IS_MAX>(sse_val2, sse_extremum),
                             comp<T, IS_MAX>(sse_val3, sse_extremum)))) != 0)
        {
            if constexpr (!std::is_same_v<T, int8_t> &&
                          !std::is_same_v<T, uint8_t>)
            {
                // The above tests excluding int8_t/uint8_t is due to the fact
                // with those small ranges of values we will quickly converge
                // to the minimum, so no need to do the below "smart" test.

                if (++hits == size / 16)
                {
                    // If we have an almost sorted array, then using this code path
                    // will hurt performance. Arbitrary give up if we get here
                    // more than 1. / 16 of the size of the array.
                    // fprintf(stderr, "going to non-vector path\n");
                    break;
                }
            }
            for (size_t j = 0; j < VALS_PER_ITER; j++)
            {
                update(i + j);
            }
            sse_extremum = set1(extremum);
            if constexpr (HAS_NODATA)
            {
                sse_neutral = set1_unshifted(extremum);
            }
        }
    }
    for (; i < size; ++i)
    {
        update(i);
    }
    return idx_of_extremum;
}

#else

/************************************************************************/
/*                    extremum_element_with_nan()                       */
/************************************************************************/

template <class T, bool IS_MAX, bool HAS_NODATA>
inline size_t extremum_element_with_nan(const T *v, size_t size, T /* nodata */)
{
    static_assert(!HAS_NODATA);
    if (size == 0)
        return 0;
    size_t idx_of_extremum = 0;
    auto extremum = v[0];
    bool extremum_is_nan = std::isnan(extremum);
    size_t i = 1;
    for (; i < size; ++i)
    {
        if (compScalar<T, IS_MAX>(v[i], extremum) ||
            (extremum_is_nan && !std::isnan(v[i])))
        {
            extremum = v[i];
            idx_of_extremum = i;
            extremum_is_nan = false;
        }
    }
    return idx_of_extremum;
}
#endif

/************************************************************************/
/*                         extremum_element()                           */
/************************************************************************/

#ifdef GDAL_MINMAX_ELEMENT_USE_SSE2

template <>
size_t extremum_element<uint8_t, true>(const uint8_t *v, size_t size,
                                       uint8_t noDataValue)
{
    return extremum_element_with_nan<uint8_t, true, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<uint8_t, false>(const uint8_t *v, size_t size,
                                        uint8_t noDataValue)
{
    return extremum_element_with_nan<uint8_t, false, true>(v, size,
                                                           noDataValue);
}

template <>
size_t extremum_element<uint8_t, true>(const uint8_t *v, size_t size)
{
    return extremum_element_with_nan<uint8_t, true, false>(v, size, 0);
}

template <>
size_t extremum_element<uint8_t, false>(const uint8_t *v, size_t size)
{
    return extremum_element_with_nan<uint8_t, false, false>(v, size, 0);
}

template <>
size_t extremum_element<int8_t, true>(const int8_t *v, size_t size,
                                      int8_t noDataValue)
{
    return extremum_element_with_nan<int8_t, true, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<int8_t, false>(const int8_t *v, size_t size,
                                       int8_t noDataValue)
{
    return extremum_element_with_nan<int8_t, false, true>(v, size, noDataValue);
}

template <> size_t extremum_element<int8_t, true>(const int8_t *v, size_t size)
{
    return extremum_element_with_nan<int8_t, true, false>(v, size, 0);
}

template <> size_t extremum_element<int8_t, false>(const int8_t *v, size_t size)
{
    return extremum_element_with_nan<int8_t, false, false>(v, size, 0);
}

template <>
size_t extremum_element<uint16_t, true>(const uint16_t *v, size_t size,
                                        uint16_t noDataValue)
{
    return extremum_element_with_nan<uint16_t, true, true>(v, size,
                                                           noDataValue);
}

template <>
size_t extremum_element<uint16_t, false>(const uint16_t *v, size_t size,
                                         uint16_t noDataValue)
{
    return extremum_element_with_nan<uint16_t, false, true>(v, size,
                                                            noDataValue);
}

template <>
size_t extremum_element<uint16_t, true>(const uint16_t *v, size_t size)
{
    return extremum_element_with_nan<uint16_t, true, false>(v, size, 0);
}

template <>
size_t extremum_element<uint16_t, false>(const uint16_t *v, size_t size)
{
    return extremum_element_with_nan<uint16_t, false, false>(v, size, 0);
}

template <>
size_t extremum_element<int16_t, true>(const int16_t *v, size_t size,
                                       int16_t noDataValue)
{
    return extremum_element_with_nan<int16_t, true, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<int16_t, false>(const int16_t *v, size_t size,
                                        int16_t noDataValue)
{
    return extremum_element_with_nan<int16_t, false, true>(v, size,
                                                           noDataValue);
}

template <>
size_t extremum_element<int16_t, true>(const int16_t *v, size_t size)
{
    return extremum_element_with_nan<int16_t, true, false>(v, size, 0);
}

template <>
size_t extremum_element<int16_t, false>(const int16_t *v, size_t size)
{
    return extremum_element_with_nan<int16_t, false, false>(v, size, 0);
}

template <>
size_t extremum_element<uint32_t, true>(const uint32_t *v, size_t size,
                                        uint32_t noDataValue)
{
    return extremum_element_with_nan<uint32_t, true, true>(v, size,
                                                           noDataValue);
}

template <>
size_t extremum_element<uint32_t, false>(const uint32_t *v, size_t size,
                                         uint32_t noDataValue)
{
    return extremum_element_with_nan<uint32_t, false, true>(v, size,
                                                            noDataValue);
}

template <>
size_t extremum_element<uint32_t, true>(const uint32_t *v, size_t size)
{
    return extremum_element_with_nan<uint32_t, true, false>(v, size, 0);
}

template <>
size_t extremum_element<uint32_t, false>(const uint32_t *v, size_t size)
{
    return extremum_element_with_nan<uint32_t, false, false>(v, size, 0);
}

template <>
size_t extremum_element<int32_t, true>(const int32_t *v, size_t size,
                                       int32_t noDataValue)
{
    return extremum_element_with_nan<int32_t, true, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<int32_t, false>(const int32_t *v, size_t size,
                                        int32_t noDataValue)
{
    return extremum_element_with_nan<int32_t, false, true>(v, size,
                                                           noDataValue);
}

template <>
size_t extremum_element<int32_t, true>(const int32_t *v, size_t size)
{
    return extremum_element_with_nan<int32_t, true, false>(v, size, 0);
}

template <>
size_t extremum_element<int32_t, false>(const int32_t *v, size_t size)
{
    return extremum_element_with_nan<int32_t, false, false>(v, size, 0);
}

template <>
size_t extremum_element<float, true>(const float *v, size_t size,
                                     float noDataValue)
{
    return extremum_element_with_nan<float, true, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<float, false>(const float *v, size_t size,
                                      float noDataValue)
{
    return extremum_element_with_nan<float, false, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<double, true>(const double *v, size_t size,
                                      double noDataValue)
{
    return extremum_element_with_nan<double, true, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<double, false>(const double *v, size_t size,
                                       double noDataValue)
{
    return extremum_element_with_nan<double, false, true>(v, size, noDataValue);
}

#endif

template <> size_t extremum_element<float, true>(const float *v, size_t size)
{
    return extremum_element_with_nan<float, true, false>(v, size, 0);
}

template <> size_t extremum_element<double, true>(const double *v, size_t size)
{
    return extremum_element_with_nan<double, true, false>(v, size, 0);
}

template <> size_t extremum_element<float, false>(const float *v, size_t size)
{
    return extremum_element_with_nan<float, false, false>(v, size, 0);
}

template <> size_t extremum_element<double, false>(const double *v, size_t size)
{
    return extremum_element_with_nan<double, false, false>(v, size, 0);
}

/************************************************************************/
/*                       extremum_element_with_nan()                    */
/************************************************************************/

template <class T, bool IS_MAX>
inline size_t extremum_element_with_nan(const T *v, size_t size, T noDataValue)
{
    if (std::isnan(noDataValue))
        return extremum_element_with_nan<T, IS_MAX, false>(v, size, 0);
    if (size == 0)
        return 0;
    size_t idx_of_extremum = 0;
    auto extremum = v[0];
    bool extremum_is_nan_or_nodata =
        std::isnan(extremum) || (extremum == noDataValue);
    size_t i = 1;
    for (; i < size; ++i)
    {
        if (v[i] != noDataValue &&
            (compScalar<T, IS_MAX>(v[i], extremum) ||
             (extremum_is_nan_or_nodata && !std::isnan(v[i]))))
        {
            extremum = v[i];
            idx_of_extremum = i;
            extremum_is_nan_or_nodata = false;
        }
    }
    return idx_of_extremum;
}

/************************************************************************/
/*                            extremum_element()                        */
/************************************************************************/

#if !defined(GDAL_MINMAX_ELEMENT_USE_SSE2)

template <>
size_t extremum_element<float, true>(const float *v, size_t size,
                                     float noDataValue)
{
    return extremum_element_with_nan<float, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<float, false>(const float *v, size_t size,
                                      float noDataValue)
{
    return extremum_element_with_nan<float, false>(v, size, noDataValue);
}

template <>
size_t extremum_element<double, true>(const double *v, size_t size,
                                      double noDataValue)
{
    return extremum_element_with_nan<double, true>(v, size, noDataValue);
}

template <>
size_t extremum_element<double, false>(const double *v, size_t size,
                                       double noDataValue)
{
    return extremum_element_with_nan<double, false>(v, size, noDataValue);
}

#endif

template <class T, bool IS_MAX>
inline size_t extremum_element(const T *buffer, size_t size, bool bHasNoData,
                               T noDataValue)
{
    if (bHasNoData)
        return extremum_element<T, IS_MAX>(buffer, size, noDataValue);
    else
        return extremum_element<T, IS_MAX>(buffer, size);
}

#else

template <class T, bool IS_MAX>
inline size_t extremum_element(const T *buffer, size_t size, bool bHasNoData,
                               T noDataValue)
{
    if (bHasNoData)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            if (std::isnan(noDataValue))
            {
                if constexpr (IS_MAX)
                {
                    return std::max_element(buffer, buffer + size,
                                            [](T a, T b) {
                                                return std::isnan(b)   ? false
                                                       : std::isnan(a) ? true
                                                                       : a < b;
                                            }) -
                           buffer;
                }
                else
                {
                    return std::min_element(buffer, buffer + size,
                                            [](T a, T b) {
                                                return std::isnan(b)   ? true
                                                       : std::isnan(a) ? false
                                                                       : a < b;
                                            }) -
                           buffer;
                }
            }
            else
            {
                if constexpr (IS_MAX)
                {
                    return std::max_element(buffer, buffer + size,
                                            [noDataValue](T a, T b)
                                            {
                                                return std::isnan(b)   ? false
                                                       : std::isnan(a) ? true
                                                       : (b == noDataValue)
                                                           ? false
                                                           : (a == noDataValue)
                                                                 ? true
                                                                 : a < b;
                                            }) -
                           buffer;
                }
                else
                {
                    return std::min_element(buffer, buffer + size,
                                            [noDataValue](T a, T b)
                                            {
                                                return std::isnan(b)   ? true
                                                       : std::isnan(a) ? false
                                                       : (b == noDataValue)
                                                           ? true
                                                           : (a == noDataValue)
                                                                 ? false
                                                                 : a < b;
                                            }) -
                           buffer;
                }
            }
        }
        else
        {
            if constexpr (IS_MAX)
            {
                return std::max_element(buffer, buffer + size,
                                        [noDataValue](T a, T b) {
                                            return (b == noDataValue)   ? false
                                                   : (a == noDataValue) ? true
                                                                        : a < b;
                                        }) -
                       buffer;
            }
            else
            {
                return std::min_element(buffer, buffer + size,
                                        [noDataValue](T a, T b) {
                                            return (b == noDataValue)   ? true
                                                   : (a == noDataValue) ? false
                                                                        : a < b;
                                        }) -
                       buffer;
            }
        }
    }
    else
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            if constexpr (IS_MAX)
            {
                return std::max_element(buffer, buffer + size,
                                        [](T a, T b) {
                                            return std::isnan(b)   ? false
                                                   : std::isnan(a) ? true
                                                                   : a < b;
                                        }) -
                       buffer;
            }
            else
            {
                return std::min_element(buffer, buffer + size,
                                        [](T a, T b) {
                                            return std::isnan(b)   ? true
                                                   : std::isnan(a) ? false
                                                                   : a < b;
                                        }) -
                       buffer;
            }
        }
        else
        {
            if constexpr (IS_MAX)
            {
                return std::max_element(buffer, buffer + size) - buffer;
            }
            else
            {
                return std::min_element(buffer, buffer + size) - buffer;
            }
        }
    }
}
#endif

template <bool IS_MAX>
size_t extremum_element(const void *buffer, size_t nElts, GDALDataType eDT,
                        bool bHasNoData, double dfNoDataValue)
{
    switch (eDT)
    {
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 7, 0)
    case GDT_Int8:
    {
        using T = int8_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
#endif
    case GDT_Byte:
    {
        using T = uint8_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_Int16:
    {
        using T = int16_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_UInt16:
    {
        using T = uint16_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_Int32:
    {
        using T = int32_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_UInt32:
    {
        using T = uint32_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 5, 0)
    case GDT_Int64:
    {
        using T = int64_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_UInt64:
    {
        using T = uint64_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
#endif
    case GDT_Float32:
    {
        using T = float;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_Float64:
    {
        using T = double;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return extremum_element<T, IS_MAX>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    default:
        break;
    }
    CPLError(CE_Failure, CPLE_NotSupported,
             "%s not supported for this data type.", __FUNCTION__);
    return 0;
}

}  // namespace detail

/************************************************************************/
/*                            max_element()                             */
/************************************************************************/

/** Return the index of the element where the maximum value is hit.
 *
 * If it is hit in several locations, it is not specified which one will be
 * returned.
 *
 * @param buffer Vector of nElts elements of type eDT.
 * @param nElts Number of elements in buffer.
 * @param eDT Data type of the elements of buffer.
 * @param bHasNoData Whether dfNoDataValue is valid.
 * @param dfNoDataValue Nodata value, only taken into account if bHasNoData == true
 *
 * @since GDAL 3.11
 */
inline size_t max_element(const void *buffer, size_t nElts, GDALDataType eDT,
                          bool bHasNoData, double dfNoDataValue)
{
    return detail::extremum_element<true>(buffer, nElts, eDT, bHasNoData,
                                          dfNoDataValue);
}

/************************************************************************/
/*                            min_element()                             */
/************************************************************************/

/** Return the index of the element where the minimum value is hit.
 *
 * If it is hit in several locations, it is not specified which one will be
 * returned.
 *
 * @param buffer Vector of nElts elements of type eDT.
 * @param nElts Number of elements in buffer.
 * @param eDT Data type of the elements of buffer.
 * @param bHasNoData Whether dfNoDataValue is valid.
 * @param dfNoDataValue Nodata value, only taken into account if bHasNoData == true
 *
 * @since GDAL 3.11
 */
inline size_t min_element(const void *buffer, size_t nElts, GDALDataType eDT,
                          bool bHasNoData, double dfNoDataValue)
{
    return detail::extremum_element<false>(buffer, nElts, eDT, bHasNoData,
                                           dfNoDataValue);
}

namespace detail
{

#ifdef NOT_EFFICIENT

/************************************************************************/
/*                         minmax_element()                             */
/************************************************************************/

template <class T>
std::pair<size_t, size_t> minmax_element(const T *v, size_t size, T noDataValue)
{
    static_assert(!(std::is_floating_point_v<T>));
    if (size == 0)
        return std::pair(0, 0);
    size_t idx_of_min = 0;
    size_t idx_of_max = 0;
    T vmin = v[0];
    T vmax = v[0];
    bool extremum_is_nodata = vmin == noDataValue;
    size_t i = 1;
    for (; i < size; ++i)
    {
        if (v[i] != noDataValue && (v[i] < vmin || extremum_is_nodata))
        {
            vmin = v[i];
            idx_of_min = i;
            extremum_is_nodata = false;
        }
        if (v[i] != noDataValue && (v[i] > vmax || extremum_is_nodata))
        {
            vmax = v[i];
            idx_of_max = i;
            extremum_is_nodata = false;
        }
    }
    return std::pair(idx_of_min, idx_of_max);
}

template <class T>
std::pair<size_t, size_t> minmax_element(const T *v, size_t size)
{
    static_assert(!(std::is_floating_point_v<T>));
    if (size == 0)
        return std::pair(0, 0);
    size_t idx_of_min = 0;
    size_t idx_of_max = 0;
    T vmin = v[0];
    T vmax = v[0];
    size_t i = 1;
    for (; i < size; ++i)
    {
        if (v[i] < vmin)
        {
            vmin = v[i];
            idx_of_min = i;
        }
        if (v[i] > vmax)
        {
            vmax = v[i];
            idx_of_max = i;
        }
    }
    return std::pair(idx_of_min, idx_of_max);
}

template <class T>
inline std::pair<size_t, size_t> minmax_element_with_nan(const T *v,
                                                         size_t size)
{
    if (size == 0)
        return std::pair(0, 0);
    size_t idx_of_min = 0;
    size_t idx_of_max = 0;
    T vmin = v[0];
    T vmax = v[0];
    size_t i = 1;
    if (std::isnan(v[0]))
    {
        for (; i < size; ++i)
        {
            if (!std::isnan(v[i]))
            {
                vmin = v[i];
                idx_of_min = i;
                vmax = v[i];
                idx_of_max = i;
                break;
            }
        }
    }
    for (; i < size; ++i)
    {
        if (v[i] < vmin)
        {
            vmin = v[i];
            idx_of_min = i;
        }
        if (v[i] > vmax)
        {
            vmax = v[i];
            idx_of_max = i;
        }
    }
    return std::pair(idx_of_min, idx_of_max);
}

template <>
std::pair<size_t, size_t> minmax_element<float>(const float *v, size_t size)
{
    return minmax_element_with_nan<float>(v, size);
}

template <>
std::pair<size_t, size_t> minmax_element<double>(const double *v, size_t size)
{
    return minmax_element_with_nan<double>(v, size);
}

template <class T>
inline std::pair<size_t, size_t> minmax_element(const T *buffer, size_t size,
                                                bool bHasNoData, T noDataValue)
{
    if (bHasNoData)
    {
        return minmax_element<T>(buffer, size, noDataValue);
    }
    else
    {
        return minmax_element<T>(buffer, size);
    }
}
#else

/************************************************************************/
/*                         minmax_element()                             */
/************************************************************************/

template <class T>
inline std::pair<size_t, size_t> minmax_element(const T *buffer, size_t size,
                                                bool bHasNoData, T noDataValue)
{
#ifdef NOT_EFFICIENT
    if (bHasNoData)
    {
        return minmax_element<T>(buffer, size, noDataValue);
    }
    else
    {
        return minmax_element<T>(buffer, size);
        //auto [imin, imax] = std::minmax_element(buffer, buffer + size);
        //return std::pair(imin - buffer, imax - buffer);
    }
#else

#if !defined(GDAL_MINMAX_ELEMENT_USE_SSE2)
    if constexpr (!std::is_floating_point_v<T>)
    {
        if (!bHasNoData)
        {
            auto [min_iter, max_iter] =
                std::minmax_element(buffer, buffer + size);
            return std::pair(min_iter - buffer, max_iter - buffer);
        }
    }
#endif

    // Using separately min and max is more efficient than computing them
    // within the same loop
    return std::pair(
        extremum_element<T, false>(buffer, size, bHasNoData, noDataValue),
        extremum_element<T, true>(buffer, size, bHasNoData, noDataValue));

#endif
}
#endif

}  // namespace detail

/************************************************************************/
/*                          minmax_element()                            */
/************************************************************************/

/** Return the index of the elements where the minimum and maximum values are hit.
 *
 * If they are hit in several locations, it is not specified which one will be
 * returned (contrary to std::minmax_element).
 *
 * @param buffer Vector of nElts elements of type eDT.
 * @param nElts Number of elements in buffer.
 * @param eDT Data type of the elements of buffer.
 * @param bHasNoData Whether dfNoDataValue is valid.
 * @param dfNoDataValue Nodata value, only taken into account if bHasNoData == true
 *
 * @since GDAL 3.11
 */
inline std::pair<size_t, size_t> minmax_element(const void *buffer,
                                                size_t nElts, GDALDataType eDT,
                                                bool bHasNoData,
                                                double dfNoDataValue)
{
    switch (eDT)
    {
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 7, 0)
    case GDT_Int8:
    {
        using T = int8_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
#endif
    case GDT_Byte:
    {
        using T = uint8_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_Int16:
    {
        using T = int16_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_UInt16:
    {
        using T = uint16_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_Int32:
    {
        using T = int32_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_UInt32:
    {
        using T = uint32_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 5, 0)
    case GDT_Int64:
    {
        using T = int64_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_UInt64:
    {
        using T = uint64_t;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
#endif
    case GDT_Float32:
    {
        using T = float;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    case GDT_Float64:
    {
        using T = double;
        bHasNoData = bHasNoData && GDALIsValueExactAs<T>(dfNoDataValue);
        return detail::minmax_element<T>(
            static_cast<const T *>(buffer), nElts, bHasNoData,
            bHasNoData ? static_cast<T>(dfNoDataValue) : 0);
    }
    default:
        break;
    }
    CPLError(CE_Failure, CPLE_NotSupported,
             "%s not supported for this data type.", __FUNCTION__);
    return std::pair(0, 0);
}

}  // namespace GDAL_MINMAXELT_NS

#endif  // GDAL_MINMAX_ELEMENT_INCLUDED
