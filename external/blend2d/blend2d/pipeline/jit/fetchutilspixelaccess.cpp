// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/fetchutilscoverage_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>

namespace bl::Pipeline::JIT::FetchUtils {

static void satisfy_pixels_a8(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept;
static void satisfy_pixels_rgba32(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept;

// bl::Pipeline::Jit::FetchUtils - Fetch & Store
// =============================================

static uint32_t calculate_vec_count(uint32_t vec_size, uint32_t n) noexcept {
  uint32_t shift = IntOps::ctz(vec_size);
  return (n + vec_size - 1) >> shift;
}

#if defined(BL_JIT_ARCH_A64)
// Provides a specialized AArch64 implementation of a byte granularity vector fetch/store.
static void fetch_vec8_aarrch64(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, uint32_t n, AdvanceMode advance_mode) noexcept {
  BackendCompiler* cc = pc->cc;
  uint32_t i = 0;

  if (advance_mode == AdvanceMode::kNoAdvance) {
    while (i < n) {
      uint32_t idx = i / 16u;
      uint32_t remaining = n - i;

      if (remaining >= 32u) {
        cc->ldp(d_vec[idx], d_vec[idx + 1], a64::ptr(s_ptr, int32_t(i)));
        i += 32;
      }
      else {
        uint32_t count = bl_min<uint32_t>(n - i, 16u);
        pc->v_load_iany(d_vec[idx], a64::ptr(s_ptr, int32_t(i)), count, Alignment(1));
        i += count;
      }
    }
  }
  else {
    while (i < n) {
      uint32_t idx = i / 16u;
      uint32_t remaining = n - i;

      if (remaining >= 32u) {
        cc->ldp(d_vec[idx], d_vec[idx + 1], a64::ptr_post(s_ptr, 32));
        i += 32;
      }
      else {
        uint32_t count = bl_min<uint32_t>(n - i, 16u);
        pc->v_load_iany(d_vec[idx], mem_ptr(s_ptr), count, Alignment(1));
        pc->add(s_ptr, s_ptr, count);

        i += count;
      }
    }
  }
}

static void store_vec8_aarrch64(PipeCompiler* pc, const Gp& d_ptr, const VecArray& s_vec, uint32_t n, AdvanceMode advance_mode) noexcept {
  BackendCompiler* cc = pc->cc;
  uint32_t i = 0;

  if (advance_mode == AdvanceMode::kNoAdvance) {
    while (i < n) {
      uint32_t idx = i / 16u;
      uint32_t remaining = n - i;

      if (remaining >= 32u) {
        cc->stp(s_vec[idx], s_vec[idx + 1], a64::ptr(d_ptr, int32_t(i)));
        i += 32;
      }
      else {
        uint32_t count = bl_min<uint32_t>(n - i, 16u);
        pc->v_load_iany(s_vec[idx], a64::ptr(d_ptr, int32_t(i)), count, Alignment(1));
        i += count;
      }
    }
  }
  else {
    while (i < n) {
      uint32_t idx = i / 16u;
      uint32_t remaining = n - i;

      if (remaining >= 32u) {
        cc->stp(s_vec[idx], s_vec[idx + 1], a64::ptr_post(d_ptr, 32));
        i += 32;
      }
      else {
        uint32_t count = bl_min<uint32_t>(n - i, 16u);
        pc->v_load_iany(s_vec[idx], mem_ptr(d_ptr), count, Alignment(1));
        pc->add(d_ptr, d_ptr, count);

        i += count;
      }
    }
  }
}
#endif // BL_JIT_ARCH_A64

void fetch_vec8(PipeCompiler* pc, const VecArray& d_vec_, Gp s_ptr, uint32_t n, AdvanceMode advance_mode) noexcept {
  VecArray d_vec(d_vec_);
  d_vec.truncate(calculate_vec_count(d_vec[0].size(), n));

  BL_ASSERT(!d_vec.is_empty());

#if defined(BL_JIT_ARCH_A64)
  fetch_vec8_aarrch64(pc, d_vec, s_ptr, n, advance_mode);
#else
  uint32_t offset = 0;

  for (size_t idx = 0; idx < d_vec.size(); idx++) {
    uint32_t remaining = n - offset;
    uint32_t fetch_size = bl_min<uint32_t>(d_vec[idx].size(), remaining);

    pc->v_load_iany(d_vec[idx], mem_ptr(s_ptr, int32_t(offset)), fetch_size, Alignment(1));
    offset += fetch_size;

    if (offset >= n)
      break;
  }

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add(s_ptr, s_ptr, n);
  }
#endif
}

void store_vec8(PipeCompiler* pc, const Gp& d_ptr, const VecArray& s_vec_, uint32_t n, AdvanceMode advance_mode) noexcept {
  VecArray s_vec(s_vec_);
  s_vec.truncate(calculate_vec_count(s_vec[0].size(), n));

  BL_ASSERT(!s_vec.is_empty());

#if defined(BL_JIT_ARCH_A64)
  store_vec8_aarrch64(pc, d_ptr, s_vec, n, advance_mode);
#else
  uint32_t offset = 0;

  for (size_t idx = 0; idx < s_vec.size(); idx++) {
    uint32_t remaining = n - offset;
    uint32_t store_size = bl_min<uint32_t>(s_vec[idx].size(), remaining);

    pc->v_store_iany(mem_ptr(d_ptr, int32_t(offset)), s_vec[idx], store_size, Alignment(1));
    offset += store_size;

    if (offset >= n)
      break;
  }

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add(d_ptr, d_ptr, n);
  }
#endif
}

void fetch_vec32(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode) noexcept {
  fetch_vec8(pc, d_vec, s_ptr, n * 4u, advance_mode);
}

void store_vec32(PipeCompiler* pc, const Gp& d_ptr, const VecArray& s_vec, uint32_t n, AdvanceMode advance_mode) noexcept {
  store_vec8(pc, d_ptr, s_vec, n * 4u, advance_mode);
}

void fetch_vec8(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  if (predicate.is_empty())
    fetch_vec8(pc, d_vec, s_ptr, n, advance_mode);
  else
    fetch_predicated_vec8(pc, d_vec, s_ptr, n, advance_mode, predicate);
}

void fetch_vec32(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  if (predicate.is_empty())
    fetch_vec32(pc, d_vec, s_ptr, n, advance_mode);
  else
    fetch_predicated_vec32(pc, d_vec, s_ptr, n, advance_mode, predicate);
}

void store_vec8(PipeCompiler* pc, const Gp& d_ptr, const VecArray& s_vec, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  if (predicate.is_empty())
    store_vec8(pc, d_ptr, s_vec, n, advance_mode);
  else
    store_predicated_vec8(pc, d_ptr, s_vec, n, advance_mode, predicate);
}

void store_vec32(PipeCompiler* pc, const Gp& d_ptr, const VecArray& s_vec, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  if (predicate.is_empty())
    store_vec32(pc, d_ptr, s_vec, n, advance_mode);
  else
    store_predicated_vec32(pc, d_ptr, s_vec, n, advance_mode, predicate);
}

// bl::Pipeline::Jit::FetchUtils - Fetch Miscellaneous
// ===================================================

void fetch_second_32bit_element(PipeCompiler* pc, const Vec& vec, const Mem& src) noexcept {
#if defined(BL_JIT_ARCH_X86)
  if (!pc->has_sse4_1()) {
    Vec tmp = pc->new_vec128("@tmp");
    pc->v_loadu32(tmp, src);
    pc->v_interleave_lo_u32(vec, vec, tmp);
  }
  else
#endif
  {
    pc->v_insert_u32(vec, src, 1);
  }
}

void fetch_third_32bit_element(PipeCompiler* pc, const Vec& vec, const Mem& src) noexcept {
#if defined(BL_JIT_ARCH_X86)
  if (!pc->has_sse4_1()) {
    Vec tmp = pc->new_vec128("@tmp");
    pc->v_loadu32(tmp, src);
    pc->v_interleave_lo_u64(vec, vec, tmp);
  }
  else
#endif
  {
    pc->v_insert_u32(vec, src, 2);
  }
}

// bl::Pipeline::Jit::FetchUtils - Predicated Fetch
// ================================================

static void add_shifted(PipeCompiler* pc, const Gp& dst, const Gp& src, uint32_t shift) noexcept {
#if defined(BL_JIT_ARCH_X86)
  pc->shl(src, src, shift);
  pc->add(dst, dst, src);
#else
  pc->add_scaled(dst, src, 1 << shift);
#endif
};

static void fetch_predicated_vec8_1to3(PipeCompiler* pc, const Vec& d_vec, Gp s_ptr, AdvanceMode advance_mode, const Gp& count) noexcept {
#if defined(BL_JIT_ARCH_A64)
  // Predicated load of 1-3 elements can be simplified to the following on AArch64:
  //   - load the first element at [0]    (always valid).
  //   - load the last element at [i - 1] (always valid, possibly overlapping with the first element if count==2).
  //   - load the mid element by using CINC instruction (incrementing when count >= 2).
  a64::Compiler* cc = pc->cc;

  Gp mid = pc->new_gpz("@mid");
  Gp last = pc->new_gpz("@last");

  cc->cmp(count, 2);
  cc->cinc(mid, s_ptr, CondCode::kUnsignedGE);

  if (advance_mode == AdvanceMode::kAdvance) {
    cc->ld1r(d_vec.b16(), a64::ptr_post(s_ptr, count.clone_as(s_ptr)));
  }
  else {
    cc->ldr(d_vec.b(), a64::ptr(s_ptr));
  }

  cc->ld1(d_vec.b(1), a64::ptr(mid));
  cc->cinc(last, mid, CondCode::kUnsignedGT);
  cc->ld1(d_vec.b(2), a64::ptr(last));

#else
  Gp acc = pc->new_gp32("@acc");
  Gp tmp = pc->new_gp32("@tmp");
  Gp mid = pc->new_gpz("@mid");

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->load_u8(acc, mem_ptr(s_ptr));
    pc->add(mid, s_ptr, 2);
    pc->add(s_ptr, s_ptr, count.clone_as(s_ptr));
    pc->load_u8(tmp, mem_ptr(s_ptr, -1));
    pc->umin(mid, mid, s_ptr);
    add_shifted(pc, acc, tmp, 16);
    pc->load_u8(tmp, mem_ptr(mid, -1));
    add_shifted(pc, acc, tmp, 8);
  }
  else {
    Gp end = pc->new_gpz("@end");

    pc->add(end, s_ptr, count.clone_as(s_ptr));
    pc->load_u8(tmp, mem_ptr(end, -1));
    pc->load_u8(acc, mem_ptr(s_ptr));
    add_shifted(pc, acc, tmp, 16);

    pc->add(mid, s_ptr, 2);
    pc->umin(mid, mid, end);
    pc->load_u8(tmp, mem_ptr(mid, -1));
    add_shifted(pc, acc, tmp, 8);
  }

  pc->s_mov_u32(d_vec, acc);
#endif
}

// Predicated load of 1-7 bytes.
static void fetch_predicated_vec8_1to7(PipeCompiler* pc, const Vec& d_vec, Gp s_ptr, AdvanceMode advance_mode, const Gp& count) noexcept {
#if defined(BL_JIT_ARCH_X86)
  if (pc->is_32bit()) {
    // Not optimized, probably not worth spending time on trying to optimize this version as we don't expect 32-bit
    // targets to be important.
    Label L_Iter = pc->new_label();
    Label L_Done = pc->new_label();

    Gp i = pc->new_gp32("@fetch_x");
    Gp acc = pc->new_gp32("@fetch_acc");
    Vec tmp = pc->new_vec128("@fetch_tmp");

    pc->mov(i, count);
    pc->mov(acc, 0);
    pc->v_xor_i32(d_vec, d_vec, d_vec);
    pc->j(L_Iter, ucmp_lt(i, 4));

    pc->v_loadu32(d_vec, x86::ptr(s_ptr, i, 0, -4));
    pc->j(L_Done, sub_z(i, 4));

    pc->bind(L_Iter);
    pc->load_shift_u8(acc, x86::ptr(s_ptr, i, 0, -1));
    pc->v_slli_u64(d_vec, d_vec, 8);
    pc->j(L_Iter, sub_nz(i, 1));

    pc->bind(L_Done);
    pc->s_mov(tmp, acc);
    pc->v_or_i32(d_vec, d_vec, tmp);

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, count.clone_as(s_ptr));
    }

    return;
  }
#endif // BL_JIT_ARCH_X86

  // This implementation uses a single branch to skip the loading of the rest when `count == 1`. The reason is that we
  // want to use 3x 16-bit fetches to fetch 2..6 bytes, and combine that with the first byte if `count & 1 == 1`. This
  // approach seems to be good and it's also pretty short. Since the branch depends on `count == 1` it should also make
  // branch predictor happier as we expect that `count == 2..7` case should be much more likely than `count == 1`.
  Label L_Done = pc->new_label();

  Gp acc = pc->new_gpz("@fetch_acc");
  Gp index0 = pc->new_gpz("@fetch_index0");
  Gp index1 = pc->new_gpz("@fetch_index1");

  pc->load_u8(acc, ptr(s_ptr));
  pc->j(L_Done, cmp_eq(count.r32(), 1));

  // This is how indexes are calculated for count:
  //   - count == 2 -> index0 = 0 | index0' = 0 | index1 = 0
  //   - count == 3 -> index0 = 1 | index0' = 1 | index1 = 1
  //   - count == 4 -> index0 = 2 | index0' = 2 | index1 = 0
  //   - count == 5 -> index0 = 3 | index0' = 3 | index1 = 1
  //   - count == 6 -> index0 = 4 | index0' = 2 | index1 = 0
  //   - count == 7 -> index0 = 5 | index0' = 3 | index1 = 1
  pc->shl(acc, acc, 24);
  pc->sub(index0.r32(), count.r32(), 2);
  pc->and_(index1.r32(), count.r32(), 0x1);
  pc->load_merge_u16(acc, ptr(s_ptr, index0));

  pc->add(index1.r32(), index1.r32(), 2);
  pc->umin(index0.r32(), index0.r32(), index1.r32());
  pc->load_shift_u16(acc, ptr(s_ptr, index0));

  pc->and_(index1.r32(), index1.r32(), 1);
  pc->load_shift_u16(acc, ptr(s_ptr, index1));

  pc->shl(index1.r32(), index1.r32(), 3);
  pc->rol(acc, acc, index1.r64());

  pc->bind(L_Done);
  pc->s_mov_u64(d_vec, acc);

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add(s_ptr, s_ptr, count.clone_as(s_ptr));
  }
}

static void fetch_predicated_vec8_4to15(PipeCompiler* pc, const Vec& d_vec, Gp s_ptr, AdvanceMode advance_mode, const Gp& count) noexcept {
  Gp end = pc->new_gpz("@end");

#if defined(BL_JIT_ARCH_X86)
  if (!pc->has_sse3()) {
    BackendCompiler* cc = pc->cc;

    Vec acc = pc->new_vec128("@acc");
    Vec tmp = pc->new_vec128("@tmp");
    Gp shift = pc->new_gp32("@shift");

    Label L_Done = pc->new_label();
    Label L_LessThan8 = pc->new_label();

    pc->neg(shift, count.clone_as(shift));
    pc->shl(shift, shift, 3);
    pc->j(L_LessThan8, ucmp_lt(count, 8));

    pc->add(shift, shift, 16 * 8);
    pc->v_loadu64(d_vec, mem_ptr(s_ptr));
    pc->s_mov_u32(tmp, shift);

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, count.clone_as(s_ptr));
      pc->v_loadu64(acc, x86::ptr(s_ptr, -8));
    }
    else {
      pc->v_loadu64(acc, x86::ptr(s_ptr, count.clone_as(s_ptr), 0, -8));
    }

    cc->psrlq(acc, tmp);
    pc->v_interleave_lo_u64(d_vec, d_vec, acc);
    pc->j(L_Done);

    pc->bind(L_LessThan8);
    pc->add(shift, shift, 8 * 8);
    pc->v_loadu32(d_vec, mem_ptr(s_ptr));
    pc->s_mov_u32(tmp, shift);

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, count.clone_as(s_ptr));
      pc->v_loadu32(acc, x86::ptr(s_ptr, -4));
    }
    else {
      pc->v_loadu32(acc, x86::ptr(s_ptr, count.clone_as(s_ptr), 0, -4));
    }

    cc->psrld(acc, tmp);
    pc->v_interleave_lo_u32(d_vec, d_vec, acc);

    pc->bind(L_Done);
    return;
  }
#endif // BL_JIT_ARCH_X86

  // Common implementation that targets both X86 and AArch64.
  Vec v_pred = pc->new_vec128("@pred");
  Mem m_pred = pc->simd_mem_const(pc->ct<CommonTable>().swizu8_load_tail_0_to_16, Bcst::kNA_Unique, v_pred);
  m_pred.set_index(count.clone_as(s_ptr));

#if defined(BL_JIT_ARCH_X86)
  // Temporaries needed to compose a single 128-bit vector from 4 32-bit elements.
  Vec tmp0;
  Vec tmp1;

  if (!pc->has_sse4_1()) {
    tmp0 = pc->new_vec128("tmp0");
    tmp1 = pc->new_vec128("tmp1");
  }
#endif // BL_JIT_ARCH_X86

  auto&& fetch_next_32 = [&](const Gp& src, uint32_t i) noexcept {
    Mem p = mem_ptr(src);
#if defined(BL_JIT_ARCH_X86)
    if (!pc->has_sse4_1()) {
      switch (i) {
        case 1:
          pc->v_loadu32(tmp0, p);
          pc->v_interleave_lo_u32(d_vec, d_vec, tmp0);
          break;
        case 2:
          pc->v_loadu32(tmp0, p);
          break;
        case 3:
          pc->v_loadu32(tmp1, p);
          pc->v_interleave_lo_u32(tmp0, tmp0, tmp1);
          pc->v_interleave_lo_u64(d_vec, d_vec, tmp0);
          break;
      }
      return;
    }
#endif // BL_JIT_ARCH_X86
    pc->v_insert_u32(d_vec, p, i);
  };

  pc->v_loadu32(d_vec, mem_ptr(s_ptr));
  pc->add_ext(end, s_ptr, count.clone_as(s_ptr), 1, -4);
  pc->v_loada128(v_pred, m_pred);

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add(s_ptr, s_ptr, 4);
    pc->umin(s_ptr, s_ptr, end);
    fetch_next_32(s_ptr, 1);

    pc->add(s_ptr, s_ptr, 4);
    pc->umin(s_ptr, s_ptr, end);
    fetch_next_32(s_ptr, 2);

    pc->add(s_ptr, s_ptr, 4);
    pc->umin(s_ptr, s_ptr, end);
    fetch_next_32(s_ptr, 3);

    pc->add(s_ptr, s_ptr, 4);
  }
  else {
    Gp mid = pc->new_gpz("@mid");

    pc->add(mid, s_ptr, 4);
    pc->umin(mid, mid, end);
    fetch_next_32(mid, 1);

    pc->add(mid, s_ptr, 8);
    pc->umin(mid, mid, end);
    fetch_next_32(mid, 2);

    pc->add(mid, s_ptr, 12);
    pc->umin(mid, mid, end);
    fetch_next_32(mid, 3);
  }

  pc->v_swizzlev_u8(d_vec, d_vec, v_pred);
}

