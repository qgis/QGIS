// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#ifdef BL_TARGET_OPT_AVX2

#include <blend2d/core/pixelconverter_p.h>
#include <blend2d/simd/simd_p.h>

// PixelConverter - Copy (AVX2)
// ============================

BLResult bl_convert_copy_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  const size_t bytes_per_pixel = bl_pixel_converter_get_data(self)->mem_copy_data.bytes_per_pixel;
  const size_t byte_width = size_t(w) * bytes_per_pixel;

  // Use a generic copy if `byte_width` is small as we would not be able to
  // utilize SIMD properly - in general we want to use at least 16-byte RW.
  if (byte_width < 16)
    return bl_convert_copy(self, dst_data, dst_stride, src_data, src_stride, w, h, options);

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= intptr_t(byte_width + gap);
  src_stride -= intptr_t(byte_width);

  for (uint32_t y = h; y != 0; y--) {
    size_t i = byte_width;

    BL_NOUNROLL
    while (i >= 64) {
      Vec32xU8 p0 = loadu<Vec32xU8>(src_data +  0);
      Vec32xU8 p1 = loadu<Vec32xU8>(src_data + 32);

      storeu(dst_data +  0, p0);
      storeu(dst_data + 32, p1);

      dst_data += 64;
      src_data += 64;
      i -= 64;
    }

    BL_NOUNROLL
    while (i >= 16) {
      storeu(dst_data, loadu<Vec16xU8>(src_data));

      dst_data += 16;
      src_data += 16;
      i -= 16;
    }

    if (i) {
      dst_data += i;
      src_data += i;
      storeu(dst_data - 16, loadu<Vec16xU8>(src_data - 16));
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// PixelConverter - Copy|Or (AVX2)
// ===============================

BLResult bl_convert_copy_or_8888_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  if (!options) {
    options = &bl_pixel_converter_default_options;
  }

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 4;

  Vec32xU8 fill_mask = make256_u32<Vec32xU8>(bl_pixel_converter_get_data(self)->mem_copy_data.fill_mask);
  Vec32xU8 load_store_mask = loada_64_i8_i32<Vec32xU8>(bl::common_table.loadstore16_lo8_msk8() + (w & 7u));

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 32) {
      Vec32xU8 p0 = fill_mask | loadu<Vec32xU8>(src_data +  0);
      Vec32xU8 p1 = fill_mask | loadu<Vec32xU8>(src_data + 32);
      Vec32xU8 p2 = fill_mask | loadu<Vec32xU8>(src_data + 64);
      Vec32xU8 p3 = fill_mask | loadu<Vec32xU8>(src_data + 96);

      storeu(dst_data +  0, p0);
      storeu(dst_data + 32, p1);
      storeu(dst_data + 64, p2);
      storeu(dst_data + 96, p3);

      dst_data += 128;
      src_data += 128;
      i -= 32;
    }

    BL_NOUNROLL
    while (i >= 8) {
      Vec32xU8 p0 = fill_mask | loadu<Vec32xU8>(src_data);
      storeu(dst_data, p0);

      dst_data += 32;
      src_data += 32;
      i -= 8;
    }

    if (i) {
      Vec32xU8 p0 = fill_mask | loadu_256_mask32<Vec32xU8>(src_data, load_store_mask);
      storeu_256_mask32(dst_data, p0, load_store_mask);

      dst_data += i * 4;
      src_data += i * 4;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// PixelConverter - Copy|Shufb (AVX2)
// ==================================

BLResult bl_convert_copy_shufb_8888_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  if (!options) {
    options = &bl_pixel_converter_default_options;
  }

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::ShufbData& d = bl_pixel_converter_get_data(self)->shufb_data;

  Vec32xU8 fill_mask = make256_u32<Vec32xU8>(bl_pixel_converter_get_data(self)->mem_copy_data.fill_mask);
  Vec32xU8 predicate = broadcast_i128<Vec32xU8>(loadu<Vec16xU8>(d.shufb_predicate));
  Vec32xU8 load_store_mask = loada_64_i8_i32<Vec32xU8>(bl::common_table.loadstore16_lo8_msk8() + (w & 7u));

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 32) {
      Vec32xU8 p0 = loadu<Vec32xU8>(src_data +  0);
      Vec32xU8 p1 = loadu<Vec32xU8>(src_data + 32);
      Vec32xU8 p2 = loadu<Vec32xU8>(src_data + 64);
      Vec32xU8 p3 = loadu<Vec32xU8>(src_data + 96);

      storeu(dst_data +  0, swizzlev_u8(p0, predicate) | fill_mask);
      storeu(dst_data + 32, swizzlev_u8(p1, predicate) | fill_mask);
      storeu(dst_data + 64, swizzlev_u8(p2, predicate) | fill_mask);
      storeu(dst_data + 96, swizzlev_u8(p3, predicate) | fill_mask);

      dst_data += 128;
      src_data += 128;
      i -= 32;
    }

    BL_NOUNROLL
    while (i >= 8) {
      Vec32xU8 p0 = loadu<Vec32xU8>(src_data);
      storeu(dst_data, swizzlev_u8(p0, predicate) | fill_mask);

      dst_data += 32;
      src_data += 32;
      i -= 8;
    }

    if (i) {
      Vec32xU8 p0 = loadu_256_mask32<Vec32xU8>(src_data, load_store_mask);
      storeu_256_mask32(dst_data, swizzlev_u8(p0, predicate) | fill_mask, load_store_mask);

      dst_data += i * 4;
      src_data += i * 4;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// PixelConverter - RGB32 <- RGB24 (AVX2)
// ======================================

BLResult bl_convert_rgb32_from_rgb24_shufb_avx2(
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

  Vec32xU8 fill_mask = make256_u32<Vec32xU8>(bl_pixel_converter_get_data(self)->mem_copy_data.fill_mask);
  Vec32xU8 predicate = broadcast_i128<Vec32xU8>(loadu<Vec16xU8>(d.shufb_predicate));
  Vec16xU8 load_store_mask = loada_32_i8_i32<Vec16xU8>(bl::common_table.loadstore16_lo8_msk8() + (w & 3u));

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 32) {
      Vec32xU8 p0, p1, p2, p3;
      Vec32xU8 q0, q1, q2, q3;

      p0 = loadu_128<Vec32xU8>(src_data +  0);          // [x5|z4 y4 x4|z3 y3 x3 z2|y2 x2 z1 y1|x1 z0 y0 x0]
      p1 = loadu_128<Vec32xU8>(src_data + 16);          // [yA|xA|z9 y9|x9 z8 y8 x8|z7 y7 x7 z6|y6 x6 z5 y5]
      p3 = loadu_128<Vec32xU8>(src_data + 32);          // [zF yF xF zE|yE xE zD yD|xD zC yC xC|zB yB xB zA]

      p2 = alignr_u128<8>(p3, p1);                      // [-- -- -- --|zB yB xB zA|yA|xA|z9 y9|x9 z8 y8 x8]
      p1 = alignr_u128<12>(p1, p0);                     // [-- -- -- --|z7 y7 x7 z6|y6 x6 z5 y5|x5|z4 y4 x4]
      p3 = srlb_u128<4>(p3);                            // [-- -- -- --|zF yF xF zE|yE xE zD yD|xD zC yC xC]

      p0 = interleave_i128(p0, p1);
      p2 = interleave_i128(p2, p3);

      q0 = loadu_128<Vec32xU8>(src_data + 48);          // [x5|z4 y4 x4|z3 y3 x3 z2|y2 x2 z1 y1|x1 z0 y0 x0]
      q1 = loadu_128<Vec32xU8>(src_data + 64);          // [yA|xA|z9 y9|x9 z8 y8 x8|z7 y7 x7 z6|y6 x6 z5 y5]
      q3 = loadu_128<Vec32xU8>(src_data + 80);          // [zF yF xF zE|yE xE zD yD|xD zC yC xC|zB yB xB zA]

      q2 = alignr_u128<8>(q3, q1);                      // [-- -- -- --|zB yB xB zA|yA|xA|z9 y9|x9 z8 y8 x8]
      q1 = alignr_u128<12>(q1, q0);                     // [-- -- -- --|z7 y7 x7 z6|y6 x6 z5 y5|x5|z4 y4 x4]
      q3 = srlb_u128<4>(q3);                            // [-- -- -- --|zF yF xF zE|yE xE zD yD|xD zC yC xC]

      q0 = interleave_i128(q0, q1);
      q2 = interleave_i128(q2, q3);

      storeu(dst_data +  0, swizzlev_u8(p0, predicate) | fill_mask);
      storeu(dst_data + 32, swizzlev_u8(p2, predicate) | fill_mask);
      storeu(dst_data + 64, swizzlev_u8(q0, predicate) | fill_mask);
      storeu(dst_data + 96, swizzlev_u8(q2, predicate) | fill_mask);

      dst_data += 128;
      src_data += 96;
      i -= 32;
    }

    BL_NOUNROLL
    while (i >= 8) {
      Vec16xU8 p0, p1;

      p0 = loadu<Vec16xU8>(src_data);                   // [x5|z4 y4 x4|z3 y3 x3 z2|y2 x2 z1 y1|x1 z0 y0 x0]
      p1 = loadu_64<Vec16xU8>(src_data + 16);           // [-- -- -- --|-- -- -- --|z7 y7 x7 z6|y6 x6 z5 y5]
      p1 = alignr_u128<12>(p1, p0);                     // [-- -- -- --|z7 y7 x7 z6|y6 x6 z5 y5|x5|z4 y4 x4]

      storeu(dst_data +  0, swizzlev_u8(p0, vec_128(predicate)) | vec_128(fill_mask));
      storeu(dst_data + 16, swizzlev_u8(p1, vec_128(predicate)) | vec_128(fill_mask));

      dst_data += 32;
      src_data += 24;
      i -= 8;
    }

    if (i >= 4) {
      Vec16xU8 p0;

      p0 = loadu_64<Vec16xU8>(src_data);                // [-- -- -- --|-- -- -- --|y2 x2 z1 y1|x1 z0 y0 x0]
      p0 = insert_m32<2>(p0, src_data + 8);             // [-- -- -- --|z3 y3 x3 z2|y2 x2 z1 y1|x1 z0 y0 x0]

      storeu(dst_data, swizzlev_u8(p0, vec_128(predicate)) | vec_128(fill_mask));

      dst_data += 16;
      src_data += 12;
      i -= 4;
    }

    if (i) {
      Vec16xU8 p0 = make_zero<Vec16xU8>();
      p0 = insert_m24<0>(p0, src_data + 0);             // [-- -- -- --|-- -- -- --|-- -- -- --|-- z0 y0 x0]
      if (i >= 2) {
        p0 = insert_m24<3>(p0, src_data + 3);           // [-- -- -- --|-- -- -- --|-- -- z1 y1|x1 z0 y0 x0]
        if (i >= 3) {
          p0 = insert_m24<6>(p0, src_data + 6);         // [-- -- -- --|-- -- -- z2|y2 x2 z1 y1|x1 z0 y0 x0]
        }
      }

      storeu_128_mask32(dst_data, swizzlev_u8(p0, vec_128(predicate)) | vec_128(fill_mask), load_store_mask);

      dst_data += i * 4;
      src_data += i * 3;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// PixelConverter - Premultiply (AVX2)
// ===================================

template<uint32_t A_Shift, bool UseShufB>
static BL_INLINE BLResult bl_convert_premultiply_8888_template_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::PremultiplyData& d = bl_pixel_converter_get_data(self)->premultiply_data;

  Vec32xU8 zero = make_zero<Vec32xU8>();
  Vec32xU8 fill_mask = make256_u32<Vec32xU8>(d.fill_mask);
  Vec16xU16 alpha_mask = make256_u64<Vec16xU16>(uint64_t(0xFFu) << (A_Shift * 2));

  Vec32xU8 predicate;
  if (UseShufB)
    predicate = broadcast_i128<Vec32xU8>(loadu<Vec16xU8>(d.shufb_predicate));

  Vec32xU8 load_store_mask_lo = loada_64_i8_i32<Vec32xU8>(&bl::common_table.loadstore16_lo8_msk8()[w & 15]);
  Vec32xU8 load_store_mask_hi = loada_64_i8_i32<Vec32xU8>(&bl::common_table.loadstore16_hi8_msk8()[w & 15]);

  // Alpha byte-index that can be used by instructions that perform shuffling.
  constexpr uint32_t AI = A_Shift / 8u;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 16) {
      Vec32xU8 packed0 = loadu<Vec32xU8>(src_data +  0);
      Vec32xU8 packed1 = loadu<Vec32xU8>(src_data + 32);

      if (UseShufB) {
        packed0 = swizzlev_u8(packed0, predicate);
        packed1 = swizzlev_u8(packed1, predicate);
      }

      Vec16xU16 p1 = vec_u16(interleave_hi_u8(packed0, zero));
      Vec16xU16 p0 = vec_u16(interleave_lo_u8(packed0, zero));
      Vec16xU16 p3 = vec_u16(interleave_hi_u8(packed1, zero));
      Vec16xU16 p2 = vec_u16(interleave_lo_u8(packed1, zero));

      p0 = div255_u16((p0 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p0));
      p1 = div255_u16((p1 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p1));
      p2 = div255_u16((p2 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p2));
      p3 = div255_u16((p3 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p3));

      storeu(dst_data +  0, vec_u8(packs_128_i16_u8(p0, p1)) | fill_mask);
      storeu(dst_data + 32, vec_u8(packs_128_i16_u8(p2, p3)) | fill_mask);

      dst_data += 64;
      src_data += 64;
      i -= 16;
    }

    if (i) {
      Vec32xU8 packed0 = loadu_256_mask32<Vec32xU8>(src_data +  0, load_store_mask_lo);
      Vec32xU8 packed1 = loadu_256_mask32<Vec32xU8>(src_data + 32, load_store_mask_hi);

      if (UseShufB) {
        packed0 = swizzlev_u8(packed0, predicate);
        packed1 = swizzlev_u8(packed1, predicate);
      }

      Vec16xU16 p1 = vec_u16(interleave_hi_u8(packed0, zero));
      Vec16xU16 p0 = vec_u16(interleave_lo_u8(packed0, zero));
      Vec16xU16 p3 = vec_u16(interleave_hi_u8(packed1, zero));
      Vec16xU16 p2 = vec_u16(interleave_lo_u8(packed1, zero));

      p0 = div255_u16((p0 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p0));
      p1 = div255_u16((p1 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p1));
      p2 = div255_u16((p2 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p2));
      p3 = div255_u16((p3 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p3));

      storeu_256_mask32(dst_data +  0, vec_u8(packs_128_i16_u8(p0, p1)) | fill_mask, load_store_mask_lo);
      storeu_256_mask32(dst_data + 32, vec_u8(packs_128_i16_u8(p2, p3)) | fill_mask, load_store_mask_hi);

      dst_data += i * 4;
      src_data += i * 4;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

BLResult bl_convert_premultiply_8888_leading_alpha_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_template_avx2<24, false>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_premultiply_8888_trailing_alpha_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_template_avx2<0, false>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_premultiply_8888_leading_alpha_shufb_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_template_avx2<24, true>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_premultiply_8888_trailing_alpha_shufb_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_template_avx2<0, true>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

// PixelConverter - Unpremultiply (PMULLD) (AVX2)
// ==============================================

template<uint32_t A_Shift>
static BL_INLINE BLResult bl_convert_unpremultiply_8888_pmulld_template_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  bl_unused(self);

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4u + gap;
  src_stride -= uintptr_t(w) * 4u;

  const uint32_t* rcp_table = bl::common_table.unpremultiply_rcp;

  Vec8xU32 half = make256_u32(0x8000u);
  Vec32xU8 alpha_mask = make256_u32<Vec32xU8>(0xFFu << A_Shift);
  Vec8xU32 component_mask = make256_u32<Vec8xU32>(0xFFu);
  Vec32xU8 load_store_mask = loada_64_i8_i32<Vec32xU8>(bl::common_table.loadstore16_lo8_msk8() + (w & 7));

  // Alpha byte-index that can be used by instructions that perform shuffling.
  constexpr uint32_t AI = A_Shift / 8u;
  constexpr uint32_t RI = (AI + 1) % 4;
  constexpr uint32_t GI = (AI + 2) % 4;
  constexpr uint32_t BI = (AI + 3) % 4;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 8) {
      Vec32xU8 pix = loadu<Vec32xU8>(src_data);

      Vec4xU32 rcp_lo = loada_32<Vec4xU32>(rcp_table + src_data[0 * 4 + AI]);
      Vec4xU32 rcp_hi = loada_32<Vec4xU32>(rcp_table + src_data[4 * 4 + AI]);

      rcp_lo = insert_m32<1>(rcp_lo, rcp_table + src_data[1 * 4 + AI]);
      rcp_hi = insert_m32<1>(rcp_hi, rcp_table + src_data[5 * 4 + AI]);

      rcp_lo = insert_m32<2>(rcp_lo, rcp_table + src_data[2 * 4 + AI]);
      rcp_hi = insert_m32<2>(rcp_hi, rcp_table + src_data[6 * 4 + AI]);

      rcp_lo = insert_m32<3>(rcp_lo, rcp_table + src_data[3 * 4 + AI]);
      rcp_hi = insert_m32<3>(rcp_hi, rcp_table + src_data[7 * 4 + AI]);

      Vec8xU32 rcp = interleave_i128<Vec8xU32>(rcp_lo, rcp_hi);
      Vec8xU32 pr = vec_u32(srli_u32<RI * 8>(pix));
      Vec8xU32 pg = vec_u32(srli_u32<GI * 8>(pix));
      Vec8xU32 pb = vec_u32(srli_u32<BI * 8>(pix));

      if (RI != 3) pr = pr & component_mask;
      if (GI != 3) pg = pg & component_mask;
      if (BI != 3) pb = pb & component_mask;

      pix = pix & alpha_mask;
      pr = slli_i32<RI * 8>(srli_u32<16>(pr * rcp + half));
      pg = slli_i32<GI * 8>(srli_u32<16>(pg * rcp + half));
      pb = slli_i32<BI * 8>(srli_u32<16>(pb * rcp + half));
      pix = pix | vec_u8(pr) | vec_u8(pg) | vec_u8(pb);
      storeu(dst_data, pix);

      dst_data += 32;
      src_data += 32;
      i -= 8;
    }

    if (i) {
      Vec32xU8 pix = loadu_256_mask32<Vec32xU8>(src_data, load_store_mask);

      Vec4xU32 rcp_lo = loada_32<Vec4xU32>(rcp_table + src_data[0 * 4 + AI]);
      Vec4xU32 rcp_hi = loada_32<Vec4xU32>(rcp_table + src_data[4 * 4 + AI]);

      rcp_lo = insert_m32<1>(rcp_lo, rcp_table + src_data[1 * 4 + AI]);
      rcp_hi = insert_m32<1>(rcp_hi, rcp_table + src_data[5 * 4 + AI]);

      rcp_lo = insert_m32<2>(rcp_lo, rcp_table + src_data[2 * 4 + AI]);
      rcp_hi = insert_m32<2>(rcp_hi, rcp_table + src_data[6 * 4 + AI]);

      rcp_lo = insert_m32<3>(rcp_lo, rcp_table + src_data[3 * 4 + AI]);
      rcp_hi = insert_m32<3>(rcp_hi, rcp_table + src_data[7 * 4 + AI]);

      Vec8xU32 rcp = interleave_i128<Vec8xU32>(rcp_lo, rcp_hi);
      Vec8xU32 pr = vec_u32(srli_u32<RI * 8>(pix));
      Vec8xU32 pg = vec_u32(srli_u32<GI * 8>(pix));
      Vec8xU32 pb = vec_u32(srli_u32<BI * 8>(pix));

      if (RI != 3) pr = pr & component_mask;
      if (GI != 3) pg = pg & component_mask;
      if (BI != 3) pb = pb & component_mask;

      pix = pix & alpha_mask;
      pr = slli_i32<RI * 8>(srli_u32<16>(pr * rcp + half));
      pg = slli_i32<GI * 8>(srli_u32<16>(pg * rcp + half));
      pb = slli_i32<BI * 8>(srli_u32<16>(pb * rcp + half));
      pix = pix | vec_u8(pr) | vec_u8(pg) | vec_u8(pb);
      storeu_256_mask32(dst_data, pix, load_store_mask);

      dst_data += i * 4;
      src_data += i * 4;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

BLResult bl_convert_unpremultiply_8888_leading_alpha_pmulld_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_unpremultiply_8888_pmulld_template_avx2<24>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_unpremultiply_8888_trailing_alpha_pmulld_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_unpremultiply_8888_pmulld_template_avx2<0>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

// PixelConverter - Unpremultiply (FLOAT) (AVX2)
// =============================================

template<uint32_t A_Shift>
static BL_INLINE BLResult bl_convert_unpremultiply_8888_float_template_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  using namespace SIMD;

  bl_unused(self);

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4u + gap;
  src_stride -= uintptr_t(w) * 4u;

  Vec32xU8 alpha_mask = make256_u32<Vec32xU8>(0xFFu << A_Shift);
  Vec8xU32 component_mask = make256_u32(0xFFu);
  Vec32xU8 load_store_mask = loada_64_i8_i32<Vec32xU8>(bl::common_table.loadstore16_lo8_msk8() + (w & 7u));

  Vec8xF32 f32_255 = make256_f32(255.0001f);
  Vec8xF32 f32_lessThanOne = make256_f32(0.1f);

  // Alpha byte-index that can be used by instructions that perform shuffling.
  constexpr uint32_t AI = A_Shift / 8u;
  constexpr uint32_t RI = (AI + 1) % 4;
  constexpr uint32_t GI = (AI + 2) % 4;
  constexpr uint32_t BI = (AI + 3) % 4;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 8) {
      Vec32xU8 pix = loadu<Vec32xU8>(src_data);
      Vec8xU32 pa = vec_u32(srli_u32<AI * 8>(pix));

      if (AI != 3) pa = pa & component_mask;

      Vec8xF32 fa = cvt_i32_f32(pa);
      fa = f32_255 / max(fa, f32_lessThanOne);

      Vec8xU32 pr = vec_u32(srli_u32<RI * 8>(pix));
      Vec8xU32 pg = vec_u32(srli_u32<GI * 8>(pix));
      Vec8xU32 pb = vec_u32(srli_u32<BI * 8>(pix));

      if (RI != 3) pr = pr & component_mask;
      if (GI != 3) pg = pg & component_mask;
      if (BI != 3) pb = pb & component_mask;

      pr = vec_u32(cvt_f32_i32(cvt_i32_f32(pr) * fa));
      pg = vec_u32(cvt_f32_i32(cvt_i32_f32(pg) * fa));
      pb = vec_u32(cvt_f32_i32(cvt_i32_f32(pb) * fa));
      pix = pix & alpha_mask;

      pr = slli_i32<RI * 8>(pr);
      pg = slli_i32<GI * 8>(pg);
      pb = slli_i32<BI * 8>(pb);
      pix = pix | vec_u8(pr) | vec_u8(pg) | vec_u8(pb);

      storeu(dst_data, pix);

      dst_data += 32;
      src_data += 32;
      i -= 8;
    }

    if (i) {
      Vec32xU8 pix = loadu_256_mask32<Vec32xU8>(src_data, load_store_mask);
      Vec8xU32 pa = vec_u32(srli_u32<AI * 8>(pix));

      if (AI != 3) pa = pa & component_mask;

      Vec8xF32 fa = cvt_i32_f32(pa);
      fa = f32_255 / max(fa, f32_lessThanOne);

      Vec8xU32 pr = vec_u32(srli_u32<RI * 8>(pix));
      Vec8xU32 pg = vec_u32(srli_u32<GI * 8>(pix));
      Vec8xU32 pb = vec_u32(srli_u32<BI * 8>(pix));

      if (RI != 3) pr = pr & component_mask;
      if (GI != 3) pg = pg & component_mask;
      if (BI != 3) pb = pb & component_mask;

      pr = vec_u32(cvt_f32_i32(cvt_i32_f32(pr) * fa));
      pg = vec_u32(cvt_f32_i32(cvt_i32_f32(pg) * fa));
      pb = vec_u32(cvt_f32_i32(cvt_i32_f32(pb) * fa));
      pix = pix & alpha_mask;

      pr = slli_i32<RI * 8>(pr);
      pg = slli_i32<GI * 8>(pg);
      pb = slli_i32<BI * 8>(pb);
      pix = pix | vec_u8(pr) | vec_u8(pg) | vec_u8(pb);

      storeu_256_mask32(dst_data, pix, load_store_mask);

      dst_data += i * 4;
      src_data += i * 4;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

BLResult bl_convert_unpremultiply_8888_leading_alpha_float_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_unpremultiply_8888_float_template_avx2<24>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_unpremultiply_8888_trailing_alpha_float_avx2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_unpremultiply_8888_float_template_avx2<0>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

#endif
