// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/bitarray_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>

namespace bl::BitArrayInternal {

// bl::BitArray - Private - Commons
// ================================

static constexpr size_t kSSOWordCapacity = uint32_t(BLBitArray::kSSOWordCount);
static constexpr size_t kSSOBitCapacity = kSSOWordCapacity * uint32_t(BitArrayOps::kNumBits);

static BL_INLINE_CONSTEXPR size_t bit_index_of(size_t word_index) noexcept { return word_index * BitArrayOps::kNumBits; }
static BL_INLINE_CONSTEXPR size_t word_index_of(size_t bit_index) noexcept { return bit_index / BitArrayOps::kNumBits; }

static BL_INLINE_CONSTEXPR size_t word_count_from_bit_count(size_t bit_count) noexcept {
  return BL_TARGET_ARCH_BITS >= 64 ? uint32_t((uint64_t(bit_count) + BitArrayOps::kBitMask) / BitArrayOps::kNumBits)
                                   : (bit_count / BitArrayOps::kNumBits) + uint32_t((bit_count & BitArrayOps::kBitMask) != 0u);
}

static BL_INLINE_CONSTEXPR size_t bit_count_from_word_count(size_t word_count) noexcept {
  return bl_min<size_t>(word_count * BitArrayOps::kNumBits, 0xFFFFFFFFu);
}

static BL_INLINE_CONSTEXPR BLObjectImplSize impl_size_from_word_capacity(size_t word_capacity) noexcept {
  return BLObjectImplSize(sizeof(BLBitArrayImpl) + word_capacity * sizeof(uint32_t));
}

static BL_INLINE_CONSTEXPR size_t word_capacity_from_impl_size(BLObjectImplSize impl_size) noexcept {
  return (impl_size.value() - sizeof(BLBitArrayImpl)) / sizeof(uint32_t);
}

static BL_INLINE_NODEBUG BLObjectImplSize expand_impl_size(BLObjectImplSize impl_size) noexcept {
  return bl_object_expand_impl_size(impl_size);
}

// bl::BitArray - Private - SSO Representation
// ===========================================

static BL_INLINE BLResult init_sso(BLBitArrayCore* self, size_t size = 0) noexcept {
  self->_d.init_static(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_BIT_ARRAY) | BLObjectInfo::from_abcp(0, 0, 0, uint32_t(size)));
  return BL_SUCCESS;
}

static BL_INLINE void set_sso_size(BLBitArrayCore* self, size_t new_size) noexcept {
  BL_ASSERT(self->_d.sso());
  self->_d.info.set_p_field(uint32_t(new_size));
}

// bl::BitArray - Private - Memory Management
// ==========================================

static BL_INLINE BLResult init_dynamic(BLBitArrayCore* self, BLObjectImplSize impl_size, size_t size = 0u) noexcept {
  BL_ASSERT(size <= UINT32_MAX);

  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_BIT_ARRAY);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLBitArrayImpl>(self, info, impl_size));

  BLBitArrayImpl* impl = get_impl(self);
  impl->capacity = uint32_t(bit_count_from_word_count(word_capacity_from_impl_size(impl_size)));
  impl->size = uint32_t(size);
  return BL_SUCCESS;
}

// bl::BitArray - Private - Modify Op
// ==================================

// A helper function that makes the BitArray mutable, but only if `from` is within its bounds.
static BL_NOINLINE BLResult make_mutable_for_modify_op(BLBitArrayCore* self, size_t from, BitData* out) noexcept {
  if (self->_d.sso()) {
    size_t size = get_sso_size(self);
    if (from >= size)
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    *out = BitData{self->_d.u32_data, get_sso_size(self)};
    return BL_SUCCESS;
  }
  else {
    BLBitArrayImpl* self_impl = get_impl(self);
    size_t size = self_impl->size;

    if (from >= size)
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    if (is_impl_mutable(self_impl)) {
      *out = BitData{self_impl->data(), size};
      return BL_SUCCESS;
    }

    BLBitArrayCore newO;
    if (size <= kSSOBitCapacity) {
      init_sso(&newO, size);

      *out = BitData{self->_d.u32_data, size};
      return replace_instance(self, &newO);
    }

    BL_PROPAGATE(init_dynamic(&newO, impl_size_from_word_capacity(word_count_from_bit_count(size)), size));
    *out = BitData{get_impl(&newO)->data(), size};
    return replace_instance(self, &newO);
  }
}

