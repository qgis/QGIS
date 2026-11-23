// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/fonttagdata_p.h>
#include <blend2d/core/fontvariationsettings_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl {
namespace FontVariationSettingsInternal {

// bl::FontVariationSettings - SSO Utilities
// =========================================

//! A constant that can be used to increment / decrement a size in SSO representation.
static constexpr uint32_t kSSOSizeIncrement = (1u << BL_OBJECT_INFO_A_SHIFT);

//! Number of bits that represents a variation id in SSO mode.
static constexpr uint32_t kSSOTagBitSize = 5u;

//! Mask of a single SSO tag value (id).
static constexpr uint32_t kSSOTagBitMask = (1u << kSSOTagBitSize) - 1;

static BL_INLINE BLResult init_sso(BLFontVariationSettingsCore* self, size_t size = 0) noexcept {
  self->_d.init_static(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS) |
                      BLObjectInfo::from_abcp(uint32_t(size)));
  return BL_SUCCESS;
}

static BL_INLINE size_t get_sso_size(const BLFontVariationSettingsCore* self) noexcept { return self->_d.info.a_field(); }
static BL_INLINE void set_sso_size(BLFontVariationSettingsCore* self, size_t size) noexcept { self->_d.info.set_a_field(uint32_t(size)); }

static BL_INLINE float get_sso_value_at(const BLFontVariationSettingsCore* self, size_t index) noexcept {return self->_d.f32_data[index]; }

static BL_INLINE BLResult set_sso_value_at(BLFontVariationSettingsCore* self, size_t index, float value) noexcept {
  self->_d.f32_data[index] = value;
  return BL_SUCCESS;
}

static BL_INLINE bool find_sso_tag(const BLFontVariationSettingsCore* self, uint32_t id, size_t* index_out) noexcept {
  uint32_t sso_bits = self->_d.info.bits;
  size_t size = get_sso_size(self);

  size_t i = 0;
  for (i = 0; i < size; i++, sso_bits >>= kSSOTagBitSize) {
    uint32_t sso_id = sso_bits & kSSOTagBitMask;
    if (sso_id < id)
      continue;
    *index_out = i;
    return id == sso_id;
  }

  *index_out = i;
  return false;
}

static bool convert_items_to_sso(BLFontVariationSettingsCore* dst, const BLFontVariationItem* items, size_t size) noexcept {
  BL_ASSERT(size <= BLFontVariationSettings::kSSOCapacity);

  init_sso(dst, size);

  uint32_t id_shift = 0;
  uint32_t sso_bits = 0;
  float* sso_values = dst->_d.f32_data;

  for (size_t i = 0; i < size; i++, id_shift += kSSOTagBitSize) {
    uint32_t id = FontTagData::variation_tag_to_id(items[i].tag);
    float value = items[i].value;

    if (id == FontTagData::kInvalidId)
      return false;

    sso_bits |= id << id_shift;
    sso_values[i] = value;
  }

  dst->_d.info.bits |= sso_bits;
  return true;
}

// bl::FontVariationSettings - Impl Utilities
// ==========================================

static BL_INLINE constexpr size_t get_maximum_size() noexcept {
  return FontTagData::kUniqueTagCount;
}

static BL_INLINE BLObjectImplSize expand_impl_size(BLObjectImplSize impl_size) noexcept {
  return bl_object_expand_impl_size(impl_size);
}

static BL_INLINE BLResult init_dynamic(BLFontVariationSettingsCore* self, BLObjectImplSize impl_size, size_t size = 0u) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontVariationSettingsImpl>(self, info, impl_size));

  BLFontVariationSettingsImpl* impl = get_impl(self);
  BLFontVariationItem* items = PtrOps::offset<BLFontVariationItem>(impl, sizeof(BLFontVariationSettingsImpl));

  impl->data = items;
  impl->size = size;
  impl->capacity = capacity_from_impl_size(impl_size);

  BL_ASSERT(size <= impl->capacity);
  return BL_SUCCESS;
}

