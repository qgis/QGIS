// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/core/runtime_p.h>
#include <blend2d/pipeline/jit/compoppart_p.h>
#include <blend2d/pipeline/jit/compoputils_p.h>
#include <blend2d/pipeline/jit/fetchpart_p.h>
#include <blend2d/pipeline/jit/fetchpatternpart_p.h>
#include <blend2d/pipeline/jit/fetchpixelptrpart_p.h>
#include <blend2d/pipeline/jit/fetchsolidpart_p.h>
#include <blend2d/pipeline/jit/fetchutilscoverage_p.h>
#include <blend2d/pipeline/jit/fetchutilsinlineloops_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::JIT::CompOpPart - Construction & Destruction
// ==========================================================

CompOpPart::CompOpPart(PipeCompiler* pc, CompOpExt comp_op, FetchPart* dst_part, FetchPart* src_part) noexcept
  : PipePart(pc, PipePartType::kComposite),
    _comp_op(comp_op),
    _pixel_type(dst_part->has_rgb() ? PixelType::kRGBA32 : PixelType::kA8),
    _coverage_format(PixelCoverageFormat::kUnpacked),
    _is_in_partial_mode(false),
    _has_da(dst_part->has_alpha()),
    _has_sa(src_part->has_alpha()),
    _solid_pre("solid", _pixel_type),
    _partial_pixel("partial", _pixel_type) {

  _mask->reset();

  // Initialize the children of this part.
  _children[kIndexDstPart] = dst_part;
  _children[kIndexSrcPart] = src_part;
  _child_count = 2;

#if defined(BL_JIT_ARCH_X86)
  VecWidth max_vec_width = VecWidth::k128;
  switch (pixel_type()) {
    case PixelType::kA8: {
      max_vec_width = VecWidth::k512;
      break;
    }

    case PixelType::kRGBA32: {
      switch (comp_op) {
        case CompOpExt::kSrcOver    :
        case CompOpExt::kSrcCopy    :
        case CompOpExt::kSrcIn      :
        case CompOpExt::kSrcOut     :
        case CompOpExt::kSrcAtop    :
        case CompOpExt::kDstOver    :
        case CompOpExt::kDstIn      :
        case CompOpExt::kDstOut     :
        case CompOpExt::kDstAtop    :
        case CompOpExt::kXor        :
        case CompOpExt::kClear      :
        case CompOpExt::kPlus       :
        case CompOpExt::kMinus      :
        case CompOpExt::kModulate   :
        case CompOpExt::kMultiply   :
        case CompOpExt::kScreen     :
        case CompOpExt::kOverlay    :
        case CompOpExt::kDarken     :
        case CompOpExt::kLighten    :
        case CompOpExt::kLinearBurn :
        case CompOpExt::kPinLight   :
        case CompOpExt::kHardLight  :
        case CompOpExt::kDifference :
        case CompOpExt::kExclusion  :
          max_vec_width = VecWidth::k512;
          break;

        case CompOpExt::kColorDodge :
        case CompOpExt::kColorBurn  :
        case CompOpExt::kLinearLight:
        case CompOpExt::kSoftLight  :
          break;

        default:
          break;
      }
      break;
    }

    default:
      BL_NOT_REACHED();
  }
  _max_vec_width_supported = max_vec_width;
#elif defined(BL_JIT_ARCH_A64)
  // TODO: [JIT] OPTIMIZATION: Every composition mode should use packed in the future (AArch64).
  if (is_src_copy() || is_src_over() || is_screen()) {
    _coverage_format = PixelCoverageFormat::kPacked;
  }
#endif
}

// bl::Pipeline::JIT::CompOpPart - Prepare
// =======================================

void CompOpPart::prepare_part() noexcept {
  bool is_solid = src_part()->is_solid();
  uint32_t max_pixels = 0;
  uint32_t pixel_limit = 64;

  _part_flags |= (dst_part()->part_flags() | src_part()->part_flags()) & PipePartFlags::kFetchFlags;

  if (src_part()->has_masked_access() && dst_part()->has_masked_access()) {
    _part_flags |= PipePartFlags::kMaskedAccess;
  }

  // Limit the maximum pixel-step to 4 it the style is not solid and the target is not 64-bit.
  // There's not enough registers to process 8 pixels in parallel in 32-bit mode.
  if (bl_runtime_is_32bit() && !is_solid && _pixel_type != PixelType::kA8) {
    pixel_limit = 4;
  }

  // Decrease the maximum pixels to 4 if the source is expensive to fetch. In such case fetching and processing more
  // pixels would result in emitting bloated pipelines that are not faster compared to pipelines working with just
  // 4 pixels at a time.
  if (dst_part()->is_expensive() || src_part()->is_expensive()) {
    pixel_limit = 4;
  }

  switch (pixel_type()) {
    case PixelType::kA8: {
      max_pixels = 8;
      break;
    }

    case PixelType::kRGBA32: {
      switch (comp_op()) {
        case CompOpExt::kSrcOver    : max_pixels = 8; break;
        case CompOpExt::kSrcCopy    : max_pixels = 8; break;
        case CompOpExt::kSrcIn      : max_pixels = 8; break;
        case CompOpExt::kSrcOut     : max_pixels = 8; break;
        case CompOpExt::kSrcAtop    : max_pixels = 8; break;
        case CompOpExt::kDstOver    : max_pixels = 8; break;
        case CompOpExt::kDstIn      : max_pixels = 8; break;
        case CompOpExt::kDstOut     : max_pixels = 8; break;
        case CompOpExt::kDstAtop    : max_pixels = 8; break;
        case CompOpExt::kXor        : max_pixels = 8; break;
        case CompOpExt::kClear      : max_pixels = 8; break;
        case CompOpExt::kPlus       : max_pixels = 8; break;
        case CompOpExt::kMinus      : max_pixels = 4; break;
        case CompOpExt::kModulate   : max_pixels = 8; break;
        case CompOpExt::kMultiply   : max_pixels = 8; break;
        case CompOpExt::kScreen     : max_pixels = 8; break;
        case CompOpExt::kOverlay    : max_pixels = 4; break;
        case CompOpExt::kDarken     : max_pixels = 8; break;
        case CompOpExt::kLighten    : max_pixels = 8; break;
        case CompOpExt::kColorDodge : max_pixels = 1; break;
        case CompOpExt::kColorBurn  : max_pixels = 1; break;
        case CompOpExt::kLinearBurn : max_pixels = 8; break;
        case CompOpExt::kLinearLight: max_pixels = 1; break;
        case CompOpExt::kPinLight   : max_pixels = 4; break;
        case CompOpExt::kHardLight  : max_pixels = 4; break;
        case CompOpExt::kSoftLight  : max_pixels = 1; break;
        case CompOpExt::kDifference : max_pixels = 4; break;
        case CompOpExt::kExclusion  : max_pixels = 4; break;

        default:
          BL_NOT_REACHED();
      }
      break;
    }

    default:
      BL_NOT_REACHED();
  }

  if (max_pixels > 1) {
    max_pixels *= pc->vec_multiplier();
    pixel_limit *= pc->vec_multiplier();
  }

  // Decrease to N pixels at a time if the fetch part doesn't support more.
  // This is suboptimal, but can happen if the fetch part is not optimized.
  max_pixels = bl_min(max_pixels, pixel_limit, src_part()->max_pixels());

  if (is_rgba32_pixel()) {
    if (max_pixels >= 4) {
      _min_alignment = Alignment(16);
    }
  }

  set_max_pixels(max_pixels);
}

// bl::Pipeline::JIT::CompOpPart - Init & Fini
// ===========================================

void CompOpPart::init(const PipeFunction& fn, Gp& x, Gp& y, uint32_t pixel_granularity) noexcept {
  _pixel_granularity = PixelCount(pixel_granularity);

  dst_part()->init(fn, x, y, pixel_type(), pixel_granularity);
  src_part()->init(fn, x, y, pixel_type(), pixel_granularity);
}

void CompOpPart::fini() noexcept {
  dst_part()->fini();
  src_part()->fini();

  _pixel_granularity = PixelCount(0);
}

// bl::Pipeline::JIT::CompOpPart - Optimization Opportunities
// ==========================================================

bool CompOpPart::should_optimize_opaque_fill() const noexcept {
  // Should be always optimized if the source is not solid.
  if (!src_part()->is_solid()) {
    return true;
  }

  // Do not optimize if the CompOp is TypeA. This operator doesn't need any
  // special handling as the source pixel is multiplied with mask before it's
  // passed to the compositor.
  if (bl_test_flag(comp_op_flags(), CompOpFlags::kTypeA)) {
    return false;
  }

  // Modulate operator just needs to multiply source with mask and add (1 - m)
  // to it.
  if (is_modulate()) {
    return false;
  }

  // We assume that in all other cases there is a benefit of using optimized
  // `c_mask` loop for a fully opaque mask.
  return true;
}

bool CompOpPart::should_just_copy_opaque_fill() const noexcept {
  if (!is_src_copy()) {
    return false;
  }

  if (src_part()->is_solid()) {
    return true;
  }

  if (src_part()->is_fetch_type(FetchType::kPatternAlignedBlit) && src_part()->format() == dst_part()->format()) {
    return true;
  }

  return false;
}

// bl::Pipeline::JIT::CompOpPart - Advance
// =======================================

void CompOpPart::start_at_x(const Gp& x) noexcept {
  dst_part()->start_at_x(x);
  src_part()->start_at_x(x);
}

void CompOpPart::advance_x(const Gp& x, const Gp& diff) noexcept {
  dst_part()->advance_x(x, diff);
  src_part()->advance_x(x, diff);
}

void CompOpPart::advance_y() noexcept {
  dst_part()->advance_y();
  src_part()->advance_y();
}

// bl::Pipeline::JIT::CompOpPart - Prefetch & Postfetch
// ====================================================

void CompOpPart::enter_n() noexcept {
  dst_part()->enter_n();
  src_part()->enter_n();
}

void CompOpPart::leave_n() noexcept {
  dst_part()->leave_n();
  src_part()->leave_n();
}

void CompOpPart::prefetch_n() noexcept {
  dst_part()->prefetch_n();
  src_part()->prefetch_n();
}

void CompOpPart::postfetch_n() noexcept {
  dst_part()->postfetch_n();
  src_part()->postfetch_n();
}

// bl::Pipeline::JIT::CompOpPart - Fetch
// =====================================

void CompOpPart::dst_fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  dst_part()->fetch(p, n, flags, predicate);
}

void CompOpPart::src_fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  // Pixels must match as we have already pre-configured the CompOpPart.
  BL_ASSERT(p.type() == pixel_type());

  if (p.count() == PixelCount(0)) {
    p.set_count(n);
  }

  // Composition with a preprocessed solid color.
  if (is_using_solid_pre()) {
    Pixel& s = _solid_pre;

    // INJECT:
    {
      ScopedInjector injector(cc, &_c_mask_loop_hook);
      FetchUtils::satisfy_solid_pixels(pc, s, flags);
    }

    if (p.isRGBA32()) {
      VecWidth pc_vec_width = pc->vec_width_of(DataWidth::k32, n);
      VecWidth uc_vec_width = pc->vec_width_of(DataWidth::k64, n);

      size_t pc_count = pc->vec_count_of(DataWidth::k32, n);
      size_t uc_count = pc->vec_count_of(DataWidth::k64, n);

      if (bl_test_flag(flags, PixelFlags::kImmutable)) {
        if (bl_test_flag(flags, PixelFlags::kPC)) {
          p.pc.init(VecWidthUtils::clone_vec_as(s.pc[0], pc_vec_width));
        }

        if (bl_test_flag(flags, PixelFlags::kUC)) {
          p.uc.init(VecWidthUtils::clone_vec_as(s.uc[0], uc_vec_width));
        }

        if (bl_test_flag(flags, PixelFlags::kUA)) {
          p.ua.init(VecWidthUtils::clone_vec_as(s.ua[0], uc_vec_width));
        }

        if (bl_test_flag(flags, PixelFlags::kUI)) {
          p.ui.init(VecWidthUtils::clone_vec_as(s.ui[0], uc_vec_width));
        }
      }
      else {
        if (bl_test_flag(flags, PixelFlags::kPC)) {
          pc->new_vec_array(p.pc, pc_count, pc_vec_width, p.name(), "pc");
          pc->v_mov(p.pc, VecWidthUtils::clone_vec_as(s.pc[0], pc_vec_width));
        }

        if (bl_test_flag(flags, PixelFlags::kUC)) {
          pc->new_vec_array(p.uc, uc_count, uc_vec_width, p.name(), "uc");
          pc->v_mov(p.uc, VecWidthUtils::clone_vec_as(s.uc[0], uc_vec_width));
        }

        if (bl_test_flag(flags, PixelFlags::kUA)) {
          pc->new_vec_array(p.ua, uc_count, uc_vec_width, p.name(), "ua");
          pc->v_mov(p.ua, VecWidthUtils::clone_vec_as(s.ua[0], uc_vec_width));
        }

        if (bl_test_flag(flags, PixelFlags::kUI)) {
          pc->new_vec_array(p.ui, uc_count, uc_vec_width, p.name(), "ui");
          pc->v_mov(p.ui, VecWidthUtils::clone_vec_as(s.ui[0], uc_vec_width));
        }
      }
    }
    else if (p.isA8()) {
      // TODO: [JIT] UNIMPLEMENTED: A8 pipepine.
      BL_ASSERT(false);
    }

    return;
  }

  // Partial mode is designed to fetch pixels on the right side of the border one by one, so it's an error
  // if the pipeline requests more than 1 pixel at a time.
  if (is_in_partial_mode()) {
    BL_ASSERT(n == PixelCount(1));

    if (p.isRGBA32()) {
      if (!bl_test_flag(flags, PixelFlags::kImmutable)) {
        if (bl_test_flag(flags, PixelFlags::kUC)) {
          pc->new_vec128_array(p.uc, 1, "uc");
          pc->v_cvt_u8_lo_to_u16(p.uc[0], _partial_pixel.pc[0]);
        }
        else {
          pc->new_vec128_array(p.pc, 1, "pc");
          pc->v_mov(p.pc[0], _partial_pixel.pc[0]);
        }
      }
      else {
        p.pc.init(_partial_pixel.pc[0]);
      }
    }
    else if (p.isA8()) {
      p.sa = pc->new_gp32("sa");
      pc->s_extract_u16(p.sa, _partial_pixel.ua[0], 0);
    }

    FetchUtils::satisfy_pixels(pc, p, flags);
    return;
  }

  src_part()->fetch(p, n, flags, predicate);
}

// bl::Pipeline::JIT::CompOpPart - PartialFetch
// ============================================

void CompOpPart::enter_partial_mode(PixelFlags partial_flags) noexcept {
  // Doesn't apply to solid fills.
  if (is_using_solid_pre()) {
    return;
  }

  // TODO: [JIT] We only support partial fetch of 4 pixels at the moment.
  BL_ASSERT(!is_in_partial_mode());
  BL_ASSERT(pixel_granularity() == PixelCount(4));

  switch (pixel_type()) {
    case PixelType::kA8: {
      src_fetch(_partial_pixel, pixel_granularity(), PixelFlags::kUA | partial_flags, pc->empty_predicate());
      break;
    }

    case PixelType::kRGBA32: {
      src_fetch(_partial_pixel, pixel_granularity(), PixelFlags::kPC | partial_flags, pc->empty_predicate());
      break;
    }

    default:
      BL_NOT_REACHED();
  }

  _is_in_partial_mode = true;
}

void CompOpPart::exit_partial_mode() noexcept {
  // Doesn't apply to solid fills.
  if (is_using_solid_pre()) {
    return;
  }

  BL_ASSERT(is_in_partial_mode());

  _is_in_partial_mode = false;
  _partial_pixel.reset_all_except_type_and_name();
}

