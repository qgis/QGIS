// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FILLPART_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FILLPART_P_H_INCLUDED

#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/pipeline/jit/pipepart_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! Pipeline fill part.
class FillPart : public PipePart {
public:
  enum : uint32_t {
    kIndexDstPart = 0,
    kIndexCompOpPart = 1
  };

  //! Fill type.
  FillType _fill_type;

  FillPart(PipeCompiler* pc, FillType fill_type, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept;

  BL_INLINE_NODEBUG FetchPixelPtrPart* dst_part() const noexcept {
    return reinterpret_cast<FetchPixelPtrPart*>(_children[kIndexDstPart]);
  }

  BL_INLINE_NODEBUG void set_dst_part(FetchPixelPtrPart* part) noexcept {
    _children[kIndexDstPart] = reinterpret_cast<PipePart*>(part);
  }

  BL_INLINE_NODEBUG CompOpPart* comp_op_part() const noexcept {
    return reinterpret_cast<CompOpPart*>(_children[kIndexCompOpPart]);
  }

  BL_INLINE_NODEBUG void set_comp_op_part(FetchPixelPtrPart* part) noexcept {
    _children[kIndexCompOpPart] = reinterpret_cast<PipePart*>(part);
  }

  //! Returns fill type, see \ref FillType.
  BL_INLINE_NODEBUG FillType fill_type() const noexcept { return _fill_type; }
  //! Tests whether the fill type matches `fill_type`.
  BL_INLINE_NODEBUG bool is_fill_type(FillType fill_type) const noexcept { return _fill_type == fill_type; }

  //! Tests whether fill-type is a pure rectangular fill.
  //!
  //! Rectangle fills have some properties that can be exploited by other parts. For example if a fill is rectangular
  //! the pipeline may recalculate stride of source and destination pointers to address the width. There are currently
  //! many optimizations that individual parts do.
  BL_INLINE_NODEBUG bool is_analytic_fill() const noexcept { return _fill_type == FillType::kAnalytic; }

  //! Compiles the fill part.
  virtual void compile(const PipeFunction& fn) noexcept;
};

class FillBoxAPart final : public FillPart {
public:
  FillBoxAPart(PipeCompiler* pc, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept;

  void compile(const PipeFunction& fn) noexcept override;
};

class FillMaskPart final : public FillPart {
public:
  FillMaskPart(PipeCompiler* pc, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept;

  void compile(const PipeFunction& fn) noexcept override;
  void deadvance_dst_ptr(const Gp& dst_ptr, const Gp& x, int dst_bpp) noexcept;
};

class FillAnalyticPart final : public FillPart {
public:
  FillAnalyticPart(PipeCompiler* pc, FetchPixelPtrPart* dst_part, CompOpPart* comp_op_part) noexcept;

  void compile(const PipeFunction& fn) noexcept override;

  BL_INLINE_NODEBUG VecWidth vec_width() const noexcept { return bl_min(pc->vec_width(), VecWidth::k256); }

  inline void count_zeros(const Gp& dst, const Gp& src) noexcept {
    UniOpRR op = BitOrder::kPrivate == BitOrder::kLSB ? UniOpRR::kCTZ : UniOpRR::kCLZ;
    pc->emit_2i(op, dst, src);
  }

  inline void shift_mask(const Gp& dst, const Gp& src1, const Gp& src2) noexcept {
    UniOpRRR op = BitOrder::kPrivate == BitOrder::kLSB ? UniOpRRR::kSll : UniOpRRR::kSrl;
    pc->emit_3i(op, dst, src1, src2);
  }

  void accumulate_coverages(const Vec& acc) noexcept;
  void normalize_coverages(const Vec& acc) noexcept;

  //! Calculates masks for 4 pixels - this works for both NonZero and EvenOdd fill rules.
  void calc_masks_from_cells(const Vec& msk, const Vec& acc, const Vec& fill_rule_mask, const Vec& global_alpha) noexcept;

  //! Expands the calculated mask in a way so it can be used by the compositor.
  void expand_mask(const VecArray& msk, PixelCount pixel_count) noexcept;

  //! Emits the following:
  //!
  //! ```
  //! dst_ptr -= x * dst_bpp;
  //! cell_ptr -= x * 4;
  //! ```
  void deadvance_dst_ptr_and_cell_ptr(const Gp& dst_ptr, const Gp& cell_ptr, const Gp& x, uint32_t dst_bpp) noexcept;
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FILLPART_P_H_INCLUDED
