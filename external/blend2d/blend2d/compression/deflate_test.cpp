// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/array_p.h>
#include <blend2d/core/random_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/compression/deflatedecoder_p.h>
#include <blend2d/compression/deflatedefs_p.h>
#include <blend2d/compression/deflateencoder_p.h>

// bl::Compression - Deflate - Tests
// =================================

namespace bl::Compression::Tests {

enum class TestStrategy {
  kWholeData = 0,
  kChunkedData = 1,
  kBytePerByte = 2,

  kMaxValue = kBytePerByte
};

enum class TestRandomMode {
  //! Random data where there are repeat sequences, to test both literals and lengths.
  kRandomDataWithRepeats = 0,
  //! Random data that only uses nibbles, but don't have repeat sequences - to test Huffman literals.
  kRandomDataWithNibbles = 1,
  //! Random data contains only two values (0x00 and 0xFF).
  kRandomDataWithTwoLiterals = 2,
  //! The whole input data contains zeros
  kAllZeros = 3,

  kMaxValue = kAllZeros
};

static const char* stringify_strategy(TestStrategy strategy) noexcept {
  switch (strategy) {
    case TestStrategy::kWholeData: return "whole data";
    case TestStrategy::kChunkedData: return "chunked data";
    case TestStrategy::kBytePerByte: return "byte per byte";
    default:
      return "unknown";
  }
}

static const char* stringify_random_mode(TestRandomMode mode) noexcept {
  switch (mode) {
    case TestRandomMode::kRandomDataWithRepeats: return "repeats";
    case TestRandomMode::kRandomDataWithNibbles: return "nibbles";
    case TestRandomMode::kRandomDataWithTwoLiterals: return "two-literals";
    case TestRandomMode::kAllZeros: return "zeros";
    default:
      return "unknown";
  }
}

class SimpleBitWriter {
public:
  BLArray<uint8_t>& dst;
  BLBitWord bit_word {};
  size_t bit_length {};

  SimpleBitWriter(BLArray<uint8_t>& dst) noexcept : dst(dst) {}
  ~SimpleBitWriter() noexcept { finalize(); }

  void align_to_byte() noexcept {
    bit_length = (bit_length + 7u) & ~size_t(7);
  }

  void flush() noexcept {
    while (bit_length >= 8) {
      EXPECT_SUCCESS(dst.append(uint8_t(bit_word & 0xFFu)));
      bit_word >>= 8;
      bit_length -= 8;
    }
  }

  void finalize() noexcept {
    align_to_byte();
    flush();
  }

  void append(size_t bits, size_t n) noexcept {
    bit_word |= bits << bit_length;
    bit_length += n;
    flush();
  }
};

static BLResult append_random_bytes(BLArray<uint8_t>& array, BLRandom& rnd, size_t n, TestRandomMode random_mode) noexcept {
  uint8_t* dst_data;
  BL_PROPAGATE(array.modify_op(BL_MODIFY_OP_APPEND_GROW, n, &dst_data));

  uint8_t* dst_ptr = dst_data;
  size_t i = n;

  switch (random_mode) {
    case TestRandomMode::kRandomDataWithRepeats: {
      while (i >= 4) {
        uint32_t cat = rnd.next_uint32();

        size_t pos = PtrOps::byte_offset(dst_data, dst_ptr);
        if ((cat & 0x7) == 0x7 && pos > 16) {
          // Repeat sequence of some past bytes.
          size_t offset = (size_t((cat >> 16)) % bl_min<size_t>(pos, 32767)) + 1;
          size_t length = bl_max<size_t>((((cat >> 8) & 0xFF) + 3) % i, 3u);

          size_t end = i - length;

          while (i != end) {
            dst_ptr[0] = dst_ptr[-intptr_t(offset)];
            dst_ptr++;
            i--;
          }

          continue;
        }
        else {
          uint32_t val = rnd.next_uint32();
          if (cat & 0x80000000) {
            // Repeat sequence of a single BYTE.
            val = (val & 0xFFu) * 0x01010101u;
          }
          else {
            // Sequence of random bytes.
            val = IntOps::byteSwap32LE(val);
          }

          memcpy(dst_ptr, &val, 4);
          dst_ptr += 4;
          i -= 4;
        }
      }

      if (i) {
        uint32_t val = IntOps::byteSwap32LE(rnd.next_uint32());
        memcpy(dst_ptr, &val, i);
      }
      break;
    }

    case TestRandomMode::kRandomDataWithNibbles: {
      while (i >= 8) {
        uint64_t val = rnd.next_uint64() & 0x0F0F0F0F0F0F0F0Fu;
        memcpy(dst_ptr, &val, 8);

        dst_ptr += 8;
        i -= 8;
      }

      if (i) {
        uint64_t val = rnd.next_uint64() & 0x0F0F0F0F0F0F0F0Fu;
        memcpy(dst_ptr, &val, i);
      }
      break;
    }

    case TestRandomMode::kRandomDataWithTwoLiterals: {
      while (i >= 8) {
        uint64_t val = (rnd.next_uint64() & 0x0101010101010101u) * 0xFFu;
        memcpy(dst_ptr, &val, 8);

        dst_ptr += 8;
        i -= 8;
      }

      if (i) {
        uint64_t val = (rnd.next_uint64() & 0x0101010101010101u) * 0xFFu;
        memcpy(dst_ptr, &val, i);
      }
      break;
    }

    case TestRandomMode::kAllZeros: {
      memset(dst_ptr, 0, i);
      break;
    }
  }

  return BL_SUCCESS;
}

static size_t compare_decoded_data(const uint8_t* a, const uint8_t* b, size_t n) noexcept {
  for (size_t i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return i;
    }
  }
  return SIZE_MAX;
}

