// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_REFERENCE_FETCHGENERIC_P_H_INCLUDED
#define BLEND2D_PIPELINE_REFERENCE_FETCHGENERIC_P_H_INCLUDED

#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/pipeline/reference/pixelgeneric_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/vecops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_reference
//! \{

namespace bl::Pipeline::Reference {

namespace {

// Fetch - Solid
// =============

template<typename PixelT>
struct FetchSolid {
  typedef PixelT PixelType;
  enum : uint32_t { kIsSolid = 1 };

  PixelType _src;

  BL_INLINE void _init_fetch(const void* fetch_data) noexcept {
    _src = PixelIO<PixelType, FormatExt::kPRGB32>::fetch(&static_cast<const FetchData::Solid*>(fetch_data)->prgb32);
  }

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    bl_unused(ctx_data, x_pos, y_pos, rect_width);

    _init_fetch(fetch_data);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    bl_unused(ctx_data, y_pos);

    _init_fetch(fetch_data);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }
  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    bl_unused(x_pos, x_diff);
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE void advance_y() noexcept {}

  BL_INLINE PixelType fetch() const noexcept {
    return _src;
  }
};

// Fetch - Non Solid
// =================

struct FetchNonSolid {
  enum : uint32_t { kIsSolid = 0 };
};

// Fetch - Pattern - Utilities
// ===========================

struct FetchPatternVertAAExtendCtxAny {
  const uint8_t* _pixel_ptr;

  intptr_t _stride0;
  intptr_t _stride1;

  intptr_t _yStop0;
  intptr_t _yStop1;

  uintptr_t _yRewindOffset;
  intptr_t _pixel_ptr_rewind_offset;

  intptr_t _y;

  BL_INLINE void init(const FetchData::Pattern* pattern, uint32_t y_pos) noexcept {
    _pixel_ptr = static_cast<const uint8_t*>(pattern->src.pixel_data);

    _stride0 = pattern->src.stride;
    _stride1 = _stride0;

    _yStop0 = intptr_t(pattern->src.size.h);
    _yStop1 = _yStop0;

    _yRewindOffset = pattern->simple.v_extend_data.y_rewind_offset;
    _pixel_ptr_rewind_offset = pattern->simple.v_extend_data.pixel_ptr_rewind_offset;

    _y = intptr_t(y_pos) + intptr_t(pattern->simple.ty);

    intptr_t ry = pattern->simple.ry;
    if (ry == 0) {
      // Vertical Extend - Pad
      // ---------------------

      intptr_t clampedY = bl_clamp<intptr_t>(_y, 0, _yStop0 - 1);
      _pixel_ptr += clampedY * _stride0;

      if (_y != clampedY) {
        // The current Y is padded at the moment so we have to setup stride0 and yStop0. If we are
        // padded before the first scanline, then we may hit yStop0 at some point and then go non-padded
        // for a while, otherwise, if we are padded past the last scanline we would stay there forever.
        _stride0 = 0;
        _yStop0 = 0;
      }
      else {
        // The current Y is within bounds, so setup stride1 and yStop1 as we will go to the end and
        // then after the end is matched we will stay at the end (it would pad to the last scanline).
        _stride1 = 0;
        _yStop1 = 0;
      }
    }
    else {
      // Vertical Extend - Repeat or Reflect
      // -----------------------------------

      _y = intptr_t(IntOps::pmod(uint32_t(_y), uint32_t(ry)));

      // If reflecting, we need few additional checks to reflect vertically. We are either reflecting now
      // (the first check) or we would be reflecting after the initial repeat (the second condition).
      if (_y >= _yStop0) {
        _pixel_ptr += (_yStop0 - 1) * _stride0;
        _stride0 = -_stride0;
        _y -= _yStop0;
      }
      else if (_yStop0 != ry) {
        _stride1 = -_stride0;
      }

      _pixel_ptr += _y * _stride0;
    }
  }

  BL_INLINE void advance1() noexcept {
    if (++_y == _yStop0) {
      BLInternal::swap(_yStop0, _yStop1);
      BLInternal::swap(_stride0, _stride1);

      _y -= _yRewindOffset;
      _pixel_ptr -= _pixel_ptr_rewind_offset;
    }
    else {
      _pixel_ptr += _stride0;
    }
  }

  BL_INLINE_NODEBUG const uint8_t* pixel_ptr() const noexcept { return _pixel_ptr; }
};

struct FetchPatternVertFyExtendCtxAny {
  const uint8_t* _pixelPtr0;
  FetchPatternVertAAExtendCtxAny _ctx;

  BL_INLINE void init(const FetchData::Pattern* pattern, uint32_t y_pos) noexcept {
    _ctx.init(pattern, y_pos);
    _pixelPtr0 = _ctx.pixel_ptr();
    _ctx.advance1();
  }

  BL_INLINE void advance1() noexcept {
    _pixelPtr0 = _ctx.pixel_ptr();
    _ctx.advance1();
  }

  BL_INLINE_NODEBUG const uint8_t* pixelPtr0() const noexcept { return _pixelPtr0; }
  BL_INLINE_NODEBUG const uint8_t* pixelPtr1() const noexcept { return _ctx.pixel_ptr(); }
};

template<uint32_t kBPP>
struct FetchPatternHorzExtendCtxPad {
  intptr_t _x;
  intptr_t _tx;
  intptr_t _mx;

