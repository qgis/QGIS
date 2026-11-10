// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// Based on code written in 2014-2016 by Eric Biggers <ebiggers3@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright
// and related and neighboring rights to this software to the public domain
// worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//
// In addition the following libdeflate commits have been used for the purpose of bug-fixing:
//   #f2f0df7 [2017] - deflate_compress: fix corruption with long literal run

#include <blend2d/core/api-build_p.h>
#include <blend2d/compression/checksum_p.h>
#include <blend2d/compression/deflatedefs_p.h>
#include <blend2d/compression/deflateencoder_p.h>
#include <blend2d/compression/deflateencoderutils_p.h>
#include <blend2d/compression/matchfinder_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl::Compression::Deflate {

// bl::Compression::Deflate::Encoder - Options & Settings
// ======================================================

// One is subtracted from this table, which then forms the real value.
static constexpr uint8_t kMinimumInputSizeToCompress[13] = {
  0,      // Level #0 (underflows to SIZE_MAX when 1 is subtracted when it's zero extended to size_t).
  1 + 60, // Level #1
  1 + 55, // Level #2
  1 + 50, // Level #3
  1 + 45, // Level #4
  1 + 40, // Level #5
  1 + 35, // Level #6
  1 + 30, // Level #7
  1 + 25, // Level #8
  1 + 20, // Level #9
  1 + 16, // Level #10
  1 + 12, // Level #11
  1 + 8   // Level #12
};

struct EncoderCompressionOptions {
  uint16_t max_search_depth;
  uint16_t nice_match_length;
  uint16_t optimal_passes;
};

static constexpr EncoderCompressionOptions kEncoderCompressionOptions[] = {
  // MaxDepth | NiceMatchLength | Passes
  {         0 ,               0 , 0 }, // Compression level #00 (None).

  {         2 ,               8 , 0 }, // Compression level #01 (Greedy).
  {         6 ,              10 , 0 }, // Compression level #02 (Greedy).
  {        12 ,              14 , 0 }, // Compression level #03 (Greedy).
  {        16 ,              30 , 0 }, // Compression level #04 (Greedy).

  {        16 ,              30 , 0 }, // Compression level #05 (Lazy).
  {        35 ,              65 , 0 }, // Compression level #06 (Lazy).
  {       100 ,             130 , 0 }, // Compression level #07 (Lazy).

  {        12 ,              20 , 1 }, // Compression level #08 (NearOptimal).
  {        16 ,              26 , 2 }, // Compression level #09 (NearOptimal).
  {        30 ,              50 , 2 }, // Compression level #10 (NearOptimal).
  {        60 ,              80 , 3 }, // Compression level #11 (NearOptimal).
  {       100 ,             133 , 4 }  // Compression level #12 (NearOptimal).
};

// bl::Compression::Deflate::Encoder - Constants
// =============================================

// The compressor always chooses a block of at least kEncoderMinBlockLength bytes, except if the last block has
// to be shorter.
static constexpr uint32_t kEncoderMinBlockLength = 10000u;

// The compressor attempts to end blocks after kEncoderSoftMaxBlockLength bytes, but the final length might be
// slightly longer due to matches extending beyond this limit.
static constexpr uint32_t kEncoderSoftMaxBlockLength = 300000u;

// The number of observed matches or literals that represents sufficient data to decide whether the current block
// should be terminated or not.
static constexpr uint32_t kEncoderNumObservationsPerBlockCheck = 512u;

// These are the compressor-side limits on the codeword lengths for each Huffman code. To make outputting bits
// slightly faster, some of these limits are lower than the limits defined by the DEFLATE format. This does not
// significantly affect the compression ratio, at least for the block lengths we use.
static constexpr uint32_t kEncoderMaxLitlenCodewordLen = 14;

// Constants specific to the near-optimal parsing algorithm.

// The maximum number of matches the matchfinder can find at a single position. Since the matchfinder never finds
// more than one match for the same length, presuming one of each possible length is sufficient for an upper bound.
// This says nothing about whether it is worthwhile to consider so many matches; this is just defining the worst
// case.
static constexpr uint32_t kEncoderMaxMatchesPerPos = kMaxMatchLen - kMinMatchLen + 1u;

// The number of lz_match structures in the match cache, excluding the extra "overflow" entries. This value should
// be high enough so that nearly the time, all matches found in a given block can fit in the match cache. However,
// fallback behavior (immediately terminating the block) on cache overflow is still required.
static constexpr uint32_t kEncoderMatchCacheLength = kEncoderSoftMaxBlockLength * 5u;

// The NoStatBits value for a given alphabet is the number of bits assumed to be needed to output a symbol that
// was unused in the previous optimization pass. Assigning a default cost allows the symbol to be used in the next
// optimization pass. However, the cost should be relatively high because the symbol probably won't be used very
// many times (if at all).
static constexpr uint32_t kLiteralNoStatBits = 13;
static constexpr uint32_t kLengthNoStatBits = 13;
static constexpr uint32_t kOffsetNoStatBits = 10;

// bl::Compression::Deflate::Encoder - Tables
// ==========================================

static const uint8_t kDeflateMinOutputSizeByFormat[] = {
  0,    // RAW  - no extra size.
  2 + 4 // ZLIB - 2 bytes header and 4 bytes ADLER32 checksum.
};

static const uint8_t kDeflateExtraPrecodeBitCount[kNumPrecodeSymbols] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 7
};

// Length slot => length slot base value.
static const uint32_t kEncoderLengthSlotBase[29] = {
  3   , 4   , 5   , 6   , 7   , 8   , 9   , 10  ,
  11  , 13  , 15  , 17  , 19  , 23  , 27  , 31  ,
  35  , 43  , 51  , 59  , 67  , 83  , 99  , 115 ,
  131 , 163 , 195 , 227 , 258
};

// Length slot => number of extra length bits.
static const uint8_t kEncoderExtraLengthBitCount[29] = {
  0   , 0   , 0   , 0   , 0   , 0   , 0   , 0 ,
  1   , 1   , 1   , 1   , 2   , 2   , 2   , 2 ,
  3   , 3   , 3   , 3   , 4   , 4   , 4   , 4 ,
  5   , 5   , 5   , 5   , 0
};

// Length => Length slot.
static const uint8_t kEncoderLengthSlotLUT[kMaxMatchLen + 1] = {
  0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 12,
  12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16,
  16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18,
  18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20,
  20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
  21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
  22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
  23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25,
  25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
  25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26,
  26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
  26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 28
};

// Offset slot => offset slot base value.
static const uint32_t kEncoderOffsetSlotBase[30] = {
  1    , 2    , 3    , 4     , 5     , 7     , 9     , 13    ,
  17   , 25   , 33   , 49    , 65    , 97    , 129   , 193   ,
  257  , 385  , 513  , 769   , 1025  , 1537  , 2049  , 3073  ,
  4097 , 6145 , 8193 , 12289 , 16385 , 24577
};

// Offset slot => number of extra offset bits.
static const uint8_t kEncoderExtraOffsetBitCount[30] = {
  0    , 0    , 0    , 0     , 1     , 1     , 2     , 2     ,
  3    , 3    , 4    , 4     , 5     , 5     , 6     , 6     ,
  7    , 7    , 8    , 8     , 9     , 9     , 10    , 10    ,
  11   , 11   , 12   , 12    , 13    , 13
};

// Ported from libdeflate #88d45c7 and #a50c51b
//
// Table: 'offset - 1 => offset_slot' for offset <= 256
struct DeflateOffsetSlotGen {
  static constexpr size_t value(size_t index) noexcept {
    return index + 1 < 2     ?  1 - 1 :
           index + 1 < 3     ?  2 - 1 :
           index + 1 < 4     ?  3 - 1 :
           index + 1 < 5     ?  4 - 1 :
           index + 1 < 7     ?  5 - 1 :
           index + 1 < 9     ?  6 - 1 :
           index + 1 < 13    ?  7 - 1 :
           index + 1 < 17    ?  8 - 1 :
           index + 1 < 25    ?  9 - 1 :
           index + 1 < 33    ? 10 - 1 :
           index + 1 < 49    ? 11 - 1 :
           index + 1 < 65    ? 12 - 1 :
           index + 1 < 97    ? 13 - 1 :
           index + 1 < 129   ? 14 - 1 :
           index + 1 < 193   ? 15 - 1 :
           index + 1 < 257   ? 16 - 1 : 0;
  }
};
static constexpr LookupTable<uint8_t, 256> kEncoderOffsetSlotLUT = make_lookup_table<uint8_t, 256, DeflateOffsetSlotGen>();

// Offset => Offset Slot.
static BL_INLINE uint32_t deflate_get_offset_slot(uint32_t offset) noexcept {
  // 1 <= offset <= 32768 here. For 1 <= offset <= 256, kEncoderOffsetSlotLUT[offset - 1] gives the slot.
  //
  // For 257 <= offset <= 32768, we take advantage of the fact that 257 is the beginning of slot 16, and
  // each slot [16..30) is exactly 1 << 7 == 128 times larger than each slot [2..16) (since the number of
  // extra bits increases by 1 every 2 slots). Thus, the slot is:
  //
  //   kEncoderOffsetSlotLUT[2 + ((offset - 257) >> 7)] + (16 - 2) == kEncoderOffsetSlotLUT[((offset - 1) >> 7)] + 14
  //
  // Define 'n = (offset <= 256) ? 0 : 7'. Then any offset is handled by:
  //
  //   kEncoderOffsetSlotLUT[(offset - 1) >> n] + (n << 1)
  //
  // For better performance, replace 'n = (offset <= 256) ? 0 : 7' with the equivalent (for offset <= 536871168)
  // 'n = (256 - offset) >> 29'.
  uint32_t n = (256 - offset) >> 29;
  return kEncoderOffsetSlotLUT[(offset - 1) >> n] + (n << 1);
}

// bl::Compression::Deflate::Encoder - Structs
// ===========================================

//! Codewords for the DEFLATE Huffman codes.
struct CodeWords {
  uint32_t litlen[kNumLitLenSymbols];
  uint32_t offset[kNumOffsetSymbols];
};

//! Codeword lengths (in bits) for the DEFLATE Huffman codes.
//!
//! A zero length means the corresponding symbol had zero frequency.
struct Lens {
  uint8_t litlen[kNumLitLenSymbols];
  uint8_t offset[kNumOffsetSymbols];
};

//! Codewords and lengths for the DEFLATE Huffman codes.
struct Codes {
  CodeWords codewords;
  Lens lens;
};

//! Symbol frequency counters for the DEFLATE Huffman codes.
struct Freqs {
  uint32_t litlen[kNumLitLenSymbols];
  uint32_t offset[kNumOffsetSymbols];
};

//! Costs for the near-optimal parsing algorithm.
struct Costs {
  //! The cost to output each possible literal.
  uint32_t literal[kNumLiterals];

  //! The cost to output each possible match length.
  uint32_t length[kMaxMatchLen + 1];

  //! The cost to output a match offset of each possible offset slot.
  uint32_t offset_slot[kNumOffsetSymbols];
};

//! Represents a run of literals followed by a match or end-of-block. This struct is needed to temporarily store
//! items chosen by the parser, since items cannot be written until all items for the block have been chosen and
//! the block's Huffman codes have been computed.
struct Sequence {
  // Bits 0..22: the number of literals in this run. This may be 0 and can be at most about `kEncoderSoftMaxBlockLength`.
  // The literals are not stored explicitly in this structure; instead, they are read directly from the uncompressed data.
  //
  // Bits 23..31: the length of the match which follows the literals, or 0 if this literal run was the last in the block,
  // so there is no match which follows it.
  uint32_t litrunlen_and_length;

  // If 'length' doesn't indicate end-of-block, then this is the offset of the match which follows the literals.
  uint16_t offset;

  //! If 'length' doesn't indicate end-of-block, then this is the offset symbol of the match which follows the literals.
  uint8_t offset_symbol;

  //! If 'length' doesn't indicate end-of-block, then this is the length slot of the match which follows the literals.
  uint8_t length_slot;
};

static constexpr uint32_t kNOOptimumOffsetShift = 9u;
static constexpr uint32_t kNOOptimumLengthMask = (1u << kNOOptimumOffsetShift) - 1;

//! Represents a byte position in the input data and a node in the graph of possible match/literal choices for the current
//! block.
//!
//! Logically, each incoming edge to this node is labeled with a literal or a match that can be taken to reach this position
//! from an earlier position; and each outgoing edge from this node is labeled with a literal or a match that can be taken to
//! advance from this position to a later position.
//!
//! But these "edges" are actually stored elsewhere (in 'match_cache').
struct OptimumNode {
  //! The minimum cost to reach the end of the block from this position.
  uint32_t cost_to_end;

