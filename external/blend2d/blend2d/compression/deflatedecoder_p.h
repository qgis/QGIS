// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_DEFLATEDECODER_P_H_INCLUDED
#define BLEND2D_COMPRESSION_DEFLATEDECODER_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array.h>
#include <blend2d/compression/deflatedefs_p.h>

//! \cond INTERNAL

// #define BL_COMPRESSION_STATISTICS

#ifdef BL_COMPRESSION_STATISTICS
  #define BL_DECODER_UPDATE_STATISTICS(...) __VA_ARGS__
#else
  #define BL_DECODER_UPDATE_STATISTICS(...)
#endif

namespace bl::Compression::Deflate {

// Forward declarations.
class Decoder;

//! Deflate decoder state.
enum class DecoderState : uint32_t {
  kDecompressHuffmanBlock,
  kCopyUncompressedBlock,

  kDecompressHuffmanInterruptedMatch,

  kZlibHeader,
  kBlockHeader,
  kUncompressedHeader,
  kStaticHuffmanHeader,
  kDynamicHuffmanHeader,
  kDynamicHuffmanPreCodeLens,
  kDynamicHuffmanLitLenOffsetCodes,

  kDone,
  kInvalid
};

//! Deflate decoder flags.
enum class DecoderFlags : uint8_t {
  //! No flags.
  kNone = 0u,

  //! Decompressing a final block (after the block ends the decompression is done).
  kFinalBlock = 0x01u,

  //! Static Huffman tables are active, which means they don't have to be recreated in case that additional static
  //! Huffman block is encountered immediately after the previous block. This idea comes originally from libdeflate.
  kStaticTableActive = 0x02u,

  kOptimizedTableActive = 0x04u
};

BL_DEFINE_ENUM_FLAGS(DecoderFlags);

//! Deflate decoder options that can be set by users.
enum class DecoderOptions : uint8_t {
  //! No options.
  kNone = 0,

