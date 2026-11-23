// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_P_H_INCLUDED
#define BLEND2D_SUPPORT_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Memory Buffer
//! \{

//! Memory buffer.
//!
//! Memory buffer is a helper class which holds pointer to an allocated memory block, which will be released
//! automatically by `ScopedBuffer` destructor or  `reset()` call.
class ScopedBuffer {
public:
  BL_NONCOPYABLE(ScopedBuffer)

  void* _mem;
  void* _buf;
  size_t _capacity;

  BL_INLINE ScopedBuffer() noexcept
    : _mem(nullptr),
      _buf(nullptr),
      _capacity(0) {}

  BL_INLINE ~ScopedBuffer() noexcept {
    _reset();
  }

protected:
  BL_INLINE ScopedBuffer(void* mem, void* buf, size_t capacity) noexcept
    : _mem(mem),
      _buf(buf),
      _capacity(capacity) {}

public:
  [[nodiscard]]
  BL_INLINE void* get() const noexcept { return _mem; }

  [[nodiscard]]
  BL_INLINE size_t capacity() const noexcept { return _capacity; }

  [[nodiscard]]
  BL_INLINE void* alloc(size_t size) noexcept {
    if (size <= _capacity)
      return _mem;

    if (_mem != _buf)
      free(_mem);

    _mem = malloc(size);
    _capacity = size;

    return _mem;
  }

  [[nodiscard]]
  BL_NOINLINE void* alloc_zeroed(size_t size) noexcept {
    void* p = alloc(size);
    if (p)
      memset(p, 0, size);
    return p;
  }

  BL_INLINE void _reset() noexcept {
    if (_mem != _buf)
      free(_mem);
  }

  BL_INLINE void reset() noexcept {
    _reset();

    _mem = nullptr;
    _capacity = 0;
  }
};

//! Memory buffer (temporary).
//!
//! This template is for fast routines that need to use memory allocated on the stack, but the memory requirement
//! is not known at compile time. The number of bytes allocated on the stack is described by `N` parameter.
template<size_t N>
class ScopedBufferTmp : public ScopedBuffer {
public:
  BL_NONCOPYABLE(ScopedBufferTmp)

  uint8_t _storage[N];

  BL_INLINE ScopedBufferTmp() noexcept
    : ScopedBuffer(_storage, _storage, N) {}

  BL_INLINE ~ScopedBufferTmp() noexcept {}

  using ScopedBuffer::alloc;

  BL_INLINE void reset() noexcept {
    _reset();
    _mem = _buf;
    _capacity = N;
  }
};

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_P_H_INCLUDED