static void fetch_predicated_vec8_1to15(PipeCompiler* pc, const Vec& d_vec, Gp s_ptr, AdvanceMode advance_mode, const Gp& count) noexcept {
  Label L_LessThan4 = pc->new_label();
  Label L_Done = pc->new_label();

  pc->j(L_LessThan4, ucmp_lt(count, 4));
  fetch_predicated_vec8_4to15(pc, d_vec, s_ptr, advance_mode, count);
  pc->j(L_Done);

  pc->bind(L_LessThan4);
  fetch_predicated_vec8_1to3(pc, d_vec, s_ptr, advance_mode, count);
  pc->bind(L_Done);
}

static void fetch_predicated_vec8_v128(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  size_t vec_count = d_vec.size();
  Gp count = predicate.count();

  // Handle small cases first.
  if (n <= 2) {
    // Never empty & never full -> there is exactly a single element to load.
    pc->v_load8(d_vec[0], mem_ptr(s_ptr));

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, predicate.count().clone_as(s_ptr));
    }
  }
  else if (n <= 4) {
    fetch_predicated_vec8_1to3(pc, d_vec[0], s_ptr, advance_mode, count);
  }
  else if (n <= 8) {
    fetch_predicated_vec8_1to7(pc, d_vec[0], s_ptr, advance_mode, count);
  }
  else if (n <= 16) {
    fetch_predicated_vec8_1to15(pc, d_vec[0], s_ptr, advance_mode, count);
  }
  else {
    BL_ASSERT(vec_count > 1);

    // TODO: [JIT] UNIMPLEMENTED: Predicated fetch - multiple vector registers.
    bl_unused(vec_count);
    BL_NOT_REACHED();
  }
}

#if defined(BL_JIT_ARCH_X86)
static void fetch_predicated_vec8_avx(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  size_t vec_count = d_vec.size();

  if (n <= 4) {
    fetch_predicated_vec8_1to3(pc, d_vec[0], s_ptr, advance_mode, predicate.count());
    return;
  }

  if (n <= 8) {
    fetch_predicated_vec8_1to7(pc, d_vec[0], s_ptr, advance_mode, predicate.count());
    return;
  }

  BackendCompiler* cc = pc->cc;
  InstId load_inst_id = pc->has_avx2() ? x86::Inst::kIdVpmaskmovd : x86::Inst::kIdVmaskmovps;
  size_t vec_element_count = d_vec[0].size() / 4u;

  Label L_LessThan4 = pc->new_label();
  Label L_Done = pc->new_label();

  Gp count = predicate.count();
  Gp countDiv4 = pc->new_gp32("@countDiv4");
  Gp tail_pixels = pc->new_gp32("@tail_pixels");
  Gp tail_shift = pc->new_gp32("@tail_shift");

  Vec v_tail = pc->new_similar_reg(d_vec[0], "v_tail");
  Vec v_pred = pc->new_similar_reg(d_vec[0], "v_pred");
  Mem m_pred = pc->simd_mem_const(pc->ct<CommonTable>().loadstore16_lo8_msk8(), Bcst::kNA_Unique, v_pred);
  m_pred.set_index(countDiv4.clone_as(s_ptr));

  pc->j(L_LessThan4, ucmp_lt(count, 4));
  pc->neg(tail_shift, count.r32());
  pc->shl(tail_shift, tail_shift, 3);
  pc->load_u32(tail_pixels, x86::ptr(s_ptr, count.clone_as(s_ptr), 0, -4));
  pc->shr(tail_pixels, tail_pixels, tail_shift);
  pc->shr(countDiv4, count, 2);
  pc->v_broadcast_u32(v_tail, tail_pixels);

  Mem s_mem = mem_ptr(s_ptr);
  for (size_t i = 0; i < vec_count; i++) {
    cc->vpmovsxbd(v_pred, m_pred);
    cc->emit(load_inst_id, d_vec[i], v_pred, s_mem);
    cc->vpblendvb(d_vec[i], v_tail, d_vec[i], v_pred);

    s_mem.add_offset(int32_t(d_vec[i].size()));
    m_pred.add_offset(-int32_t(vec_element_count * 8));
  }

  pc->j(L_Done);

  pc->bind(L_LessThan4);
  fetch_predicated_vec8_1to3(pc, d_vec[0], s_ptr, AdvanceMode::kNoAdvance, count);
  for (size_t i = 1; i < vec_count; i++) {
    pc->v_zero_i(d_vec[i]);
  }

  pc->bind(L_Done);

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add(s_ptr, s_ptr, count.clone_as(s_ptr));
  }
}

static void fetch_predicated_vec8_avx512(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  size_t vec_count = d_vec.size();
  size_t vec_element_count = d_vec[0].size();

  Gp count = predicate.count();

  if (vec_count == 1u) {
    pc->v_load_predicated_u8(d_vec[0], mem_ptr(s_ptr), n, predicate);

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, count.clone_as(s_ptr));
    }
  }
  else {
    BL_ASSERT(n >= 64);
    BL_ASSERT(d_vec[0].is_vec512());

    BackendCompiler* cc = pc->cc;

    Gp n_mov = pc->new_gpz("n_mov");
    Gp n_pred = pc->new_gpz("n_pred");
    KReg kPred = cc->new_kq("kPred");

    if (vec_element_count <= 32 || pc->is_64bit()) {
      // NOTE: BHZI instruction is used to create the load mask. It's a pretty interesting instruction as unlike others
      // it uses 8 bits of index, which are basically saturated to OperandSize. This is great for our use as the maximum
      // registers we load is 4, which is 256-1 bytes total (we decrement one byte as predicated is not intended to load
      // ALL bytes).
      //
      // Additionally, we use POPCNT to count bits in the mask, which are then used to decrement n_pred and possibly
      // increment the source pointer.
      Gp gp_pred = pc->new_gpz("gp_pred");
      pc->mov(gp_pred, -1);
      pc->mov(n_pred.clone_as(count), count);

      for (size_t i = 0; i < vec_count; i++) {
        Gp n_dec = n_pred;
        if (i != vec_count - 1) {
          n_dec = n_mov;
        }

        if (vec_element_count == 64u) {
          cc->bzhi(gp_pred, gp_pred, n_pred);
          cc->kmovq(kPred, gp_pred);
          if (i != vec_count - 1) {
            cc->popcnt(n_mov, gp_pred);
          }
        }
        else if (vec_element_count == 32u) {
          cc->bzhi(gp_pred.r32(), gp_pred.r32(), n_pred.r32());
          cc->kmovd(kPred, gp_pred.r32());
          if (i != vec_count - 1) {
            cc->popcnt(n_mov.r32(), gp_pred.r32());
          }
        }
        else {
          cc->bzhi(gp_pred.r32(), gp_pred.r32(), n_pred.r32());
          cc->kmovw(kPred, gp_pred.r32());
          if (i != vec_count - 1) {
            cc->movzx(n_mov.r32(), gp_pred.r16());
            cc->popcnt(n_mov.r32(), n_mov.r32());
          }
        }

        if (advance_mode == AdvanceMode::kAdvance) {
          cc->k(kPred).z().vmovdqu8(d_vec[i], mem_ptr(s_ptr));
          cc->add(s_ptr, n_dec);
        }
        else {
          cc->k(kPred).z().vmovdqu8(d_vec[i], mem_ptr(s_ptr, int32_t(i * vec_element_count)));
        }

        if (i < vec_count - 1u) {
          cc->sub(n_pred, n_dec);
        }
      }
    }
    else {
      x86::Mem mem = pc->_get_mem_const(pc->ct<CommonTable>().k_msk64_data);
      mem.set_index(n_mov);
      mem.set_shift(3);
      pc->mov(n_pred.clone_as(count), count);

      for (size_t i = 0; i < vec_count; i++) {
        pc->umin(n_mov, n_pred, vec_element_count);

        if (vec_element_count == 64u)
          cc->kmovq(kPred, mem);
        else if (vec_element_count == 32u)
          cc->kmovd(kPred, mem);
        else
          cc->kmovw(kPred, mem);

        if (advance_mode == AdvanceMode::kAdvance) {
          cc->k(kPred).z().vmovdqu8(d_vec[i], mem_ptr(s_ptr));
          cc->add(s_ptr, n_mov);
        }
        else {
          cc->k(kPred).z().vmovdqu8(d_vec[i], mem_ptr(s_ptr, int32_t(i * vec_element_count)));
        }

        if (i < vec_count - 1u) {
          cc->sub(n_pred, n_mov);
        }
      }
    }
  }
}

static void fetch_predicated_vec32_avx(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  size_t vec_count = d_vec.size();
  size_t vec_element_count = d_vec[0].size() / 4u;

  Gp count = predicate.count();
  Mem s_mem = mem_ptr(s_ptr);

  if (vec_count == 1u) {
    pc->v_load_predicated_u32(d_vec[0], s_mem, n, predicate);
  }
  else {
    BackendCompiler* cc = pc->cc;
    InstId load_inst_id = pc->has_avx2() ? x86::Inst::kIdVpmaskmovd : x86::Inst::kIdVmaskmovps;

    Vec v_pred = pc->new_similar_reg(d_vec[0], "v_pred");
    Mem m_pred = pc->simd_mem_const(pc->ct<CommonTable>().loadstore16_lo8_msk8(), Bcst::kNA_Unique, v_pred);
    m_pred.set_index(count.clone_as(s_ptr), 3);

    for (size_t i = 0; i < vec_count; i++) {
      cc->vpmovsxbd(v_pred, m_pred);
      cc->emit(load_inst_id, d_vec[i], v_pred, s_mem);

      s_mem.add_offset(int32_t(d_vec[i].size()));
      m_pred.add_offset(-int32_t(vec_element_count * 8));
    }
  }

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add_scaled(s_ptr, count.clone_as(s_ptr), 4u);
  }
}

static void fetch_predicated_vec32_avx512(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  size_t vec_count = d_vec.size();
  size_t vec_element_count = d_vec[0].size() / 4u;

  Gp count = predicate.count();
  Mem s_mem = mem_ptr(s_ptr);

  if (vec_count == 1u) {
    pc->v_load_predicated_u32(d_vec[0], s_mem, n, predicate);
  }
  else {
    BackendCompiler* cc = pc->cc;
    Gp gp_pred;
    KReg kPred;

    if (vec_count <= 2u) {
      gp_pred = pc->new_gp32("gp_pred");
      kPred = cc->new_kd("kPred");
    }
    else {
      gp_pred = pc->new_gp64("gp_pred");
      kPred = cc->new_kq("kPred");
    }

    pc->mov(gp_pred, -1);
    cc->bzhi(gp_pred, gp_pred, count.clone_as(gp_pred));

    if (vec_count <= 2u)
      cc->kmovd(kPred, gp_pred);
    else
      cc->kmovq(kPred, gp_pred);

    for (size_t i = 0; i < vec_count; i++) {
      cc->k(kPred).z().vmovdqu32(d_vec[i], s_mem);
      s_mem.add_offset(int32_t(d_vec[i].size()));

      if (i + 1u != vec_count) {
        if (vec_count <= 2u)
          cc->kshiftrd(kPred, kPred, vec_element_count);
        else
          cc->kshiftrq(kPred, kPred, vec_element_count);
      }
    }
  }

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add_scaled(s_ptr, count.clone_as(s_ptr), 4u);
  }
}
#endif // BL_JIT_ARCH_X86

// The following code implements fetching 128-bit vectors without any kind of hardware support. We employ two
// strategies. If the number of vectors to fetch is greater than 1 we branch to the implementation depending
// on whether we can fetch at least one FULL vector - and then we fetch the rest without branches. If we cannot
// fetch a FULL vector, we would use branches to fetch individual lanes.
static void fetch_predicated_vec32_v128(PipeCompiler* pc, const VecArray& d_vec, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  bl_unused(n);

  size_t vec_count = d_vec.size();

  Gp count = predicate.count();

  // Zero all vectors except the first, which is guaranteed to be modified by the fetch.
  //
  // NOTE: We have to zero the registers as otherwise they would contain garbage, which would then be processed.
  // The garbage is actually only the part of the problem - much bigger problem would be AsmJit not being to
  // compute exact liveness, which could possible make the life of d_vec[1..N] to span across most of the function.
  for (size_t i = 1u; i < vec_count; i++) {
    pc->v_zero_i(d_vec[i]);
  }

  Label L_Done;

  Gp adjusted1 = pc->new_gpz("@adjusted1");
  Gp adjusted2 = pc->new_gpz("@adjusted2");

  pc->add_ext(adjusted2, s_ptr, count.clone_as(s_ptr), 4, -4);

  if (vec_count > 1u) {
    // TODO: [JIT] UNIMPLEMENTED: Not expected to have more than 2 - 2 vectors would be unpacked to 4, which is the limit.
    BL_ASSERT(vec_count == 2);

    L_Done = pc->new_label();

    Label L_TailOnly = pc->new_label();
    pc->j(L_TailOnly, ucmp_lt(count, 4));

    pc->add(adjusted1, s_ptr, 16);
    pc->umin(adjusted1, adjusted1, adjusted2);

    pc->v_loadu128(d_vec[0], mem_ptr(s_ptr));
    pc->v_loadu32(d_vec[1], mem_ptr(adjusted1));

    pc->add(adjusted1, s_ptr, 20);
    pc->umin(adjusted1, adjusted1, adjusted2);
    fetch_second_32bit_element(pc, d_vec[1], mem_ptr(adjusted1));
    fetch_third_32bit_element(pc, d_vec[1], mem_ptr(adjusted2));

    pc->j(L_Done);
    pc->bind(L_TailOnly);
  }

  {
    pc->v_loadu32(d_vec[0], mem_ptr(s_ptr));
    pc->add(adjusted1, s_ptr, 4);
    pc->umin(adjusted1, adjusted1, adjusted2);
    fetch_second_32bit_element(pc, d_vec[0], mem_ptr(adjusted1));
    fetch_third_32bit_element(pc, d_vec[0], mem_ptr(adjusted2));
  }

  if (L_Done.is_valid()) {
    pc->bind(L_Done);
  }

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add(s_ptr, adjusted2, 4);
  }

  predicate.add_materialized_end_ptr(s_ptr, adjusted1, adjusted2);
}

