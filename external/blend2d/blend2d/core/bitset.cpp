// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/bitset_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/scopedbuffer_p.h>

namespace bl {
namespace BitSetInternal {

// bl::BitSet - Constants
// ======================

static constexpr uint32_t kInitialImplSize = 128;

//! Number of temporary segments locally allocated in BitSet processing functions.
static constexpr uint32_t kTmpSegmentDataSize = 128;

// bl::BitSet - Bit/Word Utilities
// ===============================

static BL_INLINE_NODEBUG uint32_t bit_index_of(uint32_t word_index) noexcept { return word_index * BitSetOps::kNumBits; }
static BL_INLINE_NODEBUG uint32_t word_index_of(uint32_t bit_index) noexcept { return bit_index / BitSetOps::kNumBits; }

static BL_INLINE_NODEBUG uint32_t align_bit_down_to_segment(uint32_t bit_index) noexcept { return bit_index & ~uint32_t(kSegmentBitMask); }
static BL_INLINE_NODEBUG uint32_t align_word_down_to_segment(uint32_t word_index) noexcept { return word_index & ~uint32_t(kSegmentWordCount - 1u); }
static BL_INLINE_NODEBUG uint32_t align_word_up_to_segment(uint32_t word_index) noexcept { return (word_index + (kSegmentWordCount - 1)) & ~uint32_t(kSegmentWordCount - 1u); }

static BL_INLINE_NODEBUG bool is_bit_aligned_to_segment(uint32_t bit_index) noexcept { return (bit_index & kSegmentBitMask) == 0u; }
static BL_INLINE_NODEBUG bool is_word_aligned_to_segment(uint32_t word_index) noexcept { return (word_index & (kSegmentWordCount - 1u)) == 0u; }

// bl::BitSet - PopCount
// =====================

static BL_NOINLINE uint32_t bit_count(const uint32_t* data, size_t n) noexcept {
  uint32_t count = 0;

  BL_NOUNROLL
  for (size_t i = 0; i < n; i++)
    if (data[i])
      count += IntOps::pop_count(data[i]);

  return count;
}

// bl::BitSet - Segment Inserters
// ==============================

//! A helper struct that is used in places where a limited number of segments may be inserted.
template<size_t N>
struct StaticSegmentInserter {
  BLBitSetSegment _segments[N];
  uint32_t _count = 0;

  BL_INLINE_NODEBUG BLBitSetSegment* segments() noexcept { return _segments; }
  BL_INLINE_NODEBUG const BLBitSetSegment* segments() const noexcept { return _segments; }

  BL_INLINE_NODEBUG BLBitSetSegment& current() noexcept { return _segments[_count]; }
  BL_INLINE_NODEBUG const BLBitSetSegment& current() const noexcept { return _segments[_count]; }

  BL_INLINE BLBitSetSegment& prev() noexcept {
    BL_ASSERT(_count > 0);
    return _segments[_count - 1];
  }

  BL_INLINE const BLBitSetSegment& prev() const noexcept {
    BL_ASSERT(_count > 0);
    return _segments[_count - 1];
  }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return _count == 0;}
  BL_INLINE_NODEBUG uint32_t count() const noexcept { return _count; }

  BL_INLINE void advance() noexcept {
    BL_ASSERT(_count != N);
    _count++;
  }
};

//! A helper struct that is used in places where a dynamic number of segments is inserted.
struct DynamicSegmentInserter {
  BLBitSetSegment* _segments = nullptr;
  uint32_t _index = 0;
  uint32_t _capacity = 0;

  BL_INLINE_NODEBUG DynamicSegmentInserter() noexcept {}

  BL_INLINE_NODEBUG DynamicSegmentInserter(BLBitSetSegment* segments, uint32_t capacity) noexcept
    : _segments(segments),
      _index(0),
      _capacity(capacity) {}

  BL_INLINE_NODEBUG void reset(BLBitSetSegment* segments, uint32_t capacity) noexcept {
    _segments = segments;
    _index = 0;
    _capacity = capacity;
  }

  BL_INLINE_NODEBUG BLBitSetSegment* segments() noexcept { return _segments; }
  BL_INLINE_NODEBUG const BLBitSetSegment* segments() const noexcept { return _segments; }

  BL_INLINE BLBitSetSegment& current() noexcept {
    BL_ASSERT(_index < _capacity);
    return _segments[_index];
  }

  BL_INLINE const BLBitSetSegment& current() const noexcept {
    BL_ASSERT(_index < _capacity);
    return _segments[_index];
  }

  BL_INLINE BLBitSetSegment& prev() noexcept {
    BL_ASSERT(_index > 0);
    return _segments[_index - 1];
  }

  BL_INLINE const BLBitSetSegment& prev() const noexcept {
    BL_ASSERT(_index > 0);
    return _segments[_index - 1];
  }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return _index == 0; }
  BL_INLINE_NODEBUG uint32_t index() const noexcept { return _index; }
  BL_INLINE_NODEBUG uint32_t capacity() const noexcept { return _capacity; }

  BL_INLINE void advance() noexcept {
    BL_ASSERT(_index != _capacity);
    _index++;
  }
};

// bl::BitSet - Data Analysis
// ==========================

class QuickDataAnalysis {
public:
  uint32_t _acc_and;
  uint32_t _acc_or;

  BL_INLINE_NODEBUG bool is_zero() const noexcept { return _acc_or == 0u; }
  BL_INLINE_NODEBUG bool is_full() const noexcept { return _acc_and == 0xFFFFFFFFu; }
};

static BL_INLINE QuickDataAnalysis quick_data_analysis(const uint32_t* segment_words) noexcept {
  uint32_t acc_and = segment_words[0];
  uint32_t acc_or = segment_words[0];

  for (uint32_t i = 1; i < kSegmentWordCount; i++) {
    acc_or |= segment_words[i];
    acc_and &= segment_words[i];
  }

  return QuickDataAnalysis{acc_and, acc_or};
}

struct PreciseDataAnalysis {
  enum class Type : uint32_t {
    kDense = 0,
    kRange = 1,
    kEmpty = 2
  };

  Type type;
  uint32_t start;
  uint32_t end;

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return type == Type::kEmpty; }
  BL_INLINE_NODEBUG bool is_dense() const noexcept { return type == Type::kDense; }
  BL_INLINE_NODEBUG bool is_range() const noexcept { return type == Type::kRange; }
};

static PreciseDataAnalysis precise_data_analysis(uint32_t start_word, const uint32_t* data, uint32_t word_count) noexcept {
  BL_ASSERT(word_count > 0);

  // Finds the first non-zero word - in SSO dense data the termination should not be necessary as dense SSO data
  // should always contain at least one non-zero bit. However, we are defensive and return if all words are zero.
  uint32_t i = 0;
  uint32_t n = word_count;

  BL_NOUNROLL
  while (data[i] == 0)
    if (++i == word_count)
      return PreciseDataAnalysis{PreciseDataAnalysis::Type::kEmpty, 0, 0};

  // Finds the last non-zero word - this cannot fail as we have already found a non-zero word in `data`.
  BL_NOUNROLL
  while (data[--n] == 0)
    continue;

  uint32_t start_zeros = BitSetOps::count_zeros_from_start(data[i]);
  uint32_t end_zeros = BitSetOps::count_zeros_from_end(data[n]);

  uint32_t range_start = bit_index_of(start_word + i) + start_zeros;
  uint32_t range_end = bit_index_of(start_word + n) + BitSetOps::kNumBits - end_zeros;

  // Single word case.
  if (i == n) {
    uint32_t mask = BitSetOps::shift_to_end(BitSetOps::non_zero_start_mask(BitSetOps::kNumBits - (start_zeros + end_zeros)), start_zeros);
    return PreciseDataAnalysis{(PreciseDataAnalysis::Type)(data[i] == mask), range_start, range_end};
  }

  PreciseDataAnalysis::Type type = PreciseDataAnalysis::Type::kRange;

  // Multiple word cases - checks both start & end words and verifies that all words in between have only ones.
  if (data[i] != BitSetOps::non_zero_end_mask(BitSetOps::kNumBits - start_zeros) ||
      data[n] != BitSetOps::non_zero_start_mask(BitSetOps::kNumBits - end_zeros)) {
    type = PreciseDataAnalysis::Type::kDense;
  }
  else {
    BL_NOUNROLL
    while (++i != n) {
      if (data[i] != BitSetOps::ones()) {
        type = PreciseDataAnalysis::Type::kDense;
        break;
      }
    }
  }

  return PreciseDataAnalysis{type, range_start, range_end};
}

// bl::BitSet - SSO Range - Init
// =============================

static BL_INLINE BLResult init_sso_empty(BLBitSetCore* self) noexcept {
  self->_d.init_static(BLObjectInfo{BLBitSet::kSSOEmptySignature});
  return BL_SUCCESS;
}

static BL_INLINE BLResult init_sso_range(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  self->_d.init_static(BLObjectInfo{BLBitSet::kSSOEmptySignature});
  return set_sso_range(self, start_bit, end_bit);
}

// bl::BitSet - SSO Dense - Commons
// ================================

static BL_INLINE uint32_t get_sso_word_count_from_data(const uint32_t* data, uint32_t n) noexcept {
  while (n && data[n - 1] == 0)
    n--;
  return n;
}

// bl::BitSet - SSO Dense - Init
// =============================

static BL_INLINE BLResult init_sso_dense(BLBitSetCore* self, uint32_t word_index) noexcept {
  BL_ASSERT(word_index <= kSSOLastWord);
  self->_d.init_static(BLObjectInfo{BLBitSet::kSSODenseSignature});
  self->_d.u32_data[2] = word_index;
  return BL_SUCCESS;
}

static BL_INLINE BLResult init_sso_dense_with_data(BLBitSetCore* self, uint32_t word_index, const uint32_t* data, uint32_t n) noexcept {
  BL_ASSERT(n > 0 && n <= kSSOWordCount);
  init_sso_dense(self, word_index);
  MemOps::copy_forward_inline_t(self->_d.u32_data, data, n);
  return BL_SUCCESS;
}

// bl::BitSet - SSO Dense - Chop
// =============================

static SSODenseInfo chop_sso_dense_data(const BLBitSetCore* self, uint32_t dst[kSSOWordCount], uint32_t start_bit, uint32_t end_bit) noexcept {
  SSODenseInfo info = get_sso_dense_info(self);

  uint32_t first_bit = bl_max(start_bit, info.start_bit());
  uint32_t last_bit = bl_min(end_bit - 1, info.last_bit());

  if (first_bit > last_bit) {
    info._word_count = 0;
    return info;
  }

  MemOps::fill_small_t(dst, uint32_t(0u), kSSOWordCount);
  BitSetOps::bit_array_fill(dst, first_bit - info.start_bit(), last_bit - first_bit + 1);
  MemOps::combine_small<BitOperator::And>(dst, self->_d.u32_data, kSSOWordCount);

  return info;
}

// bl::BitSet - Dynamic - Capacity
// ===============================

static BL_INLINE_CONSTEXPR uint32_t capacity_from_impl_size(BLObjectImplSize impl_size) noexcept {
  return uint32_t((impl_size.value() - sizeof(BLBitSetImpl)) / sizeof(BLBitSetSegment));
}

static BL_INLINE_CONSTEXPR BLObjectImplSize impl_size_from_capacity(uint32_t capacity) noexcept {
  return BLObjectImplSize(sizeof(BLBitSetImpl) + capacity * sizeof(BLBitSetSegment));
}

static BL_INLINE_NODEBUG BLObjectImplSize align_impl_size_to_minimum(BLObjectImplSize impl_size) noexcept {
  return BLObjectImplSize(bl_max<size_t>(impl_size.value(), kInitialImplSize));
}

static BL_INLINE_NODEBUG BLObjectImplSize expand_impl_size(BLObjectImplSize impl_size) noexcept {
  return align_impl_size_to_minimum(bl_object_expand_impl_size(impl_size));
}

// bl::BitSet - Dynamic - Init
// ===========================

static BL_INLINE BLResult init_dynamic(BLBitSetCore* self, BLObjectImplSize impl_size) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_BIT_SET);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLBitSetImpl>(self, info, impl_size));

  BLBitSetImpl* impl = get_impl(self);
  impl->segment_capacity = capacity_from_impl_size(impl_size);
  impl->segment_count = 0;
  return BL_SUCCESS;
}

static BL_NOINLINE BLResult init_dynamic_with_data(BLBitSetCore* self, BLObjectImplSize impl_size, const BLBitSetSegment* segment_data, uint32_t segment_count) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_BIT_SET);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLBitSetImpl>(self, info, impl_size));

  BLBitSetImpl* impl = get_impl(self);
  impl->segment_capacity = capacity_from_impl_size(impl_size);
  impl->segment_count = segment_count;
  memcpy(impl->segment_data(), segment_data, segment_count * sizeof(BLBitSetSegment));
  return BL_SUCCESS;
}

// bl::BitSet - Dynamic - Cached Cardinality
// =========================================

//! Returns cached cardinality.
//!
//! If the returned value is zero it means that the cardinality is either not cached or zero. This means that zero
//! is always an unreliable value, which cannot be trusted. The implementation in general resets cardinality to zero
//! every time the BitSet is modified.
static BL_INLINE uint32_t get_cached_cardinality(const BLBitSetCore* self) noexcept { return self->_d.u32_data[2]; }

//! Resets cached cardinality to zero, which signalizes that it's not valid.
static BL_INLINE BLResult reset_cached_cardinality(BLBitSetCore* self) noexcept {
  self->_d.u32_data[2] = 0;
  return BL_SUCCESS;
}

//! Updates cached cardinality to `cardinality` after the cardinality has been calculated.
static BL_INLINE void update_cached_cardinality(const BLBitSetCore* self, uint32_t cardinality) noexcept {
  const_cast<BLBitSetCore*>(self)->_d.u32_data[2] = cardinality;
}

// bl::BitSet - Dynamic - Segment Utilities
// ========================================

struct SegmentWordIndex {
  uint32_t index;
};

// Helper for bl::lower_bound() and bl::upper_bound().
static BL_INLINE bool operator<(const BLBitSetSegment& a, const SegmentWordIndex& b) noexcept {
  return a.end_word() <= b.index;
}

static BL_INLINE bool has_segment_word_index(const BLBitSetSegment& segment, uint32_t word_index) noexcept {
  return Range{segment.start_word(), segment.end_word()}.has_index(word_index);
}

static BL_INLINE bool has_segment_bit_index(const BLBitSetSegment& segment, uint32_t bit_index) noexcept {
  return Range{segment.start_word(), segment.end_word()}.has_index(word_index_of(bit_index));
}

static BL_INLINE void init_dense_segment(BLBitSetSegment& segment, uint32_t start_word) noexcept {
  segment._start_word = start_word;
  segment.clear_data();
}

static BL_INLINE void init_dense_segment_with_data(BLBitSetSegment& segment, uint32_t start_word, const uint32_t* word_data) noexcept {
  segment._start_word = start_word;
  MemOps::copy_forward_inline_t(segment.data(), word_data, kSegmentWordCount);
}

static BL_INLINE void init_dense_segment_with_range(BLBitSetSegment& segment, uint32_t start_bit, uint32_t range_size) noexcept {
  uint32_t start_word = word_index_of(align_bit_down_to_segment(start_bit));
  segment._start_word = start_word;
  segment.clear_data();

  BitSetOps::bit_array_fill(segment.data(), start_bit & kSegmentBitMask, range_size);
}

static BL_INLINE void init_dense_segment_with_ones(BLBitSetSegment& segment, uint32_t start_word) noexcept {
  segment._start_word = start_word;
  segment.fill_data();
}

static BL_INLINE void init_range_segment(BLBitSetSegment& segment, uint32_t start_word, uint32_t end_word) noexcept {
  uint32_t n_words = end_word - start_word;
  uint32_t filler = IntOps::bool_as_mask<uint32_t>(n_words < kSegmentWordCount * 2);

  segment._start_word = start_word | (~filler & BL_BIT_SET_RANGE_MASK);
  segment._data[0] = filler | end_word;
  MemOps::fill_inline_t(segment._data + 1, filler, kSegmentWordCount - 1);
}

static BL_INLINE bool is_segment_data_zero(const uint32_t* word_data) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t u = MemOps::readU64<BL_BYTE_ORDER_NATIVE, 4>(word_data);
  for (uint32_t i = 1; i < kSegmentWordCount / 2; i++)
    u |= MemOps::readU64<BL_BYTE_ORDER_NATIVE, 4>(reinterpret_cast<const uint64_t*>(word_data) + i);
  return u == 0u;
