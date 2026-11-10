// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTFEATURESETTINGS_P_H_INCLUDED
#define BLEND2D_FONTFEATURESETTINGS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/fontfeaturesettings.h>
#include <blend2d/core/fonttagdata_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/bitops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace FontFeatureSettingsInternal {

//! \name BLFontFeatureSettings - Internals - Common Functionality (Container)
//! \{

static BL_INLINE constexpr BLObjectImplSize impl_size_from_capacity(size_t capacity) noexcept {
  return BLObjectImplSize(sizeof(BLFontFeatureSettingsImpl) + capacity * sizeof(BLFontFeatureItem));
}

static BL_INLINE constexpr size_t capacity_from_impl_size(BLObjectImplSize impl_size) noexcept {
  return (impl_size.value() - sizeof(BLFontFeatureSettingsImpl)) / sizeof(BLFontFeatureItem);
}

//! \}

//! \name BLFontFeatureSettings - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLFontFeatureSettingsImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

static BL_INLINE BLResult free_impl(BLFontFeatureSettingsImpl* impl) noexcept {
  return ObjectInternal::free_impl(impl);
}

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLFontFeatureSettingsImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLFontFeatureSettings - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE BLFontFeatureSettingsImpl* get_impl(const BLFontFeatureSettingsCore* self) noexcept {
  return static_cast<BLFontFeatureSettingsImpl*>(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLFontFeatureSettingsCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLFontFeatureSettingsCore* self) noexcept {
  return self->_d.info.is_ref_counted_object() ? release_impl<RCMode::kForce>(get_impl(self)) : BLResult(BL_SUCCESS);
}

static BL_INLINE BLResult replace_instance(BLFontFeatureSettingsCore* self, const BLFontFeatureSettingsCore* other) noexcept {
  BLFontFeatureSettingsImpl* impl = get_impl(self);
  BLObjectInfo info = self->_d.info;

  self->_d = other->_d;
  return info.is_ref_counted_object() ? release_impl<RCMode::kForce>(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLFontFeatureSettings - Internals - SSO Fat Representation
//! \{

using FontTagData::FeatureInfo;
using FatBitOps = ParametrizedBitOps<BitOrder::kLSB, uint32_t>;

static constexpr uint32_t kSSOFatFeatureCount = 4;
static constexpr uint32_t kSSOFatFeatureTagBitCount = 8;
static constexpr uint32_t kSSOFatFeatureTagBitMask = (1u << kSSOFatFeatureTagBitCount) - 1u;
static constexpr uint32_t kSSOFatFeatureValueBitCount = 4;
static constexpr uint32_t kSSOFatFeatureValueBitMask = (1u << kSSOFatFeatureValueBitCount) - 1u;

// 'zero' is used by SSO, thus it can never be used in fat feature data.
static constexpr uint32_t kSSOInvalidFatFeatureId = 0xFFu;
// 32-bit pattern that is used to initialize SSO storage.
static constexpr uint32_t kSSOInvalidFatFeaturePattern = 0xFFFFFFFFu;

static BL_INLINE bool has_sso_bit_tag(const BLFontFeatureSettingsCore* self, uint32_t index) noexcept {
  return (self->_d.u32_data[0] >> index) & 0x1u;
}

static BL_INLINE uint32_t get_sso_bit_value(const BLFontFeatureSettingsCore* self, uint32_t index) noexcept {
  return (self->_d.u32_data[1] >> index) & 0x1u;
}

static BL_INLINE uint32_t get_sso_fat_value(const BLFontFeatureSettingsCore* self, uint32_t index) noexcept {
  return (self->_d.info.bits >> (index * kSSOFatFeatureValueBitCount)) & kSSOFatFeatureValueBitMask;
}

static BL_INLINE bool find_sso_fat_tag(const BLFontFeatureSettingsCore* self, uint32_t feature_id, uint32_t* index_out) noexcept {
  uint32_t tags = self->_d.u32_data[2];

  for (uint32_t index = 0; index < kSSOFatFeatureCount; index++, tags >>= kSSOFatFeatureTagBitCount) {
    uint32_t id = tags & kSSOFatFeatureTagBitMask;
    if (id == kSSOInvalidFatFeatureId || id >= feature_id) {
      *index_out = index;
      return id == feature_id;
    }
  }

  *index_out = kSSOFatFeatureCount;
  return false;
}

static BL_INLINE uint32_t get_sso_tag_value(const BLFontFeatureSettingsCore* self, BLTag feature_tag, uint32_t not_found_value = BL_FONT_FEATURE_INVALID_VALUE) noexcept {
  BL_ASSERT(self->_d.sso());

  uint32_t feature_id = FontTagData::feature_tag_to_id(feature_tag);
  if (feature_id == FontTagData::kInvalidId)
    return not_found_value;

  FontTagData::FeatureInfo feature_info = FontTagData::feature_info_table[feature_id];
  if (feature_info.has_bit_id()) {
    uint32_t feature_bit_id = feature_info.bit_id;
    if (!has_sso_bit_tag(self, feature_bit_id))
      return not_found_value;

    return get_sso_bit_value(self, feature_bit_id);
  }
  else {
    uint32_t index;
    if (!find_sso_fat_tag(self, feature_id, &index))
      return not_found_value;

    return get_sso_fat_value(self, index);
  }
}

static BL_INLINE uint32_t get_dynamic_tag_value(const BLFontFeatureSettingsCore* self, BLTag feature_tag, uint32_t not_found_value = BL_FONT_FEATURE_INVALID_VALUE) noexcept {
  const BLFontFeatureSettingsImpl* self_impl = get_impl(self);
  const BLFontFeatureItem* data = self_impl->data;

  size_t size = self_impl->size;
  size_t index = lower_bound(data, self_impl->size, feature_tag, [](const BLFontFeatureItem& item, uint32_t tag) noexcept { return item.tag < tag; });

  if (index < size && data[index].tag == feature_tag)
    return data[index].value;
  else
    return not_found_value;
}

template<bool kSSO>
static BL_INLINE uint32_t get_tag_value(const BLFontFeatureSettingsCore* self, BLTag feature_tag, uint32_t not_found_value = BL_FONT_FEATURE_INVALID_VALUE) noexcept {
  return kSSO ? get_sso_tag_value(self, feature_tag, not_found_value) : get_dynamic_tag_value(self, feature_tag, not_found_value);
}

template<bool kSSO>
static BL_INLINE bool is_feature_enabled_for_plan(const BLFontFeatureSettingsCore* self, BLTag feature_tag) noexcept {
  uint32_t feature_id = FontTagData::feature_tag_to_id(feature_tag);
  uint32_t feature_info_index = bl_min<uint32_t>(feature_id, FontTagData::kFeatureIdCount);
  const FontTagData::FeatureInfo& feature_info = FontTagData::feature_info_table[feature_info_index];

  return get_tag_value<kSSO>(self, feature_tag, feature_info.enabled_by_default) > 0u;
}

} // {FontFeatureSettingsInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FONTFEATURESETTINGS_P_H_INCLUDED
