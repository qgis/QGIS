// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_ARENABITARRAY_P_H_INCLUDED
#define BLEND2D_SUPPORT_ARENABITARRAY_P_H_INCLUDED

#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/bitops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Arena allocated bit array that uses `T` as an underlying bitword.
//!
//! T is usually wither `uint32_t` for compatibility with public API or `BLBitWord`, for maximum performance.
template<typename T>
class ArenaBitArray {
public:
  BL_NONCOPYABLE(ArenaBitArray)

  typedef ParametrizedBitOps<BitOrder::kMSB, T> Ops;

  //! \name Constants
  //! \{

  enum : uint32_t { kTSizeInBits = sizeof(T) * 8u };

  //! \}

  //! \name Members
  //! \{

  //! Bits.
  T* _data = nullptr;
  //! Size of the bit array (in bits).
  uint32_t _size = 0;
  //! Capacity of the bit array (in bits).
  uint32_t _capacity = 0;

  //! \}

  //! \cond INTERNAL
  //! \name Internal
  //! \{

  static BL_INLINE uint32_t _words_per_bits(uint32_t bit_count) noexcept {
    return ((bit_count + kTSizeInBits - 1) / kTSizeInBits);
  }

  static BL_INLINE void _zero_bits(T* dst, uint32_t word_count) noexcept {
    for (uint32_t i = 0; i < word_count; i++)
      dst[i] = 0;
  }

  static BL_INLINE void _fill_bits(T* dst, uint32_t word_count) noexcept {
    for (uint32_t i = 0; i < word_count; i++)
      dst[i] = ~T(0);
  }

  static BL_INLINE void _copy_bits(T* dst, const T* src, uint32_t word_count) noexcept {
    for (uint32_t i = 0; i < word_count; i++)
      dst[i] = src[i];
  }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ArenaBitArray() noexcept {}

  BL_INLINE ArenaBitArray(ArenaBitArray&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE bool operator==(const ArenaBitArray& other) const noexcept { return  eq(other); }
  BL_INLINE bool operator!=(const ArenaBitArray& other) const noexcept { return !eq(other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the bit array is empty (has no bits).
  BL_INLINE bool is_empty() const noexcept { return _size == 0; }
  //! Returns the size of this bit array (in bits).
  BL_INLINE uint32_t size() const noexcept { return _size; }
  //! Returns the capacity of this bit array (in bits).
  BL_INLINE uint32_t capacity() const noexcept { return _capacity; }

  //! Returns the size of the `T[]` array in `T` units.
  BL_INLINE uint32_t size_in_words() const noexcept { return _words_per_bits(_size); }
  //! Returns the capacity of the `T[]` array in `T` units.
  BL_INLINE uint32_t capacity_in_words() const noexcept { return _words_per_bits(_capacity); }

  //! REturns bit array data as `T[]`.
  BL_INLINE T* data() noexcept { return _data; }
  //! \overload
  BL_INLINE const T* data() const noexcept { return _data; }

  //! \}

  //! \name Utilities
  //! \{

  BL_INLINE void swap(ArenaBitArray& other) noexcept {
    BLInternal::swap(_data, other._data);
    BLInternal::swap(_size, other._size);
    BLInternal::swap(_capacity, other._capacity);
  }

  BL_INLINE void clear() noexcept {
    _size = 0;
  }

  BL_INLINE void reset() noexcept {
    _data = nullptr;
    _size = 0;
    _capacity = 0;
  }

  BL_INLINE void truncate(uint32_t new_size) noexcept {
    _size = bl_min(_size, new_size);
    _clear_unused_bits();
  }

  BL_INLINE bool bit_at(uint32_t index) const noexcept {
    BL_ASSERT(index < _size);
    return Ops::bit_array_test_bit(_data, index);
  }

  BL_INLINE void set_bit(uint32_t index) noexcept {
    BL_ASSERT(index < _size);
    Ops::bit_array_set_bit(_data, index);
  }

  BL_INLINE void fill_bits(uint32_t start, uint32_t count) noexcept {
    BL_ASSERT(start <= _size);
    BL_ASSERT(_size - start >= count);

    Ops::bit_array_fill(_data, start, count);
  }

  BL_INLINE void fill_all() noexcept {
    _fill_bits(_data, _words_per_bits(_size));
    _clear_unused_bits();
  }

  BL_INLINE void clear_bit(uint32_t index) noexcept {
    BL_ASSERT(index < _size);
    Ops::bit_array_clear_bit(_data, index);
  }

  BL_INLINE void clear_bits(uint32_t start, uint32_t count) noexcept {
    BL_ASSERT(start <= _size);
    BL_ASSERT(_size - start >= count);

    Ops::bit_array_clear(_data, start, count);
  }

  BL_INLINE void clear_all() noexcept {
    _zero_bits(_data, _words_per_bits(_size));
  }

  //! Performs a logical bitwise AND between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are set to zero.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  BL_INLINE void and_(const ArenaBitArray& other) noexcept {
    T* dst = _data;
    const T* src = other._data;

    uint32_t this_bit_word_count = size_in_words();
    uint32_t other_bit_word_count = other.size_in_words();
    uint32_t common_bit_word_count = bl_min(this_bit_word_count, other_bit_word_count);

    uint32_t i = 0;
    while (i < common_bit_word_count) {
      dst[i] = dst[i] & src[i];
      i++;
    }

    while (i < this_bit_word_count) {
      dst[i] = 0;
      i++;
    }
  }

  //! Performs a logical bitwise AND between bits specified in this array and negated bits in `other`. If `other`
  //! has less bits than `this` then all remaining bits are kept intact.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  BL_INLINE void and_not(const ArenaBitArray& other) noexcept {
    T* dst = _data;
    const T* src = other._data;

    uint32_t common_bit_word_count = _words_per_bits(bl_min(_size, other._size));
    for (uint32_t i = 0; i < common_bit_word_count; i++)
      dst[i] = dst[i] & ~src[i];
  }

  //! Performs a logical bitwise OP between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are kept intact.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  BL_INLINE void or_(const ArenaBitArray& other) noexcept {
    T* dst = _data;
    const T* src = other._data;

    uint32_t common_bit_word_count = _words_per_bits(bl_min(_size, other._size));
    for (uint32_t i = 0; i < common_bit_word_count; i++)
      dst[i] = dst[i] | src[i];
    _clear_unused_bits();
  }

