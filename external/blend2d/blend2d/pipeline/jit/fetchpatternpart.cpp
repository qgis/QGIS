// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/compoppart_p.h>
#include <blend2d/pipeline/jit/fetchpatternpart_p.h>
#include <blend2d/pipeline/jit/fetchutilsbilinear_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/support/intops_p.h>

namespace bl::Pipeline::JIT {

#define REL_PATTERN(FIELD) BL_OFFSET_OF(FetchData::Pattern, FIELD)

// bl::Pipeline::JIT::FetchPadRoRContext
// =====================================

class FetchPadRoRContext {
public:
  PipeCompiler* pc {};
  FetchSimplePatternPart* _fetch_part {};

  //! Whether the pattern fetcher has fractional X.
  bool _has_frac_x {};
  //! Whether the pattern fetcher has fractional Y.
  bool _has_frac_y {};

  //! Horizontal extend mode.
  ExtendMode _extend_x {};

  //! Describes the current pixel index.
  uint32_t _fetch_index {};

  //! Describes the current index related to advancing.
  //!
  //! \note Fetch index and advance index could be different. In Fx and FxFy case, the fetcher needs to advance before
  //! a next index is calculated, because the current index was already either pre-fetched or fetched by previous loop
  //! iteration, which implies that we never want to fetch the current index again. The reason we use two counters is
  //! actually simplicity - `next_index()` uses `_fetch_index` and `_advance_pad_x()` uses `_advance_index`.
  uint32_t _advance_index {};

  //! Index extractor used to extract indexes from a vector so we can use them in regular [base + index] address.
  FetchUtils::IndexExtractor _index_extractor;

  Gp _x;
  Gp _w;
  Gp _idx;
  Gp _predicate_count;

  BL_NOINLINE explicit FetchPadRoRContext(FetchSimplePatternPart* fetch_part, PixelPredicate& predicate) noexcept
    : pc(fetch_part->pc),
      _fetch_part(fetch_part),
      _has_frac_x(fetch_part->has_frac_x()),
      _has_frac_y(fetch_part->has_frac_y()),
      _extend_x(fetch_part->extend_x()),
      _fetch_index(0),
      _advance_index(0),
      _index_extractor(fetch_part->pc) {

    if (!predicate.is_empty()) {
      _predicate_count = predicate.count();
    }

    FetchSimplePatternPart::SimpleRegs* f = &_fetch_part->f;

    if (_extend_x == ExtendMode::kPad) {
      _x = f->x;
      _w = f->w;
      _idx = f->x_padded;
    }
    else {
      _idx = pc->new_gpz("@idx");
    }
  }

  BL_INLINE_NODEBUG bool has_predicate() const noexcept { return _predicate_count.is_valid(); }

  BL_NOINLINE void begin() noexcept {
    if (_extend_x == ExtendMode::kPad) {
      // Nothing to setup here as each index is calculated by advancing `x` and then padding to `x_padded`.
    }
    else {
      FetchSimplePatternPart::SimpleRegs* f = &_fetch_part->f;
      Vec v_idx = pc->new_vec128("@v_idx");
      Vec v_src = f->x_vec_4;

      if (has_predicate()) {
        Gp gp_off = pc->gpz(_predicate_count);
        Vec v_off = pc->new_vec128("@v_off");

        Mem m = pc->tmp_stack(PipeCompiler::StackId::kCustom, 16);
        v_src = pc->new_vec128("@v_src");

        pc->v_storea128(m, f->x_set_4);

#if defined(BL_JIT_ARCH_X86)
        m.set_index(gp_off, 2);
#else
        Gp gp_base = pc->new_gpz("@base");
        pc->cc->load_address_of(gp_base, m);
        m = mem_ptr(gp_base, gp_off, 2);
#endif
        pc->v_broadcast_u32(v_off, m);

        pc->shift_or_rotate_left(v_src, f->x_vec_4, 4);
        pc->v_add_i32(f->x_vec_4, f->x_vec_4, v_off);
        _fixup_reflected_x();
        pc->v_alignr_u128(v_src, f->x_vec_4, v_src, 4);
      }

      pc->v_srai_i32(v_idx, v_src, 31);
      pc->v_xor_i32(v_idx, v_idx, v_src);

      if (!has_predicate()) {
        pc->v_add_i32(f->x_vec_4, f->x_vec_4, f->x_inc_4);
      }

      _index_extractor.begin(FetchUtils::IndexExtractor::kTypeUInt32, v_idx);
    }
  }

  BL_NOINLINE void end() noexcept {
    if (_extend_x == ExtendMode::kPad) {
      if (!_has_frac_x) {
        _advance_pad_x();
      }
    }
    else {
      if (!has_predicate()) {
        _fixup_reflected_x();
      }
    }
  }

  BL_NOINLINE Gp next_index() noexcept {
    if (_extend_x == ExtendMode::kPad) {
      if (_has_frac_x || _fetch_index != 0) {
        _advance_pad_x();
      }

      _fetch_index++;
      return _idx;
    }
    else {
      _index_extractor.extract(_idx.r32(), _fetch_index);

      _fetch_index++;
      return _idx;
    }
  }

  BL_NOINLINE void _advance_pad_x() noexcept {
    if (has_predicate() && _advance_index >= 2u) {
      // Make the last fetch point to the last predicated value, which would be correct if the pattern gets advanced.
      if (_advance_index == 2) {
        pc->add_ext(_x, _x, _predicate_count.clone_as(_x), 1, -2);
        pc->cmov(_idx.r32(), _x.r32(), ucmp_le(_x, _w));
      }
    }
    else {
      pc->inc(_x);
      pc->cmov(_idx.r32(), _x.r32(), ucmp_le(_x, _w));
    }

    _advance_index++;
  }

  BL_NOINLINE void _fixup_reflected_x() noexcept {
    FetchSimplePatternPart::SimpleRegs* f = &_fetch_part->f;
    Vec v_tmp = pc->new_vec128("v_tmp");

    pc->v_cmp_gt_i32(v_tmp, f->x_vec_4, f->x_max_4);
    pc->v_and_i32(v_tmp, v_tmp, f->x_nrm_4);
    pc->v_sub_i32(f->x_vec_4, f->x_vec_4, v_tmp);
  }
};

// bl::Pipeline::JIT::FetchPatternPart - Construction & Destruction
// ================================================================

FetchPatternPart::FetchPatternPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchPart(pc, fetch_type, format) {}

// bl::Pipeline::JIT::FetchSimplePatternPart - Construction & Destruction
// ======================================================================

FetchSimplePatternPart::FetchSimplePatternPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchPatternPart(pc, fetch_type, format) {

  static constexpr ExtendMode fExtendTable[] = { ExtendMode::kPad, ExtendMode::kRoR };

  _part_flags |= PipePartFlags::kAdvanceXNeedsDiff;
  _idx_shift = 0;
  _max_pixels = 4;

  // Setup registers, extend mode, and the maximum number of pixels that can be fetched at once.
  switch (fetch_type) {
    case FetchType::kPatternAlignedBlit:
      _part_flags |= PipePartFlags::kAdvanceXIsSimple;
      _max_vec_width_supported = kMaxPlatformWidth;
      _max_pixels = kUnlimitedMaxPixels;

      if (pc->has_masked_access_of(bpp()))
        _part_flags |= PipePartFlags::kMaskedAccess;
      break;

    case FetchType::kPatternAlignedPad:
      // TODO: [JIT] OPTIMIZATION: We have removed fetch2x4, so `_max_pixels` cannot be raised to 8.
      // _max_pixels = 8;
      _extend_x = ExtendMode::kPad;
      break;

    case FetchType::kPatternAlignedRepeat:
      _extend_x = ExtendMode::kRepeat;
#if defined(BL_JIT_ARCH_X86)
      _max_vec_width_supported = VecWidth::k256;
#endif // BL_JIT_ARCH_X86
      break;

    case FetchType::kPatternAlignedRoR:
      _extend_x = ExtendMode::kRoR;
      break;

    case FetchType::kPatternFxPad:
    case FetchType::kPatternFxRoR:
      _extend_x = fExtendTable[uint32_t(fetch_type) - uint32_t(FetchType::kPatternFxPad)];
      break;

    case FetchType::kPatternFyPad:
    case FetchType::kPatternFyRoR:
      _extend_x = fExtendTable[uint32_t(fetch_type) - uint32_t(FetchType::kPatternFyPad)];
      break;

    case FetchType::kPatternFxFyPad:
    case FetchType::kPatternFxFyRoR:
      _extend_x = fExtendTable[uint32_t(fetch_type) - uint32_t(FetchType::kPatternFxFyPad)];
      add_part_flags(PipePartFlags::kExpensive);
      break;

    default:
      BL_NOT_REACHED();
  }

  if (extend_x() == ExtendMode::kPad || extend_x() == ExtendMode::kRoR) {
    if (IntOps::is_power_of_2(_bpp))
      _idx_shift = uint8_t(IntOps::ctz(_bpp));
  }

  OpUtils::reset_var_struct(&f, sizeof(f));
}

// bl::Pipeline::JIT::FetchSimplePatternPart - Init & Fini
// =======================================================

void FetchSimplePatternPart::_init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept {
  if (is_aligned_blit()) {
    // This is a special-case designed only for rectangular blits that never
    // go out of image bounds (this implies that no extend mode is applied).
    BL_ASSERT(is_rect_fill());

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    f->stride       = pc->new_gpz("f.stride");       // Mem.
    f->srcp1        = pc->new_gpz("f.srcp1");        // Reg.
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    pc->load(f->stride, mem_ptr(fn.fetch_data(), REL_PATTERN(src.stride)));
    pc->sub(f->srcp1.r32(), y.r32(), mem_ptr(fn.fetch_data(), REL_PATTERN(simple.ty)));
    pc->mul(f->srcp1, f->srcp1, f->stride);

    pc->add(f->srcp1, f->srcp1, mem_ptr(fn.fetch_data(), REL_PATTERN(src.pixel_data)));
    pc->prefetch(mem_ptr(f->srcp1));

    Gp cut = pc->new_gpz("@stride_cut");
    pc->mul(cut.r32(), mem_ptr(fn.fetch_data(), REL_PATTERN(src.size.w)), int(bpp()));
    pc->sub(f->stride, f->stride, cut);
  }
  else {
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    f->srcp0        = pc->new_gpz("f.srcp0");        // Reg.
    f->srcp1        = pc->new_gpz("f.srcp1");        // Reg (Fy|FxFy).
    f->w            = pc->new_gp32("f.w");           // Mem.
    f->h            = pc->new_gp32("f.h");           // Mem.
    f->y            = pc->new_gp32("f.y");           // Reg.

    f->stride       = pc->new_gpz("f.stride");       // Init only.
    f->ry           = pc->new_gp32("f.ry");          // Init only.
    f->v_extend_data  = cc->new_stack(sizeof(FetchData::Pattern::VertExtendData), 16, "f.v_extend_data");
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Apply alpha offset to source pointers.
    if (_alpha_fetch && extend_x() != ExtendMode::kRepeat) {
      _fetch_info.apply_alpha_offset();
    }

    pc->add(f->y, y, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.ty)));

    // The idea is that both Fx and Fy are compatible with FxFy so we increment Y if this is Fx only fetch.
    if (is_pattern_fx()) {
      pc->inc(f->y);
    }