#else
  uint32_t u = word_data[0];
  for (uint32_t i = 1; i < kSegmentWordCount; i++)
    u |= word_data[i];
  return u == 0u;
#endif
}

static BL_INLINE bool is_segment_data_filled(const uint32_t* word_data) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t u = MemOps::readU64<BL_BYTE_ORDER_NATIVE, 4>(word_data);
  for (uint32_t i = 1; i < kSegmentWordCount / 2; i++)
    u &= MemOps::readU64<BL_BYTE_ORDER_NATIVE, 4>(reinterpret_cast<const uint64_t*>(word_data) + i);
  return ~u == 0u;
#else
  uint32_t u = word_data[0];
  for (uint32_t i = 1; i < kSegmentWordCount; i++)
    u &= word_data[i];
  return ~u == 0u;
#endif
}

// NOTE: These functions take an advantage of knowing that segments are fixed bit arrays. We are only interested
// in low part of `bit_index` as we know that each segment's bit-start is aligned to `kSegmentBitCount`.

static BL_INLINE void add_segment_bit(BLBitSetSegment& segment, uint32_t bit_index) noexcept {
  BL_ASSERT(has_segment_bit_index(segment, bit_index));
  BitSetOps::bit_array_set_bit(segment.data(), bit_index & kSegmentBitMask);
}

static BL_INLINE void add_segment_range(BLBitSetSegment& segment, uint32_t start_bit, uint32_t count) noexcept {
  BL_ASSERT(count > 0);
  BL_ASSERT(has_segment_bit_index(segment, start_bit));
  BL_ASSERT(has_segment_bit_index(segment, start_bit + count - 1));
  BitSetOps::bit_array_fill(segment.data(), start_bit & kSegmentBitMask, count);
}

static BL_INLINE void clear_segment_bit(BLBitSetSegment& segment, uint32_t bit_index) noexcept {
  BL_ASSERT(has_segment_bit_index(segment, bit_index));
  BitSetOps::bit_array_clear_bit(segment.data(), bit_index & kSegmentBitMask);
}

static BL_INLINE bool test_segment_bit(const BLBitSetSegment& segment, uint32_t bit_index) noexcept {
  BL_ASSERT(has_segment_bit_index(segment, bit_index));
  return BitSetOps::bit_array_test_bit(segment.data(), bit_index & kSegmentBitMask);
}

// bl::BitSet - Dynamic - SegmentIterator
// ======================================

class SegmentIterator {
public:
  BLBitSetSegment* segment_ptr;
  BLBitSetSegment* segment_end;

  uint32_t cur_word;
  uint32_t end_word;

  BL_INLINE SegmentIterator(const SegmentIterator& other) = default;

  BL_INLINE SegmentIterator(BLBitSetSegment* segment_data, uint32_t segment_count) noexcept {
    reset(segment_data, segment_count);
  }

  BL_INLINE void reset(BLBitSetSegment* segment_data, uint32_t segment_count) noexcept {
    segment_ptr = segment_data;
    segment_end = segment_data + segment_count;

    cur_word = segment_ptr != segment_end ? segment_ptr->start_word() : kInvalidIndex;
    end_word = segment_ptr != segment_end ? segment_ptr->end_word() : kInvalidIndex;
  }

  BL_INLINE bool valid() const noexcept {
    return segment_ptr != segment_end;
  }

  BL_INLINE uint32_t* word_data() const noexcept {
    BL_ASSERT(valid());

    return segment_ptr->_data;
  }

  BL_INLINE uint32_t word_at(size_t index) const noexcept {
    BL_ASSERT(valid());

    return segment_ptr->_data[index];
  }

  BL_INLINE uint32_t start_word() const noexcept {
    BL_ASSERT(valid());

    return segment_ptr->start_word();
  }

  BL_INLINE uint32_t end() const noexcept {
    BL_ASSERT(valid());

    return segment_ptr->end_word();
  }

  BL_INLINE bool all_ones() const noexcept {
    BL_ASSERT(valid());

    return segment_ptr->all_ones();
  }

  BL_INLINE void advance_to(uint32_t index_word) noexcept {
    BL_ASSERT(valid());

    cur_word = index_word;
    if (cur_word == end_word)
      advance_segment();
  }

  BL_INLINE void advance_segment() noexcept {
    BL_ASSERT(valid());

    segment_ptr++;
    cur_word = segment_ptr != segment_end ? segment_ptr->start_word() : kInvalidIndex;
    end_word = segment_ptr != segment_end ? segment_ptr->end_word() : kInvalidIndex;
  }
};

// bl::BitSet - Dynamic - Chop Segments
// ====================================

struct ChoppedSegments {
  // Indexes of start and end segments in the middle.
  uint32_t _middle_index[2];
  // Could of leading [0] and trailing[1] segments.
  uint32_t _extra_count[2];

  // 4 segments should be enough, but... let's have 2 more in case we have overlooked something.
  BLBitSetSegment _extra_data[6];

  BL_INLINE void reset() noexcept {
    _middle_index[0] = 0u;
    _middle_index[1] = 0u;
    _extra_count[0] = 0u;
    _extra_count[1] = 0u;
  }

  BL_INLINE bool is_empty() const noexcept { return final_count() == 0; }
  BL_INLINE bool has_middle_segments() const noexcept { return _middle_index[1] > _middle_index[0]; }

  BL_INLINE uint32_t middle_index() const noexcept { return _middle_index[0]; }
  BL_INLINE uint32_t middle_count() const noexcept { return _middle_index[1] - _middle_index[0]; }

  BL_INLINE uint32_t leading_count() const noexcept { return _extra_count[0]; }
  BL_INLINE uint32_t trailing_count() const noexcept { return _extra_count[1]; }

  BL_INLINE uint32_t final_count() const noexcept { return middle_count() + leading_count() + trailing_count(); }

  BL_INLINE const BLBitSetSegment* extra_data() const noexcept { return _extra_data; }
  BL_INLINE const BLBitSetSegment* leading_data() const noexcept { return _extra_data; }
  BL_INLINE const BLBitSetSegment* trailing_data() const noexcept { return _extra_data + _extra_count[0]; }
};

static void chop_segments(const BLBitSetSegment* segment_data, uint32_t segment_count, uint32_t start_bit, uint32_t end_bit, ChoppedSegments* out) noexcept {
  uint32_t bit_index = start_bit;
  uint32_t last_bit = end_bit - 1;
  uint32_t aligned_end_word = word_index_of(align_bit_down_to_segment(end_bit));

  uint32_t middle_index = 0;
  uint32_t extra_index = 0;
  uint32_t prev_extra_index = 0;

  // Initially we want to find segment for the initial bit and in the second iteration for the end bit.
  uint32_t find_bit_index = bit_index;

  out->reset();

  for (uint32_t i = 0; i < 2; i++) {
    middle_index += uint32_t(bl::lower_bound(segment_data + middle_index, segment_count - middle_index, SegmentWordIndex{word_index_of(find_bit_index)}));
    if (middle_index >= segment_count) {
      out->_middle_index[i] = middle_index;
      break;
    }

    // Either an overlapping segment or a segment immediately after bit_index.
    const BLBitSetSegment& segment = segment_data[middle_index];

    // Normalize bit_index to start at the segment boundary if it was lower - this skips uninteresting area of the BitSet.
    bit_index = bl_max(bit_index, segment.start_bit());

    // If the segment overlaps, process it.
    if (bit_index < end_bit && has_segment_bit_index(segment, bit_index)) {
      // Skip this segment if this is a leading index. Trailing segment doesn't need this as it's always used as end.
      middle_index += (1 - i);

      // The worst case is splitting up a range segment into 3 segments (leading, middle, and trailing).
      if (segment.all_ones()) {
        // Not a loop, just to be able to skip outside.
        do {
          // Leading segment.
          if (!is_bit_aligned_to_segment(bit_index)) {
            BLBitSetSegment& leading_segment = out->_extra_data[extra_index++];

            uint32_t range_size = bl_min<uint32_t>(end_bit - bit_index, kSegmentBitCount - (bit_index & kSegmentBitMask));
            init_dense_segment_with_range(leading_segment, bit_index, range_size);

            bit_index += range_size;
            if (bit_index >= end_bit)
              break;
          }

          // Middle segment - at this point it's guaranteed that `bit_index` is aligned to a segment boundary.
          uint32_t middle_word_count = bl_min(aligned_end_word, segment._range_end_word()) - word_index_of(bit_index);
          if (middle_word_count >= kSegmentWordCount) {
            BLBitSetSegment& middle_segment = out->_extra_data[extra_index++];
            uint32_t word_index = word_index_of(bit_index);

            if (middle_word_count >= kSegmentWordCount * 2u)
              init_range_segment(middle_segment, word_index, word_index + middle_word_count);
            else
              init_dense_segment_with_ones(middle_segment, word_index);

            bit_index += middle_word_count * BitSetOps::kNumBits;
            if (bit_index >= end_bit)
              break;
          }

          // Trailing segment - bit_index is aligned to a segment boundary - end_index is not.
          if (bit_index <= segment.last_bit()) {
            BLBitSetSegment& trailing_segment = out->_extra_data[extra_index++];

            uint32_t range_size = bl_min<uint32_t>(last_bit, segment.last_bit()) - bit_index + 1;
            init_dense_segment_with_range(trailing_segment, bit_index, range_size);
            bit_index += range_size;
          }
        } while (false);
      }
      else {
        // Dense segment - easy case, just create a small dense segment with range, and combine it with this segment.
        uint32_t range_size = bl_min<uint32_t>(end_bit - bit_index, kSegmentBitCount - (bit_index & kSegmentBitMask));

        BLBitSetSegment& extra_segment = out->_extra_data[extra_index++];
        init_dense_segment_with_range(extra_segment, bit_index, range_size);

        BitSetOps::bit_array_combine_words<BitOperator::And>(extra_segment.data(), segment.data(), kSegmentWordCount);
        bit_index += range_size;
      }
    }

    out->_middle_index[i] = middle_index;
    out->_extra_count[i] = extra_index - prev_extra_index;

    find_bit_index = end_bit;
    prev_extra_index = extra_index;

    if (bit_index >= end_bit)
      break;
  }

  // Normalize middle indexes to make it easier to count the number of middle segments.
  if (out->_middle_index[1] < out->_middle_index[0])
    out->_middle_index[1] = out->_middle_index[0];
}

// bl::BitSet - Dynamic - Test Operations
// ======================================

BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)

template<typename Result>
struct BaseTestOp {
  typedef Result ResultType;

  static constexpr bool kSkipA0 = false;
  static constexpr bool kSkipA1 = false;
  static constexpr bool kSkipB0 = false;
  static constexpr bool kSkipB1 = false;
};

struct EqualsTestOp : public BaseTestOp<bool> {
  BL_INLINE bool make_result() const noexcept { return true; }

  template<typename T>
  BL_INLINE bool make_result(const T& a, const T& b) const noexcept { return false; }

  template<typename T>
  BL_INLINE bool should_terminate(const T& a, const T& b) const noexcept { return a != b; }
};

struct CompareTestOp : public BaseTestOp<int> {
  BL_INLINE int make_result() const noexcept { return 0; }

  template<typename T>
  BL_INLINE int make_result(const T& a, const T& b) const noexcept { return BitSetOps::compare(a, b); }

  template<typename T>
  BL_INLINE bool should_terminate(const T& a, const T& b) const noexcept { return a != b; }
};

struct SubsumesTestOp : public BaseTestOp<bool> {
  static constexpr bool kSkipA1 = true;
  static constexpr bool kSkipB0 = true;

  BL_INLINE bool make_result() const noexcept { return true; }

  template<typename T>
  BL_INLINE bool make_result(const T& a, const T& b) const noexcept { return false; }

  template<typename T>
  BL_INLINE bool should_terminate(const T& a, const T& b) const noexcept { return (a & b) != b; }
};

struct IntersectsTestOp : public BaseTestOp<bool> {
  static constexpr bool kSkipA0 = true;
  static constexpr bool kSkipB0 = true;

  BL_INLINE bool make_result() const noexcept { return false; }

  template<typename T>
  BL_INLINE bool make_result(const T& a, const T& b) const noexcept { return true; }

  template<typename T>
  BL_INLINE bool should_terminate(const T& a, const T& b) const noexcept { return (a & b) != 0; }
};

BL_DIAGNOSTIC_POP

template<typename Op>
static BL_INLINE typename Op::ResultType test_op(BLBitSetSegment* aSegmentData, uint32_t aSegmentCount, BLBitSetSegment* bSegmentData, uint32_t bSegmentCount, const Op& op) noexcept {
  constexpr uint32_t k0 = 0;
  constexpr uint32_t k1 = IntOps::all_ones<uint32_t>();

  SegmentIterator a_iter(aSegmentData, aSegmentCount);
  SegmentIterator b_iter(bSegmentData, bSegmentCount);

  for (;;) {
    if (a_iter.cur_word == b_iter.cur_word) {
      // End of bit-data.
      if (a_iter.cur_word == kInvalidIndex)
        return op.make_result();

      uint32_t ab_end_word = bl_min(a_iter.end_word, b_iter.end_word);
      if (a_iter.all_ones()) {
        if (b_iter.all_ones()) {
          // 'A' is all ones and 'B' is all ones.
          if (!Op::kSkipA1 && !Op::kSkipB1) {
            if (op.should_terminate(k1, k1))
              return op.make_result(k1, k1);
          }

          b_iter.advance_to(ab_end_word);
        }
        else {
          // 'A' is all ones and 'B' has bit-data.
          if (!Op::kSkipA1) {
            for (uint32_t i = 0; i < kSegmentWordCount; i++)
              if (op.should_terminate(k1, b_iter.word_at(i)))
                return op.make_result(k1, b_iter.word_at(i));
          }

          b_iter.advance_segment();
        }

        a_iter.advance_to(ab_end_word);
      }
      else {
        if (b_iter.all_ones()) {
          // 'A' has bit-data and 'B' is all ones.
          if (!Op::kSkipB1) {
            for (uint32_t i = 0; i < kSegmentWordCount; i++)
              if (op.should_terminate(a_iter.word_at(i), k1))
                return op.make_result(a_iter.word_at(i), k1);
          }

          b_iter.advance_to(ab_end_word);
        }
        else {
          // Both 'A' and 'B' have bit-data.
          for (uint32_t i = 0; i < kSegmentWordCount; i++)
            if (op.should_terminate(a_iter.word_at(i), b_iter.word_at(i)))
              return op.make_result(a_iter.word_at(i), b_iter.word_at(i));

          b_iter.advance_segment();
        }

        a_iter.advance_segment();
      }
    }
    else if (a_iter.cur_word < b_iter.cur_word) {
      // 'A' is not at the end and 'B' is all zeros until `ab_end_word`.
      BL_ASSERT(a_iter.valid());
      uint32_t ab_end_word = bl_min(a_iter.end(), b_iter.cur_word);

      if (!Op::kSkipB0) {
        // uint32_t aDataIndex = a_iter.data_index();
        if (a_iter.all_ones()) {
          // 'A' is all ones and 'B' is all zeros.
          if (op.should_terminate(k1, k0))
            return op.make_result(k1, k0);
        }
        else {
          // 'A' has bit-data and 'B' is all zeros.
          for (uint32_t i = 0; i < kSegmentWordCount; i++)
            if (op.should_terminate(a_iter.word_at(i), k0))
              return op.make_result(a_iter.word_at(i), k0);
        }
      }

      a_iter.advance_to(ab_end_word);
    }
    else {
      // 'A' is all zeros until `ab_end_word` and 'B' is not at the end.
      BL_ASSERT(b_iter.valid());
      uint32_t ab_end_word = bl_min(b_iter.end(), a_iter.cur_word);

      if (!Op::kSkipA0) {
        if (b_iter.all_ones()) {
          if (op.should_terminate(k0, k1))
            return op.make_result(k0, k1);
        }
        else {
          for (uint32_t i = 0; i < kSegmentWordCount; i++)
            if (op.should_terminate(k0, b_iter.word_at(i)))
              return op.make_result(k0, b_iter.word_at(i));
        }
      }

      b_iter.advance_to(ab_end_word);
    }
  }
};

// bl::BitSet - Dynamic - Segments From Range
// ==========================================

static BL_INLINE uint32_t segment_count_from_range(uint32_t start_bit, uint32_t end_bit) noexcept {
  uint32_t last_bit = end_bit - 1;

  uint32_t start_segment_id = start_bit / kSegmentBitCount;
  uint32_t last_segment_id = last_bit / kSegmentBitCount;

  uint32_t max_segments = bl_min<uint32_t>(last_segment_id - start_segment_id + 1, 3);
  uint32_t collapsed = uint32_t(is_bit_aligned_to_segment(start_bit)) +
                       uint32_t(is_bit_aligned_to_segment(end_bit  )) ;

  if (collapsed >= max_segments)
    collapsed = max_segments - 1;

  return max_segments - collapsed;
}

