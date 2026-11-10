// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/fontfeaturesettings_p.h>
#include <blend2d/core/fonttagdata_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl {
namespace FontFeatureSettingsInternal {

// bl::FontFeatureSettings - SSO Utilities
// =======================================

static BL_INLINE BLResult init_sso(BLFontFeatureSettingsCore* self, size_t size = 0) noexcept {
  self->_d.init_static(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS) | BLObjectInfo::from_abcp(uint32_t(size)));
  self->_d.u32_data[2] = kSSOInvalidFatFeaturePattern;
  return BL_SUCCESS;
}

static BL_INLINE size_t get_sso_size(const BLFontFeatureSettingsCore* self) noexcept {
  return self->_d.info.a_field();
}

static BL_INLINE void set_sso_size(BLFontFeatureSettingsCore* self, size_t size) noexcept {
  self->_d.info.set_a_field(uint32_t(size));
}

static BL_INLINE void add_sso_bit_tag(BLFontFeatureSettingsCore* self, uint32_t index, uint32_t value) noexcept {
  uint32_t bit = 1u << index;

  BL_ASSERT((self->_d.u32_data[0] & bit) == 0u);
  BL_ASSERT((self->_d.u32_data[1] & bit) == 0u);

  self->_d.u32_data[0] |= bit;
  self->_d.u32_data[1] |= value << index;
  self->_d.info.bits += 1u << BL_OBJECT_INFO_A_SHIFT;
}

static BL_INLINE void update_sso_bit_value(BLFontFeatureSettingsCore* self, uint32_t index, uint32_t value) noexcept {
  uint32_t bit = 1u << index;

  BL_ASSERT((self->_d.u32_data[0] & bit) != 0u);

  self->_d.u32_data[1] = (self->_d.u32_data[1] & ~bit) | (value << index);
}

static BL_INLINE void remove_sso_bit_tag(BLFontFeatureSettingsCore* self, uint32_t index) noexcept {
  uint32_t bit = 1u << index;

  BL_ASSERT(self->_d.info.a_field() > 0u);
  BL_ASSERT((self->_d.u32_data[0] & bit) != 0u);

  self->_d.u32_data[0] &= ~bit;
  self->_d.u32_data[1] &= ~bit;
  self->_d.info.bits -= 1u << BL_OBJECT_INFO_A_SHIFT;
}

static BL_INLINE void add_sso_fat_tag(BLFontFeatureSettingsCore* self, uint32_t index, uint32_t feature_id, uint32_t value) noexcept {
  BL_ASSERT(index < kSSOFatFeatureCount);
  BL_ASSERT(feature_id < kSSOInvalidFatFeatureId);
  BL_ASSERT(value <= kSSOFatFeatureValueBitMask);

  constexpr uint32_t kValueDataMask = (1u << (kSSOFatFeatureCount * kSSOFatFeatureValueBitCount)) - 1u;

  uint32_t tag_offset = index * kSSOFatFeatureTagBitCount;
  uint32_t val_offset = index * kSSOFatFeatureValueBitCount;

  uint32_t tags = self->_d.u32_data[2];
  uint32_t vals = self->_d.info.bits & kValueDataMask;

  uint32_t tags_lsb_mask = ((1u << tag_offset) - 1u);
  uint32_t vals_lsb_mask = ((1u << val_offset) - 1u);

  tags = (tags & tags_lsb_mask) | ((tags & ~tags_lsb_mask) << kSSOFatFeatureTagBitCount) | (feature_id << tag_offset);
  vals = (vals & vals_lsb_mask) | ((vals & ~vals_lsb_mask) << kSSOFatFeatureValueBitCount) | (value << val_offset);

  self->_d.u32_data[2] = tags;
  self->_d.info.bits = ((self->_d.info.bits & ~kValueDataMask) + (1u << BL_OBJECT_INFO_A_SHIFT)) | (vals & kValueDataMask);
}

