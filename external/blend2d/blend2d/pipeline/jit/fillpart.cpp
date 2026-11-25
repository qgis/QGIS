// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/compoppart_p.h>
#include <blend2d/pipeline/jit/fillpart_p.h>
#include <blend2d/pipeline/jit/fetchpart_p.h>
#include <blend2d/pipeline/jit/fetchpixelptrpart_p.h>
#include <blend2d/pipeline/jit/fetchutilscoverage_p.h>
#include <blend2d/pipeline/jit/fetchutilsinlineloops_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::JIT::FillPart - Utilities
// =======================================

static uint32_t calculate_coverage_byte_count(PixelCount pixel_count, PixelType pixel_type, PixelCoverageFormat coverage_format) noexcept {
  DataWidth data_width = DataWidth::k8;

  switch (coverage_format) {
    case PixelCoverageFormat::kPacked:
      data_width = DataWidth::k8;
      break;

    case PixelCoverageFormat::kUnpacked:
      data_width = DataWidth::k16;
      break;

    default:
      BL_NOT_REACHED();
  }

  uint32_t count = uint32_t(pixel_count);
  switch (pixel_type) {
    case PixelType::kA8:
      break;

    case PixelType::kRGBA32:
      count *= 4u;
      break;

    default:
      BL_NOT_REACHED();
  }

  return (1u << uint32_t(data_width)) * count;
}

static void init_vec_coverage(
  PipeCompiler* pc,
  VecArray& dst,
  PixelCount max_pixel_count,
  VecWidth acc_vec_width,
  VecWidth max_vec_width,
  PixelType pixel_type,
  PixelCoverageFormat coverage_format) noexcept {

  uint32_t coverage_byte_count = calculate_coverage_byte_count(max_pixel_count, pixel_type, coverage_format);
  VecWidth vec_width = VecWidthUtils::vec_width_for_byte_count(max_vec_width, coverage_byte_count);
  size_t vec_count = VecWidthUtils::vec_count_for_byte_count(vec_width, coverage_byte_count);

  pc->new_vec_array(dst, vec_count, bl_max(vec_width, acc_vec_width), "vm");

  // The width of the register must match the accumulator (as otherwise AsmJit could
  // spill and only load a part of it in case the vector width of `dst` is smaller).
  dst.set_vec_width(vec_width);
}

static void pass_vec_coverage(
  VecArray& dst,
  const VecArray& src,
  PixelCount pixel_count,
  PixelType pixel_type,
  PixelCoverageFormat coverage_format) noexcept {

  uint32_t coverage_byte_count = calculate_coverage_byte_count(pixel_count, pixel_type, coverage_format);
  VecWidth vec_width = VecWidthUtils::vec_width_for_byte_count(VecWidthUtils::vec_width_of(src[0]), coverage_byte_count);
  size_t vec_count = VecWidthUtils::vec_count_for_byte_count(vec_width, coverage_byte_count);

  // We can use at most what was given to us, or less in case that the current
  // `pixel_count` is less than `max_pixel_count` passed to `init_vec_coverage()`.
  BL_ASSERT(vec_count <= src.size());

  dst._size = vec_count;
  for (size_t i = 0; i < vec_count; i++) {
    dst.v[i].reset();
    dst.v[i].as<asmjit::Reg>().set_signature_and_id(VecWidthUtils::signature_of(vec_width), src.v[i].id());
  }
}

// bl::Pipeline::JIT::FillPart - Construction & Destruction
// ========================================================

