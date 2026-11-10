// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_SUPPORT_ARENABITSET_P_H_INCLUDED
#define ASMJIT_SUPPORT_ARENABITSET_P_H_INCLUDED

#include <asmjit/support/arena.h>
#include <asmjit/support/span.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_support
//! \{

using BitWord = Support::BitWord;

namespace BitOps {
namespace {

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG size_t size_in_bits(const Span<T>& span) noexcept { return span.size() * Support::bit_size_of<T>; }

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG size_t size_in_words(size_t num_bits) noexcept { return (num_bits + Support::bit_size_of<T> - 1u) / Support::bit_size_of<T>; }

template<typename T, typename Index>
[[nodiscard]]
ASMJIT_INLINE bool bit_at(const Span<T>& span, const Index& index) noexcept {
  size_t i = Support::as_basic_uint(index);
  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  return bool((span[word_index] >> bit_index) & 0x1u);
}

template<typename T, typename Index>
ASMJIT_INLINE void set_bit(const Span<T>& span, const Index& index, bool value) noexcept {
  size_t i = Support::as_basic_uint(index);
  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T and_mask = T(~(T(1u) << bit_index));
  T bit_mask = T(T(value) << bit_index);

  T& bit_word = span[word_index];
  bit_word = T((bit_word & and_mask) | bit_mask);
}

template<typename T, typename Index>
ASMJIT_INLINE void clear_bit(const Span<T>& span, const Index& index) noexcept {
  size_t i = Support::as_basic_uint(index);

  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T and_mask = T(~(T(1u) << bit_index));

  T& bit_word = span[word_index];
  bit_word = T(bit_word & and_mask);
}

template<typename T, typename Index>
ASMJIT_INLINE void or_bit(const Span<T>& span, const Index& index, bool value) noexcept {
  size_t i = Support::as_basic_uint(index);

  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T bit_mask = T(T(value) << bit_index);

  T& bit_word = span[word_index];
  bit_word = T(bit_word | bit_mask);
}

template<typename T, typename Index>
ASMJIT_INLINE void xor_bit(const Span<T>& span, const Index& index, bool value) noexcept {
  size_t i = Support::as_basic_uint(index);
  size_t word_index = i / Support::bit_size_of<T>;
  size_t bit_index = i % Support::bit_size_of<T>;

  T bit_mask = T(T(value) << bit_index);

  T& bit_word = span[word_index];
  bit_word = T(bit_word ^ bit_mask);
}

template<typename Op, typename T, typename... Args>
ASMJIT_INLINE void combine_spans(Span<T> dst, Args&&... args) noexcept {
  size_t size = dst.size();

  for (size_t i = 0u; i < size; i++) {
    dst[i] = Op::op(args[i]...);
  }
}

template<typename T, typename... Args>
ASMJIT_INLINE void or_(Span<T> dst, Args&&... args) noexcept {
  return combine_spans<Support::Or>(dst, std::forward<Args>(args)...);
}

} // {anonymous}
} // {BitOps}

//! Arena-allocated bit vector.
class ArenaBitSet {
public:
  ASMJIT_NONCOPYABLE(ArenaBitSet)

  //! \name Members
  //! \{

  //! Bits.
  BitWord* _data {};
  //! Size of the bit-vector (in bits).
  uint32_t _size {};
  //! Capacity of the bit-vector (in bits).
  uint32_t _capacity {};

  //! \}

  //! \cond INTERNAL
  //! \name Internal
  //! \{

  static ASMJIT_INLINE_NODEBUG size_t _words_per_bits(size_t num_bits) noexcept {
    return ((num_bits + Support::bit_size_of<BitWord> - 1u) / Support::bit_size_of<BitWord>);
  }

  static ASMJIT_INLINE_NODEBUG void _zero_bits(BitWord* dst, size_t bit_word_count) noexcept {
    for (size_t i = 0; i < bit_word_count; i++) {
      dst[i] = 0;
    }
  }

  static ASMJIT_INLINE_NODEBUG void _fill_bits(BitWord* dst, size_t bit_word_count) noexcept {
    for (size_t i = 0; i < bit_word_count; i++) {
      dst[i] = ~BitWord(0);
    }
  }

  static ASMJIT_INLINE_NODEBUG void _copy_bits(BitWord* dst, const BitWord* src, size_t bit_word_count) noexcept {
    for (size_t i = 0; i < bit_word_count; i++) {
      dst[i] = src[i];
    }
  }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ArenaBitSet() noexcept {}

  ASMJIT_INLINE_NODEBUG ArenaBitSet(ArenaBitSet&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const ArenaBitSet& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const ArenaBitSet& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the bit-vector is empty (has no bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }

  //! Returns the size of this bit-vector (in bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }

  //! Returns the capacity of this bit-vector (in bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t capacity() const noexcept { return _capacity; }

  //! Returns the size of the `BitWord[]` array in `BitWord` units.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size_in_bit_words() const noexcept { return _words_per_bits(_size); }

  //! Returns the capacity of the `BitWord[]` array in `BitWord` units.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t capacity_in_bit_words() const noexcept { return _words_per_bits(_capacity); }

  //! Returns bit-vector data as `BitWord[]`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BitWord* data() noexcept { return _data; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BitWord* data() const noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<BitWord> as_span() noexcept { return Span<BitWord>(_data, size_in_bit_words()); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const BitWord> as_span() const noexcept { return Span<BitWord>(_data, size_in_bit_words()); }

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE_NODEBUG void swap(ArenaBitSet& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  ASMJIT_INLINE_NODEBUG void clear() noexcept {
    _size = 0;
  }

  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _data = nullptr;
    _size = 0u;
    _capacity = 0u;
  }

  ASMJIT_INLINE_NODEBUG void truncate(uint32_t new_size) noexcept {
    _size = Support::min(_size, new_size);
    _clear_unused_bits();
  }

  template<typename Index>
  [[nodiscard]]
  inline bool bit_at(const Index& index) const noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    return Support::bit_vector_get_bit(_data, Support::as_basic_uint(index));
  }

