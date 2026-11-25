// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_WORKERMANAGER_P_H_INCLUDED
#define BLEND2D_RASTER_WORKERMANAGER_P_H_INCLUDED

#include <blend2d/core/image.h>
#include <blend2d/core/path.h>
#include <blend2d/geometry/commons_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/renderbatch_p.h>
#include <blend2d/raster/renderqueue_p.h>
#include <blend2d/raster/statedata_p.h>
#include <blend2d/raster/workdata_p.h>
#include <blend2d/raster/workersynchronization_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenalist_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/zeroallocator_p.h>
#include <blend2d/threading/atomic_p.h>
#include <blend2d/threading/conditionvariable_p.h>
#include <blend2d/threading/mutex_p.h>
#include <blend2d/threading/thread_p.h>
#include <blend2d/threading/threadpool_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

template<typename T>
struct PreallocatedStructPool {
  //! \name Members
  //! \{

  T* ptr {};
  T* end {};

  //! \}

  //! \name Interface
  //! \{

  BL_INLINE_NODEBUG bool exhausted() const noexcept { return ptr >= end; }

  BL_INLINE_NODEBUG void reset() noexcept {
    ptr = nullptr;
    end = nullptr;
  }

  BL_INLINE void advance(size_t n = 1) noexcept {
    BL_ASSERT(!exhausted());
    ptr += n;
  }

  BL_INLINE BLResult preallocate(ArenaAllocator& allocator, uint32_t count) noexcept {
    constexpr uint32_t kAlignment = uint32_t(alignof(T));
    constexpr uint32_t kItemSize = uint32_t(sizeof(T));

    allocator.align(kAlignment);

    size_t remaining = allocator.remaining_size();

    // If there is not enough space to allocate all the items, then reduce the number of items to be allocated.
    // This makes it possible to use memory that would otherwise be wasted (the allocation of the requested number
    // of items would require a new block).
    if (remaining >= kItemSize && remaining < count * kItemSize)
      count = uint32_t(remaining) / kItemSize;

    T* allocated_ptr = allocator.allocT<T>(count * kItemSize, kAlignment);
    if (BL_UNLIKELY(!allocated_ptr))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    ptr = allocated_ptr;
    end = allocated_ptr + count;
    return BL_SUCCESS;
  }

  //! \}
};

struct PreallocatedBytePool {
  //! \name Members
  //! \{

  uint8_t* ptr {};
  uint8_t* end {};

  //! \}

  //! \name Interface
  //! \{

  BL_INLINE_NODEBUG bool exhausted() const noexcept {
    // NOTE: Must `ptr >= end` as we might over-allocate for some specific purposes (like allocating 2 things at
    // once). In such case the over-allocation is not accounted in the `end` pointer and the code that uses the
    // pool simply allocates more - but it must guarantee that it doesn't allocate more than extra bytes reserved
    // for this use-case.
    //
    // Most often this would be used when both Fill and Stroke shared states have to be created - to simplify the
    // logic and minimize error handling in the rendering context, both states are allocated at once.
    return ptr >= end;
  }

  BL_INLINE_NODEBUG void reset() noexcept {
    ptr = nullptr;
    end = nullptr;
  }

  BL_INLINE void* alloc(size_t size) noexcept {
    void* p = ptr;
    ptr += size;
    return p;
  }

  BL_INLINE BLResult preallocate(ArenaAllocator& allocator, uint32_t minimum_size, uint32_t default_size, uint32_t extra_size, uint32_t alignment) noexcept {
    allocator.align(alignment);

    size_t remaining = allocator.remaining_size();
    size_t n = default_size;

    // Just consume everything in case the buffer is not long enough to hold the `default_size`, however,
    // also check whether it can hold `minimum_byte_count` - if not, a new buffer has to be allocated, which would
    // be handled by `allocator.alloc()` automatically when the required allocation size exceeds the remaining
    // capacity of the current block.
    if (remaining >= minimum_size + extra_size && remaining < default_size + extra_size)
      n = remaining - extra_size;

    uint8_t* allocated_ptr = static_cast<uint8_t*>(allocator.alloc(n + extra_size, alignment));
    if (BL_UNLIKELY(!allocated_ptr))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    ptr = allocated_ptr;
    end = allocated_ptr + n;
    return BL_SUCCESS;
  }

  //! \}
};

class WorkerManager {
public:
  BL_NONCOPYABLE(WorkerManager)

  enum : uint32_t { kAllocatorAlignment = 8 };

  //! \name Members
  //! \{

  //! Arena allocator used to allocate commands, jobs, and related data.
  ArenaAllocator _allocator;

  //! Current batch where objects are appended to.
  RenderBatch* _current_batch;
  //! Command appender.
  RenderCommandAppender _command_appender;
  //! Job appender.
  RenderJobAppender _job_appender;

