// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/fetchutilsinlineloops_p.h>

namespace bl::Pipeline::JIT::FetchUtils {

// bl::Pipeline::JIT::FetchUtils - FillSpan & FillRect Loops
// =========================================================

static BL_NOINLINE void emit_mem_fill_sequence(PipeCompiler* pc, Mem d_ptr, Vec s_vec, uint32_t num_bytes, AdvanceMode advance_mode) noexcept {
  uint32_t n = num_bytes;

#if defined(BL_JIT_ARCH_X86)
  if (s_vec.size() > 32 && n <= 32)
    s_vec = s_vec.ymm();

  if (s_vec.size() > 16 && n <= 16)
    s_vec = s_vec.xmm();

  uint32_t vec_size = s_vec.size();
  for (uint32_t i = 0; i < n; i += vec_size) {
    pc->v_storeuvec(d_ptr, s_vec);
    d_ptr.add_offset_lo32(int32_t(vec_size));
  }

  if (advance_mode == AdvanceMode::kAdvance) {
    Gp dPtrBase = d_ptr.base_reg().as<Gp>();
    pc->add(dPtrBase, dPtrBase, num_bytes);
  }
#elif defined(BL_JIT_ARCH_A64)
  BackendCompiler* cc = pc->cc;

  bool post_index = advance_mode == AdvanceMode::kAdvance && !d_ptr.has_offset();
  if (post_index) {
    d_ptr.set_offset_mode(OffsetMode::kPostIndex);
  }

  while (n >= 32u) {
    if (post_index)
      d_ptr.set_offset_lo32(32);

    cc->stp(s_vec, s_vec, d_ptr);
    if (!post_index)
      d_ptr.add_offset_lo32(32);

    n -= 32u;
  }

  for (uint32_t count = 16; count != 0; count >>= 1) {
    if (n >= count) {
      Vec v = s_vec;

      if (post_index) {
        d_ptr.set_offset_lo32(int32_t(count));
      }

      pc->v_store_iany(d_ptr, v, count, Alignment(1));
      if (!post_index)
        d_ptr.add_offset_lo32(int32_t(count));

      n -= count;
    }
  }

  // In case that any of the two pointers had an offset, we have to advance here...
  if (advance_mode == AdvanceMode::kAdvance && !post_index) {
    Gp dPtrBase = d_ptr.base_reg().as<Gp>();
    pc->add(dPtrBase, dPtrBase, num_bytes);
  }
#else
  #error "Unknown architecture"
#endif
}

void inline_fill_span_loop(
  PipeCompiler* pc,
  Gp dst,
  Vec src,
  Gp i,
  uint32_t main_loop_size, uint32_t item_size, uint32_t item_granularity) noexcept {

  BL_ASSERT(IntOps::is_power_of_2(item_size));
  BL_ASSERT(item_size <= 16u);

  uint32_t granularity_in_bytes = item_size * item_granularity;
  uint32_t main_step_in_items = main_loop_size / item_size;

  BL_ASSERT(IntOps::is_power_of_2(granularity_in_bytes));
  BL_ASSERT(main_step_in_items * item_size == main_loop_size);

  BL_ASSERT(main_loop_size >= 16u);
  BL_ASSERT(main_loop_size >= granularity_in_bytes);

  uint32_t k;
  uint32_t vec_size = src.size();

  // Granularity >= 16 Bytes
  // -----------------------

  if (granularity_in_bytes >= 16u) {
    Label L_End = pc->new_label();

    // MainLoop
    // --------

    {
      Label L_MainIter = pc->new_label();
      Label L_MainSkip = pc->new_label();

      pc->j(L_MainSkip, sub_c(i, main_step_in_items));
      pc->bind(L_MainIter);
      emit_mem_fill_sequence(pc, mem_ptr(dst), src, main_loop_size, AdvanceMode::kAdvance);
      pc->j(L_MainIter, sub_nc(i, main_step_in_items));

      pc->bind(L_MainSkip);
      pc->j(L_End, add_z(i, main_step_in_items));
    }

    // TailLoop / TailSequence
    // -----------------------

    if (main_loop_size * 2 > granularity_in_bytes) {
      Label L_TailIter = pc->new_label();

      pc->bind(L_TailIter);
      emit_mem_fill_sequence(pc, mem_ptr(dst), src, granularity_in_bytes, AdvanceMode::kAdvance);
      pc->j(L_TailIter, sub_nz(i, item_granularity));
    }
    else if (main_loop_size * 2 == granularity_in_bytes) {
      emit_mem_fill_sequence(pc, mem_ptr(dst), src, granularity_in_bytes, AdvanceMode::kAdvance);
    }

    pc->bind(L_End);
    return;
  }

  // Granularity == 4 Bytes
  // ----------------------

  if (granularity_in_bytes == 4u) {
    BL_ASSERT(item_size <= 4u);

    uint32_t size_shift = IntOps::ctz(item_size);
    uint32_t align_pattern = ((vec_size - 1u) * item_size) & (vec_size - 1u);

    uint32_t one_step_in_items = 4u >> size_shift;
    uint32_t tail_step_in_items = 16u >> size_shift;

    if (vec_size >= 32u) {
      // Make `i` contain the number of 32-bit units to fill.
      Gp i_ptr = i.clone_as(dst);
      if (item_size != 4u)
        pc->shr(i, i, 2u - size_shift);

#if defined(BL_JIT_ARCH_X86)
      if (pc->has_masked_access_of(4) && pc->has_cpu_hint(CpuHints::kVecMaskedStore)) {
        Label L_MainIter = pc->new_label();
        Label L_MainSkip = pc->new_label();
        Label L_TailIter = pc->new_label();
        Label L_TailSkip = pc->new_label();
        Label L_End = pc->new_label();

        pc->j(L_MainSkip, sub_c(i_ptr, vec_size));

        pc->bind(L_MainIter);
        emit_mem_fill_sequence(pc, mem_ptr(dst), src, vec_size * 4u, AdvanceMode::kAdvance);
        pc->j(L_MainIter, sub_nc(i_ptr, vec_size));

        pc->bind(L_MainSkip);
        pc->j(L_TailSkip, add_s(i_ptr, vec_size - vec_size / 4u));

        pc->bind(L_TailIter);
        emit_mem_fill_sequence(pc, mem_ptr(dst), src, vec_size, AdvanceMode::kAdvance);
        pc->j(L_TailIter, sub_nc(i_ptr, vec_size / 4u));

        pc->bind(L_TailSkip);
        pc->j(L_End, add_z(i_ptr, vec_size / 4u));

        PixelPredicate predicate(vec_size / 4u, PredicateFlags::kNeverFull, i);
        pc->v_store_predicated_u32(mem_ptr(dst), src, vec_size / 4u, predicate);

        pc->lea(dst, mem_ptr(dst, i_ptr, 2));
        pc->bind(L_End);
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        Label L_LargeIter = pc->new_label();
        Label L_SmallIter = pc->new_label();
        Label L_SmallCheck = pc->new_label();
        Label L_TinyCase16 = pc->new_label();
        Label L_TinyCase8 = pc->new_label();
        Label L_TinyCase4 = pc->new_label();
        Label L_TinyCase2 = pc->new_label();
        Label L_End = pc->new_label();

        pc->j(vec_size == 64 ? L_TinyCase16 : L_TinyCase8, sub_c(i_ptr, vec_size / 4u));
        pc->j(L_SmallIter, ucmp_lt(i_ptr, vec_size));

        // Align to a vec_size, but keep two LSB bits in case the alignment is unfixable.
        pc->v_storeuvec(mem_ptr(dst), src);
        pc->add(dst, dst, vec_size);
        pc->lea(i_ptr, mem_ptr(dst, i_ptr, 2));
        pc->and_(dst, dst, -int(vec_size) | 0x3);
        pc->sub(i_ptr, i_ptr, dst);
        pc->sar(i_ptr, i_ptr, 2);
        pc->sub(i_ptr, i_ptr, vec_size);

        pc->bind(L_LargeIter);
        emit_mem_fill_sequence(pc, mem_ptr(dst), src, vec_size * 4, AdvanceMode::kAdvance);
        pc->j(L_LargeIter, sub_ugt(i_ptr, vec_size));

        pc->add(i_ptr, i_ptr, vec_size);
        pc->j(L_SmallCheck);

        pc->bind(L_SmallIter);
        pc->v_storeuvec(mem_ptr(dst), src);
        pc->add(dst, dst, vec_size);
        pc->bind(L_SmallCheck);
        pc->j(L_SmallIter, sub_ugt(i_ptr, vec_size / 4u));

        pc->add_ext(dst, dst, i_ptr, 4, int32_t(vec_size));
        pc->v_storeuvec(mem_ptr(dst, -int(vec_size)), src);
        pc->j(L_End);

        if (vec_size == 64) {
          pc->bind(L_TinyCase16);
          pc->j(L_TinyCase8, bt_z(i, 3));
          pc->v_storeu256(mem_ptr(dst), src);
          pc->add(dst, dst, 32);
        }

        pc->bind(L_TinyCase8);
        pc->j(L_TinyCase4, bt_z(i, 2));
        pc->v_storeu128(mem_ptr(dst), src);
        pc->add(dst, dst, 16);

        pc->bind(L_TinyCase4);
        pc->j(L_TinyCase2, bt_z(i, 1));
        pc->v_storeu64(mem_ptr(dst), src);
        pc->add(dst, dst, 8);

        pc->bind(L_TinyCase2);
        pc->and_(i, i, 0x1);
        pc->shl(i, i, 2);
        pc->add(dst, dst, i_ptr);
        pc->v_storea32(mem_ptr(dst, -4), src);

        pc->bind(L_End);
      }
    }
    else {
      Label L_Finalize = pc->new_label();
      Label L_End = pc->new_label();

      // Preparation / Alignment
      // -----------------------

      {
        pc->j(L_Finalize, ucmp_lt(i, one_step_in_items * (vec_size / 4u)));

        Gp i_ptr = i.clone_as(dst);
        if (size_shift)
          pc->shl(i_ptr, i_ptr, size_shift);
        pc->add(i_ptr, i_ptr, dst);

        pc->v_storeuvec(mem_ptr(dst), src);

        pc->add(dst, dst, src.size());
        pc->and_(dst, dst, -1 ^ int(align_pattern));

        if (size_shift == 0) {
          pc->j(L_End, sub_z(i_ptr, dst));
        }
        else {
          pc->sub(i_ptr, i_ptr, dst);
          pc->j(L_End, shr_z(i_ptr, size_shift));
        }
      }

      // MainLoop
      // --------

      {
        Label L_MainIter = pc->new_label();
        Label L_MainSkip = pc->new_label();

        pc->j(L_MainSkip, sub_c(i, main_step_in_items));

        pc->bind(L_MainIter);
        emit_mem_fill_sequence(pc, mem_ptr(dst), src.v128(), main_loop_size, AdvanceMode::kAdvance);
        pc->j(L_MainIter, sub_nc(i, main_step_in_items));

        pc->bind(L_MainSkip);
        pc->j(L_End, add_z(i, main_step_in_items));
      }

      // TailLoop / TailSequence
      // -----------------------

      if (main_loop_size > vec_size * 2u) {
        Label L_TailIter = pc->new_label();
        Label L_TailSkip = pc->new_label();

        pc->j(L_TailSkip, sub_c(i, tail_step_in_items));

        pc->bind(L_TailIter);
        pc->v_storeavec(mem_ptr(dst), src);
        pc->add(dst, dst, vec_size);
        pc->j(L_TailIter, sub_nc(i, tail_step_in_items));

        pc->bind(L_TailSkip);
        pc->j(L_End, add_z(i, tail_step_in_items));
      }
      else if (main_loop_size >= vec_size * 2u) {
        pc->j(L_Finalize, ucmp_lt(i, tail_step_in_items));

        pc->v_storeavec(mem_ptr(dst), src);
        pc->add(dst, dst, vec_size);
        pc->j(L_End, sub_z(i, tail_step_in_items));
      }

      // Finalize
      // --------

      {
        Label L_Store1 = pc->new_label();

        pc->bind(L_Finalize);
        pc->j(L_Store1, ucmp_lt(i, 8u / item_size));

        pc->v_storeu64(mem_ptr(dst), src);
        pc->add(dst, dst, 8);
        pc->j(L_End, sub_z(i, 8u / item_size));

        pc->bind(L_Store1);
        pc->v_storea32(mem_ptr(dst), src);
        pc->add(dst, dst, 4);
      }

      pc->bind(L_End);
    }

    return;
  }

  // Granularity == 1 Byte
  // ---------------------

  if (granularity_in_bytes == 1) {
    BL_ASSERT(item_size == 1u);

    Label L_Finalize = pc->new_label();
    Label L_End      = pc->new_label();

    // Preparation / Alignment
    // -----------------------

    {
      Label L_Small = pc->new_label();
      Label L_Large = pc->new_label();
      Gp src_gp = pc->new_gp32("src_gp");

      pc->j(L_Large, ucmp_gt(i, 15));
      pc->s_mov_u32(src_gp, src);

      pc->bind(L_Small);
      pc->store_u8(ptr(dst), src_gp);
      pc->inc(dst);
      pc->j(L_Small, sub_nz(i, 1));
      pc->j(L_End);

      pc->bind(L_Large);
      Gp i_ptr = i.clone_as(dst);
      pc->add(i_ptr, i_ptr, dst);

      pc->v_storeu128(mem_ptr(dst), src);
      pc->add(dst, dst, 16);
      pc->and_(dst, dst, -16);

      pc->j(L_End, sub_z(i_ptr, dst));
    }

    // MainLoop
    // --------

    {
      Label L_MainIter = pc->new_label();
      Label L_MainSkip = pc->new_label();

      pc->j(L_MainSkip, sub_c(i, main_loop_size));

      pc->bind(L_MainIter);
      for (k = 0; k < main_loop_size; k += 16u)
        pc->v_storea128(mem_ptr(dst, int(k)), src);
      pc->add(dst, dst, main_loop_size);
      pc->j(L_MainIter, sub_nc(i, main_loop_size));

      pc->bind(L_MainSkip);
      pc->j(L_End, add_z(i, main_loop_size));
    }

    // TailLoop / TailSequence
    // -----------------------

    if (main_loop_size > 32) {
      Label L_TailIter = pc->new_label();
      Label L_TailSkip = pc->new_label();

      pc->j(L_TailSkip, sub_c(i, 16));

      pc->bind(L_TailIter);
      pc->v_storea128(mem_ptr(dst), src);
      pc->add(dst, dst, 16);
      pc->j(L_TailIter, sub_nc(i, 16));

      pc->bind(L_TailSkip);
      pc->j(L_End, add_z(i, 16));
    }
    else if (main_loop_size >= 32) {
      pc->j(L_Finalize, scmp_lt(i, 16));
      pc->v_storea128(mem_ptr(dst, int(k)), src);
      pc->add(dst, dst, 16);
      pc->j(L_End, sub_z(i, 16));
    }

    // Finalize
    // --------

    {
      pc->add(dst, dst, i.clone_as(dst));
      pc->v_storeu128(mem_ptr(dst, -16), src);
    }

    pc->bind(L_End);
    return;
  }

  BL_NOT_REACHED();
}

// Inlines a whole FillRect loop that uses axis-aligned (AA) coordinates, taking advantage of the width of the
// rectangle. In many cases the rendering context has to deal with axis aligned rectangles of various sizes.
// When the width is large it pays off to align the destination pointer, however, when the size is relatively
// small, it doesn't matter whether the destination pointer is aligned or not, and aligning it explicitly can be
// a waste of cycles as all the instructions to align it can be much more expensive than few unaligned stores.
//
// Additionally, when filling a rectangle it pays off to specialize for various widths before we enter the
// scanline loop, because if we don't do that then each scanline iteration would require to do the same checks
// again and again. Based on testing if the number of bytes per iteration is less than 192 or 256 (this depends
// on target architecture and micro-architecture) then it pays off to specialize. Widths larger than 256 bytes
// don't need width specialization and generally benefit from aligning the destination pointer to native vector
// width.
void inline_fill_rect_loop(
  PipeCompiler* pc,
  Gp dst_ptr,
  Gp stride,
  Gp w,
  Gp h,
  Vec src,
  uint32_t item_size, const Label& end) noexcept {

  Label L_End(end);
  Label L_Width_LE_256 = pc->new_label();
  Label L_Width_LE_192 = pc->new_label();
  Label L_Width_LE_160 = pc->new_label();
  Label L_Width_LE_128 = pc->new_label();
  Label L_Width_LE_96 = pc->new_label();
  Label L_Width_LE_64 = pc->new_label();
  Label L_Width_LE_32 = pc->new_label();
  Label L_Width_LE_16 = pc->new_label();
  Label L_Width_LT_8 = pc->new_label();

  Label L_Width_LT_4; // Only used if necessary unit size is less than 4 bytes.
  Label L_Width_LT_2; // Only used if necessary unit size is less than 2 bytes.

  BL_ASSERT(IntOps::is_power_of_2(item_size));
  uint32_t size_shift = IntOps::ctz(item_size);
  uint32_t size_mask = item_size - 1u;

  uint32_t store_alignment = src.size();
  uint32_t store_alignment_mask = store_alignment - 1;

  if (!L_End.is_valid())
    L_End = pc->new_label();

  Gp end_index_a = pc->new_gpz("end_index_a");
  Gp end_index_b = pc->new_gpz("end_index_b");
  Gp src32b = pc->new_gp32("src32");

  VecArray src256b;
  VecArray src512b;
  VecArray src_align_size;

#if defined(BL_JIT_ARCH_X86)
  if (src.is_vec128()) {
    src256b.init(src, src);
    src512b.init(src, src, src, src);
  }
  else {
    src256b.init(src.v256());
    if (src.is_vec256())
      src512b.init(src, src);
    else
      src512b.init(src);
  }
  src = src.v128();
#else
  src256b.init(src, src);
  src512b.init(src, src, src, src);
#endif

  if (store_alignment <= 16)
    src_align_size.init(src);
  else if (store_alignment <= 32)
    src_align_size = src256b;
  else
    src_align_size = src512b;

  pc->mul(end_index_a.r32(), w, item_size);

  pc->j(L_Width_LE_32, ucmp_le(w, 32 >> size_shift));
  pc->j(L_Width_LE_256, ucmp_le(w, 256 >> size_shift));

  // Fill Rect - Width > 256 Bytes
  // -----------------------------

  {
    Label L_ScanlineLoop = pc->new_label();
    Label L_ScanlineEnd = pc->new_label();
    Label L_MainLoop = pc->new_label();
    Label L_MainLoop4x = pc->new_label();
    Label L_MainSkip4x = pc->new_label();

    Gp dst_aligned = pc->new_gpz("dst_aligned");
    Gp i = pc->new_gpz("i");

    pc->bind(L_ScanlineLoop);
    pc->add(i, dst_ptr, end_index_a);
    pc->add(dst_aligned, dst_ptr, store_alignment);
    pc->v_storeuvec(mem_ptr(dst_ptr), src_align_size);
    pc->and_(dst_aligned, dst_aligned, ~uint64_t(store_alignment_mask ^ size_mask));

    if (store_alignment == 64)
      pc->v_storeuvec(mem_ptr(i, -64), src512b);
    else
      pc->v_storeuvec(mem_ptr(i, -32), src256b);

    pc->sub(i, i, dst_aligned);
    pc->shr(i, i, store_alignment == 64u ? 6 : 5);
    pc->j(L_MainSkip4x, sub_c(i.r32(), 4));

    pc->bind(L_MainLoop4x);
    if (store_alignment == 64) {
      pc->v_storeuvec(mem_ptr(dst_aligned), src512b);
      pc->v_storeuvec(mem_ptr(dst_aligned, 64), src512b);
      pc->v_storeuvec(mem_ptr(dst_aligned, 128), src512b);
      pc->v_storeuvec(mem_ptr(dst_aligned, 192), src512b);
      pc->add(dst_aligned, dst_aligned, 256);
    }
    else {
      pc->v_storeuvec(mem_ptr(dst_aligned), src512b);
      pc->v_storeuvec(mem_ptr(dst_aligned, 64), src512b);
      pc->add(dst_aligned, dst_aligned, 128);
    }
    pc->j(L_MainLoop4x, sub_nc(i.r32(), 4));

    pc->bind(L_MainSkip4x);
    pc->j(L_ScanlineEnd, add_z(i.r32(), 4));

    pc->bind(L_MainLoop);
    if (store_alignment == 64) {
      pc->v_storeuvec(mem_ptr(dst_aligned), src512b);
      pc->add(dst_aligned, dst_aligned, 64);
    }
    else {
      pc->v_storeuvec(mem_ptr(dst_aligned), src256b);
      pc->add(dst_aligned, dst_aligned, 32);
    }
    pc->j(L_MainLoop, sub_nz(i.r32(), 1));

    pc->bind(L_ScanlineEnd);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->j(L_ScanlineLoop, sub_nz(h, 1));

    pc->j(L_End);
  }

  // Fill Rect - Width > 192 && Width <= 256 Bytes
  // ---------------------------------------------

  pc->bind(L_Width_LE_256);

  pc->sub(end_index_b, end_index_a, 32);
  pc->sub(end_index_a, end_index_a, 16);

  pc->j(L_Width_LE_128, ucmp_le(w, 128 >> size_shift));
  pc->j(L_Width_LE_192, ucmp_le(w, 192 >> size_shift));

  {
    Label L_ScanlineLoop = pc->new_label();
    Gp dst_end = pc->new_gpz("dst_end");

    pc->bind(L_ScanlineLoop);
    pc->v_storeuvec(mem_ptr(dst_ptr), src512b);
    pc->add(dst_end, dst_ptr, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_ptr, 64), src512b);
    pc->v_storeuvec(mem_ptr(dst_ptr, 128), src512b);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->v_storeuvec(mem_ptr(dst_end, -32), src512b);
    pc->j(L_ScanlineLoop, sub_nz(h, 1));

    pc->j(L_End);
  }

