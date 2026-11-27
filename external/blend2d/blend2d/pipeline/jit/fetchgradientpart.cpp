// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/compoppart_p.h>
#include <blend2d/pipeline/jit/fetchgradientpart_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelgather_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>

namespace bl::Pipeline::JIT {

#define REL_GRADIENT(FIELD) BL_OFFSET_OF(FetchData::Gradient, FIELD)

// bl::Pipeline::JIT::GradientDitheringContext
// ===========================================

static void rotate_dither_bytes_right(PipeCompiler* pc, const Vec& vec, const Gp& count) noexcept {
  Gp count_as_index = pc->gpz(count);

#if defined(BL_JIT_ARCH_X86)
  if (!pc->has_ssse3()) {
    Mem lo = pc->tmp_stack(PipeCompiler::StackId::kCustom, 32);
    Mem hi = lo.clone_adjusted(16);

    pc->v_storea128(lo, vec);
    pc->v_storea128(hi, vec);

    Mem rotated = lo;
    rotated.set_index(count_as_index);
    pc->v_loadu128(vec, rotated);

    return;
  }
#endif

  Mem m_pred = pc->simd_mem_const(pc->ct<CommonTable>().swizu8_rotate_right, Bcst::kNA, vec);

#if defined(BL_JIT_ARCH_X86)
  m_pred.set_index(count_as_index);
  if (!pc->has_avx()) {
    Vec v_pred = pc->new_similar_reg(vec, "@v_pred");
    pc->v_loadu128(v_pred, m_pred);
    pc->v_swizzlev_u8(vec, vec, v_pred);
    return;
  }
#else
  Gp base = pc->new_gpz("@swizu8_rotate_base");
  pc->cc->load_address_of(base, m_pred);
  m_pred = mem_ptr(base, count_as_index);
#endif

  pc->v_swizzlev_u8(vec, vec, m_pred);
}

void GradientDitheringContext::init_y(const PipeFunction& fn, const Gp& x, const Gp& y) noexcept {
  _dm_position = pc->new_gp32("dm.position");
  _dm_origin_x = pc->new_gp32("dm.origin_x");
  _dm_values = pc->new_vec_with_width(pc->vec_width(), "dm.values");
  _is_rect_fill = x.is_valid();

  pc->load_u32(_dm_position, mem_ptr(fn.ctx_data(), BL_OFFSET_OF(ContextData, pixel_origin.y)));
  pc->load_u32(_dm_origin_x, mem_ptr(fn.ctx_data(), BL_OFFSET_OF(ContextData, pixel_origin.x)));

  pc->add(_dm_position, _dm_position, y.r32());
  if (is_rect_fill())
    pc->add(_dm_origin_x, _dm_origin_x, x.r32());

  pc->and_(_dm_position, _dm_position, 15);
  if (is_rect_fill())
    pc->and_(_dm_origin_x, _dm_origin_x, 15);

  pc->shl(_dm_position, _dm_position, 5);
  if (is_rect_fill())
    pc->add(_dm_position, _dm_position, _dm_origin_x);
}

void GradientDitheringContext::advance_y() noexcept {
  pc->add(_dm_position, _dm_position, 16 * 2);
  pc->and_(_dm_position, _dm_position, 16 * 16 * 2 - 1);
}

void GradientDitheringContext::start_at_x(const Gp& x) noexcept {
  Gp dm_position = _dm_position;

  if (!is_rect_fill()) {
    // If not rectangular, we have to calculate the final position according to `x`.
    dm_position = pc->new_gp32("dm.final_position");

    pc->mov(dm_position, _dm_origin_x);
    pc->add(dm_position, dm_position, x.r32());
    pc->and_(dm_position, dm_position, 15);
    pc->add(dm_position, dm_position, _dm_position);
  }

  const int bayer_matrix_16x16_offset = int(uintptr_t(pc->ct<CommonTable>().bayer_matrix_16x16) - uintptr_t(pc->ct_ptr()));

  Mem m;
#if defined(BL_JIT_ARCH_X86)
  if (pc->is_32bit()) {
    m = x86::ptr(uint64_t(uintptr_t(pc->ct_ptr()) + uintptr_t(bayer_matrix_16x16_offset)), dm_position);
  }
  else {
    pc->_init_vec_const_table_ptr();
    m = mem_ptr(pc->_common_table_ptr, dm_position.r64(), 0, bayer_matrix_16x16_offset - pc->_common_table_offset);
  }
#else
  pc->_init_vec_const_table_ptr();
  Gp dither_row = pc->new_gpz("@dither_row");
  pc->add(dither_row, pc->_common_table_ptr, bayer_matrix_16x16_offset - pc->_common_table_offset);
  m = mem_ptr(dither_row, dm_position.r64());
#endif

  if (_dm_values.is_vec128()) {
    pc->v_loadu128(_dm_values, m);
  }
  else {
    pc->v_broadcast_v128_u32(_dm_values, m);
  }
}

void GradientDitheringContext::advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept {
  bl_unused(x);

  if (diff_within_bounds) {
    rotate_dither_bytes_right(pc, _dm_values, diff);
  }
  else {
    Gp diff_0_to_15 = pc->new_similar_reg(diff, "@diff_0_to_15");
    pc->and_(diff_0_to_15, diff, 0xF);
    rotate_dither_bytes_right(pc, _dm_values, diff_0_to_15);
  }
}

void GradientDitheringContext::advance_x_after_fetch(uint32_t n) noexcept {
  // The compiler would optimize this to a cheap shuffle whenever possible.
  pc->v_alignr_u128(_dm_values, _dm_values, _dm_values, n & 15);
}

void GradientDitheringContext::dither_unpacked_pixels(Pixel& p, AdvanceMode advance_mode) noexcept {
  VecWidth vec_width = VecWidthUtils::vec_width_of(p.uc[0]);

  Operand shuffle_predicate = pc->simd_const(&common_table.swizu8_dither_rgba64_lo, Bcst::kNA_Unique, vec_width);
  Vec dither_predicate = pc->new_similar_reg(p.uc[0], "dither_predicate");
  Vec dither_threshold = pc->new_similar_reg(p.uc[0], "dither_threshold");

  Vec dm_values = _dm_values;

  switch (uint32_t(p.count())) {
    case 1: {
#if defined(BL_JIT_ARCH_X86)
      if (!pc->has_ssse3()) {
        pc->v_interleave_lo_u8(dither_predicate, dm_values, pc->simd_const(&common_table.p_0000000000000000, Bcst::kNA, dither_predicate));
        pc->v_swizzle_lo_u16x4(dither_predicate, dither_predicate, swizzle(0, 0, 0, 0));
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        pc->v_swizzlev_u8(dither_predicate, dm_values.clone_as(dither_predicate), shuffle_predicate);
      }

      pc->v_swizzle_lo_u16x4(dither_threshold, p.uc[0], swizzle(3, 3, 3, 3));
      pc->v_adds_u16(p.uc[0], p.uc[0], dither_predicate);
      pc->v_min_u16(p.uc[0], p.uc[0], dither_threshold);
      pc->v_srli_u16(p.uc[0], p.uc[0], 8);

      if (advance_mode == AdvanceMode::kAdvance) {
        advance_x_after_fetch(1);
      }
      break;
    }

    case 4:
    case 8:
    case 16: {
#if defined(BL_JIT_ARCH_X86)
      if (!p.uc[0].is_vec128()) {
        for (uint32_t i = 0; i < p.uc.size(); i++) {
          // At least AVX2: VPSHUFB is available...
          pc->v_swizzlev_u8(dither_predicate, dm_values.clone_as(dither_predicate), shuffle_predicate);
          pc->v_expand_alpha_16(dither_threshold, p.uc[i]);
          pc->v_adds_u16(p.uc[i], p.uc[i], dither_predicate);
          pc->v_min_u16(p.uc[i], p.uc[i], dither_threshold);

          Swizzle4 swiz = p.uc[0].is_vec256() ? swizzle(0, 3, 2, 1) : swizzle(1, 0, 3, 2);

          if (advance_mode == AdvanceMode::kNoAdvance) {
            if (i + 1 == p.uc.size()) {
              break;
            }

            if (dm_values.id() == _dm_values.id()) {
              dm_values = pc->new_similar_reg(dither_predicate, "dm.local");
              pc->v_swizzle_u32x4(dm_values, _dm_values.clone_as(dm_values), swiz);
              continue;
            }
          }

          pc->v_swizzle_u32x4(dm_values, dm_values, swiz);
        }
        pc->v_srli_u16(p.uc, p.uc, 8);
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        for (uint32_t i = 0; i < p.uc.size(); i++) {
          Vec dm = (i == 0) ? dm_values.clone_as(dither_predicate) : dither_predicate;

#if defined(BL_JIT_ARCH_X86)
          if (!pc->has_ssse3()) {
            pc->v_interleave_lo_u8(dither_predicate, dm, pc->simd_const(&common_table.p_0000000000000000, Bcst::kNA, dither_predicate));
            pc->v_interleave_lo_u16(dither_predicate, dither_predicate, dither_predicate);
            pc->v_swizzle_u32x4(dither_predicate, dither_predicate, swizzle(1, 1, 0, 0));
          }
          else
#endif // BL_JIT_ARCH_X86
          {
            pc->v_swizzlev_u8(dither_predicate, dm, shuffle_predicate);
          }

          pc->v_expand_alpha_16(dither_threshold, p.uc[i]);
          pc->v_adds_u16(p.uc[i], p.uc[i], dither_predicate);

          if (i + 1u < p.uc.size())
            pc->v_swizzle_lo_u16x4(dither_predicate, dm_values.clone_as(dither_predicate), swizzle(0, 3, 2, 1));

          pc->v_min_u16(p.uc[i], p.uc[i], dither_threshold);
        }

        if (advance_mode == AdvanceMode::kAdvance) {
          Swizzle4 swiz = p.count() == PixelCount(4) ? swizzle(0, 3, 2, 1) : swizzle(1, 0, 3, 2);
          pc->v_swizzle_u32x4(dm_values, dm_values, swiz);
        }

        pc->v_srli_u16(p.uc, p.uc, 8);
      }
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::JIT::FetchGradientPart - Construction & Destruction
// =================================================================

FetchGradientPart::FetchGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchPart(pc, fetch_type, format),
    _dithering_context(pc) {}

void FetchGradientPart::fetch_single_pixel(Pixel& dst, PixelFlags flags, const Gp& idx) noexcept {
  Mem src = mem_ptr(_table_ptr, idx, uint32_t(table_ptr_shift()));
  if (dithering_enabled()) {
    pc->new_vec_array(dst.uc, 1, VecWidth::k128, dst.name(), "uc");
    pc->v_loadu64(dst.uc[0], src);
    _dithering_context.dither_unpacked_pixels(dst, AdvanceMode::kAdvance);
  }
  else {
    FetchUtils::fetch_pixel(pc, dst, flags, PixelFetchInfo(FormatExt::kPRGB32), src);
  }
}

void FetchGradientPart::fetch_multiple_pixels(Pixel& dst, PixelCount n, PixelFlags flags, const Vec& idx, FetchUtils::IndexLayout index_layout, GatherMode mode, InterleaveCallback cb, void* cb_data) noexcept {
  Mem src = mem_ptr(_table_ptr);
  uint32_t idx_shift = uint32_t(table_ptr_shift());

  if (dithering_enabled()) {
    dst.set_type(PixelType::kRGBA64);
    FetchUtils::gather_pixels(pc, dst, n, PixelFlags::kUC, PixelFetchInfo(FormatExt::kPRGB64), src, idx, idx_shift, index_layout, mode, cb, cb_data);
    _dithering_context.dither_unpacked_pixels(dst, mode == GatherMode::kFetchAll ? AdvanceMode::kAdvance : AdvanceMode::kNoAdvance);

    dst.set_type(PixelType::kRGBA32);
    FetchUtils::satisfy_pixels(pc, dst, flags);
  }
  else {
    FetchUtils::gather_pixels(pc, dst, n, flags, fetch_info(), src, idx, idx_shift, index_layout, mode, cb, cb_data);
  }
}

// bl::Pipeline::JIT::FetchLinearGradientPart - Construction & Destruction
// =======================================================================

FetchLinearGradientPart::FetchLinearGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchGradientPart(pc, fetch_type, format) {

  bool dither = false;
  switch (fetch_type) {
    case FetchType::kGradientLinearNNPad:
      _extend_mode = ExtendMode::kPad;
      break;

    case FetchType::kGradientLinearNNRoR:
      _extend_mode = ExtendMode::kRoR;
      break;

    case FetchType::kGradientLinearDitherPad:
      _extend_mode = ExtendMode::kPad;
      dither = true;
      break;

    case FetchType::kGradientLinearDitherRoR:
      _extend_mode = ExtendMode::kRoR;
      dither = true;
      break;

    default:
      BL_NOT_REACHED();
  }

  _max_vec_width_supported = kMaxPlatformWidth;

  add_part_flags(PipePartFlags::kExpensive |
               PipePartFlags::kMaskedAccess |
               PipePartFlags::kAdvanceXNeedsDiff);
  set_dithering_enabled(dither);
  OpUtils::reset_var_struct(&f, sizeof(f));
}

// bl::Pipeline::JIT::FetchLinearGradientPart - Prepare
// ====================================================

void FetchLinearGradientPart::prepare_part() noexcept {
#if defined(BL_JIT_ARCH_X86)
  _max_pixels = uint8_t(pc->has_ssse3() ? 8 : 4);
#else
  _max_pixels = 8;
#endif
}

// bl::Pipeline::JIT::FetchLinearGradientPart - Init & Fini
// ========================================================

void FetchLinearGradientPart::_init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept {
  VecWidth vw = vec_width();

  // Local Registers
  // ---------------

  _table_ptr = pc->new_gpz("f.table");                 // Reg.
  f->pt = pc->new_vec_with_width(vw, "f.pt");          // Reg.
  f->dt = pc->new_vec_with_width(vw, "f.dt");          // Reg/Mem.
  f->dt_n = pc->new_vec_with_width(vw, "f.dt_n");      // Reg/Mem.
  f->py = pc->new_vec_with_width(vw, "f.py");          // Reg/Mem.
  f->dy = pc->new_vec_with_width(vw, "f.dy");          // Reg/Mem.
  f->maxi = pc->new_vec_with_width(vw, "f.maxi");      // Reg/Mem.
  f->rori = pc->new_vec_with_width(vw, "f.rori");      // Reg/Mem [RoR only].
  f->v_idx = pc->new_vec_with_width(vw, "f.v_idx");    // Reg/Tmp.

  // In 64-bit mode it's easier to use IMUL for 64-bit multiplication instead of SIMD, because
  // we need to multiply a scalar anyway that we then broadcast and add to our 'f.pt' vector.
  if (pc->is_64bit()) {
    f->dt_gp = pc->new_gp64("f.dt_gp");                // Reg/Mem.
  }

  // Part Initialization
  // -------------------

  pc->load(_table_ptr, mem_ptr(fn.fetch_data(), REL_GRADIENT(lut.data)));

  if (dithering_enabled())
    _dithering_context.init_y(fn, x, y);

  pc->s_mov_u32(f->py, y);
  pc->v_broadcast_u64(f->dy, mem_ptr(fn.fetch_data(), REL_GRADIENT(linear.dy.u64)));
  pc->v_broadcast_u64(f->py, f->py);
  pc->v_mul_u64_lo_u32(f->py, f->dy, f->py);
  pc->v_broadcast_u64(f->dt, mem_ptr(fn.fetch_data(), REL_GRADIENT(linear.dt.u64)));

  if (is_pad()) {
    pc->v_broadcast_u16(f->maxi, mem_ptr(fn.fetch_data(), REL_GRADIENT(linear.maxi)));
  }
  else {
    pc->v_broadcast_u32(f->maxi, mem_ptr(fn.fetch_data(), REL_GRADIENT(linear.maxi)));
    pc->v_broadcast_u16(f->rori, mem_ptr(fn.fetch_data(), REL_GRADIENT(linear.rori)));
  }

  pc->v_loadu128(f->pt, mem_ptr(fn.fetch_data(), REL_GRADIENT(linear.pt)));
  pc->v_slli_i64(f->dt_n, f->dt, 1u);

#if defined(BL_JIT_ARCH_X86)
  if (pc->use_256bit_simd()) {
    cc->vperm2i128(f->dt_n, f->dt_n, f->dt_n, perm_2x128_imm(Perm2x128::kALo, Perm2x128::kZero));
    cc->vperm2i128(f->pt, f->pt, f->pt, perm_2x128_imm(Perm2x128::kALo, Perm2x128::kALo));
    pc->v_add_i64(f->pt, f->pt, f->dt_n);
    pc->v_slli_i64(f->dt_n, f->dt, 2u);
  }
#endif // BL_JIT_ARCH_X86

  pc->v_add_i64(f->py, f->py, f->pt);

#if defined(BL_JIT_ARCH_X86)
  // If we cannot use PACKUSDW, which was introduced by SSE4.1 we subtract 32768 from the pointer
  // and use PACKSSDW instead. However, if we do this, we have to adjust everything else accordingly.
  if (is_pad() && !pc->has_sse4_1()) {
    pc->v_sub_i32(f->py, f->py, pc->simd_const(&ct.p_0000800000000000, Bcst::k32, f->py));
    pc->v_sub_i16(f->maxi, f->maxi, pc->simd_const(&ct.p_8000800080008000, Bcst::kNA, f->maxi));
  }
#endif // BL_JIT_ARCH_X86

  if (pc->is_64bit())
    pc->s_mov_u64(f->dt_gp, f->dt);

  if (is_rect_fill()) {
    Vec adv = pc->new_similar_reg(f->dt, "f.adv");
    calc_advance_x(adv, x);
    pc->v_add_i64(f->py, f->py, adv);
  }

  if (pixel_granularity() > 1)
    enter_n();
}

void FetchLinearGradientPart::_fini_part() noexcept {}

// bl::Pipeline::JIT::FetchLinearGradientPart - Advance
// ====================================================

void FetchLinearGradientPart::advance_y() noexcept {
  pc->v_add_i64(f->py, f->py, f->dy);

  if (dithering_enabled())
    _dithering_context.advance_y();
}

void FetchLinearGradientPart::start_at_x(const Gp& x) noexcept {
  if (!is_rect_fill()) {
    calc_advance_x(f->pt, x);
    pc->v_add_i64(f->pt, f->pt, f->py);
  }
  else {
    pc->v_mov(f->pt, f->py);
  }

  if (dithering_enabled())
    _dithering_context.start_at_x(x);
}

void FetchLinearGradientPart::advance_x(const Gp& x, const Gp& diff) noexcept {
  advance_x(x, diff, false);
}

void FetchLinearGradientPart::advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept {
  Vec adv = pc->new_similar_reg(f->pt, "f.adv");
  calc_advance_x(adv, diff);
  pc->v_add_i64(f->pt, f->pt, adv);

  if (dithering_enabled())
    _dithering_context.advance_x(x, diff, diff_within_bounds);
}

void FetchLinearGradientPart::calc_advance_x(const Vec& dst, const Gp& diff) const noexcept {
  // Use 64-bit multiply on 64-bit targets as it's much shorter than doing a vectorized 64x32 multiply.
  if (pc->is_64bit()) {
    Gp adv_tmp = pc->new_gp64("f.adv_tmp");
    pc->mul(adv_tmp, diff.r64(), f->dt_gp);
    pc->v_broadcast_u64(dst, adv_tmp);
  }
  else {
    pc->v_broadcast_u32(dst, diff);
    pc->v_mul_u64_lo_u32(dst, f->dt, dst);
  }
}

// bl::Pipeline::JIT::FetchLinearGradientPart - Fetch
// ==================================================

void FetchLinearGradientPart::enter_n() noexcept {}
void FetchLinearGradientPart::leave_n() noexcept {}

void FetchLinearGradientPart::prefetch_n() noexcept {}
void FetchLinearGradientPart::postfetch_n() noexcept {}

void FetchLinearGradientPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  p.set_count(n);

  GatherMode gather_mode = predicate.gather_mode();

  switch (uint32_t(n)) {
    case 1: {
      BL_ASSERT(predicate.is_empty());

      Gp r_idx = pc->new_gp32("f.r_idx");
      Vec v_idx = pc->new_vec128("f.v_idx");
      uint32_t vIdxLane = 1u + uint32_t(!is_pad());

      if (is_pad()) {
#if defined(BL_JIT_ARCH_X86)
        if (!pc->has_sse4_1()) {
          pc->v_packs_i32_i16(v_idx, f->pt.v128(), f->pt.v128());
          pc->v_min_i16(v_idx, v_idx, f->maxi.v128());
          pc->v_add_i16(v_idx, v_idx, pc->simd_const(&ct.p_8000800080008000, Bcst::kNA, v_idx));
        }
        else
#endif // BL_JIT_ARCH_X86
        {
          pc->v_packs_i32_u16(v_idx, f->pt.v128(), f->pt.v128());
          pc->v_min_u16(v_idx, v_idx, f->maxi.v128());
        }
      }
      else {
        Vec v_tmp = pc->new_vec128("f.v_tmp");
        pc->v_and_i32(v_idx, f->pt.v128(), f->maxi.v128());
        pc->v_xor_i32(v_tmp, v_idx, f->rori.v128());
        pc->v_min_i16(v_idx, v_idx, v_tmp);
      }

      pc->v_add_i64(f->pt, f->pt, f->dt);
      pc->s_extract_u16(r_idx, v_idx, vIdxLane);
      fetch_single_pixel(p, flags, r_idx);
      FetchUtils::satisfy_pixels(pc, p, flags);
      break;
    }

    case 4: {
      Vec v_idx = f->v_idx;
      Vec v_tmp = pc->new_similar_reg(v_idx, "f.v_tmp");
      Vec v_pt = f->pt;

      if (!predicate.is_empty()) {
        v_pt = pc->new_similar_reg(v_pt, "@pt");
      }

#if defined(BL_JIT_ARCH_X86)
      if (pc->use_256bit_simd()) {
        if (is_pad()) {
          pc->v_packs_i32_u16(v_idx, f->pt, f->pt);
          pc->v_add_i64(v_pt, f->pt, f->dt_n);
          pc->v_min_u16(v_idx, v_idx, f->maxi);
        }
        else {
          pc->v_and_i32(v_idx, f->pt, f->maxi);
          pc->v_add_i64(v_pt, f->pt, f->dt_n);
          pc->v_and_i32(v_tmp, v_pt, f->maxi);
          pc->v_packs_i32_u16(v_idx, v_idx, v_tmp);
          pc->v_xor_i32(v_tmp, v_idx, f->rori);
          pc->v_min_u16(v_idx, v_idx, v_tmp);
        }
        pc->v_swizzle_u64x4(v_idx, v_idx, swizzle(3, 1, 2, 0));

        fetch_multiple_pixels(p, n, flags, v_idx.v128(), FetchUtils::IndexLayout::kUInt32Hi16, gather_mode);
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        FetchUtils::IndexLayout index_layout = FetchUtils::IndexLayout::kUInt16;

        if (pc->has_non_destructive_src()) {
          pc->v_add_i64(v_tmp, f->pt, f->dt_n);
          pc->v_interleave_shuffle_u32x4(v_idx, f->pt, v_tmp, swizzle(3, 1, 3, 1));
          pc->v_add_i64(v_pt, v_tmp, f->dt_n);
        }
        else {
          pc->v_mov(v_idx, f->pt);
          pc->v_add_i64(v_pt, f->pt, f->dt_n);
          pc->v_interleave_shuffle_u32x4(v_idx, v_idx, v_pt, swizzle(3, 1, 3, 1));
          pc->v_add_i64(v_pt, v_pt, f->dt_n);
        }

        if (is_pad()) {
#if defined(BL_JIT_ARCH_X86)
          if (!pc->has_sse4_1()) {
            pc->v_packs_i32_i16(v_idx, v_idx, v_idx);
            pc->v_min_i16(v_idx, v_idx, f->maxi);
            pc->v_add_i16(v_idx, v_idx, pc->simd_const(&ct.p_8000800080008000, Bcst::kNA, v_idx));
          }
          else
#endif // BL_JIT_ARCH_X86
          {
            pc->v_packs_i32_u16(v_idx, v_idx, v_idx);
            pc->v_min_u16(v_idx, v_idx, f->maxi);
          }
        }
        else {
          index_layout = FetchUtils::IndexLayout::kUInt32Lo16;
          pc->v_and_i32(v_idx, v_idx, f->maxi);
          pc->v_xor_i32(v_tmp, v_idx, f->rori);
          pc->v_min_i16(v_idx, v_idx, v_tmp);
        }

        fetch_multiple_pixels(p, n, flags, v_idx.v128(), index_layout, gather_mode);
      }

      FetchUtils::satisfy_pixels(pc, p, flags);
      break;
    }

    case 8: {
      Vec v_idx = f->v_idx;
      Vec v_tmp = pc->new_similar_reg(v_idx, "f.v_tmp");
      Vec v_pt = f->pt;

      if (!predicate.is_empty()) {
        v_pt = pc->new_similar_reg(v_pt, "@pt");
      }

#if defined(BL_JIT_ARCH_X86)
      if (pc->vec_width() >= VecWidth::k256) {
        if (is_pad()) {
          pc->v_add_i64(v_tmp, f->pt, f->dt_n);
          pc->v_packs_i32_u16(v_idx, f->pt, v_tmp);

          if (predicate.is_empty()) {
            pc->v_add_i64(v_pt, v_tmp, f->dt_n);
          }

          pc->v_min_u16(v_idx, v_idx, f->maxi);
          pc->v_swizzle_u64x4(v_idx, v_idx, swizzle(3, 1, 2, 0));
        }
        else {
          pc->v_and_i32(v_idx, f->pt, f->maxi);
          pc->v_add_i64(v_pt, f->pt, f->dt_n);
          pc->v_and_i32(v_tmp, v_pt, f->maxi);
          pc->v_packs_i32_u16(v_idx, v_idx, v_tmp);

          if (predicate.is_empty()) {
            pc->v_add_i64(v_pt, v_pt, f->dt_n);
          }

          pc->v_xor_i32(v_tmp, v_idx, f->rori);
          pc->v_min_u16(v_idx, v_idx, v_tmp);
          pc->v_swizzle_u64x4(v_idx, v_idx, swizzle(3, 1, 2, 0));
        }

        fetch_multiple_pixels(p, n, flags, v_idx, FetchUtils::IndexLayout::kUInt32Hi16, gather_mode);
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        pc->v_add_i64(v_tmp, f->pt, f->dt_n);
        pc->v_interleave_shuffle_u32x4(v_idx, f->pt, v_tmp, swizzle(3, 1, 3, 1));
        pc->v_add_i64(v_tmp, v_tmp, f->dt_n);
        pc->v_add_i64(v_pt, v_tmp, f->dt_n);
        pc->v_interleave_shuffle_u32x4(v_tmp, v_tmp, v_pt, swizzle(3, 1, 3, 1));

        if (predicate.is_empty()) {
          pc->v_add_i64(v_pt, v_pt, f->dt_n);
        }

        if (is_pad()) {
#if defined(BL_JIT_ARCH_X86)
          if (!pc->has_sse4_1()) {
            pc->v_packs_i32_i16(v_idx, v_idx, v_tmp);
            pc->v_min_i16(v_idx, v_idx, f->maxi);
            pc->v_add_i16(v_idx, v_idx, pc->simd_const(&ct.p_8000800080008000, Bcst::kNA, v_idx));
          }
          else
#endif // BL_JIT_ARCH_X86
          {
            pc->v_packs_i32_u16(v_idx, v_idx, v_tmp);
            pc->v_min_u16(v_idx, v_idx, f->maxi);
          }
        }
        else {
          pc->v_and_i32(v_idx, v_idx, f->maxi);
          pc->v_and_i32(v_tmp, v_tmp, f->maxi);
          pc->v_packs_i32_i16(v_idx, v_idx, v_tmp);
          pc->v_xor_i32(v_tmp, v_idx, f->rori);
          pc->v_min_i16(v_idx, v_idx, v_tmp);
        }

        fetch_multiple_pixels(p, n, flags, v_idx, FetchUtils::IndexLayout::kUInt16, gather_mode);
      }

      FetchUtils::satisfy_pixels(pc, p, flags);
      break;
    }

    default:
      BL_NOT_REACHED();
  }

  if (!predicate.is_empty()) {
    advance_x(pc->_gp_none, predicate.count().r32());
  }
}

// bl::Pipeline::JIT::FetchRadialGradientPart - Construction & Destruction
// =======================================================================

FetchRadialGradientPart::FetchRadialGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchGradientPart(pc, fetch_type, format) {

  _max_vec_width_supported = kMaxPlatformWidth;

  bool dither = false;
  switch (fetch_type) {
    case FetchType::kGradientRadialNNPad:
      _extend_mode = ExtendMode::kPad;
      break;

    case FetchType::kGradientRadialNNRoR:
      _extend_mode = ExtendMode::kRoR;
      break;

    case FetchType::kGradientRadialDitherPad:
      _extend_mode = ExtendMode::kPad;
      dither = true;
      break;

    case FetchType::kGradientRadialDitherRoR:
      _extend_mode = ExtendMode::kRoR;
      dither = true;
      break;

    default:
      BL_NOT_REACHED();
  }

  add_part_flags(PipePartFlags::kAdvanceXNeedsDiff |
                 PipePartFlags::kMaskedAccess |
                 PipePartFlags::kExpensive);
  set_dithering_enabled(dither);
  OpUtils::reset_var_struct(&f, sizeof(f));
}

// bl::Pipeline::JIT::FetchRadialGradientPart - Prepare
// ====================================================

void FetchRadialGradientPart::prepare_part() noexcept {
  VecWidth vw = vec_width();
  _max_pixels = uint8_t(4u << uint32_t(vw));
}

// bl::Pipeline::JIT::FetchRadialGradientPart - Init & Fini
// ========================================================

void FetchRadialGradientPart::_init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept {
  // Local Registers
  // ---------------

  VecWidth vw = vec_width();

  _table_ptr = pc->new_gpz("f.table");                     // Reg.

  f->ty_tx = pc->new_vec128_f64x2("f.ty_tx");              // Mem.
  f->yy_yx = pc->new_vec128_f64x2("f.yy_yx");              // Mem.
  f->dd0_b0 = pc->new_vec128_f64x2("f.dd0_b0");            // Mem.
  f->ddy_by = pc->new_vec128_f64x2("f.ddy_by");            // Mem.

  f->vy = pc->new_vec128_f64x2("f.vy");                    // Reg/Mem.

  f->inv2a_4a = pc->new_vec128_f64x2("f.inv2a_4a");        // Reg/Mem.
  f->sqinv2a_sqfr = pc->new_vec128_f64x2("f.sqinv2a_sqfr");// Reg/Mem.

  f->d = pc->new_vec_with_width(vw, "f.d");                // Reg.
  f->b = pc->new_vec_with_width(vw, "f.b");                // Reg.
  f->dd = pc->new_vec_with_width(vw, "f.dd");              // Reg/Mem.
  f->vx = pc->new_vec_with_width(vw, "f.vx");              // Reg.
  f->value = pc->new_vec_with_width(vw, "f.value");        // Reg.

  f->bd = pc->new_vec_with_width(vw, "f.bd");              // Reg/Mem.
  f->ddd = pc->new_vec_with_width(vw, "f.ddd");            // Reg/Mem.

  f->vmaxi = pc->new_vec_with_width(vw, "f.vmaxi");        // Reg/Mem.

  // Part Initialization
  // -------------------

  if (dithering_enabled())
    _dithering_context.init_y(fn, x, y);

  pc->load(_table_ptr, mem_ptr(fn.fetch_data(), REL_GRADIENT(lut.data)));

  pc->v_loadu128_f64(f->ty_tx, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.tx)));
  pc->v_loadu128_f64(f->yy_yx, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.yx)));

  pc->v_loadu128_f64(f->inv2a_4a, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.amul4)));
  pc->v_loadu128_f64(f->sqinv2a_sqfr, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.sq_fr)));

  pc->v_loadu128_f64(f->dd0_b0, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.b0)));
  pc->v_loadu128_f64(f->ddy_by, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.by)));
  pc->v_broadcast_f32(f->bd, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.f32_bd)));
  pc->v_broadcast_f32(f->ddd, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.f32_ddd)));

  pc->s_cvt_int_to_f64(f->vy, y);
  pc->v_broadcast_f64(f->vy, f->vy);

  if (is_pad()) {
#if defined(BL_JIT_ARCH_X86)
    if (vw > VecWidth::k128) {
      pc->v_broadcast_u32(f->vmaxi, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.maxi)));
    }
    else