void fetch_predicated_vec8(PipeCompiler* pc, const VecArray& d_vec_, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  // Restrict the number of vectors to match `n` exactly.
  VecArray d_vec(d_vec_);
  d_vec.truncate(calculate_vec_count(d_vec[0].size(), n));

  BL_ASSERT(!d_vec.is_empty());
  BL_ASSERT(n >= 2u);

#if defined(BL_JIT_ARCH_X86)
  if (n <= 16)
    d_vec[0] = d_vec[0].v128();
  else if (n <= 32u && d_vec.size() == 1u)
    d_vec[0] = d_vec[0].v256();

  // Don't spoil the generic implementation with 256-bit and 512-bit vectors. In AVX/AVX2/AVX-512 cases we always
  // want to use masked loads as they are always relatively cheap and should be cheaper than branching or scalar loads.
  if (pc->has_avx512()) {
    fetch_predicated_vec8_avx512(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }

  // Must be XMM/YMM if AVX-512 is not available.
  BL_ASSERT(!d_vec[0].is_vec512());

  if (pc->has_avx()) {
    fetch_predicated_vec8_avx(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }

  // Must be XMM if AVX is not available.
  BL_ASSERT(d_vec[0].is_vec128());
#endif // BL_JIT_ARCH_X86

  fetch_predicated_vec8_v128(pc, d_vec, s_ptr, n, advance_mode, predicate);
}

void fetch_predicated_vec32(PipeCompiler* pc, const VecArray& d_vec_, Gp s_ptr, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  // Restrict the number of vectors to match `n` exactly.
  VecArray d_vec(d_vec_);
  d_vec.truncate(calculate_vec_count(d_vec[0].size(), n * 4u));

  BL_ASSERT(!d_vec.is_empty());
  BL_ASSERT(n >= 2u);

#if defined(BL_JIT_ARCH_X86)
  if (n <= 4)
    d_vec[0] = d_vec[0].v128();
  else if (n <= 8u && d_vec.size() == 1u)
    d_vec[0] = d_vec[0].v256();

  // Don't spoil the generic implementation with 256-bit and 512-bit vectors. In AVX/AVX2/AVX-512 cases we always
  // want to use masked loads as they are always relatively cheap and should be cheaper than branching or scalar loads.
  if (pc->has_avx512()) {
    fetch_predicated_vec32_avx512(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }

  // Must be XMM/YMM if AVX-512 is not available.
  BL_ASSERT(!d_vec[0].is_vec512());

  if (pc->has_avx()) {
    fetch_predicated_vec32_avx(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }

  // Must be XMM if AVX is not available.
  BL_ASSERT(d_vec[0].is_vec128());
#endif // BL_JIT_ARCH_X86

  fetch_predicated_vec32_v128(pc, d_vec, s_ptr, n, advance_mode, predicate);
}

// bl::Pipeline::Jit::FetchUtils - Predicated Store
// ================================================

#if defined(BL_JIT_ARCH_X86)
static void store_predicated_vec8_avx512(PipeCompiler* pc, Gp d_ptr, VecArray s_vec, uint32_t n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  BackendCompiler* cc = pc->cc;

  size_t vec_count = s_vec.size();
  size_t vec_element_count = s_vec[0].size();

  Gp count = predicate.count();

  // If there is a multiple of input vectors and they are not ZMMs, convert to ZMMs first so we can use as little
  // writes as possible. We are compiling for AVX-512 machine so we have 512-bit SIMD.
  if (vec_count > 1u) {
    if (s_vec[0].is_vec128()) {
      Vec v256 = pc->new_vec512("@store_256");
      if (vec_count == 4u) {
        Vec v512 = pc->new_vec512("@store_512");

        pc->v_insert_v128(v512.ymm(), s_vec[0].ymm(), s_vec[1].xmm(), 1);
        pc->v_insert_v128(v256.ymm(), s_vec[2].ymm(), s_vec[3].xmm(), 1);
        pc->v_insert_v256(v512, v512, v256, 1);

        s_vec.init(v512);
        vec_count = 1;
        vec_element_count = 64;
      }
      else if (vec_count == 2u) {
        pc->v_insert_v128(v256, s_vec[0].ymm(), s_vec[1].xmm(), 1);

        s_vec.init(v256);
        vec_count = 1;
        vec_element_count = 32;
      }
      else {
        // 3 elements? No...
        BL_NOT_REACHED();
      }
    }
    else if (s_vec[0].is_vec256()) {
      VecArray new_vec;
      size_t new_count = (vec_count + 1u) / 2u;

      pc->new_vec_array(new_vec, new_count, VecWidth::k512, "@store_vec");
      pc->v_insert_v256(new_vec, s_vec.even(), s_vec.odd(), 1);

      s_vec = new_vec;
      vec_count = new_count;
      vec_element_count = 64;
    }
  }

  // Simplified case used when there is only one vector to store.
  if (vec_count == 1u) {
    pc->v_store_predicated_u8(mem_ptr(d_ptr), s_vec[0], n, predicate);
    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(d_ptr, d_ptr, count.clone_as(d_ptr));
    }
    return;
  }

  // Predicated writes are very expensive on all modern HW due to store forwarding. In general we want to minimize
  // the number of write operations that involve predication so we try to store as many vectors as possible by using
  // regular stores. This complicates the code a bit, but improved the performance on all the hardware tested.
  Vec v_tail = pc->new_similar_reg(s_vec[0], "@v_tail");
  Gp remaining = pc->new_gpz("@remaining");

  if (advance_mode == AdvanceMode::kNoAdvance) {
    Gp dPtrCopy = pc->new_similar_reg(d_ptr, "@dPtrCopy");
    pc->mov(dPtrCopy, d_ptr);
    d_ptr = dPtrCopy;
  }

  Label L_Tail = pc->new_label();
  Label L_Done = pc->new_label();

  pc->mov(remaining.r32(), count.r32());
  for (size_t i = 0; i < vec_count - 1; i++) {
    pc->v_mov(v_tail, s_vec[i]);
    pc->j(L_Tail, sub_c(remaining.r32(), vec_element_count));
    pc->v_store_iany(mem_ptr(d_ptr), s_vec[i], vec_element_count, Alignment(1));
    pc->add(d_ptr, d_ptr, vec_element_count);
  }
  pc->v_mov(v_tail, s_vec[vec_count - 1]);

  pc->bind(L_Tail);
  pc->j(L_Done, add_z(remaining.r32(), vec_element_count));
  KReg kPred = pc->make_mask_predicate(predicate, vec_element_count, remaining);
  cc->k(kPred).vmovdqu8(mem_ptr(d_ptr), v_tail);

  pc->bind(L_Done);
}
#endif // BL_JIT_ARCH_X86

void store_predicated_vec8(PipeCompiler* pc, const Gp& d_ptr_, const VecArray& s_vec_, uint32_t n, AdvanceMode advance_mode_, PixelPredicate& predicate) noexcept {
  // Restrict the number of vectors to match `n` exactly.
  VecArray s_vec(s_vec_);
  s_vec.truncate(calculate_vec_count(s_vec[0].size(), n));

  AdvanceMode advance_mode(advance_mode_);

  BL_ASSERT(!s_vec.is_empty());
  BL_ASSERT(n >= 2u);

#if defined(BL_JIT_ARCH_X86)
  if (n <= 16u) {
    s_vec[0] = s_vec[0].v128();
  }
  else if (n <= 32u && s_vec.size() == 1u) {
    s_vec[0] = s_vec[0].v256();
  }

  if (pc->has_avx512()) {
    store_predicated_vec8_avx512(pc, d_ptr_, s_vec, n, advance_mode, predicate);
    return;
  }
#endif // BL_JIT_ARCH_X86

  Gp d_ptr = d_ptr_;
  Gp count = predicate.count();

  Mem d_mem = mem_ptr(d_ptr);
  size_t size_minus_one = s_vec.size() - 1u;

  Vec v_last = s_vec[size_minus_one];
  bool tail_can_be_empty = false;

  size_t remaining = n;
  size_t element_count = v_last.size();

  auto makeDPtrCopy = [&]() noexcept {
    BL_ASSERT(advance_mode == AdvanceMode::kNoAdvance);

    d_ptr = pc->new_similar_reg(d_ptr, "@dPtrCopy");
    advance_mode = AdvanceMode::kAdvance;

    pc->mov(d_ptr, d_ptr_);
  };

  if (size_minus_one || !v_last.is_vec128()) {
    count = pc->new_similar_reg(count, "@count");
    v_last = pc->new_similar_reg(v_last, "@v_last");
    tail_can_be_empty = true;

    pc->mov(count, predicate.count());
    pc->v_mov(v_last, s_vec[0]);

    if (advance_mode == AdvanceMode::kNoAdvance)
      makeDPtrCopy();
  }

  // Process whole vectors in case that there is more than one vector in `s_vec`.
  if (size_minus_one) {
    Label L_Tail = pc->new_label();
    size_t required_count = element_count;

    for (size_t i = 0; i < size_minus_one; i++) {
      pc->j(L_Tail, ucmp_lt(count, required_count));
      pc->v_storeuvec_u32(d_mem, s_vec[i]);
      pc->add(d_ptr, d_ptr, v_last.size());
      pc->v_mov(v_last, s_vec[i + 1]);

      BL_ASSERT(remaining >= element_count);
      remaining -= element_count;
      required_count += element_count;
    }

    pc->bind(L_Tail);
  }

#if defined(BL_JIT_ARCH_X86)
  if (v_last.is_vec512()) {
    BL_ASSERT(remaining > 32u);

    Label L_StoreSkip32 = pc->new_label();
    pc->j(L_StoreSkip32, bt_z(count, 5));
    pc->v_storeu256(d_mem, v_last.ymm());
    pc->v_extract_v256(v_last.ymm(), v_last, 1);
    pc->add(d_ptr, d_ptr, 32);
    pc->bind(L_StoreSkip32);

    v_last = v_last.ymm();
    remaining -= 32u;
  }

  if (v_last.is_vec256()) {
    BL_ASSERT(remaining > 16u);

    Label L_StoreSkip16 = pc->new_label();
    pc->j(L_StoreSkip16, bt_z(count, 4));
    pc->v_storeu128(d_mem, v_last.xmm());
    pc->v_extract_v128(v_last.xmm(), v_last, 1);
    pc->add(d_ptr, d_ptr, 16);
    pc->bind(L_StoreSkip16);

    v_last = v_last.xmm();
    remaining -= 16u;
  }
#endif // BL_JIT_ARCH_X86

  if (remaining > 8u) {
    Label L_StoreSkip8 = pc->new_label();
    pc->j(L_StoreSkip8, bt_z(count, 3));
    pc->v_storeu64(d_mem, v_last);
    pc->shift_or_rotate_right(v_last, v_last, 8);
    pc->add(d_ptr, d_ptr, 8);
    pc->bind(L_StoreSkip8);

    remaining -= 8u;
  }

  if (remaining > 4u) {
    Label L_StoreSkip4 = pc->new_label();
    pc->j(L_StoreSkip4, bt_z(count, 2));
    pc->v_storeu32(d_mem, v_last);
    pc->add(d_ptr, d_ptr, 4);
    pc->shift_or_rotate_right(v_last, v_last, 4);
    pc->bind(L_StoreSkip4);

    remaining -= 4u;
  }

  Gp gp_last = pc->new_gp32("@gp_last");
  pc->s_mov_u32(gp_last, v_last);

  if (remaining > 2u) {
    Label L_StoreSkip2 = pc->new_label();
    pc->j(L_StoreSkip2, bt_z(count, 1));
    pc->store_u16(d_mem, gp_last);
    pc->add(d_ptr, d_ptr, 2);
    pc->shr(gp_last, gp_last, 16);
    pc->bind(L_StoreSkip2);

    remaining -= 2u;
  }

  Label L_StoreSkip1 = pc->new_label();
  pc->j(L_StoreSkip1, bt_z(count, 0));
  pc->store_u8(d_mem, gp_last);
  pc->add(d_ptr, d_ptr, 1);
  pc->bind(L_StoreSkip1);

  // Fix a warning that a variable is set, but never used. It's used in asserts and on x86 target.
  bl_unused(remaining);

  // Let's keep it if for some reason we would need it in the future.
  bl_unused(tail_can_be_empty);
}

void store_predicated_vec32(PipeCompiler* pc, const Gp& d_ptr_, const VecArray& s_vec_, uint32_t n, AdvanceMode advance_mode_, PixelPredicate& predicate) noexcept {
  // Restrict the number of vectors to match `n` exactly.
  VecArray s_vec(s_vec_);
  s_vec.truncate(calculate_vec_count(s_vec[0].size(), n * 4u));

  BL_ASSERT(!s_vec.is_empty());
  BL_ASSERT(n >= 2u);

#if defined(BL_JIT_ARCH_X86)
  if (n <= 4)
    s_vec[0] = s_vec[0].v128();
  else if (n <= 8u && s_vec.size() == 1u)
    s_vec[0] = s_vec[0].v256();
#endif // BL_JIT_ARCH_X86

  AdvanceMode advance_mode(advance_mode_);

  Gp d_ptr = d_ptr_;
  Gp count = predicate.count();

  Mem d_mem = mem_ptr(d_ptr);
  size_t size_minus_one = s_vec.size() - 1u;

  Vec v_last = s_vec[size_minus_one];
  bool tail_can_be_empty = false;

  size_t remaining = n;
  size_t element_count = v_last.size() / 4u;

  auto makeDPtrCopy = [&]() noexcept {
    BL_ASSERT(advance_mode == AdvanceMode::kNoAdvance);

    d_ptr = pc->new_similar_reg(d_ptr, "@dPtrCopy");
    advance_mode = AdvanceMode::kAdvance;

    pc->mov(d_ptr, d_ptr_);
  };

  if (size_minus_one || !v_last.is_vec128()) {
    count = pc->new_similar_reg(count, "@count");
    v_last = pc->new_similar_reg(v_last, "@v_last");
    tail_can_be_empty = true;

    pc->mov(count, predicate.count());
    pc->v_mov(v_last, s_vec[0]);

    if (advance_mode == AdvanceMode::kNoAdvance)
      makeDPtrCopy();
  }

  // Process whole vectors in case that there is more than one vector in `s_vec`. It makes no sense to process
  // ALL vectors with a predicate as that would be unnecessarily complicated and possibly not that efficient
  // considering the high cost of predicated stores of tested micro-architectures.
  if (size_minus_one) {
    Label L_Tail = pc->new_label();
    size_t required_count = element_count;

    for (size_t i = 0; i < size_minus_one; i++) {
      pc->j(L_Tail, ucmp_lt(count, required_count));
      pc->v_storeuvec_u32(d_mem, s_vec[i]);
      pc->add(d_ptr, d_ptr, v_last.size());
      pc->v_mov(v_last, s_vec[i + 1]);

      BL_ASSERT(remaining >= element_count);
      remaining -= element_count;
      required_count += element_count;
    }

    pc->bind(L_Tail);
  }

#if defined(BL_JIT_ARCH_X86)
  // Let's use AVX/AVX2/AVX-512 masking stores if fast store with mask is enabled.
  if (pc->has_cpu_hint(CpuHints::kVecMaskedStore)) {
    pc->v_store_predicated_u32(d_mem, v_last, remaining, predicate);

    // Local advancing can be true, however, if we stored with predicate it means that the initial pointer
    // can be untouched. So check against the passed `advance_mode_` instead of advance, which would be true
    // if there was multiple vector registers to store.
    if (advance_mode_ == AdvanceMode::kAdvance) {
      pc->add_scaled(d_ptr, count.clone_as(d_ptr), 4u);
    }

    return;
  }

  if (v_last.is_vec512()) {
    BL_ASSERT(remaining > 8u);

    Label L_StoreSkip8 = pc->new_label();
    pc->j(L_StoreSkip8, bt_z(count, 3));
    pc->v_storeu256(d_mem, v_last.ymm());
    pc->v_extract_v256(v_last.ymm(), v_last, 1);
    pc->add(d_ptr, d_ptr, 32);
    pc->bind(L_StoreSkip8);

    v_last = v_last.ymm();
    remaining -= 8u;
  }

  if (v_last.is_vec256()) {
    BL_ASSERT(remaining > 4u);

    Label L_StoreSkip4 = pc->new_label();
    pc->j(L_StoreSkip4, bt_z(count, 2));
    pc->v_storeu128(d_mem, v_last.xmm());
    pc->v_extract_v128(v_last.xmm(), v_last, 1);
    pc->add(d_ptr, d_ptr, 16);
    pc->bind(L_StoreSkip4);

    v_last = v_last.xmm();
    remaining -= 4u;
  }
#endif // BL_JIT_ARCH_X86

  Label L_TailDone;

  if (tail_can_be_empty)
    L_TailDone = pc->new_label();

  if (count.id() != predicate.count().id()) {
    if (!tail_can_be_empty)
      pc->and_(count, count, 0x3);
    else
      pc->j(L_TailDone, and_z(count, 0x3));
  }
  else if (tail_can_be_empty) {
    pc->j(L_TailDone, cmp_eq(count, 0));
  }

  Gp adjusted1;
  Gp adjusted2;

  if (const PixelPredicate::MaterializedEndPtr* materialized = predicate.find_materialized_end_ptr(d_ptr_)) {
    adjusted1 = materialized->adjusted1;
    adjusted2 = materialized->adjusted2;
  }
  else {
    adjusted1 = pc->new_gpz("@adjusted1");
    adjusted2 = pc->new_gpz("@adjusted2");

    pc->add_ext(adjusted2, d_ptr, count.clone_as(d_ptr), 4, -4);
    pc->add(adjusted1, d_ptr, 4);
    pc->umin(adjusted1, adjusted1, adjusted2);
  }

  pc->v_store_extract_u32(mem_ptr(adjusted2), v_last, 2);
  pc->v_store_extract_u32(mem_ptr(adjusted1), v_last, 1);
  pc->v_storeu32_u32(mem_ptr(d_ptr), v_last);

  if (advance_mode_ == AdvanceMode::kAdvance)
    pc->add(d_ptr, adjusted2, 4);

  if (tail_can_be_empty) {
    pc->bind(L_TailDone);
  }

  // Fix a warning that a variable is set, but never used. It's used in asserts and on x86 target.
  bl_unused(remaining);
}

// bl::Pipeline::Jit::FetchUtils - Fetch Mask
// ==========================================

static void multiply_packed_mask_with_global_alpha(PipeCompiler* pc, VecArray vm, uint32_t n, GlobalAlpha* ga) noexcept {
  BL_ASSERT(vm.size() > 0u);
  BL_ASSERT(ga != nullptr);

  size_t vc = calculate_vec_count(vm[0].size(), n);
  vm.truncate(vc);

#if defined(BL_JIT_ARCH_A64)
  const Vec& pa = ga->pa();

  if (n <= 8u) {
    pc->v_mulw_lo_u8(vm, vm, pa);
    pc->v_srli_rnd_acc_u16(vm, vm, 8);
    pc->v_srlni_rnd_lo_u16(vm, vm, 8);
  }
  else {
    VecArray vt;
    pc->new_vec_array(vt, vc, vm.vec_width(), "@vt0");

    pc->v_mulw_hi_u8(vt, vm, pa);
    pc->v_mulw_lo_u8(vm, vm, pa);

    pc->v_srli_rnd_acc_u16(vm, vm, 8);
    pc->v_srli_rnd_acc_u16(vt, vt, 8);

    pc->v_srlni_rnd_lo_u16(vm, vm, 8);
    pc->v_srlni_rnd_hi_u16(vm, vt, 8);
  }
#else
  Vec ua = ga->ua().clone_as(vm[0]);

  if (n <= 8u) {
    pc->v_cvt_u8_lo_to_u16(vm, vm);
    pc->v_mul_u16(vm, vm, ua);
    pc->v_div255_u16(vm);
    pc->v_packs_i16_u8(vm, vm, vm);
  }
  else {
    Operand zero = pc->simd_const(&pc->ct<CommonTable>().p_0000000000000000, Bcst::kNA, vm[0]);

    VecArray vt;
    pc->new_vec_array(vt, vc, vm.vec_width(), "@vt0");

    pc->v_interleave_hi_u8(vt, vm, zero);
    pc->v_interleave_lo_u8(vm, vm, zero);
    pc->v_mul_u16(vt, vt, ua);
    pc->v_mul_u16(vm, vm, ua);
    pc->v_div255_u16(vt);
    pc->v_div255_u16(vm);
    pc->v_packs_i16_u8(vm, vm, vt);
  }
#endif
}

void fetch_mask_a8_into_pa(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate, GlobalAlpha* ga) noexcept {
  BL_ASSERT(d_vec.size() >= pc->vec_count_of(DataWidth::k8, n) && d_vec.vec_width() == pc->vec_width_of(DataWidth::k8, n));

  fetch_vec8(pc, d_vec, s_ptr, uint32_t(n), advance_mode, predicate);

  if (ga) {
    multiply_packed_mask_with_global_alpha(pc, d_vec, uint32_t(n), ga);
  }
}

void fetch_mask_a8_into_ua(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate, GlobalAlpha* ga) noexcept {
  BL_ASSERT(d_vec.size() >= pc->vec_count_of(DataWidth::k16, n) && d_vec.vec_width() == pc->vec_width_of(DataWidth::k16, n));

  size_t vc = pc->vec_count_of(DataWidth::k16, n);
  Mem m = ptr(s_ptr);

  if (predicate.is_empty()) {
    switch (uint32_t(n)) {
      case 2:
#if defined(BL_JIT_ARCH_X86)
        if (pc->has_avx2()) {
          pc->v_broadcast_u16(d_vec[0], m);
        }
        else
#endif // BL_JIT_ARCH_X86
        {
          pc->v_loadu16(d_vec[0], m);
        }
        pc->v_cvt_u8_lo_to_u16(d_vec[0], d_vec[0]);
        break;

      case 4:
        pc->v_loada32(d_vec[0], m);
        pc->v_cvt_u8_lo_to_u16(d_vec[0], d_vec[0]);
        break;

      case 8:
        pc->v_cvt_u8_lo_to_u16(d_vec[0], m);
        break;

      default: {
        for (size_t i = 0; i < vc; i++) {
          pc->v_cvt_u8_lo_to_u16(d_vec[i], m);
          m.add_offset_lo32(int32_t(d_vec[i].size() / 2u));
        }
        break;
      }
    }

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, uint32_t(n));
    }
  }
  else {
    if (n <= PixelCount(8)) {
      fetch_predicated_vec8(pc, d_vec, s_ptr, uint32_t(n), advance_mode, predicate);
      pc->v_cvt_u8_lo_to_u16(d_vec[0], d_vec[0]);
    }
#if defined(BL_JIT_ARCH_X86)
    else if (d_vec[0].size() > 16u) {
      VecArray lo = d_vec.clone_as(VecWidth(uint32_t(d_vec.vec_width()) - 1u));
      fetch_predicated_vec8(pc, lo, s_ptr, uint32_t(n), advance_mode, predicate);
      pc->v_cvt_u8_lo_to_u16(d_vec, d_vec);
    }
#endif // BL_JIT_ARCH_X86
    else {
      VecArray even = d_vec.even();
      VecArray odd = d_vec.odd();

      fetch_predicated_vec8(pc, even, s_ptr, uint32_t(n), advance_mode, predicate);

      pc->v_cvt_u8_hi_to_u16(odd, even);
      pc->v_cvt_u8_lo_to_u16(even, even);
    }
  }

  if (ga) {
    pc->v_mul_i16(d_vec, d_vec, ga->ua().clone_as(d_vec[0]));
    pc->v_div255_u16(d_vec);
  }
}

#if defined(BL_JIT_ARCH_X86)
// Works for SSE4.1, AVX/AVX2, and AVX-512 cases.
static void fetch_mask_a8_into_pc_by_expanding_to_32bits(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, GlobalAlpha* ga) noexcept {
  pc->v_loaduvec_u8_to_u32(d_vec, ptr(s_ptr));

  if (advance_mode == AdvanceMode::kAdvance) {
    pc->add(s_ptr, s_ptr, uint32_t(n));
  }

  // TODO: [JIT] We can save some multiplications if we only extend to 16 bits, then multiply, and then shuffle.
  if (ga) {
    pc->v_mul_u16(d_vec, d_vec, ga->ua());
    pc->v_div255_u16(d_vec);
  }

  pc->v_swizzlev_u8(d_vec, d_vec, pc->simd_const(&pc->ct<CommonTable>().swizu8_xxx3xxx2xxx1xxx0_to_3333222211110000, Bcst::kNA, d_vec));
}

// AVX2 and AVX-512 code using YMM/ZMM registers require a different approach compared to 128-bit registers as we
// are going to cross 128-bit boundaries, which usually require either zero-extension or using one of AVX2/AVX-512
// permute instructions.
static void expand_a8_mask_to_pc_ymm_zmm(PipeCompiler* pc, VecArray& d_vec, VecArray& a_vec) noexcept {
  // Number of 4-vec chunks for swizzling - each 4-vec chunk is swizzled/unpacked independently.
  size_t q_count = (d_vec.size() + 3) / 4;

  // AVX512_VBMI provides VPERMB, which we want to use - on modern micro-architectures such as Zen4+ it's as fast as
  // VPSHUFB.
  if (d_vec.is_vec512() && pc->has_avx512_vbmi()) {
    Vec predicate0 = pc->simd_vec_const(&pc->ct<CommonTable>().permu8_a8_to_rgba32_pc, Bcst::kNA_Unique, d_vec);
    Vec predicate1;

    if (d_vec.size() >= 2u) {
      predicate1 = pc->simd_vec_const(&pc->ct<CommonTable>().permu8_a8_to_rgba32_pc_second, Bcst::kNA_Unique, d_vec);
    }

    for (size_t q = 0; q < q_count; q++) {
      size_t d = q * 4;
      size_t remain = bl_min(d_vec.size() - d, size_t(4));

      if (remain >= 3) {
        pc->v_extract_v256(d_vec[d + 2], a_vec[q], 1);
      }

      if (remain >= 2) {
        pc->v_permute_u8(d_vec[d + 1], predicate1, a_vec[q]);
      }

      pc->v_permute_u8(d_vec[d], predicate0, a_vec[q]);

      if (remain >= 4) {
        pc->v_permute_u8(d_vec[d + 3], predicate1, d_vec[d + 2]);
      }

      if (remain >= 3) {
        pc->v_permute_u8(d_vec[d + 2], predicate0, d_vec[d + 2]);
      }
    }
  }
  else if (d_vec.is_vec512()) {
    Vec predicate = pc->simd_vec_const(&pc->ct<CommonTable>().swizu8_xxx3xxx2xxx1xxx0_to_3333222211110000, Bcst::kNA, d_vec);

    for (size_t q = 0; q < q_count; q++) {
      size_t d = q * 4;
      size_t remain = bl_min(d_vec.size() - d, size_t(4));

      for (size_t i = 1; i < remain; i++) {
        Vec& dv = d_vec[d + i];
        pc->v_extract_v128(dv, a_vec[q], uint32_t(i));
      }

      for (size_t i = 0; i < remain; i++) {
        Vec& dv = d_vec[d + i];
        pc->v_cvt_u8_to_u32(dv, i == 0 ? a_vec[q] : dv);
        pc->v_swizzlev_u8(dv, dv, predicate);
      }
    }
  }
  else {
    BL_ASSERT(d_vec.is_vec256());

    Vec predicate = pc->simd_vec_const(&pc->ct<CommonTable>().swizu8_xxx3xxx2xxx1xxx0_to_3333222211110000, Bcst::kNA, d_vec);

    for (size_t q = 0; q < q_count; q++) {
      size_t d = q * 4;
      size_t remain = bl_min(d_vec.size() - d, size_t(4));

      if (remain >= 3) {
        pc->v_swizzle_u64x4(d_vec[d + 2], a_vec[q], swizzle(1, 0, 3, 2));
      }

      if (remain >= 2) {
        pc->v_swizzle_u32x4(d_vec[d + 1], a_vec[q], swizzle(1, 0, 3, 2));
      }

      if (remain >= 4) {
        pc->v_swizzle_u32x4(d_vec[d + 3], d_vec[d + 2], swizzle(1, 0, 3, 2));
      }

      for (size_t i = 0; i < remain; i++) {
        Vec& dv = d_vec[d + i];
        pc->v_cvt_u8_to_u32(dv, i == 0 ? a_vec[q] : dv);
        pc->v_swizzlev_u8(dv, dv, predicate);
      }
    }
  }
}
#endif // BL_JIT_ARCH_X86

void fetch_mask_a8_into_pc(PipeCompiler* pc, VecArray d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate, GlobalAlpha* ga) noexcept {
  VecWidth vw = d_vec.vec_width();
  size_t vc = VecWidthUtils::vec_count_of(vw, DataWidth::k32, uint32_t(n));

  BL_ASSERT(d_vec.size() >= vc);
  d_vec.truncate(vc);

#if defined(BL_JIT_ARCH_X86)
  // The easiest way to do this is to extend BYTE to DWORD and then to use a single VPSHUFB predicate to expand
  // alpha values to all required lanes. This saves registers that would be otherwise used to hold more predicates.
  //
  // NOTE: This approach is only suitable for X86 as we can zero extend BYTE to DWORD during the load itself, which
  // makes it the best approach as we can use a single predicate to duplicate the alpha to all required lanes.
  if (predicate.is_empty() && pc->has_sse4_1() && n >= PixelCount(4)) {
    fetch_mask_a8_into_pc_by_expanding_to_32bits(pc, d_vec, s_ptr, n, advance_mode, ga);
    return;
  }
#endif // BL_JIT_ARCH_X86

  VecArray a_vec = d_vec.every_nth(4);
  fetch_vec8(pc, a_vec, s_ptr, uint32_t(n), advance_mode, predicate);

  // TODO: [JIT] This is not optimal in X86 case - we should zero extend to U16, multiply, and then expand to U32.
  if (ga) {
    multiply_packed_mask_with_global_alpha(pc, a_vec, uint32_t(n), ga);
  }

#if defined(BL_JIT_ARCH_X86)
  if (!d_vec.is_vec128()) {
    // At least 8 pixels should be fetched in order to use YMM registers and 16 pixels in order to use ZMM registers.
    BL_ASSERT(uint32_t(n) >= d_vec[0].size() / 4u);

    expand_a8_mask_to_pc_ymm_zmm(pc, d_vec, a_vec);
    return;
  }
#endif // BL_JIT_ARCH_X86

  // Number of 4-vec chunks for swizzling - each 4-vec chunk is swizzled/unpacked independently.
  size_t q_count = (d_vec.size() + 3u) / 4u;

  // We have two choices - use interleave sequences (2 interleaves are required to expand one A8 to 4 channels)
  // or use VPSHUFB/TBL (table lookup) instructions to do only a single table lookup per register.
  bool use_interleave_sequence = (n <= PixelCount(8));

#if defined(BL_JIT_ARCH_X86)
  if (!pc->has_ssse3()) {
    use_interleave_sequence = true;
  }
#endif // BL_JIT_ARCH_X86

  if (use_interleave_sequence) {
    for (size_t q = 0; q < q_count; q++) {
      size_t d = q * 4;
      size_t remain = vc - d;

      Vec a0 = a_vec[q];

      if (remain >= 4) {
        pc->v_interleave_hi_u8(d_vec[d + 2], a0, a0);
      }

      pc->v_interleave_lo_u8(d_vec[d + 0], a0, a0);

      if (remain >= 2) {
        pc->v_interleave_hi_u16(d_vec[d + 1], d_vec[d + 0], d_vec[d + 0]);
      }

      pc->v_interleave_lo_u16(d_vec[d + 0], d_vec[d + 0], d_vec[d + 0]);

      if (remain >= 4) {
        pc->v_interleave_hi_u16(d_vec[d + 3], d_vec[d + 2], d_vec[d + 2]);
      }

      if (remain >= 3) {
        pc->v_interleave_lo_u16(d_vec[d + 2], d_vec[d + 2], d_vec[d + 2]);
      }
    }
  }
  else {
    // Maximum number of registers in VecArray is 8, thus we can have up to 2 valid registers in
    // d_vec that we are going to shuffle to 1-8 registers by using a table lookup (VPSHUFB or TBL).
    bool limit_predicate_count = pc->vec_reg_count() < 32u;

    Operand swiz[4];
    swiz[0] = pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxxxxxxx3210_to_3333222211110000, Bcst::kNA, d_vec);

    if (vc >= 2u) {
      swiz[1] = pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxxx3210xxxx_to_3333222211110000, Bcst::kNA, d_vec);
    }

    if (vc >= 3u) {
      swiz[2] = limit_predicate_count ? swiz[0] : pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxx3210xxxxxxxx_to_3333222211110000, Bcst::kNA, d_vec);
    }

    if (vc >= 4u) {
      swiz[3] = limit_predicate_count ? swiz[1] : pc->simd_const(&pc->ct<CommonTable>().swizu8_3210xxxxxxxxxxxx_to_3333222211110000, Bcst::kNA, d_vec);
    }

    for (size_t q = 0; q < q_count; q++) {
      size_t d = q * 4;
      size_t remain = vc - d;

      Vec a0 = a_vec[q];

      if (remain >= 3u) {
        Vec a1 = a0;
        if (limit_predicate_count) {
          a1 = d_vec[d + 2];
          pc->v_swizzle_u32x4(a1, a0, swizzle(3, 2, 3, 2));
        }

        if (remain >= 4u) {
          pc->v_swizzlev_u8(d_vec[d + 3], a1, swiz[3]);
        }

        pc->v_swizzlev_u8(d_vec[d + 2], a1, swiz[2]);
      }

      if (remain >= 2u) {
        pc->v_swizzlev_u8(d_vec[d + 1], a0, swiz[1]);
      }

      pc->v_swizzlev_u8(d_vec[d], a0, swiz[0]);
    }
  }
}