  // Fill Rect - Width > 160 && Width <= 192 Bytes
  // ---------------------------------------------

  // NOTE: This one was added as it seems that memory store pressure is bottlenecking
  // more than an additional branch, especially if the height is not super small.
  pc->bind(L_Width_LE_192);
  pc->j(L_Width_LE_160, ucmp_le(w, 160 >> size_shift));

  {
    Label L_ScanlineLoop = pc->new_label();
    Gp dst_end = pc->new_gpz("dst_end");

    pc->bind(L_ScanlineLoop);
    pc->v_storeuvec(mem_ptr(dst_ptr), src512b);
    pc->add(dst_end, dst_ptr, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_ptr, 64), src512b);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->v_storeuvec(mem_ptr(dst_end, -32), src512b);
    pc->j(L_ScanlineLoop, sub_nz(h, 1));

    pc->j(L_End);
  }

  // Fill Rect - Width > 128 && Width <= 160 Bytes
  // ---------------------------------------------

  // NOTE: This one was added as it seems that memory store pressure is bottlenecking
  // more than an additional branch, especially if the height is not super small.
  pc->bind(L_Width_LE_160);

  {
    Label L_ScanlineLoop = pc->new_label();
    Gp dst_end = pc->new_gpz("dst_end");

    pc->bind(L_ScanlineLoop);
    pc->v_storeuvec(mem_ptr(dst_ptr), src512b);
    pc->add(dst_end, dst_ptr, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_ptr, 64), src512b);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->v_storeuvec(mem_ptr(dst_end), src256b);
    pc->j(L_ScanlineLoop, sub_nz(h, 1));

    pc->j(L_End);
  }

  // Fill Rect - Width > 96 && Width <= 128 Bytes
  // --------------------------------------------

  pc->bind(L_Width_LE_128);
  pc->j(L_Width_LE_64, ucmp_le(w, 64 >> size_shift));
  pc->j(L_Width_LE_96, ucmp_le(w, 96 >> size_shift));

  {
    Label L_ScanlineLoop2x = pc->new_label();
    Gp dst_alt = pc->new_gpz("dst_alt");
    Gp dst_end = pc->new_gpz("dst_end");

    pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
    pc->add(dst_end, dst_ptr, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_ptr,  0), src512b);
    pc->v_storeuvec(mem_ptr(dst_ptr, 64), src256b);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->v_storeuvec(mem_ptr(dst_end    ), src256b);
    pc->j(L_End, sub_z(h, 1));

    pc->bind(L_ScanlineLoop2x);
    pc->add(dst_alt, dst_ptr, stride);
    pc->v_storeuvec(mem_ptr(dst_ptr,  0), src512b);
    pc->add(dst_end, dst_ptr, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_ptr, 64), src256b);
    pc->add_scaled(dst_ptr, stride, 2);
    pc->v_storeuvec(mem_ptr(dst_end    ), src256b);
    pc->add(dst_end, dst_alt, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_alt,  0), src512b);
    pc->v_storeuvec(mem_ptr(dst_alt, 64), src256b);
    pc->v_storeuvec(mem_ptr(dst_end    ), src256b);
    pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

    pc->j(L_End);
  }

  // Fill Rect - Width > 64 && Width <= 96 Bytes
  // --------------------------------------------

  pc->bind(L_Width_LE_96);

  {
    Label L_ScanlineLoop2x = pc->new_label();
    Gp dst_alt = pc->new_gpz("dst_alt");
    Gp dst_end = pc->new_gpz("dst_end");

    pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
    pc->add(dst_end, dst_ptr, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_ptr), src512b);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->v_storeuvec(mem_ptr(dst_end), src256b);
    pc->j(L_End, sub_z(h, 1));

    pc->bind(L_ScanlineLoop2x);
    pc->add(dst_alt, dst_ptr, stride);
    pc->v_storeuvec(mem_ptr(dst_ptr), src512b);
    pc->add(dst_end, dst_ptr, end_index_b);
    pc->add_scaled(dst_ptr, stride, 2);
    pc->v_storeuvec(mem_ptr(dst_alt), src512b);
    pc->add(dst_alt, dst_alt, end_index_b);
    pc->v_storeuvec(mem_ptr(dst_end), src256b);
    pc->v_storeuvec(mem_ptr(dst_alt), src256b);
    pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

    pc->j(L_End);
  }

  // Fill Rect - Width > 32 && Width <= 64 Bytes
  // -------------------------------------------

  pc->bind(L_Width_LE_64);

  {
    Label L_ScanlineLoop2x = pc->new_label();
    Gp dst_alt = pc->new_gpz("dst_alt");

    pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
    pc->v_storeu128(mem_ptr(dst_ptr, end_index_a), src);
    pc->v_storeu128(mem_ptr(dst_ptr, end_index_b), src);
    pc->v_storeuvec(mem_ptr(dst_ptr), src256b);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->j(L_End, sub_z(h, 1));

    pc->bind(L_ScanlineLoop2x);
    pc->add(dst_alt, dst_ptr, stride);
    pc->v_storeu128(mem_ptr(dst_ptr, end_index_a), src);
    pc->v_storeu128(mem_ptr(dst_ptr, end_index_b), src);
    pc->v_storeuvec(mem_ptr(dst_ptr), src256b);
    pc->add(dst_ptr, dst_alt, stride);
    pc->v_storeu128(mem_ptr(dst_alt, end_index_a), src);
    pc->v_storeu128(mem_ptr(dst_alt, end_index_b), src);
    pc->v_storeuvec(mem_ptr(dst_alt), src256b);
    pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

    pc->j(L_End);
  }

  // Fill Rect - Width > 16 && Width <= 32 Bytes
  // -------------------------------------------

  pc->bind(L_Width_LE_32);
  pc->j(L_Width_LE_16, ucmp_le(w, 16 >> size_shift));

  {
    Label L_ScanlineLoop2x = pc->new_label();
    Gp dst_alt = pc->new_gpz("dst_alt");

    pc->sub(end_index_a, end_index_a, 16);

    pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
    pc->v_storeu128(mem_ptr(dst_ptr, end_index_a), src);
    pc->v_storeu128(mem_ptr(dst_ptr), src);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->j(L_End, sub_z(h, 1));

    pc->bind(L_ScanlineLoop2x);
    pc->add(dst_alt, dst_ptr, stride);
    pc->v_storeu128(mem_ptr(dst_ptr), src);
    pc->v_storeu128(mem_ptr(dst_ptr, end_index_a), src);
    pc->add(dst_ptr, dst_alt, stride);
    pc->v_storeu128(mem_ptr(dst_alt), src);
    pc->v_storeu128(mem_ptr(dst_alt, end_index_a), src);
    pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

    pc->j(L_End);
  }

  // Fill Rect - Width >= 8 && Width <= 16 Bytes
  // ------------------------------------------

  pc->bind(L_Width_LE_16);
  pc->j(L_Width_LT_8, ucmp_lt(w, 8 >> size_shift));

  {
    Label L_ScanlineLoop2x = pc->new_label();
    Gp dst_alt = pc->new_gpz("dst_alt");

    pc->sub(end_index_a, end_index_a, 8);

    pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
    pc->v_storeu64(mem_ptr(dst_ptr, end_index_a), src);
    pc->v_storeu64(mem_ptr(dst_ptr), src);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->j(L_End, sub_z(h, 1));

    pc->bind(L_ScanlineLoop2x);
    pc->add(dst_alt, dst_ptr, stride);
    pc->v_storeu64(mem_ptr(dst_ptr, end_index_a), src);
    pc->v_storeu64(mem_ptr(dst_ptr), src);
    pc->add(dst_ptr, dst_alt, stride);
    pc->v_storeu64(mem_ptr(dst_alt, end_index_a), src);
    pc->v_storeu64(mem_ptr(dst_alt), src);
    pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

    pc->j(L_End);
  }

  // Fill Rect - Width < 8 Bytes
  // ---------------------------

  if (item_size <= 4) {
    BL_ASSERT(L_Width_LT_8.is_valid());

    pc->bind(L_Width_LT_8);
    pc->s_mov_u32(src32b, src);

    if (item_size == 4) {
      // We know that if the unit size is 4 bytes or more it's only one item at a time.
      Label L_ScanlineLoop2x = pc->new_label();

      pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
      pc->store_u32(mem_ptr(dst_ptr), src32b);
      pc->add(dst_ptr, dst_ptr, stride);
      pc->j(L_End, sub_z(h, 1));

      pc->bind(L_ScanlineLoop2x);
      pc->store_u32(mem_ptr(dst_ptr), src32b);
      pc->store_u32(mem_ptr(dst_ptr, stride), src32b);
      pc->add_ext(dst_ptr, dst_ptr, stride, 2);
      pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

      pc->j(L_End);
    }
    else {
      // Fill Rect - Width >= 4 && Width < 8 Bytes
      Label L_ScanlineLoop2x = pc->new_label();
      L_Width_LT_4 = pc->new_label();

      Gp dst_alt = pc->new_gpz("dst_alt");

      pc->j(L_Width_LT_4, ucmp_lt(w, 4 >> size_shift));
      pc->sub(end_index_a, end_index_a, 4);

      pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
      pc->store_u32(mem_ptr(dst_ptr), src32b);
      pc->store_u32(mem_ptr(dst_ptr, end_index_a), src32b);
      pc->add(dst_ptr, dst_ptr, stride);
      pc->j(L_End, sub_z(h, 1));

      pc->bind(L_ScanlineLoop2x);
      pc->add(dst_alt, dst_ptr, stride);
      pc->store_u32(mem_ptr(dst_ptr, end_index_a), src32b);
      pc->store_u32(mem_ptr(dst_ptr), src32b);
      pc->add(dst_ptr, dst_alt, stride);
      pc->store_u32(mem_ptr(dst_alt, end_index_a), src32b);
      pc->store_u32(mem_ptr(dst_alt), src32b);
      pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

      pc->j(L_End);
    }
  }

  // Fill Rect - Width < 4 Bytes
  // ---------------------------

  if (item_size <= 2) {
    BL_ASSERT(L_Width_LT_4.is_valid());

    pc->bind(L_Width_LT_4);

    if (item_size == 2) {
      // We know that if the unit size is 2 bytes or more it's only one item at a time.
      Label L_ScanlineLoop2x = pc->new_label();

      pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
      pc->store_u16(mem_ptr(dst_ptr), src32b);
      pc->add(dst_ptr, dst_ptr, stride);
      pc->j(L_End, sub_z(h, 1));

      pc->bind(L_ScanlineLoop2x);
      pc->store_u16(mem_ptr(dst_ptr), src32b);
      pc->store_u16(mem_ptr(dst_ptr, stride), src32b);
      pc->add_ext(dst_ptr, dst_ptr, stride, 2);
      pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

      pc->j(L_End);
    }
    else {
      // Fill Rect - Width >= 2 && Width < 4 Bytes
      Label L_ScanlineLoop2x = pc->new_label();
      L_Width_LT_2 = pc->new_label();

      Gp dst_alt = pc->new_gpz("dst_alt");

      pc->j(L_Width_LT_2, ucmp_lt(w, 2));
      pc->sub(end_index_a, end_index_a, 2);

      pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
      pc->store_u16(mem_ptr(dst_ptr), src32b);
      pc->store_u16(mem_ptr(dst_ptr, end_index_a), src32b);
      pc->add(dst_ptr, dst_ptr, stride);
      pc->j(L_End, sub_z(h, 1));

      pc->bind(L_ScanlineLoop2x);
      pc->add(dst_alt, dst_ptr, stride);
      pc->store_u16(mem_ptr(dst_ptr, end_index_a), src32b);
      pc->store_u16(mem_ptr(dst_ptr), src32b);
      pc->add(dst_ptr, dst_alt, stride);
      pc->store_u16(mem_ptr(dst_alt, end_index_a), src32b);
      pc->store_u16(mem_ptr(dst_alt), src32b);
      pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

      pc->j(L_End);
    }
  }

  // Fill Rect - Width < 2 Bytes
  // ---------------------------

  if (item_size == 1) {
    BL_ASSERT(L_Width_LT_2.is_valid());

    Label L_ScanlineLoop2x = pc->new_label();

    pc->bind(L_Width_LT_2);
    pc->j(L_ScanlineLoop2x, test_z(h, 0x1));
    pc->store_u8(mem_ptr(dst_ptr), src32b);
    pc->add(dst_ptr, dst_ptr, stride);
    pc->j(L_End, sub_z(h, 1));

    pc->bind(L_ScanlineLoop2x);
    pc->store_u8(mem_ptr(dst_ptr), src32b);
    pc->store_u8(mem_ptr(dst_ptr, stride), src32b);
    pc->add_ext(dst_ptr, dst_ptr, stride, 2);
    pc->j(L_ScanlineLoop2x, sub_nz(h, 2));

    pc->j(L_End);
  }

  // Fill Rect - End
  // ---------------

  if (end.is_valid())
    pc->j(end);
  else
    pc->bind(L_End);
}

