// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_BITARRAY_H
#define BLEND2D_BITARRAY_H

#include <blend2d/core/object.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLBitArray - C API
//! \{

//! BitArray container [C API].
struct BLBitArrayCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLBitArray)
};

//! \cond INTERNAL
//! BitArray container [C API Impl].
struct BLBitArrayImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Size in bit units.
  uint32_t size;
  //! Capacity in bit-word units.
  uint32_t capacity;

#ifdef __cplusplus
  //! Pointer to array data.
  BL_INLINE_NODEBUG uint32_t* data() noexcept { return reinterpret_cast<uint32_t*>(this + 1); }

  //! Pointer to array data (const).
  BL_INLINE_NODEBUG const uint32_t* data() const noexcept { return reinterpret_cast<const uint32_t*>(this + 1); }
#endif
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_bit_array_init(BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_init_move(BLBitArrayCore* self, BLBitArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_init_weak(BLBitArrayCore* self, const BLBitArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_destroy(BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_reset(BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_assign_move(BLBitArrayCore* self, BLBitArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_assign_weak(BLBitArrayCore* self, const BLBitArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_assign_words(BLBitArrayCore* self, const uint32_t* word_data, uint32_t word_count) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_array_is_empty(const BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_bit_array_get_size(const BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_bit_array_get_word_count(const BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_bit_array_get_capacity(const BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API const uint32_t* BL_CDECL bl_bit_array_get_data(const BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_bit_array_get_cardinality(const BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_bit_array_get_cardinality_in_range(const BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_array_has_bit(const BLBitArrayCore* self, uint32_t bit_index) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_array_has_bits_in_range(const BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_array_subsumes(const BLBitArrayCore* a, const BLBitArrayCore* b) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_array_intersects(const BLBitArrayCore* a, const BLBitArrayCore* b) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_array_get_range(const BLBitArrayCore* self, uint32_t* start_out, uint32_t* end_out) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_bit_array_equals(const BLBitArrayCore* a, const BLBitArrayCore* b) BL_NOEXCEPT_C;
BL_API int BL_CDECL bl_bit_array_compare(const BLBitArrayCore* a, const BLBitArrayCore* b) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_clear(BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_resize(BLBitArrayCore* self, uint32_t n_bits) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_reserve(BLBitArrayCore* self, uint32_t n_bits) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_shrink(BLBitArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_set_bit(BLBitArrayCore* self, uint32_t bit_index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_fill_range(BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_fill_words(BLBitArrayCore* self, uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_clear_bit(BLBitArrayCore* self, uint32_t bit_index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_clear_range(BLBitArrayCore* self, uint32_t start_bit, uint32_t end_bit) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_clear_word(BLBitArrayCore* self, uint32_t bit_index, uint32_t word_value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_clear_words(BLBitArrayCore* self, uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_replace_op(BLBitArrayCore* self, uint32_t n_bits, uint32_t** data_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_replace_bit(BLBitArrayCore* self, uint32_t bit_index, bool bit_value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_replace_word(BLBitArrayCore* self, uint32_t bit_index, uint32_t word_value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_replace_words(BLBitArrayCore* self, uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_append_bit(BLBitArrayCore* self, bool bit_value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_append_word(BLBitArrayCore* self, uint32_t word_value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_bit_array_append_words(BLBitArrayCore* self, const uint32_t* word_data, uint32_t word_count) BL_NOEXCEPT_C;

// TODO: Future API (BitArray).
/*
BL_API BLResult BL_CDECL bl_bit_array_combine(BLBitArrayCore* dst, const BLBitArrayCore* a, const BLBitArrayCore* b, BLBooleanOp boolean_op) BL_NOEXCEPT_C;
*/

BL_END_C_DECLS

//! \}

//! \}

//! \addtogroup bl_containers
//! \{

//! \name BLBitArray - C++ API
//! \{
#ifdef __cplusplus

//! BitArray container [C++ API].
class BLBitArray final : public BLBitArrayCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  enum : uint32_t {
    //! Number of words that can be used by SSO representation.
    kSSOWordCount = 3,

    //! Signature of SSO representation of an empty BitArray.
    kSSOEmptySignature = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_BIT_ARRAY)
  };

  [[nodiscard]]
  BL_INLINE_NODEBUG BLBitArrayImpl* _impl() const noexcept { return static_cast<BLBitArrayImpl*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLBitArray() noexcept {
    _d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  BL_INLINE_NODEBUG BLBitArray(BLBitArray&& other) noexcept {
    _d = other._d;
    other._d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  BL_INLINE_NODEBUG BLBitArray(const BLBitArray& other) noexcept {
    bl_bit_array_init_weak(this, &other);
  }

  //! Destroys the BitArray.
  BL_INLINE_NODEBUG ~BLBitArray() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_bit_array_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Tests whether the BitArray has a content.
  //!
  //! \note This is essentially the opposite of `is_empty()`.
  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return !is_empty(); }

  //! Move assignment.
  //!
  //! \note The `other` BitArray is reset by move assignment, so its state after the move operation is the same as
  //! a default constructed BitArray.
  BL_INLINE_NODEBUG BLBitArray& operator=(BLBitArray&& other) noexcept { bl_bit_array_assign_move(this, &other); return *this; }
  //! Copy assignment, performs weak copy of the data held by the `other` BitArray.
  BL_INLINE_NODEBUG BLBitArray& operator=(const BLBitArray& other) noexcept { bl_bit_array_assign_weak(this, &other); return *this; }

  BL_INLINE_NODEBUG bool operator==(const BLBitArray& other) const noexcept { return equals(other); }
  BL_INLINE_NODEBUG bool operator!=(const BLBitArray& other) const noexcept { return !equals(other); }
  BL_INLINE_NODEBUG bool operator<(const BLBitArray& other) const noexcept { return compare(other) < 0; }
  BL_INLINE_NODEBUG bool operator<=(const BLBitArray& other) const noexcept { return compare(other) <= 0; }
  BL_INLINE_NODEBUG bool operator>(const BLBitArray& other) const noexcept { return compare(other) > 0; }
  BL_INLINE_NODEBUG bool operator>=(const BLBitArray& other) const noexcept { return compare(other) >= 0; }

  //! \}

  //! \name Common Functionality
  //! \{

  //! Clears the content of the BitArray and releases its data.
  //!
  //! After reset the BitArray content matches a default constructed instance.
  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_bit_array_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLBitArray after reset.
    BL_ASSUME(_d.info.bits == kSSOEmptySignature);

    return result;
  }

  //! Swaps the content of this string with the `other` string.
  BL_INLINE_NODEBUG void swap(BLBitArrayCore& other) noexcept { _d.swap(other._d); }

  //! \name Accessors
  //! \{

  //! Tests whether the BitArray is empty (has no content).
  //!
  //! Returns `true` if the BitArray's size is zero.
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return bl_bit_array_is_empty(this); }

  //! Returns the size of the BitArray in bits.
  BL_INLINE_NODEBUG uint32_t size() const noexcept { return _d.sso() ? uint32_t(_d.p_field()) : _impl()->size; }

  //! Returns number of BitWords this BitArray uses.
  BL_INLINE_NODEBUG uint32_t word_count() const noexcept {
    return sizeof(void*) >= 8 ? (uint32_t((uint64_t(size()) + 31u) / 32u))
                              : (size() / 32u + uint32_t((size() & 31u) != 0u));
  }

  //! Returns the capacity of the BitArray in bits.
  BL_INLINE_NODEBUG uint32_t capacity() const noexcept { return _d.sso() ? uint32_t(kSSOWordCount * 32u) : _impl()->capacity; }

  //! Returns the number of bits set in the BitArray.
  BL_INLINE_NODEBUG uint32_t cardinality() const noexcept { return bl_bit_array_get_cardinality(this); }

  //! Returns the number of bits set in the given `[start_bit, end_bit)` range.
  BL_INLINE_NODEBUG uint32_t cardinality_in_range(uint32_t start_bit, uint32_t end_bit) const noexcept { return bl_bit_array_get_cardinality_in_range(this, start_bit, end_bit); }

  //! Returns bit data.
  BL_INLINE_NODEBUG const uint32_t* data() const noexcept { return _d.sso() ? _d.u32_data : _impl()->data(); }

  //! \}

  //! \name Test Operations
  //! \{

  //! Returns a bit-value at the given `bit_index`.
  BL_INLINE_NODEBUG bool has_bit(uint32_t bit_index) const noexcept { return bl_bit_array_has_bit(this, bit_index); }
  //! Returns whether the bit-set has at least on bit in the given `[start_bit, endbit)` range.
  BL_INLINE_NODEBUG bool has_bits_in_range(uint32_t start_bit, uint32_t end_bit) const noexcept { return bl_bit_array_has_bits_in_range(this, start_bit, end_bit); }

  //! Returns whether this BitArray subsumes `other`.
  BL_INLINE_NODEBUG bool subsumes(const BLBitArrayCore& other) const noexcept { return bl_bit_array_subsumes(this, &other); }
  //! Returns whether this BitArray intersects with `other`.
  BL_INLINE_NODEBUG bool intersects(const BLBitArrayCore& other) const noexcept { return bl_bit_array_intersects(this, &other); }

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Returns whether this BitArray and `other` are bitwise equal.
  BL_INLINE_NODEBUG bool equals(const BLBitArrayCore& other) const noexcept { return bl_bit_array_equals(this, &other); }
  //! Compares this BitArray with `other` and returns either `-1`, `0`, or `1`.
  BL_INLINE_NODEBUG int compare(const BLBitArrayCore& other) const noexcept { return bl_bit_array_compare(this, &other); }

  //! \}

  //! \name Content Manipulation
  //! \{

  //! Move assignment, the same as `operator=`, but returns a `BLResult` instead of `this`.
  BL_INLINE_NODEBUG BLResult assign(BLBitArrayCore&& other) noexcept { return bl_bit_array_assign_move(this, &other); }

  //! Copy assignment, the same as `operator=`, but returns a `BLResult` instead of `this`.
  BL_INLINE_NODEBUG BLResult assign(const BLBitArrayCore& other) noexcept { return bl_bit_array_assign_weak(this, &other); }

  //! Replaces the content of the BitArray by bits specified by `word_data` of size `word_count` [the size is in uint32_t units].
  BL_INLINE_NODEBUG BLResult assign_words(const uint32_t* word_data, uint32_t word_count) noexcept { return bl_bit_array_assign_words(this, word_data, word_count); }

  //! Clears the content of the BitArray without releasing its dynamically allocated data, if possible.
  BL_INLINE_NODEBUG BLResult clear() noexcept { return bl_bit_array_clear(this); }

  //! Resizes the BitArray so its size matches `n_bits`.
  BL_INLINE_NODEBUG BLResult resize(uint32_t n_bits) noexcept { return bl_bit_array_resize(this, n_bits); }

  //! Reserves `n_bits` in the BitArray (capacity would match `n_bits`) without changing its size.
  BL_INLINE_NODEBUG BLResult reserve(uint32_t n_bits) noexcept { return bl_bit_array_resize(this, n_bits); }

  //! Shrinks the capacity of the BitArray to match the actual content with the intention to save memory.
  BL_INLINE_NODEBUG BLResult shrink() noexcept { return bl_bit_array_shrink(this); }

  //! Sets a bit to true at the given `bit_index`.
  BL_INLINE_NODEBUG BLResult set_bit(uint32_t bit_index) noexcept { return bl_bit_array_set_bit(this, bit_index); }

  //! Fills bits in `[start_bit, end_bit)` range to true.
  BL_INLINE_NODEBUG BLResult fill_range(uint32_t start_bit, uint32_t end_bit) noexcept { return bl_bit_array_fill_range(this, start_bit, end_bit); }

  //! Fills bits starting from `bit_index` specified by `word_data` and `word_count` to true (zeros in word_data are ignored).
  //!
  //! \note This operation uses an `OR` operator - bits in `word_data` are combined with OR operator with existing bits in BitArray.
  BL_INLINE_NODEBUG BLResult fill_words(uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) noexcept { return bl_bit_array_fill_words(this, bit_index, word_data, word_count); }

  //! Sets a bit to false at the given `bit_index`.
  BL_INLINE_NODEBUG BLResult clear_bit(uint32_t bit_index) noexcept { return bl_bit_array_clear_bit(this, bit_index); }

  //! Sets bits in `[start_bit, end_bit)` range to false.
  BL_INLINE_NODEBUG BLResult clear_range(uint32_t start_bit, uint32_t end_bit) noexcept { return bl_bit_array_clear_range(this, start_bit, end_bit); }

  //! Sets bits starting from `bit_index` specified by `word_value` to false (zeros in word_value are ignored).
  //!
  //! \note This operation uses an `AND_NOT` operator - bits in `word_data` are negated and then combined with AND operator with existing bits in BitArray.
  BL_INLINE_NODEBUG BLResult clear_word(uint32_t bit_index, uint32_t word_value) noexcept { return bl_bit_array_clear_word(this, bit_index, word_value); }

  //! Sets bits starting from `bit_index` specified by `word_data` and `word_count` to false (zeros in word_data are ignored).
  //!
  //! \note This operation uses an `AND_NOT` operator - bits in `word_data` are negated and then combined with AND operator with existing bits in BitArray.
  BL_INLINE_NODEBUG BLResult clear_words(uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) noexcept { return bl_bit_array_clear_words(this, bit_index, word_data, word_count); }

  //! Makes the BitArray mutable with the intention to replace all bits of it.
  //!
  //! \note All bits in the BitArray will be set to zero.
  BL_INLINE_NODEBUG BLResult replace_op(uint32_t n_bits, uint32_t** data_out) noexcept { return bl_bit_array_replace_op(this, n_bits, data_out); }

  //! Replaces a bit in the BitArray at the given `bit_index` to match `bit_value`.
  BL_INLINE_NODEBUG BLResult replace_bit(uint32_t bit_index, bool bit_value) noexcept { return bl_bit_array_replace_bit(this, bit_index, bit_value); }

  //! Replaces bits starting from `bit_index` to match the bits specified by `word_value`.
  //!
  //! \note Replaced bits from BitArray are not combined by using any operator, `word_value` is copied as is, thus
  //! replaces fully the existing bits.
  BL_INLINE_NODEBUG BLResult replace_word(uint32_t bit_index, uint32_t word_value) noexcept { return bl_bit_array_replace_word(this, bit_index, word_value); }

  //! Replaces bits starting from `bit_index` to match the bits specified by `word_data` and `word_count`.
  //!
  //! \note Replaced bits from BitArray are not combined by using any operator, `word_data` is copied as is, thus
  //! replaces fully the existing bits.
  BL_INLINE_NODEBUG BLResult replace_words(uint32_t bit_index, const uint32_t* word_data, uint32_t word_count) noexcept { return bl_bit_array_replace_words(this, bit_index, word_data, word_count); }

  //! Appends a bit `bit_value` to the BitArray.
  BL_INLINE_NODEBUG BLResult append_bit(bool bit_value) noexcept { return bl_bit_array_append_bit(this, bit_value); }

  //! Appends a single word `word_value` to the BitArray.
  BL_INLINE_NODEBUG BLResult append_word(uint32_t word_value) noexcept { return bl_bit_array_append_word(this, word_value); }

  //! Appends whole words to the BitArray.
  BL_INLINE_NODEBUG BLResult append_words(const uint32_t* word_data, uint32_t word_count) noexcept { return bl_bit_array_append_words(this, word_data, word_count); }

  /*
  // TODO: Future API (BitArray).

  BL_INLINE_NODEBUG BLResult and_(const BLBitArrayCore& other) noexcept { return bl_bit_array_combine(this, this, &other, BL_BOOLEAN_OP_AND); }
  BL_INLINE_NODEBUG BLResult or_(const BLBitArrayCore& other) noexcept { return bl_bit_array_combine(this, this, &other, BL_BOOLEAN_OP_OR); }
  BL_INLINE_NODEBUG BLResult xor_(const BLBitArrayCore& other) noexcept { return bl_bit_array_combine(this, this, &other, BL_BOOLEAN_OP_XOR); }
  BL_INLINE_NODEBUG BLResult and_not(const BLBitArrayCore& other) noexcept { return bl_bit_array_combine(this, this, &other, BL_BOOLEAN_OP_AND_NOT); }
  BL_INLINE_NODEBUG BLResult not_and(const BLBitArrayCore& other) noexcept { return bl_bit_array_combine(this, this, &other, BL_BOOLEAN_OP_NOT_AND); }
  BL_INLINE_NODEBUG BLResult combine(const BLBitArrayCore& other, BLBooleanOp boolean_op) noexcept { return bl_bit_array_combine(this, this, &other, boolean_op); }

  static BL_INLINE_NODEBUG BLResult and_(BLBitArrayCore& dst, const BLBitArrayCore& a, const BLBitArrayCore& b) noexcept { return bl_bit_array_combine(&dst, &a, &b, BL_BOOLEAN_OP_AND); }
  static BL_INLINE_NODEBUG BLResult or_(BLBitArrayCore& dst, const BLBitArrayCore& a, const BLBitArrayCore& b) noexcept { return bl_bit_array_combine(&dst, &a, &b, BL_BOOLEAN_OP_OR); }
  static BL_INLINE_NODEBUG BLResult xor_(BLBitArrayCore& dst, const BLBitArrayCore& a, const BLBitArrayCore& b) noexcept { return bl_bit_array_combine(&dst, &a, &b, BL_BOOLEAN_OP_XOR); }
  static BL_INLINE_NODEBUG BLResult and_not(BLBitArrayCore& dst, const BLBitArrayCore& a, const BLBitArrayCore& b) noexcept { return bl_bit_array_combine(&dst, &a, &b, BL_BOOLEAN_OP_AND_NOT); }
  static BL_INLINE_NODEBUG BLResult not_and(BLBitArrayCore& dst, const BLBitArrayCore& a, const BLBitArrayCore& b) noexcept { return bl_bit_array_combine(&dst, &a, &b, BL_BOOLEAN_OP_NOT_AND); }
  static BL_INLINE_NODEBUG BLResult combine(BLBitArrayCore& dst, const BLBitArrayCore& a, const BLBitArrayCore& b, BLBooleanOp boolean_op) noexcept { return bl_bit_array_combine(&dst, &a, &b, boolean_op); }
  */

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_BITARRAY_H