  //! The output buffer has enough capacity for the decoded stream, thus the decoder should never realloc.
  kNeverReallocOutputBuffer = 0x01u
};

BL_DEFINE_ENUM_FLAGS(DecoderOptions);

// Number of "fast" bits we use for each of the Deflate Huffman codes, along with their corresponding ENOUGH values,
// which represent the size of each table including all subtables (ENOUGH values were computed using the utility
// program 'enough' from Zlib).
//
// Zlib treats its equivalents of TABLE_BITS as maximum values; whenever it builds a table, it caps the actual table
// bits to the longest codeword. This makes sense in theory, as there's no need for the table to be any larger than
// needed to support the longest codeword. However, having the table bits be a compile-time constant is beneficial to
// the performance of the decode loop, so there is a trade-off. Using dynamic table bits for the litlen table comes
// from libdeflate, due to its larger maximum size.
//
// Each TABLE_BITS value has a corresponding ENOUGH value that gives the worst-case maximum number of decode table
// entries, including the main table and all subtables. The `enough` value depends on three parameters:
//
//  (1) the maximum number of symbols in the code
//  (2) the maximum number of main table bits
//  (3) the maximum allowed codeword length

// For the precode, we use `kDecoderPrecodeTableBits == 7` since this is the maximum precode codeword length. This
// avoids ever needing subtables.
static constexpr uint32_t kDecoderPrecodeTableBits = 7;
static constexpr uint32_t kDecoderPrecodeTableEntries = 128; // ./enough 19 7 7.

// For the litlen and offset codes, we cannot realistically avoid ever needing subtables, since litlen and offset
// codewords can be up to 15 bits. Having more bits reduces the number of lookups that need a subtable, which
// increases performance; however, it increases memory usage and makes building the table take much longer, which
// decreases performance. We choose values that work well in practice, making subtables rarely needed without making
// the tables too large.
static constexpr uint32_t kDecoderLitLenTableBits = 11;
static constexpr uint32_t kDecoderLitLenTableEntries = 2342; // ./enough 288 11 15.

// static constexpr uint32_t kDecoderOffsetTableBits = 8;
// static constexpr uint32_t kDecoderOffsetTableEntries = 402;  // ./enough 32 8 15.

static constexpr uint32_t kDecoderOffsetTableBits = 9;
static constexpr uint32_t kDecoderOffsetTableEntries = 594;  // ./enough 32 9 15.

//! Decode entry is an entry in a deflate decode table, which represents either a static or dynamic Huffman table.
//! The reason why to put the value into a struct is purely readability, to not confuse any other value with this
//! entry.
//!
//! The following constants were designed in a way to maximize the performance of DecodeEntry processing. The
//! values were designed in a way that a fast literal is always checked first, so literals don't need any form
//! of compatibility with other codes, just a quick check for identifying them. Then when the decoder doesn't
//! match a literal, it should be able to unconditionally process a subtable - hence both length and end of block
//! entries can be treated as sub-tables as well. Once a subtable is decoded the decoder has to check again for
//! literal, offset, and end of block.
//!
//! - LitLen Table:
//!
//!   - Single/Multi-Literal (Non-Subtable) Entry:
//!      - Bits [31-28] -  4 bits - full bit-length of the first literal entry.
//!      - Bits [27   ] -  1 bit  - literal flag (set).
//!      - Bits [26   ] -  1 bit  - offset & length flag (unset).
//!      - Bits [25   ] -  1 bit  - offset | length flag (unset).
//!      - Bits [24   ] -  1 bit  - end of block flag (unset).
//!      - Bits [23-08] - 16 bits - one or two literals (up to 2 bytes).
//!      - Bits [07-06] -  2 bits - number of literals (1 or 2).
//!      - Bits [05-00] -  6 bits - full bit-length of either one or two literals.
//!
//!   - Single Literal (Subtable) Entry:
//!      - Bits [31-28] -  4 bits - full bit-length of the literal entry.
//!      - Bits [27   ] -  1 bit  - literal flag (set).
//!      - Bits [26   ] -  1 bit  - offset & length flag (unset).
//!      - Bits [25   ] -  1 bit  - offset | length flag (unset).
//!      - Bits [24   ] -  1 bit  - end of block flag (unset).
//!      - Bits [23-16] -  8 bits - always zero.
//!      - Bits [15-08] -  8 bits - literal value (always one literal).
//!      - Bits [07-06] -  2 bits - number of literals (always 1).
//!      - Bits [05-00] -  6 bits - full bit-length of the literal entry.
//!
//!   - Length Entry:
//!      - Bits [31-28] -  4 bits - base bit-length of the length entry (without extra bits).
//!      - Bits [27   ] -  1 bit  - literal flag (unset).
//!      - Bits [26   ] -  1 bit  - offset & length flag (unset).
//!      - Bits [25   ] -  1 bit  - offset | length flag (set).
//!      - Bits [24   ] -  1 bit  - end of block flag (unset).
//!      - Bits [23   ] -  1 bit  - always zero.
//!      - Bits [12-08] - 15 bits - base length value (9 bits used).
//!      - Bits [07-06] -  2 bits - always zero.
//!      - Bits [05-00] -  6 bits - full bit-length of length entry.
//!
//!   - End of Block Entry:
//!      - Bits [31-28] -  4 bits - base bit-length of the end-of-block entry (zero if top entry).
//!      - Bits [27   ] -  1 bit  - literal flag (unset).
//!      - Bits [26   ] -  1 bit  - offset & length flag (unset).
//!      - Bits [25   ] -  1 bit  - offset | length flag (unset).
//!      - Bits [24   ] -  1 bit  - end of block flag (set).
//!      - Bits [23   ] -  1 bit  - zero if valid end-of-block, non-zero if this entry is invalid.
//!      - Bits [22-08] - 15 bits - always zero.
//!      - Bits [07-06] -  2 bits - always zero.
//!      - Bits [05-00] -  6 bits - full bit-length of end-of-block entry.
//!
//!   - Subtable Pointer:
//!      - Bits [31-28] -  4 bits - base bit-length excluding the number of subtable index bits.
//!      - Bits [27   ] -  1 bit  - literal flag (unset).
//!      - Bits [26   ] -  1 bit  - offset & length flag (unset).
//!      - Bits [25   ] -  1 bit  - offset | length flag (unset).
//!      - Bits [24   ] -  1 bit  - end of block flag (unset).
//!      - Bits [23   ] -  1 bits - always zero.
//!      - Bits [23-08] - 15 bits - subtable start (base) index (12 bits used).
//!      - Bits [07-06] -  2 bits - always zero.
//!      - Bits [05-00] -  6 bits - full bit-length including the number of subtable index bits.
//!
//! - Offset Table:
//!
//!   - Offset Entry:
//!      - Bits [31-28] -  4 bits - base bit-length of the offset entry excluding extra bits.
//!      - Bits [27   ] -  1 bit  - literal flag (unset).
//!      - Bits [26   ] -  1 bit  - offset & length flag (unset).
//!      - Bits [25   ] -  1 bit  - offset | length flag (set).
//!      - Bits [24   ] -  1 bit  - end of block flag (unset).
//!      - Bits [23   ] -  1 bits - always zero.
//!      - Bits [22-08] - 15 bits - offset value.
//!      - Bits [07-06] -  2 bits - always zero.
//!      - Bits [05-00] -  6 bits - full bit-length of the offset entry including extra bits.
//!
//!   - Subtable pointer:
//!      - Bits [31-28] -  4 bits - base bit-length excluding the number of subtable index bits.
//!      - Bits [27   ] -  1 bit  - literal flag (unset).
//!      - Bits [26   ] -  1 bit  - offset & length flag (unset).
//!      - Bits [25   ] -  1 bit  - offset | length flag (unset).
//!      - Bits [24   ] -  1 bit  - end of block flag (unset).
//!      - Bits [23   ] -  1 bits - always zero.
//!      - Bits [22-08] - 15 bits - subtable start (base) index (12 bits used).
//!      - Bits [07-06] -  2 bits - always zero.
//!      - Bits [05-00] -  6 bits - full bit-length including the number of subtable index bits.
//!
//! - Precode Table
//!
//!   - Precode Entry
//!      - Bits [31-28] -  4 bits - pre-code base length bits.
//!      - Bits [27-24] -  4 bits - always zero.
//!      - Bits [23-16] -  8 bits - pre-code repeat.
//!      - Bits [15-08] -  8 bits - pre-code value (only 5 bits used).
//!      - Bits [07-00] -  8 bits - pre-code entry bit-length including extra length bits.
struct DecodeEntry {
  uint32_t value;

