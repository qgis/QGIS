// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FETCHUTILSBILINEAR_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FETCHUTILSBILINEAR_P_H_INCLUDED

#include <blend2d/core/format.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelgather_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {
namespace FetchUtils {

// Bilinear interpolation with calculated weights
// ==============================================
//
//   P' = [Px0y0 * (256 - Wx) * (256 - Wy) +
//         Px1y0 * (Wx      ) * (256 - Wy) +
//         Px0y1 * (256 - Wx) * (Wy      ) +
//         Px1y1 * (Wx      ) * (Wy      ) ]
//
//   P' = [Px0y0 * (256 - Wx) + Px1y0 * Wx] * (256 - Wy) +
//        [Px0y1 * (256 - Wx) + Px1y1 * Wx] * Wy
//
//   P' = [Px0y0 * (256 - Wy) + Px0y1 * Wy] * (256 - Wx) +
//        [Px1y0 * (256 - Wy) + Px1y1 * Wy] * Wx

//! Fetch 1xA8 pixel by doing a bilinear interpolation with its neighbors.
//!
//! Weights = {256-wy, wy, 256-wy, wy, 256-wx, wx, 256-wx, wx}
template<typename Pixels, typename Stride>
BL_NOINLINE void filter_bilinear_a8_1x(
  PipeCompiler* pc,
  Vec& out,
  const Pixels& pixels,
  const Stride& stride,
  PixelFetchInfo f_info,
  uint32_t index_shift,
  const Vec& indexes,
  const Vec& weights) noexcept {

  IndexExtractor extractor(pc);

  Gp pix_src_row0 = pc->new_gpz("pix_src_row0");
  Gp pix_src_row1 = pc->new_gpz("pix_src_row1");
  Gp pix_src_off = pc->new_gpz("pix_src_off");
  Gp pix_acc = pc->new_gp32("pix_acc");
  Vec wTmp = pc->new_vec128("wTmp");

  extractor.begin(IndexExtractor::kTypeUInt32, indexes);
  extractor.extract(pix_src_row0, 2);
  extractor.extract(pix_src_row1, 3);

  int32_t fetch_alpha_offset = f_info.fetch_alpha_offset();

  pc->mul(pix_src_row0, pix_src_row0, stride);
  pc->mul(pix_src_row1, pix_src_row1, stride);
  pc->add(pix_src_row0, pix_src_row0, pixels);
  pc->add(pix_src_row1, pix_src_row1, pixels);

#if defined(BL_JIT_ARCH_X86)
  Mem row0m = mem_ptr(pix_src_row0, pix_src_off, index_shift, fetch_alpha_offset);
  Mem row1m = mem_ptr(pix_src_row1, pix_src_off, index_shift, fetch_alpha_offset);
#else
  Mem row0m;
  Mem row1m;

  if (fetch_alpha_offset != 0) {
    Gp pixSrcRow0a = pc->new_similar_reg(pix_src_row0, "@row0_alpha");
    Gp pixSrcRow1a = pc->new_similar_reg(pix_src_row1, "@row1_alpha");

    pc->add(pixSrcRow0a, pix_src_row0, fetch_alpha_offset);
    pc->add(pixSrcRow1a, pix_src_row1, fetch_alpha_offset);

    row0m = mem_ptr(pixSrcRow0a, pix_src_off, index_shift);
    row1m = mem_ptr(pixSrcRow1a, pix_src_off, index_shift);
  }
  else {
    row0m = mem_ptr(pix_src_row0, pix_src_off, index_shift);
    row1m = mem_ptr(pix_src_row1, pix_src_off, index_shift);
  }
#endif

  extractor.extract(pix_src_off, 0);
  pc->load_u8(pix_acc, row0m);       // [0    , 0    , 0    , Px0y0]
  pc->load_shift_u8(pix_acc, row1m); // [0    , 0    , Px0y0, Px0y1]

  extractor.extract(pix_src_off, 1);
  pc->load_shift_u8(pix_acc, row0m); // [0    , Px0y0, Px0y1, Px1y0]
  pc->load_shift_u8(pix_acc, row1m); // [Px0y0, Px0y1, Px1y0, Px1y1]

  pc->s_mov_u32(out, pix_acc);
  pc->v_swizzle_u32x4(wTmp, weights, swizzle(3, 3, 2, 2));

  pc->v_cvt_u8_lo_to_u16(out, out);
  pc->v_mhadd_i16_to_i32(out, out, wTmp);
  pc->v_srli_u16(out, out, 8);
  pc->v_packs_i32_i16(out, out, out);
  pc->v_mhadd_i16_to_i32(out, out, weights);
  pc->v_srli_u16(out, out, 8);
}

//! Fetch 1xPRGB pixel by doing a bilinear interpolation with its neighbors.
//!
//! Weights = {256-wy, 256-wy, wy, wy, 256-wx, 256-wx, wx, wx}
template<typename Pixels, typename Stride>
BL_NOINLINE void filter_bilinear_argb32_1x(
  PipeCompiler* pc,
  Vec& out,
  const Pixels& pixels,
  const Stride& stride,
  const Vec& indexes,
  const Vec& weights) noexcept {

  IndexExtractor extractor(pc);

  Gp pix_src_row0 = pc->new_gpz("pix_src_row0");
  Gp pix_src_row1 = pc->new_gpz("pix_src_row1");
  Gp pix_src_off = pc->new_gpz("pix_src_off");

  Vec pix_top = pc->new_vec128("pix_top");
  Vec pix_bot = pc->new_vec128("pix_bot");

  Vec pix_tmp0 = out;
  Vec pix_tmp1 = pc->new_vec128("pix_tmp1");

  extractor.begin(IndexExtractor::kTypeUInt32, indexes);
  extractor.extract(pix_src_row0, 2);
  extractor.extract(pix_src_row1, 3);

  pc->mul(pix_src_row0, pix_src_row0, stride);
  pc->mul(pix_src_row1, pix_src_row1, stride);
  pc->add(pix_src_row0, pix_src_row0, pixels);
  pc->add(pix_src_row1, pix_src_row1, pixels);

  extractor.extract(pix_src_off, 0);
  pc->v_loada32(pix_top, mem_ptr(pix_src_row0, pix_src_off, 2));
  pc->v_loada32(pix_bot, mem_ptr(pix_src_row1, pix_src_off, 2));
  extractor.extract(pix_src_off, 1);

  FetchUtils::fetch_second_32bit_element(pc, pix_top, mem_ptr(pix_src_row0, pix_src_off, 2));
  FetchUtils::fetch_second_32bit_element(pc, pix_bot, mem_ptr(pix_src_row1, pix_src_off, 2));

  pc->v_swizzle_u32x4(pix_tmp0, weights, swizzle(3, 3, 3, 3));
  pc->v_cvt_u8_lo_to_u16(pix_top, pix_top);

  pc->v_swizzle_u32x4(pix_tmp1, weights, swizzle(2, 2, 2, 2));
  pc->v_cvt_u8_lo_to_u16(pix_bot, pix_bot);

  pc->v_mul_u16(pix_top, pix_top, pix_tmp0);
  pc->v_mul_u16(pix_bot, pix_bot, pix_tmp1);

  pc->v_add_i16(pix_bot, pix_bot, pix_top);
  pc->v_srli_u16(pix_bot, pix_bot, 8);

  pc->v_swizzle_u32x4(pix_top, weights, swizzle(0, 0, 1, 1));
  pc->v_mul_u16(pix_top, pix_top, pix_bot);

  pc->v_swizzle_u32x4(pix_tmp0, pix_top, swizzle(1, 0, 3, 2));
  pc->v_add_i16(pix_tmp0, pix_tmp0, pix_top);
  pc->v_srli_u16(pix_tmp0, pix_tmp0, 8);
}

} // {FetchUtils}
} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FETCHUTILSBILINEAR_P_H_INCLUDED
