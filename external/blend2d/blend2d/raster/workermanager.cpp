// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/raster/rastercontext_p.h>
#include <blend2d/raster/workdata_p.h>
#include <blend2d/raster/workermanager_p.h>
#include <blend2d/support/intops_p.h>

namespace bl::RasterEngine {

// bl::RasterEngine::WorkerManager - Init
// ======================================

BLResult WorkerManager::init(BLRasterContextImpl* ctx_impl, const BLContextCreateInfo* create_info) noexcept {
  uint32_t init_flags = create_info->flags;
  uint32_t thread_count = create_info->thread_count;
  uint32_t command_queue_limit = IntOps::align_up(create_info->command_queue_limit, kRenderQueueCapacity);

  BL_ASSERT(!is_active());
  BL_ASSERT(thread_count > 0);

  ArenaAllocator& zone = ctx_impl->base_zone;
  ArenaAllocator::StatePtr zone_state = zone.save_state();

  // We must enforce some hard limit here...
  if (thread_count > BL_RUNTIME_MAX_THREAD_COUNT)
    thread_count = BL_RUNTIME_MAX_THREAD_COUNT;

  // If the command queue limit is not specified, use the default.
  if (command_queue_limit == 0)
    command_queue_limit = BL_RASTER_CONTEXT_DEFAULT_COMMAND_QUEUE_LIMIT;

  // We count the user thread as a worker thread as well. In this case this one doesn't need a separate work_data
  // as it can use the 'sync_work_data' owned by the rendering context.
  uint32_t worker_count = thread_count - 1;

  // Fallback to synchronous rendering immediately if this combination was selected.
  if (!worker_count && (init_flags & BL_CONTEXT_CREATE_FLAG_FALLBACK_TO_SYNC))
    return BL_SUCCESS;

  // Forces the zone-allocator to preallocate the first block of memory, if not allocated yet.
  size_t batch_context_size = sizeof(RenderBatch) +
                            RenderJobQueue::size_of() +
                            RenderCommandQueue::size_of();
  BL_PROPAGATE(_allocator.ensure(batch_context_size));

  // Allocate space for worker threads data.
  if (worker_count) {
    BLThread** worker_threads = zone.allocT<BLThread*>(IntOps::align_up(worker_count * sizeof(void*), 8));
    WorkData** work_data_storage = zone.allocT<WorkData*>(IntOps::align_up(worker_count * sizeof(void*), 8));

    if (!worker_threads || !work_data_storage) {
      zone.restore_state(zone_state);
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
    }

    // Get global thread-pool or create an isolated one.
    BLThreadPool* thread_pool = nullptr;
    if (init_flags & BL_CONTEXT_CREATE_FLAG_ISOLATED_THREAD_POOL) {
      thread_pool = bl_thread_pool_create();
      if (!thread_pool)
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
    }
    else {
      thread_pool = bl_thread_pool_global()->add_ref();
    }

    // Acquire threads passed to thread-pool.
    BLResult reason = BL_SUCCESS;
    uint32_t acquire_thread_flags = 0;
    uint32_t n = thread_pool->acquire_threads(worker_threads, worker_count, acquire_thread_flags, &reason);

    if (reason != BL_SUCCESS)
      ctx_impl->sync_work_data.accumulate_error(reason);

    for (uint32_t i = 0; i < n; i++) {
      // NOTE: We really want work data to be aligned to the cache line as each instance will be used from a different
      // thread. This means that they should not interfere with each other as that could slow down things significantly.
      WorkData* work_data = zone.allocT<WorkData>(IntOps::align_up(sizeof(WorkData), BL_CACHE_LINE_SIZE), BL_CACHE_LINE_SIZE);
      work_data_storage[i] = work_data;

      if (!work_data) {
        ctx_impl->sync_work_data.accumulate_error(bl_make_error(BL_ERROR_OUT_OF_MEMORY));
        thread_pool->release_threads(worker_threads, n);
        n = 0;
        break;
      }
    }

    if (!n) {
      thread_pool->release();
      thread_pool = nullptr;
      worker_threads = nullptr;
      work_data_storage = nullptr;
      zone.restore_state(zone_state);

      // Fallback to synchronous rendering - nothing else to clean up as we haven't initialized anything.
      if (init_flags & BL_CONTEXT_CREATE_FLAG_FALLBACK_TO_SYNC)
        return BL_SUCCESS;
    }
    else {
      // Initialize worker contexts.
      WorkerSynchronization* synchronization = &_synchronization;
      for (uint32_t i = 0; i < n; i++) {
        bl_call_ctor(*work_data_storage[i], ctx_impl, synchronization, i + 1);
        work_data_storage[i]->init_band_data(ctx_impl->band_height(), ctx_impl->band_count(), ctx_impl->command_quantization_shift_aa());
      }
    }

    _thread_pool = thread_pool;
    _worker_threads = worker_threads;
    _work_data_storage = work_data_storage;
    _thread_count = n;
  }
  else {
    // In this case we use the worker manager, but we don't really manage any threads...
    _thread_count = 0;
  }

  _is_active = true;
  _band_count = ctx_impl->band_count();
  _command_queue_limit = command_queue_limit;

  init_first_batch();
  return BL_SUCCESS;
}

BLResult WorkerManager::init_work_memory(size_t zeroed_memory_size) noexcept {
  uint32_t n = thread_count();
  for (uint32_t i = 0; i < n; i++) {
    BL_PROPAGATE(_work_data_storage[i]->zero_buffer.ensure(zeroed_memory_size));
  }
  return BL_SUCCESS;
}

// bl::RasterEngine::WorkerManager - Reset
// =======================================

void WorkerManager::reset() noexcept {
  if (!is_active())
    return;

  _is_active = false;

  if (_thread_pool) {
    for (uint32_t i = 0; i < _thread_count; i++)
      bl_call_dtor(*_work_data_storage[i]);

    _thread_pool->release_threads(_worker_threads, _thread_count);
    _thread_pool->release();

    _thread_pool = nullptr;
    _worker_threads = nullptr;
    _work_data_storage = nullptr;
    _thread_count = 0;
  }

  _command_queue_count = 0;
  _command_queue_limit = 0;
  _state_slot_count = 0;
}

} // {bl::RasterEngine}