static void test_deflate_invalid_stream_with_data(const char* test_name, Deflate::FormatType format, BLResult expected_result, BLDataView compressed) noexcept {
  BLArray<uint8_t> output;
  Deflate::Decoder decoder;

  decoder.init(format);
  BLResult result = decoder.decode(output, compressed);
  EXPECT_EQ(result, expected_result)
    .message("Decompressing invalid stream '%s' didn't fail (0x%08X returned, 0x%08X expected)", test_name, result, expected_result);
}

// The content of this test comes from a libdeflate test - `test_incomplete_codes.c`.
static void test_deflate_empty_offset_code() noexcept {
  // Generate a DEFLATE stream containing a "dynamic Huffman" block containing literals,
  // but no offsets; and having an empty offset code (all codeword lengths set to 0).
  static const uint8_t expected[] = {
    uint8_t('A'),
    uint8_t('B'),
    uint8_t('A'),
    uint8_t('A')
  };

  // Litlen code:
  //   litlensym_A                   freq=3 len=1 codeword= 0
  //   litlensym_B                   freq=1 len=2 codeword=01
  //   litlensym_256 (end-of-block)  freq=1 len=2 codeword=11
  //
  // Offset code:
  //   (empty)
  //
  // Litlen and offset codeword lengths:
  //   [0..'A'-1]   = 0  presym_18
  //   ['A']        = 1  presym_1
  //   ['B']        = 2  presym_2
  //   ['B'+1..255] = 0  presym_18 presym_18
  //   [256]        = 2  presym_2
  //   [257]        = 0  presym_0
  //
  // Precode:
  //   presym_0   freq=1 len=3 codeword=011
  //   presym_1   freq=1 len=3 codeword=111
  //   presym_2   freq=2 len=2 codeword= 01
  //   presym_18  freq=3 len=1 codeword=  0

  BLArray<uint8_t> input;
  {
    SimpleBitWriter writer(input);

    // Block header:
    writer.append(1, 1);    // BFINAL: 1
    writer.append(2, 2);    // BTYPE: DYNAMIC_HUFFMAN
    writer.append(0, 5);    // num litlen symbols: 0 + 257
    writer.append(0, 5);    // num offset symbols: 0 + 1
    writer.append(14, 4);   // num explicit precode lens: 14 + 4

    // Precode codeword lengths:
    //   permutation == [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
    writer.append(0, 3);    // presym_16: len=0
    writer.append(0, 3);    // presym_17: len=0
    writer.append(1, 3);    // presym_18: len=1
    writer.append(3, 3);    // presym_0 : len=3
    writer.append(0, 3);    // presym_8 : len=0
    writer.append(0, 3);    // presym_7 : len=0
    writer.append(0, 3);    // presym_9 : len=0
    writer.append(0, 3);    // presym_6 : len=0
    writer.append(0, 3);    // presym_10: len=0
    writer.append(0, 3);    // presym_5 : len=0
    writer.append(0, 3);    // presym_11: len=0
    writer.append(0, 3);    // presym_4 : len=0
    writer.append(0, 3);    // presym_12: len=0
    writer.append(0, 3);    // presym_3 : len=0
    writer.append(0, 3);    // presym_13: len=0
    writer.append(2, 3);    // presym_2 : len=2
    writer.append(0, 3);    // presym_14: len=0
    writer.append(3, 3);    // presym_1 : len=3

    // Litlen and offset codeword lengths:
    writer.append(0x0, 1);  // presym_18
    writer.append(54, 7);   // ... 11 + 54 zeroes
    writer.append(0x7, 3);  // presym_1
    writer.append(0x1, 2);  // presym_2
    writer.append(0x0, 1);  // presym_18,
    writer.append(89, 7);   // ... 11 + 89 zeroes
    writer.append(0x0, 1);  // presym_18
    writer.append(78, 7);   // ... 11 + 78 zeroes
    writer.append(0x1, 2);  // presym_2
    writer.append(0x3, 3);  // presym_0

    // Litlen symbols:
    writer.append(0x0, 1);  // litlensym_A
    writer.append(0x1, 2);  // litlensym_B
    writer.append(0x0, 1);  // litlensym_A
    writer.append(0x0, 1);  // litlensym_A
    writer.append(0x3, 2);  // litlensym_256 (end-of-block)

    writer.finalize();
  }

  BLArray<uint8_t> output;
  Deflate::Decoder decoder;

  decoder.init(Deflate::FormatType::kRaw);
  EXPECT_SUCCESS(decoder.decode(output, input.view()));

  EXPECT_EQ(output.size(), sizeof(expected));

  size_t index = compare_decoded_data(output.data(), expected, sizeof(expected));
  EXPECT_EQ(index, SIZE_MAX)
    .message("Output data doesn't match at %zu: output(0x%08X) != expected(0x%08X)", index, output[index], expected[index]);
}

