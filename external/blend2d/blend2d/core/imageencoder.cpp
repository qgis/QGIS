// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/imageencoder.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>

// bl::ImageEncoder - Globals
// ==========================

namespace bl {
namespace ImageEncoderInternal {

static BLObjectEternalVirtualImpl<BLImageEncoderImpl, BLImageEncoderVirt> default_encoder;

} // {ImageEncoderInternal}
} // {bl}

// bl::ImageEncoder - API - Init & Destroy
// =======================================

BL_API_IMPL BLResult bl_image_encoder_init(BLImageEncoderCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_ENCODER]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_encoder_init_move(BLImageEncoderCore* self, BLImageEncoderCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image_encoder());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_ENCODER]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_encoder_init_weak(BLImageEncoderCore* self, const BLImageEncoderCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image_encoder());

  return bl_object_private_init_weak_tagged(self, other);
}

BL_API_IMPL BLResult bl_image_encoder_destroy(BLImageEncoderCore* self) noexcept {
  BL_ASSERT(self->_d.is_image_encoder());

  return bl::ObjectInternal::release_virtual_instance(self);
}

// bl::ImageEncoder - API - Reset
// ==============================

BL_API_IMPL BLResult bl_image_encoder_reset(BLImageEncoderCore* self) noexcept {
  BL_ASSERT(self->_d.is_image_encoder());

  return bl::ObjectInternal::replace_virtual_instance(self, static_cast<BLImageEncoderCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE_ENCODER]));
}

// bl::ImageEncoder - API - Assign
// ===============================

BL_API_IMPL BLResult bl_image_encoder_assign_move(BLImageEncoderCore* self, BLImageEncoderCore* other) noexcept {
  BL_ASSERT(self->_d.is_image_encoder());
  BL_ASSERT(other->_d.is_image_encoder());

  BLImageEncoderCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_ENCODER]._d;
  return bl::ObjectInternal::replace_virtual_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_image_encoder_assign_weak(BLImageEncoderCore* self, const BLImageEncoderCore* other) noexcept {
  BL_ASSERT(self->_d.is_image_encoder());
  BL_ASSERT(other->_d.is_image_encoder());

  return bl::ObjectInternal::assign_virtual_instance(self, other);
}

// bl::ImageEncoder - API - Interface
// ==================================

BL_API_IMPL BLResult bl_image_encoder_restart(BLImageEncoderCore* self) noexcept {
  BL_ASSERT(self->_d.is_image_encoder());
  BLImageEncoderImpl* self_impl = self->_impl();

  return self_impl->virt->restart(self_impl);
}

BL_API_IMPL BLResult bl_image_encoder_write_frame(BLImageEncoderCore* self, BLArrayCore* dst, const BLImageCore* src) noexcept {
  BL_ASSERT(self->_d.is_image_encoder());
  BLImageEncoderImpl* self_impl = self->_impl();

  return self_impl->virt->write_frame(self_impl, dst, src);
}

// bl::ImageEncoder - Virtual Functions (Null)
// ===========================================

static BLResult BL_CDECL bl_image_encoder_impl_destroy(BLObjectImpl* impl) noexcept {
  bl_unused(impl);
  return BL_SUCCESS;
}

static uint32_t BL_CDECL bl_image_encoder_impl_restart(BLImageEncoderImpl* impl) noexcept {
  bl_unused(impl);
  return BL_ERROR_INVALID_STATE;
}

static BLResult BL_CDECL bl_image_encoder_impl_write_frame(BLImageEncoderImpl* impl, BLArrayCore* dst, const BLImageCore* image) noexcept {
  bl_unused(impl, dst, image);
  return BL_ERROR_INVALID_STATE;
}

// bl::ImageEncoder - Runtime Registration
// =======================================

void bl_image_encoder_rt_init(BLRuntimeContext* rt) noexcept {
  using namespace bl::ImageEncoderInternal;

  bl_unused(rt);

  // Initialize default BLImageEncoder.
  default_encoder.virt.base.destroy = bl_image_encoder_impl_destroy;
  default_encoder.virt.base.get_property = bl_object_impl_get_property;
  default_encoder.virt.base.set_property = bl_object_impl_set_property;
  default_encoder.virt.restart = bl_image_encoder_impl_restart;
  default_encoder.virt.write_frame = bl_image_encoder_impl_write_frame;
  default_encoder.impl->ctor(
    &default_encoder.virt,
    static_cast<BLImageCodecCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]));
  default_encoder.impl->last_result = BL_ERROR_NOT_INITIALIZED;

  bl_object_defaults[BL_OBJECT_TYPE_IMAGE_ENCODER]._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_ENCODER), &default_encoder.impl);
}
