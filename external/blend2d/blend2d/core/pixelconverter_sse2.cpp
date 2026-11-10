// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#ifdef BL_TARGET_OPT_SSE2

#include <blend2d/core/pixelconverter_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/memops_p.h>

using namespace SIMD;

// PixelConverter - Copy (SSE2)
// ============================

BLResult bl_convert_copy_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  const size_t bytes_per_pixel = bl_pixel_converter_get_data(self)->mem_copy_data.bytes_per_pixel;
  const size_t byte_width = size_t(w) * bytes_per_pixel;

  // Use a generic copy if `byte_width` is small as we would not be able to
  // utilize SIMD properly - in general we want to use at least 16-byte RW.
  if (byte_width < 16)
    return bl_convert_copy(self, dst_data, dst_stride, src_data, src_stride, w, h, options);

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(byte_width) + uintptr_t(gap);
  src_stride -= uintptr_t(byte_width);

  for (uint32_t y = h; y != 0; y--) {
    size_t i = byte_width;
    size_t alignment = 16 - (uintptr_t(dst_data) & 0xFu);

    storeu(dst_data, loadu<Vec16xU8>(src_data));

    i -= alignment;
    dst_data += alignment;
    src_data += alignment;

    BL_NOUNROLL
    while (i >= 64) {
      Vec16xU8 p0 = loadu<Vec16xU8>(src_data +  0);
      Vec16xU8 p1 = loadu<Vec16xU8>(src_data + 16);
      storea(dst_data +  0, p0);
      storea(dst_data + 16, p1);

      Vec16xU8 p2 = loadu<Vec16xU8>(src_data + 32);
      Vec16xU8 p3 = loadu<Vec16xU8>(src_data + 48);
      storea(dst_data + 32, p2);
      storea(dst_data + 48, p3);

      dst_data += 64;
      src_data += 64;
      i -= 64;
    }

    BL_NOUNROLL
    while (i >= 16) {
      storea(dst_data, loadu<Vec16xU8>(src_data));

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

// PixelConverter - Copy|Or (SSE2)
// ===============================

BLResult bl_convert_copy_or_8888_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  Vec16xU8 fill_mask = make128_u32<Vec16xU8>(bl_pixel_converter_get_data(self)->mem_copy_data.fill_mask);

  dst_stride -= uintptr_t(w) * 4u + gap;
  src_stride -= uintptr_t(w) * 4u;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 16) {
      Vec16xU8 p0 = loadu<Vec16xU8>(src_data +  0);
      Vec16xU8 p1 = loadu<Vec16xU8>(src_data + 16);
      storeu(dst_data +  0, p0 | fill_mask);
      storeu(dst_data + 16, p1 | fill_mask);

      Vec16xU8 p2 = loadu<Vec16xU8>(src_data + 32);
      Vec16xU8 p3 = loadu<Vec16xU8>(src_data + 48);
      storeu(dst_data + 32, p2 | fill_mask);
      storeu(dst_data + 48, p3 | fill_mask);

      dst_data += 64;
      src_data += 64;
      i -= 16;
    }

    BL_NOUNROLL
    while (i >= 4) {
      Vec16xU8 p0 = loadu<Vec16xU8>(src_data);
      storeu(dst_data, p0 | fill_mask);

      dst_data += 16;
      src_data += 16;
      i -= 4;
    }

    BL_NOUNROLL
    while (i) {
      Vec16xU8 p0 = loadu_32<Vec16xU8>(src_data);
      storeu_32(dst_data, p0 | fill_mask);

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

// PixelConverter - Premultiply (SSE2)
// ===================================

template<uint32_t A_Shift>
static BL_INLINE BLResult bl_convert_premultiply_8888_template_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4u + gap;
  src_stride -= uintptr_t(w) * 4u;

  const BLPixelConverterData::PremultiplyData& d = bl_pixel_converter_get_data(self)->premultiply_data;
  Vec16xU8 fill_mask = make128_u32<Vec16xU8>(d.fill_mask);
  Vec8xU16 alpha_mask = make128_u64<Vec8xU16>(uint64_t(0xFFu) << (A_Shift * 2));

  // Alpha byte-index that can be used by instructions that perform shuffling.
  constexpr uint32_t AI = A_Shift / 8u;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 4) {
      Vec16xU8 packed = loadu<Vec16xU8>(src_data);
      Vec8xU16 p1 = vec_u16(unpack_hi64_u8_u16(packed));
      Vec8xU16 p0 = vec_u16(unpack_lo64_u8_u16(packed));

      p1 = div255_u16((p1 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p1));
      p0 = div255_u16((p0 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p0));
      storeu(dst_data, vec_u8(packs_128_i16_u8(p0, p1)) | fill_mask);

      dst_data += 16;
      src_data += 16;
      i -= 4;
    }

    BL_NOUNROLL
    while (i) {
      Vec16xU8 packed = loadu_32<Vec16xU8>(src_data);
      Vec8xU16 p0 = vec_u16(unpack_lo64_u8_u16(packed));

      p0 = div255_u16((p0 | alpha_mask) * swizzle_u16<AI, AI, AI, AI>(p0));
      storeu_32(dst_data, vec_u8(packs_128_i16_u8(p0)) | fill_mask);

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

BLResult bl_convert_premultiply_8888_leading_alpha_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_template_sse2<24>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_premultiply_8888_trailing_alpha_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_premultiply_8888_template_sse2<0>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

// PixelConverter - Unpremultiply (SSE2)
// =====================================

template<uint32_t A_Shift>
static BL_INLINE BLResult bl_convert_unpremultiply_8888_template_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  bl_unused(self);

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4u + gap;
  src_stride -= uintptr_t(w) * 4u;

  const uint32_t* rcp_table = bl::common_table.unpremultiply_pmaddwd_rcp;
  const uint32_t* rnd_table = bl::common_table.unpremultiply_pmaddwd_rnd;

  Vec16xU8 alpha_mask = make128_u32<Vec16xU8>(0xFFu << A_Shift);
  Vec4xU32 component_mask = make128_u32<Vec4xU32>(0xFFu);

  // Alpha byte-index that can be used by instructions that perform shuffling.
  constexpr uint32_t AI = A_Shift / 8u;
  constexpr uint32_t RI = (AI + 1) % 4;
  constexpr uint32_t GI = (AI + 2) % 4;
  constexpr uint32_t BI = (AI + 3) % 4;

  constexpr uint32_t A = AI == 0 ? 3 : 0;
  constexpr uint32_t B = AI == 1 ? 3 : 0;
  constexpr uint32_t C = AI == 2 ? 3 : 0;
  constexpr uint32_t D = AI == 3 ? 3 : 0;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 4) {
      size_t idx0 = src_data[0 * 4 + AI];
      size_t idx1 = src_data[1 * 4 + AI];
      Vec16xU8 pix = loadu<Vec16xU8>(src_data);

      Vec4xU32 rcp0 = loada_32<Vec4xU32>(rcp_table + idx0);
      Vec4xU32 rcp1 = loada_32<Vec4xU32>(rcp_table + idx1);
      Vec4xU32 rnd0 = loada_32<Vec4xU32>(rnd_table + idx0);
      Vec4xU32 rnd1 = loada_32<Vec4xU32>(rnd_table + idx1);

      size_t idx2 = src_data[2 * 4 + AI];
      size_t idx3 = src_data[3 * 4 + AI];
      rcp0 = interleave_lo_u32(rcp0, rcp1);
      rnd0 = interleave_lo_u32(rnd0, rnd1);

      Vec4xU32 rcp2 = loada_32<Vec4xU32>(rcp_table + idx2);
      Vec4xU32 rcp3 = loada_32<Vec4xU32>(rcp_table + idx3);
      Vec4xU32 rnd2 = loada_32<Vec4xU32>(rnd_table + idx2);
      Vec4xU32 rnd3 = loada_32<Vec4xU32>(rnd_table + idx3);

      rcp2 = interleave_lo_u32(rcp2, rcp3);
      rnd2 = interleave_lo_u32(rnd2, rnd3);
      rcp0 = interleave_lo_u64(rcp0, rcp2);
      rnd0 = interleave_lo_u64(rnd0, rnd2);

      Vec4xU32 pr = vec_u32(srli_u32<RI * 8>(pix));
      Vec4xU32 pg = vec_u32(srli_u32<GI * 8>(pix));
      Vec4xU32 pb = vec_u32(srli_u32<BI * 8>(pix));

      if (RI != 3) pr &= component_mask;
      if (GI != 3) pg &= component_mask;
      if (BI != 3) pb &= component_mask;

      pr = maddw_i16_i32(pr | slli_i32<16 + 6>(pr), rcp0);
      pg = maddw_i16_i32(pg | slli_i32<16 + 6>(pg), rcp0);
      pb = maddw_i16_i32(pb | slli_i32<16 + 6>(pb), rcp0);
      pix = pix & alpha_mask;

      pr = slli_i32<RI * 8>(srli_u32<13>(pr + rnd0));
      pg = slli_i32<GI * 8>(srli_u32<13>(pg + rnd0));
      pb = slli_i32<BI * 8>(srli_u32<13>(pb + rnd0));
      storeu(dst_data, pix | vec_u8(pr) | vec_u8(pg) | vec_u8(pb));

      dst_data += 16;
      src_data += 16;
      i -= 4;
    }

    BL_NOUNROLL
    while (i) {
      size_t idx0 = src_data[AI];
      Vec16xU8 pix = loadu_32<Vec16xU8>(src_data);

      Vec4xU32 p0 = vec_u32(unpack_lo32_u8_u32(pix));
      Vec4xU32 rcp0 = swizzle_u32<D, C, B, A>(loada_32<Vec4xU32>(rcp_table + idx0));
      Vec4xU32 rnd0 = swizzle_u32<D, C, B, A>(loada_32<Vec4xU32>(rnd_table + idx0));

      p0 = p0 | slli_i32<16 + 6>(p0);
      pix = pix & alpha_mask;

      p0 = maddw_i16_i32(p0, rcp0);
      p0 = srli_u32<13>(p0 + rnd0);
      storeu_32(dst_data, vec_u8(packs_128_i32_u8(p0)) | pix);

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

BLResult bl_convert_unpremultiply_8888_leading_alpha_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_unpremultiply_8888_template_sse2<24>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

BLResult bl_convert_unpremultiply_8888_trailing_alpha_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return bl_convert_unpremultiply_8888_template_sse2<0>(self, dst_data, dst_stride, src_data, src_stride, w, h, options);
}

// bl::PixelConverter - RGB32 From A8/L8 (SSE2)
// ============================================

BLResult bl_convert_8888_from_x8_sse2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w);

  const BLPixelConverterData::Rgb32FromX8Data& d = bl_pixel_converter_get_data(self)->rgb32FromX8Data;
  uint32_t fillMask32 = d.fill_mask;
  uint32_t zeroMask32 = d.zero_mask;

  Vec16xU8 fill_mask = make128_u32<Vec16xU8>(fillMask32);
  Vec16xU8 zero_mask = make128_u32<Vec16xU8>(zeroMask32);

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;

    BL_NOUNROLL
    while (i >= 16) {
      Vec16xU8 p0, p1, p2, p3;

      p0 = loadu<Vec16xU8>(src_data);
      p2 = interleave_hi_u8(p0, p0);
      p0 = interleave_lo_u8(p0, p0);

      p1 = interleave_hi_u16(p0, p0);
      p0 = interleave_lo_u16(p0, p0);
      p3 = interleave_hi_u16(p2, p2);
      p2 = interleave_lo_u16(p2, p2);

      storeu(dst_data +  0, (p0 & zero_mask) | fill_mask);
      storeu(dst_data + 16, (p1 & zero_mask) | fill_mask);
      storeu(dst_data + 32, (p2 & zero_mask) | fill_mask);
      storeu(dst_data + 48, (p3 & zero_mask) | fill_mask);

      dst_data += 64;
      src_data += 16;
      i -= 16;
    }

    BL_NOUNROLL
    while (i >= 4) {
      Vec16xU8 p0 = loadu_32<Vec16xU8>(src_data);
      p0 = interleave_lo_u8(p0, p0);
      p0 = interleave_lo_u16(p0, p0);
      storeu(dst_data, (p0 & zero_mask) | fill_mask);

      dst_data += 16;
      src_data += 4;
      i -= 4;
    }

    BL_NOUNROLL
    while (i) {
      bl::MemOps::writeU32u(dst_data, ((uint32_t(src_data[0]) * 0x01010101u) & zeroMask32) | fillMask32);
      dst_data += 4;
      src_data += 1;
      i--;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

#endif