void CompOpPart::next_partial_pixel() noexcept {
  if (!is_in_partial_mode())
    return;

  switch (pixel_type()) {
    case PixelType::kA8: {
      const Vec& pix = _partial_pixel.ua[0];
      pc->shift_or_rotate_right(pix, pix, 2);
      break;
    }

    case PixelType::kRGBA32: {
      const Vec& pix = _partial_pixel.pc[0];
      pc->shift_or_rotate_right(pix, pix, 4);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::JIT::CompOpPart - CMask - Init & Fini
// ===================================================

void CompOpPart::c_mask_init(const Mem& mem) noexcept {
  switch (pixel_type()) {
    case PixelType::kA8: {
      Gp mGp = pc->new_gp32("msk");
      pc->load_u8(mGp, mem);
      c_mask_init_a8(mGp, Vec());
      break;
    }

    case PixelType::kRGBA32: {
      Vec vm = pc->new_vec("vm");
      if (coverage_format() == PixelCoverageFormat::kPacked)
        pc->v_broadcast_u8z(vm, mem);
      else
        pc->v_broadcast_u16z(vm, mem);
      c_mask_init_rgba32(vm);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void CompOpPart::c_mask_init(const Gp& sm_, const Vec& vm_) noexcept {
  Gp sm(sm_);
  Vec vm(vm_);

  switch (pixel_type()) {
    case PixelType::kA8: {
      c_mask_init_a8(sm, vm);
      break;
    }

    case PixelType::kRGBA32: {
      if (!vm.is_valid() && sm.is_valid()) {
        vm = pc->new_vec("vm");
        if (coverage_format() == PixelCoverageFormat::kPacked)
          pc->v_broadcast_u8z(vm, sm);
        else
          pc->v_broadcast_u16z(vm, sm);
      }

      c_mask_init_rgba32(vm);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void CompOpPart::c_mask_init_opaque() noexcept {
  switch (pixel_type()) {
    case PixelType::kA8: {
      c_mask_init_a8(Gp(), Vec());
      break;
    }

    case PixelType::kRGBA32: {
      c_mask_init_rgba32(Vec());
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void CompOpPart::c_mask_fini() noexcept {
  switch (pixel_type()) {
    case PixelType::kA8: {
      c_mask_fini_a8();
      break;
    }

    case PixelType::kRGBA32: {
      c_mask_fini_rgba32();
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void CompOpPart::_c_mask_loop_init(CMaskLoopType loop_type) noexcept {
  // Make sure `_c_mask_loop_init()` and `_c_mask_loop_fini()` are used as a pair.
  BL_ASSERT(_c_mask_loop_type == CMaskLoopType::kNone);
  BL_ASSERT(_c_mask_loop_hook == nullptr);

  _c_mask_loop_type = loop_type;
  _c_mask_loop_hook = cc->cursor();
}

void CompOpPart::_c_mask_loop_fini() noexcept {
  // Make sure `_c_mask_loop_init()` and `_c_mask_loop_fini()` are used as a pair.
  BL_ASSERT(_c_mask_loop_type != CMaskLoopType::kNone);
  BL_ASSERT(_c_mask_loop_hook != nullptr);

  _c_mask_loop_type = CMaskLoopType::kNone;
  _c_mask_loop_hook = nullptr;
}

// bl::Pipeline::JIT::CompOpPart - CMask - Generic Loop
// ====================================================

void CompOpPart::c_mask_generic_loop(Gp& i) noexcept {
  if (is_loop_opaque() && should_just_copy_opaque_fill()) {
    c_mask_memcpy_or_memset_loop(i);
    return;
  }

  c_mask_generic_loop_vec(i);
}

void CompOpPart::c_mask_generic_loop_vec(Gp& i) noexcept {
  uint32_t n = max_pixels();

  Gp d_ptr = dst_part()->as<FetchPixelPtrPart>()->ptr();

  // 1 pixel at a time.
  if (n == 1) {
    Label L_Loop = pc->new_label();

    pc->bind(L_Loop);
    c_mask_proc_store_advance(d_ptr, PixelCount(1), Alignment(1));
    pc->j(L_Loop, sub_nz(i, 1));

    return;
  }

  BL_ASSERT(min_alignment() >= Alignment(1));
  // uint32_t alignment_mask = min_alignment().value() - 1;

  // 4 pixels at a time.
  if (n == 4) {
    Label L_Loop = pc->new_label();
    Label L_Tail = pc->new_label();
    Label L_Done = pc->new_label();

    enter_n();
    prefetch_n();

    pc->j(L_Tail, sub_c(i, n));

    pc->bind(L_Loop);
    c_mask_proc_store_advance(d_ptr, PixelCount(n));
    pc->j(L_Loop, sub_nc(i, n));

    pc->bind(L_Tail);
    pc->j(L_Done, add_z(i, n));

    PixelPredicate predicate;
    predicate.init(n, PredicateFlags::kNeverFull, i);
    c_mask_proc_store_advance(d_ptr, PixelCount(n), Alignment(1), predicate);

    pc->bind(L_Done);

    postfetch_n();
    leave_n();
    return;
  }

  // 8 pixels at a time.
  if (n == 8) {
    Label L_LoopN = pc->new_label();
    Label L_SkipN = pc->new_label();
    Label L_Exit = pc->new_label();

    enter_n();
    prefetch_n();

    pc->j(L_SkipN, sub_c(i, n));

    pc->bind(L_LoopN);
    c_mask_proc_store_advance(d_ptr, PixelCount(n), Alignment(1));
    pc->j(L_LoopN, sub_nc(i, n));

    pc->bind(L_SkipN);
    pc->j(L_Exit, add_z(i, n));

    if (pc->use_512bit_simd()) {
      PixelPredicate predicate(n, PredicateFlags::kNeverFull, i);
      c_mask_proc_store_advance(d_ptr, PixelCount(n), Alignment(1), predicate);
    }
    else {
      Label L_Skip4 = pc->new_label();
      pc->j(L_Skip4, ucmp_lt(i, 4));
      c_mask_proc_store_advance(d_ptr, PixelCount(4), Alignment(1));
      pc->j(L_Exit, sub_z(i, 4));

      pc->bind(L_Skip4);
      PixelPredicate predicate(8u, PredicateFlags::kNeverFull, i);
      c_mask_proc_store_advance(d_ptr, PixelCount(4), Alignment(1), predicate);
    }

    pc->bind(L_Exit);

    postfetch_n();
    leave_n();

    return;
  }
  // 16 pixels at a time.
  if (n == 16) {
    Label L_LoopN = pc->new_label();
    Label L_SkipN = pc->new_label();
    Label L_Exit = pc->new_label();

    enter_n();
    prefetch_n();

    pc->j(L_SkipN, sub_c(i, n));

    pc->bind(L_LoopN);
    c_mask_proc_store_advance(d_ptr, PixelCount(n), Alignment(1));
    pc->j(L_LoopN, sub_nc(i, n));

    pc->bind(L_SkipN);
    pc->j(L_Exit, add_z(i, n));

    if (pc->use_512bit_simd()) {
      PixelPredicate predicate(n, PredicateFlags::kNeverFull, i);
      c_mask_proc_store_advance(d_ptr, PixelCount(n), Alignment(1), predicate);
    }
    else {
      Label L_Skip8 = pc->new_label();
      pc->j(L_Skip8, ucmp_lt(i, 8));
      c_mask_proc_store_advance(d_ptr, PixelCount(8), Alignment(1));
      pc->j(L_Exit, sub_z(i, 8));

      pc->bind(L_Skip8);
      PixelPredicate predicate(8u, PredicateFlags::kNeverFull, i);
      c_mask_proc_store_advance(d_ptr, PixelCount(8), Alignment(1), predicate);
    }

    pc->bind(L_Exit);

    postfetch_n();
    leave_n();

    return;
  }

  // 32 pixels at a time.
  if (n == 32) {
    Label L_LoopN = pc->new_label();
    Label L_SkipN = pc->new_label();
    Label L_Loop8 = pc->new_label();
    Label L_Skip8 = pc->new_label();
    Label L_Exit = pc->new_label();

    enter_n();
    prefetch_n();

    pc->j(L_SkipN, sub_c(i, n));

    pc->bind(L_LoopN);
    c_mask_proc_store_advance(d_ptr, PixelCount(n), Alignment(1));
    pc->j(L_LoopN, sub_nc(i, n));

    pc->bind(L_SkipN);
    pc->j(L_Exit, add_z(i, n));
    pc->j(L_Skip8, sub_c(i, 8));

    pc->bind(L_Loop8);
    c_mask_proc_store_advance(d_ptr, PixelCount(8), Alignment(1));
    pc->j(L_Loop8, sub_nc(i, 8));

    pc->bind(L_Skip8);
    pc->j(L_Exit, add_z(i, 8));

    PixelPredicate predicate(8u, PredicateFlags::kNeverFull, i);
    c_mask_proc_store_advance(d_ptr, PixelCount(8), Alignment(1), predicate);

    pc->bind(L_Exit);

    postfetch_n();
    leave_n();

    return;
  }

  BL_NOT_REACHED();
}

// bl::Pipeline::JIT::CompOpPart - CMask - Granular Loop
// =====================================================

void CompOpPart::c_mask_granular_loop(Gp& i) noexcept {
  if (is_loop_opaque() && should_just_copy_opaque_fill()) {
    c_mask_memcpy_or_memset_loop(i);
    return;
  }

  c_mask_granular_loop_vec(i);
}

void CompOpPart::c_mask_granular_loop_vec(Gp& i) noexcept {
  BL_ASSERT(pixel_granularity() == PixelCount(4));

  Gp d_ptr = dst_part()->as<FetchPixelPtrPart>()->ptr();
  if (pixel_granularity() == PixelCount(4)) {
    // 1 pixel at a time.
    if (max_pixels() == 1) {
      Label L_Loop = pc->new_label();
      Label L_Step = pc->new_label();

      pc->bind(L_Loop);
      enter_partial_mode();

      pc->bind(L_Step);
      c_mask_proc_store_advance(d_ptr, PixelCount(1));
      pc->dec(i);
      next_partial_pixel();

      pc->j(L_Step, test_nz(i, 0x3));
      exit_partial_mode();

      pc->j(L_Loop, test_nz(i));
      return;
    }

    // 4 pixels at a time.
    if (max_pixels() == 4) {
      Label L_Loop = pc->new_label();

      pc->bind(L_Loop);
      c_mask_proc_store_advance(d_ptr, PixelCount(4));
      pc->j(L_Loop, sub_nz(i, 4));

      return;
    }

    // 8 pixels at a time.
    if (max_pixels() == 8) {
      Label L_Loop_Iter8 = pc->new_label();
      Label L_Skip = pc->new_label();
      Label L_End = pc->new_label();

      pc->j(L_Skip, sub_c(i, 8));

      pc->bind(L_Loop_Iter8);
      c_mask_proc_store_advance(d_ptr, PixelCount(8));
      pc->j(L_Loop_Iter8, sub_nc(i, 8));

      pc->bind(L_Skip);
      pc->j(L_End, add_z(i, 8));

      // 4 remaining pixels.
      c_mask_proc_store_advance(d_ptr, PixelCount(4));

      pc->bind(L_End);
      return;
    }

    // 16 pixels at a time.
    if (max_pixels() == 16) {
      Label L_Loop_Iter16 = pc->new_label();
      Label L_Loop_Iter4 = pc->new_label();
      Label L_Skip = pc->new_label();
      Label L_End = pc->new_label();

      pc->j(L_Skip, sub_c(i, 16));

      pc->bind(L_Loop_Iter16);
      c_mask_proc_store_advance(d_ptr, PixelCount(16));
      pc->j(L_Loop_Iter16, sub_nc(i, 16));

      pc->bind(L_Skip);
      pc->j(L_End, add_z(i, 16));

      // 4 remaining pixels.
      pc->bind(L_Loop_Iter4);
      c_mask_proc_store_advance(d_ptr, PixelCount(4));
      pc->j(L_Loop_Iter4, sub_nz(i, 4));

      pc->bind(L_End);
      return;
    }

    // 32 pixels at a time.
    if (max_pixels() == 32) {
      Label L_Loop_Iter32 = pc->new_label();
      Label L_Loop_Iter4 = pc->new_label();
      Label L_Skip = pc->new_label();
      Label L_End = pc->new_label();

      pc->j(L_Skip, sub_c(i, 32));

      pc->bind(L_Loop_Iter32);
      c_mask_proc_store_advance(d_ptr, PixelCount(32));
      pc->j(L_Loop_Iter32, sub_nc(i, 32));

      pc->bind(L_Skip);
      pc->j(L_End, add_z(i, 32));

      // 4 remaining pixels.
      pc->bind(L_Loop_Iter4);
      c_mask_proc_store_advance(d_ptr, PixelCount(4));
      pc->j(L_Loop_Iter4, sub_nz(i, 4));

      pc->bind(L_End);
      return;
    }
  }

  BL_NOT_REACHED();
}

// bl::Pipeline::JIT::CompOpPart - CMask - MemCopy & MemSet Loop
// =============================================================

void CompOpPart::c_mask_memcpy_or_memset_loop(Gp& i) noexcept {
  BL_ASSERT(should_just_copy_opaque_fill());
  Gp d_ptr = dst_part()->as<FetchPixelPtrPart>()->ptr();

  if (src_part()->is_solid()) {
    // Optimized solid opaque fill -> MemSet.
    BL_ASSERT(_solid_opt.px.is_valid());
    FetchUtils::inline_fill_span_loop(pc, d_ptr, _solid_opt.px, i, 64, dst_part()->bpp(), uint32_t(pixel_granularity()));
  }
  else if (src_part()->is_fetch_type(FetchType::kPatternAlignedBlit)) {
    // Optimized solid opaque blit -> MemCopy.
    FetchUtils::inline_copy_span_loop(pc, d_ptr, src_part()->as<FetchSimplePatternPart>()->f->srcp1, i, 64, dst_part()->bpp(), uint32_t(pixel_granularity()), dst_part()->format());
  }
  else {
    BL_NOT_REACHED();
  }
}

// bl::Pipeline::JIT::CompOpPart - CMask - Composition Helpers
// ===========================================================

void CompOpPart::c_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, Alignment alignment) noexcept {
  PixelPredicate ptr_mask;
  c_mask_proc_store_advance(d_ptr, n, alignment, ptr_mask);
}

void CompOpPart::c_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, Alignment alignment, PixelPredicate& predicate) noexcept {
  Pixel d_pix("d", pixel_type());

  switch (pixel_type()) {
    case PixelType::kA8: {
      if (n == PixelCount(1))
        c_mask_proc_a8_gp(d_pix, PixelFlags::kSA | PixelFlags::kImmutable);
      else
        c_mask_proc_a8_vec(d_pix, n, PixelFlags::kImmutable, predicate);
      FetchUtils::store_pixels_and_advance(pc, d_ptr, d_pix, n, 1, alignment, predicate);
      break;
    }

    case PixelType::kRGBA32: {
      c_mask_proc_rgba32_vec(d_pix, n, PixelFlags::kImmutable, predicate);
      FetchUtils::store_pixels_and_advance(pc, d_ptr, d_pix, n, 4, alignment, predicate);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::JIT::CompOpPart - VMask - Composition Helpers
// ===========================================================

enum class CompOpLoopStrategy : uint32_t {
  kLoop1,
  kLoopNTail4,
  kLoopNTailN
};

void CompOpPart::v_mask_generic_loop(Gp& i, const Gp& d_ptr, const Gp& mPtr, GlobalAlpha* ga, const Label& done) noexcept {
  CompOpLoopStrategy strategy = CompOpLoopStrategy::kLoop1;

  if (max_pixels() >= 8) {
    strategy = CompOpLoopStrategy::kLoopNTail4;
  }
  else if (max_pixels() >= 4) {
    strategy = CompOpLoopStrategy::kLoopNTailN;
  }

  switch (strategy) {
    case CompOpLoopStrategy::kLoop1: {
      Label L_Loop1 = pc->new_label();
      Label L_Done = done.is_valid() ? done : pc->new_label();

      pc->bind(L_Loop1);
      v_mask_generic_step(d_ptr, PixelCount(1), mPtr, ga);
      pc->j(L_Loop1, sub_nz(i, 1));

      if (done.is_valid())
        pc->j(L_Done);
      else
        pc->bind(L_Done);

      break;
    }

    case CompOpLoopStrategy::kLoopNTail4: {
      uint32_t n = bl_min<uint32_t>(max_pixels(), 8);

      Label L_LoopN = pc->new_label();
      Label L_SkipN = pc->new_label();
      Label L_Skip4 = pc->new_label();
      Label L_Done = pc->new_label();

      enter_n();
      prefetch_n();

      pc->j(L_SkipN, sub_c(i, n));

      pc->bind(L_LoopN);
      v_mask_generic_step(d_ptr, PixelCount(n), mPtr, ga);
      pc->j(L_LoopN, sub_nc(i, n));

      pc->bind(L_SkipN);
      pc->j(L_Done, add_z(i, n));

      pc->j(L_Skip4, ucmp_lt(i, 4));
      v_mask_generic_step(d_ptr, PixelCount(4), mPtr, ga);
      pc->j(L_Done, sub_z(i, 4));

      pc->bind(L_Skip4);
      PixelPredicate predicate(n, PredicateFlags::kNeverFull, i);
      v_mask_generic_step(d_ptr, PixelCount(4), mPtr, ga, predicate);
      pc->bind(L_Done);

      postfetch_n();
      leave_n();

      if (done.is_valid())
        pc->j(done);

      break;
    }

    case CompOpLoopStrategy::kLoopNTailN: {
      uint32_t n = bl_min<uint32_t>(max_pixels(), 8);

      Label L_LoopN = pc->new_label();
      Label L_SkipN = pc->new_label();
      Label L_Done = pc->new_label();

      enter_n();
      prefetch_n();

      pc->j(L_SkipN, sub_c(i, n));

      pc->bind(L_LoopN);
      v_mask_generic_step(d_ptr, PixelCount(n), mPtr, ga);
      pc->j(L_LoopN, sub_nc(i, n));

      pc->bind(L_SkipN);
      pc->j(L_Done, add_z(i, n));

      PixelPredicate predicate(n, PredicateFlags::kNeverFull, i);
      v_mask_generic_step(d_ptr, PixelCount(n), mPtr, ga, predicate);

      pc->bind(L_Done);

      postfetch_n();
      leave_n();

      if (done.is_valid())
        pc->j(done);

      break;
    }
  }
}

void CompOpPart::v_mask_generic_step(const Gp& d_ptr, PixelCount n, const Gp& mPtr, GlobalAlpha* ga) noexcept {
  PixelPredicate no_predicate;
  v_mask_generic_step(d_ptr, n, mPtr, ga, no_predicate);
}

void CompOpPart::v_mask_generic_step(const Gp& d_ptr, PixelCount n, const Gp& mPtr, GlobalAlpha* ga, PixelPredicate& predicate) noexcept {
  switch (pixel_type()) {
    case PixelType::kA8: {
      if (n == PixelCount(1)) {
        BL_ASSERT(predicate.is_empty());

        Gp sm = pc->new_gp32("sm");
        pc->load_u8(sm, mem_ptr(mPtr));
        pc->add(mPtr, mPtr, uint32_t(n));

        if (ga) {
          pc->mul(sm, sm, ga->sa().r32());
          pc->div_255_u32(sm, sm);
        }

        Pixel d_pix("d", pixel_type());
        v_mask_proc_a8_gp(d_pix, PixelFlags::kSA | PixelFlags::kImmutable, sm, PixelCoverageFlags::kNone);
        FetchUtils::store_pixels_and_advance(pc, d_ptr, d_pix, n, 1, Alignment(1), pc->empty_predicate());
      }
      else {
        VecArray vm;
        FetchUtils::fetch_mask_a8(pc, vm, mPtr, n, pixel_type(), coverage_format(), AdvanceMode::kAdvance, predicate, ga);
        v_mask_proc_store_advance(d_ptr, n, vm, PixelCoverageFlags::kNone, Alignment(1), predicate);
      }
      break;
    }

    case PixelType::kRGBA32: {
      VecArray vm;
      FetchUtils::fetch_mask_a8(pc, vm, mPtr, n, pixel_type(), coverage_format(), AdvanceMode::kAdvance, predicate, ga);
      v_mask_proc_store_advance(d_ptr, n, vm, PixelCoverageFlags::kNone, Alignment(1), predicate);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void CompOpPart::v_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, const VecArray& vm, PixelCoverageFlags coverage_flags, Alignment alignment) noexcept {
  PixelPredicate ptr_mask;
  v_mask_proc_store_advance(d_ptr, n, vm, coverage_flags, alignment, ptr_mask);
}

void CompOpPart::v_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, const VecArray& vm, PixelCoverageFlags coverage_flags, Alignment alignment, PixelPredicate& predicate) noexcept {
  Pixel d_pix("d", pixel_type());

  switch (pixel_type()) {
    case PixelType::kA8: {
      BL_ASSERT(n != PixelCount(1));

      v_mask_proc_a8_vec(d_pix, n, PixelFlags::kPA | PixelFlags::kImmutable, vm, coverage_flags, predicate);
      FetchUtils::store_pixels_and_advance(pc, d_ptr, d_pix, n, 1, alignment, predicate);
      break;
    }

    case PixelType::kRGBA32: {
      v_mask_proc_rgba32_vec(d_pix, n, PixelFlags::kImmutable, vm, coverage_flags, predicate);
      FetchUtils::store_pixels_and_advance(pc, d_ptr, d_pix, n, 4, alignment, predicate);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void CompOpPart::v_mask_proc(Pixel& out, PixelFlags flags, Gp& msk, PixelCoverageFlags coverage_flags) noexcept {
  switch (pixel_type()) {
    case PixelType::kA8: {
      v_mask_proc_a8_gp(out, flags, msk, coverage_flags);
      break;
    }

    case PixelType::kRGBA32: {
      Vec vm = pc->new_vec128("c.vm");

#if defined(BL_JIT_ARCH_X86)
      if (!pc->has_avx()) {
        pc->s_mov_u32(vm, msk);
        pc->v_swizzle_lo_u16x4(vm, vm, swizzle(0, 0, 0, 0));
      }
      else
#endif
      {
        pc->v_broadcast_u16(vm, msk);
      }

      VecArray vm_(vm);
      v_mask_proc_rgba32_vec(out, PixelCount(1), flags, vm_, PixelCoverageFlags::kNone, pc->empty_predicate());
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// bl::Pipeline::JIT::CompOpPart - CMask - Init & Fini - A8
// ========================================================

void CompOpPart::c_mask_init_a8(const Gp& sm_, const Vec& vm_) noexcept {
  Gp sm(sm_);
  Vec vm(vm_);

  bool has_mask = sm.is_valid() || vm.is_valid();
  if (has_mask) {
    // SM must be 32-bit, so make it 32-bit if it's 64-bit for any reason.
    if (sm.is_valid()) {
      sm = sm.r32();
    }

    if (vm.is_valid() && !sm.is_valid()) {
      sm = pc->new_gp32("sm");
      pc->s_extract_u16(sm, vm, 0);
    }

    _mask->sm = sm;
    _mask->vm = vm;
  }

  if (src_part()->is_solid()) {
    Pixel& s = src_part()->as<FetchSolidPart>()->_pixel;
    SolidPixel& o = _solid_opt;
    bool convert_to_vec = true;

    // CMaskInit - A8 - Solid - SrcCopy
    // --------------------------------

    if (is_src_copy()) {
      if (!has_mask) {
        // Xa = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);
        o.sa = s.sa;

        if (max_pixels() > 1) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPA);
          o.px = s.pa[0];
        }

        convert_to_vec = false;
      }
      else {
#if defined(BL_JIT_ARCH_A64)
        // Xa  = (Sa * m)
        // Vn  = (1 - m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("p.sx");
        o.sy = pc->new_gp32("p.sy");

        pc->mul(o.sx, s.sa, sm);
        pc->inv_u8(o.sy, sm);

        if (max_pixels() > 1) {
          o.ux = pc->new_vec("p.ux");
          o.vn = pc->new_vec("p.vn");

          pc->v_broadcast_u16(o.ux, o.sx);
          pc->v_broadcast_u8(o.vn, o.sy);
        }

        convert_to_vec = false;
#else
        // Xa = (Sa * m) + <Rounding>
        // Ya = (1 - m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("p.sx");
        o.sy = sm;

        pc->mul(o.sx, s.sa, o.sy);
        pc->add(o.sx, o.sx, imm(0x80)); // Rounding
        pc->inv_u8(o.sy, o.sy);
#endif
      }
    }

    // CMaskInit - A8 - Solid - SrcOver
    // --------------------------------

    else if (is_src_over()) {
      if (!has_mask) {
        // Xa = Sa * 1 + 0.5 <Rounding>
        // Ya = 1 - Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("p.sx");
        o.sy = sm;

        pc->mov(o.sx, s.sa);
        pc->shl(o.sx, o.sx, 8);
        pc->sub(o.sx, o.sx, s.sa);
        pc->inv_u8(o.sy, o.sy);
      }
      else {
        // Xa = Sa * m + 0.5 <Rounding>
        // Ya = 1 - (Sa * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("p.sx");
        o.sy = sm;

        pc->mul(o.sy, sm, s.sa);
        pc->div_255_u32(o.sy, o.sy);

        pc->shl(o.sx, o.sy, imm(8));
        pc->sub(o.sx, o.sx, o.sy);
#if defined(BL_JIT_ARCH_X86)
        pc->add(o.sx, o.sx, imm(0x80));
#endif  // BL_JIT_ARCH_X86
        pc->inv_u8(o.sy, o.sy);
      }

#if defined(BL_JIT_ARCH_A64)
      if (max_pixels() > 1) {
        o.ux = pc->new_vec("p.ux");
        o.py = pc->new_vec("p.py");

        pc->v_broadcast_u16(o.ux, o.sx);
        pc->v_broadcast_u8(o.py, o.sy);
      }

      convert_to_vec = false;
#endif // BL_JIT_ARCH_A64
    }

    // CMaskInit - A8 - Solid - SrcIn
    // ------------------------------

    else if (is_src_in()) {
      if (!has_mask) {
        // Xa = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = s.sa;
        if (max_pixels() > 1) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA);
          o.ux = s.ua[0];
        }
      }
      else {
        // Xa = Sa * m + (1 - m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("o.sx");
        pc->mul(o.sx, s.sa, sm);
        pc->div_255_u32(o.sx, o.sx);
        pc->inv_u8(sm, sm);
        pc->add(o.sx, o.sx, sm);
      }
    }

    // CMaskInit - A8 - Solid - SrcOut
    // -------------------------------

    else if (is_src_out()) {
      if (!has_mask) {
        // Xa = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = s.sa;
        if (max_pixels() > 1) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA);
          o.ux = s.ua[0];
        }
      }
      else {
        // Xa = Sa * m
        // Ya = 1  - m
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("o.sx");
        o.sy = sm;

        pc->mul(o.sx, s.sa, o.sy);
        pc->div_255_u32(o.sx, o.sx);
        pc->inv_u8(o.sy, o.sy);
      }
    }

    // CMaskInit - A8 - Solid - DstOut
    // -------------------------------

    else if (is_dst_out()) {
      if (!has_mask) {
        // Xa = 1 - Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("o.sx");
        pc->inv_u8(o.sx, s.sa);

        if (max_pixels() > 1) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUI);
          o.ux = s.ui[0];
        }
      }
      else {
        // Xa = 1 - (Sa * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = sm;
        pc->mul(o.sx, sm, s.sa);
        pc->div_255_u32(o.sx, o.sx);
        pc->inv_u8(o.sx, o.sx);
      }
    }

    // CMaskInit - A8 - Solid - Xor
    // ----------------------------

    else if (is_xor()) {
      if (!has_mask) {
        // Xa = Sa
        // Ya = 1 - Xa (SIMD only)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);
        o.sx = s.sa;

        if (max_pixels() > 1) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA | PixelFlags::kUI);

          o.ux = s.ua[0];
          o.uy = s.ui[0];
        }
      }
      else {
        // Xa = Sa * m
        // Ya = 1 - Xa (SIMD only)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);

        o.sx = pc->new_gp32("o.sx");
        pc->mul(o.sx, sm, s.sa);
        pc->div_255_u32(o.sx, o.sx);

        if (max_pixels() > 1) {
          o.ux = pc->new_vec("o.ux");
          o.uy = pc->new_vec("o.uy");
          pc->v_broadcast_u16(o.ux, o.sx);
          pc->v_inv255_u16(o.uy, o.ux);
        }
      }
    }

    // CMaskInit - A8 - Solid - Plus
    // -----------------------------

    else if (is_plus()) {
      if (!has_mask) {
        // Xa = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA | PixelFlags::kPA);
        o.sa = s.sa;
        o.px = s.pa[0];
        convert_to_vec = false;
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kSA);
        o.sx = sm;
        pc->mul(o.sx, o.sx, s.sa);
        pc->div_255_u32(o.sx, o.sx);

        if (max_pixels() > 1) {
          o.px = pc->new_vec("o.px");
          pc->mul(o.sx, o.sx, 0x01010101);
          pc->v_broadcast_u32(o.px, o.sx);
          pc->shr(o.sx, o.sx, imm(24));
        }

        convert_to_vec = false;
      }
    }

    // CMaskInit - A8 - Solid - Extras
    // -------------------------------

    if (convert_to_vec && max_pixels() > 1) {
      if (o.sx.is_valid() && !o.ux.is_valid()) {
        if (coverage_format() == PixelCoverageFormat::kPacked) {
          o.px = pc->new_vec("p.px");
          pc->v_broadcast_u8(o.px, o.sx);
        }
        else {
          o.ux = pc->new_vec("p.ux");
          pc->v_broadcast_u16(o.ux, o.sx);
        }
      }

      if (o.sy.is_valid() && !o.uy.is_valid()) {
        if (coverage_format() == PixelCoverageFormat::kPacked) {
          o.py = pc->new_vec("p.py");
          pc->v_broadcast_u8(o.py, o.sy);
        }
        else {
          o.uy = pc->new_vec("p.uy");
          pc->v_broadcast_u16(o.uy, o.sy);
        }
      }
    }
  }
  else {
    if (sm.is_valid() && !vm.is_valid() && max_pixels() > 1) {
      vm = pc->new_vec("vm");
      if (coverage_format() == PixelCoverageFormat::kPacked) {
        pc->v_broadcast_u8z(vm, sm);
      }
      else {
        pc->v_broadcast_u16z(vm, sm);
      }
      _mask->vm = vm;
    }

    /*
    // CMaskInit - A8 - NonSolid - SrcCopy
    // -----------------------------------

    if (is_src_copy()) {
      if (has_mask) {
        Vec vn = pc->new_vec("vn");
        pc->v_inv255_u16(vn, m);
        _mask->vec.vn = vn;
      }
    }
    */
  }

  _c_mask_loop_init(has_mask ? CMaskLoopType::kVariant : CMaskLoopType::kOpaque);
}

void CompOpPart::c_mask_fini_a8() noexcept {
  if (src_part()->is_solid()) {
    _solid_opt.reset();
    _solid_pre.reset();
  }
  else {
    // TODO: [JIT] ???
  }

  _mask->reset();
  _c_mask_loop_fini();
}

// bl::Pipeline::JIT::CompOpPart - CMask - Proc - A8
// =================================================

void CompOpPart::c_mask_proc_a8_gp(Pixel& out, PixelFlags flags) noexcept {
  out.set_count(PixelCount(1));

  bool has_mask = is_loop_c_mask();

  if (src_part()->is_solid()) {
    Pixel d("d", pixel_type());
    SolidPixel& o = _solid_opt;

    Gp& da = d.sa;
    Gp sx = pc->new_gp32("sx");

    // CMaskProc - A8 - SrcCopy
    // ------------------------

    if (is_src_copy()) {
      if (!has_mask) {
        // Da' = Xa
        out.sa = o.sa;
        out.make_immutable();
      }
      else {
        // Da' = Xa  + Da .(1 - m)
        dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

        pc->mul(da, da, o.sy),
        pc->add(da, da, o.sx);
        pc->mul_257_hu16(da, da);

        out.sa = da;
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - SrcOver
    // ------------------------

    if (is_src_over()) {
      // Da' = Xa + Da .Ya
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(da, da, o.sy);
      pc->add(da, da, o.sx);
      pc->mul_257_hu16(da, da);

      out.sa = da;

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - SrcIn & DstOut
    // -------------------------------

    if (is_src_in() || is_dst_out()) {
      // Da' = Xa.Da
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(da, da, o.sx);
      pc->div_255_u32(da, da);
      out.sa = da;

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - SrcOut
    // -----------------------

    if (is_src_out()) {
      if (!has_mask) {
        // Da' = Xa.(1 - Da)
        dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

        pc->inv_u8(da, da);
        pc->mul(da, da, o.sx);
        pc->div_255_u32(da, da);
        out.sa = da;
      }
      else {
        // Da' = Xa.(1 - Da) + Da.Ya
        dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

        pc->inv_u8(sx, da);
        pc->mul(da, da, o.sy);
        pc->mul(sx, sx, o.sx);
        pc->add(da, da, sx);
        pc->div_255_u32(da, da);
        out.sa = da;
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - Xor
    // --------------------

    if (is_xor()) {
      // Da' = Xa.(1 - Da) + Da.Ya
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sx, da, o.sy);
      pc->inv_u8(da, da);
      pc->mul(da, da, o.sx);
      pc->add(da, da, sx);
      pc->div_255_u32(da, da);
      out.sa = da;

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - Plus
    // ---------------------

    if (is_plus()) {
      // Da' = Clamp(Da + Xa)
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->adds_u8(da, da, o.sx);
      out.sa = da;

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }
  }

  v_mask_proc_a8_gp(out, flags, _mask->sm, PixelCoverageFlags::kImmutable);
}

void CompOpPart::c_mask_proc_a8_vec(Pixel& out, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  out.set_count(n);

  bool has_mask = is_loop_c_mask();

  if (src_part()->is_solid()) {
    Pixel d("d", pixel_type());
    SolidPixel& o = _solid_opt;

    VecWidth pa_vec_width = pc->vec_width_of(DataWidth::k8, n);
    VecWidth ua_vec_width = pc->vec_width_of(DataWidth::k16, n);
    const size_t full_n = pc->vec_count_of(DataWidth::k16, n);

    VecArray xa;
    pc->new_vec_array(xa, full_n, ua_vec_width, "x");

    // CMaskProc - A8 - SrcCopy
    // ------------------------

    if (is_src_copy()) {
      if (!has_mask) {
        // Da' = Xa
        out.pa.init(VecWidthUtils::clone_vec_as(o.px, pa_vec_width));
        out.make_immutable();
      }
      else {
#if defined(BL_JIT_ARCH_A64)
        dst_fetch(d, n, PixelFlags::kPA, predicate);

        CompOpUtils::mul_u8_widen(pc, xa, d.pa, o.vn, uint32_t(n));
        pc->v_add_u16(xa, xa, o.ux);
        CompOpUtils::combine_div255_and_out_a8(pc, out, flags, xa);
#else
        // Da' = Xa + Da .(1 - m)
        dst_fetch(d, n, PixelFlags::kUA, predicate);

        Vec s_ux = o.ux.clone_as(d.ua[0]);
        Vec s_uy = o.uy.clone_as(d.ua[0]);

        pc->v_mul_i16(d.ua, d.ua, s_uy),
        pc->v_add_i16(d.ua, d.ua, s_ux);
        pc->v_mul257_hi_u16(d.ua, d.ua);

        out.ua.init(d.ua);
#endif
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - SrcOver
    // ------------------------

    if (is_src_over()) {
#if defined(BL_JIT_ARCH_A64)
      // Da' = Xa + Da.Ya
      dst_fetch(d, n, PixelFlags::kPA, predicate);

      CompOpUtils::mul_u8_widen(pc, xa, d.pa, o.py, uint32_t(n));
      pc->v_add_i16(xa, xa, o.ux);
      CompOpUtils::combine_div255_and_out_a8(pc, out, flags, xa);
#else
      // Da' = Xa + Da.Ya
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      Vec s_ux = o.ux.clone_as(d.ua[0]);
      Vec s_uy = o.uy.clone_as(d.ua[0]);

      pc->v_mul_i16(d.ua, d.ua, s_uy);
      pc->v_add_i16(d.ua, d.ua, s_ux);
      pc->v_mul257_hi_u16(d.ua, d.ua);

      out.ua.init(d.ua);
#endif

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - SrcIn & DstOut
    // -------------------------------

    if (is_src_in() || is_dst_out()) {
      // Da' = Xa.Da
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      Vec s_ux = o.ux.clone_as(d.ua[0]);

      pc->v_mul_u16(d.ua, d.ua, s_ux);
      pc->v_div255_u16(d.ua);
      out.ua.init(d.ua);

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - SrcOut
    // -----------------------

    if (is_src_out()) {
      if (!has_mask) {
        // Da' = Xa.(1 - Da)
        dst_fetch(d, n, PixelFlags::kUA, predicate);

        Vec s_ux = o.ux.clone_as(d.ua[0]);

        pc->v_inv255_u16(d.ua, d.ua);
        pc->v_mul_u16(d.ua, d.ua, s_ux);
        pc->v_div255_u16(d.ua);
        out.ua.init(d.ua);
      }
      else {
        // Da' = Xa.(1 - Da) + Da.Ya
        dst_fetch(d, n, PixelFlags::kUA, predicate);

        Vec s_ux = o.ux.clone_as(d.ua[0]);
        Vec s_uy = o.uy.clone_as(d.ua[0]);

        pc->v_inv255_u16(xa, d.ua);
        pc->v_mul_u16(xa, xa, s_ux);
        pc->v_mul_u16(d.ua, d.ua, s_uy);
        pc->v_add_i16(d.ua, d.ua, xa);
        pc->v_div255_u16(d.ua);
        out.ua.init(d.ua);
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - Xor
    // --------------------

    if (is_xor()) {
      // Da' = Xa.(1 - Da) + Da.Ya
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      Vec s_ux = o.ux.clone_as(d.ua[0]);
      Vec s_uy = o.uy.clone_as(d.ua[0]);

      pc->v_mul_u16(xa, d.ua, s_uy);
      pc->v_inv255_u16(d.ua, d.ua);
      pc->v_mul_u16(d.ua, d.ua, s_ux);
      pc->v_add_i16(d.ua, d.ua, xa);
      pc->v_div255_u16(d.ua);
      out.ua.init(d.ua);

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - A8 - Plus
    // ---------------------

    if (is_plus()) {
      // Da' = Clamp(Da + Xa)
      dst_fetch(d, n, PixelFlags::kPA, predicate);

      Vec s_px = o.px.clone_as(d.pa[0]);

      pc->v_adds_u8(d.pa, d.pa, s_px);
      out.pa.init(d.pa);

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }
  }

  VecArray vm;
  if (_mask->vm.is_valid())
    vm.init(_mask->vm);
  v_mask_proc_a8_vec(out, n, flags, vm, PixelCoverageFlags::kRepeatedImmutable, predicate);
}

// bl::Pipeline::JIT::CompOpPart - VMask Proc - A8 (Scalar)
// ========================================================

void CompOpPart::v_mask_proc_a8_gp(Pixel& out, PixelFlags flags, const Gp& msk, PixelCoverageFlags coverage_flags) noexcept {
  bool has_mask = msk.is_valid();

  Pixel d("d", PixelType::kA8);
  Pixel s("s", PixelType::kA8);

  Gp x = pc->new_gp32("@x");
  Gp y = pc->new_gp32("@y");

  Gp& da = d.sa;
  Gp& sa = s.sa;

  out.set_count(PixelCount(1));

  // VMask - A8 - SrcCopy
  // --------------------

  if (is_src_copy()) {
    if (!has_mask) {
      // Da' = Sa
      src_fetch(out, PixelCount(1), flags, pc->empty_predicate());
    }
    else {
      // Da' = Sa.m + Da.(1 - m)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sa, sa, msk);
      pc->inv_u8(msk, msk);
      pc->mul(da, da, msk);

      if (bl_test_flag(coverage_flags, PixelCoverageFlags::kImmutable))
        pc->inv_u8(msk, msk);

      pc->add(da, da, sa);
      pc->div_255_u32(da, da);

      out.sa = da;
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - SrcOver
  // --------------------

  if (is_src_over()) {
    if (!has_mask) {
      // Da' = Sa + Da.(1 - Sa)
      src_fetch(s, PixelCount(1), PixelFlags::kSA | PixelFlags::kImmutable, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->inv_u8(x, sa);
      pc->mul(da, da, x);
      pc->div_255_u32(da, da);
      pc->add(da, da, sa);
    }
    else {
      // Da' = Sa.m + Da.(1 - Sa.m)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sa, sa, msk);
      pc->div_255_u32(sa, sa);
      pc->inv_u8(x, sa);
      pc->mul(da, da, x);
      pc->div_255_u32(da, da);
      pc->add(da, da, sa);
    }

    out.sa = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - SrcIn
  // ------------------

  if (is_src_in()) {
    if (!has_mask) {
      // Da' = Sa.Da
      src_fetch(s, PixelCount(1), PixelFlags::kSA | PixelFlags::kImmutable, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(da, da, sa);
      pc->div_255_u32(da, da);
    }
    else {
      // Da' = Da.(Sa.m) + Da.(1 - m)
      //     = Da.(Sa.m + 1 - m)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sa, sa, msk);
      pc->div_255_u32(sa, sa);
      pc->add(sa, sa, imm(255));
      pc->sub(sa, sa, msk);
      pc->mul(da, da, sa);
      pc->div_255_u32(da, da);
    }

    out.sa = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - SrcOut
  // -------------------

  if (is_src_out()) {
    if (!has_mask) {
      // Da' = Sa.(1 - Da)
      src_fetch(s, PixelCount(1), PixelFlags::kSA | PixelFlags::kImmutable, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->inv_u8(da, da);
      pc->mul(da, da, sa);
      pc->div_255_u32(da, da);
    }
    else {
      // Da' = Sa.m.(1 - Da) + Da.(1 - m)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sa, sa, msk);
      pc->div_255_u32(sa, sa);

      pc->inv_u8(x, da);
      pc->inv_u8(msk, msk);
      pc->mul(sa, sa, x);
      pc->mul(da, da, msk);

      if (bl_test_flag(coverage_flags, PixelCoverageFlags::kImmutable))
        pc->inv_u8(msk, msk);

      pc->add(da, da, sa);
      pc->div_255_u32(da, da);
    }

    out.sa = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - DstOut
  // -------------------

  if (is_dst_out()) {
    if (!has_mask) {
      // Da' = Da.(1 - Sa)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->inv_u8(sa, sa);
      pc->mul(da, da, sa);
      pc->div_255_u32(da, da);
    }
    else {
      // Da' = Da.(1 - Sa.m)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sa, sa, msk);
      pc->div_255_u32(sa, sa);
      pc->inv_u8(sa, sa);
      pc->mul(da, da, sa);
      pc->div_255_u32(da, da);
    }

    out.sa = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Xor
  // ----------------

  if (is_xor()) {
    if (!has_mask) {
      // Da' = Da.(1 - Sa) + Sa.(1 - Da)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->inv_u8(y, sa);
      pc->inv_u8(x, da);

      pc->mul(da, da, y);
      pc->mul(sa, sa, x);
      pc->add(da, da, sa);
      pc->div_255_u32(da, da);
    }
    else {
      // Da' = Da.(1 - Sa.m) + Sa.m.(1 - Da)
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sa, sa, msk);
      pc->div_255_u32(sa, sa);

      pc->inv_u8(y, sa);
      pc->inv_u8(x, da);

      pc->mul(da, da, y);
      pc->mul(sa, sa, x);
      pc->add(da, da, sa);
      pc->div_255_u32(da, da);
    }

    out.sa = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Plus
  // -----------------

  if (is_plus()) {
    // Da' = Clamp(Da + Sa)
    // Da' = Clamp(Da + Sa.m)
    if (has_mask) {
      src_fetch(s, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());

      pc->mul(sa, sa, msk);
      pc->div_255_u32(sa, sa);
    }
    else {
      src_fetch(s, PixelCount(1), PixelFlags::kSA | PixelFlags::kImmutable, pc->empty_predicate());
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
    }

    pc->adds_u8(da, da, sa);

    out.sa = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Invert
  // -------------------

  if (is_alpha_inv()) {
    // Da' = 1 - Da
    // Da' = Da.(1 - m) + (1 - Da).m
    if (has_mask) {
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      pc->inv_u8(x, msk);
      pc->mul(x, x, da);
      pc->inv_u8(da, da);
      pc->mul(da, da, msk);
      pc->add(da, da, x);
      pc->div_255_u32(da, da);
    }
    else {
      dst_fetch(d, PixelCount(1), PixelFlags::kSA, pc->empty_predicate());
      pc->inv_u8(da, da);
    }

    out.sa = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Invalid
  // --------------------

  BL_NOT_REACHED();
}

// bl::Pipeline::JIT::CompOpPart - VMask - Proc - A8 (Vec)
// =======================================================

void CompOpPart::v_mask_proc_a8_vec(Pixel& out, PixelCount n, PixelFlags flags, const VecArray& vm_, PixelCoverageFlags coverage_flags, PixelPredicate& predicate) noexcept {
  VecWidth vw = pc->vec_width_of(DataWidth::k16, n);
  size_t full_n = pc->vec_count_of(DataWidth::k16, n);

  VecArray vm = vm_.clone_as(vw);
  bool has_mask = !vm.is_empty();

  Pixel d("d", PixelType::kA8);
  Pixel s("s", PixelType::kA8);

  VecArray& da = d.ua;
  VecArray& sa = s.ua;

  VecArray xv, yv;
  pc->new_vec_array(xv, full_n, vw, "x");
  pc->new_vec_array(yv, full_n, vw, "y");

  out.set_count(n);

  // VMask - A8 - SrcCopy
  // --------------------

  if (is_src_copy()) {
    if (!has_mask) {
      // Da' = Sa
      src_fetch(out, n, flags, predicate);
    }
    else {
#if defined(BL_JIT_ARCH_A64)
      src_fetch(s, n, PixelFlags::kPA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPA, predicate);

      VecArray& vs = s.pa;
      VecArray& vd = d.pa;
      VecArray vn;

      CompOpUtils::mul_u8_widen(pc, xv, vs, vm, uint32_t(n));
      v_mask_proc_rgba32_invert_mask(vn, vm, coverage_flags);

      CompOpUtils::madd_u8_widen(pc, xv, vd, vn, uint32_t(n));
      v_mask_proc_rgba32_invert_done(vn, vm, coverage_flags);

      CompOpUtils::combine_div255_and_out_a8(pc, out, flags, xv);
#else
      // Da' = Sa.m + Da.(1 - m)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(sa, sa, vm);
      pc->v_inv255_u16(vm, vm);
      pc->v_mul_u16(da, da, vm);

      if (bl_test_flag(coverage_flags, PixelCoverageFlags::kImmutable))
        pc->v_inv255_u16(vm, vm);

      pc->v_add_i16(da, da, sa);
      pc->v_div255_u16(da);

      out.ua = da;
#endif
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - SrcOver
  // --------------------

  if (is_src_over()) {
#if defined(BL_JIT_ARCH_A64)
    if (!has_mask) {
      src_fetch(s, n, PixelFlags::kPA | PixelFlags::kPI | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPA, predicate);

      CompOpUtils::mul_u8_widen(pc, xv, d.pa, s.pi, uint32_t(n));
      CompOpUtils::div255_pack(pc, d.pa, xv);
      pc->v_add_u8(d.pa, d.pa, s.pa);
      out.pa.init(d.pa);
    }
    else {
      VecArray zv;
      pc->new_vec_array(zv, full_n, vw, "z");

      src_fetch(s, n, PixelFlags::kPA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPA, predicate);

      VecArray xv_half = xv.half();
      VecArray yv_half = yv.half();

      CompOpUtils::mul_u8_widen(pc, xv, s.pa, vm, uint32_t(n));
      CompOpUtils::div255_pack(pc, xv_half, xv);

      pc->v_not_u32(yv_half, xv_half);

      CompOpUtils::mul_u8_widen(pc, zv, d.pa, yv_half, uint32_t(n));
      CompOpUtils::div255_pack(pc, d.pa, zv);

      pc->v_add_u8(d.pa, d.pa, xv_half);
      out.pa.init(d.pa);
    }
#else
    if (!has_mask) {
      // Da' = Sa + Da.(1 - Sa)
      src_fetch(s, n, PixelFlags::kUA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_inv255_u16(xv, sa);
      pc->v_mul_u16(da, da, xv);
      pc->v_div255_u16(da);
      pc->v_add_i16(da, da, sa);
      out.ua = da;
    }
    else {
      // Da' = Sa.m + Da.(1 - Sa.m)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(sa, sa, vm);
      pc->v_div255_u16(sa);
      pc->v_inv255_u16(xv, sa);
      pc->v_mul_u16(da, da, xv);
      pc->v_div255_u16(da);
      pc->v_add_i16(da, da, sa);
      out.ua = da;
    }
#endif

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - SrcIn
  // ------------------

  if (is_src_in()) {
    if (!has_mask) {
      // Da' = Sa.Da
      src_fetch(s, n, PixelFlags::kUA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(da, da, sa);
      pc->v_div255_u16(da);
    }
    else {
      // Da' = Da.(Sa.m) + Da.(1 - m)
      //     = Da.(Sa.m + 1 - m)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(sa, sa, vm);
      pc->v_div255_u16(sa);
      pc->v_add_i16(sa, sa, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, sa));
      pc->v_sub_i16(sa, sa, vm);
      pc->v_mul_u16(da, da, sa);
      pc->v_div255_u16(da);
    }

    out.ua = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - SrcOut
  // -------------------

  if (is_src_out()) {
    if (!has_mask) {
      // Da' = Sa.(1 - Da)
      src_fetch(s, n, PixelFlags::kUA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_inv255_u16(da, da);
      pc->v_mul_u16(da, da, sa);
      pc->v_div255_u16(da);
    }
    else {
      // Da' = Sa.m.(1 - Da) + Da.(1 - m)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(sa, sa, vm);
      pc->v_div255_u16(sa);

      pc->v_inv255_u16(xv, da);
      pc->v_inv255_u16(vm, vm);
      pc->v_mul_u16(sa, sa, xv);
      pc->v_mul_u16(da, da, vm);

      if (bl_test_flag(coverage_flags, PixelCoverageFlags::kImmutable))
        pc->v_inv255_u16(vm, vm);

      pc->v_add_i16(da, da, sa);
      pc->v_div255_u16(da);
    }

    out.ua = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - DstOut
  // -------------------

  if (is_dst_out()) {
    if (!has_mask) {
      // Da' = Da.(1 - Sa)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_inv255_u16(sa, sa);
      pc->v_mul_u16(da, da, sa);
      pc->v_div255_u16(da);
    }
    else {
      // Da' = Da.(1 - Sa.m)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(sa, sa, vm);
      pc->v_div255_u16(sa);
      pc->v_inv255_u16(sa, sa);
      pc->v_mul_u16(da, da, sa);
      pc->v_div255_u16(da);
    }

    out.ua = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Xor
  // ----------------

  if (is_xor()) {
    if (!has_mask) {
      // Da' = Da.(1 - Sa) + Sa.(1 - Da)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_inv255_u16(yv, sa);
      pc->v_inv255_u16(xv, da);

      pc->v_mul_u16(da, da, yv);
      pc->v_mul_u16(sa, sa, xv);
      pc->v_add_i16(da, da, sa);
      pc->v_div255_u16(da);
    }
    else {
      // Da' = Da.(1 - Sa.m) + Sa.m.(1 - Da)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(sa, sa, vm);
      pc->v_div255_u16(sa);

      pc->v_inv255_u16(yv, sa);
      pc->v_inv255_u16(xv, da);

      pc->v_mul_u16(da, da, yv);
      pc->v_mul_u16(sa, sa, xv);
      pc->v_add_i16(da, da, sa);
      pc->v_div255_u16(da);
    }

    out.ua = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Plus
  // -----------------

  if (is_plus()) {
    if (!has_mask) {
      // Da' = Clamp(Da + Sa)
      src_fetch(s, n, PixelFlags::kPA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPA, predicate);

      pc->v_adds_u8(d.pa, d.pa, s.pa);
      out.pa = d.pa;
    }
    else {
      // Da' = Clamp(Da + Sa.m)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      pc->v_mul_u16(s.ua, s.ua, vm);
      pc->v_div255_u16(s.ua);
      pc->v_adds_u8(d.ua, d.ua, s.ua);
      out.ua = d.ua;
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Invert
  // -------------------

  if (is_alpha_inv()) {
    if (!has_mask) {
      // Da' = 1 - Da
      dst_fetch(d, n, PixelFlags::kUA, predicate);
      pc->v_inv255_u16(da, da);
    }
    else {
      // Da' = Da.(1 - m) + (1 - Da).m
      dst_fetch(d, n, PixelFlags::kUA, predicate);
      pc->v_inv255_u16(xv, vm);
      pc->v_mul_u16(xv, xv, da);
      pc->v_inv255_u16(da, da);
      pc->v_mul_u16(da, da, vm);
      pc->v_add_i16(da, da, xv);
      pc->v_div255_u16(da);
    }

    out.ua = da;
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMask - A8 - Invalid
  // --------------------

  BL_NOT_REACHED();
}

// bl::Pipeline::JIT::CompOpPart - CMask - Init & Fini - RGBA
// ==========================================================

void CompOpPart::c_mask_init_rgba32(const Vec& vm) noexcept {
  bool has_mask = vm.is_valid();
  bool use_da = has_da();

  if (src_part()->is_solid()) {
    Pixel& s = src_part()->as<FetchSolidPart>()->_pixel;
    SolidPixel& o = _solid_opt;

    // CMaskInit - RGBA32 - Solid - SrcCopy
    // ------------------------------------

    if (is_src_copy()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC);

        o.px = s.pc[0];
      }
      else {
#if defined(BL_JIT_ARCH_A64)
        // Xca = (Sca * m)
        // Xa  = (Sa  * m)
        // Im  = (1 - m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC);

        o.ux = pc->new_similar_reg(s.pc[0], "solid.ux");
        o.vn = vm;

        pc->v_mulw_lo_u8(o.ux, s.pc[0], vm);
        pc->v_not_u32(o.vn, vm);
#else
        // Xca = (Sca * m) + 0.5 <Rounding>
        // Xa  = (Sa  * m) + 0.5 <Rounding>
        // Im  = (1 - m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.vn = vm;

        pc->v_mul_u16(o.ux, s.uc[0], o.vn);
        pc->v_add_i16(o.ux, o.ux, pc->simd_const(&ct.p_0080008000800080, Bcst::kNA, o.ux));
        pc->v_inv255_u16(o.vn, o.vn);
#endif
      }
    }

    // CMaskInit - RGBA32 - Solid - SrcOver
    // ------------------------------------

    else if (is_src_over()) {
#if defined(BL_JIT_ARCH_A64)
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        // Yca = 1 - Sa
        // Ya  = 1 - Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC | PixelFlags::kPI | PixelFlags::kImmutable);

        o.px = s.pc[0];
        o.py = s.pi[0];
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        // Yca = 1 - (Sa * m)
        // Ya  = 1 - (Sa * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC | PixelFlags::kImmutable);

        o.px = pc->new_similar_reg(s.pc[0], "solid.px");
        o.py = pc->new_similar_reg(s.pc[0], "solid.py");

        pc->v_mulw_lo_u8(o.px, s.pc[0], vm);
        CompOpUtils::div255_pack(pc, o.px, o.px);
        pc->v_swizzle_u32x4(o.px, o.px, swizzle(0, 0, 0, 0));

        pc->v_not_u32(o.py, o.px);
        pc->v_swizzlev_u8(o.py, o.py, pc->simd_vec_const(&ct.swizu8_3xxx2xxx1xxx0xxx_to_3333222211110000, Bcst::kNA, o.py));
      }
#else
      if (!has_mask) {
        // Xca = Sca * 1 + 0.5 <Rounding>
        // Xa  = Sa  * 1 + 0.5 <Rounding>
        // Yca = 1 - Sa
        // Ya  = 1 - Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC | PixelFlags::kUI | PixelFlags::kImmutable);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.uy = s.ui[0];

        pc->v_slli_i16(o.ux, s.uc[0], 8);
        pc->v_sub_i16(o.ux, o.ux, s.uc[0]);
        pc->v_add_i16(o.ux, o.ux, pc->simd_const(&ct.p_0080008000800080, Bcst::kNA, o.ux));
      }
      else {
        // Xca = Sca * m + 0.5 <Rounding>
        // Xa  = Sa  * m + 0.5 <Rounding>
        // Yca = 1 - (Sa * m)
        // Ya  = 1 - (Sa * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC | PixelFlags::kImmutable);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");

        pc->v_mul_u16(o.uy, s.uc[0], vm);
        pc->v_div255_u16(o.uy);

        pc->v_slli_i16(o.ux, o.uy, 8);
        pc->v_sub_i16(o.ux, o.ux, o.uy);
        pc->v_add_i16(o.ux, o.ux, pc->simd_const(&ct.p_0080008000800080, Bcst::kNA, o.ux));

        pc->v_expand_alpha_16(o.uy, o.uy);
        pc->v_inv255_u16(o.uy, o.uy);
      }
#endif
    }

    // CMaskInit - RGBA32 - Solid - SrcIn | SrcOut
    // -------------------------------------------

    else if (is_src_in() || is_src_out()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = s.uc[0];
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        // Im  = 1   - m
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.vn = vm;

        pc->v_mul_u16(o.ux, s.uc[0], vm);
        pc->v_div255_u16(o.ux);
        pc->v_inv255_u16(vm, vm);
      }
    }

    // CMaskInit - RGBA32 - Solid - SrcAtop & Xor & Darken & Lighten
    // -------------------------------------------------------------

    else if (is_src_atop() || is_xor() || is_darken() || is_lighten()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        // Yca = 1 - Sa
        // Ya  = 1 - Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC | PixelFlags::kUI);

        o.ux = s.uc[0];
        o.uy = s.ui[0];
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        // Yca = 1 - (Sa * m)
        // Ya  = 1 - (Sa * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.uy = vm;

        pc->v_mul_u16(o.ux, s.uc[0], o.uy);
        pc->v_div255_u16(o.ux);

        pc->v_expand_alpha_16(o.uy, o.ux, false);
        pc->v_swizzle_u32x4(o.uy, o.uy, swizzle(0, 0, 0, 0));
        pc->v_inv255_u16(o.uy, o.uy);
      }
    }

    // CMaskInit - RGBA32 - Solid - Dst
    // --------------------------------

    else if (is_dst_copy()) {
      BL_NOT_REACHED();
    }

    // CMaskInit - RGBA32 - Solid - DstOver
    // ------------------------------------

    else if (is_dst_over()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = s.uc[0];
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        pc->v_mul_u16(o.ux, s.uc[0], vm);
        pc->v_div255_u16(o.ux);
      }
    }

    // CMaskInit - RGBA32 - Solid - DstIn
    // ----------------------------------

    else if (is_dst_in()) {
      if (!has_mask) {
        // Xca = Sa
        // Xa  = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA);

        o.ux = s.ua[0];
      }
      else {
        // Xca = 1 - m.(1 - Sa)
        // Xa  = 1 - m.(1 - Sa)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA);

        o.ux = pc->new_similar_reg(s.ua[0], "solid.ux");
        pc->v_mov(o.ux, s.ua[0]);
        pc->v_inv255_u16(o.ux, o.ux);
        pc->v_mul_u16(o.ux, o.ux, vm);
        pc->v_div255_u16(o.ux);
        pc->v_inv255_u16(o.ux, o.ux);
      }
    }

    // CMaskInit - RGBA32 - Solid - DstOut
    // -----------------------------------

    else if (is_dst_out()) {
      if (!has_mask) {
        // Xca = 1 - Sa
        // Xa  = 1 - Sa
        if (use_da) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUI);

          o.ux = s.ui[0];
        }
        // Xca = 1 - Sa
        // Xa  = 1
        else {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA);

          o.ux = pc->new_similar_reg(s.ua[0], "solid.ux");
          pc->v_mov(o.ux, s.ua[0]);
          pc->vNegRgb8W(o.ux, o.ux);
        }
      }
      else {
        // Xca = 1 - (Sa * m)
        // Xa  = 1 - (Sa * m)
        if (use_da) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA);

          o.ux = vm;
          pc->v_mul_u16(o.ux, o.ux, s.ua[0]);
          pc->v_div255_u16(o.ux);
          pc->v_inv255_u16(o.ux, o.ux);
        }
        // Xca = 1 - (Sa * m)
        // Xa  = 1
        else {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUA);

          o.ux = vm;
          pc->v_mul_u16(o.ux, o.ux, s.ua[0]);
          pc->v_div255_u16(o.ux);
          pc->v_inv255_u16(o.ux, o.ux);
          pc->vFillAlpha255W(o.ux, o.ux);
        }
      }
    }

    // CMaskInit - RGBA32 - Solid - DstAtop
    // ------------------------------------

    else if (is_dst_atop()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        // Yca = Sa
        // Ya  = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC | PixelFlags::kUA);

        o.ux = s.uc[0];
        o.uy = s.ua[0];
      }
      else {
        // Xca = Sca.m
        // Xa  = Sa .m
        // Yca = Sa .m + (1 - m)
        // Ya  = Sa .m + (1 - m)

        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");
        pc->v_mul_u16(o.ux, s.uc[0], vm);
        pc->v_inv255_u16(o.uy, vm);
        pc->v_div255_u16(o.ux);
        pc->v_add_i16(o.uy, o.uy, o.ux);
        pc->v_expand_alpha_16(o.uy, o.uy);
      }
    }

    // CMaskInit - RGBA32 - Solid - Plus
    // ---------------------------------

    else if (is_plus()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC);

        o.px = s.pc[0];
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.px = pc->new_similar_reg(s.pc[0], "solid.px");
        pc->v_mul_u16(o.px, s.uc[0], vm);
        pc->v_div255_u16(o.px);
        pc->v_packs_i16_u8(o.px, o.px, o.px);
      }
    }

    // CMaskInit - RGBA32 - Solid - Minus
    // ----------------------------------

    else if (is_minus()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = 0
        // Yca = Sca
        // Ya  = Sa
        if (use_da) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

          o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
          o.uy = s.uc[0];
          pc->v_mov(o.ux, o.uy);
          pc->vZeroAlphaW(o.ux, o.ux);
        }
        else {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC);

          o.px = pc->new_similar_reg(s.pc[0], "solid.px");
          pc->v_mov(o.px, s.pc[0]);
          pc->vZeroAlphaB(o.px, o.px);
        }
      }
      else {
        // Xca = Sca
        // Xa  = 0
        // Yca = Sca
        // Ya  = Sa
        // M   = m       <Alpha channel is set to 256>
        // N   = 1 - m   <Alpha channel is set to 0  >
        if (use_da) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

          o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
          o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");
          o.vm = vm;
          o.vn = pc->new_similar_reg(s.uc[0], "vn");

          pc->vZeroAlphaW(o.ux, s.uc[0]);
          pc->v_mov(o.uy, s.uc[0]);

          pc->v_inv255_u16(o.vn, o.vm);
          pc->vZeroAlphaW(o.vm, o.vm);
          pc->vZeroAlphaW(o.vn, o.vn);
          pc->vFillAlpha255W(o.vm, o.vm);
        }
        else {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

          o.ux = pc->new_similar_reg(s.uc[0], "ux");
          o.vm = vm;
          o.vn = pc->new_similar_reg(s.uc[0], "vn");
          pc->vZeroAlphaW(o.ux, s.uc[0]);
          pc->v_inv255_u16(o.vn, o.vm);
        }
      }
    }

    // CMaskInit - RGBA32 - Solid - Modulate
    // -------------------------------------

    else if (is_modulate()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = s.uc[0];
      }
      else {
        // Xca = Sca * m + (1 - m)
        // Xa  = Sa  * m + (1 - m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        pc->v_mul_u16(o.ux, s.uc[0], vm);
        pc->v_div255_u16(o.ux);
        pc->v_add_i16(o.ux, o.ux, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, o.ux));
        pc->v_sub_i16(o.ux, o.ux, vm);
      }
    }

    // CMaskInit - RGBA32 - Solid - Multiply
    // -------------------------------------

    else if (is_multiply()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        // Yca = Sca + (1 - Sa)
        // Ya  = Sa  + (1 - Sa)
        if (use_da) {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC | PixelFlags::kUI);

          o.ux = s.uc[0];
          o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");

          pc->v_mov(o.uy, s.ui[0]);
          pc->v_add_i16(o.uy, o.uy, o.ux);
        }
        // Yca = Sca + (1 - Sa)
        // Ya  = Sa  + (1 - Sa)
        else {
          src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC | PixelFlags::kUI);

          o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");
          pc->v_mov(o.uy, s.ui[0]);
          pc->v_add_i16(o.uy, o.uy, s.uc[0]);
        }
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        // Yca = Sca * m + (1 - Sa * m)
        // Ya  = Sa  * m + (1 - Sa * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");

        pc->v_mul_u16(o.ux, s.uc[0], vm);
        pc->v_div255_u16(o.ux);
        pc->v_swizzle_lo_u16x4(o.uy, o.ux, swizzle(3, 3, 3, 3));
        pc->v_inv255_u16(o.uy, o.uy);
        pc->v_swizzle_u32x4(o.uy, o.uy, swizzle(0, 0, 0, 0));
        pc->v_add_i16(o.uy, o.uy, o.ux);
      }
    }

    // CMaskInit - RGBA32 - Solid - Screen
    // -----------------------------------

    else if (is_screen()) {
#if defined(BL_JIT_ARCH_A64)
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        // Yca = 1 - Sca
        // Ya  = 1 - Sa
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC | PixelFlags::kImmutable);

        o.px = s.pc[0];
        o.py = pc->new_similar_reg(s.pc[0], "solid.py");

        pc->v_not_u32(o.py, o.px);
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        // Yca = 1 - (Sca * m)
        // Ya  = 1 - (Sa  * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kPC | PixelFlags::kImmutable);

        o.px = pc->new_similar_reg(s.pc[0], "solid.px");
        o.py = pc->new_similar_reg(s.pc[0], "solid.py");

        pc->v_mulw_lo_u8(o.px, s.pc[0], vm);
        CompOpUtils::div255_pack(pc, o.px, o.px);
        pc->v_swizzle_u32x4(o.px, o.px, swizzle(0, 0, 0, 0));

        pc->v_not_u32(o.py, o.px);
      }
#else
      if (!has_mask) {
        // Xca = Sca * 1 + 0.5 <Rounding>
        // Xa  = Sa  * 1 + 0.5 <Rounding>
        // Yca = 1 - Sca
        // Ya  = 1 - Sa

        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");

        pc->v_inv255_u16(o.uy, o.ux);
        pc->v_slli_i16(o.ux, s.uc[0], 8);
        pc->v_sub_i16(o.ux, o.ux, s.uc[0]);
        pc->v_add_i16(o.ux, o.ux, pc->simd_const(&ct.p_0080008000800080, Bcst::kNA, o.ux));
      }
      else {
        // Xca = Sca * m + 0.5 <Rounding>
        // Xa  = Sa  * m + 0.5 <Rounding>
        // Yca = 1 - (Sca * m)
        // Ya  = 1 - (Sa  * m)
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "solid.ux");
        o.uy = pc->new_similar_reg(s.uc[0], "solid.uy");

        pc->v_mul_u16(o.uy, s.uc[0], vm);
        pc->v_div255_u16(o.uy);
        pc->v_slli_i16(o.ux, o.uy, 8);
        pc->v_sub_i16(o.ux, o.ux, o.uy);
        pc->v_add_i16(o.ux, o.ux, pc->simd_const(&ct.p_0080008000800080, Bcst::kNA, o.ux));
        pc->v_inv255_u16(o.uy, o.uy);
      }
#endif
    }

    // CMaskInit - RGBA32 - Solid - LinearBurn & Difference & Exclusion
    // ----------------------------------------------------------------

    else if (is_linear_burn() || is_difference() || is_exclusion()) {
      if (!has_mask) {
        // Xca = Sca
        // Xa  = Sa
        // Yca = Sa
        // Ya  = Sa
        src_part()->as<FetchSolidPart>()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC | PixelFlags::kUA);

        o.ux = s.uc[0];
        o.uy = s.ua[0];
      }
      else {
        // Xca = Sca * m
        // Xa  = Sa  * m
        // Yca = Sa  * m
        // Ya  = Sa  * m
        src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

        o.ux = pc->new_similar_reg(s.uc[0], "ux");
        o.uy = pc->new_similar_reg(s.uc[0], "uy");

        pc->v_mul_u16(o.ux, s.uc[0], vm);
        pc->v_div255_u16(o.ux);
        pc->v_swizzle_lo_u16x4(o.uy, o.ux, swizzle(3, 3, 3, 3));
        pc->v_swizzle_u32x4(o.uy, o.uy, swizzle(0, 0, 0, 0));
      }
    }

    // CMaskInit - RGBA32 - Solid - TypeA (Non-Opaque)
    // -----------------------------------------------

    else if (bl_test_flag(comp_op_flags(), CompOpFlags::kTypeA) && has_mask) {
      // Multiply the source pixel with the mask if `TypeA`.
      src_part()->as<FetchSolidPart>()->init_solid_flags(PixelFlags::kUC);

      Pixel& pre = _solid_pre;
      pre.set_count(PixelCount(1));
      pre.uc.init(pc->new_similar_reg(s.uc[0], "pre.uc"));

      pc->v_mul_u16(pre.uc[0], s.uc[0], vm);
      pc->v_div255_u16(pre.uc[0]);
    }

    // CMaskInit - RGBA32 - Solid - No Optimizations
    // ---------------------------------------------

    else {
      // No optimization. The compositor will simply use the mask provided.
      _mask->vm = vm;
    }
  }
  else {
    _mask->vm = vm;

    // CMaskInit - RGBA32 - NonSolid - SrcCopy
    // ---------------------------------------

    if (is_src_copy()) {
      if (has_mask) {
        _mask->vn = pc->new_similar_reg(vm, "vn");
        if (coverage_format() == PixelCoverageFormat::kPacked)
          pc->v_not_u32(_mask->vn, vm);
        else
          pc->v_inv255_u16(_mask->vn, vm);
      }
    }
  }

  _c_mask_loop_init(has_mask ? CMaskLoopType::kVariant : CMaskLoopType::kOpaque);
}

void CompOpPart::c_mask_fini_rgba32() noexcept {
  if (src_part()->is_solid()) {
    _solid_opt.reset();
    _solid_pre.reset();
  }
  else {
    // TODO: [JIT] ???
  }

  _mask->reset();
  _c_mask_loop_fini();
}

// bl::Pipeline::JIT::CompOpPart - CMask - Proc - RGBA
// ===================================================

static VecArray x_pack_pixels(PipeCompiler* pc, VecArray& src, PixelCount n, const char* name) noexcept {
  VecArray out;

#if defined(BL_JIT_ARCH_X86)
  if (!pc->has_avx()) {
    out = src.even();
    pc->x_packs_i16_u8(out, out, src.odd());
  }
  else
#endif // BL_JIT_ARCH_X86
  {
    FetchUtils::_x_pack_pixel(pc, out, src, uint32_t(n) * 4u, "", name);
  }

  return out;
}

void CompOpPart::c_mask_proc_rgba32_vec(Pixel& out, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  bool has_mask = is_loop_c_mask();

  VecWidth vw = pc->vec_width_of(DataWidth::k64, n);
  size_t full_n = pc->vec_count_of(DataWidth::k64, n);
  uint32_t use_hi = n > PixelCount(1);

  out.set_count(n);

  if (src_part()->is_solid()) {
    Pixel d("d", pixel_type());
    SolidPixel& o = _solid_opt;

    VecArray xv, yv, zv;
    pc->new_vec_array(xv, full_n, vw, "x");
    pc->new_vec_array(yv, full_n, vw, "y");
    pc->new_vec_array(zv, full_n, vw, "z");

    bool use_da = has_da();

    // CMaskProc - RGBA32 - SrcCopy
    // ----------------------------

    if (is_src_copy()) {
      if (!has_mask) {
        // Dca' = Xca
        // Da'  = Xa
        out.pc = VecArray(o.px).clone_as(vw);
        out.make_immutable();
      }
      else {
#if defined(BL_JIT_ARCH_A64)
        dst_fetch(d, n, PixelFlags::kPC, predicate);

        CompOpUtils::mul_u8_widen(pc, xv, d.pc, o.vn, uint32_t(n) * 4u);
        pc->v_add_u16(xv, xv, o.ux);
        CompOpUtils::combineDiv255AndOutRGBA32(pc, out, flags, xv);
#else
        // Dca' = Xca + Dca.(1 - m)
        // Da'  = Xa  + Da .(1 - m)
        dst_fetch(d, n, PixelFlags::kUC, predicate);
        VecArray& dv = d.uc;

        Vec s_ux = o.ux.clone_as(dv[0]);
        Vec s_vn = o.vn.clone_as(dv[0]);

        pc->v_mul_u16(dv, dv, s_vn);
        pc->v_add_i16(dv, dv, s_ux);
        pc->v_mul257_hi_u16(dv, dv);
        out.uc.init(dv);
#endif
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - SrcOver & Screen
    // -------------------------------------

    if (is_src_over() || is_screen()) {
#if defined(BL_JIT_ARCH_A64)
      dst_fetch(d, n, PixelFlags::kPC, predicate);

      CompOpUtils::mul_u8_widen(pc, xv, d.pc, o.py, uint32_t(n) * 4u);
      CompOpUtils::div255_pack(pc, d.pc, xv);
      pc->v_add_u8(d.pc, d.pc, o.px);
      out.pc.init(d.pc);
#else
      // Dca' = Xca + Dca.Yca
      // Da'  = Xa  + Da .Ya
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);
      Vec s_uy = o.uy.clone_as(dv[0]);

      pc->v_mul_u16(dv, dv, s_uy);
      pc->v_add_i16(dv, dv, s_ux);
      pc->v_mul257_hi_u16(dv, dv);

      out.uc.init(dv);
#endif

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - SrcIn
    // --------------------------

    if (is_src_in()) {
      // Dca' = Xca.Da
      // Da'  = Xa .Da
      if (!has_mask) {
        dst_fetch(d, n, PixelFlags::kUA, predicate);
        VecArray& dv = d.ua;

        Vec s_ux = o.ux.clone_as(dv[0]);

        pc->v_mul_u16(dv, dv, s_ux);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      // Dca' = Xca.Da + Dca.(1 - m)
      // Da'  = Xa .Da + Da .(1 - m)
      else {
        dst_fetch(d, n, PixelFlags::kUC | PixelFlags::kUA, predicate);
        VecArray& dv = d.uc;
        VecArray& da = d.ua;

        Vec s_ux = o.ux.clone_as(dv[0]);
        Vec s_vn = o.vn.clone_as(dv[0]);

        pc->v_mul_u16(dv, dv, s_vn);
        pc->v_madd_u16(dv, da, s_ux, dv);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - SrcOut
    // ---------------------------

    if (is_src_out()) {
      // Dca' = Xca.(1 - Da)
      // Da'  = Xa .(1 - Da)
      if (!has_mask) {
        dst_fetch(d, n, PixelFlags::kUI, predicate);
        VecArray& dv = d.ui;

        Vec s_ux = o.ux.clone_as(dv[0]);

        pc->v_mul_u16(dv, dv, s_ux);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      // Dca' = Xca.(1 - Da) + Dca.(1 - m)
      // Da'  = Xa .(1 - Da) + Da .(1 - m)
      else {
        dst_fetch(d, n, PixelFlags::kUC, predicate);
        VecArray& dv = d.uc;

        Vec s_ux = o.ux.clone_as(dv[0]);
        Vec s_vn = o.vn.clone_as(dv[0]);

        pc->v_expand_alpha_16(xv, dv, use_hi);
        pc->v_inv255_u16(xv, xv);
        pc->v_mul_u16(xv, xv, s_ux);
        pc->v_mul_u16(dv, dv, s_vn);
        pc->v_add_i16(dv, dv, xv);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - SrcAtop
    // ----------------------------

    if (is_src_atop()) {
      // Dca' = Xca.Da + Dca.Yca
      // Da'  = Xa .Da + Da .Ya
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);
      Vec s_uy = o.uy.clone_as(dv[0]);

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(dv, dv, s_uy);
      pc->v_mul_u16(xv, xv, s_ux);

      pc->v_add_i16(dv, dv, xv);
      pc->v_div255_u16(dv);

      out.uc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - Dst
    // ------------------------

    if (is_dst_copy()) {
      // Dca' = Dca
      // Da'  = Da
      BL_NOT_REACHED();
    }

    // CMaskProc - RGBA32 - DstOver
    // ----------------------------

    if (is_dst_over()) {
      // Dca' = Xca.(1 - Da) + Dca
      // Da'  = Xa .(1 - Da) + Da
      dst_fetch(d, n, PixelFlags::kPC | PixelFlags::kUI, predicate);
      VecArray& dv = d.ui;

      Vec s_ux = o.ux.clone_as(dv[0]);

      pc->v_mul_u16(dv, dv, s_ux);
      pc->v_div255_u16(dv);

      VecArray dh = x_pack_pixels(pc, dv, n, "d");
      dh = dh.clone_as(d.pc[0]);
      pc->v_add_i32(dh, dh, d.pc);

      out.pc.init(dh);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - DstIn & DstOut
    // -----------------------------------

    if (is_dst_in() || is_dst_out()) {
      // Dca' = Xca.Dca
      // Da'  = Xa .Da
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);

      pc->v_mul_u16(dv, dv, s_ux);
      pc->v_div255_u16(dv);

      out.uc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - DstAtop | Xor | Multiply
    // ---------------------------------------------

    if (is_dst_atop() || is_xor() || is_multiply()) {
      if (use_da) {
        // Dca' = Xca.(1 - Da) + Dca.Yca
        // Da'  = Xa .(1 - Da) + Da .Ya
        dst_fetch(d, n, PixelFlags::kUC, predicate);
        VecArray& dv = d.uc;

        Vec s_ux = o.ux.clone_as(dv[0]);
        Vec s_uy = o.uy.clone_as(dv[0]);

        pc->v_expand_alpha_16(xv, dv, use_hi);
        pc->v_mul_u16(dv, dv, s_uy);
        pc->v_inv255_u16(xv, xv);
        pc->v_mul_u16(xv, xv, s_ux);

        pc->v_add_i16(dv, dv, xv);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      else {
        // Dca' = Dca.Yca
        // Da'  = Da .Ya
        dst_fetch(d, n, PixelFlags::kUC, predicate);
        VecArray& dv = d.uc;

        Vec s_uy = o.uy.clone_as(dv[0]);

        pc->v_mul_u16(dv, dv, s_uy);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - Plus
    // -------------------------

    if (is_plus()) {
      // Dca' = Clamp(Dca + Sca)
      // Da'  = Clamp(Da  + Sa )
      dst_fetch(d, n, PixelFlags::kPC, predicate);
      VecArray& dv = d.pc;

      Vec s_px = o.px.clone_as(dv[0]);

      pc->v_adds_u8(dv, dv, s_px);

      out.pc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - Minus
    // --------------------------

    if (is_minus()) {
      if (!has_mask) {
        if (use_da) {
          // Dca' = Clamp(Dca - Xca) + Yca.(1 - Da)
          // Da'  = Da + Ya.(1 - Da)
          dst_fetch(d, n, PixelFlags::kUC, predicate);
          VecArray& dv = d.uc;

          Vec s_ux = o.ux.clone_as(dv[0]);
          Vec s_uy = o.uy.clone_as(dv[0]);

          pc->v_expand_alpha_16(xv, dv, use_hi);
          pc->v_inv255_u16(xv, xv);
          pc->v_mul_u16(xv, xv, s_uy);
          pc->v_subs_u16(dv, dv, s_ux);
          pc->v_div255_u16(xv);

          pc->v_add_i16(dv, dv, xv);
          out.uc.init(dv);
        }
        else {
          // Dca' = Clamp(Dca - Xca)
          // Da'  = <unchanged>
          dst_fetch(d, n, PixelFlags::kPC, predicate);
          VecArray& dh = d.pc;

          Vec s_px = o.px.clone_as(dh[0]);

          pc->v_subs_u8(dh, dh, s_px);
          out.pc.init(dh);
        }
      }
      else {
        if (use_da) {
          // Dca' = (Clamp(Dca - Xca) + Yca.(1 - Da)).m + Dca.(1 - m)
          // Da'  = Da + Ya.(1 - Da)
          dst_fetch(d, n, PixelFlags::kUC, predicate);
          VecArray& dv = d.uc;

          Vec s_ux = o.ux.clone_as(dv[0]);
          Vec s_uy = o.uy.clone_as(dv[0]);
          Vec s_vn = o.vn.clone_as(dv[0]);
          Vec s_vm = o.vm.clone_as(dv[0]);

          pc->v_expand_alpha_16(xv, dv, use_hi);
          pc->v_inv255_u16(xv, xv);
          pc->v_mul_u16(yv, dv, s_vn);
          pc->v_subs_u16(dv, dv, s_ux);
          pc->v_mul_u16(xv, xv, s_uy);
          pc->v_div255_u16(xv);
          pc->v_add_i16(dv, dv, xv);
          pc->v_mul_u16(dv, dv, s_vm);

          pc->v_add_i16(dv, dv, yv);
          pc->v_div255_u16(dv);
          out.uc.init(dv);
        }
        else {
          // Dca' = Clamp(Dca - Xca).m + Dca.(1 - m)
          // Da'  = <unchanged>
          dst_fetch(d, n, PixelFlags::kUC, predicate);
          VecArray& dv = d.uc;

          Vec s_ux = o.ux.clone_as(dv[0]);
          Vec s_vn = o.vn.clone_as(dv[0]);
          Vec s_vm = o.vm.clone_as(dv[0]);

          pc->v_mul_u16(yv, dv, s_vn);
          pc->v_subs_u16(dv, dv, s_ux);
          pc->v_mul_u16(dv, dv, s_vm);

          pc->v_add_i16(dv, dv, yv);
          pc->v_div255_u16(dv);
          out.uc.init(dv);
        }
      }

      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - Modulate
    // -----------------------------

    if (is_modulate()) {
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);

      // Dca' = Dca.Xca
      // Da'  = Da .Xa
      pc->v_mul_u16(dv, dv, s_ux);
      pc->v_div255_u16(dv);

      if (!use_da)
        pc->vFillAlpha255W(dv, dv);

      out.uc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - Darken & Lighten
    // -------------------------------------

    if (is_darken() || is_lighten()) {
      // Dca' = minmax(Dca + Xca.(1 - Da), Xca + Dca.Yca)
      // Da'  = Xa + Da.Ya
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);
      Vec s_uy = o.uy.clone_as(dv[0]);

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_inv255_u16(xv, xv);
      pc->v_mul_u16(xv, xv, s_ux);
      pc->v_div255_u16(xv);
      pc->v_add_i16(xv, xv, dv);
      pc->v_mul_u16(dv, dv, s_uy);
      pc->v_div255_u16(dv);
      pc->v_add_i16(dv, dv, s_ux);

      if (is_darken())
        pc->v_min_u8(dv, dv, xv);
      else
        pc->v_max_u8(dv, dv, xv);

      out.uc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - LinearBurn
    // -------------------------------

    if (is_linear_burn()) {
      // Dca' = Dca + Xca - Yca.Da
      // Da'  = Da  + Xa  - Ya .Da
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);
      Vec s_uy = o.uy.clone_as(dv[0]);

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(xv, xv, s_uy);
      pc->v_add_i16(dv, dv, s_ux);
      pc->v_div255_u16(xv);
      pc->v_subs_u16(dv, dv, xv);

      out.uc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - Difference
    // -------------------------------

    if (is_difference()) {
      // Dca' = Dca + Sca - 2.min(Sca.Da, Dca.Sa)
      // Da'  = Da  + Sa  -   min(Sa .Da, Da .Sa)
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);
      Vec s_uy = o.uy.clone_as(dv[0]);

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(yv, s_uy, dv);
      pc->v_mul_u16(xv, xv, s_ux);
      pc->v_add_i16(dv, dv, s_ux);
      pc->v_min_u16(yv, yv, xv);
      pc->v_div255_u16(yv);
      pc->v_sub_i16(dv, dv, yv);
      pc->vZeroAlphaW(yv, yv);
      pc->v_sub_i16(dv, dv, yv);

      out.uc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }

    // CMaskProc - RGBA32 - Exclusion
    // ------------------------------

    if (is_exclusion()) {
      // Dca' = Dca + Xca - 2.Xca.Dca
      // Da'  = Da + Xa - Xa.Da
      dst_fetch(d, n, PixelFlags::kUC, predicate);
      VecArray& dv = d.uc;

      Vec s_ux = o.ux.clone_as(dv[0]);

      pc->v_mul_u16(xv, dv, s_ux);
      pc->v_add_i16(dv, dv, s_ux);
      pc->v_div255_u16(xv);
      pc->v_sub_i16(dv, dv, xv);
      pc->vZeroAlphaW(xv, xv);
      pc->v_sub_i16(dv, dv, xv);

      out.uc.init(dv);
      FetchUtils::satisfy_pixels(pc, out, flags);
      return;
    }
  }

  VecArray vm;
  if (_mask->vm.is_valid()) {
    vm.init(_mask->vm);
  }

  v_mask_proc_rgba32_vec(out, n, flags, vm, PixelCoverageFlags::kImmutable, predicate);
}

// bl::Pipeline::JIT::CompOpPart - VMask - RGBA32 (Vec)
// ====================================================

void CompOpPart::v_mask_proc_rgba32_vec(Pixel& out, PixelCount n, PixelFlags flags, const VecArray& vm_, PixelCoverageFlags coverage_flags, PixelPredicate& predicate) noexcept {
  VecWidth vw = pc->vec_width_of(DataWidth::k64, n);
  size_t full_n = pc->vec_count_of(DataWidth::k64, n);

  const uint32_t use_hi = n > PixelCount(1);
  const uint32_t n_split = full_n == 1u ? 1u : 2u;

  VecArray vm = vm_.clone_as(vw);
  bool has_mask = !vm.is_empty();

  bool use_da = has_da();
  bool use_sa = has_sa() || is_loop_c_mask() || has_mask;

  VecArray xv, yv, zv;
  pc->new_vec_array(xv, full_n, vw, "x");
  pc->new_vec_array(yv, full_n, vw, "y");
  pc->new_vec_array(zv, full_n, vw, "z");

  Pixel d("d", PixelType::kRGBA32);
  Pixel s("s", PixelType::kRGBA32);

  out.set_count(n);

  // VMaskProc - RGBA32 - SrcCopy
  // ----------------------------

  if (is_src_copy()) {
    // Composition:
    //   Da - Optional.
    //   Sa - Optional.

    if (!has_mask) {
      // Dca' = Sca
      // Da'  = Sa
      src_fetch(out, n, flags, predicate);
    }
    else {
      // Dca' = Sca.m + Dca.(1 - m)
      // Da'  = Sa .m + Da .(1 - m)
#if defined(BL_JIT_ARCH_A64)
      src_fetch(s, n, PixelFlags::kPC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPC, predicate);

      VecArray& vs = s.pc;
      VecArray& vd = d.pc;
      VecArray vn;

      CompOpUtils::mul_u8_widen(pc, xv, vs, vm, uint32_t(n) * 4u);
      v_mask_proc_rgba32_invert_mask(vn, vm, coverage_flags);

      CompOpUtils::madd_u8_widen(pc, xv, vd, vn, uint32_t(n) * 4u);
      v_mask_proc_rgba32_invert_done(vn, vm, coverage_flags);

      CompOpUtils::combineDiv255AndOutRGBA32(pc, out, flags, xv);
#else
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& vs = s.uc;
      VecArray& vd = d.uc;

      pc->v_mul_u16(vs, vs, vm);

      VecArray vn;
      v_mask_proc_rgba32_invert_mask(vn, vm, coverage_flags);

      pc->v_mul_u16(vd, vd, vn);
      pc->v_add_i16(vd, vd, vs);
      v_mask_proc_rgba32_invert_done(vn, vm, coverage_flags);

      pc->v_div255_u16(vd);
      out.uc.init(vd);
#endif
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - SrcOver
  // ----------------------------

  if (is_src_over()) {
    // Composition:
    //   Da - Optional.
    //   Sa - Required, otherwise SRC_COPY.

#if defined(BL_JIT_ARCH_A64)
    if (!has_mask) {
      src_fetch(s, n, PixelFlags::kPC | PixelFlags::kPI | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPC, predicate);

      CompOpUtils::mul_u8_widen(pc, xv, d.pc, s.pi, uint32_t(n) * 4u);
      CompOpUtils::div255_pack(pc, d.pc, xv);
      pc->v_add_u8(d.pc, d.pc, s.pc);
      out.pc.init(d.pc);
    }
    else {
      src_fetch(s, n, PixelFlags::kPC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPC, predicate);

      VecArray xv_half = xv.half();
      VecArray yv_half = yv.half();

      CompOpUtils::mul_u8_widen(pc, xv, s.pc, vm, uint32_t(n) * 4u);
      CompOpUtils::div255_pack(pc, xv_half, xv);

      pc->v_swizzlev_u8(yv_half, xv_half, pc->simd_vec_const(&ct.swizu8_3xxx2xxx1xxx0xxx_to_3333222211110000, Bcst::kNA, yv_half));
      pc->v_not_u32(yv_half, yv_half);

      CompOpUtils::mul_u8_widen(pc, zv, d.pc, yv_half, uint32_t(n) * 4u);
      CompOpUtils::div255_pack(pc, d.pc, zv);
      pc->v_add_u8(d.pc, d.pc, xv_half);
      out.pc.init(d.pc);
    }
#else
    if (!has_mask) {
      // Dca' = Sca + Dca.(1 - Sa)
      // Da'  = Sa  + Da .(1 - Sa)
      src_fetch(s, n, PixelFlags::kPC | PixelFlags::kUI | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& uv = s.ui;
      VecArray& dv = d.uc;

      pc->v_mul_u16(dv, dv, uv);
      pc->v_div255_u16(dv);

      VecArray dh = x_pack_pixels(pc, dv, n, "d");
      dh = dh.clone_as(s.pc[0]);
      pc->v_add_i32(dh, dh, s.pc);

      out.pc.init(dh);
    }
    else {
      // Dca' = Sca.m + Dca.(1 - Sa.m)
      // Da'  = Sa .m + Da .(1 - Sa.m)
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.uc;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);

      pc->v_expand_alpha_16(xv, sv, use_hi);
      pc->v_inv255_u16(xv, xv);
      pc->v_mul_u16(dv, dv, xv);
      pc->v_div255_u16(dv);

      pc->v_add_i16(dv, dv, sv);
      out.uc.init(dv);
    }
#endif

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - SrcIn
  // --------------------------

  if (is_src_in()) {
    // Composition:
    //   Da - Required, otherwise SRC_COPY.
    //   Sa - Optional.

    if (!has_mask) {
      // Dca' = Sca.Da
      // Da'  = Sa .Da
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUA, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.ua;

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }
    else {
      // Dca' = Sca.m.Da + Dca.(1 - m)
      // Da'  = Sa .m.Da + Da .(1 - m)
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.uc;

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(xv, xv, sv);
      pc->v_div255_u16(xv);
      pc->v_mul_u16(xv, xv, vm);

      VecArray vn;
      v_mask_proc_rgba32_invert_mask(vn, vm, coverage_flags);

      pc->v_mul_u16(dv, dv, vn);
      v_mask_proc_rgba32_invert_done(vn, vm, coverage_flags);

      pc->v_add_i16(dv, dv, xv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - SrcOut
  // ---------------------------

  if (is_src_out()) {
    // Composition:
    //   Da - Required, otherwise CLEAR.
    //   Sa - Optional.

    if (!has_mask) {
      // Dca' = Sca.(1 - Da)
      // Da'  = Sa .(1 - Da)
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUI, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.ui;

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }
    else {
      // Dca' = Sca.(1 - Da).m + Dca.(1 - m)
      // Da'  = Sa .(1 - Da).m + Da .(1 - m)
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.uc;

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_inv255_u16(xv, xv);

      pc->v_mul_u16(xv, xv, sv);
      pc->v_div255_u16(xv);
      pc->v_mul_u16(xv, xv, vm);

      VecArray vn;
      v_mask_proc_rgba32_invert_mask(vn, vm, coverage_flags);

      pc->v_mul_u16(dv, dv, vn);
      v_mask_proc_rgba32_invert_done(vn, vm, coverage_flags);

      pc->v_add_i16(dv, dv, xv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - SrcAtop
  // ----------------------------

  if (is_src_atop()) {
    // Composition:
    //   Da - Required.
    //   Sa - Required.

    if (!has_mask) {
      // Dca' = Sca.Da + Dca.(1 - Sa)
      // Da'  = Sa .Da + Da .(1 - Sa) = Da
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kUI | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& uv = s.ui;
      VecArray& dv = d.uc;

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(dv, dv, uv);
      pc->v_mul_u16(xv, xv, sv);
      pc->v_add_i16(dv, dv, xv);
      pc->v_div255_u16(dv);

      out.uc.init(dv);
    }
    else {
      // Dca' = Sca.Da.m + Dca.(1 - Sa.m)
      // Da'  = Sa .Da.m + Da .(1 - Sa.m) = Da
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.uc;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);

      pc->v_expand_alpha_16(xv, sv, use_hi);
      pc->v_inv255_u16(xv, xv);
      pc->v_expand_alpha_16(yv, dv, use_hi);
      pc->v_mul_u16(dv, dv, xv);
      pc->v_mul_u16(yv, yv, sv);
      pc->v_add_i16(dv, dv, yv);
      pc->v_div255_u16(dv);

      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Dst
  // ------------------------

  if (is_dst_copy()) {
    // Dca' = Dca
    // Da'  = Da
    BL_NOT_REACHED();
  }

  // VMaskProc - RGBA32 - DstOver
  // ----------------------------

  if (is_dst_over()) {
    // Composition:
    //   Da - Required, otherwise DST_COPY.
    //   Sa - Optional.

    if (!has_mask) {
      // Dca' = Dca + Sca.(1 - Da)
      // Da'  = Da  + Sa .(1 - Da)
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPC | PixelFlags::kUI, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.ui;

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);

      VecArray dh = x_pack_pixels(pc, dv, n, "d");
      dh = dh.clone_as(d.pc[0]);
      pc->v_add_i32(dh, dh, d.pc);

      out.pc.init(dh);
    }
    else {
      // Dca' = Dca + Sca.m.(1 - Da)
      // Da'  = Da  + Sa .m.(1 - Da)
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kPC | PixelFlags::kUI, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.ui;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);

      VecArray dh = x_pack_pixels(pc, dv, n, "d");
      dh = dh.clone_as(d.pc[0]);
      pc->v_add_i32(dh, dh, d.pc);

      out.pc.init(dh);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - DstIn
  // --------------------------

  if (is_dst_in()) {
    // Composition:
    //   Da - Optional.
    //   Sa - Required, otherwise DST_COPY.

    if (!has_mask) {
      // Dca' = Dca.Sa
      // Da'  = Da .Sa
      src_fetch(s, n, PixelFlags::kUA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.ua;
      VecArray& dv = d.uc;

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }
    else {
      // Dca' = Dca.(1 - m.(1 - Sa))
      // Da'  = Da .(1 - m.(1 - Sa))
      src_fetch(s, n, PixelFlags::kUI, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.ui;
      VecArray& dv = d.uc;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
      pc->v_inv255_u16(sv, sv);

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - DstOut
  // ---------------------------

  if (is_dst_out()) {
    // Composition:
    //   Da - Optional.
    //   Sa - Required, otherwise CLEAR.

    if (!has_mask) {
      // Dca' = Dca.(1 - Sa)
      // Da'  = Da .(1 - Sa)
      src_fetch(s, n, PixelFlags::kUI | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.ui;
      VecArray& dv = d.uc;

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }
    else {
      // Dca' = Dca.(1 - Sa.m)
      // Da'  = Da .(1 - Sa.m)
      src_fetch(s, n, PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.ua;
      VecArray& dv = d.uc;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
      pc->v_inv255_u16(sv, sv);

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    if (!use_da)
      FetchUtils::fill_alpha_channel(pc, out);
    return;
  }

  // VMaskProc - RGBA32 - DstAtop
  // ----------------------------

  if (is_dst_atop()) {
    // Composition:
    //   Da - Required.
    //   Sa - Required.

    if (!has_mask) {
      // Dca' = Dca.Sa + Sca.(1 - Da)
      // Da'  = Da .Sa + Sa .(1 - Da)
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kUA | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& uv = s.ua;
      VecArray& dv = d.uc;

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(dv, dv, uv);
      pc->v_inv255_u16(xv, xv);
      pc->v_mul_u16(xv, xv, sv);

      pc->v_add_i16(dv, dv, xv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }
    else {
      // Dca' = Dca.(1 - m.(1 - Sa)) + Sca.m.(1 - Da)
      // Da'  = Da .(1 - m.(1 - Sa)) + Sa .m.(1 - Da)
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kUI, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& uv = s.ui;
      VecArray& dv = d.uc;

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(sv, sv, vm);
      pc->v_mul_u16(uv, uv, vm);

      pc->v_div255_u16(sv);
      pc->v_div255_u16(uv);
      pc->v_inv255_u16(xv, xv);
      pc->v_inv255_u16(uv, uv);
      pc->v_mul_u16(xv, xv, sv);
      pc->v_mul_u16(dv, dv, uv);

      pc->v_add_i16(dv, dv, xv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Xor
  // ------------------------

  if (is_xor()) {
    // Composition:
    //   Da - Required.
    //   Sa - Required.

    if (!has_mask) {
      // Dca' = Dca.(1 - Sa) + Sca.(1 - Da)
      // Da'  = Da .(1 - Sa) + Sa .(1 - Da)
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kUI | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& uv = s.ui;
      VecArray& dv = d.uc;

      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_mul_u16(dv, dv, uv);
      pc->v_inv255_u16(xv, xv);
      pc->v_mul_u16(xv, xv, sv);

      pc->v_add_i16(dv, dv, xv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }
    else {
      // Dca' = Dca.(1 - Sa.m) + Sca.m.(1 - Da)
      // Da'  = Da .(1 - Sa.m) + Sa .m.(1 - Da)
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.uc;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);

      pc->v_expand_alpha_16(xv, sv, use_hi);
      pc->v_expand_alpha_16(yv, dv, use_hi);
      pc->v_inv255_u16(xv, xv);
      pc->v_inv255_u16(yv, yv);
      pc->v_mul_u16(dv, dv, xv);
      pc->v_mul_u16(sv, sv, yv);

      pc->v_add_i16(dv, dv, sv);
      pc->v_div255_u16(dv);
      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Plus
  // -------------------------

  if (is_plus()) {
    if (!has_mask) {
      // Dca' = Clamp(Dca + Sca)
      // Da'  = Clamp(Da  + Sa )
      src_fetch(s, n, PixelFlags::kPC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kPC, predicate);

      VecArray& sh = s.pc;
      VecArray& dh = d.pc;

      pc->v_adds_u8(dh, dh, sh);
      out.pc.init(dh);
    }
    else {
      // Dca' = Clamp(Dca + Sca.m)
      // Da'  = Clamp(Da  + Sa .m)
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kPC, predicate);

      VecArray& sv = s.uc;
      VecArray& dh = d.pc;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);

      VecArray sh = x_pack_pixels(pc, sv, n, "s");
      pc->v_adds_u8(dh, dh, sh.clone_as(dh[0]));

      out.pc.init(dh);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Minus
  // --------------------------

  if (is_minus()) {
    if (!has_mask) {
      // Dca' = Clamp(Dca - Sca) + Sca.(1 - Da)
      // Da'  = Da + Sa.(1 - Da)
      if (use_da) {
        src_fetch(s, n, PixelFlags::kUC, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_expand_alpha_16(xv, dv, use_hi);
        pc->v_inv255_u16(xv, xv);
        pc->v_mul_u16(xv, xv, sv);
        pc->vZeroAlphaW(sv, sv);
        pc->v_div255_u16(xv);

        pc->v_subs_u16(dv, dv, sv);
        pc->v_add_i16(dv, dv, xv);
        out.uc.init(dv);
      }
      // Dca' = Clamp(Dca - Sca)
      // Da'  = <unchanged>
      else {
        src_fetch(s, n, PixelFlags::kPC, predicate);
        dst_fetch(d, n, PixelFlags::kPC, predicate);

        VecArray& sh = s.pc;
        VecArray& dh = d.pc;

        pc->vZeroAlphaB(sh, sh);
        pc->v_subs_u8(dh, dh, sh);

        out.pc.init(dh);
      }
    }
    else {
      // Dca' = (Clamp(Dca - Sca) + Sca.(1 - Da)).m + Dca.(1 - m)
      // Da'  = Da + Sa.m(1 - Da)
      if (use_da) {
        src_fetch(s, n, PixelFlags::kUC, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_expand_alpha_16(xv, dv, use_hi);
        pc->v_mov(yv, dv);
        pc->v_inv255_u16(xv, xv);
        pc->v_subs_u16(dv, dv, sv);
        pc->v_mul_u16(sv, sv, xv);

        pc->vZeroAlphaW(dv, dv);
        pc->v_div255_u16(sv);
        pc->v_add_i16(dv, dv, sv);
        pc->v_mul_u16(dv, dv, vm);

        pc->vZeroAlphaW(vm, vm);
        pc->v_inv255_u16(vm, vm);

        pc->v_mul_u16(yv, yv, vm);

        if (bl_test_flag(coverage_flags, PixelCoverageFlags::kImmutable)) {
          pc->v_inv255_u16(vm[0], vm[0]);
          pc->v_swizzle_u32x4(vm[0], vm[0], swizzle(2, 2, 0, 0));
        }

        pc->v_add_i16(dv, dv, yv);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      // Dca' = Clamp(Dca - Sca).m + Dca.(1 - m)
      // Da'  = <unchanged>
      else {
        src_fetch(s, n, PixelFlags::kUC, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_inv255_u16(xv, vm);
        pc->vZeroAlphaW(sv, sv);

        pc->v_mul_u16(xv, xv, dv);
        pc->v_subs_u16(dv, dv, sv);
        pc->v_mul_u16(dv, dv, vm);

        pc->v_add_i16(dv, dv, xv);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Modulate
  // -----------------------------

  if (is_modulate()) {
    VecArray& dv = d.uc;
    VecArray& sv = s.uc;

    if (!has_mask) {
      // Dca' = Dca.Sca
      // Da'  = Da .Sa
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);
    }
    else {
      // Dca' = Dca.(Sca.m + 1 - m)
      // Da'  = Da .(Sa .m + 1 - m)
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
      pc->v_add_i16(sv, sv, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, sv));
      pc->v_sub_i16(sv, sv, vm);
      pc->v_mul_u16(dv, dv, sv);
      pc->v_div255_u16(dv);

      out.uc.init(dv);
    }

    if (!use_da)
      pc->vFillAlpha255W(dv, dv);

    out.uc.init(dv);
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Multiply
  // -----------------------------

  if (is_multiply()) {
    if (!has_mask) {
      if (use_da && use_sa) {
        // Dca' = Dca.(Sca + 1 - Sa) + Sca.(1 - Da)
        // Da'  = Da .(Sa  + 1 - Sa) + Sa .(1 - Da)
        src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        // SPLIT.
        for (unsigned int i = 0; i < n_split; i++) {
          VecArray sh = sv.even_odd(i);
          VecArray dh = dv.even_odd(i);
          VecArray xh = xv.even_odd(i);
          VecArray yh = yv.even_odd(i);

          pc->v_expand_alpha_16(yh, sh, use_hi);
          pc->v_expand_alpha_16(xh, dh, use_hi);
          pc->v_inv255_u16(yh, yh);
          pc->v_add_i16(yh, yh, sh);
          pc->v_inv255_u16(xh, xh);
          pc->v_mul_u16(dh, dh, yh);
          pc->v_mul_u16(xh, xh, sh);
          pc->v_add_i16(dh, dh, xh);
        }

        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      else if (use_da) {
        // Dca' = Sc.(Dca + 1 - Da)
        // Da'  = 1 .(Da  + 1 - Da) = 1
        src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_expand_alpha_16(xv, dv, use_hi);
        pc->v_inv255_u16(xv, xv);
        pc->v_add_i16(dv, dv, xv);
        pc->v_mul_u16(dv, dv, sv);

        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      else if (has_sa()) {
        // Dc'  = Dc.(Sca + 1 - Sa)
        // Da'  = Da.(Sa  + 1 - Sa)
        src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_expand_alpha_16(xv, sv, use_hi);
        pc->v_inv255_u16(xv, xv);
        pc->v_add_i16(xv, xv, sv);
        pc->v_mul_u16(dv, dv, xv);

        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      else {
        // Dc' = Dc.Sc
        src_fetch(s, n, PixelFlags::kUC | PixelFlags::kImmutable, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_mul_u16(dv, dv, sv);
        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
    }
    else {
      if (use_da) {
        // Dca' = Dca.(Sca.m + 1 - Sa.m) + Sca.m(1 - Da)
        // Da'  = Da .(Sa .m + 1 - Sa.m) + Sa .m(1 - Da)
        src_fetch(s, n, PixelFlags::kUC, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_mul_u16(sv, sv, vm);
        pc->v_div255_u16(sv);

        // SPLIT.
        for (unsigned int i = 0; i < n_split; i++) {
          VecArray sh = sv.even_odd(i);
          VecArray dh = dv.even_odd(i);
          VecArray xh = xv.even_odd(i);
          VecArray yh = yv.even_odd(i);

          pc->v_expand_alpha_16(yh, sh, use_hi);
          pc->v_expand_alpha_16(xh, dh, use_hi);
          pc->v_inv255_u16(yh, yh);
          pc->v_add_i16(yh, yh, sh);
          pc->v_inv255_u16(xh, xh);
          pc->v_mul_u16(dh, dh, yh);
          pc->v_mul_u16(xh, xh, sh);
          pc->v_add_i16(dh, dh, xh);
        }

        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
      else {
        src_fetch(s, n, PixelFlags::kUC, predicate);
        dst_fetch(d, n, PixelFlags::kUC, predicate);

        VecArray& sv = s.uc;
        VecArray& dv = d.uc;

        pc->v_mul_u16(sv, sv, vm);
        pc->v_div255_u16(sv);

        pc->v_expand_alpha_16(xv, sv, use_hi);
        pc->v_inv255_u16(xv, xv);
        pc->v_add_i16(xv, xv, sv);
        pc->v_mul_u16(dv, dv, xv);

        pc->v_div255_u16(dv);
        out.uc.init(dv);
      }
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Overlay
  // ----------------------------

  if (is_overlay()) {
    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
      use_sa = true;
    }

    if (use_sa) {
      // if (2.Dca < Da)
      //   Dca' = Dca + Sca - (Dca.Sa + Sca.Da - 2.Sca.Dca)
      //   Da'  = Da  + Sa  - (Da .Sa + Sa .Da - 2.Sa .Da ) - Sa.Da
      //   Da'  = Da  + Sa  - Sa.Da
      // else
      //   Dca' = Dca + Sca + (Dca.Sa + Sca.Da - 2.Sca.Dca) - Sa.Da
      //   Da'  = Da  + Sa  + (Da .Sa + Sa .Da - 2.Sa .Da ) - Sa.Da
      //   Da'  = Da  + Sa  - Sa.Da

      for (unsigned int i = 0; i < n_split; i++) {
        VecArray sh = sv.even_odd(i);
        VecArray dh = dv.even_odd(i);

        VecArray xh = xv.even_odd(i);
        VecArray yh = yv.even_odd(i);
        VecArray zh = zv.even_odd(i);

        if (!use_da)
          pc->vFillAlpha255W(dh, dh);

        pc->v_expand_alpha_16(xh, dh, use_hi);
        pc->v_expand_alpha_16(yh, sh, use_hi);

        pc->v_mul_u16(xh, xh, sh);                                 // Sca.Da
        pc->v_mul_u16(yh, yh, dh);                                 // Dca.Sa
        pc->v_mul_u16(zh, dh, sh);                                 // Dca.Sca

        pc->v_add_i16(sh, sh, dh);                                 // Dca + Sca
        pc->v_sub_i16(xh, xh, zh);                                 // Sca.Da - Dca.Sca
        pc->vZeroAlphaW(zh, zh);
        pc->v_add_i16(xh, xh, yh);                                 // Dca.Sa + Sca.Da - Dca.Sca
        pc->v_expand_alpha_16(yh, dh, use_hi);                     // Da
        pc->v_sub_i16(xh, xh, zh);                                 // [C=Dca.Sa + Sca.Da - 2.Dca.Sca] [A=Sa.Da]

        pc->v_slli_i16(dh, dh, 1);                                 // 2.Dca
        pc->v_cmp_gt_i16(yh, yh, dh);                              // 2.Dca < Da
        pc->v_div255_u16(xh);
        pc->v_or_i64(yh, yh, pc->simd_const(&ct.p_FFFF000000000000, Bcst::k64, yh));

        pc->v_expand_alpha_16(zh, xh, use_hi);
        // if (2.Dca < Da)
        //   X = [C = -(Dca.Sa + Sca.Da - 2.Sca.Dca)] [A = -Sa.Da]
        // else
        //   X = [C =  (Dca.Sa + Sca.Da - 2.Sca.Dca)] [A = -Sa.Da]
        pc->v_xor_i32(xh, xh, yh);
        pc->v_sub_i16(xh, xh, yh);

        // if (2.Dca < Da)
        //   Y = [C = 0] [A = 0]
        // else
        //   Y = [C = Sa.Da] [A = 0]
        pc->v_bic_i32(yh, zh, yh);

        pc->v_add_i16(sh, sh, xh);
        pc->v_sub_i16(sh, sh, yh);
      }

      out.uc.init(sv);
    }
    else if (use_da) {
      // if (2.Dca < Da)
      //   Dca' = Sc.(1 + 2.Dca - Da)
      //   Da'  = 1
      // else
      //   Dca' = 2.Dca - Da + Sc.(1 - (2.Dca - Da))
      //   Da'  = 1

      pc->v_expand_alpha_16(xv, dv, use_hi);                       // Da
      pc->v_slli_i16(dv, dv, 1);                                   // 2.Dca

      pc->v_cmp_gt_i16(yv, xv, dv);                                //  (2.Dca < Da) ? -1 : 0
      pc->v_sub_i16(xv, xv, dv);                                   // -(2.Dca - Da)

      pc->v_xor_i32(xv, xv, yv);
      pc->v_sub_i16(xv, xv, yv);                                   // 2.Dca < Da ? 2.Dca - Da : -(2.Dca - Da)
      pc->v_bic_i32(yv, xv, yv);                                   // 2.Dca < Da ? 0          : -(2.Dca - Da)
      pc->v_add_i16(xv, xv, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, xv));

      pc->v_mul_u16(xv, xv, sv);
      pc->v_div255_u16(xv);
      pc->v_sub_i16(xv, xv, yv);

      out.uc.init(xv);
    }
    else {
      // if (2.Dc < 1)
      //   Dc'  = 2.Dc.Sc
      // else
      //   Dc'  = 2.Dc + 2.Sc - 1 - 2.Dc.Sc

      pc->v_mul_u16(xv, dv, sv);                                                                 // Dc.Sc
      pc->v_cmp_gt_i16(yv, dv, pc->simd_const(&ct.p_007F007F007F007F, Bcst::kNA, yv));           // !(2.Dc < 1)
      pc->v_add_i16(dv, dv, sv);                                                                 // Dc + Sc
      pc->v_div255_u16(xv);

      pc->v_slli_i16(dv, dv, 1);                                                                 // 2.Dc + 2.Sc
      pc->v_slli_i16(xv, xv, 1);                                                                 // 2.Dc.Sc
      pc->v_sub_i16(dv, dv, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, dv));              // 2.Dc + 2.Sc - 1

      pc->v_xor_i32(xv, xv, yv);
      pc->v_and_i32(dv, dv, yv);                                                                 // 2.Dc < 1 ? 0 : 2.Dc + 2.Sc - 1
      pc->v_sub_i16(xv, xv, yv);                                                                 // 2.Dc < 1 ? 2.Dc.Sc : -2.Dc.Sc
      pc->v_add_i16(dv, dv, xv);                                                                 // 2.Dc < 1 ? 2.Dc.Sc : 2.Dc + 2.Sc - 1 - 2.Dc.Sc

      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Screen
  // ---------------------------

  if (is_screen()) {
#if defined(BL_JIT_ARCH_A64)
    src_fetch(s, n, PixelFlags::kPC | PixelFlags::kImmutable, predicate);
    dst_fetch(d, n, PixelFlags::kPC, predicate);

    VecArray xv_half = xv.half();
    VecArray yv_half = yv.half();

    VecArray src = s.pc;

    if (has_mask) {
      CompOpUtils::mul_u8_widen(pc, xv, src, vm, uint32_t(n) * 4u);
      CompOpUtils::div255_pack(pc, xv_half, xv);
      src = xv_half;
    }

    pc->v_not_u32(yv_half, src);

    CompOpUtils::mul_u8_widen(pc, zv, d.pc, yv_half, uint32_t(n) * 4u);
    CompOpUtils::div255_pack(pc, d.pc, zv);

    pc->v_add_u8(d.pc, d.pc, src);
    out.pc.init(d.pc);
#else
    // Dca' = Sca + Dca.(1 - Sca)
    // Da'  = Sa  + Da .(1 - Sa)
    src_fetch(s, n, PixelFlags::kUC | (has_mask ? PixelFlags::kNone : PixelFlags::kImmutable), predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
    }

    pc->v_inv255_u16(xv, sv);
    pc->v_mul_u16(dv, dv, xv);
    pc->v_div255_u16(dv);
    pc->v_add_i16(dv, dv, sv);
    out.uc.init(dv);
#endif

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Darken & Lighten
  // -------------------------------------

  if (is_darken() || is_lighten()) {
    UniOpVVV min_or_max = is_darken() ? UniOpVVV::kMinU8 : UniOpVVV::kMaxU8;

    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
      use_sa = true;
    }

    if (use_sa && use_da) {
      // Dca' = minmax(Dca + Sca.(1 - Da), Sca + Dca.(1 - Sa))
      // Da'  = Sa + Da.(1 - Sa)
      for (unsigned int i = 0; i < n_split; i++) {
        VecArray sh = sv.even_odd(i);
        VecArray dh = dv.even_odd(i);
        VecArray xh = xv.even_odd(i);
        VecArray yh = yv.even_odd(i);

        pc->v_expand_alpha_16(xh, dh, use_hi);
        pc->v_expand_alpha_16(yh, sh, use_hi);

        pc->v_inv255_u16(xh, xh);
        pc->v_inv255_u16(yh, yh);

        pc->v_mul_u16(xh, xh, sh);
        pc->v_mul_u16(yh, yh, dh);
        pc->v_div255_u16_2x(xh, yh);

        pc->v_add_i16(dh, dh, xh);
        pc->v_add_i16(sh, sh, yh);

        pc->emit_3v(min_or_max, dh, dh, sh);
      }

      out.uc.init(dv);
    }
    else if (use_da) {
      // Dca' = minmax(Dca + Sc.(1 - Da), Sc)
      // Da'  = 1
      pc->v_expand_alpha_16(xv, dv, use_hi);
      pc->v_inv255_u16(xv, xv);
      pc->v_mul_u16(xv, xv, sv);
      pc->v_div255_u16(xv);
      pc->v_add_i16(dv, dv, xv);
      pc->emit_3v(min_or_max, dv, dv, sv);

      out.uc.init(dv);
    }
    else if (use_sa) {
      // Dc' = minmax(Dc, Sca + Dc.(1 - Sa))
      pc->v_expand_alpha_16(xv, sv, use_hi);
      pc->v_inv255_u16(xv, xv);
      pc->v_mul_u16(xv, xv, dv);
      pc->v_div255_u16(xv);
      pc->v_add_i16(xv, xv, sv);
      pc->emit_3v(min_or_max, dv, dv, xv);

      out.uc.init(dv);
    }
    else {
      // Dc' = minmax(Dc, Sc)
      pc->emit_3v(min_or_max, dv, dv, sv);

      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - ColorDodge (SCALAR)
  // ----------------------------------------

  if (is_color_dodge() && n == PixelCount(1)) {
    // Dca' = min(Dca.Sa.Sa / max(Sa - Sca, 0.001), Sa.Da) + Sca.(1 - Da) + Dca.(1 - Sa);
    // Da'  = min(Da .Sa.Sa / max(Sa - Sa , 0.001), Sa.Da) + Sa .(1 - Da) + Da .(1 - Sa);

    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kPC, predicate);

    Vec& s0 = s.uc[0];
    Vec& d0 = d.pc[0];
    Vec& x0 = xv[0];
    Vec& y0 = yv[0];
    Vec& z0 = zv[0];

    if (has_mask) {
      pc->v_mul_u16(s0, s0, vm[0]);
      pc->v_div255_u16(s0);
    }

    pc->v_cvt_u8_to_u32(d0, d0);
    pc->v_cvt_u16_lo_to_u32(s0, s0);

    pc->v_cvt_i32_to_f32(y0, s0);
    pc->v_cvt_i32_to_f32(z0, d0);
    pc->v_packs_i32_i16(d0, d0, s0);

    pc->vExpandAlphaPS(x0, y0);
    pc->v_xor_f32(y0, y0, pc->simd_const(&ct.p_8000000080000000, Bcst::k32, y0));
    pc->v_mul_f32(z0, z0, x0);
    pc->v_and_f32(y0, y0, pc->simd_const(&ct.p_FFFFFFFF_FFFFFFFF_FFFFFFFF_0, Bcst::kNA, y0));
    pc->v_add_f32(y0, y0, x0);

    pc->v_max_f32(y0, y0, pc->simd_const(&ct.f32_1e_m3, Bcst::k32, y0));
    pc->v_div_f32(z0, z0, y0);

    pc->v_swizzle_u32x4(s0, d0, swizzle(1, 1, 3, 3));
    pc->vExpandAlphaHi16(s0, s0);
    pc->vExpandAlphaLo16(s0, s0);
    pc->v_inv255_u16(s0, s0);
    pc->v_mul_u16(d0, d0, s0);
    pc->v_swizzle_u32x4(s0, d0, swizzle(1, 0, 3, 2));
    pc->v_add_i16(d0, d0, s0);

    pc->v_mul_f32(z0, z0, x0);
    pc->vExpandAlphaPS(x0, z0);
    pc->v_min_f32(z0, z0, x0);

    pc->v_cvt_trunc_f32_to_i32(z0, z0);
    pc->xPackU32ToU16Lo(z0, z0);
    pc->v_add_i16(d0, d0, z0);

    pc->v_div255_u16(d0);
    out.uc.init(d0);

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - ColorBurn (SCALAR)
  // ---------------------------------------

  if (is_color_burn() && n == PixelCount(1)) {
    // Dca' = Sa.Da - min(Sa.Da, (Da - Dca).Sa.Sa / max(Sca, 0.001)) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa.Da - min(Sa.Da, (Da - Da ).Sa.Sa / max(Sa , 0.001)) + Sa .(1 - Da) + Da .(1 - Sa)
    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kPC, predicate);

    Vec& s0 = s.uc[0];
    Vec& d0 = d.pc[0];
    Vec& x0 = xv[0];
    Vec& y0 = yv[0];
    Vec& z0 = zv[0];

    if (has_mask) {
      pc->v_mul_u16(s0, s0, vm[0]);
      pc->v_div255_u16(s0);
    }

    pc->v_cvt_u8_to_u32(d0, d0);
    pc->v_cvt_u16_lo_to_u32(s0, s0);

    pc->v_cvt_i32_to_f32(y0, s0);
    pc->v_cvt_i32_to_f32(z0, d0);
    pc->v_packs_i32_i16(d0, d0, s0);

    pc->vExpandAlphaPS(x0, y0);
    pc->v_max_f32(y0, y0, pc->simd_const(&ct.f32_1e_m3, Bcst::k32, y0));
    pc->v_mul_f32(z0, z0, x0);                                     // Dca.Sa

    pc->vExpandAlphaPS(x0, z0);                                    // Sa.Da
    pc->v_xor_f32(z0, z0, pc->simd_const(&ct.p_8000000080000000, Bcst::k32, z0));

    pc->v_and_f32(z0, z0, pc->simd_const(&ct.p_FFFFFFFF_FFFFFFFF_FFFFFFFF_0, Bcst::kNA, z0));
    pc->v_add_f32(z0, z0, x0);                                     // (Da - Dxa).Sa
    pc->v_div_f32(z0, z0, y0);

    pc->v_swizzle_u32x4(s0, d0, swizzle(1, 1, 3, 3));
    pc->vExpandAlphaHi16(s0, s0);
    pc->vExpandAlphaLo16(s0, s0);
    pc->v_inv255_u16(s0, s0);
    pc->v_mul_u16(d0, d0, s0);
    pc->v_swizzle_u32x4(s0, d0, swizzle(1, 0, 3, 2));
    pc->v_add_i16(d0, d0, s0);

    pc->vExpandAlphaPS(x0, y0);                                    // Sa
    pc->v_mul_f32(z0, z0, x0);
    pc->vExpandAlphaPS(x0, z0);                                    // Sa.Da
    pc->v_min_f32(z0, z0, x0);
    pc->v_and_f32(z0, z0, pc->simd_const(&ct.p_FFFFFFFF_FFFFFFFF_FFFFFFFF_0, Bcst::kNA, z0));
    pc->v_sub_f32(x0, x0, z0);

    pc->v_cvt_trunc_f32_to_i32(x0, x0);
    pc->xPackU32ToU16Lo(x0, x0);
    pc->v_add_i16(d0, d0, x0);

    pc->v_div255_u16(d0);
    out.uc.init(d0);

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - LinearBurn
  // -------------------------------

  if (is_linear_burn()) {
    src_fetch(s, n, PixelFlags::kUC | (has_mask ? PixelFlags::kNone : PixelFlags::kImmutable), predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
    }

    if (use_da && use_sa) {
      // Dca' = Dca + Sca - Sa.Da
      // Da'  = Da  + Sa  - Sa.Da
      pc->v_expand_alpha_16(xv, sv, use_hi);
      pc->v_expand_alpha_16(yv, dv, use_hi);
      pc->v_mul_u16(xv, xv, yv);
      pc->v_div255_u16(xv);
      pc->v_add_i16(dv, dv, sv);
      pc->v_subs_u16(dv, dv, xv);
    }
    else if (use_da || use_sa) {
      pc->v_expand_alpha_16(xv, use_da ? dv : sv, use_hi);
      pc->v_add_i16(dv, dv, sv);
      pc->v_subs_u16(dv, dv, xv);
    }
    else {
      // Dca' = Dc + Sc - 1
      pc->v_add_i16(dv, dv, sv);
      pc->v_subs_u16(dv, dv, pc->simd_const(&ct.p_000000FF00FF00FF, Bcst::kNA, dv));
    }

    out.uc.init(dv);
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - LinearLight
  // --------------------------------

  if (is_linear_light() && n == PixelCount(1)) {
    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
      use_sa = 1;
    }

    if (use_sa || use_da) {
      // Dca' = min(max((Dca.Sa + 2.Sca.Da - Sa.Da), 0), Sa.Da) + Sca.(1 - Da) + Dca.(1 - Sa)
      // Da'  = min(max((Da .Sa + 2.Sa .Da - Sa.Da), 0), Sa.Da) + Sa .(1 - Da) + Da .(1 - Sa)

      Vec& d0 = dv[0];
      Vec& s0 = sv[0];
      Vec& x0 = xv[0];
      Vec& y0 = yv[0];

      pc->vExpandAlphaLo16(y0, d0);
      pc->vExpandAlphaLo16(x0, s0);

      pc->v_interleave_lo_u64(d0, d0, s0);
      pc->v_interleave_lo_u64(x0, x0, y0);

      pc->v_mov(s0, d0);
      pc->v_mul_u16(d0, d0, x0);
      pc->v_inv255_u16(x0, x0);
      pc->v_div255_u16(d0);

      pc->v_mul_u16(s0, s0, x0);
      pc->v_swap_u64(x0, s0);
      pc->v_swap_u64(y0, d0);
      pc->v_add_i16(s0, s0, x0);
      pc->v_add_i16(d0, d0, y0);
      pc->vExpandAlphaLo16(x0, y0);
      pc->v_add_i16(d0, d0, y0);
      pc->v_div255_u16(s0);

      pc->v_subs_u16(d0, d0, x0);
      pc->v_min_i16(d0, d0, x0);

      pc->v_add_i16(d0, d0, s0);
      out.uc.init(d0);
    }
    else {
      // Dc' = min(max((Dc + 2.Sc - 1), 0), 1)
      pc->v_slli_i16(sv, sv, 1);
      pc->v_add_i16(dv, dv, sv);
      pc->v_subs_u16(dv, dv, pc->simd_const(&ct.p_000000FF00FF00FF, Bcst::kNA, dv));
      pc->v_min_i16(dv, dv, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, dv));

      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - PinLight
  // -----------------------------

  if (is_pin_light()) {
    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);

      use_sa = true;
    }

    if (use_sa && use_da) {
      // if 2.Sca <= Sa
      //   Dca' = min(Dca + Sca - Sca.Da, Dca + Sca + Sca.Da - Dca.Sa)
      //   Da'  = min(Da  + Sa  - Sa .Da, Da  + Sa  + Sa .Da - Da .Sa) = Da + Sa.(1 - Da)
      // else
      //   Dca' = max(Dca + Sca - Sca.Da, Dca + Sca + Sca.Da - Dca.Sa - Da.Sa)
      //   Da'  = max(Da  + Sa  - Sa .Da, Da  + Sa  + Sa .Da - Da .Sa - Da.Sa) = Da + Sa.(1 - Da)

      pc->v_expand_alpha_16(yv, sv, use_hi);                                                       // Sa
      pc->v_expand_alpha_16(xv, dv, use_hi);                                                       // Da

      pc->v_mul_u16(yv, yv, dv);                                                                   // Dca.Sa
      pc->v_mul_u16(xv, xv, sv);                                                                   // Sca.Da
      pc->v_add_i16(dv, dv, sv);                                                                   // Dca + Sca
      pc->v_div255_u16_2x(yv, xv);

      pc->v_sub_i16(yv, yv, dv);                                                                   // Dca.Sa - Dca - Sca
      pc->v_sub_i16(dv, dv, xv);                                                                   // Dca + Sca - Sca.Da
      pc->v_sub_i16(xv, xv, yv);                                                                   // Dca + Sca + Sca.Da - Dca.Sa

      pc->v_expand_alpha_16(yv, sv, use_hi);                                                       // Sa
      pc->v_slli_i16(sv, sv, 1);                                                                   // 2.Sca
      pc->v_cmp_gt_i16(sv, sv, yv);                                                                // !(2.Sca <= Sa)

      pc->v_sub_i16(zv, dv, xv);
      pc->v_expand_alpha_16(zv, zv, use_hi);                                                       // -Da.Sa
      pc->v_and_i32(zv, zv, sv);                                                                   // 2.Sca <= Sa ? 0 : -Da.Sa
      pc->v_add_i16(xv, xv, zv);                                                                   // 2.Sca <= Sa ? Dca + Sca + Sca.Da - Dca.Sa : Dca + Sca + Sca.Da - Dca.Sa - Da.Sa

      // if 2.Sca <= Sa:
      //   min(dv, xv)
      // else
      //   max(dv, xv) <- ~min(~dv, ~xv)
      pc->v_xor_i32(dv, dv, sv);
      pc->v_xor_i32(xv, xv, sv);
      pc->v_min_i16(dv, dv, xv);
      pc->v_xor_i32(dv, dv, sv);

      out.uc.init(dv);
    }
    else if (use_da) {
      // if 2.Sc <= 1
      //   Dca' = min(Dca + Sc - Sc.Da, Sc + Sc.Da)
      //   Da'  = min(Da  + 1  - 1 .Da, 1  + 1 .Da) = 1
      // else
      //   Dca' = max(Dca + Sc - Sc.Da, Sc + Sc.Da - Da)
      //   Da'  = max(Da  + 1  - 1 .Da, 1  + 1 .Da - Da) = 1

      pc->v_expand_alpha_16(xv, dv, use_hi);                                                       // Da
      pc->v_mul_u16(xv, xv, sv);                                                                   // Sc.Da
      pc->v_add_i16(dv, dv, sv);                                                                   // Dca + Sc
      pc->v_div255_u16(xv);

      pc->v_cmp_gt_i16(yv, sv, pc->simd_const(&ct.p_007F007F007F007F, Bcst::kNA, yv));             // !(2.Sc <= 1)
      pc->v_add_i16(sv, sv, xv);                                                                   // Sc + Sc.Da
      pc->v_sub_i16(dv, dv, xv);                                                                   // Dca + Sc - Sc.Da
      pc->v_expand_alpha_16(xv, xv);                                                               // Da
      pc->v_and_i32(xv, xv, yv);                                                                   // 2.Sc <= 1 ? 0 : Da
      pc->v_sub_i16(sv, sv, xv);                                                                   // 2.Sc <= 1 ? Sc + Sc.Da : Sc + Sc.Da - Da

      // if 2.Sc <= 1:
      //   min(dv, sv)
      // else
      //   max(dv, sv) <- ~min(~dv, ~sv)
      pc->v_xor_i32(dv, dv, yv);
      pc->v_xor_i32(sv, sv, yv);
      pc->v_min_i16(dv, dv, sv);
      pc->v_xor_i32(dv, dv, yv);

      out.uc.init(dv);
    }
    else if (use_sa) {
      // if 2.Sca <= Sa
      //   Dc' = min(Dc, Dc + 2.Sca - Dc.Sa)
      // else
      //   Dc' = max(Dc, Dc + 2.Sca - Dc.Sa - Sa)

      pc->v_expand_alpha_16(xv, sv, use_hi);                                                       // Sa
      pc->v_slli_i16(sv, sv, 1);                                                                   // 2.Sca
      pc->v_cmp_gt_i16(yv, sv, xv);                                                                // !(2.Sca <= Sa)
      pc->v_and_i32(yv, yv, xv);                                                                   // 2.Sca <= Sa ? 0 : Sa
      pc->v_mul_u16(xv, xv, dv);                                                                   // Dc.Sa
      pc->v_add_i16(sv, sv, dv);                                                                   // Dc + 2.Sca
      pc->v_div255_u16(xv);
      pc->v_sub_i16(sv, sv, yv);                                                                   // 2.Sca <= Sa ? Dc + 2.Sca : Dc + 2.Sca - Sa
      pc->v_cmp_eq_i16(yv, yv, pc->simd_const(&ct.p_0000000000000000, Bcst::kNA, yv));             // 2.Sc <= 1
      pc->v_sub_i16(sv, sv, xv);                                                                   // 2.Sca <= Sa ? Dc + 2.Sca - Dc.Sa : Dc + 2.Sca - Dc.Sa - Sa

      // if 2.Sc <= 1:
      //   min(dv, sv)
      // else
      //   max(dv, sv) <- ~min(~dv, ~sv)
      pc->v_xor_i32(dv, dv, yv);
      pc->v_xor_i32(sv, sv, yv);
      pc->v_max_i16(dv, dv, sv);
      pc->v_xor_i32(dv, dv, yv);

      out.uc.init(dv);
    }
    else {
      // if 2.Sc <= 1
      //   Dc' = min(Dc, 2.Sc)
      // else
      //   Dc' = max(Dc, 2.Sc - 1)

      pc->v_slli_i16(sv, sv, 1);                                                                   // 2.Sc
      pc->v_min_i16(xv, sv, dv);                                                                   // min(Dc, 2.Sc)

      pc->v_cmp_gt_i16(yv, sv, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, yv));             // !(2.Sc <= 1)
      pc->v_sub_i16(sv, sv, pc->simd_const(&ct.p_00FF00FF00FF00FF, Bcst::kNA, sv));                // 2.Sc - 1
      pc->v_max_i16(dv, dv, sv);                                                                   // max(Dc, 2.Sc - 1)

      pc->v_blendv_u8(xv, xv, dv, yv);                                                             // 2.Sc <= 1 ? min(Dc, 2.Sc) : max(Dc, 2.Sc - 1)
      out.uc.init(xv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - HardLight
  // ------------------------------

  if (is_hard_light()) {
    // if (2.Sca < Sa)
    //   Dca' = Dca + Sca - (Dca.Sa + Sca.Da - 2.Sca.Dca)
    //   Da'  = Da  + Sa  - Sa.Da
    // else
    //   Dca' = Dca + Sca + (Dca.Sa + Sca.Da - 2.Sca.Dca) - Sa.Da
    //   Da'  = Da  + Sa  - Sa.Da
    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
    }

    // SPLIT.
    for (unsigned int i = 0; i < n_split; i++) {
      VecArray sh = sv.even_odd(i);
      VecArray dh = dv.even_odd(i);
      VecArray xh = xv.even_odd(i);
      VecArray yh = yv.even_odd(i);
      VecArray zh = zv.even_odd(i);

      pc->v_expand_alpha_16(xh, dh, use_hi);
      pc->v_expand_alpha_16(yh, sh, use_hi);

      pc->v_mul_u16(xh, xh, sh); // Sca.Da
      pc->v_mul_u16(yh, yh, dh); // Dca.Sa
      pc->v_mul_u16(zh, dh, sh); // Dca.Sca

      pc->v_add_i16(dh, dh, sh);
      pc->v_sub_i16(xh, xh, zh);
      pc->v_add_i16(xh, xh, yh);
      pc->v_sub_i16(xh, xh, zh);

      pc->v_expand_alpha_16(yh, yh, use_hi);
      pc->v_expand_alpha_16(zh, sh, use_hi);
      pc->v_div255_u16_2x(xh, yh);

      pc->v_slli_i16(sh, sh, 1);
      pc->v_cmp_gt_i16(zh, zh, sh);

      pc->v_xor_i32(xh, xh, zh);
      pc->v_sub_i16(xh, xh, zh);
      pc->vZeroAlphaW(zh, zh);
      pc->v_bic_i32(zh, yh, zh);
      pc->v_add_i16(dh, dh, xh);
      pc->v_sub_i16(dh, dh, zh);
    }

    out.uc.init(dv);
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - SoftLight (SCALAR)
  // ---------------------------------------

  if (is_soft_light() && n == PixelCount(1)) {
    // Dc = Dca/Da
    //
    // Dca' =
    //   if 2.Sca - Sa <= 0
    //     Dca + Sca.(1 - Da) + (2.Sca - Sa).Da.[[              Dc.(1 - Dc)           ]]
    //   else if 2.Sca - Sa > 0 and 4.Dc <= 1
    //     Dca + Sca.(1 - Da) + (2.Sca - Sa).Da.[[ 4.Dc.(4.Dc.Dc + Dc - 4.Dc + 1) - Dc]]
    //   else
    //     Dca + Sca.(1 - Da) + (2.Sca - Sa).Da.[[             sqrt(Dc) - Dc          ]]
    // Da'  = Da + Sa - Sa.Da
    src_fetch(s, n, PixelFlags::kUC, predicate);
    dst_fetch(d, n, PixelFlags::kPC, predicate);

    Vec& s0 = s.uc[0];
    Vec& d0 = d.pc[0];

    Vec  a0 = pc->new_vec128("a0");
    Vec  b0 = pc->new_vec128("b0");
    Vec& x0 = xv[0];
    Vec& y0 = yv[0];
    Vec& z0 = zv[0];

    if (has_mask) {
      pc->v_mul_u16(s0, s0, vm[0]);
      pc->v_div255_u16(s0);
    }

    pc->v_cvt_u8_to_u32(d0, d0);
    pc->v_cvt_u16_lo_to_u32(s0, s0);
    pc->v_broadcast_v128_f32(x0, pc->_get_mem_const(&ct.f32_1div255));

    pc->v_cvt_i32_to_f32(s0, s0);
    pc->v_cvt_i32_to_f32(d0, d0);

    pc->v_mul_f32(s0, s0, x0);                                                                     // Sca (0..1)
    pc->v_mul_f32(d0, d0, x0);                                                                     // Dca (0..1)

    pc->vExpandAlphaPS(b0, d0);                                                                    // Da
    pc->v_mul_f32(x0, s0, b0);                                                                     // Sca.Da
    pc->v_max_f32(b0, b0, pc->simd_const(&ct.f32_1e_m3, Bcst::k32, b0));                           // max(Da, 0.001)

    pc->v_div_f32(a0, d0, b0);                                                                     // Dc <- Dca/Da
    pc->v_add_f32(d0, d0, s0);                                                                     // Dca + Sca

    pc->vExpandAlphaPS(y0, s0);                                                                    // Sa

    pc->v_sub_f32(d0, d0, x0);                                                                     // Dca + Sca.(1 - Da)
    pc->v_add_f32(s0, s0, s0);                                                                     // 2.Sca
    pc->v_mul_f32(z0, a0, pc->simd_const(&ct.f32_4, Bcst::k32, z0));                               // 4.Dc

    pc->v_sqrt_f32(x0, a0);                                                                        // sqrt(Dc)
    pc->v_sub_f32(s0, s0, y0);                                                                     // 2.Sca - Sa

    pc->v_mov(y0, z0);                                                                             // 4.Dc
    pc->v_madd_f32(z0, z0, a0, a0);                                                                // 4.Dc.Dc + Dc
    pc->v_mul_f32(s0, s0, b0);                                                                     // (2.Sca - Sa).Da

    pc->v_sub_f32(z0, z0, y0);                                                                     // 4.Dc.Dc + Dc - 4.Dc
    pc->v_broadcast_v128_f32(b0, pc->_get_mem_const(&ct.f32_1));                                        // 1

    pc->v_add_f32(z0, z0, b0);                                                                     // 4.Dc.Dc + Dc - 4.Dc + 1
    pc->v_mul_f32(z0, z0, y0);                                                                     // 4.Dc(4.Dc.Dc + Dc - 4.Dc + 1)
    pc->v_cmp_le_f32(y0, y0, b0);                                                                  // 4.Dc <= 1

    pc->v_and_f32(z0, z0, y0);
    pc->v_bic_f32(y0, x0, y0);

    pc->v_zero_f(x0);
    pc->v_or_f32(z0, z0, y0);                                                                      // (4.Dc(4.Dc.Dc + Dc - 4.Dc + 1)) or sqrt(Dc)

    pc->v_cmp_lt_f32(x0, x0, s0);                                                                  // 2.Sca - Sa > 0
    pc->v_sub_f32(z0, z0, a0);                                                                     // [[4.Dc(4.Dc.Dc + Dc - 4.Dc + 1) or sqrt(Dc)]] - Dc

    pc->v_sub_f32(b0, b0, a0);                                                                     // 1 - Dc
    pc->v_and_f32(z0, z0, x0);

    pc->v_mul_f32(b0, b0, a0);                                                                     // Dc.(1 - Dc)
    pc->v_bic_f32(x0, b0, x0);
    pc->v_and_f32(s0, s0, pc->simd_const(&ct.p_FFFFFFFF_FFFFFFFF_FFFFFFFF_0, Bcst::kNA, s0));      // Zero alpha.

    pc->v_or_f32(z0, z0, x0);
    pc->v_mul_f32(s0, s0, z0);

    pc->v_add_f32(d0, d0, s0);
    pc->v_mul_f32(d0, d0, pc->simd_const(&ct.f32_255, Bcst::k32, d0));

    pc->v_cvt_round_f32_to_i32(d0, d0);
    pc->v_packs_i32_i16(d0, d0, d0);
    pc->v_packs_i16_u8(d0, d0, d0);
    out.pc.init(d0);

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Difference
  // -------------------------------

  if (is_difference()) {
    // Dca' = Dca + Sca - 2.min(Sca.Da, Dca.Sa)
    // Da'  = Da  + Sa  -   min(Sa .Da, Da .Sa)
    if (!has_mask) {
      src_fetch(s, n, PixelFlags::kUC | PixelFlags::kUA, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& uv = s.ua;
      VecArray& dv = d.uc;

      // SPLIT.
      for (unsigned int i = 0; i < n_split; i++) {
        VecArray sh = sv.even_odd(i);
        VecArray uh = uv.even_odd(i);
        VecArray dh = dv.even_odd(i);
        VecArray xh = xv.even_odd(i);

        pc->v_expand_alpha_16(xh, dh, use_hi);
        pc->v_mul_u16(uh, uh, dh);
        pc->v_mul_u16(xh, xh, sh);
        pc->v_add_i16(dh, dh, sh);
        pc->v_min_u16(uh, uh, xh);
      }

      pc->v_div255_u16(uv);
      pc->v_sub_i16(dv, dv, uv);

      pc->vZeroAlphaW(uv, uv);
      pc->v_sub_i16(dv, dv, uv);
      out.uc.init(dv);
    }
    // Dca' = Dca + Sca.m - 2.min(Sca.Da, Dca.Sa).m
    // Da'  = Da  + Sa .m -   min(Sa .Da, Da .Sa).m
    else {
      src_fetch(s, n, PixelFlags::kUC, predicate);
      dst_fetch(d, n, PixelFlags::kUC, predicate);

      VecArray& sv = s.uc;
      VecArray& dv = d.uc;

      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);

      // SPLIT.
      for (unsigned int i = 0; i < n_split; i++) {
        VecArray sh = sv.even_odd(i);
        VecArray dh = dv.even_odd(i);
        VecArray xh = xv.even_odd(i);
        VecArray yh = yv.even_odd(i);

        pc->v_expand_alpha_16(yh, sh, use_hi);
        pc->v_expand_alpha_16(xh, dh, use_hi);
        pc->v_mul_u16(yh, yh, dh);
        pc->v_mul_u16(xh, xh, sh);
        pc->v_add_i16(dh, dh, sh);
        pc->v_min_u16(yh, yh, xh);
      }

      pc->v_div255_u16(yv);
      pc->v_sub_i16(dv, dv, yv);

      pc->vZeroAlphaW(yv, yv);
      pc->v_sub_i16(dv, dv, yv);
      out.uc.init(dv);
    }

    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Exclusion
  // ------------------------------

  if (is_exclusion()) {
    // Dca' = Dca + Sca - 2.Sca.Dca
    // Da'  = Da + Sa - Sa.Da
    src_fetch(s, n, PixelFlags::kUC | (has_mask ? PixelFlags::kNone : PixelFlags::kImmutable), predicate);
    dst_fetch(d, n, PixelFlags::kUC, predicate);

    VecArray& sv = s.uc;
    VecArray& dv = d.uc;

    if (has_mask) {
      pc->v_mul_u16(sv, sv, vm);
      pc->v_div255_u16(sv);
    }

    pc->v_mul_u16(xv, dv, sv);
    pc->v_add_i16(dv, dv, sv);
    pc->v_div255_u16(xv);
    pc->v_sub_i16(dv, dv, xv);

    pc->vZeroAlphaW(xv, xv);
    pc->v_sub_i16(dv, dv, xv);

    out.uc.init(dv);
    FetchUtils::satisfy_pixels(pc, out, flags);
    return;
  }

  // VMaskProc - RGBA32 - Invalid
  // ----------------------------

  BL_NOT_REACHED();
}

static void CompOpPart_negateMask(CompOpPart* self, VecArray& vn, const VecArray& vm) noexcept {
  PipeCompiler* pc = self->pc;

  switch (self->coverage_format()) {
    case PixelCoverageFormat::kPacked:
      pc->v_not_u32(vn, vm);
      break;

    case PixelCoverageFormat::kUnpacked:
      pc->v_inv255_u16(vn, vm);
      break;

    default:
      BL_NOT_REACHED();
  }
}

void CompOpPart::v_mask_proc_rgba32_invert_mask(VecArray& vn, const VecArray& vm, PixelCoverageFlags coverage_flags) noexcept {
  bl_unused(coverage_flags);
  size_t size = vm.size();

  if (c_mask_loop_type() == CMaskLoopType::kVariant) {
    if (_mask->vn.is_valid()) {
      bool ok = true;

      // TODO: [JIT] A leftover from a template-based code, I don't understand
      // it anymore and it seems it's unnecessary so verify this and all places
      // that hit `ok == false`.
      for (size_t i = 0; i < bl_min(vn.size(), size); i++)
        if (vn[i].id() != vm[i].id())
          ok = false;

      if (ok) {
        vn.init(_mask->vn.clone_as(vm[0]));
        return;
      }
    }
  }

  if (vn.is_empty())
    pc->new_vec_array(vn, size, vm[0], "vn");

  CompOpPart_negateMask(this, vn, vm);
}

void CompOpPart::v_mask_proc_rgba32_invert_done(VecArray& vn, const VecArray& vm, PixelCoverageFlags coverage_flags) noexcept {
  if (!bl_test_flag(coverage_flags, PixelCoverageFlags::kImmutable))
    return;

  // The inverted mask must be the same, masks cannot be empty as this is called after `v_mask_proc_rgba32_invert_mask()`.
  BL_ASSERT(!vn.is_empty());
  BL_ASSERT(!vm.is_empty());
  BL_ASSERT(vn.size() == vm.size());

  if (vn[0].id() != vm[0].id())
    return;

  CompOpPart_negateMask(this, vn, vn);
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