    pc->load_u32(f->h, mem_ptr(fn.fetch_data(), REL_PATTERN(src.size.h)));
    pc->load_u32(f->ry, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.ry)));
    pc->load(f->stride, mem_ptr(fn.fetch_data(), REL_PATTERN(src.stride)));

    // Vertical Extend
    // ---------------
    //
    // Vertical extend modes are not hardcoded in the generated pipeline to decrease the number of possible pipeline
    // combinations. This means that the compiled pipeline supports all vertical extend modes. The amount of code that
    // handles vertical extend modes has been minimized so runtime overhead during `advance_y()` should be negligible.

    {
      // Vertical Extend - Prepare
      // -------------------------

      Label L_VertRoR = pc->new_label();
      Label L_VertSwap = pc->new_label();
      Label L_VertDone = pc->new_label();

      Gp y_mod = pc->new_gpz("f.y_mod").r32();
      Gp hMinus1 = pc->new_gpz("f.hMinus1").r32();
      Gp yModReg = y_mod.clone_as(f->stride);

      VecArray vStrideStopVec;
      Vec vRewindDataVec = pc->new_vec128("f.vRewindData");

      if (pc->is_32bit()) {
        pc->new_vec_array(vStrideStopVec, 1, VecWidth::k128, "f.vStrideStopVec");

        constexpr int kRewindDataOffset = 16;
        pc->v_loadu64(vRewindDataVec, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.v_extend_data) + kRewindDataOffset));
        pc->v_storeu64(f->v_extend_data.clone_adjusted(kRewindDataOffset), vRewindDataVec);
      }
      else {
        constexpr int kRewindDataOffset = 32;

#if defined(BL_JIT_ARCH_X86)
        if (pc->has_avx2())
        {
          pc->new_vec_array(vStrideStopVec, 1, VecWidth::k256, "f.vStrideStopVec");
        }
        else
#endif // BL_JIT_ARCH_X86
        {
          pc->new_vec_array(vStrideStopVec, 2, VecWidth::k128, "f.vStrideStopVec");
        }

        pc->v_loadu128(vRewindDataVec, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.v_extend_data) + kRewindDataOffset));
        pc->v_storea128(f->v_extend_data.clone_adjusted(kRewindDataOffset), vRewindDataVec);
      }

      pc->v_loadavec(vStrideStopVec, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.v_extend_data)), Alignment(8));

      // Don't do anything if we are within bounds as this is the case v_extend_data was prepared for.
      pc->mov(y_mod, f->y);
      pc->j(L_VertDone, ucmp_lt(f->y, f->h));

      // Decide between PAD and RoR.
      pc->j(L_VertRoR, test_nz(f->ry));

      // Handle PAD - we know that we are outside of bounds, so y_mod would become either 0 or h-1.
      pc->sar(y_mod, y_mod, 31);
      pc->sub(hMinus1, f->h, 1);

      pc->bic(y_mod, hMinus1, y_mod);
      pc->j(L_VertSwap);

      // Handle RoR - we have to repeat to `ry`, which is double the height in reflect case.
      pc->bind(L_VertRoR);
      pc->umod(f->y, f->y, f->ry);
      pc->mov(y_mod, f->y);

      // If we are within bounds already it means this is either repeat or reflection, which is in repeat part.
      pc->j(L_VertDone, ucmp_lt(f->y, f->h));

      // We are reflecting at the moment, `y_mod` has to be updated.
      pc->sub(y_mod, y_mod, f->ry);
      pc->sub(f->y, f->y, f->h);
      pc->not_(y_mod, y_mod);

      // Vertical Extend - Done
      // ----------------------

      pc->bind(L_VertSwap);
      swap_stride_stop_data(vStrideStopVec);

      pc->bind(L_VertDone);
      pc->mul(yModReg, yModReg, f->stride);
      pc->v_storeavec(f->v_extend_data, vStrideStopVec, Alignment(16));
      pc->add(f->srcp1, y_mod.clone_as(f->srcp1), mem_ptr(fn.fetch_data(), REL_PATTERN(src.pixel_data)));

      if (_fetch_info.applied_offset()) {
        pc->add(f->srcp1, f->srcp1, _fetch_info.applied_offset());
      }
    }

    // Horizontal Extend
    // -----------------
    //
    // Horizontal extend modes are hardcoded for performance reasons. Every extend mode
    // requires different strategy to make horizontal advancing as fast as possible.

    if (extend_x() == ExtendMode::kPad) {
      // Horizontal Pad
      // --------------
      //
      // There is not much to invent to clamp horizontally. The `f->x` is a raw coordinate that is clamped each
      // time it's used as an index. To make it fast we use two variables `x` and `x_padded`, which always contains
      // `x` clamped to `[x, w]` range. The advantage of this approach is that every time we increment `1` to `x` we
      // need only 2 instructions to calculate new `x_padded` value as it was already padded to the previous index,
      // and it could only get greater by `1` or stay where it was in a case we already reached the width `w`.

      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      f->x          = pc->new_gp32("f.x");             // Reg.
      f->x_padded    = pc->new_gpz("f.x_padded");      // Reg.
      f->x_origin    = pc->new_gp32("f.x_origin");       // Mem.
      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      pc->load_u32(f->w, mem_ptr(fn.fetch_data(), REL_PATTERN(src.size.w)));
      pc->load_u32(f->x_origin, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.tx)));

      // Fy pattern falls to Fx/Fy/FxFy category, which means that it's compatible with FxFy, we must increment the
      // X origin in that case as we know that weights for the first pixel are all zeros (compatibility with FxFy).
      if (is_pattern_fy()) {
        pc->inc(f->x_origin);
      }

      if (is_rect_fill()) {
        pc->add(f->x_origin, f->x_origin, x);
      }

      pc->dec(f->w);
    }

    if (extend_x() == ExtendMode::kRepeat) {
      // Horizontal Repeat - AA-Only, Large Fills
      // ----------------------------------------
      //
      // This extend mode is only used to blit patterns that are tiled and that exceed some predefined width-limit
      // (like 16|32|etc). It's specialized for larger patterns because it contains a condition in fetchN() that
      // jumps if `f->x` is at the end or near of the patterns end. That's why the pattern width should be large
      // enough that this branch is not mispredicted often. For smaller patterns RoR more is more suitable as there
      // is no branch required and the repeat|reflect is handled by SIMD instructions.
      //
      // This implementation generally uses two tricks to make the tiling faster:
      //
      //   1. It changes row indexing from [0..width) to [-width..0). The reason for such change is that when ADD
      //      instruction is executed it updates processor FLAGS register, if SIGN flag is zero it means that repeat
      //      is needed. This saves us one condition.
      //
      //   2. It multiplies X coordinates (all of them) by pattern's BPP (bytes per pixel). The reason is to completely
      //      eliminate `index * scale` in memory addressing (and in case of weird BPP to eliminate IMUL).

      // NOTE: These all must be `intptr_t` because of memory indexing and the
      // use of the sign (when f->x is used as an index it's always negative).

      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      f->x           = pc->new_gpz("f.x");            // Reg.
      f->x_origin    = pc->new_gpz("f.x_origin");     // Mem.
      f->x_restart   = pc->new_gpz("f.x_restart");    // Mem.
      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      pc->load_u32(f->w, mem_ptr(fn.fetch_data(), REL_PATTERN(src.size.w)));
      pc->load_u32(f->x_origin.r32(), mem_ptr(fn.fetch_data(), REL_PATTERN(simple.tx)));

      if (is_pattern_fy())
        pc->inc(f->x_origin.r32());

      if (is_rect_fill()) {
        pc->add(f->x_origin.r32(), f->x_origin.r32(), x);
        pc->umod(f->x_origin.r32(), f->x_origin.r32(), f->w);
      }

      pc->mul(f->w      , f->w      , int(bpp()));
      pc->mul(f->x_origin, f->x_origin, int(bpp()));

      pc->sub(f->x_origin, f->x_origin, f->w.clone_as(f->x_origin));
      pc->add(f->srcp1, f->srcp1, f->w.clone_as(f->srcp1));
      pc->neg(f->x_restart, f->w.clone_as(f->x_restart));
    }

    if (extend_x() == ExtendMode::kRoR) {
      // Horizontal RoR [Repeat or Reflect]
      // ----------------------------------
      //
      // This mode handles both Repeat and Reflect cases. It uses the following formula to either REPEAT or REFLECT
      // X coordinate:
      //
      //   int index = (x >> 31) ^ x;
      //
      // The beauty of this method is that if X is negative it reflects, if it's positive it's kept as is. Then the
      // implementation handles both modes the following way:
      //
      //   1. REPEAT - X is always bound to interval [0...Width), so when the index is calculated it never reflects.
      //      When `f->x` reaches the pattern width it's simply corrected as `f->x -= f->rx`, where `f->rx` is equal
      //      to `pattern.size.w`.
      //
      //   2. REFLECT - X is always bound to interval [-Width...Width) so it can reflect. When `f->x` reaches the
      //      pattern width it's simply corrected as `f->x -= f->rx`, where `f->rx` is equal to `pattern.size.w * 2`
      //      so it goes negative.

      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      f->x          = pc->new_gp32("f.x");                // Reg.
      f->x_origin   = pc->new_gp32("f.x_origin");         // Mem.
      f->x_restart  = pc->new_gp32("f.x_restart");        // Mem.
      f->rx         = pc->new_gp32("f.rx");               // Mem.

      if (max_pixels() >= 4) {
        f->x_vec_4    = pc->new_vec128("f.x_vec_4");          // Reg (fetchN).
        f->x_set_4    = pc->new_vec128("f.x_set_4");          // Mem (fetchN).
        f->x_inc_4    = pc->new_vec128("f.x_inc_4");          // Mem (fetchN).
        f->x_nrm_4    = pc->new_vec128("f.x_nrm_4");          // Mem (fetchN).
        f->x_max_4    = pc->new_vec128("f.x_max_4");          // Mem (fetchN).
      }
      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      pc->load_u32(f->w , mem_ptr(fn.fetch_data(), REL_PATTERN(src.size.w)));
      pc->load_u32(f->rx, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.rx)));

      if (max_pixels() >= 4) {
        pc->v_cvt_u8_to_u32(f->x_set_4, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.ix)));
        pc->v_swizzle_u32x4(f->x_inc_4, f->x_set_4, swizzle(3, 3, 3, 3));

        if (!has_frac_x()) {
          pc->v_sllb_u128(f->x_set_4, f->x_set_4, 4);
        }
      }

      pc->sub(f->x_restart, f->w, f->rx);
      pc->dec(f->w);

      if (max_pixels() >= 4) {
        pc->v_broadcast_u32(f->x_max_4, f->w);
        pc->v_broadcast_u32(f->x_nrm_4, f->rx);
      }

      pc->load_u32(f->x_origin, mem_ptr(fn.fetch_data(), REL_PATTERN(simple.tx)));

      if (is_pattern_fy()) {
        pc->inc(f->x_origin);
      }

      if (is_rect_fill()) {
        Gp norm = pc->new_gp32("@norm");

        pc->add(f->x_origin, f->x_origin, x);
        pc->umod(f->x_origin, f->x_origin, f->rx);

        pc->select(norm, Imm(0), f->rx, ucmp_le(f->x_origin, f->w));
        pc->sub(f->x_origin, f->x_origin, norm);
      }
    }

    // Fractional - Fx|Fy|FxFy
    // -----------------------

    if (is_pattern_unaligned()) {
      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      f->pix_l       = pc->new_vec128("f.pix_l");           // Reg (Fx|FxFy).

      f->wa         = pc->new_vec128("f.wa");             // Reg/Mem (RGBA mode).
      f->wb         = pc->new_vec128("f.wb");             // Reg/Mem (RGBA mode).
      f->wc         = pc->new_vec128("f.wc");             // Reg/Mem (RGBA mode).
      f->wd         = pc->new_vec128("f.wd");             // Reg/Mem (RGBA mode).

      f->wc_wd      = pc->new_vec128("f.wc_wd");          // Reg/Mem (RGBA mode).
      f->wa_wb      = pc->new_vec128("f.wa_wb");          // Reg/Mem (RGBA mode).

      f->wd_wb      = pc->new_vec128("f.wd_wb");          // Reg/Mem (Alpha mode).
      f->wa_wc      = pc->new_vec128("f.wa_wc");          // Reg/Mem (Alpha mode).
      f->wb_wd      = pc->new_vec128("f.wb_wd");          // Reg/Mem (Alpha mode).
      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      Vec weights = pc->new_vec128("weights");
      Mem wPtr = mem_ptr(fn.fetch_data(), REL_PATTERN(simple.wa));

      // [00 Wd 00 Wc 00 Wb 00 Wa]
      pc->v_loadu128(weights, wPtr);
      // [Wd Wc Wb Wa Wd Wc Wb Wa]
      pc->v_packs_i32_i16(weights, weights, weights);

      if (is_alpha_fetch()) {
        if (is_pattern_fy()) {
          pc->v_swizzle_lo_u16x4(f->wd_wb, weights, swizzle(3, 1, 3, 1));
          if (max_pixels() >= 4) {
            pc->v_swizzle_u32x4(f->wd_wb, f->wd_wb, swizzle(1, 0, 1, 0));
          }
        }
        else if (is_pattern_fx()) {
          pc->v_swizzle_u32x4(f->wc_wd, weights, swizzle(3, 3, 3, 3));
        }
        else {
          pc->v_swizzle_lo_u16x4(f->wa_wc, weights, swizzle(2, 0, 2, 0));
          pc->v_swizzle_lo_u16x4(f->wb_wd, weights, swizzle(3, 1, 3, 1));
          if (max_pixels() >= 4) {
            pc->v_swizzle_u32x4(f->wa_wc, f->wa_wc, swizzle(1, 0, 1, 0));
            pc->v_swizzle_u32x4(f->wb_wd, f->wb_wd, swizzle(1, 0, 1, 0));
          }
        }
      }
      else {
        // [Wd Wd Wc Wc Wb Wb Wa Wa]
        pc->v_interleave_lo_u16(weights, weights, weights);

        if (is_pattern_fy()) {
          pc->v_swizzle_u32x4(f->wb, weights, swizzle(1, 1, 1, 1));
          pc->v_swizzle_u32x4(f->wd, weights, swizzle(3, 3, 3, 3));
        }
        else if (is_pattern_fx()) {
          pc->v_swizzle_u32x4(f->wc_wd, weights, swizzle(3, 3, 2, 2));
          if (max_pixels() >= 4) {
            pc->v_swizzle_u32x4(f->wc, weights, swizzle(2, 2, 2, 2));
            pc->v_swizzle_u32x4(f->wd, weights, swizzle(3, 3, 3, 3));
          }
        }
        else {
          pc->v_swizzle_u32x4(f->wa_wc, weights, swizzle(0, 0, 2, 2));
          pc->v_swizzle_u32x4(f->wb_wd, weights, swizzle(1, 1, 3, 3));

          if (max_pixels() >= 4) {
            pc->v_swizzle_u32x4(f->wa, weights, swizzle(0, 0, 0, 0));
            pc->v_swizzle_u32x4(f->wb, weights, swizzle(1, 1, 1, 1));
            pc->v_swizzle_u32x4(f->wc, weights, swizzle(2, 2, 2, 2));
            pc->v_swizzle_u32x4(f->wd, weights, swizzle(3, 3, 3, 3));
          }
        }
      }
    }

    // If the pattern has a fractional Y then advance in vertical direction.
    // This ensures that both `srcp0` and `srcp1` are initialized, otherwise
    // `srcp0` would contain undefined content.
    if (has_frac_y()) {
      advance_y();
    }
  }
}