void fetch_mask_a8_into_uc(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate, GlobalAlpha* ga) noexcept {
  BL_ASSERT(d_vec.size() >= pc->vec_count_of(DataWidth::k64, n) && d_vec.vec_width() == pc->vec_width_of(DataWidth::k64, n));

#if defined(BL_JIT_ARCH_X86)
  VecWidth vec_width = pc->vec_width_of(DataWidth::k64, n);
#endif // BL_JIT_ARCH_X86

  size_t vec_count = pc->vec_count_of(DataWidth::k64, n);
  Mem m = ptr(s_ptr);

  // Maybe unused on AArch64 in release mode.
  bl_unused(vec_count);

  switch (uint32_t(n)) {
    case 1: {
      BL_ASSERT(predicate.is_empty());
      BL_ASSERT(vec_count == 1);

#if defined(BL_JIT_ARCH_X86)
      if (!pc->has_avx2()) {
        pc->v_load8(d_vec[0], m);
        if (advance_mode == AdvanceMode::kAdvance) {
          pc->add(s_ptr, s_ptr, uint32_t(n));
        }
        pc->v_swizzle_lo_u16x4(d_vec[0], d_vec[0], swizzle(0, 0, 0, 0));
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        pc->v_broadcast_u8(d_vec[0], m);
        if (advance_mode == AdvanceMode::kAdvance) {
          pc->add(s_ptr, s_ptr, uint32_t(n));
        }
        pc->v_cvt_u8_lo_to_u16(d_vec[0], d_vec[0]);
      }

      if (ga) {
        pc->v_mul_i16(d_vec[0], d_vec[0], ga->ua().clone_as(d_vec[0]));
        pc->v_div255_u16(d_vec[0]);
      }
      break;
    }

    case 2: {
      BL_ASSERT(vec_count == 1);

#if defined(BL_JIT_ARCH_X86)
      if (!predicate.is_empty() || !pc->has_avx2()) {
        fetch_vec8(pc, d_vec, s_ptr, uint32_t(n), advance_mode, predicate);
        pc->v_interleave_lo_u8(d_vec[0], d_vec[0], d_vec[0]);
        pc->v_interleave_lo_u16(d_vec[0], d_vec[0], d_vec[0]);
        pc->v_cvt_u8_lo_to_u16(d_vec[0], d_vec[0]);
      }
      else {
        pc->v_loadu16_u8_to_u64(d_vec[0], m);
        if (advance_mode == AdvanceMode::kAdvance) {
          pc->add(s_ptr, s_ptr, uint32_t(n));
        }
        pc->v_swizzlev_u8(d_vec[0], d_vec[0], pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxx1xxxxxxx0_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d_vec[0]));
      }
#else
      fetch_vec8(pc, d_vec, s_ptr, uint32_t(n), advance_mode, predicate);
      pc->v_swizzlev_u8(d_vec[0], d_vec[0], pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxxxxxxxxx10_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d_vec[0]));
#endif // BL_JIT_ARCH_X86

      if (ga) {
        pc->v_mul_i16(d_vec[0], d_vec[0], ga->ua().clone_as(d_vec[0]));
        pc->v_div255_u16(d_vec[0]);
      }
      break;
    }

    case 4: {
#if defined(BL_JIT_ARCH_X86)
      if (vec_width >= VecWidth::k256) {
        if (predicate.is_empty()) {
          pc->v_loadu32_u8_to_u64(d_vec[0], m);
          if (advance_mode == AdvanceMode::kAdvance) {
            pc->add(s_ptr, s_ptr, uint32_t(n));
          }
        }
        else {
          fetch_vec8(pc, d_vec, s_ptr, uint32_t(n), advance_mode, predicate);
          pc->cc->vpmovzxbq(d_vec[0], d_vec[0].xmm());
        }
        pc->v_swizzlev_u8(d_vec[0], d_vec[0], pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxx1xxxxxxx0_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d_vec[0]));

        if (ga) {
          pc->v_mul_i16(d_vec[0], d_vec[0], ga->ua().clone_as(d_vec[0]));
          pc->v_div255_u16(d_vec[0]);
        }
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        fetch_vec8(pc, d_vec, s_ptr, uint32_t(n), advance_mode, predicate);
        pc->v_cvt_u8_lo_to_u16(d_vec[0], d_vec[0]);

        if (ga) {
          pc->v_mul_i16(d_vec[0], d_vec[0], ga->ua().clone_as(d_vec[0]));
          pc->v_div255_u16(d_vec[0]);
        }

        pc->v_interleave_lo_u16(d_vec[0], d_vec[0], d_vec[0]);        // d_vec[0] = [M3 M3 M2 M2 M1 M1 M0 M0]
        pc->v_swizzle_u32x4(d_vec[1], d_vec[0], swizzle(3, 3, 2, 2)); // d_vec[1] = [M3 M3 M3 M3 M2 M2 M2 M2]
        pc->v_swizzle_u32x4(d_vec[0], d_vec[0], swizzle(1, 1, 0, 0)); // d_vec[0] = [M1 M1 M1 M1 M0 M0 M0 M0]
      }
      break;
    }

    default: {
#if defined(BL_JIT_ARCH_X86)
      if (vec_width >= VecWidth::k256) {
        if (predicate.is_empty()) {
          for (size_t i = 0; i < vec_count; i++) {
            pc->v_loaduvec_u8_to_u64(d_vec[i], m);
            m.add_offset_lo32(int32_t(d_vec[i].size() / 8u));
          }

          if (advance_mode == AdvanceMode::kAdvance) {
            pc->add(s_ptr, s_ptr, uint32_t(n));
          }

          if (ga) {
            Vec ua = ga->ua().clone_as(d_vec[0]);
            if (pc->has_cpu_hint(CpuHints::kVecFastIntMul32)) {
              pc->v_mul_i32(d_vec, d_vec, ua);
              pc->v_div255_u16(d_vec);
              pc->v_swizzle_u32x4(d_vec, d_vec, swizzle(2, 2, 0, 0));
            }
            else {
              pc->v_mul_i16(d_vec, d_vec, ua);
              pc->v_div255_u16(d_vec);
              pc->v_swizzlev_u8(d_vec, d_vec, pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxx1xxxxxxx0_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d_vec[0]));
            }
          }
          else {
            pc->v_swizzlev_u8(d_vec, d_vec, pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxx1xxxxxxx0_to_z1z1z1z1z0z0z0z0, Bcst::kNA, d_vec[0]));
          }
        }
        else {
          VecArray pm;
          VecArray um;

          pc->new_vec_array(pm, pc->vec_count_of(DataWidth::k8, n), pc->vec_width_of(DataWidth::k8, n), "pm");
          pc->new_vec_array(um, pc->vec_count_of(DataWidth::k16, n), pc->vec_width_of(DataWidth::k16, n), "um");

          fetch_vec8(pc, pm, s_ptr, uint32_t(n), advance_mode, predicate);

          if (um.size() == 1) {
            pc->v_cvt_u8_lo_to_u16(um, pm);
          }
          else {
            pc->v_cvt_u8_hi_to_u16(um.odd(), pm);
            pc->v_cvt_u8_lo_to_u16(um.even(), pm);
          }

          if (ga) {
            pc->v_mul_i16(um, um, ga->ua().clone_as(um[0]));
            pc->v_div255_u16(um);
          }

          if (d_vec[0].is_vec512()) {
            if (pc->has_avx512_vbmi()) {
              // Extract 128-bit vectors and then use VPERMB to permute 8 elements to 512-bit width.
              Vec pred = pc->simd_vec_const(&pc->ct<CommonTable>().permu8_4xu8_lo_to_rgba32_uc, Bcst::kNA_Unique, d_vec);

              for (size_t i = 1; i < d_vec.size(); i++) {
                pc->v_extract_v128(d_vec[i], um[0], uint32_t(i));
              }

              for (size_t i = 0; i < d_vec.size(); i++) {
                pc->v_permute_u8(d_vec[i], pred, (i == 0 ? um[0] : d_vec[i]).clone_as(d_vec[i]));
              }
            }
            else {
              Vec pred = pc->simd_vec_const(&pc->ct<CommonTable>().swizu8_xxxxxxx1xxxxxxx0_to_z1z1z1z1z0z0z0z0, Bcst::kNA_Unique, d_vec);
              for (size_t i = 1; i < d_vec.size(); i++) {
                pc->v_extract_v128(d_vec[i], um[0], uint32_t(i));
              }

              for (size_t i = 0; i < d_vec.size(); i++) {
                pc->v_cvt_u8_to_u32(d_vec[i], i == 0 ? um[0] : d_vec[i]);
                pc->v_swizzlev_u8(d_vec[i], d_vec[i], pred);
              }
            }
          }
          else if (d_vec[0].is_vec256()) {
            Vec pred = pc->simd_vec_const(&pc->ct<CommonTable>().swizu8_xxxxxxx1xxxxxxx0_to_z1z1z1z1z0z0z0z0, Bcst::kNA_Unique, d_vec);

            if (d_vec.size() >= 2)
              pc->v_swizzle_u64x2(d_vec[1].xmm(), um[0].xmm(), swizzle(0, 1));

            if (d_vec.size() >= 3)
              pc->v_extract_v128(d_vec[2].xmm(), um[0], 1);

            if (d_vec.size() >= 4)
              pc->v_swizzle_u64x4(d_vec[3], um[0], swizzle(3, 3, 3, 3));

            for (size_t i = 0; i < d_vec.size(); i++) {
              pc->v_cvt_u8_to_u32(d_vec[i], i == 0 ? um[0] : d_vec[i]);
              pc->v_swizzlev_u8(d_vec[i], d_vec[i], pred);
            }
          }
          else {
            BL_NOT_REACHED();
          }
        }
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        // Maximum pixels for 128-bit SIMD is 8 - there are no registers for more...
        BL_ASSERT(n == PixelCount(8));

        if (predicate.is_empty()) {
          pc->v_cvt_u8_lo_to_u16(d_vec[0], m);
          if (advance_mode == AdvanceMode::kAdvance) {
            pc->add(s_ptr, s_ptr, uint32_t(n));
          }
        }
        else {
          fetch_vec8(pc, VecArray(d_vec[0]), s_ptr, uint32_t(n), advance_mode, predicate);
          pc->v_cvt_u8_lo_to_u16(d_vec[0], d_vec[0]);
        }

        if (ga) {
          pc->v_mul_i16(d_vec[0], d_vec[0], ga->ua().clone_as(d_vec[0]));
          pc->v_div255_u16(d_vec[0]);
        }

        pc->v_interleave_hi_u16(d_vec[2], d_vec[0], d_vec[0]);        // d_vec[2] = [M7 M7 M6 M6 M5 M5 M4 M4]
        pc->v_interleave_lo_u16(d_vec[0], d_vec[0], d_vec[0]);        // d_vec[0] = [M3 M3 M2 M2 M1 M1 M0 M0]
        pc->v_swizzle_u32x4(d_vec[3], d_vec[2], swizzle(3, 3, 2, 2)); // d_vec[3] = [M7 M7 M7 M7 M6 M6 M6 M6]
        pc->v_swizzle_u32x4(d_vec[1], d_vec[0], swizzle(3, 3, 2, 2)); // d_vec[1] = [M3 M3 M3 M3 M2 M2 M2 M2]
        pc->v_swizzle_u32x4(d_vec[0], d_vec[0], swizzle(1, 1, 0, 0)); // d_vec[0] = [M1 M1 M1 M1 M0 M0 M0 M0]
        pc->v_swizzle_u32x4(d_vec[2], d_vec[2], swizzle(1, 1, 0, 0)); // d_vec[2] = [M5 M5 M5 M5 M4 M4 M4 M4]
      }
      break;
    }
  }
}

