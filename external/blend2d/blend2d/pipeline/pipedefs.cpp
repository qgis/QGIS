// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/context.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/traits_p.h>

namespace bl::Pipeline {
namespace FetchUtils {

// Pipeline - FetchData - Extend Modes
// ===================================

static BL_INLINE uint32_t extend_xFromExtendMode(uint32_t extend_mode) noexcept {
  BL_ASSERT(extend_mode <= BL_EXTEND_MODE_COMPLEX_MAX_VALUE);

  constexpr uint32_t kTable = (BL_EXTEND_MODE_PAD     <<  0) | // [pad-x     pad-y    ]
                              (BL_EXTEND_MODE_REPEAT  <<  2) | // [repeat-x  repeat-y ]
                              (BL_EXTEND_MODE_REFLECT <<  4) | // [reflect-x reflect-y]
                              (BL_EXTEND_MODE_PAD     <<  6) | // [pad-x     repeat-y ]
                              (BL_EXTEND_MODE_PAD     <<  8) | // [pad-x     reflect-y]
                              (BL_EXTEND_MODE_REPEAT  << 10) | // [repeat-x  pad-y    ]
                              (BL_EXTEND_MODE_REPEAT  << 12) | // [repeat-x  reflect-y]
                              (BL_EXTEND_MODE_REFLECT << 14) | // [reflect-x pad-y    ]
                              (BL_EXTEND_MODE_REFLECT << 16) ; // [reflect-x repeat-y ]
  return (kTable >> (extend_mode * 2u)) & 0x3u;
}

static BL_INLINE uint32_t extend_yFromExtendMode(uint32_t extend_mode) noexcept {
  BL_ASSERT(extend_mode <= BL_EXTEND_MODE_COMPLEX_MAX_VALUE);

  constexpr uint32_t kTable = (BL_EXTEND_MODE_PAD     <<  0) | // [pad-x     pad-y    ]
                              (BL_EXTEND_MODE_REPEAT  <<  2) | // [repeat-x  repeat-y ]
                              (BL_EXTEND_MODE_REFLECT <<  4) | // [reflect-x reflect-y]
                              (BL_EXTEND_MODE_REPEAT  <<  6) | // [pad-x     repeat-y ]
                              (BL_EXTEND_MODE_REFLECT <<  8) | // [pad-x     reflect-y]
                              (BL_EXTEND_MODE_PAD     << 10) | // [repeat-x  pad-y    ]
                              (BL_EXTEND_MODE_REFLECT << 12) | // [repeat-x  reflect-y]
                              (BL_EXTEND_MODE_PAD     << 14) | // [reflect-x pad-y    ]
                              (BL_EXTEND_MODE_REPEAT  << 16) ; // [reflect-x repeat-y ]
  return (kTable >> (extend_mode * 2u)) & 0x3u;
}

// Pipeline - FetchData - Init Pattern
// ===================================

static BL_INLINE Signature init_pattern_tx_ty(FetchData::Pattern& fetch_data, FetchType fetch_base, uint32_t extend_mode, int tx, int ty, bool is_fractional) noexcept {
  uint32_t extend_x = extend_xFromExtendMode(extend_mode);
  uint32_t extend_y = extend_yFromExtendMode(extend_mode);
  uint32_t ix_index = 17;

  int rx = 0;
  int ry = 0;

  // If the pattern width/height is 1 all extend modes produce the same output. However, it's safer to just set it to
  // PAD as FetchPatternPart requires `width` to be equal or greater than 2 if the extend mode is REPEAT or REFLECT.
  if (fetch_data.src.size.w <= 1) extend_x = BL_EXTEND_MODE_PAD;
  if (fetch_data.src.size.h <= 1) extend_y = BL_EXTEND_MODE_PAD;

  if (extend_x >= BL_EXTEND_MODE_REPEAT) {
    bool is_reflect = extend_x == BL_EXTEND_MODE_REFLECT;

    rx = int(fetch_data.src.size.w) << uint32_t(is_reflect);
    if (unsigned(tx) >= unsigned(rx))
      tx %= rx;
    if (tx < 0)
      tx += rx;

    // In extreme cases, when `rx` is very small, fetch4()/fetch8() functions may overflow `x` if they increment more
    // than they can fix by subtracting `rw` in case of overflow (and overflow happens as it's used to start over). To
    // fix this and simplify the compiled code we simply precalculate these constants so they are always safe.
    ix_index = bl_min<uint32_t>(uint32_t(rx), 17);

    // Don't specialize `Repeat vs Reflect` when we are not pixel aligned.
    if (is_fractional)
      extend_x = 1; // TODO: Naming...
  }

  // Setup v_extend_data initially for PADding, then refine in REPEAT|REFLECT case.
  FetchData::Pattern::VertExtendData& ext_data = fetch_data.simple.v_extend_data;
  ext_data.stride[0] = fetch_data.src.stride;
  ext_data.stride[1] = 0;
  ext_data.y_stop[0] = uint32_t(fetch_data.src.size.h);
  ext_data.y_stop[1] = 0;
  ext_data.y_rewind_offset = 0;
  ext_data.pixel_ptr_rewind_offset = (extend_y != BL_EXTEND_MODE_REPEAT ? intptr_t(0) : intptr_t(fetch_data.src.size.h - 1)) * fetch_data.src.stride;

  if (extend_y >= BL_EXTEND_MODE_REPEAT) {
    ry = int(fetch_data.src.size.h) << uint32_t(extend_y == BL_EXTEND_MODE_REFLECT);
    if (unsigned(ty) >= unsigned(ry))
      ty %= ry;
    if (ty < 0)
      ty += ry;

    ext_data.stride[1] = (extend_y == BL_EXTEND_MODE_REPEAT) ? fetch_data.src.stride : -fetch_data.src.stride;
    ext_data.y_stop[1] = uint32_t(fetch_data.src.size.h);
    ext_data.y_rewind_offset = uint32_t(fetch_data.src.size.h);
  }

  fetch_data.simple.tx = tx;
  fetch_data.simple.ty = ty;
  fetch_data.simple.rx = rx;
  fetch_data.simple.ry = ry;
  fetch_data.simple.ix = modulo_table[ix_index];

  return Signature::from_fetch_type(FetchType(uint32_t(fetch_base) + extend_x));
}

Signature init_pattern_ax_ay(FetchData::Pattern& fetch_data, BLExtendMode extend_mode, int x, int y) noexcept {
  return init_pattern_tx_ty(fetch_data, FetchType::kPatternAlignedPad, extend_mode, -x, -y, false);
}

Signature init_pattern_fx_fy(FetchData::Pattern& fetch_data, BLExtendMode extend_mode, BLPatternQuality quality, uint32_t bytes_per_pixel, int64_t tx64, int64_t ty64) noexcept {
  bl_unused(bytes_per_pixel);

  FetchType fetch_base = FetchType::kPatternAlignedPad;
  uint32_t wx = uint32_t(tx64 & 0xFF);
  uint32_t wy = uint32_t(ty64 & 0xFF);

  int tx = -int(tx64 >> 8);
  int ty = -int(ty64 >> 8);

  // If one or both `wx` or `wy` are non-zero it means that the translation is fractional. In that case we must
  // calculate weights of [x0 y0], [x1 y0], [x0 y1], and [x1 y1] pixels.
  bool is_fractional = (wx | wy) != 0;
  if (is_fractional) {
    if (quality == BL_PATTERN_QUALITY_NEAREST) {
      tx -= (wx >= 128);
      ty -= (wy >= 128);
      is_fractional = false;
    }
    else {
      fetch_data.simple.wa = ((      wy) * (      wx)      ) >> 8; // [x0 y0]
      fetch_data.simple.wb = ((      wy) * (256 - wx) + 255) >> 8; // [x1 y0]
      fetch_data.simple.wc = ((256 - wy) * (      wx)      ) >> 8; // [x0 y1]
      fetch_data.simple.wd = ((256 - wy) * (256 - wx) + 255) >> 8; // [x1 y1]

      // The FxFy fetcher must work even when one or both `wx` or `wy` are zero, so we always decrement `tx` and `ty`.
      // In addition, Fx or Fy fetcher can be replaced by FxFy if there is no Fx or Fy implementation (typically this
      // could happen if we are running portable pipeline without any optimizations).
      tx--;
      ty--;

      if (wy == 0)
        fetch_base = FetchType::kPatternFxPad;
      else if (wx == 0)
        fetch_base = FetchType::kPatternFyPad;
      else
        fetch_base = FetchType::kPatternFxFyPad;
    }
  }

  return init_pattern_tx_ty(fetch_data, fetch_base, extend_mode, tx, ty, is_fractional);
}

Signature init_pattern_affine(FetchData::Pattern& fetch_data, BLExtendMode extend_mode, BLPatternQuality quality, uint32_t bytes_per_pixel, const BLMatrix2D& transform) noexcept {
  // Inverted transformation matrix.
  BLMatrix2D inv;
  if (BL_UNLIKELY(BLMatrix2D::invert(inv, transform) != BL_SUCCESS)) {
    return Signature::from_pending_flag(1);
  }

  // Pattern bounds.
  int tw = int(fetch_data.src.size.w);
  int th = int(fetch_data.src.size.h);

  if (BL_UNLIKELY(tw == 0)) {
    return Signature::from_pending_flag(1);
  }

  double xx = inv.m00;
  double xy = inv.m01;
  double yx = inv.m10;
  double yy = inv.m11;

  if (bool_and(Math::is_near_one(xx),
               Math::is_near_zero(xy),
               Math::is_near_zero(yx),
               Math::is_near_one(yy))) {
    int64_t tx64 = Math::floor_to_int64(-inv.m20 * 256.0);
    int64_t ty64 = Math::floor_to_int64(-inv.m21 * 256.0);
    return init_pattern_fx_fy(fetch_data, extend_mode, quality, bytes_per_pixel, tx64, ty64);
  }

  FetchType fetch_type =
    quality == BL_PATTERN_QUALITY_NEAREST
      ? FetchType::kPatternAffineNNAny
      : FetchType::kPatternAffineBIAny;

#if 1 // BL_TARGET_ARCH_X86
  uint32_t opt = bl_max(tw, th) < 32767 &&
                 fetch_data.src.stride >= 0 &&
                 fetch_data.src.stride <= intptr_t(Traits::max_value<int16_t>());

  // TODO: [JIT] OPTIMIZATION: Not implemented for bilinear yet.
  if (quality == BL_PATTERN_QUALITY_BILINEAR) {
    opt = 0;
  }
#else
  constexpr uint32_t opt = 0;
#endif // BL_TARGET_ARCH_X86

  fetch_type = FetchType(uint32_t(fetch_type) + opt);

  // Pattern X/Y extends.
  uint32_t extend_x = extend_xFromExtendMode(extend_mode);
  uint32_t extend_y = extend_yFromExtendMode(extend_mode);

  // Translation.
  double tx = inv.m20;
  double ty = inv.m21;

  tx += 0.5 * (xx + yx);
  ty += 0.5 * (xy + yy);

  // 32x32 fixed point scale as double, equals to `pow(2, 32)`.
  double fp_scale = 4294967296.0;

  // Overflow check of X/Y. When this check passes we decrement rx/ry from the overflown values.
  int ox = Traits::max_value<int32_t>();
  int oy = Traits::max_value<int32_t>();

  // Normalization of X/Y. These values are added to the current `px` and `py` when they overflow the repeat|reflect
  // bounds.
  int rx = 0;
  int ry = 0;

  fetch_data.affine.min_x = 0;
  fetch_data.affine.min_y = 0;

  fetch_data.affine.max_x = int32_t(tw - 1);
  fetch_data.affine.max_y = int32_t(th - 1);

  fetch_data.affine.cor_x = int32_t(tw - 1);
  fetch_data.affine.cor_y = int32_t(th - 1);

  if (extend_x != BL_EXTEND_MODE_PAD) {
    fetch_data.affine.min_x = Traits::min_value<int32_t>();
    if (extend_x == BL_EXTEND_MODE_REPEAT)
      fetch_data.affine.cor_x = 0;

    ox = tw;
    if (extend_x == BL_EXTEND_MODE_REFLECT)
      tw *= 2;

    if (xx < 0.0) {
      xx = -xx;
      yx = -yx;
      tx = double(tw) - tx;

      if (extend_x == BL_EXTEND_MODE_REPEAT) {
        ox = 0;
        fetch_data.affine.cor_x = fetch_data.affine.max_x;
        fetch_data.affine.max_x = -1;
      }
    }
    ox--;
  }

  if (extend_y != BL_EXTEND_MODE_PAD) {
    fetch_data.affine.min_y = Traits::min_value<int32_t>();
    if (extend_y == BL_EXTEND_MODE_REPEAT)
      fetch_data.affine.cor_y = 0;

    oy = th;
    if (extend_y == BL_EXTEND_MODE_REFLECT)
      th *= 2;

    if (xy < 0.0) {
      xy = -xy;
      yy = -yy;
      ty = double(th) - ty;

      if (extend_y == BL_EXTEND_MODE_REPEAT) {
        oy = 0;
        fetch_data.affine.cor_y = fetch_data.affine.max_y;
        fetch_data.affine.max_y = -1;
      }
    }
    oy--;
  }

  // Keep the center of the pixel at [0.5, 0.5] if the filter is NEAREST so it can properly round to the nearest
  // pixel during the fetch phase. However, if the filter is not NEAREST the `tx` and `ty` have to be translated
  // by -0.5 so the position starts at the beginning of the pixel.
  if (quality != BL_PATTERN_QUALITY_NEAREST) {
    tx -= 0.5;
    ty -= 0.5;
  }

  // Pattern boundaries converted to `double`.
  double tw_d = double(tw);
  double th_d = double(th);

  // Normalize the matrix in a way that it won't overflow the pattern more than once per a single iteration. Happens
  // when scaling part is very small. Only useful for repeated / reflected cases.
  if (extend_x == BL_EXTEND_MODE_PAD) {
    tw_d = 2147483647.0;
  }
  else {
    tx = fmod(tx, tw_d);
    rx = tw;
    if (xx >= tw_d) xx = fmod(xx, tw_d);
  }

  if (extend_y == BL_EXTEND_MODE_PAD) {
    th_d = 2147483647.0;
  }
  else {
    ty = fmod(ty, th_d);
    ry = th;
    if (xy >= th_d) xy = fmod(xy, th_d);
  }

  xx *= fp_scale;
  xy *= fp_scale;
  yx *= fp_scale;
  yy *= fp_scale;
  tx *= fp_scale;
  ty *= fp_scale;

  // To prevent undefined behavior and thus passing invalid integer coordinates to the fetcher, we have to verify that
  // double to int64 conversion is actually valid. To do that we simply combine min/max in a way to always propagate
  // NaNs in case there is any.
  double all_min = bl_min(bl_min(yy, yx), bl_min(xy, xx));
  double all_max = bl_max(bl_max(xx, xy), bl_max(yx, yy));

  all_min = bl_min(all_min, bl_min(ty, tx));
  all_max = bl_max(bl_max(tx, ty), all_max);

  if (all_min >= double(INT64_MIN + 1) && all_max <= double(INT64_MAX)) {
    fetch_data.affine.xx.i64 = Math::floor_to_int64(xx);
    fetch_data.affine.xy.i64 = Math::floor_to_int64(xy);
    fetch_data.affine.yx.i64 = Math::floor_to_int64(yx);
    fetch_data.affine.yy.i64 = Math::floor_to_int64(yy);
    fetch_data.affine.tx.i64 = Math::floor_to_int64(tx);
    fetch_data.affine.ty.i64 = Math::floor_to_int64(ty);
  }
  else {
    fetch_data.affine.xx.i64 = 0;
    fetch_data.affine.xy.i64 = 0;
    fetch_data.affine.yx.i64 = 0;
    fetch_data.affine.yy.i64 = 0;
    fetch_data.affine.tx.i64 = 0;
    fetch_data.affine.ty.i64 = 0;
  }

  fetch_data.affine.rx.i64 = IntOps::shl(int64_t(rx), 32);
  fetch_data.affine.ry.i64 = IntOps::shl(int64_t(ry), 32);

  fetch_data.affine.ox.i32_hi = ox;
  fetch_data.affine.ox.i32_lo = Traits::max_value<int32_t>();
  fetch_data.affine.oy.i32_hi = oy;
  fetch_data.affine.oy.i32_lo = Traits::max_value<int32_t>();

  fetch_data.affine.tw = tw_d;
  fetch_data.affine.th = th_d;

  fetch_data.affine.xx2.u64 = fetch_data.affine.xx.u64 << 1u;
  fetch_data.affine.xy2.u64 = fetch_data.affine.xy.u64 << 1u;

  if (extend_x >= BL_EXTEND_MODE_REPEAT && fetch_data.affine.xx2.u32_hi >= uint32_t(tw)) fetch_data.affine.xx2.u32_hi %= uint32_t(tw);
  if (extend_y >= BL_EXTEND_MODE_REPEAT && fetch_data.affine.xy2.u32_hi >= uint32_t(th)) fetch_data.affine.xy2.u32_hi %= uint32_t(th);

  if (opt) {
    fetch_data.affine.addr_mul16[0] = int16_t(bytes_per_pixel);
    fetch_data.affine.addr_mul16[1] = int16_t(fetch_data.src.stride);
  }
  else {
    fetch_data.affine.addr_mul32[0] = int32_t(bytes_per_pixel);
    fetch_data.affine.addr_mul32[1] = int32_t(fetch_data.src.stride);
  }

  return Signature::from_fetch_type(fetch_type);
}

// FetchData - Init Gradient
// =========================

static BL_INLINE Signature init_linear_gradient(FetchData::Gradient& fetch_data, const BLLinearGradientValues& values, BLExtendMode extend_mode, BLGradientQuality quality, const BLMatrix2D& transform) noexcept {
  BL_ASSERT(extend_mode <= BL_EXTEND_MODE_SIMPLE_MAX_VALUE);
  BL_ASSERT(fetch_data.lut.size > 0u);

  // Inverted transformation matrix.
  BLMatrix2D inv;
  if (BLMatrix2D::invert(inv, transform) != BL_SUCCESS)
    return Signature::from_pending_flag(1);

  BLPoint p0(values.x0, values.y0);
  BLPoint p1(values.x1, values.y1);

  uint32_t lut_size = fetch_data.lut.size;
  uint32_t maxi = extend_mode == BL_EXTEND_MODE_REFLECT ? lut_size * 2u - 1u : lut_size - 1u;
  uint32_t rori = extend_mode == BL_EXTEND_MODE_REFLECT ? maxi : 0u;

  // Distance between [x0, y0] and [x1, y1], before transform.
  double ax = p1.x - p0.x;
  double ay = p1.y - p0.y;
  double dist = ax * ax + ay * ay;

  // Invert origin and move it to the center of the pixel.
  BLPoint o = BLPoint(0.5, 0.5) - transform.map_point(p0);

  double dt = ax * inv.m00 + ay * inv.m01;
  double dy = ax * inv.m10 + ay * inv.m11;

  double scale = double(int64_t(uint64_t(lut_size) << 32)) / dist;
  double offset = o.x * dt + o.y * dy;

  dt *= scale;
  dy *= scale;
  offset *= scale;

  fetch_data.linear.dy.i64 = Math::floor_to_int64(dy);
  fetch_data.linear.dt.i64 = Math::floor_to_int64(dt);
  fetch_data.linear.pt[0].i64 = Math::floor_to_int64(offset);
  fetch_data.linear.pt[1].u64 = fetch_data.linear.pt[0].u64 + fetch_data.linear.dt.u64;

  fetch_data.linear.maxi = maxi;
  fetch_data.linear.rori = rori;

  FetchType fetch_type_base =
    quality < BL_GRADIENT_QUALITY_DITHER
      ? FetchType::kGradientLinearNNPad
      : FetchType::kGradientLinearDitherPad;

  return Signature::from_fetch_type(FetchType(uint32_t(fetch_type_base) + uint32_t(extend_mode != BL_EXTEND_MODE_PAD)));
}

// The radial gradient uses the following equation:
//
//    b = x * fx + y * fy
//    d = x^2 * (r^2 - fy^2) + y^2 * (r^2 - fx^2) + x*y * (2*fx*fy)
//
//    pos = ((b + sqrt(d))) * scale)
//
// Simplified to:
//
//    C1 = r^2 - fy^2
//    C2 = r^2 - fx^2
//    C3 = 2 * fx * fy
//
//    b = x*fx + y*fy
//    d = x^2 * C1 + y^2 * C2 + x*y * C3
//
//    pos = ((b + sqrt(d))) * scale)
//
// Radial gradient function can be defined as follows:
//
//    D = C1*(x^2) + C2*(y^2) + C3*(x*y)
//
// Which could be rewritten as:
//
//    D = D1 + D2 + D3
//
//    Where: D1 = C1*(x^2)
//           D2 = C2*(y^2)
//           D3 = C3*(x*y)
//
// The variables `x` and `y` increase linearly, thus we can use multiple differentiation to get delta (d) and
// delta-of-delta (dd).
//
// Deltas for `C*(x^2)` at `t`:
//
//   C*x*x: 1st delta `d`  at step `t`: C*(t^2) + 2*C*x
//   C*x*x: 2nd delta `dd` at step `t`: 2*C *t^2
//
//   ( Hint, use Mathematica DifferenceDelta[x*x*C, {x, 1, t}] )
//
// Deltas for `C*(x*y)` at `t`:
//
//   C*x*y: 1st delta `d`  at step `tx/ty`: C*x*ty + C*y*tx + C*tx*ty
//   C*x*y: 2nd delta `dd` at step `tx/ty`: 2*C * tx*ty
static BL_INLINE Signature init_radial_gradient(FetchData::Gradient& fetch_data, const BLRadialGradientValues& values, BLExtendMode extend_mode, BLGradientQuality quality, const BLMatrix2D& transform) noexcept {
  BL_ASSERT(extend_mode <= BL_EXTEND_MODE_SIMPLE_MAX_VALUE);
  BL_ASSERT(fetch_data.lut.size > 0u);

  BLMatrix2D inv;
  if (BLMatrix2D::invert(inv, transform) != BL_SUCCESS)
    return Signature::from_pending_flag(1);

  uint32_t lut_size = fetch_data.lut.size;
  uint32_t maxi = extend_mode == BL_EXTEND_MODE_REFLECT ? lut_size * 2u - 1u : lut_size - 1u;
  uint32_t rori = extend_mode == BL_EXTEND_MODE_REFLECT ? maxi : 0u;

  fetch_data.radial.maxi = maxi;
  fetch_data.radial.rori = rori;

  BLPoint cp = BLPoint(values.x0, values.y0);
  BLPoint fp = BLPoint(values.x1, values.y1);

  double cr = values.r0;
  double fr = values.r1;

  BLPoint dp = cp - fp;
  double dr = cr - fr;

  double sq_dx_plus_dy = Math::square(dp.x) + Math::square(dp.y);
  double dx_plus_dy = Math::sqrt(sq_dx_plus_dy);

  // Numerical stability falls apart when the focal point is very close to
  // the border. So shift it slightly away from it to improve stability.
  double dist_from_border = bl_abs(dx_plus_dy - dr);
  constexpr double dist_limit = 0.5;

  if (dist_from_border < dist_limit) {
    BLPoint dp0 = (dp * (dr - dist_limit)) / dx_plus_dy;
    BLPoint dp1 = (dp * (dr + dist_limit)) / dx_plus_dy;

    double dp0_dist = bl_abs(Math::square(dp0.x) + Math::square(dp0.y) - sq_dx_plus_dy);
    double dp1_dist = bl_abs(Math::square(dp1.x) + Math::square(dp1.y) - sq_dx_plus_dy);

    dp = (dp0_dist < dp1_dist) ? dp0 : dp1;
    fp = cp - dp;
    sq_dx_plus_dy = Math::square(dp.x) + Math::square(dp.y);
  }

  double a = Math::square(dr) - sq_dx_plus_dy;
  double sq_fr = Math::square(fr);
  double scale = double(lut_size);

  double xx = inv.m00;
  double xy = inv.m01;
  double yx = inv.m10;
  double yy = inv.m11;
  BLPoint tp = BLPoint(inv.m20 + (xx + xy) * 0.5, inv.m21 + (yx + yy) * 0.5) - fp;

  fetch_data.radial.tx = tp.x;
  fetch_data.radial.ty = tp.y;
  fetch_data.radial.yx = yx;
  fetch_data.radial.yy = yy;

  double a_mul_4 = a * 4.0;
  double inv2a = (scale * 0.5) / a; // scale * (1 / 2a) => (scale * 0.5) / a
  double sq_inv2a = Math::square(inv2a);

  fetch_data.radial.amul4 = a_mul_4;
  fetch_data.radial.inv2a = inv2a;
  fetch_data.radial.sq_inv2a = sq_inv2a;
  fetch_data.radial.sq_fr = sq_fr;

  double sq_xx_plus_sq_yx = Math::square(xx) + Math::square(xy);
  double b0 = 2.0 * (dr * fr + tp.x * dp.x + tp.y * dp.y);
  double bx = 2.0 * (dp.x * xx + dp.y * xy);
  double by = 2.0 * (dp.x * yx + dp.y * yy);

  fetch_data.radial.b0 = -b0;
  fetch_data.radial.by = -by;

  double bx_mul_2 = bx * 2.0;
  double sq_bx = Math::square(bx);

  double dd0 = sq_bx + bx_mul_2 * b0 + a_mul_4 * (sq_xx_plus_sq_yx + 2.0 * (tp.x * xx + tp.y * xy));
  double ddy = bx_mul_2 * by + a_mul_4 * (2.0 * (xx * yx + yy * xy));

  double ddd_half = (sq_bx + a_mul_4 * sq_xx_plus_sq_yx);
  double ddd_half_inv = ddd_half * sq_inv2a;

  fetch_data.radial.dd0 = dd0 - ddd_half;
  fetch_data.radial.ddy = ddy;
  fetch_data.radial.f32_bd = float(-bx * inv2a);
  fetch_data.radial.f32_ddd = float(ddd_half_inv);

  FetchType fetch_type_base =
    quality < BL_GRADIENT_QUALITY_DITHER
      ? FetchType::kGradientRadialNNPad
      : FetchType::kGradientRadialDitherPad;

  return Signature::from_fetch_type(FetchType(uint32_t(fetch_type_base) + uint32_t(extend_mode != BL_EXTEND_MODE_PAD)));
}

// Coefficients used by conic gradient fetcher for 256 entry table. If the table size is different or repeat
// is not 1 the values have to be scaled by `init_conic_gradient()`. Fetcher always uses scaled values.
//
// Polynomial to approximate `atan(x) * N / 2PI`:
//   `x * (Q0 + x^2 * (Q1 + x^2 * (Q2 + x^2 * Q3)))`
//
// The following numbers were obtained by `lolremez` (minmax tool for approximations) for N==256:
//
// Atan is an odd function, so we take advantage of it (see lolremez docs):
//   1. E=|atan(x) * N / 2PI - P(x)                  | <- subs. `P(x)` by `x*Q(x^2))`
//   2. E=|atan(x) * N / 2PI - x*Q(x^2)              | <- subs. `x^2` by `y`
//   3. E=|atan(sqrt(y)) * N / 2PI - sqrt(y) * Q(y)  | <- eliminate `y` from Q side - div by `y`
//   4. E=|atan(sqrt(y)) * N / (2PI * sqrt(y)) - Q(y)|
//
// LolRemez C++ code:
//
// ```
//   real f(real const& x) {
//     real y = sqrt(x);
//     return atan(y) * real(N) / (real(2) * real::R_PI * y);
//   }
//
//   real g(real const& x) {
//     return re(sqrt(x));
//   }
//
//   int main(int argc, char **argv) {
//     RemezSolver<3, real> solver;
//     solver.Run("1e-1000", 1, f, g, 40);
//     return 0;
//   }
// ```
static constexpr double conic_gradient_q_coeff_256[4] = {
  4.071421038552e+1, -1.311160794048e+1, 6.017670215625, -1.623253505085
};

static BL_INLINE Signature init_conic_gradient(FetchData::Gradient& fetch_data, const BLConicGradientValues& values, BLExtendMode extend_mode, BLGradientQuality quality, const BLMatrix2D& transform) noexcept {
  bl_unused(extend_mode);

  BLPoint c(values.x0, values.y0);
  double angle = values.angle;
  double repeat = values.repeat;

  uint32_t lut_size = fetch_data.lut.size;

  // Invert the origin and move it to the center of the pixel.
  c = BLPoint(0.5, 0.5) - transform.map_point(c);

  BLPoint v = transform.map_vector(BLPoint(1.0, 0.0));
  double matrix_angle = atan2(v.y, v.x);

  BLMatrix2D updated_transform(transform);
  updated_transform.rotate(-matrix_angle, c);

  angle += matrix_angle;
  double off = Math::frac(angle / -Math::kPI_MUL_2);

  if (off != 0.0)
    off = -1.0 + off;

  BLMatrix2D inv;
  if (BLMatrix2D::invert(inv, updated_transform) != BL_SUCCESS)
    return Signature::from_pending_flag(1);

  fetch_data.conic.tx = c.x * inv.m00 + c.y * inv.m10;
  fetch_data.conic.ty = c.x * inv.m01 + c.y * inv.m11;
  fetch_data.conic.yx = inv.m10;
  fetch_data.conic.yy = inv.m11;

  double lutSizeD = double(int(lut_size));
  double repeated_lut_size = lutSizeD * repeat;
  double qScale = repeated_lut_size / 256.0;

  fetch_data.conic.q_coeff[0] = float(conic_gradient_q_coeff_256[0] * qScale);
  fetch_data.conic.q_coeff[1] = float(conic_gradient_q_coeff_256[1] * qScale);
  fetch_data.conic.q_coeff[2] = float(conic_gradient_q_coeff_256[2] * qScale);
  fetch_data.conic.q_coeff[3] = float(conic_gradient_q_coeff_256[3] * qScale);

  fetch_data.conic.n_div_1_2_4[0] = float(repeated_lut_size);
  fetch_data.conic.n_div_1_2_4[1] = float(repeated_lut_size * 0.5);
  fetch_data.conic.n_div_1_2_4[2] = float(repeated_lut_size * 0.25);
  fetch_data.conic.offset = float(off * repeated_lut_size - 0.5);
  fetch_data.conic.xx = float(inv.m00);

  fetch_data.conic.maxi = INT32_MAX;
  fetch_data.conic.rori = lut_size - 1u;

  FetchType fetch_type =
    quality < BL_GRADIENT_QUALITY_DITHER
      ? FetchType::kGradientConicNN
      : FetchType::kGradientConicDither;

  return Signature::from_fetch_type(fetch_type);
}

Signature init_gradient(
  FetchData::Gradient& fetch_data,
  BLGradientType gradient_type,
  BLExtendMode extend_mode,
  BLGradientQuality quality,
  const void* values, const void* lut_data, uint32_t lut_size, const BLMatrix2D& transform) noexcept {

  // Initialize LUT.
  fetch_data.lut.data = lut_data;
  fetch_data.lut.size = lut_size;

  // Initialize gradient by type.
  switch (gradient_type) {
    case BL_GRADIENT_TYPE_LINEAR:
      return init_linear_gradient(fetch_data, *static_cast<const BLLinearGradientValues*>(values), extend_mode, quality, transform);

    case BL_GRADIENT_TYPE_RADIAL:
      return init_radial_gradient(fetch_data, *static_cast<const BLRadialGradientValues*>(values), extend_mode, quality, transform);

    case BL_GRADIENT_TYPE_CONIC:
      return init_conic_gradient(fetch_data, *static_cast<const BLConicGradientValues*>(values), extend_mode, quality, transform);

    default:
      // Should not happen, but be defensive.
      return Signature::from_pending_flag(1);
  }
}

} // {FetchUtils}
} // {bl::Pipeline}