void FetchSimplePatternPart::_fini_part() noexcept {}

// bl::Pipeline::JIT::FetchSimplePatternPart - Utilities
// =====================================================

void FetchSimplePatternPart::swap_stride_stop_data(VecArray& v) noexcept {
  if (pc->is_32bit())
    pc->v_swap_u32(v, v);
  else
    pc->v_swap_u64(v, v);
}

// bl::Pipeline::JIT::FetchSimplePatternPart - Advance
// ===================================================

void FetchSimplePatternPart::advance_y() noexcept {
  if (is_aligned_blit()) {
    // Blit AA
    // -------

    // That's the beauty of AABlit - no checks needed, no extend modes used.
    pc->add(f->srcp1, f->srcp1, f->stride);
  }
  else {
    // Vertical Extend Mode Handling
    // -----------------------------

    int kStrideArrayOffset = 0;
    int kYStopArrayOffset = int(pc->register_size()) * 2;
    int kYRewindOffset = int(pc->register_size()) * 4;
    int kPixelPtrRewindOffset = int(pc->register_size()) * 5;

    Label L_Done = pc->new_label();
    Label L_YStop = pc->new_label();

    pc->inc(f->y);

    // If this pattern fetch uses two source pointers (one for current scanline
    // and one for previous one) copy current to the previous so it can be used
    // (only fetchers that use Fy).
    if (has_frac_y()) {
      pc->mov(f->srcp0, f->srcp1);
    }

    pc->j(L_YStop, cmp_eq(f->y, f->v_extend_data.clone_adjusted(kYStopArrayOffset)));
    pc->add(f->srcp1, f->srcp1, f->v_extend_data.clone_adjusted(kStrideArrayOffset));
    pc->bind(L_Done);

    PipeInjectAtTheEnd injected(pc);
    pc->bind(L_YStop);

    // Swap stride and y_stop pairs.
    if (pc->is_64bit()) {
#if defined(BL_JIT_ARCH_X86)
      if (pc->has_avx2()) {
        Vec v = pc->new_vec256("f.v_tmp");
        pc->v_swap_u64(v, f->v_extend_data);
        pc->v_storeu256(f->v_extend_data, v);
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        Vec v = pc->new_vec128("f.v_tmp");
        Mem stride_array = f->v_extend_data.clone_adjusted(kStrideArrayOffset);
        Mem yStopArray = f->v_extend_data.clone_adjusted(kYStopArrayOffset);
        pc->v_swap_u64(v, stride_array);
        pc->v_storea128(stride_array, v);
        pc->v_swap_u64(v, yStopArray);
        pc->v_storea128(yStopArray, v);
      }
    }
    else {
      Vec v0 = pc->new_vec128("f.v_tmp");
      pc->v_swap_u32(v0, f->v_extend_data);
      pc->v_storea128(f->v_extend_data, v0);
    }

    // Rewind y and pixel-ptr.
    pc->sub(f->y, f->y, f->v_extend_data.clone_adjusted(kYRewindOffset));
    pc->sub(f->srcp1, f->srcp1, f->v_extend_data.clone_adjusted(kPixelPtrRewindOffset));
    pc->j(L_Done);
  }
}

void FetchSimplePatternPart::start_at_x(const Gp& x) noexcept {
  if (is_aligned_blit()) {
    // Blit AA
    // -------

    // TODO: [JIT] OPTIMIZATION: Relax this constraint.
    // Rectangular blits only.
    BL_ASSERT(is_rect_fill());
  }
  else if (extend_x() == ExtendMode::kPad) {
    // Horizontal Pad
    // --------------

    if (!is_rect_fill())
      pc->add(f->x, f->x_origin, x);                      // f->x = f->x_origin + x;
    else
      pc->mov(f->x, f->x_origin);                         // f->x = f->x_origin;
    pc->sbound(f->x_padded.r32(), f->x, f->w);            // f->x_padded = signed_bound(f->x, f->w)
  }
  else if (extend_x() == ExtendMode::kRepeat) {
    // Horizontal Repeat - AA-Only, Large Fills
    // ----------------------------------------

    pc->mov(f->x, f->x_origin);                           // f->x = f->x_origin;
    if (!is_rect_fill()) {                                // if (!RectFill) {
      pc->add_scaled(f->x, x.clone_as(f->x), int(bpp())); //   f->x += x * pattern.bpp;
      repeat_or_reflect_x();                              //   f->x = repeat_large(f->x);
    }                                                     // }
  }
  else if (extend_x() == ExtendMode::kRoR) {
    // Horizontal RoR [Repeat or Reflect]
    // ----------------------------------

    pc->mov(f->x, f->x_origin);                           // f->x = f->x_origin;
    if (!is_rect_fill()) {                                // if (!RectFill) {
      pc->add(f->x, f->x, x);                             //   f->x += x;
      repeat_or_reflect_x();                              //   f->x = repeat_or_reflect(f->x);
    }                                                     // }
  }
  else {
    BL_NOT_REACHED();
  }

  prefetch_acc_x();

  if (pixel_granularity() > 1)
    enter_n();
}

void FetchSimplePatternPart::advance_x(const Gp& x, const Gp& diff) noexcept {
  bl_unused(x);
  Gp fx32 = f->x.r32();

  if (pixel_granularity() > 1) {
    leave_n();
  }

  if (is_aligned_blit()) {
    // Blit AA
    // -------

    pc->add_scaled(f->srcp1, diff.clone_as(f->srcp1), int(bpp()));
  }
  else if (extend_x() == ExtendMode::kPad) {
    // Horizontal Pad
    // --------------

    pc->add(fx32, fx32, diff);                             // f->x += diff;
    pc->sbound(f->x_padded.r32(), f->x, f->w);              // f->x_padded = signed_bound(f->x, f->w)
  }
  else if (extend_x() == ExtendMode::kRepeat) {
    // Horizontal Repeat - AA-Only, Large Fills
    // ----------------------------------------

    pc->add_scaled(f->x, diff.clone_as(f->x), int(bpp()));  // f->x += diff * pattern.bpp;
    repeat_or_reflect_x();                                    // f->x = repeat_large(f->x);
  }
  else if (extend_x() == ExtendMode::kRoR) {
    // Horizontal RoR [Repeat or Reflect]
    // ----------------------------------

    pc->add(fx32, fx32, diff);                             // f->x += diff;
    repeat_or_reflect_x();                                    // f->x = repeat_or_reflect(f->x);
  }

  prefetch_acc_x();

  if (pixel_granularity() > 1) {
    enter_n();
  }
}

void FetchSimplePatternPart::advance_x_by_one() noexcept {
  if (is_aligned_blit()) {
    // Blit AA
    // -------

    pc->add(f->srcp1, f->srcp1, int(bpp()));
  }
  else if (extend_x() == ExtendMode::kPad) {
    // Horizontal Pad
    // --------------

    pc->inc(f->x);
    pc->cmov(f->x_padded.r32(), f->x, ucmp_le(f->x, f->w));
  }
  else if (extend_x() == ExtendMode::kRepeat) {
    // Horizontal Repeat - AA-Only, Large Fills
    // ----------------------------------------

    pc->cmov(f->x, f->x_restart, add_z(f->x, int(bpp())));
  }
  else if (extend_x() == ExtendMode::kRoR) {
    // Horizontal RoR [Repeat or Reflect]
    // ----------------------------------

    pc->inc(f->x);
    pc->cmov(f->x, f->x_restart, scmp_gt(f->x, f->w));
  }
}