// The content of this test comes from a libdeflate test - `test_incomplete_codes.c`.
static void test_deflate_singleton_litrunlen_code() noexcept {
  // Test that a litrunlen code containing only one symbol is accepted.

  // Litlen code:
  //   litlensym_256 (end-of-block)  freq=1 len=1 codeword=0
  //
  // Offset code:
  //   (empty)
  //
  // Litlen and offset codeword lengths:
  //   [0..256]  = 0  presym_18 presym_18
  //   [256]     = 1  presym_1
  //   [257]     = 0  presym_0
  //
  // Precode:
  //   presym_0   freq=1 len=2 codeword=01
  //   presym_1   freq=1 len=2 codeword=11
  //   presym_18  freq=2 len=1 codeword= 0
  BLArray<uint8_t> input;
  {
    SimpleBitWriter writer(input);

    // Block header:
    writer.append(1, 1);    // BFINAL: 1
    writer.append(2, 2);    // BTYPE: DYNAMIC_HUFFMAN
    writer.append(0, 5);    // num litlen symbols: 0 + 257
    writer.append(0, 5);    // num offset symbols: 0 + 1
    writer.append(14, 4);   // num explicit precode lens: 14 + 4

    // Precode codeword lengths:
    //   permutation == [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]

    writer.append(0, 3);    // presym_16: len=0
    writer.append(0, 3);    // presym_17: len=0
    writer.append(1, 3);    // presym_18: len=1
    writer.append(2, 3);    // presym_0 : len=2
    writer.append(0, 3);    // presym_8 : len=0
    writer.append(0, 3);    // prexym_7 : len=0
    writer.append(0, 3);    // prexym_9 : len=0
    writer.append(0, 3);    // prexym_6 : len=0
    writer.append(0, 3);    // prexym_10: len=0
    writer.append(0, 3);    // prexym_5 : len=0
    writer.append(0, 3);    // prexym_11: len=0
    writer.append(0, 3);    // prexym_4 : len=0
    writer.append(0, 3);    // prexym_12: len=0
    writer.append(0, 3);    // prexym_3 : len=0
    writer.append(0, 3);    // prexym_13: len=0
    writer.append(0, 3);    // prexym_2 : len=0
    writer.append(0, 3);    // prexym_14: len=0
    writer.append(2, 3);    // presym_1: len=2

    // Litlen and offset codeword lengths:
    writer.append(0, 1);    // presym_18
    writer.append(117, 7);  // ... 11 + 117 zeroes
    writer.append(0, 1);    // presym_18
    writer.append(117, 7);  // ... 11 + 117 zeroes
    writer.append(0x3, 2);  // presym_1
    writer.append(0x1, 2);  // presym_0

    // Litlen symbols:
    writer.append(0x0, 1);  // litlensym_256 (end-of-block)

    writer.finalize();
  }

  BLArray<uint8_t> output;
  Deflate::Decoder decoder;

  decoder.init(Deflate::FormatType::kRaw);
  EXPECT_SUCCESS(decoder.decode(output, input.view()));
  EXPECT_EQ(output.size(), 0u);
}