  //! Represents the literal or match that must be chosen from here to reach the end of the block with the minimum cost.
  //! Equivalently, this can be interpreted as the label of the outgoing edge on the minimum-cost path to the "end of block"
  //! node from this node.
  //!
  //! Notes on the match/literal representation used here:
  //!   - The low bits of 'item' are the length: 1 if this is a literal, or the match length if this is a match.
  //!   - The high bits of 'item' are the actual literal byte if this is a literal, or the match offset if this is a match.
  uint32_t item;
};

// Block split statistics. See "Block splitting algorithm" below.
static constexpr uint32_t NUM_LITERAL_OBSERVATION_TYPES = 8u;
static constexpr uint32_t NUM_MATCH_OBSERVATION_TYPES = 2u;
static constexpr uint32_t NUM_OBSERVATION_TYPES = NUM_LITERAL_OBSERVATION_TYPES + NUM_MATCH_OBSERVATION_TYPES;

struct BlockSplitStats {
  uint32_t new_observations[NUM_OBSERVATION_TYPES];
  uint32_t observations[NUM_OBSERVATION_TYPES];
  uint32_t num_new_observations;
  uint32_t num_observations;
};

// bl::Compression::Deflate::Encoder - Encoder Impl
// ================================================

// Deflate encoder implementation.
struct EncoderImpl {
  using PrepareFunc = void (BL_CDECL*)(EncoderImpl* impl) noexcept;
  using CompressFunc = size_t (BL_CDECL*)(EncoderImpl* impl, const uint8_t *, size_t, uint8_t *, size_t) noexcept;

  //! The pointer, which must be freed in order to free the Impl, because of an alignment requirement of Impl.
  void* allocated_ptr;

  // Format type.
  FormatType format;
  // The compression level with which this compressor was created.
  uint32_t compression_level;
  // Minimum input size to actually attempt to compress it (depends on compression level).
  size_t min_input_size;

  // Pointer to the prepare() implementation.
  PrepareFunc prepare_func;
  // Pointer to the compress() implementation.
  CompressFunc compress_func;

  // Frequency counters for the current block.
  Freqs freqs;
  // Dynamic Huffman codes for the current block.
  Codes codes;
  // Static Huffman codes.
  Codes static_codes;
  // Block split statistics for the currently pending block.
  BlockSplitStats split_stats;

  // The "nice" match length: if a match of this length is found, choose it immediately without further
  // consideration.
  uint32_t nice_match_length;

  // The maximum search depth: consider at most this many potential matches at each position.
  uint32_t max_search_depth;

  // Precode space.
  struct Precode {
    uint32_t freqs[kNumPrecodeSymbols];
    uint8_t lens[kNumPrecodeSymbols];
    uint32_t codewords[kNumPrecodeSymbols];
    uint32_t items[kNumLitLenSymbols + kNumOffsetSymbols];
    uint32_t litlen_symbol_count;
    uint32_t offset_symbol_count;
    uint32_t explicit_len_count;
    uint32_t item_count;
  };

  // Temporary space for Huffman code output.
  Precode precode;
};

struct GreedyEncoderImpl : public EncoderImpl {
  // Hash chains matchfinder.
  hc_matchfinder hc_mf;

  // The matches and literals that the parser has chosen for the current
  // block. The required length of this array is limited by the maximum
  // number of matches that can ever be chosen for a single block, plus one
  // for the special entry at the end.
  Sequence sequences[DIV_ROUND_UP(kEncoderSoftMaxBlockLength, kMinMatchLen) + 1];
};

struct NearOptimalEncoderImpl : public EncoderImpl {
  uint32_t num_optim_passes;

  // Binary tree matchfinder.
  bt_matchfinder bt_mf;

  // Cached matches for the current block. This array contains the matches that were found at each position in the
  // block. Specifically, for each position, there is a list of matches found at that position, if any, sorted by
  // strictly increasing length. In addition, following the matches for each position, there is a special 'lz_match'
  // whose 'length' member contains the number of matches found at that position, and whose 'offset' member contains
  // the literal at that position.
  //
  // Note: in rare cases, there will be a very high number of matches in the block and this array will overflow. If
  // this happens, we force the end of the current block. kEncoderMatchCacheLength is the length at which we actually
  // check for overflow. The extra slots beyond this are enough to absorb the worst case overflow, which occurs if
  // starting at `&match_cache[kEncoderMatchCacheLength - 1]`, we write `kEncoderMaxMatchesPerPos` matches and a match
  // count header, then skip searching for matches at 'kMaxMatchLen - 1' positions and write the match count header for
  // each.
  lz_match match_cache[kEncoderMatchCacheLength + kEncoderMaxMatchesPerPos + kMaxMatchLen - 1];

  // Array of nodes, one per position, for running the  minimum-cost path algorithm.
  //
  // This array must be large enough to accommodate the worst-case number of nodes, which occurs if we find
  // a match of length kMaxMatchLen at position kEncoderSoftMaxBlockLength - 1, producing a block of length
  // `kEncoderSoftMaxBlockLength - 1 + kMaxMatchLen`. Add one for the end-of-block node.
  OptimumNode optimum_nodes[kEncoderSoftMaxBlockLength - 1 + kMaxMatchLen + 1];

  // The current cost model being used.
  Costs costs;

  // A table that maps match offset to offset slot. This differs from kEncoderOffsetSlotLUT[] in that this is a full
  // map, not a condensed one. The full map is more appropriate for the near-optimal parser, since the near-optimal
  // parser does more offset => offset_slot translations, it doesn't intersperse them with matchfinding (so cache
  // evictions are less of a concern), and it uses more memory anyway.
  uint8_t offset_slot_full[kMaxMatchOffset + 1];
};

// bl::Compression::Deflate::Encoder - Heap
// ========================================

// Given the binary tree node A[subtree_idx] whose children already satisfy the maxheap property, swap the node with
// its greater child until it is greater than both its children, so that the maxheap property is satisfied in the
// subtree rooted at A[subtree_idx].
static void heapify_subtree(uint32_t* A, uint32_t length, uint32_t subtree_idx) noexcept {
  uint32_t v = A[subtree_idx];
  uint32_t parent_idx = subtree_idx;
  uint32_t child_idx;

  while ((child_idx = parent_idx * 2) <= length) {
    if (child_idx < length && A[child_idx + 1] > A[child_idx])
      child_idx++;
    if (v >= A[child_idx])
      break;
    A[parent_idx] = A[child_idx];
    parent_idx = child_idx;
  }
  A[parent_idx] = v;
}

// Rearrange the array 'A' so that it satisfies the maxheap property. 'A' uses 1-based indices, so the children of A[i]
// are A[i*2] and A[i*2 + 1].
static void heapify_array(uint32_t* A, uint32_t length) noexcept {
  for (uint32_t subtree_idx = length / 2; subtree_idx >= 1; subtree_idx--) {
    heapify_subtree(A, length, subtree_idx);
  }
}

// Sort the array 'A', which contains 'length' uint32_t 32-bit integers.
//
// Note: name this function heap_sort() instead of heapsort() to avoid colliding with heapsort() from stdlib.h on
// BSD-derived systems --- though this isn't necessary when compiling with -D_ANSI_SOURCE, which is the better solution.
static void heap_sort(uint32_t* A, uint32_t length) noexcept {
  A--; // Use 1-based indices.

  heapify_array(A, length);

  while (length >= 2) {
    uint32_t tmp = A[length];
    A[length] = A[1];
    A[1] = tmp;
    length--;
    heapify_subtree(A, length, 1);
  }
}

// bl::Compression::Deflate::Encoder - Huffman Tree Building
// =========================================================

static constexpr uint32_t NUM_SYMBOL_BITS = 10;
static constexpr uint32_t SYMBOL_MASK = (1u << NUM_SYMBOL_BITS) - 1u;

// Sort the symbols primarily by frequency and secondarily by symbol value. Discard symbols with zero frequency and
// fill in an array with the remaining symbols, along with their frequencies. The low NUM_SYMBOL_BITS bits of each
// array entry will contain the symbol value, and the remaining bits will contain the frequency.
//
// \param num_syms
//     Number of symbols in the alphabet. Can't be greater than (1 << NUM_SYMBOL_BITS).
//
// \param freqs[num_syms]
//     The frequency of each symbol.
//
// \param lens[num_syms]
//     An array that eventually will hold the length of each codeword. This function only fills in the codeword
//     lengths for symbols that have zero frequency, which are not well defined per se but will be set to 0.
//
// \param symout[num_syms]
//     The output array, described above.
//
// Returns the number of entries in 'symout' that were filled. This is the number of symbols that have
// nonzero frequency.
static uint32_t sort_symbols(uint32_t num_syms, const uint32_t* BL_RESTRICT freqs, uint8_t* BL_RESTRICT lens, uint32_t* BL_RESTRICT symout) noexcept {
  uint32_t sym;

  uint32_t num_counters = num_syms;
  uint32_t counters[kMaxSymbolCount] {};

  // Count the frequencies.
  for (sym = 0; sym < num_syms; sym++)
    counters[bl_min(freqs[sym], num_counters - 1)]++;

  // Make the counters cumulative, ignoring the zero-th, which counted symbols with zero
  // frequency. As a side effect, this calculates the number of symbols with nonzero frequency.
  uint32_t num_used_syms = 0;
  for (uint32_t i = 1; i < num_counters; i++) {
    uint32_t count = counters[i];
    counters[i] = num_used_syms;
    num_used_syms += count;
  }

  // Sort nonzero-frequency symbols using the counters. At the same time, set the codeword
  // lengths of zero-frequency symbols to 0.
  for (sym = 0; sym < num_syms; sym++) {
    uint32_t freq = freqs[sym];
    if (freq != 0)
      symout[counters[bl_min(freq, num_counters - 1)]++] = sym | (freq << NUM_SYMBOL_BITS);
    else
      lens[sym] = 0;
  }

  // Sort the symbols counted in the last counter.
  heap_sort(symout + counters[num_counters - 2],
      counters[num_counters - 1] - counters[num_counters - 2]);

  return num_used_syms;
}

// Build the Huffman tree.
//
// This is an optimized implementation that
//  (a) takes advantage of the frequencies being already sorted;
//  (b) only generates non-leaf nodes, since the non-leaf nodes of a Huffman tree are sufficient to generate
//      a canonical code;
//  (c) Only stores parent pointers, not child pointers;
//  (d) Produces the nodes in the same memory used for input frequency information.
//
// Array 'A', which contains 'sym_count' entries, is used for both input and output. For this function,
// 'sym_count' must be at least 2.
//
// For input, the array must contain the frequencies of the symbols, sorted in increasing order. Specifically,
// each entry must contain a frequency left shifted by NUM_SYMBOL_BITS bits. Any data in the low NUM_SYMBOL_BITS
// bits of the entries will be ignored by this function. Although these bits will, in fact, contain the symbols
// that correspond to the frequencies, this function is concerned with frequencies only and keeps the symbols
// as-is.
//
// For output, this function will produce the non-leaf nodes of the Huffman tree. These nodes will be stored in
// the first `(sym_count - 1)` entries of the array. Entry `A[sym_count - 2]` will represent the root node. Each
// other node will contain the zero-based index of its parent node in `A`, left shifted by `NUM_SYMBOL_BITS` bits.
// The low `NUM_SYMBOL_BITS` bits of each entry in A will be kept as-is. Again, note that although these low bits
// will, in fact, contain a symbol value, this symbol will have *no relationship* with the Huffman tree node that
// happens to occupy the same slot. This is because this implementation only generates the non-leaf nodes of the
// tree.
static void build_tree(uint32_t* A, uint32_t sym_count) noexcept {
  uint32_t i = 0u; // Index, in `A`, of next lowest frequency symbol that has not yet been processed.
  uint32_t b = 0u; // Index, in `A`, of next lowest frequency parentless non-leaf node; or, if equal to `e`,
                   // then no such node exists yet.
  uint32_t e = 0u; // Index, in `A`, of next node to allocate as a non-leaf.

  do {
    // Choose the two next lowest frequency entries.
    uint32_t m = (i != sym_count && (b == e || (A[i] >> NUM_SYMBOL_BITS) <= (A[b] >> NUM_SYMBOL_BITS))) ? i++ : b++;
    uint32_t n = (i != sym_count && (b == e || (A[i] >> NUM_SYMBOL_BITS) <= (A[b] >> NUM_SYMBOL_BITS))) ? i++ : b++;

    // Allocate a non-leaf node and link the entries to it.
    //
    // If we link an entry that we're visiting for the first  time (via index `i`), then we're actually linking
    // a leaf node and it will have no effect, since the leaf will be overwritten with a non-leaf when index `e`
    // catches up to it. But it's not any slower to unconditionally set the parent index.
    //
    // We also compute the frequency of the non-leaf node as the sum of its two children's frequencies.
    uint32_t freq_shifted = (A[m] & ~SYMBOL_MASK) + (A[n] & ~SYMBOL_MASK);
    A[m] = (A[m] & SYMBOL_MASK) | (e << NUM_SYMBOL_BITS);
    A[n] = (A[n] & SYMBOL_MASK) | (e << NUM_SYMBOL_BITS);
    A[e] = (A[e] & SYMBOL_MASK) | freq_shifted;
    e++;
  } while (sym_count - e > 1);
  // When just one entry remains, it is a "leaf" that was linked to some other node. We ignore it, since the rest
  // of the array contains the non-leaves which we need. (Note that we're assuming the cases with 0 or 1 symbols
  // were handled separately)
}

