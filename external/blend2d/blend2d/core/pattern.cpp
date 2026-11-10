// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/pattern_p.h>
#include <blend2d/core/runtime_p.h>

namespace bl {
namespace PatternInternal {

// bl::Pattern - Globals
// =====================

static BLObjectEternalImpl<BLPatternPrivateImpl> default_impl;

// bl::Pattern - Internals
// =======================

static BL_INLINE BLResult alloc_impl(BLPatternCore* self, const BLImageCore* image, const BLRectI& area, BLExtendMode extend_mode, const BLMatrix2D* transform, BLTransformType transform_type) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_PATTERN);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLPatternPrivateImpl>(self, info));

  set_extend_mode(self, extend_mode);
  set_transform_type(self, transform_type);

  BLPatternPrivateImpl* impl = get_impl(self);
  bl_call_ctor(impl->image.dcast(), image->dcast());
  impl->transform = *transform;
  impl->area = area;

  return BL_SUCCESS;
}

BLResult free_impl(BLPatternPrivateImpl* impl) noexcept {
  bl_image_destroy(&impl->image);
  return ObjectInternal::free_impl(impl);
}

static BL_NOINLINE BLResult make_mutable_copy_of(BLPatternCore* self, const BLPatternCore* other) noexcept {
  BLPatternPrivateImpl* other_impl = get_impl(other);

  BLPatternCore newO;
  BL_PROPAGATE(alloc_impl(&newO, &other_impl->image, other_impl->area, get_extend_mode(self), &other_impl->transform, get_transform_type(self)));

  return replace_instance(self, &newO);
}

static BL_INLINE BLResult make_mutable(BLPatternCore* self) noexcept {
  if (!is_impl_mutable(get_impl(self)))
    return make_mutable_copy_of(self, self);
  else
    return BL_SUCCESS;
}

} // {PatternInternal}
} // {bl}

// bl::Pattern - API - Init & Destroy
// ==================================

BL_API_IMPL BLResult bl_pattern_init(BLPatternCore* self) noexcept {
  using namespace bl::PatternInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_PATTERN]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_pattern_init_move(BLPatternCore* self, BLPatternCore* other) noexcept {
  using namespace bl::PatternInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_pattern());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_PATTERN]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_pattern_init_weak(BLPatternCore* self, const BLPatternCore* other) noexcept {
  using namespace bl::PatternInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_pattern());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_pattern_init_as(BLPatternCore* self, const BLImageCore* image, const BLRectI* area, BLExtendMode extend_mode, const BLMatrix2D* transform) noexcept {
  using namespace bl::PatternInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_PATTERN]._d;

  if (!image)
    image = static_cast<BLImageCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE]);

  BLImageImpl* image_impl = bl::ImageInternal::get_impl(image);
  BLRectI image_area(0, 0, image_impl->size.w, image_impl->size.h);

  if (BL_UNLIKELY(extend_mode > BL_EXTEND_MODE_COMPLEX_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (!area)
    area = &image_area;
  else if (*area != image_area && !is_area_valid(*area, image_impl->size))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLTransformType transform_type = BL_TRANSFORM_TYPE_IDENTITY;
  if (!transform)
    transform = &bl::TransformInternal::identity_transform;
  else
    transform_type = transform->type();

  return alloc_impl(self, image, *area, extend_mode, transform, transform_type);
}

BL_API_IMPL BLResult bl_pattern_destroy(BLPatternCore* self) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  return release_instance(self);
}

// bl::Pattern - API - Reset
// =========================

BL_API_IMPL BLResult bl_pattern_reset(BLPatternCore* self) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  return replace_instance(self, static_cast<BLPatternCore*>(&bl_object_defaults[BL_OBJECT_TYPE_PATTERN]));
}

// bl::Pattern - API - Assign
// ==========================

