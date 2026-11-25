// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_SUPPORT_SPAN_H_INCLUDED
#define ASMJIT_SUPPORT_SPAN_H_INCLUDED

#include <asmjit/core/globals.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

//! Forward iterator to avoid including `<iterator>` header for iteration over arrays, specialized for AsmJit use.
template<typename T>
class SpanForwardIterator {
public:
  //! \name Members
  //! \{

  T* ptr {};

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_CONSTEXPR bool operator==(const T* other) const noexcept { return ptr == other; }
  ASMJIT_INLINE_CONSTEXPR bool operator==(const SpanForwardIterator& other) const noexcept { return ptr == other.ptr; }

  ASMJIT_INLINE_CONSTEXPR bool operator!=(const T* other) const noexcept { return ptr != other; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const SpanForwardIterator& other) const noexcept { return ptr != other.ptr; }

  ASMJIT_INLINE_CONSTEXPR SpanForwardIterator& operator++() noexcept { ptr++; return *this; }
  ASMJIT_INLINE_CONSTEXPR SpanForwardIterator operator++(int) noexcept { SpanForwardIterator prev(*this); ptr++; return prev; }

  ASMJIT_INLINE_CONSTEXPR T& operator*() const noexcept { return *ptr; }
  ASMJIT_INLINE_CONSTEXPR T* operator->() const noexcept { return ptr; }

  ASMJIT_INLINE_CONSTEXPR operator T*() const noexcept { return ptr; }

  //! \}
};

//! Reverse iterator to avoid including `<iterator>` header for iteration over arrays, specialized for AsmJit use.
template<typename T>
class SpanReverseIterator {
public:
  //! \name Members
  //! \{

  T* ptr {};

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_CONSTEXPR bool operator==(const T* other) const noexcept { return ptr == other; }
  ASMJIT_INLINE_CONSTEXPR bool operator==(const SpanReverseIterator& other) const noexcept { return ptr == other.ptr; }

  ASMJIT_INLINE_CONSTEXPR bool operator!=(const T* other) const noexcept { return ptr != other; }
  ASMJIT_INLINE_CONSTEXPR bool operator!=(const SpanReverseIterator& other) const noexcept { return ptr != other.ptr; }

  ASMJIT_INLINE_CONSTEXPR SpanReverseIterator& operator++() noexcept { ptr--; return *this; }
  ASMJIT_INLINE_CONSTEXPR SpanReverseIterator operator++(int) noexcept { SpanReverseIterator prev(*this); ptr--; return prev; }

  ASMJIT_INLINE_CONSTEXPR T& operator*() const noexcept { return ptr[-1]; }
  ASMJIT_INLINE_CONSTEXPR T* operator->() const noexcept { return &ptr[-1]; }

  ASMJIT_INLINE_CONSTEXPR operator T*() const noexcept { return &ptr[-1]; }

  //! \}
};

template<typename T>
class SpanForwardIteratorAdaptor {
public:
  //! \name Types
  //! \{

  using iterator = SpanForwardIterator<T>;

  //! \}

  //! \name Members
  //! \{

  T* _begin {};
  T* _end {};

  //! \}

  //! \name C++ Compatibility
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator begin() const noexcept { return iterator{_begin}; };

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator end() const noexcept { return iterator{_end}; };

  //! \}
};

template<typename T>
class SpanReverseIteratorAdaptor {
public:
  //! \name Types
  //! \{

  using iterator = SpanReverseIterator<T>;

  //! \}

  //! \name Members
  //! \{

  T* _begin {};
  T* _end {};

  //! \}

  //! \name C++ Compatibility
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator begin() const noexcept { return iterator{_end}; };

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR iterator end() const noexcept { return iterator{_begin}; };

  //! \}
};

template<typename T>
struct Span {
  //! \name Members
  //! \{

