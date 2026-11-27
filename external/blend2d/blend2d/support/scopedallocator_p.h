
// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_SCOPEDALLOCATOR_P_H_INCLUDED
#define BLEND2D_SUPPORT_SCOPEDALLOCATOR_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! A simple allocator that can be used to remember allocated memory so it can be then freed in one go. Typically
//! used in areas where some heap allocation is required and at the end of the work it will all be freed.
class ScopedAllocator {
public:
  BL_NONCOPYABLE(ScopedAllocator)

  struct Link { Link* next; };

  uint8_t* pool_ptr {};
  uint8_t* pool_mem {};
  uint8_t* pool_end {};
  Link* links {};

  BL_INLINE ScopedAllocator() noexcept = default;
  BL_INLINE ScopedAllocator(void* pool_mem, size_t pool_size) noexcept
    : pool_ptr(static_cast<uint8_t*>(pool_mem)),
      pool_mem(static_cast<uint8_t*>(pool_mem)),
      pool_end(static_cast<uint8_t*>(pool_mem) + pool_size),
      links(nullptr) {}
  BL_INLINE ~ScopedAllocator() noexcept { reset(); }

  void* alloc(size_t size, size_t alignment = 1) noexcept;
  void reset() noexcept;
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_SCOPEDALLOCATOR_P_H_INCLUDED
