// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_THREADING_CONDITIONVARIABLE_P_H_INCLUDED
#define BLEND2D_THREADING_CONDITIONVARIABLE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/threading/mutex_p.h>
#include <blend2d/threading/threadingutils_p.h>

#if !defined(_WIN32)
  #include <pthread.h>
  #include <sys/time.h>
#endif

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name Condition Variable
//! \{

class BLConditionVariable {
public:
  BL_NONCOPYABLE(BLConditionVariable)

#ifdef _WIN32
  CONDITION_VARIABLE handle;

  BL_INLINE BLConditionVariable() noexcept : handle(CONDITION_VARIABLE_INIT) {}
  BL_INLINE ~BLConditionVariable() noexcept {}

  BL_INLINE void signal() noexcept { WakeConditionVariable(&handle); }
  BL_INLINE void broadcast() noexcept { WakeAllConditionVariable(&handle); }

  BL_INLINE BLResult wait(BLMutex& mutex) noexcept {
    BOOL ret = SleepConditionVariableSRW(&handle, &mutex.handle, INFINITE, 0);
    return ret ? BL_SUCCESS : bl_make_error(BL_ERROR_INVALID_STATE);
  }

  BL_INLINE BLResult wait_for(BLMutex& mutex, uint64_t microseconds) noexcept {
    uint32_t milliseconds = uint32_t(bl_min<uint64_t>(microseconds / 1000u, INFINITE));
    BOOL ret = SleepConditionVariableSRW(&handle, &mutex.handle, milliseconds, 0);

    if (ret)
      return BL_SUCCESS;

    // We don't trace `BL_ERROR_TIMED_OUT` as it's not unexpected.
    return BL_ERROR_TIMED_OUT;
  }
#else
  pthread_cond_t handle;

  BL_INLINE BLConditionVariable() noexcept : handle(PTHREAD_COND_INITIALIZER) {}
  BL_INLINE ~BLConditionVariable() noexcept { pthread_cond_destroy(&handle); }

  BL_INLINE void signal() noexcept {
    int ret = pthread_cond_signal(&handle);
    BL_ASSERT(ret == 0);
    bl_unused(ret);
  }

  BL_INLINE void broadcast() noexcept {
    int ret = pthread_cond_broadcast(&handle);
    BL_ASSERT(ret == 0);
    bl_unused(ret);
  }

  BL_INLINE BLResult wait(BLMutex& mutex) noexcept {
    int ret = pthread_cond_wait(&handle, &mutex.handle);
    return ret == 0 ? BL_SUCCESS : bl_make_error(BL_ERROR_INVALID_STATE);
  }

  BL_INLINE BLResult wait_for(BLMutex& mutex, uint64_t microseconds) noexcept {
    struct timespec abs_time;
    BLThreadingUtils::get_abs_time_for_wait_condition(abs_time, microseconds);
    return wait_until(mutex, &abs_time);
  }

  BL_INLINE BLResult wait_until(BLMutex& mutex, const struct timespec* abs_time) noexcept {
    int ret = pthread_cond_timedwait(&handle, &mutex.handle, abs_time);
    if (ret == 0)
      return BL_SUCCESS;

    // We don't trace `BL_ERROR_TIMED_OUT` as it's not unexpected.
    return BL_ERROR_TIMED_OUT;
  }
#endif
};

//! \}

//! \}
//! \endcond

#endif // BLEND2D_THREADING_CONDITIONVARIABLE_P_H_INCLUDED