  //! Preallocated fetch data - multiple FetchData structs are allocated at a time, and then used during dispatching.
  PreallocatedStructPool<RenderFetchData> _fetch_data_pool;

  //! Preallocated shared data pool - used by shared fill and stroke states.
  PreallocatedBytePool _shared_data_pool;

  //! Thread-pool that owns worker threads.
  BLThreadPool* _thread_pool;
  //! Worker threads acquired from `_thread_pool`.
  BLThread** _worker_threads;
  //! Work data for each worker thread.
  WorkData** _work_data_storage;

  //! Work synchronization
  WorkerSynchronization _synchronization;

  //! Indicates that a worker manager is active.
  uint32_t _is_active;
  //! Number of worker threads.
  uint32_t _thread_count;
  //! Number of bands,
  uint32_t _band_count;
  //! Batch id, an incrementing number that is assigned to FetchData.
  uint32_t _batch_id;
  //! Number of commands in the queue.
  uint32_t _command_queue_count;
  //! Maximum number of commands in a queue.
  uint32_t _command_queue_limit;
  //! Count of data slots.
  uint32_t _state_slot_count;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE WorkerManager() noexcept
    : _allocator(131072, kAllocatorAlignment),
      _current_batch{},
      _command_appender{},
      _job_appender{},
      _fetch_data_pool{},
      _shared_data_pool{},
      _thread_pool{},
      _worker_threads{},
      _work_data_storage{},
      _synchronization(),
      _is_active{},
      _thread_count{},
      _band_count{},
      _batch_id{1},
      _command_queue_count{},
      _command_queue_limit{},
      _state_slot_count{} {}

  BL_INLINE ~WorkerManager() noexcept {
    // Cannot be active upon destruction!
    BL_ASSERT(!is_active());
  }

  //! \}

  //! \name Explicit Initialization
  //! \{

  //! Initializes the worker manager with the specified number of threads.
  BLResult init(BLRasterContextImpl* ctx_impl, const BLContextCreateInfo* create_info) noexcept;

  BLResult init_work_memory(size_t zeroed_memory_size) noexcept;

  BL_INLINE void init_first_batch() noexcept {
    RenderBatch* batch = _allocator.alloc_zeroedT<RenderBatch>();

    // We have preallocated enough, cannot happen.
    BL_ASSERT(batch != nullptr);

    batch->_command_list.reset(new_command_queue());
    batch->_job_list.reset(new_job_queue());

    // We have preallocated enough, cannot happen.
    BL_ASSERT(batch->_command_list.first() != nullptr);
    BL_ASSERT(batch->_job_list.first() != nullptr);

    _current_batch = batch;
    _job_appender.reset(*batch->_job_list.first());
    _command_appender.reset(*batch->_command_list.first());

    BLResult result = _preallocate_fetch_data_pool() |
                      _preallocate_shared_data_pool();

    // We have preallocated enough, cannot happen.
    BL_ASSERT(result == BL_SUCCESS);
    bl_unused(result);

    _command_queue_count = 0;
    _state_slot_count = 0;
  }

  //! Releases all acquired threads and destroys all work contexts.
  //!
  //! \note It's only safe to call `reset()` after all threads have finalized their work. It would be disaster to
  //! call `reset()` when one or more thread is still running as reset destroys all work contexts, so the threads
  //! would be using freed memory.
  void reset() noexcept;

  //! \}

  //! \name Interface
  //! \{

  //! Returns `true` when the worker manager is active.
  BL_INLINE_NODEBUG bool is_active() const noexcept { return _is_active != 0; }

  BL_INLINE_NODEBUG uint32_t thread_count() const noexcept { return _thread_count; }

  //! \}

  //! \name Command Data
  //! \{

  BL_INLINE_NODEBUG RenderCommandAppender& command_appender() noexcept { return _command_appender; }

  BL_INLINE_NODEBUG RenderCommand* current_command() noexcept { return _command_appender.current_command(); }
  BL_INLINE_NODEBUG uint32_t next_state_slot_index() noexcept { return _state_slot_count++; }

  BL_INLINE_NODEBUG bool is_command_queue_full() const noexcept { return _command_appender.full(); }

  BL_INLINE bool has_pending_commands() const noexcept {
    RenderCommandQueue* first = _current_batch->_command_list.first();
    RenderCommandQueue* last = _current_batch->_command_list.last();

    return first != last || !_command_appender.is_empty();
  }

  BL_INLINE RenderCommandQueue* new_command_queue() noexcept {
    RenderCommandQueue* p = _allocator.allocNoAlignT<RenderCommandQueue>(IntOps::align_up(RenderCommandQueue::size_of(), kAllocatorAlignment));
    if (BL_UNLIKELY(!p))
      return nullptr;
    return new(BLInternal::PlacementNew{p}) RenderCommandQueue();
  }