FillPart::FillPart(PipeCompiler* pc, FillType fill_type, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept
  : PipePart(pc, PipePartType::kFill),
    _fill_type(fill_type) {

  // Initialize the children of this part.
  _children[kIndexDstPart] = dst_part;
  _children[kIndexCompOpPart] = comp_op_part;
  _child_count = 2;
}

// [[pure virtual]]
void FillPart::compile(const PipeFunction& fn) noexcept {
  bl_unused(fn);
  BL_NOT_REACHED();
}

// bl::Pipeline::JIT::FillBoxAPart - Construction & Destruction
// ============================================================

FillBoxAPart::FillBoxAPart(PipeCompiler* pc, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept
  : FillPart(pc, FillType::kBoxA, dst_part, comp_op_part) {

  add_part_flags(PipePartFlags::kRectFill);
  _max_vec_width_supported = kMaxPlatformWidth;
}

// bl::Pipeline::JIT::FillBoxAPart - Compile
// =========================================

void FillBoxAPart::compile(const PipeFunction& fn) noexcept {
  // Prepare
  // -------

  _init_global_hook(cc->cursor());

  int dst_bpp = int(dst_part()->bpp());
  bool is_src_copy_fill = comp_op_part()->is_src_copy() && comp_op_part()->src_part()->is_solid();

  // Local Registers
  // ---------------

  Gp ctx_data = fn.ctx_data();                         // Reg/Init.
  Gp fill_data = fn.fill_data();                       // Reg/Init.

  Gp dst_ptr = pc->new_gpz("dst_ptr");                 // Reg.
  Gp dst_stride = pc->new_gpz("dst_stride");           // Reg/Mem.

  Gp x = pc->new_gp32("x");                            // Reg.
  Gp y = pc->new_gp32("y");                            // Reg/Mem.
  Gp w = pc->new_gp32("w");                            // Reg/Mem.
  Gp ga_sm = pc->new_gp32("ga.sm");                    // Reg/Tmp.

  // Prolog
  // ------

  pc->load(dst_stride, mem_ptr(ctx_data, BL_OFFSET_OF(ContextData, dst.stride)));
  pc->load_u32(y, mem_ptr(fill_data, BL_OFFSET_OF(FillData::BoxA, box.y0)));
  pc->load_u32(w, mem_ptr(fill_data, BL_OFFSET_OF(FillData::BoxA, box.x0)));

  pc->mul(dst_ptr, dst_stride, y.clone_as(dst_ptr));

  dst_part()->init_ptr(dst_ptr);
  comp_op_part()->init(fn, w, y, 1);

  pc->add_ext(dst_ptr, dst_ptr, w, uint32_t(dst_bpp));
  pc->sub(w, mem_ptr(fill_data, BL_OFFSET_OF(FillData::BoxA, box.x1)), w);
  pc->sub(y, mem_ptr(fill_data, BL_OFFSET_OF(FillData::BoxA, box.y1)), y);
  pc->mul(x, w, dst_bpp);
  pc->add(dst_ptr, dst_ptr, mem_ptr(ctx_data, BL_OFFSET_OF(ContextData, dst.pixel_data)));

  if (is_src_copy_fill) {
    Label L_NotStride = pc->new_label();

    pc->j(L_NotStride, cmp_ne(x.clone_as(dst_stride), dst_stride));
    pc->mul(w, w, y);
    pc->mov(y, 1);
    pc->bind(L_NotStride);
  }
  else {
    // Only subtract from destination stride if this is not a solid rectangular fill.
    pc->sub(dst_stride, dst_stride, x.clone_as(dst_stride));
  }

  // Loop
  // ----

  if (comp_op_part()->should_optimize_opaque_fill()) {
    Label L_SemiAlphaInit = pc->new_label();
    Label L_End  = pc->new_label();

    pc->load_u32(ga_sm, mem_ptr(fill_data, BL_OFFSET_OF(FillData::BoxA, alpha)));
    pc->j(L_SemiAlphaInit, cmp_ne(ga_sm, 255));

    // Full Alpha
    // ----------

    if (is_src_copy_fill) {
      // Optimize fill rect if it can be implemented as a memset. The main reason is
      // that if the width is reasonably small we want to only check that condition once.
      comp_op_part()->c_mask_init_opaque();
      BL_ASSERT(comp_op_part()->_solid_opt.px.is_valid());

      FetchUtils::inline_fill_rect_loop(pc, dst_ptr, dst_stride, w, y, comp_op_part()->_solid_opt.px, dst_part()->bpp(), L_End);
      comp_op_part()->c_mask_fini();
    }
    else {
      Label L_AdvanceY = pc->new_label();
      Label L_ProcessY = pc->new_label();

      comp_op_part()->c_mask_init_opaque();
      pc->j(L_ProcessY);

      pc->bind(L_AdvanceY);
      comp_op_part()->advance_y();
      pc->add(dst_ptr, dst_ptr, dst_stride);

      pc->bind(L_ProcessY);
      pc->mov(x, w);
      comp_op_part()->start_at_x(pc->_gp_none);
      comp_op_part()->c_mask_generic_loop(x);
      pc->j(L_AdvanceY, sub_nz(y, 1));

      comp_op_part()->c_mask_fini();
      pc->j(L_End);
    }

    // Semi Alpha
    // ----------

    {
      Label L_AdvanceY = pc->new_label();
      Label L_ProcessY = pc->new_label();

      pc->bind(L_SemiAlphaInit);

      if (is_src_copy_fill) {
        // This was not accounted yet as `inline_fill_rect_loop()` expects full stride, so we have to account this now.
        pc->sub(dst_stride, dst_stride, x.clone_as(dst_stride));
      }

      comp_op_part()->c_mask_init(ga_sm, Vec());
      pc->j(L_ProcessY);

      pc->bind(L_AdvanceY);
      comp_op_part()->advance_y();
      pc->add(dst_ptr, dst_ptr, dst_stride);

      pc->bind(L_ProcessY);
      pc->mov(x, w);
      comp_op_part()->start_at_x(pc->_gp_none);
      comp_op_part()->c_mask_generic_loop(x);
      pc->j(L_AdvanceY, sub_nz(y, 1));

      comp_op_part()->c_mask_fini();
      pc->bind(L_End);
    }
  }
  else {
    Label L_AdvanceY = pc->new_label();
    Label L_ProcessY = pc->new_label();

    comp_op_part()->c_mask_init(mem_ptr(fill_data, BL_OFFSET_OF(FillData::BoxA, alpha)));
    pc->j(L_ProcessY);

    pc->bind(L_AdvanceY);
    comp_op_part()->advance_y();
    pc->add(dst_ptr, dst_ptr, dst_stride);

    pc->bind(L_ProcessY);
    pc->mov(x, w);
    comp_op_part()->start_at_x(pc->_gp_none);
    comp_op_part()->c_mask_generic_loop(x);
    pc->j(L_AdvanceY, sub_nz(y, 1));

    comp_op_part()->c_mask_fini();
  }

  // Epilog
  // ------

  comp_op_part()->fini();
  _fini_global_hook();
}

// bl::Pipeline::JIT::FillMaskPart - Construction & Destruction
// ============================================================

FillMaskPart::FillMaskPart(PipeCompiler* pc, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept
  : FillPart(pc, FillType::kMask, dst_part, comp_op_part) {

  _max_vec_width_supported = kMaxPlatformWidth;
}

// bl::Pipeline::JIT::FillMaskPart - Compile
// =========================================

void FillMaskPart::compile(const PipeFunction& fn) noexcept {
  // EndOrRepeat is expected to be zero for fast termination of the scanline.
  BL_STATIC_ASSERT(uint32_t(MaskCommandType::kEndOrRepeat) == 0);

  // Prepare
  // -------

  _init_global_hook(cc->cursor());

  int dst_bpp = int(dst_part()->bpp());
  constexpr int kMaskCmdSize = int(sizeof(MaskCommand));

#if defined(BL_JIT_ARCH_X86)
  constexpr int label_alignment = 8;
#else
  constexpr int label_alignment = 4;
#endif

  // Local Labels
  // ------------

  Label L_ScanlineInit = pc->new_label();
  Label L_ScanlineDone = pc->new_label();
  Label L_ScanlineSkip = pc->new_label();

  Label L_ProcessNext = pc->new_label();
  Label L_ProcessCmd = pc->new_label();
  Label L_CMaskInit = pc->new_label();
  Label L_VMaskA8WithoutGA = pc->new_label();
  Label L_End = pc->new_label();

  // Local Registers
  // ---------------

  Gp ctx_data = fn.ctx_data();                         // Reg/Init.
  Gp fill_data = fn.fill_data();                       // Reg/Init.

  Gp dst_ptr = pc->new_gpz("dst_ptr");                  // Reg.
  Gp dst_stride = pc->new_gpz("dst_stride");            // Reg/Mem.

  Gp i = pc->new_gp32("i");                            // Reg.
  Gp x = pc->new_gp32("x");                            // Reg.
  Gp y = pc->new_gp32("y");                            // Reg/Mem.

  Gp cmd_type = pc->new_gp32("cmd_type");              // Reg/Tmp.
  Gp cmd_ptr = pc->new_gpz("cmd_ptr");                  // Reg/Mem.
  Gp cmd_begin = pc->new_gpz("cmd_begin");              // Mem.
  Gp mask_value = pc->new_gpz("mask_value");            // Reg.
  Gp mask_advance = pc->new_gpz("mask_advance");        // Reg/Tmp

  GlobalAlpha ga;

  // Prolog
  // ------

  // Initialize the destination.
  pc->load(dst_stride, mem_ptr(ctx_data, BL_OFFSET_OF(ContextData, dst.stride)));
  pc->load_u32(y, mem_ptr(fill_data, BL_OFFSET_OF(FillData, mask.box.y0)));

  pc->mul(dst_ptr, dst_stride, y.clone_as(dst_ptr));
  pc->add(dst_ptr, dst_ptr, mem_ptr(ctx_data, BL_OFFSET_OF(ContextData, dst.pixel_data)));

  // Initialize pipeline parts.
  dst_part()->init_ptr(dst_ptr);
  comp_op_part()->init(fn, pc->_gp_none, y, 1);

  // Initialize mask pointers.
  pc->load(cmd_ptr, mem_ptr(fill_data, BL_OFFSET_OF(FillData, mask.mask_command_data)));

  // Initialize global alpha.
  ga.init_from_mem(pc, mem_ptr(fill_data, BL_OFFSET_OF(FillData, mask.alpha)));

  // y = fill_data->box.y1 - fill_data->box.y0;
  pc->sub(y, mem_ptr(fill_data, BL_OFFSET_OF(FillData, mask.box.y1)), y);
  pc->j(L_ScanlineInit);

  // Scanline Done
  // -------------

  Gp repeat = pc->new_gp32("repeat");

  pc->align(AlignMode::kCode, label_alignment);
  pc->bind(L_ScanlineDone);
  deadvance_dst_ptr(dst_ptr, x, int(dst_bpp));

  pc->bind(L_ScanlineSkip);
  pc->load_u32(repeat, mem_ptr(cmd_ptr, BL_OFFSET_OF(MaskCommand, _x0)));
  pc->j(L_End, sub_z(y, 1));

  pc->sub(repeat, repeat, 1);
  pc->add(dst_ptr, dst_ptr, dst_stride);
  pc->store_u32(mem_ptr(cmd_ptr, BL_OFFSET_OF(MaskCommand, _x0)), repeat);
  pc->add(cmd_ptr, cmd_ptr, kMaskCmdSize);
  comp_op_part()->advance_y();
  pc->cmov(cmd_ptr, cmd_begin, cmp_ne(repeat, 0));

  // Scanline Init
  // -------------

  pc->bind(L_ScanlineInit);
  pc->load_u32(cmd_type, mem_ptr(cmd_ptr, BL_OFFSET_OF(MaskCommand, _x1_and_type)));
  pc->mov(cmd_begin, cmd_ptr);
  pc->load_u32(x, mem_ptr(cmd_ptr, BL_OFFSET_OF(MaskCommand, _x0)));
  // This is not really common, but it's possible to skip entire scanlines with `kEndOrRepeat`.
  pc->j(L_ScanlineSkip, test_z(cmd_type, MaskCommand::kTypeMask));

  pc->add_scaled(dst_ptr, x.clone_as(dst_ptr), dst_bpp);
  comp_op_part()->start_at_x(x);
  pc->j(L_ProcessCmd);

  // Process Command
  // ---------------

  pc->bind(L_ProcessNext);
  pc->load_u32(cmd_type, mem_ptr(cmd_ptr, kMaskCmdSize + BL_OFFSET_OF(MaskCommand, _x1_and_type)));
  pc->load_u32(i, mem_ptr(cmd_ptr, kMaskCmdSize + BL_OFFSET_OF(MaskCommand, _x0)));
  pc->add(cmd_ptr, cmd_ptr, kMaskCmdSize);
  pc->j(L_ScanlineDone, test_z(cmd_type, MaskCommand::kTypeMask));

  // Only emit the jump if there is something significant to skip.
  if (comp_op_part()->has_part_flag(PipePartFlags::kAdvanceXIsSimple))
    pc->sub(i, i, x);
  else
    pc->j(L_ProcessCmd, sub_z(i, x));

  pc->add(x, x, i);
  pc->add_scaled(dst_ptr, i.clone_as(dst_ptr), dst_bpp);
  comp_op_part()->advance_x(x, i);

  pc->bind(L_ProcessCmd);

#if defined(BL_JIT_ARCH_X86)
  if (pc->has_bmi2() && pc->is_64bit())
  {
    // This saves one instruction on X86_64 as RORX provides a non-destructive destination.
    pc->ror(i.r64(), cmd_type.r64(), MaskCommand::kTypeBits);
  }
  else
#endif // BL_JIT_ARCH_X86
  {
    pc->shr(i, cmd_type, MaskCommand::kTypeBits);
  }

  pc->and_(cmd_type, cmd_type, MaskCommand::kTypeMask);
  pc->sub(i, i, x);
  pc->load(mask_value, mem_ptr(cmd_ptr, BL_OFFSET_OF(MaskCommand, _value.data)));
  pc->add(x, x, i);

  // We know the command is not kEndOrRepeat, which allows this little trick.
  pc->j(L_CMaskInit, cmp_eq(cmd_type, uint32_t(MaskCommandType::kCMask)));

  // VMask Command
  // -------------

  // Increments the advance in the mask command in case it would be repeated.
  pc->load(mask_advance, mem_ptr(cmd_ptr, BL_OFFSET_OF(MaskCommand, _mask_advance)));
  pc->mem_add(mem_ptr(cmd_ptr, BL_OFFSET_OF(MaskCommand, _value.ptr)), mask_advance);

  pc->j(L_VMaskA8WithoutGA, cmp_eq(cmd_type, uint32_t(MaskCommandType::kVMaskA8WithoutGA)));
  comp_op_part()->v_mask_generic_loop(i, dst_ptr, mask_value, nullptr, L_ProcessNext);

  pc->bind(L_VMaskA8WithoutGA);
  comp_op_part()->v_mask_generic_loop(i, dst_ptr, mask_value, &ga, L_ProcessNext);

  // CMask Command
  // -------------

  pc->align(AlignMode::kCode, label_alignment);
  pc->bind(L_CMaskInit);
  if (comp_op_part()->should_optimize_opaque_fill()) {
    Label L_CLoop_Msk = pc->new_label();
    pc->j(L_CLoop_Msk, cmp_ne(mask_value.r32(), 255));

    comp_op_part()->c_mask_init_opaque();
    comp_op_part()->c_mask_generic_loop(i);
    comp_op_part()->c_mask_fini();
    pc->j(L_ProcessNext);

    pc->align(AlignMode::kCode, label_alignment);
    pc->bind(L_CLoop_Msk);
  }

  comp_op_part()->c_mask_init(mask_value.r32(), Vec());
  comp_op_part()->c_mask_generic_loop(i);
  comp_op_part()->c_mask_fini();
  pc->j(L_ProcessNext);

  // Epilog
  // ------

  pc->bind(L_End);
  comp_op_part()->fini();
  _fini_global_hook();
}

void FillMaskPart::deadvance_dst_ptr(const Gp& dst_ptr, const Gp& x, int dst_bpp) noexcept {
  Gp x_adv = x.clone_as(dst_ptr);

  if (IntOps::is_power_of_2(dst_bpp)) {
    if (dst_bpp > 1)
      pc->shl(x_adv, x_adv, IntOps::ctz(dst_bpp));
    pc->sub(dst_ptr, dst_ptr, x_adv);
  }
  else {
    Gp dst_adv = pc->new_gpz("dst_adv");
    pc->mul(dst_adv, x_adv, dst_bpp);
    pc->sub(dst_ptr, dst_ptr, dst_adv);
  }
}

// bl::Pipeline::JIT::FillAnalyticPart - Construction & Destruction
// ================================================================

FillAnalyticPart::FillAnalyticPart(PipeCompiler* pc, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept
  : FillPart(pc, FillType::kAnalytic, dst_part, comp_op_part) {

  _max_vec_width_supported = kMaxPlatformWidth;
}

// bl::Pipeline::JIT::FillAnalyticPart - Compile
// =============================================

void FillAnalyticPart::compile(const PipeFunction& fn) noexcept {
  // Prepare
  // -------

  _init_global_hook(cc->cursor());

  PixelType pixel_type = comp_op_part()->pixel_type();
  PixelCoverageFormat coverage_format = comp_op_part()->coverage_format();

  uint32_t dst_bpp = dst_part()->bpp();
  uint32_t max_pixels = comp_op_part()->max_pixels();

  // v_proc SIMD width describes SIMD width used to accumulate coverages and then to calculate alpha masks. In
  // general if we only calculate 4 coverages at once we only need 128-bit SIMD. However, 8 and more coverages
  // need 256-bit SIMD or higher, if available. At the moment we use always a single register for this purpose,
  // so SIMD width determines how many pixels we can process in a v_mask loop at a time.
  uint32_t v_proc_pixel_count = 0;
  VecWidth v_proc_width = pc->vec_width();

  if (pc->vec_width() >= VecWidth::k256 && max_pixels >= 8) {
    v_proc_pixel_count = 8;
    v_proc_width = VecWidth::k256;
  }
  else {
    v_proc_pixel_count = bl_min<uint32_t>(max_pixels, 4);
    v_proc_width = VecWidth::k128;
  }

  int bw_size = int(sizeof(BLBitWord));
  int bw_size_in_bits = bw_size * 8;

  int pixels_per_one_bit = 4;
  int pixels_per_one_bit_shift = int(IntOps::ctz(pixels_per_one_bit));

  int pixel_granularity = pixels_per_one_bit;
  int pixels_per_bit_word = pixels_per_one_bit * bw_size_in_bits;
  int pixels_per_bit_word_shift = int(IntOps::ctz(pixels_per_bit_word));

  if (comp_op_part()->max_pixels_of_children() < 4)
    pixel_granularity = 1;

  // Local Labels
  // ------------

  Label L_BitScan_Init = pc->new_label();
  Label L_BitScan_Iter = pc->new_label();
  Label L_BitScan_Match = pc->new_label();
  Label L_BitScan_End = pc->new_label();

  Label L_VLoop_Init = pc->new_label();
  Label L_CLoop_Init = pc->new_label();

  Label L_VTail_Init;

  if (max_pixels >= 4) {
    L_VTail_Init = pc->new_label();
  }

  Label L_Scanline_Done0 = pc->new_label();
  Label L_Scanline_Done1 = pc->new_label();
  Label L_Scanline_AdvY = pc->new_label();
  Label L_Scanline_Iter = pc->new_label();
  Label L_Scanline_Init = pc->new_label();

  Label L_End = pc->new_label();

  // Local Registers
  // ---------------

  Gp ctx_data = fn.ctx_data();                                   // Init.
  Gp fill_data = fn.fill_data();                                 // Init.

  Gp dst_ptr = pc->new_gpz("dst_ptr");                           // Reg.
  Gp dst_stride = pc->new_gpz("dst_stride");                     // Mem.

  Gp bit_ptr = pc->new_gpz("bit_ptr");                           // Reg.
  Gp bit_ptr_end = pc->new_gpz("bit_ptr_end");                   // Reg/Mem.

  Gp bit_ptr_run_len = pc->new_gpz("bit_ptr_run_len");           // Mem.
  Gp bit_ptr_skip_len = pc->new_gpz("bit_ptr_skip_len");         // Mem.

  Gp cell_ptr = pc->new_gpz("cell_ptr");                         // Reg.
  Gp cell_stride = pc->new_gpz("cell_stride");                   // Mem.

  Gp x0 = pc->new_gp32("x0");                                    // Reg
  Gp x_off = pc->new_gp32("x_off");                              // Reg/Mem.
  Gp x_end = pc->new_gp32("x_end");                              // Mem.
  Gp x_start = pc->new_gp32("x_start");                          // Mem.

  Gp y = pc->new_gp32("y");                                      // Reg/Mem.
  Gp i = pc->new_gp32("i");                                      // Reg.
  Gp c_mask_alpha = pc->new_gp32("c_mask_alpha");                // Reg/Tmp.

  Gp bit_word = pc->new_gpz("bit_word");                         // Reg/Mem.
  Gp bit_word_tmp = pc->new_gpz("bit_word_tmp");                 // Reg/Tmp.

  Vec acc = pc->new_vec_with_width(v_proc_width, "acc");                      // Reg.
  Vec global_alpha = pc->new_vec_with_width(v_proc_width, "global_alpha");    // Mem.
  Vec fill_rule_mask = pc->new_vec_with_width(v_proc_width, "fill_rule_mask");// Mem.
  Vec vec_zero;                                                  // Reg/Tmp.

  Pixel d_pix("d", pixel_type);                                  // Reg.

  VecArray m;                                                    // Reg.
  VecArray comp_cov;                                             // Tmp (only for passing coverages to the compositor).
  init_vec_coverage(pc, m, PixelCount(max_pixels), VecWidthUtils::vec_width_of(acc), pc->vec_width(), pixel_type, coverage_format);

  // Prolog
  // ------

  // Initialize the destination.
  pc->load_u32(y, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, box.y0)));
  pc->load(dst_stride, mem_ptr(ctx_data, BL_OFFSET_OF(ContextData, dst.stride)));

  pc->mul(dst_ptr, y.clone_as(dst_ptr), dst_stride);
  pc->add(dst_ptr, dst_ptr, mem_ptr(ctx_data, BL_OFFSET_OF(ContextData, dst.pixel_data)));

  // Initialize cell pointers.
  pc->load(bit_ptr_skip_len, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, bit_stride)));
  pc->load(cell_stride, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, cell_stride)));

  pc->load(bit_ptr, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, bit_top_ptr)));
  pc->load(cell_ptr, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, cell_top_ptr)));

  // Initialize pipeline parts.
  dst_part()->init_ptr(dst_ptr);
  comp_op_part()->init(fn, pc->_gp_none, y, uint32_t(pixel_granularity));

  // y = fill_data->box.y1 - fill_data->box.y0;
  pc->sub(y, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, box.y1)), y);

  // Decompose the original `bit_stride` to bit_ptr_run_len + bit_ptr_skip_len, where:
  //   - `bit_ptr_run_len` - Number of BitWords (in byte units) active in this band.
  //   - `bit_ptr_run_skip` - Number of BitWords (in byte units) to skip for this band.
  pc->shr(x_start, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, box.x0)), pixels_per_bit_word_shift);
  pc->load_u32(x_end, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, box.x1)));
  pc->shr(bit_ptr_run_len.r32(), x_end, pixels_per_bit_word_shift);

  pc->sub(bit_ptr_run_len.r32(), bit_ptr_run_len.r32(), x_start);
  pc->inc(bit_ptr_run_len.r32());
  pc->shl(bit_ptr_run_len, bit_ptr_run_len, IntOps::ctz(bw_size));
  pc->sub(bit_ptr_skip_len, bit_ptr_skip_len, bit_ptr_run_len);

  // Make `x_start` to become the X offset of the first active BitWord.
  pc->lea(bit_ptr, mem_ptr(bit_ptr, x_start.clone_as(bit_ptr), IntOps::ctz(bw_size)));
  pc->shl(x_start, x_start, pixels_per_bit_word_shift);

  // Initialize global alpha and fill-rule.
  pc->v_broadcast_u16(global_alpha, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, alpha)));
  pc->v_broadcast_u32(fill_rule_mask, mem_ptr(fill_data, BL_OFFSET_OF(FillData::Analytic, fill_rule_mask)));