#endif // BL_JIT_ARCH_X86
    {
      pc->v_broadcast_u16(f->vmaxi, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.maxi)));
    }
  }
  else {
    f->vrori = pc->new_vec_with_width(vw, "f.vrori");
    pc->v_broadcast_u32(f->vmaxi, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.maxi)));
    pc->v_broadcast_u16(f->vrori, mem_ptr(fn.fetch_data(), REL_GRADIENT(radial.rori)));
  }

  if (is_rect_fill()) {
    f->vx_start = pc->new_similar_reg(f->vx, "f.vx_start");
    init_vx(f->vx_start, x);
  }
}

void FetchRadialGradientPart::_fini_part() noexcept {}

// bl::Pipeline::JIT::FetchRadialGradientPart - Advance
// ====================================================

void FetchRadialGradientPart::advance_y() noexcept {
  pc->v_add_f64(f->vy, f->vy, pc->simd_const(&ct.f64_1, Bcst::k64, f->vy));

  if (dithering_enabled())
    _dithering_context.advance_y();
}

void FetchRadialGradientPart::start_at_x(const Gp& x) noexcept {
  Vec v0 = pc->new_vec128_f64x2("@v0");
  Vec v1 = pc->new_vec128_f64x2("@v1");
  Vec v2 = pc->new_vec128_f64x2("@v2");
  Vec v3 = pc->new_vec128_f64x2("@v3");

  pc->v_madd_f64(v1, f->vy, f->yy_yx, f->ty_tx);          // v1    = [ ty  + Y * yy      | tx + Y * yx          ] => [  py  |  px  ]
  pc->v_madd_f64(v0, f->vy, f->ddy_by, f->dd0_b0);        // v0    = [ dd0 + Y * ddy     | b0 + Y * by          ] => [  dd  |   b  ]
  pc->v_mul_f64(v1, v1, v1);                              // v1    = [ (ty + Y * yy)^2   | (tx + Y * xx) ^ 2    ] => [ py^2 | px^2 ]
  pc->s_mul_f64(v2, v0, v0);                              // v2    = [ ?                 | b^2                  ]

  pc->v_dup_hi_f64(v3, f->inv2a_4a);                      // v3    = [ 1 / 2a            | 1 / 2a               ]
  pc->v_hadd_f64(v1, v1, v1);                             // v1    = [ py^2 + px^2       | py^2 + px^2          ]

  pc->s_sub_f64(v1, v1, f->sqinv2a_sqfr);                 // v1    = [ ?                 | py^2 + px^2 - fr^2   ]
  pc->s_madd_f64(v2, v1, f->inv2a_4a, v2);                // v2    = [ ?                 |b^2+4a(py^2+px^2-fr^2)] => [ ?    | d    ]
  pc->v_combine_hi_lo_f64(v2, v0, v2);                    // v2    = [ dd                | d                    ]
  pc->s_mul_f64(v0, v0, v3);                              // v0    = [ ?                 | b * (1/2a)           ]
  pc->v_dup_hi_f64(v3, f->sqinv2a_sqfr);                  // v3    = [ (1/2a)^2          | (1/2a)^2             ]
  pc->v_mul_f64(v2, v2, v3);                              // v2    = [ dd * (1/2a)^2     | d * (1/2a)^2         ]

  pc->v_cvt_f64_to_f32_lo(f->b.v128(), v0);
  pc->v_cvt_f64_to_f32_lo(f->d.v128(), v2);

  pc->v_broadcast_f32(f->b, f->b);
  pc->v_swizzle_f32x4(f->dd, f->d, swizzle(1, 1, 1, 1));
  pc->v_broadcast_f32(f->d, f->d);
  pc->v_broadcast_f32(f->dd, f->dd);

  if (is_rect_fill())
    pc->v_mov(f->vx, f->vx_start);
  else
    init_vx(f->vx, x);

  if (dithering_enabled())
    _dithering_context.start_at_x(x);
}

