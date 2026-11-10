// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_COMPOPPART_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_COMPOPPART_P_H_INCLUDED

#include <blend2d/core/compopinfo_p.h>
#include <blend2d/pipeline/jit/fetchpart_p.h>
#include <blend2d/pipeline/jit/fetchutilscoverage_p.h>
#include <blend2d/pipeline/jit/pipepart_p.h>
#include <blend2d/pipeline/jit/pipeprimitives_p.h>
#include <blend2d/support/wrap_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! Pipeline combine part.
class CompOpPart : public PipePart {
public:
  static inline constexpr uint32_t kIndexDstPart = 0;
  static inline constexpr uint32_t kIndexSrcPart = 1;

  //! \name Members
  //! \{

  //! Composition operator.
  CompOpExt _comp_op{};
  //! Pixel type of the composition.
  PixelType _pixel_type = PixelType::kNone;
  //! The current span mode.
  CMaskLoopType _c_mask_loop_type = CMaskLoopType::kNone;
  //! Pixel coverage format expected by the compositor.
  PixelCoverageFormat _coverage_format = PixelCoverageFormat::kNone;
  //! Maximum pixels the compositor can handle at a time.
  uint8_t _max_pixels = 0;
  //! Pixel granularity.
  PixelCount _pixel_granularity {};
  //! Minimum alignment required to process `_max_pixels`.
  Alignment _min_alignment {1};

  uint8_t _is_in_partial_mode : 1;
  //! Whether the destination format has an alpha component.
  uint8_t _has_da : 1;
  //! Whether the source format has an alpha component.
  uint8_t _has_sa : 1;

  //! A hook that is used by the current loop.
  asmjit::BaseNode* _c_mask_loop_hook = nullptr;
  //! Optimized solid pixel for operators that allow it.
  SolidPixel _solid_opt;
  //! Pre-processed solid pixel for TypeA operators that always use `v_mask_proc?()`.
  Pixel _solid_pre {};
  //! Partial fetch that happened at the end of the scanline (border case).
  Pixel _partial_pixel {};
  //! Const mask.
  Wrap<PipeCMask> _mask;

  //! \}

  //! \name Construction & Destruction
  //! \{

  CompOpPart(PipeCompiler* pc, CompOpExt comp_op, FetchPart* dst_part, FetchPart* src_part) noexcept;

  //! \}

  //! \name Children
  //! \{

  BL_INLINE_NODEBUG FetchPart* dst_part() const noexcept { return reinterpret_cast<FetchPart*>(_children[kIndexDstPart]); }
  BL_INLINE_NODEBUG FetchPart* src_part() const noexcept { return reinterpret_cast<FetchPart*>(_children[kIndexSrcPart]); }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the composition operator id, see \ref BLCompOp.
  BL_INLINE_NODEBUG CompOpExt comp_op() const noexcept { return _comp_op; }

  BL_INLINE_NODEBUG bool is_src_copy() const noexcept { return _comp_op == CompOpExt::kSrcCopy; }
  BL_INLINE_NODEBUG bool is_src_over() const noexcept { return _comp_op == CompOpExt::kSrcOver; }
  BL_INLINE_NODEBUG bool is_src_in() const noexcept { return _comp_op == CompOpExt::kSrcIn; }
  BL_INLINE_NODEBUG bool is_src_out() const noexcept { return _comp_op == CompOpExt::kSrcOut; }
  BL_INLINE_NODEBUG bool is_src_atop() const noexcept { return _comp_op == CompOpExt::kSrcAtop; }
  BL_INLINE_NODEBUG bool is_dst_copy() const noexcept { return _comp_op == CompOpExt::kDstCopy; }
  BL_INLINE_NODEBUG bool is_dst_over() const noexcept { return _comp_op == CompOpExt::kDstOver; }
  BL_INLINE_NODEBUG bool is_dst_in() const noexcept { return _comp_op == CompOpExt::kDstIn; }
  BL_INLINE_NODEBUG bool is_dst_out() const noexcept { return _comp_op == CompOpExt::kDstOut; }
  BL_INLINE_NODEBUG bool is_dst_atop() const noexcept { return _comp_op == CompOpExt::kDstAtop; }
  BL_INLINE_NODEBUG bool is_xor() const noexcept { return _comp_op == CompOpExt::kXor; }
  BL_INLINE_NODEBUG bool is_plus() const noexcept { return _comp_op == CompOpExt::kPlus; }
  BL_INLINE_NODEBUG bool is_minus() const noexcept { return _comp_op == CompOpExt::kMinus; }
  BL_INLINE_NODEBUG bool is_modulate() const noexcept { return _comp_op == CompOpExt::kModulate; }
  BL_INLINE_NODEBUG bool is_multiply() const noexcept { return _comp_op == CompOpExt::kMultiply; }
  BL_INLINE_NODEBUG bool is_screen() const noexcept { return _comp_op == CompOpExt::kScreen; }
  BL_INLINE_NODEBUG bool is_overlay() const noexcept { return _comp_op == CompOpExt::kOverlay; }
  BL_INLINE_NODEBUG bool is_darken() const noexcept { return _comp_op == CompOpExt::kDarken; }
  BL_INLINE_NODEBUG bool is_lighten() const noexcept { return _comp_op == CompOpExt::kLighten; }
  BL_INLINE_NODEBUG bool is_color_dodge() const noexcept { return _comp_op == CompOpExt::kColorDodge; }
  BL_INLINE_NODEBUG bool is_color_burn() const noexcept { return _comp_op == CompOpExt::kColorBurn; }
  BL_INLINE_NODEBUG bool is_linear_burn() const noexcept { return _comp_op == CompOpExt::kLinearBurn; }
  BL_INLINE_NODEBUG bool is_linear_light() const noexcept { return _comp_op == CompOpExt::kLinearLight; }
  BL_INLINE_NODEBUG bool is_pin_light() const noexcept { return _comp_op == CompOpExt::kPinLight; }
  BL_INLINE_NODEBUG bool is_hard_light() const noexcept { return _comp_op == CompOpExt::kHardLight; }
  BL_INLINE_NODEBUG bool is_soft_light() const noexcept { return _comp_op == CompOpExt::kSoftLight; }
  BL_INLINE_NODEBUG bool is_difference() const noexcept { return _comp_op == CompOpExt::kDifference; }
  BL_INLINE_NODEBUG bool is_exclusion() const noexcept { return _comp_op == CompOpExt::kExclusion; }