// Returns the original size of the BitArray when passed to this function (basically it returns the index where to append the bits).
static BL_NOINLINE BLResult make_mutable_for_append_op(BLBitArrayCore* self, size_t append_bit_count, size_t* bit_index, BitData* out) noexcept {
  BL_ASSERT(append_bit_count > 0u);

  BitData d;
  if (self->_d.sso()) {
    d = BitData{self->_d.u32_data, get_sso_size(self)};
    *bit_index = d.size;

    size_t remaining_capacity = size_t(kSSOBitCapacity) - d.size;
    if (append_bit_count <= remaining_capacity) {
      size_t new_size = d.size + append_bit_count;
      set_sso_size(self, new_size);

      *out = BitData{d.data, new_size};
      return BL_SUCCESS;
    }

    if (BL_UNLIKELY(append_bit_count > size_t(UINT32_MAX) - d.size))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
  }
  else {
    BLBitArrayImpl* self_impl = get_impl(self);

    d = BitData{self_impl->data(), self_impl->size};
    *bit_index = d.size;

    size_t remaining_capacity = size_t(self_impl->capacity) - d.size;
    size_t mutable_msk = IntOps::bool_as_mask<size_t>(is_impl_mutable(self_impl));

    if (append_bit_count <= (remaining_capacity & mutable_msk)) {
      size_t new_size = d.size + append_bit_count;
      size_t from_word = word_index_of(d.size + BitArrayOps::kBitMask);
      size_t last_word = word_index_of(new_size - 1u);

      MemOps::fill_inline_t(d.data + from_word, uint32_t(0), last_word - from_word + 1);
      self_impl->size = uint32_t(new_size);

      *out = BitData{d.data, new_size};
      return BL_SUCCESS;
    }
  }

  OverflowFlag of{};
  size_t new_size = IntOps::add_overflow(d.size, append_bit_count, &of);

  if (BL_UNLIKELY(of))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  size_t old_word_count = word_count_from_bit_count(d.size);
  size_t new_word_count = word_count_from_bit_count(new_size);
  BLObjectImplSize impl_size = impl_size_from_word_capacity(new_word_count);

  BLBitArrayCore newO;
  BL_PROPAGATE(init_dynamic(&newO, expand_impl_size(impl_size), new_size));

  BLBitArrayImpl* new_impl = get_impl(&newO);
  MemOps::copy_forward_inline_t(new_impl->data(), d.data, old_word_count);
  MemOps::fill_inline_t(new_impl->data() + old_word_count, uint32_t(0), new_word_count - old_word_count);

  *out = BitData{new_impl->data(), new_size};
  return replace_instance(self, &newO);
}

// bl::BitArray - Private - Combine Op
// ===================================

