// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/imagecodec.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/codec/bmpcodec_p.h>
#include <blend2d/codec/jpegcodec_p.h>
#include <blend2d/codec/pngcodec_p.h>
#include <blend2d/codec/qoicodec_p.h>
#include <blend2d/support/stringops_p.h>
#include <blend2d/support/wrap_p.h>
#include <blend2d/threading/mutex_p.h>

namespace bl {
namespace ImageCodecInternal {

// bl::ImageCodec - Globals
// ========================

static BLObjectEternalVirtualImpl<BLImageCodecImpl, BLImageCodecVirt> default_codec;
static Wrap<BLArray<BLImageCodec>> builtin_codecs_array;
static Wrap<BLSharedMutex> builtin_codecs_mutex;

} // {BLImageCodecInternal}
} // {bl}

// bl::ImageCodec - API - Init & Destroy
// =====================================

BL_API_IMPL BLResult bl_image_codec_init(BLImageCodecCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_codec_init_move(BLImageCodecCore* self, BLImageCodecCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image_codec());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_codec_init_weak(BLImageCodecCore* self, const BLImageCodecCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image_codec());

  return bl_object_private_init_weak_tagged(self, other);
}

BL_API_IMPL BLResult bl_image_codec_init_by_name(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]._d;
  return bl_image_codec_find_by_name(self, name, size, codecs);
}

BL_API_IMPL BLResult bl_image_codec_destroy(BLImageCodecCore* self) noexcept {
  return bl::ObjectInternal::release_virtual_instance(self);
}

// bl::ImageCodec - API - Reset
// ============================

BL_API_IMPL BLResult bl_image_codec_reset(BLImageCodecCore* self) noexcept {
  BL_ASSERT(self->_d.is_image_codec());

  return bl::ObjectInternal::replace_virtual_instance(self, static_cast<BLImageCodecCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]));
}

// bl::ImageCodec - API - Assign
// =============================

BL_API_IMPL BLResult bl_image_codec_assign_move(BLImageCodecCore* self, BLImageCodecCore* other) noexcept {
  BL_ASSERT(self->_d.is_image_codec());
  BL_ASSERT(other->_d.is_image_codec());

  BLImageCodecCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]._d;
  return bl::ObjectInternal::replace_virtual_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_image_codec_assign_weak(BLImageCodecCore* self, const BLImageCodecCore* other) noexcept {
  BL_ASSERT(self->_d.is_image_codec());
  BL_ASSERT(other->_d.is_image_codec());

  return bl::ObjectInternal::assign_virtual_instance(self, other);
}

// bl::ImageCodec - API - Inspect Data
// ===================================

BL_API_IMPL uint32_t bl_image_codec_inspect_data(const BLImageCodecCore* self, const void* data, size_t size) noexcept {
  BL_ASSERT(self->_d.is_image_codec());
  BLImageCodecImpl* self_impl = self->dcast()._impl();

  return self_impl->virt->inspect_data(self_impl, static_cast<const uint8_t*>(data), size);
}

// bl::ImageCodec - API - Find By Name & Extension & Data
// ======================================================