BL_API_IMPL BLResult bl_pattern_assign_move(BLPatternCore* self, BLPatternCore* other) noexcept {
  using namespace bl::PatternInternal;

  BL_ASSERT(self->_d.is_pattern());
  BL_ASSERT(other->_d.is_pattern());

  BLPatternCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_PATTERN]._d;
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_pattern_assign_weak(BLPatternCore* self, const BLPatternCore* other) noexcept {
  using namespace bl::PatternInternal;

  BL_ASSERT(self->_d.is_pattern());
  BL_ASSERT(other->_d.is_pattern());

  retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_pattern_assign_deep(BLPatternCore* self, const BLPatternCore* other) noexcept {
  using namespace bl::PatternInternal;

  BL_ASSERT(self->_d.is_pattern());
  BL_ASSERT(other->_d.is_pattern());

  if (!is_instance_mutable(self))
    return make_mutable_copy_of(self, other);

  BLPatternPrivateImpl* self_impl = get_impl(self);
  BLPatternPrivateImpl* other_impl = get_impl(other);

  self->_d.info.set_b_field(other->_d.info.b_field());
  self->_d.info.set_c_field(other->_d.info.c_field());
  self_impl->transform = other_impl->transform;
  self_impl->area = other_impl->area;
  return bl_image_assign_weak(&self_impl->image, &other_impl->image);
}

// bl::Pattern - API - Create
// ==========================

BL_API_IMPL BLResult bl_pattern_create(BLPatternCore* self, const BLImageCore* image, const BLRectI* area, BLExtendMode extend_mode, const BLMatrix2D* transform) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  if (!image)
    image = static_cast<BLImageCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE]);

  BLImageImpl* image_impl = bl::ImageInternal::get_impl(image);
  BLRectI image_area(0, 0, image_impl->size.w, image_impl->size.h);

  if (BL_UNLIKELY(extend_mode > BL_EXTEND_MODE_COMPLEX_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (!area)
    area = &image_area;
  else if (*area != image_area && !is_area_valid(*area, image_impl->size))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLTransformType transform_type = BL_TRANSFORM_TYPE_IDENTITY;
  if (!transform)
    transform = &bl::TransformInternal::identity_transform;
  else
    transform_type = transform->type();

  if (!is_instance_mutable(self)) {
    BLPatternCore newO;
    BL_PROPAGATE(alloc_impl(&newO, image, *area, extend_mode, transform, transform_type));

    return replace_instance(self, &newO);
  }
  else {
    BLPatternPrivateImpl* self_impl = get_impl(self);

    set_extend_mode(self, extend_mode);
    set_transform_type(self, transform_type);
    self_impl->area = *area;
    self_impl->transform = *transform;

    return bl_image_assign_weak(&self_impl->image, image);
  }
}

// bl::Pattern - API - Image & Area
// ================================

BL_API_IMPL BLResult bl_pattern_get_image(const BLPatternCore* self, BLImageCore* image) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  BLPatternPrivateImpl* self_impl = get_impl(self);
  return bl_image_assign_weak(image, &self_impl->image);
}

BL_API_IMPL BLResult bl_pattern_set_image(BLPatternCore* self, const BLImageCore* image, const BLRectI* area) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  if (!image)
    image = static_cast<BLImageCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE]);

  BLImageImpl* image_impl = bl::ImageInternal::get_impl(image);
  BLRectI image_area(0, 0, image_impl->size.w, image_impl->size.h);

  if (!area)
    area = &image_area;
  else if (*area != image_area && !is_area_valid(*area, image->dcast().size()))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BL_PROPAGATE(make_mutable(self));
  BLPatternPrivateImpl* self_impl = get_impl(self);

  self_impl->area = *area;
  return bl_image_assign_weak(&self_impl->image, image);
}

BL_API_IMPL BLResult bl_pattern_reset_image(BLPatternCore* self) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  return bl_pattern_set_image(self, nullptr, nullptr);
}

