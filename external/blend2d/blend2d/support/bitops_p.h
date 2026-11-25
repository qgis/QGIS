// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_BITOPS_P_H_INCLUDED
#define BLEND2D_SUPPORT_BITOPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/support/traits_p.h>
#include <blend2d/support/intops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Constants
//! \{

//! Defines an ordering of bits in a bit-word or bit-array.
enum class BitOrder : uint32_t {
  //! Least significant bit is considered first.
  kLSB = 0,
  //! Most significant bit is considered first.
  kMSB = 1,
  //! Bit ordering used in public interface.
  kPublic = kMSB,
  //! Bit ordering that is fastest on the given architecture, but used only internally.
  kPrivate = BL_TARGET_ARCH_X86 ? kLSB : kMSB
};

//! \}

namespace BitOperator {
namespace {

//! \name Bit Operators
//! \{

struct Assign {
  template<typename T>
  static BL_INLINE_NODEBUG T op(const T&, const T& b) noexcept { return b; }

  template<typename T>
  static BL_INLINE_NODEBUG T op_masked(const T& a, const T& b, T mask) noexcept { return (a & ~mask) | (b & mask); }
};

struct AssignNot {
  template<typename T>
  static BL_INLINE_NODEBUG T op(const T&, const T& b) noexcept { return ~b; }

  template<typename T>
  static BL_INLINE_NODEBUG T op_masked(const T& a, const T& b, T mask) noexcept { return (a & ~mask) | (~b & mask); }
};

struct And {
  template<typename T>
  static BL_INLINE_NODEBUG T op(const T& a, const T& b) noexcept { return a & b; }

  template<typename T>
  static BL_INLINE_NODEBUG T op_masked(const T& a, const T& b, T mask) noexcept { return a & (b | ~mask); }
};

struct AndNot {
  template<typename T>
  static BL_INLINE_NODEBUG T op(const T& a, const T& b) noexcept { return a & ~b; }

  template<typename T>
  static BL_INLINE_NODEBUG T op_masked(const T& a, const T& b, T mask) noexcept { return a & ~(b & mask); }
};

struct NotAnd {
  template<typename T>
  static BL_INLINE_NODEBUG T op(const T& a, const T& b) noexcept { return ~a & b; }

  template<typename T>
  static BL_INLINE_NODEBUG T op_masked(const T& a, const T& b, T mask) noexcept { return (a ^ mask) & (b | ~mask); }
};

struct Or {
  template<typename T>
  static BL_INLINE_NODEBUG T op(const T& a, const T& b) noexcept { return a | b; }

  template<typename T>
  static BL_INLINE_NODEBUG T op_masked(const T& a, const T& b, T mask) noexcept { return a | (b & mask); }
};

struct Xor {
  template<typename T>
  static BL_INLINE_NODEBUG T op(const T& a, const T& b) noexcept { return a ^ b; }

  template<typename T>
  static BL_INLINE_NODEBUG T op_masked(const T& a, const T& b, T mask) noexcept { return a ^ (b & mask); }
};

//! \}

} // {anonymous}
} // {BitOperator}

namespace {

//! \name Parametrized Bit Operators
//! \{

//! Parametrized bit operations.
//!
//! This class acts as a namespace and allows to parametrize how bits are stored in a BitWord. The reason for
//! parametrization is architecture constraints. X86 architecture prefers LSB ordering, because of the performance
//! of BSF and TZCNT instructions. Since TZCNT instruction is BSF with REP prefix compilers can savely emit TZCNT
//! instead of BSF, but it's not possible to emit LZCNT instead of BSR as LZCNT returns a different result (count
//! of zeros instead of first zero index).
//!
//! ARM and other architectures only implement LZCNT (count leading zeros) and counting trailing zeros means emitting
//! more instructions to workaround the missing instruction.
template<BitOrder kBO, typename T>
struct ParametrizedBitOps {
  using U = std::make_unsigned_t<T>;