  BL_INLINE void _init_pattern(const FetchData::Pattern* pattern) noexcept {
    _tx = intptr_t(uintptr_t(pattern->simple.tx) * kBPP);
    _mx = intptr_t(uintptr_t(pattern->src.size.w - 1) * kBPP);
  }

  BL_INLINE void rect_init(const FetchData::Pattern* pattern, uint32_t x_pos, uint32_t rect_width) noexcept {
    bl_unused(rect_width);
    _init_pattern(pattern);

    _tx += intptr_t(uintptr_t(x_pos) * kBPP);
  }

  BL_INLINE void rect_start(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
    _x = _tx;
  }

  BL_INLINE void span_init(const FetchData::Pattern* pattern) noexcept {
    _init_pattern(pattern);
  }

  BL_INLINE void span_start(uint32_t x_pos) noexcept {
    _x = intptr_t(uintptr_t(x_pos) * kBPP) + _tx;
  }

  BL_INLINE void span_advance(uint32_t x_pos, uint32_t x_diff) noexcept {
    bl_unused(x_pos);
    _x += intptr_t(uintptr_t(x_diff) * kBPP);
  }

  BL_INLINE void span_end(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE size_t index() const noexcept {
    return size_t(bl_clamp<intptr_t>(_x, 0, _mx));
  }

  BL_INLINE void advance1() noexcept {
    _x += kBPP;
  }
};

template<uint32_t kBPP>
struct FetchPatternHorzExtendCtxRepeat {
  uintptr_t _x;
  uintptr_t _tx;
  uintptr_t _w;

  BL_INLINE void _init_pattern(const FetchData::Pattern* pattern) noexcept {
    _w = uintptr_t(pattern->src.size.w) * kBPP;
    _tx = uintptr_t(pattern->simple.tx) * kBPP;
  }

  BL_INLINE void rect_init(const FetchData::Pattern* pattern, uint32_t x_pos, uint32_t rect_width) noexcept {
    bl_unused(rect_width);

    _init_pattern(pattern);
    _tx = IntOps::pmod(uintptr_t(x_pos) * kBPP + _tx, _w);
  }

  BL_INLINE void rect_start(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
    _x = _tx;
  }

  BL_INLINE void span_init(const FetchData::Pattern* pattern) noexcept {
    _init_pattern(pattern);
  }

  BL_INLINE void span_start(uint32_t x_pos) noexcept {
    _x = IntOps::pmod(uintptr_t(x_pos) * kBPP + _tx, _w);
  }

  BL_INLINE void span_advance(uint32_t x_pos, uint32_t x_diff) noexcept {
    bl_unused(x_pos);

    _x += x_diff * kBPP;
    if (_x >= _w)
      _x = IntOps::pmod(_x, _w);
  }

  BL_INLINE void span_end(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE size_t index() const noexcept {
    return _x;
  }

  BL_INLINE void advance1() noexcept {
    _x += kBPP;
    if (_x >= _w)
      _x = 0;
  }
};

template<uint32_t kBPP>
struct FetchPatternHorzExtendCtxRoR {
  intptr_t _x;
  intptr_t _tx;
  uintptr_t _rx;
  uintptr_t _w;

  BL_INLINE void _init_pattern(const FetchData::Pattern* pattern) noexcept {
    _w = uintptr_t(pattern->src.size.w);
    _rx = uintptr_t(pattern->simple.rx);
    _tx = intptr_t(pattern->simple.tx);
  }

  BL_INLINE void rect_init(const FetchData::Pattern* pattern, uint32_t x_pos, uint32_t rect_width) noexcept {
    bl_unused(rect_width);

    _init_pattern(pattern);
    _tx = intptr_t(IntOps::pmod(uintptr_t(x_pos) + uintptr_t(_tx), _rx));
    if (_tx >= intptr_t(_w))
      _tx -= intptr_t(_rx);
  }

  BL_INLINE void rect_start(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
    _x = _tx;
  }

  BL_INLINE void span_init(const FetchData::Pattern* pattern) noexcept {
    _init_pattern(pattern);
  }

  BL_INLINE void span_start(uint32_t x_pos) noexcept {
    _x = intptr_t(IntOps::pmod(uintptr_t(x_pos) + uintptr_t(_tx), uintptr_t(_rx)));
    if (_x >= intptr_t(_w))
      _x -= _rx;
  }

  BL_INLINE void span_advance(uint32_t x_pos, uint32_t x_diff) noexcept {
    bl_unused(x_pos);

    _x += x_diff;
    if (_x >= intptr_t(_w)) {
      _x = intptr_t(IntOps::pmod(uintptr_t(_x), _rx));
      if (_x >= intptr_t(_w)) {
        _x -= _rx;
      }
    }
  }

