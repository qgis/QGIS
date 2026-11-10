// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIXELOPS_SCALAR_P_H_INCLUDED
#define BLEND2D_PIXELOPS_SCALAR_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/rgba_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/tables/tables_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace PixelOps {
namespace Scalar {

//! \name Scalar Pixel Utilities
//! \{

[[nodiscard]]
static BL_INLINE uint32_t neg255(uint32_t x) noexcept { return x ^ 0xFF; }

//! Integer division by 255 with correct rounding semantics.
//!
//! Possible implementations:
//!   - `((x + 128) + ((x + 128) >> 8)) >> 8` (used by scalar operations and AVX+ impls).
//!   - `((x + 128) * 257) >> 16` (used by SSE2 to SSE4.1 impl, but not by AVX).
[[nodiscard]]
static BL_INLINE uint32_t udiv255(uint32_t x) noexcept { return ((x + 0x80u) * 0x101u) >> 16; }

[[nodiscard]]
static BL_INLINE uint32_t udiv65535(uint32_t x) noexcept { return ((x + 0x8000u) + ((x + 0x8000u) >> 16)) >> 16; }

[[nodiscard]]
static BL_INLINE uint64_t udiv65535_packed(uint64_t x) noexcept {
  constexpr uint64_t kHalfPacked = 0x0000800000008000u;
  constexpr uint64_t kMaskPacked = 0x0000FFFF0000FFFFu;

  x += kHalfPacked;
  return ((x + ((x >> 16) & kMaskPacked)) >> 16) & kMaskPacked;
}

static BL_INLINE void unpremultiply_rgb_8bit(uint32_t& r, uint32_t& g, uint32_t& b, uint32_t a) noexcept {
  uint32_t recip = common_table.unpremultiply_rcp[a];
  r = (r * recip + 0x8000u) >> 16;
  g = (g * recip + 0x8000u) >> 16;
  b = (b * recip + 0x8000u) >> 16;
}

//! \}

//! \name Scalar Pixel Conversion
//! \{

static BL_INLINE uint32_t cvt_xrgb32_0888_from_xrgb16_0555(uint32_t src) noexcept {
  uint32_t t0 = src;       // [00000000] [00000000] [XRRRRRGG] [GGGBBBBB]
  uint32_t t1;
  uint32_t t2;

  t0 = (t0 * 0x00080008u); // [RRRGGGGG] [BBBBBXRR] [RRRGGGGG] [BBBBB000]
  t0 = (t0 & 0x1F03E0F8u); // [000GGGGG] [000000RR] [RRR00000] [BBBBB000]
  t0 = (t0 | (t0 >> 5));   // [000GGGGG] [GGGGG0RR] [RRRRRRRR] [BBBBBBBB]

  t1 = t0 >> 13;           // [00000000] [00000000] [GGGGGGGG] [GG0RRRRR]
  t2 = t0 << 6;            // [GGGGGGG0] [RRRRRRRR] [RRBBBBBB] [BB000000]

  t0 &= 0x000000FFu;       // [00000000] [00000000] [00000000] [BBBBBBBB]
  t1 &= 0x0000FF00u;       // [00000000] [00000000] [GGGGGGGG] [00000000]
  t2 &= 0x00FF0000u;       // [00000000] [RRRRRRRR] [00000000] [00000000]

  return 0xFF000000u | t0 | t1 | t2;
}

static BL_INLINE uint32_t cvt_xrgb32_0888_from_xrgb16_0565(uint32_t src) noexcept {
  uint32_t t0 = src;       // [00000000] [00000000] [RRRRRGGG] [GGGBBBBB]
  uint32_t t1 = src;
  uint32_t t2;

  t0 = (t0 & 0x0000F81Fu); // [00000000] [00000000] [RRRRR000] [000BBBBB]
  t1 = (t1 & 0x000007E0u); // [00000000] [00000000] [00000GGG] [GGG00000]

  t0 = (t0 * 0x21u);       // [00000000] [000RRRRR] [RRRRR0BB] [BBBBBBBB]
  t1 = (t1 * 0x41u);       // [00000000] [0000000G] [GGGGGGGG] [GGG00000]

  t2 = (t0 << 3);          // [00000000] [RRRRRRRR] [RR0BBBBB] [BBBBB000]
  t0 = (t0 >> 2);          // [00000000] [00000RRR] [RRRRRRR0] [BBBBBBBB]
  t1 = (t1 >> 1);          // [00000000] [00000000] [GGGGGGGG] [GGGG0000]

  t0 = t0 & 0x000000FFu;   // [00000000] [00000000] [00000000] [BBBBBBBB]
  t1 = t1 & 0x0000FF00u;   // [00000000] [00000000] [GGGGGGGG] [00000000]
  t2 = t2 & 0x00FF0000u;   // [00000000] [RRRRRRRR] [00000000] [00000000]

  return 0xFF000000u | t0 | t1 | t2;
}

static BL_INLINE uint32_t cvt_argb32_8888_from_argb16_4444(uint32_t src) noexcept {
  uint32_t t0 = src;       // [00000000] [00000000] [AAAARRRR] [GGGGBBBB]
  uint32_t t1;
  uint32_t t2;

  t1 = t0 << 12;           // [0000AAAA] [RRRRGGGG] [BBBB0000] [00000000]
  t2 = t0 << 4;            // [00000000] [0000AAAA] [RRRRGGGG] [BBBB0000]

  t0 = t0 | t1;            // [0000AAAA] [RRRRGGGG] [XXXXRRRR] [GGGGBBBB]
  t1 = t2 << 4;            // [00000000] [AAAARRRR] [GGGGBBBB] [00000000]

  t0 &= 0x0F00000Fu;       // [0000AAAA] [00000000] [00000000] [0000BBBB]
  t1 &= 0x000F0000u;       // [00000000] [0000RRRR] [00000000] [00000000]
  t2 &= 0x00000F00u;       // [00000000] [00000000] [0000GGGG] [00000000]

  t0 += t1;                // [0000AAAA] [0000RRRR] [00000000] [0000BBBB]
  t0 += t2;                // [0000AAAA] [0000RRRR] [0000GGGG] [0000BBBB]

  return t0 * 0x11u;       // [AAAAAAAA] [RRRRRRRR] [GGGGGGGG] [BBBBBBBB]
}

static BL_INLINE uint32_t cvt_prgb32_8888_from_argb32_8888(uint32_t val32, uint32_t _a) noexcept {
#if BL_TARGET_SIMD_I
  using namespace SIMD;

  Vec8xU16 p0 = unpack_lo64_u8_u16(cast_from_u32<Vec8xU16>(val32 | 0xFF000000u));
  Vec8xU16 a0 = swizzle_lo_u16<3, 3, 3, 3>(cast_from_u32<Vec8xU16>(_a));

  return cast_to_u32(packz_128_u16_u8(div255_u16(p0 * a0)));
#else
  val32 |= 0xFF000000u;

  uint32_t rb = (val32     ) & 0x00FF00FFu;
  uint32_t ag = (val32 >> 8) & 0x00FF00FFu;

  rb = (rb * _a) + 0x00800080u;
  ag = (ag * _a) + 0x00800080u;

  rb = (rb + ((rb >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;
  ag = (ag + ((ag >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;

  return ag | (rb >> 8);
#endif
}

static BL_INLINE uint32_t cvt_prgb32_8888_from_argb32_8888(uint32_t val32) noexcept {
#if BL_TARGET_SIMD_I
  using namespace SIMD;

  Vec8xU16 p0 = unpack_lo64_u8_u16(cast_from_u32<Vec8xU16>(val32));
  Vec8xU16 a0 = swizzle_lo_u16<3, 3, 3, 3>(p0);

  return cast_to_u32(packz_128_u16_u8(div255_u16((p0 | common_table.p_00FF000000000000.as<Vec8xU16>()) * a0)));
#else
  return cvt_prgb32_8888_from_argb32_8888(val32, val32 >> 24);
#endif
}

static BL_INLINE uint32_t cvt_argb32_8888_from_prgb32_8888(uint32_t val32) noexcept {
  uint32_t r = ((val32 >> 16) & 0xFFu);
  uint32_t g = ((val32 >>  8) & 0xFFu);
  uint32_t b = ((val32      ) & 0xFFu);
  uint32_t a = ((val32 >> 24));

  unpremultiply_rgb_8bit(r, g, b, a);
  return (a << 24) | (r << 16) | (g << 8) | b;
}

static BL_INLINE uint32_t cvt_abgr32_8888_from_prgb32_8888(uint32_t val32) noexcept {
  uint32_t r = ((val32 >> 16) & 0xFFu);
  uint32_t g = ((val32 >>  8) & 0xFFu);
  uint32_t b = ((val32      ) & 0xFFu);
  uint32_t a = ((val32 >> 24));

  unpremultiply_rgb_8bit(r, g, b, a);
  return (a << 24) | (b << 16) | (g << 8) | r;
}

static BL_INLINE uint64_t cvt_prgb64_8888_from_argb64_8888(uint64_t val64, uint32_t _a) noexcept {
#if BL_TARGET_ARCH_X86
  using namespace SIMD;

  Vec8xU16 v0 = cast_from_u64<Vec8xU16>(val64 | 0xFFFF000000000000u);
  Vec8xU16 a0 = swizzle_u16<3, 3, 3, 3>(cast_from_u32<Vec8xU16>(_a));

  Vec8xU16 v_lo = mul_u16(v0, a0);
  Vec8xU16 v_hi = mulh_u16(v0, a0);
  Vec4xU32 p0 = div65535_u32(vec_u32(interleave_lo_u16(v_lo, v_hi)));

  return cast_to_u64(packz_128_u32_u16(p0));
#else
  if constexpr (BL_TARGET_ARCH_BITS >= 64) {
    uint64_t rb = udiv65535_packed((val64 & 0x0000FFFF0000FFFFu) * _a);
    uint32_t g = udiv65535(uint32_t((val64 >> 16) & 0xFFFFu) * _a);
    return (uint64_t(_a) << 48) | rb | (g << 16);
  }
  else {
    uint32_t r = udiv65535(uint32_t((val64 >> 32) & 0xFFFFu) * _a);
    uint32_t g = udiv65535(uint32_t((val64 >> 16) & 0xFFFFu) * _a);
    uint32_t b = udiv65535(uint32_t((val64      ) & 0xFFFFu) * _a);
    return (uint64_t(_a) << 48) | (uint64_t(r) << 32) | (g << 16) | b;
  }
#endif
}

static BL_INLINE uint64_t cvt_prgb64_8888_from_argb64_8888(uint64_t val64) noexcept {
#if BL_TARGET_ARCH_X86
  using namespace SIMD;

  Vec8xU16 v0 = cast_from_u64<Vec8xU16>(val64);
  Vec8xU16 a0 = swizzle_u16<3, 3, 3, 3>(v0);
  v0 |= common_table.p_FFFF000000000000.as<Vec8xU16>();

  Vec8xU16 v_lo = mul_u16(v0, a0);
  Vec8xU16 v_hi = mulh_u16(v0, a0);
  Vec4xU32 p0 = div65535_u32(vec_u32(interleave_lo_u16(v_lo, v_hi)));

  return cast_to_u64(packz_128_u32_u16(p0));
#else
  return cvt_prgb64_8888_from_argb64_8888(val64, uint32_t(val64 >> 48));
#endif
}

//! \}

} // {Scalar}
} // {PixelOps}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_PIXELOPS_SCALAR_P_H_INCLUDED