  static constexpr BitOrder kBitOrder = kBO;
  static constexpr BitOrder kReverseBitOrder = BitOrder(uint32_t(kBO) ^ 1u);

  enum : uint32_t {
    kIsLSB = (kBO == BitOrder::kLSB),
    kIsMSB = (kBO == BitOrder::kMSB),

    kNumBits = IntOps::bit_size_of<T>(),
    kBitMask = kNumBits - 1
  };

  static BL_INLINE_CONSTEXPR T zero() noexcept { return T(0); }
  static BL_INLINE_CONSTEXPR T ones() noexcept { return IntOps::all_ones<T>(); }

  template<typename Index>
  static BL_INLINE_CONSTEXPR bool has_bit(const T& x, const Index& index) noexcept {
    return kIsLSB ? bool((x >> index) & 0x1) : bool((x >> (index ^ Index(kBitMask))) & 0x1);
  }

  template<typename Count>
  static BL_INLINE_CONSTEXPR T shift_to_start(const T& x, const Count& y) noexcept {
    return kIsLSB ? IntOps::shr(x, y) : IntOps::shl(x, y);
  }

  template<typename Count>
  static BL_INLINE_CONSTEXPR T shift_to_end(const T& x, const Count& y) noexcept {
    return kIsLSB ? IntOps::shl(x, y) : IntOps::shr(x, y);
  }

  template<typename Count>
  static BL_INLINE_CONSTEXPR T non_zero_start_mask(const Count& count = 1) noexcept {
    return kIsLSB ? IntOps::non_zero_lsb_mask<T>(count) : IntOps::non_zero_msb_mask<T>(count);
  }

  template<typename Index, typename Count>
  static BL_INLINE_CONSTEXPR T non_zero_start_mask(const Count& count, const Index& index) noexcept {
    return shift_to_end(non_zero_start_mask(count), index);
  }

  template<typename N>
  static BL_INLINE_CONSTEXPR T non_zero_end_mask(const N& n = 1) noexcept {
    return kIsLSB ? IntOps::non_zero_msb_mask<T>(n) : IntOps::non_zero_lsb_mask<T>(n);
  }

  template<typename Index, typename Count>
  static BL_INLINE_CONSTEXPR T non_zero_end_mask(const Count& count, const Index& index) noexcept {
    return shift_to_start(non_zero_end_mask(count), index);
  }

  template<typename Index>
  static BL_INLINE_CONSTEXPR T index_as_mask(const Index& index) noexcept {
    return kIsLSB ? IntOps::shl(T(1), index) : IntOps::shr(IntOps::non_zero_msb_mask<T>(), index);
  }

  template<typename Index, typename Value>
  static BL_INLINE_CONSTEXPR T index_as_mask(const Index& index, const Value& value) noexcept {
    return kIsLSB ? IntOps::shl(T(value), index) : IntOps::shr(T(value) << kBitMask, index);
  }

  static BL_INLINE_NODEBUG uint32_t count_zeros_from_start(const T& x) noexcept {
    return kIsLSB ? IntOps::ctz(x) : IntOps::clz(x);
  }

  static BL_INLINE_NODEBUG uint32_t count_zeros_from_end(const T& x) noexcept {
    return kIsLSB ? IntOps::clz(x) : IntOps::ctz(x);
  }

  static BL_INLINE int compare(const T& x, const T& y) noexcept {
    T xv = kIsLSB ? IntOps::bit_swap(x) : x;
    T yv = kIsLSB ? IntOps::bit_swap(y) : y;

    return int(xv > yv) - int(xv < yv);
  }

  static BL_INLINE bool bit_array_test_bit(const T* buf, size_t index) noexcept {
    size_t word_index = index / kNumBits;
    size_t bit_index = index % kNumBits;

    return (buf[word_index] & index_as_mask(bit_index)) != 0u;
  }

