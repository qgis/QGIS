// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERFETCHDATA_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERFETCHDATA_P_H_INCLUDED

#include <blend2d/raster/rasterdefs_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

//! A small struct that precedes `Pipeline::FetchData` in `RenderFetchData` struct.
//!
//! When a pipeline signature is built, there is a lot of unknowns and in general two code paths to build SOLID
//! and NON-SOLID pipelines. However, it's just a detail and the only thing that NON-SOLID render call needs is
//! to make sure that FetchData has been properly setup. This is only problem when rendering with a default fill
//! or stroke style, because in order to make style assignment fast, some calculations are postponed up to the place
//! we would hit once we know that the style is really going to be used - in general, some properties are materialized
//! lazily.
//!
//! To make this materialization simpler, we have a little prefix before a real `Pipeline::FetchData` that contains
//! a signature (other members are here just to use the space as FetchData should be aligned to 16 bytes, so we need
//! a 16 byte prefix as well). When the signature has only a PendingFlag set, it means that the FetchData hasn't been
//! setup yet and it has to be setup before the pipeline signature can be obtained.
//!
//! \note In some cases, this header can be left uninitialized in a single-threaded rendering in case that the
//! FetchData is constructed in place and allocated statically. In general, if it doesn't survive the render call
//! (which happens in single-threaded rendering a lot) then these fields are not really needed.
struct RenderFetchDataHeader {
  //! \name Members
  //! \{

  //! Signature if the fetch data is initialized, otherwise a Signature with PendingFlag bit set (last MSB).
  Pipeline::Signature signature;
  //! Batch id.
  uint32_t batch_id;
  //! Non-atomic reference count (never manipulated concurrently by multiple threads, usually the user thread only).
  uint32_t ref_count;

  union {
    uint32_t packed;

    struct {
      //! Pixel format of the source (possibly resolved to FRGB/ZERO, etc).
      uint8_t format;
      //! Extra bits, which can be used by the rendering engine to store some essential information required to materialize
      //! the FetchData.
      uint8_t custom[3];
    };
  } extra;

  //! \}

  //! \name Initialization
  //! \{

  //! Initializes the fetch data header by resetting all header members and initializing the reference count to `rc`.
  BL_INLINE void init_header(uint32_t rc, FormatExt format = FormatExt::kNone) noexcept {
    signature.reset();
    batch_id = 0;
    ref_count = rc;
    extra.packed = 0;
    extra.format = uint8_t(format);
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG bool is_solid() const noexcept { return signature.is_solid(); }

  BL_INLINE void retain(uint32_t n = 1) noexcept { ref_count += n; }

  BL_INLINE_NODEBUG const void* get_pipeline_data() const noexcept {
    return reinterpret_cast<const uint8_t*>(this) + sizeof(RenderFetchDataHeader);
  }

  //! \}
};

BL_STATIC_ASSERT(sizeof(RenderFetchDataHeader) == 16);

//! FetchData that can only hold a solid color.
struct RenderFetchDataSolid : public RenderFetchDataHeader {
  Pipeline::FetchData::Solid pipeline_data;
};

//! Raster context fetch data.
//!
//! Contains pipeline fetch data and additional members that are required by
//! the rendering engine for proper pipeline construction and memory management.
struct alignas(16) RenderFetchData : public RenderFetchDataHeader {
  //! \name Types
  //! \{

  typedef void (BL_CDECL* DestroyFunc)(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data) noexcept;

  //! \}

  //! \name Members
  //! \{

  //! Fetch data part, which is used by pipelines.
  Pipeline::FetchData pipeline_data;

  //! Link to the external object holding the style data (BLImage or BLGradient).
  BLObjectCore style;

  //! Releases this fetch_data to the rendering context, can only be called if the reference count is decreased
  //! to zero. Don't use manually.
  DestroyFunc destroy_func;

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG bool is_pending() const noexcept { return signature.has_pending_flag(); }
  BL_INLINE_NODEBUG Pipeline::FetchType fetch_type() const noexcept { return signature.fetch_type(); }

  template<typename T>
  BL_INLINE_NODEBUG const T& style_as() const noexcept { return static_cast<const T&>(style); }

  BL_INLINE_NODEBUG const BLImage& image() const noexcept { return static_cast<const BLImage&>(style); }
  BL_INLINE_NODEBUG const BLPattern& pattern() const noexcept { return static_cast<const BLPattern&>(style); }
  BL_INLINE_NODEBUG const BLGradient& gradient() const noexcept { return static_cast<const BLGradient&>(style); }

  //! \}

  //! \name Initialization
  //! \{

  BL_INLINE void init_style_object(const BLObjectCore* src) noexcept { style._d = src->_d; }
  BL_INLINE void init_destroy_func(DestroyFunc fn) noexcept { destroy_func = fn; }

  BL_INLINE void init_style_object_and_destroy_func(const BLObjectCore* src, DestroyFunc fn) noexcept {
    init_style_object(src);
    init_destroy_func(fn);
  }

  BL_INLINE void init_image_source(const BLImageImpl* image_impl, const BLRectI& area) noexcept {
    BL_ASSERT(area.x >= 0);
    BL_ASSERT(area.y >= 0);
    BL_ASSERT(area.w >= 0);
    BL_ASSERT(area.h >= 0);

    const uint8_t* src_pixel_data = static_cast<const uint8_t*>(image_impl->pixel_data);
    intptr_t src_stride = image_impl->stride;
    uint32_t src_bytes_per_pixel = image_impl->depth / 8u;
    Pipeline::FetchUtils::init_image_source(pipeline_data.pattern,
      (src_pixel_data + intptr_t(uint32_t(area.y)) * src_stride) + uint32_t(area.x) * src_bytes_per_pixel, image_impl->stride, area.w, area.h);
  }

  // Initializes `fetch_data` for a blit. Blits are never repeating and are always 1:1 (no scaling, no fractional translation).
  BL_INLINE bool setup_pattern_blit(int tx, int ty) noexcept {
    signature = Pipeline::FetchUtils::init_pattern_blit(pipeline_data.pattern, tx, ty);
    return true;
  }

  BL_INLINE bool setup_pattern_fx_fy(BLExtendMode extend_mode, BLPatternQuality quality, uint32_t bytes_per_pixel, int64_t tx_fixed, int64_t ty_fixed) noexcept {
    signature = Pipeline::FetchUtils::init_pattern_fx_fy(pipeline_data.pattern, extend_mode, quality, bytes_per_pixel, tx_fixed, ty_fixed);
    return true;
  }

  BL_INLINE bool setup_pattern_affine(BLExtendMode extend_mode, BLPatternQuality quality, uint32_t bytes_per_pixel, const BLMatrix2D& transform) noexcept {
    signature = Pipeline::FetchUtils::init_pattern_affine(pipeline_data.pattern, extend_mode, quality, bytes_per_pixel, transform);
    return !signature.has_pending_flag();
  }

  //! \}

  //! \name Reference Counting
  //! \{

  BL_INLINE void release(BLRasterContextImpl* ctx_impl) noexcept {
    if (--ref_count == 0)
      destroy_func(ctx_impl, this);
  }

  //! \}
};

BL_HIDDEN BLResult compute_pending_fetch_data(RenderFetchData* fetch_data) noexcept;

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERFETCHDATA_P_H_INCLUDED