static BL_NOINLINE uint32_t init_segments_from_range(BLBitSetSegment* dst, uint32_t start_bit, uint32_t end_bit) noexcept {
  uint32_t n = 0;
  uint32_t remain = end_bit - start_bit;

  if (!is_bit_aligned_to_segment(start_bit) || (start_bit & ~kSegmentBitMask) == ((end_bit - 1) & ~kSegmentBitMask)) {
    uint32_t segment_bit_index = start_bit & kSegmentBitMask;
    uint32_t size = bl_min<uint32_t>(remain, kSegmentBitCount - segment_bit_index);

    init_dense_segment_with_range(dst[n++], start_bit, size);
    remain -= size;
    start_bit += size;

    if (remain == 0)
      return n;
  }

  if (remain >= kSegmentBitCount) {
    uint32_t size = remain & ~kSegmentBitMask;
    init_range_segment(dst[n], word_index_of(start_bit), word_index_of(start_bit + size));

    n++;
    remain &= kSegmentBitMask;
    start_bit += size;
  }

  if (remain)
    init_dense_segment_with_range(dst[n++], start_bit, remain);

  return n;
}

static BL_NOINLINE uint32_t init_segments_from_dense_data(BLBitSetSegment* dst, uint32_t start_word, const uint32_t* words, uint32_t count) noexcept {
  uint32_t first_segment_id = start_word / kSegmentWordCount;
  uint32_t last_segment_id = (start_word + count - 1) / kSegmentWordCount;
  uint32_t word_index = start_word;

  for (uint32_t segment_id = first_segment_id; segment_id <= last_segment_id; segment_id++) {
    uint32_t segment_start_word = segment_id * kSegmentWordCount;
    uint32_t i = word_index % kSegmentWordCount;
    uint32_t n = bl_min<uint32_t>(kSegmentWordCount - i, count);

    init_dense_segment(*dst, segment_start_word);
    count -= n;
    word_index += n;

    n += i;
    do {
      dst->_data[i] = *words++;
    } while(++i != n);
  }

  return last_segment_id - first_segment_id + 1u;
}

static BL_INLINE uint32_t make_segments_from_sso_bitset(BLBitSetSegment* dst, const BLBitSetCore* self) noexcept {
  BL_ASSERT(self->_d.sso());

  if (self->_d.is_bit_set_range()) {
    Range range = get_sso_range(self);
    return init_segments_from_range(dst, range.start, range.end);
  }
  else {
    SSODenseInfo info = get_sso_dense_info(self);
    return init_segments_from_dense_data(dst, info.start_word(), self->_d.u32_data, info.word_count());
  }
}

// bl::BitSet - Dynamic - WordData to Segments
// ===========================================

struct WordDataAnalysis {
  uint32_t segment_count;
  uint32_t zero_segment_count;
};

// Returns the exact number of segments that is necessary to represent the given data. The returned number is the
// optimal case (with zero segments removed and consecutive full segments joined into a range segment).
static WordDataAnalysis analyze_word_data_for_assignment(uint32_t start_word, const uint32_t* word_data, uint32_t word_count) noexcept {
  // Should only be called when there are actually words to assign.
  BL_ASSERT(word_count > 0);

  // It's required to remove empty words before running the analysis.
  BL_ASSERT(word_data[0] != 0);
  BL_ASSERT(word_data[word_count -1] != 0);

  uint32_t zero_count = 0;
  uint32_t insert_count = 0;

  // If a leading word doesn't start on a segment boundary, then count it as an entire segment.
  uint32_t leading_alignment_offset = start_word - align_word_down_to_segment(start_word);
  if (leading_alignment_offset) {
    insert_count++;

    uint32_t leading_alignment_words_used = kSegmentWordCount - leading_alignment_offset;
    if (leading_alignment_words_used >= word_count)
      return { insert_count, zero_count };

    word_data += leading_alignment_words_used;
    word_count -= leading_alignment_words_used;
  }

  // If a trailing segment doesn't end on a segment boundary, count it as an entire segment too.
  if (word_count & (kSegmentWordCount - 1u)) {
    insert_count++;
    word_count &= ~(kSegmentWordCount - 1u);
  }

  // Process words that form whole segments.
  if (word_count) {
    const uint32_t* end = word_data + word_count;

    do {
      QuickDataAnalysis qa = quick_data_analysis(word_data);
      word_data += kSegmentWordCount;

      if (qa.is_zero()) {
        zero_count++;
        continue;
      }

      insert_count++;

      if (qa.is_full())
        while (word_data != end && is_segment_data_filled(word_data))
          word_data += kSegmentWordCount;
    } while (word_data != end);
  }

  return WordDataAnalysis { insert_count, zero_count };
}

// Returns the exact number of segments that is necessary to insert the given word_data into an existing
// BitSet. The real addition can produce less segments in certain scenarios, but never more segments...
//
// NOTE: The given segment_data must be adjusted to start_word - the caller must find which segment will
// be the first overlapping segment (or the next overlapping segment) by using `bl::lower_bound()`.
static WordDataAnalysis analyze_word_data_for_combining(uint32_t start_word, const uint32_t* word_data, uint32_t word_count, const BLBitSetSegment* segment_data, uint32_t segment_count) noexcept {
  // Should only be called when there are actually words to assign.
  BL_ASSERT(word_count > 0);

  // It's required to remove empty words before running the analysis.
  BL_ASSERT(word_data[0] != 0);
  BL_ASSERT(word_data[word_count -1] != 0);

  uint32_t word_index = start_word;
  uint32_t zero_count = 0;
  uint32_t insert_count = 0;

  // Let's only use `segment_data` and `segment_end` to avoid indexing into `segment_data`.
  const BLBitSetSegment* segment_end = segment_data + segment_count;

  // Process data that form a leading segment (only required if the data doesn't start on a segment boundary).
  uint32_t leading_alignment_offset = word_index - align_word_down_to_segment(word_index);
  if (leading_alignment_offset) {
    insert_count += uint32_t(!(segment_data != segment_end && has_segment_word_index(*segment_data, word_index)));

    uint32_t leading_alignment_words_used = kSegmentWordCount - leading_alignment_offset;
    if (leading_alignment_words_used >= word_count)
      return { insert_count, zero_count };

    word_data += leading_alignment_words_used;
    word_index += leading_alignment_words_used;
    word_count -= leading_alignment_words_used;

    if (segment_data != segment_end && segment_data->end_word() == word_index)
      segment_data++;
  }

  uint32_t trailing_word_count = word_count & (kSegmentWordCount - 1);
  const uint32_t* word_end = word_data + (word_count - trailing_word_count);

  // Process words that form whole segments.
  while (word_data != word_end) {
    if (segment_data != segment_end && has_segment_word_index(*segment_data, word_index)) {
      word_data += kSegmentWordCount;
      word_index += kSegmentWordCount;

      if (segment_data->end_word() == word_index)
        segment_data++;
    }
    else {
      QuickDataAnalysis qa = quick_data_analysis(word_data);

      word_data += kSegmentWordCount;
      word_index += kSegmentWordCount;

      if (qa.is_zero()) {
        zero_count++;
        continue;
      }

      insert_count++;

      if (qa.is_full()) {
        uint32_t word_check = 0xFFFFFFFFu;
        if (segment_data != segment_end)
          word_check = segment_data->start_word();

        while (word_index < word_check && word_data != word_end && is_segment_data_filled(word_data)) {
          word_data += kSegmentWordCount;
          word_index += kSegmentWordCount;
        }
      }
    }
  }

  // Process data that form a trailing segment (only required if the data doesn't end on a segment boundary).
  if (trailing_word_count) {
    insert_count += uint32_t(!(segment_data != segment_end && has_segment_word_index(*segment_data, word_index)));
  }

  return WordDataAnalysis { insert_count, zero_count };
}

static bool get_range_from_analyzed_word_data(uint32_t start_word, const uint32_t* word_data, uint32_t word_count, Range* range_out) noexcept {
  // Should only be called when there are actually words to assign.
  BL_ASSERT(word_count > 0);

  // It's required to remove empty words before running the analysis.
  BL_ASSERT(word_data[0] != 0);
  BL_ASSERT(word_data[word_count -1] != 0);

  uint32_t first_word_bits = word_data[0];
  uint32_t last_word_bits = word_data[word_count - 1];

  uint32_t start_zeros = BitSetOps::count_zeros_from_start(first_word_bits);
  uint32_t end_zeros = BitSetOps::count_zeros_from_end(last_word_bits);

  range_out->start = bit_index_of(start_word + 0) + start_zeros;
  range_out->end = bit_index_of(start_word + word_count - 1) + BitSetOps::kNumBits - end_zeros;

  // Single word case.
  if (word_count == 1) {
    uint32_t mask = BitSetOps::shift_to_end(BitSetOps::non_zero_start_mask(BitSetOps::kNumBits - (start_zeros + end_zeros)), start_zeros);
    return word_data[0] == mask;
  }

  // Multiple word cases - first check whether the first and last words describe a consecutive mask.
  if (first_word_bits != BitSetOps::non_zero_end_mask(BitSetOps::kNumBits - start_zeros) ||
      last_word_bits != BitSetOps::non_zero_start_mask(BitSetOps::kNumBits - end_zeros)) {
    return false;
  }

  // Now verify that all other words that form first, middle, and last segment are all ones.
  //
  // NOTE: This function is only called after `analyze_word_data_for_assignment()`, which means that we know that there
  // are no zero segments and we know that the maximum number of segments all words form are 3. This means that we
  // don't have to process all words, only those that describe the first two segments and the last one (because there
  // are no other segments). If the range is really large, we can skip a lot of words.
  uint32_t first_words_to_check = bl_min<uint32_t>(word_count - 2, kSegmentWordCount * 2 - 1);
  uint32_t last_words_to_check = bl_min<uint32_t>(word_count - 2, kSegmentWordCount - 1);

  return MemOps::test_small_t(word_data + 1, first_words_to_check, BitSetOps::ones()) &&
         MemOps::test_small_t(word_data + word_count - 1 - last_words_to_check, last_words_to_check, BitSetOps::ones());
}

// bl::BitSet - Dynamic - Splice Operation
// =======================================

// Replaces a segment at the given `index` by segments defined by `insert_data` and `insert_count` (internal).
static BLResult splice_internal(BLBitSetCore* self, BLBitSetSegment* segment_data, uint32_t segment_count, uint32_t index, uint32_t delete_count, const BLBitSetSegment* insert_data, uint32_t insert_count, bool can_modify) noexcept {
  uint32_t final_segment_count = segment_count + insert_count - delete_count;
  uint32_t additional_segment_count = insert_count - delete_count;

  if (can_modify) {
    BLBitSetImpl* self_impl = get_impl(self);
    if (self_impl->segment_capacity >= final_segment_count) {
      self_impl->segment_count = final_segment_count;

      if (delete_count != insert_count)
        memmove(segment_data + index + insert_count, segment_data + index + delete_count, (segment_count - index - delete_count) * sizeof(BLBitSetSegment));

      MemOps::copy_forward_inline_t(segment_data + index, insert_data, insert_count);
      return reset_cached_cardinality(self);
    }
  }

  BLBitSetCore tmp = *self;
  BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(segment_count + additional_segment_count));
  BL_PROPAGATE(init_dynamic(self, impl_size));

  BLBitSetImpl* self_impl = get_impl(self);
  self_impl->segment_count = segment_count + additional_segment_count;

  MemOps::copy_forward_inline_t(self_impl->segment_data(), segment_data, index);
  MemOps::copy_forward_inline_t(self_impl->segment_data() + index, insert_data, insert_count);
  MemOps::copy_forward_inline_t(self_impl->segment_data() + index + insert_count, segment_data + index + delete_count, segment_count - index - delete_count);

  return release_instance(&tmp);
}

} // {BitSetInternal}
} // {bl}

// bl::BitSet - API - Init & Destroy
// =================================

BL_API_IMPL BLResult bl_bit_set_init(BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;

  return init_sso_empty(self);
}

BL_API_IMPL BLResult bl_bit_set_init_move(BLBitSetCore* self, BLBitSetCore* other) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_bit_set());

  self->_d = other->_d;
  return init_sso_empty(other);
}

BL_API_IMPL BLResult bl_bit_set_init_weak(BLBitSetCore* self, const BLBitSetCore* other) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_bit_set());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_bit_set_init_range(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitSetInternal;

  uint32_t mask = uint32_t(-int32_t(start_bit < end_bit));
  init_sso_range(self, start_bit & mask, end_bit & mask);
  return mask ? BL_SUCCESS : bl_make_error(BL_ERROR_INVALID_VALUE);
}

BL_API_IMPL BLResult bl_bit_set_destroy(BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  return release_instance(self);
}

// bl::BitSet - API - Reset
// ========================

BL_API_IMPL BLResult bl_bit_set_reset(BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  release_instance(self);
  return init_sso_empty(self);
}

// bl::BitSet - API - Assign BitSet
// ================================

BL_API_IMPL BLResult bl_bit_set_assign_move(BLBitSetCore* self, BLBitSetCore* other) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(self->_d.is_bit_set());
  BL_ASSERT(other->_d.is_bit_set());

  BLBitSetCore tmp = *other;
  init_sso_empty(other);
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_bit_set_assign_weak(BLBitSetCore* self, const BLBitSetCore* other) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(self->_d.is_bit_set());
  BL_ASSERT(other->_d.is_bit_set());

  retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_bit_set_assign_deep(BLBitSetCore* self, const BLBitSetCore* other) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(self->_d.is_bit_set());
  BL_ASSERT(other->_d.is_bit_set());

  if (other->_d.sso())
    return replace_instance(self, other);

  BLBitSetImpl* other_impl = get_impl(other);
  uint32_t segment_count = other_impl->segment_count;

  if (!segment_count)
    return bl_bit_set_clear(self);

  BLBitSetImpl* self_impl = get_impl(self);
  if (!self->_d.sso() && is_impl_mutable(self_impl)) {
    if (self_impl->segment_capacity >= segment_count) {
      memcpy(self_impl->segment_data(), other_impl->segment_data(), segment_count * sizeof(BLBitSetSegment));
      self_impl->segment_count = segment_count;
      reset_cached_cardinality(self);
      return BL_SUCCESS;
    }
  }

  BLBitSetCore tmp;
  BLObjectImplSize tmp_impl_size = impl_size_from_capacity(segment_count);

  BL_PROPAGATE(init_dynamic_with_data(&tmp, tmp_impl_size, other_impl->segment_data(), segment_count));
  return replace_instance(self, &tmp);
}

// bl::BitSet - API - Assign Range
// ===============================

BL_API_IMPL BLResult bl_bit_set_assign_range(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (BL_UNLIKELY(start_bit >= end_bit)) {
    if (start_bit > end_bit)
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    else
      return bl_bit_set_clear(self);
  }

  if (!self->_d.sso()) {
    BLBitSetImpl* self_impl = get_impl(self);
    if (is_impl_mutable(self_impl)) {
      uint32_t segment_count = segment_count_from_range(start_bit, end_bit);

      if (self_impl->segment_capacity >= segment_count) {
        self_impl->segment_count = init_segments_from_range(self_impl->segment_data(), start_bit, end_bit);
        return reset_cached_cardinality(self);
      }
    }

    // If we cannot use dynamic BitSet let's just release it and use SSO Range.
    release_instance(self);
  }

  return init_sso_range(self, start_bit, end_bit);
}

// bl::BitSet - API - Assign Words
// ===============================

