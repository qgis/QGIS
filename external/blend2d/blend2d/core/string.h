// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_STRING_H_INCLUDED
#define BLEND2D_STRING_H_INCLUDED

#include <blend2d/core/object.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLString - C API
//! \{

//! Byte string [C API].
struct BLStringCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLString)
};

//! \cond INTERNAL
//! Byte string [Impl].
struct BLStringImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! String size [in bytes].
  size_t size;
  //! String data capacity [in bytes].
  size_t capacity;

#ifdef __cplusplus
  //! String data [null terminated] follows BLStringImpl data.
  [[nodiscard]]
  BL_INLINE_NODEBUG char* data() noexcept { return reinterpret_cast<char*>(this) + sizeof(BLStringImpl); }
#endif
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_string_init(BLStringCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_init_move(BLStringCore* self, BLStringCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_init_weak(BLStringCore* self, const BLStringCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_init_with_data(BLStringCore* self, const char* str, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_destroy(BLStringCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_reset(BLStringCore* self) BL_NOEXCEPT_C;
BL_API const char* BL_CDECL bl_string_get_data(const BLStringCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API size_t BL_CDECL bl_string_get_size(const BLStringCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API size_t BL_CDECL bl_string_get_capacity(const BLStringCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_string_clear(BLStringCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_shrink(BLStringCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_reserve(BLStringCore* self, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_resize(BLStringCore* self, size_t n, char fill) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_make_mutable(BLStringCore* self, char** data_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_modify_op(BLStringCore* self, BLModifyOp op, size_t n, char** data_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_insert_op(BLStringCore* self, size_t index, size_t n, char** data_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_assign_move(BLStringCore* self, BLStringCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_assign_weak(BLStringCore* self, const BLStringCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_assign_deep(BLStringCore* self, const BLStringCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_assign_data(BLStringCore* self, const char* str, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_apply_op_char(BLStringCore* self, BLModifyOp op, char c, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_apply_op_data(BLStringCore* self, BLModifyOp op, const char* str, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_apply_op_string(BLStringCore* self, BLModifyOp op, const BLStringCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_apply_op_format(BLStringCore* self, BLModifyOp op, const char* fmt, ...) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_apply_op_format_v(BLStringCore* self, BLModifyOp op, const char* fmt, va_list ap) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_insert_char(BLStringCore* self, size_t index, char c, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_insert_data(BLStringCore* self, size_t index, const char* str, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_insert_string(BLStringCore* self, size_t index, const BLStringCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_remove_index(BLStringCore* self, size_t index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_string_remove_range(BLStringCore* self, size_t r_start, size_t r_end) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_string_equals(const BLStringCore* a, const BLStringCore* b) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_string_equals_data(const BLStringCore* self, const char* str, size_t n) BL_NOEXCEPT_C BL_PURE;
BL_API int BL_CDECL bl_string_compare(const BLStringCore* a, const BLStringCore* b) BL_NOEXCEPT_C BL_PURE;
BL_API int BL_CDECL bl_string_compare_data(const BLStringCore* self, const char* str, size_t n) BL_NOEXCEPT_C BL_PURE;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_containers
//! \{

//! \name BLString - C++ API
//! \{
#ifdef __cplusplus

//! Byte string [C++ API].
//!
//! Blend2D always uses UTF-8 encoding in public APIs so all strings are assumed UTF-8 by default. However, `BLString`
//! doesn't guarantee any assumptions about the encoding of the data it holds. It can hold arbitrary byte sequence and
//! act as a raw byte-string when this functionality is desired.
class BLString final : public BLStringCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  //! Capacity of an SSO string - depends actually on architecture endianness.
  static inline constexpr uint32_t kSSOCapacity =
    BL_BYTE_ORDER == 1234
      ? BLObjectDetail::kStaticDataSize + 2u
      : BLObjectDetail::kStaticDataSize - 1u;

  //! Signature of SSO representation of an empty string (with size XORed with `kSSOCapacity`).
  //!
  //! This mask can be used to get quickly SSO string size.
  static inline constexpr uint32_t kSSOEmptySignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_STRING) | BLObjectInfo::pack_abcp(kSSOCapacity);

  [[nodiscard]]
  BL_INLINE_NODEBUG BLStringImpl* _impl() const noexcept { return static_cast<BLStringImpl*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Creates an empty string.
  BL_INLINE_NODEBUG BLString() noexcept {
    _d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  //! Move constructor.
  //!
  //! \note The `other` string is always reset by a move construction, so it becomes an empty string.
  BL_INLINE_NODEBUG BLString(BLString&& other) noexcept {
    _d = other._d;
    other._d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  //! Copy constructor, performs weak copy of the data held by the `other` string.
  BL_INLINE_NODEBUG BLString(const BLString& other) noexcept { bl_string_init_weak(this, &other); }

  //! Constructor that creates a string from the given string `view`.
  //!
  //! \note See other constructors for more details.
  BL_INLINE_NODEBUG explicit BLString(BLStringView view) noexcept { bl_string_init_with_data(this, view.data, view.size); }

  //! Constructor that creates a string from the given dat[[nodiscard]]a specified by `str` and `size`. If `size` is `SIZE_MAX`
  //! the string is assumed to be null terminated.
  //!
  //! This is a convenience function that doesn't provide error handling. If size exceeds small string capacity
  //! and dynamic allocation failed then a default empty string would be constructed.
  BL_INLINE_NODEBUG explicit BLString(const char* str, size_t size = SIZE_MAX) noexcept { bl_string_init_with_data(this, str, size); }

  //! Destroys the string.
  BL_INLINE_NODEBUG ~BLString() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_string_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Tests whether the string has a content.
  //!
  //! \note This is essentially the opposite of `is_empty()`.
  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return !is_empty(); }

  //! Move assignment.
  //!
  //! \note The `other` string is reset by move construction, so it becomes an empty string.
  BL_INLINE_NODEBUG BLString& operator=(BLString&& other) noexcept { bl_string_assign_move(this, &other); return *this; }
  //! Copy assignment, performs weak copy of the data held by the `other` string.
  BL_INLINE_NODEBUG BLString& operator=(const BLString& other) noexcept { bl_string_assign_weak(this, &other); return *this; }

  [[nodiscard]] BL_INLINE_NODEBUG bool operator==(const BLString& other) const noexcept { return  equals(other); }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator!=(const BLString& other) const noexcept { return !equals(other); }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator< (const BLString& other) const noexcept { return compare(other) <  0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator<=(const BLString& other) const noexcept { return compare(other) <= 0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator> (const BLString& other) const noexcept { return compare(other) >  0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator>=(const BLString& other) const noexcept { return compare(other) >= 0; }

  [[nodiscard]] BL_INLINE_NODEBUG bool operator==(const BLStringView& view) const noexcept { return  equals(view); }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator!=(const BLStringView& view) const noexcept { return !equals(view); }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator< (const BLStringView& view) const noexcept { return compare(view) <  0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator<=(const BLStringView& view) const noexcept { return compare(view) <= 0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator> (const BLStringView& view) const noexcept { return compare(view) >  0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator>=(const BLStringView& view) const noexcept { return compare(view) >= 0; }

  [[nodiscard]] BL_INLINE_NODEBUG bool operator==(const char* str) const noexcept { return  equals(str); }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator!=(const char* str) const noexcept { return !equals(str); }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator< (const char* str) const noexcept { return compare(str) <  0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator<=(const char* str) const noexcept { return compare(str) <= 0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator> (const char* str) const noexcept { return compare(str) >  0; }
  [[nodiscard]] BL_INLINE_NODEBUG bool operator>=(const char* str) const noexcept { return compare(str) >= 0; }

  //! Returns a character at the given `index`.
  //!
  //! \note This is the same as calling `at(index)`.
  [[nodiscard]]
  BL_INLINE_NODEBUG const char& operator[](size_t index) const noexcept { return at(index); }

  //! \}

  //! \name Common Functionality
  //! \{

  //! Clears the content of the string and releases its data.
  //!
  //! After reset the string content matches a default constructed string.
  BL_INLINE_NODEBUG BLResult reset() noexcept { return bl_string_reset(this); }

  //! Swaps the content of this string with the `other` string.
  BL_INLINE_NODEBUG void swap(BLString& other) noexcept { _d.swap(other._d); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the string is empty (has no content).
  //!
  //! Returns `true` if the string's length is zero.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size() == 0; }

  //! Returns a character at the given `index`.
  //!
  //! \note Index must be valid and cannot be out of bounds - there is an assertion.
  [[nodiscard]]
  BL_INLINE const char& at(size_t index) const noexcept {
    BL_ASSERT(index <= size());
    return data()[index];
  }

  //! Returns the size of the string [in bytes].
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t size() const noexcept {
    return _d.sso() ? size_t((_d.info.bits ^ kSSOEmptySignature) >> BL_OBJECT_INFO_A_SHIFT) : _impl()->size;
  }

  //! Returns the capacity of the string [in bytes].
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return _d.sso() ? size_t(kSSOCapacity) : _impl()->capacity; }

  //! Returns a pointer to the data of the string.
  [[nodiscard]]
  BL_INLINE_NODEBUG const char* data() const noexcept { return _d.sso() ? _d.char_data : _impl()->data(); }

  //! Returns a pointer to the beginning of string data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const char* begin() const noexcept { return data(); }

  //! Returns a pointer to the end of string data (iterator compatibility).
  //!
  //! The returned pointer points to the string null terminator.
  [[nodiscard]]
  BL_INLINE_NODEBUG const char* end() const noexcept { return data() + size(); }

  //! Returns the content of the string as `BLStringView`.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLStringView view() const noexcept { return BLStringView { data(), size() }; }

  //! \}

  //! \name Data Manipulation
  //! \{

  //! Clears the content of the string without releasing its dynamically allocated data, if possible.
  BL_INLINE_NODEBUG BLResult clear() noexcept { return bl_string_clear(this); }
  //! Shrinks the capacity of the string to match the actual content.
  BL_INLINE_NODEBUG BLResult shrink() noexcept { return bl_string_shrink(this); }
  //! Reserves at least `n` bytes in the string for further manipulation (most probably appending).
  BL_INLINE_NODEBUG BLResult reserve(size_t n) noexcept { return bl_string_reserve(this, n); }
  //! Resizes the string to `n` and fills the additional data by `fill` pattern.
  BL_INLINE_NODEBUG BLResult resize(size_t n, char fill = '\0') noexcept { return bl_string_resize(this, n, fill); }

  //! Makes the string mutable.
  //!
  //! This operation checks whether the string is mutable and if not it makes a deep copy of its content so it can be
  //! modified. Please note that you can only modify the content that is defined by its length property. Even if the
  //! string had higher capacity before `make_mutable()` it's not guaranteed that the possible new data would match that
  //! capacity.
  //!
  //! If you want to make the string mutable for the purpose of appending or making other modifications please consider
  //! using `modify_op()` and `insert_op()` instead.
  BL_INLINE_NODEBUG BLResult make_mutable(char** data_out) noexcept { return bl_string_make_mutable(this, data_out); }
  BL_INLINE_NODEBUG BLResult modify_op(BLModifyOp op, size_t n, char** data_out) noexcept { return bl_string_modify_op(this, op, n, data_out); }
  BL_INLINE_NODEBUG BLResult insert_op(size_t index, size_t n, char** data_out) noexcept { return bl_string_insert_op(this, index, n, data_out); }

  //! Replaces the content of the string by `c` character or multiple characters if `n` is greater than one.
  BL_INLINE_NODEBUG BLResult assign(char c, size_t n = 1) noexcept { return bl_string_apply_op_char(this, BL_MODIFY_OP_ASSIGN_FIT, c, n); }

  //! Move assignment, the same as `operator=()`, but returns a `BLResult` instead of `this`.
  BL_INLINE_NODEBUG BLResult assign(BLString&& other) noexcept { return bl_string_assign_move(this, &other); }

  //! Copy assignment, the same as `operator=()`, but returns a `BLResult` instead of `this`.
  BL_INLINE_NODEBUG BLResult assign(const BLString& other) noexcept { return bl_string_assign_weak(this, &other); }

  //! Replaces the string by the content described by the given string `view`.
  BL_INLINE_NODEBUG BLResult assign(BLStringView view) noexcept { return bl_string_assign_data(this, view.data, view.size); }

  //! Replaces the string by `str` data of the given length `n`.
  //!
  //! \note The implementation assumes null terminated string if `n` equals to `SIZE_MAX`.
  BL_INLINE_NODEBUG BLResult assign(const char* str, size_t n = SIZE_MAX) noexcept { return bl_string_assign_data(this, str, n); }

  //! Copy assignment, but creates a deep copy of the `other` string instead of weak copy.
  BL_INLINE_NODEBUG BLResult assign_deep(const BLString& other) noexcept { return bl_string_assign_deep(this, &other); }

  //! Replaces the content of the string by a result of calling `snprintf(fmt, args...)`.
  template<typename... Args>
  BL_INLINE_NODEBUG BLResult assign_format(const char* fmt, Args&&... args) noexcept { return bl_string_apply_op_format(this, BL_MODIFY_OP_ASSIGN_FIT, fmt, BLInternal::forward<Args>(args)...); }

  //! Replaces the content of the string by a result of calling `vsnprintf(fmt, ap)`.
  BL_INLINE_NODEBUG BLResult assignFormatV(const char* fmt, va_list ap) noexcept { return bl_string_apply_op_format_v(this, BL_MODIFY_OP_ASSIGN_FIT, fmt, ap); }

  //! Truncates the string length to `n`.
  //!
  //! It does nothing if the the string length is less than `n`.
  BL_INLINE_NODEBUG BLResult truncate(size_t n) noexcept { return n < size() ? bl_string_resize(this, n, '\0') : BLResult(BL_SUCCESS); }

  BL_INLINE_NODEBUG BLResult append(char c, size_t n = 1) noexcept { return bl_string_apply_op_char(this, BL_MODIFY_OP_APPEND_GROW, c, n); }
  BL_INLINE_NODEBUG BLResult append(const BLString& other) noexcept { return bl_string_apply_op_string(this, BL_MODIFY_OP_APPEND_GROW, &other); }
  BL_INLINE_NODEBUG BLResult append(BLStringView view) noexcept { return bl_string_apply_op_data(this, BL_MODIFY_OP_APPEND_GROW, view.data, view.size); }
  BL_INLINE_NODEBUG BLResult append(const char* str, size_t n = SIZE_MAX) noexcept { return bl_string_apply_op_data(this, BL_MODIFY_OP_APPEND_GROW, str, n); }

  template<typename... Args>
  BL_INLINE_NODEBUG BLResult append_format(const char* fmt, Args&&... args) noexcept { return bl_string_apply_op_format(this, BL_MODIFY_OP_APPEND_GROW, fmt, BLInternal::forward<Args>(args)...); }
  BL_INLINE_NODEBUG BLResult appendFormatV(const char* fmt, va_list ap) noexcept { return bl_string_apply_op_format_v(this, BL_MODIFY_OP_APPEND_GROW, fmt, ap); }

  BL_INLINE_NODEBUG BLResult prepend(char c, size_t n = 1) noexcept { return bl_string_insert_char(this, 0, c, n); }
  BL_INLINE_NODEBUG BLResult prepend(const BLString& other) noexcept { return bl_string_insert_string(this, 0, &other); }
  BL_INLINE_NODEBUG BLResult prepend(BLStringView view) noexcept { return bl_string_insert_data(this, 0, view.data, view.size); }
  BL_INLINE_NODEBUG BLResult prepend(const char* str, size_t n = SIZE_MAX) noexcept { return bl_string_insert_data(this, 0, str, n); }

  BL_INLINE_NODEBUG BLResult insert(size_t index, char c, size_t n = 1) noexcept { return bl_string_insert_char(this, index, c, n); }
  BL_INLINE_NODEBUG BLResult insert(size_t index, const BLString& other) noexcept { return bl_string_insert_string(this, index, &other); }
  BL_INLINE_NODEBUG BLResult insert(size_t index, BLStringView view) noexcept { return bl_string_insert_data(this, index, view.data, view.size); }
  BL_INLINE_NODEBUG BLResult insert(size_t index, const char* str, size_t n = SIZE_MAX) noexcept { return bl_string_insert_data(this, index, str, n); }

  BL_INLINE_NODEBUG BLResult remove(size_t index) noexcept { return bl_string_remove_index(this, index); }
  BL_INLINE_NODEBUG BLResult remove(const BLRange& range) noexcept { return bl_string_remove_range(this, range.start, range.end); }

  //! \name Equality & Comparison
  //! \{

  //! Returns whether this string and `other` are equal (i.e. their contents match).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLString& other) const noexcept { return bl_string_equals(this, &other); }

  //! Returns whether this string and other string `view` are equal.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(BLStringView view) const noexcept { return bl_string_equals_data(this, view.data, view.size); }

  //! Returns whether this string and the given string data `str` of length `n` are equal.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const char* str, size_t n = SIZE_MAX) const noexcept { return bl_string_equals_data(this, str, n); }

  //! Compares this string with `other` and returns either `-1`, `0`, or `1`.
  [[nodiscard]]
  BL_INLINE_NODEBUG int compare(const BLString& other) const noexcept { return bl_string_compare(this, &other); }

  //! Compares this string with other string `view` and returns either `-1`, `0`, or `1`.
  [[nodiscard]]
  BL_INLINE_NODEBUG int compare(BLStringView view) const noexcept { return bl_string_compare_data(this, view.data, view.size); }

  //! Compares this string with other string data and returns either `-1`, `0`, or `1`.
  [[nodiscard]]
  BL_INLINE_NODEBUG int compare(const char* str, size_t n = SIZE_MAX) const noexcept { return bl_string_compare_data(this, str, n); }

  //! \}

  //! \name Search
  //! \{

  //! Returns the first index at which a given character `c` can be found in the string, or `SIZE_MAX` if not present.
  [[nodiscard]]
  BL_INLINE size_t index_of(char c) const noexcept { return index_of(c, 0); }

  //! Returns the index at which a given character `c` can be found in the string starting from `from_index`, or `SIZE_MAX`
  //! if not present.
  [[nodiscard]]
  BL_INLINE size_t index_of(char c, size_t from_index) const noexcept {
    size_t iEnd = size();
    const char* p = data();

    for (size_t i = from_index; i < iEnd; i++)
      if (p[i] == c)
        return i;

    return SIZE_MAX;
  }

  //! Returns the last index at which a given character `c` can be found in the string, or `SIZE_MAX` if not present.
  [[nodiscard]]
  BL_INLINE size_t last_index_of(char c) const noexcept {
    size_t i = size();
    const char* p = data();

    while (--i != SIZE_MAX && !(p[i] == c))
      continue;

    return i;
  }

  //! Returns the index at which a given character `c` can be found in the string starting from `from_index` and ending
  //! at `0`, or `SIZE_MAX` if not present.
  [[nodiscard]]
  BL_INLINE size_t last_index_of(char c, size_t from_index) const noexcept {
    size_t i = size() - 1;
    const char* p = data();

    if (i == SIZE_MAX)
      return i;

    i = bl_min<size_t>(i, from_index);
    while (!(p[i] == c) && --i != SIZE_MAX)
      continue;

    return i;
  }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_STRING_H_INCLUDED