void fetch_mask_a8(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, PixelType pixel_type, PixelCoverageFormat coverage_format, AdvanceMode advance_mode, PixelPredicate& predicate, GlobalAlpha* ga) noexcept {
  switch (pixel_type) {
    case PixelType::kA8: {
      BL_ASSERT(n != PixelCount(1));

      if (coverage_format == PixelCoverageFormat::kPacked) {
        VecWidth vec_width = pc->vec_width_of(DataWidth::k8, n);
        size_t vec_count = pc->vec_count_of(DataWidth::k8, n);

        pc->new_vec_array(d_vec, vec_count, vec_width, "vm");
        fetch_mask_a8_into_pa(pc, d_vec, s_ptr, n, advance_mode, predicate, ga);
      }
      else {
        VecWidth vec_width = pc->vec_width_of(DataWidth::k16, n);
        size_t vec_count = pc->vec_count_of(DataWidth::k16, n);

        pc->new_vec_array(d_vec, vec_count, vec_width, "vm");
        fetch_mask_a8_into_ua(pc, d_vec, s_ptr, n, advance_mode, predicate, ga);
      }
      break;
    }

    case PixelType::kRGBA32: {
      if (coverage_format == PixelCoverageFormat::kPacked) {
        VecWidth vec_width = pc->vec_width_of(DataWidth::k32, n);
        size_t vec_count = pc->vec_count_of(DataWidth::k32, n);

        pc->new_vec_array(d_vec, vec_count, vec_width, "vm");
        fetch_mask_a8_into_pc(pc, d_vec, s_ptr, n, advance_mode, predicate, ga);
      }
      else {
        VecWidth vec_width = pc->vec_width_of(DataWidth::k64, n);
        size_t vec_count = pc->vec_count_of(DataWidth::k64, n);

        pc->new_vec_array(d_vec, vec_count, vec_width, "vm");
        fetch_mask_a8_into_uc(pc, d_vec, s_ptr, n, advance_mode, predicate, ga);
      }

      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::Jit::FetchUtils - Fetch Pixel(s)
// ==============================================

#if defined(BL_JIT_ARCH_X86)
static void v_permute_op(PipeCompiler* pc, Vec dst, Vec predicate, Operand src, UniOpVVV op, uint32_t byte_quantity) noexcept {
  OperandSignature sgn;

  if (byte_quantity == 64u) {
    sgn = OperandSignature{asmjit::RegTraits<asmjit::RegType::kVec512>::kSignature};
  }
  else if (byte_quantity == 32u) {
    sgn = OperandSignature{asmjit::RegTraits<asmjit::RegType::kVec256>::kSignature};
  }
  else {
    sgn = OperandSignature{asmjit::RegTraits<asmjit::RegType::kVec128>::kSignature};
  }

  dst.set_signature(sgn);;
  predicate.set_signature(sgn);

  if (src.is_reg()) {
    src.set_signature(sgn);
  }

  pc->emit_3v(op, dst, predicate, src);
};

static void v_prgb32_to_pa_vpermb(PipeCompiler* pc, const Vec& dst, const Vec& predicate, const Operand& src, PixelCount n) noexcept {
  return v_permute_op(pc, dst, predicate, src, UniOpVVV::kPermuteU8, uint32_t(n) * 4u);
};

static void v_prgb32_to_ua_vpermw(PipeCompiler* pc, const Vec& dst, const Vec& predicate, const Operand& src, PixelCount n) noexcept {
  return v_permute_op(pc, dst, predicate, src, UniOpVVV::kPermuteU16, uint32_t(n) * 4u);
};

static void fetch_prgb32_into_pa_avx512(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  VecWidth pc_width = VecWidthUtils::vec_width_of(VecWidth::k512, DataWidth::k32, uint32_t(n));
  size_t pc_count = VecWidthUtils::vec_count_of(pc_width, DataWidth::k32, uint32_t(n));

  uint32_t d_shift = uint32_t(d_vec.vec_width());
  uint32_t d_mask = (1u << d_shift) - 1u;

  uint32_t iter = 0;
  uint32_t remaining = uint32_t(n);

  if (pc->has_avx512_vbmi()) {
    // In AVX512_VBMI case we can use VPERMT2B to shuffle two registers at once (and micro-architecturally the cost
    // is either the same as VPERMB [AMD] or 2xVPERMB [Intel]). This approach seems to be the most efficient.
    Vec byte_perm = pc->simd_vec_const(&pc->ct<CommonTable>().permu8_pc_to_pa, Bcst::kNA_Unique, pc_width);

    if (predicate.is_empty()) {
      Mem m = ptr(s_ptr);

      if (pc_width == VecWidth::k128 || pc_count == 1u) {
        // If there is only a single register to load or all destination registers are XMMs it's actually very simple.
        do {
          uint32_t quantity = bl_min<uint32_t>(remaining, 16u);
          const Vec& dv = d_vec[iter];

          v_prgb32_to_pa_vpermb(pc, dv, byte_perm, m, PixelCount(quantity));
          m.add_offset_lo32(int32_t(quantity * 4u));

          iter++;
          remaining -= quantity;
        } while (remaining > 0u);
      }
      else {
        do {
          uint32_t quantity = bl_min<uint32_t>(remaining, 64u);
          const Vec& dv = d_vec[iter];

          if (quantity >= 64u) {
            // Four ZMM registers to permute.
            Vec tv = pc->new_vec_with_width(pc_width, "@tmp_vec");
            pc->v_loadu512(dv.zmm(), m);
            pc->cc->vpermt2b(dv.zmm(), byte_perm.zmm(), m.clone_adjusted(64));

            pc->v_loadu512(tv.zmm(), m.clone_adjusted(128));
            pc->cc->vpermt2b(tv.zmm(), byte_perm.zmm(), m.clone_adjusted(192));

            pc->v_insert_v256(dv, dv, tv, 1);
          }
          else if (quantity >= 32u) {
            // Two ZMM registers to permute.
            pc->v_loadu512(dv.zmm(), m);
            pc->cc->vpermt2b(dv.zmm(), byte_perm, m.clone_adjusted(64));
          }
          else {
            v_prgb32_to_pa_vpermb(pc, dv, byte_perm, m, PixelCount(quantity));
          }

          m.add_offset_lo32(int32_t(quantity * 4u));

          iter++;
          remaining -= quantity;
        } while (remaining > 0u);
      }

      if (advance_mode == AdvanceMode::kAdvance) {
        pc->add(s_ptr, s_ptr, uint32_t(n) * 4u);
      }
    }
    else {
      VecArray pc_vec;
      pc->new_vec_array(pc_vec, pc_count, pc_width, "@tmp_pc_vec");

      // We really want each fourth register to point to the original d_vec (so we don't have to move afterwards).
      for (size_t i = 0; i < pc_count; i += 4) {
        pc_vec.reassign(i, d_vec[i / 4u]);
      }

      fetch_vec32(pc, pc_vec, s_ptr, uint32_t(n), advance_mode, predicate);

      if (pc_width == VecWidth::k128 || pc_count == 1u) {
        // If there is only a single register to load or all destination registers are XMMs it's actually very simple.
        do {
          uint32_t quantity = bl_min<uint32_t>(remaining, 16u);
          const Vec& dv = d_vec[iter];

          v_prgb32_to_pa_vpermb(pc, dv, byte_perm, pc_vec[iter], PixelCount{quantity});

          iter++;
          remaining -= quantity;
        } while (remaining > 0u);
      }
      else {
        uint32_t pc_idx = 0;
        do {
          uint32_t quantity = bl_min<uint32_t>(remaining, 64u);
          const Vec& dv = d_vec[iter];

          if (quantity >= 64u) {
            // Four ZMM registers to permute.
            pc->cc->vpermt2b(pc_vec[pc_idx + 0].zmm(), byte_perm.zmm(), pc_vec[pc_idx + 1].zmm());
            pc->cc->vpermt2b(pc_vec[pc_idx + 2].zmm(), byte_perm.zmm(), pc_vec[pc_idx + 3].zmm());

            pc->v_insert_v256(dv, pc_vec[pc_idx + 0], pc_vec[pc_idx + 2], 1);
            pc_idx += 4;
          }
          else if (quantity >= 32u) {
            // Two ZMM registers to permute.
            pc->cc->vpermt2b(pc_vec[pc_idx].zmm(), byte_perm.zmm(), pc_vec[pc_idx + 1].zmm());
            BL_ASSERT(dv.id() == pc_vec[pc_idx].id());

            pc_idx += 2;
          }
          else {
            v_prgb32_to_pa_vpermb(pc, dv, byte_perm, pc_vec[pc_idx], PixelCount{quantity});
            pc_idx++;
          }

          iter++;
          remaining -= quantity;
        } while (remaining > 0u);
      }
    }
  }
  else if (predicate.is_empty()) {
    Mem m = ptr(s_ptr);
    Vec secondary;

    if (pc_count > 1u) {
      secondary = pc->new_vec_with_width(pc_width, "@tmp_vec");
    }

    do {
      uint32_t quantity = bl_min<uint32_t>(remaining, 16u);
      uint32_t fraction = iter & d_mask;

      const Vec& dv = d_vec[iter >> d_shift];
      const Vec& tv = fraction ? secondary : dv;

      if (quantity >= 16u) {
        pc->v_srli_u32(tv.zmm(), m, 24);
        pc->cc->vpmovdb(tv.xmm(), tv.zmm());
      }
      else if (quantity >= 8u) {
        pc->v_srli_u32(tv.ymm(), m, 24);
        pc->cc->vpmovdb(tv.xmm(), tv.ymm());
      }
      else if (quantity >= 4u) {
        pc->v_srli_u32(tv.xmm(), m, 24);
        pc->cc->vpmovdb(tv.xmm(), tv.xmm());
      }
      else {
        BL_NOT_REACHED();
      }

      if (fraction == 1u) {
        pc->v_insert_v128(dv.ymm(), dv.ymm(), tv.xmm(), fraction);
      }
      else if (fraction > 1u) {
        pc->v_insert_v128(dv.zmm(), dv.zmm(), tv.xmm(), fraction);
      }

      m.add_offset_lo32(int32_t(quantity * 4u));

      iter++;
      remaining -= quantity;
    } while (remaining > 0u);

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, uint32_t(n) * 4u);
    }
  }
  else {
    VecArray tVec;

    pc->new_vec_array(tVec, pc_count, pc_width, "@tmp_vec");
    fetch_vec32(pc, tVec, s_ptr, uint32_t(n), advance_mode, predicate);

    do {
      uint32_t quantity = bl_min<uint32_t>(remaining, 16u);
      uint32_t fraction = iter & d_mask;

      const Vec& dv = d_vec[iter >> d_shift];
      const Vec& tv = fraction ? tVec[iter] : dv;

      if (quantity >= 16u) {
        pc->v_srli_u32(tv.zmm(), tv.zmm(), 24);
        pc->cc->vpmovdb(tv.zmm(), tVec[iter].zmm());
      }
      else if (quantity >= 8u) {
        pc->v_srli_u32(tv.ymm(), tv.ymm(), 24);
        pc->cc->vpmovdb(tv.ymm(), tVec[iter].ymm());
      }
      else if (quantity >= 4u) {
        pc->v_srli_u32(tv.xmm(), tv.xmm(), 24);
        pc->cc->vpmovdb(tv.xmm(), tVec[iter].xmm());
      }
      else {
        BL_NOT_REACHED();
      }

      if (fraction == 1u) {
        pc->v_insert_v128(dv.ymm(), dv.ymm(), tv.xmm(), fraction);
      }
      else if (fraction > 1u) {
        pc->v_insert_v128(dv.zmm(), dv.zmm(), tv.xmm(), fraction);
      }

      iter++;
      remaining -= quantity;
    } while (remaining > 0u);
  }
}

static void fetch_prgb32_into_pa_avx2(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  VecWidth pc_width = VecWidthUtils::vec_width_of(VecWidth::k256, DataWidth::k32, uint32_t(n));
  size_t pc_count = VecWidthUtils::vec_count_of(pc_width, DataWidth::k32, uint32_t(n));

  VecArray tVec;
  pc->new_vec_array(tVec, pc_count, pc_width, "@tmp_vec");
  fetch_vec32(pc, tVec, s_ptr, uint32_t(n), advance_mode, predicate);

  uint32_t d_idx = 0;
  uint32_t t_idx = 0;
  uint32_t remaining = uint32_t(n);

  pc->v_srli_u32(tVec, tVec, 24);

  do {
    const Vec& dv = d_vec[d_idx];
    uint32_t quantity = bl_min<uint32_t>(remaining, 32u);

    if (quantity >= 16u) {
      Vec vpermd_pred = pc->simd_vec_const(&pc->ct<CommonTable>().permu32_fix_2x_pack_avx2, Bcst::kNA_Unique, pc_width);

      const Vec& sv0 = tVec[t_idx + 0];
      const Vec& sv1 = tVec[t_idx + 1];

      if (quantity == 32u) {
        const Vec& sv2 = tVec[t_idx + 2];
        const Vec& sv3 = tVec[t_idx + 3];

        pc->v_packs_i32_u16(sv0, sv0, sv1);
        pc->v_packs_i32_u16(sv2, sv2, sv3);
        pc->v_packs_i16_u8(sv0, sv0, sv2);
        pc->cc->vpermd(dv, vpermd_pred, sv0);

        t_idx += 4;
      }
      else if (quantity == 16) {
        pc->v_packs_i32_u16(sv0, sv0, sv1);
        pc->v_packs_i16_u8(sv0, sv0, sv0);
        pc->cc->vpermd(dv, vpermd_pred, sv0);

        t_idx += 2;
      }
    }
    else {
      const Vec& sv = tVec[t_idx];

      if (quantity == 8u) {
        pc->v_packs_i32_u16(sv, sv, sv);
        pc->v_swizzle_u64x4(sv, sv, swizzle(3, 1, 2, 0));
        pc->v_packs_i16_u8(dv.xmm(), sv.xmm(), sv.xmm());
      }
      else {
        pc->v_packs_i32_u16(dv.xmm(), sv.xmm(), sv.xmm());
        pc->v_packs_i16_u8(dv.xmm(), dv.xmm(), dv.xmm());
      }

      t_idx++;
    }

    d_idx++;
    remaining -= quantity;
  } while (remaining > 0u);
}

static void fetch_prgb32_into_ua_avx512(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  VecWidth pc_width = VecWidthUtils::vec_width_of(VecWidth::k512, DataWidth::k32, uint32_t(n));
  size_t pc_count = VecWidthUtils::vec_count_of(pc_width, DataWidth::k32, uint32_t(n));

  uint32_t iter = 0;
  uint32_t remaining = uint32_t(n);

  // A baseline AVX512 ISA offers VPERMT2W to shuffle two registers at once at 16-bit quantities, which is sufficient
  // for our case (converting a 32-bit ARGB pixel into an unpacked 16-bit alpha). We always want to shift by 8 at the
  // end as that means shifting half registers in case we load multiple ones.
  Vec permute_predicate = pc->simd_vec_const(&pc->ct<CommonTable>().permu16_pc_to_ua, Bcst::kNA_Unique, pc_width);

  if (predicate.is_empty()) {
    Mem m = ptr(s_ptr);

    if (pc_width == VecWidth::k128 || pc_count == 1u) {
      // If there is only a single register to load or all destination registers are XMMs it's actually very simple.
      do {
        uint32_t quantity = bl_min<uint32_t>(remaining, 16u);
        const Vec& dv = d_vec[iter];

        v_prgb32_to_ua_vpermw(pc, dv, permute_predicate, m, PixelCount{quantity});
        m.add_offset_lo32(int32_t(quantity * 4u));

        iter++;
        remaining -= quantity;
      } while (remaining > 0u);
    }
    else {
      do {
        uint32_t quantity = bl_min<uint32_t>(remaining, 64u);
        const Vec& dv = d_vec[iter];

        if (quantity >= 64u) {
          // Four ZMM registers to permute.
          Vec tv = pc->new_vec_with_width(pc_width, "@tmp_vec");
          pc->v_loadu512(dv.zmm(), m);
          pc->cc->vpermt2w(dv.zmm(), permute_predicate.zmm(), m.clone_adjusted(64));

          pc->v_loadu512(tv.zmm(), m.clone_adjusted(128));
          pc->cc->vpermt2w(tv.zmm(), permute_predicate.zmm(), m.clone_adjusted(192));

          pc->v_insert_v256(dv, dv, tv, 1);
        }
        else if (quantity >= 32u) {
          // Two ZMM registers to permute.
          pc->v_loadu512(dv.zmm(), m);
          pc->cc->vpermt2w(dv.zmm(), permute_predicate, m.clone_adjusted(64));
        }
        else {
          v_prgb32_to_ua_vpermw(pc, dv, permute_predicate, m, PixelCount{quantity});
        }

        m.add_offset_lo32(int32_t(quantity * 4u));

        iter++;
        remaining -= quantity;
      } while (remaining > 0u);
    }

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, uint32_t(n) * 4u);
    }
  }
  else {
    VecArray pc_vec;
    pc->new_vec_array(pc_vec, pc_count, pc_width, "@tmp_pc_vec");

    // We really want each second register to point to the original d_vec (so we don't have to move afterwards).
    for (size_t i = 0; i < pc_count; i += 2) {
      pc_vec.reassign(i, d_vec[i / 2u]);
    }

    fetch_vec32(pc, pc_vec, s_ptr, uint32_t(n), advance_mode, predicate);

    if (pc_width == VecWidth::k128 || pc_count == 1u) {
      // If there is only a single register to load or all destination registers are XMMs it's actually very simple.
      do {
        uint32_t quantity = bl_min<uint32_t>(remaining, 16u);
        const Vec& dv = d_vec[iter];

        v_prgb32_to_ua_vpermw(pc, dv, permute_predicate, pc_vec[iter], PixelCount{quantity});

        iter++;
        remaining -= quantity;
      } while (remaining > 0u);
    }
    else {
      uint32_t pc_idx = 0;
      do {
        uint32_t quantity = bl_min<uint32_t>(remaining, 64u);
        const Vec& dv = d_vec[iter];

        if (quantity >= 32u) {
          // Two ZMM registers to permute.
          pc->cc->vpermt2w(pc_vec[pc_idx].zmm(), permute_predicate.zmm(), pc_vec[pc_idx + 1].zmm());
          BL_ASSERT(dv.id() == pc_vec[pc_idx].id());

          pc_idx += 2;
        }
        else {
          v_prgb32_to_ua_vpermw(pc, dv, permute_predicate, pc_vec[pc_idx], PixelCount{quantity});
          pc_idx++;
        }

        iter++;
        remaining -= quantity;
      } while (remaining > 0u);
    }
  }

  // Apply the final shift by 8 to get unpacked alpha from [Ax] packed data.
  pc->v_srli_u16(d_vec, d_vec, 8);
}