void FetchSimplePatternPart::repeat_or_reflect_x() noexcept {
  if (is_aligned_blit()) {
    // Blit AA
    // -------

    // Nothing...
  }
  else if (extend_x() == ExtendMode::kRepeat) {
    // Horizontal Repeat - AA-Only, Large Fills
    // ----------------------------------------

    Label L_HorzSkip = pc->new_label();

    pc->j(L_HorzSkip, scmp_lt(f->x, 0));                   // if (f->x >= 0 &&
    pc->j(L_HorzSkip, add_s(f->x, f->x_restart));          //     f->x -= f->w >= 0) {
    // `f->x` too large to be corrected by `f->w`, so do it the slow way:
    pc->umod(f->x.r32(), f->x.r32(), f->w.r32());          //   f->x %= f->w;
    pc->add(f->x, f->x, f->x_restart);                     //   f->x -= f->w;
    pc->bind(L_HorzSkip);                                  // }
  }
  else if (extend_x() == ExtendMode::kRoR) {
    // Horizontal RoR [Repeat or Reflect]
    // ----------------------------------

    Label L_HorzSkip = pc->new_label();
    Gp norm = pc->new_gp32("@norm");

    pc->j(L_HorzSkip, scmp_lt(f->x, f->rx));               // if (f->x >= f->rx) {
    pc->umod(f->x, f->x, f->rx);                           //   f->x %= f->rx;
    pc->bind(L_HorzSkip);                                  // }
    pc->select(norm, Imm(0), f->rx, scmp_le(f->x, f->w));  // norm = (f->x < f->w) ? 0 : f->rx;
    pc->sub(f->x, f->x, norm);                             // f->x -= norm;
  }
}

void FetchSimplePatternPart::prefetch_acc_x() noexcept {
  if (!has_frac_x())
    return;

  Gp idx;

  // Horizontal Pad
  // --------------

  if (extend_x() == ExtendMode::kPad) {
    idx = f->x_padded;
  }

  // Horizontal Repeat - AA-Only, Large Fills
  // ----------------------------------------

  if (extend_x() == ExtendMode::kRepeat) {
    idx = f->x;
  }

  // Horizontal RoR [Repeat or Reflect]
  // ----------------------------------

  if (extend_x() == ExtendMode::kRoR) {
    idx = pc->new_gpz("@idx");
    pc->reflect(idx.r32(), f->x);
  }

  if (is_alpha_fetch()) {
    if (is_pattern_fx()) {
      pc->v_load8(f->pix_l, mem_ptr(f->srcp1, idx, _idx_shift));
    }
    else {
      pc->v_load8(f->pix_l, mem_ptr(f->srcp0, idx, _idx_shift));
      pc->x_insert_word_or_byte(f->pix_l, mem_ptr(f->srcp1, idx, _idx_shift), 1);
    }
  }
  else {
    if (is_pattern_fx()) {
      pc->v_broadcast_u32(f->pix_l, mem_ptr(f->srcp1, idx, _idx_shift));
    }
    else {
      pc->v_loadu32(f->pix_l, mem_ptr(f->srcp1, idx, _idx_shift));
      FetchUtils::fetch_second_32bit_element(pc, f->pix_l, mem_ptr(f->srcp0, idx, _idx_shift));
    }
  }
}

// bl::Pipeline::JIT::FetchSimplePatternPart - Fetch
// =================================================

void FetchSimplePatternPart::enter_n() noexcept {
  if (is_aligned_blit()) {
    // Blit AA
    // -------

    // Nothing...
  }
  else if (extend_x() == ExtendMode::kPad) {
    // Horizontal Pad
    // --------------

    // Nothing...
  }
  else if (extend_x() == ExtendMode::kRoR) {
    // Horizontal RoR [Repeat or Reflect]
    // ----------------------------------

    Vec xFix4 = pc->new_vec128("@xFix4");
    pc->v_broadcast_u32(f->x_vec_4, f->x.r32());
    pc->v_add_i32(f->x_vec_4, f->x_vec_4, f->x_set_4);

    pc->v_cmp_gt_i32(xFix4, f->x_vec_4, f->x_max_4);
    pc->v_and_i32(xFix4, xFix4, f->x_nrm_4);
    pc->v_sub_i32(f->x_vec_4, f->x_vec_4, xFix4);
  }
}

void FetchSimplePatternPart::leave_n() noexcept {
  if (is_aligned_blit()) {
    // Blit AA
    // -------

    // Nothing...
  }
  else if (extend_x() == ExtendMode::kPad) {
    // Horizontal Pad
    // --------------

    // Nothing...
  }
  else if (extend_x() == ExtendMode::kRoR) {
    // Horizontal RoR [Repeat or Reflect]
    // ----------------------------------

    pc->s_mov_u32(f->x.r32(), f->x_vec_4);

    if (has_frac_x()) {
      pc->dec(f->x);
      pc->cmov(f->x, f->w, scmp_lt(f->x, f->x_restart));
    }
  }
}

void FetchSimplePatternPart::prefetch_n() noexcept {}
void FetchSimplePatternPart::postfetch_n() noexcept {}

void FetchSimplePatternPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  p.set_count(n);

  if (!bl_test_flag(flags, PixelFlags::kPA_PI_UA_UI | PixelFlags::kPC_UC)) {
    if (p.isRGBA32())
      flags |= PixelFlags::kPC;
    else
      flags |= PixelFlags::kPA;
  }

  if (is_aligned_blit()) {
    FetchUtils::fetch_pixels(pc, p, n, flags, fetch_info(), f->srcp1, Alignment(1), AdvanceMode::kAdvance, predicate);
    return;
  }

  if (!predicate.is_empty()) {
    flags |= PixelFlags::kLastPartial;
  }

  GatherMode gather_mode = predicate.gather_mode();

  switch (uint32_t(n)) {
    case 1: {
      BL_ASSERT(predicate.is_empty());

      Gp idx;

      // Pattern AA or Fx/Fy
      // -------------------

      if (has_frac_x()) {
        advance_x_by_one();
      }

      if (extend_x() == ExtendMode::kPad) {
        idx = f->x_padded;
      }
      else if (extend_x() == ExtendMode::kRepeat) {
        idx = f->x;
      }
      else if (extend_x() == ExtendMode::kRoR) {
        idx = pc->new_gpz("@idx");
        pc->reflect(idx.r32(), f->x);
      }

      if (is_pattern_aligned()) {
        FetchUtils::fetch_pixel(pc, p, flags, fetch_info(), mem_ptr(f->srcp1, idx, _idx_shift));
        advance_x_by_one();
      }
      else if (is_pattern_fy()) {
        if (is_alpha_fetch()) {
          Vec pixA = pc->new_vec128("@pixA");

          FetchUtils::x_fetch_unpacked_a8_2x(pc, pixA, fetch_info(), mem_ptr(f->srcp1, idx, _idx_shift), mem_ptr(f->srcp0, idx, _idx_shift));
          pc->v_mhadd_i16_to_i32(pixA, pixA, f->wd_wb);
          pc->v_srli_u16(pixA, pixA, 8);

          advance_x_by_one();

          FetchUtils::x_assign_unpacked_alpha_values(pc, p, flags, pixA);
          FetchUtils::satisfy_pixels(pc, p, flags);
        }
        else if (p.isRGBA32()) {
          Vec pix0 = pc->new_vec128("@pix0");
          Vec pix1 = pc->new_vec128("@pix1");

          pc->v_loadu32(pix0, mem_ptr(f->srcp0, idx, _idx_shift));
          pc->v_loadu32(pix1, mem_ptr(f->srcp1, idx, _idx_shift));

          pc->v_cvt_u8_lo_to_u16(pix0, pix0);
          pc->v_cvt_u8_lo_to_u16(pix1, pix1);

          pc->v_mul_u16(pix0, pix0, f->wb);
          pc->v_mul_u16(pix1, pix1, f->wd);

          advance_x_by_one();

          pc->v_add_u16(pix0, pix0, pix1);
          pc->v_srli_u16(pix0, pix0, 8);

          p.uc.init(pix0);
          FetchUtils::satisfy_pixels(pc, p, flags);
        }
      }
      else if (is_pattern_fx()) {
        if (is_alpha_fetch()) {
          Vec pix_l = f->pix_l;
          Vec pixA = pc->new_vec128("@pixA");

          pc->x_insert_word_or_byte(pix_l, mem_ptr(f->srcp1, idx, _idx_shift), 1);
          pc->v_mhadd_i16_to_i32(pixA, pix_l, f->wc_wd);
          pc->v_srli_u32(pix_l, pix_l, 16);
          pc->v_srli_u16(pixA, pixA, 8);

          FetchUtils::x_assign_unpacked_alpha_values(pc, p, flags, pixA);
          FetchUtils::satisfy_pixels(pc, p, flags);
        }
        else if (p.isRGBA32()) {
          Vec pix_l = f->pix_l;
          Vec pix0 = pc->new_vec128("@pix0");
          Vec pix1 = pc->new_vec128("@pix1");

          pc->v_insert_u32(pix_l, mem_ptr(f->srcp1, idx, _idx_shift), 1);
          pc->v_cvt_u8_lo_to_u16(pix0, pix_l);
          pc->v_mul_u16(pix0, pix0, f->wc_wd);
          pc->v_swizzle_u32x4(pix_l, pix_l, swizzle(1, 1, 1, 1));
          pc->v_swap_u64(pix1, pix0);

          pc->v_add_u16(pix0, pix0, pix1);
          pc->v_srli_u16(pix0, pix0, 8);

          p.uc.init(pix0);
          FetchUtils::satisfy_pixels(pc, p, flags);
        }
      }
      else if (is_pattern_fx_fy()) {
        if (is_alpha_fetch()) {
          Vec pix_l = f->pix_l;
          Vec pixA = pc->new_vec128("@pixA");
          Vec pixB = pc->new_vec128("@pixB");

          pc->v_load_u8_u16_2x(pixB, mem_ptr(f->srcp0, idx, _idx_shift), mem_ptr(f->srcp1, idx, _idx_shift));
          pc->v_mhadd_i16_to_i32(pixA, pix_l, f->wa_wc);
          pc->v_mov(pix_l, pixB);
          pc->v_mhadd_i16_to_i32(pixB, pixB, f->wb_wd);
          pc->v_add_i32(pixA, pixA, pixB);
          pc->v_srli_u16(pixA, pixA, 8);

          FetchUtils::x_assign_unpacked_alpha_values(pc, p, flags, pixA);
          FetchUtils::satisfy_pixels(pc, p, flags);
        }
        else if (p.isRGBA32()) {
          Vec pix_l = f->pix_l;
          Vec pix0 = pc->new_vec128("@pix0");
          Vec pix1 = pc->new_vec128("@pix1");

          pc->v_cvt_u8_lo_to_u16(pix0, pix_l);
          pc->v_loadu32(pix_l, mem_ptr(f->srcp1, idx, _idx_shift));
          FetchUtils::fetch_second_32bit_element(pc, pix_l, mem_ptr(f->srcp0, idx, _idx_shift));
          pc->v_cvt_u8_lo_to_u16(pix1, pix_l);

          pc->v_mul_u16(pix0, pix0, f->wa_wc);
          pc->v_mul_u16(pix1, pix1, f->wb_wd);
          pc->v_add_u16(pix0, pix0, pix1);
          pc->v_swap_u64(pix1, pix0);
          pc->v_add_u16(pix0, pix0, pix1);
          pc->v_srli_u16(pix0, pix0, 8);

          p.uc.init(pix0);
          FetchUtils::satisfy_pixels(pc, p, flags);
        }
      }
      break;
    }

    case 4: {
      PixelType intermediate_type = is_alpha_fetch() ? PixelType::kA8 : PixelType::kRGBA32;
      PixelFlags intermediate_flags = is_alpha_fetch() ? PixelFlags::kUA : PixelFlags::kUC;

      // Horizontal Pad | RoR
      // --------------------

      if (extend_x() == ExtendMode::kPad || extend_x() == ExtendMode::kRoR) {
        FetchPadRoRContext pCtx(this, predicate);
        pCtx.begin();

        // Horizontal Pad | RoR - Aligned
        // ------------------------------

        if (is_pattern_aligned()) {
          FetchUtils::FetchContext fCtx(pc, &p, PixelCount(4), flags, fetch_info(), gather_mode);

          fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
          fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
          fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));

          if (predicate.is_empty()) {
            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
          }

          pCtx.end();
          fCtx.end();

          FetchUtils::satisfy_pixels(pc, p, flags);
        }

        // Horizontal Pad | RoR - Fy
        // -------------------------

        if (is_pattern_fy()) {
          Gp idx;

          if (is_alpha_fetch()) {
            Pixel fPix("fPix", intermediate_type);
            FetchUtils::FetchContext fCtx(pc, &fPix, PixelCount(8), intermediate_flags, fetch_info(), GatherMode::kFetchAll);

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            fCtx.end();
            pCtx.end();

            Vec& pix0 = fPix.ua[0];

            pc->v_mhadd_i16_to_i32(pix0, pix0, f->wd_wb);
            pc->v_srli_u16(pix0, pix0, 8);

            pc->v_packs_i32_i16(pix0, pix0, pix0);
            FetchUtils::x_assign_unpacked_alpha_values(pc, p, flags, pix0);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
          else if (p.isRGBA32()) {
            Pixel pix0("pix0", intermediate_type);
            Pixel pix1("pix1", intermediate_type);

            FetchUtils::FetchContext a_ctx(pc, &pix0, PixelCount(4), intermediate_flags, fetch_info(), gather_mode);
            FetchUtils::FetchContext b_ctx(pc, &pix1, PixelCount(4), intermediate_flags, fetch_info(), gather_mode);

            idx = pCtx.next_index();
            a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            if (predicate.is_empty()) {
              idx = pCtx.next_index();
              a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
              b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));
            }

            a_ctx.end();
            b_ctx.end();
            pCtx.end();

            pc->v_mul_u16(pix0.uc, pix0.uc, f->wb);
            pc->v_mul_u16(pix1.uc, pix1.uc, f->wd);

            pc->v_add_u16(pix0.uc, pix0.uc, pix1.uc);
            pc->v_srli_u16(pix0.uc, pix0.uc, 8);

            p.uc.init(pix0.uc[0], pix0.uc[1]);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
        }

        // Horizontal Pad | RoR - Fx
        // -------------------------

        if (is_pattern_fx()) {
          if (is_alpha_fetch()) {
            Pixel fPix("fPix", intermediate_type);
            FetchUtils::FetchContext fCtx(pc, &fPix, PixelCount(4), intermediate_flags, fetch_info(), GatherMode::kFetchAll);

            Vec& pixA = fPix.ua[0];
            Vec& pix_l = f->pix_l;

            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));

            fCtx.end();
            pCtx.end();

            pc->v_interleave_lo_u16(pixA, pixA, pixA);
            pc->v_sllb_u128(pixA, pixA, 2);

            pc->v_or_i32(pix_l, pix_l, pixA);
            pc->v_mhadd_i16_to_i32(pixA, pix_l, f->wc_wd);

            pc->v_srlb_u128(pix_l, pix_l, 14);
            pc->v_srli_u32(pixA, pixA, 8);
            pc->v_packs_i32_i16(pixA, pixA, pixA);

            FetchUtils::x_assign_unpacked_alpha_values(pc, p, flags, pixA);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
          else if (p.isRGBA32()) {
            Pixel fPix("fPix", intermediate_type);
            FetchUtils::FetchContext fCtx(pc, &fPix, PixelCount(4), PixelFlags::kPC, fetch_info(), GatherMode::kFetchAll);

            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, pCtx.next_index(), _idx_shift));

            fCtx.end();
            pCtx.end();

            Vec pix_l = f->pix_l;
            Vec pix0 = pc->new_vec128("@pix0");
            Vec pix1 = pc->new_vec128("@pix1");
            Vec pix2 = fPix.pc[0];
            Vec pix3 = pc->new_vec128("@pix3");

            pc->v_alignr_u128(pix0, pix2, pix_l, 12);
            pc->v_swizzle_u32x4(pix_l, pix2, swizzle(3, 3, 3, 3));

            pc->v_cvt_u8_hi_to_u16(pix1, pix0);
            pc->v_mul_u16(pix1, pix1, f->wc);

            pc->v_cvt_u8_lo_to_u16(pix0, pix0);
            pc->v_mul_u16(pix0, pix0, f->wc);

            pc->v_cvt_u8_hi_to_u16(pix3, pix2);
            pc->v_madd_u16(pix1, pix3, f->wd, pix1);

            pc->v_cvt_u8_lo_to_u16(pix2, pix2);
            pc->v_madd_u16(pix0, pix2, f->wd, pix0);

            pc->v_srli_u16(pix1, pix1, 8);
            pc->v_srli_u16(pix0, pix0, 8);

            p.uc.init(pix0, pix1);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
        }

        // Horizontal Pad | RoR - FxFy
        // ---------------------------

        if (is_pattern_fx_fy()) {
          Gp idx;

          if (is_alpha_fetch()) {
            Pixel fPix("fPix", intermediate_type);
            FetchUtils::FetchContext fCtx(pc, &fPix, PixelCount(8), intermediate_flags, fetch_info(), GatherMode::kFetchAll);

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            fCtx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            fCtx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            fCtx.end();
            pCtx.end();

            Vec pix_l = f->pix_l;
            Vec pixA = fPix.ua[0];
            Vec pixB = pc->new_vec128("pixB");

            pc->v_sllb_u128(pixB, pixA, 4);
            pc->v_or_i32(pixB, pixB, pix_l);
            pc->v_srlb_u128(pix_l, pixA, 12);

            pc->v_mhadd_i16_to_i32(pixA, pixA, f->wb_wd);
            pc->v_mhadd_i16_to_i32(pixB, pixB, f->wa_wc);

            pc->v_add_i32(pixA, pixA, pixB);
            pc->v_srli_u32(pixA, pixA, 8);
            pc->v_packs_i32_i16(pixA, pixA, pixA);

            FetchUtils::x_assign_unpacked_alpha_values(pc, p, flags, pixA);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
          else if (p.isRGBA32()) {
            Pixel a_pix("a_pix", intermediate_type);
            Pixel b_pix("b_pix", intermediate_type);

            FetchUtils::FetchContext a_ctx(pc, &a_pix, PixelCount(4), PixelFlags::kPC, fetch_info(), GatherMode::kFetchAll);
            FetchUtils::FetchContext b_ctx(pc, &b_pix, PixelCount(4), PixelFlags::kPC, fetch_info(), GatherMode::kFetchAll);

            idx = pCtx.next_index();
            a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            idx = pCtx.next_index();
            a_ctx.fetch_pixel(mem_ptr(f->srcp0, idx, _idx_shift));
            b_ctx.fetch_pixel(mem_ptr(f->srcp1, idx, _idx_shift));

            a_ctx.end();
            b_ctx.end();
            pCtx.end();

            Vec pix_l = f->pix_l;
            Vec pix0 = pc->new_vec128("@pix0");
            Vec pix1 = pc->new_vec128("@pix1");
            Vec pix2 = pc->new_vec128("@pix2");
            Vec pix3 = pc->new_vec128("@pix3");

            Vec pixP = a_pix.pc[0];
            Vec pixQ = b_pix.pc[0];

            pc->v_cvt_u8_lo_to_u16(pix0, pixP);
            pc->v_mul_u16(pix0, pix0, f->wb);
            pc->v_cvt_u8_hi_to_u16(pix1, pixP);
            pc->v_mul_u16(pix1, pix1, f->wb);

            pc->v_cvt_u8_lo_to_u16(pix2, pixQ);
            pc->v_mul_u16(pix2, pix2, f->wd);
            pc->v_cvt_u8_hi_to_u16(pix3, pixQ);
            pc->v_mul_u16(pix3, pix3, f->wd);

            pc->v_add_u16(pix0, pix0, pix2);
            pc->v_swizzle_u32x4(pix2, f->pix_l, swizzle(1, 0, 1, 0));
            pc->v_add_u16(pix1, pix1, pix3);

            pc->v_interleave_shuffle_u32x4(pix_l, pixQ, pixP, swizzle(3, 3, 3, 3));
            pc->v_alignr_u128(pixP, pixP, pix2, 12);
            pc->v_swizzle_u32x4(pix2, pix2, swizzle(2, 2, 2, 2));

            pc->shift_or_rotate_right(pix_l, pix_l, 4);
            pc->v_alignr_u128(pixQ, pixQ, pix2, 12);

            pc->v_cvt_u8_lo_to_u16(pix2, pixP);
            pc->v_mul_u16(pix2, pix2, f->wa);
            pc->v_cvt_u8_lo_to_u16(pix3, pixQ);
            pc->v_mul_u16(pix3, pix3, f->wc);

            pc->v_cvt_u8_hi_to_u16(pixP, pixP);
            pc->v_mul_u16(pixP, pixP, f->wa);
            pc->v_add_u16(pix2, pix2, pix3);

            pc->v_cvt_u8_hi_to_u16(pixQ, pixQ);
            pc->v_mul_u16(pixQ, pixQ, f->wc);
            pc->v_add_u16(pixP, pixP, pixQ);

            pc->v_add_u16(pix0, pix0, pix2);
            pc->v_add_u16(pix1, pix1, pixP);

            pc->v_srli_u16(pix0, pix0, 8);
            pc->v_srli_u16(pix1, pix1, 8);

            p.uc.init(pix0, pix1);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
        }
      }

      // Horizontal Repeat - AA-Only (Large Fills)
      // -----------------------------------------

      if (extend_x() == ExtendMode::kRepeat) {
        // Only generated for AA patterns.
        BL_ASSERT(is_pattern_aligned());

        PixelFlags overridden_flags = flags;
        if (pc->use_256bit_simd() && p.isRGBA32()) {
          overridden_flags = PixelFlags::kPC;
        }

        FetchUtils::FetchContext fCtx(pc, &p, PixelCount(4), overridden_flags, fetch_info(), gather_mode);
        Gp x = f->x;

        if (predicate.is_empty()) {
          Label L_Done = pc->new_label();
          Label L_Repeat = pc->new_label();

          int offset = int(4 * bpp());

#if defined(BL_JIT_ARCH_X86)
          // This forms a pointer that takes advantage of X86 addressing [base + index + offset].
          // What we want to do in the fast case is to just read [base + x - offset], because we have
          // just incremented the offset, so we want to read the pointer `srcp1 + x` pointer before x
          // was incremented.
          Mem mem = mem_ptr(f->srcp1, x, 0, -offset);
#else
          // AArch64 addressing is more restricted than x86 one, so we can form either a [base + index]
          // or [base + offset] address.
          Gp src_base = pc->new_similar_reg(f->srcp1, "src_base");
          pc->sub(src_base, f->srcp1, offset);
          Mem mem = mem_ptr(src_base, x);
#endif
          pc->j(L_Repeat, add_c(x, offset));

          // TODO: [JIT] This should use FetchUtils::fetch_pixels() instead - it's identical.
          //
          // The problem here is only that we want the same registers where the pixels are fetched, where
          // pixels are allocated by FetchUtils::FetchContext. However, if we tweak fetch_pixels() and add
          // a parameter to reuse the existing vector registers (or simply fetch to existing ones, if
          // provided) then this code could be removed.
          if (p.isRGBA32()) {
            if (bl_test_flag(overridden_flags, PixelFlags::kPC)) {
              const Vec& reg = p.pc[0];

              switch (format()) {
                case FormatExt::kPRGB32:
                case FormatExt::kXRGB32: {
                  pc->v_loadu128(reg, mem);
                  break;
                }

                case FormatExt::kA8: {
                  pc->v_loadu32(reg, mem);
                  pc->v_interleave_lo_u8(reg, reg, reg);
                  pc->v_interleave_lo_u16(reg, reg, reg);
                  break;
                }

                default:
                  BL_NOT_REACHED();
              }
            }
            else {
              const Vec& uc0 = p.uc[0];
              const Vec& uc1 = p.uc[1];

              switch (format()) {
                case FormatExt::kPRGB32:
                case FormatExt::kXRGB32: {
                  pc->v_cvt_u8_lo_to_u16(uc0, mem);
                  pc->v_cvt_u8_lo_to_u16(uc1, mem.clone_adjusted(8));
                  break;
                }

                case FormatExt::kA8: {
                  pc->v_loadu32(uc0, mem);
                  pc->v_interleave_lo_u8(uc0, uc0, uc0);
                  pc->v_cvt_u8_lo_to_u16(uc0, uc0);
                  pc->v_swizzle_u32x4(uc1, uc0, swizzle(3, 3, 2, 2));
                  pc->v_swizzle_u32x4(uc0, uc0, swizzle(1, 1, 0, 0));
                  break;
                }

                default:
                  BL_NOT_REACHED();
              }
            }
          }
          else {
            if (bl_test_flag(overridden_flags, PixelFlags::kPA)) {
              const Vec& reg = p.pa[0];
              switch (format()) {
                case FormatExt::kPRGB32:
                case FormatExt::kXRGB32: {
                  pc->v_loadu128(reg, mem);

  #if defined(BL_JIT_ARCH_X86)
                  if (!pc->has_ssse3()) {
                    pc->v_srli_u32(reg, reg, 24);
                    pc->v_packs_i32_i16(reg, reg, reg);
                    pc->v_packs_i16_u8(reg, reg, reg);
                  }
                  else
  #endif // BL_JIT_ARCH_X86
                  {
                    pc->v_swizzlev_u8(reg, reg, pc->simd_const(&ct.swizu8_3xxx2xxx1xxx0xxx_to_zzzzzzzzzzzz3210, Bcst::kNA, reg));
                  }
                  break;
                }

                case FormatExt::kA8: {
                  pc->v_loadu32(reg, mem);
                  break;
                }

                default:
                  BL_NOT_REACHED();
              }
            }
            else {
              const Vec& reg = p.ua[0];
              switch (format()) {
                case FormatExt::kPRGB32:
                case FormatExt::kXRGB32: {
                  pc->v_loadu128(reg, mem);
                  pc->v_srli_u32(reg, reg, 24);
                  pc->v_packs_i32_i16(reg, reg, reg);
                  break;
                }

                case FormatExt::kA8: {
                  pc->v_loadu32(reg, mem);
                  pc->v_cvt_u8_lo_to_u16(reg, reg);
                  break;
                }

                default:
                  BL_NOT_REACHED();
              }
            }
          }

          pc->bind(L_Done);

          {
            PipeInjectAtTheEnd injected(pc);
            pc->bind(L_Repeat);

            fCtx.fetch_pixel(mem);

#if defined(BL_JIT_ARCH_X86)
            mem.add_offset_lo32(offset);
#else
            mem = mem_ptr(f->srcp1, x);
#endif

            pc->cmov(x, f->x_restart, sub_z(x, offset - int(bpp())));
            fCtx.fetch_pixel(mem);

            pc->cmov(x, f->x_restart, add_z(x, bpp()));
            fCtx.fetch_pixel(mem);

            pc->cmov(x, f->x_restart, add_z(x, bpp()));
            fCtx.fetch_pixel(mem);

            pc->cmov(x, f->x_restart, add_z(x, bpp()));
            fCtx.end();

            pc->j(L_Done);
          }
        }
        else {
          uint32_t kMsk = ((bpp()        ) << 16) | // `predicate.count == 2` => always fetch 1, then 1 next.
                          ((bpp() * 0x11u) << 24) ; // `predicate.count == 3` => always fetch 1, then 2 next.

          Gp t0 = pc->new_gpz("@t0");
          Gp t1 = pc->new_gpz("@t1");

          pc->mov(t0.r32(), kMsk);
          pc->shl(t1.r32(), predicate.count().r32(), 3);
          pc->shr(t0.r32(), t0.r32(), t1.r32());

          Mem mem = mem_ptr(f->srcp1, x);
          fCtx.fetch_pixel(mem);
          pc->mov(t1.r32(), 0x0F);
          pc->cmov(x, f->x_restart, add_z(x, bpp()));
          pc->and_(t1.r32(), t1.r32(), t0.r32());

          fCtx.fetch_pixel(mem);
          pc->shr(t0.r32(), t0.r32(), 4);
          pc->cmov(x, f->x_restart, add_z(x, t1));
          pc->and_(t1.r32(), t1.r32(), t0.r32());

          fCtx.fetch_pixel(mem);
          pc->cmov(x, f->x_restart, add_z(x, t1));

          fCtx.end();
        }

        FetchUtils::satisfy_pixels(pc, p, flags);
      }
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::JIT::FetchAffinePatternPart - Construction & Destruction
// ======================================================================

FetchAffinePatternPart::FetchAffinePatternPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchPatternPart(pc, fetch_type, format) {

  _part_flags |= PipePartFlags::kAdvanceXNeedsDiff;
  _max_pixels = 4;

  switch (fetch_type) {
    case FetchType::kPatternAffineNNAny:
    case FetchType::kPatternAffineNNOpt:
      add_part_flags(PipePartFlags::kExpensive);
      break;

    case FetchType::kPatternAffineBIAny:
    case FetchType::kPatternAffineBIOpt:
      // TODO: [JIT] OPTIMIZATION: Implement fetch4.
      _max_pixels = 1;
      add_part_flags(PipePartFlags::kExpensive);
      break;

    default:
      BL_NOT_REACHED();
  }

  OpUtils::reset_var_struct(&f, sizeof(f));

  if (IntOps::is_power_of_2(_bpp))
    _idx_shift = uint8_t(IntOps::ctz(_bpp));
}

// bl::Pipeline::JIT::FetchAffinePatternPart - Init & Fini
// =======================================================

void FetchAffinePatternPart::_init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  f->srctop         = pc->new_gpz("f.srctop");        // Mem.
  f->stride         = pc->new_gpz("f.stride");        // Mem.

  f->xx_xy          = pc->new_vec128("f.xx_xy");          // Reg.
  f->yx_yy          = pc->new_vec128("f.yx_yy");          // Reg/Mem.
  f->tx_ty          = pc->new_vec128("f.tx_ty");          // Reg/Mem.
  f->px_py          = pc->new_vec128("f.px_py");          // Reg.
  f->ox_oy          = pc->new_vec128("f.ox_oy");          // Reg/Mem.
  f->rx_ry          = pc->new_vec128("f.rx_ry");          // Reg/Mem.
  f->qx_qy          = pc->new_vec128("f.qx_qy");          // Reg     [fetch4].
  f->xx2_xy2        = pc->new_vec128("f.xx2_xy2");        // Reg/Mem [fetch4].
  f->minx_miny      = pc->new_vec128("f.minx_miny");      // Reg/Mem.
  f->maxx_maxy      = pc->new_vec128("f.maxx_maxy");      // Reg/Mem.
  f->corx_cory      = pc->new_vec128("f.corx_cory");      // Reg/Mem.
  f->tw_th          = pc->new_vec128("f.tw_th");          // Reg/Mem.

  f->v_idx           = pc->new_vec128("f.v_idx");           // Reg/Tmp.
  f->vAddrMul       = pc->new_vec128("f.vAddrMul");       // Reg/Tmp.
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  pc->load(f->srctop, mem_ptr(fn.fetch_data(), REL_PATTERN(src.pixel_data)));
  pc->load(f->stride, mem_ptr(fn.fetch_data(), REL_PATTERN(src.stride)));

#if defined(BL_JIT_ARCH_A64)
  // Apply alpha offset to source pointers when on AArch64 as we cannot use offsets together with indexes.
  if (_alpha_fetch) {
    _fetch_info.apply_alpha_offset();
    if (_fetch_info.applied_offset()) {
      pc->add(f->srctop, f->srctop, _fetch_info.applied_offset());
    }
  }
#endif // BL_JIT_ARCH_A64

  pc->v_loadu128(f->xx_xy, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.xx)));
  pc->v_loadu128(f->yx_yy, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.yx)));

  pc->s_mov_u32(f->tx_ty, y);
  pc->v_swizzle_u32x4(f->tx_ty, f->tx_ty, swizzle(1, 0, 1, 0));
  pc->v_mul_u64_lo_u32(f->tx_ty, f->yx_yy, f->tx_ty);
  pc->v_add_i64(f->tx_ty, f->tx_ty, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.tx)));

  // RoR: `tw_th` and `rx_ry` are only used by repeated or reflected patterns.
  pc->v_loadu128(f->rx_ry, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.rx)));
  pc->v_loadu128(f->tw_th, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.tw)));

  pc->v_loadu128(f->ox_oy, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.ox)));
  pc->v_loadu128(f->xx2_xy2, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.xx2)));

  // Pad: [MaxY | MaxX | MinY | MinX]
  pc->v_loadu128(f->minx_miny, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.min_x)));
  pc->v_loadu64(f->corx_cory, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.cor_x)));

  if (is_optimized()) {
    pc->v_packs_i32_i16(f->minx_miny, f->minx_miny, f->minx_miny);        // [MaxY|MaxX|MinY|MinX|MaxY|MaxX|MinY|MinX]
    pc->v_swizzle_u32x4(f->maxx_maxy, f->minx_miny, swizzle(1, 1, 1, 1)); // [MaxY|MaxX|MaxY|MaxX|MaxY|MaxX|MaxY|MaxX]
    pc->v_swizzle_u32x4(f->minx_miny, f->minx_miny, swizzle(0, 0, 0, 0)); // [MinY|MinX|MinY|MinX|MinY|MinX|MinY|MinX]
  }
  else if (fetch_type() == FetchType::kPatternAffineNNAny) {
    // NOTE: This is a slightly different layout than others to match [V]PMADDWD instruction on X86.
    pc->v_swizzle_u32x4(f->maxx_maxy, f->minx_miny, swizzle(3, 2, 3, 2)); // [MaxY|MaxX|MaxY|MaxX]
    pc->v_swizzle_u32x4(f->minx_miny, f->minx_miny, swizzle(1, 0, 1, 0)); // [MinY|MinX|MinY|MinX]
    pc->v_swizzle_u32x4(f->corx_cory, f->corx_cory, swizzle(1, 0, 1, 0)); // [CorY|CorX|CorY|CorX]
  }
  else {
    pc->v_swizzle_u32x4(f->maxx_maxy, f->minx_miny, swizzle(3, 3, 2, 2)); // [MaxY|MaxY|MaxX|MaxX]
    pc->v_swizzle_u32x4(f->minx_miny, f->minx_miny, swizzle(1, 1, 0, 0)); // [MinY|MinY|MinX|MinX]
    pc->v_swizzle_u32x4(f->corx_cory, f->corx_cory, swizzle(1, 1, 0, 0)); // [CorY|CorY|CorX|CorX]
  }

  if (is_optimized())
    pc->v_broadcast_u32(f->vAddrMul, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.addr_mul16)));
  else
    pc->v_broadcast_u64(f->vAddrMul, mem_ptr(fn.fetch_data(), REL_PATTERN(affine.addr_mul32)));

  if (is_rect_fill()) {
    advance_px_py(f->tx_ty, x);
    normalize_px_py(f->tx_ty);
  }
}