// The content of this test comes from a libdeflate test - `test_incomplete_codes.c`.
static void test_deflate_singleton_offset_code() noexcept {
  // Test that an offset code containing only one symbol is accepted.
  static const uint8_t expected[] = {
    255,
    255,
    255,
    255
  };

  // Litlen code:
  //   litlensym_255                 freq=1 len=1 codeword= 0
  //   litlensym_256 (end-of-block)  freq=1 len=2 codeword=01
  //   litlensym_257 (len 3)         freq=1 len=2 codeword=11
  //
  // Offset code:
  //   offsetsym_0 (offset 0)        freq=1 len=1 codeword=0
  //
  // Litlen and offset codeword lengths:
  //   [0..254] = 0  presym_{18,18}
  //   [255]    = 1  presym_1
  //   [256]    = 1  presym_2
  //   [257]    = 1  presym_2
  //   [258]    = 1  presym_1
  //
  // Precode:
  //   presym_1  freq=2 len=2 codeword=01
  //   presym_2  freq=2 len=2 codeword=11
  //   presym_18 freq=2 len=1 codeword= 0
  BLArray<uint8_t> input;
  {
    SimpleBitWriter writer(input);

    // Block header:
    writer.append(1, 1);    // BFINAL: 1
    writer.append(2, 2);    // BTYPE: DYNAMIC_HUFFMAN
    writer.append(1, 5);    // num litlen symbols: 1 + 257
    writer.append(0, 5);    // num offset symbols: 0 + 1
    writer.append(14, 4);   // num explicit precode lens: 14 + 4

    // Precode codeword lengths:
    //   permutation == [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
    writer.append(0, 3);    // presym_16: len=0
    writer.append(0, 3);    // presym_17: len=0
    writer.append(1, 3);    // presym_18: len=1
    writer.append(0, 3);    // presym_0 : len=0
    writer.append(0, 3);    // presym_8 : len=0
    writer.append(0, 3);    // presym_7 : len=0
    writer.append(0, 3);    // presym_9 : len=0
    writer.append(0, 3);    // presym_6 : len=0
    writer.append(0, 3);    // presym_10: len=0
    writer.append(0, 3);    // presym_5 : len=0
    writer.append(0, 3);    // presym_11: len=0
    writer.append(0, 3);    // presym_4 : len=0
    writer.append(0, 3);    // presym_12: len=0
    writer.append(0, 3);    // presym_3 : len=0
    writer.append(0, 3);    // presym_13: len=0
    writer.append(2, 3);    // presym_2 : len=2
    writer.append(0, 3);    // presym_14: len=0
    writer.append(2, 3);    // presym_1 : len=2

    // Litlen and offset codeword lengths
    writer.append(0x0, 1);  // presym_18
    writer.append(117, 7);  // ... 11 + 117 zeroes
    writer.append(0x0, 1);  // presym_18
    writer.append(116, 7);  // ... 11 + 116 zeroes
    writer.append(0x1, 2);  // presym_1
    writer.append(0x3, 2);  // presym_2
    writer.append(0x3, 2);  // presym_2
    writer.append(0x1, 2);  // presym_1

    // Literal
    writer.append(0x0, 1);  // litlensym_255

    // Match
    writer.append(0x3, 2);  // litlensym_257
    writer.append(0x0, 1);  // offsetsym_0

    // End of block
    writer.append(0x1, 2);  // litlensym_256

    writer.finalize();
  }

  BLArray<uint8_t> output;
  Deflate::Decoder decoder;

  decoder.init(Deflate::FormatType::kRaw);
  EXPECT_SUCCESS(decoder.decode(output, input.view()));

  EXPECT_EQ(output.size(), sizeof(expected));

  size_t index = compare_decoded_data(output.data(), expected, sizeof(expected));
  EXPECT_EQ(index, SIZE_MAX)
    .message("Output data doesn't match at %zu: output(0x%08X) != expected(0x%08X)", index, output[index], expected[index]);
}

// The content of this test comes from a libdeflate test - `test_incomplete_codes.c`.
static void test_deflate_singleton_offset_code_notsymzero() noexcept {
  // Test that an offset code containing only one symbol is accepted, even if
  // that symbol is not symbol 0. The codeword should be '0' in either case.
  static const uint8_t expected[] = {
    254,
    255,
    254,
    255,
    254
  };

  // Litlen code:
  //   litlensym_254                 len=2 codeword=00
  //   litlensym_255                 len=2 codeword=10
  //   litlensym_256 (end-of-block)  len=2 codeword=01
  //   litlensym_257 (len 3)         len=2 codeword=11
  //
  // Offset code:
  //   offsetsym_1 (offset 2)        len=1 codeword=0
  //
  // Litlen and offset codeword lengths:
  //   [0..253] = 0  presym_{18,18}
  //   [254]    = 2  presym_2
  //   [255]    = 2  presym_2
  //   [256]    = 2  presym_2
  //   [257]    = 2  presym_2
  //   [258]    = 0  presym_0
  //   [259]    = 1  presym_1
  //
  // Precode:
  //   presym_0   len=2 codeword=00
  //   presym_1   len=2 codeword=10
  //   presym_2   len=2 codeword=01
  //   presym_18  len=2 codeword=11
  BLArray<uint8_t> input;
  {
    SimpleBitWriter writer(input);

    // Block header:
    writer.append(1, 1);    // BFINAL: 1
    writer.append(2, 2);    // BTYPE: DYNAMIC_HUFFMAN
    writer.append(1, 5);    // num litlen symbols: 1 + 257
    writer.append(1, 5);    // num offset symbols: 1 + 1
    writer.append(14, 4);   // num explicit precode lens: 14 + 4

    // Precode codeword lengths:
    //   permutation == [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
    writer.append(0, 3);    // presym_16: len=0
    writer.append(0, 3);    // presym_17: len=0
    writer.append(2, 3);    // presym_18: len=2
    writer.append(2, 3);    // presym_0 : len=2
    writer.append(0, 3);    // presym_8 : len=0
    writer.append(0, 3);    // presym_7 : len=0
    writer.append(0, 3);    // presym_9 : len=0
    writer.append(0, 3);    // presym_6 : len=0
    writer.append(0, 3);    // presym_10: len=0
    writer.append(0, 3);    // presym_5 : len=0
    writer.append(0, 3);    // presym_11: len=0
    writer.append(0, 3);    // presym_4 : len=0
    writer.append(0, 3);    // presym_12: len=0
    writer.append(0, 3);    // presym_3 : len=0
    writer.append(0, 3);    // presym_13: len=0
    writer.append(2, 3);    // presym_2 : len=2
    writer.append(0, 3);    // presym_14: len=0
    writer.append(2, 3);    // presym_1 : len=2

    // Litlen and offset codeword lengths
    writer.append(0x3, 2);  // presym_18
    writer.append(117, 7);  // ... 11 + 117 zeroes
    writer.append(0x3, 2);  // presym_18
    writer.append(115, 7);  // ... 11 + 115 zeroes
    writer.append(0x1, 2);  // presym_2
    writer.append(0x1, 2);  // presym_2
    writer.append(0x1, 2);  // presym_2
    writer.append(0x1, 2);  // presym_2
    writer.append(0x0, 2);  // presym_0
    writer.append(0x2, 2);  // presym_1

    // Literals
    writer.append(0x0, 2);  // litlensym_254
    writer.append(0x2, 2);  // litlensym_255

    // Match
    writer.append(0x3, 2);  // litlensym_257
    writer.append(0x0, 1);  // offsetsym_1

    // End of block
    writer.append(0x1, 2);  // litlensym_256

    writer.finalize();
  }

  BLArray<uint8_t> output;
  Deflate::Decoder decoder;

  decoder.init(Deflate::FormatType::kRaw);
  EXPECT_SUCCESS(decoder.decode(output, input.view()));

  EXPECT_EQ(output.size(), sizeof(expected));

  size_t index = compare_decoded_data(output.data(), expected, sizeof(expected));
  EXPECT_EQ(index, SIZE_MAX)
    .message("Output data doesn't match at %zu: output(0x%08X) != expected(0x%08X)", index, output[index], expected[index]);
}