template<typename BitOp>
static BL_INLINE BLResult combine_word_data(BitData d, size_t bit_index, const uint32_t* word_data, size_t word_count) noexcept {
  size_t bit_end = bit_index + bl_min(bit_index_of(word_count), d.size - bit_index);
  size_t bit_count = bit_end - bit_index;

  size_t word_index = word_index_of(bit_index);
  uint32_t* dst = d.data + word_index;
  uint32_t bit_shift = uint32_t(bit_index & BitArrayOps::kBitMask);

  // Special case - if `word_data` is aligned to a word boundary, we don't have to shift the input BitWords.
  if (bit_shift == 0u) {
    word_count = bl_min(word_count_from_bit_count(bit_count), word_count);
    uint32_t end_bit_count = uint32_t(bit_end & BitArrayOps::kBitMask);

    size_t end = word_count - size_t(end_bit_count != 0);
    BitArrayOps::bit_array_combine_words<BitOp>(dst, word_data, end);

    if (end_bit_count)
      dst[end] = BitOp::op_masked(dst[end], word_data[end], BitArrayOps::non_zero_start_mask(end_bit_count));

    return BL_SUCCESS;
  }

  uint32_t w = word_data[0];
  uint32_t bit_shift_inv = BitArrayOps::kNumBits - bit_shift;

  // Special case - if the number of processed bits is less than number of the remaining bits in the current BitWord.
  if (bit_count <= bit_shift_inv) {
    uint32_t mask = BitArrayOps::non_zero_start_mask(bit_count, bit_shift);
    dst[0] = BitOp::op_masked(dst[0], BitArrayOps::shift_to_end(w, bit_shift), mask);
    return BL_SUCCESS;
  }

  // Process the first BitWord, which is not fully combined (must combine under a write-mask).
  dst[0] = BitOp::op_masked(dst[0], BitArrayOps::shift_to_end(w, bit_shift), BitArrayOps::non_zero_end_mask(bit_shift_inv));
  bit_count -= bit_shift_inv;

  // Process guaranteed BitWord quantities.
  size_t i = 1;
  size_t n = word_index_of(bit_count);

  while (i <= n) {
    uint32_t prev_word_bits = BitArrayOps::shift_to_start(w, bit_shift_inv);
    w = word_data[i];
    dst[i] = BitOp::op(dst[i], prev_word_bits | BitArrayOps::shift_to_end(w, bit_shift));

    i++;
  }

  bit_count &= BitArrayOps::kBitMask;
  if (bit_count == 0)
    return BL_SUCCESS;

  uint32_t last_word_bits = BitArrayOps::shift_to_start(w, bit_shift_inv);
  if (bit_shift_inv < bit_count)
    last_word_bits |= BitArrayOps::shift_to_end(word_data[i], bit_shift);

  dst[i] = BitOp::op_masked(dst[i], last_word_bits, BitArrayOps::non_zero_start_mask(bit_count));
  return BL_SUCCESS;
}

} // {bl::BitArrayInternal}

// bl::BitArray - API - Init & Destroy
// ===================================

BL_API_IMPL BLResult bl_bit_array_init(BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  return init_sso(self);
}

BL_API_IMPL BLResult bl_bit_array_init_move(BLBitArrayCore* self, BLBitArrayCore* other) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(other->_d.is_bit_array());

  BLBitArrayCore tmp = *other;
  init_sso(other);
  *self = tmp;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_array_init_weak(BLBitArrayCore* self, const BLBitArrayCore* other) noexcept {
  using namespace bl::BitArrayInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_bit_array());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_bit_array_destroy(BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  return release_instance(self);
}

// bl::BitArray - API - Reset
// ==========================

BL_API_IMPL BLResult bl_bit_array_reset(BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  release_instance(self);
  return init_sso(self);
}

// bl::BitArray - API - Assign
// ===========================