static BL_INLINE void update_sso_fat_value(BLFontFeatureSettingsCore* self, uint32_t index, uint32_t value) noexcept {
  BL_ASSERT(index < kSSOFatFeatureCount);
  BL_ASSERT(value <= kSSOFatFeatureValueBitMask);

  uint32_t value_offset = index * kSSOFatFeatureValueBitCount;
  uint32_t mask = kSSOFatFeatureValueBitMask << value_offset;

  self->_d.info.bits = (self->_d.info.bits & ~mask) | value << value_offset;
}

static BL_INLINE void remove_sso_fat_tag(BLFontFeatureSettingsCore* self, uint32_t index) noexcept {
  BL_ASSERT(self->_d.info.a_field() > 0u);
  BL_ASSERT(index < kSSOFatFeatureCount);

  constexpr uint32_t kValueDataMask = (1u << (kSSOFatFeatureCount * kSSOFatFeatureValueBitCount)) - 1u;

  uint32_t tag_offset = index * kSSOFatFeatureTagBitCount;
  uint32_t val_offset = index * kSSOFatFeatureValueBitCount;

  uint32_t tags = self->_d.u32_data[2];
  uint32_t vals = self->_d.info.bits & kValueDataMask;

  uint32_t tags_lsb_mask = ((1u << tag_offset) - 1u);
  uint32_t vals_lsb_mask = ((1u << val_offset) - 1u);

  tags = (tags & tags_lsb_mask) | ((tags >> kSSOFatFeatureTagBitCount) & ~tags_lsb_mask) | (kSSOInvalidFatFeatureId << ((kSSOFatFeatureCount - 1) * kSSOFatFeatureTagBitCount));
  vals = (vals & vals_lsb_mask) | ((vals >> kSSOFatFeatureValueBitCount) & ~tags_lsb_mask);

  self->_d.u32_data[2] = tags;
  self->_d.info.bits = ((self->_d.info.bits & ~kValueDataMask) - (1u << BL_OBJECT_INFO_A_SHIFT)) | (vals & kValueDataMask);
}

static BL_INLINE bool can_insert_sso_fat_tag(const BLFontFeatureSettingsCore* self) noexcept {
  uint32_t last_id = self->_d.u32_data[2] >> ((kSSOFatFeatureCount - 1u) * kSSOFatFeatureTagBitCount);
  return last_id == kSSOInvalidFatFeatureId;
}

static bool convert_items_to_sso(BLFontFeatureSettingsCore* self, const BLFontFeatureItem* items, size_t size) noexcept {
  BL_ASSERT(size <= BLFontFeatureSettings::kSSOCapacity);

  uint32_t info_bits = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS) | BLObjectInfo::pack_abcp(uint32_t(size));

  uint32_t bit_tag_ids = 0;
  uint32_t bit_values = 0;

  uint32_t fat_index = 0;
  uint32_t fat_tag_ids = kSSOInvalidFatFeaturePattern;
  uint32_t fat_values = info_bits;

  for (size_t i = 0; i < size; i++) {
    uint32_t id = FontTagData::feature_tag_to_id(items[i].tag);
    uint32_t value = items[i].value;

    if (id == FontTagData::kInvalidId)
      return false;

    FeatureInfo feature_info = FontTagData::feature_info_table[id];
    if (feature_info.has_bit_id()) {
      if (value > 1u)
        return false;

      uint32_t bit_id = feature_info.bit_id;
      bit_tag_ids |= uint32_t(1) << bit_id;
      bit_values |= uint32_t(value) << bit_id;
    }
    else {
      if (value > kSSOFatFeatureValueBitMask || fat_index >= kSSOFatFeatureCount)
        return false;

      fat_tag_ids ^= (id ^ kSSOInvalidFatFeatureId) << (fat_index * kSSOFatFeatureTagBitCount);
      fat_values |= value << (fat_index * kSSOFatFeatureValueBitCount);
      fat_index++;
    }
  }

  self->_d.u32_data[0] = bit_tag_ids;
  self->_d.u32_data[1] = bit_values;
  self->_d.u32_data[2] = fat_tag_ids;
  self->_d.u32_data[3] = fat_values;

  return true;
}

