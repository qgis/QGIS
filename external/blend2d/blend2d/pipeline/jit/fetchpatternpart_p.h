// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FETCHPATTERNPART_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FETCHPATTERNPART_P_H_INCLUDED

#include <blend2d/pipeline/jit/fetchpart_p.h>
#include <blend2d/support/wrap_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! Base class for all pattern fetch parts.
class FetchPatternPart : public FetchPart {
public:
  //! How many bits to shift the `x` index to get the address to the pixel. If this value is 0xFF it means that
  //! shifting is not possible or that the pixel was already pre-shifted.
  uint8_t _idx_shift = 0xFFu;

  //! Extend in X direction, used only by `FetchSimplePatternPart`.
  ExtendMode _extend_x {};

  FetchPatternPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  //! Tests whether the fetch-type is simple pattern {axis-aligned or axis-unaligned}.
  BL_INLINE_NODEBUG bool is_simple() const noexcept { return is_fetch_type(FetchType::kPatternSimpleFirst, FetchType::kPatternSimpleLast); }
  //! Tests whether the fetch-type is an affine pattern style.
  BL_INLINE_NODEBUG bool is_affine() const noexcept { return is_fetch_type(FetchType::kPatternAffineFirst, FetchType::kPatternAffineLast); }
};

//! Simple pattern fetch part.
//!
//! Simple pattern fetch doesn't do scaling or affine transformations, however, can perform fractional pixel
//! translation described as Fx and Fy values.
class FetchSimplePatternPart : public FetchPatternPart {
public:
  //! Aligned and fractional blits.
  struct SimpleRegs {
    //! Pointer to the previous scanline and/or pixel (fractional).
    Gp srcp0;
    //! Pointer to the current scanline and/or pixel (aligned).
    Gp srcp1;
    //! Pattern stride, used only by aligned blits.
    Gp stride;

    //! Vertical extend data.
    Mem v_extend_data;

    //! X position.
    Gp x;
    //! Y position (counter, decreases to zero).
    Gp y;

    //! Pattern width (32-bit).
    Gp w;
    //! Pattern height (32-bit).
    Gp h;

    //! X repeat/reflect.
    Gp rx;
    //! Y repeat/reflect.
    Gp ry;

    //! X padded to [0-W) range.
    Gp x_padded;
    //! X origin, assigned to `x` at the beginning of each scanline.
    Gp x_origin;
    //! X restart (used by scalar implementation, points to either -W or 0).
    Gp x_restart;

    //! Last loaded pixel (or combined pixel) of the first (srcp0) scanline.
    Vec pix_l;

    // Weights used in RGBA mode.
    Vec wa, wb, wc, wd;

    Vec wa_wb;
    Vec wc_wd;

    // Weights used in alpha-only mode.
    Vec wd_wb;
    Vec wa_wc;
    Vec wb_wd;

    //! X position vector  `[  x, x+1, x+2, x+3]`.
    Vec x_vec_4;
    //! X setup vector     `[  0,   1,   2,   3]`.
    Vec x_set_4;
    //! X increment vector `[  4,   4,   4,   4]`.
    Vec x_inc_4;
    //! X normalize vector.
    Vec x_nrm_4;
    //! X maximum vector   `[max, max, max, max]`.
    Vec x_max_4;
  };

  Wrap<SimpleRegs> f;

  FetchSimplePatternPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  //! Tests whether the fetch-type is axis-aligned blit (no extend modes, no overflows)
  BL_INLINE_NODEBUG bool is_aligned_blit() const noexcept { return is_fetch_type(FetchType::kPatternAlignedBlit); }
  //! Tests whether the fetch-type is axis-aligned pattern.
  BL_INLINE_NODEBUG bool is_pattern_aligned() const noexcept { return is_fetch_type(FetchType::kPatternAlignedFirst, FetchType::kPatternAlignedLast); }
  //! Tests whether the fetch-type is a "FracBi" pattern style.
  BL_INLINE_NODEBUG bool is_pattern_unaligned() const noexcept { return is_fetch_type(FetchType::kPatternUnalignedFirst, FetchType::kPatternUnalignedLast); }
  //! Tests whether the fetch-type is a "FracBiX" pattern style.
  BL_INLINE_NODEBUG bool is_pattern_fx() const noexcept { return is_fetch_type(FetchType::kPatternFxFirst, FetchType::kPatternFxLast); }
  //! Tests whether the fetch-type is a "FracBiY" pattern style.
  BL_INLINE_NODEBUG bool is_pattern_fy() const noexcept { return is_fetch_type(FetchType::kPatternFyFirst, FetchType::kPatternFyLast); }
  //! Tests whether the fetch-type is a "FracBiXY" pattern style.
  BL_INLINE_NODEBUG bool is_pattern_fx_fy() const noexcept { return is_fetch_type(FetchType::kPatternFxFyFirst, FetchType::kPatternFxFyLast); }