  //! Shifts and sizes of packed decode entry values.
  enum Constants : uint32_t {
    // All Entries
    // -----------

    kFullLengthOffset      = 0,    //!< Entry Full length (including extra) bit-offset.
    kFullLengthNBits       = 8,    //!< Entry Full length (including extra) bit-length.

    kBaseLengthOffset      = 28,   //!< Entry Base length (excluding extra) bit-offset.
    kBaseLengthNBits       = 4,    //!< Entry Base length (excluding extra) bit-length.

    kPayloadOffset         = 8,    //!< entry payload bit-offset (shared between offset, length, and sub-table handling).
    kPayloadNBits          = 15,   //!< entry payload bit-length (shared between offset, length, and sub-table handling).

    kEndOfBlockInvalidFlag = 1u << 23,
    kEndOfBlockFlag        = 1u << 24,
    kOffOrLenFlag          = 1u << 25,
    kOffAndLenFlag         = 1u << 26,
    kLiteralFlag           = 1u << 27,

    // Precode Entry
    // -------------

    kPrecodeValueOffset    = 8,    //!< Precode value bit-offset.
    kPrecodeValueNBits     = 8,    //!< Precode value bit-length (only 5 bits used)

    kPrecodeRepeatOffset   = 16,   //!< Precode repeat bit-offset.
    kPrecodeRepeatNBits    = 8,    //!< Precode repeat bit-length (only 4 bits used).

    // Offset Entry
    // ------------

    // Offset base value is compatible with entry payload.
    kOffsetBaseValueOffset = 8,    //!< Base offset value bit-offset in decode entry.
    kOffsetBaseValueNBits  = 15,   //!< Base offset value bit-size in decode entry.

    // LitLen Entry
    // ------------

    kLiteralCountOffset    = 6,    //!< Literal count offset (3 bits at this offset).

    kSubTableLiteralOffset = 8,    //!< Literal bit-offset in a sub-table.
    kSubTableLiteralNBits  = 8     //!< Literal bit-length in a sub-table
  };
};

template<typename Entry, uint32_t Size>
struct DecodeTable {
  Entry entries[Size];

  BL_INLINE_NODEBUG Entry& operator[](size_t index) noexcept {
    BL_ASSERT(index < Size);
    return entries[index];
  }

  BL_INLINE_NODEBUG const Entry& operator[](size_t index) const noexcept {
    BL_ASSERT(index < Size);
    return entries[index];
  }
};

struct DecodeTables {
  // The arrays aren't all needed at the same time. 'precode_lens' and 'precode_decode_table' are unneeded after
  // 'lens' has been filled. Furthermore, 'lens' need not be retained after building the litlen and offset decode
  // tables. In fact, 'lens' can be in union with 'litlen_decode_table' provided that 'offset_decode_table' is
  // separate and is built first.
  union {
    uint8_t precode_lens[kNumPrecodeSymbols];
    struct {
      uint8_t lens[kNumLitLenSymbols + kNumOffsetSymbols + kMaxLensOverrun];
      DecodeTable<DecodeEntry, kDecoderPrecodeTableEntries> precode_decode_table;
    };
    struct {
      DecodeTable<DecodeEntry, kDecoderLitLenTableEntries> litlen_decode_table;
    };
  };