  BL_INLINE void span_end(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE size_t index() const noexcept {
    intptr_t mask = IntOps::sar(_x, bl::IntOps::bit_size_of<intptr_t>() - 1u);
    return size_t(_x ^ mask) * kBPP;
  }

  BL_INLINE void advance1() noexcept {
    if (++_x >= intptr_t(_w)) {
      _x -= _rx;
    }
  }
};

struct FetchPatternAffineCtx {
  Vec::u64x2 xx_xy;
  Vec::u64x2 yx_yy;
  Vec::u64x2 tx_ty;
  Vec::u64x2 px_py;
  Vec::i32x2 ox_oy;
  Vec::i32x2 rx_ry;
  Vec::i32x2 minx_miny;
  Vec::i32x2 maxx_maxy;
  Vec::i32x2 corx_cory;
  Vec::i32x2 tw_th;
  // Vec::f64x2 tw_th;

  BL_INLINE void _init_pattern(const FetchData::Pattern* pattern) noexcept {
    xx_xy = Vec::u64x2{pattern->affine.xx.u64, pattern->affine.xy.u64};
    yx_yy = Vec::u64x2{pattern->affine.yx.u64, pattern->affine.yy.u64};
    tx_ty = Vec::u64x2{pattern->affine.tx.u64, pattern->affine.ty.u64};
    ox_oy = Vec::i32x2{int32_t(pattern->affine.ox.u64 >> 32), int32_t(pattern->affine.oy.u64 >> 32)};
    rx_ry = Vec::i32x2{int32_t(pattern->affine.rx.u64 >> 32), int32_t(pattern->affine.ry.u64 >> 32)};
    minx_miny = Vec::i32x2{pattern->affine.min_x, pattern->affine.min_y};
    maxx_maxy = Vec::i32x2{pattern->affine.max_x, pattern->affine.max_y};
    corx_cory = Vec::i32x2{pattern->affine.cor_x, pattern->affine.cor_y};

    tw_th = Vec::i32x2{int(pattern->affine.tw), int(pattern->affine.th)};
  }

  BL_INLINE Vec::u64x2 normalize_px_py(const Vec::u64x2& v) const noexcept {
    uint32_t x = uint32_t(int32_t(v.x >> 32) % tw_th.x);
    uint32_t y = uint32_t(int32_t(v.y >> 32) % tw_th.y);

    if (int32_t(x) < 0) x += uint32_t(rx_ry.x);
    if (int32_t(y) < 0) y += uint32_t(rx_ry.y);

    if (int32_t(x) > ox_oy.x) x -= uint32_t(rx_ry.x);
    if (int32_t(y) > ox_oy.y) y -= uint32_t(rx_ry.y);

    return Vec::u64x2{(uint64_t(x) << 32) | (v.x & 0xFFFFFFFF), (uint64_t(y) << 32) | (v.y & 0xFFFFFFFF)};
  }

  BL_INLINE void rectInitY(ContextData* ctx_data, const FetchData::Pattern* pattern, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    bl_unused(ctx_data, rect_width);

    _init_pattern(pattern);
    tx_ty += yx_yy * uint64_t(y_pos) + xx_xy * uint64_t(x_pos);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);

    px_py = normalize_px_py(tx_ty);
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const FetchData::Pattern* pattern, uint32_t y_pos) noexcept {
    bl_unused(ctx_data);

    _init_pattern(pattern);
    tx_ty += yx_yy * uint64_t(y_pos);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    px_py = normalize_px_py(tx_ty + xx_xy * uint64_t(x_pos));
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    bl_unused(x_pos);

    px_py = normalize_px_py(px_py + xx_xy * uint64_t(x_diff));
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    tx_ty += yx_yy;
  }

  BL_INLINE uint32_t fracX() const noexcept { return uint32_t(px_py.x & 0xFFFFFFFFu) >> 24; }
  BL_INLINE uint32_t fracY() const noexcept { return uint32_t(px_py.y & 0xFFFFFFFFu) >> 24; }

  BL_INLINE Vec::u32x2 index(int32_t offX = 0, int32_t offY = 0) const noexcept {
    int32_t x = int32_t(px_py.x >> 32) + offX;
    int32_t y = int32_t(px_py.y >> 32) + offY;

    // Step A - Handle a possible underflow (PAD).
    x = bl_max(x, minx_miny.x);
    y = bl_max(y, minx_miny.y);

    // Step B - Handle a possible overflow (PAD | Bilinear overflow).
    if (x > maxx_maxy.x) x = corx_cory.x;
    if (y > maxx_maxy.y) y = corx_cory.y;

    // Step C - Handle a possible reflection (RoR).
    x = x ^ (x >> 31);
    y = y ^ (y >> 31);

    return Vec::u32x2{uint32_t(x), uint32_t(y)};
  }