static BL_NOINLINE BLResult init_dynamic_from_sso(BLFontVariationSettingsCore* self, BLObjectImplSize impl_size, const BLFontVariationSettingsCore* sso_map) noexcept {
  size_t size = get_sso_size(sso_map);
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontVariationSettingsImpl>(self, info, impl_size));

  BLFontVariationSettingsImpl* impl = get_impl(self);
  BLFontVariationItem* items = PtrOps::offset<BLFontVariationItem>(impl, sizeof(BLFontVariationSettingsImpl));

  impl->data = items;
  impl->size = size;
  impl->capacity = capacity_from_impl_size(impl_size);

  BL_ASSERT(size <= impl->capacity);
  uint32_t sso_bits = sso_map->_d.info.bits;

  const float* sso_values = sso_map->_d.f32_data;
  for (size_t i = 0; i < size; i++, sso_bits >>= kSSOTagBitSize)
    items[i] = BLFontVariationItem{FontTagData::variation_id_to_tag_table[sso_bits & kSSOTagBitMask], sso_values[i]};

  return BL_SUCCESS;
}

static BL_NOINLINE BLResult init_dynamic_from_data(BLFontVariationSettingsCore* self, BLObjectImplSize impl_size, const BLFontVariationItem* src, size_t size) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontVariationSettingsImpl>(self, info, impl_size));

  BLFontVariationSettingsImpl* impl = get_impl(self);
  BLFontVariationItem* items = PtrOps::offset<BLFontVariationItem>(impl, sizeof(BLFontVariationSettingsImpl));

  impl->data = items;
  impl->size = size;
  impl->capacity = capacity_from_impl_size(impl_size);

  BL_ASSERT(size <= impl->capacity);
  memcpy(items, src, size * sizeof(BLFontVariationItem));

  return BL_SUCCESS;
}

} // {FontVariationSettingsInternal}
} // {bl}

// bl::FontVariationSettings - API - Init & Destroy
// ================================================

BL_API_IMPL BLResult bl_font_variation_settings_init(BLFontVariationSettingsCore* self) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  return init_sso(self);
}

BL_API_IMPL BLResult bl_font_variation_settings_init_move(BLFontVariationSettingsCore* self, BLFontVariationSettingsCore* other) noexcept {
  using namespace bl::FontVariationSettingsInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_variation_settings());

  self->_d = other->_d;
  return init_sso(other);
}

BL_API_IMPL BLResult bl_font_variation_settings_init_weak(BLFontVariationSettingsCore* self, const BLFontVariationSettingsCore* other) noexcept {
  using namespace bl::FontVariationSettingsInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_variation_settings());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_font_variation_settings_destroy(BLFontVariationSettingsCore* self) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  return release_instance(self);
}

// bl::FontVariationSettings - API - Reset & Clear
// ===============================================

BL_API_IMPL BLResult bl_font_variation_settings_reset(BLFontVariationSettingsCore* self) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  release_instance(self);
  return init_sso(self);
}

BL_API_IMPL BLResult bl_font_variation_settings_clear(BLFontVariationSettingsCore* self) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  if (self->_d.sso())
    return init_sso(self);

  BLFontVariationSettingsImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    self_impl->size = 0;
    return BL_SUCCESS;
  }
  else {
    release_instance(self);
    return init_sso(self);
  }
}

// bl::FontVariationSettings - API - Shrink
// ========================================

BL_API_IMPL BLResult bl_font_variation_settings_shrink(BLFontVariationSettingsCore* self) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  if (self->_d.sso())
    return BL_SUCCESS;

  BLFontVariationSettingsImpl* self_impl = get_impl(self);
  BLFontVariationItem* items = self_impl->data;
  size_t size = self_impl->size;

  BLFontVariationSettingsCore tmp;
  if (size <= BLFontVariationSettings::kSSOCapacity && convert_items_to_sso(&tmp, items, size))
    return replace_instance(self, &tmp);

  BLObjectImplSize current_size = impl_size_from_capacity(self_impl->capacity);
  BLObjectImplSize shrunk_size = impl_size_from_capacity(self_impl->size);

  if (shrunk_size + BL_OBJECT_IMPL_ALIGNMENT > current_size)
    return BL_SUCCESS;

  BL_PROPAGATE(init_dynamic_from_data(&tmp, shrunk_size, items, size));
  return replace_instance(self, &tmp);
}

// bl::FontVariationSettings - API - Assign
// ========================================

