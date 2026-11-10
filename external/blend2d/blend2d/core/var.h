
// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_VAR_H_INCLUDED
#define BLEND2D_VAR_H_INCLUDED

#include <blend2d/core/object.h>
#include <blend2d/core/rgba.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLVar - C API
//! \{

//! Variant [C API].
struct BLVarCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLVar)
};

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_var_init_type(BLUnknown* self, BLObjectType type) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_null(BLUnknown* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_bool(BLUnknown* self, bool value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_int32(BLUnknown* self, int32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_int64(BLUnknown* self, int64_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_uint32(BLUnknown* self, uint32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_uint64(BLUnknown* self, uint64_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_double(BLUnknown* self, double value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_rgba(BLUnknown* self, const BLRgba* rgba) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_rgba32(BLUnknown* self, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_rgba64(BLUnknown* self, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_move(BLUnknown* self, BLUnknown* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_init_weak(BLUnknown* self, const BLUnknown* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_destroy(BLUnknown* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_reset(BLUnknown* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_var_assign_null(BLUnknown* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_bool(BLUnknown* self, bool value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_int32(BLUnknown* self, int32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_int64(BLUnknown* self, int64_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_uint32(BLUnknown* self, uint32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_uint64(BLUnknown* self, uint64_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_double(BLUnknown* self, double value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_rgba(BLUnknown* self, const BLRgba* rgba) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_rgba32(BLUnknown* self, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_rgba64(BLUnknown* self, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_move(BLUnknown* self, BLUnknown* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_assign_weak(BLUnknown* self, const BLUnknown* other) BL_NOEXCEPT_C;

BL_API BLObjectType BL_CDECL bl_var_get_type(const BLUnknown* self) BL_NOEXCEPT_C BL_PURE;

BL_API BLResult BL_CDECL bl_var_to_bool(const BLUnknown* self, bool* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_int32(const BLUnknown* self, int32_t* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_int64(const BLUnknown* self, int64_t* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_uint32(const BLUnknown* self, uint32_t* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_uint64(const BLUnknown* self, uint64_t* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_double(const BLUnknown* self, double* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_rgba(const BLUnknown* self, BLRgba* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_rgba32(const BLUnknown* self, uint32_t* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_var_to_rgba64(const BLUnknown* self, uint64_t* out) BL_NOEXCEPT_C;

BL_API bool BL_CDECL bl_var_equals(const BLUnknown* a, const BLUnknown* b) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_null(const BLUnknown* self) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_bool(const BLUnknown* self, bool value) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_int64(const BLUnknown* self, int64_t value) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_uint64(const BLUnknown* self, uint64_t value) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_double(const BLUnknown* self, double value) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_rgba(const BLUnknown* self, const BLRgba* rgba) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_rgba32(const BLUnknown* self, uint32_t rgba32) BL_NOEXCEPT_C BL_PURE;
BL_API bool BL_CDECL bl_var_equals_rgba64(const BLUnknown* self, uint64_t rgba64) BL_NOEXCEPT_C BL_PURE;

BL_API bool BL_CDECL bl_var_strict_equals(const BLUnknown* a, const BLUnknown* b) BL_NOEXCEPT_C BL_PURE;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_globals
//! \{

//! \name BLVar - C++ API
//! \{

#ifdef __cplusplus

//! \cond INTERNAL
namespace BLInternal {
namespace {

//! Helper class that provides static functions that are called by `BLVar` member functions. There are
//! several specializations for primitive types like int/float and also for special types like `BLRgba`.
template<typename T, uint32_t Category>
struct VarOps;

template<typename T>
struct VarOps<T, kTypeCategoryBool> {
  static BL_INLINE_NODEBUG void init(BLVarCore* self, const T& value) noexcept {
    self->_d.init_bool(value);
  }

  static BL_INLINE_NODEBUG BLResult assign(BLVarCore* self, const T& value) noexcept {
    return bl_var_assign_bool(self, value);
  }

  static BL_INLINE_NODEBUG bool equals(const BLVarCore* self, const T& value) noexcept {
    return bl_var_equals_bool(self, value);
  }
};

template<typename T>
struct VarOps<T, kTypeCategoryInt> {
  static BL_INLINE void init(BLVarCore* self, const T& value) noexcept {
    if constexpr (std::is_signed_v<T>)
      self->_d.init_int64(int64_t(value));
    else
      self->_d.init_uint64(uint64_t(value));
  }

  static BL_INLINE BLResult assign(BLVarCore* self, const T& value) noexcept {
    if constexpr (sizeof(T) <= 4 && std::is_signed_v<T>)
      return bl_var_assign_int32(self, int32_t(value));
    else if constexpr (sizeof(T) <= 4)
      return bl_var_assign_uint32(self, uint32_t(value));
    else if constexpr (std::is_signed_v<T>)
      return bl_var_assign_int64(self, int64_t(value));
    else
      return bl_var_assign_uint64(self, uint64_t(value));
  }

  static BL_INLINE bool equals(const BLVarCore* self, const T& value) noexcept {
    if constexpr (std::is_signed_v<T>)
      return bl_var_equals_int64(self, int64_t(value));
    else
      return bl_var_equals_uint64(self, uint64_t(value));
  }
};

template<typename T>
struct VarOps<T, kTypeCategoryFloat> {
  static BL_INLINE_NODEBUG void init(BLVarCore* self, const T& value) noexcept {
    self->_d.init_double(double(value));
  }

  static BL_INLINE_NODEBUG BLResult assign(BLVarCore* self, const T& value) noexcept {
    return bl_var_assign_double(self, double(value));
  }

  static BL_INLINE_NODEBUG bool equals(const BLVarCore* self, const T& value) noexcept {
    return bl_var_equals_double(self, double(value));
  }
};

template<typename T>
struct VarOps<T, kTypeCategoryObject> {
  static BL_INLINE_NODEBUG void init(BLVarCore* self, T&& value) noexcept { bl_var_init_move(self, &value); }
  static BL_INLINE_NODEBUG void init(BLVarCore* self, const T& value) noexcept { bl_var_init_weak(self, &value); }

  static BL_INLINE_NODEBUG BLResult assign(BLVarCore* self, T&& value) noexcept { return bl_var_assign_move(self, &value); }
  static BL_INLINE_NODEBUG BLResult assign(BLVarCore* self, const T& value) noexcept { return bl_var_assign_weak(self, &value); }

  static BL_INLINE_NODEBUG bool equals(const BLVarCore* self, const T& value) noexcept { return bl_var_equals(self, &value); }
};

template<>
struct VarOps<BLRgba, kTypeCategoryStruct> {
  static BL_INLINE void init(BLVarCore* self, const BLRgba& value) noexcept {
    // Inlined version of `bl_var_init_rgba()`.
    uint32_t r = bl_bit_cast<uint32_t>(value.r);
    uint32_t g = bl_bit_cast<uint32_t>(value.g);
    uint32_t b = bl_bit_cast<uint32_t>(value.b);
    uint32_t a = bl_max<uint32_t>(bl_bit_cast<uint32_t>(value.a), 0);
    self->_d.init_u32x4(r, g, b, a);
  }

  static BL_INLINE_NODEBUG BLResult assign(BLVarCore* self, const BLRgba& value) noexcept { return bl_var_assign_rgba(self, &value); }
  static BL_INLINE_NODEBUG bool equals(const BLVarCore* self, const BLRgba& value) noexcept { return bl_var_strict_equals(self, &value); }
};

template<>
struct VarOps<BLRgba32, kTypeCategoryStruct> {
  static BL_INLINE void init(BLVarCore* self, const BLRgba32& rgba32) noexcept { self->_d.init_rgba32(rgba32.value); }
  static BL_INLINE_NODEBUG BLResult assign(BLVarCore* self, const BLRgba32 rgba32) noexcept { return bl_var_assign_rgba32(self, rgba32.value); }
  static BL_INLINE_NODEBUG bool equals(const BLVarCore* self, const BLRgba32& rgba32) noexcept { return bl_var_equals_rgba32(self, rgba32.value); }
};

template<>
struct VarOps<BLRgba64, kTypeCategoryStruct> {
  static BL_INLINE void init(BLVarCore* self, const BLRgba64& rgba64) noexcept { self->_d.init_rgba64(rgba64.value); }
  static BL_INLINE_NODEBUG BLResult assign(BLVarCore* self, const BLRgba64 rgba64) noexcept { return bl_var_assign_rgba64(self, rgba64.value); }
  static BL_INLINE_NODEBUG bool equals(const BLVarCore* self, const BLRgba64& rgba64) noexcept { return bl_var_equals_rgba64(self, rgba64.value); }
};

template<typename T>
struct VarCastImpl {
  static BL_INLINE_NODEBUG T* cast(BLVarCore* self) noexcept { return static_cast<T*>(static_cast<BLObjectCore*>(self)); }
  static BL_INLINE_NODEBUG const T* cast(const BLVarCore* self) noexcept { return static_cast<const T*>(static_cast<const BLObjectCore*>(self)); }
};

template<>
struct VarCastImpl<BLRgba> {
  static BL_INLINE_NODEBUG BLRgba* cast(BLVarCore* self) noexcept { return &self->_d.rgba; }
  static BL_INLINE_NODEBUG const BLRgba* cast(const BLVarCore* self) noexcept { return &self->_d.rgba; }
};

template<>
struct VarCastImpl<BLRgba32> {
  static BL_INLINE_NODEBUG BLRgba32* cast(BLVarCore* self) noexcept { return &self->_d.rgba32; }
  static BL_INLINE_NODEBUG const BLRgba32* cast(const BLVarCore* self) noexcept { return &self->_d.rgba32; }
};

template<>
struct VarCastImpl<BLRgba64> {
  static BL_INLINE_NODEBUG BLRgba64* cast(BLVarCore* self) noexcept { return &self->_d.rgba64; }
  static BL_INLINE_NODEBUG const BLRgba64* cast(const BLVarCore* self) noexcept { return &self->_d.rgba64; }
};

} // {anonymous}
} // {BLInternal}
//! \endcond

//! Variant [C++ API].
class BLVar final : public BLVarCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  static inline constexpr uint32_t kNullSignature = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_NULL);

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLVar() noexcept {
    _d.init_static(BLObjectInfo{kNullSignature});
  }

  BL_INLINE_NODEBUG BLVar(BLVar&& other) noexcept {
    _d = other._d;
    other._d.init_static(BLObjectInfo{kNullSignature});
  }

  BL_INLINE_NODEBUG BLVar(const BLVar& other) noexcept { bl_var_init_weak(this, &other); }

  template<typename T>
  BL_INLINE_NODEBUG explicit BLVar(T&& value) noexcept {
    using DecayT = std::decay_t<T>;
    BLInternal::VarOps<DecayT, BLInternal::TypeTraits<DecayT>::kCategory>::init(this, BLInternal::forward<T>(value));
  }

  BL_INLINE_NODEBUG ~BLVar() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_var_destroy(this);
    }
  }

  //! \}

  //! \name Static Construction
  //! \{

  static BL_INLINE_NODEBUG BLVar null() noexcept { return BLVar(); }

  //! \}

  //! \name Overloaded Operators
  //! \{

  template<typename T>
  BL_INLINE_NODEBUG BLVar& operator=(T&& value) noexcept { assign(BLInternal::forward<T>(value)); return *this; }

  template<typename T>
  BL_INLINE_NODEBUG bool operator==(const T& other) const noexcept { return equals(other); }

  template<typename T>
  BL_INLINE_NODEBUG bool operator!=(const T& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult reset() noexcept { return bl_var_reset(this); }
  BL_INLINE_NODEBUG void swap(BLVarCore& other) noexcept { _d.swap(other._d); }

  //! \}

  //! \name Type Accessors
  //! \{

  //! Returns the type of the underlying object.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLObjectType type() const noexcept { return _d.get_type(); }

  //! Tests whether this `BLVar` instance represents a `BLArray<T>` storing any supported type.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_array() const noexcept { return _d.is_array(); }

  //! Tests whether this `BLVar` instance represents `BLBitArray`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_bit_array() const noexcept { return _d.is_bit_array(); }

  //! Tests whether this `BLVar` instance represents `BLBitSet`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_bit_set() const noexcept { return _d.is_bit_set(); }

  //! Tests whether this `BLVar` instance represents a boxed `bool` value.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_bool() const noexcept { return _d.is_bool(); }

  //! Tests whether this `BLVar` instance represents `BLContext`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_context() const noexcept { return _d.is_context(); }

  //! Tests whether this `BLVar` instance represents a boxed `double` value.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_double() const noexcept { return _d.is_double(); }

  //! Tests whether this `BLVar` instance represents `BLFont`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_font() const noexcept { return _d.is_font(); }

  //! Tests whether this `BLVar` instance represents `BLFontData`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_font_data() const noexcept { return _d.is_font_data(); }

  //! Tests whether this `BLVar` instance represents `BLFontFace`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_font_face() const noexcept { return _d.is_font_face(); }

  //! Tests whether this `BLVar` instance represents `BLFontManager`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_font_manager() const noexcept { return _d.is_font_manager(); }

  //! Tests whether this `BLVar` instance represents `BLGradient`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_gradient() const noexcept { return _d.is_gradient(); }

  //! Tests whether this `BLVar` instance represents `BLImage`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_image() const noexcept { return _d.is_image(); }

  //! Tests whether this `BLVar` instance represents `BLImageCodec`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_image_codec() const noexcept { return _d.is_image_codec(); }

  //! Tests whether this `BLVar` instance represents `BLImageDecoder`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_image_decoder() const noexcept { return _d.is_image_decoder(); }

  //! Tests whether this `BLVar` instance represents `BLImageEncoder`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_image_encoder() const noexcept { return _d.is_image_encoder(); }

  //! Tests whether this `BLVar` instance represents a boxed `int64_t` value.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_int64() const noexcept { return _d.is_int64(); }

  //! Tests whether this `BLVar` instance represents a null value.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_null() const noexcept { return _d.is_null(); }

  //! Tests whether this `BLVar` instance represents `BLPath`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_path() const noexcept { return _d.is_path(); }

  //! Tests whether this `BLVar` instance represents `BLPattern.`
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_pattern() const noexcept { return _d.is_pattern(); }

  //! Tests whether this `BLVar` instance represents `BLString`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_string() const noexcept { return _d.is_string(); }

  //! Tests whether this `BLVar` instance represents boxed `BLRgba`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_rgba() const noexcept { return _d.is_rgba(); }

  //! Tests whether this `BLVar` instance represents boxed `BLRgba32`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_rgba32() const noexcept { return _d.is_rgba32(); }

  //! Tests whether this `BLVar` instance represents boxed `BLRgba64`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_rgba64() const noexcept { return _d.is_rgba64(); }

  //! Tests whether this `BLVar` instance represents a boxed `uint64_t` value.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_uint64() const noexcept { return _d.is_uint64(); }

  //! Tests whether this `BLVar` instance is a style that can be used with the rendering context.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_style() const noexcept { return _d.is_style(); }

  //! Converts this value to `bool` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_bool(bool* out) const noexcept { return bl_var_to_bool(this, out); }
  //! Converts this value to `int32_t` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_int32(int32_t* out) const noexcept { return bl_var_to_int32(this, out); }
  //! Converts this value to `int64_t` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_int64(int64_t* out) const noexcept { return bl_var_to_int64(this, out); }
  //! Converts this value to `uint32_t` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_uint32(uint32_t* out) const noexcept { return bl_var_to_uint32(this, out); }
  //! Converts this value to `uint64_t` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_uint64(uint64_t* out) const noexcept { return bl_var_to_uint64(this, out); }
  //! Converts this value to `double` precision floating point and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_double(double* out) const noexcept { return bl_var_to_double(this, out); }
  //! Converts this value to `BLRgba` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_rgba(BLRgba* out) const noexcept { return bl_var_to_rgba(this, out); }
  //! Converts this value to `BLRgba32` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_rgba32(BLRgba32* out) const noexcept { return bl_var_to_rgba32(this, &out->value); }
  //! Converts this value to `BLRgba64` and stores the result in `out`.
  BL_INLINE_NODEBUG BLResult to_rgba64(BLRgba64* out) const noexcept { return bl_var_to_rgba64(this, &out->value); }

  //! \}

  //! \name Properties
  //! \{

  BL_DEFINE_OBJECT_PROPERTY_API

  //! \}

  //! \name Casts
  //! \{

  //! Casts this \ref BLVar instance to `T&`.
  template<typename T>
  BL_INLINE_NODEBUG T& as() noexcept { return *BLInternal::VarCastImpl<T>::cast(this); }

  //! Casts this \ref BLVar instance to `const T&`.
  template<typename T>
  BL_INLINE_NODEBUG const T& as() const noexcept { return *BLInternal::VarCastImpl<T>::cast(this); }

  //! \}

  //! \name Assignment
  //! \{

  template<typename T>
  BL_INLINE_NODEBUG BLResult assign(T&& value) noexcept {
    using DecayT = std::decay_t<T>;
    return BLInternal::VarOps<DecayT, BLInternal::TypeTraits<DecayT>::kCategory>::assign(this, BLInternal::forward<T>(value));
  }

  //! \}

  //! \name Equality
  //! \{

  template<typename T>
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const T& value) const noexcept {
    using DecayT = std::decay_t<T>;
    return BLInternal::VarOps<DecayT, BLInternal::TypeTraits<DecayT>::kCategory>::equals(this, value);
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool strict_equals(const BLVarCore& other) const noexcept { return bl_var_strict_equals(this, &other); }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_VAR_H_INCLUDED