namespace bl {
namespace ImageCodecInternal {

static bool match_extension(const char* extensions, const char* match, size_t match_size) noexcept {
  while (*extensions) {
    // Match each extension in `extensions` string list delimited by '|'.
    const char* p = extensions;

    while (*p && *p != '|') {
      p++;
    }

    size_t ext_size = (size_t)(p - extensions);
    if (ext_size == match_size && StringOps::memeq_ci(extensions, match, match_size)) {
      return true;
    }

    if (*p == '|') {
      p++;
    }

    extensions = p;
  }
  return false;
}

// Returns possibly advanced `match` and fixes the `size`.
static const char* keep_only_extension_in_match(const char* match, size_t& size) noexcept {
  const char* end = match + size;
  const char* p = end;

  while (p != match && p[-1] != '.') {
    p--;
  }

  size = (size_t)(end - p);
  return p;
}

static BLResult find_codec_by_name(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) noexcept {
  for (const BLImageCodec& codec : codecs->dcast<BLArray<BLImageCodec>>().view()) {
    if (codec.name() == BLStringView{name, size}) {
      return bl_image_codec_assign_weak(self, &codec);
    }
  }

  return bl_make_error(BL_ERROR_IMAGE_NO_MATCHING_CODEC);
}

static BLResult find_codec_by_extension(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) noexcept {
  for (const BLImageCodec& codec : codecs->dcast<BLArray<BLImageCodec>>().view()) {
    if (match_extension(codec.extensions().data(), name, size)) {
      return bl_image_codec_assign_weak(self, &codec);
    }
  }

  return bl_make_error(BL_ERROR_IMAGE_NO_MATCHING_CODEC);
}

static BLResult find_codec_by_data(BLImageCodecCore* self, const void* data, size_t size, const BLArrayCore* codecs) noexcept {
  uint32_t best_score = 0;
  const BLImageCodec* candidate = nullptr;

  for (const BLImageCodec& codec : codecs->dcast<BLArray<BLImageCodec>>().view()) {
    uint32_t score = codec.inspect_data(data, size);
    if (best_score < score) {
      best_score = score;
      candidate = &codec;
    }
  }

  if (candidate) {
    return bl_image_codec_assign_weak(self, candidate);
  }

  return bl_make_error(BL_ERROR_IMAGE_NO_MATCHING_CODEC);
}

} // {ImageCodecInternal}
} // {bl}

BL_API_IMPL BLResult bl_image_codec_find_by_name(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) noexcept {
  using namespace bl::ImageCodecInternal;
  BL_ASSERT(self->_d.is_image_codec());

  if (size == SIZE_MAX) {
    size = strlen(name);
  }

  if (!size) {
    return bl_make_error(BL_ERROR_IMAGE_NO_MATCHING_CODEC);
  }

  if (codecs) {
    return find_codec_by_name(self, name, size, codecs);
  }
  else {
    return builtin_codecs_mutex->protect_shared([&] { return find_codec_by_name(self, name, size, &builtin_codecs_array); });
  }
}

BL_API_IMPL BLResult bl_image_codec_find_by_extension(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) noexcept {
  using namespace bl::ImageCodecInternal;
  BL_ASSERT(self->_d.is_image_codec());

  if (size == SIZE_MAX) {
    size = strlen(name);
  }

  name = keep_only_extension_in_match(name, size);
  if (codecs) {
    return find_codec_by_extension(self, name, size, codecs);
  }
  else {
    return builtin_codecs_mutex->protect_shared([&] { return find_codec_by_extension(self, name, size, &builtin_codecs_array); });
  }
}

BL_API_IMPL BLResult bl_image_codec_find_by_data(BLImageCodecCore* self, const void* data, size_t size, const BLArrayCore* codecs) noexcept {
  using namespace bl::ImageCodecInternal;
  BL_ASSERT(self->_d.is_image_codec());

  if (codecs) {
    return find_codec_by_data(self, data, size, codecs);
  }
  else {
    return builtin_codecs_mutex->protect_shared([&] { return find_codec_by_data(self, data, size, &builtin_codecs_array); });
  }
}

BL_API_IMPL BLResult bl_image_codec_create_decoder(const BLImageCodecCore* self, BLImageDecoderCore* dst) noexcept {
  BL_ASSERT(self->_d.is_image_codec());
  BLImageCodecImpl* self_impl = self->dcast()._impl();

  return self_impl->virt->create_decoder(self_impl, dst);
}

BL_API_IMPL BLResult bl_image_codec_create_encoder(const BLImageCodecCore* self, BLImageEncoderCore* dst) noexcept {
  BL_ASSERT(self->_d.is_image_codec());
  BLImageCodecImpl* self_impl = self->dcast()._impl();

  return self_impl->virt->create_encoder(self_impl, dst);
}

// bl::ImageCodec - API - Built-In Codecs (Global)
// ===============================================