  static BL_INLINE void bit_array_set_bit(T* buf, size_t index) noexcept {
    size_t word_index = index / kNumBits;
    size_t bit_index = index % kNumBits;

    buf[word_index] |= index_as_mask(bit_index);
  }

  static BL_INLINE void bit_array_or_bit(T* buf, size_t index, bool value) noexcept {
    size_t word_index = index / kNumBits;
    size_t bit_index = index % kNumBits;

    buf[word_index] |= index_as_mask(bit_index, value);
  }

  static BL_INLINE void bit_array_clear_bit(T* buf, size_t index) noexcept {
    size_t word_index = index / kNumBits;
    size_t bit_index = index % kNumBits;

    buf[word_index] &= ~index_as_mask(bit_index);
  }

  template<class BitOp, class FullOp>
  static BL_INLINE void bit_array_op(T* buf, size_t index, size_t count) noexcept {
    if (count == 0)
      return;

    size_t word_index = index / kNumBits; // T[]
    size_t bit_index = index % kNumBits; // T[][]

    // The first BitWord requires special handling to preserve bits outside the fill region.
    size_t firstNBits = bl_min<size_t>(kNumBits - bit_index, count);
    T firstNBitsMask = shift_to_end(non_zero_start_mask(firstNBits), bit_index);

    buf[word_index] = BitOp::op(buf[word_index], firstNBitsMask);
    count -= firstNBits;
    if (count == 0)
      return;
    word_index++;

    // All bits between the first and last affected BitWords can be just filled.
    while (count >= kNumBits) {
      buf[word_index] = FullOp::op(buf[word_index], ones());
      word_index++;
      count -= kNumBits;
    }

    // The last BitWord requires special handling as well.
    if (count) {
      T lastNBitsMask = non_zero_start_mask(count);
      buf[word_index] = BitOp::op(buf[word_index], lastNBitsMask);
    }
  }

  template<class BitOp>
  static BL_INLINE void bit_array_combine_words(T* dst, const T* src, size_t count) noexcept {
    for (size_t i = 0; i < count; i++)
      dst[i] = BitOp::op(dst[i], src[i]);
  }

  //! Fills `count` of bits in bit-vector `buf` starting at bit-index `index`.
  static BL_INLINE void bit_array_fill(T* buf, size_t index, size_t count) noexcept {
    bit_array_op<BitOperator::Or, BitOperator::Assign>(buf, index, count);
  }

  static BL_INLINE void bit_array_and(T* buf, size_t index, size_t count) noexcept {
    bit_array_op<BitOperator::And, BitOperator::Assign>(buf, index, count);
  }

  //! Clears `count` of bits in bit-vector `buf` starting at bit-index `index`.
  static BL_INLINE void bit_array_clear(T* buf, size_t index, size_t count) noexcept {
    bit_array_op<BitOperator::AndNot, BitOperator::AssignNot>(buf, index, count);
  }

  static BL_INLINE void bit_array_not_and(T* buf, size_t index, size_t count) noexcept {
    bit_array_op<BitOperator::NotAnd, BitOperator::Assign>(buf, index, count);
  }

  template<typename IndexType>
  static BL_INLINE bool bit_array_first_bit(const T* data, size_t count, IndexType* index_out) noexcept {
    for (size_t i = 0; i < count; i++) {
      T bits = data[i];
      if (bits) {
        *index_out = IndexType(count_zeros_from_start(bits) + i * kNumBits);
        return true;
      }
    }

    *index_out = IntOps::all_ones<IndexType>();
    return false;
  }

  template<typename IndexType>
  static BL_INLINE bool bit_array_last_bit(const T* data, size_t count, IndexType* index_out) noexcept {
    size_t i = count;
    while (i) {
      T bits = data[--i];
      if (bits) {
        *index_out = IndexType((kBitMask - count_zeros_from_end(bits)) + i * kNumBits);
        return true;
      }
    }

    *index_out = IntOps::all_ones<IndexType>();
    return false;
  }

