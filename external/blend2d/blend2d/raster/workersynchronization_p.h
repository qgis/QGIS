// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_WORKERSYNCHRONIZATION_P_H_INCLUDED
#define BLEND2D_RASTER_WORKERSYNCHRONIZATION_P_H_INCLUDED

#include <blend2d/threading/atomic_p.h>
#include <blend2d/threading/conditionvariable_p.h>
#include <blend2d/threading/mutex_p.h>
#include <blend2d/threading/tsanutils_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

class alignas(BL_CACHE_LINE_SIZE) WorkerSynchronization {
public:
  BL_NONCOPYABLE(WorkerSynchronization)

  struct alignas(BL_CACHE_LINE_SIZE) Header {
    bool use_futex;
    Threading::TSanBarrier barrier;
  };

  struct alignas(BL_CACHE_LINE_SIZE) Status {
    // These are used by both portable and futex implementation.
    uint32_t jobs_running_count;
    uint32_t threads_running_count;
    uint32_t waiting_for_completion;

    uint8_t padding[64 - 12];

    // These are only really used by futex implementation, however, the variables are always stored to.
    uint32_t futex_jobs_finished;
    uint32_t futex_bands_finished;
  };

  struct alignas(BL_CACHE_LINE_SIZE) PortableData {
    BL_INLINE PortableData() noexcept {}
    BL_INLINE ~PortableData() noexcept {}

    BLMutex mutex;
    BLConditionVariable jobs_condition;
    BLConditionVariable done_condition;
  };

  Header _header;
  Status _status;
  PortableData _portable_data;

  WorkerSynchronization() noexcept;
  ~WorkerSynchronization() noexcept;

  BL_INLINE_NODEBUG bool use_futex() const noexcept { return _header.use_futex; }

  BL_INLINE void before_start(uint32_t thread_count, bool has_jobs) noexcept {
    bl_atomic_store_relaxed(&_status.jobs_running_count, has_jobs ? uint32_t(thread_count + 1) : uint32_t(0));
    bl_atomic_store_relaxed(&_status.threads_running_count, thread_count);
    bl_atomic_store_strong(&_status.futex_jobs_finished, 0u);

    _header.barrier.release();
  }

  BL_INLINE void thread_started() noexcept {
    _header.barrier.acquire();
  }

  // Called when there are no jobs at all to acknowledge that `wait_for_jobs_to_finish()` would never be called.
  BL_INLINE void no_jobs_to_wait_for() noexcept {
    bl_unused(
      bl_atomic_fetch_strong(&_status.futex_jobs_finished)
    );
  }

  void wait_for_jobs_to_finish() noexcept;
  void thread_done() noexcept;
  void wait_for_threads_to_finish() noexcept;
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_WORKERSYNCHRONIZATION_P_H_INCLUDED
