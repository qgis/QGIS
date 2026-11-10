// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/bitset_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/fontfeaturesettings_p.h>
#include <blend2d/core/fontmanager_p.h>
#include <blend2d/core/fontvariationsettings_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/pattern_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/core/var_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>

// bl::Object - Globals
// ====================

BLObjectCore bl_object_defaults[BL_OBJECT_TYPE_MAX_VALUE + 1];
const BLObjectImplHeader bl_object_header_with_ref_count_eq_0 = { 0, 0 };
const BLObjectImplHeader bl_object_header_with_ref_count_eq_1 = { 1, 0 };

void bl_object_destroy_external_data_dummy(void* impl, void* external_data, void* user_data) noexcept {
  bl_unused(impl, external_data, user_data);
}

// bl::Object - API - Alloc & Free Impl
// ====================================

static BL_INLINE BLResult bl_object_alloc_impl_internal(BLObjectCore* self, uint32_t object_info, size_t impl_size, size_t impl_flags, size_t impl_alignment, bool is_external = false) noexcept {
  if (BL_UNLIKELY(impl_size > BL_OBJECT_IMPL_MAX_SIZE))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  impl_size = bl::IntOps::align_up(impl_size, impl_alignment);

  size_t header_size = sizeof(BLObjectImplHeader) + (is_external ? sizeof(BLObjectExternalInfo) : size_t(0));
  size_t allocation_size = impl_size + header_size + impl_alignment;

  void* ptr = malloc(allocation_size);
  if (BL_UNLIKELY(!ptr))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  BLObjectImpl* impl = static_cast<BLObjectImpl*>(
    bl::IntOps::align_up(bl::PtrOps::offset(ptr, header_size), impl_alignment));
  BLObjectImplHeader* impl_header = bl::ObjectInternal::get_impl_header(impl);

  size_t alignment_offset = size_t(uintptr_t(impl) - uintptr_t(ptr)) - header_size;
  BL_ASSERT((alignment_offset & ~BLObjectImplHeader::kAlignmentOffsetMask) == 0);

  impl_header->ref_count = impl_flags & BLObjectImplHeader::kRefCountedAndImmutableFlags;
  impl_header->flags = impl_flags | alignment_offset;

  self->_d.clear_static_data();
  self->_d.impl = impl;
  self->_d.info.bits = object_info | BL_OBJECT_INFO_D_FLAG | BL_OBJECT_INFO_M_FLAG | BL_OBJECT_INFO_R_FLAG;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_object_alloc_impl(BLObjectCore* self, uint32_t object_info, size_t impl_size) noexcept {
  size_t flags = BLObjectImplHeader::kRefCountedFlag;
  return bl_object_alloc_impl_internal(self, object_info, impl_size, flags, BL_OBJECT_IMPL_ALIGNMENT);
}

BL_API_IMPL BLResult bl_object_alloc_impl_aligned(BLObjectCore* self, uint32_t object_info, size_t impl_size, size_t impl_alignment) noexcept {
  if (!bl::IntOps::is_power_of_2(impl_alignment))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  size_t flags = BLObjectImplHeader::kRefCountedFlag;
  impl_alignment = bl_clamp<size_t>(impl_alignment, 16, 128);
  return bl_object_alloc_impl_internal(self, object_info, impl_size, flags, impl_alignment);
}

BL_API_IMPL BLResult bl_object_alloc_impl_external(BLObjectCore* self, uint32_t object_info, size_t impl_size, bool immutable, BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {
  size_t flags = (BLObjectImplHeader::kRefCountedFlag) |
                 (BLObjectImplHeader::kExternalFlag) |
                 (size_t(immutable) << BLObjectImplHeader::kImmutableFlagShift);

  BL_PROPAGATE(bl_object_alloc_impl_internal(self, object_info, impl_size, flags, BL_OBJECT_IMPL_ALIGNMENT, true));
  bl::ObjectInternal::init_external_destroy_func(self->_d.impl, destroy_func, user_data);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_object_free_impl(BLObjectImpl* impl) noexcept {
  return bl::ObjectInternal::free_impl(impl);
}

BLResult bl_object_destroy_unknown_impl(BLObjectImpl* impl, BLObjectInfo info) noexcept {
  BL_ASSERT(info.is_dynamic_object());

  if (info.is_virtual_object())
    return bl::ObjectInternal::free_virtual_impl(impl);

  BLObjectType type = info.raw_type();
  switch (type) {
    case BL_OBJECT_TYPE_GRADIENT:
      return bl::GradientInternal::free_impl(static_cast<BLGradientPrivateImpl*>(impl));

    case BL_OBJECT_TYPE_PATTERN:
      return bl::PatternInternal::free_impl(static_cast<BLPatternPrivateImpl*>(impl));

    case BL_OBJECT_TYPE_STRING:
      return bl::StringInternal::free_impl(static_cast<BLStringImpl*>(impl));

    case BL_OBJECT_TYPE_PATH:
      return bl::PathInternal::free_impl(static_cast<BLPathPrivateImpl*>(impl));

    case BL_OBJECT_TYPE_IMAGE:
      return bl::ImageInternal::free_impl(static_cast<BLImagePrivateImpl*>(impl));

    case BL_OBJECT_TYPE_FONT:
      return bl::FontInternal::free_impl(static_cast<BLFontPrivateImpl*>(impl));

    case BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS:
      return bl::FontFeatureSettingsInternal::free_impl(static_cast<BLFontFeatureSettingsImpl*>(impl));

    case BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS:
      return bl::FontVariationSettingsInternal::free_impl(static_cast<BLFontVariationSettingsImpl*>(impl));

    case BL_OBJECT_TYPE_ARRAY_OBJECT:
    case BL_OBJECT_TYPE_ARRAY_INT8:
    case BL_OBJECT_TYPE_ARRAY_UINT8:
    case BL_OBJECT_TYPE_ARRAY_INT16:
    case BL_OBJECT_TYPE_ARRAY_UINT16:
    case BL_OBJECT_TYPE_ARRAY_INT32:
    case BL_OBJECT_TYPE_ARRAY_UINT32:
    case BL_OBJECT_TYPE_ARRAY_INT64:
    case BL_OBJECT_TYPE_ARRAY_UINT64:
    case BL_OBJECT_TYPE_ARRAY_FLOAT32:
    case BL_OBJECT_TYPE_ARRAY_FLOAT64:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_1:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_2:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_3:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_4:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_6:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_8:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_10:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_12:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_16:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_20:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_24:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_32:
      return bl::ArrayInternal::free_impl(static_cast<BLArrayImpl*>(impl));

    case BL_OBJECT_TYPE_BIT_SET:
      // NOTE: It's guaranteed that this BitSet is dynamic, so we don't have to correct the type.
      return bl::BitSetInternal::free_impl(static_cast<BLBitSetImpl*>(impl));

    default:
      // TODO: This shouldn't happen.
      return bl::ObjectInternal::free_impl(impl);
  }
}

// bl::Object - API - Construction & Destruction
// =============================================

BL_API_IMPL BLResult bl_object_init_move(BLUnknown* self, BLUnknown* other) noexcept {
  BL_ASSERT(self != other);

  return bl_object_private_init_move_unknown(bl_as_object(self), bl_as_object(other));
}

BL_API_IMPL BLResult bl_object_init_weak(BLUnknown* self, const BLUnknown* other) noexcept {
  BL_ASSERT(self != other);

  return bl_object_private_init_weak_unknown(bl_as_object(self), bl_as_object(other));
}

// bl::Object - API - Reset
// ========================

BL_API_IMPL BLResult bl_object_reset(BLUnknown* self) noexcept {
  BLObjectType type = bl_as_object(self)->_d.get_type();

  bl::ObjectInternal::release_unknown_instance(bl_as_object(self));
  bl_as_object(self)->_d = bl_object_defaults[type]._d;

  return BL_SUCCESS;
}

// bl::Object - API - Assign
// =========================

BL_API_IMPL BLResult bl_object_assign_move(BLUnknown* self, BLUnknown* other) noexcept {
  BLObjectType type = bl_as_object(other)->_d.get_type();
  BLObjectCore tmp = *bl_as_object(other);

  bl_as_object(other)->_d = bl_object_defaults[type]._d;
  bl::ObjectInternal::release_unknown_instance(bl_as_object(self));

  bl_as_object(self)->_d = tmp._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_object_assign_weak(BLUnknown* self, const BLUnknown* other) noexcept {
  return bl_object_private_assign_weak_unknown(bl_as_object(self), bl_as_object(other));
}

// bl::Object - API - Properties
// =============================

BL_API_IMPL BLResult bl_object_get_property(const BLUnknown* self, const char* name, size_t name_size, BLVarCore* value_out) noexcept {
  if (name_size == SIZE_MAX)
    name_size = strlen(name);

  if (!bl_as_object(self)->_d.is_virtual_object())
    return bl_make_error(BL_ERROR_INVALID_KEY);

  const BLObjectVirtImpl* impl = static_cast<const BLObjectVirtImpl*>(bl_as_object(self)->_d.impl);
  return impl->virt->base.get_property(impl, name, name_size, value_out);
}

BL_API_IMPL BLResult bl_object_get_property_bool(const BLUnknown* self, const char* name, size_t name_size, bool* value_out) noexcept {
  BLVarCore v;
  v._d.init_null();

  *value_out = false;
  BL_PROPAGATE(bl_object_get_property(self, name, name_size, &v));

  BLResult result = bl_var_to_bool(&v, value_out);
  bl_var_destroy(&v);
  return result;
}

BL_API_IMPL BLResult bl_object_get_property_int32(const BLUnknown* self, const char* name, size_t name_size, int32_t* value_out) noexcept {
  BLVarCore v;
  v._d.init_null();

  *value_out = 0;
  BL_PROPAGATE(bl_object_get_property(self, name, name_size, &v));

  BLResult result = bl_var_to_int32(&v, value_out);
  bl_var_destroy(&v);
  return result;
}

BL_API_IMPL BLResult bl_object_get_property_int64(const BLUnknown* self, const char* name, size_t name_size, int64_t* value_out) noexcept {
  BLVarCore v;
  v._d.init_null();

  *value_out = 0;
  BL_PROPAGATE(bl_object_get_property(self, name, name_size, &v));

  BLResult result = bl_var_to_int64(&v, value_out);
  bl_var_destroy(&v);
  return result;
}

BL_API_IMPL BLResult bl_object_get_property_uint32(const BLUnknown* self, const char* name, size_t name_size, uint32_t* value_out) noexcept {
  BLVarCore v;
  v._d.init_null();

  *value_out = 0;
  BL_PROPAGATE(bl_object_get_property(self, name, name_size, &v));

  BLResult result = bl_var_to_uint32(&v, value_out);
  bl_var_destroy(&v);
  return result;
}

BL_API_IMPL BLResult bl_object_get_property_uint64(const BLUnknown* self, const char* name, size_t name_size, uint64_t* value_out) noexcept {
  BLVarCore v;
  v._d.init_null();

  *value_out = 0;
  BL_PROPAGATE(bl_object_get_property(self, name, name_size, &v));

  BLResult result = bl_var_to_uint64(&v, value_out);
  bl_var_destroy(&v);
  return result;
}

BL_API_IMPL BLResult bl_object_get_property_double(const BLUnknown* self, const char* name, size_t name_size, double* value_out) noexcept {
  BLVarCore v;
  v._d.init_null();

  *value_out = 0.0;
  BL_PROPAGATE(bl_object_get_property(self, name, name_size, &v));

  BLResult result = bl_var_to_double(&v, value_out);
  bl_var_destroy(&v);
  return result;
}

BL_API_IMPL BLResult bl_object_set_property(BLUnknown* self, const char* name, size_t name_size, const BLUnknown* value) noexcept {
  if (name_size == SIZE_MAX)
    name_size = strlen(name);

  if (!bl_as_object(self)->_d.is_virtual_object())
    return bl_make_error(BL_ERROR_INVALID_KEY);

  BLObjectVirtImpl* impl = static_cast<BLObjectVirtImpl*>(bl_as_object(self)->_d.impl);
  return impl->virt->base.set_property(impl, name, name_size, static_cast<const BLVarCore*>(value));
}

BL_API_IMPL BLResult bl_object_set_property_bool(BLUnknown* self, const char* name, size_t name_size, bool value) noexcept {
  // NOTE: Bool value is always in SSO mode, no need to call BLVarCore destructor.
  BLVarCore v;
  v._d.init_bool(value);
  return bl_object_set_property(self, name, name_size, &v);
}

BL_API_IMPL BLResult bl_object_set_property_int32(BLUnknown* self, const char* name, size_t name_size, int32_t value) noexcept {
  // NOTE: Integer value is always in SSO mode, no need to call BLVarCore destructor.
  BLVarCore v;
  v._d.init_int64(value);
  return bl_object_set_property(self, name, name_size, &v);
}

BL_API_IMPL BLResult bl_object_set_property_int64(BLUnknown* self, const char* name, size_t name_size, int64_t value) noexcept {
  // NOTE: Integer value is always in SSO mode, no need to call BLVarCore destructor.
  BLVarCore v;
  v._d.init_int64(value);
  return bl_object_set_property(self, name, name_size, &v);
}

BL_API_IMPL BLResult bl_object_set_property_uint32(BLUnknown* self, const char* name, size_t name_size, uint32_t value) noexcept {
  // NOTE: Integer value is always in SSO mode, no need to call BLVarCore destructor.
  BLVarCore v;
  v._d.init_uint64(value);
  return bl_object_set_property(self, name, name_size, &v);
}

BL_API_IMPL BLResult bl_object_set_property_uint64(BLUnknown* self, const char* name, size_t name_size, uint64_t value) noexcept {
  // NOTE: Integer value is always in SSO mode, no need to call BLVarCore destructor.
  BLVarCore v;
  v._d.init_uint64(value);
  return bl_object_set_property(self, name, name_size, &v);
}

BL_API_IMPL BLResult bl_object_set_property_double(BLUnknown* self, const char* name, size_t name_size, double value) noexcept {
  // NOTE: Double value is always in SSO mode, no need to call BLVarCore destructor.
  BLVarCore v;
  v._d.init_double(value);
  return bl_object_set_property(self, name, name_size, &v);
}

BLResult bl_object_impl_get_property(const BLObjectImpl* impl, const char* name, size_t name_size, BLVarCore* value_out) noexcept {
  bl_unused(impl, name, name_size, value_out);
  return bl_make_error(BL_ERROR_INVALID_KEY);
}

BLResult bl_object_impl_set_property(BLObjectImpl* impl, const char* name, size_t name_size, const BLVarCore* value) noexcept {
  bl_unused(impl, name, name_size, value);
  return bl_make_error(BL_ERROR_INVALID_KEY);
}