  template<typename Index>
  inline void set_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bit_vector_set_bit(_data, Support::as_basic_uint(index), value);
  }

  template<typename Index>
  inline void add_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bit_vector_or_bit(_data, Support::as_basic_uint(index), value);
  }

  template<typename Index>
  inline void clear_bit(const Index& index) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bit_vector_set_bit(_data, Support::as_basic_uint(index), false);
  }

  template<typename Index>
  inline void xor_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(Support::as_basic_uint(index) < _size);
    Support::bit_vector_xor_bit(_data, Support::as_basic_uint(index), value);
  }

  ASMJIT_INLINE Error append(Arena& arena, bool value) noexcept {
    uint32_t index = _size;
    if (ASMJIT_UNLIKELY(index >= _capacity))
      return _append(arena, value);

    uint32_t idx = index / Support::bit_size_of<BitWord>;
    uint32_t bit = index % Support::bit_size_of<BitWord>;

    if (bit == 0)
      _data[idx] = BitWord(value) << bit;
    else
      _data[idx] |= BitWord(value) << bit;

    _size++;
    return Error::kOk;
  }

  Error copy_from(Arena& arena, const ArenaBitSet& other) noexcept;

  ASMJIT_INLINE void clear_all() noexcept {
    _zero_bits(_data, _words_per_bits(_size));
  }

  ASMJIT_INLINE void fill_all() noexcept {
    _fill_bits(_data, _words_per_bits(_size));
    _clear_unused_bits();
  }

  ASMJIT_INLINE void clear_bits(size_t start, size_t count) noexcept {
    ASMJIT_ASSERT(start <= size_t(_size));
    ASMJIT_ASSERT(size_t(_size) - start >= count);

    Support::bit_vector_clear(_data, start, count);
  }

  ASMJIT_INLINE void fill_bits(size_t start, size_t count) noexcept {
    ASMJIT_ASSERT(start <= size_t(_size));
    ASMJIT_ASSERT(size_t(_size) - start >= count);

    Support::bit_vector_fill(_data, start, count);
  }

  //! Performs a logical bitwise AND between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are set to zero.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void and_(const ArenaBitSet& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    size_t this_bit_word_count = size_in_bit_words();
    size_t other_bit_word_count = other.size_in_bit_words();
    size_t common_bit_word_count = Support::min(this_bit_word_count, other_bit_word_count);

    size_t i = 0;
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
  ASMJIT_INLINE void and_not(const ArenaBitSet& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    size_t common_bit_word_count = _words_per_bits(Support::min(_size, other._size));
    for (size_t i = 0; i < common_bit_word_count; i++) {
      dst[i] = dst[i] & ~src[i];
    }
  }

  //! Performs a logical bitwise OP between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are kept intact.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void or_(const ArenaBitSet& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    size_t common_bit_word_count = _words_per_bits(Support::min(_size, other._size));
    for (size_t i = 0; i < common_bit_word_count; i++) {
      dst[i] = dst[i] | src[i];
    }
    _clear_unused_bits();
  }

  ASMJIT_INLINE void _clear_unused_bits() noexcept {
    uint32_t idx = _size / Support::bit_size_of<BitWord>;
    uint32_t bit = _size % Support::bit_size_of<BitWord>;

    if (!bit) {
      return;
    }

    _data[idx] &= (BitWord(1) << bit) - 1u;
  }

  [[nodiscard]]
  ASMJIT_INLINE bool equals(const ArenaBitSet& other) const noexcept {
    if (_size != other._size) {
      return false;
    }

    const BitWord* a_data = _data;
    const BitWord* b_data = other._data;
    size_t bit_word_count = _words_per_bits(_size);

    for (size_t i = 0; i < bit_word_count; i++) {
      if (a_data[i] != b_data[i]) {
        return false;
      }
    }
    return true;
  }

  //! \}

  //! \name Memory Management
  //! \{

  inline void release(Arena& arena) noexcept {
    if (!_data) {
      return;
    }

    arena.free_reusable(_data, _capacity / 8u);
    reset();
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Error resize(Arena& arena, size_t new_size, bool new_bits_value = false) noexcept {
    return _resize(arena, new_size, new_size, new_bits_value);
  }

  Error _resize(Arena& arena, size_t new_size, size_t ideal_capacity, bool new_bits_value) noexcept;
  Error _append(Arena& arena, bool value) noexcept;

  //! \}

  //! \name Iterators
  //! \{

  class ForEachBitSet : public Support::BitVectorIterator<BitWord> {
  public:
    inline explicit ForEachBitSet(const ArenaBitSet& bit_vector) noexcept
      : Support::BitVectorIterator<BitWord>(bit_vector.as_span()) {}
  };

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_SUPPORT_ARENABITSET_P_H_INCLUDED
