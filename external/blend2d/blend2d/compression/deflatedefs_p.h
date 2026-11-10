// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_DEFLATEDEFS_P_H_INCLUDED
#define BLEND2D_COMPRESSION_DEFLATEDEFS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array.h>

//! \cond INTERNAL

namespace bl::Compression::Deflate {

enum class FormatType : uint32_t {
  kRaw,
  kZlib
};

//! Block type.
enum class BlockType : uint32_t {
  kUncompressed = 0,
  kStaticHuffman = 1,
  kDynamicHuffman = 2
};

//! Minimum supported match length (in bytes).
static constexpr uint32_t kMinMatchLen = 3;
//! Maximum supported match length (in bytes).
static constexpr uint32_t kMaxMatchLen = 258;

//! Minimum supported match offset (in bytes).
static constexpr uint32_t kMinMatchOffset = 1;
//! Maximum supported match offset (in bytes).
static constexpr uint32_t kMaxMatchOffset = 32768;

//! Maximum window size.
static constexpr uint32_t kMaxWindowSize = 32768;

// Number of symbols in each Huffman code.
//
// NOTE for the literal/length and offset codes, these are actually the
// maximum values; a given block might use fewer symbols.
static constexpr uint32_t kNumPrecodeSymbols = 19;
static constexpr uint32_t kNumLitLenSymbols = 288;
static constexpr uint32_t kNumOffsetSymbols = 32;

// Division of symbols in the literal/length code
static constexpr uint32_t kNumLiterals = 256;
static constexpr uint32_t kEndOfBlock = 256;
static constexpr uint32_t kFirstLengthSymbol = 257;
static constexpr uint32_t kNumLengthSymbols = 31;

//! The maximum number of symbols across all codes.
static constexpr uint32_t kMaxSymbolCount = bl_max(bl_max(kNumPrecodeSymbols, kNumLitLenSymbols), kNumOffsetSymbols);

// Maximum codeword length, in bits, within each Huffman code.
static constexpr uint32_t kMaxPreCodeWordLen = 7;
static constexpr uint32_t kMaxLitLenCodeWordLen = 15;
static constexpr uint32_t kMaxOffsetCodeWordLen = 15;

//! The maximum codeword length across all codes.
static constexpr uint32_t kMaxCodeWordLen = 15;
// Maximum number of extra bits that may be required to represent a match length.
static constexpr uint32_t kMaxExtraLengthBits = 5;
// Maximum number of extra bits that may be required to represent a match offset.
static constexpr uint32_t kMaxExtraOffsetBits = 13;

//! Maximum possible overrun when decoding codeword lengths.
static constexpr uint32_t kMaxLensOverrun = 137;

// The maximum number of bits in which a match can be represented. This is the absolute worst case,
// which assumes the longest possible Huffman codewords and the maximum numbers of extra bits.
static constexpr uint32_t kMaxMatchBits = kMaxLitLenCodeWordLen + kMaxExtraLengthBits + kMaxOffsetCodeWordLen + kMaxExtraOffsetBits;

// The order in which precode lengths are stored.
extern const uint8_t kPrecodeLensPermutation[kNumPrecodeSymbols];

#define DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))

static BL_INLINE uint32_t loaded_u32_to_u24(uint32_t v) noexcept {
  return BL_BYTE_ORDER == 1234 ? v & 0xFFFFFFu : v >> 8;
}

} // {bl::Compression::Deflate}

//! \endcond

#endif // BLEND2D_COMPRESSION_DEFLATEDEFS_P_H_INCLUDED
