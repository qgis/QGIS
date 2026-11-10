// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/raster/rastercontext_p.h>
#include <blend2d/raster/workdata_p.h>

namespace bl::RasterEngine {

// bl::RasterEngine::WorkData - Construction & Destruction
// =======================================================

WorkData::WorkData(BLRasterContextImpl* ctx_impl, WorkerSynchronization* synchronization, uint32_t worker_id) noexcept
  : ctx_impl(ctx_impl),
    synchronization(synchronization),
    _batch(nullptr),
    ctx_data(),
    clip_mode(BL_CLIP_MODE_ALIGNED_RECT),
    _commandQuantizationShiftAA(0),
    _command_quantization_shift_fp(0),
    reserved{},
    _worker_id(worker_id),
    _band_height(0),
    _accumulated_error_flags(0),
    work_zone(65536, 8),
    work_state{},
    zero_buffer(),
    edge_storage(),
    edge_builder(&work_zone, &edge_storage) {}

WorkData::~WorkData() noexcept {
  if (edge_storage.band_edges())
    bl_zero_allocator_release(edge_storage.band_edges(), edge_storage.band_capacity() * kEdgeListSize);
}

// bl::RasterEngine::WorkData - Initialization
// ===========================================

BLResult WorkData::init_band_data(uint32_t band_height, uint32_t band_count, uint32_t command_quantization_shift) noexcept {
  // Can only happen if the storage was already allocated.
  if (band_count <= edge_storage.band_capacity()) {
    _band_height = band_height;
    edge_storage.init_data(edge_storage.band_edges(), band_count, edge_storage.band_capacity(), band_height);
  }
  else {
    size_t allocated_size = 0;
    EdgeList<int>* edges = static_cast<EdgeList<int>*>(
      bl_zero_allocator_resize(
        edge_storage.band_edges(),
        edge_storage.band_capacity() * kEdgeListSize,
        band_count * kEdgeListSize,
        &allocated_size));

    if (BL_UNLIKELY(!edges)) {
      edge_storage.reset();
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
    }

    uint32_t band_capacity = uint32_t(allocated_size / kEdgeListSize);
    _band_height = band_height;
    edge_storage.init_data(edges, band_count, band_capacity, band_height);
  }

  _commandQuantizationShiftAA = uint8_t(command_quantization_shift);
  _command_quantization_shift_fp = uint8_t(command_quantization_shift + 8);

  return BL_SUCCESS;
}

// bl::RasterEngine::WorkData - Error Accumulation
// ===============================================

BLResult WorkData::accumulate_error(BLResult error) noexcept {
  switch (error) {
    // Should not happen.
    case BL_SUCCESS: break;

    case BL_ERROR_INVALID_VALUE        : _accumulated_error_flags |= BL_CONTEXT_ERROR_FLAG_INVALID_VALUE        ; break;
    case BL_ERROR_INVALID_GEOMETRY     : _accumulated_error_flags |= BL_CONTEXT_ERROR_FLAG_INVALID_GEOMETRY     ; break;
    case BL_ERROR_INVALID_GLYPH        : _accumulated_error_flags |= BL_CONTEXT_ERROR_FLAG_INVALID_GLYPH        ; break;
    case BL_ERROR_FONT_NOT_INITIALIZED : _accumulated_error_flags |= BL_CONTEXT_ERROR_FLAG_INVALID_FONT         ; break;
    case BL_ERROR_THREAD_POOL_EXHAUSTED: _accumulated_error_flags |= BL_CONTEXT_ERROR_FLAG_THREAD_POOL_EXHAUSTED; break;
    case BL_ERROR_OUT_OF_MEMORY        : _accumulated_error_flags |= BL_CONTEXT_ERROR_FLAG_OUT_OF_MEMORY        ; break;
    default                            : _accumulated_error_flags |= BL_CONTEXT_ERROR_FLAG_UNKNOWN_ERROR        ; break;
  }
  return error;
}

} // {bl::RasterEngine}