  DecodeTable<DecodeEntry, kDecoderOffsetTableEntries> offset_decode_table;
};

//! Information of a decode table that the decoder can take advantage of.
struct DecodeTableInfo {
  //! The number of table bits (table size in bits) - if this value is 0 the table is invalid.
  uint8_t table_bits;
  //! Maximum codeword bit-length.
  uint8_t max_code_len;
  //! Mask of lengths of all symbols below 256 (used to build a multi-literal table metadata).
  uint16_t literal_mask;
};

enum class DecoderFastStatus : uint32_t {
  kOk,
  kBlockDone,
  kInvalidData
};

struct DecoderFastResult {
  DecoderFastStatus status;

  uint8_t* dst_ptr;
  const uint8_t* src_ptr;
};

typedef DecoderFastResult (BL_CDECL* DecoderFastFunc)(
  Decoder* ctx,
  uint8_t* dst_start,
  uint8_t* dst_ptr,
  uint8_t* dst_end,
  const uint8_t* src_ptr,
  const uint8_t* src_end
) noexcept;

//! Decompression context
class Decoder {
public:
  DecoderState _state {};                 // Decoder state - it's stateful to support data streaming.
  DecoderFlags _flags {};                 // Decoder flags - for example last block flag.
  DecoderOptions _options {};             // Decoder options.

  BLBitWord _bit_word {};                 // Current bit-buffer data (always persisted to support streaming).
  size_t _bit_length {};                  // Number of bits in `_bit_word` data.
  size_t _copy_remaining {};              // Valid when copying uncompressed data (DecoderState::kCopyUncompressedBlock).

  uint32_t _litlen_symbol_count {};       // Number of litlen symbols in the current Huffman block.
  uint32_t _offset_symbol_count {};       // Number of offset symbols in the current Huffman block.

  uint32_t _work_index {};                // Work index used during decode table construction (meaning depends on state).
  uint32_t _work_count {};                // Number of work items indexed by _work_index (meaning depends on state).
  uint64_t _processed_bytes {};           // Total number of processed input bytes.

  DecodeTableInfo _precode_table_info {}; // Precode table information.
  DecodeTableInfo _offset_table_info {};  // Offset table information.
  DecodeTableInfo _litlen_table_info {};  // LitLen table information.
  uint32_t _litlen_fast_table_bits {};    // Number of bits used by the fast table.

#ifdef BL_COMPRESSION_STATISTICS
  struct Statistics {
    struct Stream {
      uint64_t dynamic_block_count;
      uint64_t static_block_count;
    } stream;
    struct Fast {
      uint64_t num_restarts;
      uint64_t num_iterations;
      uint64_t quick_literal_entries;
      uint64_t quick_literal_loops;
      uint64_t match_entries;
      uint64_t match_loops;
      uint64_t match_bails_because_of_literal;
      uint64_t match_bails_because_of_sub_offset;
      uint64_t match_near;
      uint64_t match_up_to_8;
      uint64_t match_up_to_16;
      uint64_t match_up_to_32;
      uint64_t match_up_to_64;
      uint64_t match_more_than_8;
      uint64_t match_more_than_16;
      uint64_t match_more_than_32;
      uint64_t match_more_than_64;
      uint64_t subtable_lookups;
      uint64_t subtable_literal_entries;
      uint64_t subtable_offset_entries;
      uint64_t subtable_length_entries;
    } fast;
    struct Tail {
      uint64_t num_restarts;
      uint64_t num_iterations;
      uint64_t quick_literal_entries;
      uint64_t match_entries;
      uint64_t subtable_lookups;
      uint64_t subtable_literal_entries;
      uint64_t subtable_offset_entries;
      uint64_t subtable_length_entries;
    } tail;
  } statistics {};
#endif // BL_COMPRESSION_STATISTICS

#if BL_TARGET_ARCH_BITS >= 64
  DecoderFastFunc _fast_decode_func {};
#endif // BL_TARGET_ARCH_BITS

  DecodeTables tables;

  BLResult init(FormatType format, DecoderOptions options = DecoderOptions::kNone) noexcept;
  BLResult decode(BLArray<uint8_t>& dst, BLDataView input) noexcept;
};

} // {bl::Compression::Deflate}

//! \endcond

#endif // BLEND2D_COMPRESSION_DEFLATEDECODER_P_H_INCLUDED
