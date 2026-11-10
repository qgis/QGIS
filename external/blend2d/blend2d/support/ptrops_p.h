// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_PTROPS_P_H_INCLUDED
#define BLEND2D_SUPPORT_PTROPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace PtrOps {
namespace {

//! \name Pointer Arithmetic
//! \{

template<typename T, typename Offset>
[[nodiscard]]
static BL_INLINE_NODEBUG T* offset(T* ptr, Offset offset) noexcept { return (T*)((uintptr_t)(ptr) + (uintptr_t)(intptr_t)offset); }

template<typename T, typename P, typename Offset>
[[nodiscard]]
static BL_INLINE_NODEBUG T* offset(P* ptr, Offset offset) noexcept { return (T*)((uintptr_t)(ptr) + (uintptr_t)(intptr_t)offset); }

template<typename T, typename Offset>
[[nodiscard]]
static BL_INLINE_NODEBUG T* deoffset(T* ptr, Offset offset) noexcept { return (T*)((uintptr_t)(ptr) - (uintptr_t)(intptr_t)offset); }

template<typename T, typename P, typename Offset>
[[nodiscard]]
static BL_INLINE_NODEBUG T* deoffset(P* ptr, Offset offset) noexcept { return (T*)((uintptr_t)(ptr) - (uintptr_t)(intptr_t)offset); }

template<typename T, typename U>
[[nodiscard]]
static BL_INLINE_NODEBUG bool both_aligned(const T* ptr1, const U* ptr2, size_t alignment) noexcept {
  return (((uintptr_t)(ptr1) | (uintptr_t)(ptr2)) % uintptr_t(alignment)) == 0;
}

template<typename T, typename U>
[[nodiscard]]
static BL_INLINE_NODEBUG bool have_equal_alignment(const T* ptr1, const U* ptr2, size_t alignment) noexcept {
  return (((uintptr_t)(ptr1) ^ (uintptr_t)(ptr2)) % uintptr_t(alignment)) == 0;
}

[[nodiscard]]
static BL_INLINE_NODEBUG size_t byte_offset(const void* base, const void* ptr) noexcept {
  // The `byte_offset` function expects `ptr` to always be greater than base - the result must be
  // zero/positive as it's represented by an unsigned type.
  BL_ASSERT(static_cast<const uint8_t*>(ptr) >= static_cast<const uint8_t*>(base));

  return (size_t)(static_cast<const uint8_t*>(ptr) - static_cast<const uint8_t*>(base));
}

[[nodiscard]]
static BL_INLINE_NODEBUG size_t bytes_until(const void* ptr, const void* end) noexcept {
  // The `bytes_until` function requires `end` to always be greater than or equal to `ptr` as it
  // describes the end of a buffer where data is stored or read from.
  BL_ASSERT(static_cast<const uint8_t*>(ptr) <= static_cast<const uint8_t*>(end));

  return (size_t)(static_cast<const uint8_t*>(end) - static_cast<const uint8_t*>(ptr));
}

//! \}

} // {anonymous}
} // {PtrOps}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_PTROPS_P_H_INCLUDED