#if defined(BL_JIT_ARCH_X86)
  vec_zero = pc->new_vec128("vec_zero");
  // We shift left by 7 bits so we can use [V]PMULHUW in `calc_masks_from_cells()` on X86 ISA. In order to make that
  // work, we have to also shift `fill_rule_mask` left by 1, so the total shift left is 8, which is what we want for
  // [V]PMULHUW.
  pc->v_slli_i16(global_alpha, global_alpha, 7);
  pc->v_slli_i16(fill_rule_mask, fill_rule_mask, 1);
#else
  // In non-x86 case we want to keep zero in `vec_zero` - no need to clear it every time we want to clear memory.
  vec_zero = pc->simd_vec_zero(acc);
#endif

  pc->j(L_Scanline_Init);

  // BitScan
  // -------

  // Called by Scanline iterator on the first non-zero BitWord it matches. The responsibility of BitScan is to find
  // the first bit in the passed BitWord followed by matching the bit that ends this match. This would essentially
  // produce the first [x0, x1) span that has to be composited as 'VMask' loop.

  pc->bind(L_BitScan_Init);                                      // L_BitScan_Init:

  count_zeros(x0.clone_as(bit_word), bit_word);                  //   x0 = ctz(bit_word) or clz(bit_word);
  pc->store_zero_reg(mem_ptr(bit_ptr, -bw_size));                //   bit_ptr[-1] = 0;
  pc->mov(bit_word_tmp, -1);                                     //   bit_word_tmp = -1; (all ones).
  shift_mask(bit_word_tmp, bit_word_tmp, x0);                    //   bit_word_tmp = bit_word_tmp << x0 or bit_word_tmp >> x0

  // Convert bit offset `x0` into a pixel offset. We must consider `x_off` as it's only zero for the very first
  // BitWord (all others are multiplies of `pixels_per_bit_word`).
  pc->add_ext(x0, x_off, x0, 1 << pixels_per_one_bit_shift);     //   x0 = x_off + (x0 << pixels_per_one_bit_shift);

  // Load the given cells to `m0` and clear the BitWord and all cells it represents in memory. This is important as
  // the compositor has to clear the memory during composition. If this is a rare case where `x0` points at the end
  // of the raster there is still one cell that is non-zero. This makes sure it's cleared.

  pc->add_scaled(dst_ptr, x0.clone_as(dst_ptr), int(dst_bpp));   //   dst_ptr += x0 * dst_bpp;
  pc->add_scaled(cell_ptr, x0.clone_as(cell_ptr), 4);            //   cell_ptr += x0 * sizeof(uint32_t);

  // Rare case - line rasterized at the end of the raster boundary. In 99% cases this is a clipped line that was
  // rasterized as vertical-only line at the end of the render box. This is a completely valid case that produces
  // nothing.

  pc->j(L_Scanline_Done0, ucmp_ge(x0, x_end));                   //   if (x0 >= x_end) goto L_Scanline_Done0;

  // Setup compositor and source/destination parts. This is required as the fetcher needs to know where to start.
  // And since `start_at_x()` can only be called once per scanline we must do it here.

  comp_op_part()->start_at_x(x0);                                //   <CompOpPart::StartAtX>

  if (max_pixels > 1)
    comp_op_part()->prefetch_n();                                //   <CompOpPart::PrefetchN>
  else if (pixel_granularity > 1)
    comp_op_part()->src_part()->prefetch_n();

  pc->v_loada32(acc, pc->_get_mem_const(&ct.p_0002000000020000));

  // If `bit_word ^ bit_word_tmp` results in non-zero value it means that the current span ends within the same BitWord,
  // otherwise the span crosses multiple BitWords.

  pc->j(L_BitScan_Match, xor_nz(bit_word, bit_word_tmp));        //   if ((bit_word ^= bit_word_tmp) != 0) goto L_BitScan_Match;

  // Okay, so the span crosses multiple BitWords. Firstly we have to make sure this was not the last one. If that's
  // the case we must terminate the scanning immediately.

  pc->mov(i, bw_size_in_bits);                                   //   i = bw_size_in_bits;
  pc->j(L_BitScan_End, cmp_eq(bit_ptr, bit_ptr_end));            //   if (bit_ptr == bit_ptr_end) goto L_BitScan_End;

  // A BitScan loop - iterates over all consecutive BitWords and finds those that don't have all bits set to 1.

  pc->bind(L_BitScan_Iter);                                      // L_BitScan_Iter:
  pc->load(bit_word, mem_ptr(bit_ptr));                          //   bit_word = bit_ptr[0];
  pc->store_zero_reg(mem_ptr(bit_ptr));                          //   bit_ptr[0] = 0;
  pc->add(x_off, x_off, pixels_per_bit_word);                    //   x_off += pixels_per_bit_word;
  pc->add(bit_ptr, bit_ptr, bw_size);                            //   bit_ptr += bw_size;
  pc->j(L_BitScan_Match, xor_nz(bit_word, -1));                  //   if ((bit_word ^= -1) != 0) goto L_BitScan_Match;
  pc->j(L_BitScan_End, cmp_eq(bit_ptr, bit_ptr_end));            //   if (bit_ptr == bit_ptr_end) goto L_BitScan_End;
  pc->j(L_BitScan_Iter);                                         //   goto L_BitScan_Iter;

  pc->bind(L_BitScan_Match);                                     // L_BitScan_Match:
  count_zeros(i.clone_as(bit_word), bit_word);                   //   i = ctz(bit_word) or clz(bit_word);

  pc->bind(L_BitScan_End);                                       // L_BitScan_End:

