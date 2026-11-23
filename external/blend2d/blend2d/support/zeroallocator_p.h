// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_ZEROALLOCATOR_P_H_INCLUDED
#define BLEND2D_SUPPORT_ZEROALLOCATOR_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

BL_HIDDEN void* bl_zero_allocator_alloc(size_t size, size_t* allocated_size) noexcept;
BL_HIDDEN void* bl_zero_allocator_resize(void* prev_ptr, size_t prev_size, size_t size, size_t* allocated_size) noexcept;
BL_HIDDEN void  bl_zero_allocator_release(void* ptr, size_t size) noexcept;

namespace bl {

//! Memory buffer that is initially zeroed and that must be zeroed upon release.
class ZeroBuffer {
public:
  BL_NONCOPYABLE(ZeroBuffer)

  //! \name Members
  //! \{

  //! Zero allocated data.
  uint8_t* data = nullptr;
  //! Size of the buffer.
  size_t size = 0;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ZeroBuffer() noexcept = default;

  BL_INLINE ZeroBuffer(ZeroBuffer&& other) noexcept
    : data(other.data),
      size(other.size) {
    other.data = nullptr;
    other.size = 0;
  }

  BL_INLINE ~ZeroBuffer() noexcept {
    if (data)
      bl_zero_allocator_release(data, size);
  }

  //! \}

  //! \name Allocation
  //! \{

  [[nodiscard]]
  BL_INLINE BLResult ensure(size_t minimum_size) noexcept {
    if (minimum_size <= size)
      return BL_SUCCESS;

    data = static_cast<uint8_t*>(bl_zero_allocator_resize(data, size, minimum_size, &size));
    return data ? BL_SUCCESS : bl_make_error(BL_ERROR_OUT_OF_MEMORY);
  }

  BL_INLINE void release() noexcept {
    if (data) {
      bl_zero_allocator_release(data, size);
      data = nullptr;
      size = 0;
    }
  }

  //! \}
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_ZEROALLOCATOR_P_H_INCLUDED