static void convert_sso_to_items(const BLFontFeatureSettingsCore* self, BLFontFeatureItem* items) noexcept {
  constexpr uint32_t kDummyFatTagId = 0xFFFFFFFFu;

  uint32_t bit_tag_ids = self->_d.u32_data[0];
  uint32_t bit_values = self->_d.u32_data[1];
  uint32_t fat_tag_ids = self->_d.u32_data[2];
  uint32_t fat_values = self->_d.info.bits;
  uint32_t fat_feature_tag_id = fat_tag_ids & kSSOFatFeatureTagBitMask;

  // Marks the end of fat tags (since we have removed one we don't have to check for the end, this is the end).
  fat_tag_ids >>= kSSOFatFeatureTagBitCount;
  fat_tag_ids |= kSSOInvalidFatFeatureId << ((kSSOFatFeatureCount - 1u) * kSSOFatFeatureTagBitCount);

  if (fat_feature_tag_id == kSSOInvalidFatFeatureId)
    fat_feature_tag_id = kDummyFatTagId;

  ParametrizedBitOps<BitOrder::kLSB, uint32_t>::BitIterator bit_iterator(bit_tag_ids);
  while (bit_iterator.has_next()) {
    uint32_t bit_index = bit_iterator.next();
    uint32_t bit_feature_tag_id = uint32_t(FontTagData::feature_bit_id_to_feature_id(bit_index));
    while (bit_feature_tag_id > fat_feature_tag_id) {
      *items++ = BLFontFeatureItem{FontTagData::feature_id_to_tag_table[fat_feature_tag_id], fat_values & kSSOFatFeatureValueBitMask};

      fat_feature_tag_id = fat_tag_ids & kSSOFatFeatureTagBitMask;
      if (fat_feature_tag_id == kSSOInvalidFatFeatureId)
        fat_feature_tag_id = kDummyFatTagId;

      fat_tag_ids >>= kSSOFatFeatureTagBitCount;
      fat_values >>= kSSOFatFeatureValueBitCount;
    }

    *items++ = BLFontFeatureItem{FontTagData::feature_id_to_tag_table[bit_feature_tag_id], (bit_values >> bit_index) & 0x1u};
  }

  if (fat_feature_tag_id == kDummyFatTagId)
    return;

  do {
    *items++ = BLFontFeatureItem{FontTagData::feature_id_to_tag_table[fat_feature_tag_id], fat_values & kSSOFatFeatureValueBitMask};
    fat_feature_tag_id = fat_tag_ids & kSSOFatFeatureTagBitMask;
    fat_tag_ids >>= kSSOFatFeatureTagBitCount;
    fat_values >>= kSSOFatFeatureValueBitCount;
  } while (fat_feature_tag_id != kSSOInvalidFatFeatureId);
}

// bl::FontFeatureSettings - Impl Utilities
// ========================================

static BL_INLINE constexpr size_t get_maximum_size() noexcept {
  return FontTagData::kUniqueTagCount;
}

static BL_INLINE BLObjectImplSize expand_impl_size(BLObjectImplSize impl_size) noexcept {
  return bl_object_expand_impl_size(impl_size);
}

static BL_INLINE BLResult init_dynamic(BLFontFeatureSettingsCore* self, BLObjectImplSize impl_size, size_t size = 0u) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontFeatureSettingsImpl>(self, info, impl_size));

  BLFontFeatureSettingsImpl* impl = get_impl(self);
  BLFontFeatureItem* items = PtrOps::offset<BLFontFeatureItem>(impl, sizeof(BLFontFeatureSettingsImpl));

  impl->data = items;
  impl->size = size;
  impl->capacity = capacity_from_impl_size(impl_size);

  BL_ASSERT(size <= impl->capacity);
  return BL_SUCCESS;
}

