// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_ARRAY_H_INCLUDED
#define BLEND2D_ARRAY_H_INCLUDED

#include <blend2d/core/object.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLArray - C API
//!
//! Array functionality is provided by \ref BLArrayCore in C API and wrapped by \ref BLArray<T> template in C++ API.
//!
//! C API users must call either generic functions with `Item` suffix or correct specialized functions in case
//! of typed arrays. For example if you create a `BLArray<uint32_t>` in C then you can only modify it through
//! functions that have either `U32` or `Item` suffix. Arrays of signed types are treated as arrays of unsigned
//! types at API level as there is no difference between them from an implementation perspective.
//!
//! \{

//! Array container [C API].
struct BLArrayCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL

#ifdef __cplusplus
  //! \cond INTERNAL

  template<typename T>
  [[nodiscard]]
  BL_INLINE_NODEBUG T& dcast() noexcept { return static_cast<T&>(*this); }

  template<typename T>
  [[nodiscard]]
  BL_INLINE_NODEBUG const T& dcast() const noexcept { return static_cast<const T&>(*this); }

  //! \endcond
#endif
};

//! \cond INTERNAL
//! Array container [C API Impl].
struct BLArrayImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Pointer to array data.
  void* data;
  //! Array size [in items].
  size_t size;
  //! Array capacity [in items].
  size_t capacity;

#ifdef __cplusplus
  //! Returns the pointer to the `data` casted to `T*`.
  template<typename T>
  [[nodiscard]]
  BL_INLINE_NODEBUG T* data_as() noexcept { return (T*)data; }

  //! Returns the pointer to the `data` casted to `const T*`.
  template<typename T>
  [[nodiscard]]
  BL_INLINE_NODEBUG const T* data_as() const noexcept { return (const T*)data; }