BL_API_IMPL BLResult bl_font_variation_settings_assign_move(BLFontVariationSettingsCore* self, BLFontVariationSettingsCore* other) noexcept {
  using namespace bl::FontVariationSettingsInternal;

  BL_ASSERT(self->_d.is_font_variation_settings());
  BL_ASSERT(other->_d.is_font_variation_settings());

  BLFontVariationSettingsCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS]._d;
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_font_variation_settings_assign_weak(BLFontVariationSettingsCore* self, const BLFontVariationSettingsCore* other) noexcept {
  using namespace bl::FontVariationSettingsInternal;

  BL_ASSERT(self->_d.is_font_variation_settings());
  BL_ASSERT(other->_d.is_font_variation_settings());

  retain_instance(other);
  return replace_instance(self, other);
}

// bl::FontVariationSettings - API - Accessors
// ===========================================

BL_API_IMPL size_t bl_font_variation_settings_get_size(const BLFontVariationSettingsCore* self) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  if (self->_d.sso())
    return get_sso_size(self);
  else
    return get_impl(self)->size;
}

BL_API_IMPL size_t bl_font_variation_settings_get_capacity(const BLFontVariationSettingsCore* self) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  if (self->_d.sso())
    return BLFontVariationSettings::kSSOCapacity;
  else
    return get_impl(self)->capacity;
}

BL_API_IMPL BLResult bl_font_variation_settings_get_view(const BLFontVariationSettingsCore* self, BLFontVariationSettingsView* out) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    BLFontVariationItem* items = out->sso_data;
    size_t size = get_sso_size(self);

    uint32_t sso_bits = self->_d.info.bits;
    const float* sso_values = self->_d.f32_data;

    out->data = items;
    out->size = size;

    for (size_t i = 0; i < size; i++, sso_bits >>= kSSOTagBitSize)
      items[i] = BLFontVariationItem{bl::FontTagData::variation_id_to_tag_table[sso_bits & kSSOTagBitMask], sso_values[i]};

    return BL_SUCCESS;
  }

  // Dynamic Mode
  // ------------

  const BLFontVariationSettingsImpl* self_impl = get_impl(self);
  out->data = self_impl->data;
  out->size = self_impl->size;
  return BL_SUCCESS;
}