  //! Tests whether the fetch is pattern style that has fractional `x` or `x & y`.
  BL_INLINE_NODEBUG bool has_frac_x() const noexcept { return is_pattern_fx() || is_pattern_fx_fy(); }
  //! Tests whether the fetch is pattern style that has fractional `y` or `x & y`.
  BL_INLINE_NODEBUG bool has_frac_y() const noexcept { return is_pattern_fy() || is_pattern_fx_fy(); }

  //! Returns the extend-x mode.
  BL_INLINE_NODEBUG ExtendMode extend_x() const noexcept { return _extend_x; }

  void _init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept override;
  void _fini_part() noexcept override;

  void swap_stride_stop_data(VecArray& v) noexcept;

  void advance_y() noexcept override;
  void start_at_x(const Gp& x) noexcept override;
  void advance_x(const Gp& x, const Gp& diff) noexcept override;

  void advance_x_by_one() noexcept;
  void repeat_or_reflect_x() noexcept;
  void prefetch_acc_x() noexcept;

  void enter_n() noexcept override;
  void leave_n() noexcept override;
  void prefetch_n() noexcept override;
  void postfetch_n() noexcept override;

  void fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept override;
};

//! Affine pattern fetch part.
class FetchAffinePatternPart : public FetchPatternPart {
public:
  struct AffineRegs {
    //! Pattern pixels (pointer to the first scanline).
    Gp srctop;
    //! Pattern stride.
    Gp stride;

    //! Horizontal X/Y increments.
    Vec xx_xy;
    //! Vertical X/Y increments.
    Vec yx_yy;
    Vec tx_ty;
    Vec px_py;
    Vec ox_oy;
    //! Normalization after `px_py` gets out of bounds.
    Vec rx_ry;
    //! Like `px_py` but one pixel ahead [fetch4].
    Vec qx_qy;
    //! Advance twice (like `xx_xy`, but doubled) [fetch4].
    Vec xx2_xy2;

    //! Pad minimum coords.
    Vec minx_miny;
    //! Pad maximum coords.
    Vec maxx_maxy;
    //! Correction values (bilinear only).
    Vec corx_cory;
    //! Pattern width and height as doubles.
    Vec tw_th;

    //! Vector of pattern indexes.
    Vec v_idx;
    //! Vector containing multipliers for Y/X pairs.
    Vec vAddrMul;
  };

  enum ClampStep : uint32_t {
    kClampStepA_NN,
    kClampStepA_BI,

    kClampStepB_NN,
    kClampStepB_BI,

    kClampStepC_NN,
    kClampStepC_BI
  };

  Wrap<AffineRegs> f;

  FetchAffinePatternPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  BL_INLINE_NODEBUG bool is_affine_nn() const noexcept { return is_fetch_type(FetchType::kPatternAffineNNAny) || is_fetch_type(FetchType::kPatternAffineNNOpt); }
  BL_INLINE_NODEBUG bool is_affine_bi() const noexcept { return is_fetch_type(FetchType::kPatternAffineBIAny) || is_fetch_type(FetchType::kPatternAffineBIOpt); }
  BL_INLINE_NODEBUG bool is_optimized() const noexcept { return is_fetch_type(FetchType::kPatternAffineNNOpt) || is_fetch_type(FetchType::kPatternAffineBIOpt); }

  void _init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept override;
  void _fini_part() noexcept override;

  void advance_y() noexcept override;
  void start_at_x(const Gp& x) noexcept override;
  void advance_x(const Gp& x, const Gp& diff) noexcept override;

  void advance_px_py(Vec& px_py, const Gp& i) noexcept;
  void normalize_px_py(Vec& px_py) noexcept;
  void clamp_vec_idx_32(Vec& dst, const Vec& src, ClampStep step) noexcept;

  void enter_n() noexcept override;
  void leave_n() noexcept override;
  void prefetch_n() noexcept override;
  void postfetch_n() noexcept override;

  void fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept override;
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FETCHPATTERNPART_P_H_INCLUDED
