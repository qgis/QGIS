// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CODEC_JPEGHUFFMAN_P_H_INCLUDED
#define BLEND2D_CODEC_JPEGHUFFMAN_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_codec_impl
//! \{

namespace bl::Jpeg {

// Acceleration constants for huffman decoding. Accel bits should be either 8 or 9. More bits consume more memory,
// but allow easier decoding of a code that fits within the number of bits. Libjpeg uses 8 bits with a comment that
// 8 bits is sufficient for decoding approximately 95% of codes so we use the same.
static constexpr uint32_t kHuffmanAccelBits = 8;
static constexpr uint32_t kHuffmanAccelSize = 1 << kHuffmanAccelBits;
static constexpr uint32_t kHuffmanAccelMask = kHuffmanAccelSize - 1;

//! JPEG Huffman decompression table.
struct DecoderHuffmanTable {
  //! Largest code of length k (-1 if none).
  uint32_t max_code[18];
  //! Value offsets (deltas) for codes of length k.
  int32_t delta[17];
  uint16_t code[256];
  uint8_t size[257];
  //! Huffman symbols, in order of increasing code length (part of DHT marker).
  uint8_t values[256];
  //! Acceleration table for decoding huffman codes up to `kHuffmanAccelBits`.
  uint8_t accel[kHuffmanAccelSize];
};

struct DecoderHuffmanACTable : public DecoderHuffmanTable {
  //! Additional table that decodes both magnitude and value of small ACs in one go.
  int16_t ac_accel[kHuffmanAccelSize];
};

struct DecoderHuffmanDCTable : public DecoderHuffmanTable {
  // No additional fields.
};

//! JPEG decoder's bit-stream.
//!
//! Holds the current decoder position in a bit-stream, but it's not used to fetch bits from it. Use
//! `DecoderBitReader` to actually read from the bit-stream.
struct DecoderBitStream {
  //! Data pointer (points to the byte to be processed).
  const uint8_t* ptr;
  //! End of input (points to the first invalid byte).
  const uint8_t* end;
  //! Machine word that contains available bits.
  BLBitWord bit_data;
  //! Number of valid bits in `bit_data`.
  size_t bit_count;
  //! TODO: [JPEG] Document EOB run.
  uint32_t eob_run;
  //! Restart counter in the current stream (reset by DRI and RST markers).
  uint32_t restart_counter;

  BL_INLINE void reset(const uint8_t* ptr_, const uint8_t* end_) noexcept {
    ptr = ptr_;
    end = end_;
    bit_data = 0;
    bit_count = 0;
    eob_run = 0;
  }

  BL_INLINE void reset() noexcept { reset(nullptr, nullptr); }
};

//! JPEG decoder's bit-reader.
//!
//! Class that is used to read data from `DecoderBitStream`.
struct DecoderBitReader {
  //! Data pointer (points to the byte to be processed).
  const uint8_t* ptr;
  //! End of input (points to the first invalid byte).
  const uint8_t* end;
  //! Machine word that contains available bits.
  BLBitWord bit_data;
  //! Number of valid bits in `bit_data`.
  size_t bit_count;

  BL_INLINE explicit DecoderBitReader(DecoderBitStream& stream) noexcept
    : ptr(stream.ptr),
      end(stream.end),
      bit_data(stream.bit_data),
      bit_count(stream.bit_count) {}

  BL_INLINE void done(DecoderBitStream& stream) noexcept {
    stream.bit_data = bit_data;
    stream.bit_count = bit_count;
    stream.ptr = ptr;
    stream.end = end;
  }

  BL_INLINE bool at_end() const noexcept { return ptr == end; }
  BL_INLINE bool has_bits(size_t n) const noexcept { return bit_count >= n; }

  BL_INLINE BLResult require_bits(size_t n) const noexcept {
    if (BL_UNLIKELY(!has_bits(n)))
      return bl_make_error(BL_ERROR_DECOMPRESSION_FAILED);
    return BL_SUCCESS;
  }

  BL_INLINE void flush() noexcept {
    bit_data = 0;
    bit_count = 0;
  }

  BL_INLINE void advance(size_t n_bytes) noexcept {
    BL_ASSERT(PtrOps::bytes_until(ptr, end) >= n_bytes);
    ptr += n_bytes;
  }

