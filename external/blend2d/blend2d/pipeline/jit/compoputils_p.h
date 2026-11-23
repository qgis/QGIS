// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_COMPOPUTILS_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_COMPOPUTILS_P_H_INCLUDED

#include <blend2d/core/compopinfo_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/support/wrap_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {
namespace CompOpUtils {

#if defined(BL_JIT_ARCH_A64)
template<typename Dst, typename Src0, typename Src1>
static void mul_u8_widen(PipeCompiler* pc, const Dst& dst, const Src0& src0, const Src1& src1, uint32_t count) noexcept {
  pc->v_mulw_lo_u8(dst.even(), src0, src1);
  if (count > 8)
    pc->v_mulw_hi_u8(dst.odd(), src0, src1);
}

template<typename Dst, typename Src0, typename Src1>
static void madd_u8_widen(PipeCompiler* pc, const Dst& dst, const Src0& src0, const Src1& src1, uint32_t count) noexcept {
  pc->v_maddw_lo_u8(dst.even(), src0, src1);
  if (count > 8)
    pc->v_maddw_hi_u8(dst.odd(), src0, src1);
}

static void div255_pack(PipeCompiler* pc, const Vec& dst, const Vec& src) noexcept {
  pc->v_srli_rnd_acc_u16(src, src, 8);
  pc->cc->rshrn(dst.b8(), src.h8(), 8);
}

static void div255_pack(PipeCompiler* pc, const VecArray& dst, const VecArray& src) noexcept {
  pc->v_srli_rnd_acc_u16(src, src, 8);
  for (uint32_t i = 0; i < src.size(); i++) {
    if ((i & 1) == 0)
      pc->cc->rshrn(dst[i / 2].b8(), src[i].h8(), 8);
    else
      pc->cc->rshrn2(dst[i / 2].b16(), src[i].h8(), 8);
  }
}
#endif

static void combine_div255_and_out_a8(PipeCompiler* pc, Pixel& out, PixelFlags flags, const VecArray& pix) noexcept {
#if defined(BL_JIT_ARCH_A64)
  if (!bl_test_flag(flags, PixelFlags::kUA)) {
    pc->v_srli_rnd_acc_u16(pix, pix, 8);
    for (uint32_t i = 0; i < pix.size(); i++) {
      if ((i & 1) == 0)
        pc->cc->rshrn(pix[i / 2].b8(), pix[i].h8(), 8);
      else
        pc->cc->rshrn2(pix[i / 2].b16(), pix[i].h8(), 8);
    }
    out.pa.init(pix.half());
  }
  else
#endif // BL_JIT_ARCH_A64
  {
    bl_unused(flags);

    pc->v_div255_u16(pix);
    out.ua.init(pix);
  }
}

static void combineDiv255AndOutRGBA32(PipeCompiler* pc, Pixel& out, PixelFlags flags, const VecArray& pix) noexcept {
#if defined(BL_JIT_ARCH_A64)
  if (!bl_test_flag(flags, PixelFlags::kUC)) {
    pc->v_srli_rnd_acc_u16(pix, pix, 8);
    for (uint32_t i = 0; i < pix.size(); i++) {
      if ((i & 1) == 0)
        pc->cc->rshrn(pix[i / 2].b8(), pix[i].h8(), 8);
      else
        pc->cc->rshrn2(pix[i / 2].b16(), pix[i].h8(), 8);
    }
    out.pc.init(pix.half());
  }
  else
#endif // BL_JIT_ARCH_A64
  {
    bl_unused(flags);

    pc->v_div255_u16(pix);
    out.uc.init(pix);
  }
}

} // {CompOpUtils}
} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_COMPOPUTILS_P_H_INCLUDED