// The content of this test comes from a libdeflate test - `test_invalid_streams.c`.
static void test_deflate_too_many_codeword_lengths() noexcept {
  // Test that DEFLATE decompression returns an error if a block header
  // contains too many encoded litlen and offset codeword lengths.

  // Litlen code:
  //   litlensym_255                 len=1 codeword=0
  //   litlensym_256 (end-of-block)  len=1 codeword=1
  //
  // Offset code:
  //   (empty)
  //
  // Litlen and offset codeword lengths:
  //   [0..254] = 0  presym_{18,18}
  //   [255]    = 1  presym_1
  //   [256]    = 1  presym_1
  //   [257...] = 0  presym_18 [TOO MANY]
  //
  // Precode:
  //   presym_1   len=1 codeword=0
  //   presym_18  len=1 codeword=1
  BLArray<uint8_t> input;
  {
    SimpleBitWriter writer(input);

    // Block header:
    writer.append(1, 1);    // BFINAL: 1
    writer.append(2, 2);    // BTYPE: DYNAMIC_HUFFMAN

    writer.append(0, 5);    // num litlen symbols: 0 + 257
    writer.append(0, 5);    // num offset symbols: 0 + 1
    writer.append(14, 4);   // num explicit precode lens: 14 + 4

    // Precode codeword lengths:
    //   permutation == [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
    writer.append(0, 3);    // presym_16: len=0
    writer.append(0, 3);    // presym_17: len=0
    writer.append(1, 3);    // presym_18: len=1
    writer.append(0, 3);    // presym_0 : len=0
    writer.append(0, 3);    // presym_8 : len=0
    writer.append(0, 3);    // presym_7 : len=0
    writer.append(0, 3);    // presym_9 : len=0
    writer.append(0, 3);    // presym_6 : len=0
    writer.append(0, 3);    // presym_10: len=0
    writer.append(0, 3);    // presym_5 : len=0
    writer.append(0, 3);    // presym_11: len=0
    writer.append(0, 3);    // presym_4 : len=0
    writer.append(0, 3);    // presym_12: len=0
    writer.append(0, 3);    // presym_3 : len=0
    writer.append(0, 3);    // presym_13: len=0
    writer.append(0, 3);    // presym_2 : len=0
    writer.append(0, 3);    // presym_14: len=0
    writer.append(1, 3);    // presym_1 : len=1

    // Litlen and offset codeword lengths
    writer.append(0x1, 1);  // presym_18
    writer.append(117, 7);  // ... 11 + 117 zeroes
    writer.append(0x1, 1);  // presym_18
    writer.append(116, 7);  // ... 11 + 116 zeroes
    writer.append(0x0, 1);  // presym_1
    writer.append(0x0, 1);  // presym_1
    writer.append(0x1, 1);  // presym_18
    writer.append(117, 7);  // ... 11 + 117 zeroes [!!! TOO MANY !!!]

    // Literal
    writer.append(0x0, 0);  // litlensym_255

    // End of block
    writer.append(0x1, 1);  // litlensym_256

    writer.finalize();
  }

  test_deflate_invalid_stream_with_data("too many codeword lengths", Deflate::FormatType::kRaw, BL_ERROR_DECOMPRESSION_FAILED, input.view());
}

