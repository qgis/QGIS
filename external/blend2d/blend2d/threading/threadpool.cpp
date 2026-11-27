// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/fixedbitarray_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/wrap_p.h>
#include <blend2d/threading/atomic_p.h>
#include <blend2d/threading/conditionvariable_p.h>
#include <blend2d/threading/mutex_p.h>
#include <blend2d/threading/threadpool_p.h>

#ifdef _WIN32
  #include <process.h>
#endif

// ThreadPool - Globals
// ====================

static BLThreadPoolVirt bl_thread_pool_virt;

// ThreadPool - Internal
// =====================

class BLInternalThreadPool : public BLThreadPool {
public:
  enum : uint32_t { kMaxThreadCount = 64 };
  typedef bl::FixedBitArray<BLBitWord, kMaxThreadCount> PooledThreadBitArray;

  //! \name Members
  //! \{

  //! Counts the number of references to the thread pool from outside (not counting threads).
  size_t ref_count {};
  //! Counts one reference from outside and each thread the thread pool manages.
  size_t internal_ref_count {};

  uint32_t stack_size {};
  uint32_t max_thread_count {};
  uint32_t created_thread_count {};
  uint32_t pooled_thread_count {};
  uint32_t acquired_thread_count {};
  uint32_t destroyWaitTimeInMS {};
  uint32_t waiting_on_destroy {};

  BLMutex mutex;
  BLConditionVariable destroy_condition;
  BLThreadAttributes thread_attributes {};

  PooledThreadBitArray pooled_thread_bits {};
  BLThread* threads[kMaxThreadCount] {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_NOINLINE explicit BLInternalThreadPool() noexcept
    : BLThreadPool { &bl_thread_pool_virt },
      ref_count(1),
      internal_ref_count(1),
      max_thread_count(kMaxThreadCount),
      destroyWaitTimeInMS(100),
      mutex(),
      destroy_condition() {}

  BL_NOINLINE ~BLInternalThreadPool() noexcept {}

  //! \}

  BL_NOINLINE void perform_exit_cleanup() noexcept {
    uint32_t num_tries = 5;
    uint64_t wait_time = (uint64_t(destroyWaitTimeInMS) * 1000u) / num_tries;

    do {
      cleanup(BL_THREAD_QUIT_ON_EXIT);

      BLLockGuard<BLMutex> guard(mutex);
      if (bl_atomic_fetch_strong(&created_thread_count) != 0) {
        waiting_on_destroy = 1;
        if (destroy_condition.wait_for(mutex, wait_time) == BL_SUCCESS)
          break;
      }
    } while (--num_tries);
  }
};

static bl::Wrap<BLInternalThreadPool> bl_global_thread_pool;

// ThreadPool - Create & Destroy
// =============================

BLThreadPool* bl_thread_pool_create() noexcept {
  void* p = malloc(sizeof(BLInternalThreadPool));
  if (!p)
    return nullptr;
  return new(BLInternal::PlacementNew{p}) BLInternalThreadPool();
}

static void bl_thread_pool_release_internal(BLInternalThreadPool* self) noexcept {
  if (bl_atomic_fetch_sub_strong(&self->internal_ref_count) == 1) {
    self->~BLInternalThreadPool();
    if (self != &bl_global_thread_pool)
      free(self);
  }
}

// ThreadPool - AddRef & Release
// =============================

static BLThreadPool* BL_CDECL bl_thread_pool_add_ref(BLThreadPool* self_) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  bl_atomic_fetch_add_relaxed(&self->ref_count);
  return self;
}

static BLResult BL_CDECL bl_thread_pool_release(BLThreadPool* self_) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);

  // Dereference the number of outside references. If that hits zero it means to destroy the thread pool.
  // However, we have to first shut down all the threads, and then we can actually destroy the pool itself.
  if (bl_atomic_fetch_sub_strong(&self->ref_count) == 1) {
    // First try to destroy all threads - this could possibly fail.
    if (bl_atomic_fetch_strong(&self->created_thread_count) != 0)
      self->perform_exit_cleanup();

    bl_thread_pool_release_internal(self);
  }

  return BL_SUCCESS;
}

// ThreadPool - Accessors
// ======================

static uint32_t BL_CDECL bl_thread_pool_max_thread_count(const BLThreadPool* self_) noexcept {
  const BLInternalThreadPool* self = static_cast<const BLInternalThreadPool*>(self_);
  return self->max_thread_count;
}

static uint32_t BL_CDECL bl_thread_pool_pooled_thread_count(const BLThreadPool* self_) noexcept {
  const BLInternalThreadPool* self = static_cast<const BLInternalThreadPool*>(self_);
  return self->pooled_thread_count;
}