  //! Iterates over each bit in a number which is set to 1.
  //!
  //! Example of use:
  //!
  //! ```
  //! uint32_t bits_to_iterate = 0x110F;
  //! BitIterator<uint32_t> it(bits_to_iterate);
  //!
  //! while (it.has_next()) {
  //!   uint32_t bit_index = it.next();
  //!   printf("Bit at %u is set\n", unsigned(bit_index));
  //! }
  //! ```
  class BitIterator {
  public:
    T _bit_word;

    BL_INLINE_NODEBUG explicit BitIterator(T bit_word = 0) noexcept
      : _bit_word(bit_word) {}

    BL_INLINE_NODEBUG BitIterator(const BitIterator& other) noexcept = default;
    BL_INLINE_NODEBUG BitIterator& operator=(const BitIterator& other) noexcept = default;

    BL_INLINE_NODEBUG void init(T bit_word) noexcept { _bit_word = bit_word; }
    BL_INLINE_NODEBUG bool has_next() const noexcept { return _bit_word != 0; }

    BL_INLINE uint32_t next() noexcept {
      BL_ASSERT(_bit_word != 0);
      uint32_t index = count_zeros_from_start(_bit_word);
      _bit_word ^= index_as_mask(index);
      return index;
    }
  };

  //! Iterates over each bit in a BitWord, but shifts each iterated index by `kBitsPerChunkShift`.
  //!
  //! This class is used for very specific needs, currently only necessary on AArch64 targets when it comes to
  //! SIMD to GP vector mask handling, essentially workarounding the missing x86's `[V]PMOVMSKB` instruction.
  template<uint32_t kBitsPerChunkShift>
  class BitChunkIterator {
  public:
    T _bit_word;

    BL_INLINE_NODEBUG BitChunkIterator(T bit_word = 0) noexcept
      : _bit_word(bit_word) {}

    BL_INLINE_NODEBUG BitChunkIterator(const BitChunkIterator& other) noexcept = default;
    BL_INLINE_NODEBUG BitChunkIterator& operator=(const BitChunkIterator& other) noexcept = default;

    BL_INLINE_NODEBUG void init(T bit_word) noexcept { _bit_word = bit_word; }
    BL_INLINE_NODEBUG bool has_next() const noexcept { return _bit_word != 0; }

    BL_INLINE uint32_t next() noexcept {
      BL_ASSERT(_bit_word != 0);
      uint32_t index = count_zeros_from_start(_bit_word);
      _bit_word ^= index_as_mask(index);
      return index >> kBitsPerChunkShift;
    }
  };

  class BitVectorIterator {
  public:
    const T* _ptr;
    size_t _idx;
    size_t _end;
    T _current;

    BL_INLINE BitVectorIterator(const T* data, size_t num_bit_words, size_t start = 0) noexcept {
      init(data, num_bit_words, start);
    }

    BL_INLINE void init(const T* data, size_t num_bit_words, size_t start = 0) noexcept {
      const T* ptr = data + (start / kNumBits);

      size_t idx = IntOps::align_down(start, kNumBits);
      size_t end = num_bit_words * kNumBits;

      T bit_word = T(0);
      if (idx < end) {
        T firstNBitsMask = shift_to_end(ones(), start % kNumBits);
        bit_word = *ptr++ & firstNBitsMask;
        while (!bit_word && (idx += kNumBits) < end)
          bit_word = *ptr++;
      }

      _ptr = ptr;
      _idx = idx;
      _end = end;
      _current = bit_word;
    }

    BL_INLINE_NODEBUG bool has_next() const noexcept {
      return _current != T(0);
    }

    BL_INLINE size_t next() noexcept {
      BL_ASSERT(_current != T(0));

      uint32_t cnt = count_zeros_from_start(_current);
      T bit_word = _current ^ index_as_mask(cnt);

      size_t n = _idx + cnt;
      while (!bit_word && (_idx += kNumBits) < _end)
        bit_word = *_ptr++;

      _current = bit_word;
      return n;
    }

