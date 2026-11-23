// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_STATEDATA_P_H_INCLUDED
#define BLEND2D_RASTER_STATEDATA_P_H_INCLUDED

#include <blend2d/core/geometry.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/raster/styledata_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

//! Raster rendering context state - based on public `BLContextState`.
class alignas(16) RasterContextState : public BLContextState {
public:
  union {
    uint32_t transform_types_packed;
    struct {
      //! Type of final transformation matrix that scales to fixed point.
      uint8_t final_transform_fixed_type;
      //! Type of meta transformation matrix that scales to fixed point.
      uint8_t meta_transform_fixed_type;
      //! Type of final transformation matrix.
      uint8_t final_transform_type;
      //! Type of meta transformation matrix.
      uint8_t meta_transform_type;
      //! Type of the identity transformation matrix (used by Style API).
      uint8_t identity_transform_type;
    };

    struct {
      uint8_t fixed_transform_types[2];
      //! Transform types indexed by \ref BLContextStyleTransformMode (used by Style API).
      uint8_t transform_types[uint32_t(BL_CONTEXT_STYLE_TRANSFORM_MODE_MAX_VALUE) + 1u];
    };
  };

  //! Global alpha as integer (0..255 or 0..65535).
  uint32_t global_alpha_i;
  //! Current fill or stroke alpha converted to integer indexed by style slot, see \ref BLContextStyleSlot.
  uint32_t styleAlphaI[2];

  //! Curve flattening tolerance scaled by `fp_scale_d`.
  double toleranceFixedD;

  //! Fill and stroke styles, and one additional style that is never used in practice, but is used during error checking.
  StyleData style[2];

  //! Integral offset to add to input coordinates in case integral transform is ok.
  BLPointI translation_i;

  //! Meta matrix scaled by `fp_scale`.
  alignas(16) BLMatrix2D meta_transform_fixed;
  //! Result of `(meta_transform * user_transform) * fp_scale`.
  alignas(16) BLMatrix2D final_transform_fixed;

  //! Meta clip-box (int).
  alignas(16) BLBoxI meta_clip_box_i;
  //! Final clip box (int).
  alignas(16) BLBoxI final_clip_box_i;
  //! Final clip-box (double).
  alignas(16) BLBox final_clip_box_d;
};

//! Structure that holds a previously saved state, see \ref BLContext::save() and \ref BLContext::restore().
//!
//! \note The struct is designed to have no gaps required by alignment so the order of members doesn't have to make
//! much sense.
struct alignas(16) SavedState {
  //! Link to the previous state.
  SavedState* prev_state;
  //! State ID (only valid if a cookie was used).
  uint64_t state_id;

  //! Context hints.
  BLContextHints hints;
  //! Composition operator.
  uint8_t comp_op;
  //! Fill rule.
  uint8_t fill_rule;
  //! Current type of a style object of fill and stroke operations indexed by \ref BLContextStyleSlot.
  uint8_t style_type[2];

  //! Clip mode.
  uint8_t clip_mode;
  //! Padding at the moment.
  uint8_t reserved[7];

  //! Copy of previous `BLRasterContextImpl::_context_flags`.
  ContextFlags prev_context_flags;

  union {
    uint32_t transform_types_packed;
    struct {
      //! Type of final matrix that scales to fixed point.
      uint8_t final_transform_fixed_type;
      //! Type of meta matrix that scales to fixed point.
      uint8_t meta_transform_fixed_type;
      //! Type of final matrix.
      uint8_t final_transform_type;
      //! Type of meta matrix.
      uint8_t meta_transform_type;
    };
  };
  //! Global alpha as integer (0..255 or 0..65535).
  uint32_t global_alpha_i;
  //! Alpha value (0..255 or 0..65535).
  uint32_t styleAlphaI[2];

  //! Global alpha value [0, 1].
  double global_alpha;
  //! Fill and stroke alpha values [0, 1].
  double style_alpha[2];
  //! Fill and stroke styles.
  StyleData style[2];

  //! Approximation options.
  BLApproximationOptions approximation_options;
  //! Stroke options.
  BLStrokeOptionsCore stroke_options;

  //! Final clip_box (double).
  BLBox final_clip_box_d;

  //! Integral translation, if possible.
  BLPointI translation_i;
  //! Meta or final transformation matrix (depending on flags).
  BLMatrix2D alt_transform;
  //! User transformation matrix.
  BLMatrix2D user_transform;
};

struct Matrix2x2 {
  double m[4];
};

//! A shared fill state is used by asynchronous rendering context and can be shared between multiple rendering jobs.
struct SharedFillState {
  BLBox final_clip_box_fixed_d;
  Matrix2x2 final_transform_fixed;
  double toleranceFixedD;
};

//! A shared stroke state is used by asynchronous rendering context and can be shared between multiple rendering jobs.
struct SharedBaseStrokeState {
  BLStrokeOptions stroke_options;
  BLApproximationOptions approximation_options;

  BL_INLINE explicit SharedBaseStrokeState(const BLStrokeOptions& stroke_options, const BLApproximationOptions& approximation_options) noexcept
    : stroke_options(stroke_options),
      approximation_options(approximation_options) {}
};

//! A shared stroke state that is used by strokes with specific transform_order.
struct SharedExtendedStrokeState : public SharedBaseStrokeState {
  Matrix2x2 user_transform;
  Matrix2x2 meta_transform_fixed;

  BL_INLINE explicit SharedExtendedStrokeState(const BLStrokeOptions& stroke_options, const BLApproximationOptions& approximation_options) noexcept
    : SharedBaseStrokeState(stroke_options, approximation_options) {}
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_STATEDATA_P_H_INCLUDED