#if defined(BL_JIT_ARCH_X86)
  if (v_proc_pixel_count == 8) {
    pc->v_add_i32(acc.v256(), acc.v256(), mem_ptr(cell_ptr));    //   acc[7:0] += cell_ptr[7:0];
  }
  else
#endif // BL_JIT_ARCH_X86
  {
    pc->v_add_i32(acc.v128(), acc.v128(), mem_ptr(cell_ptr));    //   acc[3:0] += cell_ptr[3:0];
  }

  pc->mov(bit_word_tmp, -1);                                     //   bit_word_tmp = -1; (all ones).
  shift_mask(bit_word_tmp, bit_word_tmp, i);                     //   bit_word_tmp = bit_word_tmp << i or bit_word_tmp >> i;
  pc->shl(i, i, pixels_per_one_bit_shift);                       //   i <<= pixels_per_one_bit_shift;

  pc->xor_(bit_word, bit_word, bit_word_tmp);                    //   bit_word ^= bit_word_tmp;
  pc->add(i, i, x_off);                                          //   i += x_off;

  // In cases where the raster width is not a multiply of `pixels_per_one_bit` we must make sure we won't overflow it.

  pc->umin(i, i, x_end);                                         //   i = min(i, x_end);
#if defined(BL_JIT_ARCH_X86)
  pc->v_zero_i(vec_zero);                                        //   vec_zero = 0;
#endif // BL_JIT_ARCH_X86
  pc->v_storea128(mem_ptr(cell_ptr), vec_zero);                  //   cell_ptr[3:0] = 0;

  // `i` is now the number of pixels (and cells) to composite by using `v_mask`.

  pc->sub(i, i, x0);                                             //   i -= x0;
  pc->add(x0, x0, i);                                            //   x0 += i;
  pc->j(L_VLoop_Init);                                           //   goto L_VLoop_Init;

  // VMaskLoop - Main VMask Loop - 8 Pixels (256-bit SIMD)
  // -----------------------------------------------------