  T* _data {};
  size_t _size {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_CONSTEXPR Span() noexcept = default;

  template<typename Other>
  ASMJIT_INLINE_CONSTEXPR Span(Span<Other> other) noexcept
    : _data(other.data()),
      _size(other.size()) {}

  ASMJIT_INLINE_CONSTEXPR Span(T* data, size_t size) noexcept
    : _data(data),
      _size(size) {}

  template<size_t N>
  static ASMJIT_INLINE_CONSTEXPR Span<T> from_array(T(&array)[N]) noexcept {
    return Span<T>(array, N);
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  template<typename OtherT>
  ASMJIT_INLINE_CONSTEXPR T& operator=(Span<OtherT> other) noexcept {
    _data = other._data;
    _size = other._size;
    return *this;
  }

  template<typename OtherT>
  [[nodiscard]]
  ASMJIT_INLINE bool operator==(const Span<OtherT>& other) noexcept { return equals(other); }

  template<typename OtherT>
  [[nodiscard]]
  ASMJIT_INLINE bool operator!=(const Span<OtherT>& other) noexcept { return !equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE T& operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }

  [[nodiscard]]
  ASMJIT_INLINE const T& operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }

  //! \}

  //! \name Common Functionality
  //! \{

  template<typename OtherT>
  [[nodiscard]]
  ASMJIT_INLINE bool equals(Span<OtherT> other) const noexcept {
    size_t size = _size;

    if (size != other.size()) {
      return false;
    }

    for (size_t i = 0u; i < size; i++) {
      if (_data[i] != other._data[i]) {
        return false;
      }
    }

    return true;
  }

  ASMJIT_INLINE void swap(Span<T>& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
  }

  //! \}

  //! \name Data Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T* data() noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* data() const noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* cdata() const noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR size_t size() const noexcept { return _size; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_empty() const noexcept { return _size == 0u; }

  //! Returns a reference to the first element of the span.
  //!
  //! \remarks The span must have at least one element. Attempting to use `first()` on empty span will trigger
  //! an assertion failure in debug builds.
  [[nodiscard]]
  ASMJIT_INLINE T& first() noexcept { return operator[](0u); }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE const T& first() const noexcept { return operator[](0u); }

  //! Returns a reference to the last element of the span.
  //!
  //! \remarks The span must have at least one element. Attempting to use `last()` on empty span will trigger
  //! an assertion failure in debug builds.
  [[nodiscard]]
  ASMJIT_INLINE T& last() noexcept { return operator[](_size - 1u); }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE const T& last() const noexcept { return operator[](_size - 1u); }

  //! \}

  //! \name Utility Functions
  //! \{

  //! Tests whether the vector contains `value`.
  template<typename Value>
  [[nodiscard]]
  ASMJIT_INLINE bool contains(Value&& value) const noexcept {
    size_t size = _size;

    for (size_t i = 0u; i < size; i++) {
      if (_data[i] == value) {
        return true;
      }
    }

    return false;
  }

  //! Returns the first index of the given `value` or `SIZE_MAX` if it wasn't found'.
  template<typename Value>
  [[nodiscard]]
  ASMJIT_INLINE size_t index_of(Value&& value) const noexcept {
    size_t size = _size;

    for (size_t i = 0u; i < size; i++) {
      if (_data[i] == value) {
        return i;
      }
    }

    return SIZE_MAX;
  }

  //! Returns the last index of the given `value` or `SIZE_MAX` if it wasn't found.
  template<typename Value>
  [[nodiscard]]
  ASMJIT_INLINE size_t last_index_of(Value&& value) const noexcept {
    size_t i = _size;

    while (--i != SIZE_MAX) {
      if (_data[i] == value) {
        break;
      }
    }

    return i;
  }

  //! \}

  //! \name Iteration
  //! \{

  ASMJIT_INLINE_NODEBUG T* begin() const noexcept { return _data; }
  ASMJIT_INLINE_NODEBUG T* end() const noexcept { return _data + _size; }

  ASMJIT_INLINE_NODEBUG SpanForwardIteratorAdaptor<T> iterate() const noexcept { return SpanForwardIteratorAdaptor<T>{begin(), end()}; }
  ASMJIT_INLINE_NODEBUG SpanReverseIteratorAdaptor<T> iterate_reverse() const noexcept { return SpanReverseIteratorAdaptor<T>{begin(), end()}; }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_SUPPORT_SPAN_H_INCLUDED
