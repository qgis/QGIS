// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_REFERENCE_PIXELBUFFERPTR_P_H_INCLUDED
#define BLEND2D_PIPELINE_REFERENCE_PIXELBUFFERPTR_P_H_INCLUDED

#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/support/intops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_reference
//! \{

namespace bl::Pipeline::Reference {
namespace {

template<uint32_t BytesPerPixel>
class PixelBufferPtr {
public:
  enum : uint32_t { kBytesPerPixel = BytesPerPixel };

  uint8_t* _ptr = nullptr;
  intptr_t _stride = 0;

  BL_INLINE PixelBufferPtr() noexcept = default;
  BL_INLINE PixelBufferPtr(uint8_t* ptr, intptr_t stride) noexcept
    : _ptr(ptr),
      _stride(stride) {}

  BL_INLINE uint8_t* ptr() const noexcept { return _ptr; }
  BL_INLINE intptr_t stride() const noexcept { return _stride; }

  BL_INLINE void set_ptr(uint8_t* ptr) noexcept { _ptr = ptr; }
  BL_INLINE void set_stride(intptr_t stride) noexcept { _stride = stride; }

  template<typename T>
  BL_INLINE void init_rect(const T& x, const T& y, const T& width) noexcept {
    advance_y(y);
    advance_x(x);

    // Each AdvanceY would advance to the beginning of the next scanline from the
    // end of the current scanline. Since this is a FillRect operation we assume
    // that we would never skip a scanline, which makes this trick possible.
    _stride -= intptr_t(size_t(blAsUInt(width)) * kBytesPerPixel);
  }

  template<typename T>
  BL_INLINE void init_generic(const T& y) noexcept {
    advance_y(y);
  }

  template<typename T>
  BL_INLINE void advance_x(const T& x) noexcept { _ptr += size_t(IntOps::asStdUInt(x)) * kBytesPerPixel; }
  template<typename T>
  BL_INLINE void advance_y(const T& y) noexcept { _ptr += intptr_t(size_t(IntOps::asStdUInt(y))) * _stride; }

  template<typename T>
  BL_INLINE void deadvanceX(const T& x) noexcept { _ptr -= size_t(IntOps::asStdUInt(x)) * kBytesPerPixel; }

  BL_INLINE void advance_x() noexcept { advance_x(1); }
  BL_INLINE void advance_y() noexcept { advance_y(1); }
};

} // {anonymous}
} // {bl::Pipeline::Reference}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_REFERENCE_PIXELBUFFERPTR_P_H_INCLUDED