static void fetch_prgb32_into_ua_avx2(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  VecWidth pc_width = VecWidthUtils::vec_width_of(VecWidth::k256, DataWidth::k32, uint32_t(n));
  size_t pc_count = VecWidthUtils::vec_count_of(pc_width, DataWidth::k32, uint32_t(n));

  VecArray tVec;
  pc->new_vec_array(tVec, pc_count, pc_width, "@tmp_vec");
  fetch_vec32(pc, tVec, s_ptr, uint32_t(n), advance_mode, predicate);

  uint32_t d_idx = 0;
  uint32_t t_idx = 0;
  uint32_t remaining = uint32_t(n);

  pc->v_srli_u32(tVec, tVec, 24);

  do {
    uint32_t quantity = bl_min<uint32_t>(remaining, 16u);

    const Vec& dv = d_vec[d_idx];
    const Vec& sv0 = tVec[t_idx + 0];

    if (quantity == 16u) {
      const Vec& sv1 = tVec[t_idx + 1];

      pc->v_packs_i32_u16(sv0, sv0, sv1);
      pc->v_swizzle_u64x4(dv.ymm(), sv0, swizzle(3, 1, 2, 0));

      t_idx += 2;
    }
    else if (quantity == 8u) {
      pc->v_packs_i32_u16(sv0, sv0, sv0);
      pc->v_swizzle_u64x4(dv.ymm(), sv0, swizzle(3, 1, 2, 0));

      t_idx++;
    }
    else {
      pc->v_packs_i32_u16(dv.xmm(), sv0.xmm(), sv0.xmm());

      t_idx++;
    }

    d_idx++;
    remaining -= quantity;
  } while (remaining > 0u);
}
#endif // BL_JIT_ARCH_X86

