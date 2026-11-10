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

#include <blend2d/simd/simd_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/core/runtime_p.h>

//! \cond INTERNAL

namespace bl::Compression::Deflate {

typedef int16_t mf_pos_t;

static constexpr uint32_t MATCHFINDER_WINDOW_SIZE = kMaxWindowSize;
static constexpr mf_pos_t MATCHFINDER_WINDOW_SIZE_NEG = mf_pos_t(0 - int(MATCHFINDER_WINDOW_SIZE));

// Initialize the hash table portion of the matchfinder (an optimized memset).
static BL_INLINE void matchfinder_init(mf_pos_t *data, size_t num_entries) {
#if BL_TARGET_SIMD_I >= 256
  using namespace SIMD;

  size_t i = num_entries / 64u;
  BL_ASSERT((num_entries % 64u) == 0);

  Vec16xI16 ws = make256_i16(MATCHFINDER_WINDOW_SIZE_NEG);

  while (i) {
    storea(data +  0, ws);
    storea(data + 16, ws);
    storea(data + 32, ws);
    storea(data + 48, ws);

    data += 64u;
    i--;
  }
#elif BL_TARGET_SIMD_I == 128
  using namespace SIMD;

  size_t i = num_entries / 32u;
  BL_ASSERT((num_entries % 32u) == 0);

  Vec8xI16 ws = make128_i16(MATCHFINDER_WINDOW_SIZE_NEG);

  while (i) {
    storea(data +  0, ws);
    storea(data +  8, ws);
    storea(data + 16, ws);
    storea(data + 24, ws);

    data += 32u;
    i--;
  }
#else
  for (size_t i = 0; i < num_entries; i++) {
    data[i] = MATCHFINDER_WINDOW_SIZE_NEG;
  }
#endif
}

// Slide the matchfinder by WINDOW_SIZE bytes.
//
// This must be called just after each WINDOW_SIZE bytes have been run through the matchfinder.
//
// This will subtract WINDOW_SIZE bytes from each entry in the array specified. The effect is that all entries
// are updated to be relative to the current position, rather than the position WINDOW_SIZE bytes prior.
//
// Underflow is detected and replaced with signed saturation. This ensures that once the sliding window has
// passed over a position, that position forever remains out of bounds.
//
// The array passed in must contain all matchfinder data that is position-relative. Concretely, this will
// include the hash table as well as the table of positions that is used to link together the sequences in
// each hash bucket. Note that in the latter table, the links are 1-ary in the case of "hash chains", and 2-ary
// in the case of "binary trees". In either case, the links need to be rebased in the same way.
static BL_INLINE void matchfinder_rebase(mf_pos_t *data, size_t num_entries) noexcept {
  size_t i = num_entries;

#if BL_TARGET_SIMD_I >= 256
  using namespace SIMD;

  Vec16xI16 ws = make256_i16(MATCHFINDER_WINDOW_SIZE_NEG);
  while (i >= 64) {
    Vec16xI16 v0 = adds_i16(loada<Vec16xI16>(data +  0), ws);
    Vec16xI16 v1 = adds_i16(loada<Vec16xI16>(data + 16), ws);
    Vec16xI16 v2 = adds_i16(loada<Vec16xI16>(data + 32), ws);
    Vec16xI16 v3 = adds_i16(loada<Vec16xI16>(data + 48), ws);

    storea(data +  0, v0);
    storea(data + 16, v1);
    storea(data + 32, v2);
    storea(data + 48, v3);

    data += 64;
    i -= 64;
  }
#elif BL_TARGET_SIMD_I >= 128
  using namespace SIMD;

  Vec8xI16 ws = make128_i16(MATCHFINDER_WINDOW_SIZE_NEG);
  while (i >= 32) {
    Vec8xI16 v0 = adds_i16(loada<Vec8xI16>(data +  0), ws);
    Vec8xI16 v1 = adds_i16(loada<Vec8xI16>(data +  8), ws);
    Vec8xI16 v2 = adds_i16(loada<Vec8xI16>(data + 16), ws);
    Vec8xI16 v3 = adds_i16(loada<Vec8xI16>(data + 24), ws);

    storea(data +  0, v0);
    storea(data +  8, v1);
    storea(data + 16, v2);
    storea(data + 24, v3);

    data += 32;
    i -= 32;
  }
#endif

  if (MATCHFINDER_WINDOW_SIZE == 32768) {
    // Branchless version for 32768 byte windows. If the value was already negative, clear all
    // bits except the sign bit; this changes the value to -32768. Otherwise, set the sign bit;
    // this is equivalent to subtracting 32768.
    while (i) {
      uint16_t v = uint16_t(*data);
      *data++ = int16_t((v & ~(int16_t(v) >> 15)) | 0x8000u);
      i--;
    }
  }
  else {
    while (i) {
      if (*data >= 0)
        data[0] = mf_pos_t(data[0] - MATCHFINDER_WINDOW_SIZE_NEG);
      else
        data[0] = mf_pos_t(MATCHFINDER_WINDOW_SIZE_NEG);
      data++;
      i--;
    }
  }
}

// The hash function: given a sequence prefix held in the low-order bits of a 32-bit value, multiply by
// a carefully-chosen large constant. Discard any bits of the product that don't fit in a 32-bit value,
// but take the next-highest `num_bits` bits of the product as the hash value, as those have the most
// randomness.
static BL_INLINE uint32_t lz_hash(uint32_t seq, unsigned num_bits) noexcept {
  return (seq * 0x1E35A7BDu) >> (32 - num_bits);
}

// Return the number of bytes at @matchptr that match the bytes at @strptr,
// up to a maximum of @max_len.  Initially, @start_len bytes are matched.
static BL_INLINE uint32_t lz_extend(const uint8_t* strptr, const uint8_t* matchptr, uint32_t start_len, uint32_t max_len) {
  uint32_t len = start_len;
  BLBitWord v_word;

  if (MemOps::kUnalignedMemIO) {
    if (BL_LIKELY(max_len - len >= uint32_t(4 * sizeof(BLBitWord)))) {
    #define COMPARE_WORD_STEP                                                                     \
      v_word = MemOps::loadu<BLBitWord>(&matchptr[len]) ^ MemOps::loadu<BLBitWord>(&strptr[len]); \
      if (v_word != 0) {                                                                          \
        goto word_differs;                                                                        \
      }                                                                                           \
      len += uint32_t(sizeof(BLBitWord));

      COMPARE_WORD_STEP
      COMPARE_WORD_STEP
      COMPARE_WORD_STEP
      COMPARE_WORD_STEP
    #undef COMPARE_WORD_STEP
    }

    while (len + uint32_t(sizeof(BLBitWord)) <= max_len) {
      v_word = MemOps::loadu<BLBitWord>(&matchptr[len]) ^ MemOps::loadu<BLBitWord>(&strptr[len]);
      if (v_word != 0) {
        goto word_differs;
      }
      len += uint32_t(sizeof(BLBitWord));
    }
  }

  while (len < max_len && matchptr[len] == strptr[len])
    len++;
  return len;

word_differs:
  if constexpr (BL_BYTE_ORDER == 1234)
    len += IntOps::ctz(v_word) >> 3;
  else
    len += IntOps::clz(v_word) >> 3;
  return len;
}

// Lempel-Ziv matchfinding with a hash table of binary trees
//
// This is a Binary Trees (bt) based matchfinder.
//
// The main data structure is a hash table where each hash bucket contains a binary tree of sequences whose first 4
// bytes share the same hash code. Each sequence is identified by its starting position in the input buffer. Each
// binary tree is always sorted such that each left child represents a sequence lexicographically lesser than its
// parent and each right child represents a sequence lexicographically greater than its parent.
//
// The algorithm processes the input buffer sequentially. At each byte position, the hash code of the first 4 bytes
// of the sequence beginning at that position (the sequence being matched against) is computed. This identifies the
// hash bucket to use for that position. Then, a new binary tree node is created to represent the current sequence.
// Then, in a single tree traversal, the hash bucket's binary tree is searched for matches and is re-rooted at the
// new node.
//
// Compared to the simpler algorithm that uses linked lists instead of binary trees (see hc_matchfinder_p.h), the
// binary tree version gains more information at each node visitation. Ideally, the binary tree version will examine
// only 'log(n)' nodes to find the same matches that the linked list version will find by examining 'n' nodes. In
// addition, the binary tree version can examine fewer bytes at each node by taking advantage of the common prefixes
// that result from the sort order, whereas the linked list version may have to examine up to the full length of the
// match at each node.
//
// However, it is not always best to use the binary tree version. It requires nearly twice as much memory as the
// linked list version, and it takes time to keep the binary trees sorted, even at positions where the compressor
// does not need matches. Generally, when doing fast compression on small buffers, binary trees are the wrong approach.
// They are best suited for thorough compression and/or large buffers.

static constexpr uint32_t BT_MATCHFINDER_HASH3_ORDER = 16;
static constexpr uint32_t BT_MATCHFINDER_HASH3_WAYS = 2;
static constexpr uint32_t BT_MATCHFINDER_HASH4_ORDER = 16;

BL_STATIC_ASSERT(BT_MATCHFINDER_HASH3_WAYS >= 1 && BT_MATCHFINDER_HASH3_WAYS <= 2);

static constexpr uint32_t BT_MATCHFINDER_TOTAL_HASH_LENGTH =
  ((1u << BT_MATCHFINDER_HASH3_ORDER) * BT_MATCHFINDER_HASH3_WAYS +
   (1u << BT_MATCHFINDER_HASH4_ORDER));

// Representation of a match found by the bt_matchfinder.
struct lz_match {
  // The number of bytes matched.
  uint16_t length;
  // The offset back from the current position that was matched.
  uint16_t offset;
};

struct alignas(64) bt_matchfinder {
  // The hash table for finding length 3 matches.
  mf_pos_t hash3_tab[1UL << BT_MATCHFINDER_HASH3_ORDER][BT_MATCHFINDER_HASH3_WAYS];
  // The hash table which contains the roots of the binary trees for finding length 4+ matches.
  mf_pos_t hash4_tab[1UL << BT_MATCHFINDER_HASH4_ORDER];
  // The child node references for the binary trees. The left and right children of the node for
  // the sequence with position 'pos' are 'child_tab[pos * 2]' and 'child_tab[pos * 2 + 1]',
  // respectively.
  mf_pos_t child_tab[2UL * MATCHFINDER_WINDOW_SIZE];
};

/* Prepare the matchfinder for a new input buffer.  */
static BL_INLINE void bt_matchfinder_init(bt_matchfinder* mf)
{
  matchfinder_init((mf_pos_t *)mf, BT_MATCHFINDER_TOTAL_HASH_LENGTH);
}

static BL_INLINE void bt_matchfinder_slide_window(bt_matchfinder* mf)
{
  matchfinder_rebase((mf_pos_t *)mf, sizeof(struct bt_matchfinder) / sizeof(mf_pos_t));
}

static BL_INLINE mf_pos_t* bt_left_child(bt_matchfinder* mf, int32_t node)
{
  return &mf->child_tab[2 * (node & int32_t(MATCHFINDER_WINDOW_SIZE - 1)) + 0];
}

static BL_INLINE mf_pos_t* bt_right_child(bt_matchfinder* mf, int32_t node)
{
  return &mf->child_tab[2 * (node & int32_t(MATCHFINDER_WINDOW_SIZE - 1)) + 1];
}

// The minimum permissible value of 'max_len' for bt_matchfinder_get_matches()
// and bt_matchfinder_skip_position(). There must be sufficiently many bytes
// remaining to load a 32-bit integer from the next position.
#define BT_MATCHFINDER_REQUIRED_NBYTES  5

// Advance the binary tree matchfinder by one byte, optionally recording
// matches. `record_matches` should be a compile-time constant.
static BL_INLINE lz_match* bt_matchfinder_advance_one_byte(bt_matchfinder* BL_RESTRICT mf,
  const uint8_t* const BL_RESTRICT in_base,
  const ptrdiff_t cur_pos,
  const uint32_t max_len,
  const uint32_t nice_len,
  const uint32_t max_search_depth,
  uint32_t* const BL_RESTRICT next_hashes,
  uint32_t* const BL_RESTRICT best_len_ret,
  lz_match* BL_RESTRICT lz_matchptr,
  const bool record_matches) noexcept
{
  const uint8_t *in_next = in_base + cur_pos;
  uint32_t depth_remaining = max_search_depth;
  const int32_t cutoff = int32_t(cur_pos - ptrdiff_t(MATCHFINDER_WINDOW_SIZE));
  uint32_t best_len = 3;
  uint32_t next_hash_seq = MemOps::loadu_le<uint32_t>(in_next + 1);

  uint32_t hash3 = next_hashes[0];
  uint32_t hash4 = next_hashes[1];

  next_hashes[0] = lz_hash(next_hash_seq & 0xFFFFFFu, BT_MATCHFINDER_HASH3_ORDER);
  next_hashes[1] = lz_hash(next_hash_seq            , BT_MATCHFINDER_HASH4_ORDER);

  bl_prefetch_w(&mf->hash3_tab[next_hashes[0]]);
  bl_prefetch_w(&mf->hash4_tab[next_hashes[1]]);

  int32_t cur_node = mf->hash3_tab[hash3][0];
  mf->hash3_tab[hash3][0] = mf_pos_t(cur_pos);

  int32_t cur_node_2 = 0;
  if constexpr (BT_MATCHFINDER_HASH3_WAYS >= 2) {
    cur_node_2 = mf->hash3_tab[hash3][1];
    mf->hash3_tab[hash3][1] = mf_pos_t(cur_node);
  }

  if (record_matches && cur_node > cutoff) {
    uint32_t seq3 = MemOps::readU24u(in_next);
    if (seq3 == MemOps::readU24u(&in_base[cur_node])) {
      lz_matchptr->length = 3;
      lz_matchptr->offset = uint16_t(in_next - &in_base[cur_node]);
      lz_matchptr++;
    }
    else {
      if constexpr (BT_MATCHFINDER_HASH3_WAYS >= 2) {
        if (cur_node_2 > cutoff && seq3 == MemOps::readU24u(&in_base[cur_node_2])) {
          lz_matchptr->length = 3;
          lz_matchptr->offset = uint16_t(in_next - &in_base[cur_node_2]);
          lz_matchptr++;
        }
      }
    }
  }

  cur_node = mf->hash4_tab[hash4];
  mf->hash4_tab[hash4] = mf_pos_t(cur_pos);

  mf_pos_t* pending_lt_ptr = bt_left_child(mf, int32_t(cur_pos));
  mf_pos_t* pending_gt_ptr = bt_right_child(mf, int32_t(cur_pos));

  if (cur_node <= cutoff) {
    *pending_lt_ptr = MATCHFINDER_WINDOW_SIZE_NEG;
    *pending_gt_ptr = MATCHFINDER_WINDOW_SIZE_NEG;
    *best_len_ret = best_len;
    return lz_matchptr;
  }

  uint32_t best_lt_len = 0;
  uint32_t best_gt_len = 0;
  uint32_t len = 0;

  for (;;) {
    const uint8_t* matchptr = &in_base[cur_node];

    if (matchptr[len] == in_next[len]) {
      len = lz_extend(in_next, matchptr, len + 1, max_len);
      if (!record_matches || len > best_len) {
        if (record_matches) {
          best_len = len;
          lz_matchptr->length = uint16_t(len);
          lz_matchptr->offset = uint16_t(in_next - matchptr);
          lz_matchptr++;
        }
        if (len >= nice_len) {
          *pending_lt_ptr = *bt_left_child(mf, cur_node);
          *pending_gt_ptr = *bt_right_child(mf, cur_node);
          *best_len_ret = best_len;
          return lz_matchptr;
        }
      }
    }

    if (matchptr[len] < in_next[len]) {
      *pending_lt_ptr = mf_pos_t(cur_node);
      pending_lt_ptr = bt_right_child(mf, cur_node);
      cur_node = *pending_lt_ptr;
      best_lt_len = len;
      len = bl_min(len, best_gt_len);
    }
    else {
      *pending_gt_ptr = mf_pos_t(cur_node);
      pending_gt_ptr = bt_left_child(mf, cur_node);
      cur_node = *pending_gt_ptr;
      best_gt_len = len;
      len = bl_min(len, best_lt_len);
    }

    if (cur_node <= cutoff || !--depth_remaining) {
      *pending_lt_ptr = MATCHFINDER_WINDOW_SIZE_NEG;
      *pending_gt_ptr = MATCHFINDER_WINDOW_SIZE_NEG;
      *best_len_ret = best_len;
      return lz_matchptr;
    }
  }
}

/*
 * Retrieve a list of matches with the current position.
 *
 * @mf
 *  The matchfinder structure.
 * @in_base
 *  Pointer to the next byte in the input buffer to process _at the last
 *  time bt_matchfinder_init() or bt_matchfinder_slide_window() was called_.
 * @cur_pos
 *  The current position in the input buffer relative to @in_base (the
 *  position of the sequence being matched against).
 * @max_len
 *  The maximum permissible match length at this position.  Must be >=
 *  BT_MATCHFINDER_REQUIRED_NBYTES.
 * @nice_len
 *  Stop searching if a match of at least this length is found.
 *  Must be <= @max_len.
 * @max_search_depth
 *  Limit on the number of potential matches to consider.  Must be >= 1.
 * @next_hashes
 *  The precomputed hash codes for the sequence beginning at @in_next.
 *  These will be used and then updated with the precomputed hashcodes for
 *  the sequence beginning at @in_next + 1.
 * @best_len_ret
 *  If a match of length >= 4 was found, then the length of the longest such
 *  match is written here; otherwise 3 is written here.  (Note: this is
 *  redundant with the 'lz_match' array, but this is easier for the
 *  compiler to optimize when inlined and the caller immediately does a
 *  check against 'best_len'.)
 * @lz_matchptr
 *  An array in which this function will record the matches.  The recorded
 *  matches will be sorted by strictly increasing length and (non-strictly)
 *  increasing offset.  The maximum number of matches that may be found is
 *  'nice_len - 2'.
 *
 * The return value is a pointer to the next available slot in the @lz_matchptr
 * array.  (If no matches were found, this will be the same as @lz_matchptr.)
 */
static BL_INLINE lz_match* bt_matchfinder_get_matches(bt_matchfinder* mf,
  const uint8_t* in_base,
  ptrdiff_t cur_pos,
  uint32_t max_len,
  uint32_t nice_len,
  uint32_t max_search_depth,
  uint32_t next_hashes[2],
  uint32_t* best_len_ret,
  lz_match* lz_matchptr) noexcept
{
  return bt_matchfinder_advance_one_byte(mf, in_base, cur_pos, max_len, nice_len, max_search_depth, next_hashes, best_len_ret, lz_matchptr, true);
}

/*
 * Advance the matchfinder, but don't record any matches.
 *
 * This is very similar to bt_matchfinder_get_matches() because both functions
 * must do hashing and tree re-rooting.
 */
static BL_INLINE void bt_matchfinder_skip_position(
  bt_matchfinder *mf,
  const uint8_t *in_base,
  ptrdiff_t cur_pos,
  uint32_t nice_len,
  uint32_t max_search_depth,
  uint32_t next_hashes[2]) noexcept
{
  uint32_t best_len;
  bt_matchfinder_advance_one_byte(mf, in_base, cur_pos, nice_len, nice_len, max_search_depth, next_hashes, &best_len, nullptr, false);
}

/*
 * hc_matchfinder_p.h - Lempel-Ziv matchfinding with a hash table of linked lists
 *
 * Written in 2014-2016 by Eric Biggers <ebiggers3@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 * ---------------------------------------------------------------------------
 *
 *           Algorithm
 *
 * This is a Hash Chains (hc) based matchfinder.
 *
 * The main data structure is a hash table where each hash bucket contains a
 * linked list (or "chain") of sequences whose first 4 bytes share the same hash
 * code.  Each sequence is identified by its starting position in the input
 * buffer.
 *
 * The algorithm processes the input buffer sequentially.  At each byte
 * position, the hash code of the first 4 bytes of the sequence beginning at
 * that position (the sequence being matched against) is computed.  This
 * identifies the hash bucket to use for that position.  Then, this hash
 * bucket's linked list is searched for matches.  Then, a new linked list node
 * is created to represent the current sequence and is prepended to the list.
 *
 * This algorithm has several useful properties:
 *
 * - It only finds true Lempel-Ziv matches; i.e., those where the matching
 *   sequence occurs prior to the sequence being matched against.
 *
 * - The sequences in each linked list are always sorted by decreasing starting
 *   position.  Therefore, the closest (smallest offset) matches are found
 *   first, which in many compression formats tend to be the cheapest to encode.
 *
 * - Although fast running time is not guaranteed due to the possibility of the
 *   lists getting very long, the worst degenerate behavior can be easily
 *   prevented by capping the number of nodes searched at each position.
 *
 * - If the compressor decides not to search for matches at a certain position,
 *   then that position can be quickly inserted without searching the list.
 *
 * - The algorithm is adaptable to sliding windows: just store the positions
 *   relative to a "base" value that is updated from time to time, and stop
 *   searching each list when the sequences get too far away.
 *
 * ----------------------------------------------------------------------------
 *
 *         Optimizations
 *
 * The main hash table and chains handle length 4+ matches.  Length 3 matches
 * are handled by a separate hash table with no chains.  This works well for
 * typical "greedy" or "lazy"-style compressors, where length 3 matches are
 * often only helpful if they have small offsets.  Instead of searching a full
 * chain for length 3+ matches, the algorithm just checks for one close length 3
 * match, then focuses on finding length 4+ matches.
 *
 * The longest_match() and skip_positions() functions are inlined into the
 * compressors that use them.  This isn't just about saving the overhead of a
 * function call.  These functions are intended to be called from the inner
 * loops of compressors, where giving the compiler more control over register
 * allocation is very helpful.  There is also significant benefit to be gained
 * from allowing the CPU to predict branches independently at each call site.
 * For example, "lazy"-style compressors can be written with two calls to
 * longest_match(), each of which starts with a different 'best_len' and
 * therefore has significantly different performance characteristics.
 *
 * Although any hash function can be used, a multiplicative hash is fast and
 * works well.
 *
 * On some processors, it is significantly faster to extend matches by whole
 * words (32 or 64 bits) instead of by individual bytes.  For this to be the
 * case, the processor must implement unaligned memory accesses efficiently and
 * must have either a fast "find first set bit" instruction or a fast "find last
 * set bit" instruction, depending on the processor's endianness.
 *
 * The code uses one loop for finding the first match and one loop for finding a
 * longer match.  Each of these loops is tuned for its respective task and in
 * combination are faster than a single generalized loop that handles both
 * tasks.
 *
 * The code also uses a tight inner loop that only compares the last and first
 * bytes of a potential match.  It is only when these bytes match that a full
 * match extension is attempted.
 *
 * ----------------------------------------------------------------------------
 */

#define HC_MATCHFINDER_HASH3_ORDER  15
#define HC_MATCHFINDER_HASH4_ORDER  16

#define HC_MATCHFINDER_TOTAL_HASH_LENGTH ((1UL << HC_MATCHFINDER_HASH3_ORDER) + (1UL << HC_MATCHFINDER_HASH4_ORDER))

struct alignas(64) hc_matchfinder {
  // The hash table for finding length 3 matches.
  mf_pos_t hash3_tab[1UL << HC_MATCHFINDER_HASH3_ORDER];