// Given the stripped-down Huffman tree constructed by build_tree(), determine the number of codewords that should
// be assigned each possible length, taking into account the length-limited constraint.
//
// \param A
//     The array produced by build_tree(), containing parent index information for the non-leaf nodes of the Huffman
//     tree. Each entry in this array is a node; a node's parent always has a greater index than that node itself.
//     This function will overwrite the parent index information in this array, so essentially it will destroy the
//     tree. However, the data in the low NUM_SYMBOL_BITS of each entry will be preserved.
//
// \param root_idx
//     The 0-based index of the root node in 'A', and consequently one less than the number of tree node entries in
//     'A'. (Or, really 2 less than the actual length of 'A'.)
//
// \param len_counts
//     An array of length ('max_codeword_len' + 1) in which the number of codewords having each `length <= max_codeword_len`
//     will be returned.
//
// \param max_codeword_len
//     The maximum permissible codeword length.
static void compute_length_counts(uint32_t* BL_RESTRICT A, uint32_t root_idx, uint32_t* BL_RESTRICT len_counts, uint32_t max_codeword_len) noexcept {
  // The key observations are:
  //
  // (1) We can traverse the non-leaf nodes of the tree, always visiting a parent before its children, by simply
  //     iterating through the array in reverse order. Consequently, we can compute the depth of each node in one
  //     pass, overwriting the parent indices with depths.
  //
  // (2) We can initially assume that in the real Huffman tree, both children of the root are leaves. This corresponds
  //     to two codewords of length 1. Then, whenever we visit a (non-leaf) node during the traversal, we modify this
  //     assumption to account for the current node *not* being a leaf, but rather its two children being leaves.
  //     This causes the loss of one codeword for the current depth and the addition of two codewords for the current
  //     depth plus one.
  //
  // (3) We can handle the length-limited constraint fairly easily by simply using the largest length available when a
  //     depth exceeds max_codeword_len.

  for (uint32_t len = 0; len <= max_codeword_len; len++)
    len_counts[len] = 0;
  len_counts[1] = 2;

  // Set the root node's depth to 0.
  A[root_idx] &= SYMBOL_MASK;

  for (int node = int(root_idx) - 1; node >= 0; node--) {
    // Calculate the depth of this node.
    uint32_t parent = A[node] >> NUM_SYMBOL_BITS;
    uint32_t parent_depth = A[parent] >> NUM_SYMBOL_BITS;
    uint32_t depth = parent_depth + 1;
    uint32_t len = depth;

    // Set the depth of this node so that it is available when its children (if any) are processed.
    A[node] = (A[node] & SYMBOL_MASK) | (depth << NUM_SYMBOL_BITS);

    // If needed, decrease the length to meet the length-limited constraint. This is not the optimal
    // method for generating length-limited Huffman codes! But it should be good enough.
    if (len >= max_codeword_len) {
      len = max_codeword_len;
      do {
        len--;
      } while (len_counts[len] == 0);
    }

    // Account for the fact that we have a non-leaf node at the current depth.
    len_counts[len]--;
    len_counts[len + 1] += 2;
  }
}

// Generate the codewords for a canonical Huffman code.
//
// \param A
//     The output array for codewords. In addition, initially this array must contain the symbols, sorted
//     primarily by frequency and secondarily by symbol value, in the low NUM_SYMBOL_BITS bits of each entry.
//
// \param len
//     Output array for codeword lengths.
//
// \param len_counts
//     An array that provides the number of codewords that will have each possible length <= max_codeword_len.
//
// \param max_codeword_len
//     Maximum length, in bits, of each codeword.
//
// \param num_syms
//     Number of symbols in the alphabet, including symbols with zero frequency. This is the length of the 'A'
//     and 'len' arrays.
static void gen_codewords(uint32_t* BL_RESTRICT A, uint8_t* BL_RESTRICT lens, const uint32_t* BL_RESTRICT len_counts, uint32_t max_codeword_len, uint32_t num_syms) noexcept {
  // Given the number of codewords that will have each length, assign codeword lengths to symbols. We do this
  // by assigning the lengths in decreasing order to the symbols sorted primarily by increasing frequency and
  // secondarily by increasing symbol value.
  for (uint32_t i = 0, len = max_codeword_len; len >= 1; len--) {
    uint32_t count = len_counts[len];
    while (count--)
      lens[A[i++] & SYMBOL_MASK] = uint8_t(len);
  }

  // Generate the codewords themselves. We initialize the 'next_codewords' array to provide the lexicographically
  // first codeword of each length, then assign codewords in symbol order. This produces a canonical code.
  uint32_t next_codewords[kMaxCodeWordLen + 1];
  next_codewords[0] = 0;
  next_codewords[1] = 0;
  for (uint32_t len = 2; len <= max_codeword_len; len++) {
    next_codewords[len] = (next_codewords[len - 1] + len_counts[len - 1]) << 1;
  }

  for (uint32_t sym = 0; sym < num_syms; sym++) {
    A[sym] = next_codewords[lens[sym]]++;
  }
}

// bl::Compression::Deflate::Encoder - Huffman Code Building
// =========================================================

// Given an alphabet and the frequency of each symbol in it, construct a length-limited canonical Huffman code.
//
// \param num_syms
//     The number of symbols in the alphabet. The symbols are the integers in the range [0, num_syms - 1]. This
//     parameter must be at least 2 and can't be greater than `(1 << NUM_SYMBOL_BITS)`.
//
// \param max_codeword_len
//     The maximum permissible codeword length.
//
// \param freqs
//     An array of @num_syms entries, each of which specifies the frequency of the corresponding symbol. It is
//     valid for some, none, or all of the frequencies to be 0.
//
// \param lens
//     An array of @num_syms entries in which this function will return the length, in bits, of the codeword
//     assigned to each symbol. Symbols with 0 frequency will not have codewords per se, but their entries in
//     this array will be set to 0. No lengths greater than @max_codeword_len will be assigned.
//
// \param codewords
//     An array of @num_syms entries in which this function will return the codeword for each symbol,r
//     right-justified and padded on the left with zeroes. Codewords for symbols with 0 frequency will
//     be undefined.
//
// This function builds a length-limited canonical Huffman code.
//
// A length-limited Huffman code contains no codewords longer than some specified length, and has exactly (with
// some algorithms) or approximately (with the algorithm used here) the minimum weighted path length from the
// root, given this constraint.
//
// A canonical Huffman code satisfies the properties that a longer codeword never lexicographically precedes a
// shorter codeword, and the lexicographic ordering of codewords of the same length is the same as the lexicographic
// ordering of the corresponding symbols. A canonical Huffman code, or more generally a canonical prefix code, can
// be reconstructed from only a list containing the codeword length of each symbol.
//
// The classic algorithm to generate a Huffman code creates a node for each symbol, then inserts these nodes into
// a min-heap keyed by symbol frequency. Then, repeatedly, the two lowest-frequency nodes are removed from the
// min-heap and added as the children of a new node having frequency equal to the sum of its two children, which is
// then inserted into the min-heap. When only a single node remains in the min-heap, it is the root of the Huffman
// tree. The codeword for each symbol is determined by the path needed to reach the corresponding node from the root.
// Descending to the left child appends a 0 bit, whereas descending to the right child appends a 1 bit.
//
// The classic algorithm is relatively easy to understand, but it is subject to a number of inefficiencies. In
// practice, it is fastest to first sort the symbols by frequency. (This itself can be subject to an optimization
// based on the fact that most frequencies tend to be low.)  At the same time, we sort secondarily by symbol value,
// which aids the process of generating a canonical code. Then, during tree construction, no heap is necessary
// because both the leaf nodes and the unparented non-leaf nodes can be easily maintained in sorted order.
// Consequently, there can never be more than two possibilities for the next-lowest-frequency node.
//
// In addition, because we're generating a canonical code, we actually don't need the leaf nodes of the tree at all,
// only the non-leaf nodes. This is because for canonical code generation we don't need to know where the symbols
// are in the tree. Rather, we only need to know how many leaf nodes have each depth (codeword length). And this
// information can, in fact, be quickly generated from the tree of non-leaves only.
//
// Furthermore, we can build this stripped-down Huffman tree directly in the array in which the codewords are to be
// generated, provided that these array slots are large enough to hold a symbol and frequency value.
//
// Still furthermore, we don't even need to maintain explicit child pointers. We only need the parent pointers, and
// even those can be overwritten in-place with depth information as part of the process of extracting codeword lengths
// from the tree. So in summary, we do NOT need a big structure like:
//
// ```
// struct HuffmanTreeNode {
//   uint32_t int symbol;
//   uint32_t int frequency;
//   uint32_t int depth;
//   HuffmanTreeNode* left_child;
//   HuffmanTreeNode* right_child;
// };
// ```
//
// ... which often gets used in "naive" implementations of Huffman code generation.
//
// Many of these optimizations are based on the implementation in 7-Zip (source file: C/HuffEnc.c), which has been
// placed in the public domain by Igor Pavlov.
static void make_canonical_huffman_code(uint32_t num_syms, uint32_t max_codeword_len, const uint32_t* BL_RESTRICT freqs, uint8_t* BL_RESTRICT lens, uint32_t* BL_RESTRICT codewords) noexcept {
  BL_STATIC_ASSERT(kMaxSymbolCount <= 1 << NUM_SYMBOL_BITS);

  // We begin by sorting the symbols primarily by frequency and secondarily by symbol value. As an optimization,
  // the array used for this purpose ('A') shares storage with the space in which we will eventually return the
  // codewords.
  uint32_t num_used_syms = sort_symbols(num_syms, freqs, lens, codewords);

  // 'num_used_syms' is the number of symbols with nonzero frequency. This may be less than @num_syms. `num_used_syms`
  // is also the number of entries in 'A' that are valid. Each entry consists of a distinct symbol and a non-zero
  // frequency packed into a 32-bit integer.

  // A complete Huffman code must contain at least 2 codewords. Yet, it's possible that fewer than 2 symbols were
  // used. When this happens, it's usually for the offset code (0-1 symbols used). But it's also theoretically
  // possible for the litlen and precodes (1 symbol used).
  //
  // The DEFLATE RFC explicitly allows the offset code to contain just 1 codeword, or even be completely empty.
  // But it's silent about the other codes. It also doesn't say whether, in the 1-codeword case, the codeword
  // (which it says must be 1 bit) is '0' or '1'.
  //
  // In any case, some DEFLATE decompressors reject these cases. Zlib generally allows them, but it does reject
  // precodes that have just 1 codeword. More problematically, Zlib v1.2.1 and earlier rejected empty offset codes,
  // and this behavior can also be seen in other software.
  //
  // Other DEFLATE compressors, including zlib, always send at least 2 codewords in order to make a complete Huffman
  // code. Therefore, this is a case where practice does not entirely match the specification. We follow practice by
  // generating 2 codewords of length 1: codeword '0' for symbol 0, and codeword '1' for another symbol - the used
  // symbol if it exists and is not symbol 0, otherwise symbol 1. This does worsen the compression ratio by having
  // to store 1-2 unnecessary offset codeword lengths. But this only affects rare cases such as blocks containing
  // all literals, and it only makes a tiny difference.
  if (BL_UNLIKELY(num_used_syms < 2u)) {
    uint32_t sym = num_used_syms ? (codewords[0] & SYMBOL_MASK) : 0;
    uint32_t nonzero_idx = sym ? sym : 1;

    codewords[0] = 0;
    lens[0] = 1;
    codewords[nonzero_idx] = 1;
    lens[nonzero_idx] = 1;
    return;
  }

  // Build a stripped-down version of the Huffman tree, sharing the array 'A' with
  // the symbol values. Then extract length counts from the tree and use them to
  // generate the final codewords.
  build_tree(codewords, num_used_syms);

  {
    uint32_t len_counts[kMaxCodeWordLen + 1];
    compute_length_counts(codewords, num_used_syms - 2, len_counts, max_codeword_len);
    gen_codewords(codewords, lens, len_counts, max_codeword_len, num_syms);
  }
}

