// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_GRADIENT_H_INCLUDED
#define BLEND2D_GRADIENT_H_INCLUDED

#include <blend2d/core/geometry.h>
#include <blend2d/core/matrix.h>
#include <blend2d/core/object.h>
#include <blend2d/core/rgba.h>

//! \addtogroup bl_styling
//! \{

//! \name BLGradient - Constants
//! \{

//! Gradient type.
BL_DEFINE_ENUM(BLGradientType) {
  //! Linear gradient type.
  BL_GRADIENT_TYPE_LINEAR = 0,
  //! Radial gradient type.
  BL_GRADIENT_TYPE_RADIAL = 1,
  //! Conic gradient type.
  BL_GRADIENT_TYPE_CONIC = 2,

  //! Maximum value of `BLGradientType`.
  BL_GRADIENT_TYPE_MAX_VALUE = 2

  BL_FORCE_ENUM_UINT32(BL_GRADIENT_TYPE)
};

//! Gradient data index.
BL_DEFINE_ENUM(BLGradientValue) {
  //! x0 - start 'x' for a Linear gradient and `x` center for both Radial and Conic gradients.
  BL_GRADIENT_VALUE_COMMON_X0 = 0,
  //! y0 - start 'y' for a Linear gradient and `y` center for both Radial and Conic gradients.
  BL_GRADIENT_VALUE_COMMON_Y0 = 1,
  //! x1 - end 'x' for a Linear gradient and focal point `x` for a Radial gradient.
  BL_GRADIENT_VALUE_COMMON_X1 = 2,
  //! y1 - end 'y' for a Linear/gradient and focal point `y` for a Radial gradient.
  BL_GRADIENT_VALUE_COMMON_Y1 = 3,
  //! Radial gradient center radius.
  BL_GRADIENT_VALUE_RADIAL_R0 = 4,
  //! Radial gradient focal radius.
  BL_GRADIENT_VALUE_RADIAL_R1 = 5,
  //! Conic gradient angle.
  BL_GRADIENT_VALUE_CONIC_ANGLE = 2,
  //! Conic gradient angle.
  BL_GRADIENT_VALUE_CONIC_REPEAT = 3,

  //! Maximum value of `BLGradientValue`.
  BL_GRADIENT_VALUE_MAX_VALUE = 5

  BL_FORCE_ENUM_UINT32(BL_GRADIENT_VALUE)
};

//! Gradient rendering quality.
BL_DEFINE_ENUM(BLGradientQuality) {
  //! Nearest neighbor.
  BL_GRADIENT_QUALITY_NEAREST = 0,

  //! Use smoothing, if available (currently never available).
  BL_GRADIENT_QUALITY_SMOOTH = 1,

  //! The renderer will use an implementation-specific dithering algorithm to prevent banding.
  BL_GRADIENT_QUALITY_DITHER = 2,

  //! Maximum value of `BLGradientQuality`.
  BL_GRADIENT_QUALITY_MAX_VALUE = 2

  BL_FORCE_ENUM_UINT32(BL_GRADIENT_QUALITY)
};

//! \}

//! \name BLGradient - Structs
//! \{

//! Defines an `offset` and `rgba` color that us used by \ref BLGradient to define a linear transition between colors.
struct BLGradientStop {
  double offset;
  BLRgba64 rgba;

#ifdef __cplusplus
  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLGradientStop() noexcept = default;
  BL_INLINE_NODEBUG BLGradientStop(const BLGradientStop& other) noexcept = default;

  BL_INLINE_NODEBUG BLGradientStop(double offset_value, const BLRgba32& rgba32_value) noexcept
    : offset(offset_value),
      rgba(rgba32_value) {}

  BL_INLINE_NODEBUG BLGradientStop(double offset_value, const BLRgba64& rgba64_value) noexcept
    : offset(offset_value),
      rgba(rgba64_value) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG BLGradientStop& operator=(const BLGradientStop& other) noexcept = default;

