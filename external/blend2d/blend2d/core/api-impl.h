// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_API_IMPL_H_INCLUDED
#define BLEND2D_API_IMPL_H_INCLUDED

#include <blend2d/core/api.h>
#include <blend2d/core/object.h>

#include <atomic>

//! \addtogroup bl_impl
//! \{

//! \name Atomic Operations
//!
//! Atomic operations are used extensively across Blend2D for reference counting and caching. You should always
//! use operations defined here as they implement all possible cases that Blend2D deals with in a correct way
//! (and if there is a bug it makes it fixable in a single place).
//!
//! \{

//! Atomically increments `n` to value `x` (relaxed semantics). The old value is returned.
template<typename T>
static BL_INLINE T bl_atomic_fetch_add_relaxed(T* x, T n = T(1)) noexcept {
  return ((std::atomic<T>*)x)->fetch_add(n, std::memory_order_relaxed);
}

//! Atomically increments `n` to value `x` (strong semantics). The old value is returned.
template<typename T>
static BL_INLINE T bl_atomic_fetch_add_strong(T* x, T n = T(1)) noexcept {
  return ((std::atomic<T>*)x)->fetch_add(n, std::memory_order_acq_rel);
}

//! Atomically decrements `n` from value `x` (relaxed semantics). The old value is returned.
template<typename T>
static BL_INLINE T bl_atomic_fetch_sub_relaxed(T* x, T n = T(1)) noexcept {
  return ((std::atomic<T>*)x)->fetch_sub(n, std::memory_order_relaxed);
}

//! Atomically decrements `n` from value `x` (strong semantics). The old value is returned.
template<typename T>
static BL_INLINE T bl_atomic_fetch_sub_strong(T* x, T n = T(1)) noexcept {
  return ((std::atomic<T>*)x)->fetch_sub(n, std::memory_order_acq_rel);
}

//! \}

//! \}

#endif // BLEND2D_API_IMPL_H_INCLUDED