void FetchAffinePatternPart::_fini_part() noexcept {}

// bl::Pipeline::JIT::FetchAffinePatternPart - Advance
// ===================================================

void FetchAffinePatternPart::advance_y() noexcept {
  pc->v_add_i64(f->tx_ty, f->tx_ty, f->yx_yy);

  if (is_rect_fill())
    normalize_px_py(f->tx_ty);
}

void FetchAffinePatternPart::start_at_x(const Gp& x) noexcept {
  if (is_rect_fill()) {
    pc->v_mov(f->px_py, f->tx_ty);
  }
  else {
    // Similar to `advance_px_py()`, however, we don't need a temporary here...
    pc->s_mov_u32(f->px_py, x.r32());
    pc->v_swizzle_u32x4(f->px_py, f->px_py, swizzle(1, 0, 1, 0));
    pc->v_mul_u64_lo_u32(f->px_py, f->xx_xy, f->px_py);
    pc->v_add_i64(f->px_py, f->px_py, f->tx_ty);

    normalize_px_py(f->px_py);
  }

  if (pixel_granularity() > 1)
    enter_n();
}

void FetchAffinePatternPart::advance_x(const Gp& x, const Gp& diff) noexcept {
  bl_unused(x);
  BL_ASSERT(!is_rect_fill());

  if (pixel_granularity() > 1)
    leave_n();

  advance_px_py(f->px_py, diff);
  normalize_px_py(f->px_py);

  if (pixel_granularity() > 1)
    enter_n();
}