#if defined(BL_JIT_ARCH_X86)
  if (v_proc_pixel_count == 8u) {
    Label L_VLoop_Iter8 = pc->new_label();
    Label L_VLoop_End = pc->new_label();

    pc->bind(L_VLoop_Iter8);                                     // L_VLoop_Iter8:
    pc->v_extract_v128(acc, acc, 1);

    pass_vec_coverage(comp_cov, m, PixelCount(8), pixel_type, coverage_format);
    comp_op_part()->v_mask_proc_store_advance(dst_ptr, PixelCount(8), comp_cov, PixelCoverageFlags::kNone);

    pc->add(cell_ptr, cell_ptr, 8 * 4);                          //   cell_ptr += 8 * sizeof(uint32_t);
    pc->v_add_i32(acc, acc, mem_ptr(cell_ptr));                  //   acc[7:0] += cell_ptr[7:0]
    pc->v_zero_i(vec_zero);                                      //   vec_zero = 0;
    pc->v_storeu256(mem_ptr(cell_ptr, -16), vec_zero.v256());    //   cell_ptr[3:-4] = 0;

    pc->bind(L_VLoop_Init);                                      // L_VLoop_Init:
    accumulate_coverages(acc);
    calc_masks_from_cells(m[0], acc, fill_rule_mask, global_alpha);
    normalize_coverages(acc);
    expand_mask(m, PixelCount(8));

    pc->j(L_VLoop_Iter8, sub_nc(i, 8));                          //   if ((i -= 8) >= 0) goto L_VLoop_Iter8;
    pc->j(L_VLoop_End, add_z(i, 8));                             //   if ((i += 8) == 0) goto L_VLoop_End;
    pc->j(L_VTail_Init, ucmp_lt(i, 4));                          //   if (i < 4) goto L_VTail_Init;

    pc->add(cell_ptr, cell_ptr, 4 * 4);                          //   cell_ptr += 4 * sizeof(uint32_t);
    pc->v_zero_i(vec_zero);                                      //   vec_zero = 0;
    pc->v_storea128(mem_ptr(cell_ptr), vec_zero.v128());         //   cell_ptr[3:0] = 0;

    pass_vec_coverage(comp_cov, m, PixelCount(4), pixel_type, coverage_format);
    comp_op_part()->v_mask_proc_store_advance(dst_ptr, PixelCount(4), comp_cov, PixelCoverageFlags::kImmutable);
    if (pixel_type == PixelType::kRGBA32) {
      if (m[0].is_vec512())
        pc->cc->vshufi32x4(m[0], m[0], m[0], x86::shuffle_imm(3, 2, 3, 2)); // m[0] = [a7 a7 a7 a7 a6 a6 a6 a6|a5 a5 a5 a5 a4 a4 a4 a4]
      else
        pc->v_mov(m[0], m[1]);                                   //   m[0] = [a7 a7 a7 a7 a6 a6 a6 a6|a5 a5 a5 a5 a4 a4 a4 a4]
    }
    else if (pixel_type == PixelType::kA8) {
      pc->v_swizzle_u32x4(m[0], m[0], swizzle(3, 2, 3, 2));      //   m[0] = [?? ?? ?? ?? ?? ?? ?? ??|a7 a6 a5 a4 a7 a6 a5 a4]
    }
    else {
      BL_NOT_REACHED();
    }

    pc->v_extract_v128(acc, acc, 1);
    pc->j(L_VTail_Init, sub_nz(i, 4));                           //   if ((i -= 4) > 0) goto L_VTail_Init;

    pc->bind(L_VLoop_End);                                       // L_VLoop_End:
    pc->v_extract_v128(acc, acc, 0);
    pc->j(L_Scanline_Done1, ucmp_ge(x0, x_end));                 //   if (x0 >= x_end) goto L_Scanline_Done1;
  }
  else
#endif

  // VMask Loop - Main VMask Loop - 4 Pixels
  // ---------------------------------------

  if (v_proc_pixel_count == 4u) {
    Label L_VLoop_Cont = pc->new_label();

    pc->bind(L_VLoop_Cont);                                      // L_VLoop_Cont:

    pass_vec_coverage(comp_cov, m, PixelCount(4), pixel_type, coverage_format);
    comp_op_part()->v_mask_proc_store_advance(dst_ptr, PixelCount(4), comp_cov, PixelCoverageFlags::kNone);

    pc->add(cell_ptr, cell_ptr, 4 * 4);                          //   cell_ptr += 4 * sizeof(uint32_t);
    pc->v_add_i32(acc, acc, mem_ptr(cell_ptr));                  //   acc[3:0] += cell_ptr[3:0];
#if defined(BL_JIT_ARCH_X86)
    pc->v_zero_i(vec_zero);                                      //   vec_zero = 0;
#endif // BL_JIT_ARCH_X86
    pc->v_storea128(mem_ptr(cell_ptr), vec_zero);                //   cell_ptr[3:0] = 0;
    d_pix.reset_all_except_type_and_name();

    pc->bind(L_VLoop_Init);                                      // L_VLoop_Init:
    accumulate_coverages(acc);
    calc_masks_from_cells(m[0], acc, fill_rule_mask, global_alpha);
    normalize_coverages(acc);
    expand_mask(m, PixelCount(4));

    pc->j(L_VLoop_Cont, sub_nc(i, 4));                           //   if ((i -= 4) >= 0) goto L_VLoop_Cont;
    pc->j(L_VTail_Init, add_nz(i, 4));                           //   if ((i += 4) != 0) goto L_VTail_Init;
    pc->j(L_Scanline_Done1, ucmp_ge(x0, x_end));                 //   if (x0 >= x_end) goto L_Scanline_Done1;
  }

  // VMask Loop - Main VMask Loop - 1 Pixel
  // --------------------------------------

  else {
    Label L_VLoop_Iter = pc->new_label();
    Label L_VLoop_Step = pc->new_label();

    Gp n = pc->new_gp32("n");

    pc->bind(L_VLoop_Iter);                                      // L_VLoop_Iter:
    pc->umin(n, i, 4);                                           //   n = umin(i, 4);
    pc->sub(i, i, n);                                            //   i -= n;
    pc->add_scaled(cell_ptr, n, 4);                              //   cell_ptr += n * 4;

    if (pixel_granularity >= 4)
      comp_op_part()->enter_partial_mode();                      //   <CompOpPart::enter_partial_mode>

    if (pixel_type == PixelType::kRGBA32) {
      constexpr PixelFlags kPC_Immutable = PixelFlags::kPC | PixelFlags::kImmutable;

#if defined(BL_JIT_ARCH_X86)
      if (!pc->has_avx2()) {
        // Broadcasts were introduced by AVX2, so we generally don't want to use code that relies on them as they
        // would expand to more than a single instruction. So instead of a broadcast, we pre-shift the input in a
        // way so we can use a single [V]PSHUFLW to shuffle the components to places where the compositor needs them.
        pc->v_sllb_u128(m[0], m[0], 6);                          //   m0[7:0] = [__ a3 a2 a1 a0 __ __ __]

        pc->bind(L_VLoop_Step);                                  // L_VLoop_Step:
        pc->v_swizzle_lo_u16x4(m[0], m[0], swizzle(3, 3, 3, 3)); // m0[7:0] = [__ a3 a2 a1 a0 a0 a0 a0]

        comp_cov.init(m[0].v128());
        comp_op_part()->v_mask_proc_rgba32_vec(d_pix, PixelCount(1), kPC_Immutable, comp_cov, PixelCoverageFlags::kImmutable, pc->empty_predicate());
      }
      else
#endif
      {
        Vec vm_tmp = pc->new_vec128("@vm_tmp");
        pc->bind(L_VLoop_Step);                                  // L_VLoop_Step:

        if (coverage_format == PixelCoverageFormat::kPacked)
          pc->v_broadcast_u8(vm_tmp, m[0].v128());               //   vm_tmp[15:0] = [a0 a0 a0 a0 a0 a0 a0 a0|a0 a0 a0 a0 a0 a0 a0 a0]
        else
          pc->v_broadcast_u16(vm_tmp, m[0].v128());              //   vm_tmp[15:0] = [_0 a0 _0 a0 _0 a0 _0 a0|_0 a0 _0 a0 _0 a0 _0 a0]

        comp_cov.init(vm_tmp);
        comp_op_part()->v_mask_proc_rgba32_vec(d_pix, PixelCount(1), kPC_Immutable, comp_cov, PixelCoverageFlags::kNone, pc->empty_predicate());
      }

      pc->xStorePixel(dst_ptr, d_pix.pc[0], 1, dst_bpp, Alignment(1));
      d_pix.reset_all_except_type_and_name();
    }
    else if (pixel_type == PixelType::kA8) {
      pc->bind(L_VLoop_Step);                                    // L_VLoop_Step:

      Gp msk = pc->new_gp32("@msk");
      pc->s_extract_u16(msk, m[0], 0);

      comp_op_part()->v_mask_proc_a8_gp(d_pix, PixelFlags::kSA | PixelFlags::kImmutable, msk, PixelCoverageFlags::kNone);

      pc->store_u8(mem_ptr(dst_ptr), d_pix.sa);
      d_pix.reset_all_except_type_and_name();
    }

    pc->add(dst_ptr, dst_ptr, dst_bpp);                          //   dst_ptr += dst_bpp;
    pc->shift_or_rotate_right(m[0], m[0], 2);                    //   m0[15:0] = [??, m[15:2]]

    if (pixel_granularity >= 4)                                  //   if (pixel_granularity >= 4)
      comp_op_part()->next_partial_pixel();                      //     <CompOpPart::next_partial_pixel>

    pc->j(L_VLoop_Step, sub_nz(n, 1));                           //   if (--n != 0) goto L_VLoop_Step;

    if (pixel_granularity >= 4)                                  //   if (pixel_granularity >= 4)
      comp_op_part()->exit_partial_mode();                       //     <CompOpPart::exit_partial_mode>

#if defined(BL_JIT_ARCH_X86)
    if (!pc->has_avx()) {
      // We must use unaligned loads here as we don't know whether we are at the end of the scanline.
      // In that case `cell_ptr` could already be misaligned if the image width is not divisible by 4.
      Vec cov_tmp = pc->new_vec128("@cov_tmp");
      pc->v_loadu128(cov_tmp, mem_ptr(cell_ptr));                //   cov_tmp[3:0] = cell_ptr[3:0];
      pc->v_add_i32(acc, acc, cov_tmp);                          //   acc[3:0] += cov_tmp
    }
    else
#endif // BL_JIT_ARCH_X86
    {
      pc->v_add_i32(acc, acc, mem_ptr(cell_ptr));                //   acc[3:0] += cell_ptr[3:0]
    }

#if defined(BL_JIT_ARCH_X86)
    pc->v_zero_i(vec_zero);                                      //   vec_zero = 0;
#endif
    pc->v_storeu128(mem_ptr(cell_ptr), vec_zero);                //   cell_ptr[3:0] = 0;

    pc->bind(L_VLoop_Init);                                      // L_VLoop_Init:

    accumulate_coverages(acc);
    calc_masks_from_cells(m[0], acc, fill_rule_mask, global_alpha);
    normalize_coverages(acc);

    pc->j(L_VLoop_Iter, test_nz(i));                             //   if (i != 0) goto L_VLoop_Iter;
    pc->j(L_Scanline_Done1, ucmp_ge(x0, x_end));                 //   if (x0 >= x_end) goto L_Scanline_Done1;
  }

  // BitGap
  // ------

  // If we are here we are at the end of `v_mask` loop. There are two possibilities:
  //
  //   1. There is a gap between bits in a single or multiple BitWords. This means that there is a possibility
  //      for a `c_mask` loop which could be fully opaque, semi-transparent, or fully transparent (a real gap).
  //
  //   2. This was the last span and there are no more bits in consecutive BitWords. We will not consider this as
  //      a special case and just process the remaining BitWords in a normal way (scanning until the end of the
  //      current scanline).

  Label L_BitGap_Match = pc->new_label();
  Label L_BitGap_Cont = pc->new_label();

  pc->j(L_BitGap_Match, test_nz(bit_word));                      //   if (bit_word != 0) goto L_BitGap_Match;

  // Loop unrolled 2x as we could be inside a larger span.

  pc->bind(L_BitGap_Cont);                                       // L_BitGap_Cont:
  pc->add(x_off, x_off, pixels_per_bit_word);                    //   x_off += pixels_per_bit_word;
  pc->j(L_Scanline_Done1, cmp_eq(bit_ptr, bit_ptr_end));         //   if (bit_ptr == bit_ptr_end) goto L_Scanline_Done1;

  pc->load(bit_word, mem_ptr(bit_ptr));                          //   bit_word = bit_ptr[0];
  pc->add(bit_ptr, bit_ptr, bw_size);                            //   bit_ptr += bw_size;
  pc->j(L_BitGap_Match, test_nz(bit_word));                      //   if (bit_word != 0) goto L_BitGap_Match;

  pc->add(x_off, x_off, pixels_per_bit_word);                    //   x_off += pixels_per_bit_word;
  pc->j(L_Scanline_Done1, cmp_eq(bit_ptr, bit_ptr_end));         //   if (bit_ptr == bit_ptr_end) goto L_Scanline_Done1;

  pc->load(bit_word, mem_ptr(bit_ptr));                          //   bit_word = bit_ptr[0];
  pc->add(bit_ptr, bit_ptr, bw_size);                            //   bit_ptr += bw_size;
  pc->j(L_BitGap_Cont, test_z(bit_word));                        //   if (bit_word == 0) goto L_BitGap_Cont;

  pc->bind(L_BitGap_Match);                                      // L_BitGap_Match:
  pc->store_zero_reg(mem_ptr(bit_ptr, -bw_size));                //   bit_ptr[-1] = 0;
  count_zeros(i.clone_as(bit_word), bit_word);                   //   i = ctz(bit_word) or clz(bit_word);
  pc->mov(bit_word_tmp, -1);                                     //   bit_word_tmp = -1; (all ones)

  if (coverage_format == PixelCoverageFormat::kPacked)
    pc->s_extract_u8(c_mask_alpha, m[0], 0);                     //   c_mask_alpha = s_extract_u8(m0, 0);
  else
    pc->s_extract_u16(c_mask_alpha, m[0], 0);                    //   c_mask_alpha = s_extract_u16(m0, 0);

  shift_mask(bit_word_tmp, bit_word_tmp, i);                     //   bit_word_tmp = bit_word_tmp << i or bit_word_tmp >> i;
  pc->shl(i, i, imm(pixels_per_one_bit_shift));                  //   i <<= pixels_per_one_bit_shift;

  pc->xor_(bit_word, bit_word, bit_word_tmp);                    //   bit_word ^= bit_word_tmp;
  pc->add(i, i, x_off);                                          //   i += x_off;
  pc->sub(i, i, x0);                                             //   i -= x0;
  pc->add(x0, x0, i);                                            //   x0 += i;
  pc->add_scaled(cell_ptr, i.clone_as(cell_ptr), 4);             //   cell_ptr += i * sizeof(uint32_t);
  pc->j(L_CLoop_Init, test_nz(c_mask_alpha));                    //   if (c_mask_alpha != 0) goto L_CLoop_Init;

  // Fully-Transparent span where `c_mask_alpha == 0`.

  pc->add_scaled(dst_ptr, i.clone_as(dst_ptr), int(dst_bpp));    //   dst_ptr += i * dst_bpp;

  if (v_proc_pixel_count >= 4)
    comp_op_part()->postfetch_n();

  comp_op_part()->advance_x(x0, i);

  if (v_proc_pixel_count >= 4)
    comp_op_part()->prefetch_n();

  pc->j(L_BitScan_Match, test_nz(bit_word));                     //   if (bit_word != 0) goto L_BitScan_Match;
  pc->j(L_BitScan_Iter);                                         //   goto L_BitScan_Iter;

  // CMask - Loop
  // ------------

  pc->bind(L_CLoop_Init);                                        // L_CLoop_Init:
  if (comp_op_part()->should_optimize_opaque_fill()) {
    Label L_CLoop_Msk = pc->new_label();
    pc->j(L_CLoop_Msk, cmp_ne(c_mask_alpha, 255));               //   if (c_mask_alpha != 255) goto L_CLoop_Msk

    comp_op_part()->c_mask_init_opaque();
    if (pixel_granularity >= 4)
      comp_op_part()->c_mask_granular_loop(i);
    else
      comp_op_part()->c_mask_generic_loop(i);
    comp_op_part()->c_mask_fini();

    pc->j(L_BitScan_Match, test_nz(bit_word));                   //   if (bit_word != 0) goto L_BitScan_Match;
    pc->j(L_BitScan_Iter);                                       //   goto L_BitScan_Iter;

    pc->bind(L_CLoop_Msk);                                       // L_CLoop_Msk:
  }

  if (coverage_format == PixelCoverageFormat::kPacked) {
    pc->v_broadcast_u8(m[0], m[0]);                              //   m0 = [a0 a0 a0 a0 a0 a0 a0 a0|a0 a0 a0 a0 a0 a0 a0 a0]
  }