static BL_INLINE BLResult normalize_word_data_params(uint32_t& start_word, const uint32_t*& word_data, uint32_t& word_count) noexcept {
  using namespace bl::BitSetInternal;

  if (BL_UNLIKELY(start_word > kLastWord))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (word_count >= kLastWord + 1u - start_word) {
    if (word_count > kLastWord + 1u - start_word)
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    // Make sure the last word doesn't have the last bit set. This bit is not indexable, so refuse it.
    if (word_count > 0 && (word_data[word_count - 1] & 1u) != 0)
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  // Skip zero words from the beginning and from the end.
  const uint32_t* end = word_data + word_count;

  while (word_data != end && word_data[0] == 0u) {
    word_data++;
    start_word++;
  }

  while (word_data != end && end[-1] == 0u)
    end--;

  word_count = (uint32_t)(size_t)(end - word_data);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_set_assign_words(BLBitSetCore* self, uint32_t start_word, const uint32_t* word_data, uint32_t word_count) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  BL_PROPAGATE(normalize_word_data_params(start_word, word_data, word_count));
  if (!word_count)
    return bl_bit_set_clear(self);

  BLBitSetCore tmp;
  uint32_t word_index_end = start_word + word_count;
  uint32_t start_word_aligned_to_segment = align_word_down_to_segment(start_word);

  bool changed_in_place = false;
  uint32_t mutable_segment_capacity = 0;
  BLBitSetSegment* dst_segment = nullptr;

  // Avoid analysis if the BitSet is dynamic, mutable, and has enough capacity to hold the whole data in dense segments.
  if (!self->_d.sso()) {
    BLBitSetImpl* self_impl = get_impl(self);
    if (is_impl_mutable(self_impl)) {
      mutable_segment_capacity = self_impl->segment_capacity;

      uint32_t end_word_aligned_up_to_segment = align_word_up_to_segment(start_word + word_count);
      uint32_t worst_case_segments_requirement = (end_word_aligned_up_to_segment - start_word_aligned_to_segment) / kSegmentWordCount;

      changed_in_place = (mutable_segment_capacity >= worst_case_segments_requirement);
      dst_segment = self_impl->segment_data();
    }
  }

  if (!changed_in_place) {
    WordDataAnalysis analysis = analyze_word_data_for_assignment(start_word, word_data, word_count);
    changed_in_place = mutable_segment_capacity >= analysis.segment_count;

    // A second chance or SSO attempt.
    if (!changed_in_place) {
      // If we cannot use the existing Impl, because it's not mutable, or doesn't have the required capacity, try
      // to use SSO instead of allocating a new Impl. SSO is possible if there is at most `kSSOWordCount` words or
      // if the data represents a range (all bits in `word_data` are consecutive).
      if (word_count <= kSSOWordCount) {
        uint32_t sso_start_word = bl_min<uint32_t>(start_word, kSSOLastWord);
        uint32_t sso_word_offset = start_word - sso_start_word;

        init_sso_dense(&tmp, sso_start_word);
        bl::MemOps::copy_forward_inline_t(tmp._d.u32_data + sso_word_offset, word_data, word_count);
        return replace_instance(self, &tmp);
      }

      // NOTE: 4 or more segments never describe a range - the maximum is 3 (leading, middle, and trailing segment).
      Range range;
      if (analysis.segment_count <= 3 && analysis.zero_segment_count == 0 && get_range_from_analyzed_word_data(start_word, word_data, word_count, &range)) {
        init_sso_range(&tmp, range.start, range.end);
        return replace_instance(self, &tmp);
      }

      // Allocate a new Impl.
      BLObjectImplSize impl_size = impl_size_from_capacity(bl_max(analysis.segment_count, capacity_from_impl_size(BLObjectImplSize{kInitialImplSize})));
      BL_PROPAGATE(init_dynamic(&tmp, impl_size));
      dst_segment = get_impl(&tmp)->segment_data();
    }
  }

  {
    uint32_t word_index = align_word_down_to_segment(start_word);
    uint32_t end_word_aligned_down_to_segment = align_word_down_to_segment(start_word + word_count);

    // The leading segment requires special handling if it doesn't start on a segment boundary.
    if (word_index != start_word) {
      uint32_t segment_word_offset = start_word - word_index;
      uint32_t segment_word_count = bl_min<uint32_t>(word_count, kSegmentWordCount - segment_word_offset);

      init_dense_segment(*dst_segment, word_index);
      bl::MemOps::copy_forward_inline_t(dst_segment->data() + segment_word_offset, word_data, segment_word_count);

      dst_segment++;
      word_data += segment_word_count;
      word_index += kSegmentWordCount;
    }

    // Process words that form whole segments.
    while (word_index < end_word_aligned_down_to_segment) {
      QuickDataAnalysis qa = quick_data_analysis(word_data);

      // Handle adding of Range segments.
      if (qa.is_full()) {
        const uint32_t* current_word_data = word_data + kSegmentWordCount;
        uint32_t segment_end_index = word_index + kSegmentWordCount;

        while (segment_end_index < end_word_aligned_down_to_segment && is_segment_data_filled(current_word_data)) {
          current_word_data += kSegmentWordCount;
          segment_end_index += kSegmentWordCount;
        }

        // Only add a Range segment if the range spans across at least 2 dense segments.
        if (segment_end_index - word_index > kSegmentWordCount) {
          init_range_segment(*dst_segment, word_index, segment_end_index);

          dst_segment++;
          word_data = current_word_data;
          word_index = segment_end_index;
          continue;
        }
      }

      if (!qa.is_zero()) {
        init_dense_segment_with_data(*dst_segment, word_index, word_data);
        dst_segment++;
      }

      word_data += kSegmentWordCount;
      word_index += kSegmentWordCount;
    }

    // Trailing segment requires special handling, if it doesn't end on a segment boundary.
    if (word_index != word_index_end) {
      init_dense_segment(*dst_segment, word_index);
      bl::MemOps::copy_forward_inline_t(dst_segment->data(), word_data, (size_t)(word_index_end - word_index));

      dst_segment++;
    }
  }

  if (changed_in_place) {
    BLBitSetImpl* self_impl = get_impl(self);
    self_impl->segment_count = (uint32_t)(size_t)(dst_segment - self_impl->segment_data());
    return reset_cached_cardinality(self);
  }
  else {
    BLBitSetImpl* tmp_impl = get_impl(&tmp);
    tmp_impl->segment_count = (uint32_t)(size_t)(dst_segment - tmp_impl->segment_data());
    return replace_instance(self, &tmp);
  }
}

// bl::BitSet - API - Accessors
// ============================

BL_API_IMPL bool bl_bit_set_is_empty(const BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (self->_d.sso())
    return is_sso_empty(self);

  uint32_t cardinality = get_cached_cardinality(self);
  if (cardinality)
    return false;

  const BLBitSetImpl* self_impl = get_impl(self);
  const BLBitSetSegment* segment_data = self_impl->segment_data();
  uint32_t segment_count = self_impl->segment_count;

  for (uint32_t i = 0; i < segment_count; i++)
    if (segment_data[i].all_ones() || !is_segment_data_zero(segment_data[i].data()))
      return false;

  return true;
}

BL_API_IMPL BLResult bl_bit_set_get_data(const BLBitSetCore* self, BLBitSetData* out) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (self->_d.sso()) {
    out->segment_data = out->sso_segments;
    out->segment_count = make_segments_from_sso_bitset(out->sso_segments, self);
  }
  else {
    const BLBitSetImpl* self_impl = get_impl(self);
    out->segment_data = self_impl->segment_data();
    out->segment_count = self_impl->segment_count;
  }

  return BL_SUCCESS;
}

BL_API_IMPL uint32_t bl_bit_set_get_segment_count(const BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range()) {
      Range range = get_sso_range(self);
      if (range.is_empty())
        return 0;
      else
        return segment_count_from_range(range.start, range.end);
    }
    else {
      SSODenseInfo info = get_sso_dense_info(self);
      uint32_t first_segment_id = info.start_word() / kSegmentWordCount;
      uint32_t last_segment_id = info.last_word() / kSegmentWordCount;
      return 1u + uint32_t(first_segment_id != last_segment_id);
    }
  }

  return get_impl(self)->segment_count;
}

BL_API_IMPL uint32_t bl_bit_set_get_segment_capacity(const BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (self->_d.sso())
    return 0;

  return get_impl(self)->segment_capacity;
}

// bl::BitSet - API - Bit Test Operations
// ======================================

BL_API_IMPL bool bl_bit_set_has_bit(const BLBitSetCore* self, uint32_t bit_index) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  uint32_t word_index = word_index_of(bit_index);

  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range())
      return get_sso_range(self).has_index(bit_index);

    SSODenseInfo info = get_sso_dense_info(self);
    if (info.has_index(bit_index))
      return bl::BitSetOps::has_bit(self->_d.u32_data[word_index - info.start_word()], bit_index % bl::BitSetOps::kNumBits);
    else
      return false;
  }
  else {
    const BLBitSetImpl* self_impl = get_impl(self);
    const BLBitSetSegment* segment_data = self_impl->segment_data();

    uint32_t segment_count = self_impl->segment_count;
    uint32_t segment_index = uint32_t(bl::lower_bound(segment_data, segment_count, SegmentWordIndex{word_index}));

    if (segment_index >= segment_count)
      return false;

    const BLBitSetSegment& segment = segment_data[segment_index];
    if (!has_segment_word_index(segment, word_index))
      return false;

    return segment.all_ones() || test_segment_bit(segment, bit_index);
  }
}

BL_API_IMPL bool bl_bit_set_has_bits_in_range(const BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (start_bit >= end_bit)
    return false;

  uint32_t last_bit = end_bit - 1;
  BLBitSetSegment sso_segment;

  uint32_t cur_word;
  uint32_t end_word;

  const BLBitSetSegment* segment_ptr;
  const BLBitSetSegment* segment_end;

  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range())
      return get_sso_range(self).intersect(start_bit, end_bit).valid();

    SSODenseInfo info = get_sso_dense_info(self);
    start_bit = bl_max(start_bit, info.start_bit());
    last_bit = bl_min(last_bit, info.last_bit());

    if (start_bit > last_bit)
      return false;

    end_bit = last_bit + 1;

    cur_word = word_index_of(start_bit);
    end_word = word_index_of(last_bit) + 1u;

    init_dense_segment(sso_segment, cur_word);
    bl::MemOps::copy_forward_inline_t(sso_segment._data, self->_d.u32_data + (cur_word - info.start_word()), info.end_word() - cur_word);

    segment_ptr = &sso_segment;
    segment_end = segment_ptr + 1;
  }
  else {
    BLBitSetImpl* self_impl = get_impl(self);

    cur_word = word_index_of(start_bit);
    end_word = word_index_of(last_bit) + 1u;

    segment_ptr = self_impl->segment_data();
    segment_end = self_impl->segment_data() + self_impl->segment_count;
    segment_ptr += bl::lower_bound(segment_ptr, self_impl->segment_count, SegmentWordIndex{cur_word});

    // False if the range doesn't overlap any segment.
    if (segment_ptr == segment_end || end_word <= segment_ptr->start_word())
      return false;
  }

  // We handle start of the range separately as we have to construct a mask that would have the start index and
  // possibly also an end index (if the range is small) accounted. This means that the next loop can consider that
  // the range starts at a word boundary and has to handle only the end index, not both start and end indexes.
  if (has_segment_word_index(*segment_ptr, cur_word)) {
    if (segment_ptr->all_ones())
      return true;

    uint32_t index = start_bit % bl::BitSetOps::kNumBits;
    uint32_t mask = bl::BitSetOps::non_zero_start_mask(bl_min<uint32_t>(bl::BitSetOps::kNumBits - index, end_bit - start_bit), index);

    if (segment_ptr->word_at(cur_word - segment_ptr->_dense_start_word()) & mask)
      return true;

    if (++cur_word >= end_word)
      return false;
  }

  // It's guaranteed that if we are here the range is aligned at word boundary and starts always with 0 bit for
  // each word processed here. The loop has to handle the end index though as the range doesn't have to cross
  // each processed word.
  do {
    cur_word = bl_max(segment_ptr->start_word(), cur_word);
    if (cur_word >= end_word)
      return false;

    uint32_t n = bl_min(segment_ptr->end_word(), end_word) - cur_word;
    if (n) {
      if (segment_ptr->all_ones())
        return true;

      do {
        uint32_t bits = segment_ptr->word_at(cur_word - segment_ptr->_dense_start_word());
        cur_word++;

        if (bits) {
          uint32_t count = cur_word != end_word ? 32 : ((end_bit - 1) % bl::BitSetOps::kNumBits) + 1;
          uint32_t mask = bl::BitSetOps::non_zero_start_mask(count);
          return (bits & mask) != 0;
        }
      } while (--n);
    }
  } while (++segment_ptr < segment_end);

  return false;
}

// bl::BitSet - API - Subsumes Test
// ================================

BL_API_IMPL bool bl_bit_set_subsumes(const BLBitSetCore* a, const BLBitSetCore* b) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(a->_d.is_bit_set());
  BL_ASSERT(b->_d.is_bit_set());

  BLBitSetSegment aSSOSegmentData[3];
  BLBitSetSegment bSSOSegmentData[3];

  BLBitSetSegment* aSegmentData;
  BLBitSetSegment* bSegmentData;

  uint32_t aSegmentCount;
  uint32_t bSegmentCount;

  if (a->_d.sso()) {
    aSegmentData = aSSOSegmentData;
    aSegmentCount = make_segments_from_sso_bitset(aSegmentData, a);
  }
  else {
    aSegmentData = get_impl(a)->segment_data();
    aSegmentCount = get_impl(a)->segment_count;
  }

  if (b->_d.sso()) {
    bSegmentData = bSSOSegmentData;
    bSegmentCount = make_segments_from_sso_bitset(bSegmentData, b);
  }
  else {
    bSegmentData = get_impl(b)->segment_data();
    bSegmentCount = get_impl(b)->segment_count;
  }

  return test_op(aSegmentData, aSegmentCount, bSegmentData, bSegmentCount, SubsumesTestOp{});
}

// bl::BitSet - API - Intersects Test
// ==================================

BL_API_IMPL bool bl_bit_set_intersects(const BLBitSetCore* a, const BLBitSetCore* b) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(a->_d.is_bit_set());
  BL_ASSERT(b->_d.is_bit_set());

  BLBitSetSegment sso_segment_data[3];
  BLBitSetSegment* aSegmentData;
  BLBitSetSegment* bSegmentData;

  uint32_t aSegmentCount;
  uint32_t bSegmentCount;

  // Make 'a' the SSO BitSet to make the logic simpler as the intersection is commutative.
  if (b->_d.sso())
    BLInternal::swap(a, b);

  // Handle intersection of SSO BitSets.
  if (a->_d.sso()) {
    if (a->_d.is_bit_set_range()) {
      Range range = get_sso_range(a);
      return bl_bit_set_has_bits_in_range(b, range.start, range.end);
    }

    if (b->_d.sso()) {
      if (b->_d.is_bit_set_range()) {
        Range range = get_sso_range(b);
        return bl_bit_set_has_bits_in_range(a, range.start, range.end);
      }

      // Both 'a' and 'b' are SSO Dense representations.
      uint32_t aWordIndex = get_sso_word_index(a);
      uint32_t bWordIndex = get_sso_word_index(b);

      const uint32_t* aWordData = a->_d.u32_data;
      const uint32_t* bWordData = b->_d.u32_data;

      // Make `aWordIndex <= bWordIndex`.
      if (aWordIndex > bWordIndex) {
        BLInternal::swap(aWordData, bWordData);
        BLInternal::swap(aWordIndex, bWordIndex);
      }

      uint32_t distance = bWordIndex - aWordIndex;
      if (distance >= kSSOWordCount)
        return false;

      aWordData += distance;
      uint32_t n = kSSOWordCount - distance;

      do {
        n--;
        if (aWordData[n] & bWordData[n])
          return true;
      } while (n);

      return false;
    }

    aSegmentData = sso_segment_data;
    aSegmentCount = init_segments_from_dense_data(aSegmentData, get_sso_word_index(a), a->_d.u32_data, kSSOWordCount);
  }
  else {
    aSegmentData = get_impl(a)->segment_data();
    aSegmentCount = get_impl(a)->segment_count;
  }

  bSegmentData = get_impl(b)->segment_data();
  bSegmentCount = get_impl(b)->segment_count;

  return test_op(aSegmentData, aSegmentCount, bSegmentData, bSegmentCount, IntersectsTestOp{});
}

// bl::BitSet - API - Range Query
// ==============================

BL_API_IMPL bool bl_bit_set_get_range(const BLBitSetCore* self, uint32_t* start_out, uint32_t* end_out) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range()) {
      Range range = get_sso_range(self);
      *start_out = range.start;
      *end_out = range.end;
      return true;
    }
    else {
      SSODenseInfo info = get_sso_dense_info(self);
      PreciseDataAnalysis pa = precise_data_analysis(info.start_word(), self->_d.u32_data, info.word_count());

      *start_out = pa.start;
      *end_out = pa.end;
      return !pa.is_empty();
    }
  }
  else {
    const BLBitSetImpl* self_impl = get_impl(self);

    const BLBitSetSegment* segment_ptr = self_impl->segment_data();
    const BLBitSetSegment* segment_end = self_impl->segment_data_end();

    uint32_t first_bit = 0;
    while (segment_ptr != segment_end) {
      if (segment_ptr->all_ones()) {
        first_bit = segment_ptr->start_bit();
        break;
      }

      if (bl::BitSetOps::bit_array_first_bit(segment_ptr->data(), kSegmentWordCount, &first_bit)) {
        first_bit += segment_ptr->start_bit();
        break;
      }

      segment_ptr++;
    }

    if (segment_ptr == segment_end) {
      *start_out = 0;
      *end_out = 0;
      return false;
    }

    uint32_t last_bit = 0;
    while (segment_ptr != segment_end) {
      segment_end--;

      if (segment_end->all_ones()) {
        last_bit = segment_end->last_bit();
        break;
      }

      if (bl::BitSetOps::bit_array_last_bit(segment_end->data(), kSegmentWordCount, &last_bit)) {
        last_bit += segment_end->start_bit();
        break;
      }
    }

    *start_out = first_bit;
    *end_out = last_bit + 1;
    return true;
  }
}

