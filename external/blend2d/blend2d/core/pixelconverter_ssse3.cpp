// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#ifdef BL_TARGET_OPT_SSSE3

#include <blend2d/core/pixelconverter_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/memops_p.h>

// PixelConverter - Copy|Shufb (SSSE3)
// ===================================

BLResult bl_convert_copy_shufb_8888_ssse3(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::ShufbData& d = bl_pixel_converter_get_data(self)->shufb_data;
  Vec16xU8 fill_mask = make128_u32<Vec16xU8>(d.fill_mask);
  Vec16xU8 predicate = loadu<Vec16xU8>(d.shufb_predicate);

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 16) {
      Vec16xU8 p0 = loadu<Vec16xU8>(src_data +  0);
      Vec16xU8 p1 = loadu<Vec16xU8>(src_data + 16);
      Vec16xU8 p2 = loadu<Vec16xU8>(src_data + 32);
      Vec16xU8 p3 = loadu<Vec16xU8>(src_data + 48);

      storeu(dst_data +  0, swizzlev_u8(p0, predicate) | fill_mask);
      storeu(dst_data + 16, swizzlev_u8(p1, predicate) | fill_mask);
      storeu(dst_data + 32, swizzlev_u8(p2, predicate) | fill_mask);
      storeu(dst_data + 48, swizzlev_u8(p3, predicate) | fill_mask);

      dst_data += 64;
      src_data += 64;
      i -= 16;
    }

    BL_NOUNROLL
    while (i >= 4) {
      Vec16xU8 p0 = loadu<Vec16xU8>(src_data);
      storeu(dst_data, swizzlev_u8(p0, predicate) | fill_mask);

      dst_data += 16;
      src_data += 16;
      i -= 4;
    }

    BL_NOUNROLL
    while (i) {
      Vec16xU8 p0 = loadu_32<Vec16xU8>(src_data);
      storeu_32(dst_data, swizzlev_u8(p0, predicate) | fill_mask);

      dst_data += 4;
      src_data += 4;
      i--;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// PixelConverter - RGB32 <- RGB24 (SSSE3)
// =======================================

BLResult bl_convert_rgb32_from_rgb24_shufb_ssse3(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 3;

  const BLPixelConverterData::ShufbData& d = bl_pixel_converter_get_data(self)->shufb_data;
  Vec16xU8 fill_mask = make128_u32<Vec16xU8>(d.fill_mask);
  Vec16xU8 predicate = loadu<Vec16xU8>(d.shufb_predicate);

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 16) {
      Vec16xU8 p0 = loadu<Vec16xU8>(src_data +  0);      // [x5|z4 y4 x4|z3 y3 x3 z2|y2 x2 z1 y1|x1 z0 y0 x0]
      Vec16xU8 p1 = loadu<Vec16xU8>(src_data + 16);      // [yA|xA|z9 y9|x9 z8 y8 x8|z7 y7 x7 z6|y6 x6 z5 y5]
      Vec16xU8 p2;
      Vec16xU8 p3 = loadu<Vec16xU8>(src_data + 32);      // [zF yF xF zE|yE xE zD yD|xD zC yC xC|zB yB xB zA]

      p2 = alignr_u128<8>(p3, p1);                       // [-- -- -- --|zB yB xB zA|yA|xA|z9 y9|x9 z8 y8 x8]
      p1 = alignr_u128<12>(p1, p0);                      // [-- -- -- --|z7 y7 x7 z6|y6 x6 z5 y5|x5|z4 y4 x4]
      p3 = srlb_u128<4>(p3);                             // [-- -- -- --|zF yF xF zE|yE xE zD yD|xD zC yC xC]

      storeu(dst_data +  0, swizzlev_u8(p0, predicate) | fill_mask);
      storeu(dst_data + 16, swizzlev_u8(p1, predicate) | fill_mask);
      storeu(dst_data + 32, swizzlev_u8(p2, predicate) | fill_mask);
      storeu(dst_data + 48, swizzlev_u8(p3, predicate) | fill_mask);

      dst_data += 64;
      src_data += 48;
      i -= 16;
    }

    if (i >= 8) {
      Vec16xU8 p0 = loadu<Vec16xU8>(src_data);           // [x5|z4 y4 x4|z3 y3 x3 z2|y2 x2 z1 y1|x1 z0 y0 x0]
      Vec16xU8 p1 = loadu_64<Vec16xU8>(src_data + 16);   // [-- -- -- --|-- -- -- --|z7 y7 x7 z6|y6 x6 z5 y5]
      p1 = alignr_u128<12>(p1, p0);                      // [-- -- -- --|z7 y7 x7 z6|y6 x6 z5 y5|x5|z4 y4 x4]

      storeu(dst_data +  0, swizzlev_u8(p0, predicate) | fill_mask);
      storeu(dst_data + 16, swizzlev_u8(p1, predicate) | fill_mask);

      dst_data += 32;
      src_data += 24;
      i -= 8;
    }

    if (i >= 4) {
      Vec16xU8 p0 = loadu_64<Vec16xU8>(src_data +  0);   // [-- -- -- --|-- -- -- --|y2 x2 z1 y1|x1 z0 y0 x0]
      Vec16xU8 p1 = loadu_32<Vec16xU8>(src_data +  8);   // [-- -- -- --|-- -- -- --|-- -- -- --|z3 y3 x3 z2]
      p0 = interleave_lo_u64(p0, p1);                    // [-- -- -- --|z3 y3 x3 z2|y2 x2 z1 y1|x1 z0 y0 x0]

      storeu(dst_data, swizzlev_u8(p0, predicate) | fill_mask);

      dst_data += 16;
      src_data += 12;
      i -= 4;
    }

    BL_NOUNROLL
    while (i) {
      uint32_t yx = bl::MemOps::readU16u(src_data + 0);
      uint32_t z  = bl::MemOps::readU8(src_data + 2);
      Vec16xU8 p0 = cast_from_u32<Vec16xU8>((z << 16) | yx);

      storeu_32(dst_data, swizzlev_u8(p0, predicate) | fill_mask);

      dst_data += 4;
      src_data += 3;
      i--;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Premultiply (SSSE3)
// ========================================

template<uint32_t A_Shift>
static BL_INLINE BLResult bl_convert_premultiply_8888_shufb_template_ssse3(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4u + gap;
  src_stride -= uintptr_t(w) * 4u;

  const BLPixelConverterData::ShufbData& d = bl_pixel_converter_get_data(self)->shufb_data;
  Vec8xU16 a255 = make128_u64<Vec8xU16>(uint64_t(0xFFu) << (A_Shift * 2));
  Vec16xU8 fill_mask = make128_u32<Vec16xU8>(d.fill_mask);
  Vec16xU8 predicate = loadu<Vec16xU8>(d.shufb_predicate);

  // Alpha byte-index that can be used by instructions that perform shuffling.
  constexpr uint32_t AI = A_Shift / 8u;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 4) {
      Vec16xU8 packed = swizzlev_u8(loadu<Vec16xU8>(src_data), predicate);
      Vec8xU16 p1 = vec_u16(unpack_hi64_u8_u16(packed));
      Vec8xU16 p0 = vec_u16(unpack_lo64_u8_u16(packed));

      p0 = div255_u16((p0 | a255) * swizzle_u16<AI, AI, AI, AI>(p0));
      p1 = div255_u16((p1 | a255) * swizzle_u16<AI, AI, AI, AI>(p1));

      packed = vec_u8(packs_128_i16_u8(p0, p1));
      storeu(dst_data, packed | fill_mask);

      dst_data += 16;
      src_data += 16;
      i -= 4;
    }

    BL_NOUNROLL
    while (i) {
      Vec16xU8 packed = swizzlev_u8(loadu_32<Vec16xU8>(src_data), predicate);
      Vec8xU16 p0 = vec_u16(unpack_lo64_u8_u16(packed));

      p0 = div255_u16((p0 | a255) * swizzle_lo_u16<AI, AI, AI, AI>(p0));

      packed = vec_u8(packs_128_i16_u8(p0, p0));
      storeu_32(dst_data, packed | fill_mask);

      dst_data += 4;
      src_data += 4;
      i--;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

BLResult bl_convert_premultiply_8888_leading_alpha_shufb_ssse3(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_shufb_template_ssse3<24>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_premultiply_8888_trailing_alpha_shufb_ssse3(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_shufb_template_ssse3<0>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}
#endif