static BLResult BL_CDECL bl_thread_pool_set_thread_attributes(BLThreadPool* self_, const BLThreadAttributes* attributes) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  // Verify that the provided `stack_size` is okay.
  //   - POSIX   - Minimum stack size is `PTHREAD_STACK_MIN`, some implementations enforce alignment to a page-size.
  //   - WINDOWS - Minimum stack size is `SYSTEM_INFO::dwAllocationGranularity`, alignment should follow the
  //               granularity as well, however, WinAPI would align stack size if it's not properly aligned.
  uint32_t stack_size = attributes->stack_size;
  if (stack_size) {
    const BLRuntimeSystemInfo& si = bl_runtime_context.system_info;
    if (stack_size < si.thread_stack_size || !bl::IntOps::is_aligned(stack_size, si.allocation_granularity))
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  self->thread_attributes = *attributes;
  return BL_SUCCESS;
}

// ThreadPool - Cleanup
// ====================

static void bl_thread_pool_thread_exit_func(BLThread* thread, void* data) noexcept {
  BLInternalThreadPool* thread_pool = static_cast<BLInternalThreadPool*>(data);
  thread->destroy();

  if (bl_atomic_fetch_sub_strong(&thread_pool->created_thread_count) == 1) {
    thread_pool->mutex.protect([&]() {
      if (thread_pool->waiting_on_destroy)
        thread_pool->destroy_condition.signal();
    });
  }

  bl_thread_pool_release_internal(thread_pool);
}

static uint32_t BL_CDECL bl_thread_pool_cleanup(BLThreadPool* self_, uint32_t thread_quit_flags) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  uint32_t n = 0;
  uint32_t bw_index = 0;
  uint32_t pooled_thread_count = self->pooled_thread_count;

  if (!pooled_thread_count)
    return 0;

  do {
    BLBitWord mask = self->pooled_thread_bits.data[bw_index];
    bl::BitWordIterator<BLBitWord> it(mask);

    while (it.has_next()) {
      uint32_t thread_index = bw_index * bl::IntOps::bit_size_of<BLBitWord>() + it.next();
      BLThread* thread = self->threads[thread_index];

      self->threads[thread_index] = nullptr;
      thread->quit(thread_quit_flags);

      n++;
    }
    self->pooled_thread_bits.data[bw_index] = 0;
  } while (++bw_index < BLInternalThreadPool::PooledThreadBitArray::kFixedArraySize);

  self->pooled_thread_count = pooled_thread_count - n;
  return n;
}

// ThreadPool - Acquire & Release
// ==============================

static void bl_thread_pool_release_threads_internal(BLInternalThreadPool* self, BLThread** threads, uint32_t n) noexcept {
  uint32_t i = 0;
  uint32_t bw_index = 0;

  do {
    BLBitWord mask = self->pooled_thread_bits.data[bw_index] ^ bl::IntOps::all_ones<BLBitWord>();
    bl::BitWordIterator<BLBitWord> it(mask);

    while (it.has_next()) {
      uint32_t bit = it.next();
      mask ^= bl::IntOps::lsb_bit_at<BLBitWord>(bit);

      uint32_t thread_index = bw_index * bl::IntOps::bit_size_of<BLBitWord>() + bit;
      BL_ASSERT(self->threads[thread_index] == nullptr);

      BLThread* thread = threads[i];
      self->threads[thread_index] = thread;

      if (++i >= n)
        break;
    }

    self->pooled_thread_bits.data[bw_index] = mask ^ bl::IntOps::all_ones<BLBitWord>();
  } while (i < n && ++bw_index < BLInternalThreadPool::PooledThreadBitArray::kFixedArraySize);

  // This shouldn't happen. What is acquired must be released. If more threads are released than acquired it means
  // the API was used wrongly. Not sure we want to recover.
  BL_ASSERT(i == n);

  self->pooled_thread_count += n;
  self->acquired_thread_count -= n;
}