#if defined(BL_JIT_ARCH_X86)
  else if (!pc->has_avx2()) {
    pc->v_swizzle_u32x4(m[0], m[0], swizzle(0, 0, 0, 0));        //   m0 = [_0 a0 _0 a0 _0 a0 _0 a0|_0 a0 _0 a0 _0 a0 _0 a0]
  }
#endif
  else {
    pc->v_broadcast_u16(m[0], m[0]);                             //   m0 = [_0 a0 _0 a0 _0 a0 _0 a0|_0 a0 _0 a0 _0 a0 _0 a0]
  }

  comp_op_part()->c_mask_init(c_mask_alpha, m[0]);
  if (pixel_granularity >= 4)
    comp_op_part()->c_mask_granular_loop(i);
  else
    comp_op_part()->c_mask_generic_loop(i);
  comp_op_part()->c_mask_fini();

  pc->j(L_BitScan_Match, test_nz(bit_word));                     //   if (bit_word != 0) goto L_BitScan_Match;
  pc->j(L_BitScan_Iter);                                         //   goto L_BitScan_Iter;

  // VMask - Tail - Tail `v_mask` loop for pixels near the end of the scanline
  // ------------------------------------------------------------------------

  if (max_pixels >= 4u) {
    Label L_VTail_Cont = pc->new_label();

    Vec m128 = m[0].v128();
    VecArray msk(m128);

    // Tail loop can handle up to `pixels_per_one_bit - 1`.
    if (pixel_type == PixelType::kRGBA32) {
      bool hasV256Mask = m[0].size() >= 32u;

      pc->bind(L_VTail_Init);                                    // L_VTail_Init:
      pc->add_scaled(cell_ptr, i, 4);                            //   cell_ptr += i * sizeof(uint32_t);

      if (coverage_format == PixelCoverageFormat::kUnpacked && !hasV256Mask) {
        pc->v_swap_u64(m[1], m[1]);
      }
      comp_op_part()->enter_partial_mode();                      //   <CompOpPart::enter_partial_mode>

      pc->bind(L_VTail_Cont);                                    // L_VTail_Cont:
      comp_op_part()->v_mask_proc_rgba32_vec(d_pix, PixelCount(1), PixelFlags::kPC | PixelFlags::kImmutable, msk, PixelCoverageFlags::kImmutable, pc->empty_predicate());

      pc->xStorePixel(dst_ptr, d_pix.pc[0], 1, dst_bpp, Alignment(1));
      pc->add(dst_ptr, dst_ptr, dst_bpp);                        //   dst_ptr += dst_bpp;

      if (coverage_format == PixelCoverageFormat::kPacked) {
        pc->shift_or_rotate_right(m[0], m[0], 4);                //   m0[15:0] = [????, m[15:4]]
      }
      else {
#if defined(BL_JIT_ARCH_X86)
        if (hasV256Mask) {
          // All 4 expanded masks for ARGB channels are in a single register, so just permute.
          pc->v_swizzle_u64x4(m[0], m[0], swizzle(0, 3, 2, 1));
        }
        else
#endif
        {
          pc->v_interleave_hi_u64(m[0], m[0], m[1]);
        }
      }

      comp_op_part()->next_partial_pixel();                      //   <CompOpPart::next_partial_pixel>
      d_pix.reset_all_except_type_and_name();
      pc->j(L_VTail_Cont, sub_nz(i, 1));                         //   if (--i) goto L_VTail_Cont;

      comp_op_part()->exit_partial_mode();                       //   <CompOpPart::exit_partial_mode>
    }
    else if (pixel_type == PixelType::kA8) {
      Gp mScalar = pc->new_gp32("mScalar");

      pc->bind(L_VTail_Init);                                    // L_VTail_Init:
      pc->add_scaled(cell_ptr, i, 4);                            //   cell_ptr += i * sizeof(uint32_t);
      comp_op_part()->enter_partial_mode();                      //   <CompOpPart::enter_partial_mode>

      pc->bind(L_VTail_Cont);                                    // L_VTail_Cont:
      if (coverage_format == PixelCoverageFormat::kPacked)
        pc->s_extract_u8(mScalar, m128, 0);
      else
        pc->s_extract_u16(mScalar, m128, 0);
      comp_op_part()->v_mask_proc_a8_gp(d_pix, PixelFlags::kSA | PixelFlags::kImmutable, mScalar, PixelCoverageFlags::kNone);

      pc->store_u8(mem_ptr(dst_ptr), d_pix.sa);
      pc->add(dst_ptr, dst_ptr, dst_bpp);                        //   dst_ptr += dst_bpp;
      if (coverage_format == PixelCoverageFormat::kPacked)
        pc->shift_or_rotate_right(m128, m128, 1);                //   m0[15:0] = [?, m[15:1]]
      else
        pc->shift_or_rotate_right(m128, m128, 2);                //   m0[15:0] = [??, m[15:2]]
      comp_op_part()->next_partial_pixel();                      //   <CompOpPart::next_partial_pixel>
      d_pix.reset_all_except_type_and_name();
      pc->j(L_VTail_Cont, sub_nz(i, 1));                         //   if (--i) goto L_VTail_Cont;

      comp_op_part()->exit_partial_mode();                       //   <CompOpPart::exit_partial_mode>
    }

    // Since this was a tail loop we know that there is nothing to be processed afterwards, because tail loop is only
    // possible at the end of the scanline boundary / clip region.
  }

  // Scanline Iterator
  // -----------------

  // This loop is used to quickly test bit_words in `bit_ptr`. In some cases the whole scanline could be empty, so this
  // loop makes sure we won't enter more complicated loops if this happens. It's also used to quickly find the first
  // bit, which is non-zero - in that case it jumps directly to BitScan section.
  //
  // NOTE: Storing zeros to `cell_ptr` must be unaligned here as we may be at the end of the scanline.

  pc->bind(L_Scanline_Done0);                                    // L_Scanline_Done0:
#if defined(BL_JIT_ARCH_X86)
  pc->v_zero_i(vec_zero);                                        //   vec_zero = 0;
#endif // BL_JIT_ARCH_X86
  pc->v_storeu128(mem_ptr(cell_ptr), vec_zero);                  //   cell_ptr[3:0] = 0;

  pc->bind(L_Scanline_Done1);                                    // L_Scanline_Done1:
  deadvance_dst_ptr_and_cell_ptr(dst_ptr,                        //   dst_ptr -= x0 * dst_bpp;
                            cell_ptr, x0, dst_bpp);              //   cell_ptr -= x0 * sizeof(uint32_t);
  pc->j(L_End, sub_z(y, 1));                                     //   if (--y == 0) goto L_End;
  pc->mov(bit_ptr, bit_ptr_end);                                 //   bit_ptr = bit_ptr_end;

  pc->bind(L_Scanline_AdvY);                                     // L_Scanline_AdvY:
  pc->add(dst_ptr, dst_ptr, dst_stride);                         //   dst_ptr += dst_stride;
  pc->add(bit_ptr, bit_ptr, bit_ptr_skip_len);                   //   bit_ptr += bit_ptr_skip_len;
  pc->add(cell_ptr, cell_ptr, cell_stride);                      //   cell_ptr += cell_stride;
  comp_op_part()->advance_y();                                   //   <CompOpPart::AdvanceY>

  pc->bind(L_Scanline_Init);                                     // L_Scanline_Init:
  pc->mov(x_off, x_start);                                       //   x_off = x_start;
  pc->add(bit_ptr_end, bit_ptr, bit_ptr_run_len);                //   bit_ptr_end = bit_ptr + bit_ptr_run_len;

  pc->bind(L_Scanline_Iter);                                     // L_Scanline_Iter:
  pc->load(bit_word, mem_ptr(bit_ptr));                          //   bit_word = bit_ptr[0];
  pc->add(bit_ptr, bit_ptr, bw_size);                            //   bit_ptr += bw_size;
  pc->j(L_BitScan_Init, test_nz(bit_word));                      //   if (bit_word != 0) goto L_BitScan_Init;

  pc->add(x_off, x_off, pixels_per_bit_word);                    //   x_off += pixels_per_bit_word;
  pc->j(L_Scanline_Iter, cmp_ne(bit_ptr, bit_ptr_end));          //   if (bit_ptr != bit_ptr_end) goto L_Scanline_Iter;
  pc->j(L_Scanline_AdvY, sub_nz(y, 1));                          //   if (--y) goto L_Scanline_AdvY;

  // Epilog
  // ------

  pc->bind(L_End);
  comp_op_part()->fini();
  _fini_global_hook();
}

void FillAnalyticPart::accumulate_coverages(const Vec& acc) noexcept {
  Vec tmp = pc->new_similar_reg<Vec>(acc, "vCovTmp");

  pc->v_sllb_u128(tmp, acc, 4);                                  //   tmp[7:0]  = [  c6    c5    c4    0  |  c2    c1    c0    0  ];
  pc->v_add_i32(acc, acc, tmp);                                  //   acc[7:0]  = [c7:c6 c6:c5 c5:c4   c4 |c3:c2 c2:c1 c1:c0   c0 ];
  pc->v_sllb_u128(tmp, acc, 8);                                  //   tmp[7:0]  = [c5:c4   c4    0     0  |c1:c0   c0    0     0  ];
  pc->v_add_i32(acc, acc, tmp);                                  //   acc[7:0]  = [c7:c4 c6:c4 c5:c4   c4 |c3:c0 c2:c0 c1:c0   c0 ];

#if defined(BL_JIT_ARCH_X86)
  if (acc.is_vec256()) {
    pc->v_swizzle_u32x4(tmp.v128(), acc.v128(), swizzle(3, 3, 3, 3));
    cc->vperm2i128(tmp, tmp, tmp, perm_2x128_imm(Perm2x128::kALo, Perm2x128::kZero));
    pc->v_add_i32(acc, acc, tmp);                                //   acc[7:0]  = [c7:c0 c6:c0 c5:c0 c4:c0|c3:c0 c2:c0 c1:c0   c0 ];
  }
#endif // BL_JIT_ARCH_X86
}

void FillAnalyticPart::normalize_coverages(const Vec& acc) noexcept {
  pc->v_srlb_u128(acc, acc, 12);                                 //   acc[3:0]  = [  0     0     0     c0 ];
}

// Calculate masks from cell and store them to a vector of the following layout:
//
//   [__ __ __ __ a7 a6 a5 a4|__ __ __ __ a3 a2 a1 a0]
//
// NOTE: Depending on the vector size the output mask is for either 4 or 8 pixels.
void FillAnalyticPart::calc_masks_from_cells(const Vec& msk_, const Vec& acc, const Vec& fill_rule_mask, const Vec& global_alpha) noexcept {
  Vec msk = msk_.clone_as(acc);

#if defined(BL_JIT_ARCH_X86)
  // This implementation is a bit tricky. In the original AGG and FreeType `A8_SHIFT + 1` is used. However, we don't do
  // that and mask out the last bit through `fill_rule_mask`. The reason we do this is that our `global_alpha` is already
  // pre-shifted by `7` bits left and we only need to shift the final mask by one bit left after it's been calculated.
  // So instead of shifting it left later we clear the LSB bit now and that's it, we saved one instruction.
  pc->v_srai_i32(msk, acc, A8Info::kShift);
  pc->v_and_i32(msk, msk, fill_rule_mask);

  // We have to make sure that the cleared LSB bit stays zero. Since we only use SUB with even value and abs we are
  // fine. However, that packing would not be safe if there was no "v_min_i16", which makes sure we are always safe.
  Operand i_0x00000200 = pc->simd_const(&ct.p_0000020000000200, Bcst::k32, msk);
  pc->v_sub_i32(msk, msk, i_0x00000200);
  pc->v_abs_i32(msk, msk);

  if (pc->has_sse4_1()) {
    // This is not really faster, but it uses the same constant as one of the previous operations, potentially saving
    // us a register.
    pc->v_min_u32(msk, msk, i_0x00000200);
    pc->v_packs_i32_i16(msk, msk, msk);
  }
  else {
    pc->v_packs_i32_i16(msk, msk, msk);
    pc->v_min_i16(msk, msk, pc->simd_const(&ct.p_0200020002000200, Bcst::kNA, msk));
  }

  // Multiply masks by global alpha, this would output masks in [0, 255] range.
  pc->v_mulh_u16(msk, msk, global_alpha);
#else
  // This implementation doesn't need any tricks as a lot of SIMD primitives are just provided natively.
  pc->v_srai_i32(msk, acc, A8Info::kShift + 1);
  pc->v_and_i32(msk, msk, fill_rule_mask);

  pc->v_sub_i32(msk, msk, pc->simd_const(&ct.p_0000010000000100, Bcst::k32, msk));
  pc->v_abs_i32(msk, msk);
  pc->v_min_u32(msk, msk, pc->simd_const(&ct.p_0000010000000100, Bcst::kNA, msk));

  pc->v_mul_u16(msk, msk, global_alpha);
  pc->cc->shrn(msk.h4(), msk.s4(), 8);
#endif
}

