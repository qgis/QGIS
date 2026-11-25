// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_DEFLATEENCODERUTILS_P_H_INCLUDED
#define BLEND2D_COMPRESSION_DEFLATEENCODERUTILS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/compression/deflatedefs_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL

namespace bl::Compression::Deflate {

// We want to write machine work quantities, so the minimum padding is a machine word.
static constexpr uint32_t kMinOutputBufferPadding = sizeof(BLBitWord);

namespace {

static constexpr bool can_buffer_n(size_t n) noexcept { return n + 7 < IntOps::bit_size_of<BLBitWord>(); }

struct OutputBuffer {
  //! Pointer to the beginning of the output buffer.
  uint8_t* begin;
  //! Current pointer in the output buffer (points to the position where a next byte will be written).
  uint8_t* ptr;
  //! End of the output buffer (pointer to a position which is past the last byte).
  uint8_t* end;

  BL_INLINE_NODEBUG void init(uint8_t* output, size_t size) noexcept {
    BL_ASSERT(size >= kMinOutputBufferPadding);
    begin = output;
    ptr = output;
    end = output + size - kMinOutputBufferPadding;
  }

  BL_INLINE_NODEBUG void reset() noexcept { *this = OutputBuffer{}; }
  BL_INLINE_NODEBUG bool can_write() const noexcept { return ptr < end; }

  BL_INLINE_NODEBUG size_t byte_offset() const noexcept { return PtrOps::byte_offset(begin, ptr); }
  BL_INLINE_NODEBUG size_t remaining_bytes() const noexcept { return PtrOps::bytes_until(ptr, end); }
};

//! Bit-buffer used by the output stream.
//!
//! Bit buffer can hold at most 31 bits on a 32-bit target and 63 bits on a 64-bit target. The reason is that when
//! shifting more bits is undefined behavior (and most ISA implementations would shift nothing if the number of the
//! bits to shift is the same as the machine word size), so we avoid it by making sure the bit-buffer is never 100%
//! full.
struct OutputBits {
  //! Bits to flush.
  BLBitWord bit_word;
  //! Number of bits in `bit_word`, cannot exceed `sizeof(bit_word) * 8 - 1`.
  size_t bit_length;

  BL_INLINE_NODEBUG void reset() noexcept { *this = OutputBits{}; }

  BL_INLINE_NODEBUG BLBitWord all() const noexcept { return bit_word; }
  BL_INLINE_NODEBUG size_t length() const noexcept { return bit_length; }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return bit_length == 0; }
  BL_INLINE_NODEBUG bool was_properly_flushed() const noexcept { return bit_length <= 7 && (bit_word >> bit_length) == 0; }
  BL_INLINE_NODEBUG size_t remaining_bits() const noexcept { return (IntOps::bit_size_of<BLBitWord>() - 1u) - bit_length; }

  template<typename T>
  BL_INLINE_NODEBUG void add(const T& bits, size_t count) noexcept {
    BL_ASSERT(bit_length + count < IntOps::bit_size_of<BLBitWord>());

    bit_word |= size_t(bits) << bit_length;
    bit_length += count;
  }

  BL_INLINE_NODEBUG void align_to_bytes() noexcept {
    bit_length = (bit_length + 7u) & ~size_t(7);
  }

  BL_INLINE_NODEBUG void flush(OutputBuffer& buffer) noexcept {
    size_t n = bit_length / 8u;

    BL_ASSERT(n != IntOps::bit_size_of<BLBitWord>());
    BL_ASSERT(buffer.can_write());

    if constexpr (MemOps::kUnalignedMemIO) {
      MemOps::storeu_le(buffer.ptr, bit_word);
      buffer.ptr += n;

      bit_word >>= n * 8u;
      bit_length &= 7;
    }
    else {
      // Flush a byte at a time.
      while (n) {
        buffer.ptr[0] = uint8_t(bit_word & 0xFFu);
        buffer.ptr++;
        bit_word >>= 8;
        n--;
      }
      bit_length &= 7;
    }
  }

  template<size_t kN>
  BL_INLINE_NODEBUG void flushIfCannotBufferN(OutputBuffer& buffer) noexcept {
    if constexpr (!can_buffer_n(kN)) {
      flush(buffer);
    }
  }

  BL_INLINE_NODEBUG void flush_final_byte(OutputBuffer& buffer) noexcept {
    if (!is_empty()) {
      BL_ASSERT(length() <= 7u);
      buffer.ptr[0] = uint8_t(bit_word & 0xFFu);
      buffer.ptr++;

      bit_word = 0;
      bit_length = 0;
    }
  }
};

//! Output stream combines `OutputBits` and `OutputBuffer` - so it offers both bit & byte granularity.
struct OutputStream {
  //! Output bit-buffer contains N bits up to `sizeof(BLBitWord) * 8 - 1`, which flushed to output `buffer`.
  OutputBits bits;
  //! Output buffer and a position in it (ptr).
  OutputBuffer buffer;
};

} // {anonymous}
} // {bl::Compression::Deflate}

//! \endcond

#endif // BLEND2D_COMPRESSION_DEFLATEENCODERUTILS_P_H_INCLUDED
