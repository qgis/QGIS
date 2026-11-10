// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_BITSET_H
#define BLEND2D_BITSET_H

#include <blend2d/core/object.h>

//! \addtogroup bl_containers
//! \{

//! \name BLBitSet - Constants
//! \{

BL_DEFINE_ENUM(BLBitSetConstants) {
  //! Invalid bit-index.
  //!
  //! This is the only index that cannot be stored in `BLBitSet`.
  BL_BIT_SET_INVALID_INDEX = 0xFFFFFFFFu,

  //! Range mask used by `BLBitsetSegment::start` value - if set the segment is a range of all ones.
  BL_BIT_SET_RANGE_MASK = 0x80000000u,

  //! Number of words in a BLBitSetSegment.
  BL_BIT_SET_SEGMENT_WORD_COUNT = 4u
};

//! \}

//! \name BLBitSet - Structs
//! \{

//! BitSet segment.
//!
//! Segment provides either a dense set of bits starting at `start` or a range of bits all set to one. The start of
//! the segment is always aligned to segment size, which can be calculated as `32 * BL_BIT_SET_SEGMENT_WORD_COUNT`.
//! Even ranges are aligned to this value, thus up to 3 segments are used to describe a range that doesn't start/end
//! at the segment boundary.
//!
//! When the segment describes dense bits its size is always fixed and represents `32 * BL_BIT_SET_SEGMENT_WORD_COUNT`
//! bits, which is currently 128 bits. However, when the segment describes all ones, the first value in data `data[0]`
//! describes the last bit of the range, which means that an arbitrary range can be encoded within a single segment.
struct BLBitSetSegment {
  uint32_t _start_word;
  uint32_t _data[BL_BIT_SET_SEGMENT_WORD_COUNT];

#ifdef __cplusplus
  BL_INLINE_NODEBUG bool all_ones() const noexcept { return (_start_word & BL_BIT_SET_RANGE_MASK) != 0; }
  BL_INLINE_NODEBUG void clear_data() noexcept { memset(_data, 0, sizeof(_data)); }
  BL_INLINE_NODEBUG void fill_data() noexcept { memset(_data, 0xFF, sizeof(_data)); }

  BL_INLINE_NODEBUG uint32_t* data() noexcept { return _data; }
  BL_INLINE_NODEBUG const uint32_t* data() const noexcept { return _data; }
  BL_INLINE_NODEBUG uint32_t word_at(size_t index) const noexcept { return _data[index]; }

  BL_INLINE_NODEBUG uint32_t _range_start_word() const noexcept { return _start_word & ~BL_BIT_SET_RANGE_MASK; }
  BL_INLINE_NODEBUG uint32_t _range_end_word() const noexcept { return _data[0]; }

  BL_INLINE_NODEBUG uint32_t _dense_start_word() const noexcept { return _start_word; }
  BL_INLINE_NODEBUG uint32_t _dense_end_word() const noexcept { return _start_word + BL_BIT_SET_SEGMENT_WORD_COUNT; }

  BL_INLINE_NODEBUG void _set_range_start_word(uint32_t index) noexcept { _start_word = index; }
  BL_INLINE_NODEBUG void _set_range_end_word(uint32_t index) noexcept { _data[0] = index; }

  BL_INLINE_NODEBUG uint32_t start_word() const noexcept { return _start_word & ~BL_BIT_SET_RANGE_MASK; }
  BL_INLINE_NODEBUG uint32_t start_segment_id() const noexcept { return start_word() / BL_BIT_SET_SEGMENT_WORD_COUNT; }
  BL_INLINE_NODEBUG uint32_t start_bit() const noexcept { return _start_word * 32u; }

  BL_INLINE uint32_t end_word() const noexcept {
    uint32_t range_end = _range_end_word();
    uint32_t dense_end = _dense_end_word();
    return all_ones() ? range_end : dense_end;
  }