BL_API_IMPL BLResult bl_bit_array_assign_move(BLBitArrayCore* self, BLBitArrayCore* other) noexcept {
  using namespace bl::BitArrayInternal;

  BL_ASSERT(self->_d.is_bit_array());
  BL_ASSERT(other->_d.is_bit_array());

  BLBitArrayCore tmp = *other;
  init_sso(other);
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_bit_array_assign_weak(BLBitArrayCore* self, const BLBitArrayCore* other) noexcept {
  using namespace bl::BitArrayInternal;

  BL_ASSERT(self->_d.is_bit_array());
  BL_ASSERT(other->_d.is_bit_array());

  retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_bit_array_assign_words(BLBitArrayCore* self, const uint32_t* word_data, uint32_t word_count) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  if (self->_d.sso()) {
    if (word_count <= kSSOWordCapacity) {
      set_sso_size(self, word_count * bl::BitArrayOps::kNumBits);
      bl::MemOps::copy_forward_inline_t(self->_d.u32_data, word_data, word_count);
      bl::MemOps::fill_inline_t(self->_d.u32_data + word_count, uint32_t(0), kSSOWordCapacity - word_count);
      return BL_SUCCESS;
    }
  }
  else {
    BLBitArrayImpl* self_impl = get_impl(self);

    size_t capacity_in_words = word_count_from_bit_count(self_impl->capacity);
    size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

    if ((word_count | immutable_msk) > capacity_in_words) {
      BLBitArrayCore newO;
      init_sso(&newO, size_t(word_count) * bl::BitArrayOps::kNumBits);
      bl::MemOps::copy_forward_inline_t(newO._d.u32_data, word_data, word_count);

      return replace_instance(self, &newO);
    }
  }

  BLBitArrayCore newO;
  BL_PROPAGATE(init_dynamic(&newO, impl_size_from_word_capacity(word_count)));

  BLBitArrayImpl* new_impl = get_impl(&newO);
  bl::MemOps::copy_forward_inline_t(new_impl->data(), word_data, word_count);

  return replace_instance(self, &newO);
}

// bl::BitArray - API - Accessors
// ==============================

BL_API_IMPL bool bl_bit_array_is_empty(const BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  return get_size(self) == 0;
}

BL_API_IMPL uint32_t bl_bit_array_get_size(const BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  return uint32_t(get_size(self));
}

BL_API_IMPL uint32_t bl_bit_array_get_word_count(const BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  return uint32_t(word_count_from_bit_count(get_size(self)));
}

BL_API_IMPL uint32_t bl_bit_array_get_capacity(const BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  return uint32_t(get_capacity(self));
}

BL_API_IMPL const uint32_t* bl_bit_array_get_data(const BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d = unpack(self);
  return d.data;
}

BL_API_IMPL uint32_t bl_bit_array_get_cardinality(const BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d = unpack(self);
  if (!d.size)
    return 0u;

  bl::IntOps::PopCounter<uint32_t> counter;
  counter.add_array(d.data, word_count_from_bit_count(d.size));
  return counter.get();
}

BL_API_IMPL uint32_t bl_bit_array_get_cardinality_in_range(const BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d = unpack(self);
  size_t start = start_bit;
  size_t end = bl_min<size_t>(end_bit, d.size);

  if (start >= end)
    return 0u;

  size_t start_word = word_index_of(start);
  size_t last_word = word_index_of(end - 1u);
  bl::IntOps::PopCounter<uint32_t> counter;

  if (start_word == last_word) {
    // Special case - the range is within a single BitWord.
    uint32_t mask = bl::BitArrayOps::non_zero_start_mask(end - start, start_bit);
    counter.add_item(d.data[start_word] & mask);
  }
  else {
    uint32_t start_mask = bl::BitArrayOps::non_zero_end_mask(bl::BitArrayOps::kNumBits - uint32_t(start & bl::BitArrayOps::kBitMask));
    uint32_t end_mask = bl::BitArrayOps::non_zero_start_mask((uint32_t(end - 1u) & bl::BitArrayOps::kBitMask) + 1u);

    counter.add_item(d.data[start_word] & start_mask);
    counter.add_array(d.data + start_word + 1, last_word - start_word - 1);
    counter.add_item(d.data[last_word] & end_mask);
  }

  return counter.get();
}

BL_API_IMPL bool bl_bit_array_has_bit(const BLBitArrayCore* self, uint32_t bit_index) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d = unpack(self);
  if (bit_index >= d.size)
    return false;

  return bl::BitArrayOps::bit_array_test_bit(d.data, bit_index);
}

BL_API_IMPL bool bl_bit_array_has_bits_in_range(const BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d = unpack(self);
  size_t start = start_bit;
  size_t end = bl_min(d.size, size_t(end_bit));

  if (start >= end)
    return false;

  size_t start_word = word_index_of(start);
  size_t end_word = word_index_of(end);

  if (start_word == end_word) {
    // Special case - the range is within a single BitWord.
    uint32_t mask = bl::BitArrayOps::non_zero_start_mask(end - start, start);
    return (d.data[start_word] & mask) != 0u;
  }

  uint32_t start_mask = bl::BitArrayOps::non_zero_end_mask(bl::BitArrayOps::kNumBits - (start & bl::BitArrayOps::kBitMask));
  if (d.data[start_word] & start_mask)
    return true;

  for (size_t i = start_word + 1; i < end_word; i++)
    if (d.data[i])
      return true;

  uint32_t end_mask = bl::BitArrayOps::non_zero_start_mask(end & bl::BitArrayOps::kBitMask);
  return (d.data[end_word] & end_mask) != 0u;
}