// Clear the Huffman symbol frequency counters.
//
// This must be called when starting a new DEFLATE block.
static BL_INLINE void reset_symbol_frequencies(EncoderImpl* impl) noexcept {
  memset(&impl->freqs, 0, sizeof(impl->freqs));
}

// Reverse the Huffman codeword 'codeword', which is 'len' bits in length.
static BL_INLINE uint32_t reverse16_bit_code(uint32_t codeword, uint32_t len) noexcept {
  // The following branchless algorithm is faster than going bit by bit.
  //
  // NOTE: since no codewords are longer than 16 bits, we only need to
  // reverse the low 16 bits of the 'uint32_t'.
  BL_STATIC_ASSERT(kMaxCodeWordLen <= 16);

  codeword = ((codeword & 0x5555) << 1) | ((codeword & 0xAAAA) >> 1);
  codeword = ((codeword & 0x3333) << 2) | ((codeword & 0xCCCC) >> 2);
  codeword = ((codeword & 0x0F0F) << 4) | ((codeword & 0xF0F0) >> 4);
  codeword = ((codeword & 0x00FF) << 8) | ((codeword & 0xFF00) >> 8);

  // Return the high `len` bits of the bit-reversed 16-bit value.
  return codeword >> (16 - len);
}

// Make a canonical Huffman code with bit-reversed codewords.
static BL_NOINLINE void deflate_make_huffman_code(
  uint32_t num_syms,
  uint32_t max_codeword_len,
  const uint32_t freqs[],
  uint8_t lens[],
  uint32_t codewords[]
) noexcept {
  make_canonical_huffman_code(num_syms, max_codeword_len, freqs, lens, codewords);
  uint32_t sym = 0;

  if constexpr (sizeof(BLBitWord) >= 8) {
    // THEORETICAL: Reversing 4x16-bit integers at a time requires codeword to be max 16-bits long.
    BL_STATIC_ASSERT(kMaxCodeWordLen <= 16);

    uint32_t fast_reverse_count = num_syms / 4u;
    while (fast_reverse_count) {
      uint32_t c0 = codewords[sym + 0];
      uint32_t c1 = codewords[sym + 1];
      uint32_t c2 = codewords[sym + 2];
      uint32_t c3 = codewords[sym + 3];

      uint64_t bits = ((uint64_t(c0) <<  0) | (uint64_t(c1) << 16)) |
                      ((uint64_t(c2) << 32) | (uint64_t(c3) << 48)) ;

      bits = ((bits & 0x5555555555555555u) << 1) | ((bits & 0xAAAAAAAAAAAAAAAAu) >> 1);
      bits = ((bits & 0x3333333333333333u) << 2) | ((bits & 0xCCCCCCCCCCCCCCCCu) >> 2);
      bits = ((bits & 0x0F0F0F0F0F0F0F0Fu) << 4) | ((bits & 0xF0F0F0F0F0F0F0F0u) >> 4);
      bits = ((bits & 0x00FF00FF00FF00FFu) << 8) | ((bits & 0xFF00FF00FF00FF00u) >> 8);

      c0 = uint32_t((bits >>  0) & 0xFFFFu) >> (16u - lens[sym + 0]);
      c1 = uint32_t((bits >> 16) & 0xFFFFu) >> (16u - lens[sym + 1]);
      c2 = uint32_t((bits >> 32) & 0xFFFFu) >> (16u - lens[sym + 2]);
      c3 = uint32_t((bits >> 48) & 0xFFFFu) >> (16u - lens[sym + 3]);

      codewords[sym + 0] = c0;
      codewords[sym + 1] = c1;
      codewords[sym + 2] = c2;
      codewords[sym + 3] = c3;

      sym += 4;
      fast_reverse_count--;
    }
  }

  while (sym < num_syms) {
    codewords[sym] = reverse16_bit_code(codewords[sym], lens[sym]);
    sym++;
  }
}

// Build the literal/length and offset Huffman codes for a DEFLATE block.
//
// This takes as input the frequency tables for each code and produces as output
// a set of tables that map symbols to codewords and codeword lengths.
static BL_NOINLINE void deflate_make_huffman_codes(const Freqs *freqs, Codes *codes) noexcept {
  BL_STATIC_ASSERT(kEncoderMaxLitlenCodewordLen <= kMaxLitLenCodeWordLen);

  deflate_make_huffman_code(kNumLitLenSymbols, kEncoderMaxLitlenCodewordLen, freqs->litlen, codes->lens.litlen, codes->codewords.litlen);
  deflate_make_huffman_code(kNumOffsetSymbols, kMaxOffsetCodeWordLen, freqs->offset, codes->lens.offset, codes->codewords.offset);
}

// Initialize impl->static_codes.
static BL_NOINLINE void init_static_codes(EncoderImpl* impl) noexcept {
  uint32_t i;

  for (i = 0; i < 144; i++)
    impl->freqs.litlen[i] = 1 << (9 - 8);
  for (; i < 256; i++)
    impl->freqs.litlen[i] = 1 << (9 - 9);
  for (; i < 280; i++)
    impl->freqs.litlen[i] = 1 << (9 - 7);
  for (; i < 288; i++)
    impl->freqs.litlen[i] = 1 << (9 - 8);

  for (i = 0; i < 32; i++)
    impl->freqs.offset[i] = 1 << (5 - 5);

  deflate_make_huffman_codes(&impl->freqs, &impl->static_codes);
}

static uint32_t deflate_compute_precode_items(
  const uint8_t* BL_RESTRICT lens,
  const uint32_t num_lens,
  uint32_t* BL_RESTRICT precode_freqs,
  uint32_t* BL_RESTRICT precode_items
) noexcept {
  uint32_t* itemptr = precode_items;
  uint32_t run_start = 0;

  memset(precode_freqs, 0, kNumPrecodeSymbols * sizeof(precode_freqs[0]));

  do {
    // Find the next run of codeword lengths.

    // len = the length being repeated.
    uint8_t len = lens[run_start];

    // Extend the run.
    uint32_t run_end = run_start;
    do {
      run_end++;
    } while (run_end != num_lens && len == lens[run_end]);

    if (len == 0) {
      // Run of zeroes.

      // Symbol 18: RLE 11 to 138 zeroes at a time.
      while ((run_end - run_start) >= 11) {
        uint32_t extra_bits = bl_min<uint32_t>((run_end - run_start) - 11, 0x7F);
        precode_freqs[18]++;
        *itemptr++ = 18 | (extra_bits << 5);
        run_start += 11 + extra_bits;
      }

      // Symbol 17: RLE 3 to 10 zeroes at a time.
      if ((run_end - run_start) >= 3) {
        uint32_t extra_bits = bl_min<uint32_t>((run_end - run_start) - 3, 0x7);
        precode_freqs[17]++;
        *itemptr++ = 17 | (extra_bits << 5);
        run_start += 3 + extra_bits;
      }
    }
    else {
      // A run of nonzero lengths.

      // Symbol 16: RLE 3 to 6 of the previous length.
      if ((run_end - run_start) >= 4) {
        precode_freqs[len]++;
        *itemptr++ = len;
        run_start++;
        do {
          uint32_t extra_bits = bl_min<uint32_t>((run_end - run_start) - 3, 0x3);
          precode_freqs[16]++;
          *itemptr++ = 16 | (extra_bits << 5);
          run_start += 3 + extra_bits;
        } while ((run_end - run_start) >= 3);
      }
    }

    // Output any remaining lengths without RLE.
    while (run_start != run_end) {
      precode_freqs[len]++;
      *itemptr++ = len;
      run_start++;
    }
  } while (run_start != num_lens);

  return uint32_t(itemptr - precode_items);
}

// Huffman codeword lengths for dynamic Huffman blocks are compressed using a separate Huffman code, the "precode",
// which contains a symbol for each possible codeword length in the larger code as well as several special symbols
// to represent repeated codeword lengths (a form of run-length encoding). The precode is itself constructed in
// canonical form, and its codeword lengths are represented literally in 19 3-bit fields that immediately precede
// the compressed codeword lengths of the larger code.

// Precompute the information needed to output Huffman codes.
static void deflate_precompute_huffman_header(EncoderImpl* impl) noexcept {
  EncoderImpl::Precode& precode = impl->precode;

  // Compute how many litlen and offset symbols are needed.
  for (precode.litlen_symbol_count = kNumLitLenSymbols; precode.litlen_symbol_count > 257; precode.litlen_symbol_count--) {
    if (impl->codes.lens.litlen[precode.litlen_symbol_count - 1] != 0) {
      break;
    }
  }

  for (precode.offset_symbol_count = kNumOffsetSymbols; precode.offset_symbol_count > 1; precode.offset_symbol_count--) {
    if (impl->codes.lens.offset[precode.offset_symbol_count - 1] != 0) {
      break;
    }
  }

  // If we're not using the full set of literal/length codeword lengths, then temporarily move the offset codeword
  // lengths over so that the literal/length and offset codeword lengths are contiguous.
  BL_STATIC_ASSERT(offsetof(Lens, offset) == kNumLitLenSymbols);

  if (precode.litlen_symbol_count != kNumLitLenSymbols) {
    memmove((uint8_t *)&impl->codes.lens + precode.litlen_symbol_count, (uint8_t *)&impl->codes.lens + kNumLitLenSymbols, precode.offset_symbol_count);
  }

  // Compute the "items" (RLE / literal tokens and extra bits) with which the codeword lengths in the larger code
  // will be output.
  precode.item_count = deflate_compute_precode_items((uint8_t *)&impl->codes.lens, precode.litlen_symbol_count + precode.offset_symbol_count, precode.freqs, precode.items);

  // Build the precode.
  deflate_make_huffman_code(kNumPrecodeSymbols, kMaxPreCodeWordLen, precode.freqs, precode.lens, precode.codewords);

  // Count how many precode lengths we actually need to output.
  for (precode.explicit_len_count = kNumPrecodeSymbols; precode.explicit_len_count > 4; precode.explicit_len_count--) {
    if (precode.lens[kPrecodeLensPermutation[precode.explicit_len_count - 1]] != 0) {
      break;
    }
  }

  // Restore the offset codeword lengths if needed.
  if (precode.litlen_symbol_count != kNumLitLenSymbols) {
    memmove((uint8_t *)&impl->codes.lens + kNumLitLenSymbols, (uint8_t *)&impl->codes.lens + precode.litlen_symbol_count, precode.offset_symbol_count);
  }
}

// bl::Compression::Deflate::Encoder - Uncompressed Blocks
// =======================================================

static void write_uncompressed_blocks(OutputStream& os, const uint8_t* data, size_t data_size, bool is_final) noexcept {
  BL_ASSERT(os.bits.was_properly_flushed());

  OutputBits bits = os.bits;
  OutputBuffer buf = os.buffer;

  size_t block_size = bl_min<size_t>(data_size, 0xFFFFu);
  uint32_t block_is_final = uint32_t(is_final && data_size == block_size);

  // The first uncompressed block header must use the remaining BYTE (if any). All consecutive block headers always
  // start with new BYTE (as uncompressed data is not a bit-stream, it's a byte-stream, so it ends on a byte boundary).
  bits.add(block_is_final, 1);
  bits.add(uint32_t(BlockType::kUncompressed), 2);
  bits.align_to_bytes();
  bits.flush(buf);

  // Aligning to bytes means the bit-buffer must be completely clean.
  BL_ASSERT(bits.length() == 0);
  BL_ASSERT(buf.remaining_bytes() >= 4 + block_size);

  os.bits = bits;

  for (;;) {
    MemOps::storeu_le(buf.ptr, uint16_t(block_size));
    buf.ptr += 2;

    MemOps::storeu_le(buf.ptr, uint16_t(block_size ^ 0xFFFFu));
    buf.ptr += 2;

    memcpy(buf.ptr, data, block_size);
    data += block_size;
    buf.ptr += block_size;

    data_size -= block_size;
    if (data_size == 0) {
      break;
    }

    // Start another block.
    block_size = bl_min<size_t>(data_size, 0xFFFFu);
    block_is_final = uint32_t(is_final && data_size == block_size);

    BL_ASSERT(buf.remaining_bytes() >= 5 + block_size);
    buf.ptr[0] = uint8_t(block_is_final | (uint32_t(BlockType::kUncompressed) << 1));
    buf.ptr++;
  }

  os.buffer.ptr = buf.ptr;
}