  BL_INLINE void advance_x() noexcept {
    px_py += xx_xy;

    int32_t x = int32_t(px_py.x >> 32);
    int32_t y = int32_t(px_py.y >> 32);

    x -= (IntOps::bool_as_mask<int32_t>(x > ox_oy.x) & rx_ry.x);
    y -= (IntOps::bool_as_mask<int32_t>(y > ox_oy.y) & rx_ry.y);

    px_py = Vec::u64x2{(uint64_t(uint32_t(x)) << 32) | (px_py.x & 0xFFFFFFFFu),
                       (uint64_t(uint32_t(y)) << 32) | (px_py.y & 0xFFFFFFFFu)};
  }
};

// Fetch - Pattern - Aligned
// =========================

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternAlignedBlit : public FetchNonSolid {
  typedef DstPixelT PixelType;

  static inline constexpr uint32_t kSrcBPP = FormatMetadata<kFormat>::kBPP;

  const uint8_t* pixel_ptr;
  intptr_t stride;

  BL_INLINE void _init_fetch(const FetchData::Pattern* pattern) noexcept {
    pixel_ptr = static_cast<const uint8_t*>(pattern->src.pixel_data);
    stride = pattern->src.stride;
  }

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    bl_unused(ctx_data);

    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _init_fetch(pattern);

    uint32_t tx = uint32_t(pattern->simple.tx);
    uint32_t ty = uint32_t(pattern->simple.ty);

    pixel_ptr += intptr_t(y_pos - ty) * stride + intptr_t(x_pos - tx) * intptr_t(kSrcBPP);
    stride -= intptr_t(uintptr_t(rect_width) * kSrcBPP);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    bl_unused(ctx_data);

    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _init_fetch(pattern);

    uint32_t tx = uint32_t(pattern->simple.tx);
    uint32_t ty = uint32_t(pattern->simple.ty);

    pixel_ptr += intptr_t(uintptr_t(y_pos - ty)) * stride;
    pixel_ptr -= uintptr_t(tx) * kSrcBPP;
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    pixel_ptr += uintptr_t(x_pos) * kSrcBPP;
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    bl_unused(x_pos);

    pixel_ptr += uintptr_t(x_diff) * kSrcBPP;
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    pixel_ptr -= uintptr_t(x_pos) * kSrcBPP;
  }

  BL_INLINE void advance_y() noexcept {
    pixel_ptr += stride;
  }

  BL_INLINE PixelType fetch() noexcept {
    PixelType pixel = PixelIO<PixelType, kFormat>::fetch(pixel_ptr);
    pixel_ptr += kSrcBPP;
    return pixel;
  }
};

template<typename DstPixelT, FormatExt kFormat, typename CtxX>
struct FetchPatternAlignedAny : public FetchNonSolid {
  typedef DstPixelT PixelType;

  FetchPatternVertAAExtendCtxAny _ctxY;
  CtxX _ctxX;

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    bl_unused(ctx_data);

    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _ctxY.init(pattern, y_pos);
    _ctxX.rect_init(pattern, x_pos, rect_width);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    _ctxX.rect_start(x_pos);
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    bl_unused(ctx_data);

    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _ctxY.init(pattern, y_pos);
    _ctxX.span_init(pattern);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    _ctxX.span_start(x_pos);
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    _ctxX.span_advance(x_pos, x_diff);
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    _ctxX.span_end(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    _ctxY.advance1();
  }

  BL_INLINE PixelType fetch() noexcept {
    PixelType pixel = PixelIO<PixelType, kFormat>::fetch(_ctxY.pixel_ptr() + _ctxX.index());
    _ctxX.advance1();
    return pixel;
  }
};

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternAlignedPad : public FetchPatternAlignedAny<DstPixelT, kFormat, FetchPatternHorzExtendCtxPad<FormatMetadata<kFormat>::kBPP>> {};

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternAlignedRepeat : public FetchPatternAlignedAny<DstPixelT, kFormat, FetchPatternHorzExtendCtxRepeat<FormatMetadata<kFormat>::kBPP>> {};

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternAlignedRoR : public FetchPatternAlignedAny<DstPixelT, kFormat, FetchPatternHorzExtendCtxRoR<FormatMetadata<kFormat>::kBPP>> {};

// Fetch - Pattern - FxFy
// ======================

template<typename DstPixelT, FormatExt kFormat, typename CtxX>
struct FetchPatternFxFyAny : public FetchNonSolid {
  typedef DstPixelT PixelType;

  FetchPatternVertFyExtendCtxAny _ctxY;
  CtxX _ctxX;
  typename PixelType::Unpacked _pAcc;

  uint32_t _wa;
  uint32_t _wb;
  uint32_t _wc;
  uint32_t _wd;

  BL_INLINE void _initAccX() noexcept {
    size_t index = _ctxX.index();
    PixelType p0 = PixelIO<PixelType, kFormat>::fetch(_ctxY.pixelPtr0() + index);
    PixelType p1 = PixelIO<PixelType, kFormat>::fetch(_ctxY.pixelPtr1() + index);

    auto pA = p0.unpack() * Pixel::Repeat{_wa};
    auto pC = p1.unpack() * Pixel::Repeat{_wc};
    _pAcc = pA + pC;
  }

  BL_INLINE void _updateAccX(const typename PixelType::Unpacked& p) noexcept {
    _pAcc = p;
  }