  BL_INLINE void _clear_unused_bits() noexcept {
    uint32_t idx = _size / kTSizeInBits;
    uint32_t bit = _size % kTSizeInBits;

    if (!bit)
      return;
    _data[idx] &= Ops::non_zero_start_mask(bit);
  }

  BL_INLINE bool eq(const ArenaBitArray& other) const noexcept {
    if (_size != other._size)
      return false;

    const T* a_data = _data;
    const T* b_data = other._data;
    uint32_t count = size_in_words();

    for (uint32_t i = 0; i < count; i++)
      if (a_data[i] != b_data[i])
        return false;

    return true;
  }

  //! \}

  //! \name Memory Management
  //! \{

  BL_INLINE void release(ArenaAllocator* allocator) noexcept {
    if (!_data)
      return;

    allocator->release(_data, _capacity / 8u);
    reset();
  }

  BL_INLINE BLResult resize(ArenaAllocator* allocator, uint32_t new_size, bool new_bits_value = false) noexcept {
    return _resize(allocator, new_size, new_size, new_bits_value);
  }

  BL_NOINLINE BLResult _resize(ArenaAllocator* allocator, uint32_t new_size, uint32_t capacity_hint, bool new_bits_value) noexcept {
    BL_ASSERT(capacity_hint >= new_size);

    if (new_size <= _size) {
      // The size after the resize is lesser than or equal to the current size.
      _size = new_size;
      _clear_unused_bits();
      return BL_SUCCESS;
    }

    uint32_t old_size = _size;
    T* data = _data;

    if (new_size > _capacity) {
      // Realloc needed, calculate the minimum capacity (in bytes) required.
      uint32_t capacity_in_bits = IntOps::align_up<uint32_t>(capacity_hint, kTSizeInBits);

      if (BL_UNLIKELY(capacity_in_bits < new_size))
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

      // Normalize to bytes.
      uint32_t capacity_in_bytes = capacity_in_bits / 8;

      T* new_data = static_cast<T*>(allocator->alloc(capacity_in_bytes, alignof(T)));
      if (BL_UNLIKELY(!new_data))
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

      _copy_bits(new_data, data, _words_per_bits(old_size));

      if (data)
        allocator->release(data, capacity_in_bytes);
      data = new_data;

      _data = data;
      _capacity = capacity_in_bits;
    }

    // Start (of the old size) and end (of the new size) bits
    uint32_t idx = old_size / kTSizeInBits;
    uint32_t start_bit = old_size % kTSizeInBits;

    // Set new bits to either 0 or 1. The `pattern` is used to set multiple bits per word and contains either all
    // zeros or all ones.
    T pattern = IntOps::bool_as_mask<T>(new_bits_value);

    // First initialize the last word of the old size.
    if (start_bit)
      data[idx++] |= Ops::shift_to_end(pattern, start_bit);

    // Initialize all words after the last word of the old size.
    uint32_t end_index = _words_per_bits(new_size);
    while (idx < end_index)
      data[idx++] = pattern;

    _size = new_size;
    _clear_unused_bits();
    return BL_SUCCESS;
  }

  //! \}
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_ARENABITARRAY_P_H_INCLUDED