  BL_INLINE_NODEBUG bool is_alpha_inv() const noexcept { return _comp_op == CompOpExt::kAlphaInv; }

  //! Returns the composition operator flags.
  BL_INLINE_NODEBUG CompOpFlags comp_op_flags() const noexcept { return comp_op_info_table[size_t(_comp_op)].flags(); }
  //! Returns a pixel coverage format, which must be honored when calling the composition API.
  BL_INLINE_NODEBUG PixelCoverageFormat coverage_format() const noexcept { return _coverage_format; }

  //! Tests whether the destination pixel format has an alpha component.
  BL_INLINE_NODEBUG bool has_da() const noexcept { return _has_da != 0; }
  //! Tests whether the source pixel format has an alpha component.
  BL_INLINE_NODEBUG bool has_sa() const noexcept { return _has_sa != 0; }

  BL_INLINE_NODEBUG PixelType pixel_type() const noexcept { return _pixel_type; }
  BL_INLINE_NODEBUG bool is_a8_pixel() const noexcept { return _pixel_type == PixelType::kA8; }
  BL_INLINE_NODEBUG bool is_rgba32_pixel() const noexcept { return _pixel_type == PixelType::kRGBA32; }

  //! Returns the current loop mode.
  BL_INLINE_NODEBUG CMaskLoopType c_mask_loop_type() const noexcept { return _c_mask_loop_type; }
  //! Tests whether the current loop is fully opaque (no mask).
  BL_INLINE_NODEBUG bool is_loop_opaque() const noexcept { return _c_mask_loop_type == CMaskLoopType::kOpaque; }
  //! Tests whether the current loop is `CMask` (constant mask).
  BL_INLINE_NODEBUG bool is_loop_c_mask() const noexcept { return _c_mask_loop_type == CMaskLoopType::kVariant; }

  //! Returns the maximum pixels the composite part can handle at a time.
  //!
  //! \note This value is configured in a way that it's always one if the fetch part doesn't support more. This makes
  //! it easy to use it in loop compilers. In other words, the value doesn't describe the real implementation of the
  //! composite part.
  BL_INLINE_NODEBUG uint32_t max_pixels() const noexcept { return _max_pixels; }
  //! Returns the maximum pixels the children of this part can handle.
  BL_INLINE_NODEBUG uint32_t max_pixels_of_children() const noexcept { return bl_min(dst_part()->max_pixels(), src_part()->max_pixels()); }

  BL_INLINE void set_max_pixels(uint32_t max_pixels) noexcept {
    BL_ASSERT(max_pixels <= 0xFF);
    _max_pixels = uint8_t(max_pixels);
  }

  //! Returns pixel granularity passed to `init()`, otherwise the result should be zero.
  BL_INLINE_NODEBUG PixelCount pixel_granularity() const noexcept { return _pixel_granularity; }
  //! Returns the minimum destination alignment required to the maximum number of pixels `_max_pixels`.
  BL_INLINE_NODEBUG Alignment min_alignment() const noexcept { return _min_alignment; }