  BL_INLINE void _init_fx_fy(const FetchData::Pattern* pattern) noexcept {
    _wa = pattern->simple.wa;
    _wb = pattern->simple.wb;
    _wc = pattern->simple.wc;
    _wd = pattern->simple.wd;
  }

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    bl_unused(ctx_data);

    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _ctxY.init(pattern, y_pos);
    _ctxX.rect_init(pattern, x_pos, rect_width);
    _init_fx_fy(pattern);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    _ctxX.rect_start(x_pos);
    _initAccX();
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    bl_unused(ctx_data);

    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _ctxY.init(pattern, y_pos);
    _ctxX.span_init(pattern);
    _init_fx_fy(pattern);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    _ctxX.span_start(x_pos);
    _initAccX();
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    _ctxX.span_advance(x_pos, x_diff);
    _initAccX();
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    _ctxX.span_end(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    _ctxY.advance1();
  }

  BL_INLINE PixelType fetch() noexcept {
    _ctxX.advance1();

    size_t index = _ctxX.index();
    auto pixel0 = PixelIO<PixelType, kFormat>::fetch(_ctxY.pixelPtr0() + index).unpack();
    auto pixel1 = PixelIO<PixelType, kFormat>::fetch(_ctxY.pixelPtr1() + index).unpack();
    auto unpacked = pixel0 * Pixel::Repeat{_wb} + pixel1 * Pixel::Repeat{_wd} + _pAcc;

    _updateAccX(pixel0 * Pixel::Repeat{_wa} + pixel1 * Pixel::Repeat{_wc});
    return unpacked.div256().pack();
  }
};

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternFxFyPad : public FetchPatternFxFyAny<DstPixelT, kFormat, FetchPatternHorzExtendCtxPad<FormatMetadata<kFormat>::kBPP>> {};

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternFxFyRoR : public FetchPatternFxFyAny<DstPixelT, kFormat, FetchPatternHorzExtendCtxRoR<FormatMetadata<kFormat>::kBPP>> {};

// Fetch - Pattern - Affine
// ========================

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternAffineNNBase : public FetchNonSolid {
  const uint8_t* _pixel_data;
  intptr_t _stride;
  FetchPatternAffineCtx _ctx;

  BL_INLINE void _init_affine(const FetchData::Pattern* pattern) noexcept {
    _pixel_data = pattern->src.pixel_data;
    _stride = pattern->src.stride;
  }

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _init_affine(pattern);
    _ctx.rectInitY(ctx_data, pattern, x_pos, y_pos, rect_width);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    _ctx.rectStartX(x_pos);
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    const FetchData::Pattern* pattern = static_cast<const FetchData::Pattern*>(fetch_data);
    _init_affine(pattern);
    _ctx.spanInitY(ctx_data, pattern, y_pos);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    _ctx.spanStartX(x_pos);
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    _ctx.spanAdvanceX(x_pos, x_diff);
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    _ctx.spanEndX(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    _ctx.advance_y();
  }
};

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternAffineNNAny : public FetchPatternAffineNNBase<DstPixelT, kFormat> {
  typedef DstPixelT PixelType;

  static inline constexpr uint32_t kSrcBPP = FormatMetadata<kFormat>::kBPP;

  using FetchPatternAffineNNBase<DstPixelT, kFormat>::_pixel_data;
  using FetchPatternAffineNNBase<DstPixelT, kFormat>::_stride;
  using FetchPatternAffineNNBase<DstPixelT, kFormat>::_ctx;

  BL_INLINE PixelType fetch() noexcept {
    Vec::u32x2 index = _ctx.index();
    _ctx.advance_x();

    const uint8_t* p = _pixel_data + intptr_t(index.y) * _stride + size_t(index.x) * kSrcBPP;
    return PixelIO<PixelType, kFormat>::fetch(p);
  }
};

template<typename DstPixelT, FormatExt kFormat>
struct FetchPatternAffineBIAny : public FetchPatternAffineNNBase<DstPixelT, kFormat> {
  typedef DstPixelT PixelType;

  static inline constexpr uint32_t kSrcBPP = FormatMetadata<kFormat>::kBPP;

  using FetchPatternAffineNNBase<DstPixelT, kFormat>::_pixel_data;
  using FetchPatternAffineNNBase<DstPixelT, kFormat>::_stride;
  using FetchPatternAffineNNBase<DstPixelT, kFormat>::_ctx;