// bl::Pipeline::JIT::FetchUtils - CopySpan & CopyRect Loops
// =========================================================

static BL_NOINLINE void emit_mem_copy_sequence(
  PipeCompiler* pc,
  Mem d_ptr, bool dst_aligned,
  Mem s_ptr, bool src_aligned, uint32_t num_bytes, const Vec& fill_mask, AdvanceMode advance_mode) noexcept {

#if defined(BL_JIT_ARCH_X86)
  BackendCompiler* cc = pc->cc;

  VecArray t;

  uint32_t n = num_bytes / 16;
  uint32_t limit = 2;
  pc->new_vec128_array(t, bl_min(n, limit), "t");

  uint32_t fetch_inst = pc->has_avx() ? x86::Inst::kIdVmovaps : x86::Inst::kIdMovaps;
  uint32_t store_inst = pc->has_avx() ? x86::Inst::kIdVmovaps : x86::Inst::kIdMovaps;

  if (!src_aligned) fetch_inst = pc->has_avx() ? x86::Inst::kIdVmovups : x86::Inst::kIdMovups;
  if (!dst_aligned) store_inst = pc->has_avx() ? x86::Inst::kIdVmovups : x86::Inst::kIdMovups;

  do {
    uint32_t i;
    uint32_t count = bl_min<uint32_t>(n, limit);

    if (pc->has_avx() && fill_mask.is_valid()) {
      // Shortest code for this use case. AVX allows to read from unaligned
      // memory, so if we use VEC instructions we are generally safe here.
      for (i = 0; i < count; i++) {
        pc->v_or_i32(t[i], fill_mask, s_ptr);
        s_ptr.add_offset_lo32(16);
      }

      for (i = 0; i < count; i++) {
        cc->emit(store_inst, d_ptr, t[i]);
        d_ptr.add_offset_lo32(16);
      }
    }
    else {
      for (i = 0; i < count; i++) {
        cc->emit(fetch_inst, t[i], s_ptr);
        s_ptr.add_offset_lo32(16);
      }

      for (i = 0; i < count; i++)
        if (fill_mask.is_valid())
          pc->v_or_i32(t[i], t[i], fill_mask);

      for (i = 0; i < count; i++) {
        cc->emit(store_inst, d_ptr, t[i]);
        d_ptr.add_offset_lo32(16);
      }
    }

    n -= count;
  } while (n > 0);

  if (advance_mode == AdvanceMode::kAdvance) {
    Gp sPtrBase = s_ptr.base_reg().as<Gp>();
    Gp dPtrBase = d_ptr.base_reg().as<Gp>();

    pc->add(sPtrBase, sPtrBase, num_bytes);
    pc->add(dPtrBase, dPtrBase, num_bytes);
  }
#elif defined(BL_JIT_ARCH_A64)
  bl_unused(dst_aligned, src_aligned);

  BackendCompiler* cc = pc->cc;
  uint32_t n = num_bytes;

  VecArray t;
  pc->new_vec128_array(t, bl_min<uint32_t>((n + 15u) / 16u, 4u), "t");

  bool post_index = (advance_mode == AdvanceMode::kAdvance) && !d_ptr.has_offset() && !s_ptr.has_offset();
  if (post_index) {
    d_ptr.set_offset_mode(OffsetMode::kPostIndex);
    s_ptr.set_offset_mode(OffsetMode::kPostIndex);
  }

  while (n >= 32u) {
    uint32_t vec_count = bl_min<uint32_t>(n / 32u, 2u) * 2u;

    if (post_index) {
      // Always emit a pair of ldp/stp if we are using post-index as this seems to be
      // faster on many CPUs (the dependency of post-indexing is hidden in this case).
      vec_count = 2;
      d_ptr.set_offset_lo32(32);
      s_ptr.set_offset_lo32(32);
    }

    for (uint32_t i = 0; i < vec_count; i += 2) {
      cc->ldp(t[i + 0], t[i + 1], s_ptr);
      if (!post_index) {
        s_ptr.add_offset_lo32(32);
      }
    }

    if (fill_mask.is_valid()) {
      for (uint32_t i = 0; i < vec_count; i++) {
        pc->v_or_i32(t[i], t[i], fill_mask);
      }
    }

    for (uint32_t i = 0; i < vec_count; i += 2) {
      cc->stp(t[i + 0], t[i + 1], d_ptr);
      if (!post_index) {
        d_ptr.add_offset_lo32(32);
      }
    }

    n -= vec_count * 16u;
  }

  for (uint32_t count = 16; count != 0; count >>= 1) {
    if (n >= count) {
      Vec v = t[0];

      if (post_index) {
        d_ptr.set_offset_lo32(int32_t(count));
        s_ptr.set_offset_lo32(int32_t(count));
      }

      pc->v_load_iany(v, s_ptr, count, Alignment(1));
      if (!post_index) {
        s_ptr.add_offset_lo32(int32_t(count));
      }

      if (fill_mask.is_valid()) {
        pc->v_or_i32(t[0], t[0], fill_mask);
      }

      pc->v_store_iany(d_ptr, v, count, Alignment(1));
      if (!post_index) {
        d_ptr.add_offset_lo32(int32_t(count));
      }

      n -= count;
    }
  }

  // In case that any of the two pointers had an offset, we have to advance here...
  if (advance_mode == AdvanceMode::kAdvance && !post_index) {
    Gp sPtrBase = s_ptr.base_reg().as<Gp>();
    Gp dPtrBase = d_ptr.base_reg().as<Gp>();

    pc->add(sPtrBase, sPtrBase, num_bytes);
    pc->add(dPtrBase, dPtrBase, num_bytes);
  }
#else
  #error "Unknown architecture"
#endif
}

