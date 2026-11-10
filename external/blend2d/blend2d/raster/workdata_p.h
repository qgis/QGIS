// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_WORKDATA_P_H_INCLUDED
#define BLEND2D_RASTER_WORKDATA_P_H_INCLUDED

#include <blend2d/core/image.h>
#include <blend2d/core/path.h>
#include <blend2d/geometry/commons_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/zeroallocator_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

class RenderBatch;
class WorkerSynchronization;

//! Provides data used by both single-threaded and multi-threaded render command processing. Single-threaded rendering
//! context uses this data synchronously to process commands that are required before using pipelines. Multi-threaded
//! rendering context uses 1 + N WorkData instances, where the first one can be used synchronously by the rendering
//! context to perform synchronous tasks while the remaining WorkData is used per worker thread.
class WorkData {
public:
  BL_NONCOPYABLE(WorkData)

  enum : uint32_t { kSyncWorkerId = 0u };
  enum : size_t { kEdgeListSize = sizeof(EdgeList<int>) };

  //! Rendering context impl.
  BLRasterContextImpl* ctx_impl {};
  //! Worker synchronization.
  WorkerSynchronization* synchronization {};
  //! Batch data to process in case this data is used in a worker thread.
  RenderBatch* _batch {};
  //! Context data used by pipelines (either the destination data or layer).
  Pipeline::ContextData ctx_data {};

  //! Clip mode.
  uint8_t clip_mode {};
  //! Quantization shift of vertical coordinates - used to store quantized coordinates in command queue (aligned coordinates).
  uint8_t _commandQuantizationShiftAA;
  //! Quantization shift of vertical coordinates - used to store quantized coordinates in command queue (fractional coordinates).
  uint8_t _command_quantization_shift_fp;
  //! Reserved.
  uint8_t reserved[2] {};
  //! Id of the worker that uses this WorkData.
  uint32_t _worker_id {};
  //! Band height.
  uint32_t _band_height {};
  //! Accumulated error flags.
  uint32_t _accumulated_error_flags {};

  //! Temporary paths.
  BLPath tmp_path[4];
  //! Temporary glyph buffer used by high-level text rendering calls.
  BLGlyphBuffer glyph_buffer;

  //! Arena memory used by the worker context.
  ArenaAllocator work_zone;
  //! The last state of the zone to be reverted to in case of failure.
  ArenaAllocator::StatePtr work_state {};
  //! Zero memory filled by rasterizers and zeroed back by pipelines.
  ZeroBuffer zero_buffer;
  //! Edge storage.
  EdgeStorage<int> edge_storage;
  //! Edge builder.
  EdgeBuilder<int> edge_builder;

  explicit WorkData(BLRasterContextImpl* ctx_impl, WorkerSynchronization* synchronization, uint32_t worker_id = kSyncWorkerId) noexcept;
  ~WorkData() noexcept;

  // NOTE: `init_context_data()` is called after `init_band_data()` in `bl_raster_context_impl_attach()`.

  BL_INLINE void init_batch(RenderBatch* batch) noexcept { bl_atomic_store_strong(&_batch, batch); }
  BL_INLINE void reset_batch() noexcept { init_batch(nullptr); }
  BL_INLINE RenderBatch* acquire_batch() noexcept { return bl_atomic_fetch_strong(&_batch); }

  BL_INLINE void init_context_data(const BLImageData& dst_data, const BLPointI& pixel_origin) noexcept {
    ctx_data.dst = dst_data;
    ctx_data.pixel_origin = pixel_origin;
  }

  BLResult init_band_data(uint32_t band_height, uint32_t band_count, uint32_t command_quantization_shift) noexcept;

  BL_INLINE_NODEBUG bool is_sync() const noexcept { return _worker_id == kSyncWorkerId; }
  BL_INLINE_NODEBUG const BLSizeI& dst_size() const noexcept { return ctx_data.dst.size; }
  BL_INLINE_NODEBUG uint32_t worker_id() const noexcept { return _worker_id; }
  BL_INLINE_NODEBUG uint32_t band_height() const noexcept { return _band_height; }
  BL_INLINE_NODEBUG uint32_t band_height_fixed() const noexcept { return _band_height << 8; }
  BL_INLINE_NODEBUG uint32_t band_count() const noexcept { return edge_storage.band_count(); }

  BL_INLINE_NODEBUG uint32_t command_quantization_shift_aa() const noexcept { return _commandQuantizationShiftAA; }
  BL_INLINE_NODEBUG uint32_t command_quantization_shift_fp() const noexcept { return _command_quantization_shift_fp; }

  BL_INLINE_NODEBUG BLContextErrorFlags accumulated_error_flags() const noexcept { return BLContextErrorFlags(_accumulated_error_flags); }

  BL_INLINE_NODEBUG void accumulate_error_flag(BLContextErrorFlags flag) noexcept { _accumulated_error_flags |= uint32_t(flag); }
  BL_INLINE_NODEBUG void clean_accumulated_error_flags() noexcept { _accumulated_error_flags = 0; }

  BL_INLINE void avoid_cache_line_sharing() noexcept {
    work_zone.align(BL_CACHE_LINE_SIZE);
  }

  BL_INLINE void start_over() noexcept {
    work_zone.clear();
    work_state = ArenaAllocator::StatePtr{};
    edge_storage.clear();
  }

  BL_INLINE void save_state() noexcept {
    work_state = work_zone.save_state();
  }

  BL_INLINE void restore_state() noexcept {
    work_zone.restore_state(work_state);
  }

  BL_INLINE void revert_edge_builder() noexcept {
    edge_builder.merge_bounding_box();
    edge_storage.clear();
    work_zone.restore_state(work_state);
  }

  //! Accumulates the error result into error flags of this work-data. Used by both synchronous and asynchronous
  //! rendering context to accumulate errors that may happen during the rendering.
  BLResult accumulate_error(BLResult error) noexcept;
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_WORKDATA_P_H_INCLUDED
