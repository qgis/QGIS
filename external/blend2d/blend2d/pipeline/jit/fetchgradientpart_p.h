// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FETCHGRADIENTPART_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FETCHGRADIENTPART_P_H_INCLUDED

#include <blend2d/pipeline/jit/fetchpart_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelgather_p.h>
#include <blend2d/support/wrap_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

class GradientDitheringContext {
public:
  //! \name Members
  //! \{

  PipeCompiler* pc {};
  bool _is_rect_fill {};
  Gp _dm_position;
  Gp _dm_origin_x;
  Vec _dm_values;

  //! \}

  BL_INLINE explicit GradientDitheringContext(PipeCompiler* pc) noexcept
    : pc(pc) {}

  //! Returns whether this dithering context is used in a rectangular fill.
  BL_INLINE_NODEBUG bool is_rect_fill() const noexcept { return _is_rect_fill; }

  void init_y(const PipeFunction& fn, const Gp& x, const Gp& y) noexcept;
  void advance_y() noexcept;

  void start_at_x(const Gp& x) noexcept;
  void advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept;
  void advance_x_after_fetch(uint32_t n) noexcept;

  void dither_unpacked_pixels(Pixel& p, AdvanceMode advance_mode) noexcept;
};

//! Base class for all gradient fetch parts.
class FetchGradientPart : public FetchPart {
public:
  ExtendMode _extend_mode {};
  bool _dithering_enabled {};

  Gp _table_ptr;
  GradientDitheringContext _dithering_context;

  FetchGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  //! Returns the gradient extend mode.
  BL_INLINE_NODEBUG ExtendMode extend_mode() const noexcept { return _extend_mode; }

  //! Returns true if the gradient extend mode is Pad.
  BL_INLINE_NODEBUG bool is_pad() const noexcept { return _extend_mode == ExtendMode::kPad; }
  //! Returns true if the gradient extend mode is RoR.
  BL_INLINE_NODEBUG bool is_ror() const noexcept { return _extend_mode == ExtendMode::kRoR; }

  BL_INLINE_NODEBUG bool dithering_enabled() const noexcept { return _dithering_enabled; }

  BL_INLINE_NODEBUG void set_dithering_enabled(bool value) noexcept {
    _dithering_enabled = value;
    if (value)
      _part_flags |= PipePartFlags::kAdvanceXNeedsX;
  }

  BL_INLINE_NODEBUG int table_ptr_shift() const noexcept { return _dithering_enabled ? 3 : 2; }

  void fetch_single_pixel(Pixel& dst, PixelFlags flags, const Gp& idx) noexcept;

  void fetch_multiple_pixels(Pixel& dst, PixelCount n, PixelFlags flags, const Vec& idx, FetchUtils::IndexLayout index_layout, GatherMode mode, InterleaveCallback cb, void* cb_data) noexcept;

  inline void fetch_multiple_pixels(Pixel& dst, PixelCount n, PixelFlags flags, const Vec& idx, FetchUtils::IndexLayout index_layout, GatherMode mode) noexcept {
    fetch_multiple_pixels(dst, n, flags, idx, index_layout, mode, dummy_interleave_callback, nullptr);
  }

  template<class InterleaveFunc>
  inline void fetch_multiple_pixels(Pixel& dst, PixelCount n, PixelFlags flags, const Vec& idx, FetchUtils::IndexLayout index_layout, GatherMode mode, InterleaveFunc&& interleave_func) noexcept {
    fetch_multiple_pixels(dst, n, flags, idx, index_layout, mode, [](uint32_t step, void* data) noexcept {
      (*static_cast<const InterleaveFunc*>(data))(step);
    }, (void*)&interleave_func);
  }
};

//! Linear gradient fetch part.
class FetchLinearGradientPart : public FetchGradientPart {
public:
  struct LinearRegs {
    Gp dt_gp;
    Vec pt;
    Vec dt;
    Vec dt_n;
    Vec py;
    Vec dy;
    Vec maxi;
    Vec rori;
    Vec v_idx;
  };