// bl::BitSet - API - Cardinality Query
// ====================================

namespace bl {
namespace BitSetInternal {

class SegmentCardinalityAggregator {
public:
  uint32_t _dense_cardinality_in_bits = 0;
  uint32_t _range_cardinality_in_words = 0;

  BL_INLINE uint32_t value() const noexcept {
    return _dense_cardinality_in_bits + _range_cardinality_in_words * BitSetOps::kNumBits;
  }

  BL_INLINE void aggregate(const BLBitSetSegment& segment) noexcept {
    if (segment.all_ones())
      _range_cardinality_in_words += segment._range_end_word() - segment._range_start_word();
    else
      _dense_cardinality_in_bits += bit_count(segment.data(), kSegmentWordCount);
  }

  BL_INLINE void aggregate(const BLBitSetSegment* segment_data, uint32_t segment_count) noexcept {
    for (uint32_t i = 0; i < segment_count; i++)
      aggregate(segment_data[i]);
  }
};

} // {BitSetInternal}
} // {bl}

BL_API_IMPL uint32_t bl_bit_set_get_cardinality(const BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range())
      return get_sso_range(self).size();

    return bit_count(self->_d.u32_data, kSSOWordCount);
  }

  uint32_t cardinality = get_cached_cardinality(self);
  if (cardinality)
    return cardinality;

  const BLBitSetImpl* self_impl = get_impl(self);
  SegmentCardinalityAggregator aggregator;

  aggregator.aggregate(self_impl->segment_data(), self_impl->segment_count);
  cardinality = aggregator.value();

  update_cached_cardinality(self, cardinality);
  return cardinality;
}

BL_API_IMPL uint32_t bl_bit_set_get_cardinality_in_range(const BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (start_bit >= end_bit)
    return 0u;

  // SSO BitSet
  // ----------

  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range()) {
      Range range = get_sso_range(self).intersect(start_bit, end_bit);
      return range.is_empty() ? 0u : range.size();
    }
    else {
      uint32_t tmp[kSSOWordCount];
      SSODenseInfo info = chop_sso_dense_data(self, tmp, start_bit, end_bit);

      if (!info.word_count())
        return 0;

      return bit_count(tmp, info.word_count());
    }
  }

  // Dynamic BitSet
  // --------------

  const BLBitSetImpl* self_impl = get_impl(self);
  const BLBitSetSegment* segment_data = self_impl->segment_data();
  uint32_t segment_count = self_impl->segment_count;

  if (!segment_count)
    return BL_SUCCESS;

  ChoppedSegments chopped;
  chop_segments(segment_data, segment_count, start_bit, end_bit, &chopped);

  if (chopped.is_empty())
    return 0;

  // Use the default cardinality getter if the BitSet was not chopped at all, because it's cached.
  if (chopped.middle_index() == 0 && chopped.middle_count() == segment_count && (chopped.leading_count() | chopped.trailing_count()) == 0)
    return bl_bit_set_get_cardinality(self);

  SegmentCardinalityAggregator aggregator;
  aggregator.aggregate(self_impl->segment_data() + chopped.middle_index(), chopped.middle_count());
  aggregator.aggregate(chopped.extra_data(), chopped.leading_count() + chopped.trailing_count());
  return aggregator.value();
}

// bl::BitSet - API - Equality & Comparison
// ========================================

BL_API_IMPL bool bl_bit_set_equals(const BLBitSetCore* a, const BLBitSetCore* b) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(a->_d.is_bit_set());
  BL_ASSERT(b->_d.is_bit_set());

  if (a->_d == b->_d)
    return true;

  BLBitSetSegment* aSegmentData;
  BLBitSetSegment* bSegmentData;
  BLBitSetSegment sso_segment_data[3];

  uint32_t aSegmentCount;
  uint32_t bSegmentCount;

  if (a->_d.sso() == b->_d.sso()) {
    if (a->_d.sso()) {
      // Both 'a' and 'b' are SSO. We know that 'a' and 'b' are not binary equal, which means that if both objects
      // are in the same storage mode (like both are SSO Data or both are SSO Range) they are definitely not equal.
      if (a->_d.is_bit_set_range() == b->_d.is_bit_set_range())
        return false;

      // One BitSet is SSO Data and the other is SSO Range - let's make 'a' to be the SSO Data one.
      if (a->_d.is_bit_set_range())
        BLInternal::swap(a, b);

      SSODenseInfo a_info = get_sso_dense_info(a);
      PreciseDataAnalysis aPA = precise_data_analysis(a_info.start_word(), a->_d.u32_data, a_info.word_count());

      Range b_range = get_sso_range(b);
      return aPA.is_range() && aPA.start == b_range.start && aPA.end == b_range.end;
    }

    // Both 'a' and 'b' are dynamic BitSets.
    BLBitSetImpl* a_impl = get_impl(a);
    BLBitSetImpl* b_impl = get_impl(b);

    aSegmentData = a_impl->segment_data();
    aSegmentCount = a_impl->segment_count;

    bSegmentData = b_impl->segment_data();
    bSegmentCount = b_impl->segment_count;
  }
  else {
    // One BitSet is SSO, the other isn't - make 'a' the SSO one.
    if (!a->_d.sso())
      BLInternal::swap(a, b);

    aSegmentData = sso_segment_data;
    aSegmentCount = make_segments_from_sso_bitset(aSegmentData, a);

    BLBitSetImpl* b_impl = get_impl(b);
    bSegmentData = b_impl->segment_data();
    bSegmentCount = b_impl->segment_count;
  }

  return test_op(aSegmentData, aSegmentCount, bSegmentData, bSegmentCount, EqualsTestOp{});
}

BL_API_IMPL int bl_bit_set_compare(const BLBitSetCore* a, const BLBitSetCore* b) noexcept {
  using namespace bl::BitSetInternal;

  BL_ASSERT(a->_d.is_bit_set());
  BL_ASSERT(b->_d.is_bit_set());

  BLBitSetSegment aSSOSegmentData[3];
  BLBitSetSegment bSSOSegmentData[3];

  BLBitSetSegment* aSegmentData;
  BLBitSetSegment* bSegmentData;

  uint32_t aSegmentCount;
  uint32_t bSegmentCount;

  if (a->_d.sso()) {
    aSegmentData = aSSOSegmentData;
    aSegmentCount = make_segments_from_sso_bitset(aSegmentData, a);
  }
  else {
    aSegmentData = get_impl(a)->segment_data();
    aSegmentCount = get_impl(a)->segment_count;
  }

  if (b->_d.sso()) {
    bSegmentData = bSSOSegmentData;
    bSegmentCount = make_segments_from_sso_bitset(bSegmentData, b);
  }
  else {
    bSegmentData = get_impl(b)->segment_data();
    bSegmentCount = get_impl(b)->segment_count;
  }

  return test_op(aSegmentData, aSegmentCount, bSegmentData, bSegmentCount, CompareTestOp{});
}

// bl::BitSet - API - Data Manipulation - Clear
// ============================================

BL_API_IMPL BLResult bl_bit_set_clear(BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (!self->_d.sso()) {
    BLBitSetImpl* self_impl = get_impl(self);
    if (is_impl_mutable(self_impl)) {
      self_impl->segment_count = 0;
      return reset_cached_cardinality(self);
    }
    release_instance(self);
  }

  return init_sso_empty(self);
}

// bl::BitSet - API - Data Manipulation - Shrink & Optimize
// ========================================================

namespace bl {
namespace BitSetInternal {

// Calculates the number of segments required to make a BitSet optimized. Optimized BitSet uses
// ranges where applicable and doesn't have any zero segments (Dense segments with all bits zero).
static uint32_t get_optimized_segment_count(const BLBitSetSegment* segment_data, uint32_t segment_count) noexcept {
  uint32_t optimized_segment_count = 0;
  const BLBitSetSegment* segment_end = segment_data + segment_count;

  while (segment_data != segment_end) {
    segment_data++;
    optimized_segment_count++;

    if (!segment_data[-1].all_ones()) {
      QuickDataAnalysis qa = quick_data_analysis(segment_data[-1].data());
      optimized_segment_count -= uint32_t(qa.is_zero());

      if (qa.is_zero() || !qa.is_full())
        continue;
    }

    // Range segment or Dense segment having all ones.
    uint32_t end_word = segment_data[-1].end_word();
    while (segment_data != segment_end && segment_data->start_word() == end_word && (segment_data->all_ones() || is_segment_data_filled(segment_data->data()))) {
      end_word = segment_data->end_word();
      segment_data++;
    }
  }

  return optimized_segment_count;
}

// Copies `src` segments to `dst` and optimizes the output during the copy. The number of segments used
// should match the result of `get_optimized_segment_count()` if called with source segments and their size.
static BLBitSetSegment* copy_optimized_segments(BLBitSetSegment* dst, const BLBitSetSegment* src_data, uint32_t src_count) noexcept {
  const BLBitSetSegment* src_end = src_data + src_count;

  while (src_data != src_end) {
    uint32_t start_word = src_data->start_word();
    src_data++;

    if (!src_data[-1].all_ones()) {
      QuickDataAnalysis qa = quick_data_analysis(src_data[-1].data());
      if (qa.is_zero())
        continue;

      if (!qa.is_full()) {
        init_dense_segment_with_data(*dst++, start_word, src_data[-1].data());
        continue;
      }
    }

    // Range segment or Dense segment having all ones.
    uint32_t end_word = src_data[-1].end_word();
    while (src_data != src_end && src_data->start_word() == end_word && (src_data->all_ones() || is_segment_data_filled(src_data->data()))) {
      end_word = src_data->end_word();
      src_data++;
    }

    init_range_segment(*dst++, start_word, end_word);
  }

  return dst;
}

static bool test_segments_for_range(const BLBitSetSegment* segment_data, uint32_t segment_count, Range* out) noexcept {
  Range range{};

  for (uint32_t i = 0; i < segment_count; i++) {
    uint32_t start_word = segment_data->start_word();
    uint32_t end_word = segment_data->end_word();

    Range local;

    if (segment_data->all_ones()) {
      local.reset(start_word * BitSetOps::kNumBits, end_word * BitSetOps::kNumBits);
    }
    else {
      PreciseDataAnalysis pa = precise_data_analysis(start_word, segment_data->data(), kSegmentWordCount);
      if (!pa.is_range())
        return false;
      local.reset(pa.start, pa.end);
    }

    if (i == 0) {
      range = local;
      continue;
    }

    if (range.end != local.start)
      return false;
    else
      range.end = local.end;
  }

  *out = range;
  return range.valid();
}

static BLResult optimize_internal(BLBitSetCore* self, bool shrink) noexcept {
  if (self->_d.sso()) {
    if (!self->_d.is_bit_set_range()) {
      // Switch to SSO Range if the Dense data actually form a range - SSO Range is preferred over SSO Dense data.
      SSODenseInfo info = get_sso_dense_info(self);
      PreciseDataAnalysis pa = precise_data_analysis(info.start_word(), self->_d.u32_data, info.word_count());

      if (pa.is_range())
        return init_sso_range(self, pa.start, pa.end);

      if (pa.is_empty())
        return init_sso_empty(self);
    }

    return BL_SUCCESS;
  }

  BLBitSetImpl* self_impl = get_impl(self);
  BLBitSetSegment* segment_data = self_impl->segment_data();
  uint32_t segment_count = self_impl->segment_count;
  uint32_t optimized_segment_count = get_optimized_segment_count(segment_data, segment_count);

  if (optimized_segment_count == 0)
    return bl_bit_set_clear(self);

  // Switch to SSO Dense|Range in case shrink() was called and it's possible.
  if (shrink && optimized_segment_count <= 3) {
    BLBitSetSegment optimized_segment_data[3];
    copy_optimized_segments(optimized_segment_data, segment_data, segment_count);

    // Try SSO range representation.
    Range range;
    if (test_segments_for_range(optimized_segment_data, optimized_segment_count, &range)) {
      BLBitSetCore tmp;
      init_sso_range(&tmp, range.start, range.end);
      return replace_instance(self, &tmp);
    }

    // Try SSO dense representation.
    if (optimized_segment_count <= 2) {
      if (optimized_segment_count == 1 || optimized_segment_data[0].end_word() == optimized_segment_data[1].start_word()) {
        uint32_t optimized_word_data[kSegmentWordCount * 2];
        bl::MemOps::copy_forward_inline_t(optimized_word_data, optimized_segment_data[0].data(), kSegmentWordCount);
        if (optimized_segment_count > 1)
          bl::MemOps::copy_forward_inline_t(optimized_word_data + kSegmentWordCount, optimized_segment_data[1].data(), kSegmentWordCount);

        // Skip zero words from the beginning and from the end.
        const uint32_t* word_data = optimized_word_data;
        const uint32_t* word_end = word_data + optimized_segment_count * kSegmentWordCount;

        while (word_data != word_end && word_data[0] == 0u)
          word_data++;

        while (word_data != word_end && word_end[-1] == 0u)
          word_end--;

        uint32_t start_word = optimized_segment_data[0].start_word() + uint32_t(word_data - optimized_word_data);
        uint32_t word_count = (uint32_t)(size_t)(word_end - word_data);

        if (word_count <= kSSOWordCount) {
          uint32_t sso_start_word = bl_min<uint32_t>(start_word, kSSOLastWord);
          uint32_t sso_word_offset = start_word - sso_start_word;

          BLBitSetCore tmp;
          init_sso_dense(&tmp, sso_start_word);
          bl::MemOps::copy_forward_inline_t(tmp._d.u32_data + sso_word_offset, word_data, word_count);
          return replace_instance(self, &tmp);
        }
      }
    }
  }

  if (segment_count == optimized_segment_count)
    return BL_SUCCESS;

  if (is_impl_mutable(self_impl)) {
    copy_optimized_segments(segment_data, segment_data, segment_count);
    self_impl->segment_count = optimized_segment_count;

    // NOTE: No need to reset cardinality here as it hasn't changed.
    return BL_SUCCESS;
  }
  else {
    BLBitSetCore tmp;
    BLObjectImplSize impl_size = impl_size_from_capacity(optimized_segment_count);

    BL_PROPAGATE(init_dynamic(&tmp, impl_size));
    BLBitSetImpl* tmp_impl = get_impl(&tmp);

    copy_optimized_segments(tmp_impl->segment_data(), segment_data, segment_count);
    tmp_impl->segment_count = optimized_segment_count;

    return replace_instance(self, &tmp);
  }
}

} // {BitSetInternal}
} // {bl}

BL_API_IMPL BLResult bl_bit_set_shrink(BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  return optimize_internal(self, true);
}

BL_API_IMPL BLResult bl_bit_set_optimize(BLBitSetCore* self) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  return optimize_internal(self, false);
}

// bl::BitSet - API - Data Manipulation - Chop
// ===========================================