  BL_INLINE_NODEBUG bool operator==(const BLGradientStop& other) const noexcept { return  equals(other); }
  BL_INLINE_NODEBUG bool operator!=(const BLGradientStop& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept {
    offset = 0.0;
    rgba.reset();
  }

  BL_INLINE_NODEBUG void reset(double offset_value, const BLRgba32& rgba32_value) noexcept {
    offset = offset_value;
    rgba.reset(rgba32_value);
  }

  BL_INLINE_NODEBUG void reset(double offset_value, const BLRgba64& rgba64_value) noexcept {
    offset = offset_value;
    rgba.reset(rgba64_value);
  }

  BL_INLINE_NODEBUG bool equals(const BLGradientStop& other) const noexcept {
    return BLInternal::bool_and(bl_equals(offset, other.offset),
                                bl_equals(rgba  , other.rgba  ));
  }

  //! \}
#endif
};

//! Linear gradient values packed into a structure.
struct BLLinearGradientValues {
  double x0;
  double y0;
  double x1;
  double y1;

#ifdef __cplusplus
  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLLinearGradientValues() noexcept = default;
  BL_INLINE_NODEBUG BLLinearGradientValues(const BLLinearGradientValues& other) noexcept = default;
  BL_INLINE_NODEBUG BLLinearGradientValues& operator=(const BLLinearGradientValues& other) noexcept = default;

  BL_INLINE_NODEBUG BLLinearGradientValues(double x0_value, double y0_value, double x1_value, double y1_value) noexcept
    : x0(x0_value),
      y0(y0_value),
      x1(x1_value),
      y1(y1_value) {}

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLLinearGradientValues{}; }

  //! \}
#endif
};

//! Radial gradient values packed into a structure.
struct BLRadialGradientValues {
  double x0;
  double y0;
  double x1;
  double y1;
  double r0;
  double r1;

#ifdef __cplusplus
  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLRadialGradientValues() noexcept = default;
  BL_INLINE_NODEBUG BLRadialGradientValues(const BLRadialGradientValues& other) noexcept = default;
  BL_INLINE_NODEBUG BLRadialGradientValues& operator=(const BLRadialGradientValues& other) noexcept = default;

  BL_INLINE_NODEBUG BLRadialGradientValues(double x0_value, double y0_value, double x1_value, double y1_value, double r0_value, double r1_value = 0.0) noexcept
    : x0(x0_value),
      y0(y0_value),
      x1(x1_value),
      y1(y1_value),
      r0(r0_value),
      r1(r1_value) {}

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLRadialGradientValues{}; }

  //! \}
#endif
};

//! Conic gradient values packed into a structure.
struct BLConicGradientValues {
  double x0;
  double y0;
  double angle;
  double repeat;

#ifdef __cplusplus
  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLConicGradientValues() noexcept = default;
  BL_INLINE_NODEBUG BLConicGradientValues(const BLConicGradientValues& other) noexcept = default;
  BL_INLINE_NODEBUG BLConicGradientValues& operator=(const BLConicGradientValues& other) noexcept = default;

  BL_INLINE_NODEBUG BLConicGradientValues(double x0_value, double y0_value, double angle_value, double repeat_value = 1.0) noexcept
    : x0(x0_value),
      y0(y0_value),
      angle(angle_value),
      repeat(repeat_value) {}

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLConicGradientValues{}; }

  //! \}
#endif
};

//! \}
//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLGradient - C API
//! \{

//! Gradient [C API].
struct BLGradientCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLGradient)
};

