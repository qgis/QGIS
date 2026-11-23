// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_BITSET_P_H
#define BLEND2D_BITSET_P_H

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/bitset.h>
#include <blend2d/core/object_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name BLBitSet - Types
//! \{

typedef ParametrizedBitOps<BitOrder::kMSB, uint32_t> BitSetOps;

//! \}

namespace BitSetInternal {

//! \name BLBitSet - Constants
//! \{

static constexpr uint32_t kInvalidIndex = BL_BIT_SET_INVALID_INDEX;
static constexpr uint32_t kSegmentWordCount = BL_BIT_SET_SEGMENT_WORD_COUNT;

static constexpr uint32_t kSSOWordCount = BLBitSet::kSSOWordCount;
static constexpr uint32_t kLastWord = 0xFFFFFFFFu / BitSetOps::kNumBits;

//! Index of the last large word index of SSE Dense BitSet.
//!
//! We don't want to overflow the maximum addressable word, so large word index cannot exceed this value.
static constexpr uint32_t kSSOLastWord = kLastWord - kSSOWordCount + 1;

static constexpr uint32_t kSegmentBitCount = BL_BIT_SET_SEGMENT_WORD_COUNT * IntOps::bit_size_of<uint32_t>();
static constexpr uint32_t kSegmentBitMask = kSegmentBitCount - 1;

/*
// TODO: Unused

static constexpr uint32_t kMaxSegmentCount = (kLastWord + 1) / kSegmentWordCount;
static constexpr uint32_t kMaxImplSize = uint32_t(sizeof(BLBitSetImpl) + kMaxSegmentCount * sizeof(BLBitSetSegment));
*/

//! \}

//! \name BLBitSet - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLBitSetImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

static BL_INLINE BLResult free_impl(BLBitSetImpl* impl) noexcept {
  return ObjectInternal::free_impl(impl);
}

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLBitSetImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLBitSet - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE_NODEBUG BLBitSetImpl* get_impl(const BLBitSetCore* self) noexcept {
  return static_cast<BLBitSetImpl*>(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLBitSetCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLBitSetCore* self) noexcept {
  return self->_d.is_ref_counted_object() ? release_impl<RCMode::kForce>(get_impl(self)) : BLResult(BL_SUCCESS);
}

static BL_INLINE BLResult replace_instance(BLBitSetCore* self, const BLBitSetCore* other) noexcept {
  // NOTE: UBSAN doesn't like casting the impl in case the BitSet is in SSO mode, so wait with the cast.
  void* impl = static_cast<void*>(self->_d.impl);
  BLObjectInfo info = self->_d.info;

  self->_d = other->_d;
  return info.is_ref_counted_object() ? release_impl<RCMode::kForce>(static_cast<BLBitSetImpl*>(impl)) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLBitSet - Internals - SSO Commons
//! \{

//! SSO BitSet is empty when the first 2 words are zero.
//!
//! This check handles both dense SSO data and SSO ranges.
static BL_INLINE_NODEBUG bool is_sso_empty(const BLBitSetCore* self) noexcept { return self->_d.u64_data[0] == 0; }

static BL_INLINE_NODEBUG uint32_t get_sso_range_start(const BLBitSetCore* self) noexcept { return self->_d.u32_data[0]; }
static BL_INLINE_NODEBUG uint32_t get_sso_range_end(const BLBitSetCore* self) noexcept { return self->_d.u32_data[1]; }
static BL_INLINE_NODEBUG uint32_t get_sso_word_index(const BLBitSetCore* self) noexcept { return self->_d.u32_data[2]; }

//! \}

//! \name BLBitSet - Internals - SSO Range
//! \{

struct Range {
  uint32_t start;
  uint32_t end;

  BL_INLINE_NODEBUG void reset(uint32_t r_start, uint32_t r_end) noexcept {
    start = r_start;
    end = r_end;
  }

  BL_INLINE_NODEBUG bool valid() const noexcept { return start < end; }
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return start >= end; }

  BL_INLINE_NODEBUG bool has_index(uint32_t index) const noexcept { return index - start < end - start; }
  BL_INLINE_NODEBUG uint32_t size() const noexcept { return end - start; }

  BL_INLINE_NODEBUG Range intersect(uint32_t r_start, uint32_t r_end) const noexcept { return Range{bl_max(start, r_start), bl_min(end, r_end)}; }
  BL_INLINE_NODEBUG Range intersect(const Range& other) const noexcept { return intersect(other.start, other.end); }

  BL_INLINE void normalize() noexcept {
    uint32_t mask = IntOps::bool_as_mask<uint32_t>(valid());
    start &= mask;
    end &= mask;
  }
};

static BL_INLINE Range get_sso_range(const BLBitSetCore* self) noexcept {
  return Range{self->_d.u32_data[0], self->_d.u32_data[1]};
}

static BL_INLINE BLResult set_sso_range_start(BLBitSetCore* self, uint32_t value) noexcept {
  self->_d.u32_data[0] = value;
  return BL_SUCCESS;
}

static BL_INLINE BLResult set_sso_range_end(BLBitSetCore* self, uint32_t value) noexcept {
  self->_d.u32_data[1] = value;
  return BL_SUCCESS;
}

static BL_INLINE BLResult set_sso_range(BLBitSetCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  self->_d.u32_data[0] = start_bit;
  self->_d.u32_data[1] = end_bit;
  return BL_SUCCESS;
}

//! \}

//! \name BLBitSet - Internals - SSO Dense
//! \{

struct SSODenseInfo {
  uint32_t _word_index;
  uint32_t _word_count;

  BL_INLINE_NODEBUG uint32_t start_word() const noexcept { return _word_index; }
  BL_INLINE_NODEBUG uint32_t last_word() const noexcept { return _word_index + _word_count - 1; }
  BL_INLINE_NODEBUG uint32_t end_word() const noexcept { return _word_index + _word_count; }
  BL_INLINE_NODEBUG uint32_t word_count() const noexcept { return _word_count; }

  BL_INLINE_NODEBUG uint32_t start_bit() const noexcept { return _word_index * BitSetOps::kNumBits; }
  BL_INLINE_NODEBUG uint32_t last_bit() const noexcept { return (_word_index + _word_count) * BitSetOps::kNumBits - 1; }

  BL_INLINE_NODEBUG bool has_index(uint32_t index) const noexcept { return (index / BitSetOps::kNumBits) - _word_index < _word_count; }
};

static BL_INLINE SSODenseInfo get_sso_dense_info(const BLBitSetCore* self) noexcept {
  return SSODenseInfo{get_sso_word_index(self), kSSOWordCount};
}

//! \}

} // {BitSetInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_BITSET_P_H