BL_API_IMPL BLResult bl_bit_set_chop(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (BL_UNLIKELY(start_bit >= end_bit)) {
    if (start_bit > end_bit)
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    else
      return bl_bit_set_clear(self);
  }

  // SSO BitSet
  // ----------

  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range()) {
      Range range = get_sso_range(self).intersect(start_bit, end_bit);
      range.normalize();
      return init_sso_range(self, range.start, range.end);
    }
    else {
      uint32_t tmp[kSSOWordCount + 2];
      SSODenseInfo info = chop_sso_dense_data(self, tmp, start_bit, end_bit);

      uint32_t i = 0;
      BL_NOUNROLL
      while (tmp[i] == 0)
        if (++i == info.word_count())
          return init_sso_empty(self);

      tmp[kSSOWordCount + 0] = 0u;
      tmp[kSSOWordCount + 1] = 0u;

      uint32_t start_word = bl_min<uint32_t>(info.start_word() + i, kSSOLastWord);
      uint32_t word_offset = start_word - info.start_word();
      return init_sso_dense_with_data(self, start_word, tmp + word_offset, kSSOWordCount);
    }
  }

  // Dynamic BitSet
  // --------------

  BLBitSetImpl* self_impl = get_impl(self);
  uint32_t segment_count = self_impl->segment_count;
  BLBitSetSegment* segment_data = self_impl->segment_data();

  if (!segment_count)
    return BL_SUCCESS;

  ChoppedSegments chopped;
  chop_segments(segment_data, segment_count, start_bit, end_bit, &chopped);

  if (chopped.is_empty())
    return bl_bit_set_clear(self);

  uint32_t final_count = chopped.final_count();
  if (is_impl_mutable(self_impl) && self_impl->segment_capacity >= final_count) {
    if (chopped.leading_count() != chopped.middle_index())
      memmove(segment_data + chopped.leading_count(), segment_data + chopped.middle_index(), chopped.middle_count() * sizeof(BLBitSetSegment));

    bl::MemOps::copy_forward_inline_t(segment_data, chopped.leading_data(), chopped.leading_count());
    bl::MemOps::copy_forward_inline_t(segment_data + chopped.leading_count() + chopped.middle_count(), chopped.trailing_data(), chopped.trailing_count());

    self_impl->segment_count = final_count;
    reset_cached_cardinality(self);

    return BL_SUCCESS;
  }
  else {
    BLBitSetCore tmp;
    BL_PROPAGATE(init_dynamic(&tmp, impl_size_from_capacity(final_count)));

    return replace_instance(self, &tmp);
  }
}

// bl::BitSet - API - Data Manipulation - Add Bit
// ==============================================

BL_API_IMPL BLResult bl_bit_set_add_bit(BLBitSetCore* self, uint32_t bit_index) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (BL_UNLIKELY(bit_index == kInvalidIndex))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLBitSetSegment sso_segments[3];
  BLBitSetSegment* segment_data;

  bool can_modify = false;
  uint32_t segment_count;

  // SSO BitSet
  // ----------

  if (self->_d.sso()) {
    // SSO mode - first check whether the result of the operation can still be stored in SSO storage.
    if (self->_d.is_bit_set_range()) {
      Range rSSO = get_sso_range(self);

      // Extend the SSO range if the given `bit_index` is next to its start/end.
      if (bit_index == rSSO.end)
        return set_sso_range_end(self, bit_index + 1);

      if (bit_index + 1 == rSSO.start)
        return set_sso_range_start(self, bit_index);

      // Update an empty range [0, 0) if the BitSet is empty.
      if (rSSO.is_empty())
        return set_sso_range(self, bit_index, bit_index + 1);

      // Do nothing if the given `bit_index` lies within the SSO range.
      if (rSSO.has_index(bit_index))
        return BL_SUCCESS;

      // Try to turn this SSO Range into a SSO Dense representation as the result is not a range anymore.
      uint32_t dense_first_word = word_index_of(bl_min(rSSO.start, bit_index));
      uint32_t dense_last_word = word_index_of(bl_max(rSSO.end - 1, bit_index));

      // We don't want the SSO data to overflow the addressable words.
      dense_first_word = bl_min<uint32_t>(dense_first_word, kSSOLastWord);

      if (dense_last_word - dense_first_word < kSSOWordCount) {
        init_sso_dense(self, dense_first_word);
        bl::BitSetOps::bit_array_fill(self->_d.u32_data, rSSO.start - bit_index_of(dense_first_word), rSSO.size());
        bl::BitSetOps::bit_array_set_bit(self->_d.u32_data, bit_index - bit_index_of(dense_first_word));
        return BL_SUCCESS;
      }
    }
    else {
      // First try whether the `bit_index` bit lies within the dense SSO data.
      SSODenseInfo info = get_sso_dense_info(self);
      uint32_t word_index = word_index_of(bit_index);

      if (word_index < info.end_word()) {
        // Just set the bit if it lies within the current window.
        uint32_t start_word = info.start_word();
        if (word_index >= start_word) {
          bl::BitSetOps::bit_array_set_bit(self->_d.u32_data, bit_index - info.start_bit());
          return BL_SUCCESS;
        }

        // Alternatively, the `bit_index` could be slightly before the `start`, and in such case we have to test whether
        // there are zero words at the end of the current data. In that case we would have to update the SSO index.
        uint32_t n = get_sso_word_count_from_data(self->_d.u32_data, info.word_count());

        if (word_index + kSSOWordCount >= start_word + n) {
          uint32_t tmp[kSSOWordCount];
          bl::MemOps::copy_forward_inline_t(tmp, self->_d.u32_data, kSSOWordCount);

          init_sso_dense(self, word_index);
          bl::MemOps::copy_forward_inline_t(self->_d.u32_data + (start_word - word_index), tmp, n);
          self->_d.u32_data[0] |= bl::BitSetOps::index_as_mask(bit_index % bl::BitSetOps::kNumBits);

          return BL_SUCCESS;
        }
      }

      // Now we know for sure that the given `bit_index` is outside of a possible dense SSO area. The only possible
      // case to consider to remain in SSO mode is to check whether the BitSet is actually a range that can be extended
      // by the given `bit_index` - it can only be extended if the bit_index is actually on the border of the range.
      PreciseDataAnalysis pa = precise_data_analysis(info.start_word(), self->_d.u32_data, info.word_count());
      BL_ASSERT(!pa.is_empty());

      if (pa.is_range()) {
        if (bit_index == pa.end)
          return init_sso_range(self, pa.start, bit_index + 1);

        if (bit_index == pa.start - 1)
          return init_sso_range(self, bit_index, pa.end);
      }
    }

    // The result of the operation cannot be represented as SSO BitSet. The easiest way to turn this BitSet into a
    // Dynamic representation is to convert the existing SSO representation into segments, and then pretend that this
    // BitSet is not mutable - this would basically go the same path as an immutable BitSet, which is being changed.
    segment_data = sso_segments;
    segment_count = make_segments_from_sso_bitset(segment_data, self);
  }
  else {
    BLBitSetImpl* self_impl = get_impl(self);
    can_modify = is_impl_mutable(self_impl);
    segment_data = self_impl->segment_data();
    segment_count = self_impl->segment_count;
  }

  // Dynamic BitSet
  // --------------

  uint32_t segment_index;
  uint32_t word_index = word_index_of(bit_index);

  // Optimize the search in case that add_range() is repeatedly called with an increasing bit index.
  if (segment_count && segment_data[segment_count - 1].start_word() <= word_index)
    segment_index = segment_count - uint32_t(segment_data[segment_count - 1].end_word() > word_index);
  else
    segment_index = uint32_t(bl::lower_bound(segment_data, segment_count, SegmentWordIndex{word_index}));

  if (segment_index < segment_count) {
    BLBitSetSegment& segment = segment_data[segment_index];
    if (has_segment_bit_index(segment, bit_index)) {
      if (segment.all_ones())
        return BL_SUCCESS;

      if (can_modify) {
        add_segment_bit(segment, bit_index);
        return reset_cached_cardinality(self);
      }

      // This prevents making a deep copy in case this is an immutable BitSet and the given `bit_index` bit is already set.
      if (test_segment_bit(segment, bit_index))
        return BL_SUCCESS;

      BLBitSetCore tmp = *self;
      BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(segment_count));

      BL_PROPAGATE(init_dynamic_with_data(self, impl_size, segment_data, segment_count));
      BLBitSetSegment& dst_segment = get_impl(self)->segment_data()[segment_index];
      add_segment_bit(dst_segment, bit_index);
      return release_instance(&tmp);
    }
  }

  // If we are here it means that the given `bit_index` bit is outside of all segments. This means that we need to
  // insert a new segment to the BitSet. If there is a space in BitSet we can insert it on the fly, if not, or the
  // BitSet is not mutable, we create a new BitSet and insert to it the segments we need.
  uint32_t segment_start_word = word_index_of(bit_index & ~kSegmentBitMask);

  if (can_modify && get_impl(self)->segment_capacity > segment_count) {
    // Existing instance can be modified.
    BLBitSetImpl* self_impl = get_impl(self);

    self_impl->segment_count++;
    bl::MemOps::copy_backward_inline_t(segment_data + segment_index + 1, segment_data + segment_index, segment_count - segment_index);

    BLBitSetSegment& dst_segment = segment_data[segment_index];
    init_dense_segment(dst_segment, segment_start_word);
    add_segment_bit(dst_segment, bit_index);

    return reset_cached_cardinality(self);
  }
  else {
    // A new BitSet instance has to be created.
    BLBitSetCore tmp = *self;
    BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(segment_count + 1));

    BL_PROPAGATE(init_dynamic(self, impl_size));
    BLBitSetImpl* self_impl = get_impl(self);

    bl::MemOps::copy_forward_inline_t(self_impl->segment_data(), segment_data, segment_index);
    bl::MemOps::copy_forward_inline_t(self_impl->segment_data() + segment_index + 1, segment_data + segment_index, segment_count - segment_index);
    self_impl->segment_count = segment_count + 1;

    BLBitSetSegment& dst_segment = self_impl->segment_data()[segment_index];
    init_dense_segment(dst_segment, segment_start_word);
    add_segment_bit(dst_segment, bit_index);

    return release_instance(&tmp);
  }
}

// bl::BitSet - API - Data Manipulation - Add Range
// ================================================

BL_API_IMPL BLResult bl_bit_set_add_range(BLBitSetCore* self, uint32_t range_start_bit, uint32_t range_end_bit) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (BL_UNLIKELY(range_start_bit >= range_end_bit)) {
    if (range_start_bit > range_end_bit)
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    return BL_SUCCESS;
  }

  BLBitSetSegment sso_segments[3];
  BLBitSetSegment* segment_data;

  bool can_modify = false;
  uint32_t segment_count;

  uint32_t range_start_word = word_index_of(range_start_bit);
  uint32_t range_last_word = word_index_of(range_end_bit - 1);

  // SSO BitSet
  // ----------

  if (self->_d.sso()) {
    // SSO mode - first check whether the result of the operation can still be stored in SSO storage.
    if (self->_d.is_bit_set_range()) {
      Range rSSO = get_sso_range(self);

      // Update the SSO range if the given range extends SSO range.
      if ((range_start_bit <= rSSO.end) & (range_end_bit >= rSSO.start))
        return set_sso_range(self, bl_min(range_start_bit, rSSO.start), bl_max(range_end_bit, rSSO.end));

      if (rSSO.is_empty())
        return set_sso_range(self, range_start_bit, range_end_bit);

      // Try to turn this SSO Range into a SSO Dense representation as the result is not a range anymore.
      uint32_t dense_first_word = bl_min(range_start_word, word_index_of(rSSO.start));
      uint32_t dense_last_word = bl_max(range_last_word, word_index_of(rSSO.end - 1));

      // We don't want the SSO data to overflow the addressable words.
      dense_first_word = bl_min<uint32_t>(dense_first_word, kSSOLastWord);

      if (dense_last_word - dense_first_word < kSSOWordCount) {
        init_sso_dense(self, dense_first_word);
        bl::BitSetOps::bit_array_fill(self->_d.u32_data, rSSO.start - bit_index_of(dense_first_word), rSSO.size());
        bl::BitSetOps::bit_array_fill(self->_d.u32_data, range_start_bit - bit_index_of(dense_first_word), range_end_bit - range_start_bit);
        return BL_SUCCESS;
      }
    }
    else {
      // First try whether the range lies within the dense SSO data.
      SSODenseInfo info = get_sso_dense_info(self);

      if (range_last_word < info.end_word()) {
        // Just fill the range if it lies within the current window.
        uint32_t iStartWord = info.start_word();
        if (range_start_word >= iStartWord) {
          bl::BitSetOps::bit_array_fill(self->_d.u32_data, range_start_bit - info.start_bit(), range_end_bit - range_start_bit);
          return BL_SUCCESS;
        }

        // Alternatively, the range could be slightly before the start of the dense data, and in such case we have
        // to test whether there are zero words at the end of the current data and update SSO dense data start when
        // necessary.
        uint32_t n = get_sso_word_count_from_data(self->_d.u32_data, info.word_count());

        if ((range_last_word - range_start_word) < kSSOWordCount && range_last_word < iStartWord + n) {
          uint32_t tmp[kSSOWordCount];
          bl::MemOps::copy_forward_inline_t(tmp, self->_d.u32_data, kSSOWordCount);

          init_sso_dense(self, range_start_word);
          bl::MemOps::copy_forward_inline_t(self->_d.u32_data + (iStartWord - range_start_word), tmp, n);
          bl::BitSetOps::bit_array_fill(self->_d.u32_data, range_start_bit - bit_index_of(range_start_word), range_end_bit - range_start_bit);

          return BL_SUCCESS;
        }
      }

      // We have to guarantee that a result of any operaton in SSO mode must also stay in SSO mode if representable.
      // To simplify all the remaining checks we copy the current content to a temporary buffer and fill the
      // intersecting part of it, otherwise we wouldn't do it properly and we will miss cases that we shouldn't.
      uint32_t tmp[kSSOWordCount];
      bl::MemOps::copy_forward_inline_t(tmp, self->_d.u32_data, kSSOWordCount);

      Range intersection = Range{range_start_word, range_last_word + 1}.intersect(info.start_word(), info.end_word());
      if (!intersection.is_empty()) {
        uint32_t iFirst = bl_max(info.start_bit(), range_start_bit);
        uint32_t iLast = bl_min(info.last_bit(), range_end_bit - 1);
        bl::BitSetOps::bit_array_fill(tmp, iFirst - info.start_bit(), iLast - iFirst + 1);
      }

      PreciseDataAnalysis pa = precise_data_analysis(info.start_word(), tmp, info.word_count());
      BL_ASSERT(!pa.is_empty());

      if (pa.is_range() && ((range_start_bit <= pa.end) & (range_end_bit >= pa.start)))
        return init_sso_range(self, bl_min(range_start_bit, pa.start), bl_max(range_end_bit, pa.end));
    }

    // The result of the operation cannot be represented as SSO BitSet.
    segment_data = sso_segments;
    segment_count = make_segments_from_sso_bitset(segment_data, self);
  }
  else {
    BLBitSetImpl* self_impl = get_impl(self);

    can_modify = is_impl_mutable(self_impl);
    segment_data = self_impl->segment_data();
    segment_count = self_impl->segment_count;
  }

  // Dynamic BitSet
  // --------------

  // Optimize the search in case that add_range() is repeatedly called with increasing start/end indexes.
  uint32_t segment_index;
  if (segment_count && segment_data[segment_count - 1].start_word() <= range_start_word)
    segment_index = segment_count - uint32_t(segment_data[segment_count - 1].end_word() > range_start_word);
  else
    segment_index = uint32_t(bl::lower_bound(segment_data, segment_count, SegmentWordIndex{range_start_word}));

  // If the range spans across a single segment or segments that have all bits set, we can avoid a more generic case.
  while (segment_index < segment_count) {
    BLBitSetSegment& segment = segment_data[segment_index];
    if (!has_segment_word_index(segment, range_start_word))
      break;

    if (segment.all_ones()) {
      // Skip intersecting segments, which are all ones.
      range_start_word = segment._range_end_word();
      range_start_bit = bit_index_of(range_start_word);

      // Quicky return if this Range segment completely subsumes the range to be added.
      if (range_start_bit >= range_end_bit)
        return BL_SUCCESS;

      segment_index++;
    }
    else {
      // Only change data within a single segment. The reason is that we cannot start changing segments without
      // knowing whether we would need to grow the BitSet, which could fail if memory allocation fails. Blend2D
      // API is transactional, which means that on failure the content of the BitSet must be kept unmodified.
      if (can_modify && range_last_word < segment._dense_end_word()) {
        add_segment_range(segment, range_start_bit, range_end_bit - range_start_bit);
        return reset_cached_cardinality(self);
      }

      break;
    }
  }

  // Build an array of segments that will replace matching segments in the BitSet.
  StaticSegmentInserter<8> inserter;
  uint32_t insert_index = segment_index;

  do {
    // Create a Range segment if the range starts/ends a segment boundary or spans across multiple segments.
    uint32_t range_size = range_end_bit - range_start_bit;
    if (is_bit_aligned_to_segment(range_start_bit) && range_size >= kSegmentBitCount) {
      uint32_t segment_end_word = word_index_of(align_bit_down_to_segment(range_end_bit));

      // Check whether it would be possible to merge this Range segment with a previous Range segment.
      if (inserter.is_empty() && segment_index > 0) {
        const BLBitSetSegment& prev = segment_data[segment_index - 1];
        if (prev.all_ones() && prev._range_end_word() == range_start_word) {
          // Merging is possible - this effectively decreases the index for insertion as we replace a previous segment.
          insert_index--;

          // Don't duplicate the code required to insert a new range here as there is few cases to handle.
          range_start_word = prev.start_word();
          goto InitRange;
        }
      }

      // We know that we cannot merge this range with the previous one. In general it's required to have at least
      // two segments in order to create a Range segment, otherwise a regular Dense segment must be used.
      if (range_size >= kSegmentBitCount * 2) {
InitRange:
        init_range_segment(inserter.current(), range_start_word, segment_end_word);
        inserter.advance();

        range_start_word = segment_end_word;
        range_start_bit = bit_index_of(range_start_word);

        // Discard all segments that the new Range segment overlaps.
        while (segment_index < segment_count && segment_data[segment_index].start_word() < range_start_word)
          segment_index++;

        // If the last discarded segment overruns this one, then we have to merge it
        if (segment_index != 0) {
          const BLBitSetSegment& prev = segment_data[segment_index - 1];
          if (prev.all_ones() && prev._range_end_word() > range_start_word) {
            inserter.prev()._set_range_end_word(prev._range_end_word());
            break;
          }
        }

        continue;
      }
    }

    // Create a Dense segment if the Range check failed.
    range_size = bl_min<uint32_t>(range_size, kSegmentBitCount - (range_start_bit & kSegmentBitMask));
    init_dense_segment_with_range(inserter.current(), range_start_bit, range_size);
    inserter.advance();

    if (segment_index < segment_count && has_segment_word_index(segment_data[segment_index], range_start_word)) {
      if (segment_data[segment_index].all_ones()) {
        // This cannot happen with a leading segment as the case must have been already detected in the previous loop.
        // We know that a Range segment spans always at least 2 segments, so we can safely terminate the loop even
        // when this is a middle segment followed by a trailing one.
        BL_ASSERT(is_bit_aligned_to_segment(range_start_bit));
        break;
      }
      else {
        bl::BitSetOps::bit_array_combine_words<bl::BitOperator::Or>(inserter.prev().data(), segment_data[segment_index].data(), kSegmentWordCount);
        segment_index++;
      }
    }

    range_start_bit += range_size;
    range_start_word = word_index_of(range_start_bit);
  } while (range_start_bit < range_end_bit);

  if (segment_index < segment_count) {
    BLBitSetSegment& next = segment_data[segment_index];
    if (next.all_ones() && next.start_word() <= inserter.prev().start_word()) {
      init_range_segment(inserter.current(), inserter.prev().start_word(), next.end_word());
      inserter.advance();
      segment_index++;
    }
  }

  return splice_internal(self, segment_data, segment_count, insert_index, segment_index - insert_index, inserter.segments(), inserter.count(), can_modify);
}

