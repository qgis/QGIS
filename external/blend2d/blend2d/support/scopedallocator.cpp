// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/scopedallocator_p.h>

namespace bl {

// bl::ScopedAllocator - Alloc
// ===========================

void* ScopedAllocator::alloc(size_t size, size_t alignment) noexcept {
  // First try to allocate from the local memory pool.
  uint8_t* p = IntOps::align_up(pool_ptr, alignment);
  size_t remain = size_t(IntOps::usub_saturate((uintptr_t)pool_end, (uintptr_t)p));

  if (remain >= size) {
    pool_ptr = p + size;
    return p;
  }

  // Bail to malloc if local pool was either not provided or didn't have the required capacity.
  size_t size_with_overhead = size + sizeof(Link) + (alignment - 1);
  p = static_cast<uint8_t*>(malloc(size_with_overhead));

  if (p == nullptr)
    return nullptr;

  reinterpret_cast<Link*>(p)->next = links;
  links = reinterpret_cast<Link*>(p);

  return IntOps::align_up(p + sizeof(Link), alignment);
}

// ScopedAllocator - Reset
// =======================

void ScopedAllocator::reset() noexcept {
  Link* link = links;
  while (link) {
    Link* next = link->next;
    free(link);
    link = next;
  }

  links = nullptr;
  pool_ptr = pool_mem;
}

} // {bl}