BL_API_IMPL bool bl_font_variation_settings_has_value(const BLFontVariationSettingsCore* self, BLTag variation_tag) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    uint32_t id = bl::FontTagData::variation_tag_to_id(variation_tag);
    if (id == bl::FontTagData::kInvalidId)
      return false;

    size_t index;
    return find_sso_tag(self, id, &index);
  }

  // Dynamic Mode
  // ------------

  const BLFontVariationSettingsImpl* self_impl = get_impl(self);
  const BLFontVariationItem* data = self_impl->data;

  size_t size = self_impl->size;
  size_t index = bl::lower_bound(data, self_impl->size, variation_tag, [](const BLFontVariationItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  return index < size && data[index].tag == variation_tag;
}

BL_API_IMPL float bl_font_variation_settings_get_value(const BLFontVariationSettingsCore* self, BLTag variation_tag) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    uint32_t id = bl::FontTagData::variation_tag_to_id(variation_tag);
    if (id == bl::FontTagData::kInvalidId)
      return bl::Math::nan<float>();

    size_t index;
    if (find_sso_tag(self, id, &index))
      return get_sso_value_at(self, index);
    else
      return bl::Math::nan<float>();
  }

  // Dynamic Mode
  // ------------

  const BLFontVariationSettingsImpl* self_impl = get_impl(self);
  const BLFontVariationItem* data = self_impl->data;

  size_t size = self_impl->size;
  size_t index = bl::lower_bound(data, self_impl->size, variation_tag, [](const BLFontVariationItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  if (index < size && data[index].tag == variation_tag)
    return data[index].value;
  else
    return bl::Math::nan<float>();
}

BL_API_IMPL BLResult bl_font_variation_settings_set_value(BLFontVariationSettingsCore* self, BLTag variation_tag, float value) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  if (BL_UNLIKELY(value > 65535u))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  // SSO Mode
  // --------

  bool can_modify = true;

  if (self->_d.sso()) {
    size_t size = get_sso_size(self);

    if (value <= 1) {
      uint32_t id = bl::FontTagData::variation_tag_to_id(variation_tag);
      if (id != bl::FontTagData::kInvalidId) {
        size_t index;
        if (find_sso_tag(self, id, &index)) {
          set_sso_value_at(self, index, value);
          return BL_SUCCESS;
        }

        if (size < BLFontVariationSettings::kSSOCapacity) {
          // Every inserted tag must be inserted in a way to make tags sorted and we know where to insert (index).
          float* sso_values = self->_d.f32_data;
          size_t nTagsAfterIndex = size - index;
          bl::MemOps::copy_backward_inline_t(sso_values + index + 1u, sso_values + index, nTagsAfterIndex);
          sso_values[index] = value;

          // Update the tag and object info - updates the size (increments one), adds a new tag, and shifts all ids after `index`.
          uint32_t sso_bits = self->_d.info.bits + kSSOSizeIncrement;
          uint32_t bit_index = uint32_t(index * kSSOTagBitSize);
          uint32_t tags_after_index_mask = ((1u << (nTagsAfterIndex * kSSOTagBitSize)) - 1u) << bit_index;
          self->_d.info.bits = (sso_bits & ~tags_after_index_mask) | ((sso_bits & tags_after_index_mask) << kSSOTagBitSize) | (id << bit_index);
          return BL_SUCCESS;
        }
      }
      else {
        if (BL_UNLIKELY(!bl::FontTagData::is_valid_tag(variation_tag)))
          return bl_make_error(BL_ERROR_INVALID_VALUE);
      }
    }

    // Turn the SSO settings to dynamic settings, because some (or multiple) cases below are true:
    //   a) The `tag` doesn't have a corresponding variation id, thus it cannot be used in SSO mode.
    //   b) There is no room in SSO storage to insert another tag/value pair.
    BLObjectImplSize impl_size = bl_object_align_impl_size(impl_size_from_capacity(bl_max<size_t>(size + 1, 4u)));
    BLFontVariationSettingsCore tmp;

    // NOTE: This will turn the SSO settings into a dynamic settings - it's guaranteed that all further operations will succeed.
    BL_PROPAGATE(init_dynamic_from_sso(&tmp, impl_size, self));
    *self = tmp;
  }
  else {
    if (BL_UNLIKELY(!bl::FontTagData::is_valid_tag(variation_tag)))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    can_modify = is_impl_mutable(get_impl(self));
  }

  // Dynamic Mode
  // ------------

  BLFontVariationSettingsImpl* self_impl = get_impl(self);
  BLFontVariationItem* items = self_impl->data;

  size_t size = self_impl->size;
  size_t index = bl::lower_bound(items, size, variation_tag, [](const BLFontVariationItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  // Overwrite the value if the `variation_tag` is already in the settings.
  if (index < size && items[index].tag == variation_tag) {
    if (items[index].value == value)
      return BL_SUCCESS;

    if (can_modify) {
      items[index].value = value;
      return BL_SUCCESS;
    }
    else {
      BLFontVariationSettingsCore tmp;
      BL_PROPAGATE(init_dynamic_from_data(&tmp, impl_size_from_capacity(size), items, size));
      get_impl(&tmp)->data[index].value = value;
      return replace_instance(self, &tmp);
    }
  }

  if (BL_UNLIKELY(!bl::FontTagData::is_valid_tag(variation_tag)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  // Insert a new variation tag if it's not in the settings.
  size_t nTagsAfterIndex = size - index;
  if (can_modify && self_impl->capacity > size) {
    bl::MemOps::copy_backward_inline_t(items + index + 1, items + index, nTagsAfterIndex);
    items[index] = BLFontVariationItem{variation_tag, value};
    self_impl->size = size + 1;
    return BL_SUCCESS;
  }
  else {
    BLFontVariationSettingsCore tmp;
    BL_PROPAGATE(init_dynamic(&tmp, expand_impl_size(impl_size_from_capacity(size + 1)), size + 1));

    BLFontVariationItem* dst = get_impl(&tmp)->data;
    bl::MemOps::copy_forward_inline_t(dst, items, index);
    dst[index] = BLFontVariationItem{variation_tag, value};
    bl::MemOps::copy_forward_inline_t(dst + index + 1, items + index, nTagsAfterIndex);

    return replace_instance(self, &tmp);
  }
}

BL_API_IMPL BLResult bl_font_variation_settings_remove_value(BLFontVariationSettingsCore* self, BLTag variation_tag) noexcept {
  using namespace bl::FontVariationSettingsInternal;
  BL_ASSERT(self->_d.is_font_variation_settings());

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    uint32_t id = bl::FontTagData::variation_tag_to_id(variation_tag);
    if (id == bl::FontTagData::kInvalidId)
      return BL_SUCCESS;

    size_t size = get_sso_size(self);
    size_t index;

    if (!find_sso_tag(self, id, &index))
      return BL_SUCCESS;

    size_t i = index;
    float* sso_values = self->_d.f32_data;

    while (i < size) {
      sso_values[i] = sso_values[i + 1];
      i++;
    }

    // Clear the value that has been removed. The reason for doing this is to make sure that two settings that have
    // the same SSO data would be binary equal (there would not be garbage in data after the size in SSO storage).
    sso_values[size - 1] = 0.0f;

    // Shift the bit data representing tags (ids) so they are in correct places  after the removal operation.
    uint32_t sso_bits = self->_d.info.bits;
    uint32_t bit_index = uint32_t(index * kSSOTagBitSize);
    uint32_t tags_to_shift = uint32_t(size - index - 1);
    uint32_t remaining_keys_after_index_mask = ((1u << (tags_to_shift * kSSOTagBitSize)) - 1u) << (bit_index + kSSOTagBitSize);

    self->_d.info.bits = (sso_bits & ~(BL_OBJECT_INFO_A_MASK | remaining_keys_after_index_mask | (kSSOTagBitMask << bit_index))) |
                         ((sso_bits & remaining_keys_after_index_mask) >> kSSOTagBitSize) |
                         (uint32_t(size - 1u) << BL_OBJECT_INFO_A_SHIFT);
    return BL_SUCCESS;
  }

  // Dynamic Mode
  // ------------

  BLFontVariationSettingsImpl* self_impl = get_impl(self);
  BLFontVariationItem* items = self_impl->data;

  size_t size = self_impl->size;
  size_t index = bl::lower_bound(items, self_impl->size, variation_tag, [](const BLFontVariationItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  if (index >= size || items[index].tag != variation_tag)
    return BL_SUCCESS;

  if (is_impl_mutable(self_impl)) {
    self_impl->size = size - 1;
    bl::MemOps::copy_forward_inline_t(items + index, items + index + 1, size - index - 1);
    return BL_SUCCESS;
  }
  else {
    BLFontVariationSettingsCore tmp;
    BL_PROPAGATE(init_dynamic(&tmp, expand_impl_size(impl_size_from_capacity(size - 1)), size - 1));

    BLFontVariationItem* dst = get_impl(&tmp)->data;
    bl::MemOps::copy_forward_inline_t(dst, items, index);
    bl::MemOps::copy_forward_inline_t(dst + index, items + index + 1, size - index - 1);

    return replace_instance(self, &tmp);
  }
}

// bl::FontVariationSettings - API - Equals
// ========================================

BL_API_IMPL bool bl_font_variation_settings_equals(const BLFontVariationSettingsCore* a, const BLFontVariationSettingsCore* b) noexcept {
  using namespace bl::FontVariationSettingsInternal;

  BL_ASSERT(a->_d.is_font_variation_settings());
  BL_ASSERT(b->_d.is_font_variation_settings());

  if (a->_d == b->_d)
    return true;

  if (a->_d.sso() == b->_d.sso()) {
    // Both are SSO: They must be binary equal, if not, they are not equal.
    if (a->_d.sso())
      return false;

    // Both are dynamic.
    const BLFontVariationSettingsImpl* a_impl = get_impl(a);
    const BLFontVariationSettingsImpl* b_impl = get_impl(b);

    size_t size = a_impl->size;
    if (size != b_impl->size)
      return false;

    return memcmp(a_impl->data, b_impl->data, size * sizeof(BLFontVariationItem)) == 0;
  }
  else {
    // One is SSO and one is dynamic, make `a` the SSO one.
    if (b->_d.sso())
      BLInternal::swap(a, b);

    const BLFontVariationSettingsImpl* b_impl = get_impl(b);
    size_t size = get_sso_size(a);

    if (size != b_impl->size)
      return false;

    uint32_t a_bits = a->_d.info.bits;
    const float* a_values = a->_d.f32_data;
    const BLFontVariationItem* b_items = b_impl->data;

    for (size_t i = 0; i < size; i++, a_bits >>= kSSOTagBitSize) {
      uint32_t a_tag = bl::FontTagData::variation_id_to_tag_table[a_bits & kSSOTagBitMask];
      float a_value = a_values[i];

      if (b_items[i].tag != a_tag || b_items[i].value != a_value)
        return false;
    }

    return true;
  }
}

// bl::FontVariationSettings - Runtime Registration
// ================================================

void bl_font_variation_settings_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  // Initialize BLFontVariationSettings.
  bl_object_defaults[BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS]._d.init_static(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS));
}