void fetch_prgb32_into_pa(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
#if defined(BL_JIT_ARCH_X86)
  if (pc->has_avx512() && n >= PixelCount(4)) {
    fetch_prgb32_into_pa_avx512(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }
  else if (pc->has_avx2() && d_vec.is_vec256() && n >= PixelCount(8)) {
    fetch_prgb32_into_pa_avx2(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }
#endif // BL_JIT_ARCH_X86

  VecWidth pc_width = VecWidthUtils::vec_width_of(VecWidth::k128, DataWidth::k32, uint32_t(n));
  size_t pc_count = VecWidthUtils::vec_count_of(pc_width, DataWidth::k32, uint32_t(n));

  VecArray tVec;
  pc->new_vec_array(tVec, pc_count, pc_width, "@tmp_vec");

  // We really want each fourth register to point to the original d_vec (so we don't have to move afterwards).
  for (size_t i = 0; i < pc_count; i += 4) {
    tVec.reassign(i, d_vec[i / 4u]);
  }

  fetch_vec32(pc, tVec, s_ptr, uint32_t(n), advance_mode, predicate);

  size_t d_idx = 0;
  size_t t_idx = 0;
  uint32_t remaining = uint32_t(n);

  pc->v_srli_u32(tVec, tVec, 24);

  do {
    const Vec& dv = d_vec[d_idx];
    uint32_t quantity = bl_min<uint32_t>(remaining, 16u);

    if (quantity > 8u) {
      pc->v_packs_i32_u16(tVec[t_idx + 0], tVec[t_idx + 0], tVec[t_idx + 1]);
      pc->v_packs_i32_u16(tVec[t_idx + 2], tVec[t_idx + 2], tVec[t_idx + 3]);
      pc->v_packs_i16_u8(dv, tVec[t_idx + 0], tVec[t_idx + 2]);

      t_idx += 4;
    }
    else if (quantity > 4u) {
      pc->v_packs_i32_u16(dv, tVec[t_idx + 0], tVec[t_idx + 1]);
      pc->v_packs_i16_u8(dv, dv, dv);

      t_idx += 2;
    }
    else {
      pc->v_packs_i32_u16(dv, tVec[t_idx + 0], tVec[t_idx + 0]);
      pc->v_packs_i16_u8(dv, dv, dv);

      t_idx++;
    }

    d_idx++;
    remaining -= quantity;
  } while (remaining > 0u);
}

void fetch_prgb32_into_ua(PipeCompiler* pc, VecArray& d_vec, const Gp& s_ptr, PixelCount n, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
#if defined(BL_JIT_ARCH_X86)
  if (pc->has_avx512() && n >= PixelCount(8)) {
    fetch_prgb32_into_ua_avx512(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }
  else if (pc->has_avx2() && d_vec.is_vec256() && n >= PixelCount(8)) {
    fetch_prgb32_into_ua_avx2(pc, d_vec, s_ptr, n, advance_mode, predicate);
    return;
  }
#endif // BL_JIT_ARCH_X86

  VecWidth pc_width = VecWidthUtils::vec_width_of(VecWidth::k128, DataWidth::k32, uint32_t(n));
  size_t pc_count = VecWidthUtils::vec_count_of(pc_width, DataWidth::k32, uint32_t(n));

  VecArray tVec;
  pc->new_vec_array(tVec, pc_count, pc_width, "@tmp_vec");

  // We really want each second register to point to the original d_vec (so we don't have to move afterwards).
  for (size_t i = 0; i < pc_count; i += 2) {
    tVec.reassign(i, d_vec[i / 2u]);
  }

  fetch_vec32(pc, tVec, s_ptr, uint32_t(n), advance_mode, predicate);

  size_t d_idx = 0;
  size_t t_idx = 0;
  uint32_t remaining = uint32_t(n);

  pc->v_srli_u32(tVec, tVec, 24);

  do {
    const Vec& dv = d_vec[d_idx];
    uint32_t quantity = bl_min<uint32_t>(remaining, 16u);

    if (quantity > 4u) {
      pc->v_packs_i32_u16(dv, tVec[t_idx + 0], tVec[t_idx + 1]);
      t_idx += 2;
    }
    else {
      pc->v_packs_i32_u16(dv, tVec[t_idx + 0], tVec[t_idx + 0]);
      t_idx++;
    }

    d_idx++;
    remaining -= quantity;
  } while (remaining > 0u);
}

static void fetch_pixels_a8(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, Gp s_ptr, Alignment alignment, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  BL_ASSERT(p.isA8());
  BL_ASSERT(n > PixelCount(1));

  // TODO: Do we need it in general?
  bl_unused(alignment);

  p.set_count(n);

  // It's forbidden to use PA in single-pixel case (scalar mode) and SA in multiple-pixel case (vector mode).
  BL_ASSERT(uint32_t(n != PixelCount(1)) ^ uint32_t(bl_test_flag(flags, PixelFlags::kSA)));

  // It's forbidden to request both - PA and UA.
  BL_ASSERT((flags & (PixelFlags::kPA | PixelFlags::kUA)) != (PixelFlags::kPA | PixelFlags::kUA));

  VecWidth pa_width = pc->vec_width_of(DataWidth::k8, n);
  size_t pa_count = pc->vec_count_of(DataWidth::k8, n);

  VecWidth ua_width = pc->vec_width_of(DataWidth::k16, n);
  size_t ua_count = pc->vec_count_of(DataWidth::k16, n);

  switch (f_info.format()) {
    // A8 <- PRGB32.
    case FormatExt::kPRGB32: {
      if (bl_test_flag(flags, PixelFlags::kPA)) {
        pc->new_vec_array(p.pa, pa_count, pa_width, p.name(), "pa");
        fetch_prgb32_into_pa(pc, p.pa, s_ptr, n, advance_mode, predicate);
      }
      else {
        pc->new_vec_array(p.ua, ua_count, ua_width, p.name(), "ua");
        fetch_prgb32_into_ua(pc, p.ua, s_ptr, n, advance_mode, predicate);
      }
      break;
    }

    // A8 <- A8.
    case FormatExt::kA8: {
      if (bl_test_flag(flags, PixelFlags::kPA)) {
        pc->new_vec_array(p.pa, pa_count, pa_width, p.name(), "pa");
        fetch_mask_a8_into_pa(pc, p.pa, s_ptr, n, advance_mode, predicate);
      }
      else {
        pc->new_vec_array(p.ua, ua_count, ua_width, p.name(), "ua");
        fetch_mask_a8_into_ua(pc, p.ua, s_ptr, n, advance_mode, predicate);
      }
      break;
    }

    default:
      BL_NOT_REACHED();
  }

  satisfy_pixels_a8(pc, p, flags);
}

static void fetch_pixels_rgba32(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, Gp s_ptr, Alignment alignment, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  BL_ASSERT(p.isRGBA32());
  BL_ASSERT(n > PixelCount(1));

  p.set_count(n);

  Mem s_mem = ptr(s_ptr);
  uint32_t src_bpp = f_info.bpp();

  VecWidth pc_width = pc->vec_width_of(DataWidth::k32, n);
  size_t pc_count = VecWidthUtils::vec_count_of(pc_width, DataWidth::k32, n);

  VecWidth uc_width = pc->vec_width_of(DataWidth::k64, n);
  size_t uc_count = VecWidthUtils::vec_count_of(uc_width, DataWidth::k64, n);

  switch (f_info.format()) {
    // RGBA32 <- PRGB32 | XRGB32.
    case FormatExt::kPRGB32:
    case FormatExt::kXRGB32: {
      if (!predicate.is_empty()) {
        pc->new_vec_array(p.pc, pc_count, pc_width, p.name(), "pc");
        fetch_predicated_vec32(pc, p.pc, s_ptr, uint32_t(n), advance_mode, predicate);
      }
      else {
        switch (uint32_t(n)) {
          case 1: {
            pc->new_vec128_array(p.pc, 1, p.name(), "pc");
            pc->v_loada32(p.pc[0], s_mem);

            break;
          }

          case 2: {
#if defined(BL_JIT_ARCH_X86)
            if (bl_test_flag(flags, PixelFlags::kUC) && pc->has_sse4_1()) {
              pc->new_vec128_array(p.uc, 1, p.name(), "uc");
              pc->v_cvt_u8_lo_to_u16(p.pc[0].xmm(), s_mem);
            }
            else
#endif // BL_JIT_ARCH_X86
            {
              pc->new_vec128_array(p.pc, 1, p.name(), "pc");
              pc->v_loadu64(p.pc[0], s_mem);
            }

            break;
          }

          case 4: {
#if defined(BL_JIT_ARCH_X86)
            if (!bl_test_flag(flags, PixelFlags::kPC) && pc->use_256bit_simd()) {
              pc->new_vec256_array(p.uc, 1, p.name(), "uc");
              pc->v_cvt_u8_lo_to_u16(p.uc[0].ymm(), s_mem);
            }
            else if (!bl_test_flag(flags, PixelFlags::kPC) && pc->has_sse4_1()) {
              pc->new_vec128_array(p.uc, 2, p.name(), "uc");
              pc->v_cvt_u8_lo_to_u16(p.uc[0].xmm(), s_mem);
              s_mem.add_offset_lo32(8);
              pc->v_cvt_u8_lo_to_u16(p.uc[1].xmm(), s_mem);
            }
            else
#endif // BL_JIT_ARCH_X86
            {
              pc->new_vec128_array(p.pc, 1, p.name(), "pc");
              pc->v_loada128(p.pc[0], s_mem, alignment);
            }

            break;
          }

          case 8:
          case 16:
          case 32: {
#if defined(BL_JIT_ARCH_X86)
            if (pc->vec_width() >= VecWidth::k256) {
              if (bl_test_flag(flags, PixelFlags::kPC)) {
                pc->new_vec_array(p.pc, pc_count, pc_width, p.name(), "pc");
                for (uint32_t i = 0; i < pc_count; i++) {
                  pc->v_loadavec(p.pc[i], s_mem, alignment);
                  s_mem.add_offset_lo32(int32_t(p.pc[i].size()));
                }
              }
              else {
                pc->new_vec_array(p.uc, uc_count, uc_width, p.name(), "uc");
                for (uint32_t i = 0; i < uc_count; i++) {
                  pc->v_cvt_u8_lo_to_u16(p.uc[i], s_mem);
                  s_mem.add_offset_lo32(int32_t(p.uc[i].size() / 2u));
                }
              }
            }
            else if (!bl_test_flag(flags, PixelFlags::kPC) && pc->has_sse4_1()) {
              pc->new_vec128_array(p.uc, uc_count, p.name(), "uc");
              for (uint32_t i = 0; i < uc_count; i++) {
                pc->v_cvt_u8_lo_to_u16(p.uc[i], s_mem);
                s_mem.add_offset_lo32(8);
              }
            }
            else
#endif // BL_JIT_ARCH_X86
            {
              pc->new_vec128_array(p.pc, pc_count, p.name(), "pc");
              pc->v_loadavec(p.pc, s_mem, alignment);
            }

            break;
          }

          default:
            BL_NOT_REACHED();
        }

        if (advance_mode == AdvanceMode::kAdvance) {
          pc->add(s_ptr, s_ptr, uint32_t(n) * src_bpp);
        }
      }

      if (f_info.format() == FormatExt::kXRGB32) {
        fill_alpha_channel(pc, p);
      }

      break;
    }

    // RGBA32 <- A8.
    case FormatExt::kA8: {
      if (bl_test_flag(flags, PixelFlags::kPC)) {
        pc->new_vec_array(p.pc, pc_count, pc_width, p.name(), "pc");
        fetch_mask_a8_into_pc(pc, p.pc, s_ptr, n, advance_mode, predicate);
      }
      else {
        pc->new_vec_array(p.uc, uc_count, uc_width, p.name(), "uc");
        fetch_mask_a8_into_uc(pc, p.uc, s_ptr, n, advance_mode, predicate);
      }

      break;
    }

    // RGBA32 <- Unknown?
    default:
      BL_NOT_REACHED();
  }

  satisfy_pixels_rgba32(pc, p, flags);
}

void fetch_pixel(PipeCompiler* pc, Pixel& p, PixelFlags flags, PixelFetchInfo f_info, Mem s_mem) noexcept {
  p.set_count(PixelCount{1u});

  switch (p.type()) {
    case PixelType::kA8: {
      switch (f_info.format()) {
        case FormatExt::kPRGB32: {
          p.sa = pc->new_gp32("a");
#if defined(BL_JIT_ARCH_X86)
          s_mem.add_offset(f_info.fetch_alpha_offset());
          pc->load_u8(p.sa, s_mem);
#else
          if (f_info.fetch_alpha_offset() == 0) {
            pc->load_u8(p.sa, s_mem);
          }
          else {
            pc->load_u32(p.sa, s_mem);
            pc->shr(p.sa, p.sa, 24);
          }
#endif
          break;
        }

        case FormatExt::kXRGB32: {
          p.sa = pc->new_gp32("a");
          pc->mov(p.sa, 255);
          break;
        }

        case FormatExt::kA8: {
          p.sa = pc->new_gp32("a");
          pc->load_u8(p.sa, s_mem);
          break;
        }

        default:
          BL_NOT_REACHED();
      }

      satisfy_pixels_a8(pc, p, flags);
      return;
    }

    case PixelType::kRGBA32: {
      switch (f_info.format()) {
        case FormatExt::kA8: {
          if (bl_test_flag(flags, PixelFlags::kPC)) {
            pc->new_vec128_array(p.pc, 1, p.name(), "pc");

#if defined(BL_JIT_ARCH_X86)
            if (!pc->has_avx2()) {
              Gp tmp = pc->new_gp32("tmp");
              pc->load_u8(tmp, s_mem);
              pc->mul(tmp, tmp, 0x01010101u);
              pc->s_mov_u32(p.pc[0], tmp);
            }
            else
            {
              pc->v_broadcast_u8(p.pc[0], s_mem);
            }
#else
            pc->v_load8(p.pc[0], s_mem);
#endif // BL_JIT_ARCH_X86
          }
          else {
            pc->new_vec128_array(p.uc, 1, p.name(), "uc");

#if defined(BL_JIT_ARCH_X86)
            if (!pc->has_avx2()) {
              pc->v_load8(p.uc[0], s_mem);
              pc->v_swizzle_lo_u16x4(p.uc[0], p.uc[0], swizzle(0, 0, 0, 0));
            }
            else
            {
              pc->v_broadcast_u8(p.uc[0], s_mem);
              pc->v_cvt_u8_lo_to_u16(p.uc[0], p.uc[0]);
            }
#else
            pc->v_load8(p.pc[0], s_mem);
            pc->v_broadcast_u16(p.pc[0], p.pc[0]);
#endif
          }
          break;
        }

        // RGBA32 <- PRGB32 | XRGB32.
        case FormatExt::kPRGB32:
        case FormatExt::kXRGB32: {
          pc->new_vec128_array(p.pc, 1, p.name(), "pc");
          pc->v_loada32(p.pc[0], s_mem);
          break;
        }

        default:
          BL_NOT_REACHED();
      }

      satisfy_pixels_rgba32(pc, p, flags);
      return;
    }

    default:
      BL_NOT_REACHED();
  }
}

void fetch_pixels(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, const Gp& s_ptr, Alignment alignment, AdvanceMode advance_mode) noexcept {
  fetch_pixels(pc, p, n, flags, f_info, s_ptr, alignment, advance_mode, pc->empty_predicate());
}

void fetch_pixels(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, const Gp& s_ptr, Alignment alignment, AdvanceMode advance_mode, PixelPredicate& predicate) noexcept {
  if (n == PixelCount(1)) {
    BL_ASSERT(predicate.is_empty());
    fetch_pixel(pc, p, flags, f_info, mem_ptr(s_ptr));

    if (advance_mode == AdvanceMode::kAdvance) {
      pc->add(s_ptr, s_ptr, f_info.bpp());
    }
    return;
  }

  switch (p.type()) {
    case PixelType::kA8:
      fetch_pixels_a8(pc, p, n, flags, f_info, s_ptr, alignment, advance_mode, predicate);
      break;

    case PixelType::kRGBA32:
      fetch_pixels_rgba32(pc, p, n, flags, f_info, s_ptr, alignment, advance_mode, predicate);
      break;

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::Jit::FetchUtils - Satisfy Pixels
// ==============================================

static void satisfy_pixels_a8(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept {
  BL_ASSERT(p.type() == PixelType::kA8);
  BL_ASSERT(p.count() != PixelCount(0));

  // Scalar mode uses only SA.
  if (p.count() == PixelCount(1)) {
    BL_ASSERT( bl_test_flag(flags, PixelFlags::kSA));
    BL_ASSERT(!bl_test_flag(flags, PixelFlags::kPA | PixelFlags::kUA));

    return;
  }

  if (bl_test_flag(flags, PixelFlags::kPA) && p.pa.is_empty()) {
    // Either PA or UA, but never both.
    BL_ASSERT(!bl_test_flag(flags, PixelFlags::kUA));

    _x_pack_pixel(pc, p.pa, p.ua, uint32_t(p.count()), p.name(), "pa");
  }
  else if (bl_test_flag(flags, PixelFlags::kUA) && p.ua.is_empty()) {
    // Either PA or UA, but never both.
    BL_ASSERT(!bl_test_flag(flags, PixelFlags::kPA));

    _x_unpack_pixel(pc, p.ua, p.pa, uint32_t(p.count()), p.name(), "ua");
  }

  if (bl_test_flag(flags, PixelFlags::kPI) && p.pi.is_empty()) {
    if (!p.pa.is_empty()) {
      pc->new_vec_array(p.pi, p.pa.size(), p.pa[0], p.name(), "pi");
      pc->v_not_u32(p.pi, p.pa);
    }
    else {
      // TODO: [JIT] UNIMPLEMENTED: A8 pipeline - finalize satisfy-pixel.
      BL_ASSERT(false);
    }
  }

  if (bl_test_flag(flags, PixelFlags::kUA | PixelFlags::kUI)) {
    if (p.ua.is_empty()) {
      // TODO: [JIT] UNIMPLEMENTED: A8 pipeline - finalize satisfy-pixel.
      BL_ASSERT(false);
    }
  }
}

static void satisfy_pixels_rgba32(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept {
  BL_ASSERT(p.type() == PixelType::kRGBA32);
  BL_ASSERT(p.count() != PixelCount(0));

  if (bl_test_flag(flags, PixelFlags::kPA | PixelFlags::kPI))
    flags |= PixelFlags::kPC;

  // Quick reject if all flags were satisfied already or no flags were given.
  if ((!bl_test_flag(flags, PixelFlags::kPC) || !p.pc.is_empty()) &&
      (!bl_test_flag(flags, PixelFlags::kPA) || !p.pa.is_empty()) &&
      (!bl_test_flag(flags, PixelFlags::kPI) || !p.pi.is_empty()) &&
      (!bl_test_flag(flags, PixelFlags::kUC) || !p.uc.is_empty()) &&
      (!bl_test_flag(flags, PixelFlags::kUA) || !p.ua.is_empty()) &&
      (!bl_test_flag(flags, PixelFlags::kUI) || !p.ui.is_empty())) {
    return;
  }

  // Only fetch unpacked alpha if we already have unpacked pixels. Wait otherwise as fetch flags may contain
  // `PixelFlags::kUC`, which is handled below. This is an optimization for cases in which the caller wants
  // packed RGBA and unpacked alpha.
  if (bl_test_flag(flags, PixelFlags::kUA | PixelFlags::kUI) && p.ua.is_empty() && !p.uc.is_empty()) {
    // Emit pshuflw/pshufhw sequence for every unpacked pixel.
    pc->new_vec_array(p.ua, p.uc.size(), p.uc[0], p.name(), "ua");

#if defined(BL_JIT_ARCH_X86)
    if (!pc->has_avx()) {
      pc->v_expand_alpha_16(p.ua, p.uc, true);
    }
    else
#endif // BL_JIT_ARCH_X86
    {
      pc->v_swizzlev_u8(p.ua, p.uc, pc->simd_const(&pc->ct<CommonTable>().swizu8_32xxxxxx10xxxxxx_to_3232323210101010, Bcst::kNA, p.ua));
    }
  }

  // Pack or unpack a sequence.
  //
  // The following code handles packing or unpacking pixels. Typically, depending on a fetcher, either
  // packed or unpacked pixels are assigned to a `Pixel`. Then, the consumer of that pixel decides which
  // format to use. So, if there is a mismatch, we have to emit a pack/unpack sequence. Unpacked pixels
  // are needed for almost everything except some special cases like SRC_COPY and PLUS without a mask.

  // Either PC or UC, but never both.
  BL_ASSERT((flags & (PixelFlags::kPC | PixelFlags::kUC)) != (PixelFlags::kPC | PixelFlags::kUC));

  if (bl_test_flag(flags, PixelFlags::kPC) && p.pc.is_empty()) {
    _x_pack_pixel(pc, p.pc, p.uc, uint32_t(p.count()) * 4u, p.name(), "pc");
  }
  else if (bl_test_flag(flags, PixelFlags::kUC) && p.uc.is_empty()) {
    _x_unpack_pixel(pc, p.uc, p.pc, uint32_t(p.count()) * 4, p.name(), "uc");
  }

  if (bl_test_flag(flags, PixelFlags::kPA | PixelFlags::kPI)) {
    if (bl_test_flag(flags, PixelFlags::kPA) && p.pa.is_empty()) {
      pc->new_vec_array(p.pa, p.pc.size(), p.pc[0], p.name(), "pa");
      pc->v_swizzlev_u8(p.pa, p.pc, pc->simd_const(&pc->ct<CommonTable>().swizu8_3xxx2xxx1xxx0xxx_to_3333222211110000, Bcst::kNA, p.pc));
    }

    if (bl_test_flag(flags, PixelFlags::kPI) && p.pi.is_empty()) {
      pc->new_vec_array(p.pi, p.pc.size(), p.pc[0], p.name(), "pi");
      if (p.pa.size()) {
        pc->v_not_u32(p.pi, p.pa);
      }
      else {
        pc->v_swizzlev_u8(p.pi, p.pc, pc->simd_const(&pc->ct<CommonTable>().swizu8_3xxx2xxx1xxx0xxx_to_3333222211110000, Bcst::kNA, p.pc));
        pc->v_not_u32(p.pi, p.pi);
      }
    }
  }

  // Unpack alpha from either packed or unpacked pixels.
  if (bl_test_flag(flags, PixelFlags::kUA | PixelFlags::kUI) && p.ua.is_empty()) {
    // This time we have to really fetch A8/IA8, if we haven't before.
    BL_ASSERT(!p.pc.is_empty() || !p.uc.is_empty());

    size_t ua_count = pc->vec_count_of(DataWidth::k64, p.count());
    BL_ASSERT(ua_count <= OpArray::kMaxSize);

    if (!p.uc.is_empty()) {
      pc->new_vec_array(p.ua, ua_count, p.uc[0], p.name(), "ua");
#if defined(BL_JIT_ARCH_X86)
      if (!pc->has_avx()) {
        pc->v_expand_alpha_16(p.ua, p.uc, p.count() > PixelCount(1));
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        pc->v_swizzlev_u8(p.ua, p.uc, pc->simd_const(&pc->ct<CommonTable>().swizu8_32xxxxxx10xxxxxx_to_3232323210101010, Bcst::kNA, p.ua));
      }
    }
    else {
      if (p.count() <= PixelCount(2)) {
        pc->new_vec128_array(p.ua, ua_count, p.name(), "ua");
#if defined(BL_JIT_ARCH_X86)
        if (p.count() == PixelCount(1)) {
          pc->v_swizzle_lo_u16x4(p.ua[0], p.pc[0], swizzle(1, 1, 1, 1));
          pc->v_srli_u16(p.ua[0], p.ua[0], 8);
        }
        else if (pc->has_avx()) {
          pc->v_swizzlev_u8(p.ua[0], p.pc[0], pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxxx1xxx0xxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, p.ua[0]));
        }
        else {
          pc->v_swizzle_lo_u16x4(p.ua[0], p.pc[0], swizzle(3, 3, 1, 1));
          pc->v_swizzle_u32x4(p.ua[0], p.ua[0], swizzle(1, 1, 0, 0));
          pc->v_srli_u16(p.ua[0], p.ua[0], 8);
        }
#else
        pc->v_swizzlev_u8(p.ua[0], p.pc[0], pc->simd_const(&pc->ct<CommonTable>().swizu8_xxxxxxxx1xxx0xxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, p.ua[0]));
#endif
      }
      else {
        VecWidth uc_width = pc->vec_width_of(DataWidth::k64, p.count());
        pc->new_vec_array(p.ua, ua_count, uc_width, p.name(), "ua");

#if defined(BL_JIT_ARCH_X86)
        if (uc_width == VecWidth::k512) {
          if (ua_count == 1) {
            pc->v_cvt_u8_lo_to_u16(p.ua[0], p.pc[0].ymm());
          }
          else {
            pc->v_extract_v256(p.ua.odd().ymm(), p.pc.zmm(), 1);
            pc->v_cvt_u8_lo_to_u16(p.ua.even(), p.pc.ymm());
            pc->v_cvt_u8_lo_to_u16(p.ua.odd(), p.ua.odd().ymm());
          }

          pc->v_swizzlev_u8(p.ua, p.ua, pc->simd_const(&pc->ct<CommonTable>().swizu8_32xxxxxx10xxxxxx_to_3232323210101010, Bcst::kNA, p.ua));
        }
        else if (uc_width == VecWidth::k256) {
          if (ua_count == 1) {
            pc->v_cvt_u8_lo_to_u16(p.ua[0], p.pc[0].xmm());
          }
          else {
            pc->v_extract_v128(p.ua.odd().xmm(), p.pc.ymm(), 1);
            pc->v_cvt_u8_lo_to_u16(p.ua.even(), p.pc.xmm());
            pc->v_cvt_u8_lo_to_u16(p.ua.odd(), p.ua.odd().xmm());
          }

          pc->v_swizzlev_u8(p.ua, p.ua, pc->simd_const(&pc->ct<CommonTable>().swizu8_32xxxxxx10xxxxxx_to_3232323210101010, Bcst::kNA, p.ua));
        }
        else
#endif // BL_JIT_ARCH_X86
        {
          for (uint32_t i = 0; i < p.pc.size(); i++)
            pc->xExtractUnpackedAFromPackedARGB32_4(p.ua[i * 2], p.ua[i * 2 + 1], p.pc[i]);
        }
      }
    }
  }

  if (bl_test_flag(flags, PixelFlags::kUI) && p.ui.is_empty()) {
    if (pc->has_non_destructive_src() || bl_test_flag(flags, PixelFlags::kUA)) {
      pc->new_vec_array(p.ui, p.ua.size(), p.ua[0], p.name(), "ui");
      pc->v_inv255_u16(p.ui, p.ua);
    }
    else {
      p.ui.init(p.ua);
      pc->v_inv255_u16(p.ui, p.ua);

      p.ua.reset();
      pc->rename(p.ui, p.name(), "ui");
    }
  }
}

void satisfy_pixels(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept {
  BL_ASSERT(p.count() != PixelCount(0));

  switch (p.type()) {
    case PixelType::kA8:
      satisfy_pixels_a8(pc, p, flags);
      break;

    case PixelType::kRGBA32:
      satisfy_pixels_rgba32(pc, p, flags);
      break;

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::Jit::FetchUtils - Satisfy Solid Pixels
// ====================================================

static void satisfy_solid_pixels_a8(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept {
  BL_ASSERT(p.type() == PixelType::kA8);
  BL_ASSERT(p.count() != PixelCount(0));

  VecWidth vw = pc->vec_width();

  if (bl_test_flag(flags, PixelFlags::kPA) && p.pa.is_empty()) {
    BL_ASSERT(!p.ua.is_empty());
    pc->new_vec_array(p.pa, 1, vw, p.name(), "pa");
    pc->v_packs_i16_u8(p.pa[0], p.ua[0], p.ua[0]);
  }

  if (bl_test_flag(flags, PixelFlags::kPI) && p.pi.is_empty()) {
    if (!p.pa.is_empty()) {
      pc->new_vec_array(p.pi, 1, vw, p.name(), "pi");
      pc->v_not_u32(p.pi[0], p.pa[0]);
    }
    else {
      BL_ASSERT(!p.ua.is_empty());
      pc->new_vec_array(p.pi, 1, vw, p.name(), "pi");
      pc->v_packs_i16_u8(p.pi[0], p.ua[0], p.ua[0]);
      pc->v_not_u32(p.pi[0], p.pi[0]);
    }
  }

  // TODO: [JIT] UNIMPLEMENTED: A8 pipeline - finalize solid-alpha.
}

static void satisfy_solid_pixels_rgba32(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept {
  BL_ASSERT(p.type() == PixelType::kRGBA32);
  BL_ASSERT(p.count() != PixelCount(0));

  VecWidth vw = pc->vec_width();

  if (bl_test_flag(flags, PixelFlags::kPC) && p.pc.is_empty()) {
    BL_ASSERT(!p.uc.is_empty());

    pc->new_vec_array(p.pc, 1, vw, p.name(), "pc");
    pc->v_mov(p.pc[0], p.uc[0]);
    pc->v_packs_i16_u8(p.pc[0], p.pc[0], p.pc[0]);
  }

  if (bl_test_flag(flags, PixelFlags::kUC) && p.uc.is_empty()) {
    BL_ASSERT(!p.pc.is_empty());

    pc->new_vec_array(p.uc, 1, vw, p.name(), "uc");
    pc->v_cvt_u8_lo_to_u16(p.uc[0], p.pc[0]);
  }

  if (bl_test_flag(flags, PixelFlags::kPA | PixelFlags::kPI) && p.pa.is_empty()) {
    BL_ASSERT(!p.pc.is_empty() || !p.uc.is_empty());

    // TODO: [JIT] PORTABILITY: Requires SSSE3 on X86.
    pc->new_vec_array(p.pa, 1, vw, p.name(), "pa");
    if (!p.pc.is_empty()) {
      pc->v_swizzlev_u8(p.pa[0], p.pc[0], pc->simd_const(&pc->ct<CommonTable>().swizu8_3xxx2xxx1xxx0xxx_to_3333222211110000, Bcst::kNA, p.pa[0]));
    }
    else if (!p.uc.is_empty()) {
      pc->v_swizzlev_u8(p.pa[0], p.uc[0], pc->simd_const(&pc->ct<CommonTable>().swizu8_x1xxxxxxx0xxxxxx_to_1111000011110000, Bcst::kNA, p.pa[0]));
    }
  }

  if (bl_test_flag(flags, PixelFlags::kUA) && p.ua.is_empty()) {
    pc->new_vec_array(p.ua, 1, vw, p.name(), "ua");

    if (!p.pa.is_empty()) {
      pc->v_cvt_u8_lo_to_u16(p.ua[0], p.pa[0]);
    }
    else if (!p.uc.is_empty()) {
      pc->v_swizzle_lo_u16x4(p.ua[0], p.uc[0], swizzle(3, 3, 3, 3));
      pc->v_swizzle_u32x4(p.ua[0], p.ua[0], swizzle(1, 0, 1, 0));
    }
    else {
      pc->v_swizzle_lo_u16x4(p.ua[0], p.pc[0], swizzle(1, 1, 1, 1));
      pc->v_swizzle_u32x4(p.ua[0], p.ua[0], swizzle(1, 0, 1, 0));
      pc->v_srli_u16(p.ua[0], p.ua[0], 8);
    }
  }

  if (bl_test_flag(flags, PixelFlags::kPI)) {
    if (!p.pa.is_empty()) {
      pc->new_vec_array(p.pi, 1, vw, p.name(), "pi");
      pc->v_not_u32(p.pi[0], p.pa[0]);
    }
  }

  if (bl_test_flag(flags, PixelFlags::kUI) && p.ui.is_empty()) {
    pc->new_vec_array(p.ui, 1, vw, p.name(), "ui");

    if (!p.ua.is_empty()) {
      pc->v_inv255_u16(p.ui[0], p.ua[0]);
    }
    else if (!p.uc.is_empty()) {
      pc->v_swizzle_lo_u16x4(p.ui[0], p.uc[0], swizzle(3, 3, 3, 3));
      pc->v_swizzle_u32x4(p.ui[0], p.ui[0], swizzle(1, 0, 1, 0));
      pc->v_inv255_u16(p.ui[0], p.ui[0]);
    }
    else {
      pc->v_swizzle_lo_u16x4(p.ui[0], p.pc[0], swizzle(1, 1, 1, 1));
      pc->v_swizzle_u32x4(p.ui[0], p.ui[0], swizzle(1, 0, 1, 0));
      pc->v_srli_u16(p.ui[0], p.ui[0], 8);
      pc->v_inv255_u16(p.ui[0], p.ui[0]);
    }
  }
}

void satisfy_solid_pixels(PipeCompiler* pc, Pixel& p, PixelFlags flags) noexcept {
  BL_ASSERT(p.count() != PixelCount(0));

  switch (p.type()) {
    case PixelType::kA8:
      satisfy_solid_pixels_a8(pc, p, flags);
      break;

    case PixelType::kRGBA32:
      satisfy_solid_pixels_rgba32(pc, p, flags);
      break;

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::Jit::FetchUtils - Miscellaneous
// =============================================

// Emits a pixel packing sequence.
void _x_pack_pixel(PipeCompiler* pc, VecArray& px, VecArray& ux, uint32_t n, const char* prefix, const char* px_name) noexcept {
  BL_ASSERT( px.is_empty());
  BL_ASSERT(!ux.is_empty());

#if defined(BL_JIT_ARCH_X86)
  if (pc->has_avx512() && ux[0].reg_type() >= asmjit::RegType::kVec256) {
    VecWidth px_width = pc->vec_width_of(DataWidth::k8, uint32_t(n));
    size_t px_count = pc->vec_count_of(DataWidth::k8, uint32_t(n));
    BL_ASSERT(px_count <= OpArray::kMaxSize);

    pc->new_vec_array(px, px_count, px_width, prefix, px_name);

    if (ux.size() == 1) {
      // Pack ZMM->YMM or YMM->XMM.
      BL_ASSERT(px_count == 1);
      pc->cc->vpmovwb(px[0], ux[0]);
      ux.reset();
      return;
    }
    else if (ux[0].reg_type() >= asmjit::RegType::kVec512) {
      // Pack ZMM to ZMM.
      VecArray px_tmp;
      pc->new_vec256_array(px_tmp, ux.size(), prefix, "px_tmp");

      for (size_t i = 0; i < ux.size(); i++)
        pc->cc->vpmovwb(px_tmp[i].ymm(), ux[i]);

      for (size_t i = 0; i < ux.size(); i += 2)
        pc->cc->vinserti32x8(px[i / 2u].zmm(), px_tmp[i].zmm(), px_tmp[i + 1u].ymm(), 1);

      ux.reset();
      return;
    }
  }

  if (pc->has_avx()) {
    size_t px_count = pc->vec_count_of(DataWidth::k8, n);
    BL_ASSERT(px_count <= OpArray::kMaxSize);

    if (ux[0].reg_type() >= asmjit::RegType::kVec256) {
      if (ux.size() == 1) {
        // Pack YMM to XMM.
        BL_ASSERT(px_count == 1);

        Vec pTmp = pc->new_vec256("pTmp");
        pc->new_vec128_array(px, px_count, prefix, px_name);

        pc->v_packs_i16_u8(pTmp, ux[0], ux[0]);
        pc->v_swizzle_u64x4(px[0].ymm(), pTmp, swizzle(3, 1, 2, 0));
      }
      else {
        pc->new_vec256_array(px, px_count, prefix, px_name);
        pc->v_packs_i16_u8(px, ux.even(), ux.odd());
        pc->v_swizzle_u64x4(px, px, swizzle(3, 1, 2, 0));
      }
    }
    else {
      pc->new_vec128_array(px, px_count, prefix, px_name);
      pc->v_packs_i16_u8(px, ux.even(), ux.odd());
    }
    ux.reset();
  }
  else {
    // NOTE: This is only used by a non-AVX pipeline. Renaming makes no sense when in AVX mode. Additionally,
    // we may need to pack to XMM register from two YMM registers, so the register types don't have to match
    // if the pipeline is using 256-bit SIMD or higher.
    px.init(ux.even());
    pc->rename(px, prefix, px_name);

    pc->v_packs_i16_u8(px, ux.even(), ux.odd());
    ux.reset();
  }
#else
  size_t px_count = pc->vec_count_of(DataWidth::k8, n);
  BL_ASSERT(px_count <= OpArray::kMaxSize);

  pc->new_vec128_array(px, px_count, prefix, px_name);
  pc->v_packs_i16_u8(px, ux.even(), ux.odd());

  ux.reset();
#endif
}

// Emits a pixel unpacking sequence.
void _x_unpack_pixel(PipeCompiler* pc, VecArray& ux, VecArray& px, uint32_t n, const char* prefix, const char* ux_name) noexcept {
  BL_ASSERT( ux.is_empty());
  BL_ASSERT(!px.is_empty());

#if defined(BL_JIT_ARCH_X86)
  VecWidth ux_width = pc->vec_width_of(DataWidth::k16, n);
  size_t ux_count = pc->vec_count_of(DataWidth::k16, n);
  BL_ASSERT(ux_count <= OpArray::kMaxSize);

  if (pc->has_avx()) {
    pc->new_vec_array(ux, ux_count, ux_width, prefix, ux_name);

    if (ux_width == VecWidth::k512) {
      if (ux_count == 1) {
        pc->v_cvt_u8_lo_to_u16(ux[0], px[0].ymm());
      }
      else {
        pc->v_extract_v256(ux.odd().ymm(), px, 1);
        pc->v_cvt_u8_lo_to_u16(ux.even(), px.ymm());
        pc->v_cvt_u8_lo_to_u16(ux.odd(), ux.odd().ymm());
      }
    }
    else if (ux_width == VecWidth::k256 && n >= 16) {
      if (ux_count == 1) {
        pc->v_cvt_u8_lo_to_u16(ux[0], px[0].xmm());
      }
      else {
        pc->v_extract_v128(ux.odd().xmm(), px, 1);
        pc->v_cvt_u8_lo_to_u16(ux.even(), px.xmm());
        pc->v_cvt_u8_lo_to_u16(ux.odd(), ux.odd().xmm());
      }
    }
    else {
      for (size_t i = 0; i < ux_count; i++) {
        if (i & 1)
          pc->v_swizzlev_u8(ux[i], px[i / 2u], pc->simd_const(&pc->ct<CommonTable>().swizu8_76543210xxxxxxxx_to_z7z6z5z4z3z2z1z0, Bcst::kNA, ux[i]));
        else
          pc->v_cvt_u8_lo_to_u16(ux[i], px[i / 2u]);
      }
    }
  }
  else {
    if (n <= 8) {
      ux.init(px[0]);
      pc->v_cvt_u8_lo_to_u16(ux[0], ux[0]);
    }
    else {
      ux._size = px.size() * 2;
      for (size_t i = 0; i < px.size(); i++) {
        ux[i * 2 + 0] = px[i];
        ux[i * 2 + 1] = pc->new_vec128();
        pc->xMovzxBW_LoHi(ux[i * 2 + 0], ux[i * 2 + 1], ux[i * 2 + 0]);
      }
    }

    px.reset();
    pc->rename(ux, prefix, ux_name);
  }
#else
  size_t count = pc->vec_count_of(DataWidth::k16, n);
  BL_ASSERT(count <= OpArray::kMaxSize);

  pc->new_vec_array(ux, count, VecWidth::k128, prefix, ux_name);

  for (size_t i = 0; i < count; i++) {
    if (i & 1)
      pc->v_swizzlev_u8(ux[i], px[i / 2u], pc->simd_const(&pc->ct<CommonTable>().swizu8_76543210xxxxxxxx_to_z7z6z5z4z3z2z1z0, Bcst::kNA, ux[i]));
    else
      pc->v_cvt_u8_lo_to_u16(ux[i], px[i / 2u]);
  }
#endif
}

void x_fetch_unpacked_a8_2x(PipeCompiler* pc, const Vec& dst, PixelFetchInfo f_info, const Mem& src1, const Mem& src0) noexcept {
#if defined(BL_JIT_ARCH_X86)
  Mem m0 = src0;
  Mem m1 = src1;

  if (f_info.format() == FormatExt::kPRGB32) {
    m0.add_offset(f_info.fetch_alpha_offset());
    m1.add_offset(f_info.fetch_alpha_offset());
  }

  if (pc->has_sse4_1()) {
    pc->v_load8(dst, m0);
    pc->v_insert_u8(dst, m1, 2);
  }
  else {
    Gp a_gp = pc->new_gp32("a_gp");
    pc->load_u8(a_gp, m1);
    pc->shl(a_gp, a_gp, 16);
    pc->load_merge_u8(a_gp, m0);
    pc->s_mov_u32(dst, a_gp);
  }
#else
  Vec tmp = pc->new_similar_reg(dst, "@tmp");

  if (f_info.format() == FormatExt::kPRGB32 && f_info.fetch_alpha_offset()) {
    pc->v_loadu32(dst, src0);
    pc->v_loadu32(tmp, src1);
    pc->v_srli_u32(dst, dst, 24);
    pc->cc->ins(dst.b(2), tmp.b(3));
  }
  else {
    pc->v_load8(dst, src0);
    pc->v_load8(tmp, src1);
    pc->cc->ins(dst.b(2), tmp.b(0));
  }
#endif
}

void x_assign_unpacked_alpha_values(PipeCompiler* pc, Pixel& p, PixelFlags flags, const Vec& vec) noexcept {
  bl_unused(flags);

  BL_ASSERT(p.type() != PixelType::kNone);
  BL_ASSERT(p.count() != PixelCount(0));

  Vec v0 = vec;

  if (p.isRGBA32()) {
    switch (uint32_t(p.count())) {
      case 1: {
        pc->v_swizzle_lo_u16x4(v0, v0, swizzle(0, 0, 0, 0));

        p.uc.init(v0);
        break;
      }

      case 2: {
        pc->v_interleave_lo_u16(v0, v0, v0);
        pc->v_swizzle_u32x4(v0, v0, swizzle(1, 1, 0, 0));

        p.uc.init(v0);
        break;
      }

      case 4: {
        Vec v1 = pc->new_vec128("@v1");

        pc->v_interleave_lo_u16(v0, v0, v0);
        pc->v_swizzle_u32x4(v1, v0, swizzle(3, 3, 2, 2));
        pc->v_swizzle_u32x4(v0, v0, swizzle(1, 1, 0, 0));

        p.uc.init(v0, v1);
        break;
      }

      case 8: {
        Vec v1 = pc->new_vec128("@v1");
        Vec v2 = pc->new_vec128("@v2");
        Vec v3 = pc->new_vec128("@v3");

        pc->v_interleave_hi_u16(v2, v0, v0);
        pc->v_interleave_lo_u16(v0, v0, v0);

        pc->v_swizzle_u32x4(v1, v0, swizzle(3, 3, 2, 2));
        pc->v_swizzle_u32x4(v0, v0, swizzle(1, 1, 0, 0));
        pc->v_swizzle_u32x4(v3, v2, swizzle(3, 3, 2, 2));
        pc->v_swizzle_u32x4(v2, v2, swizzle(1, 1, 0, 0));

        p.uc.init(v0, v1, v2, v3);
        break;
      }

      default:
        BL_NOT_REACHED();
    }

    pc->rename(p.uc, "uc");
  }
  else {
    switch (uint32_t(p.count())) {
      case 1: {
        BL_ASSERT(bl_test_flag(flags, PixelFlags::kSA));

        Gp sa = pc->new_gp32("sa");
        pc->s_extract_u16(sa, vec, 0);

        p.sa = sa;
        break;
      }

      default: {
        p.ua.init(vec);
        pc->rename(p.ua, p.name(), "ua");
        break;
      }
    }
  }
}

void fill_alpha_channel(PipeCompiler* pc, Pixel& p) noexcept {
  switch (p.type()) {
    case PixelType::kRGBA32:
      if (!p.pc.is_empty()) pc->vFillAlpha255B(p.pc, p.pc);
      if (!p.uc.is_empty()) pc->vFillAlpha255W(p.uc, p.uc);
      break;

    case PixelType::kA8:
      break;

    default:
      BL_NOT_REACHED();
  }
}

void store_pixels_and_advance(PipeCompiler* pc, const Gp& d_ptr, Pixel& p, PixelCount n, uint32_t bpp, Alignment alignment, PixelPredicate& predicate) noexcept {
  Mem d_mem = mem_ptr(d_ptr);

  switch (bpp) {
    case 1: {
      if (!predicate.is_empty()) {
        // Predicated pixel count must be greater than 1!
        BL_ASSERT(n != PixelCount(1));

        satisfy_pixels(pc, p, PixelFlags::kPA | PixelFlags::kImmutable);
        store_predicated_vec8(pc, d_ptr, p.pa, uint32_t(n), AdvanceMode::kAdvance, predicate);
      }
      else {
        if (n == PixelCount(1)) {
          satisfy_pixels(pc, p, PixelFlags::kSA | PixelFlags::kImmutable);
          pc->store_u8(d_mem, p.sa);
        }
        else {
          satisfy_pixels(pc, p, PixelFlags::kPA | PixelFlags::kImmutable);

          if (n <= PixelCount(16)) {
            pc->v_store_iany(d_mem, p.pa[0], uint32_t(n), alignment);
          }
          else {
            satisfy_pixels(pc, p, PixelFlags::kPA | PixelFlags::kImmutable);

            // TODO: [JIT] OPTIMIZATION: AArch64 - Use v_storeavec with multiple Vec registers to take advantage of STP where possible.
            uint32_t pc_index = 0;
            uint32_t vec_size = p.pa[0].size();
            uint32_t pixels_per_reg = vec_size;

            for (uint32_t i = 0; i < uint32_t(n); i += pixels_per_reg) {
              pc->v_storeavec(d_mem, p.pa[pc_index], alignment);
              if (++pc_index >= p.pa.size())
                pc_index = 0;
              d_mem.add_offset(vec_size);
            }
          }
        }

        pc->add(d_ptr, d_ptr, uint32_t(n));
      }

      break;
    }

    case 4: {
      if (!predicate.is_empty()) {
        satisfy_pixels(pc, p, PixelFlags::kPC | PixelFlags::kImmutable);
        store_predicated_vec32(pc, d_ptr, p.pc, uint32_t(n), AdvanceMode::kAdvance, predicate);
      }
#if defined(BL_JIT_ARCH_X86)
      else if (pc->has_avx512() && n >= PixelCount(2) && !p.uc.is_empty() && p.pc.is_empty()) {
        uint32_t uc_index = 0;
        uint32_t vec_size = p.uc[0].size();
        uint32_t pixels_per_reg = vec_size / 8u;

        for (uint32_t i = 0; i < uint32_t(n); i += pixels_per_reg) {
          pc->cc->vpmovwb(d_mem, p.uc[uc_index]);
          if (++uc_index >= p.uc.size())
            uc_index = 0;
          d_mem.add_offset(vec_size / 2u);
        }
        pc->add(d_ptr, d_ptr, uint32_t(n) * 4);
      }
#endif
      else {
        satisfy_pixels(pc, p, PixelFlags::kPC | PixelFlags::kImmutable);

        if (n <= PixelCount(4)) {
          pc->v_store_iany(d_mem, p.pc[0], uint32_t(n) * 4u, alignment);
        }
        else {
          // TODO: [JIT] OPTIMIZATION: AArch64 - Use v_storeavec with multiple Vec registers to take advantage of STP where possible.
          uint32_t pc_index = 0;
          uint32_t vec_size = p.pc[0].size();
          uint32_t pixels_per_reg = vec_size / 4u;

          for (uint32_t i = 0; i < uint32_t(n); i += pixels_per_reg) {
            pc->v_storeavec(d_mem, p.pc[pc_index], alignment);
            if (++pc_index >= p.pc.size())
              pc_index = 0;
            d_mem.add_offset(vec_size);
          }
        }
        pc->add(d_ptr, d_ptr, uint32_t(n) * 4u);
      }

      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

} // {bl::Pipeline::JIT::FetchUtils}

#endif // !BL_BUILD_NO_JIT
