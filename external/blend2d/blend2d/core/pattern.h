// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PATTERN_H_INCLUDED
#define BLEND2D_PATTERN_H_INCLUDED

#include <blend2d/core/geometry.h>
#include <blend2d/core/image.h>
#include <blend2d/core/matrix.h>
#include <blend2d/core/object.h>

//! \addtogroup bl_styling
//! \{

//! \name BLPattern - Constants
//! \{

//! Pattern quality.
BL_DEFINE_ENUM(BLPatternQuality) {
  //! Nearest neighbor interpolation.
  BL_PATTERN_QUALITY_NEAREST = 0,
  //! Bilinear interpolation.
  BL_PATTERN_QUALITY_BILINEAR = 1,

  //! Maximum value of `BLPatternQuality`.
  BL_PATTERN_QUALITY_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_PATTERN_QUALITY)
};

//! \}
//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLPattern - C API
//! \{

//! Pattern [C API].
struct BLPatternCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLPattern)
};

//! \cond INTERNAL
//! Pattern [C API Impl].
//!
//! The following properties are stored in BLObjectInfo:
//!
//!   - Pattern extend mode is stored in BLObjectInfo's 'b' field.
//!   - Pattern matrix type is stored in BLObjectInfo's 'c' field.
struct BLPatternImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Image used by the pattern.
  BLImageCore image;

  //! Image area to use.
  BLRectI area;
  //! Pattern transformation matrix.
  BLMatrix2D transform;
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_pattern_init(BLPatternCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_init_move(BLPatternCore* self, BLPatternCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_init_weak(BLPatternCore* self, const BLPatternCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_init_as(BLPatternCore* self, const BLImageCore* image, const BLRectI* area, BLExtendMode extend_mode, const BLMatrix2D* transform) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_destroy(BLPatternCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_reset(BLPatternCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_assign_move(BLPatternCore* self, BLPatternCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_assign_weak(BLPatternCore* self, const BLPatternCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_assign_deep(BLPatternCore* self, const BLPatternCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_create(BLPatternCore* self, const BLImageCore* image, const BLRectI* area, BLExtendMode extend_mode, const BLMatrix2D* transform) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_pattern_get_image(const BLPatternCore* self, BLImageCore* image) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_set_image(BLPatternCore* self, const BLImageCore* image, const BLRectI* area) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_reset_image(BLPatternCore* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_pattern_get_area(const BLPatternCore* self, BLRectI* area_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_set_area(BLPatternCore* self, const BLRectI* area) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pattern_reset_area(BLPatternCore* self) BL_NOEXCEPT_C;

BL_API BLExtendMode BL_CDECL bl_pattern_get_extend_mode(const BLPatternCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_pattern_set_extend_mode(BLPatternCore* self, BLExtendMode extend_mode) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_pattern_get_transform(const BLPatternCore* self, BLMatrix2D* transform_out) BL_NOEXCEPT_C;
BL_API BLTransformType BL_CDECL bl_pattern_get_transform_type(const BLPatternCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_pattern_apply_transform_op(BLPatternCore* self, BLTransformOp op_type, const void* op_data) BL_NOEXCEPT_C;

BL_API bool BL_CDECL bl_pattern_equals(const BLPatternCore* a, const BLPatternCore* b) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_styling
//! \{

//! \name BLPattern - C++ API
//! \{
#ifdef __cplusplus

//! Pattern [C++ API].
class BLPattern final : public BLPatternCore {
public:
  //! \cond INTERNAL

  //! Object info values of a default constructed BLPattern.
  static inline constexpr uint32_t kDefaultSignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_PATTERN) |
    BLObjectInfo::pack_abcp(0u, BL_EXTEND_MODE_REPEAT) | BL_OBJECT_INFO_D_FLAG;

  [[nodiscard]]
  BL_INLINE_NODEBUG BLPatternImpl* _impl() const noexcept { return static_cast<BLPatternImpl*>(_d.impl); }
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE BLPattern() noexcept {
    bl_pattern_init(this);

    // Assume a default constructed BLPattern.
    BL_ASSUME(_d.info.bits == kDefaultSignature);
  }

  BL_INLINE BLPattern(BLPattern&& other) noexcept {
    bl_pattern_init_move(this, &other);

    // Assume a default initialized `other`.
    BL_ASSUME(other._d.info.bits == kDefaultSignature);
  }

  BL_INLINE BLPattern(const BLPattern& other) noexcept {
    bl_pattern_init_weak(this, &other);
  }

  BL_INLINE explicit BLPattern(const BLImage& image, BLExtendMode extend_mode = BL_EXTEND_MODE_REPEAT) noexcept {
    bl_pattern_init_as(this, &image, nullptr, extend_mode, nullptr);
  }

  BL_INLINE BLPattern(const BLImage& image, BLExtendMode extend_mode, const BLMatrix2D& transform) noexcept {
    bl_pattern_init_as(this, &image, nullptr, extend_mode, &transform);
  }

  BL_INLINE BLPattern(const BLImage& image, const BLRectI& area, BLExtendMode extend_mode = BL_EXTEND_MODE_REPEAT) noexcept {
    bl_pattern_init_as(this, &image, &area, extend_mode, nullptr);
  }

  BL_INLINE BLPattern(const BLImage& image, const BLRectI& area, BLExtendMode extend_mode, const BLMatrix2D& transform) noexcept {
    bl_pattern_init_as(this, &image, &area, extend_mode, &transform);
  }

  BL_INLINE ~BLPattern() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_pattern_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE BLPattern& operator=(BLPattern&& other) noexcept { bl_pattern_assign_move(this, &other); return *this; }
  BL_INLINE BLPattern& operator=(const BLPattern& other) noexcept { bl_pattern_assign_weak(this, &other); return *this; }

  [[nodiscard]]
  BL_INLINE bool operator==(const BLPattern& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  BL_INLINE bool operator!=(const BLPattern& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE BLResult reset() noexcept {
    BLResult result = bl_pattern_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLPattern after reset.
    BL_ASSUME(_d.info.bits == kDefaultSignature);

    return result;
  }

  BL_INLINE void swap(BLPattern& other) noexcept { _d.swap(other._d); }

  BL_INLINE BLResult assign(BLPattern&& other) noexcept { return bl_pattern_assign_move(this, &other); }
  BL_INLINE BLResult assign(const BLPattern& other) noexcept { return bl_pattern_assign_weak(this, &other); }

  [[nodiscard]]
  BL_INLINE bool equals(const BLPattern& other) const noexcept { return bl_pattern_equals(this, &other); }

  //! \}

  //! \name Create Pattern
  //! \{

  BL_INLINE BLResult create(const BLImage& image, BLExtendMode extend_mode = BL_EXTEND_MODE_REPEAT) noexcept {
    return bl_pattern_create(this, &image, nullptr, extend_mode, nullptr);
  }

  BL_INLINE BLResult create(const BLImage& image, BLExtendMode extend_mode, const BLMatrix2D& transform) noexcept {
    return bl_pattern_create(this, &image, nullptr, extend_mode, &transform);
  }

  BL_INLINE BLResult create(const BLImage& image, const BLRectI& area, BLExtendMode extend_mode = BL_EXTEND_MODE_REPEAT) noexcept {
    return bl_pattern_create(this, &image, &area, extend_mode, nullptr);
  }

  BL_INLINE BLResult create(const BLImage& image, const BLRectI& area, BLExtendMode extend_mode, const BLMatrix2D& transform) noexcept {
    return bl_pattern_create(this, &image, &area, extend_mode, &transform);
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  BL_INLINE BLImage get_image() const noexcept {
    BLImage image_out;
    bl_pattern_get_image(this, &image_out);
    return image_out;
  }

  [[nodiscard]]
  BL_INLINE BLRectI area() const noexcept {
    BLRectI area_out;
    bl_pattern_get_area(this, &area_out);
    return area_out;
  }

  //! Sets pattern image to `image` and area rectangle to [0, 0, image.width, image.height].
  BL_INLINE_NODEBUG BLResult set_image(const BLImageCore& image) noexcept { return bl_pattern_set_image(this, &image, nullptr); }
  //! Sets pattern image to `image` and area rectangle to `area`.
  BL_INLINE_NODEBUG BLResult set_image(const BLImageCore& image, const BLRectI& area) noexcept { return bl_pattern_set_image(this, &image, &area); }
  //! Resets pattern image to empty image and clears pattern area rectangle to [0, 0, 0, 0].
  BL_INLINE_NODEBUG BLResult reset_image() noexcept { return bl_pattern_reset_image(this); }

  //! Updates the pattern area rectangle to `area`.
  BL_INLINE_NODEBUG BLResult set_area(const BLRectI& area) noexcept { return bl_pattern_set_area(this, &area); }
  //! Updates the pattern area rectangle to [0, 0, image.width, image.height].
  BL_INLINE_NODEBUG BLResult reset_area() noexcept { return bl_pattern_reset_area(this); }

  [[nodiscard]]
  BL_INLINE_NODEBUG BLExtendMode extend_mode() const noexcept {
    return BLExtendMode(_d.b_field());
  }

  BL_INLINE BLResult set_extend_mode(BLExtendMode extend_mode) noexcept {
    if (BL_UNLIKELY(extend_mode > BL_EXTEND_MODE_COMPLEX_MAX_VALUE))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    _d.info.set_b_field(uint32_t(extend_mode));
    return BL_SUCCESS;
  }

  BL_INLINE BLResult reset_extend_mode() noexcept {
    _d.info.set_b_field(uint32_t(BL_EXTEND_MODE_REPEAT));
    return BL_SUCCESS;
  }

  //! \}

  //! \name Transformations
  //! \{

  [[nodiscard]]
  BL_INLINE BLMatrix2D transform() const noexcept {
    BLMatrix2D transform_out;
    bl_pattern_get_transform(this, &transform_out);
    return transform_out;
  }

  [[nodiscard]]
  BL_INLINE BLTransformType transform_type() const noexcept {
    return BLTransformType(_d.c_field());
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_transform() const noexcept {
    return transform_type() != BL_TRANSFORM_TYPE_IDENTITY;
  }

  //! Applies a transformation operation to the pattern's transformation matrix (internal).
  BL_INLINE BLResult _apply_transform_op(BLTransformOp op_type, const void* op_data) noexcept {
    return bl_pattern_apply_transform_op(this, op_type, op_data);
  }

  //! \cond INTERNAL
  //! Applies a transformation operation to the pattern's transformation matrix (internal).
  template<typename... Args>
  BL_INLINE BLResult _apply_transform_op_v(BLTransformOp op_type, Args&&... args) noexcept {
    double op_data[] = { double(args)... };
    return bl_pattern_apply_transform_op(this, op_type, op_data);
  }
  //! \endcond

  BL_INLINE BLResult set_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_ASSIGN, &transform); }
  BL_INLINE BLResult reset_transform() noexcept { return _apply_transform_op(BL_TRANSFORM_OP_RESET, nullptr); }

  BL_INLINE BLResult translate(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_TRANSLATE, x, y); }
  BL_INLINE BLResult translate(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_TRANSLATE, p.x, p.y); }
  BL_INLINE BLResult translate(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_TRANSLATE, &p); }
  BL_INLINE BLResult scale(double xy) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, xy, xy); }
  BL_INLINE BLResult scale(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, x, y); }
  BL_INLINE BLResult scale(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, p.x, p.y); }
  BL_INLINE BLResult scale(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_SCALE, &p); }
  BL_INLINE BLResult skew(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SKEW, x, y); }
  BL_INLINE BLResult skew(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_SKEW, &p); }
  BL_INLINE BLResult rotate(double angle) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_ROTATE, &angle); }
  BL_INLINE BLResult rotate(double angle, double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, x, y); }
  BL_INLINE BLResult rotate(double angle, const BLPoint& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE BLResult rotate(double angle, const BLPointI& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE BLResult apply_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_TRANSFORM, &transform); }

  BL_INLINE BLResult post_translate(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_TRANSLATE, x, y); }
  BL_INLINE BLResult post_translate(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_TRANSLATE, p.x, p.y); }
  BL_INLINE BLResult post_translate(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_TRANSLATE, &p); }
  BL_INLINE BLResult post_scale(double xy) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, xy, xy); }
  BL_INLINE BLResult post_scale(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, x, y); }
  BL_INLINE BLResult post_scale(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, p.x, p.y); }
  BL_INLINE BLResult post_scale(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_SCALE, &p); }
  BL_INLINE BLResult post_skew(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SKEW, x, y); }
  BL_INLINE BLResult post_skew(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_SKEW, &p); }
  BL_INLINE BLResult post_rotate(double angle) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_ROTATE, &angle); }
  BL_INLINE BLResult post_rotate(double angle, double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, x, y); }
  BL_INLINE BLResult post_rotate(double angle, const BLPoint& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE BLResult post_rotate(double angle, const BLPointI& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, origin.x, origin.y); }
  BL_INLINE BLResult post_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_TRANSFORM, &transform); }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_PATTERN_H_INCLUDED