  BL_INLINE void drop(size_t n) noexcept {
    bit_data <<= n;
    bit_count -= n;
  }

  template<typename T = BLBitWord>
  BL_INLINE T peek(size_t n) const noexcept {
    using U = std::make_unsigned_t<T>;
    return T(U(bit_data >> (IntOps::bit_size_of<BLBitWord>() - n)));
  }

  BL_INLINE void refill() noexcept {
    while (bit_count <= IntOps::bit_size_of<BLBitWord>() - 8 && ptr != end) {
      uint32_t tmp_byte = *ptr++;

      // The [0xFF] byte has to be escaped by [0xFF, 0x00], so read two bytes.
      if (tmp_byte == 0xFF) {
        if (BL_UNLIKELY(ptr == end))
          break;

        uint32_t tmp_marker = *ptr++;
        if (BL_UNLIKELY(tmp_marker != 0x00)) {
          ptr -= 2;
          end = ptr;
          break;
        }
      }

      bit_data += BLBitWord(tmp_byte) << ((IntOps::bit_size_of<BLBitWord>() - 8) - bit_count);
      bit_count += 8;
    }
  }

  BL_INLINE void refill_if_32bit() noexcept {
    if (IntOps::bit_size_of<BLBitWord>() <= 32)
      return refill();
  }

  // Read a single bit (0 or 1).
  template<typename T = BLBitWord>
  BL_INLINE T read_bit() noexcept {
    BL_ASSERT(bit_count >= 1);

    T result = peek<T>(1);
    drop(1);
    return result;
  }

  // Read `n` bits and sign extend.
  BL_INLINE int32_t read_signed(size_t n) noexcept {
    BL_ASSERT(bit_count >= n);

    int32_t tmp_mask = IntOps::shl(int32_t(-1), n) + 1;
    int32_t tmp_sign = -peek<int32_t>(1);

    int32_t result = peek<int32_t>(n) + (tmp_mask & ~tmp_sign);
    drop(n);
    return result;
  }

  // Read `n` bits and zero extend.
  BL_INLINE uint32_t read_unsigned(size_t n) noexcept {
    BL_ASSERT(bit_count >= n);

    uint32_t result = peek<uint32_t>(n);
    drop(n);
    return result;
  }

  template<typename T>
  BL_INLINE BLResult read_code(T& dst, const DecoderHuffmanTable* table) noexcept {
    uint32_t code = table->accel[peek(kHuffmanAccelBits)];
    uint32_t code_size;

    if (code < 255) {
      // FAST: Look at the top bits and determine the symbol id.
      code_size = table->size[code];
      if (BL_UNLIKELY(code_size > bit_count))
        return bl_make_error(BL_ERROR_DECOMPRESSION_FAILED);
    }
    else {
      // SLOW: Naive test is to shift the `bit_data` down so `s` bits are valid, then test against `max_code`. To speed
      // this up, we've pre-shifted max_code left so that it has `16-s` 0s at the end; in other words, regardless of the
      // number of bits, it wants to be compared against something shifted to have 16; that way we don't need to shift
      // inside the loop.
      code = peek<uint32_t>(16);
      code_size = kHuffmanAccelBits + 1;

      while (code >= table->max_code[code_size])
        code_size++;

      // Maximum code size is 16, check for 17/bit_count and fail if reached.
      if (BL_UNLIKELY(code_size == 17 || code_size > bit_count))
        return bl_make_error(BL_ERROR_DECOMPRESSION_FAILED);

      // Convert the huffman code to the symbol ID.
      code = peek<uint32_t>(code_size) + uint32_t(table->delta[code_size]);
    }

    // Convert the symbol ID to the resulting BYTE.
    dst = T(table->values[code]);
    drop(code_size);
    return BL_SUCCESS;
  }
};

BL_HIDDEN BLResult build_huffman_ac(DecoderHuffmanACTable* table, const uint8_t* data, size_t data_size, size_t* bytes_consumed) noexcept;
BL_HIDDEN BLResult build_huffman_dc(DecoderHuffmanDCTable* table, const uint8_t* data, size_t data_size, size_t* bytes_consumed) noexcept;

} // {bl::Jpeg}

//! \}
//! \endcond

#endif // BLEND2D_CODEC_JPEGHUFFMAN_P_H_INCLUDED
