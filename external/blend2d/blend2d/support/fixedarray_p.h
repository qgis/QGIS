// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_FIXEDARRAY_P_H_INCLUDED
#define BLEND2D_SUPPORT_FIXEDARRAY_P_H_INCLUDED

#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/span_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! A fixed array that cannot grow beyond `N`.
template<typename T, size_t N>
class FixedArray {
public:
  //! \name Constants
  //! \{

  enum : size_t { kCapacity = N };

  //! \}

  //! \name Members
  //! \{

  T _data[kCapacity];
  size_t _size;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE FixedArray() noexcept
    : _size(0) {}

  BL_INLINE FixedArray(const FixedArray& other) noexcept { assign(other.data(), other.size()); }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE FixedArray& operator=(const FixedArray& other) noexcept {
    assign(other.data(), other.size());
    return *this;
  }

  BL_INLINE T& operator[](size_t index) noexcept {
    BL_ASSERT(index < _size);
    return _data[index];
  }

  BL_INLINE const T& operator[](size_t index) const noexcept {
    BL_ASSERT(index < _size);
    return _data[index];
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE bool is_empty() const noexcept { return _size == 0; }
  BL_INLINE size_t size() const noexcept { return _size; }
  BL_INLINE size_t capacity() const noexcept { return kCapacity; }

  BL_INLINE T* data() noexcept { return _data; }
  BL_INLINE const T* data() const noexcept { return _data; }

  BL_INLINE T& front() noexcept { return operator[](0); }
  BL_INLINE const T& front() const noexcept { return operator[](0); }

  BL_INLINE T& back() noexcept { return operator[](_size - 1); }
  BL_INLINE const T& back() const noexcept { return operator[](_size - 1); }

  //! \}

  //! \name Data Manipulation
  //! \{

  BL_INLINE void clear() noexcept { _size = 0; }

  BL_INLINE void assign(const T* data, size_t size) noexcept {
    BL_ASSERT(size <= kCapacity);

    MemOps::copy_forward_inline_t(_data, data, size);
    _size = size;
  }

  BL_INLINE void append(const T& item) noexcept {
    BL_ASSERT(_size != kCapacity);

    _data[_size] = item;
    _size++;
  }

  template<typename Condition>
  BL_INLINE void append_if(const T& item, const Condition& condition) noexcept {
    BL_ASSERT(_size != kCapacity);

    _data[_size] = item;
    _size += size_t(condition);
  }

  BL_INLINE void prepend(const T& item) noexcept {
    BL_ASSERT(_size != kCapacity);

    MemOps::copy_backward_inline_t(_data + 1, _data, _size);
    _data[0] = item;
    _size++;
  }

  BL_INLINE void insert(size_t index, const T& item) noexcept {
    BL_ASSERT(index <= _size);
    BL_ASSERT(_size != kCapacity);

    MemOps::copy_backward_inline_t(_data + index + 1, _data + index, (_size - index));
    _data[index] = item;
    _size++;
  }

  BL_INLINE void _set_size(size_t size) noexcept {
    BL_ASSERT(size <= kCapacity);
    _size = size;
  }

  BL_INLINE void _increment_size(size_t n) noexcept {
    BL_ASSERT(n <= kCapacity - _size);
    _size += n;
  }

  //! \}

  //! \name Span
  //! \{

  BL_INLINE Span<T> as_span() const noexcept {
    return Span<T>(_data, _size);
  }

  BL_INLINE Span<std::add_const_t<T>> as_cspan() const noexcept {
    return Span<std::add_const_t<T>>(_data, _size);
  }

  //! \}

  //! \name Iterator Compatibility
  //! \{

  BL_INLINE T* begin() noexcept { return _data; }
  BL_INLINE const T* begin() const noexcept { return _data; }

  BL_INLINE T* end() noexcept { return _data + _size; }
  BL_INLINE const T* end() const noexcept { return _data + _size; }

  BL_INLINE const T* cbegin() const noexcept { return _data; }
  BL_INLINE const T* cend() const noexcept { return _data + _size; }

  //! \}
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_FIXEDARRAY_P_H_INCLUDED
