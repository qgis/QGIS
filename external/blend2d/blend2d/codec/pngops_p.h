// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CODEC_PNGOPS_P_H_INCLUDED
#define BLEND2D_CODEC_PNGOPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/codec/pngcodec_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_codec_impl
//! \{

namespace bl::Png::Ops {

// bl::Png::Ops - Utilities
// ========================

namespace {

//! Returns a simplified filter of the first PNG row, because no prior row exists at that point.
//! This is the only function that can replace AVG filter with `AVG0`.
BL_INLINE uint32_t simplify_filter_of_first_row(uint32_t filter) noexcept {
  constexpr uint32_t kReplacement = (kFilterTypeNone <<  0) | // None  -> None
                                    (kFilterTypeSub  <<  4) | // Sub   -> Sub
                                    (kFilterTypeNone <<  8) | // Up    -> None
                                    (kFilterTypeAvg0 << 12) | // Avg   -> Avg0 (Special-Case)
                                    (kFilterTypeSub  << 16) ; // Paeth -> Sub
  return (kReplacement >> (filter * 4)) & 0xF;
}

// Performs PNG sum filter and casts to byte.
BL_INLINE uint8_t apply_sum_filter(uint32_t a, uint32_t b) noexcept { return uint8_t((a + b) & 0xFFu); }

// Performs PNG average filter.
BL_INLINE uint32_t apply_avg_filter(uint32_t a, uint32_t b) noexcept { return (a + b) >> 1; }

// This is an optimized implementation of PNG's Paeth reference filter. This optimization originally comes from
// my previous implementation where I tried to simplify it to be more SIMD friendly. One interesting property of
// Paeth filter is:
//
//   Paeth(a, b, c) == Paeth(b, a, c);
//
// Actually what the filter needs is a minimum and maximum of `a` and `b`, so I based the implementation on
// getting those first. If you know `min(a, b)` and `max(a, b)` you can divide the interval to be checked
// against `c`. This requires division by 3, which is available above as `udiv3()`.
//
// The previous implementation looked like:
//
// ```
// uint32_t udiv3(uint32_t x) noexcept {
//   return (x * 0xABu) >> 9;
// }
//
// uint32_t apply_paeth_filter(uint32_t a, uint32_t b, uint32_t c) noexcept {
//   uint32_t min_ab = min(a, b);
//   uint32_t max_ab = max(a, b);
//   uint32_t divAB = udiv3(max_ab - min_ab);
//
//   if (c <= min_ab + divAB) return max_ab;
//   if (c >= max_ab - divAB) return min_ab;
//
//   return c;
// }
// ```
//
// Attempt #1
// ----------
//
// Although it's not bad I tried to exploit more the idea of SIMD and masking. The following code basically removes
// the need of any comparison, it relies on bit shifting and performs an arithmetic (not logical) shift of signs
// produced by `divAB + lo` and `divAB - hi`, which are then used to mask out `lo` and `hi`. The `lo` and `hi` can
// be negative after `c` is subtracted, which will basically remove the original `c` if one of the two additions is
// unmasked. The code can unmask either zero or one addition, but it never unmasks both.
//
// ```
// uint32_t udiv3(uint32_t x) noexcept {
//   return (x * 0xABu) >> 9;
// }
//
// uint32_t apply_paeth_filter(uint32_t a, uint32_t b, uint32_t c) noexcept {
//   uint32_t lo = min(a, b) - c;
//   uint32_t hi = max(a, b) - c;
//   uint32_t divAB = udiv3(hi - lo);
//
//   return c + (hi & ~uint32_t(int32_t(divAB + lo) >> 31)) +
//              (lo & ~uint32_t(int32_t(divAB - hi) >> 31)) ;
// }
// ```
//
// Attempt #2
// ----------
//
// There is even a better implementation (not invented here) that further simplifies the code and turns the
// division by 3 into an early multiplication by 3, which is basically `(a + (a << 1))` and can be rewritten
// to use LEA on x86 and shift with accumulation on ARM hardware. The following code is from stb_image library:
//
// ```
// int32_t apply_paeth_filter(int32_t a, int32_t b, int32_t c) noexcept {
//    int32_t threshold = c*3 - (a + b);
//    int32_t lo = min(a, b);
//    int32_t hi = max(a, b);
//    int32_t t0 = (hi <= threshold) ? lo : c;
//    int32_t t1 = (lo >= threshold) ? hi : t0;
//    return t1;
// }
// ```
BL_INLINE uint32_t apply_paeth_filter(uint32_t a, uint32_t b, uint32_t c) noexcept {
  int32_t threshold = int32_t(c * 3u) - int32_t(a + b);

  uint32_t min_ab = bl_min(a, b);
  uint32_t max_ab = bl_max(a, b);

  uint32_t t0 = int32_t(max_ab) > threshold ? c : min_ab;
  uint32_t t1 = threshold > int32_t(min_ab) ? t0 : max_ab;

  return t1;
}

} // {anonymous}

// bl::Png::Ops - Function Table
// =============================

//! Optimized PNG functions.
struct FunctionTable {
  using InverseFilterFunc = BLResult (BL_CDECL*)(uint8_t* p, uint32_t bpp, uint32_t bpl, uint32_t h) noexcept;

  InverseFilterFunc inverse_filter[9];
};
extern FunctionTable func_table;

void init_func_table(BLRuntimeContext* rt) noexcept;
void init_func_table_ref(FunctionTable& ft) noexcept;

#if defined(BL_BUILD_OPT_SSE2)
void init_func_table_sse2(FunctionTable& ft) noexcept;
#endif // BL_BUILD_OPT_SSE2

#if defined(BL_BUILD_OPT_AVX)
void init_func_table_avx(FunctionTable& ft) noexcept;
#endif // BL_BUILD_OPT_AVX

#if defined(BL_BUILD_OPT_ASIMD)
void init_func_table_asimd(FunctionTable& ft) noexcept;
#endif // BL_BUILD_OPT_ASIMD

} // {bl::Png::Ops}

//! \}
//! \endcond

#endif // BLEND2D_CODEC_PNGOPS_P_H_INCLUDED