  // The hash table which contains the first nodes of the linked lists for finding
  // length 4+ matches.
  mf_pos_t hash4_tab[1UL << HC_MATCHFINDER_HASH4_ORDER];

  // The "next node" references for the linked lists. The "next node" of the node
  // for the sequence with position 'pos' is 'next_tab[pos]'.
  mf_pos_t next_tab[MATCHFINDER_WINDOW_SIZE];
};

// Prepare the matchfinder for a new input buffer.
static BL_INLINE void hc_matchfinder_init(hc_matchfinder* mf) {
  matchfinder_init((mf_pos_t *)mf, HC_MATCHFINDER_TOTAL_HASH_LENGTH);
}

static BL_INLINE void hc_matchfinder_slide_window(hc_matchfinder* mf) {
  matchfinder_rebase((mf_pos_t *)mf, sizeof(hc_matchfinder) / sizeof(mf_pos_t));
}

// Find the longest match longer than 'best_len' bytes.
//
// \param mf
//     The matchfinder structure.
//
// \param in_base_p
//     Location of a pointer which points to the place in the input data the matchfinder currently stores
//     positions relative to. This may be updated by this function.
//
// \param cur_pos
//     The current position in the input buffer relative to `in_base` (the position of the sequence being
//     matched against).
//
// \param best_len
//     Require a match longer than this length.
//
// \param max_len
//     The maximum permissible match length at this position.
//
// \param nice_len
//     Stop searching if a match of at least this length is found. Must be `<= max_len`.
//
// \param max_search_depth
//     Limit on the number of potential matches to consider.  Must be >= 1.
//
// \param next_hashes
//     The precomputed hash codes for the sequence beginning at `in_next`. These will be used and
//     then updated with the precomputed hashcodes for the sequence beginning at `in_next + 1`.
//
// \param offset_ret
//     If a match is found, its offset is returned in this location.
//
// Return the length of the match found, or `best_len` if no match longer than `best_len` was found.
static BL_INLINE uint32_t hc_matchfinder_longest_match(
  hc_matchfinder* BL_RESTRICT mf,
  const uint8_t** BL_RESTRICT in_base_p,
  const uint8_t* BL_RESTRICT in_next,
  uint32_t best_len,
  const uint32_t max_len,
  const uint32_t nice_len,
  const uint32_t max_search_depth,
  uint32_t* BL_RESTRICT next_hashes,
  uint32_t* BL_RESTRICT offset_ret
) noexcept {
  uint32_t depth_remaining = max_search_depth;
  const uint8_t* best_matchptr = in_next;
  uint32_t cur_pos = uint32_t(in_next - *in_base_p);

  if (cur_pos == MATCHFINDER_WINDOW_SIZE) {
    hc_matchfinder_slide_window(mf);
    *in_base_p += MATCHFINDER_WINDOW_SIZE;
    cur_pos = 0;
  }

  const uint8_t* in_base = *in_base_p;
  mf_pos_t cutoff = mf_pos_t(cur_pos - MATCHFINDER_WINDOW_SIZE);

  // Can we read 4 bytes from 'in_next + 1'?
  if (BL_LIKELY(max_len >= 5)) {
    const uint8_t *matchptr;

    // Get the precomputed hash codes.
    uint32_t hash3 = next_hashes[0];
    uint32_t hash4 = next_hashes[1];

    // From the hash buckets, get the first node of each linked list.
    mf_pos_t cur_node3 = mf->hash3_tab[hash3];
    mf_pos_t cur_node4 = mf->hash4_tab[hash4];

    // Update for length 3 matches.  This replaces the singleton node in the 'hash3' bucket with the node
    // for the current sequence.
    mf->hash3_tab[hash3] = mf_pos_t(cur_pos);

    // Update for length 4 matches.  This prepends the node for the current sequence to the linked list
    // in the 'hash4' bucket.
    mf->hash4_tab[hash4] = mf_pos_t(cur_pos);
    mf->next_tab[cur_pos] = mf_pos_t(cur_node4);

    // Compute the next hash codes.
    uint32_t next_hash_seq = MemOps::loadu_le<uint32_t>(in_next + 1);

    next_hashes[0] = lz_hash(next_hash_seq & 0xFFFFFFu, HC_MATCHFINDER_HASH3_ORDER);
    next_hashes[1] = lz_hash(next_hash_seq            , HC_MATCHFINDER_HASH4_ORDER);

    bl_prefetch_w(&mf->hash3_tab[next_hashes[0]]);
    bl_prefetch_w(&mf->hash4_tab[next_hashes[1]]);

    if (best_len < 4) {
      // No match of length >= 4 found yet?

      // Check for a length 3 match if needed.
      if (cur_node3 <= cutoff)
        goto out;

      uint32_t seq4 = MemOps::readU32u(in_next);
      if (best_len < 3) {
        matchptr = &in_base[cur_node3];
        if (MemOps::readU24u(matchptr) == loaded_u32_to_u24(seq4)) {
          best_len = 3;
          best_matchptr = matchptr;
        }
      }

      // Check for a length 4 match.
      if (cur_node4 <= cutoff)
        goto out;

      for (;;) {
        // No length 4 match found yet.  Check the first 4 bytes.
        matchptr = &in_base[cur_node4];
        if (MemOps::readU32u(matchptr) == seq4)
          break;

        // The first 4 bytes did not match; keep trying...
        cur_node4 = mf->next_tab[cur_node4 & mf_pos_t(MATCHFINDER_WINDOW_SIZE - 1)];
        if (cur_node4 <= cutoff || !--depth_remaining)
          goto out;
      }

      // Found a match of length >= 4. Extend it to its full length.
      best_matchptr = matchptr;
      best_len = lz_extend(in_next, best_matchptr, 4, max_len);
      if (best_len >= nice_len)
        goto out;
      cur_node4 = mf->next_tab[cur_node4 & mf_pos_t(MATCHFINDER_WINDOW_SIZE - 1)];
      if (cur_node4 <= cutoff || !--depth_remaining)
        goto out;
    }
    else {
      if (cur_node4 <= cutoff || best_len >= nice_len)
        goto out;
    }

    // Check for matches of length >= 5.
    for (;;) {
      for (;;) {
        matchptr = &in_base[cur_node4];

        // Already found a length 4 match.  Try for a longer match; start by checking either the last 4 bytes and
        // the first 4 bytes, or the last byte.  (The last byte, the one which would extend the match length by 1,
        // is the most important.)
        if constexpr (MemOps::kUnalignedMem32) {
          if ((MemOps::loadu<uint32_t>(matchptr + best_len - 3) == MemOps::loadu<uint32_t>(in_next + best_len - 3)) &&
              (MemOps::loadu<uint32_t>(matchptr) == MemOps::loadu<uint32_t>(in_next)))
            break;
        }
        else {
          if (matchptr[best_len] == in_next[best_len])
            break;
        }

        // Continue to the next node in the list.
        cur_node4 = mf->next_tab[cur_node4 & mf_pos_t(MATCHFINDER_WINDOW_SIZE - 1)];
        if (cur_node4 <= cutoff || !--depth_remaining)
          goto out;
      }

      uint32_t len = MemOps::kUnalignedMem32 ? 4 : 0;
      len = lz_extend(in_next, matchptr, len, max_len);

      if (len > best_len) {
        // This is the new longest match.
        best_len = len;
        best_matchptr = matchptr;
        if (best_len >= nice_len)
          goto out;
      }

      // Continue to the next node in the list.
      cur_node4 = mf->next_tab[cur_node4 & mf_pos_t(MATCHFINDER_WINDOW_SIZE - 1)];
      if (cur_node4 <= cutoff || !--depth_remaining)
        goto out;
    }
  }

out:
  *offset_ret = uint32_t(in_next - best_matchptr);
  return best_len;
}

// Advance the matchfinder, but don't search for matches.
//
// \param mf
//     The matchfinder structure.
//
// \param in_base_p
//     Location of a pointer which points to the place in the input data the
//     matchfinder currently stores positions relative to.  This may be updated
//     by this function.
//
// \param cur_pos
//     The current position in the input buffer relative to @in_base.
//
// \param end_pos
//     The end position of the input buffer, relative to @in_base.
//
// \param next_hashes
//     The precomputed hash codes for the sequence beginning at @in_next.
//     These will be used and then updated with the precomputed hashcodes for
//     the sequence beginning at @in_next + @count.
//
// \param count
//     The number of bytes to advance.  Must be > 0.
//
// Returns `in_next + count`.
static BL_INLINE const uint8_t* hc_matchfinder_skip_positions(
  hc_matchfinder* BL_RESTRICT mf,
  const uint8_t** BL_RESTRICT in_base_p,
  const uint8_t* in_next,
  const uint8_t* in_end,
  const uint32_t count,
  uint32_t* BL_RESTRICT next_hashes
) noexcept {
  if (BL_UNLIKELY(count + 5 > PtrOps::bytes_until(in_next, in_end))) {
    return &in_next[count];
  }

  uint32_t remaining = count;
  uint32_t cur_pos = uint32_t(in_next - *in_base_p);

  uint32_t hash3 = next_hashes[0];
  uint32_t hash4 = next_hashes[1];

  do {
    if (cur_pos == MATCHFINDER_WINDOW_SIZE) {
      hc_matchfinder_slide_window(mf);
      *in_base_p += MATCHFINDER_WINDOW_SIZE;
      cur_pos = 0;
    }

    mf->hash3_tab[hash3] = mf_pos_t(cur_pos);
    mf->next_tab[cur_pos] = mf->hash4_tab[hash4];
    mf->hash4_tab[hash4] = mf_pos_t(cur_pos);

    uint32_t next_hash_seq = MemOps::loadu_le<uint32_t>(++in_next);
    hash3 = lz_hash(next_hash_seq & 0xFFFFFFu, HC_MATCHFINDER_HASH3_ORDER);
    hash4 = lz_hash(next_hash_seq            , HC_MATCHFINDER_HASH4_ORDER);
    cur_pos++;
  } while (--remaining);

  bl_prefetch_w(&mf->hash3_tab[hash3]);
  bl_prefetch_w(&mf->hash4_tab[hash4]);

  next_hashes[0] = hash3;
  next_hashes[1] = hash4;

  return in_next;
}

} // {bl::Compression::Deflate}

//! \endcond
