// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SIMD_SIMDBASE_P_H_INCLUDED
#define BLEND2D_SIMD_SIMDBASE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_simd
//! \{

namespace SIMD {

// SIMD - Register Types
// =====================

template<size_t W, typename T>
struct Vec;

template<typename V, size_t N>
struct VecArray {
  typedef typename V::ElementType ElementType;

  static inline constexpr uint32_t kW = V::kW;
  static inline constexpr uint32_t kN = N;

  V data[N];

  BL_INLINE_NODEBUG V& operator[](size_t index) noexcept { return data[index]; }
  BL_INLINE_NODEBUG const V& operator[](size_t index) const noexcept { return data[index]; }
};

template<typename V>
using VecPair = VecArray<V, 2>;

// SIMD - Immediate Values
// =======================

template<uint32_t>
struct Shift {};

// SIMD - Internal - Scalar Packing
// ================================

namespace {
namespace Internal {

BL_INLINE_NODEBUG uint16_t scalar_u16_from_2x_u8(uint8_t hi, uint8_t lo) noexcept {
  return uint16_t((uint16_t(hi) << 8) | uint16_t(lo));
}

BL_INLINE_NODEBUG uint32_t scalar_u32_from_2x_u16(uint16_t hi, uint16_t lo) noexcept {
  return uint32_t((uint32_t(hi) << 16) | uint32_t(lo));
}

BL_INLINE_NODEBUG uint32_t scalar_u32_from_4x_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return scalar_u32_from_2x_u16(scalar_u16_from_2x_u8(x3, x2), scalar_u16_from_2x_u8(x1, x0));
}

BL_INLINE_NODEBUG uint64_t scalar_u64_from_2x_u32(uint32_t hi, uint32_t lo) noexcept {
  return (uint64_t(hi) << 32) | uint64_t(lo);
}

BL_INLINE_NODEBUG uint64_t scalar_u64_from_4x_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return scalar_u64_from_2x_u32(scalar_u32_from_2x_u16(x3, x2),
                                scalar_u32_from_2x_u16(x1, x0));
}

BL_INLINE_NODEBUG uint64_t scalar_u64_from_8x_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4, uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {

  return scalar_u64_from_2x_u32(scalar_u32_from_4x_u8(x7, x6, x5, x4), scalar_u32_from_4x_u8(x3, x2, x1, x0));
}

} // {Internal}
} // {anonymous}

// SIMD - Loop Construction
// ========================

//! Define a blit that processes 4 (32-bit) pixels at a time in main loop.
#define BL_SIMD_LOOP_32x4_INIT()                                              \
  size_t mini_loop_cnt;                                                         \
  size_t main_loop_cnt;

#define BL_SIMD_LOOP_32x4_MINI_BEGIN(LOOP, DST, COUNT)                        \
  mini_loop_cnt = bl_min(size_t((uintptr_t(0) - ((uintptr_t)(DST) / 4)) & 0x3),  \
                      size_t(COUNT));                                         \
  main_loop_cnt = size_t(COUNT) - mini_loop_cnt;                                  \
  if (!mini_loop_cnt) goto On##LOOP##_MiniSkip;                                 \
                                                                              \
On##LOOP##_MiniBegin:                                                         \
  do {

#define BL_SIMD_LOOP_32x4_MINI_END(LOOP)                                      \
  } while (--mini_loop_cnt);                                                    \
                                                                              \
On##LOOP##_MiniSkip:                                                          \
  mini_loop_cnt = main_loop_cnt & 3;                                              \
  main_loop_cnt /= 4;                                                           \
  if (!main_loop_cnt) goto On##LOOP##_MainSkip;

#define BL_SIMD_LOOP_32x4_MAIN_BEGIN(LOOP)                                    \
  do {

#define BL_SIMD_LOOP_32x4_MAIN_END(LOOP)                                      \
  } while (--main_loop_cnt);                                                    \
                                                                              \
On##LOOP##_MainSkip:                                                          \
  if (mini_loop_cnt) goto On##LOOP##_MiniBegin;

} // {SIMD}

//! \}
//! \endcond

#endif // BLEND2D_SIMD_SIMDBASE_P_H_INCLUDED