// The content of this test comes from a libdeflate test - `test_overread.c`.
static void test_deflate_overread() noexcept {
  // Litlen code:
  //   litlensym_0   (0)            len=1 codeword=0
  //   litlensym_256 (end-of-block) len=1 codeword=1
  //
  // Offset code:
  //   offsetsym_0 (unused)         len=1 codeword=0
  //
  // Litlen and offset codeword lengths:
  //   [0]      = 1  presym_1
  //   [1..255] = 0  presym_{18,18}
  //   [256]    = 1  presym_1
  //   [257]    = 1  presym_1
  //
  // Precode:
  //   presym_1  len=1 codeword=0
  //   presym_18  len=1 codeword=1
  BLArray<uint8_t> input;
  {
    SimpleBitWriter writer(input);

    // Block header:
    writer.append(0, 1);    // BFINAL: 0
    writer.append(2, 2);    // BTYPE: DYNAMIC_HUFFMAN
    writer.append(0, 5);    // num litlen symbols: 0 + 257
    writer.append(0, 5);    // num offset symbols: 0 + 1
    writer.append(14, 4);   // num explicit precode lens: 14 + 4

    // Precode codeword lengths:
    //   permutation == [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
    writer.append(0, 3);    // presym_16: len=0
    writer.append(0, 3);    // presym_17: len=0
    writer.append(1, 3);    // presym_18: len=1
    writer.append(0, 3);    // presym_0 : len=0
    writer.append(0, 3);    // presym_8 : len=0
    writer.append(0, 3);    // presym_7 : len=0
    writer.append(0, 3);    // presym_9 : len=0
    writer.append(0, 3);    // presym_6 : len=0
    writer.append(0, 3);    // presym_10: len=0
    writer.append(0, 3);    // presym_5 : len=0
    writer.append(0, 3);    // presym_11: len=0
    writer.append(0, 3);    // presym_4 : len=0
    writer.append(0, 3);    // presym_12: len=0
    writer.append(0, 3);    // presym_3 : len=0
    writer.append(0, 3);    // presym_13: len=0
    writer.append(0, 3);    // presym_2 : len=0
    writer.append(0, 3);    // presym_14: len=0
    writer.append(1, 3);    // presym_1 : len=1

    // Litlen and offset codeword lengths:
    writer.append(0, 1);    // presym_1
    writer.append(1, 1);    // presym_18 ...
    writer.append(117, 7);  // ... 11 + 117 zeroes
    writer.append(1, 1);    // presym_18 ...
    writer.append(116, 7);  // ... 11 + 116 zeroes
    writer.append(0, 1);    // presym_1
    writer.append(0, 1);    // presym_1

    writer.finalize();
  }

  // NOTE: The difference between Blend2D and libdeflate is that Blend2D allows chunking, so if the data is
  // incomplete Blend2D returns `BL_ERROR_DATA_TRUNCATED` and expects used to provide the missing data. If
  // there is no missing data, the stream would be invalid.
  test_deflate_invalid_stream_with_data("too many codeword lengths", Deflate::FormatType::kRaw, BL_ERROR_DATA_TRUNCATED, input.view());
}

// The content of this test comes from a libdeflate test - `test_invalid_streams.c`
static void test_deflate_invalid_streams() noexcept {
  static const uint8_t stream1[100] = {
    0x78, 0x9C, 0x15, 0xCA, 0xC1, 0x0D, 0xC3, 0x20, 0x0C, 0x00, 0xC0, 0x7F,
    0xA6, 0xF0, 0x02, 0x40, 0xD2, 0x77, 0xE9, 0x2A, 0xC8, 0xA1, 0xA0, 0x5A,
    0x4A, 0x89, 0x65, 0x1B, 0x29, 0xF2, 0xF4, 0x51, 0xEE, 0x3D, 0x31, 0x3D,
    0x3A, 0x7C, 0xB6, 0xC4, 0xE0, 0x9C, 0xD4, 0x0E, 0x00, 0x00, 0x00, 0x3D,
    0x85, 0xA7, 0x26, 0x08, 0x33, 0x87, 0xDE, 0xD7, 0xFA, 0x80, 0x80, 0x62,
    0xD4, 0xB1, 0x32, 0x87, 0xE6, 0xD7, 0xFA, 0x80, 0x80, 0x62, 0xD4, 0xB1,
    0x26, 0x61, 0x69, 0x9D, 0xAE, 0x1C, 0x53, 0x15, 0xD4, 0x5F, 0x7B, 0x22,
    0x0B, 0x0D, 0x2B, 0x9D, 0x12, 0x32, 0x56, 0x0D, 0x4D, 0xFF, 0xB6, 0xDC,
    0x8A, 0x02, 0x27, 0x38
  };

  static const uint8_t stream2[86] = {
    0x78, 0x9C, 0x15, 0xCA, 0xC1, 0x0D, 0xC3, 0x20, 0x0C, 0x00, 0xC0, 0x7F,
    0xA6, 0xF0, 0x02, 0x40, 0xD2, 0x77, 0xE9, 0x2A, 0xC8, 0xA1, 0xA0, 0x5A,
    0x4A, 0x89, 0x65, 0x1B, 0x29, 0xF2, 0xF4, 0x51, 0xEE, 0xFF, 0xFF, 0xFF,
    0x03, 0x37, 0x08, 0x5F, 0x78, 0xC3, 0x16, 0xFC, 0xA0, 0x3D, 0x3A, 0x7C,
    0x9D, 0xAE, 0x1C, 0x53, 0x15, 0xD4, 0x5F, 0x7B, 0x22, 0x0B, 0x0D, 0x2B,
    0x9D, 0x12, 0x34, 0x56, 0x0D, 0x4D, 0xFF, 0xB6, 0xDC, 0x4E, 0xC9, 0x14,
    0x67, 0x9C, 0x3E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBA, 0xEC,
    0x0B, 0x1D
  };

  static const uint8_t stream3[31] = {
    0x78, 0x9C, 0xEA, 0xCA, 0xC1, 0x0D, 0xC3, 0x00, 0x5B, 0x2D, 0xEE, 0x7D,
    0x31, 0x78, 0x9C, 0x15, 0xCA, 0xC1, 0x0D, 0xC3, 0x20, 0x0C, 0x00, 0x18,
    0x31, 0x85, 0x07, 0x02, 0x40, 0x39, 0x13
  };

  static const uint8_t stream4[86] = {
    0x78, 0x9C, 0x15, 0xC6, 0xC1, 0x0D, 0xC3, 0x20, 0x0C, 0x00, 0xC0, 0x7F,
    0xA6, 0xF0, 0x02, 0x40, 0xD2, 0x77, 0xE9, 0x2A, 0xC8, 0xA1, 0xA0, 0x5A,
    0x4A, 0x89, 0x65, 0x1B, 0x29, 0xF2, 0xF4, 0x51, 0xEE, 0x7D, 0x30, 0x39,
    0x13, 0x37, 0x08, 0x5F, 0x78, 0xC3, 0x16, 0xFC, 0xA0, 0x3D, 0x3A, 0x7C,
    0xE0, 0x9C, 0x1B, 0x29, 0xF2, 0xF4, 0x51, 0xEE, 0xDF, 0xD2, 0x0C, 0x4E,
    0x26, 0x08, 0x32, 0x87, 0x53, 0x15, 0xD4, 0x4D, 0xFF, 0xB6, 0xDC, 0x45,
    0x8D, 0xC0, 0x3B, 0xA6, 0xF0, 0x40, 0xEE, 0x51, 0x02, 0x7D, 0x45, 0x8D,
    0x2B, 0xCA
  };

  test_deflate_invalid_stream_with_data("stream1", Deflate::FormatType::kZlib, BL_ERROR_DECOMPRESSION_FAILED, BLDataView{stream1, sizeof(stream1)});
  test_deflate_invalid_stream_with_data("stream2", Deflate::FormatType::kZlib, BL_ERROR_DECOMPRESSION_FAILED, BLDataView{stream2, sizeof(stream2)});
  test_deflate_invalid_stream_with_data("stream3", Deflate::FormatType::kZlib, BL_ERROR_DECOMPRESSION_FAILED, BLDataView{stream3, sizeof(stream3)});
  test_deflate_invalid_stream_with_data("stream4", Deflate::FormatType::kZlib, BL_ERROR_DECOMPRESSION_FAILED, BLDataView{stream4, sizeof(stream4)});
}