//! \cond INTERNAL
//! Gradient [C API Impl].
struct BLGradientImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Gradient stop data.
  BLGradientStop* stops;
  //! Gradient stop count.
  size_t size;
  //! Stop capacity.
  size_t capacity;

  //! Gradient transformation matrix.
  BLMatrix2D transform;

  union {
    //! Gradient values (coordinates, radius, angle).
    double values[BL_GRADIENT_VALUE_MAX_VALUE + 1];
    //! Linear parameters.
    BLLinearGradientValues linear;
    //! Radial parameters.
    BLRadialGradientValues radial;
    //! Conic parameters.
    BLConicGradientValues conic;
  };
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_gradient_init(BLGradientCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_init_move(BLGradientCore* self, BLGradientCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_init_weak(BLGradientCore* self, const BLGradientCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_init_as(BLGradientCore* self, BLGradientType type, const void* values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D* transform) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_destroy(BLGradientCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_reset(BLGradientCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_assign_move(BLGradientCore* self, BLGradientCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_assign_weak(BLGradientCore* self, const BLGradientCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_create(BLGradientCore* self, BLGradientType type, const void* values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D* transform) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_shrink(BLGradientCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_reserve(BLGradientCore* self, size_t n) BL_NOEXCEPT_C;
BL_API BLGradientType BL_CDECL bl_gradient_get_type(const BLGradientCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_gradient_set_type(BLGradientCore* self, BLGradientType type) BL_NOEXCEPT_C;
BL_API BLExtendMode BL_CDECL bl_gradient_get_extend_mode(const BLGradientCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_gradient_set_extend_mode(BLGradientCore* self, BLExtendMode extend_mode) BL_NOEXCEPT_C;
BL_API double BL_CDECL bl_gradient_get_value(const BLGradientCore* self, size_t index) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_gradient_set_value(BLGradientCore* self, size_t index, double value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_set_values(BLGradientCore* self, size_t index, const double* values, size_t n) BL_NOEXCEPT_C;
BL_API size_t BL_CDECL bl_gradient_get_size(const BLGradientCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API size_t BL_CDECL bl_gradient_get_capacity(const BLGradientCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API const BLGradientStop* BL_CDECL bl_gradient_get_stops(const BLGradientCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_gradient_reset_stops(BLGradientCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_assign_stops(BLGradientCore* self, const BLGradientStop* stops, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_add_stop_rgba32(BLGradientCore* self, double offset, uint32_t argb32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_add_stop_rgba64(BLGradientCore* self, double offset, uint64_t argb64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_remove_stop(BLGradientCore* self, size_t index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_remove_stop_by_offset(BLGradientCore* self, double offset, uint32_t all) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_remove_stops_by_index(BLGradientCore* self, size_t r_start, size_t r_end) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_remove_stops_by_offset(BLGradientCore* self, double offset_min, double offset_max) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_replace_stop_rgba32(BLGradientCore* self, size_t index, double offset, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_replace_stop_rgba64(BLGradientCore* self, size_t index, double offset, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API size_t BL_CDECL bl_gradient_index_of_stop(const BLGradientCore* self, double offset) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_gradient_get_transform(const BLGradientCore* self, BLMatrix2D* transform_out) BL_NOEXCEPT_C;
BL_API BLTransformType BL_CDECL bl_gradient_get_transform_type(const BLGradientCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_gradient_apply_transform_op(BLGradientCore* self, BLTransformOp op_type, const void* op_data) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_gradient_equals(const BLGradientCore* a, const BLGradientCore* b) BL_NOEXCEPT_C BL_PURE;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_styling
//! \{

//! \name BLGradient - C++ API
//! \{
#ifdef __cplusplus

//! Gradient [C++ API].
class BLGradient final : public BLGradientCore {
public:
  //! \cond INTERNAL

  //! Object info values of a default constructed BLGradient.
  static inline constexpr uint32_t kDefaultSignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_GRADIENT) | BL_OBJECT_INFO_D_FLAG;

  [[nodiscard]]
  BL_INLINE_NODEBUG BLGradientImpl* _impl() const noexcept { return static_cast<BLGradientImpl*>(_d.impl); }
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed gradient.
  //!
  //! A default constructed gradient has \ref BL_GRADIENT_TYPE_LINEAR type, all values set to zero, and has no color
  //! stops.
  BL_INLINE BLGradient() noexcept {
    bl_gradient_init(this);

    // Assume a default constructed BLGradient.
    BL_ASSUME(_d.info.bits == kDefaultSignature);
  }

  //! Move constructor.
  //!
  //! The `other` gradient is reset to its construction state after the move.
  BL_INLINE BLGradient(BLGradient&& other) noexcept {
    bl_gradient_init_move(this, &other);

    // Assume a default initialized `other`.
    BL_ASSUME(other._d.info.bits == kDefaultSignature);
  }

  //! Copy constructor creates a weak copy of `other`.
  BL_INLINE BLGradient(const BLGradient& other) noexcept {
    bl_gradient_init_weak(this, &other);
  }

  BL_INLINE explicit BLGradient(BLGradientType type, const double* values = nullptr) noexcept {
    bl_gradient_init_as(this, type, values, BL_EXTEND_MODE_PAD, nullptr, 0, nullptr);
  }

  BL_INLINE explicit BLGradient(const BLLinearGradientValues& values, BLExtendMode extend_mode = BL_EXTEND_MODE_PAD) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_LINEAR, &values, extend_mode, nullptr, 0, nullptr);
  }

  BL_INLINE explicit BLGradient(const BLRadialGradientValues& values, BLExtendMode extend_mode = BL_EXTEND_MODE_PAD) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_RADIAL, &values, extend_mode, nullptr, 0, nullptr);
  }

  BL_INLINE explicit BLGradient(const BLConicGradientValues& values, BLExtendMode extend_mode = BL_EXTEND_MODE_PAD) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_CONIC, &values, extend_mode, nullptr, 0, nullptr);
  }

  BL_INLINE BLGradient(const BLLinearGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_LINEAR, &values, extend_mode, stops, n, nullptr);
  }

  BL_INLINE BLGradient(const BLRadialGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_RADIAL, &values, extend_mode, stops, n, nullptr);
  }

  BL_INLINE BLGradient(const BLConicGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_CONIC, &values, extend_mode, stops, n, nullptr);
  }

  BL_INLINE BLGradient(const BLLinearGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D& transform) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_LINEAR, &values, extend_mode, stops, n, &transform);
  }

  BL_INLINE BLGradient(const BLRadialGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D& transform) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_RADIAL, &values, extend_mode, stops, n, &transform);
  }

  BL_INLINE BLGradient(const BLConicGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D& transform) noexcept {
    bl_gradient_init_as(this, BL_GRADIENT_TYPE_CONIC, &values, extend_mode, stops, n, &transform);
  }

  BL_INLINE ~BLGradient() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_gradient_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Move assignment operator, does the same as `assign(other)`.
  BL_INLINE BLGradient& operator=(BLGradient&& other) noexcept { bl_gradient_assign_move(this, &other); return *this; }

  //! Copy assignment operator, does the same as `assign(other)`.
  BL_INLINE BLGradient& operator=(const BLGradient& other) noexcept { bl_gradient_assign_weak(this, &other); return *this; }

  //! Equality operator, performs the same operation as `equals(other)`.
  [[nodiscard]]
  BL_INLINE bool operator==(const BLGradient& other) const noexcept { return  bl_gradient_equals(this, &other); }

  //! Inequality operator, performs the same operation as `!equals(other)`.
  [[nodiscard]]
  BL_INLINE bool operator!=(const BLGradient& other) const noexcept { return !bl_gradient_equals(this, &other); }

  //! \}

  //! \name Common Functionality
  //! \{

  //! Resets the gradient to its construction state.
  //!
  //! \note This operation always succeeds and returns \ref BL_SUCCESS. The return value is provided for convenience
  //! so `reset()` can be used in tail calls in case other functions need to return \ref BLResult.
  BL_INLINE BLResult reset() noexcept {
    BLResult result = bl_gradient_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLGradient after reset.
    BL_ASSUME(_d.info.bits == kDefaultSignature);

    return result;
  }

  //! Swaps this gradient with `other`.
  //!
  //! \note This operation always succeeds.
  BL_INLINE void swap(BLGradient& other) noexcept { _d.swap(other._d); }

  //! \}

  //! \name Create Gradient
  //! \{

  BL_INLINE BLResult create(const BLLinearGradientValues& values, BLExtendMode extend_mode = BL_EXTEND_MODE_PAD) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_LINEAR, &values, extend_mode, nullptr, 0, nullptr);
  }

  BL_INLINE BLResult create(const BLRadialGradientValues& values, BLExtendMode extend_mode = BL_EXTEND_MODE_PAD) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_RADIAL, &values, extend_mode, nullptr, 0, nullptr);
  }

  BL_INLINE BLResult create(const BLConicGradientValues& values, BLExtendMode extend_mode = BL_EXTEND_MODE_PAD) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_CONIC, &values, extend_mode, nullptr, 0, nullptr);
  }

  BL_INLINE BLResult create(const BLLinearGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_LINEAR, &values, extend_mode, stops, n, nullptr);
  }

  BL_INLINE BLResult create(const BLRadialGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_RADIAL, &values, extend_mode, stops, n, nullptr);
  }

  BL_INLINE BLResult create(const BLConicGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_CONIC, &values, extend_mode, stops, n, nullptr);
  }

  BL_INLINE BLResult create(const BLLinearGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D& transform) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_LINEAR, &values, extend_mode, stops, n, &transform);
  }

  BL_INLINE BLResult create(const BLRadialGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D& transform) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_RADIAL, &values, extend_mode, stops, n, &transform);
  }

  BL_INLINE BLResult create(const BLConicGradientValues& values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D& transform) noexcept {
    return bl_gradient_create(this, BL_GRADIENT_TYPE_CONIC, &values, extend_mode, stops, n, &transform);
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the type of the gradient.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLGradientType type() const noexcept { return BLGradientType(_d.info.a_field()); }

  //! Sets the type of the gradient.
  BL_INLINE BLResult set_type(BLGradientType type) noexcept { return bl_gradient_set_type(this, type); }

  //! Returns the gradient extend mode, see \ref BLExtendMode.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLExtendMode extend_mode() const noexcept { return BLExtendMode(_d.info.b_field()); }

  //! Set the gradient extend mode, see \ref BLExtendMode.
  BL_INLINE BLResult set_extend_mode(BLExtendMode extend_mode) noexcept { return bl_gradient_set_extend_mode(this, extend_mode); }
  //! Resets the gradient extend mode to \ref BL_EXTEND_MODE_PAD.
  BL_INLINE BLResult reset_extend_mode() noexcept { return bl_gradient_set_extend_mode(this, BL_EXTEND_MODE_PAD); }

  [[nodiscard]]
  BL_INLINE double value(size_t index) const noexcept {
    BL_ASSERT(index <= BL_GRADIENT_VALUE_MAX_VALUE);
    return _impl()->values[index];
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG const BLLinearGradientValues& linear() const noexcept { return _impl()->linear; }

  [[nodiscard]]
  BL_INLINE_NODEBUG const BLRadialGradientValues& radial() const noexcept { return _impl()->radial; }

  [[nodiscard]]
  BL_INLINE_NODEBUG const BLConicGradientValues& conic() const noexcept { return _impl()->conic; }

  BL_INLINE BLResult set_value(size_t index, double value) noexcept { return bl_gradient_set_value(this, index, value); }
  BL_INLINE BLResult set_values(size_t index, const double* values, size_t n) noexcept { return bl_gradient_set_values(this, index, values, n); }

  BL_INLINE BLResult set_values(const BLLinearGradientValues& values) noexcept { return set_values(0, (const double*)&values, sizeof(BLLinearGradientValues) / sizeof(double)); }
  BL_INLINE BLResult set_values(const BLRadialGradientValues& values) noexcept { return set_values(0, (const double*)&values, sizeof(BLRadialGradientValues) / sizeof(double)); }
  BL_INLINE BLResult set_values(const BLConicGradientValues& values) noexcept { return set_values(0, (const double*)&values, sizeof(BLConicGradientValues) / sizeof(double)); }

  [[nodiscard]]
  BL_INLINE_NODEBUG double x0() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_COMMON_X0]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double y0() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_COMMON_Y0]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double x1() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_COMMON_X1]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double y1() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_COMMON_Y1]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double r0() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_RADIAL_R0]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double r1() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_RADIAL_R1]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double angle() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_CONIC_ANGLE]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double conic_angle() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_CONIC_ANGLE]; }

  [[nodiscard]]
  BL_INLINE_NODEBUG double conic_repeat() const noexcept { return _impl()->values[BL_GRADIENT_VALUE_CONIC_REPEAT]; }

  BL_INLINE BLResult set_x0(double value) noexcept { return set_value(BL_GRADIENT_VALUE_COMMON_X0, value); }
  BL_INLINE BLResult set_y0(double value) noexcept { return set_value(BL_GRADIENT_VALUE_COMMON_Y0, value); }
  BL_INLINE BLResult set_x1(double value) noexcept { return set_value(BL_GRADIENT_VALUE_COMMON_X1, value); }
  BL_INLINE BLResult set_y1(double value) noexcept { return set_value(BL_GRADIENT_VALUE_COMMON_Y1, value); }
  BL_INLINE BLResult set_r0(double value) noexcept { return set_value(BL_GRADIENT_VALUE_RADIAL_R0, value); }
  BL_INLINE BLResult set_r1(double value) noexcept { return set_value(BL_GRADIENT_VALUE_RADIAL_R1, value); }
  BL_INLINE BLResult set_angle(double value) noexcept { return set_value(BL_GRADIENT_VALUE_CONIC_ANGLE, value); }
  BL_INLINE BLResult set_conic_angle(double value) noexcept { return set_value(BL_GRADIENT_VALUE_CONIC_ANGLE, value); }
  BL_INLINE BLResult set_conic_repeat(double value) noexcept { return set_value(BL_GRADIENT_VALUE_CONIC_REPEAT, value); }

  //! \}

  //! \name Gradient Stops
  //! \{

  //! Tests whether the gradient is empty.
  //!
  //! Empty gradient is considered any gradient that has no stops.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return _impl()->size == 0; }

  //! Returns the number of stops the gradient has.
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t size() const noexcept { return _impl()->size; }

  //! Returns the gradient capacity [in stops].
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return _impl()->capacity; }

  //! Reserves the capacity of gradient for at least `n` stops.
  BL_INLINE_NODEBUG BLResult reserve(size_t n) noexcept { return bl_gradient_reserve(this, n); }
  //! Shrinks the capacity of gradient stops to fit the current use.
  BL_INLINE_NODEBUG BLResult shrink() noexcept { return bl_gradient_shrink(this); }

  //! Returns the gradient stop data.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLGradientStop* stops() const noexcept { return _impl()->stops; }

  //! Returns a gradient stop at `i`.
  [[nodiscard]]
  BL_INLINE const BLGradientStop& stop_at(size_t i) const noexcept {
    BL_ASSERT(i < size());
    return _impl()->stops[i];
  }

  //! Returns gradient stops and their count as \ref BLArrayView<BLGradientStop>.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLArrayView<BLGradientStop> stops_view() const noexcept { return BLArrayView<BLGradientStop>{_impl()->stops, _impl()->size}; }

  //! \}

  //! \name Content Manipulation
  //! \{

  //! Move assignment of `other` gradient to this gradient.
  //!
  //! This function resets `other` to its initialization state.
  BL_INLINE_NODEBUG BLResult assign(BLGradient&& other) noexcept { return bl_gradient_assign_move(this, &other); }

  //! Copy assignment of `other` gradient to this gradient.
  //!
  //! This function creates a weak copy of `other` gradient by increasing its reference count if `other` is reference
  //! counted.
  BL_INLINE_NODEBUG BLResult assign(const BLGradient& other) noexcept { return bl_gradient_assign_weak(this, &other); }

  //! Resets all stops of the gradient.
  //!
  //! After the operation the gradient will have no color stops.
  BL_INLINE_NODEBUG BLResult reset_stops() noexcept { return bl_gradient_reset_stops(this); }

  //! Assigns colors stops of the gradient to `stops` of size `n`.
  BL_INLINE_NODEBUG BLResult assign_stops(const BLGradientStop* stops, size_t n) noexcept { return bl_gradient_assign_stops(this, stops, n); }

  //! Assigns colors stops of the gradient to `stops`.
  BL_INLINE_NODEBUG BLResult assign_stops(BLArrayView<BLGradientStop> stops) noexcept { return bl_gradient_assign_stops(this, stops.data, stops.size); }

  //! Adds a color stop described as a 32-bit color `rgba32` at the given `offset`.
  //!
  //! \note The offset value must be in `[0, 1]` range.
  BL_INLINE_NODEBUG BLResult add_stop(double offset, const BLRgba32& rgba32) noexcept { return bl_gradient_add_stop_rgba32(this, offset, rgba32.value); }

  //! Adds a color stop described as a 64-bit color `rgba64` at the given `offset`.
  //!
  //! \note The offset value must be in `[0, 1]` range.
  BL_INLINE_NODEBUG BLResult add_stop(double offset, const BLRgba64& rgba64) noexcept { return bl_gradient_add_stop_rgba64(this, offset, rgba64.value); }

  //! Removes stop at the given `index`.
  //!
  //! \note This function should be used together with `index_of_stop()`, which returns index to the stop array.
  BL_INLINE_NODEBUG BLResult remove_stop(size_t index) noexcept { return bl_gradient_remove_stop(this, index); }

  //! Removes stop at the given `offset`, which should be in `[0, 1]` range.
  //!
  //! The `all` parameter specifies whether all stops at the given offset should be removed as there are cases in
  //! which two stops can occupy the same offset to create sharp transitions. If `all` is false and there is a sharp
  //! transition only the first stop would be removed. If `all` is true both stops will be removed.
  //!
  //! \note There are never 3 stops occupying the same `offset`.
  BL_INLINE_NODEBUG BLResult remove_stop_by_offset(double offset, bool all = true) noexcept { return bl_gradient_remove_stop_by_offset(this, offset, all); }

  //! Removes all stops in the given range, which describes indexes in the stop array.
  BL_INLINE_NODEBUG BLResult remove_stops(BLRange range) noexcept { return bl_gradient_remove_stops_by_index(this, range.start, range.end); }

  //! Removes all stops in the given interval `[offset_min, offset_max]`, which specifies stop offsets, which are
  //! between [0, 1].
  BL_INLINE_NODEBUG BLResult remove_stops_by_offset(double offset_min, double offset_max) noexcept { return bl_gradient_remove_stops_by_offset(this, offset_min, offset_max); }

  //! Replaces stop at the given `index` with a new color stop described by `offset` and `rgba32`.
  //!
  //! The operation leads to the same result as `remove_stop(index)` followed by `add_stop(offset, rgba32)`.
  BL_INLINE_NODEBUG BLResult replace_stop(size_t index, double offset, const BLRgba32& rgba32) noexcept { return bl_gradient_replace_stop_rgba32(this, index, offset, rgba32.value); }

  //! Replaces stop at the given `index` with a new color stop described by `offset` and `rgba64`.
  //!
  //! The operation leads to the same result as `remove_stop(index)` followed by `add_stop(offset, rgba64)`.
  BL_INLINE_NODEBUG BLResult replace_stop(size_t index, double offset, const BLRgba64& rgba64) noexcept { return bl_gradient_replace_stop_rgba64(this, index, offset, rgba64.value); }

  //! Returns the index of a color stop in stops[] array of the given `offset`.
  //!
  //! \note If there is no such offset, `SIZE_MAX` is returned.
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t index_of_stop(double offset) const noexcept { return bl_gradient_index_of_stop(this, offset) ;}

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Tests whether the gradient equals `other`.
  //!
  //! \note The equality check returns true if both gradients are the same value-wise.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLGradient& other) const noexcept { return bl_gradient_equals(this, &other); }

  //! \}

  //! \name Transformations
  //! \{

  //! Returns the transformation matrix applied to the gradient.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLMatrix2D& transform() const noexcept { return _impl()->transform; }

  //! Returns the type of the transformation matrix returned by `transform()`.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLTransformType transform_type() const noexcept { return BLTransformType(_d.info.c_field()); }

  //! Tests whether the gradient has a non-identity transformation matrix.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_transform() const noexcept { return transform_type() != BL_TRANSFORM_TYPE_IDENTITY; }

  //! Applies a matrix operation to the current transformation matrix (internal).
  BL_INLINE_NODEBUG BLResult _apply_transform_op(BLTransformOp op_type, const void* op_data) noexcept {
    return bl_gradient_apply_transform_op(this, op_type, op_data);
  }

  //! \cond INTERNAL
  //! Applies a matrix operation to the current transformation matrix (internal).
  template<typename... Args>
  BL_INLINE BLResult _apply_transform_op_v(BLTransformOp op_type, Args&&... args) noexcept {
    double op_data[] = { double(args)... };
    return bl_gradient_apply_transform_op(this, op_type, op_data);
  }
  //! \endcond

  BL_INLINE_NODEBUG BLResult set_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_ASSIGN, &transform); }
  BL_INLINE_NODEBUG BLResult reset_transform() noexcept { return _apply_transform_op(BL_TRANSFORM_OP_RESET, nullptr); }

  BL_INLINE_NODEBUG BLResult translate(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_TRANSLATE, x, y); }
  BL_INLINE_NODEBUG BLResult translate(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_TRANSLATE, p.x, p.y); }
  BL_INLINE_NODEBUG BLResult translate(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_TRANSLATE, &p); }
  BL_INLINE_NODEBUG BLResult scale(double xy) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, xy, xy); }
  BL_INLINE_NODEBUG BLResult scale(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, x, y); }
  BL_INLINE_NODEBUG BLResult scale(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, p.x, p.y); }
  BL_INLINE_NODEBUG BLResult scale(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_SCALE, &p); }
  BL_INLINE_NODEBUG BLResult skew(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SKEW, x, y); }
  BL_INLINE_NODEBUG BLResult skew(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_SKEW, &p); }
  BL_INLINE_NODEBUG BLResult rotate(double angle) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_ROTATE, &angle); }
  BL_INLINE_NODEBUG BLResult rotate(double angle, double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, x, y); }
  BL_INLINE_NODEBUG BLResult rotate(double angle, const BLPoint& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE_NODEBUG BLResult rotate(double angle, const BLPointI& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE_NODEBUG BLResult apply_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_TRANSFORM, &transform); }

  BL_INLINE_NODEBUG BLResult post_translate(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_TRANSLATE, x, y); }
  BL_INLINE_NODEBUG BLResult post_translate(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_TRANSLATE, p.x, p.y); }
  BL_INLINE_NODEBUG BLResult post_translate(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_TRANSLATE, &p); }
  BL_INLINE_NODEBUG BLResult post_scale(double xy) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, xy, xy); }
  BL_INLINE_NODEBUG BLResult post_scale(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, x, y); }
  BL_INLINE_NODEBUG BLResult post_scale(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, p.x, p.y); }
  BL_INLINE_NODEBUG BLResult post_scale(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_SCALE, &p); }
  BL_INLINE_NODEBUG BLResult post_skew(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SKEW, x, y); }
  BL_INLINE_NODEBUG BLResult post_skew(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_SKEW, &p); }
  BL_INLINE_NODEBUG BLResult post_rotate(double angle) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_ROTATE, &angle); }
  BL_INLINE_NODEBUG BLResult post_rotate(double angle, double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, x, y); }
  BL_INLINE_NODEBUG BLResult post_rotate(double angle, const BLPoint& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE_NODEBUG BLResult post_rotate(double angle, const BLPointI& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE_NODEBUG BLResult post_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_TRANSFORM, &transform); }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_GRADIENT_H_INCLUDED