  BL_INLINE_NODEBUG bool is_using_solid_pre() const noexcept { return !_solid_pre.pc.is_empty() || !_solid_pre.uc.is_empty(); }
  BL_INLINE_NODEBUG bool is_in_partial_mode() const noexcept { return _is_in_partial_mode != 0; }

  //! \}

  void prepare_part() noexcept override;

  //! \name Initialization & Finalization
  //! \{

  void init(const PipeFunction& fn, Gp& x, Gp& y, uint32_t pixel_granularity) noexcept;
  void fini() noexcept;

  //! \}

  //! Tests whether the opaque fill should be optimized and placed into a separate loop. This means that if this
  //! function returns true two composition loops would be generated by the filler.
  bool should_optimize_opaque_fill() const noexcept;

  //! Tests whether the compositor should emit a specialized loop that contains an inlined version of `memcpy()`
  //! or `memset()`.
  bool should_just_copy_opaque_fill() const noexcept;

  void start_at_x(const Gp& x) noexcept;
  void advance_x(const Gp& x, const Gp& diff) noexcept;
  void advance_y() noexcept;

  // These are just wrappers that call these on both source & destination parts.
  void enter_n() noexcept;
  void leave_n() noexcept;
  void prefetch_n() noexcept;
  void postfetch_n() noexcept;

  void dst_fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept;
  void src_fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept;

  void enter_partial_mode(PixelFlags partial_flags = PixelFlags::kNone) noexcept;
  void exit_partial_mode() noexcept;
  void next_partial_pixel() noexcept;

  void c_mask_init(const Mem& mem) noexcept;
  void c_mask_init(const Gp& sm_, const Vec& vm_) noexcept;
  void c_mask_init_opaque() noexcept;
  void c_mask_fini() noexcept;

  void _c_mask_loop_init(CMaskLoopType loop_type) noexcept;
  void _c_mask_loop_fini() noexcept;

  void c_mask_generic_loop(Gp& i) noexcept;
  void c_mask_generic_loop_vec(Gp& i) noexcept;

  void c_mask_granular_loop(Gp& i) noexcept;
  void c_mask_granular_loop_vec(Gp& i) noexcept;

  void c_mask_memcpy_or_memset_loop(Gp& i) noexcept;

  void c_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, Alignment alignment = Alignment(1)) noexcept;
  void c_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, Alignment alignment, PixelPredicate& predicate) noexcept;

  void v_mask_generic_loop(Gp& i, const Gp& d_ptr, const Gp& mPtr, GlobalAlpha* ga, const Label& done) noexcept;
  void v_mask_generic_step(const Gp& d_ptr, PixelCount n, const Gp& mPtr, GlobalAlpha* ga) noexcept;
  void v_mask_generic_step(const Gp& d_ptr, PixelCount n, const Gp& mPtr, GlobalAlpha* ga, PixelPredicate& predicate) noexcept;

  void v_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, const VecArray& vm, PixelCoverageFlags coverage_flags, Alignment alignment = Alignment(1)) noexcept;
  void v_mask_proc_store_advance(const Gp& d_ptr, PixelCount n, const VecArray& vm, PixelCoverageFlags coverage_flags, Alignment alignment, PixelPredicate& predicate) noexcept;

  void v_mask_proc(Pixel& out, PixelFlags flags, Gp& msk, PixelCoverageFlags coverage_flags) noexcept;

  void c_mask_init_a8(const Gp& sm_, const Vec& vm_) noexcept;
  void c_mask_fini_a8() noexcept;

  void c_mask_proc_a8_gp(Pixel& out, PixelFlags flags) noexcept;
  void v_mask_proc_a8_gp(Pixel& out, PixelFlags flags, const Gp& msk, PixelCoverageFlags coverage_flags) noexcept;

  void c_mask_proc_a8_vec(Pixel& out, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept;
  void v_mask_proc_a8_vec(Pixel& out, PixelCount n, PixelFlags flags, const VecArray& vm, PixelCoverageFlags coverage_flags, PixelPredicate& predicate) noexcept;

  void c_mask_init_rgba32(const Vec& vm) noexcept;
  void c_mask_fini_rgba32() noexcept;

  void c_mask_proc_rgba32_vec(Pixel& out, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept;
  void v_mask_proc_rgba32_vec(Pixel& out, PixelCount n, PixelFlags flags, const VecArray& vm, PixelCoverageFlags coverage_flags, PixelPredicate& predicate) noexcept;

  void v_mask_proc_rgba32_invert_mask(VecArray& vn, const VecArray& vm, PixelCoverageFlags coverage_flags) noexcept;
  void v_mask_proc_rgba32_invert_done(VecArray& vn, const VecArray& vm, PixelCoverageFlags coverage_flags) noexcept;
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_COMPOPPART_P_H_INCLUDED