// bl::BitSet - API - Data Manipulation - Add Words
// ================================================

namespace bl {
namespace BitSetInternal {

// Inserts temporary segments into segment_data.
//
// SegmentData must have at least `segment_count + inserted_count` capacity - because the merged segments are inserted
// to `segment_data`. This function does merge from the end to ensure that we won't overwrite segments during merging.
static BL_INLINE void merge_inserted_segments(BLBitSetSegment* segment_data, uint32_t segment_count, const BLBitSetSegment* inserted_data, uint32_t inserted_count) noexcept {
  BLBitSetSegment* p = segment_data + segment_count + inserted_count;

  const BLBitSetSegment* segment_end = segment_data + segment_count;
  const BLBitSetSegment* inserted_end = inserted_data + inserted_count;

  while (segment_data != segment_end && inserted_data != inserted_end) {
    const BLBitSetSegment* src;
    if (segment_end[-1].start_word() > inserted_end[-1].start_word())
      src = --segment_end;
    else
      src = --inserted_end;
    *--p = *src;
  }

  while (inserted_data != inserted_end) {
    *--p = *--inserted_end;
  }

  // Make sure we ended at the correct index after merge.
  BL_ASSERT(p == segment_end);
}

} // {BitSetInternal}
} // {bl}

BL_API_IMPL BLResult bl_bit_set_add_words(BLBitSetCore* self, uint32_t start_word, const uint32_t* word_data, uint32_t word_count) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  BL_PROPAGATE(normalize_word_data_params(start_word, word_data, word_count));
  if (!word_count)
    return BL_SUCCESS;

  BLBitSetSegment sso_segment_data[3];
  BLBitSetSegment* segment_data = nullptr;
  uint32_t segment_count = 0;
  uint32_t segment_capacity = 0;

  bl::ScopedBufferTmp<sizeof(BLBitSetSegment) * kTmpSegmentDataSize> tmp_segment_buffer;
  DynamicSegmentInserter inserter;

  // SSO BitSet
  // ----------

  if (self->_d.sso()) {
    // Try some optimized SSO cases first if the BitSet is in SSO mode.
    if (is_sso_empty(self))
      return bl_bit_set_assign_words(self, start_word, word_data, word_count);

    if (!self->_d.is_bit_set_range()) {
      uint32_t sso_word_index = get_sso_word_index(self);
      uint32_t sso_word_count = get_sso_word_count_from_data(self->_d.u32_data, kSSOWordCount);

      if (start_word < sso_word_index) {
        uint32_t distance = sso_word_index - start_word;
        if (distance + sso_word_count <= kSSOWordCount) {
          BLBitSetCore tmp;
          init_sso_dense(&tmp, start_word);

          bl::MemOps::copy_forward_inline_t(tmp._d.u32_data, word_data, word_count);
          bl::MemOps::combine_small<bl::BitOperator::Or>(tmp._d.u32_data, self->_d.u32_data + distance, sso_word_count);

          self->_d = tmp._d;
          return BL_SUCCESS;
        }
      }
      else {
        uint32_t distance = start_word - sso_word_index;
        if (distance + word_count <= kSSOWordCount) {
          bl::MemOps::combine_small<bl::BitOperator::Or>(self->_d.u32_data + distance, word_data, word_count);
          return BL_SUCCESS;
        }
      }
    }

    segment_data = sso_segment_data;
    segment_count = make_segments_from_sso_bitset(segment_data, self);
  }
  else {
    BLBitSetImpl* self_impl = get_impl(self);

    segment_data = self_impl->segment_data();
    segment_count = self_impl->segment_count;

    if (!segment_count)
      return bl_bit_set_assign_words(self, start_word, word_data, word_count);

    if (is_impl_mutable(self_impl))
      segment_capacity = self_impl->segment_capacity;
  }

  // Dynamic BitSet (or SSO BitSet As Segments)
  // ------------------------------------------

  uint32_t start_word_aligned_to_segment = align_word_down_to_segment(start_word);
  uint32_t end_word_aligned_to_segment = align_word_up_to_segment(start_word + word_count);

  // Find the first segment we have to modify.
  BL_ASSERT(segment_count > 0);
  uint32_t segment_index = segment_count;

  if (segment_data[segment_count - 1].end_word() > start_word_aligned_to_segment)
    segment_index = uint32_t(bl::lower_bound(segment_data, segment_count, SegmentWordIndex{start_word_aligned_to_segment}));

  uint32_t word_index_end = start_word + word_count;
  uint32_t insert_segment_count = (end_word_aligned_to_segment - start_word_aligned_to_segment) / kSegmentWordCount;

  // We need a temporary storage for segments to be inserted in case that any of the existing segment overlaps with
  // word_data. In that case `tmp_segment_buffer` will be used to store such segments, and these segments will be merged
  // with BitSet at the end of the function.
  bool requires_temporary_storage = (segment_index != segment_count && insert_segment_count > 0);

  if (requires_temporary_storage) {
    BLBitSetSegment* p = static_cast<BLBitSetSegment*>(tmp_segment_buffer.alloc(insert_segment_count * sizeof(BLBitSetSegment)));
    if (BL_UNLIKELY(!p))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
    inserter.reset(p, insert_segment_count);
  }

  if (segment_count + insert_segment_count > segment_capacity) {
    // If there is not enough capacity or the BitSet is not mutable, do a more precise analysis.
    WordDataAnalysis analysis = analyze_word_data_for_combining(start_word, word_data, word_count, segment_data + segment_index, segment_count - segment_index);
    insert_segment_count = analysis.segment_count;

    if (segment_count + insert_segment_count > segment_capacity) {
      // Allocate a new Impl.
      BLBitSetCore tmp;
      BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(segment_count + insert_segment_count));
      BL_PROPAGATE(init_dynamic(&tmp, impl_size));

      BLBitSetImpl* new_impl = get_impl(&tmp);
      memcpy(new_impl->segment_data(), segment_data, segment_count * sizeof(BLBitSetSegment));
      segment_data = new_impl->segment_data();
      segment_capacity = get_impl(&tmp)->segment_capacity;

      replace_instance(self, &tmp);
    }
  }

  if (!requires_temporary_storage)
    inserter.reset(segment_data + segment_count, segment_capacity - segment_count);

  // Leading segment requires special handling if it doesn't start at a segment boundary.
  uint32_t word_index = start_word_aligned_to_segment;
  if (word_index != start_word) {
    uint32_t segment_word_offset = start_word - word_index;
    uint32_t segment_word_count = bl_min<uint32_t>(word_count, kSegmentWordCount - segment_word_offset);

    if (segment_index != segment_count && has_segment_word_index(segment_data[segment_index], word_index)) {
      if (!segment_data[segment_index].all_ones())
        bl::MemOps::combine_small<bl::BitOperator::Or>(segment_data[segment_index].data() + segment_word_offset, word_data, segment_word_count);

      if (segment_data[segment_index].end_word() == word_index + kSegmentWordCount)
        segment_index++;
    }
    else {
      init_dense_segment(inserter.current(), word_index);
      bl::MemOps::copy_forward_inline_t(inserter.current().data() + segment_word_offset, word_data, segment_word_count);
      inserter.advance();
    }

    word_data += segment_word_count;
    word_count -= segment_word_count;
    word_index += kSegmentWordCount;
  }

  // Main loop - word_index is aligned to a segment boundary, so process a single segment at a time.
  uint32_t word_index_aligned_end = word_index + bl::IntOps::align_down<uint32_t>(word_count, kSegmentWordCount);
  while (word_index != word_index_aligned_end) {
    // Combine with an existing segment, if there is an intersection.
    if (segment_index != segment_count) {
      BLBitSetSegment& current = segment_data[segment_index];
      if (has_segment_word_index(current, word_index)) {
        if (current.all_ones()) {
          // Terminate if the current Range segment completely subsumes the remaining words.
          if (current._range_end_word() >= word_index_end)
            break;

          uint32_t skip_count = current._range_end_word() - word_index;
          word_data += skip_count;
          word_index += skip_count;
        }
        else {
          bl::MemOps::combine_small<bl::BitOperator::Or>(current.data(), word_data, kSegmentWordCount);
          word_data += kSegmentWordCount;
          word_index += kSegmentWordCount;
        }

        segment_index++;
        continue;
      }
    }

    // The data doesn't overlap with an existing segment.
    QuickDataAnalysis qa = quick_data_analysis(word_data);
    uint32_t initial_word_index = word_index;

    // Advance here so we don't have to do it.
    word_data += kSegmentWordCount;
    word_index += kSegmentWordCount;

    // Handle a zero segment - this is a good case as BitSet builders can use more words than a
    // single segment occupies. So if the whole segment is zero, don't create it to save space.
    if (qa.is_zero())
      continue;

    // Handle a full segment - either merge with the previous range segment or try to find more
    // full segments and create a new one if merging is not possible.
    if (qa.is_full()) {
      uint32_t range_end_word = word_index_aligned_end;

      // Merge with the previous segment, if possible.
      if (segment_index > 0) {
        BLBitSetSegment& prev = segment_data[segment_index - 1];
        if (prev.all_ones() && prev._range_end_word() == initial_word_index) {
          prev._set_range_end_word(word_index);
          continue;
        }
      }

      // Merge with the next segment, if possible.
      BLBitSetSegment* next = nullptr;
      if (segment_index < segment_count) {
        next = &segment_data[segment_index];
        range_end_word = bl_min<uint32_t>(range_end_word, next->end_word());

        if (next->start_word() == word_index) {
          if (next->all_ones()) {
            next->_set_range_start_word(initial_word_index);
            continue;
          }
        }
      }

      // Analyze how many full segments are next to each other.
      while (word_index != range_end_word) {
        if (!is_segment_data_filled(word_data))
          break;

        word_data += kSegmentWordCount;
        word_index += kSegmentWordCount;
      }

      // Create a Range segment if two or more full segments are next to each other.
      if (initial_word_index - word_index > kSegmentWordCount) {
        if (next) {
          if (next->all_ones() && word_index >= next->start_word()) {
            next->_set_range_start_word(initial_word_index);
            continue;
          }

          if (word_index > next->start_word()) {
            init_range_segment(*next, initial_word_index, word_index);
            segment_index++;
            continue;
          }
        }

        // Insert a new Range segment.
        init_range_segment(inserter.current(), initial_word_index, word_index);
        inserter.advance();
        continue;
      }
    }

    // Insert a new Dense segment.
    init_dense_segment_with_data(inserter.current(), word_index - kSegmentWordCount, word_data - kSegmentWordCount);
    inserter.advance();
  }

  // Tail segment requires special handling if it doesn't end on a segment boundary.
  //
  // NOTE: We don't have to analyze the data as we already know it's not a full segment and that it's not empty.
  if (word_index < word_index_end) {
    if (segment_index != segment_count && has_segment_word_index(segment_data[segment_index], word_index)) {
      // Combine with an existing segment, if data and segment overlaps.
      BLBitSetSegment& current = segment_data[segment_index];
      if (!current.all_ones())
        bl::MemOps::combine_small<bl::BitOperator::Or>(current.data(), word_data, word_index_end - word_index_aligned_end);
      segment_index++;
    }
    else {
      // Insert a new Dense segment if data doesn't overlap with an existing segment.
      init_dense_segment(inserter.current(), word_index);
      bl::MemOps::copy_forward_inline_t(inserter.current().data(), word_data, word_index_end - word_index_aligned_end);
      inserter.advance();
    }
  }

  // Merge temporarily created segments to BitSet, if any.
  if (!inserter.is_empty() && requires_temporary_storage)
    merge_inserted_segments(segment_data, segment_count, inserter.segments(), inserter.index());

  get_impl(self)->segment_count = segment_count + inserter.index();
  reset_cached_cardinality(self);
  return BL_SUCCESS;
}

// bl::BitSet - API - Data Manipulation - Clear Bit
// ================================================