static BL_NOINLINE BLResult init_dynamic_from_sso(BLFontFeatureSettingsCore* self, BLObjectImplSize impl_size, const BLFontFeatureSettingsCore* sso_map) noexcept {
  size_t size = get_sso_size(sso_map);
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontFeatureSettingsImpl>(self, info, impl_size));

  BLFontFeatureSettingsImpl* impl = get_impl(self);
  BLFontFeatureItem* items = PtrOps::offset<BLFontFeatureItem>(impl, sizeof(BLFontFeatureSettingsImpl));

  impl->data = items;
  impl->size = size;
  impl->capacity = capacity_from_impl_size(impl_size);

  BL_ASSERT(size <= impl->capacity);
  convert_sso_to_items(sso_map, items);

  return BL_SUCCESS;
}

static BL_NOINLINE BLResult init_dynamic_from_data(BLFontFeatureSettingsCore* self, BLObjectImplSize impl_size, const BLFontFeatureItem* src, size_t size) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontFeatureSettingsImpl>(self, info, impl_size));

  BLFontFeatureSettingsImpl* impl = get_impl(self);
  BLFontFeatureItem* items = PtrOps::offset<BLFontFeatureItem>(impl, sizeof(BLFontFeatureSettingsImpl));

  impl->data = items;
  impl->size = size;
  impl->capacity = capacity_from_impl_size(impl_size);

  BL_ASSERT(size <= impl->capacity);
  memcpy(items, src, size * sizeof(BLFontFeatureItem));

  return BL_SUCCESS;
}

} // {FontFeatureSettingsInternal}
} // {bl}

// bl::FontFeatureSettings - API - Init & Destroy
// ==============================================

BL_API_IMPL BLResult bl_font_feature_settings_init(BLFontFeatureSettingsCore* self) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  return init_sso(self);
}

BL_API_IMPL BLResult bl_font_feature_settings_init_move(BLFontFeatureSettingsCore* self, BLFontFeatureSettingsCore* other) noexcept {
  using namespace bl::FontFeatureSettingsInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_feature_settings());

  self->_d = other->_d;
  return init_sso(other);
}

BL_API_IMPL BLResult bl_font_feature_settings_init_weak(BLFontFeatureSettingsCore* self, const BLFontFeatureSettingsCore* other) noexcept {
  using namespace bl::FontFeatureSettingsInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_feature_settings());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_font_feature_settings_destroy(BLFontFeatureSettingsCore* self) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  return release_instance(self);
}

// bl::FontFeatureSettings - API - Reset & Clear
// =============================================

BL_API_IMPL BLResult bl_font_feature_settings_reset(BLFontFeatureSettingsCore* self) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  release_instance(self);
  return init_sso(self);
}

BL_API_IMPL BLResult bl_font_feature_settings_clear(BLFontFeatureSettingsCore* self) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  if (self->_d.sso())
    return init_sso(self);

  BLFontFeatureSettingsImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    self_impl->size = 0;
    return BL_SUCCESS;
  }
  else {
    release_instance(self);
    return init_sso(self);
  }
}

// bl::FontFeatureSettings - API - Shrink
// ======================================

BL_API_IMPL BLResult bl_font_feature_settings_shrink(BLFontFeatureSettingsCore* self) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  if (self->_d.sso())
    return BL_SUCCESS;

  BLFontFeatureSettingsImpl* self_impl = get_impl(self);
  BLFontFeatureItem* items = self_impl->data;
  size_t size = self_impl->size;

  BLFontFeatureSettingsCore tmp;
  if (size <= BLFontFeatureSettings::kSSOCapacity && convert_items_to_sso(&tmp, items, size))
    return replace_instance(self, &tmp);

  BLObjectImplSize current_size = impl_size_from_capacity(self_impl->capacity);
  BLObjectImplSize shrunk_size = impl_size_from_capacity(self_impl->size);

  if (shrunk_size + BL_OBJECT_IMPL_ALIGNMENT > current_size)
    return BL_SUCCESS;

  BL_PROPAGATE(init_dynamic_from_data(&tmp, shrunk_size, items, size));
  return replace_instance(self, &tmp);
}

// bl::FontFeatureSettings - API - Assign
// ======================================

