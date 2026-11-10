// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/imagedecoder.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>

// bl::ImageDecoder - Globals
// ==========================

namespace bl {
namespace ImageDecoderInternal {

static BLObjectEternalVirtualImpl<BLImageDecoderImpl, BLImageDecoderVirt> default_decoder;

} // {ImageDecoderInternal}
} // {bl}

// bl::ImageDecoder - API - Init & Destroy
// =======================================

BL_API_IMPL BLResult bl_image_decoder_init(BLImageDecoderCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_DECODER]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_decoder_init_move(BLImageDecoderCore* self, BLImageDecoderCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image_decoder());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_DECODER]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_decoder_init_weak(BLImageDecoderCore* self, const BLImageDecoderCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image_decoder());

  return bl_object_private_init_weak_tagged(self, other);
}

BL_API_IMPL BLResult bl_image_decoder_destroy(BLImageDecoderCore* self) noexcept {
  return bl::ObjectInternal::release_virtual_instance(self);
}

// bl::ImageDecoder - API - Reset
// ==============================

BL_API_IMPL BLResult bl_image_decoder_reset(BLImageDecoderCore* self) noexcept {
  BL_ASSERT(self->_d.is_image_decoder());

  return bl::ObjectInternal::replace_virtual_instance(self, static_cast<BLImageDecoderCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE_DECODER]));
}

// bl::ImageDecoder - API - Assign
// ===============================

BL_API_IMPL BLResult bl_image_decoder_assign_move(BLImageDecoderCore* self, BLImageDecoderCore* other) noexcept {
  BL_ASSERT(self->_d.is_image_decoder());
  BL_ASSERT(other->_d.is_image_decoder());

  BLImageDecoderCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_DECODER]._d;
  return bl::ObjectInternal::replace_virtual_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_image_decoder_assign_weak(BLImageDecoderCore* self, const BLImageDecoderCore* other) noexcept {
  BL_ASSERT(self->_d.is_image_decoder());
  BL_ASSERT(other->_d.is_image_decoder());

  return bl::ObjectInternal::assign_virtual_instance(self, other);
}

// bl::ImageDecoder - API - Interface
// ==================================

BL_API_IMPL BLResult bl_image_decoder_restart(BLImageDecoderCore* self) noexcept {
  BL_ASSERT(self->_d.is_image_decoder());
  BLImageDecoderImpl* self_impl = self->_impl();

  return self_impl->virt->restart(self_impl);
}

BL_API_IMPL BLResult bl_image_decoder_read_info(BLImageDecoderCore* self, BLImageInfo* info_out, const uint8_t* data, size_t size) noexcept {
  BL_ASSERT(self->_d.is_image_decoder());
  BLImageDecoderImpl* self_impl = self->_impl();

  return self_impl->virt->read_info(self_impl, info_out, data, size);
}

BL_API_IMPL BLResult bl_image_decoder_read_frame(BLImageDecoderCore* self, BLImageCore* image_out, const uint8_t* data, size_t size) noexcept {
  BL_ASSERT(self->_d.is_image_decoder());
  BLImageDecoderImpl* self_impl = self->_impl();

  return self_impl->virt->read_frame(self_impl, image_out, data, size);
}

// bl::ImageDecoder - Virtual Functions (Null)
// ===========================================

static BLResult BL_CDECL bl_image_decoder_impl_destroy(BLObjectImpl* impl) noexcept {
  bl_unused(impl);
  return BL_SUCCESS;
}

static uint32_t BL_CDECL bl_image_decoder_impl_restart(BLImageDecoderImpl* impl) noexcept {
  bl_unused(impl);
  return BL_ERROR_INVALID_STATE;
}

static BLResult BL_CDECL bl_image_decoder_impl_read_info(BLImageDecoderImpl* impl, BLImageInfo* info_out, const uint8_t* data, size_t size) noexcept {
  bl_unused(impl, info_out, data, size);
  return BL_ERROR_INVALID_STATE;
}

static BLResult BL_CDECL bl_image_decoder_impl_read_frame(BLImageDecoderImpl* impl, BLImageCore* image_out, const uint8_t* data, size_t size) noexcept {
  bl_unused(impl, image_out, data, size);
  return BL_ERROR_INVALID_STATE;
}

// bl::ImageDecoder - Runtime Registration
// =======================================

void bl_image_decoder_rt_init(BLRuntimeContext* rt) noexcept {
  using namespace bl::ImageDecoderInternal;

  bl_unused(rt);

  // Initialize default BLImageDecoder.
  default_decoder.virt.base.destroy = bl_image_decoder_impl_destroy;
  default_decoder.virt.base.get_property = bl_object_impl_get_property;
  default_decoder.virt.base.set_property = bl_object_impl_set_property;
  default_decoder.virt.restart = bl_image_decoder_impl_restart;
  default_decoder.virt.read_info = bl_image_decoder_impl_read_info;
  default_decoder.virt.read_frame = bl_image_decoder_impl_read_frame;
  default_decoder.impl->ctor(
    &default_decoder.virt,
    static_cast<BLImageCodecCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]));
  default_decoder.impl->last_result = BL_ERROR_NOT_INITIALIZED;

  bl_object_defaults[BL_OBJECT_TYPE_IMAGE_DECODER]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_DECODER),
    &default_decoder.impl);
}