void FillAnalyticPart::expand_mask(const VecArray& msk, PixelCount pixel_count) noexcept {
  PixelType pixel_type = comp_op_part()->pixel_type();
  PixelCoverageFormat coverage_format = comp_op_part()->coverage_format();

  if (pixel_type == PixelType::kRGBA32) {
    switch (coverage_format) {
#if defined(BL_JIT_ARCH_A64)
      case PixelCoverageFormat::kPacked: {
        uint32_t n_regs = (uint32_t(pixel_count) + 3u) / 4u;
        for (uint32_t i = 0; i < n_regs; i++) {
          Vec v = msk[i].v128();
          pc->v_swizzlev_u8(v, v, pc->simd_const(&ct.swizu8_xxxxxxxxx3x2x1x0_to_3333222211110000, Bcst::kNA, v));
        }
        return;
      }
#endif // BL_JIT_ARCH_A64

      case PixelCoverageFormat::kUnpacked: {
        if (pixel_count == PixelCount(4)) {
          Vec cov0_128 = msk[0].v128();

          pc->v_interleave_lo_u16(cov0_128, cov0_128, cov0_128);      //   msk[0] = [a3 a3 a2 a2 a1 a1 a0 a0]
#if defined(BL_JIT_ARCH_X86)
          if (msk[0].is_vec256()) {
            pc->v_swizzle_u64x4(msk[0], msk[0], swizzle(1, 1, 0, 0)); //   msk[0] = [a3 a3 a2 a2 a3 a3 a2 a2|a1 a1 a0 a0 a1 a1 a0 a0]
            pc->v_swizzle_u32x4(msk[0], msk[0], swizzle(1, 1, 0, 0)); //   msk[0] = [a3 a3 a3 a3 a2 a2 a2 a2|a1 a1 a1 a1 a0 a0 a0 a0]
          }
          else
#endif // BL_JIT_ARCH_X86
          {
            pc->v_swizzle_u32x4(msk[1], msk[0], swizzle(3, 3, 2, 2)); //   msk[0] = [a3 a3 a3 a3 a2 a2 a2 a2]
            pc->v_swizzle_u32x4(msk[0], msk[0], swizzle(1, 1, 0, 0)); //   msk[0] = [a1 a1 a1 a1 a0 a0 a0 a0]
          }

          return;
        }

#if defined(BL_JIT_ARCH_X86)
        if (pixel_count == PixelCount(8)) {
          if (msk[0].is_vec512()) {
            if (pc->has_avx512_vbmi()) {
              Vec pred = pc->simd_vec_const(&ct.permu8_4xa8_lo_to_rgba32_uc, Bcst::kNA_Unique, msk[0]);
              pc->v_permute_u8(msk[0], pred, msk[0]);                 //   msk[0] = [4x00a7 4x00a6 4x00a5 4x00a4|4x00a3 4x00a2 4x00a1 4x00a0]
            }
            else {
              Vec msk_256 = msk[0].v256();
              Operand pred = pc->simd_const(&ct.swizu8_xxxxxxxxx3x2x1x0_to_3333222211110000, Bcst::kNA, msk_256);
              pc->v_swizzlev_u8(msk_256, msk_256, pred);              //   msk[0] = [2xa7a7 3xa6a6 3xa5a5 2xa4a4|2xa3a3 2xa2a2 2xa1a1 2xa0a0]
              pc->v_cvt_u8_lo_to_u16(msk[0], msk_256);                //   msk[0] = [4x00a7 4x00a6 4x00a5 4x00a4|4x00a3 4x00a2 4x00a1 4x00a0]
            }
          }
          else {
            //                                                             msk[0] = [__ __ __ __ a7 a6 a5 a4|__ __ __ __ a3 a2 a1 a0]
            pc->v_interleave_lo_u16(msk[0], msk[0], msk[0]);          //   msk[0] = [a7 a7 a6 a6 a5 a5 a4 a4|a3 a3 a2 a2 a1 a1 a0 a0]
            pc->v_swizzle_u64x4(msk[1], msk[0], swizzle(3, 3, 2, 2)); //   msk[1] = [a7 a7 a6 a6 a7 a7 a6 a6|a5 a5 a4 a4 a5 a5 a4 a4]
            pc->v_swizzle_u64x4(msk[0], msk[0], swizzle(1, 1, 0, 0)); //   msk[0] = [a3 a3 a2 a2 a3 a3 a2 a2|a1 a1 a0 a0 a1 a1 a0 a0]
            pc->v_interleave_lo_u32(msk[0], msk[0], msk[0]);          //   msk[0] = [a3 a3 a3 a3 a2 a2 a2 a2|a1 a1 a1 a1 a0 a0 a0 a0]
            pc->v_interleave_lo_u32(msk[1], msk[1], msk[1]);          //   msk[1] = [a7 a7 a7 a7 a6 a6 a6 a6|a5 a5 a5 a5 a4 a4 a4 a4]
          }
          return;
        }
#endif // BL_JIT_ARCH_X86

        break;
      }

      default:
        BL_NOT_REACHED();
    }
  }
  else if (pixel_type == PixelType::kA8) {
    switch (coverage_format) {
      case PixelCoverageFormat::kPacked: {
        if (pixel_count <= PixelCount(8)) {
          Vec v = msk[0].v128();
          pc->v_packs_i16_u8(v, v, v);
          return;
        }

        break;
      }

      case PixelCoverageFormat::kUnpacked: {
        if (pixel_count <= PixelCount(4))
          return;

#if defined(BL_JIT_ARCH_X86)
        // We have to convert from:
        //   msk = [?? ?? ?? ?? a7 a6 a5 a4|?? ?? ?? ?? a3 a2 a1 a0]
        // To:
        //   msk = [a7 a6 a5 a4 a3 a2 a1 a0|a7 a6 a5 a4 a3 a2 a1 a0]
        pc->v_swizzle_u64x4(msk[0].ymm(), msk[0].ymm(), swizzle(2, 0, 2, 0));
#endif // BL_JIT_ARCH_X86

        return;
      }

      default:
        BL_NOT_REACHED();
    }
  }

  BL_NOT_REACHED();
}

void FillAnalyticPart::deadvance_dst_ptr_and_cell_ptr(const Gp& dst_ptr, const Gp& cell_ptr, const Gp& x, uint32_t dst_bpp) noexcept {
  Gp x_adv = x.clone_as(dst_ptr);

#if defined(BL_JIT_ARCH_A64)
  pc->cc->sub(cell_ptr, cell_ptr, x_adv, a64::lsl(2));
  if (asmjit::Support::is_power_of_2(dst_bpp)) {
    uint32_t shift = asmjit::Support::ctz(dst_bpp);
    pc->cc->sub(dst_ptr, dst_ptr, x_adv, a64::lsl(shift));
  }
  else {
    pc->mul(x_adv, x_adv, dst_bpp);
    pc->sub(dst_ptr, dst_ptr, x_adv);
  }
#else
  if (dst_bpp == 1) {
    pc->sub(dst_ptr, dst_ptr, x_adv);
    pc->shl(x_adv, x_adv, 2);
    pc->sub(cell_ptr, cell_ptr, x_adv);
  }
  else if (dst_bpp == 2) {
    pc->shl(x_adv, x_adv, 1);
    pc->sub(dst_ptr, dst_ptr, x_adv);
    pc->shl(x_adv, x_adv, 1);
    pc->sub(cell_ptr, cell_ptr, x_adv);
  }
  else if (dst_bpp == 4) {
    pc->shl(x_adv, x_adv, 2);
    pc->sub(dst_ptr, dst_ptr, x_adv);
    pc->sub(cell_ptr, cell_ptr, x_adv);
  }
  else {
    Gp dst_adv = pc->new_gpz("dst_adv");
    pc->mul(dst_adv, x_adv, dst_bpp);
    pc->shl(x_adv, x_adv, 2);
    pc->sub(dst_ptr, dst_ptr, dst_adv);
    pc->sub(cell_ptr, cell_ptr, x_adv);
  }
#endif
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