  BL_INLINE PixelType fetch() noexcept {
    Vec::u32x2 index0 = _ctx.index();
    Vec::u32x2 index1 = _ctx.index(1, 1);

    uint32_t wx = _ctx.fracX();
    uint32_t wy = _ctx.fracY();

    _ctx.advance_x();

    uint32_t ix = 256 - wx;
    uint32_t iy = 256 - wy;

    const uint8_t* line0 = _pixel_data + intptr_t(index0.y) * _stride;
    const uint8_t* line1 = _pixel_data + intptr_t(index1.y) * _stride;

    auto p0 = PixelIO<PixelType, kFormat>::fetch(line0 + size_t(index0.x) * kSrcBPP).unpack() * Pixel::Repeat{iy} +
              PixelIO<PixelType, kFormat>::fetch(line1 + size_t(index0.x) * kSrcBPP).unpack() * Pixel::Repeat{wy} ;

    auto p1 = PixelIO<PixelType, kFormat>::fetch(line0 + size_t(index1.x) * kSrcBPP).unpack() * Pixel::Repeat{iy} +
              PixelIO<PixelType, kFormat>::fetch(line1 + size_t(index1.x) * kSrcBPP).unpack() * Pixel::Repeat{wy} ;

    p0 = p0.div256() * Pixel::Repeat{ix};
    p1 = p1.div256() * Pixel::Repeat{wx};

    return (p0 + p1).div256().pack();
  }
};

// Fetch - Pattern - Dispatch
// ==========================

template<FetchType kFetchType, typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch {};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAlignedBlit, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAlignedBlit<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAlignedPad, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAlignedPad<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAlignedRepeat, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAlignedRepeat<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAlignedRoR, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAlignedRoR<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternFxPad, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternFxFyPad<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternFyPad, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternFxFyPad<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternFxFyPad, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternFxFyPad<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternFxRoR, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternFxFyRoR<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternFyRoR, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternFxFyRoR<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternFxFyRoR, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternFxFyRoR<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAffineNNAny, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAffineNNAny<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAffineNNOpt, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAffineNNAny<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAffineBIAny, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAffineBIAny<DstPixelT, kSrcFormat>;
};

template<typename DstPixelT, FormatExt kSrcFormat>
struct FetchPatternDispatch<FetchType::kPatternAffineBIOpt, DstPixelT, kSrcFormat> {
  using Fetch = FetchPatternAffineBIAny<DstPixelT, kSrcFormat>;
};

// Fetch - Gradient - Base
// =======================

template<typename PixelType, BLGradientQuality kQuality>
struct FetchGradientBase;

template<typename PixelType>
struct FetchGradientBase<PixelType, BL_GRADIENT_QUALITY_NEAREST> : public FetchNonSolid {
  const void* _table;

  BL_INLINE void _init_gradient_base(ContextData* ctx_data, const FetchData::Gradient* gradient, uint32_t y_pos) noexcept {
    bl_unused(ctx_data, y_pos);

    _table = gradient->lut.data;
  }

  BL_INLINE void _initGradientX(uint32_t x_pos) noexcept { bl_unused(x_pos); }
  BL_INLINE void _advanceGradientY() noexcept {}

  BL_INLINE PixelType fetch_pixel(uint32_t idx) const noexcept {
    return PixelIO<PixelType, FormatExt::kPRGB32>::fetch(static_cast<const uint32_t*>(_table) + idx);
  }
};

template<typename PixelType>
struct FetchGradientBase<PixelType, BL_GRADIENT_QUALITY_DITHER> : public FetchNonSolid {
  const void* _table;
  uint32_t _dmOffsetY;
  uint32_t _dmOffsetX;

  static inline constexpr uint32_t kAdvanceYMask = (16u * 16u * 2u) - 1u;

  BL_INLINE void _init_gradient_base(ContextData* ctx_data, const FetchData::Gradient* gradient, uint32_t y_pos) noexcept {
    bl_unused(ctx_data);

    _table = gradient->lut.data;
    _dmOffsetY = ((uint32_t(ctx_data->pixel_origin.y) + y_pos) & 15u) * (16u * 2u) + (uint32_t(ctx_data->pixel_origin.x) & 15u);
  }

  BL_INLINE void _initGradientX(uint32_t x_pos) noexcept {
    _dmOffsetX = (x_pos & 15u);
  }

  BL_INLINE void _advanceGradientY() noexcept {
    _dmOffsetY = (_dmOffsetY + 16u * 2u) & kAdvanceYMask;
  }

  BL_INLINE PixelType fetch_pixel(uint32_t idx) noexcept {
    BLRgba64 v{static_cast<const uint64_t*>(_table)[idx]};
    uint32_t dd = common_table.bayer_matrix_16x16[_dmOffsetY + _dmOffsetX];

    uint32_t a = v.a() >> 8;
    uint32_t r = bl_min<uint32_t>((v.r() + dd) >> 8, a);
    uint32_t g = bl_min<uint32_t>((v.g() + dd) >> 8, a);
    uint32_t b = bl_min<uint32_t>((v.b() + dd) >> 8, a);

    _dmOffsetX = (_dmOffsetX + 1u) & 15u;
    return PixelIO<PixelType, FormatExt::kPRGB32>::make(r, g, b, a);
  }
};

// Fetch - Gradient - Linear
// =========================

template<typename PixelType, BLGradientQuality kQuality, bool kIsPad>
struct FetchLinearGradient : public FetchGradientBase<PixelType, kQuality> {
  using Base = FetchGradientBase<PixelType, kQuality>;
  using Base::fetch_pixel;

  uint64_t _pt;
  uint64_t _dt;
  uint64_t _py;
  uint64_t _dy;
  uint32_t _maxi;
  uint32_t _rori;

  BL_INLINE void _init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    const FetchData::Gradient* gradient = static_cast<const FetchData::Gradient*>(fetch_data);
    const FetchData::Gradient::Linear& linear = gradient->linear;

    Base::_init_gradient_base(ctx_data, gradient, y_pos);
    _pt = 0;
    _py = linear.pt[0].u64;
    _dt = linear.dt.u64;
    _dy = linear.dy.u64;
    _maxi = linear.maxi;
    _rori = linear.rori;
  }

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    bl_unused(rect_width);

