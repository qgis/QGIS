// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_THREADING_THREADINGUTILS_P_H_INCLUDED
#define BLEND2D_THREADING_THREADINGUTILS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

#if !defined(_WIN32)
  #include <sys/time.h>
#endif

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace BLThreadingUtils {

#if !defined(_WIN32)
static void get_abs_time_for_wait_condition(struct timespec& out, uint64_t microseconds) noexcept {
  struct timeval now;
  gettimeofday(&now, nullptr);

  int64_t sec = int64_t(now.tv_sec) + int64_t(microseconds / 1000000u);
  uint64_t nsec = (uint64_t(now.tv_usec) + uint64_t(microseconds % 1000000u)) * 1000u;

  sec += int64_t(nsec / 1000000000u);
  nsec %= 1000000000u;

  // To avoid implicit conversion warnings in case the timeval is using 32-bit types (deprecated).
  using SecType = decltype(out.tv_sec);
  using NSecType = decltype(out.tv_nsec);

  out.tv_sec = SecType(sec);
  out.tv_nsec = NSecType(nsec);
}
#endif

} // {BLThreadingUtils}

//! \}
//! \endcond

#endif // BLEND2D_THREADING_THREADINGUTILS_P_H_INCLUDED