static uint32_t bl_thread_pool_acquire_threads_internal(BLInternalThreadPool* self, BLThread** threads, uint32_t n, uint32_t flags, BLResult* reason_out) noexcept {
  BLResult reason = BL_SUCCESS;
  uint32_t n_acquired = 0;

  uint32_t pooled_thread_count = self->pooled_thread_count;
  uint32_t acquired_thread_count = self->acquired_thread_count;

  if (n > pooled_thread_count) {
    uint32_t create_thread_count = n - pooled_thread_count;
    uint32_t remaining_thread_count = self->max_thread_count - (acquired_thread_count + pooled_thread_count);

    if (create_thread_count > remaining_thread_count) {
      if (flags & BL_THREAD_POOL_ACQUIRE_FLAG_ALL_OR_NOTHING) {
        *reason_out = BL_ERROR_THREAD_POOL_EXHAUSTED;
        return 0;
      }
      create_thread_count = remaining_thread_count;
    }

    while (n_acquired < create_thread_count) {
      // We must increase the reference here as it must be accounted if it's going to start.
      bl_atomic_fetch_add_relaxed(&self->internal_ref_count);

      reason = bl_thread_create(&threads[n_acquired], &self->thread_attributes, bl_thread_pool_thread_exit_func, self);

      if (reason != BL_SUCCESS) {
        size_t prev = bl_atomic_fetch_sub_strong(&self->internal_ref_count);
        if (BL_UNLIKELY(prev == 0)) {
          bl_runtime_failure("The thread pool has been dereferenced during acquiring threads\n");
        }

        if (flags & BL_THREAD_POOL_ACQUIRE_FLAG_ALL_OR_NOTHING) {
          self->acquired_thread_count += n_acquired;
          bl_atomic_fetch_add_strong(&self->created_thread_count, n_acquired);

          bl_thread_pool_release_threads_internal(self, threads, n_acquired);
          *reason_out = reason;
          return 0;
        }

        // Don't try again... The `reason` will be propagated to the caller.
        break;
      }

      n_acquired++;
    }

    bl_atomic_fetch_add_strong(&self->created_thread_count, n_acquired);
  }

  uint32_t bw_index = 0;
  uint32_t nAcqPrev = n_acquired;

  while (n_acquired < n) {
    BLBitWord mask = self->pooled_thread_bits.data[bw_index];
    bl::BitWordIterator<BLBitWord> it(mask);

    while (it.has_next()) {
      uint32_t bit = it.next();
      mask ^= bl::IntOps::lsb_bit_at<BLBitWord>(bit);

      uint32_t thread_index = bw_index * bl::IntOps::bit_size_of<BLBitWord>() + bit;
      BLThread* thread = self->threads[thread_index];

      BL_ASSERT(thread != nullptr);
      self->threads[thread_index] = nullptr;

      threads[n_acquired] = thread;
      if (++n_acquired == n)
        break;
    }

    self->pooled_thread_bits.data[bw_index] = mask;
    if (++bw_index >= BLInternalThreadPool::PooledThreadBitArray::kFixedArraySize)
      break;
  }

  self->pooled_thread_count -= n_acquired - nAcqPrev;
  self->acquired_thread_count += n_acquired;

  *reason_out = reason;
  return n_acquired;
}

static uint32_t BL_CDECL bl_thread_pool_acquire_threads(BLThreadPool* self_, BLThread** threads, uint32_t n, uint32_t flags, BLResult* reason) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  return bl_thread_pool_acquire_threads_internal(self, threads, n, flags, reason);
}

static void BL_CDECL bl_thread_pool_release_threads(BLThreadPool* self_, BLThread** threads, uint32_t n) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  return bl_thread_pool_release_threads_internal(self, threads, n);
}

// ThreadPool - Global
// ===================

BLThreadPool* bl_thread_pool_global() noexcept { return &bl_global_thread_pool; }

// ThreadPool - Runtime Registration
// =================================

static void BL_CDECL bl_thread_pool_on_shutdown(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);
  bl_thread_pool_release(&bl_global_thread_pool);
}

static void BL_CDECL bl_thread_pool_rt_cleanup(BLRuntimeContext* rt, BLRuntimeCleanupFlags cleanup_flags) noexcept {
  bl_unused(rt);
  if (cleanup_flags & BL_RUNTIME_CLEANUP_THREAD_POOL)
    bl_global_thread_pool->cleanup();
}

void bl_thread_pool_rt_init(BLRuntimeContext* rt) noexcept {
  // ThreadPool virtual table.
  bl_thread_pool_virt.add_ref = bl_thread_pool_add_ref;
  bl_thread_pool_virt.release = bl_thread_pool_release;
  bl_thread_pool_virt.max_thread_count = bl_thread_pool_max_thread_count;
  bl_thread_pool_virt.pooled_thread_count = bl_thread_pool_pooled_thread_count;
  bl_thread_pool_virt.set_thread_attributes = bl_thread_pool_set_thread_attributes;
  bl_thread_pool_virt.cleanup = bl_thread_pool_cleanup;
  bl_thread_pool_virt.acquire_threads = bl_thread_pool_acquire_threads;
  bl_thread_pool_virt.release_threads = bl_thread_pool_release_threads;

  // ThreadPool built-in global instance.
  BLThreadAttributes attrs {};
  attrs.stack_size = rt->system_info.thread_stack_size;

  bl_global_thread_pool.init();
  bl_global_thread_pool->set_thread_attributes(attrs);

  rt->shutdown_handlers.add(bl_thread_pool_on_shutdown);
  rt->cleanup_handlers.add(bl_thread_pool_rt_cleanup);
}