  BL_INLINE void before_grow_command_queue() noexcept {
    _command_queue_count += kRenderQueueCapacity;
  }

  BL_INLINE BLResult _grow_command_queue() noexcept {
    // Can only be called when the current command queue is full.
    BL_ASSERT(_command_appender.full());

    RenderCommandQueue* command_queue = current_batch()->_command_list.last();
    _command_appender.done(*command_queue);
    current_batch()->_command_count += uint32_t(command_queue->size());

    command_queue = new_command_queue();
    if (BL_UNLIKELY(!command_queue))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    current_batch()->_command_list.append(command_queue);
    _command_appender.reset(*command_queue);

    return BL_SUCCESS;
  }

  //! \}

  //! \name Job Data
  //! \{

  BL_INLINE_NODEBUG bool is_job_queue_full() const noexcept { return _job_appender.full(); }

  BL_INLINE RenderJobQueue* new_job_queue() noexcept {
    RenderJobQueue* p = _allocator.allocNoAlignT<RenderJobQueue>(IntOps::align_up(RenderJobQueue::size_of(), kAllocatorAlignment));
    if (BL_UNLIKELY(!p))
      return nullptr;
    p->reset();
    return p;
  }

  BL_INLINE BLResult _grow_job_queue() noexcept {
    // Can only be called when the current job queue is full.
    BL_ASSERT(_job_appender.full());

    RenderJobQueue* job_queue = current_batch()->_job_list.last();
    _job_appender.done(*job_queue);
    current_batch()->_job_count += uint32_t(job_queue->size());

    job_queue = new_job_queue();
    if (BL_UNLIKELY(!job_queue))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    current_batch()->_job_list.append(job_queue);
    _job_appender.reset(*job_queue);

    return BL_SUCCESS;
  }

  BL_INLINE void add_job(RenderJob* job) noexcept {
    BL_ASSERT(!is_job_queue_full());
    _job_appender.append(job);
  }

  //! \}

  //! \name Fetch Data
  //! \{

  BL_INLINE_NODEBUG bool is_fetch_data_pool_exhausted() const noexcept {
    return _fetch_data_pool.exhausted();
  }

  BL_INLINE BLResult _preallocate_fetch_data_pool() noexcept {
    return _fetch_data_pool.preallocate(_allocator, 32);
  }

  //! \}

  //! \name Shared Data
  //! \{

  BL_INLINE_NODEBUG bool is_shared_data_pool_exhausted() const noexcept {
    return _shared_data_pool.exhausted();
  }

  BL_INLINE BLResult _preallocate_shared_data_pool() noexcept {
    constexpr uint32_t kCombinedStateSize = uint32_t(sizeof(SharedFillState) + sizeof(SharedExtendedStrokeState));

    constexpr uint32_t kMinimumSize = kCombinedStateSize;
    constexpr uint32_t kDefaultSize = kCombinedStateSize * 20;
    constexpr uint32_t kExtraSize = kCombinedStateSize;

    return _shared_data_pool.preallocate(_allocator, kMinimumSize, kDefaultSize, kExtraSize, 16u);
  }

  template<typename T>
  BL_INLINE T* allocate_from_shared_data_pool(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(_shared_data_pool.alloc(size));
  }

  //! \}

  //! \name Work Batch
  //! \{

  BL_INLINE_NODEBUG RenderBatch* current_batch() const noexcept { return _current_batch; }
  BL_INLINE_NODEBUG uint32_t current_batch_id() const noexcept { return _batch_id; }

  BL_INLINE_NODEBUG bool is_batch_full() const noexcept { return _command_queue_count >= _command_queue_limit; }

  BL_INLINE void finalize_batch() noexcept {
    RenderJobQueue* last_job_queue = _current_batch->_job_list.last();
    RenderCommandQueue* last_command_queue = _current_batch->_command_list.last();

    _job_appender.done(*last_job_queue);
    _command_appender.done(*last_command_queue);

    _current_batch->_worker_count = _thread_count + 1;
    _current_batch->_job_count += uint32_t(last_job_queue->size());
    _current_batch->_command_count += uint32_t(last_command_queue->size());
    _current_batch->_state_slot_count = _state_slot_count;
    _current_batch->_band_count = _band_count;
    // TODO: [Rendering Context] Not used. the idea is that after the batch is processed we can reuse the blocks of the allocator (basically move it after the current block).
    // _current_batch->_past_block = _allocator.past_block();

    if (++_batch_id == 0)
      _batch_id = 1;

    _command_queue_count = 0;
    _state_slot_count = 0;
  }

  //! \}
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_WORKERMANAGER_P_H_INCLUDED
