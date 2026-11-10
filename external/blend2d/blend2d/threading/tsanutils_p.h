// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_THREADING_TSANUTILS_P_H_INCLUDED
#define BLEND2D_THREADING_TSANUTILS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/threading/atomic_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace Threading {

//! Barrier implements a simple memory barrier that uses atomic operations.
//!
//! Use `release()` once before worker threads start and then `acquire()` by each worker thread.
struct Barrier {
  uint32_t barrier;

  BL_INLINE void release() const noexcept { bl_atomic_store_strong(&barrier, uint32_t(0)); }
  BL_INLINE void acquire() const noexcept { (void)bl_atomic_fetch_strong(&barrier); }
};

//! TSAN barrier implements \ref Barrier only when TSAN is running, otherwise it does nothing.
#if defined(BL_SANITIZE_THREAD)
struct TSanBarrier : public Barrier {};
#else
struct TSanBarrier {
  BL_INLINE_NODEBUG void release() const noexcept {}
  BL_INLINE_NODEBUG void acquire() const noexcept {}
};
#endif

} // {Threading}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_THREADING_TSANUTILS_P_H_INCLUDED