    _init_fetch(ctx_data, fetch_data, y_pos);
    _py += y_pos * _dy + x_pos * _dt;
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    Base::_initGradientX(x_pos);
    _pt = _py;
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    _init_fetch(ctx_data, fetch_data, y_pos);
    _py += y_pos * _dy;
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    Base::_initGradientX(x_pos);
    _pt = _py + x_pos * _dt;
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    Base::_initGradientX(x_pos);
    _pt += x_diff * _dt;
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    _py += _dy;
    Base::_advanceGradientY();
  }

  BL_INLINE PixelType fetch() noexcept {
    uint32_t idx = uint32_t(_pt >> 32);

    if (kIsPad)
      idx = uint32_t(bl_clamp<int32_t>(int32_t(idx), 0, int32_t(_maxi)));
    else
      idx = bl_min<uint32_t>(idx & _maxi, (idx & _maxi) ^ _rori);

    _pt += _dt;
    return fetch_pixel(idx);
  }
};

// Fetch - Gradient - Radial
// =========================

template<typename PixelType, BLGradientQuality kQuality, bool kIsPad>
struct FetchRadialGradient : public FetchGradientBase<PixelType, kQuality> {
  using Base = FetchGradientBase<PixelType, kQuality>;
  using Base::fetch_pixel;

  Vec::f64x2 _tp;
  Vec::f64x2 _yy_yx;

  double _b0;
  double _by;
  double _dd0;
  double _ddy;

  double _inv2a;
  double _amul4;
  double _sq_inv2a;
  double _sq_fr;

  double _y;

  float _x;
  float _b;
  float _d;
  float _dd;

  float _bd;
  float _ddd;

  uint32_t _maxi;
  uint32_t _rori;

  BL_INLINE void _init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    const FetchData::Gradient* gradient = static_cast<const FetchData::Gradient*>(fetch_data);
    const FetchData::Gradient::Radial& radial = gradient->radial;

    Base::_init_gradient_base(ctx_data, gradient, y_pos);

    _tp = Vec::f64x2{radial.tx, radial.ty};
    _yy_yx = Vec::f64x2{radial.yx, radial.yy};

    _b0 = radial.b0;
    _by = radial.by;
    _dd0 = radial.dd0;
    _ddy = radial.ddy;

    _inv2a = radial.inv2a;
    _amul4 = radial.amul4;
    _sq_fr = radial.sq_fr;
    _sq_inv2a = radial.sq_inv2a;

    _bd = radial.f32_bd;
    _ddd = radial.f32_ddd;

    _maxi = radial.maxi;
    _rori = radial.rori;

    _y = double(int32_t(y_pos));
  }

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t width) noexcept {
    bl_unused(x_pos, width);
    _init_fetch(ctx_data, fetch_data, y_pos);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    spanStartX(x_pos);
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    _init_fetch(ctx_data, fetch_data, y_pos);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    Base::_initGradientX(x_pos);

    Vec::f64x2 pt(Math::madd(_yy_yx.x, _y, _tp.x),
                  Math::madd(_yy_yx.y, _y, _tp.y));
    double b = Math::madd(_y, _by, _b0);
    double sq_dist = Math::square(pt.x) + Math::square(pt.y);

    _x = float(int32_t(x_pos));
    _b = float(b * _inv2a);
    _d = float(Math::madd(_amul4, (sq_dist - _sq_fr), Math::square(b)) * _sq_inv2a);
    _dd = float(Math::madd(_y, _ddy, _dd0) * _sq_inv2a);
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    Base::_initGradientX(x_pos);
    _x += float(int32_t(x_diff));
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    _y += 1.0;
    Base::_advanceGradientY();
  }

  BL_INLINE PixelType fetch() noexcept {
    float sq_x = Math::square(_x);

    float a = Math::sqrt(bl_abs(Math::madd(sq_x, _ddd, Math::madd(_x, _dd, _d))));
    float v = Math::madd(_x, _bd, _b) + a;

    uint32_t idx = uint32_t(Math::trunc_to_int(v));

    _x += 1.0f;

    if (kIsPad)
      idx = uint32_t(bl_clamp<int32_t>(int32_t(idx), 0, int32_t(_maxi)));
    else
      idx = bl_min<uint32_t>(idx & _maxi, (idx & _maxi) ^ _rori);
    return fetch_pixel(idx);
  }
};

template<typename PixelType, BLGradientQuality kQuality>
struct FetchRadialGradientPad : public FetchRadialGradient<PixelType, kQuality, true> {};

template<typename PixelType, BLGradientQuality kQuality>
struct FetchRadialGradientRoR : public FetchRadialGradient<PixelType, kQuality, false> {};

// Fetch - Gradient - Conic
// ========================

template<typename PixelType, BLGradientQuality kQuality>
struct FetchConicGradient : public FetchGradientBase<PixelType, kQuality> {
  using Base = FetchGradientBase<PixelType, kQuality>;
  using Base::fetch_pixel;

  Vec::f64x2 _tp;
  Vec::f64x2 _yy_yx;

  float _q_coeff[4];
  float _n_div_1;
  float _n_div_2;
  float _n_div_4;
  float _angle_offset;
  float _xx;

  int32_t _maxi;
  int32_t _rori;