// bl::BitArray - API - Testing
// ============================

BL_API_IMPL bool bl_bit_array_subsumes(const BLBitArrayCore* a, const BLBitArrayCore* b) noexcept {
  using namespace bl::BitArrayInternal;

  BL_ASSERT(a->_d.is_bit_array());
  BL_ASSERT(b->_d.is_bit_array());

  BitData ad = unpack(a);
  BitData bd = unpack(b);

  size_t shared_word_count = word_count_from_bit_count(bl_min(ad.size, bd.size));
  for (size_t i = 0; i < shared_word_count; i++)
    if ((ad.data[i] & bd.data[i]) != bd.data[i])
      return false;

  size_t bWordCount = word_count_from_bit_count(bd.size);
  for (size_t i = shared_word_count; i < bWordCount; i++)
    if (bd.data[i])
      return false;

  return true;
}

BL_API_IMPL bool bl_bit_array_intersects(const BLBitArrayCore* a, const BLBitArrayCore* b) noexcept {
  using namespace bl::BitArrayInternal;

  BL_ASSERT(a->_d.is_bit_array());
  BL_ASSERT(b->_d.is_bit_array());

  BitData ad = unpack(a);
  BitData bd = unpack(b);

  size_t shared_word_count = word_count_from_bit_count(bl_min(ad.size, bd.size));
  for (size_t i = 0; i < shared_word_count; i++)
    if ((ad.data[i] & bd.data[i]) != 0u)
      return true;

  return false;
}

BL_API_IMPL bool bl_bit_array_get_range(const BLBitArrayCore* self, uint32_t* start_out, uint32_t* end_out) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d = unpack(self);
  size_t word_count = word_count_from_bit_count(d.size);

  for (size_t i = 0; i < word_count; i++) {
    uint32_t bits = d.data[i];
    if (bits) {
      size_t start = bit_index_of(i) + bl::BitArrayOps::count_zeros_from_start(bits);
      for (size_t j = word_count; j != 0; j--) {
        bits = d.data[j - 1];
        if (bits) {
          size_t end = bit_index_of(j) - bl::BitArrayOps::count_zeros_from_end(bits);
          *start_out = uint32_t(start);
          *end_out = uint32_t(end);
          return true;
        }
      }
    }
  }

  // There are no bits set in this BitArray.
  *start_out = 0u;
  *end_out = 0u;
  return false;
}

// bl::BitArray - API - Equality & Comparison
// ==========================================

BL_API_IMPL bool bl_bit_array_equals(const BLBitArrayCore* a, const BLBitArrayCore* b) noexcept {
  using namespace bl::BitArrayInternal;

  BL_ASSERT(a->_d.is_bit_array());
  BL_ASSERT(b->_d.is_bit_array());

  BitData ad = unpack(a);
  BitData bd = unpack(b);

  if (ad.size != bd.size)
    return false;

  size_t word_count = word_count_from_bit_count(ad.size);
  for (size_t i = 0; i < word_count; i++)
    if (ad.data[i] != bd.data[i])
      return false;

  return true;
}

BL_API_IMPL int bl_bit_array_compare(const BLBitArrayCore* a, const BLBitArrayCore* b) noexcept {
  using namespace bl::BitArrayInternal;

  BL_ASSERT(a->_d.is_bit_array());
  BL_ASSERT(b->_d.is_bit_array());

  BitData ad = unpack(a);
  BitData bd = unpack(b);

  size_t min_size = bl_min(ad.size, bd.size);
  size_t word_count = word_count_from_bit_count(min_size);

  // We don't need any masking here - bits in a BitWord that are outside of a BitArray range must be zero. If one
  // of the BitArray has a greater size and any bit not used by the other filled, it's would compare as greater.
  for (size_t i = 0; i < word_count; i++)
    if (ad.data[i] != bd.data[i])
      return bl::BitArrayOps::compare(ad.data[i], bd.data[i]);

  return ad.size < bd.size ? -1 : int(ad.size > bd.size);
}