BL_API_IMPL BLResult bl_font_feature_settings_assign_move(BLFontFeatureSettingsCore* self, BLFontFeatureSettingsCore* other) noexcept {
  using namespace bl::FontFeatureSettingsInternal;

  BL_ASSERT(self->_d.is_font_feature_settings());
  BL_ASSERT(other->_d.is_font_feature_settings());

  BLFontFeatureSettingsCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS]._d;
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_font_feature_settings_assign_weak(BLFontFeatureSettingsCore* self, const BLFontFeatureSettingsCore* other) noexcept {
  using namespace bl::FontFeatureSettingsInternal;

  BL_ASSERT(self->_d.is_font_feature_settings());
  BL_ASSERT(other->_d.is_font_feature_settings());

  retain_instance(other);
  return replace_instance(self, other);
}

// bl::FontFeatureSettings - API - Accessors
// =========================================

BL_API_IMPL size_t bl_font_feature_settings_get_size(const BLFontFeatureSettingsCore* self) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  if (self->_d.sso())
    return get_sso_size(self);
  else
    return get_impl(self)->size;
}

BL_API_IMPL size_t bl_font_feature_settings_get_capacity(const BLFontFeatureSettingsCore* self) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  if (self->_d.sso())
    return BLFontFeatureSettings::kSSOCapacity;
  else
    return get_impl(self)->capacity;
}

BL_API_IMPL BLResult bl_font_feature_settings_get_view(const BLFontFeatureSettingsCore* self, BLFontFeatureSettingsView* out) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    BLFontFeatureItem* items = out->sso_data;
    size_t size = get_sso_size(self);

    out->data = items;
    out->size = size;

    if (!size)
      return BL_SUCCESS;

    convert_sso_to_items(self, items);
    return BL_SUCCESS;
  }

  // Dynamic Mode
  // ------------

  const BLFontFeatureSettingsImpl* self_impl = get_impl(self);
  out->data = self_impl->data;
  out->size = self_impl->size;
  return BL_SUCCESS;
}

BL_API_IMPL bool bl_font_feature_settings_has_value(const BLFontFeatureSettingsCore* self, BLTag feature_tag) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    uint32_t id = bl::FontTagData::feature_tag_to_id(feature_tag);
    if (id == bl::FontTagData::kInvalidId)
      return false;

    FeatureInfo feature_info = bl::FontTagData::feature_info_table[id];
    if (feature_info.has_bit_id()) {
      return has_sso_bit_tag(self, feature_info.bit_id);
    }
    else {
      uint32_t dummy_index;
      return find_sso_fat_tag(self, feature_info.bit_id, &dummy_index);
    }
  }

  // Dynamic Mode
  // ------------

  const BLFontFeatureSettingsImpl* self_impl = get_impl(self);
  const BLFontFeatureItem* data = self_impl->data;

  size_t size = self_impl->size;
  size_t index = bl::lower_bound(data, self_impl->size, feature_tag, [](const BLFontFeatureItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  return index < size && data[index].tag == feature_tag;
}

BL_API_IMPL uint32_t bl_font_feature_settings_get_value(const BLFontFeatureSettingsCore* self, BLTag feature_tag) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  if (self->_d.sso())
    return get_sso_tag_value(self, feature_tag);
  else
    return get_dynamic_tag_value(self, feature_tag);
}