void FetchRadialGradientPart::advance_x(const Gp& x, const Gp& diff) noexcept {
  advance_x(x, diff, false);
}

void FetchRadialGradientPart::advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept {
  VecWidth vw = vec_width();
  Vec vd = pc->new_vec_with_width(vw, "@vd");

  // `vd` is `diff` converted to f32 and broadcasted to all lanes.
  pc->s_cvt_int_to_f32(vd, diff);
  pc->v_broadcast_f32(vd, vd);
  pc->v_add_f32(f->vx, f->vx, vd);

  if (dithering_enabled())
    _dithering_context.advance_x(x, diff, diff_within_bounds);
}

// bl::Pipeline::JIT::FetchRadialGradientPart - Fetch
// ==================================================

void FetchRadialGradientPart::prefetch_n() noexcept {
  Vec v0 = f->value;
  Vec v1 = pc->new_similar_reg(v0, "v1");

  pc->v_mul_f32(v1, f->vx, f->vx);
  pc->v_madd_f32(v0, f->dd, f->vx, f->d);
  pc->v_madd_f32(v0, f->ddd, v1, v0);
  pc->v_abs_f32(v0, v0);
  pc->v_sqrt_f32(v0, v0);
}

void FetchRadialGradientPart::postfetch_n() noexcept {}

void FetchRadialGradientPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  p.set_count(n);

#if defined(BL_JIT_ARCH_X86)
  VecWidth vw = vec_width();
#endif // BL_JIT_ARCH_X86

  GatherMode gather_mode = predicate.gather_mode();

  switch (uint32_t(n)) {
    case 1: {
      BL_ASSERT(predicate.is_empty());

      Gp r_idx = pc->new_gpz("r_idx");
      Vec v_idx = pc->new_vec128("v_idx");
      Vec v0 = pc->new_vec128("v0");

      pc->v_mov(v0, f->d.v128());
      pc->s_mul_f32(v_idx, f->vx, f->vx);
      pc->s_madd_f32(v0, f->dd, f->vx, v0);
      pc->s_madd_f32(v0, f->ddd, v_idx, v0);
      pc->v_abs_f32(v0, v0);
      pc->s_sqrt_f32(v0, v0);
      pc->s_madd_f32(v_idx, f->bd, f->vx, f->b);
      pc->v_add_f32(f->vx, f->vx, pc->simd_const(&ct.f32_1, Bcst::k32, f->vx));

      pc->v_add_f32(v_idx, v_idx, v0);

      pc->v_cvt_trunc_f32_to_i32(v_idx, v_idx);

      apply_extend(v_idx, v_idx, v0);

      pc->s_extract_u16(r_idx, v_idx, 0u);
      fetch_single_pixel(p, flags, r_idx);

      FetchUtils::satisfy_pixels(pc, p, flags);
      break;
    }

    case 4: {
      Vec v0 = f->value;
      Vec v1 = pc->new_similar_reg(v0, "v0");
      Vec v_idx = pc->new_vec128("v_idx");

      pc->v_madd_f32(v_idx, f->bd.v128(), f->vx.v128(), f->b.v128());

      if (predicate.is_empty()) {
        pc->v_add_f32(f->vx, f->vx, pc->simd_const(&ct.f32_4, Bcst::k32, f->vx));
      }

      pc->v_add_f32(v_idx, v_idx, v0.v128());
      pc->v_cvt_trunc_f32_to_i32(v_idx, v_idx);

      FetchUtils::IndexLayout index_layout = apply_extend(v_idx, v_idx, v0.v128());

      fetch_multiple_pixels(p, n, flags, v_idx, index_layout, gather_mode, [&](uint32_t step) noexcept {
        // Don't recalculate anything if this is a predicated load as it won't be used.
        if (!predicate.is_empty())
          return;

        switch (step) {
          case 0:
            pc->v_madd_f32(v0, f->dd, f->vx, f->d);
            break;
          case 1:
            pc->v_mul_f32(v1, f->vx, f->vx);
            break;
          case 2:
            pc->v_madd_f32(v0, f->ddd, v1, v0);
            pc->v_abs_f32(v0, v0);
            break;
          case 3:
            pc->v_sqrt_f32(v0, v0);
            break;
          default:
            break;
        }
      });

      if (!predicate.is_empty()) {
        advance_x(pc->_gp_none, predicate.count(), true);
        prefetch_n();
      }

      FetchUtils::satisfy_pixels(pc, p, flags);
      break;
    }

    case 8: {
#if defined(BL_JIT_ARCH_X86)
      if (vw >= VecWidth::k256) {
        Vec v0 = f->value;
        Vec v1 = pc->new_similar_reg(v0, "v1");
        Vec v_idx = pc->new_similar_reg(v0, "v_idx");

        pc->v_madd_f32(v_idx, f->bd, f->vx, f->b);

        if (predicate.is_empty()) {
          pc->v_add_f32(f->vx, f->vx, pc->simd_const(&ct.f32_8, Bcst::k32, f->vx));
        }

        pc->v_add_f32(v_idx, v_idx, v0);
        pc->v_cvt_trunc_f32_to_i32(v_idx, v_idx);

        FetchUtils::IndexLayout index_layout = apply_extend(v_idx, v_idx, v0);

        if (predicate.is_empty()) {
          pc->v_mov(v0, f->d);
          pc->v_mul_f32(v1, f->vx, f->vx);
        }

        fetch_multiple_pixels(p, n, flags, v_idx, index_layout, gather_mode, [&](uint32_t step) noexcept {
          // Don't recalculate anything if this is a predicated load as it won't be used.
          if (!predicate.is_empty())
            return;

          switch (step) {
            case 0:
              pc->v_madd_f32(v0, f->dd, f->vx, v0);
              break;
            case 1:
              pc->v_madd_f32(v0, f->ddd, v1, v0);
              break;
            case 2:
              pc->v_abs_f32(v0, v0);
              break;
            case 3:
              pc->v_sqrt_f32(v0, v0);
              break;
            default:
              break;
          }
        });

        if (!predicate.is_empty()) {
          advance_x(pc->_gp_none, predicate.count(), true);
          prefetch_n();
        }

        FetchUtils::satisfy_pixels(pc, p, flags);
        break;
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        Vec v0 = f->value;
        Vec v_tmp = pc->new_vec128("v0");
        Vec vIdx0 = pc->new_vec128("vIdx0");
        Vec vIdx1 = pc->new_vec128("vIdx1");

        pc->v_add_f32(v_tmp, f->vx, pc->simd_const(&ct.f32_4, Bcst::k32, f->vx));
        pc->v_madd_f32(vIdx1, f->dd, v_tmp, f->d);
        pc->v_madd_f32(vIdx0, f->bd.v128(), f->vx.v128(), f->b.v128());

        if (predicate.is_empty()) {
          pc->v_add_f32(f->vx, v_tmp, pc->simd_const(&ct.f32_4, Bcst::k32, f->vx));
        }

        pc->v_mul_f32(v_tmp, v_tmp, v_tmp);
        pc->v_madd_f32(vIdx1, f->ddd, v_tmp, vIdx1);
        pc->v_abs_f32(vIdx1, vIdx1);
        pc->v_sqrt_f32(vIdx1, vIdx1);

        pc->v_add_f32(vIdx0, vIdx0, v0.v128());
        pc->v_cvt_trunc_f32_to_i32(vIdx0, vIdx0);
        pc->v_cvt_trunc_f32_to_i32(vIdx1, vIdx1);

        FetchUtils::IndexLayout index_layout = apply_extend(vIdx0, vIdx1, v_tmp);

        fetch_multiple_pixels(p, n, flags, vIdx0, index_layout, gather_mode, [&](uint32_t step) noexcept {
          // Don't recalculate anything if this is a predicated load as it won't be used.
          if (!predicate.is_empty())
            return;

          switch (step) {
            case 0:
              pc->v_madd_f32(v0, f->dd, f->vx, f->d);
              break;
            case 1:
              pc->v_mul_f32(v_tmp, f->vx, f->vx);
              break;
            case 2:
              pc->v_madd_f32(v0, f->ddd, v_tmp, v0);
              pc->v_abs_f32(v0, v0);
              break;
            case 3:
              pc->v_sqrt_f32(v0, v0);
              break;
            default:
              break;
          }
        });

        if (!predicate.is_empty()) {
          advance_x(pc->_gp_none, predicate.count(), true);
          prefetch_n();
        }

        FetchUtils::satisfy_pixels(pc, p, flags);
        break;
      }

      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void FetchRadialGradientPart::init_vx(const Vec& vx, const Gp& x) noexcept {
  Mem increments = pc->simd_mem_const(&ct.f32_increments, Bcst::kNA_Unique, vx);
  pc->s_cvt_int_to_f32(vx, x);
  pc->v_broadcast_f32(vx, vx);
  pc->v_add_f32(vx, vx, increments);
}

FetchUtils::IndexLayout FetchRadialGradientPart::apply_extend(const Vec& idx0, const Vec& idx1, const Vec& tmp) noexcept {
  if (is_pad()) {
#if defined(BL_JIT_ARCH_X86)
    if (!pc->has_sse4_1()) {
      pc->v_packs_i32_i16(idx0, idx0, idx1);
      pc->v_min_i16(idx0, idx0, f->vmaxi);
      pc->v_max_i16(idx0, idx0, pc->simd_const(&ct.p_0000000000000000, Bcst::kNA, idx0));
      return FetchUtils::IndexLayout::kUInt16;
    }

    if (vec_width() > VecWidth::k128) {
      // Must be the same when using AVX2 vectors (256-bit and wider).
      BL_ASSERT(idx0.id() == idx1.id());

      pc->v_max_i32(idx0, idx0, pc->simd_const(&ct.p_0000000000000000, Bcst::kNA, idx0));
      pc->v_min_u32(idx0, idx0, f->vmaxi.clone_as(idx0));
      return FetchUtils::IndexLayout::kUInt32Lo16;
    }
#endif // BL_JIT_ARCH_X86

    pc->v_packs_i32_u16(idx0, idx0, idx1);
    pc->v_min_u16(idx0, idx0, f->vmaxi.clone_as(idx0));
    return FetchUtils::IndexLayout::kUInt16;
  }
  else if (idx0.id() == idx1.id()) {
    pc->v_and_i32(idx0, idx0, f->vmaxi.clone_as(idx0));
    pc->v_xor_i32(tmp, idx0, f->vrori.clone_as(idx0));
    pc->v_min_i16(idx0, idx0, tmp);
    return FetchUtils::IndexLayout::kUInt32Lo16;
  }
  else {
    pc->v_and_i32(idx0, idx0, f->vmaxi.clone_as(idx0));
    pc->v_and_i32(idx1, idx1, f->vmaxi.clone_as(idx1));
    pc->v_packs_i32_i16(idx0, idx0, idx1);
    pc->v_xor_i32(tmp, idx0, f->vrori.clone_as(idx0));
    pc->v_min_i16(idx0, idx0, tmp);
    return FetchUtils::IndexLayout::kUInt16;
  }
}

// bl::Pipeline::JIT::FetchConicGradientPart - Construction & Destruction
// ======================================================================

FetchConicGradientPart::FetchConicGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchGradientPart(pc, fetch_type, format) {

  _max_vec_width_supported = kMaxPlatformWidth;

  add_part_flags(PipePartFlags::kMaskedAccess | PipePartFlags::kExpensive);
  set_dithering_enabled(fetch_type == FetchType::kGradientConicDither);
  OpUtils::reset_var_struct(&f, sizeof(f));
}

// bl::Pipeline::JIT::FetchConicGradientPart - Prepare
// ===================================================

void FetchConicGradientPart::prepare_part() noexcept {
  _max_pixels = uint8_t(4 * pc->vec_multiplier());
}

// bl::Pipeline::JIT::FetchConicGradientPart - Init & Fini
// =======================================================

void FetchConicGradientPart::_init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept {
  VecWidth vw = vec_width(max_pixels());

  // Local Registers
  // ---------------

  _table_ptr = pc->new_gpz("f.table");                 // Reg.

  f->ty_tx = pc->new_vec128_f64x2("f.ty_tx");          // Reg/Mem.
  f->yy_yx = pc->new_vec128_f64x2("f.yy_yx");          // Reg/Mem.

  f->tx = pc->new_vec_with_width(vw, "f.tx");          // Reg/Mem.
  f->xx = pc->new_vec_with_width(vw, "f.xx");          // Reg/Mem.
  f->vx = pc->new_vec_with_width(vw, "f.vx");          // Reg.

  f->ay = pc->new_vec_with_width(vw, "f.ay");          // Reg/Mem.
  f->by = pc->new_vec_with_width(vw, "f.by");          // Reg/Mem.

  f->q_coeff = pc->new_vec_with_width(vw, "f.q_coeff");// Reg/Mem.
  f->n_coeff = pc->new_vec_with_width(vw, "f.n_coeff");// Reg/Mem.

  f->maxi = pc->new_vec_with_width(vw, "f.maxi");      // Reg/Mem.
  f->rori = pc->new_vec_with_width(vw, "f.rori");      // Reg/Mem.

  // Part Initialization
  // -------------------

  pc->load(_table_ptr, mem_ptr(fn.fetch_data(), REL_GRADIENT(lut.data)));

  if (dithering_enabled())
    _dithering_context.init_y(fn, x, y);

  pc->s_cvt_int_to_f64(f->ty_tx, y);
  pc->v_loadu128_f64(f->yy_yx, mem_ptr(fn.fetch_data(), REL_GRADIENT(conic.yx)));
  pc->v_broadcast_f64(f->ty_tx, f->ty_tx);
  pc->v_madd_f64(f->ty_tx, f->ty_tx, f->yy_yx, mem_ptr(fn.fetch_data(), REL_GRADIENT(conic.tx)));

  pc->v_broadcast_v128_f32(f->q_coeff, mem_ptr(fn.fetch_data(), REL_GRADIENT(conic.q_coeff)));
  pc->v_broadcast_v128_f32(f->n_coeff, mem_ptr(fn.fetch_data(), REL_GRADIENT(conic.n_div_1_2_4)));
  pc->v_broadcast_f32(f->xx, mem_ptr(fn.fetch_data(), REL_GRADIENT(conic.xx)));
  pc->v_broadcast_u32(f->maxi, mem_ptr(fn.fetch_data(), REL_GRADIENT(conic.maxi)));
  pc->v_broadcast_u32(f->rori, mem_ptr(fn.fetch_data(), REL_GRADIENT(conic.rori)));

  if (is_rect_fill()) {
    f->vx_start = pc->new_similar_reg(f->vx, "f.vx_start");
    init_vx(f->vx_start, x);
  }
}

void FetchConicGradientPart::_fini_part() noexcept {}

// bl::Pipeline::JIT::FetchConicGradientPart - Advance
// ===================================================

void FetchConicGradientPart::advance_y() noexcept {
  pc->v_add_f64(f->ty_tx, f->ty_tx, f->yy_yx);

  if (dithering_enabled())
    _dithering_context.advance_y();
}

void FetchConicGradientPart::start_at_x(const Gp& x) noexcept {
  Vec n_div_1 = pc->new_similar_reg(f->by, "@n_div_1");

  pc->v_cvt_f64_to_f32_lo(f->by.v128(), f->ty_tx);
  pc->v_swizzle_f32x4(f->tx.v128(), f->by.v128(), swizzle(0, 0, 0, 0));
  pc->v_swizzle_f32x4(f->by.v128(), f->by.v128(), swizzle(1, 1, 1, 1));

  if (!f->by.is_vec128()) {
    pc->v_broadcast_v128_f32(f->tx, f->tx.v128());
    pc->v_broadcast_v128_f32(f->by, f->by.v128());
  }

  pc->v_swizzle_f32x4(n_div_1, f->n_coeff, swizzle(0, 0, 0, 0));
  pc->v_abs_f32(f->ay, f->by);
  pc->v_srai_i32(f->by, f->by, 31);
  pc->v_and_f32(f->by, f->by, n_div_1);

  if (is_rect_fill())
    pc->v_mov(f->vx, f->vx_start);
  else
    init_vx(f->vx, x);

  if (dithering_enabled())
    _dithering_context.start_at_x(x);
}

void FetchConicGradientPart::advance_x(const Gp& x, const Gp& diff) noexcept {
  advance_x(x, diff, false);
}

void FetchConicGradientPart::advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept {
  VecWidth vw = vec_width(max_pixels());
  Vec vd = pc->new_vec_with_width(vw, "@vd");

  // `vd` is `diff` converted to f32 and broadcasted to all lanes.
  pc->s_cvt_int_to_f32(vd, diff);
  pc->v_broadcast_f32(vd, vd);
  pc->v_add_f32(f->vx, f->vx, vd);

  if (dithering_enabled())
    _dithering_context.advance_x(x, diff, diff_within_bounds);
}

// bl::Pipeline::JIT::FetchConicGradientPart - Fetch
// =================================================

void FetchConicGradientPart::prefetch_n() noexcept {}

void FetchConicGradientPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  p.set_count(n);

  VecWidth vw = vec_width(uint32_t(n));
  GatherMode gather_mode = predicate.gather_mode();

  Vec ay = VecWidthUtils::clone_vec_as(f->ay, vw);
  Vec by = VecWidthUtils::clone_vec_as(f->by, vw);
  Vec tx = VecWidthUtils::clone_vec_as(f->tx, vw);
  Vec xx = VecWidthUtils::clone_vec_as(f->xx, vw);
  Vec q_coeff = VecWidthUtils::clone_vec_as(f->q_coeff, vw);
  Vec n_coeff = VecWidthUtils::clone_vec_as(f->n_coeff, vw);

  Vec t0 = pc->new_vec_with_width(vw, "t0");
  Vec t1 = pc->new_vec_with_width(vw, "t1");
  Vec t2 = pc->new_vec_with_width(vw, "t2");
  Vec t3 = pc->new_vec_with_width(vw, "t3");
  Vec t4 = pc->new_vec_with_width(vw, "t4");
  Vec t5 = pc->new_vec_with_width(vw, "t5");

  switch (uint32_t(n)) {
    case 1: {
      Gp idx = pc->new_gpz("f.idx");

      pc->s_madd_f32(t0, f->vx.clone_as(t0), xx, tx);
      pc->v_abs_f32(t1, t0);

      pc->s_max_f32(t3, t1, ay);
      pc->s_min_f32(t2, t1, ay);
      pc->s_cmp_eq_f32(t1, t1, t2);
      pc->s_div_f32(t2, t2, t3);

      pc->v_swizzle_f32x4(t4, n_coeff, swizzle(kNDiv4, kNDiv4, kNDiv4, kNDiv4));
      pc->v_srai_i32(t0, t0, 31);
      pc->v_and_f32(t1, t1, t4);
      pc->s_mul_f32(t3, t2, t2);
      pc->v_swizzle_f32x4(t5, q_coeff, swizzle(kQ3, kQ3, kQ3, kQ3));
      pc->v_swizzle_f32x4(t4, q_coeff, swizzle(kQ2, kQ2, kQ2, kQ2));

      pc->s_madd_f32(t4, t5, t3, t4);
      pc->v_swizzle_f32x4(t5, q_coeff, swizzle(kQ1, kQ1, kQ1, kQ1));
      pc->s_madd_f32(t5, t4, t3, t5);
      pc->v_swizzle_f32x4(t4, n_coeff, swizzle(kNDiv2, kNDiv2, kNDiv2, kNDiv2));
      pc->v_and_f32(t0, t0, t4);
      pc->v_swizzle_f32x4(t4, q_coeff, swizzle(kQ0, kQ0, kQ0, kQ0));
      pc->s_madd_f32(t4, t5, t3, t4);
      pc->s_msub_f32(t1, t4, t2, t1);

      pc->v_abs_f32(t1, t1);
      pc->s_sub_f32(t1, t1, t0);
      pc->v_abs_f32(t1, t1);

      pc->v_swizzle_f32x4(t4, n_coeff, swizzle(kAngleOffset, kAngleOffset, kAngleOffset, kAngleOffset));
      pc->s_sub_f32(t1, t1, by);
      pc->v_abs_f32(t1, t1);
      pc->s_add_f32(t1, t1, t4);

      pc->v_cvt_round_f32_to_i32(t1, t1);
      pc->v_min_i32(t1, t1, f->maxi.clone_as(t1));
      pc->v_and_i32(t1, t1, f->rori.clone_as(t1));
      pc->s_extract_u16(idx, t1, 0);

      fetch_single_pixel(p, flags, idx);
      FetchUtils::satisfy_pixels(pc, p, flags);

      pc->v_add_f32(f->vx, f->vx, pc->simd_const(&ct.f32_1, Bcst::k32, f->vx));
      break;
    }

    case 4:
    case 8:
    case 16: {
      pc->v_madd_f32(t0, f->vx.clone_as(t0), xx, tx);
      pc->v_abs_f32(t1, t0);

      pc->v_max_f32(t3, t1, ay);
      pc->v_min_f32(t2, t1, ay);
      pc->v_cmp_eq_f32(t1, t1, t2);
      pc->v_div_f32(t2, t2, t3);

      pc->v_swizzle_f32x4(t4, n_coeff, swizzle(kNDiv4, kNDiv4, kNDiv4, kNDiv4));
      pc->v_srai_i32(t0, t0, 31);
      pc->v_and_f32(t1, t1, t4);
      pc->v_mul_f32(t3, t2, t2);
      pc->v_swizzle_f32x4(t5, q_coeff, swizzle(kQ3, kQ3, kQ3, kQ3));
      pc->v_swizzle_f32x4(t4, q_coeff, swizzle(kQ2, kQ2, kQ2, kQ2));

      pc->v_madd_f32(t4, t5, t3, t4);
      pc->v_swizzle_f32x4(t5, q_coeff, swizzle(kQ1, kQ1, kQ1, kQ1));
      pc->v_madd_f32(t5, t4, t3, t5);
      pc->v_swizzle_f32x4(t4, n_coeff, swizzle(kNDiv2, kNDiv2, kNDiv2, kNDiv2));
      pc->v_and_f32(t0, t0, t4);
      pc->v_swizzle_f32x4(t4, q_coeff, swizzle(kQ0, kQ0, kQ0, kQ0));
      pc->v_madd_f32(t4, t5, t3, t4);
      pc->v_msub_f32(t1, t4, t2, t1);

      pc->v_abs_f32(t1, t1);
      pc->v_sub_f32(t1, t1, t0);
      pc->v_abs_f32(t1, t1);

      pc->v_swizzle_f32x4(t4, n_coeff, swizzle(kAngleOffset, kAngleOffset, kAngleOffset, kAngleOffset));
      pc->v_sub_f32(t1, t1, by);
      pc->v_abs_f32(t1, t1);
      pc->v_add_f32(t1, t1, t4);

      pc->v_cvt_round_f32_to_i32(t1, t1);
      pc->v_min_i32(t1, t1, f->maxi.clone_as(t1));
      pc->v_and_i32(t1, t1, f->rori.clone_as(t1));

      fetch_multiple_pixels(p, n, flags, t1, FetchUtils::IndexLayout::kUInt32Lo16, gather_mode);

      if (predicate.is_empty()) {
        if (n == PixelCount(4))
          pc->v_add_f32(f->vx, f->vx, pc->simd_const(&ct.f32_4, Bcst::k32, f->vx));
        else if (n == PixelCount(8))
          pc->v_add_f32(f->vx, f->vx, pc->simd_const(&ct.f32_8, Bcst::k32, f->vx));
        else if (n == PixelCount(16))
          pc->v_add_f32(f->vx, f->vx, pc->simd_const(&ct.f32_16, Bcst::k32, f->vx));
      }
      else {
        advance_x(pc->_gp_none, predicate.count(), true);
      }

      FetchUtils::satisfy_pixels(pc, p, flags);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void FetchConicGradientPart::init_vx(const Vec& vx, const Gp& x) noexcept {
  Mem increments = pc->simd_mem_const(&ct.f32_increments, Bcst::kNA_Unique, vx);
  pc->s_cvt_int_to_f32(vx, x);
  pc->v_broadcast_f32(vx, vx);
  pc->v_add_f32(vx, vx, increments);
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