    BL_INLINE size_t peek_next() const noexcept {
      BL_ASSERT(_current != T(0));
      return _idx + count_zeros_from_start(_current);
    }
  };

  class BitVectorFlipIterator {
  public:
    BL_INLINE BitVectorFlipIterator(const T* data, size_t num_bit_words, size_t start = 0, T xor_mask = 0) noexcept {
      init(data, num_bit_words, start, xor_mask);
    }

    BL_INLINE void init(const T* data, size_t num_bit_words, size_t start = 0, T xor_mask = 0) noexcept {
      const T* ptr = data + (start / kNumBits);

      size_t idx = IntOps::align_down(start, kNumBits);
      size_t end = num_bit_words * kNumBits;

      T bit_word = T(0);
      if (idx < end) {
        T firstNBitsMask = shift_to_end(ones(), start % kNumBits);
        bit_word = (*ptr++ ^ xor_mask) & firstNBitsMask;
        while (!bit_word && (idx += kNumBits) < end)
          bit_word = *ptr++ ^ xor_mask;
      }

      _ptr = ptr;
      _idx = idx;
      _end = end;
      _current = bit_word;
      _xor_mask = xor_mask;
    }

    BL_INLINE_NODEBUG T xor_mask() const noexcept { return _xor_mask; }
    BL_INLINE_NODEBUG bool has_next() const noexcept { return _current != T(0); }

    BL_INLINE size_t next() noexcept {
      BL_ASSERT(_current != T(0));

      uint32_t cnt = count_zeros_from_start(_current);
      T bit_word = _current ^ index_as_mask(cnt);

      size_t n = _idx + cnt;
      while (!bit_word && (_idx += kNumBits) < _end)
        bit_word = *_ptr++ ^ _xor_mask;

      _current = bit_word;
      return n;
    }

    BL_INLINE size_t next_and_flip() noexcept {
      BL_ASSERT(_current != T(0));

      uint32_t cnt = count_zeros_from_start(_current);
      T bit_word = _current ^ shift_to_end(ones(), cnt);
      _xor_mask ^= ones();

      size_t n = _idx + cnt;
      while (!bit_word && (_idx += kNumBits) < _end)
        bit_word = *_ptr++ ^ _xor_mask;

      _current = bit_word;
      return n;
    }

    BL_INLINE size_t peek_next() const noexcept {
      BL_ASSERT(_current != T(0));
      return _idx + count_zeros_from_start(_current);
    }

    const T* _ptr;
    size_t _idx;
    size_t _end;
    T _current;
    T _xor_mask;
  };
};

//! \}

using PublicBitWordOps = ParametrizedBitOps<BitOrder::kPublic, BLBitWord>;
using PrivateBitWordOps = ParametrizedBitOps<BitOrder::kPrivate, BLBitWord>;

} // {anonymous}

// TODO: REMOVE, FOR COMPATIBILITY ONLY.

//! Iterates over each bit in a number which is set to 1.
//!
//! Example of use:
//!
//! ```
//! uint32_t bits_to_iterate = 0x110F;
//! BLBitWordIterator<uint32_t> it(bits_to_iterate);
//!
//! while (it.has_next()) {
//!   uint32_t bit_index = it.next();
//!   printf("Bit at %u is set\n", unsigned(bit_index));
//! }
//! ```
template<typename T>
class BitWordIterator {
public:
  BL_INLINE explicit BitWordIterator(T bit_word) noexcept
    : _bit_word(bit_word) {}

  BL_INLINE void init(T bit_word) noexcept { _bit_word = bit_word; }
  BL_INLINE bool has_next() const noexcept { return _bit_word != 0; }

  BL_INLINE uint32_t next() noexcept {
    BL_ASSERT(_bit_word != 0);
    uint32_t index = IntOps::ctz(_bit_word);
    _bit_word ^= T(1u) << index;
    return index;
  }

  T _bit_word;
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_BITOPS_P_H_INCLUDED