void inline_copy_span_loop(
  PipeCompiler* pc,
  Gp dst,
  Gp src,
  Gp i,
  uint32_t main_loop_size, uint32_t item_size, uint32_t item_granularity, FormatExt format) noexcept {

  BL_ASSERT(IntOps::is_power_of_2(item_size));
  BL_ASSERT(item_size <= 16u);

  uint32_t granularity_in_bytes = item_size * item_granularity;
  uint32_t main_step_in_items = main_loop_size / item_size;

  BL_ASSERT(IntOps::is_power_of_2(granularity_in_bytes));
  BL_ASSERT(main_step_in_items * item_size == main_loop_size);

  BL_ASSERT(main_loop_size >= 16u);
  BL_ASSERT(main_loop_size >= granularity_in_bytes);

  Vec t0 = pc->new_vec128("t0");
  Vec fill_mask;

  if (format == FormatExt::kXRGB32)
    fill_mask = pc->simd_vec_const(&common_table.p_FF000000FF000000, Bcst::k64, t0);

  // Granularity >= 16 Bytes
  // -----------------------

  if (granularity_in_bytes >= 16u) {
    Label L_End = pc->new_label();

    // MainLoop
    // --------

    {
      Label L_MainIter = pc->new_label();
      Label L_MainSkip = pc->new_label();

      pc->j(L_MainSkip, sub_c(i, main_step_in_items));

      pc->bind(L_MainIter);
      emit_mem_copy_sequence(pc, mem_ptr(dst), false, mem_ptr(src), false, main_loop_size, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_MainIter, sub_nc(i, main_step_in_items));

      pc->bind(L_MainSkip);
      pc->j(L_End, add_z(i, main_step_in_items));
    }

    // TailLoop / TailSequence
    // -----------------------

    if (main_loop_size * 2 > granularity_in_bytes) {
      Label L_TailIter = pc->new_label();

      pc->bind(L_TailIter);
      emit_mem_copy_sequence(pc, mem_ptr(dst), false, mem_ptr(src), false, granularity_in_bytes, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_TailIter, sub_nz(i, item_granularity));
    }
    else if (main_loop_size * 2 == granularity_in_bytes) {
      emit_mem_copy_sequence(pc, mem_ptr(dst), false, mem_ptr(src), false, granularity_in_bytes, fill_mask, AdvanceMode::kAdvance);
    }

    pc->bind(L_End);
    return;
  }

  // Granularity == 4 Bytes
  // ----------------------

  if (granularity_in_bytes == 4u) {
    BL_ASSERT(item_size <= 4u);
    uint32_t size_shift = IntOps::ctz(item_size);
    uint32_t align_pattern = (15u * item_size) & 15u;

    uint32_t one_step_in_items = 4u >> size_shift;
    uint32_t tail_step_in_items = 16u >> size_shift;

    Label L_Finalize = pc->new_label();
    Label L_End      = pc->new_label();

    // Preparation / Alignment
    // -----------------------

    {
      pc->j(L_Finalize, ucmp_lt(i, one_step_in_items * 4u));

      Gp i_ptr = i.clone_as(dst);
      pc->v_loadu128(t0, mem_ptr(src));
      if (size_shift)
        pc->shl(i_ptr, i_ptr, size_shift);

      pc->add(i_ptr, i_ptr, dst);
      pc->sub(src, src, dst);
      pc->v_storeu128(mem_ptr(dst), t0);
      pc->add(dst, dst, 16);
      pc->and_(dst, dst, -1 ^ int(align_pattern));
      pc->add(src, src, dst);

      if (size_shift == 0) {
        pc->j(L_End, sub_z(i_ptr, dst));
      }
      else {
        pc->sub(i_ptr, i_ptr, dst);
        pc->j(L_End, shr_z(i_ptr, size_shift));
      }
    }

    // MainLoop
    // --------

    {
      Label L_MainIter = pc->new_label();
      Label L_MainSkip = pc->new_label();

      pc->j(L_MainSkip, sub_c(i, main_step_in_items));

      pc->bind(L_MainIter);
      emit_mem_copy_sequence(pc, mem_ptr(dst), true, mem_ptr(src), false, main_loop_size, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_MainIter, sub_nc(i, main_step_in_items));

      pc->bind(L_MainSkip);
      pc->j(L_End, add_z(i, main_step_in_items));
    }

    // TailLoop / TailSequence
    // -----------------------

    if (main_loop_size > 32) {
      Label L_TailIter = pc->new_label();
      Label L_TailSkip = pc->new_label();

      pc->j(L_TailSkip, sub_c(i, tail_step_in_items));

      pc->bind(L_TailIter);
      emit_mem_copy_sequence(pc, mem_ptr(dst), true, mem_ptr(src), false, 16, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_TailIter, sub_nc(i, tail_step_in_items));

      pc->bind(L_TailSkip);
      pc->j(L_End, add_z(i, tail_step_in_items));
    }
    else if (main_loop_size >= 32) {
      pc->j(L_Finalize, ucmp_lt(i, tail_step_in_items));

      emit_mem_copy_sequence(pc, mem_ptr(dst), true, mem_ptr(src), false, 16, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_End, sub_z(i, tail_step_in_items));
    }

    // Finalize
    // --------

    {
      Label L_Store1 = pc->new_label();

      pc->bind(L_Finalize);
      pc->j(L_Store1, ucmp_lt(i, 8u / item_size));

      pc->v_loadu64(t0, mem_ptr(src));
      pc->add(src, src, 8);
      pc->v_storeu64(mem_ptr(dst), t0);
      pc->add(dst, dst, 8);
      pc->j(L_End, sub_z(i, 8u / item_size));

      pc->bind(L_Store1);
      pc->v_loada32(t0, mem_ptr(src));
      pc->add(src, src, 4);
      pc->v_storea32(mem_ptr(dst), t0);
      pc->add(dst, dst, 4);
    }

    pc->bind(L_End);
    return;
  }

  // Granularity == 1 Byte
  // ---------------------

  if (granularity_in_bytes == 1) {
    BL_ASSERT(item_size == 1u);

    Label L_Finalize = pc->new_label();
    Label L_End      = pc->new_label();

    // Preparation / Alignment
    // -----------------------

    {
      Label L_Small = pc->new_label();
      Label L_Large = pc->new_label();

      Gp i_ptr = i.clone_as(dst);
      Gp byte_val = pc->new_gp32("@byte_val");

      pc->j(L_Large, ucmp_gt(i, 15));

      pc->bind(L_Small);
      pc->load_u8(byte_val, ptr(src));
      pc->inc(src);
      pc->store_u8(ptr(dst), byte_val);
      pc->inc(dst);
      pc->j(L_Small, sub_nz(i, 1));
      pc->j(L_End);

      pc->bind(L_Large);
      pc->v_loadu128(t0, mem_ptr(src));
      pc->add(i_ptr, i_ptr, dst);
      pc->sub(src, src, dst);

      pc->v_storeu128(mem_ptr(dst), t0);
      pc->add(dst, dst, 16);
      pc->and_(dst, dst, -16);

      pc->add(src, src, dst);
      pc->j(L_End, sub_z(i_ptr, dst));
    }

    // MainLoop
    // --------

    {
      Label L_MainIter = pc->new_label();
      Label L_MainSkip = pc->new_label();

      pc->j(L_MainSkip, sub_c(i, main_loop_size));

      pc->bind(L_MainIter);
      emit_mem_copy_sequence(pc, mem_ptr(dst), true, mem_ptr(src), false, main_loop_size, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_MainIter, sub_nc(i, main_loop_size));

      pc->bind(L_MainSkip);
      pc->j(L_End, add_z(i, main_loop_size));
    }

    // TailLoop / TailSequence
    // -----------------------

    if (main_loop_size > 32) {
      Label L_TailIter = pc->new_label();
      Label L_TailSkip = pc->new_label();

      pc->j(L_TailSkip, sub_c(i, 16));

      pc->bind(L_TailIter);
      emit_mem_copy_sequence(pc, mem_ptr(dst), true, mem_ptr(src), false, 16, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_TailIter, sub_nc(i, 16));

      pc->bind(L_TailSkip);
      pc->j(L_End, add_z(i, 16));
    }
    else if (main_loop_size >= 32) {
      pc->j(L_Finalize, ucmp_lt(i, 16));

      emit_mem_copy_sequence(pc, mem_ptr(dst), true, mem_ptr(src), false, 16, fill_mask, AdvanceMode::kAdvance);
      pc->j(L_End, sub_z(i, 16));
    }

    // Finalize
    // --------

    {
      pc->add(src, src, i.clone_as(src));
      pc->add(dst, dst, i.clone_as(dst));
      emit_mem_copy_sequence(pc, mem_ptr(dst, -16), false, mem_ptr(src, -16), false, 16, fill_mask, AdvanceMode::kNoAdvance);
    }

    pc->bind(L_End);
    return;
  }
}

} // {bl::Pipeline::JIT::FetchUtils}

#endif // !BL_BUILD_NO_JIT