BL_API_IMPL BLResult bl_font_feature_settings_set_value(BLFontFeatureSettingsCore* self, BLTag feature_tag, uint32_t value) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  if (BL_UNLIKELY(value > 65535u))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  uint32_t feature_id = bl::FontTagData::feature_tag_to_id(feature_tag);
  bool can_modify = true;

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    size_t size = get_sso_size(self);

    if (feature_id != bl::FontTagData::kInvalidId) {
      FeatureInfo feature_info = bl::FontTagData::feature_info_table[feature_id];
      if (feature_info.has_bit_id()) {
        if (value > 1u)
          return bl_make_error(BL_ERROR_INVALID_VALUE);

        uint32_t feature_bit_id = feature_info.bit_id;
        if (has_sso_bit_tag(self, feature_bit_id)) {
          update_sso_bit_value(self, feature_bit_id, value);
          return BL_SUCCESS;
        }
        else {
          add_sso_bit_tag(self, feature_bit_id, value);
          return BL_SUCCESS;
        }
      }
      else if (value <= kSSOFatFeatureTagBitMask) {
        uint32_t index;
        if (find_sso_fat_tag(self, feature_id, &index)) {
          update_sso_fat_value(self, index, value);
          return BL_SUCCESS;
        }
        else if (can_insert_sso_fat_tag(self)) {
          add_sso_fat_tag(self, index, feature_id, value);
          return BL_SUCCESS;
        }
      }
    }
    else {
      if (BL_UNLIKELY(!bl::FontTagData::is_valid_tag(feature_tag)))
        return bl_make_error(BL_ERROR_INVALID_VALUE);
    }

    // Turn the SSO settings to dynamic settings, because some (or multiple) cases below are true:
    //   a) The `feature_tag` doesn't have a corresponding feature id, thus it cannot be used in SSO mode.
    //   b) The `value` is not either 0 or 1.
    //   c) There is no room in SSO storage to insert another tag/value pair.
    BLObjectImplSize impl_size = bl_object_align_impl_size(impl_size_from_capacity(bl_max<size_t>(size + 1, 4u)));
    BLFontFeatureSettingsCore tmp;

    // NOTE: This will turn the SSO settings into dynamic settings - it's guaranteed that all further operations will succeed.
    BL_PROPAGATE(init_dynamic_from_sso(&tmp, impl_size, self));
    *self = tmp;
  }
  else {
    if (BL_UNLIKELY(!bl::FontTagData::is_valid_tag(feature_tag)))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    can_modify = is_impl_mutable(get_impl(self));
  }

  // Dynamic Mode
  // ------------

  BLFontFeatureSettingsImpl* self_impl = get_impl(self);
  BLFontFeatureItem* items = self_impl->data;

  size_t size = self_impl->size;
  size_t index = bl::lower_bound(items, size, feature_tag, [](const BLFontFeatureItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  // Overwrite the value if `feature_tag` is already in the settings.
  if (index < size && items[index].tag == feature_tag) {
    if (items[index].value == value)
      return BL_SUCCESS;

    if (can_modify) {
      items[index].value = value;
      return BL_SUCCESS;
    }
    else {
      BLFontFeatureSettingsCore tmp;
      BL_PROPAGATE(init_dynamic_from_data(&tmp, impl_size_from_capacity(size), items, size));
      get_impl(&tmp)->data[index].value = value;
      return replace_instance(self, &tmp);
    }
  }

  if (BL_UNLIKELY(!bl::FontTagData::is_valid_tag(feature_tag)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  // Insert a new tag/value pair if `feature_tag` is not in the settings.
  size_t nTagsAfterIndex = size - index;
  if (can_modify && self_impl->capacity > size) {
    bl::MemOps::copy_backward_inline_t(items + index + 1, items + index, nTagsAfterIndex);
    items[index] = BLFontFeatureItem{feature_tag, value};
    self_impl->size = size + 1;
    return BL_SUCCESS;
  }
  else {
    BLFontFeatureSettingsCore tmp;
    BL_PROPAGATE(init_dynamic(&tmp, expand_impl_size(impl_size_from_capacity(size + 1)), size + 1));

    BLFontFeatureItem* dst = get_impl(&tmp)->data;
    bl::MemOps::copy_forward_inline_t(dst, items, index);
    dst[index] = BLFontFeatureItem{feature_tag, value};
    bl::MemOps::copy_forward_inline_t(dst + index + 1, items + index, nTagsAfterIndex);

    return replace_instance(self, &tmp);
  }
}

BL_API_IMPL BLResult bl_font_feature_settings_remove_value(BLFontFeatureSettingsCore* self, BLTag feature_tag) noexcept {
  using namespace bl::FontFeatureSettingsInternal;
  BL_ASSERT(self->_d.is_font_feature_settings());

  // SSO Mode
  // --------

  if (self->_d.sso()) {
    uint32_t feature_id = bl::FontTagData::feature_tag_to_id(feature_tag);
    if (feature_id == bl::FontTagData::kInvalidId)
      return BL_SUCCESS;

    FeatureInfo feature_info = bl::FontTagData::feature_info_table[feature_id];
    if (feature_info.has_bit_id()) {
      uint32_t feature_bit_id = feature_info.bit_id;
      if (!has_sso_bit_tag(self, feature_bit_id))
        return BL_SUCCESS;

      remove_sso_bit_tag(self, feature_bit_id);
      return BL_SUCCESS;
    }
    else {
      uint32_t index;
      if (!find_sso_fat_tag(self, feature_id, &index))
        return BL_SUCCESS;

      remove_sso_fat_tag(self, index);
      return BL_SUCCESS;
    }
  }

  // Dynamic Mode
  // ------------

  BLFontFeatureSettingsImpl* self_impl = get_impl(self);
  BLFontFeatureItem* items = self_impl->data;

  size_t size = self_impl->size;
  size_t index = bl::lower_bound(items, self_impl->size, feature_tag, [](const BLFontFeatureItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  if (index >= size || items[index].tag != feature_tag)
    return BL_SUCCESS;

  if (is_impl_mutable(self_impl)) {
    self_impl->size = size - 1;
    bl::MemOps::copy_forward_inline_t(items + index, items + index + 1, size - index - 1);
    return BL_SUCCESS;
  }
  else {
    BLFontFeatureSettingsCore tmp;
    BL_PROPAGATE(init_dynamic(&tmp, expand_impl_size(impl_size_from_capacity(size - 1)), size - 1));

    BLFontFeatureItem* dst = get_impl(&tmp)->data;
    bl::MemOps::copy_forward_inline_t(dst, items, index);
    bl::MemOps::copy_forward_inline_t(dst + index, items + index + 1, size - index - 1);

    return replace_instance(self, &tmp);
  }
}

// bl::FontFeatureSettings - API - Equals
// ======================================

BL_API_IMPL bool bl_font_feature_settings_equals(const BLFontFeatureSettingsCore* a, const BLFontFeatureSettingsCore* b) noexcept {
  using namespace bl::FontFeatureSettingsInternal;

  BL_ASSERT(a->_d.is_font_feature_settings());
  BL_ASSERT(b->_d.is_font_feature_settings());

  if (a->_d == b->_d)
    return true;

  if (a->_d.sso() == b->_d.sso()) {
    // Both are SSO: They must be binary equal, if not, they are not equal.
    if (a->_d.sso())
      return false;

    // Both are dynamic.
    const BLFontFeatureSettingsImpl* a_impl = get_impl(a);
    const BLFontFeatureSettingsImpl* b_impl = get_impl(b);

    size_t size = a_impl->size;
    if (size != b_impl->size)
      return false;

    return memcmp(a_impl->data, b_impl->data, size * sizeof(BLFontFeatureItem)) == 0;
  }
  else {
    // One is SSO and one is dynamic, make `a` the SSO one.
    if (b->_d.sso())
      BLInternal::swap(a, b);

    const BLFontFeatureSettingsImpl* b_impl = get_impl(b);
    size_t size = get_sso_size(a);

    if (size != b_impl->size)
      return false;

    // NOTE: Since SSO representation is not that trivial, just try to convert B impl to SSO representation
    // and then try binary equality of two SSO instances. If B is not convertible, then A and B are not equal.
    BLFontFeatureSettingsCore bSSO;
    const BLFontFeatureItem* b_items = b_impl->data;

    BL_ASSERT(size <= BLFontFeatureSettings::kSSOCapacity);
    if (!convert_items_to_sso(&bSSO, b_items, size))
      return false;

    return a->_d == bSSO._d;
  }
}

// bl::FontFeatureSettings - Runtime Registration
// ==============================================

void bl_font_feature_settings_rt_init(BLRuntimeContext* rt) noexcept {
  using namespace bl::FontFeatureSettingsInternal;

  bl_unused(rt);

  // Initialize BLFontFeatureSettings.
  init_sso(static_cast<BLFontFeatureSettingsCore*>(&bl_object_defaults[BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS]));
}