// bl::BitArray - API - Manipulation - Clear
// =========================================

BL_API_IMPL BLResult bl_bit_array_clear(BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  if (self->_d.sso())
    return init_sso(self);

  BLBitArrayImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    self_impl->size = 0;
    return BL_SUCCESS;
  }
  else {
    release_instance(self);
    init_sso(self);
    return BL_SUCCESS;
  }
}

// bl::BitArray - API - Manipulation - Resize
// ==========================================

BL_API_IMPL BLResult bl_bit_array_resize(BLBitArrayCore* self, uint32_t n_bits) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;

  if (self->_d.sso()) {
    d = BitData{get_sso_data(self), get_sso_size(self)};
    if (n_bits <= kSSOBitCapacity) {
      if (n_bits < d.size) {
        // SSO mode requires ALL bits outside of the range to be set to zero.
        size_t i = word_index_of(n_bits);

        if (n_bits & bl::BitArrayOps::kBitMask)
          d.data[i++] &= bl::BitArrayOps::non_zero_start_mask(n_bits & bl::BitArrayOps::kBitMask);

        while (i < kSSOWordCapacity)
          d.data[i++] = 0;
      }

      set_sso_size(self, n_bits);
      return BL_SUCCESS;
    }
  }
  else {
    BLBitArrayImpl* self_impl = get_impl(self);
    size_t immutable_mask = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

    d = BitData{self_impl->data(), self_impl->size};
    if ((n_bits | immutable_mask) <= self_impl->capacity) {
      if (n_bits < d.size) {
        size_t i = word_index_of(n_bits);
        if (n_bits & bl::BitArrayOps::kBitMask)
          d.data[i] &= bl::BitArrayOps::non_zero_start_mask(n_bits & bl::BitArrayOps::kBitMask);
      }
      else {
        size_t from = word_index_of(d.size + bl::BitArrayOps::kBitMask);
        size_t end = word_count_from_bit_count(n_bits);
        bl::MemOps::fill_inline_t(d.data + from, uint32_t(0), end - from);
      }

      self_impl->size = uint32_t(n_bits);
      return BL_SUCCESS;
    }
  }

  BLBitArrayCore newO;
  uint32_t* dst;

  if (n_bits <= kSSOBitCapacity) {
    init_sso(&newO, n_bits);
    dst = newO._d.u32_data;
  }
  else {
    BLObjectImplSize impl_size = impl_size_from_word_capacity(word_count_from_bit_count(n_bits));
    BL_PROPAGATE(init_dynamic(&newO, impl_size, n_bits));
    dst = get_impl(&newO)->data();
  }

  size_t bit_count = bl_min<size_t>(n_bits, d.size);
  size_t word_count = word_count_from_bit_count(bit_count);

  bl::MemOps::copy_forward_inline_t(dst, d.data, word_count);
  uint32_t last_word_bit_count = uint32_t(bit_count & bl::BitArrayOps::kBitMask);

  if (last_word_bit_count)
    dst[word_count - 1] &= bl::BitArrayOps::non_zero_start_mask(last_word_bit_count);

  return replace_instance(self, &newO);
}

// bl::BitArray - API - Manipulation - Reserve
// ===========================================

BL_API_IMPL BLResult bl_bit_array_reserve(BLBitArrayCore* self, uint32_t n_bits) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;
  if (self->_d.sso()) {
    if (n_bits <= kSSOBitCapacity)
      return BL_SUCCESS;

    d = BitData{get_sso_data(self), get_sso_size(self)};
  }
  else {
    BLBitArrayImpl* self_impl = get_impl(self);
    size_t immutable_mask = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

    if ((n_bits | immutable_mask) <= self_impl->capacity)
      return BL_SUCCESS;

    d = BitData{self_impl->data(), self_impl->size};
  }

  BLObjectImplSize impl_size = impl_size_from_word_capacity(word_count_from_bit_count(n_bits));
  BLBitArrayCore newO;
  BL_PROPAGATE(init_dynamic(&newO, impl_size, d.size));

  BLBitArrayImpl* new_impl = get_impl(&newO);
  bl::MemOps::copy_forward_inline_t(new_impl->data(), d.data, word_count_from_bit_count(d.size));
  return replace_instance(self, &newO);
}