// bl::Compression::Deflate::Encoder - Block Writing
// =================================================

// Choose the best type of block to use (dynamic Huffman, static Huffman, or uncompressed), then output it.
static void flush_block(EncoderImpl* impl, OutputStream& os, const uint8_t* BL_RESTRICT block_begin, uint32_t block_length, bool is_final_block, bool use_item_list) noexcept {
  BL_ASSERT(os.bits.was_properly_flushed());

  // Costs are measured in bits.
  uint32_t static_cost = 0;
  uint32_t dynamic_cost = 0;

  // Tally the end-of-block symbol.
  impl->freqs.litlen[kEndOfBlock]++;

  // Build dynamic Huffman codes.
  deflate_make_huffman_codes(&impl->freqs, &impl->codes);

  // Account for the cost of sending dynamic Huffman codes.
  deflate_precompute_huffman_header(impl);
  dynamic_cost += 5 + 5 + 4 + (3 * impl->precode.explicit_len_count);

  for (uint32_t sym = 0; sym < kNumPrecodeSymbols; sym++) {
    uint32_t extra = kDeflateExtraPrecodeBitCount[sym];
    dynamic_cost += impl->precode.freqs[sym] * (extra + impl->precode.lens[sym]);
  }

  // Account for the cost of encoding literals.
  uint32_t staticLen8 = 0;
  for (uint32_t sym = 0; sym < 144; sym++) {
    staticLen8 += impl->freqs.litlen[sym];
    dynamic_cost += impl->freqs.litlen[sym] * impl->codes.lens.litlen[sym];
  }

  uint32_t staticLen9 = 0;
  for (uint32_t sym = 144; sym < 256; sym++) {
    staticLen9 += impl->freqs.litlen[sym];
    dynamic_cost += impl->freqs.litlen[sym] * impl->codes.lens.litlen[sym];
  }

  // Account for the cost of encoding the end-of-block symbol.
  static_cost += 7u + (staticLen8 * 8u) + (staticLen9 * 9u);
  dynamic_cost += impl->codes.lens.litlen[kEndOfBlock];

  // Account for the cost of encoding lengths.
  for (uint32_t sym = kFirstLengthSymbol; sym < kFirstLengthSymbol + BL_ARRAY_SIZE(kEncoderExtraLengthBitCount); sym++) {
    uint32_t extra = kEncoderExtraLengthBitCount[sym - kFirstLengthSymbol];
    static_cost += impl->freqs.litlen[sym] * (extra + impl->static_codes.lens.litlen[sym]);
    dynamic_cost += impl->freqs.litlen[sym] * (extra + impl->codes.lens.litlen[sym]);
  }

  // Account for the cost of encoding offsets.
  for (uint32_t sym = 0; sym < BL_ARRAY_SIZE(kEncoderExtraOffsetBitCount); sym++) {
    uint32_t extra = kEncoderExtraOffsetBitCount[sym];
    static_cost += impl->freqs.offset[sym] * (extra + 5);
    dynamic_cost += impl->freqs.offset[sym] * (extra + impl->codes.lens.offset[sym]);
  }

  // Compute the cost of using uncompressed blocks.
  uint32_t uncompressed_cost = (IntOps::negate(uint32_t(os.bits.length()) + 3u) & 7u) + 32u + (40u * (DIV_ROUND_UP(block_length, uint32_t(UINT16_MAX)) - 1u)) + (8u * block_length);

  // Choose the cheapest block type.
  uint32_t huffman_cost = bl_min(static_cost, dynamic_cost);
  if (uncompressed_cost < huffman_cost) {
    write_uncompressed_blocks(os, block_begin, block_length, is_final_block);
  }
  else {
    BlockType block_type = static_cost < dynamic_cost ? BlockType::kStaticHuffman : BlockType::kDynamicHuffman;
    Codes& codes = static_cost < dynamic_cost ? impl->static_codes : impl->codes;

    OutputBits bits = os.bits;
    OutputBuffer buf = os.buffer;

    // Output Huffman Block Header
    // ---------------------------

    bits.add(uint32_t(is_final_block), 1);
    bits.add(uint32_t(block_type), 2);

    // Output the Huffman Codes (Dynamic Huffman Blocks Only)
    // ------------------------------------------------------

    if (block_type == BlockType::kDynamicHuffman) {
      const EncoderImpl::Precode& precode = impl->precode;

      // Total bits - header(3) + 5 + 5 + 4 + 2 * 3 -> 22 bits for block header and precode with 2 lens.
      bits.add(precode.litlen_symbol_count - 257u, 5u);
      bits.add(precode.offset_symbol_count - 1u, 5u);
      bits.add(precode.explicit_len_count - 4u, 4u);
      bits.add(precode.lens[kPrecodeLensPermutation[0]], 3);
      bits.add(precode.lens[kPrecodeLensPermutation[1]], 3);
      bits.flush(buf);

      // Output the remaining lens of the codewords in the precode.
      if constexpr (sizeof(BLBitWord) >= 8) {
        // kNumPrecodeSymbols == 19 -> at most (19 - 2) * 3 bits will be written (51 bits).
        for (uint32_t i = 2; i < precode.explicit_len_count; i++) {
          bits.add(precode.lens[kPrecodeLensPermutation[i]], 3);
        }
        bits.flush(buf);
      }
      else {
        for (uint32_t i = 2; i < precode.explicit_len_count; i++) {
          bits.add(precode.lens[kPrecodeLensPermutation[i]], 3);
          bits.flush(buf);
        }
      }

      // Output the encoded lengths of the codewords in the larger code.
      for (uint32_t i = 0; i < precode.item_count; i++) {
        uint32_t precode_item = precode.items[i];
        uint32_t precode_sym = precode_item & 0x1Fu;
        bits.add(precode.codewords[precode_sym], precode.lens[precode_sym]);

        if (precode_sym >= 16u) {
          if (precode_sym == 16u)
            bits.add(precode_item >> 5, 2);
          else if (precode_sym == 17)
            bits.add(precode_item >> 5, 3);
          else
            bits.add(precode_item >> 5, 7);
        }
        bits.flush(buf);
      }

    }
    else if (static_cost < uncompressed_cost) {
      bits.flush(buf);
    }

    // Output Literals and Matches
    // ---------------------------

    if (!use_item_list) {
      GreedyEncoderImpl* greedy_impl = static_cast<GreedyEncoderImpl*>(impl);
      const Sequence* seq = greedy_impl->sequences;
      const uint8_t* in_next = block_begin;

      for (;;) {
        uint32_t litrunlen = seq->litrunlen_and_length & 0x7FFFFFu;
        uint32_t length = seq->litrunlen_and_length >> 23;

        if (litrunlen) {
          while (litrunlen >= 4) {
            uint32_t lit0 = in_next[0];
            uint32_t lit1 = in_next[1];
            uint32_t lit2 = in_next[2];
            uint32_t lit3 = in_next[3];

            bits.add(codes.codewords.litlen[lit0], codes.lens.litlen[lit0]);
            bits.flushIfCannotBufferN<2 * kEncoderMaxLitlenCodewordLen>(buf);

            bits.add(codes.codewords.litlen[lit1], codes.lens.litlen[lit1]);
            bits.flushIfCannotBufferN<3 * kEncoderMaxLitlenCodewordLen>(buf);

            bits.add(codes.codewords.litlen[lit2], codes.lens.litlen[lit2]);
            bits.flushIfCannotBufferN<4 * kEncoderMaxLitlenCodewordLen>(buf);

            bits.add(codes.codewords.litlen[lit3], codes.lens.litlen[lit3]);
            bits.flush(buf);

            in_next += 4;
            litrunlen -= 4;
          }

          if (litrunlen >= 1u) {
            uint32_t lit0 = in_next[0];
            bits.add(codes.codewords.litlen[lit0], codes.lens.litlen[lit0]);

            if (litrunlen >= 2u) {
              uint32_t lit1 = in_next[1];
              bits.flushIfCannotBufferN<2 * kEncoderMaxLitlenCodewordLen>(buf);
              bits.add(codes.codewords.litlen[lit1], codes.lens.litlen[lit1]);

              if (litrunlen >= 3u) {
                uint32_t lit2 = in_next[2];
                bits.flushIfCannotBufferN<3 * kEncoderMaxLitlenCodewordLen>(buf);
                bits.add(codes.codewords.litlen[lit2], codes.lens.litlen[lit2]);
              }
            }

            bits.flush(buf);
            in_next += litrunlen;
          }
        }

        if (length == 0) {
          break;
        }

        in_next += length;
        uint32_t length_slot = seq->length_slot;
        uint32_t litlen_symbol = kFirstLengthSymbol + length_slot;

        // Match length + extra bits.
        bits.add(codes.codewords.litlen[litlen_symbol], codes.lens.litlen[litlen_symbol]);
        bits.add(length - kEncoderLengthSlotBase[length_slot], kEncoderExtraLengthBitCount[length_slot]);
        bits.flushIfCannotBufferN<kEncoderMaxLitlenCodewordLen + kMaxExtraLengthBits + kMaxOffsetCodeWordLen + kMaxExtraOffsetBits>(buf);

        // Match offset + extra bits.
        uint32_t offset_symbol = seq->offset_symbol;
        bits.add(codes.codewords.offset[offset_symbol], codes.lens.offset[offset_symbol]);
        bits.flushIfCannotBufferN<kMaxOffsetCodeWordLen + kMaxExtraOffsetBits>(buf);
        bits.add(seq->offset - kEncoderOffsetSlotBase[offset_symbol], kEncoderExtraOffsetBitCount[offset_symbol]);
        bits.flush(buf);

        seq++;
      }
    }
    else {
      // Follow the minimum-cost path in the graph of possible match/literal choices for the
      // current block and write out the matches/literals using the specified Huffman codes.
      NearOptimalEncoderImpl* optimal_impl = static_cast<NearOptimalEncoderImpl*>(impl);

      OptimumNode* cur_node = &optimal_impl->optimum_nodes[0];
      OptimumNode* end_node = &optimal_impl->optimum_nodes[block_length];

      do {
        uint32_t length = cur_node->item & kNOOptimumLengthMask;
        uint32_t offset = cur_node->item >> kNOOptimumOffsetShift;

        if (length == 1) {
          // Literal.
          uint32_t litlen_symbol = offset;
          bits.add(codes.codewords.litlen[litlen_symbol], codes.lens.litlen[litlen_symbol]);
          bits.flush(buf);
        }
        else {
          // Match length + extra bits.
          uint32_t length_slot = kEncoderLengthSlotLUT[length];
          uint32_t litlen_symbol = kFirstLengthSymbol + length_slot;

          bits.add(codes.codewords.litlen[litlen_symbol], codes.lens.litlen[litlen_symbol]);
          bits.add(length - kEncoderLengthSlotBase[length_slot], kEncoderExtraLengthBitCount[length_slot]);
          bits.flushIfCannotBufferN<kEncoderMaxLitlenCodewordLen + kMaxExtraLengthBits + kMaxOffsetCodeWordLen>(buf);

          // Match offset + extra bits.
          uint32_t offset_slot = optimal_impl->offset_slot_full[offset];
          bits.add(codes.codewords.offset[offset_slot], codes.lens.offset[offset_slot]);
          bits.flushIfCannotBufferN<kEncoderMaxLitlenCodewordLen + kMaxExtraLengthBits + kMaxOffsetCodeWordLen + kMaxExtraOffsetBits>(buf);
          bits.add(offset - kEncoderOffsetSlotBase[offset_slot], kEncoderExtraOffsetBitCount[offset_slot]);
          bits.flush(buf);
        }
        cur_node += length;
      } while (cur_node != end_node);
    }

    // Output Huffman End of Block
    // ---------------------------

    bits.add(codes.codewords.litlen[kEndOfBlock], codes.lens.litlen[kEndOfBlock]);
    bits.flush(buf);

    os.bits = bits;
    os.buffer.ptr = buf.ptr;
  }
}

