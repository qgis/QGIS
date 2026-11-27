// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_PIPECOMPILER_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_PIPECOMPILER_P_H_INCLUDED

#include <blend2d/core/runtime_p.h>
#include <blend2d/pipeline/jit/jitbase_p.h>

// TODO: [JIT] The intention is to have it as independent as possible, so remove this in the future!
#include <blend2d/pipeline/jit/pipeprimitives_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! Pipeline compiler.
class PipeCompiler : public UniCompiler {
public:
  BL_NONCOPYABLE(PipeCompiler)

  //! \name Members
  //! \{

  //! Function end hook (to add 'unlikely' branches).
  asmjit::BaseNode* _func_end = nullptr;

  //! Empty predicate, used in cases where a predicate is required, but it's empty.
  PixelPredicate _empty_predicate {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  PipeCompiler(BackendCompiler* cc, const CpuFeatures& features, CpuHints cpu_hints) noexcept;
  ~PipeCompiler() noexcept;

  //! \}

  //! \name CPU SIMD Width and SIMD Width Utilities
  //! \{

  using UniCompiler::vec_width_of;

  BL_INLINE_NODEBUG VecWidth vec_width_of(DataWidth data_width, PixelCount pixel_count) const noexcept { return VecWidthUtils::vec_width_of(vec_width(), data_width, size_t(pixel_count)); }
  BL_INLINE_NODEBUG size_t vec_count_of(DataWidth data_width, size_t n) const noexcept { return VecWidthUtils::vec_count_of(vec_width(), data_width, n); }
  BL_INLINE_NODEBUG size_t vec_count_of(DataWidth data_width, PixelCount pixel_count) const noexcept { return VecWidthUtils::vec_count_of(vec_width(), data_width, size_t(pixel_count)); }

  //! \}

  //! \name Utilities
  //! \{

  BL_INLINE PixelPredicate& empty_predicate() noexcept { return _empty_predicate; }

  //! \}

  //! \name Utilities
  //! \{

  template<typename DstT, typename SrcT>
  inline void v_inv255_u16(const DstT& dst, const SrcT& src) noexcept {
    Operand u16_255 = simd_const(&ct<CommonTable>().p_00FF00FF00FF00FF, Bcst::k32, dst);
    v_xor_i32(dst, src, u16_255);
  }

  template<typename DstT, typename SrcT>
  BL_NOINLINE void v_mul257_hi_u16(const DstT& dst, const SrcT& src) {
#if defined(BL_JIT_ARCH_X86)
    v_mulh_u16(dst, src, simd_const(&ct<CommonTable>().p_0101010101010101, Bcst::kNA, dst));
#elif defined(BL_JIT_ARCH_A64)
    v_srli_acc_u16(dst, src, 8);
    v_srli_u16(dst, dst, 8);
#endif
  }

  // TODO: [JIT] Consolidate this to only one implementation.
  template<typename DstSrcT>
  BL_NOINLINE void v_div255_u16(const DstSrcT& x) {
#if defined(BL_JIT_ARCH_X86)
    Operand p_0080008000800080 = simd_const(&ct<CommonTable>().p_0080008000800080, Bcst::kNA, x);

    v_add_i16(x, x, p_0080008000800080);
    v_mul257_hi_u16(x, x);
#elif defined(BL_JIT_ARCH_A64)
    v_srli_rnd_acc_u16(x, x, 8);
    v_srli_rnd_u16(x, x, 8);
#endif
  }

  template<typename DstSrcT>
  BL_NOINLINE void v_div255_u16_2x(const DstSrcT& v0, const DstSrcT& v1) noexcept {
#if defined(BL_JIT_ARCH_X86)
    Operand p_0080008000800080 = simd_const(&ct<CommonTable>().p_0080008000800080, Bcst::kNA, v0);
    Operand p_0101010101010101 = simd_const(&ct<CommonTable>().p_0101010101010101, Bcst::kNA, v0);

    v_add_i16(v0, v0, p_0080008000800080);
    v_add_i16(v1, v1, p_0080008000800080);

    v_mulh_u16(v0, v0, p_0101010101010101);
    v_mulh_u16(v1, v1, p_0101010101010101);
#elif defined(BL_JIT_ARCH_A64)
    v_srli_rnd_acc_u16(v0, v0, 8);
    v_srli_rnd_acc_u16(v1, v1, 8);
    v_srli_rnd_u16(v0, v0, 8);
    v_srli_rnd_u16(v1, v1, 8);
#endif
  }

  // d = int(floor(a / b) * b).
  template<typename VecOrMem>
  BL_NOINLINE void v_mod_pd(const Vec& d, const Vec& a, const VecOrMem& b) noexcept {
#if defined(BL_JIT_ARCH_X86)
    if (!has_sse4_1()) {
      Vec t = new_vec128("vModTmp");

      v_div_f64(d, a, b);
      v_cvt_trunc_f64_to_i32_lo(t, d);
      v_cvt_i32_lo_to_f64(d, t);
      v_mul_f64(d, d, b);
    }
    else
#endif // BL_JIT_ARCH_X86
    {
      v_div_f64(d, a, b);
      v_trunc_f64(d, d);
      v_mul_f64(d, d, b);
    }
  }

  //! \}

#if defined(BL_JIT_ARCH_X86)
  //! \name Memory Loads & Stores with Predicate
  //! \{

  KReg make_mask_predicate(PixelPredicate& predicate, size_t last_n) noexcept;
  KReg make_mask_predicate(PixelPredicate& predicate, size_t last_n, const Gp& adjusted_count) noexcept;

  Vec make_vec_predicate32(PixelPredicate& predicate, size_t last_n) noexcept;
  Vec make_vec_predicate32(PixelPredicate& predicate, size_t last_n, const Gp& adjusted_count) noexcept;

  BL_NOINLINE void v_load_predicated_u8(const Vec& dst, const Mem& src, size_t n, PixelPredicate& predicate) noexcept{
    if (has_avx512()) {
      KReg kPred = make_mask_predicate(predicate, n);
      cc->k(kPred).z().vmovdqu8(dst, src);
    }
    else {
      BL_NOT_REACHED();
    }
  }

  BL_NOINLINE void v_store_predicated_u8(const Mem& dst, const Vec& src, size_t n, PixelPredicate& predicate) noexcept{
    if (has_avx512()) {
      KReg kPred = make_mask_predicate(predicate, n);
      cc->k(kPred).vmovdqu8(dst, src);
    }
    else {
      BL_NOT_REACHED();
    }
  }

  BL_NOINLINE void v_load_predicated_u32(const Vec& dst, const Mem& src, size_t n, PixelPredicate& predicate) noexcept{
    if (has_avx512()) {
      KReg kPred = make_mask_predicate(predicate, n);
      cc->k(kPred).z().vmovdqu32(dst, src);
    }
    else if (has_avx()) {
      Vec v_pred = make_vec_predicate32(predicate, n);
      InstId inst_id = has_avx2() ? x86::Inst::kIdVpmaskmovd : x86::Inst::kIdVmaskmovps;
      cc->emit(inst_id, dst, v_pred, src);
    }
    else {
      BL_NOT_REACHED();
    }
  }

  BL_NOINLINE void v_store_predicated_u32(const Mem& dst, const Vec& src, size_t n, PixelPredicate& predicate) noexcept{
    if (has_avx512()) {
      KReg kPred = make_mask_predicate(predicate, n);
      cc->k(kPred).vmovdqu32(dst, src);
    }
    else if (has_avx()) {
      Vec v_pred = make_vec_predicate32(predicate, n);
      InstId inst_id = has_avx2() ? x86::Inst::kIdVpmaskmovd : x86::Inst::kIdVmaskmovps;
      cc->emit(inst_id, dst, v_pred, src);
    }
    else {
      BL_NOT_REACHED();
    }
  }

  //! \}

#endif // BL_JIT_ARCH_X86

  //! \name Emit - 'X' High Level Functionality
  //! \{

  // Kind of a hack - if we don't have SSE4.1 we have to load the byte into GP register first and then we use 'PINSRW',
  // which is provided by baseline SSE2. If we have SSE4.1 then it's much easier as we can load the byte by 'PINSRB'.
  void x_insert_word_or_byte(const Vec& dst, const Mem& src, uint32_t word_index) noexcept {
#if defined(BL_JIT_ARCH_X86)
    if (has_sse4_1()) {
      Mem m = src;
      m.set_size(1);
      v_insert_u8(dst, m, word_index * 2u);
    }
    else {
      Gp tmp = new_gp32("@tmp");
      load_u8(tmp, src);
      s_insert_u16(dst, tmp, word_index);
    }
#else
    v_insert_u8(dst, src, word_index * 2);
#endif
  }

  //! \}

  //! \name Emit - Pixel Processing Utilities
  //! \{

  //! Pack 16-bit integers to unsigned 8-bit integers in an AVX2 and AVX512 aware way.
  template<typename Dst, typename Src1, typename Src2>
  BL_NOINLINE void x_packs_i16_u8(const Dst& d, const Src1& s1, const Src2& s2) noexcept {
#if defined(BL_JIT_ARCH_X86)
    if (s1.is_vec128()) {
      v_packs_i16_u8(d, s1, s2);
    }
    else {
      const Vec& v_type = OpUtils::first_op(s1).template as<Vec>();
      v_packs_i16_u8(d, s1, s2);
      v_swizzle_u64x4(d.clone_as(v_type), d.clone_as(v_type), swizzle(3, 1, 2, 0));
    }
#else
    v_packs_i16_u8(d, s1, s2);
#endif
  }

  BL_NOINLINE void xStorePixel(const Gp& d_ptr, const Vec& v_src, uint32_t count, uint32_t bpp, Alignment alignment) noexcept {
    v_store_iany(mem_ptr(d_ptr), v_src, count * bpp, alignment);
  }

  inline void xStore32_ARGB(const Mem& dst, const Vec& v_src) noexcept {
    v_storea32(dst, v_src);
  }

  BL_NOINLINE void xMovzxBW_LoHi(const Vec& d0, const Vec& d1, const Vec& s) noexcept {
    BL_ASSERT(d0.id() != d1.id());

#if defined(BL_JIT_ARCH_X86)
    if (has_sse4_1()) {
      if (d0.id() == s.id()) {
        v_swizzle_u32x4(d1, d0, swizzle(1, 0, 3, 2));
        v_cvt_u8_lo_to_u16(d0, d0);
        v_cvt_u8_lo_to_u16(d1, d1);
      }
      else {
        v_cvt_u8_lo_to_u16(d0, s);
        v_swizzle_u32x4(d1, s, swizzle(1, 0, 3, 2));
        v_cvt_u8_lo_to_u16(d1, d1);
      }
    }
    else {
      Vec zero = simd_vec_const(&ct<CommonTable>().p_0000000000000000, Bcst::k32, s);
      if (d1.id() != s.id()) {
        v_interleave_hi_u8(d1, s, zero);
        v_interleave_lo_u8(d0, s, zero);
      }
      else {
        v_interleave_lo_u8(d0, s, zero);
        v_interleave_hi_u8(d1, s, zero);
      }
    }
#elif defined(BL_JIT_ARCH_A64)
    if (d0.id() == s.id()) {
      cc->sshll2(d1, s, 0);
      cc->sshll(d0, s, 0);
    }
    else {
      cc->sshll(d0, s, 0);
      cc->sshll2(d1, s, 0);
    }
#endif
  }

  template<typename Dst, typename Src>
  inline void vExpandAlphaLo16(const Dst& d, const Src& s) noexcept { v_swizzle_lo_u16x4(d, s, swizzle(3, 3, 3, 3)); }

  template<typename Dst, typename Src>
  inline void vExpandAlphaHi16(const Dst& d, const Src& s) noexcept { v_swizzle_hi_u16x4(d, s, swizzle(3, 3, 3, 3)); }

  template<typename Dst, typename Src>
  inline void v_expand_alpha_16(const Dst& d, const Src& s, uint32_t use_hi_part = 1) noexcept {
#if defined(BL_JIT_ARCH_X86)
    if (use_hi_part) {
      if (has_avx() || (has_ssse3() && d == s)) {
        v_swizzlev_u8(d, s, simd_const(&ct<CommonTable>().swizu8_32xxxxxx10xxxxxx_to_3232323210101010, Bcst::kNA, d));
      }
      else {
        vExpandAlphaHi16(d, s);
        vExpandAlphaLo16(d, d);
      }
    }
    else {
      vExpandAlphaLo16(d, s);
    }
#elif defined(BL_JIT_ARCH_A64)
    bl_unused(use_hi_part);
    v_swizzle_u16x4(d, s, swizzle(3, 3, 3, 3));
#endif
  }

  template<typename Dst, typename Src>
  inline void vExpandAlphaPS(const Dst& d, const Src& s) noexcept { v_swizzle_u32x4(d, s, swizzle(3, 3, 3, 3)); }

  template<typename DstT, typename SrcT>
  inline void vFillAlpha255B(const DstT& dst, const SrcT& src) noexcept { v_or_i32(dst, src, simd_const(&ct<CommonTable>().p_FF000000FF000000, Bcst::k32, dst)); }
  template<typename DstT, typename SrcT>
  inline void vFillAlpha255W(const DstT& dst, const SrcT& src) noexcept { v_or_i64(dst, src, simd_const(&ct<CommonTable>().p_00FF000000000000, Bcst::k64, dst)); }

  template<typename DstT, typename SrcT>
  inline void vZeroAlphaB(const DstT& dst, const SrcT& src) noexcept { v_and_i32(dst, src, simd_mem_const(&ct<CommonTable>().p_00FFFFFF00FFFFFF, Bcst::k32, dst)); }

  template<typename DstT, typename SrcT>
  inline void vZeroAlphaW(const DstT& dst, const SrcT& src) noexcept { v_and_i64(dst, src, simd_mem_const(&ct<CommonTable>().p_0000FFFFFFFFFFFF, Bcst::k64, dst)); }

  template<typename DstT, typename SrcT>
  inline void vNegAlpha8B(const DstT& dst, const SrcT& src) noexcept { v_xor_i32(dst, src, simd_const(&ct<CommonTable>().p_FF000000FF000000, Bcst::k32, dst)); }
  template<typename DstT, typename SrcT>
  inline void vNegAlpha8W(const DstT& dst, const SrcT& src) noexcept { v_xor_i64(dst, src, simd_const(&ct<CommonTable>().p_00FF000000000000, Bcst::k64, dst)); }

  template<typename DstT, typename SrcT>
  inline void vNegRgb8B(const DstT& dst, const SrcT& src) noexcept { v_xor_i32(dst, src, simd_const(&ct<CommonTable>().p_00FFFFFF00FFFFFF, Bcst::k32, dst)); }
  template<typename DstT, typename SrcT>
  inline void vNegRgb8W(const DstT& dst, const SrcT& src) noexcept { v_xor_i64(dst, src, simd_const(&ct<CommonTable>().p_000000FF00FF00FF, Bcst::k64, dst)); }

  // Performs 32-bit unsigned modulo of 32-bit `a` (hi DWORD) with 32-bit `b` (lo DWORD).
  template<typename VecOrMem_A, typename VecOrMem_B>
  BL_NOINLINE void xModI64HIxU64LO(const Vec& d, const VecOrMem_A& a, const VecOrMem_B& b) noexcept {
    Vec t0 = new_vec128("t0");
    Vec t1 = new_vec128("t1");

    v_swizzle_u32x4(t1, b, swizzle(3, 3, 2, 0));
    v_swizzle_u32x4(d , a, swizzle(2, 0, 3, 1));

    v_cvt_i32_lo_to_f64(t1, t1);
    v_cvt_i32_lo_to_f64(t0, d);
    v_mod_pd(t0, t0, t1);
    v_cvt_trunc_f64_to_i32_lo(t0, t0);

    v_sub_i32(d, d, t0);
    v_swizzle_u32x4(d, d, swizzle(1, 3, 0, 2));
  }

  // Performs 32-bit unsigned modulo of 32-bit `a` (hi DWORD) with 64-bit `b` (DOUBLE).
  template<typename VecOrMem_A, typename VecOrMem_B>
  BL_NOINLINE void xModI64HIxDouble(const Vec& d, const VecOrMem_A& a, const VecOrMem_B& b) noexcept {
    Vec t0 = new_vec128("t0");

    v_swizzle_u32x4(d, a, swizzle(2, 0, 3, 1));
    v_cvt_i32_lo_to_f64(t0, d);
    v_mod_pd(t0, t0, b);
    v_cvt_trunc_f64_to_i32_lo(t0, t0);

    v_sub_i32(d, d, t0);
    v_swizzle_u32x4(d, d, swizzle(1, 3, 0, 2));
  }

  BL_NOINLINE void xExtractUnpackedAFromPackedARGB32_1(const Vec& d, const Vec& s) noexcept {
    v_swizzle_lo_u16x4(d, s, swizzle(1, 1, 1, 1));
    v_srli_u16(d, d, 8);
  }

  BL_NOINLINE void xExtractUnpackedAFromPackedARGB32_2(const Vec& d, const Vec& s) noexcept {
#if defined(BL_JIT_ARCH_X86)
    if (!has_ssse3()) {
      v_swizzle_lo_u16x4(d, s, swizzle(3, 3, 1, 1));
      v_swizzle_u32x4(d, d, swizzle(1, 1, 0, 0));
      v_srli_u16(d, d, 8);
      return;
    }
#endif

    v_swizzlev_u8(d, s, simd_const(&ct<CommonTable>().swizu8_xxxxxxxx1xxx0xxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d));
  }

  BL_NOINLINE void xExtractUnpackedAFromPackedARGB32_4(const Vec& d0, const Vec& d1, const Vec& s) noexcept {
    BL_ASSERT(d0.id() != d1.id());

#if defined(BL_JIT_ARCH_X86)
    if (!has_ssse3()) {
      if (d1.id() != s.id()) {
        v_swizzle_hi_u16x4(d1, s, swizzle(3, 3, 1, 1));
        v_swizzle_lo_u16x4(d0, s, swizzle(3, 3, 1, 1));

        v_swizzle_u32x4(d1, d1, swizzle(3, 3, 2, 2));
        v_swizzle_u32x4(d0, d0, swizzle(1, 1, 0, 0));

        v_srli_u16(d1, d1, 8);
        v_srli_u16(d0, d0, 8);
      }
      else {
        v_swizzle_lo_u16x4(d0, s, swizzle(3, 3, 1, 1));
        v_swizzle_hi_u16x4(d1, s, swizzle(3, 3, 1, 1));

        v_swizzle_u32x4(d0, d0, swizzle(1, 1, 0, 0));
        v_swizzle_u32x4(d1, d1, swizzle(3, 3, 2, 2));

        v_srli_u16(d0, d0, 8);
        v_srli_u16(d1, d1, 8);
      }
      return;
    }
#endif

    if (d0.id() == s.id()) {
      v_swizzlev_u8(d1, s, simd_const(&ct<CommonTable>().swizu8_1xxx0xxxxxxxxxxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d1));
      v_swizzlev_u8(d0, s, simd_const(&ct<CommonTable>().swizu8_xxxxxxxx1xxx0xxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d0));
    }
    else {
      v_swizzlev_u8(d0, s, simd_const(&ct<CommonTable>().swizu8_xxxxxxxx1xxx0xxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d0));
      v_swizzlev_u8(d1, s, simd_const(&ct<CommonTable>().swizu8_1xxx0xxxxxxxxxxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d1));
    }
  }

  BL_NOINLINE void xPackU32ToU16Lo(const Vec& d0, const Vec& s0) noexcept {
#if defined(BL_JIT_ARCH_X86)
    if (has_sse4_1()) {
      v_packs_i32_u16(d0, s0, s0);
    }
    else if (has_ssse3()) {
      v_swizzlev_u8(d0, s0, simd_const(&ct<CommonTable>().swizu8_xx76xx54xx32xx10_to_7654321076543210, Bcst::kNA, d0));
    }
    else {
      // Sign extend and then use `packssdw()`.
      v_slli_i32(d0, s0, 16);
      v_srai_i32(d0, d0, 16);
      v_packs_i32_i16(d0, d0, d0);
    }
#elif defined(BL_JIT_ARCH_A64)
    cc->sqxtun(d0.h4(), s0.s4());
#endif
  }

  BL_NOINLINE void xPackU32ToU16Lo(const VecArray& d0, const VecArray& s0) noexcept {
    for (uint32_t i = 0; i < d0.size(); i++)
      xPackU32ToU16Lo(d0[i], s0[i]);
  }
};

class PipeInjectAtTheEnd {
public:
  ScopedInjector _injector;

  BL_INLINE PipeInjectAtTheEnd(PipeCompiler* pc) noexcept
    : _injector(pc->cc, &pc->_func_end) {}
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_PIPECOMPILER_P_H_INCLUDED