  float _x;
  float _tx;
  float _ay;
  float _by;

  BL_INLINE void _init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    const FetchData::Gradient* gradient = static_cast<const FetchData::Gradient*>(fetch_data);
    const FetchData::Gradient::Conic& conic = gradient->conic;

    Base::_init_gradient_base(ctx_data, gradient, y_pos);

    _yy_yx = Vec::f64x2{conic.yx, conic.yy};
    _tp = Vec::f64x2{conic.tx, conic.ty} + _yy_yx * double(int(y_pos));

    _q_coeff[0] = conic.q_coeff[0];
    _q_coeff[1] = conic.q_coeff[1];
    _q_coeff[2] = conic.q_coeff[2];
    _q_coeff[3] = conic.q_coeff[3];

    _n_div_1 = conic.n_div_1_2_4[0];
    _n_div_2 = conic.n_div_1_2_4[1];
    _n_div_4 = conic.n_div_1_2_4[2];
    _angle_offset = conic.offset;
    _xx = conic.xx;

    _maxi = int32_t(conic.maxi);
    _rori = int32_t(conic.rori);
  }

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t width) noexcept {
    bl_unused(x_pos, width);
    _init_fetch(ctx_data, fetch_data, y_pos);
  }

  BL_INLINE void begin_scanline() noexcept {
    _tx = float(_tp.x);
    _ay = float(_tp.y);
    _by = Vec::and_(Vec::msb_mask(_ay), _n_div_1);
    _ay = Vec::abs(_ay);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    Base::_initGradientX(x_pos);
    _x = float(int(x_pos));
    begin_scanline();
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    _init_fetch(ctx_data, fetch_data, y_pos);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    Base::_initGradientX(x_pos);
    _x = float(int(x_pos));
    begin_scanline();
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    Base::_initGradientX(x_pos);
    _x += float(int32_t(x_diff));
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    bl_unused(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    _tp += _yy_yx;
    Base::_advanceGradientY();
  }

  BL_INLINE PixelType fetch() noexcept {
    float x = Math::madd(_x, _xx, _tx);
    float ax = bl_abs(x);

    float xy_min = bl_min(ax, _ay);
    float xy_max = bl_max(ax, _ay);

    float s = Vec::and_(bl_bit_cast<float>(IntOps::bool_as_mask<uint32_t>(ax == xy_min)), _n_div_4);
    float p = xy_min / xy_max;
    float p_sq = Math::square(p);

    float v = Math::madd(p_sq, _q_coeff[3], _q_coeff[2]);
    v = Math::madd(v, p_sq, _q_coeff[1]);
    v = Math::madd(v, p_sq, _q_coeff[0]);
    v = bl_abs(Math::madd(v, p, -s));
    v = bl_abs(v - Vec::and_(Vec::msb_mask(x), _n_div_2));
    v = bl_abs(v - _by) + _angle_offset;

    uint32_t idx = uint32_t(bl_min<int32_t>(Math::nearby_to_int(v), _maxi)) & uint32_t(_rori);
    _x += 1.0f;
    return fetch_pixel(idx);
  }
};

// Fetch - Gradient - Dispatch
// ===========================

template<FetchType kFetchType, typename DstPixelT>
struct FetchGradientDispatch {};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientLinearNNPad, DstPixelT> {
  using Fetch = FetchLinearGradient<DstPixelT, BL_GRADIENT_QUALITY_NEAREST, true>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientLinearNNRoR, DstPixelT> {
  using Fetch = FetchLinearGradient<DstPixelT, BL_GRADIENT_QUALITY_NEAREST, false>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientLinearDitherPad, DstPixelT> {
  using Fetch = FetchLinearGradient<DstPixelT, BL_GRADIENT_QUALITY_DITHER, true>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientLinearDitherRoR, DstPixelT> {
  using Fetch = FetchLinearGradient<DstPixelT, BL_GRADIENT_QUALITY_DITHER, false>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientRadialNNPad, DstPixelT> {
  using Fetch = FetchRadialGradient<DstPixelT, BL_GRADIENT_QUALITY_NEAREST, true>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientRadialNNRoR, DstPixelT> {
  using Fetch = FetchRadialGradient<DstPixelT, BL_GRADIENT_QUALITY_NEAREST, false>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientRadialDitherPad, DstPixelT> {
  using Fetch = FetchRadialGradient<DstPixelT, BL_GRADIENT_QUALITY_DITHER, true>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientRadialDitherRoR, DstPixelT> {
  using Fetch = FetchRadialGradient<DstPixelT, BL_GRADIENT_QUALITY_DITHER, false>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientConicNN, DstPixelT> {
  using Fetch = FetchConicGradient<DstPixelT, BL_GRADIENT_QUALITY_NEAREST>;
};

template<typename DstPixelT>
struct FetchGradientDispatch<FetchType::kGradientConicDither, DstPixelT> {
  using Fetch = FetchConicGradient<DstPixelT, BL_GRADIENT_QUALITY_DITHER>;
};

} // {anonymous}
} // {bl::Pipeline::Reference}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_REFERENCE_FETCHGENERIC_P_H_INCLUDED