void FetchAffinePatternPart::advance_px_py(Vec& px_py, const Gp& i) noexcept {
  Vec t = pc->new_vec128("@t");

  pc->s_mov_u32(t, i.r32());
  pc->v_swizzle_u32x4(t, t, swizzle(1, 0, 1, 0));
  pc->v_mul_u64_lo_u32(t, f->xx_xy, t);
  pc->v_add_i64(px_py, px_py, t);
}

void FetchAffinePatternPart::normalize_px_py(Vec& px_py) noexcept {
  Vec v0 = pc->new_vec128("v0");

  pc->v_zero_i(v0);
  pc->xModI64HIxDouble(px_py, px_py, f->tw_th);
  pc->v_cmp_gt_i32(v0, v0, px_py);
  pc->v_and_i32(v0, v0, f->rx_ry);
  pc->v_add_i32(px_py, px_py, v0);

  pc->v_cmp_gt_i32(v0, px_py, f->ox_oy);
  pc->v_and_i32(v0, v0, f->rx_ry);
  pc->v_sub_i32(px_py, px_py, v0);
}

void FetchAffinePatternPart::clamp_vec_idx_32(Vec& dst, const Vec& src, ClampStep step) noexcept {
  switch (step) {
    // Step A - Handle a possible underflow (PAD).
    //
    // We know that `minx_miny` can contain these values (per vector element):
    //
    //   a) `minx_miny == 0`         to handle PAD case.
    //   b) `minx_miny == INT32_MIN` to handle REPEAT & REFLECT cases.
    //
    // This means that we either clamp to zero if `src` is negative and `minx_miny == 0`
    // or we don't clamp at all in case that `minx_miny == INT32_MIN`. This means that
    // we don't need a pure `PMAXSD` replacement in pure SSE2 mode, just something that
    // works for the mentioned cases.
    case kClampStepA_NN:
    case kClampStepA_BI: {
#if defined(BL_JIT_ARCH_X86)
      if (!pc->has_sse4_1()) {
        if (dst.id() == src.id()) {
          Vec tmp = pc->new_vec128("f.vIdxPad");
          pc->v_mov(tmp, src);
          pc->v_cmp_gt_i32(dst, dst, f->minx_miny); // `-1` if `src` is greater than `minx_miny`.
          pc->v_and_i32(dst, dst, tmp);             // Changes `dst` to `0` if clamped.
        }
        else {
          pc->v_mov(dst, src);
          pc->v_cmp_gt_i32(dst, dst, f->minx_miny); // `-1` if `src` is greater than `minx_miny`.
          pc->v_and_i32(dst, dst, src);             // Changes `dst` to `0` if clamped.
        }
        break;
      }
#endif // BL_JIT_ARCH_X86

      pc->v_max_i32(dst, src, f->minx_miny);
      break;
    }

    // Step B - Handle a possible overflow (PAD | Bilinear overflow).
    case kClampStepB_NN:
    case kClampStepB_BI: {
      // Always performed on the same register.
      BL_ASSERT(dst.id() == src.id());

#if defined(BL_JIT_ARCH_X86)
      // TODO: [JIT] OPTIMIZATION: AVX-512 masking seems slower than AVX2.
      // if (pc->has_avx512()) {
      //   x86::KReg k = cc->new_kw("f.kTmp");
      //   cc->vpcmpgtd(k, dst, f->maxx_maxy);
      //   cc->k(k).vmovdqa32(dst, f->corx_cory);
      // }

      if (!pc->has_sse4_1()) {
        // Blend(a, b, cond) == a ^ ((a ^ b) &  cond)
        //                   == b ^ ((a ^ b) & ~cond)
        Vec tmp = pc->new_vec128("f.v_tmp");
        pc->v_xor_i32(tmp, dst, f->corx_cory);
        pc->v_cmp_gt_i32(dst, dst, f->maxx_maxy);
        pc->v_andn_i32(dst, dst, tmp);
        pc->v_xor_i32(dst, dst, f->corx_cory);
        break;
      }
#endif // BL_JIT_ARCH_X86

      Vec tmp = pc->new_vec128("f.v_tmp");
      pc->v_cmp_gt_i32(tmp, dst, f->maxx_maxy);
      pc->v_blendv_u8(dst, dst, f->corx_cory, tmp);
      break;
    }

    // Step C - Handle a possible reflection (RoR).
    case kClampStepC_NN:
    case kClampStepC_BI: {
      // Always performed on the same register.
      BL_ASSERT(dst.id() == src.id());

      Vec tmp = pc->new_vec128("f.vIdxRoR");
      pc->v_srai_i32(tmp, dst, 31);
      pc->v_xor_i32(dst, dst, tmp);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::JIT::FetchAffinePatternPart - Fetch
// =================================================

void FetchAffinePatternPart::enter_n() noexcept {
  Vec vMsk0 = pc->new_vec128("vMsk0");

  pc->v_add_i64(f->qx_qy, f->px_py, f->xx_xy);
  pc->v_cmp_gt_i32(vMsk0, f->qx_qy, f->ox_oy);
  pc->v_and_i32(vMsk0, vMsk0, f->rx_ry);
  pc->v_sub_i32(f->qx_qy, f->qx_qy, vMsk0);
}

void FetchAffinePatternPart::leave_n() noexcept {}

void FetchAffinePatternPart::prefetch_n() noexcept {}
void FetchAffinePatternPart::postfetch_n() noexcept {}

void FetchAffinePatternPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  p.set_count(n);

  GatherMode gather_mode = predicate.gather_mode();

  switch (uint32_t(n)) {
    case 1: {
      BL_ASSERT(predicate.is_empty());

      switch (fetch_type()) {
        case FetchType::kPatternAffineNNAny: {
          Gp tex_ptr = pc->new_gpz("tex_ptr");
          Gp tex_off = pc->new_gpz("tex_off");

          Vec v_idx = f->v_idx;
          Vec v_msk = pc->new_vec128("v_msk");

          clamp_vec_idx_32(v_idx, f->px_py, kClampStepA_NN);
          clamp_vec_idx_32(v_idx, v_idx, kClampStepB_NN);
          clamp_vec_idx_32(v_idx, v_idx, kClampStepC_NN);
          pc->v_add_i64(f->px_py, f->px_py, f->xx_xy);

          FetchUtils::IndexExtractor iExt(pc);
          iExt.begin(FetchUtils::IndexExtractor::kTypeUInt32, v_idx);
          iExt.extract(tex_ptr, 3);
          iExt.extract(tex_off, 1);

          pc->v_cmp_gt_i32(v_msk, f->px_py, f->ox_oy);
          pc->mul(tex_ptr, tex_ptr, f->stride);
          pc->v_and_i32(v_msk, v_msk, f->rx_ry);
          pc->v_sub_i32(f->px_py, f->px_py, v_msk);
          pc->add(tex_ptr, tex_ptr, f->srctop);

          FetchUtils::fetch_pixel(pc, p, flags, fetch_info(), mem_ptr(tex_ptr, tex_off, _idx_shift));
          FetchUtils::satisfy_pixels(pc, p, flags);
          break;
        }

        case FetchType::kPatternAffineNNOpt: {
          Gp tex_ptr = pc->new_gpz("tex_ptr");
          Vec v_idx = f->v_idx;
          Vec v_msk = pc->new_vec128("v_msk");

          pc->v_swizzle_u32x4(v_idx, f->px_py, swizzle(3, 1, 3, 1));
          pc->v_packs_i32_i16(v_idx, v_idx, v_idx);
          pc->v_max_i16(v_idx, v_idx, f->minx_miny);
          pc->v_min_i16(v_idx, v_idx, f->maxx_maxy);

          pc->v_srai_i16(v_msk, v_idx, 15);
          pc->v_xor_i32(v_idx, v_idx, v_msk);

          pc->v_add_i64(f->px_py, f->px_py, f->xx_xy);
          pc->v_mhadd_i16_to_i32(v_idx, v_idx, f->vAddrMul);

          pc->v_cmp_gt_i32(v_msk, f->px_py, f->ox_oy);
          pc->v_and_i32(v_msk, v_msk, f->rx_ry);
          pc->v_sub_i32(f->px_py, f->px_py, v_msk);
          pc->s_mov_u32(tex_ptr.r32(), v_idx);
          pc->add(tex_ptr, tex_ptr, f->srctop);

          FetchUtils::fetch_pixel(pc, p, flags, fetch_info(), mem_ptr(tex_ptr));
          FetchUtils::satisfy_pixels(pc, p, flags);
          break;
        }

        case FetchType::kPatternAffineBIAny: {
          if (is_alpha_fetch()) {
            Vec v_idx = pc->new_vec128("v_idx");
            Vec v_msk = pc->new_vec128("v_msk");
            Vec v_weights = pc->new_vec128("v_weights");

            pc->v_swizzle_u32x4(v_idx, f->px_py, swizzle(3, 3, 1, 1));
            pc->v_sub_i32(v_idx, v_idx, pc->simd_const(&ct.p_FFFFFFFF00000000, Bcst::kNA, v_idx));

            pc->v_swizzle_lo_u16x4(v_weights, f->px_py, swizzle(1, 1, 1, 1));
            clamp_vec_idx_32(v_idx, v_idx, kClampStepA_BI);

            pc->v_add_i64(f->px_py, f->px_py, f->xx_xy);
            clamp_vec_idx_32(v_idx, v_idx, kClampStepB_BI);

            pc->v_cmp_gt_i32(v_msk, f->px_py, f->ox_oy);
            pc->v_swizzle_hi_u16x4(v_weights, v_weights, swizzle(1, 1, 1, 1));

            pc->v_and_i32(v_msk, v_msk, f->rx_ry);
            pc->v_srli_u16(v_weights, v_weights, 8);

            pc->v_sub_i32(f->px_py, f->px_py, v_msk);
            pc->v_xor_i32(v_weights, v_weights, pc->simd_const(&ct.p_FFFF0000FFFF0000, Bcst::k32, v_weights));

            clamp_vec_idx_32(v_idx, v_idx, kClampStepC_BI);
            pc->v_add_u16(v_weights, v_weights, pc->simd_const(&ct.p_0101000001010000, Bcst::kNA, v_weights));

            Vec pixA = pc->new_vec128("pixA");
            FetchUtils::filter_bilinear_a8_1x(pc, pixA, f->srctop, f->stride, fetch_info(), _idx_shift, v_idx, v_weights);

            FetchUtils::x_assign_unpacked_alpha_values(pc, p, flags, pixA);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
          else if (p.isRGBA32()) {
            Vec v_idx = pc->new_vec128("v_idx");
            Vec v_msk = pc->new_vec128("v_msk");
            Vec v_weights = pc->new_vec128("v_weights");

            pc->v_swizzle_u32x4(v_idx, f->px_py, swizzle(3, 3, 1, 1));
            pc->v_sub_i32(v_idx, v_idx, pc->simd_const(&ct.p_FFFFFFFF00000000, Bcst::kNA, v_idx));

#if defined(BL_JIT_ARCH_X86)
            if (!pc->has_ssse3()) {
              pc->v_swizzle_u16x4(v_weights, f->px_py, swizzle(1, 1, 1, 1));
              pc->v_srli_u16(v_weights, v_weights, 8);
            }
            else
#endif
            {
              pc->v_swizzlev_u8(v_weights, f->px_py, pc->simd_const(&ct.swizu8_xxxx1xxxxxxx0xxx_to_z1z1z1z1z0z0z0z0, Bcst::kNA, v_weights));
            }

            pc->v_add_i64(f->px_py, f->px_py, f->xx_xy);
            clamp_vec_idx_32(v_idx, v_idx, kClampStepA_BI);
            pc->v_xor_i64(v_weights, v_weights, pc->simd_const(&ct.p_FFFFFFFF00000000, Bcst::k64, v_weights));
            pc->v_cmp_gt_i32(v_msk, f->px_py, f->ox_oy);

            clamp_vec_idx_32(v_idx, v_idx, kClampStepB_BI);
            pc->v_and_i32(v_msk, v_msk, f->rx_ry);

            pc->v_add_u16(v_weights, v_weights, pc->simd_const(&ct.p_0101010100000000, Bcst::kNA, v_weights));
            pc->v_sub_i32(f->px_py, f->px_py, v_msk);
            clamp_vec_idx_32(v_idx, v_idx, kClampStepC_BI);

            p.uc.init(pc->new_vec128("pix0"));
            FetchUtils::filter_bilinear_argb32_1x(pc, p.uc[0], f->srctop, f->stride, v_idx, v_weights);
            FetchUtils::satisfy_pixels(pc, p, flags);
          }
          break;
        }

        case FetchType::kPatternAffineBIOpt: {
          // TODO: [JIT] OPTIMIZATION: Not used at the moment.
          break;
        }

        default:
          BL_NOT_REACHED();
      }

      break;
    }

    case 4: {
      switch (fetch_type()) {
        case FetchType::kPatternAffineNNAny: {
          FetchUtils::FetchContext fCtx(pc, &p, PixelCount(4), flags, fetch_info());
          FetchUtils::IndexExtractor iExt(pc);

          Gp texPtr0 = pc->new_gpz("texPtr0");
          Gp texOff0 = pc->new_gpz("texOff0");
          Gp texPtr1 = pc->new_gpz("texPtr1");
          Gp texOff1 = pc->new_gpz("texOff1");

          Vec vIdx0 = pc->new_vec128("vIdx0");
          Vec vIdx1 = pc->new_vec128("vIdx1");
          Vec vMsk0 = pc->new_vec128("vMsk0");
          Vec vMsk1 = pc->new_vec128("vMsk1");

          pc->v_interleave_shuffle_u32x4(vIdx0, f->px_py, f->qx_qy, swizzle(3, 1, 3, 1));
          pc->v_add_i64(f->px_py, f->px_py, f->xx2_xy2);

          clamp_vec_idx_32(vIdx0, vIdx0, kClampStepA_NN);
          pc->v_add_i64(f->qx_qy, f->qx_qy, f->xx2_xy2);

          clamp_vec_idx_32(vIdx0, vIdx0, kClampStepB_NN);
          pc->v_cmp_gt_i32(vMsk0, f->px_py, f->ox_oy);
          clamp_vec_idx_32(vIdx0, vIdx0, kClampStepC_NN);

          pc->v_cmp_gt_i32(vMsk1, f->qx_qy, f->ox_oy);
          pc->v_and_i32(vMsk0, vMsk0, f->rx_ry);
          pc->v_and_i32(vMsk1, vMsk1, f->rx_ry);
          pc->v_sub_i32(f->px_py, f->px_py, vMsk0);
          pc->v_sub_i32(f->qx_qy, f->qx_qy, vMsk1);

          iExt.begin(FetchUtils::IndexExtractor::kTypeUInt32, vIdx0);
          pc->v_interleave_shuffle_u32x4(vIdx1, f->px_py, f->qx_qy, swizzle(3, 1, 3, 1));
          iExt.extract(texPtr0, 1);
          iExt.extract(texOff0, 0);

          clamp_vec_idx_32(vIdx1, vIdx1, kClampStepA_NN);
          clamp_vec_idx_32(vIdx1, vIdx1, kClampStepB_NN);

          iExt.extract(texPtr1, 3);
          iExt.extract(texOff1, 2);

          pc->mul(texPtr0, texPtr0, f->stride);
          pc->mul(texPtr1, texPtr1, f->stride);

          clamp_vec_idx_32(vIdx1, vIdx1, kClampStepC_NN);
          pc->v_add_i64(f->px_py, f->px_py, f->xx2_xy2);
          pc->v_add_i64(f->qx_qy, f->qx_qy, f->xx2_xy2);

          pc->add(texPtr0, texPtr0, f->srctop);
          pc->add(texPtr1, texPtr1, f->srctop);
          iExt.begin(FetchUtils::IndexExtractor::kTypeUInt32, vIdx1);

          fCtx.fetch_pixel(mem_ptr(texPtr0, texOff0, _idx_shift));
          iExt.extract(texPtr0, 1);
          iExt.extract(texOff0, 0);

          pc->v_cmp_gt_i32(vMsk0, f->px_py, f->ox_oy);
          pc->v_cmp_gt_i32(vMsk1, f->qx_qy, f->ox_oy);

          fCtx.fetch_pixel(mem_ptr(texPtr1, texOff1, _idx_shift));
          iExt.extract(texPtr1, 3);
          iExt.extract(texOff1, 2);
          pc->mul(texPtr0, texPtr0, f->stride);

          pc->v_and_i32(vMsk0, vMsk0, f->rx_ry);
          pc->v_and_i32(vMsk1, vMsk1, f->rx_ry);

          pc->mul(texPtr1, texPtr1, f->stride);
          pc->v_sub_i32(f->px_py, f->px_py, vMsk0);

          pc->add(texPtr0, texPtr0, f->srctop);
          pc->add(texPtr1, texPtr1, f->srctop);
          fCtx.fetch_pixel(mem_ptr(texPtr0, texOff0, _idx_shift));

          pc->v_sub_i32(f->qx_qy, f->qx_qy, vMsk1);
          fCtx.fetch_pixel(mem_ptr(texPtr1, texOff1, _idx_shift));
          fCtx.end();

          FetchUtils::satisfy_pixels(pc, p, flags);
          break;
        }

        case FetchType::kPatternAffineNNOpt: {
          Vec v_idx = f->v_idx;
          Vec vMsk0 = pc->new_vec128("vMsk0");
          Vec vMsk1 = pc->new_vec128("vMsk1");

          pc->v_interleave_shuffle_u32x4(v_idx, f->px_py, f->qx_qy, swizzle(3, 1, 3, 1));
          pc->v_add_i64(f->px_py, f->px_py, f->xx2_xy2);
          pc->v_add_i64(f->qx_qy, f->qx_qy, f->xx2_xy2);

          pc->v_cmp_gt_i32(vMsk0, f->px_py, f->ox_oy);
          pc->v_cmp_gt_i32(vMsk1, f->qx_qy, f->ox_oy);

          pc->v_and_i32(vMsk0, vMsk0, f->rx_ry);
          pc->v_and_i32(vMsk1, vMsk1, f->rx_ry);

          pc->v_sub_i32(f->px_py, f->px_py, vMsk0);
          pc->v_sub_i32(f->qx_qy, f->qx_qy, vMsk1);

          pc->v_interleave_shuffle_u32x4(vMsk0, f->px_py, f->qx_qy, swizzle(3, 1, 3, 1));
          pc->v_packs_i32_i16(v_idx, v_idx, vMsk0);

          pc->v_max_i16(v_idx, v_idx, f->minx_miny);
          pc->v_min_i16(v_idx, v_idx, f->maxx_maxy);

          pc->v_srai_i16(vMsk0, v_idx, 15);
          pc->v_xor_i32(v_idx, v_idx, vMsk0);

          pc->v_mhadd_i16_to_i32(v_idx, v_idx, f->vAddrMul);
          FetchUtils::gather_pixels(pc, p, PixelCount(4), flags, fetch_info(), mem_ptr(f->srctop), v_idx, 0, FetchUtils::IndexLayout::kUInt32, gather_mode, [&](uint32_t step) noexcept {
            switch (step) {
              case 0:
                pc->v_add_i64(f->px_py, f->px_py, f->xx2_xy2);
                pc->v_add_i64(f->qx_qy, f->qx_qy, f->xx2_xy2);
                break;
              case 1:
                pc->v_cmp_gt_i32(vMsk0, f->px_py, f->ox_oy);
                pc->v_cmp_gt_i32(vMsk1, f->qx_qy, f->ox_oy);
                break;
              case 2:
                pc->v_and_i32(vMsk0, vMsk0, f->rx_ry);
                pc->v_and_i32(vMsk1, vMsk1, f->rx_ry);
                break;
              case 3:
                pc->v_sub_i32(f->px_py, f->px_py, vMsk0);
                pc->v_sub_i32(f->qx_qy, f->qx_qy, vMsk1);
                break;
              default:
                break;
            }
          });

          FetchUtils::satisfy_pixels(pc, p, flags);
          break;
        }

        default:
          BL_NOT_REACHED();
      }

      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