static BL_INLINE void choose_literal(EncoderImpl* impl, uint32_t literal, uint32_t* litrunlen_p) noexcept {
  impl->freqs.litlen[literal]++;
  ++*litrunlen_p;
}

static BL_INLINE void choose_match(EncoderImpl* impl, uint32_t length, uint32_t offset, uint32_t* litrunlen_p, Sequence** next_seq_p) noexcept {
  Sequence *seq = *next_seq_p;
  uint32_t length_slot = kEncoderLengthSlotLUT[length];
  uint32_t offset_slot = deflate_get_offset_slot(offset);

  impl->freqs.litlen[257 + length_slot]++;
  impl->freqs.offset[offset_slot]++;

  seq->litrunlen_and_length = (uint32_t(length) << 23) | *litrunlen_p;
  seq->offset = uint16_t(offset);
  seq->length_slot = uint8_t(length_slot);
  seq->offset_symbol = uint8_t(offset_slot);

  *litrunlen_p = 0;
  *next_seq_p = seq + 1;
}

static BL_INLINE void finish_sequence(Sequence *seq, uint32_t litrunlen) noexcept {
  seq->litrunlen_and_length = litrunlen;
}

// Block splitting algorithm. The problem is to decide when it is worthwhile to start a new block with new Huffman
// codes. There is a theoretically optimal solution: recursively consider every possible block split, considering
// the exact cost of each block, and choose the minimum cost approach. But this is far too tail. Instead, as an
// approximation, we can count symbols and after every N symbols, compare the expected distribution of symbols based
// on the previous data with the actual distribution. If they differ "by enough", then start a new block.
//
// As an optimization and heuristic, we don't distinguish between every symbol but rather we combine many symbols
// into a single "observation type". For literals we only look at the high bits and low bits, and for matches we
// only look at whether the match is long or not. The assumption is that for typical "real" data, places that are
// good block boundaries will tend to be noticeable based only on changes in these aggregate frequencies, without
// looking for subtle differences in individual symbols. For example, a change from ASCII bytes to non-ASCII bytes,
// or from few matches (generally less compressible) to many matches (generally more compressible), would be easily
// noticed based on the aggregates.
//
// For determining whether the frequency distributions are "different enough" to start a new block, the simply
// heuristic of splitting when the sum of absolute differences exceeds a constant seems to be good enough. We also
// add a number proportional to the block length so that the algorithm is more likely to end long blocks than short
// blocks. This reflects the general expectation that it will become increasingly beneficial to start a new block
// as the current block grows longer.
//
// Finally, for an approximation, it is not strictly necessary that the exact symbols being used are considered.
// With "near-optimal parsing", for example, the actual symbols that will be used are unknown until after the block
// boundary is chosen and the block has been optimized. Since the final choices cannot be used, we can use
// preliminary "greedy" choices instead.

// Initialize the block split statistics when starting a new block.
static void init_block_split_stats(BlockSplitStats* stats) noexcept {
  uint32_t i;

  for (i = 0; i < NUM_OBSERVATION_TYPES; i++) {
    stats->new_observations[i] = 0;
    stats->observations[i] = 0;
  }
  stats->num_new_observations = 0;
  stats->num_observations = 0;
}

// Literal observation.
//
// Heuristic: use the top 2 bits and low 1 bits of the literal, for 8 possible literal observation types.
static BL_INLINE void observe_literal(BlockSplitStats* stats, uint8_t lit) noexcept {
  stats->new_observations[((lit >> 5) & 0x6) | (lit & 1)]++;
  stats->num_new_observations++;
}

// Match observation.
//
// Heuristic: use one observation type for "short match" and one observation type for "long match".
static BL_INLINE void observe_match(BlockSplitStats* stats, uint32_t length) noexcept {
  stats->new_observations[NUM_LITERAL_OBSERVATION_TYPES + (length >= 9)]++;
  stats->num_new_observations++;
}

static bool do_end_block_check(BlockSplitStats* stats, uint32_t block_length) noexcept {
  uint32_t i;

  if (stats->num_observations > 0) {
    // To avoid slow divisions, we do not divide by `num_observations`, but rather do all math
    // with the numbers multiplied by `num_observations`.
    uint32_t total_delta = 0;

    for (i = 0; i < NUM_OBSERVATION_TYPES; i++) {
      uint32_t expected = stats->observations[i] * stats->num_new_observations;
      uint32_t actual = stats->new_observations[i] * stats->num_observations;
      uint32_t delta = (actual > expected) ? actual - expected : expected - actual;

      total_delta += delta;
    }

    // Ready to end the block?
    if (total_delta + (block_length / 4096) * stats->num_observations >= kEncoderNumObservationsPerBlockCheck * 200 / 512 * stats->num_observations)
      return true;
  }

  for (i = 0; i < NUM_OBSERVATION_TYPES; i++) {
    stats->num_observations += stats->new_observations[i];
    stats->observations[i] += stats->new_observations[i];
    stats->new_observations[i] = 0;
  }

  stats->num_new_observations = 0;
  return false;
}

static BL_INLINE bool should_end_block(BlockSplitStats* stats, const uint8_t* in_block_begin, const uint8_t* in_next, const uint8_t* in_end) noexcept {
  // Ready to check block split statistics?
  if (stats->num_new_observations < kEncoderNumObservationsPerBlockCheck ||
      PtrOps::byte_offset(in_block_begin, in_next) < kEncoderMinBlockLength ||
      PtrOps::bytes_until(in_next, in_end) < kEncoderMinBlockLength) {
    return false;
  }

  return do_end_block_check(stats, uint32_t(in_next - in_block_begin));
}

// bl::Compression::Deflate::Encoder - Prepare
// ===========================================

// Initialize impl->offset_slot_full.
static BL_INLINE void init_offset_slot_full(NearOptimalEncoderImpl* impl) noexcept {
  uint32_t offset_slot;
  uint32_t offset;
  uint32_t offset_end;

  for (offset_slot = 0; offset_slot < BL_ARRAY_SIZE(kEncoderOffsetSlotBase); offset_slot++) {
    offset = kEncoderOffsetSlotBase[offset_slot];
    offset_end = offset + (1u << kEncoderExtraOffsetBitCount[offset_slot]);
    do {
      impl->offset_slot_full[offset] = uint8_t(offset_slot);
    } while (++offset != offset_end);
  }
}

static void BL_CDECL prepare_greedy_or_lazy(EncoderImpl* impl) noexcept {
  init_static_codes(impl);
}

static void BL_CDECL prepare_near_optimal(EncoderImpl* impl) noexcept {
  init_static_codes(impl);
  init_offset_slot_full(static_cast<NearOptimalEncoderImpl*>(impl));
}

// bl::Compression::Deflate::Encoder - Greedy Compressor
// =====================================================

// This is the "greedy" DEFLATE compressor. It always chooses the longest match.
static size_t BL_CDECL compress_greedy(EncoderImpl* impl_, const uint8_t* BL_RESTRICT in, size_t in_nbytes, uint8_t* BL_RESTRICT out, size_t out_nbytes_avail) noexcept {
  GreedyEncoderImpl* impl = static_cast<GreedyEncoderImpl*>(impl_);

  OutputStream os{};
  os.buffer.init(out, out_nbytes_avail);

  const uint8_t* in_next = in;
  const uint8_t* in_end = in_next + in_nbytes;
  const uint8_t* in_cur_base = in_next;

  uint32_t max_len = kMaxMatchLen;
  uint32_t nice_len = bl_min(impl->nice_match_length, max_len);
  uint32_t next_hashes[2] = {0, 0};

  hc_matchfinder_init(&impl->hc_mf);

  do {
    // Starting a new DEFLATE block.
    const uint8_t* in_block_begin = in_next;
    const uint8_t* in_max_block_end = in_next + bl_min<size_t>(PtrOps::bytes_until(in_next, in_end), kEncoderSoftMaxBlockLength);

    uint32_t litrunlen = 0;
    Sequence *next_seq = impl->sequences;

    init_block_split_stats(&impl->split_stats);
    reset_symbol_frequencies(impl);

    do {
      // Decrease the maximum and nice match lengths if we're approaching the end of the input buffer.
      if (BL_UNLIKELY(max_len > PtrOps::bytes_until(in_next, in_end))) {
        max_len = uint32_t(PtrOps::bytes_until(in_next, in_end));
        nice_len = bl_min(nice_len, max_len);
      }

      uint32_t offset;
      uint32_t length = hc_matchfinder_longest_match(&impl->hc_mf, &in_cur_base, in_next, kMinMatchLen - 1, max_len, nice_len, impl->max_search_depth, next_hashes, &offset);

      if (length >= kMinMatchLen) {
        // Match found.
        choose_match(impl, length, offset, &litrunlen, &next_seq);
        observe_match(&impl->split_stats, length);
        in_next = hc_matchfinder_skip_positions(&impl->hc_mf, &in_cur_base, in_next + 1, in_end, length - 1, next_hashes);
      }
      else {
        // No match found.
        choose_literal(impl, *in_next, &litrunlen);
        observe_literal(&impl->split_stats, *in_next);
        in_next++;
      }

      // Check if it's time to output another block.
    } while (in_next < in_max_block_end && !should_end_block(&impl->split_stats, in_block_begin, in_next, in_end));

    finish_sequence(next_seq, litrunlen);
    flush_block(impl, os, in_block_begin, uint32_t(in_next - in_block_begin), in_next == in_end, false);
  } while (in_next != in_end);

  os.bits.flush_final_byte(os.buffer);
  return os.buffer.byte_offset();
}

// bl::Compression::Deflate::Encoder - Lazy Compressor
// ===================================================

// This is the "lazy" DEFLATE compressor. Before choosing a match, it checks to see if there's a longer match at the
// next position. If yes, it outputs a literal and continues to the next position. If no, it outputs the match.
static size_t BL_CDECL compress_lazy(EncoderImpl* impl_, const uint8_t* BL_RESTRICT in, size_t in_nbytes, uint8_t* BL_RESTRICT out, size_t out_nbytes_avail) noexcept {
  GreedyEncoderImpl* impl = static_cast<GreedyEncoderImpl*>(impl_);

  OutputStream os{};
  os.buffer.init(out, out_nbytes_avail);

  const uint8_t *in_next = in;
  const uint8_t *in_end = in_next + in_nbytes;
  const uint8_t *in_cur_base = in_next;
  uint32_t max_len = kMaxMatchLen;
  uint32_t nice_len = bl_min(impl->nice_match_length, max_len);
  uint32_t next_hashes[2] = {0, 0};

  hc_matchfinder_init(&impl->hc_mf);

  do {
    // Starting a new DEFLATE block.
    const uint8_t * const in_block_begin = in_next;
    const uint8_t * const in_max_block_end = in_next + bl_min<size_t>(PtrOps::bytes_until(in_next, in_end), kEncoderSoftMaxBlockLength);
    uint32_t litrunlen = 0;
    Sequence *next_seq = impl->sequences;

    init_block_split_stats(&impl->split_stats);
    reset_symbol_frequencies(impl);

    do {
      uint32_t cur_offset;
      uint32_t next_len;
      uint32_t next_offset;

      if (BL_UNLIKELY(PtrOps::bytes_until(in_next, in_end) < kMaxMatchLen)) {
        max_len = uint32_t(PtrOps::bytes_until(in_next, in_end));
        nice_len = bl_min(nice_len, max_len);
      }

      // Find the longest match at the current position.
      uint32_t cur_len = hc_matchfinder_longest_match(&impl->hc_mf, &in_cur_base, in_next, kMinMatchLen - 1, max_len, nice_len, impl->max_search_depth, next_hashes, &cur_offset);
      in_next += 1;

      if (cur_len < kMinMatchLen) {
        // No match found. Choose a literal.
        choose_literal(impl, *(in_next - 1), &litrunlen);
        observe_literal(&impl->split_stats, *(in_next - 1));
        continue;
      }

have_cur_match:
      // We have a match at the current position.
      observe_match(&impl->split_stats, cur_len);

      // If the current match is very long, choose it immediately.
      if (cur_len >= nice_len) {
        choose_match(impl, cur_len, cur_offset, &litrunlen, &next_seq);
        in_next = hc_matchfinder_skip_positions(&impl->hc_mf, &in_cur_base, in_next, in_end, cur_len - 1, next_hashes);
        continue;
      }

      // Try to find a match at the next position.
      //
      // NOTE: since we already have a match at the *current* position, we use only half the `max_search_depth` when
      // checking the *next* position. This is a useful trade-off because it's more worthwhile to use a greater
      // search depth on the initial match.
      //
      // NOTE: it's possible to structure the code such that there's only one call to `longest_match()`, which
      // handles both the "find the initial match" and "try to find a longer match" cases. However, it is faster
      // to have two call sites, with `longest_match()` inlined at each.
      if (BL_UNLIKELY(PtrOps::bytes_until(in_next, in_end) < kMaxMatchLen)) {
        max_len = uint32_t(PtrOps::bytes_until(in_next, in_end));
        nice_len = bl_min(nice_len, max_len);
      }

      next_len = hc_matchfinder_longest_match(&impl->hc_mf, &in_cur_base, in_next, cur_len, max_len, nice_len, impl->max_search_depth / 2, next_hashes, &next_offset);
      in_next += 1;

      if (next_len > cur_len) {
        // Found a longer match at the next position. Output a literal. Then the next match becomes the current match.
        choose_literal(impl, *(in_next - 2), &litrunlen);
        cur_len = next_len;
        cur_offset = next_offset;
        goto have_cur_match;
      }

      // No longer match at the next position. Output the current match.
      choose_match(impl, cur_len, cur_offset, &litrunlen, &next_seq);
      in_next = hc_matchfinder_skip_positions(&impl->hc_mf, &in_cur_base, in_next, in_end, cur_len - 2, next_hashes);

      // Check if it's time to output another block.
    } while (in_next < in_max_block_end && !should_end_block(&impl->split_stats, in_block_begin, in_next, in_end));

    finish_sequence(next_seq, litrunlen);
    flush_block(impl, os, in_block_begin, uint32_t(in_next - in_block_begin), in_next == in_end, false);
  } while (in_next != in_end);

  os.bits.flush_final_byte(os.buffer);
  return os.buffer.byte_offset();
}