// bl::BitArray - API - Manipulation - Shrink
// ==========================================

BL_API_IMPL BLResult bl_bit_array_shrink(BLBitArrayCore* self) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  if (self->_d.sso())
    return BL_SUCCESS;

  BLBitArrayImpl* impl = get_impl(self);
  size_t size = impl->size;
  size_t capacity = impl->capacity;

  if (size <= kSSOBitCapacity) {
    BLBitArrayCore newO;
    init_sso(&newO, size);
    bl::MemOps::copy_forward_inline_t(newO._d.u32_data, impl->data(), word_count_from_bit_count(size));
    return replace_instance(self, &newO);
  }

  BLObjectImplSize current_impl_size = impl_size_from_word_capacity(word_count_from_bit_count(capacity));
  BLObjectImplSize optimal_impl_size = impl_size_from_word_capacity(word_count_from_bit_count(size));

  if (optimal_impl_size + BL_OBJECT_IMPL_ALIGNMENT <= current_impl_size) {
    BLBitArrayCore newO;
    BL_PROPAGATE(init_dynamic(&newO, optimal_impl_size, size));

    BLBitArrayImpl* new_impl = get_impl(&newO);
    bl::MemOps::copy_forward_inline_t(new_impl->data(), impl->data(), word_count_from_bit_count(size));
    return replace_instance(self, &newO);
  }

  return BL_SUCCESS;
}

// bl::BitArray - API - Manipulation - Set / Fill
// ==============================================