  Wrap<LinearRegs> f;

  FetchLinearGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  BL_INLINE_NODEBUG VecWidth vec_width() const noexcept { return bl_min(pc->vec_width(), VecWidth::k256); }

  void prepare_part() noexcept override;

  void _init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept override;
  void _fini_part() noexcept override;

  void advance_y() noexcept override;
  void start_at_x(const Gp& x) noexcept override;
  void advance_x(const Gp& x, const Gp& diff) noexcept override;
  void advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept;
  void calc_advance_x(const Vec& dst, const Gp& diff) const noexcept;

  void enter_n() noexcept override;
  void leave_n() noexcept override;
  void prefetch_n() noexcept override;
  void postfetch_n() noexcept override;

  void fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept override;
};

//! Radial gradient fetch part.
class FetchRadialGradientPart : public FetchGradientPart {
public:
  // `d`   - determinant.
  // `dd`  - determinant delta.
  // `ddd` - determinant-delta delta.
  struct RadialRegs {
    Vec ty_tx;
    Vec yy_yx;

    Vec dd0_b0;
    Vec ddy_by;

    Vec vy;
    Vec inv2a_4a;
    Vec sqinv2a_sqfr;

    Vec d;
    Vec b;
    Vec dd;
    Vec vx;
    Vec vx_start;
    Vec value;

    Vec bd;
    Vec ddd;

    Vec vmaxi;
    Vec vrori;
  };

  Wrap<RadialRegs> f;

  FetchRadialGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  BL_INLINE_NODEBUG VecWidth vec_width() const noexcept { return bl_min(pc->vec_width(), VecWidth::k256); }

  void prepare_part() noexcept override;

  void _init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept override;
  void _fini_part() noexcept override;

  void advance_y() noexcept override;
  void start_at_x(const Gp& x) noexcept override;
  void advance_x(const Gp& x, const Gp& diff) noexcept override;
  void advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept;

  void prefetch_n() noexcept override;
  void postfetch_n() noexcept override;

  void fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept override;

  void init_vx(const Vec& vx, const Gp& x) noexcept;
  FetchUtils::IndexLayout apply_extend(const Vec& idx0, const Vec& idx1, const Vec& tmp) noexcept;
};

//! Conic gradient fetch part.
class FetchConicGradientPart : public FetchGradientPart {
public:
  static inline constexpr uint8_t kQ0 = 0;
  static inline constexpr uint8_t kQ1 = 1;
  static inline constexpr uint8_t kQ2 = 2;
  static inline constexpr uint8_t kQ3 = 3;

  static inline constexpr uint8_t kNDiv1 = 0;
  static inline constexpr uint8_t kNDiv2 = 1;
  static inline constexpr uint8_t kNDiv4 = 2;
  static inline constexpr uint8_t kAngleOffset = 3;

  struct ConicRegs {
    Vec ty_tx;
    Vec yy_yx;

    Vec tx;
    Vec xx;
    Vec vx;
    Vec vx_start;

    Vec ay;
    Vec by;

    Vec q_coeff;
    Vec n_coeff;

    Vec maxi;
    Vec rori;
  };

  Wrap<ConicRegs> f;

  FetchConicGradientPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  BL_INLINE_NODEBUG VecWidth vec_width(uint32_t n_pixels) const noexcept {
    return bl_min(pc->vec_width(), VecWidth(n_pixels >> 3));
  }

  void prepare_part() noexcept override;

  void _init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept override;
  void _fini_part() noexcept override;

  void advance_y() noexcept override;
  void start_at_x(const Gp& x) noexcept override;
  void advance_x(const Gp& x, const Gp& diff) noexcept override;
  void advance_x(const Gp& x, const Gp& diff, bool diff_within_bounds) noexcept;

  void prefetch_n() noexcept override;

  void fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept override;

  void init_vx(const Vec& vx, const Gp& x) noexcept;
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FETCHGRADIENTPART_P_H_INCLUDED