// bl::Compression::Deflate::Encoder - Near-Optimal Compressor
// ===========================================================

// Follow the minimum-cost path in the graph of possible match/literal choices for the current block and compute the
// frequencies of the Huffman symbols that would be needed to output those matches and literals.
static void near_optimal_tally_item_list(NearOptimalEncoderImpl* impl, uint32_t block_length) noexcept {
  OptimumNode* cur_node = &impl->optimum_nodes[0];
  OptimumNode* end_node = &impl->optimum_nodes[block_length];

  do {
    uint32_t length = cur_node->item & kNOOptimumLengthMask;
    uint32_t offset = cur_node->item >> kNOOptimumOffsetShift;

    if (length == 1) {
      // Literal.
      impl->freqs.litlen[offset]++;
    }
    else {
      // Match.
      impl->freqs.litlen[257 + kEncoderLengthSlotLUT[length]]++;
      impl->freqs.offset[impl->offset_slot_full[offset]]++;
    }
    cur_node += length;
  } while (cur_node != end_node);
}

// A scaling factor that makes it possible to consider fractional bit costs. A token requiring 'n' bits to represent
// has cost n << kNOCostShift.
//
// NOTE: This is only useful as a statistical trick for when the true costs are unknown. In reality, each token in
// DEFLATE requires a whole number of bits t output.
static constexpr uint32_t kNOCostShift = 3;

static constexpr uint32_t kNOLiteralCost = 66;    // 8.25 bits/symbol.
static constexpr uint32_t kNOLengthSlotCost = 60; // 7.5 bits/symbol.
static constexpr uint32_t kNOOffsetSlotCost = 39; // 4.875 bits/symbol.

static BL_INLINE uint32_t default_literal_cost(uint32_t literal) noexcept {
  bl_unused(literal);
  return kNOLiteralCost;
}

static BL_INLINE uint32_t default_length_slot_cost(uint32_t length_slot) noexcept {
  return kNOLengthSlotCost + ((uint32_t)kEncoderExtraLengthBitCount[length_slot] << kNOCostShift);
}

static BL_INLINE uint32_t default_offset_slot_cost(uint32_t offset_slot) noexcept {
  return kNOOffsetSlotCost + ((uint32_t)kEncoderExtraOffsetBitCount[offset_slot] << kNOCostShift);
}

// Set default symbol costs for the first block's first optimization pass.
//
// It works well to assume that each symbol is equally probable. This results in each symbol being assigned a cost
// of (-log2(1.0/num_syms) * (1 << kNOCostShift)) where 'num_syms' is the number of symbols in the corresponding
// alphabet. However, we intentionally bias the parse towards matches rather than literals by using a slightly lower
// default cost for length symbols than for literals. This often improves the compression ratio slightly.
static void near_optimal_set_default_costs(NearOptimalEncoderImpl* impl) noexcept {
  uint32_t i;

  // Literals.
  for (i = 0; i < kNumLiterals; i++)
    impl->costs.literal[i] = default_literal_cost(i);

  // Lengths.
  for (i = kMinMatchLen; i <= kMaxMatchLen; i++)
    impl->costs.length[i] = default_length_slot_cost(kEncoderLengthSlotLUT[i]);

  // Offset slots.
  for (i = 0; i < BL_ARRAY_SIZE(kEncoderOffsetSlotBase); i++)
    impl->costs.offset_slot[i] = default_offset_slot_cost(i);
}

static BL_INLINE void near_optimal_adjust_cost(uint32_t *cost_p, uint32_t default_cost) noexcept {
  *cost_p += uint32_t((int32_t(default_cost) - int32_t(*cost_p)) >> 1);
}

// Adjust the costs when beginning a new block.
//
// Since the current costs have been optimized for the data, it's undesirable to throw them away and start over
// with the default costs. At the same time, we don't want to bias the parse by assuming that the next block will
// be similar to the current block. As a compromise, make the costs closer to the defaults, but don't simply set
// them to the defaults.
static void near_optimal_adjust_costs(NearOptimalEncoderImpl* impl) noexcept {
  uint32_t i;

  // Literals.
  for (i = 0; i < kNumLiterals; i++)
    near_optimal_adjust_cost(&impl->costs.literal[i], default_literal_cost(i));

  // Lengths.
  for (i = kMinMatchLen; i <= kMaxMatchLen; i++)
    near_optimal_adjust_cost(&impl->costs.length[i], default_length_slot_cost( kEncoderLengthSlotLUT[i]));

  // Offset slots.
  for (i = 0; i < BL_ARRAY_SIZE(kEncoderOffsetSlotBase); i++)
    near_optimal_adjust_cost(&impl->costs.offset_slot[i], default_offset_slot_cost(i));
}

// Find the minimum-cost path through the graph of possible match/literal choices for this block.
//
// We find the minimum cost path from `impl->optimum_nodes[0]`, which represents the node at the
// beginning of the block, to `impl->optimum_nodes[block_length]`, which represents the node at
// the end of the block. Edge costs are evaluated using the cost model `impl->costs`.
//
// The algorithm works backwards, starting at the end node and proceeding backwards one node at a
// time. At each node, the minimum cost to reach the end node is computed and the match/literal
// choice that begins that path is saved.
static void near_optimal_find_min_cost_path(NearOptimalEncoderImpl* impl, const uint32_t block_length, const lz_match* cache_ptr) noexcept {
  OptimumNode *end_node = &impl->optimum_nodes[block_length];
  OptimumNode *cur_node = end_node;

  cur_node->cost_to_end = 0;
  do {
    cur_node--;
    cache_ptr--;

    uint32_t num_matches = cache_ptr->length;
    uint32_t literal = cache_ptr->offset;

    // It's always possible to choose a literal.
    uint32_t best_cost_to_end = impl->costs.literal[literal] + (cur_node + 1)->cost_to_end;
    cur_node->item = ((uint32_t)literal << kNOOptimumOffsetShift) | 1;

    // Also consider matches if there are any.
    if (num_matches) {
      // Consider each length from the minimum (kMinMatchLen) to the length of the longest match
      // found at this position. For each length, we consider only the smallest offset for which
      // that length is available. Although this is not guaranteed to be optimal due to the
      // possibility of a larger offset costing less than a smaller offset to code, this is a very
      // useful heuristic.
      const lz_match* match = cache_ptr - num_matches;
      uint32_t len = kMinMatchLen;
      do {
        uint32_t offset = match->offset;
        uint32_t offset_slot = impl->offset_slot_full[offset];
        uint32_t offset_cost = impl->costs.offset_slot[offset_slot];
        do {
          uint32_t cost_to_end = offset_cost + impl->costs.length[len] + (cur_node + len)->cost_to_end;
          if (cost_to_end < best_cost_to_end) {
            best_cost_to_end = cost_to_end;
            cur_node->item = ((uint32_t)offset << kNOOptimumOffsetShift) | len;
          }
        } while (++len <= match->length);
      } while (++match != cache_ptr);
      cache_ptr -= num_matches;
    }

    cur_node->cost_to_end = best_cost_to_end;
  } while (cur_node != &impl->optimum_nodes[0]);
}

// Set the current cost model from the codeword lengths specified in `lens`.
static void near_optimal_set_costs_from_codes(NearOptimalEncoderImpl* impl, const Lens* lens) noexcept {
  uint32_t i;

  // Literals.
  for (i = 0; i < kNumLiterals; i++) {
    uint32_t bits = (lens->litlen[i] ? lens->litlen[i] : kLiteralNoStatBits);
    impl->costs.literal[i] = bits << kNOCostShift;
  }

  // Lengths.
  for (i = kMinMatchLen; i <= kMaxMatchLen; i++) {
    uint32_t length_slot = kEncoderLengthSlotLUT[i];
    uint32_t litlen_sym = 257 + length_slot;
    uint32_t bits = (lens->litlen[litlen_sym] ? lens->litlen[litlen_sym] : kLengthNoStatBits);
    bits += kEncoderExtraLengthBitCount[length_slot];
    impl->costs.length[i] = bits << kNOCostShift;
  }

  // Offset slots.
  for (i = 0; i < BL_ARRAY_SIZE(kEncoderOffsetSlotBase); i++) {
    uint32_t bits = (lens->offset[i] ? lens->offset[i] : kOffsetNoStatBits);
    bits += kEncoderExtraOffsetBitCount[i];
    impl->costs.offset_slot[i] = bits << kNOCostShift;
  }
}

// Choose the literal/match sequence to use for the current block. The basic algorithm finds a minimum-cost
// path through the block's graph of literal/match choices, given a cost model. However, the cost of each
// symbol is unknown until the Huffman codes have been built, but at the same time the Huffman codes depend on
// the frequencies of chosen symbols. Consequently, multiple passes must be used to try to approximate an optimal
// solution. The first pass uses default costs, mixed with the costs from the previous block if any. Later passes
// use the Huffman codeword lengths from the previous pass as the costs.
static void near_optimal_optimize_block(NearOptimalEncoderImpl* impl, uint32_t block_length, const lz_match* cache_ptr, bool is_first_block) noexcept {
  // Force the block to really end at the desired length, even if some matches extend beyond it.
  uint32_t num_passes_remaining = impl->num_optim_passes;
  for (uint32_t i = block_length; i <= bl_min(block_length - 1 + kMaxMatchLen, BL_ARRAY_SIZE(impl->optimum_nodes) - 1); i++) {
    impl->optimum_nodes[i].cost_to_end = 0x80000000u;
  }

  // Set the initial costs.
  if (is_first_block)
    near_optimal_set_default_costs(impl);
  else
    near_optimal_adjust_costs(impl);

  for (;;) {
    // Find the minimum cost path for this pass.
    near_optimal_find_min_cost_path(impl, block_length, cache_ptr);

    // Compute frequencies of the chosen symbols.
    reset_symbol_frequencies(impl);
    near_optimal_tally_item_list(impl, block_length);

    if (--num_passes_remaining == 0)
      break;

    // At least one optimization pass remains; update the costs.
    deflate_make_huffman_codes(&impl->freqs, &impl->codes);
    near_optimal_set_costs_from_codes(impl, &impl->codes.lens);
  }
}