  BL_INLINE_NODEBUG uint32_t end_segment_id() const noexcept { return end_word() / BL_BIT_SET_SEGMENT_WORD_COUNT; }
  BL_INLINE_NODEBUG uint32_t last_bit() const noexcept { return end_word() * 32u - 1u; }
#endif
};

//! BitSet data view.
struct BLBitSetData {
  const BLBitSetSegment* segment_data;
  uint32_t segment_count;
  BLBitSetSegment sso_segments[3];

#ifdef __cplusplus
  BL_INLINE_NODEBUG bool is_empty() const noexcept {
    return segment_count == 0;
  }

  BL_INLINE void reset() noexcept {
    segment_data = nullptr;
    segment_count = 0;
  }
#endif
};

//! \}
//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLBitSet - C API
//! \{

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_bit_set_init(BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_init_move(BLBitSetCore* self, BLBitSetCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_init_weak(BLBitSetCore* self, const BLBitSetCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_init_range(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_destroy(BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_reset(BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_assign_move(BLBitSetCore* self, BLBitSetCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_assign_weak(BLBitSetCore* self, const BLBitSetCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_assign_deep(BLBitSetCore* self, const BLBitSetCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_assign_range(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_assign_words(BLBitSetCore* self, uint32_t start_word, const uint32_t* word_data, uint32_t word_count) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_set_is_empty(const BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_get_data(const BLBitSetCore* self, BLBitSetData* out) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_bit_set_get_segment_count(const BLBitSetCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API uint32_t BL_CDECL bl_bit_set_get_segment_capacity(const BLBitSetCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API uint32_t BL_CDECL bl_bit_set_get_cardinality(const BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_bit_set_get_cardinality_in_range(const BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_set_has_bit(const BLBitSetCore* self, uint32_t bit_index) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_set_has_bits_in_range(const BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_set_subsumes(const BLBitSetCore* a, const BLBitSetCore* b) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_set_intersects(const BLBitSetCore* a, const BLBitSetCore* b) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_set_get_range(const BLBitSetCore* self, uint32_t* start_out, uint32_t* end_out) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_set_equals(const BLBitSetCore* a, const BLBitSetCore* b) BL_NOEXCEPT_C;
BL_API int BL_CDECL bl_bit_set_compare(const BLBitSetCore* a, const BLBitSetCore* b) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_clear(BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_shrink(BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_optimize(BLBitSetCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_chop(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_add_bit(BLBitSetCore* self, uint32_t bit_index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_add_range(BLBitSetCore* self, uint32_t range_start_bit, uint32_t range_end_bit) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_add_words(BLBitSetCore* self, uint32_t start_word, const uint32_t* word_data, uint32_t word_count) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_clear_bit(BLBitSetCore* self, uint32_t bit_index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_clear_range(BLBitSetCore* self, uint32_t range_start_bit, uint32_t range_end_bit) BL_NOEXCEPT_C;

// TODO: Future API (BitSet).
/*
BL_API BLResult BL_CDECL bl_bit_set_combine(BLBitSetCore* dst, const BLBitSetCore* a, const BLBitSetCore* b, BLBooleanOp boolean_op) BL_NOEXCEPT_C;
*/

BL_API BLResult BL_CDECL bl_bit_set_builder_commit(BLBitSetCore* self, BLBitSetBuilderCore* builder, uint32_t new_area_index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_set_builder_add_range(BLBitSetCore* self, BLBitSetBuilderCore* builder, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! BitSet container [C API].
struct BLBitSetCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLBitSet)
};

//! BitSet builder [C API].
struct BLBitSetBuilderCore {
  //! Shift to get `_area_index` from bit index, equals to `log2(kBitCount)`.
  uint32_t _area_shift;
  //! Area index - index from 0...N where each index represents `kBitCount` bits.
  uint32_t _area_index;

  /*
  //! Area word data.
  uint32_t area_words[1 << (area_shift - 5)];
  */

#ifdef __cplusplus
  enum : uint32_t {
    kInvalidAreaIndex = 0xFFFFFFFFu
  };

  BL_INLINE_NODEBUG uint32_t* area_words() noexcept { return reinterpret_cast<uint32_t*>(this + 1); }
  BL_INLINE_NODEBUG const uint32_t* area_words() const noexcept { return reinterpret_cast<const uint32_t*>(this + 1); }
#endif
};

//! \}

//! \cond INTERNAL
//! \name BLBitSet - Internals
//! \{

//! BitSet container [Impl].
struct BLBitSetImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Count of used segments in `segment_data`.
  uint32_t segment_count;
  //! Count of allocated segments in `segment_data`.
  uint32_t segment_capacity;

#ifdef __cplusplus
  BL_INLINE_NODEBUG BLBitSetSegment* segment_data() const noexcept { return (BLBitSetSegment*)(this + 1); }
  BL_INLINE_NODEBUG BLBitSetSegment* segment_data_end() const noexcept { return segment_data() + segment_count; }
#endif
};

//! \}
//! \endcond

//! \}

//! \addtogroup bl_containers
//! \{

//! \name BLBitSet - C++ API
//! \{
#ifdef __cplusplus

//! BitSet container [C++ API].
//!
//! BitSet container implements sparse BitSet that consists of segments, where each segment represents either dense
//! range of bits or a range of bits that are all set to one. In addition, the BitSet provides also a SSO mode, in
//! which it's possible to store up to 64 dense bits (2 consecutive BitWords) in the whole addressable range or a
//! range of ones. SSO mode optimizes use cases, in which very small BitSets are needed.
//!
//! The BitSet itself has been optimized for Blend2D use cases, which are the following:
//!
//!   1. Representing character coverage of fonts and unicode text. This use-case requires sparseness and ranges as
//!      some fonts, especially those designed for CJK use, provide thousands of glyphs that have pretty high code
//!      points - using BLBitArray would be very wasteful in this particular case.
class BLBitSet final : public BLBitSetCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  enum : uint32_t {
    //! Number of words that can be used by SSO dense representation (2 words => 64 bits).
    kSSOWordCount = 2,
    kSSODenseSignature = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_BIT_SET),
    kSSOEmptySignature = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_BIT_SET) | BL_OBJECT_INFO_R_FLAG
  };

  [[nodiscard]]
  BL_INLINE_NODEBUG BLBitSetImpl* _impl() const noexcept { return static_cast<BLBitSetImpl*>(_d.impl); }

  BL_INLINE_NODEBUG void _init_range_internal(uint32_t start_bit = 0u, uint32_t end_bit = 0u) noexcept {
    _d.init_static(BLObjectInfo{kSSOEmptySignature});
    _d.u32_data[0] = start_bit;
    _d.u32_data[1] = end_bit;
  }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE BLBitSet() noexcept { _init_range_internal(); }

  BL_INLINE BLBitSet(BLBitSet&& other) noexcept {
    _d = other._d;
    other._init_range_internal();
  }

  BL_INLINE BLBitSet(const BLBitSet& other) noexcept { bl_bit_set_init_weak(this, &other); }

  BL_INLINE BLBitSet(uint32_t start_bit, uint32_t end_bit) noexcept {
    uint32_t mask = uint32_t(-int32_t(start_bit < end_bit));
    _init_range_internal(start_bit & mask, end_bit & mask);
  }

  //! Destroys the BitSet.
  BL_INLINE ~BLBitSet() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_bit_set_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Tests whether the BitSet has a content.
  //!
  //! \note This is essentially the opposite of `is_empty()`.
  BL_INLINE explicit operator bool() const noexcept { return !is_empty(); }

  //! Move assignment.
  //!
  //! \note The `other` BitSet is reset by move assignment, so its state after the move operation is the same as
  //! a default constructed BitSet.
  BL_INLINE BLBitSet& operator=(BLBitSet&& other) noexcept { bl_bit_set_assign_move(this, &other); return *this; }
  //! Copy assignment, performs weak copy of the data held by the `other` BitSet.
  BL_INLINE BLBitSet& operator=(const BLBitSet& other) noexcept { bl_bit_set_assign_weak(this, &other); return *this; }

  BL_INLINE bool operator==(const BLBitSet& other) const noexcept { return  equals(other); }
  BL_INLINE bool operator!=(const BLBitSet& other) const noexcept { return !equals(other); }
  BL_INLINE bool operator<(const BLBitSet& other) const noexcept { return compare(other) < 0; }
  BL_INLINE bool operator<=(const BLBitSet& other) const noexcept { return compare(other) <= 0; }
  BL_INLINE bool operator>(const BLBitSet& other) const noexcept { return compare(other) > 0; }
  BL_INLINE bool operator>=(const BLBitSet& other) const noexcept { return compare(other) >= 0; }

  //! \}

  //! \name Common Functionality
  //! \{

  //! Clears the content of the BitSet and releases its data.
  //!
  //! After reset the BitSet content matches a default constructed instance.
  BL_INLINE BLResult reset() noexcept { return bl_bit_set_reset(this); }

  //! Swaps the content of this string with the `other` string.
  BL_INLINE void swap(BLBitSetCore& other) noexcept { _d.swap(other._d); }

  //! \name Accessors
  //! \{

  //! Tests whether the BitSet is empty (has no content).
  //!
  //! Returns `true` if the BitSet's size is zero.
  BL_INLINE bool is_empty() const noexcept { return bl_bit_set_is_empty(this); }

  //! Returns the number of segments this BitSet occupies.
  //!
  //! \note If the BitSet is in SSO mode then the returned value is the number of segments the BitSet would occupy
  //! when the BitSet was converted to dynamic.
  BL_INLINE uint32_t segment_count() const noexcept { return bl_bit_set_get_segment_count(this); }

  //! Returns the number of segments this BitSet has allocated.
  //!
  //! \note If the BitSet is in SSO mode the returned value is always zero.
  BL_INLINE uint32_t segment_capacity() const noexcept { return _d.sso() ? 0 : _impl()->segment_capacity; }

  //! Returns the range of the BitSet as `[start_out, end_out)`.
  //!
  //! Returns true if the query was successful, false if the BitSet is empty.
  BL_INLINE bool get_range(uint32_t* start_out, uint32_t* end_out) const noexcept { return bl_bit_set_get_range(this, start_out, end_out); }

  //! Returns the number of bits set in the BitSet.
  BL_INLINE uint32_t cardinality() const noexcept { return bl_bit_set_get_cardinality(this); }

  //! Returns the number of bits set in the given `[start_bit, end_bit)` range.
  BL_INLINE uint32_t cardinality_in_range(uint32_t start_bit, uint32_t end_bit) const noexcept { return bl_bit_set_get_cardinality_in_range(this, start_bit, end_bit); }

  //! Stores a normalized BitSet data represented as segments into `out`.
  //!
  //! If the BitSet is in SSO mode, it will be converter to temporary segments provided by `BLBitSetData::sso_segments`,
  //! if the BitSet is in dynamic mode (already contains segments) then only a pointer to the data will be stored into
  //! `out`.
  //!
  //! \remarks The data written into `out` can reference the data in the BitSet, thus the BitSet cannot be manipulated
  //! during the use of the data. This function is ideal for inspecting the content of the BitSet in a unique way and
  //! for implementing iterators that don't have to be aware of how SSO data is represented and used.
  BL_INLINE BLResult get_data(BLBitSetData* out) const noexcept { return bl_bit_set_get_data(this, out); }

  //! \}

  //! \name Test Operations
  //! \{

  //! Returns a bit-value at the given `bit_index`.
  BL_INLINE bool has_bit(uint32_t bit_index) const noexcept { return bl_bit_set_has_bit(this, bit_index); }
  //! Returns whether the bit-set has at least on bit in the given range `[start:end)`.
  BL_INLINE bool has_bits_in_range(uint32_t start_bit, uint32_t end_bit) const noexcept { return bl_bit_set_has_bits_in_range(this, start_bit, end_bit); }

  //! Returns whether this BitSet subsumes `other`.
  BL_INLINE bool subsumes(const BLBitSetCore& other) const noexcept { return bl_bit_set_subsumes(this, &other); }
  //! Returns whether this BitSet intersects with `other`.
  BL_INLINE bool intersects(const BLBitSetCore& other) const noexcept { return bl_bit_set_intersects(this, &other); }

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Returns whether this BitSet and `other` are bitwise equal.
  BL_INLINE bool equals(const BLBitSetCore& other) const noexcept { return bl_bit_set_equals(this, &other); }
  //! Compares this BitSet with `other` and returns either `-1`, `0`, or `1`.
  BL_INLINE int compare(const BLBitSetCore& other) const noexcept { return bl_bit_set_compare(this, &other); }

  //! \}

  //! \name Content Manipulation
  //! \{

  //! Move assignment, the same as `operator=`, but returns a `BLResult` instead of `this`.
  BL_INLINE BLResult assign(BLBitSetCore&& other) noexcept { return bl_bit_set_assign_move(this, &other); }

  //! Copy assignment, the same as `operator=`, but returns a `BLResult` instead of `this`.
  BL_INLINE BLResult assign(const BLBitSetCore& other) noexcept { return bl_bit_set_assign_weak(this, &other); }

  //! Copy assignment, but creates a deep copy of the `other` BitSet instead of weak copy.
  BL_INLINE BLResult assign_deep(const BLBitSetCore& other) noexcept { return bl_bit_set_assign_deep(this, &other); }

  //! Replaces the content of the BitSet by the given range.
  BL_INLINE BLResult assign_range(uint32_t start_bit, uint32_t end_bit) noexcept { return bl_bit_set_assign_range(this, start_bit, end_bit); }

  //! Replaces the content of the BitSet by bits specified by `word_data` of size `word_count` [the size is in uint32_t units].
  BL_INLINE BLResult assign_words(uint32_t start_word, const uint32_t* word_data, uint32_t word_count) noexcept { return bl_bit_set_assign_words(this, start_word, word_data, word_count); }

  //! Clears the content of the BitSet without releasing its dynamically allocated data, if possible.
  BL_INLINE BLResult clear() noexcept { return bl_bit_set_clear(this); }

  //! Shrinks the capacity of the BitSet to match the actual content.
  BL_INLINE BLResult shrink() noexcept { return bl_bit_set_shrink(this); }

  //! Optimizes the BitSet by clearing unused pages and by merging continuous pages, without reallocating the BitSet.
  //! This functions should always return `BL_SUCCESS`.
  BL_INLINE BLResult optimize() noexcept { return bl_bit_set_optimize(this); }

  //! Bounds the BitSet to the given interval `[start:end)`.
  BL_INLINE BLResult chop(uint32_t start_bit, uint32_t end_bit) noexcept { return bl_bit_set_chop(this, start_bit, end_bit); }
  //! Truncates the BitSet so it's maximum bit set is less than `n`.
  BL_INLINE BLResult truncate(uint32_t n) noexcept { return bl_bit_set_chop(this, 0, n); }

  //! Adds a bit to the BitSet at the given `index`.
  BL_INLINE BLResult add_bit(uint32_t bit_index) noexcept { return bl_bit_set_add_bit(this, bit_index); }
  //! Adds a range of bits `[range_start_bit:range_end_bit)` to the BitSet.
  BL_INLINE BLResult add_range(uint32_t range_start_bit, uint32_t range_end_bit) noexcept { return bl_bit_set_add_range(this, range_start_bit, range_end_bit); }
  //! Adds a dense data to the BitSet starting a bit index `start`.
  BL_INLINE BLResult add_words(uint32_t start_word, const uint32_t* word_data, uint32_t word_count) noexcept { return bl_bit_set_add_words(this, start_word, word_data, word_count); }

  //! Clears a bit in the BitSet at the given `index`.
  BL_INLINE BLResult clear_bit(uint32_t bit_index) noexcept { return bl_bit_set_clear_bit(this, bit_index); }
  //! Clears a range of bits `[range_start_bit:range_end_bit)` in the BitSet.
  BL_INLINE BLResult clear_range(uint32_t range_start_bit, uint32_t range_end_bit) noexcept { return bl_bit_set_clear_range(this, range_start_bit, range_end_bit); }

  /*
  // TODO: Future API (BitSet).

  BL_INLINE BLResult and_(const BLBitSetCore& other) noexcept { return bl_bit_set_combine(this, this, &other, BL_BOOLEAN_OP_AND); }
  BL_INLINE BLResult or_(const BLBitSetCore& other) noexcept { return bl_bit_set_combine(this, this, &other, BL_BOOLEAN_OP_OR); }
  BL_INLINE BLResult xor_(const BLBitSetCore& other) noexcept { return bl_bit_set_combine(this, this, &other, BL_BOOLEAN_OP_XOR); }
  BL_INLINE BLResult and_not(const BLBitSetCore& other) noexcept { return bl_bit_set_combine(this, this, &other, BL_BOOLEAN_OP_AND_NOT); }
  BL_INLINE BLResult not_and(const BLBitSetCore& other) noexcept { return bl_bit_set_combine(this, this, &other, BL_BOOLEAN_OP_NOT_AND); }
  BL_INLINE BLResult combine(const BLBitSetCore& other, BLBooleanOp boolean_op) noexcept { return bl_bit_set_combine(this, this, &other, boolean_op); }

  static BL_INLINE BLResult and_(BLBitSetCore& dst, const BLBitSetCore& a, const BLBitSetCore& b) noexcept { return bl_bit_set_combine(&dst, &a, &b, BL_BOOLEAN_OP_AND); }
  static BL_INLINE BLResult or_(BLBitSetCore& dst, const BLBitSetCore& a, const BLBitSetCore& b) noexcept { return bl_bit_set_combine(&dst, &a, &b, BL_BOOLEAN_OP_OR); }
  static BL_INLINE BLResult xor_(BLBitSetCore& dst, const BLBitSetCore& a, const BLBitSetCore& b) noexcept { return bl_bit_set_combine(&dst, &a, &b, BL_BOOLEAN_OP_XOR); }
  static BL_INLINE BLResult and_not(BLBitSetCore& dst, const BLBitSetCore& a, const BLBitSetCore& b) noexcept { return bl_bit_set_combine(&dst, &a, &b, BL_BOOLEAN_OP_AND_NOT); }
  static BL_INLINE BLResult not_and(BLBitSetCore& dst, const BLBitSetCore& a, const BLBitSetCore& b) noexcept { return bl_bit_set_combine(&dst, &a, &b, BL_BOOLEAN_OP_NOT_AND); }
  static BL_INLINE BLResult combine(BLBitSetCore& dst, const BLBitSetCore& a, const BLBitSetCore& b, BLBooleanOp boolean_op) noexcept { return bl_bit_set_combine(&dst, &a, &b, boolean_op); }
  */

  //! \}
};

//! BitSet builder [C++ API].
//!
//! BitSet builder is a low-level utility class that can be used to efficiently build a BitSet in C++. It maintains
//! a configurable buffer (called area) where intermediate bits are set, which are then committed to BitSet when
//! an added bit/range is outside of the area or when user is done with BitSet building. The commit uses \ref
//! bl_bit_set_builder_commit() function, which was specifically designed for `BLBitSetBuilderT<BitCount>` in addition
//! to the `BLBitSetBuilder` alias.
//!
//! \note The destructor doesn't do anything. If there are still bits to be committed, they will be lost.
template<uint32_t BitCount>
class BLBitSetBuilderT final : public BLBitSetBuilderCore {
public:
  static_assert(BitCount >= 128, "BitCount must be at least 128");
  static_assert((BitCount & (BitCount - 1)) == 0, "BitCount must be a power of 2");

  //! \name Constants
  //! \{

  enum : uint32_t {
    kAreaBitCount = BitCount,
    kAreaWordCount = kAreaBitCount / uint32_t(sizeof(uint32_t) * 8u),
    kAreaShift = BLInternal::ConstCTZ<kAreaBitCount>::kValue
  };

  //! \}

  //! \name Members
  //! \{

  //! Area words data.
  uint32_t _area_words[kAreaWordCount];

  //! BitSet we are building.
  //!
  //! \note This member is not part of C API. C API requires both BLBitSetCore and BLBitSetBuilderCore to be passed.
  BLBitSetCore* _bit_set;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Constructs a new BitSet builder having no BitSet assigned.
  BL_INLINE BLBitSetBuilderT() noexcept {
    _bit_set = nullptr;
    _area_shift = kAreaShift;
    _area_index = kInvalidAreaIndex;
  }

  //! Constructs a new BitSet builder having the given `bit_set` assigned.
  //!
  //! \note The builder only stores a pointer to the `bit_set` - the user must guarantee to not destroy the BitSet
  //! before the builder is destroyed or reset.
  BL_INLINE explicit BLBitSetBuilderT(BLBitSetCore* bit_set) noexcept {
    _bit_set = bit_set;
    _area_shift = kAreaShift;
    _area_index = kInvalidAreaIndex;
  }

  BL_INLINE BLBitSetBuilderT(const BLBitSetBuilderT&) = delete;
  BL_INLINE BLBitSetBuilderT& operator=(const BLBitSetBuilderT&) = delete;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns whether the BitSet builder is valid, which means that is has an associated `BLBitSet` instance.
  BL_INLINE bool is_valid() const noexcept { return _bit_set != nullptr; }
  //! Returns an associated `BLBitSet` instance that this builder commits to.
  BL_INLINE BLBitSet* bit_set() const noexcept { return static_cast<BLBitSet*>(_bit_set); }

  //! \}

  //! \name Builder Interface
  //! \{

  BL_INLINE BLResult reset() noexcept {
    _bit_set = nullptr;
    _area_shift = kAreaShift;
    _area_index = kInvalidAreaIndex;
    return BL_SUCCESS;
  }

  BL_INLINE BLResult reset(BLBitSetCore* bit_set) noexcept {
    _bit_set = bit_set;
    _area_shift = kAreaShift;
    _area_index = kInvalidAreaIndex;
    return BL_SUCCESS;
  }

  //! Adds a bit to the area maintained by BitSet builder.
  //!
  //! If the area of `bit_index` is different compared to the current active area, the current area will be
  //! committed to the BitSet. This is actually the only operation that can return \ref BL_ERROR_OUT_OF_MEMORY.
  BL_INLINE BLResult add_bit(uint32_t bit_index) noexcept {
    uint32_t area_index = bit_index / kAreaBitCount;
    if (_area_index != area_index)
      BL_PROPAGATE(bl_bit_set_builder_commit(_bit_set, this, area_index));

    bit_index &= kAreaBitCount - 1;
    _area_words[bit_index / 32u] |= uint32_t(0x80000000u) >> (bit_index % 32u);
    return BL_SUCCESS;
  }

  //! Adds a `[start_bit, end_bit)` range of bits to the BitSet.
  //!
  //! If the range is relatively small and fits into a single builder area, it will be added to that area.
  //! On the other hand, if the range is large, the area will be kept and the builder would call \ref
  //! BLBitSet::add_range() instead. If the are of the range is different compared to the current active area,
  //! the data in the current active area will be committed.
  BL_INLINE BLResult add_range(uint32_t start_bit, uint32_t end_bit) noexcept {
    return bl_bit_set_builder_add_range(_bit_set, this, start_bit, end_bit);
  }

  //! Commits changes in the current active area to the BitSet.
  //!
  //! \note This must be called in order to finalize building the BitSet. If this function is not called the
  //! BitSet could have missing bits that are in the current active area.
  BL_INLINE BLResult commit() noexcept {
    return bl_bit_set_builder_commit(_bit_set, this, kInvalidAreaIndex);
  }

  //! Similar to \ref commit(), but the additional parameter `new_area_index` will be used to set the current
  //! active area.
  BL_INLINE BLResult commit(uint32_t new_area_index) noexcept {
    return bl_bit_set_builder_commit(_bit_set, this, new_area_index);
  }

  //! \}
};

//! BitSet builder [C++ API] that is configured to have a temporary storage of 512 bits.
using BLBitSetBuilder = BLBitSetBuilderT<512>;

//! BitSet word iterator [C++ API].
//!
//! Low-level iterator that sees a BitSet as an array of bit words. It only iterates non-zero words and
//! returns zero at the end of iteration.
//!
//! A simple way of printing all non-zero words of a BitWord:
//!
//! ```
//! BLBitSet set;
//! set.add_range(100, 200);
//!
//! BLBitSetWordIterator it(set);
//! while (uint32_t bits = it.next_word()) {
//!   printf("{WordIndex: %u, WordData: %08X }\n", it.word_index(), bits);
//! }
//! ```
class BLBitSetWordIterator final {
public:
  //! \name Members
  //! \{

  const BLBitSetSegment* _segment_ptr;
  const BLBitSetSegment* _segment_end;
  BLBitSetData _data;
  uint32_t _word_index;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed iterator, not initialized to iterate any BitSet.
  BL_INLINE BLBitSetWordIterator() noexcept { reset(); }
  //! Creates an iterator, that will iterate the given `bit_set`.
  //!
  //! \note The `bit_set` cannot change or be destroyed during iteration.
  BL_INLINE BLBitSetWordIterator(const BLBitSetCore& bit_set) noexcept { reset(bit_set); }
  //! Creates a copy of `other` iterator.
  BL_INLINE BLBitSetWordIterator(const BLBitSetWordIterator& other) noexcept = default;

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE BLBitSetWordIterator& operator=(const BLBitSetWordIterator& other) noexcept = default;

  //! \}

  //! \name Reset
  //! \{

  //! Resets the iterator (puts it into a default constructed state).
  BL_INLINE void reset() noexcept {
    _segment_ptr = nullptr;
    _segment_end = nullptr;
    _data.reset();
    _word_index = 0;
  }

  //! Reinitializes the iterator to iterate the given `bit_set`, from the beginning.
  BL_INLINE void reset(const BLBitSetCore& bit_set) noexcept {
    bl_bit_set_get_data(&bit_set, &_data);
    _segment_ptr = _data.segment_data;
    _segment_end = _data.segment_data + _data.segment_count;
    _word_index = _segment_ptr != _segment_end ? uint32_t(_segment_ptr->start_word() - 1u) : uint32_t(0xFFFFFFFFu);
  }

  //! \}

  //! \name Iterator Interface
  //! \{

  //! Returns the next (or the first, if called the first time) non-zero word of the BitSet or zero if the
  //! iteration ended.
  //!
  //! Use `word_index()` to get the index (in word units) of the word returned.
  BL_INLINE uint32_t next_word() noexcept {
    if (_segment_ptr == _segment_end)
      return 0;

    _word_index++;
    for (;;) {
      if (_segment_ptr->all_ones()) {
        if (_word_index < _segment_ptr->_range_end_word())
          return 0xFFFFFFFFu;
      }
      else {
        uint32_t end_word = _segment_ptr->_dense_end_word();
        while (_word_index < end_word) {
          uint32_t bits = _segment_ptr->_data[_word_index & (BL_BIT_SET_SEGMENT_WORD_COUNT - 1u)];
          if (bits != 0u)
            return bits;
          _word_index++;
        }
      }

      if (++_segment_ptr == _segment_end)
        return 0;
      _word_index = _segment_ptr->start_word();
    }
  }

  //! Returns the current bit index of a word returned by `next_word()`.
  BL_INLINE uint32_t bit_index() const noexcept { return _word_index * 32u; }

  //! Returns the current word index of a word returned by `next_word()`.
  BL_INLINE uint32_t word_index() const noexcept { return _word_index; }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_BITSET_H