BL_API_IMPL BLResult bl_bit_array_set_bit(BLBitArrayCore* self, uint32_t bit_index) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;
  BL_PROPAGATE(make_mutable_for_modify_op(self, bit_index, &d));

  bl::BitArrayOps::bit_array_set_bit(d.data, bit_index);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_array_fill_range(BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  if (BL_UNLIKELY(start_bit >= end_bit))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BitData d;
  BL_PROPAGATE(make_mutable_for_modify_op(self, start_bit, &d));

  size_t end = bl_min(size_t(end_bit), d.size);
  bl::BitArrayOps::bit_array_fill(d.data, start_bit, end - start_bit);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_array_fill_words(BLBitArrayCore* self, uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;
  BL_PROPAGATE(make_mutable_for_modify_op(self, bit_index, &d));

  combine_word_data<bl::BitOperator::Or>(d, bit_index, word_data, word_count);
  return BL_SUCCESS;
}

// bl::BitArray - API - Manipulation - Clear
// =========================================

BL_API_IMPL BLResult bl_bit_array_clear_bit(BLBitArrayCore* self, uint32_t bit_index) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;
  BL_PROPAGATE(make_mutable_for_modify_op(self, bit_index, &d));

  bl::BitArrayOps::bit_array_clear_bit(d.data, bit_index);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_array_clear_range(BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  if (BL_UNLIKELY(start_bit >= end_bit))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BitData d;
  BL_PROPAGATE(make_mutable_for_modify_op(self, start_bit, &d));

  size_t end = bl_min(size_t(end_bit), d.size);
  bl::BitArrayOps::bit_array_clear(d.data, start_bit, end - start_bit);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_array_clear_word(BLBitArrayCore* self, uint32_t bit_index, uint32_t word_value) noexcept {
  return bl_bit_array_clear_words(self, bit_index, &word_value, 1);
}

BL_API_IMPL BLResult bl_bit_array_clear_words(BLBitArrayCore* self, uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;
  BL_PROPAGATE(make_mutable_for_modify_op(self, bit_index, &d));

  combine_word_data<bl::BitOperator::AndNot>(d, bit_index, word_data, word_count);
  return BL_SUCCESS;
}

// bl::BitArray - API - Manipulation - Replace
// ===========================================

BL_API_IMPL BLResult bl_bit_array_replace_op(BLBitArrayCore* self, uint32_t n_bits, uint32_t** data_out) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BLBitArrayCore newO;
  size_t word_count = word_count_from_bit_count(n_bits);
  uint32_t* dst = nullptr;

  // Not a real loop, just to be able to jump to the end without the use of the hated 'goto'.
  do {
    if (self->_d.sso()) {
      if (n_bits <= kSSOBitCapacity) {
        init_sso(self, n_bits);

        *data_out = get_sso_data(self);
        return BL_SUCCESS;
      }
    }
    else {
      BLBitArrayImpl* self_impl = get_impl(self);
      size_t immutable_mask = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

      if ((n_bits | immutable_mask) <= self_impl->capacity) {
        dst = self_impl->data();
        self_impl->size = uint32_t(n_bits);

        // Using the passed instance's Impl, it's mutable and it has enough capacity.
        break;
      }
      else if (n_bits <= kSSOBitCapacity) {
        release_instance(self);
        init_sso(self, n_bits);

        *data_out = get_sso_data(self);
        return BL_SUCCESS;
      }
    }

    BLObjectImplSize impl_size = impl_size_from_word_capacity(word_count_from_bit_count(n_bits));
    BL_PROPAGATE(init_dynamic(&newO, impl_size, n_bits));
    release_instance(self);

    dst = get_impl(&newO)->data();
    *self = newO;
  } while (0);

  // We don't know whether the C++ compiler would decide to unroll this one, that's it's only once in the body.
  bl::MemOps::fill_inline_t(dst, uint32_t(0), word_count);

  *data_out = dst;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_array_replace_bit(BLBitArrayCore* self, uint32_t bit_index, bool bit_value) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  if (bit_value)
    return bl_bit_array_set_bit(self, bit_index);
  else
    return bl_bit_array_clear_bit(self, bit_index);
}

BL_API_IMPL BLResult bl_bit_array_replace_word(BLBitArrayCore* self, uint32_t bit_index, uint32_t word_value) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  return bl_bit_array_replace_words(self, bit_index, &word_value, 1);
}

BL_API_IMPL BLResult bl_bit_array_replace_words(BLBitArrayCore* self, uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;
  BL_PROPAGATE(make_mutable_for_modify_op(self, bit_index, &d));

  combine_word_data<bl::BitOperator::Assign>(d, bit_index, word_data, word_count);
  return BL_SUCCESS;
}

// bl::BitArray - API - Manipulation - Append
// ==========================================

BL_API_IMPL BLResult bl_bit_array_append_bit(BLBitArrayCore* self, bool bit_value) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  BitData d;
  size_t bit_index;
  BL_PROPAGATE(make_mutable_for_append_op(self, 1u, &bit_index, &d));

  bl::BitArrayOps::bit_array_or_bit(d.data, bit_index, bit_value);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_bit_array_append_word(BLBitArrayCore* self, uint32_t word_value) noexcept {
  return bl_bit_array_append_words(self, &word_value, 1);
}

BL_API_IMPL BLResult bl_bit_array_append_words(BLBitArrayCore* self, const uint32_t* word_data, uint32_t word_count) noexcept {
  using namespace bl::BitArrayInternal;
  BL_ASSERT(self->_d.is_bit_array());

  if (!word_count)
    return BL_SUCCESS;

  BitData d;
  size_t bit_index;
  BL_PROPAGATE(make_mutable_for_append_op(self, size_t(word_count) * bl::BitArrayOps::kNumBits, &bit_index, &d));

  combine_word_data<bl::BitOperator::Or>(d, bit_index, word_data, word_count);
  return BL_SUCCESS;
}

// bl::BitArray - Runtime Registration
// ===================================

void bl_bit_array_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);
  bl::BitArrayInternal::init_sso(static_cast<BLBitArrayCore*>(&bl_object_defaults[BL_OBJECT_TYPE_BIT_ARRAY]));
}
