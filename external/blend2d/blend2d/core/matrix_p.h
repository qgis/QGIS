// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_MATRIX_P_H_INCLUDED
#define BLEND2D_MATRIX_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/matrix.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/math_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! A generic function that can be used to transform an array of points that use `double` precision coordinates. This
//! function will be 99.99% of time used with \ref BLMatrix2D so the `ctx` would point to a `const BLMatrix2D*` instance.
typedef BLResult (BL_CDECL* BLMapPointDArrayFunc)(const void* ctx, BLPoint* dst, const BLPoint* src, size_t count) noexcept;

namespace bl {
namespace TransformInternal {

//! Array of functions for transforming points indexed by `BLMatrixType`. Each function is optimized for the respective
//! type. This is mostly used internally, but exported for users that can take advantage of Blend2D SIMD optimziations.
extern BLMapPointDArrayFunc map_pointd_array_funcs[BL_TRANSFORM_TYPE_MAX_VALUE + 1];

BL_HIDDEN extern const BLMatrix2D identity_transform;

static BL_INLINE BLBox map_box(const BLMatrix2D& transform, const BLBox& src) noexcept {
  double x0a = src.x0 * transform.m00;
  double y0a = src.y0 * transform.m10;
  double x1a = src.x1 * transform.m00;
  double y1a = src.y1 * transform.m10;

  double x0b = src.x0 * transform.m01;
  double y0b = src.y0 * transform.m11;
  double x1b = src.x1 * transform.m01;
  double y1b = src.y1 * transform.m11;

  return BLBox(bl_min(x0a, x1a) + bl_min(y0a, y1a) + transform.m20,
               bl_min(x0b, x1b) + bl_min(y0b, y1b) + transform.m21,
               bl_max(x0a, x1a) + bl_max(y0a, y1a) + transform.m20,
               bl_max(x0b, x1b) + bl_max(y0b, y1b) + transform.m21);
}

static BL_INLINE BLBox map_box_scaled_swapped(const BLMatrix2D& transform, const BLBox& src) noexcept {
  double x0 = src.x0 * transform.m00 + src.y0 * transform.m10 + transform.m20;
  double y0 = src.x0 * transform.m01 + src.y0 * transform.m11 + transform.m21;
  double x1 = src.x1 * transform.m00 + src.y1 * transform.m10 + transform.m20;
  double y1 = src.x1 * transform.m01 + src.y1 * transform.m11 + transform.m21;

  return BLBox(bl_min(x0, x1), bl_min(y0, y1), bl_max(x0, x1), bl_max(y0, y1));
}

static BL_INLINE void multiply(BLMatrix2D& dst, const BLMatrix2D& a, const BLMatrix2D& b) noexcept {
  dst.reset(a.m00 * b.m00 + a.m01 * b.m10,
            a.m00 * b.m01 + a.m01 * b.m11,
            a.m10 * b.m00 + a.m11 * b.m10,
            a.m10 * b.m01 + a.m11 * b.m11,
            a.m20 * b.m00 + a.m21 * b.m10 + b.m20,
            a.m20 * b.m01 + a.m21 * b.m11 + b.m21);
}

} // {TransformInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_MATRIX_P_H_INCLUDED
