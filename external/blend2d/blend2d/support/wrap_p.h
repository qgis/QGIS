// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_WRAP_P_H_INCLUDED
#define BLEND2D_SUPPORT_WRAP_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Wrapper to control construction & destruction of `T`.
template<typename T>
struct alignas(alignof(T)) Wrap {
  //! Storage required to instantiate `T`.
  char _data[sizeof(T)];

  //! \name Init & Destroy
  //! \{

  //! Placement new constructor.
  template<typename... Args>
  BL_INLINE T* init(Args&&... args) noexcept {
    T* instance = static_cast<T*>(static_cast<void*>(_data));
    bl_call_ctor(*instance, BLInternal::forward<Args>(args)...);
    return instance;
  }

  //! Placement delete destructor.
  BL_INLINE void destroy() noexcept {
    T* instance = static_cast<T*>(static_cast<void*>(_data));
    bl_call_dtor(*instance);
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE T* p() noexcept { return static_cast<T*>(static_cast<void*>(_data)); }
  BL_INLINE const T* p() const noexcept { return static_cast<const T*>(static_cast<const void*>(_data)); }

  BL_INLINE operator T&() noexcept { return *p(); }
  BL_INLINE operator const T&() const noexcept { return *p(); }

  BL_INLINE T& operator()() noexcept { return *p(); }
  BL_INLINE const T& operator()() const noexcept { return *p(); }

  BL_INLINE T* operator&() noexcept { return p(); }
  BL_INLINE const T* operator&() const noexcept { return p(); }

  BL_INLINE T* operator->() noexcept { return p(); }
  BL_INLINE T const* operator->() const noexcept { return p(); }

  //! \}
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_WRAP_P_H_INCLUDED
