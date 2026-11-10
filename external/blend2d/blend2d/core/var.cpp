
// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/bitarray_p.h>
#include <blend2d/core/bitset_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/fontfeaturesettings_p.h>
#include <blend2d/core/fontvariationsettings_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/pattern_p.h>
#include <blend2d/core/rgba_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/core/var_p.h>
#include <blend2d/support/math_p.h>

// bl::Var - API - Init & Destroy
// ==============================

BL_API_IMPL BLResult bl_var_init_type(BLUnknown* self, BLObjectType type) noexcept {
  BLResult result = BL_SUCCESS;

  if (BL_UNLIKELY(uint32_t(type) > BL_OBJECT_TYPE_MAX_VALUE)) {
    type = BL_OBJECT_TYPE_NULL;
    result = bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  bl_as_object(self)->_d = bl_object_defaults[type]._d;
  return result;
}

BL_API_IMPL BLResult bl_var_init_null(BLUnknown* self) noexcept {
  bl_as_object(self)->_d.init_null();
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_bool(BLUnknown* self, bool value) noexcept {
  bl_as_object(self)->_d.init_bool(value);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_int32(BLUnknown* self, int32_t value) noexcept {
  bl_as_object(self)->_d.init_int64(value);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_int64(BLUnknown* self, int64_t value) noexcept {
  bl_as_object(self)->_d.init_int64(value);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_uint32(BLUnknown* self, uint32_t value) noexcept {
  bl_as_object(self)->_d.init_uint64(value);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_uint64(BLUnknown* self, uint64_t value) noexcept {
  bl_as_object(self)->_d.init_uint64(value);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_double(BLUnknown* self, double value) noexcept {
  bl_as_object(self)->_d.init_double(value);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_rgba(BLUnknown* self, const BLRgba* rgba) noexcept {
  return bl::VarInternal::init_rgba(bl_as_object(self), rgba);
}

BL_API_IMPL BLResult bl_var_init_rgba32(BLUnknown* self, uint32_t rgba32) noexcept {
  bl_as_object(self)->_d.init_rgba32(rgba32);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_rgba64(BLUnknown* self, uint64_t rgba64) noexcept {
  bl_as_object(self)->_d.init_rgba64(rgba64);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_init_move(BLUnknown* self, BLUnknown* other) noexcept {
  BL_ASSERT(self != other);

  return bl_object_private_init_move_unknown(bl_as_object(self), bl_as_object(other));
}

BL_API_IMPL BLResult bl_var_init_weak(BLUnknown* self, const BLUnknown* other) noexcept {
  BL_ASSERT(static_cast<void*>(self) != other);

  return bl_object_private_init_weak_unknown(bl_as_object(self), bl_as_object(other));
}

BL_API_IMPL BLResult bl_var_destroy(BLUnknown* self) noexcept {
  return bl::ObjectInternal::release_unknown_instance(bl_as_object(self));
}

// bl::Var - API - Reset
// =====================

BL_API_IMPL BLResult bl_var_reset(BLUnknown* self) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_null();
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

// bl::Var - API - Assign
// ======================

BL_API_IMPL BLResult bl_var_assign_null(BLUnknown* self) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_null();
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_bool(BLUnknown* self, bool value) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_bool(value);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_int32(BLUnknown* self, int32_t value) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_int64(value);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_int64(BLUnknown* self, int64_t value) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_int64(value);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_uint32(BLUnknown* self, uint32_t value) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_uint64(value);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_uint64(BLUnknown* self, uint64_t value) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_uint64(value);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_double(BLUnknown* self, double value) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_double(value);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_rgba(BLUnknown* self, const BLRgba* rgba) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl::VarInternal::init_rgba(bl_as_object(self), rgba);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_rgba32(BLUnknown* self, uint32_t rgba32) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_rgba32(rgba32);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_rgba64(BLUnknown* self, uint64_t rgba64) noexcept {
  BLObjectCore tmp = *bl_as_object(self);
  bl_as_object(self)->_d.init_rgba64(rgba64);
  return bl::ObjectInternal::release_unknown_instance(&tmp);
}

BL_API_IMPL BLResult bl_var_assign_move(BLUnknown* self, BLUnknown* other) noexcept {
  BLObjectType other_type = bl_as_object(other)->_d.get_type();
  BLObjectCore tmp = *bl_as_object(other);

  bl_as_object(other)->_d = bl_object_defaults[other_type]._d;
  bl::ObjectInternal::release_unknown_instance(bl_as_object(self));

  bl_as_object(self)->_d = tmp._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_var_assign_weak(BLUnknown* self, const BLUnknown* other) noexcept {
  return bl_object_private_assign_weak_unknown(bl_as_object(self), bl_as_object(other));
}

// bl::Var - API - Get Type & Value
// ================================

BL_API_IMPL BLObjectType bl_var_get_type(const BLUnknown* self) noexcept {
  return bl_as_object(self)->_d.get_type();
}

BL_API_IMPL BLResult bl_var_to_bool(const BLUnknown* self, bool* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL: {
      *out = false;
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_BOOL:
    case BL_OBJECT_TYPE_INT64:
    case BL_OBJECT_TYPE_UINT64: {
      *out = (d.u64_data[0] != 0);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_DOUBLE: {
      *out = (d.f64_data[0] != 0.0 && !bl::Math::is_nan(d.f64_data[0]));
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_STRING: {
      return !static_cast<const BLString*>(static_cast<const BLObjectCore*>(self))->is_empty();
    }

    default: {
      *out = false;
      return bl_make_error(BL_ERROR_INVALID_CONVERSION);
    }
  }
}

BL_API_IMPL BLResult bl_var_to_int32(const BLUnknown* self, int32_t* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL: {
      *out = 0;
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_BOOL: {
      *out = int32_t(d.i64_data[0] & 0xFFFFFFFFu);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_INT64: {
      int64_t v = d.i64_data[0];

      if (v < int64_t(INT32_MIN)) {
        *out = INT32_MIN;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      if (v > int64_t(INT32_MAX)) {
        *out = INT32_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      *out = int32_t(v);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_UINT64: {
      if (d.u64_data[0] > uint64_t(INT32_MAX)) {
        *out = INT32_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      *out = int32_t(d.i64_data[0]);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_DOUBLE: {
      double f = d.f64_data[0];

      if (bl::Math::is_nan(f)) {
        *out = 0;
        return bl_make_error(BL_ERROR_INVALID_CONVERSION);
      }

      if (f < double(INT32_MIN)) {
        *out = INT32_MIN;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      if (f > double(INT32_MAX)) {
        *out = INT32_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      int32_t v = bl::Math::trunc_to_int(f);

      *out = v;
      if (double(v) != f)
        return bl_make_error(BL_ERROR_OVERFLOW);
      else
        return BL_SUCCESS;
    }

    default: {
      *out = 0;
      return bl_make_error(BL_ERROR_INVALID_CONVERSION);
    }
  }
}

BL_API_IMPL BLResult bl_var_to_int64(const BLUnknown* self, int64_t* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL: {
      *out = 0;
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_BOOL:
    case BL_OBJECT_TYPE_INT64: {
      *out = d.i64_data[0];
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_UINT64: {
      if (d.u64_data[0] > uint64_t(INT64_MAX)) {
        *out = INT64_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      *out = d.i64_data[0];
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_DOUBLE: {
      double f = d.f64_data[0];

      if (bl::Math::is_nan(f)) {
        *out = 0;
        return bl_make_error(BL_ERROR_INVALID_CONVERSION);
      }

      if (f < double(INT64_MIN)) {
        *out = INT64_MIN;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      if (f > double(INT64_MAX)) {
        *out = INT64_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      int64_t v = bl::Math::trunc_to_int64(f);

      *out = v;
      if (double(v) != f)
        return bl_make_error(BL_ERROR_OVERFLOW);
      else
        return BL_SUCCESS;
    }

    default: {
      *out = 0;
      return bl_make_error(BL_ERROR_INVALID_CONVERSION);
    }
  }
}

BL_API_IMPL BLResult bl_var_to_uint32(const BLUnknown* self, uint32_t* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL: {
      *out = 0;
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_BOOL: {
      *out = uint32_t(d.u64_data[0] & 0xFFFFFFFFu);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_UINT64: {
      uint64_t v = d.u64_data[0];

      if (v > uint64_t(UINT32_MAX)) {
        *out = UINT32_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      *out = uint32_t(v);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_INT64: {
      int64_t v = d.i64_data[0];

      if (v < 0) {
        *out = 0;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      if (v > int64_t(UINT32_MAX)) {
        *out = UINT32_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      *out = uint32_t(v);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_DOUBLE: {
      double f = d.f64_data[0];

      if (bl::Math::is_nan(f)) {
        *out = 0u;
        return bl_make_error(BL_ERROR_INVALID_CONVERSION);
      }

      if (f < 0) {
        *out = 0u;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      if (f > double(UINT32_MAX)) {
        *out = UINT32_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      uint32_t v = uint32_t(f);

      *out = v;
      if (double(v) != f)
        return bl_make_error(BL_ERROR_OVERFLOW);

      return BL_SUCCESS;
    }

    default: {
      *out = 0u;
      return bl_make_error(BL_ERROR_INVALID_CONVERSION);
    }
  }
}

BL_API_IMPL BLResult bl_var_to_uint64(const BLUnknown* self, uint64_t* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL: {
      *out = 0;
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_BOOL:
    case BL_OBJECT_TYPE_UINT64: {
      *out = d.u64_data[0];
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_INT64: {
      int64_t v = d.i64_data[0];

      if (v < 0) {
        *out = 0;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      *out = uint64_t(v);
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_DOUBLE: {
      double f = d.f64_data[0];

      if (bl::Math::is_nan(f)) {
        *out = 0u;
        return bl_make_error(BL_ERROR_INVALID_CONVERSION);
      }

      if (f < 0) {
        *out = 0u;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      if (f > double(UINT64_MAX)) {
        *out = UINT64_MAX;
        return bl_make_error(BL_ERROR_OVERFLOW);
      }

      uint64_t v = uint64_t(f);

      *out = v;
      if (double(v) != f)
        return bl_make_error(BL_ERROR_OVERFLOW);

      return BL_SUCCESS;
    }

    default: {
      *out = 0u;
      return bl_make_error(BL_ERROR_INVALID_CONVERSION);
    }
  }
}

BL_API_IMPL BLResult bl_var_to_double(const BLUnknown* self, double* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL: {
      *out = 0.0;
      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_BOOL:
      *out = d.u64_data[0] ? 1.0 : 0.0;
      return BL_SUCCESS;

    case BL_OBJECT_TYPE_INT64: {
      double v = double(d.i64_data[0]);

      *out = v;
      if (int64_t(v) != d.i64_data[0])
        return bl_make_error(BL_ERROR_OVERFLOW);

      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_UINT64: {
      double v = double(d.u64_data[0]);

      *out = v;
      if (uint64_t(v) != d.u64_data[0])
        return bl_make_error(BL_ERROR_OVERFLOW);

      return BL_SUCCESS;
    }

    case BL_OBJECT_TYPE_DOUBLE: {
      *out = d.f64_data[0];
      return BL_SUCCESS;
    }

    default: {
      *out = 0.0;
      return bl_make_error(BL_ERROR_INVALID_STATE);
    }
  }
}

BL_API_IMPL BLResult bl_var_to_rgba(const BLUnknown* self, BLRgba* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  if (!d.has_object_signature()) {
    out->reset(d.f32_data[0], d.f32_data[1], d.f32_data[2], d.f32_data[3]);
    return BL_SUCCESS;
  }

  if (d.is_rgba32()) {
    *out = BLRgba(BLRgba32(d.u32_data[0]));
    return BL_SUCCESS;
  }

  if (d.is_rgba64()) {
    *out = BLRgba(BLRgba64(d.u64_data[0]));
    return BL_SUCCESS;
  }

  return bl_make_error(BL_ERROR_INVALID_STATE);
}

BL_API_IMPL BLResult bl_var_to_rgba32(const BLUnknown* self, uint32_t* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  if (d.is_rgba32()) {
    *out = d.u32_data[0];
    return BL_SUCCESS;
  }

  if (d.is_rgba64()) {
    *out = BLRgba32(BLRgba64(d.u64_data[0])).value;
    return BL_SUCCESS;
  }

  if (!d.has_object_signature()) {
    *out = reinterpret_cast<const BLRgba*>(self)->to_rgba32().value;
    return BL_SUCCESS;
  }

  return bl_make_error(BL_ERROR_INVALID_STATE);
}

BL_API_IMPL BLResult bl_var_to_rgba64(const BLUnknown* self, uint64_t* out) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  if (d.is_rgba64()) {
    *out = d.u64_data[0];
    return BL_SUCCESS;
  }

  if (d.is_rgba32()) {
    *out = BLRgba64(BLRgba32(d.u32_data[0])).value;
    return BL_SUCCESS;
  }

  if (!d.has_object_signature()) {
    *out = reinterpret_cast<const BLRgba*>(self)->to_rgba64().value;
    return BL_SUCCESS;
  }

  return bl_make_error(BL_ERROR_INVALID_STATE);
}

// bl::Var - API - Equality & Comparison
// =====================================

BL_API_IMPL bool bl_var_equals(const BLUnknown* a, const BLUnknown* b) noexcept {
  const BLObjectDetail& a_d = bl_as_object(a)->_d;
  const BLObjectDetail& b_d = bl_as_object(b)->_d;

  if (a_d == b_d)
    return true;

  BLObjectType a_type = a_d.get_type();
  BLObjectType b_type = b_d.get_type();

  if (a_type != b_type) {
    if (b_type == BL_OBJECT_TYPE_BOOL)
      return bl_var_equals_bool(a, b_d.u64_data[0] != 0);

    if (b_type == BL_OBJECT_TYPE_INT64)
      return bl_var_equals_int64(a, b_d.i64_data[0]);

    if (b_type == BL_OBJECT_TYPE_UINT64)
      return bl_var_equals_uint64(a, b_d.u64_data[0]);

    if (b_type == BL_OBJECT_TYPE_DOUBLE)
      return bl_var_equals_double(a, b_d.f64_data[0]);

    return false;
  }

  switch (a_type) {
    case BL_OBJECT_TYPE_NULL:
      // Suspicious: NULL objects should be binary equal - this should never happen.
      return true;

    case BL_OBJECT_TYPE_RGBA:
      // BLRgba must be binary equal.
      return false;

    case BL_OBJECT_TYPE_PATTERN:
      return bl_pattern_equals(static_cast<const BLPatternCore*>(a), static_cast<const BLPatternCore*>(b));

    case BL_OBJECT_TYPE_GRADIENT:
      return bl_gradient_equals(static_cast<const BLGradientCore*>(a), static_cast<const BLGradientCore*>(b));

    case BL_OBJECT_TYPE_IMAGE:
      return bl_image_equals(static_cast<const BLImageCore*>(a), static_cast<const BLImageCore*>(b));

    case BL_OBJECT_TYPE_PATH:
      return bl_path_equals(static_cast<const BLPathCore*>(a), static_cast<const BLPathCore*>(b));

    case BL_OBJECT_TYPE_FONT:
      return bl_font_equals(static_cast<const BLFontCore*>(a), static_cast<const BLFontCore*>(b));

    case BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS:
      return bl_font_feature_settings_equals(static_cast<const BLFontFeatureSettingsCore*>(a), static_cast<const BLFontFeatureSettingsCore*>(b));

    case BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS:
      return bl_font_variation_settings_equals(static_cast<const BLFontVariationSettingsCore*>(a), static_cast<const BLFontVariationSettingsCore*>(b));

    case BL_OBJECT_TYPE_BIT_SET:
      return bl_bit_set_equals(static_cast<const BLBitSetCore*>(a), static_cast<const BLBitSetCore*>(b));

    case BL_OBJECT_TYPE_BIT_ARRAY:
      return bl_bit_array_equals(static_cast<const BLBitArrayCore*>(a), static_cast<const BLBitArrayCore*>(b));

    case BL_OBJECT_TYPE_BOOL:
    case BL_OBJECT_TYPE_INT64:
    case BL_OBJECT_TYPE_UINT64:
      // These must be binary equal.
      return false;

    case BL_OBJECT_TYPE_DOUBLE:
      return a_d.f64_data[0] == a_d.f64_data[0];

    case BL_OBJECT_TYPE_STRING:
      return bl_string_equals(static_cast<const BLStringCore*>(a), static_cast<const BLStringCore*>(b));

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
      return bl_array_equals(static_cast<const BLArrayCore*>(a), static_cast<const BLArrayCore*>(b));

    default:
      return false;
  }
}

BL_API_IMPL bool bl_var_equals_null(const BLUnknown* self) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  return d.get_type() == BL_OBJECT_TYPE_NULL;
}

BL_API_IMPL bool bl_var_equals_bool(const BLUnknown* self, bool value) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL:
      return false;

    case BL_OBJECT_TYPE_BOOL:
    case BL_OBJECT_TYPE_INT64:
    case BL_OBJECT_TYPE_UINT64:
      return d.u64_data[0] == uint64_t(value);

    case BL_OBJECT_TYPE_DOUBLE:
      return d.f64_data[0] == double(value);

    default:
      return false;
  }
}

BL_API_IMPL bool bl_var_equals_int64(const BLUnknown* self, int64_t value) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL:
      return false;

    case BL_OBJECT_TYPE_BOOL:
    case BL_OBJECT_TYPE_INT64:
      return d.i64_data[0] == value;

    case BL_OBJECT_TYPE_UINT64:
      return d.i64_data[0] == value && value >= 0;

    case BL_OBJECT_TYPE_DOUBLE:
      return d.f64_data[0] == double(value) && int64_t(double(value)) == value;

    default:
      return false;
  }

}

BL_API_IMPL bool bl_var_equals_uint64(const BLUnknown* self, uint64_t value) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL:
      return false;

    case BL_OBJECT_TYPE_BOOL:
    case BL_OBJECT_TYPE_UINT64:
      return d.u64_data[0] == value;

    case BL_OBJECT_TYPE_INT64:
      return d.u64_data[0] == value && d.i64_data[0] >= 0;

    case BL_OBJECT_TYPE_DOUBLE:
      return d.f64_data[0] == double(value) && uint64_t(double(value)) == value;

    default:
      return false;
  }
}

BL_API_IMPL bool bl_var_equals_double(const BLUnknown* self, double value) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  switch (d.get_type()) {
    case BL_OBJECT_TYPE_NULL:
      return false;

    case BL_OBJECT_TYPE_BOOL:
      return double(d.u64_data[0]) == value;

    case BL_OBJECT_TYPE_INT64:
      return double(d.i64_data[0]) == value && int64_t(double(d.i64_data[0])) == d.i64_data[0];

    case BL_OBJECT_TYPE_UINT64:
      return double(d.u64_data[0]) == value && uint64_t(double(d.u64_data[0])) == d.u64_data[0];

    case BL_OBJECT_TYPE_DOUBLE:
      return d.f64_data[0] == value || (bl::Math::is_nan(d.f64_data[0]) && bl::Math::is_nan(value));

    default:
      return false;
  }
}

BL_API_IMPL bool BL_CDECL bl_var_equals_rgba(const BLUnknown* self, const BLRgba* rgba) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  if (!d.has_object_signature())
    return memcmp(d.f32_data, rgba, 4 * sizeof(float)) == 0;

  if (d.is_rgba32()) {
    BLRgba converted = BLRgba(BLRgba32(d.u32_data[0]));
    return memcmp(&converted, rgba, 4 * sizeof(float)) == 0;
  }

  if (d.is_rgba64()) {
    BLRgba converted = BLRgba(BLRgba64(d.u64_data[0]));
    return memcmp(&converted, rgba, 4 * sizeof(float)) == 0;
  }

  return false;
}

BL_API_IMPL bool BL_CDECL bl_var_equals_rgba32(const BLUnknown* self, uint32_t rgba32) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  if (d.is_rgba32()) {
    return d.u32_data[0] == rgba32;
  }

  if (d.is_rgba64()) {
    BLRgba64 converted = BLRgba64(BLRgba32(rgba32));
    return d.u64_data[0] == converted.value;
  }

  if (!d.has_object_signature()) {
    BLRgba converted = BLRgba(BLRgba32(rgba32));
    return memcmp(d.f32_data, &converted, 4 * sizeof(float)) == 0;
  }

  return false;
}

BL_API_IMPL bool BL_CDECL bl_var_equals_rgba64(const BLUnknown* self, uint64_t rgba64) noexcept {
  const BLObjectDetail& d = bl_as_object(self)->_d;

  if (d.is_rgba32()) {
    BLRgba64 converted = BLRgba64(BLRgba32(d.u32_data[0]));
    return converted.value == rgba64;
  }

  if (d.is_rgba64()) {
    return d.u64_data[0] == rgba64;
  }

  if (!d.has_object_signature()) {
    BLRgba rgba = BLRgba(BLRgba64(rgba64));
    return memcmp(d.f32_data, &rgba, 4 * sizeof(float)) == 0;
  }

  return false;
}

BL_API_IMPL bool bl_var_strict_equals(const BLUnknown* a, const BLUnknown* b) noexcept {
  const BLObjectDetail& a_d = bl_as_object(a)->_d;
  const BLObjectDetail& b_d = bl_as_object(b)->_d;

  return a_d == b_d;
}