#endif
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_array_init(BLArrayCore* self, BLObjectType array_type) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_init_move(BLArrayCore* self, BLArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_init_weak(BLArrayCore* self, const BLArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_destroy(BLArrayCore* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_array_reset(BLArrayCore* self) BL_NOEXCEPT_C;

BL_API size_t BL_CDECL bl_array_get_size(const BLArrayCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API size_t BL_CDECL bl_array_get_capacity(const BLArrayCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API size_t BL_CDECL bl_array_get_item_size(BLArrayCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API const void* BL_CDECL bl_array_get_data(const BLArrayCore* self) BL_NOEXCEPT_C BL_PURE;

BL_API BLResult BL_CDECL bl_array_clear(BLArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_shrink(BLArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_reserve(BLArrayCore* self, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_resize(BLArrayCore* self, size_t n, const void* fill) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_array_make_mutable(BLArrayCore* self, void** data_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_modify_op(BLArrayCore* self, BLModifyOp op, size_t n, void** data_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_op(BLArrayCore* self, size_t index, size_t n, void** data_out) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_array_assign_move(BLArrayCore* self, BLArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_assign_weak(BLArrayCore* self, const BLArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_assign_deep(BLArrayCore* self, const BLArrayCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_assign_data(BLArrayCore* self, const void* data, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_assign_external_data(BLArrayCore* self, void* data, size_t size, size_t capacity, BLDataAccessFlags data_access_flags, BLDestroyExternalDataFunc destroy_func, void* user_data) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_array_append_u8(BLArrayCore* self, uint8_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_append_u16(BLArrayCore* self, uint16_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_append_u32(BLArrayCore* self, uint32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_append_u64(BLArrayCore* self, uint64_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_append_f32(BLArrayCore* self, float value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_append_f64(BLArrayCore* self, double value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_append_item(BLArrayCore* self, const void* item) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_append_data(BLArrayCore* self, const void* data, size_t n) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_array_insert_u8(BLArrayCore* self, size_t index, uint8_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_u16(BLArrayCore* self, size_t index, uint16_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_u32(BLArrayCore* self, size_t index, uint32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_u64(BLArrayCore* self, size_t index, uint64_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_f32(BLArrayCore* self, size_t index, float value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_f64(BLArrayCore* self, size_t index, double value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_item(BLArrayCore* self, size_t index, const void* item) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_insert_data(BLArrayCore* self, size_t index, const void* data, size_t n) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_array_replace_u8(BLArrayCore* self, size_t index, uint8_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_replace_u16(BLArrayCore* self, size_t index, uint16_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_replace_u32(BLArrayCore* self, size_t index, uint32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_replace_u64(BLArrayCore* self, size_t index, uint64_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_replace_f32(BLArrayCore* self, size_t index, float value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_replace_f64(BLArrayCore* self, size_t index, double value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_replace_item(BLArrayCore* self, size_t index, const void* item) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_replace_data(BLArrayCore* self, size_t r_start, size_t r_end, const void* data, size_t n) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_array_remove_index(BLArrayCore* self, size_t index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_array_remove_range(BLArrayCore* self, size_t r_start, size_t r_end) BL_NOEXCEPT_C;

BL_API bool BL_CDECL bl_array_equals(const BLArrayCore* a, const BLArrayCore* b) BL_NOEXCEPT_C BL_PURE;

BL_END_C_DECLS

//! \}

//! \}

//! \addtogroup bl_containers
//! \{

//! \cond INTERNAL
//! \name BLArray - Internals
//! \{

#ifdef __cplusplus
namespace BLInternal {
namespace {

template<typename T, uint32_t TypeCategory>
struct ArrayTraitsByCategory {
  static constexpr const uint32_t kArrayType = BL_OBJECT_TYPE_NULL;

  typedef T CompatibleType;
  static BL_INLINE_NODEBUG const T& pass(const T& arg) noexcept { return arg; }
};

template<typename T>
struct ArrayTraitsByCategory<T, kTypeCategoryObject> {
  static constexpr const uint32_t kArrayType = BL_OBJECT_TYPE_ARRAY_OBJECT;

  typedef T CompatibleType;
  static BL_INLINE_NODEBUG const CompatibleType& pass(const T& arg) noexcept { return arg; }
};

template<typename T>
struct ArrayTraitsByCategory<T, kTypeCategoryPtr> {
  static constexpr const uint32_t kArrayType =
    sizeof(T) == 4 ? BL_OBJECT_TYPE_ARRAY_UINT32 : BL_OBJECT_TYPE_ARRAY_UINT64;

  using CompatibleType = BLInternal::UIntByType<T>;
  static BL_INLINE_NODEBUG CompatibleType pass(const T& arg) noexcept { return (CompatibleType)arg; }
};

template<typename T>
struct ArrayTraitsByCategory<T, kTypeCategoryInt> {
  static constexpr const uint32_t kArrayType =
    sizeof(T) == 1 && std::is_signed_v  <T> ? BL_OBJECT_TYPE_ARRAY_INT8   :
    sizeof(T) == 1 && std::is_unsigned_v<T> ? BL_OBJECT_TYPE_ARRAY_UINT8  :
    sizeof(T) == 2 && std::is_signed_v  <T> ? BL_OBJECT_TYPE_ARRAY_INT16  :
    sizeof(T) == 2 && std::is_unsigned_v<T> ? BL_OBJECT_TYPE_ARRAY_UINT16 :
    sizeof(T) == 4 && std::is_signed_v  <T> ? BL_OBJECT_TYPE_ARRAY_INT32  :
    sizeof(T) == 4 && std::is_unsigned_v<T> ? BL_OBJECT_TYPE_ARRAY_UINT32 :
    sizeof(T) == 8 && std::is_signed_v  <T> ? BL_OBJECT_TYPE_ARRAY_INT64  :
    sizeof(T) == 8 && std::is_unsigned_v<T> ? BL_OBJECT_TYPE_ARRAY_UINT64 : BL_OBJECT_TYPE_NULL;

  using CompatibleType = BLInternal::UIntByType<T>;
  static BL_INLINE_NODEBUG CompatibleType pass(const T& arg) noexcept { return (CompatibleType)arg; }
};

template<typename T>
struct ArrayTraitsByCategory<T, kTypeCategoryFloat> {
  static constexpr const uint32_t kArrayType =
    sizeof(T) == 4 ? BL_OBJECT_TYPE_ARRAY_FLOAT32 :
    sizeof(T) == 8 ? BL_OBJECT_TYPE_ARRAY_FLOAT64 : BL_OBJECT_TYPE_NULL;

  typedef T CompatibleType;
  static BL_INLINE_NODEBUG CompatibleType pass(const T& arg) noexcept { return (CompatibleType)arg; }
};

template<typename T>
struct ArrayTraitsByCategory<T, kTypeCategoryStruct> {
  static constexpr const uint32_t kArrayType =
    sizeof(T) ==  1 ? BL_OBJECT_TYPE_ARRAY_STRUCT_1  :
    sizeof(T) ==  2 ? BL_OBJECT_TYPE_ARRAY_STRUCT_2  :
    sizeof(T) ==  3 ? BL_OBJECT_TYPE_ARRAY_STRUCT_3  :
    sizeof(T) ==  4 ? BL_OBJECT_TYPE_ARRAY_STRUCT_4  :
    sizeof(T) ==  6 ? BL_OBJECT_TYPE_ARRAY_STRUCT_6  :
    sizeof(T) ==  8 ? BL_OBJECT_TYPE_ARRAY_STRUCT_8  :
    sizeof(T) == 10 ? BL_OBJECT_TYPE_ARRAY_STRUCT_10 :
    sizeof(T) == 12 ? BL_OBJECT_TYPE_ARRAY_STRUCT_12 :
    sizeof(T) == 16 ? BL_OBJECT_TYPE_ARRAY_STRUCT_16 :
    sizeof(T) == 20 ? BL_OBJECT_TYPE_ARRAY_STRUCT_20 :
    sizeof(T) == 24 ? BL_OBJECT_TYPE_ARRAY_STRUCT_24 :
    sizeof(T) == 32 ? BL_OBJECT_TYPE_ARRAY_STRUCT_32 : BL_OBJECT_TYPE_NULL;

  typedef T CompatibleType;
  static BL_INLINE_NODEBUG CompatibleType pass(const T& arg) noexcept { return (CompatibleType)arg; }
};

template<typename T>
struct ArrayTraits : public ArrayTraitsByCategory<T, TypeTraits<T>::kCategory> {};

template<typename T>
BL_INLINE_NODEBUG const T& first_in_var_args(const T& arg) noexcept { return arg; }
template<typename T, typename... Args>
BL_INLINE_NODEBUG const T& first_in_var_args(const T& arg, Args&&...) noexcept { return arg; }

template<typename T, typename Arg0>
BL_INLINE_NODEBUG void copy_to_uninitialized(T* dst, Arg0&& src) noexcept {
  bl_call_ctor(*dst, forward<Arg0>(src));
}

template<typename T, typename Arg0, typename... Args>
BL_INLINE_NODEBUG void copy_to_uninitialized(T* dst, Arg0&& arg0, Args&&... args) noexcept {
  copy_to_uninitialized(dst + 0, forward<Arg0>(arg0));
  copy_to_uninitialized(dst + 1, forward<Args>(args)...);
}

template<typename T> BL_INLINE_NODEBUG BLResult append_item(BLArrayCore* self, const T& item) noexcept { return bl_array_append_item(self, &item); }
template<> BL_INLINE_NODEBUG BLResult append_item(BLArrayCore* self, const uint8_t& item) noexcept { return bl_array_append_u8(self, item); }
template<> BL_INLINE_NODEBUG BLResult append_item(BLArrayCore* self, const uint16_t& item) noexcept { return bl_array_append_u16(self, item); }
template<> BL_INLINE_NODEBUG BLResult append_item(BLArrayCore* self, const uint32_t& item) noexcept { return bl_array_append_u32(self, item); }
template<> BL_INLINE_NODEBUG BLResult append_item(BLArrayCore* self, const uint64_t& item) noexcept { return bl_array_append_u64(self, item); }
template<> BL_INLINE_NODEBUG BLResult append_item(BLArrayCore* self, const float& item) noexcept { return bl_array_append_f32(self, item); }
template<> BL_INLINE_NODEBUG BLResult append_item(BLArrayCore* self, const double& item) noexcept { return bl_array_append_f64(self, item); }

template<typename T> BL_INLINE_NODEBUG BLResult insert_item(BLArrayCore* self, size_t index, const T& item) noexcept { return bl_array_insert_item(self, index, &item); }
template<> BL_INLINE_NODEBUG BLResult insert_item(BLArrayCore* self, size_t index, const uint8_t& item) noexcept { return bl_array_insert_u8(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult insert_item(BLArrayCore* self, size_t index, const uint16_t& item) noexcept { return bl_array_insert_u16(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult insert_item(BLArrayCore* self, size_t index, const uint32_t& item) noexcept { return bl_array_insert_u32(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult insert_item(BLArrayCore* self, size_t index, const uint64_t& item) noexcept { return bl_array_insert_u64(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult insert_item(BLArrayCore* self, size_t index, const float& item) noexcept { return bl_array_insert_f32(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult insert_item(BLArrayCore* self, size_t index, const double& item) noexcept { return bl_array_insert_f64(self, index, item); }

template<typename T> BL_INLINE_NODEBUG BLResult replace_item(BLArrayCore* self, size_t index, const T& item) noexcept { return bl_array_replace_item(self, index, &item); }
template<> BL_INLINE_NODEBUG BLResult replace_item(BLArrayCore* self, size_t index, const uint8_t& item) noexcept { return bl_array_replace_u8(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult replace_item(BLArrayCore* self, size_t index, const uint16_t& item) noexcept { return bl_array_replace_u16(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult replace_item(BLArrayCore* self, size_t index, const uint32_t& item) noexcept { return bl_array_replace_u32(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult replace_item(BLArrayCore* self, size_t index, const uint64_t& item) noexcept { return bl_array_replace_u64(self, index, item); }
template<> BL_INLINE_NODEBUG BLResult replace_item(BLArrayCore* self, size_t index, const double& item) noexcept { return bl_array_replace_f64(self, index, item); }

} // {anonymous}
} // {BLInternal}
#endif

//! \}
//! \endcond

//! \name BLArray - C++ API
//! \{
#ifdef __cplusplus

//! Array container (template) [C++ API].
template<typename T>
class BLArray final : public BLArrayCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  //! Array traits of `T`.
  typedef BLInternal::ArrayTraits<T> Traits;

  //! Implementation type of this BLArray<T> matching `T` traits.
  enum : uint32_t {
    kArrayType = Traits::kArrayType,

    //! Capacity of an SSO array - depends on the size of `T`.
    kSSOCapacity = BLObjectDetail::kStaticDataSize / uint32_t(sizeof(T)),

    //! Signature of SSO representation of an empty array.
    kSSOEmptySignature = BLObjectInfo::pack_type_with_marker(BLObjectType(kArrayType)) | BLObjectInfo::pack_abcp(0u, kSSOCapacity)
  };

  static_assert(uint32_t(kArrayType) != BL_OBJECT_TYPE_NULL,
                "Type 'T' cannot be used with 'BLArray<T>' as it's either non-trivial or non-specialized");

  [[nodiscard]]
  BL_INLINE_NODEBUG BLArrayImpl* _impl() const noexcept { return static_cast<BLArrayImpl*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed array.
  BL_INLINE_NODEBUG BLArray() noexcept {
    _d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  //! Move constructor.
  //!
  //! \note The `other` array is always reset by a move construction, so it becomes an empty array.
  BL_INLINE_NODEBUG BLArray(BLArray&& other) noexcept {
    _d = other._d;
    other._d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  //! Copy constructor, performs weak copy of the data held by the `other` array.
  BL_INLINE_NODEBUG BLArray(const BLArray& other) noexcept {
    bl_array_init_weak(this, &other);
  }

  //! Destroys the array.
  BL_INLINE_NODEBUG ~BLArray() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_array_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Tests whether the array has items. Returns `true` if the array is not empty.
  //!
  //! \note This is essentially the opposite of `is_empty()`.
  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return !is_empty(); }

  //! Move assignment.
  //!
  //! \note The `other` array is reset by move assignment, so its state after the move operation is the same as
  //! the default constructed array.
  BL_INLINE_NODEBUG BLArray& operator=(BLArray&& other) noexcept { bl_array_assign_move(this, &other); return *this; }
  //! Copy assignment, performs weak copy of the data held by the `other` array.
  BL_INLINE_NODEBUG BLArray& operator=(const BLArray& other) noexcept { bl_array_assign_weak(this, &other); return *this; }

  //! Returns true if this and `other` arrays are equal.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLArray& other) const noexcept { return equals(other); }

  //! Returns true if this and `other` arrays are not equal.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLArray& other) const noexcept { return !equals(other); }

  //! Returns a read-only reference to the array item at the given `index`.
  //!
  //! \note This is the same as calling `at(index)`.
  [[nodiscard]]
  BL_INLINE_NODEBUG const T& operator[](size_t index) const noexcept { return at(index); }

  //! \}

  //! \name Common Functionality
  //! \{

  //! Resets the array into a default constructed state by clearing its content and releasing its memory.
  //!
  //! \note This function always returns \ref BL_SUCCESS.
  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_array_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLArray<T> after reset.
    BL_ASSUME(_d.info.bits == kSSOEmptySignature);

    return result;
  }

  //! Swaps the content of this array with the `other` array.
  BL_INLINE_NODEBUG void swap(BLArray<T>& other) noexcept { _d.swap(other._d); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the array is empty.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size() == 0; }

  //! Returns the size of the array (number of items).
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t size() const noexcept { return _d.sso() ? size_t(_d.a_field()) : _impl()->size; }

  //! Returns the capacity of the array (number of items).
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return _d.sso() ? size_t(_d.b_field()) : _impl()->capacity; }

  //! Returns a pointer to the array data.
  [[nodiscard]]
  BL_INLINE_NODEBUG const T* data() const noexcept { return _d.sso() ? (const T*)(_d.char_data) : (const T*)_impl()->data; }

  //! Returns a pointer to the array data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const T* begin() const noexcept { return data(); }

  //! Returns a pointer to the end of array data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const T* end() const noexcept {
    return _d.sso() ? (const T*)(_d.char_data) + size_t(_d.a_field())
                    : (const T*)_impl()->data + _impl()->size;
  }

  //! Returns the array data as `BLArrayView<T>`.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLArrayView<T> view() const noexcept { return BLArrayView<T> { data(), size() }; }

  //! Returns a read-only reference to the array item at the given `index`.
  //!
  //! \note The index must be valid, which means it has to be less than the array length. Accessing items out of range
  //! is undefined behavior that would be caught by assertions in debug builds.
  [[nodiscard]]
  BL_INLINE const T& at(size_t index) const noexcept {
    BL_ASSERT(index < size());
    return data()[index];
  }

  //! Returns a read-only reference to the first item.
  //!
  //! \note The array must have at least one item otherwise calling `first()` would point to the end of the array,
  //! which is not initialized, and such reference would be invalid. Debug builds would catch this condition with
  //! an assertion.
  [[nodiscard]]
  BL_INLINE_NODEBUG const T& first() const noexcept { return at(0); }

  //! Returns a read-only reference to the last item.
  //!
  //! \note The array must have at least one item otherwise calling `last()`  would point to the end of the array,
  //! which is not initialized, and such reference would be invalid. Debug builds would catch this condition with
  //! an assertion.
  [[nodiscard]]
  BL_INLINE_NODEBUG const T& last() const noexcept { return at(size() - 1); }

  //! \}

  //! \name Data Manipulation
  //! \{

  //! Clears the content of the array.
  //!
  //! \note If the array uses a dynamically allocated memory and the instance is mutable the memory won't be released,
  //! it will be reused instead. Consider using `reset()` if you want to release the memory in such case instead.
  BL_INLINE_NODEBUG BLResult clear() noexcept { return bl_array_clear(this); }

  //! Shrinks the capacity of the array to fit its length.
  //!
  //! Some array operations like `append()` may grow the array more than necessary to make it faster when such
  //! manipulation operations are called consecutively. When you are done with modifications and you know the
  //! lifetime of the array won't be short you can use `shrink()` to fit its memory requirements to the number
  //! of items it stores, which could optimize the application's memory requirements.
  BL_INLINE_NODEBUG BLResult shrink() noexcept { return bl_array_shrink(this); }

  //! Reserves the array capacity to hold at least `n` items.
  BL_INLINE_NODEBUG BLResult reserve(size_t n) noexcept { return bl_array_reserve(this, n); }

  //! Truncates the length of the array to maximum `n` items.
  //!
  //! If the length of the array is less than `n`n then truncation does nothing.
  BL_INLINE_NODEBUG BLResult truncate(size_t n) noexcept { return bl_array_resize(this, bl_min(n, size()), nullptr); }

  //! Resizes the array to `n` items.
  //!
  //! If `n` is greater than the array length then all new items will be initialized by `fill` item.
  BL_INLINE_NODEBUG BLResult resize(size_t n, const T& fill) noexcept { return bl_array_resize(this, n, &fill); }

  //! Makes the array mutable by possibly creating a deep copy of the data if it's either read-only or shared with
  //! another array. Stores the pointer to the beginning of mutable data in `data_out`.
  //!
  //! ```
  //! BLArray<uint8_t> a;
  //! if (a.append(0, 1, 2, 3, 4, 5, 6, 7) != BL_SUCCESS) {
  //!   // Handle error condition.
  //! }
  //!
  //! uint8_t* data;
  //! if (a.make_mutable(&data) != BL_SUCCESS) {
  //!   // Handle error condition.
  //! }
  //!
  //! // `data` is a mutable pointer to array content of 8 items.
  //! data[0] = 100;
  //!
  //! // Calling array member functions (or C API) could invalidate `data`.
  //! a.append(9); // You shouldn't use `data` afterwards.
  //! ```
  BL_INLINE_NODEBUG BLResult make_mutable(T** data_out) noexcept {
    return bl_array_make_mutable(this, (void**)data_out);
  }

  //! Modify operation is similar to `make_mutable`, however, the `op` argument specifies the desired array operation,
  //! see \ref BLModifyOp. The pointer returned in `data_out` points to the first item to be either assigned or
  //! appended and it points to an uninitialized memory.
  //!
  //! Please note that assignments mean to wipe out the whole array content and to set the length of the array to `n`.
  //! The caller is responsible for initializing the data returned in `data_out`.
  BL_INLINE_NODEBUG BLResult modify_op(BLModifyOp op, size_t n, T** data_out) noexcept {
    return bl_array_modify_op(this, op, n, (void**)data_out);
  }

  //! Insert operation, the semantics is similar to `modify_op()`, however, items are inserted at the given `index`
  //! instead of assigned or appended.
  //!
  //! The caller is responsible for initializing the data returned in `data_out`.
  BL_INLINE_NODEBUG BLResult insert_op(size_t index, size_t n, T** data_out) noexcept {
    return bl_array_insert_op(this, index, n, (void**)data_out);
  }

  //! Similar to `modify_op()`, but the items to assign/append to the array are given after the `op` argument.
  //!
  //! \note This is a variadic template that makes such modification easier if you have a constant number of items
  //! to assign or append.
  template<typename... Args>
  BL_INLINE BLResult modify_v(BLModifyOp op, Args&&... args) noexcept {
    T* dst;
    BL_PROPAGATE(bl_array_modify_op(this, op, sizeof...(args), (void**)&dst));
    BLInternal::copy_to_uninitialized(dst, BLInternal::forward<Args>(args)...);
    return BL_SUCCESS;
  }

  //! Move assignment, the same as `operator=`, but returns a `BLResult` instead of `this`.
  BL_INLINE_NODEBUG BLResult assign(BLArray<T>&& other) noexcept {
    return bl_array_assign_move(this, &other);
  }

  //! Copy assignment, the same as `operator=`, but returns a `BLResult` instead of `this`.
  BL_INLINE_NODEBUG BLResult assign(const BLArray<T>& other) noexcept {
    return bl_array_assign_weak(this, &other);
  }

  //! Copy assignment, but creates a deep copy of the `other` array instead of weak copy.
  BL_INLINE_NODEBUG BLResult assign_deep(const BLArray<T>& other) noexcept {
    return bl_array_assign_deep(this, &other);
  }

  //! Replaces the content of the array with variadic number of items passed in `args...`.
  template<typename... Args>
  BL_INLINE_NODEBUG BLResult assign_v(Args&&... args) noexcept {
    return modify_v(BL_MODIFY_OP_ASSIGN_FIT, BLInternal::forward<Args>(args)...);
  }

  //! Replaces the content of the array by items in the passed array `view`.
  //!
  //! \note The implementation can handle `view` pointing to the array's data as well, so it's possible to create
  //! a slice of the array if required.
  BL_INLINE_NODEBUG BLResult assign_data(const BLArrayView<T>& view) noexcept {
    return bl_array_assign_data(this, (const void*)view.data, view.size);
  }

  //! Replaces the content of the array `items` of length `n`.
  //!
  //! \note The implementation can handle items pointing to the array's data as well, so it's possible to create
  //! a slice of the array if required.
  BL_INLINE_NODEBUG BLResult assign_data(const T* items, size_t n) noexcept {
    return bl_array_assign_data(this, (const void*)items, n);
  }

  //! Assign an external buffer to the array, which would replace the existing content.
  //!
  //! \param data External data buffer to use (cannot be NULL).
  //! \param size Size of the data buffer in items.
  //! \param capacity Capacity of the buffer, cannot be zero or smaller than `size`.
  //! \param access_flags Flags that describe whether the data is read-only or read-write, see \ref BLDataAccessFlags.
  //! \param destroy_func A function that would be called when the array is destroyed (can be null if you don't need it).
  //! \param user_data User data passed to `destroy_func`.
  BL_INLINE_NODEBUG BLResult assign_external_data(
    T* data,
    size_t size,
    size_t capacity,
    BLDataAccessFlags access_flags,
    BLDestroyExternalDataFunc destroy_func = nullptr,
    void* user_data = nullptr) noexcept {

    return bl_array_assign_external_data(this, data, size, capacity, access_flags, destroy_func, user_data);
  }

  //! Appends a variadic number of items items passed in `args...` to the array.
  //!
  //! \note The data in `args` cannot point to the same data that the array holds as the function that prepares the
  //! append operation has no way to know about the source (it only makes space for new data). It's an undefined
  //! behavior in such case.
  template<typename... Args>
  BL_INLINE BLResult append(Args&&... args) noexcept {
    if constexpr (sizeof...(args) == 1)
      return BLInternal::append_item(this, Traits::pass(BLInternal::first_in_var_args(BLInternal::forward<Args>(args)...)));
    else
      return modify_v(BL_MODIFY_OP_APPEND_GROW, BLInternal::forward<Args>(args)...);
  }

  //! Appends items to the array of the given array `view`.
  //!
  //! \note The implementation guarantees that a `view` pointing to the array data itself would work.
  BL_INLINE_NODEBUG BLResult append_data(const BLArrayView<T>& view) noexcept {
    return bl_array_append_data(this, (const void*)view.data, view.size);
  }

  //! Appends `items` to the array of length `n`.
  //!
  //! \note The implementation guarantees that a `items` pointing to the array data itself would work.
  BL_INLINE_NODEBUG BLResult append_data(const T* items, size_t n) noexcept {
    return bl_array_append_data(this, (const void*)items, n);
  }

  //! Prepends a variadic number of items items passed in `args...` to the array.
  //!
  //! \note The data in `args` cannot point to the same data that the array holds as the function that prepares the
  //! prepend operation has no way to know about the source (it only makes space for new data). It's an undefined
  //! behavior in such case.
  template<typename... Args>
  BL_INLINE BLResult prepend(Args&&... args) noexcept {
    if constexpr (sizeof...(args) == 1)
      return BLInternal::insert_item(this, 0, Traits::pass(BLInternal::first_in_var_args(BLInternal::forward<Args>(args)...)));
    else
      return insert(0, BLInternal::forward<Args>(args)...);
  }

  //! Prepends items to the array of the given array `view`.
  //!
  //! \note The implementation guarantees that a `view` pointing to the array data itself would work.
  BL_INLINE_NODEBUG BLResult prepend_data(const BLArrayView<T>& view) noexcept {
    return bl_array_insert_data(this, 0, (const void*)view.data, view.size);
  }

  //! Prepends `items` to the array of length `n`.
  //!
  //! \note The implementation guarantees that a `items` pointing to the array data itself would work.
  BL_INLINE_NODEBUG BLResult prepend_data(const T* items, size_t n) noexcept {
    return bl_array_insert_data(this, 0, (const void*)items, n);
  }

  //! Inserts a variadic number of items items passed in `args...` at the given `index`.
  //!
  //! \note The data in `args` cannot point to the same data that the array holds as the function that prepares the
  //! insert operation has no way to know about the source (it only makes space for new data). It's an undefined
  //! behavior in such case.
  template<typename... Args>
  BL_INLINE BLResult insert(size_t index, Args&&... args) noexcept {
    if constexpr (sizeof...(args) == 1) {
      return BLInternal::insert_item(this, index, Traits::pass(BLInternal::first_in_var_args(BLInternal::forward<Args>(args)...)));
    }
    else {
      T* dst;
      BL_PROPAGATE(bl_array_insert_op(this, index, sizeof...(args), (void**)&dst));
      BLInternal::copy_to_uninitialized(dst, BLInternal::forward<Args>(args)...);
      return BL_SUCCESS;
    }
  }

  //! Inserts items to the array of the given array `view` at the given `index`.
  //!
  //! \note The implementation guarantees that a `view` pointing to the array data itself would work.
  BL_INLINE_NODEBUG BLResult insert_data(size_t index, const BLArrayView<T>& view) noexcept {
    return bl_array_insert_data(this, index, (const void*)view.data, view.size);
  }

  //! Prepends `items` to the array of length `n` at the given `index`.
  //!
  //! \note The implementation guarantees that a `items` pointing to the array data itself would work.
  BL_INLINE_NODEBUG BLResult insert_data(size_t index, const T* items, size_t n) noexcept {
    return bl_array_insert_data(this, index, (const void*)items, n);
  }

  //! Replaces an item at the given `index` by `item`.
  BL_INLINE_NODEBUG BLResult replace(size_t index, const T& item) noexcept {
    return BLInternal::replace_item(this, index, Traits::pass(item));
  }

  //! Replaces the given `range` of items by the given array `view`.
  BL_INLINE_NODEBUG BLResult replace_data(const BLRange& range, BLArrayView<T>& view) noexcept {
    return bl_array_replace_data(this, range.start, range.end, (const void*)view.data, view.size);
  }

  //! Replaces the given `range` of items by `items` of length `n`.
  BL_INLINE_NODEBUG BLResult replace_data(const BLRange& range, const T* items, size_t n) noexcept {
    return bl_array_replace_data(this, range.start, range.end, items, n);
  }

  //! Removes an item at the given `index`.
  BL_INLINE_NODEBUG BLResult remove(size_t index) noexcept {
    return bl_array_remove_index(this, index);
  }

  //! Removes a `range` of items.
  BL_INLINE_NODEBUG BLResult remove(const BLRange& range) noexcept {
    return bl_array_remove_range(this, range.start, range.end);
  }

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Returns whether the content of this array and `other` matches.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLArray<T>& other) const noexcept {
    return bl_array_equals(this, &other);
  }

  //! \}

  //! \name Search
  //! \{

  //! Returns the first index at which a given `item` can be found in the array, or `SIZE_MAX` if not found.
  [[nodiscard]]
  BL_INLINE size_t index_of(const T& item) const noexcept {
    return index_of(item, 0);
  }

  //! Returns the index at which a given `item` can be found in the array starting from `from_index`, or `SIZE_MAX`
  //! if not present.
  [[nodiscard]]
  BL_INLINE size_t index_of(const T& item, size_t from_index) const noexcept {
    const T* p = data();
    size_t iEnd = size();

    for (size_t i = from_index; i < iEnd; i++)
      if (p[i] == item)
        return i;

    return SIZE_MAX;
  }

  //! Returns the last index at which a given `item` can be found in the array, or `SIZE_MAX` if not present.
  [[nodiscard]]
  BL_INLINE size_t last_index_of(const T& item) const noexcept {
    const T* p = data();
    size_t i = size();

    while (--i != SIZE_MAX && !(p[i] == item))
      continue;

    return i;
  }

  //! Returns the index at which a given `item` can be found in the array starting from `from_index` and ending
  //! at `0`, or `SIZE_MAX` if not present.
  [[nodiscard]]
  BL_INLINE size_t last_index_of(const T& item, size_t from_index) const noexcept {
    const T* p = data();
    size_t i = size() - 1;

    if (i == SIZE_MAX)
      return i;

    i = bl_min<size_t>(i, from_index);
    while (!(p[i] == item) && --i != SIZE_MAX)
      continue;

    return i;
  }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_ARRAY_H_INCLUDED