static void test_deflate_roundtrip(BLDataView input, Deflate::FormatType format, uint32_t compression_level, const char* test_data_name) noexcept {
  BLArray<uint8_t> encoded;

  {
    Deflate::Encoder encoder;
    EXPECT_SUCCESS(encoder.init(format, compression_level))
      .message("Failed to initialize the encoder");

    EXPECT_SUCCESS(encoder.compress(encoded, BL_MODIFY_OP_APPEND_GROW, input));
  }

  for (uint32_t strategy_index = 0; strategy_index <= uint32_t(TestStrategy::kMaxValue); strategy_index++) {
    TestStrategy strategy = TestStrategy(strategy_index);

    Deflate::Decoder decoder;
    EXPECT_SUCCESS(decoder.init(format))
      .message("Failed to initialize the decoder");

    BLArray<uint8_t> decoded;

    switch (strategy) {
      case TestStrategy::kWholeData: {
        EXPECT_SUCCESS(decoder.decode(decoded, encoded.view()))
          .message("Decompression failed (%s/%s): input.size=%zu encoded.size=%zu decoded.size=%zu (first mismatching byte at %zd)",
            stringify_strategy(strategy),
            test_data_name,
            input.size,
            encoded.size(),
            decoded.size(),
            compare_decoded_data(input.data, decoded.data(), bl_min(input.size, decoded.size())));
        break;
      }

      case TestStrategy::kChunkedData:
      case TestStrategy::kBytePerByte: {
        size_t max_chunk_size = strategy == TestStrategy::kChunkedData ? (encoded.size() + 15u) / 16u : 1;
        size_t i = 0;

        for (;;) {
          size_t chunk_size = bl_min<size_t>(encoded.size() - i, max_chunk_size);
          BLResult result = decoder.decode(decoded, BLDataView{encoded.data() + i, chunk_size});

          if (result == BL_SUCCESS)
            break;

          if (result == BL_ERROR_DATA_TRUNCATED) {
            i += chunk_size;
            if (i < encoded.size())
              continue;
          }

          EXPECT_SUCCESS(result).
            message("Decompression failed (%s/%s): input.size=%zu encoded.size=%zu decoded.size=%zu (first mismatching byte at %zd)",
              stringify_strategy(strategy),
              test_data_name,
              input.size,
              encoded.size(),
              decoded.size(),
              compare_decoded_data(input.data, decoded.data(), bl_min(input.size, decoded.size())));
        }
      }
    }

    size_t mismatch_index = compare_decoded_data(input.data, decoded.data(), bl_min(input.size, decoded.size()));
    EXPECT_EQ(input.size, decoded.size())
      .message("Input size and decoded size don't match (%s/%s): input.size=%zu encoded.size=%zu decoded.size=%zu (first mismatching byte at %zd)",
        stringify_strategy(strategy),
        test_data_name,
        input.size,
        encoded.size(),
        decoded.size(),
        mismatch_index);

    EXPECT_EQ(mismatch_index, SIZE_MAX)
      .message("Decoded data is invalid (%s/%s) at offset=%zu (decoded=0x%02X expected=0x%02X)",
        stringify_strategy(strategy),
        test_data_name,
        mismatch_index,
        input[mismatch_index],
        decoded[mismatch_index]);
  }
}