BL_API_IMPL BLResult bl_pattern_get_area(const BLPatternCore* self, BLRectI* area_out) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  BLPatternPrivateImpl* self_impl = get_impl(self);
  *area_out = self_impl->area;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_pattern_set_area(BLPatternCore* self, const BLRectI* area) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  BLPatternPrivateImpl* self_impl = get_impl(self);
  BLImageImpl* image_impl = bl::ImageInternal::get_impl(&self_impl->image);

  if (BL_UNLIKELY(!is_area_valid(*area, image_impl->size)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BL_PROPAGATE(make_mutable(self));
  self_impl = get_impl(self);
  self_impl->area = *area;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_pattern_reset_area(BLPatternCore* self) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  BLPatternPrivateImpl* self_impl = get_impl(self);
  BLSizeI size = bl::ImageInternal::get_impl(&self_impl->image)->size;

  if (self_impl->area == BLRectI(0, 0, size.w, size.h))
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self));
  self_impl = get_impl(self);
  self_impl->area.reset(0, 0, size.w, size.h);
  return BL_SUCCESS;
}

// bl::Pattern - API - Extend Mode
// ===============================

BL_API_IMPL BLExtendMode bl_pattern_get_extend_mode(const BLPatternCore* self) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  return get_extend_mode(self);
}

BL_API_IMPL BLResult bl_pattern_set_extend_mode(BLPatternCore* self, BLExtendMode extend_mode) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  if (BL_UNLIKELY(extend_mode > BL_EXTEND_MODE_COMPLEX_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  set_extend_mode(self, extend_mode);
  return BL_SUCCESS;
}

// bl::Pattern - API - Transform
// =============================

BL_API_IMPL BLResult bl_pattern_get_transform(const BLPatternCore* self, BLMatrix2D* transform_out) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  if (get_transform_type(self) == BL_TRANSFORM_TYPE_IDENTITY) {
    transform_out->reset();
  }
  else {
    BLPatternPrivateImpl* self_impl = get_impl(self);
    *transform_out = self_impl->transform;
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLTransformType bl_pattern_get_transform_type(const BLPatternCore* self) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  return get_transform_type(self);
}

BL_API_IMPL BLResult bl_pattern_apply_transform_op(BLPatternCore* self, BLTransformOp op_type, const void* op_data) noexcept {
  using namespace bl::PatternInternal;
  BL_ASSERT(self->_d.is_pattern());

  if (BL_UNLIKELY(uint32_t(op_type) > BL_TRANSFORM_OP_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (op_type == BL_TRANSFORM_OP_RESET && get_transform_type(self) == BL_TRANSFORM_TYPE_IDENTITY)
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self));
  BLPatternPrivateImpl* self_impl = get_impl(self);

  bl_matrix2d_apply_op(&self_impl->transform, op_type, op_data);
  set_transform_type(self, self_impl->transform.type());

  return BL_SUCCESS;
}

// bl::Pattern - API - Equality & Comparison
// =========================================

BL_API_IMPL bool bl_pattern_equals(const BLPatternCore* a, const BLPatternCore* b) noexcept {
  using namespace bl::PatternInternal;

  BL_ASSERT(a->_d.is_pattern());
  BL_ASSERT(b->_d.is_pattern());

  unsigned eq = unsigned(get_extend_mode(a) == get_extend_mode(b)) &
                unsigned(get_transform_type(a) == get_transform_type(b)) ;

  if (!eq)
    return false;

  const BLPatternPrivateImpl* a_impl = get_impl(a);
  const BLPatternPrivateImpl* b_impl = get_impl(b);

  if (a_impl == b_impl)
    return true;

  if (!(unsigned(a_impl->transform == b_impl->transform) & unsigned(a_impl->area == b_impl->area)))
    return false;

  return a_impl->image.dcast() == b_impl->image.dcast();
}

// bl::Pattern - Runtime Registration
// ==================================

void bl_pattern_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  bl_call_ctor(bl::PatternInternal::default_impl.impl->image.dcast());
  bl::PatternInternal::default_impl.impl->transform.reset();

  bl_object_defaults[BL_OBJECT_TYPE_PATTERN]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_PATTERN) | BLObjectInfo::from_abcp(0u, BL_EXTEND_MODE_REPEAT),
    &bl::PatternInternal::default_impl.impl);
}