// This is the "near-optimal" DEFLATE compressor. It computes the optimal representation of each DEFLATE block using
// a minimum-cost path search over the graph of possible match/literal choices for that block, assuming a certain cost
// for each Huffman symbol. For several reasons, the end result is not guaranteed to be optimal:
//
//   - Non-optimal choice of blocks
//   - Heuristic limitations on which matches are actually considered
//   - Symbol costs are unknown until the symbols have already been chosen
//     (so iterative optimization must be used)
static size_t BL_CDECL compress_near_optimal(EncoderImpl* impl_, const uint8_t* BL_RESTRICT in, size_t in_nbytes, uint8_t* BL_RESTRICT out, size_t out_nbytes_avail) noexcept {
  NearOptimalEncoderImpl* impl = static_cast<NearOptimalEncoderImpl*>(impl_);

  OutputStream os{};
  os.buffer.init(out, out_nbytes_avail);

  const uint8_t *in_next = in;
  const uint8_t *in_end = in_next + in_nbytes;
  const uint8_t *in_cur_base = in_next;
  const uint8_t *in_next_slide = in_next + bl_min<size_t>(PtrOps::bytes_until(in_next, in_end), MATCHFINDER_WINDOW_SIZE);

  uint32_t max_len = kMaxMatchLen;
  uint32_t nice_len = bl_min(impl->nice_match_length, max_len);
  uint32_t next_hashes[2] = {0, 0};

  bt_matchfinder_init(&impl->bt_mf);

  do {
    // Starting a new DEFLATE block.
    lz_match* cache_ptr = impl->match_cache;
    const uint8_t* in_block_begin = in_next;
    const uint8_t* in_max_block_end = in_next + bl_min<size_t>(PtrOps::bytes_until(in_next, in_end), kEncoderSoftMaxBlockLength);
    const uint8_t* next_observation = in_next;

    init_block_split_stats(&impl->split_stats);

    // Find matches until we decide to end the block. We end the block if any of the following is true:
    //   1. Maximum block length has been reached.
    //   2. Match catch may overflow.
    //   3. Block split heuristic says to split now.
    do {
      // Slide the window forward if needed.
      if (in_next == in_next_slide) {
        bt_matchfinder_slide_window(&impl->bt_mf);
        in_cur_base = in_next;
        in_next_slide = in_next + bl_min<size_t>(PtrOps::bytes_until(in_next, in_end), MATCHFINDER_WINDOW_SIZE);
      }

      // Decrease the maximum and nice match lengths if we're approaching the end of the input buffer.
      if (BL_UNLIKELY(max_len > PtrOps::bytes_until(in_next, in_end))) {
        max_len = uint32_t(in_end - in_next);
        nice_len = bl_min(nice_len, max_len);
      }

      // Find matches with the current position using the  binary tree matchfinder and save them in
      // `match_cache`.
      //
      // NOTE: the binary tree matchfinder is more suited for optimal parsing than the hash chain
      // matchfinder. The reasons for this include:
      //
      //   - The binary tree matchfinder can find more matches in the same number of steps.
      //   - One of the major advantages of hash chains is that skipping positions (not searching for matches at them)
      //     is faster; however, with optimal parsing we search for matches at almost all positions, so this advantage
      //     of hash chains is negated.
      lz_match* matches = cache_ptr;
      uint32_t best_len = 0;

      if (BL_LIKELY(max_len >= BT_MATCHFINDER_REQUIRED_NBYTES))
        cache_ptr = bt_matchfinder_get_matches(&impl->bt_mf, in_cur_base, in_next - in_cur_base, max_len, nice_len, impl->max_search_depth, next_hashes, &best_len, matches);

      if (in_next >= next_observation) {
        if (best_len >= 4) {
          observe_match(&impl->split_stats, best_len);
          next_observation = in_next + best_len;
        }
        else {
          observe_literal(&impl->split_stats, *in_next);
          next_observation = in_next + 1;
        }
      }

      cache_ptr->length = uint16_t(cache_ptr - matches);
      cache_ptr->offset = *in_next;
      in_next++;
      cache_ptr++;

      // If there was a very long match found, don't cache any matches for the bytes covered by that match. This avoids
      // degenerate behavior when compressing highly redundant data, where the number of matches can be very large.
      //
      // This heuristic doesn't actually hurt the compression ratio very much. If there's a long match, then the data
      // must be highly compressible, so it doesn't matter much what we do.
      if (best_len >= kMinMatchLen && best_len >= nice_len) {
        --best_len;
        do {
          if (in_next == in_next_slide) {
            bt_matchfinder_slide_window(&impl->bt_mf);
            in_cur_base = in_next;
            in_next_slide = in_next + bl_min<size_t>(PtrOps::bytes_until(in_next, in_end), MATCHFINDER_WINDOW_SIZE);
          }
          if (BL_UNLIKELY(max_len > PtrOps::bytes_until(in_next, in_end))) {
            max_len = uint32_t(PtrOps::bytes_until(in_next, in_end));
            nice_len = bl_min(nice_len, max_len);
          }
          if (max_len >= BT_MATCHFINDER_REQUIRED_NBYTES) {
            bt_matchfinder_skip_position(&impl->bt_mf, in_cur_base, in_next - in_cur_base, nice_len, impl->max_search_depth, next_hashes);
          }
          cache_ptr->length = 0;
          cache_ptr->offset = *in_next;
          in_next++;
          cache_ptr++;
        } while (--best_len);
      }
    } while (in_next < in_max_block_end && cache_ptr < &impl->match_cache[kEncoderMatchCacheLength] && !should_end_block(&impl->split_stats, in_block_begin, in_next, in_end));

    // All the matches for this block have been cached. Now choose the sequence of items to output and flush the block.
    near_optimal_optimize_block(impl, uint32_t(in_next - in_block_begin), cache_ptr, in_block_begin == in);
    flush_block(impl, os, in_block_begin, uint32_t(in_next - in_block_begin), in_next == in_end, true);
  } while (in_next != in_end);

  os.bits.flush_final_byte(os.buffer);
  return os.buffer.byte_offset();
}

// bl::Compression::Deflate::Encoder - Public API
// ==============================================

static size_t get_minimum_input_size_to_compress(uint32_t compression_level) noexcept {
  BL_ASSERT(compression_level <= BL_ARRAY_SIZE(kMinimumInputSizeToCompress));
  return size_t(kMinimumInputSizeToCompress[compression_level]) - 1u;
}

static BL_INLINE uint32_t get_zlib_compression_level_hint(uint32_t compression_level) noexcept {
  constexpr uint32_t kZlibCompressionFastest = 0;
  constexpr uint32_t kZlibCompressionFast    = 1;
  constexpr uint32_t kZlibCompressionDefault = 2;
  constexpr uint32_t kZlibCompressionSlowest = 3;

  return compression_level < 2u ? kZlibCompressionFastest :
         compression_level < 6u ? kZlibCompressionFast    :
         compression_level < 8u ? kZlibCompressionDefault : kZlibCompressionSlowest;
}

BLResult Encoder::init(FormatType format, uint32_t compression_level) noexcept {
  compression_level = bl_min(compression_level, kMaxCompressionLevel);

  constexpr size_t kImplAlignment = 64;
  size_t impl_size = compression_level == 0u ? sizeof(EncoderImpl) :
                    compression_level  < 8u ? sizeof(GreedyEncoderImpl) : sizeof(NearOptimalEncoderImpl);

  void* allocated_ptr = malloc(impl_size + kImplAlignment);
  if (BL_UNLIKELY(!allocated_ptr)) {
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
  }

  EncoderImpl* new_impl = static_cast<EncoderImpl*>(IntOps::align_up(allocated_ptr, kImplAlignment));
  new_impl->allocated_ptr = allocated_ptr;
  new_impl->format = format;
  new_impl->compression_level = compression_level;
  new_impl->min_input_size = get_minimum_input_size_to_compress(compression_level);
  new_impl->prepare_func = nullptr;
  new_impl->compress_func = nullptr;

  EncoderCompressionOptions encoder_options = kEncoderCompressionOptions[compression_level];
  new_impl->max_search_depth = encoder_options.max_search_depth;
  new_impl->nice_match_length = encoder_options.nice_match_length;

  switch (compression_level) {
    case 0: {
      break;
    }

    case 1:
    case 2:
    case 3:
    case 4: {
      new_impl->prepare_func = prepare_greedy_or_lazy;
      new_impl->compress_func = compress_greedy;
      break;
    }

    case 5:
    case 6:
    case 7: {
      new_impl->prepare_func = prepare_greedy_or_lazy;
      new_impl->compress_func = compress_lazy;
      break;
    }

    case 8:
    case 9:
    case 10:
    case 11:
    case 12: {
      NearOptimalEncoderImpl* optimal_impl = static_cast<NearOptimalEncoderImpl*>(new_impl);
      optimal_impl->prepare_func = prepare_near_optimal;
      optimal_impl->compress_func = compress_near_optimal;
      optimal_impl->num_optim_passes = encoder_options.optimal_passes;
      break;
    }
  }

  reset();
  impl = new_impl;

  return BL_SUCCESS;
}

void Encoder::reset() noexcept {
  if (impl) {
    free(impl->allocated_ptr);
    impl = nullptr;
  }
}

// The worst case is all uncompressed blocks where one block has `length <= kEncoderMinBlockLength` and
// the others have length `kEncoderMinBlockLength`. Each uncompressed block has 5 bytes of overhead: 1
// for BFINAL, BTYPE, and alignment to a byte boundary; 2 for LEN; and 2 for NLEN.
size_t Encoder::minimum_output_buffer_size(size_t input_size) const noexcept {
  constexpr size_t kUncompressedBlockOverhead = 1u + 2u + 2u;

  size_t max_block_count = bl_max<size_t>(DIV_ROUND_UP(input_size, kEncoderMinBlockLength), 1);
  size_t extra_bytes = size_t(kMinOutputBufferPadding) + kDeflateMinOutputSizeByFormat[size_t(impl->format)] + 1u;

  return extra_bytes + (max_block_count * kUncompressedBlockOverhead) + input_size;
}

static BL_NOINLINE size_t compress_deflate(EncoderImpl* impl, uint8_t* output, size_t output_size, const void* input, size_t input_size) noexcept {
  if (input_size <= impl->min_input_size) {
    // For extremely small inputs just use uncompressed blocks.
    OutputStream os{};
    os.buffer.init(output, output_size);
    write_uncompressed_blocks(os, static_cast<const uint8_t*>(input), input_size, true);
    return os.buffer.byte_offset();
  }
  else {
    BL_ASSERT(impl->prepare_func != nullptr);
    BL_ASSERT(impl->compress_func != nullptr);

    impl->prepare_func(impl);
    return impl->compress_func(impl, static_cast<const uint8_t*>(input), input_size, static_cast<uint8_t*>(output), output_size);
  }
}

size_t Encoder::compress_to(uint8_t* output, size_t output_size, const uint8_t* input, size_t input_size) noexcept {
  if (BL_UNLIKELY(output_size < kMinOutputBufferPadding + kDeflateMinOutputSizeByFormat[size_t(impl->format)]))
    return 0;

  switch (impl->format) {
    case FormatType::kRaw: {
      return compress_deflate(impl, output, output_size, input, input_size);
    }

    case FormatType::kZlib: {
      static constexpr uint32_t kZlibCompressionMethodDeflate = 8;
      static constexpr uint32_t kZlibCompressionWindow32KiB = 7;

      size_t compressed_size = compress_deflate(impl, static_cast<uint8_t*>(output) + 2, output_size - 6, input, input_size);
      if (compressed_size == 0) {
        return 0;
      }

      // Zlib header - 2 bytes (CMF and FLG).
      uint32_t hdr = (get_zlib_compression_level_hint(impl->compression_level) << 6) |
                     (kZlibCompressionMethodDeflate << 8) |
                     (kZlibCompressionWindow32KiB << 12);

      hdr |= 31u - (hdr % 31u);
      MemOps::writeU16uBE(output, hdr);

      // Zlib checksum - ADLER32 (4 bytes).
      uint32_t checksum = Checksum::adler32(static_cast<const uint8_t*>(input), input_size);
      MemOps::writeU32uBE(static_cast<uint8_t*>(output) + 2 + compressed_size, checksum);

      return compressed_size + 6;
    }

    default:
      return 0;
  }
}

BLResult Encoder::compress(BLArray<uint8_t>& dst, BLModifyOp modify_op, BLDataView input) noexcept {
  size_t input_size = input.size;

  if (input_size == 0) {
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  size_t min_output_size = minimum_output_buffer_size(input_size);
  uint8_t* output_buffer;

  BL_PROPAGATE(dst.modify_op(modify_op, min_output_size, &output_buffer));

  size_t output_size = compress_to(output_buffer, min_output_size, input.data, input.size);
  return dst.truncate(output_size);
}

} // {bl::Compression::Deflate}