BL_API_IMPL BLResult bl_bit_set_clear_bit(BLBitSetCore* self, uint32_t bit_index) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (BL_UNLIKELY(bit_index == kInvalidIndex))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLBitSetSegment sso_segments[3];
  BLBitSetSegment* segment_data;

  bool can_modify = false;
  uint32_t segment_count;

  // SSO BitSet
  // ----------

  if (self->_d.sso()) {
    // SSO mode - first check whether the result of the operation can still be represented as SSO.
    if (self->_d.is_bit_set_range()) {
      Range rSSO = get_sso_range(self);

      // Nothing to do if the `bit_index` is outside of SSO range.
      if (!rSSO.has_index(bit_index))
        return BL_SUCCESS;

      // Shrink the SSO range if the given `bit_index` is at start/end.
      if (bit_index == rSSO.start) {
        // We would never allow an empty range like [12:12) - if this happens turn the bit set to an empty one.
        if (bit_index + 1 == rSSO.end)
          return init_sso_empty(self);
        else
          return set_sso_range_start(self, bit_index + 1);
      }

      if (bit_index == rSSO.end - 1)
        return set_sso_range_end(self, bit_index);

      // We know that the bit_index is somewhere inside the SSO range, but not at the start/end. If the range can
      // be represented as a dense SSO BitSet then it's guaranteed that the result would also fit in SSO storage.
      uint32_t first_word = word_index_of(rSSO.start);
      uint32_t last_word = word_index_of(rSSO.end - 1u);

      if (last_word - first_word < kSSOWordCount) {
        init_sso_dense(self, first_word);
        bl::BitSetOps::bit_array_fill(self->_d.u32_data, rSSO.start % bl::BitSetOps::kNumBits, rSSO.size());
        bl::BitSetOps::bit_array_clear_bit(self->_d.u32_data, bit_index - bit_index_of(first_word));
        return BL_SUCCESS;
      }
    }
    else {
      // This will always succeed. However, one thing that we have to guarantee is that if the first word is cleared
      // to zero we offset the start of the BitSet to the first non-zero word - and if the cleared bit was the last
      // one in the entire BitSet we turn it to an empty BitSet, which has always the same signature in SSO mode.
      SSODenseInfo info = get_sso_dense_info(self);

      if (!info.has_index(bit_index))
        return BL_SUCCESS;

      // No data shift necessary if the first word is non-zero after the operation.
      bl::BitSetOps::bit_array_clear_bit(self->_d.u32_data, bit_index - info.start_bit());
      if (self->_d.u32_data[0] != 0)
        return BL_SUCCESS;

      // If the first word was cleared out, it would most likely have to be shifted and start index updated.
      uint32_t i = 1;
      uint32_t buffer[kSSOWordCount];
      bl::MemOps::copy_forward_inline_t(buffer, self->_d.u32_data, kSSOWordCount);

      BL_NOUNROLL
      while (buffer[i] == 0)
        if (++i == info.word_count())
          return init_sso_empty(self);

      uint32_t start_word = bl_min<uint32_t>(info.start_word() + i, kSSOLastWord);
      uint32_t shift = start_word - info.start_word();
      return init_sso_dense_with_data(self, start_word, buffer + shift, info.word_count() - shift);
    }

    // The result of the operation cannot be represented as SSO BitSet.
    segment_data = sso_segments;
    segment_count = make_segments_from_sso_bitset(segment_data, self);
  }
  else {
    BLBitSetImpl* self_impl = get_impl(self);

    can_modify = is_impl_mutable(self_impl);
    segment_data = self_impl->segment_data();
    segment_count = self_impl->segment_count;
  }

  // Dynamic BitSet
  // --------------

  // Nothing to do if the bit of the given `bit_index` is not within any segment.
  uint32_t segment_index = uint32_t(bl::lower_bound(segment_data, segment_count, SegmentWordIndex{word_index_of(bit_index)}));
  if (segment_index >= segment_count)
    return BL_SUCCESS;

  BLBitSetSegment& segment = segment_data[segment_index];
  if (!has_segment_bit_index(segment, bit_index))
    return BL_SUCCESS;

  if (segment.all_ones()) {
    // The hardest case. If this segment is all ones, it's longer run of ones, which means that we will have to split
    // the segment into 2 or 3 segments, which would replace the original one.
    StaticSegmentInserter<3> inserter;

    uint32_t initial_segment_start_word = segment._range_start_word();
    uint32_t middle_segment_start_word = word_index_of(bit_index & ~kSegmentBitMask);
    uint32_t final_segment_start_word = middle_segment_start_word + kSegmentWordCount;

    // Calculate initial segment, if exists.
    if (initial_segment_start_word < middle_segment_start_word) {
      if (middle_segment_start_word - initial_segment_start_word <= kSegmentWordCount)
        init_dense_segment_with_ones(inserter.current(), initial_segment_start_word);
      else
        init_range_segment(inserter.current(), initial_segment_start_word, middle_segment_start_word);
      inserter.advance();
    }

    // Calculate middle segment (always exists).
    init_dense_segment_with_ones(inserter.current(), middle_segment_start_word);
    clear_segment_bit(inserter.current(), bit_index);
    inserter.advance();

    // Calculate final segment, if exists.
    if (final_segment_start_word < segment._range_end_word()) {
      if (segment._range_end_word() - final_segment_start_word <= kSegmentWordCount)
        init_dense_segment_with_ones(inserter.current(), final_segment_start_word);
      else
        init_range_segment(inserter.current(), final_segment_start_word, segment._range_end_word());
      inserter.advance();
    }

    return splice_internal(self, segment_data, segment_count, segment_index, 1, inserter.segments(), inserter.count(), can_modify);
  }
  else {
    if (can_modify) {
      clear_segment_bit(segment, bit_index);
      return reset_cached_cardinality(self);
    }

    // If the BitSet is immutable we have to create a new one. First copy all segments, then modify the required one.
    BLBitSetCore tmp = *self;
    BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(segment_count));

    BL_PROPAGATE(init_dynamic_with_data(self, impl_size, segment_data, segment_count));
    BLBitSetSegment& dst_segment = get_impl(self)->segment_data()[segment_index];
    clear_segment_bit(dst_segment, bit_index);
    return release_instance(&tmp);
  }
}

// bl::BitSet - API - Data Manipulation - Clear Range
// ==================================================

BL_API_IMPL BLResult bl_bit_set_clear_range(BLBitSetCore* self, uint32_t range_start_bit, uint32_t range_end_bit) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (BL_UNLIKELY(range_start_bit >= range_end_bit)) {
    if (range_start_bit > range_end_bit)
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    return BL_SUCCESS;
  }

  BLBitSetSegment sso_segments[3];
  BLBitSetSegment* segment_data;

  bool can_modify = false;
  uint32_t segment_count;

  // SSO BitSet
  // ----------

  if (self->_d.sso()) {
    // SSO mode - first check whether the result of the operation can still be represented as SSO.
    if (self->_d.is_bit_set_range()) {
      Range rSSO = get_sso_range(self);

      // NOP if the given range doesn't cross SSO range.
      Range intersection = rSSO.intersect(range_start_bit, range_end_bit);
      if (intersection.is_empty())
        return BL_SUCCESS;

      if (intersection.start == rSSO.start) {
        // If the given range intersects SSO range fully it would make the BitSet empty.
        if (intersection.end == rSSO.end)
          return init_sso_empty(self);
        else
          return set_sso_range_start(self, intersection.end);
      }

      if (intersection.end == rSSO.end)
        return set_sso_range_end(self, intersection.start);

      // We know that the range is somewhere inside the SSO range, but not at the start/end. If the range can be
      // represented as a dense SSO BitSet then it's guaranteed that the result would also fit in SSO storage.
      uint32_t dense_first_word = word_index_of(rSSO.start);
      uint32_t dense_last_word = word_index_of(rSSO.end - 1u);

      if (dense_first_word - dense_last_word < kSSOWordCount) {
        init_sso_dense(self, dense_first_word);
        bl::BitSetOps::bit_array_fill(self->_d.u32_data, rSSO.start % bl::BitSetOps::kNumBits, rSSO.size());
        bl::BitSetOps::bit_array_clear(self->_d.u32_data, intersection.start - bit_index_of(dense_first_word), intersection.size());
        return BL_SUCCESS;
      }
    }
    else {
      // This will always succeed. However, one thing that we have to guarantee is that if the first word is cleared
      // to zero we offset the start of the BitSet to the first non-zero word - and if the cleared bit was the last
      // one in the entire BitSet we turn it to an empty BitSet, which has always the same signature in SSO mode.
      SSODenseInfo info = get_sso_dense_info(self);

      uint32_t r_start = bl_max(range_start_bit, info.start_bit());
      uint32_t r_last = bl_min(range_end_bit - 1, info.last_bit());

      // Nothing to do if the given range is outside of the SSO range.
      if (r_start > r_last)
        return BL_SUCCESS;

      // No data shift necessary if the first word is non-zero after the operation.
      bl::BitSetOps::bit_array_clear(self->_d.u32_data, r_start - info.start_bit(), r_last - r_start + 1);
      if (self->_d.u32_data[0] != 0)
        return BL_SUCCESS;

      // If the first word was cleared out, it would most likely have to be shifted and start index updated.
      uint32_t i = 1;
      uint32_t buffer[kSSOWordCount];
      bl::MemOps::copy_forward_inline_t(buffer, self->_d.u32_data, kSSOWordCount);

      BL_NOUNROLL
      while (buffer[i] == 0)
        if (++i == info.word_count())
          return init_sso_empty(self);

      uint32_t start_word = bl_min<uint32_t>(info.start_word() + i, kSSOLastWord);
      uint32_t shift = start_word - info.start_word();
      return init_sso_dense_with_data(self, start_word, buffer + shift, info.word_count() - shift);
    }

    // The result of the operation cannot be represented as SSO BitSet.
    segment_data = sso_segments;
    segment_count = make_segments_from_sso_bitset(segment_data, self);
  }
  else {
    BLBitSetImpl* self_impl = get_impl(self);

    can_modify = is_impl_mutable(self_impl);
    segment_data = self_impl->segment_data();
    segment_count = self_impl->segment_count;
  }

  // Dynamic BitSet
  // --------------

  uint32_t range_start_word = word_index_of(range_start_bit);
  uint32_t range_last_word = word_index_of(range_end_bit - 1);
  uint32_t segment_index = uint32_t(bl::lower_bound(segment_data, segment_count, SegmentWordIndex{range_start_word}));

  // If no existing segment matches the range to clear, then there is nothing to clear.
  if (segment_index >= segment_count)
    return BL_SUCCESS;

  // Build an array of segments that will replace matching segments in the BitSet.
  StaticSegmentInserter<8> inserter;
  uint32_t insert_index = segment_index;

  do {
    const BLBitSetSegment& segment = segment_data[segment_index];
    uint32_t segment_start_word = segment.start_word();
    uint32_t segment_end_word = segment.end_word();

    // Discard non-intersecting areas.
    if (range_start_word < segment_start_word) {
      range_start_word = segment_start_word;
      range_start_bit = bit_index_of(range_start_word);

      if (range_start_word > range_last_word)
        break;
    }

    // If the range to clear completely overlaps this segment, remove it.
    if (range_last_word >= segment_end_word && range_start_bit == segment.start_bit())
      continue;

    // The range to clear doesn't completely overlap this segment, so clear the bits required.
    if (segment.all_ones()) {
      // More complicated case - we have to split the range segment into 1 or 4 segments depending on
      // where the input range intersects with the segment. See the illustration below that describes
      // a possible result of this operation:
      //
      // +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
      // |           Existing range segment spanning across multiple segment boundaries            | <- Input Segment
      // +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
      //
      //                                +--------------------------+
      // + <- Segment boundaries        |   Input range to clear   |                                 <- Clear Range
      //                                +--------------------------+
      //
      // +--------+--------+--------+--------+                 +--------+--------+--------+--------+
      // |    New leading range     |DenseSeg| Cleared entirely|DenseSeg|    New trailing range    | <- Output Segments
      // +--------+--------+--------+--------+                 +--------+--------+--------+--------+
      //
      // NOTE: Every time we insert a new segment, `segment_start_word` gets updated to reflect the remaining slice.

      // Handle a possible leading segment, which won't be cleared.
      uint32_t range_start_segment_word = align_word_down_to_segment(range_start_word);
      if (segment_start_word < range_start_segment_word) {
        if (range_start_segment_word - segment_start_word >= kSegmentWordCount * 2) {
          // If the leading range spans across two or more segments, insert a Range segment.
          init_range_segment(inserter.current(), segment_start_word, range_start_segment_word);
          inserter.advance();
        }
        else {
          // If the learing range covers only a single segment, insert a Dense segment.
          init_dense_segment_with_ones(inserter.current(), segment_start_word);
          inserter.advance();
        }

        // NOTE: There must be an intersection. This is just a leading segment we have to keep, but it's guaranteed
        // that at least one additional segment will be inserted. This is the main reason there is no `continue` here.
        segment_start_word = range_start_segment_word;
        BL_ASSERT(segment_start_word < segment_end_word);
      }

      // Handle the intersection with the beginning of the range to clear (if any), if it's not at the segment boundary.
      if (!is_bit_aligned_to_segment(range_start_bit)) {
        uint32_t dense_range_index = range_start_bit & kSegmentBitMask;
        uint32_t dense_range_count = bl_min(kSegmentBitCount - dense_range_index, range_end_bit - range_start_bit);

        init_dense_segment_with_ones(inserter.current(), segment_start_word);
        bl::BitSetOps::bit_array_clear(inserter.current().data(), dense_range_index, dense_range_count);
        inserter.advance();

        range_start_word = segment_start_word;
        range_start_bit = bit_index_of(range_start_word);

        // Nothing else to do with this segment if the rest is cleared entirely.
        if (segment_start_word >= segment_end_word || range_last_word >= segment_end_word)
          continue;
      }

      // Handle the intersection with the end of the range to clear (if any), if it's not at the segment boundary.
      segment_start_word = word_index_of(align_bit_down_to_segment(range_end_bit));
      if (segment_start_word >= segment_end_word)
        continue;

      if (!is_bit_aligned_to_segment(range_end_bit) && range_start_word <= range_last_word) {
        uint32_t dense_range_index = 0;
        uint32_t dense_range_count = range_end_bit & kSegmentBitMask;

        init_dense_segment_with_ones(inserter.current(), segment_start_word);
        bl::BitSetOps::bit_array_clear(inserter.current().data(), dense_range_index, dense_range_count);
        inserter.advance();

        segment_start_word += kSegmentWordCount;
        range_start_word = segment_start_word;
        range_start_bit = bit_index_of(range_start_word);

        // Nothing else to do with this segment if the rest is cleared entirely.
        if (segment_start_word >= segment_end_word || range_last_word >= segment_end_word)
          continue;
      }

      // Handle a possible trailing segment, which won't be cleared.
      uint32_t trailing_word_count = segment_end_word - segment_start_word;
      BL_ASSERT(trailing_word_count >= 1);

      if (trailing_word_count >= kSegmentWordCount * 2) {
        // If the trailing range spans across two or more segments, insert a Range segment.
        init_range_segment(inserter.current(), segment_start_word, segment_end_word);
        inserter.advance();
      }
      else {
        // If the trailing range covers only a single segment, insert a Dense segment.
        init_dense_segment_with_ones(inserter.current(), segment_start_word);
        inserter.advance();
      }
    }
    else {
      uint32_t segment_start_bit = range_start_bit & kSegmentBitMask;
      uint32_t segment_range = range_end_bit - range_start_bit;

      if (range_last_word < segment.end_word()) {
        // If this is the only segment to touch, and the BitSet is mutable, do it in place and return
        if (can_modify && insert_index == segment_index && inserter.is_empty()) {
          bl::BitSetOps::bit_array_clear(segment_data[segment_index].data(), segment_start_bit, segment_range);
          return reset_cached_cardinality(self);
        }
      }
      else {
        segment_range = kSegmentBitCount - segment_start_bit;
      }

      inserter.current() = segment;
      bl::BitSetOps::bit_array_clear(inserter.current().data(), segment_start_bit, segment_range);
      inserter.advance();
    }
  } while (++segment_index < segment_count);

  return splice_internal(self, segment_data, segment_count, insert_index, segment_index - insert_index, inserter.segments(), inserter.count(), can_modify);
}

/*
// TODO: Future API (BitSet).

// bl::BitSet - API - Data Manipulation - Combine
// ==============================================

BL_API_IMPL BLResult bl_bit_set_combine(BLBitSetCore* dst, const BLBitSetCore* a, const BLBitSetCore* b, BLBooleanOp boolean_op) noexcept {
  BL_ASSERT(dst->_d.is_bit_set());
  BL_ASSERT(a->_d.is_bit_set());
  BL_ASSERT(b->_d.is_bit_set());

  return BL_ERROR_NOT_IMPLEMENTED;
}
*/

// bl::BitSet - API - Builder Interface
// ====================================

BL_API_IMPL BLResult bl_bit_set_builder_commit(BLBitSetCore* self, BLBitSetBuilderCore* builder, uint32_t new_area_index) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  uint32_t area_shift = builder->_area_shift;
  uint32_t word_count = (1 << area_shift) / bl::BitSetOps::kNumBits;

  if (builder->_area_index != BLBitSetBuilderCore::kInvalidAreaIndex) {
    uint32_t start_word = word_index_of(builder->_area_index << area_shift);
    BL_PROPAGATE(bl_bit_set_add_words(self, start_word, builder->area_words(), word_count));
  }

  builder->_area_index = new_area_index;
  bl::MemOps::fill_inline_t(builder->area_words(), uint32_t(0), word_count);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_set_builder_add_range(BLBitSetCore* self, BLBitSetBuilderCore* builder, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitSetInternal;
  BL_ASSERT(self->_d.is_bit_set());

  if (start_bit >= end_bit)
    return BL_SUCCESS;

  uint32_t area_shift = builder->_area_shift;
  uint32_t last_bit = end_bit - 1;
  uint32_t area_index = start_bit >> area_shift;

  // Don't try to add long ranges here.
  if (area_index != (last_bit >> area_shift))
    return bl_bit_set_add_range(self, start_bit, end_bit);

  if (area_index != builder->_area_index)
    BL_PROPAGATE(bl_bit_set_builder_commit(self, builder, area_index));

  uint32_t area_bit_index = start_bit - (area_index << area_shift);
  bl::BitSetOps::bit_array_fill(builder->area_words(), area_bit_index, end_bit - start_bit);

  return BL_SUCCESS;
}

// bl::BitSet - Runtime Registration
// =================================

void bl_bit_set_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);
  bl_object_defaults[BL_OBJECT_TYPE_BIT_SET]._d.init_static(BLObjectInfo{BLBitSet::kSSOEmptySignature});
}