BL_API_IMPL BLResult bl_image_codec_array_init_built_in_codecs(BLArrayCore* self) noexcept {
  using namespace bl::ImageCodecInternal;

  *self = builtin_codecs_mutex->protect_shared([&] {
    BLArrayCore tmp = builtin_codecs_array();
    bl::ObjectInternal::retain_instance(&tmp);
    return tmp;
  });

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_codec_array_assign_built_in_codecs(BLArrayCore* self) noexcept {
  bl_array_destroy(self);
  return bl_image_codec_array_init_built_in_codecs(self);
}

BL_API_IMPL BLResult bl_image_codec_add_to_built_in(const BLImageCodecCore* codec) noexcept {
  using namespace bl::ImageCodecInternal;
  BL_ASSERT(codec->_d.is_image_codec());

  return builtin_codecs_mutex->protect([&] {
    size_t i = builtin_codecs_array->index_of(codec->dcast());
    if (i != SIZE_MAX) {
      return bl_make_error(BL_ERROR_ALREADY_EXISTS);
    }
    return builtin_codecs_array->append(codec->dcast());
  });
}

BL_API_IMPL BLResult bl_image_codec_remove_from_built_in(const BLImageCodecCore* codec) noexcept {
  using namespace bl::ImageCodecInternal;
  BL_ASSERT(codec->_d.is_image_codec());

  return builtin_codecs_mutex->protect([&] {
    size_t i = builtin_codecs_array->index_of(codec->dcast());
    if (i == SIZE_MAX) {
      return bl_make_error(BL_ERROR_NO_ENTRY);
    }
    return builtin_codecs_array->remove(i);
  });
}

// bl::ImageCodec - Virtual Functions (Null)
// =========================================

BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)

static BLResult BL_CDECL bl_image_codec_impl_destroy(BLObjectImpl* impl) noexcept { return BL_SUCCESS; }
static uint32_t BL_CDECL bl_image_codec_impl_inspect_data(const BLImageCodecImpl* impl, const uint8_t* data, size_t size) noexcept { return 0; }
static BLResult BL_CDECL bl_image_codec_impl_create_decoder(const BLImageCodecImpl* impl, BLImageDecoderCore* dst) noexcept { return BL_ERROR_IMAGE_DECODER_NOT_PROVIDED; }
static BLResult BL_CDECL bl_image_codec_impl_create_encoder(const BLImageCodecImpl* impl, BLImageEncoderCore* dst) noexcept { return BL_ERROR_IMAGE_ENCODER_NOT_PROVIDED; }

BL_DIAGNOSTIC_POP

// bl::ImageCodec - Runtime Registration
// =====================================

static void BL_CDECL bl_image_codec_rt_shutdown(BLRuntimeContext* rt) noexcept {
  using namespace bl::ImageCodecInternal;
  bl_unused(rt);

  builtin_codecs_array.destroy();
  builtin_codecs_mutex.destroy();
}

void bl_image_codec_rt_init(BLRuntimeContext* rt) noexcept {
  using namespace bl::ImageCodecInternal;

  builtin_codecs_mutex.init();
  builtin_codecs_array.init();

  // Initialize default BLImageCodec.
  default_codec.virt.base.destroy = bl_image_codec_impl_destroy;
  default_codec.virt.base.get_property = bl_object_impl_get_property;
  default_codec.virt.base.set_property = bl_object_impl_set_property;
  default_codec.virt.inspect_data = bl_image_codec_impl_inspect_data;
  default_codec.virt.create_decoder = bl_image_codec_impl_create_decoder;
  default_codec.virt.create_encoder = bl_image_codec_impl_create_encoder;
  default_codec.impl->ctor(&default_codec.virt);

  bl_object_defaults[BL_OBJECT_TYPE_IMAGE_CODEC]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_CODEC),
    &default_codec.impl);

  rt->shutdown_handlers.add(bl_image_codec_rt_shutdown);
}

void bl_register_built_in_codecs(BLRuntimeContext* rt) noexcept {
  using namespace bl::ImageCodecInternal;

  BLArray<BLImageCodec>* codecs = &builtin_codecs_array;
  codecs->reserve(4);
  bl::Bmp::bmp_codec_on_init(rt, codecs);
  bl::Jpeg::jpeg_codec_on_init(rt, codecs);
  bl::Png::png_codec_on_init(rt, codecs);
  bl::Qoi::qoi_codec_on_init(rt, codecs);
}