static void test_deflate_litrunlen(uint32_t compression_level) noexcept {
  // The content of this test comes from a libdeflate test - `test_litrunlen_overflow.c`.
  //
  // Try to compress a file longer than 65535 bytes where no 2-byte sequence (3 would be sufficient) is
  // repeated <= 32768 bytes apart, and the distribution of bytes remains constant throughout, and yet
  // not all bytes are used so the data is still slightly compressible. There will be no matches in this
  // data, but the compressor should still output a compressed block, and this block should contain more
  // than 65535 consecutive literals, which triggered the bug.
  //
  // Note: on random data, this situation is extremely unlikely if the compressor uses all matches it
  // finds, since random data will on average have a 3-byte match every (256**3)/32768 = 512 bytes.
  BLArray<uint8_t> arr;

  for (uint32_t i = 0; i < 2; i++) {
    for (uint32_t stride = 1; stride < 251u; stride++) {
      for (uint32_t multiple = 0; multiple < 251u; multiple++) {
        arr.append(uint8_t((stride * multiple) % 251u));
      }
    }
  }

  test_deflate_roundtrip(arr.view(), Deflate::FormatType::kRaw, compression_level, "litrunlen");
}

static void test_deflate_random_data(size_t min_bytes, size_t max_bytes, size_t size_increment, uint32_t compression_level, TestRandomMode random_mode) noexcept {
  BLArray<uint8_t> input;

  for (size_t n = min_bytes; n <= max_bytes; n += size_increment) {
    input.reset();

    // In case of a bug in the compressor/decompressor, uncomment the following to quickly find the right input.
    // INFO("Testing %zu bytes", n);

    BLRandom rnd(0x1234u + n * 33u);
    EXPECT_SUCCESS(append_random_bytes(input, rnd, n, random_mode));

    test_deflate_roundtrip(input.view(), Deflate::FormatType::kRaw, compression_level, stringify_random_mode(random_mode));
  }
}

UNIT(compression_deflate, BL_TEST_GROUP_COMPRESSION_ALGORITHM) {
  INFO("Testing basic deflate tests");

  test_deflate_empty_offset_code();
  test_deflate_singleton_litrunlen_code();
  test_deflate_singleton_offset_code();
  test_deflate_singleton_offset_code_notsymzero();
  test_deflate_too_many_codeword_lengths();
  test_deflate_overread();
  test_deflate_invalid_streams();

  for (uint32_t level = 0; level <= Deflate::kMaxCompressionLevel; level++) {
    INFO("Testing deflate round-trip compression/decompression with level %u", level);

    test_deflate_litrunlen(level);
    test_deflate_random_data(1, 2000, 1, level, TestRandomMode::kRandomDataWithRepeats);
    test_deflate_random_data(1, 2000, 1, level, TestRandomMode::kRandomDataWithNibbles);
    test_deflate_random_data(1, 2000, 1, level, TestRandomMode::kRandomDataWithTwoLiterals);
    test_deflate_random_data(1, 2000, 1, level, TestRandomMode::kAllZeros);

    test_deflate_random_data(2000, 5000, 13, level, TestRandomMode::kRandomDataWithRepeats);
    test_deflate_random_data(2000, 5000, 17, level, TestRandomMode::kRandomDataWithNibbles);
    test_deflate_random_data(2000, 5000, 23, level, TestRandomMode::kRandomDataWithTwoLiterals);
    test_deflate_random_data(2000, 5000, 27, level, TestRandomMode::kAllZeros);

    test_deflate_random_data(5000, 10000, 133, level, TestRandomMode::kRandomDataWithRepeats);
    test_deflate_random_data(5000, 10000, 187, level, TestRandomMode::kRandomDataWithNibbles);
    test_deflate_random_data(5000, 10000, 571, level, TestRandomMode::kRandomDataWithTwoLiterals);
    test_deflate_random_data(5000, 10000, 666, level, TestRandomMode::kAllZeros);

    test_deflate_random_data(10000, 500000, 33333, level, TestRandomMode::kRandomDataWithRepeats);
    test_deflate_random_data(10000, 500000, 36666, level, TestRandomMode::kRandomDataWithNibbles);
    test_deflate_random_data(10000, 500000, 76643, level, TestRandomMode::kRandomDataWithTwoLiterals);
    test_deflate_random_data(10000, 500000, 99999, level, TestRandomMode::kAllZeros);

    test_deflate_random_data(1000000, 5000000, 1691939, level, TestRandomMode::kRandomDataWithRepeats);
    test_deflate_random_data(1000000, 5000000, 1491931, level, TestRandomMode::kRandomDataWithNibbles);
  }
}

} // {bl::Compression::Tests}

#endif // BL_TEST
