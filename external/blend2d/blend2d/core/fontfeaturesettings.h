// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTFEATURESETTINGS_H_INCLUDED
#define BLEND2D_FONTFEATURESETTINGS_H_INCLUDED

#include <blend2d/core/array.h>
#include <blend2d/core/bitset.h>
#include <blend2d/core/filesystem.h>
#include <blend2d/core/fontdefs.h>
#include <blend2d/core/geometry.h>
#include <blend2d/core/glyphbuffer.h>
#include <blend2d/core/object.h>
#include <blend2d/core/path.h>
#include <blend2d/core/string.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLFontFeatureSettings - C API
//! \{

//! Font feature settings [C API].
struct BLFontFeatureSettingsCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLFontFeatureSettings)
};

//! \cond INTERNAL
//! Font feature settings [C API Impl].
//!
//! \note This Impl's layout is fully compatible with \ref BLArrayImpl.
struct BLFontFeatureSettingsImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Pointer to feature items.
  BLFontFeatureItem* data;
  //! Number of feature items in `data`.
  size_t size;
  //! Capacity of `data`.
  size_t capacity;
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_font_feature_settings_init(BLFontFeatureSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_init_move(BLFontFeatureSettingsCore* self, BLFontFeatureSettingsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_init_weak(BLFontFeatureSettingsCore* self, const BLFontFeatureSettingsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_destroy(BLFontFeatureSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_reset(BLFontFeatureSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_clear(BLFontFeatureSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_shrink(BLFontFeatureSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_assign_move(BLFontFeatureSettingsCore* self, BLFontFeatureSettingsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_assign_weak(BLFontFeatureSettingsCore* self, const BLFontFeatureSettingsCore* other) BL_NOEXCEPT_C;
BL_API size_t BL_CDECL bl_font_feature_settings_get_size(const BLFontFeatureSettingsCore* self) BL_NOEXCEPT_C;
BL_API size_t BL_CDECL bl_font_feature_settings_get_capacity(const BLFontFeatureSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_get_view(const BLFontFeatureSettingsCore* self, BLFontFeatureSettingsView* out) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_feature_settings_has_value(const BLFontFeatureSettingsCore* self, BLTag feature_tag) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_font_feature_settings_get_value(const BLFontFeatureSettingsCore* self, BLTag feature_tag) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_set_value(BLFontFeatureSettingsCore* self, BLTag feature_tag, uint32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_feature_settings_remove_value(BLFontFeatureSettingsCore* self, BLTag feature_tag) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_feature_settings_equals(const BLFontFeatureSettingsCore* a, const BLFontFeatureSettingsCore* b) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_text
//! \{

//! \name BLFontFeatureSettings - Constants
//! \{

//! A constant representing an invalid font feature value in font feature tag/value pair.
BL_DEFINE_CONST uint32_t BL_FONT_FEATURE_INVALID_VALUE = 0xFFFFFFFFu;

//! \}

//! \name BLFontFeatureSettings - Structs
//! \{

//! Associates a font feature tag with a value. Tag describes the feature (as provided by the font) and `value`
//! describes its value. Some features only allow boolean values 0 and 1 and some allow values up to 65535.
//! Values above 65535 are invalid, however, only \ref BL_FONT_FEATURE_INVALID_VALUE should be used as invalid
//! value in general.
//!
//! Registered OpenType features:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/featuretags
//!   - https://helpx.adobe.com/typekit/using/open-type-syntax.html
struct BLFontFeatureItem {
  //! \name Members
  //! \{

  //! Feature tag (32-bit).
  BLTag tag;

  //! Feature value.
  //!
  //! \note values greater than 65535 are invalid.
  uint32_t value;

  //! \}

#ifdef __cplusplus
  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFontFeatureItem{}; }

  //! \}
#endif
};

//! A view unifying the representation of an internal storage used by \ref BLFontFeatureSettings.
struct BLFontFeatureSettingsView {
  //! \name Members
  //! \{

  //! Pointer to font feature items, where each item describes a tag and its value.
  //!
  //! \note If the container is in SSO mode the `data` member will point to `sso_data`.
  const BLFontFeatureItem* data;
  //! Count of items in `data.
  size_t size;
  //! Unpacked SSO items into \ref BLFontFeatureItem array.
  //!
  //! \note This member won't be initialized or zeroed in case \ref BLFontFeatureSettings is not in SSO mode. And if the
  //! container is in SSO mode only the number of items used will be overwritten by \ref BLFontFeatureSettings::get_view().
  BLFontFeatureItem sso_data[36];

  //! \}

#if defined(__cplusplus)
  //! \name C++ Iterator Compatibility
  //! \{

  //! Tests whether the view is empty.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size == 0; }

   //! Returns a const pointer to \ref BLFontFeatureSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontFeatureItem* begin() const noexcept { return data; }

  //! Returns a const pointer to the end of \ref BLFontFeatureSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontFeatureItem* end() const noexcept { return data + size; }

   //! Returns a const pointer to \ref BLFontFeatureSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontFeatureItem* cbegin() const noexcept { return data; }

  //! Returns a const pointer to the end of \ref BLFontFeatureSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontFeatureItem* cend() const noexcept { return data + size; }

  //! \}
#endif
};

//! \}

//! \name BLFontFeatureSettings - C++ API
//! \{

#ifdef __cplusplus

//! Font feature settings [C++ API].
class BLFontFeatureSettings final : public BLFontFeatureSettingsCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  //! SSO capacity of \ref BLFontFeatureSettings container.
  static inline constexpr uint32_t kSSOCapacity = 36u;

  //! Signature of an empty font feature settings.
  static inline constexpr uint32_t kSSOEmptySignature = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS);

  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontFeatureSettingsImpl* _impl() const noexcept { return static_cast<BLFontFeatureSettingsImpl*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLFontFeatureSettings() noexcept {
    _d.init_static(BLObjectInfo{kSSOEmptySignature});
    _d.u32_data[2] = 0xFFFFFFFFu;
  }

  BL_INLINE_NODEBUG BLFontFeatureSettings(BLFontFeatureSettings&& other) noexcept {
    _d = other._d;
    other._d.init_static(BLObjectInfo{kSSOEmptySignature});
    other._d.u32_data[2] = 0xFFFFFFFFu;
  }

  BL_INLINE_NODEBUG BLFontFeatureSettings(const BLFontFeatureSettings& other) noexcept {
    bl_font_feature_settings_init_weak(this, &other);
  }

  BL_INLINE_NODEBUG ~BLFontFeatureSettings() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_font_feature_settings_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG BLFontFeatureSettings& operator=(BLFontFeatureSettings&& other) noexcept { bl_font_feature_settings_assign_move(this, &other); return *this; }
  BL_INLINE_NODEBUG BLFontFeatureSettings& operator=(const BLFontFeatureSettings& other) noexcept { bl_font_feature_settings_assign_weak(this, &other); return *this; }

  BL_INLINE_NODEBUG bool operator==(const BLFontFeatureSettings& other) const noexcept { return equals(other); }
  BL_INLINE_NODEBUG bool operator!=(const BLFontFeatureSettings& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_font_feature_settings_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLFontFeatureSettings after reset.
    BL_ASSUME(_d.info.bits == kSSOEmptySignature);

    return result;
  }

  BL_INLINE_NODEBUG BLResult clear() noexcept { return bl_font_feature_settings_clear(this); }
  BL_INLINE_NODEBUG void swap(BLFontFeatureSettings& other) noexcept { _d.swap(other._d); }

  BL_INLINE_NODEBUG BLResult assign(BLFontFeatureSettings&& other) noexcept { return bl_font_feature_settings_assign_move(this, &other); }
  BL_INLINE_NODEBUG BLResult assign(const BLFontFeatureSettings& other) noexcept { return bl_font_feature_settings_assign_weak(this, &other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the container is empty, which means that no tag/value pairs are stored in it.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size() == 0; }

  //! Returns the number of feature tag/value pairs stored in the container.
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t size() const noexcept { return _d.sso() ? size_t(_d.info.a_field()) : _impl()->size; }

  //! Returns the container capacity
  //!
  //! \note If the container is in SSO mode, it would return the SSO capacity, however, such capacity can only be used
  //! for simple feature tag/value pairs. Some tags from these can only hold a boolean value (0 or 1) and ther others
  //! can hold a value from 0 to 15. So, if any tag/value pair requires a greater value than 15 it would never be able
  //! to use SSO representation.
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return _d.sso() ? size_t(kSSOCapacity) : _impl()->capacity; }

  //! Returns a normalized view of tag/value pairs as an iterable \ref BLFontFeatureItem array in the output view `out`.
  //!
  //! \note If the container is in SSO mode then all \ref BLFontFeatureItem values will be created from the underlying
  //! SSO representation and \ref BLFontFeatureSettingsView::data will point to \ref BLFontFeatureSettingsView::sso_data.
  //! If the container is dynamic, \ref BLFontFeatureSettingsView::sso_data won't be initialized and
  //! \ref BLFontFeatureSettingsView::data will point to the container's data. This means that the view cannot outlive
  //! the container, and also during iteration the view the container cannot be modified as that could invalidate the
  //! entire view.
  BL_INLINE_NODEBUG BLResult get_view(BLFontFeatureSettingsView* out) const noexcept { return bl_font_feature_settings_get_view(this, out); }

  //! Tests whether the settings contains the given `feature_tag`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_value(BLTag feature_tag) const noexcept { return bl_font_feature_settings_has_value(this, feature_tag); }

  //! Returns the value associated with the given `feature_tag`.
  //!
  //! If the `feature_tag` doesn't exist or is invalid \ref BL_FONT_FEATURE_INVALID_VALUE is returned.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t get_value(BLTag feature_tag) const noexcept { return bl_font_feature_settings_get_value(this, feature_tag); }

  //! Sets or inserts the given `feature_tag` to the settings, associating the `feature_tag` with `value`.
  //!
  //! The `feature_tag` must be valid, which means that it must contain 4 characters within ' ' to '~'
  //! range - [32, 126] in ASCII. If the given `feature_tag` is not valid or `value` is out of range
  //! (maximum value is `65535`) \ref BL_ERROR_INVALID_VALUE is returned.
  //!
  //! The following tags only support values that are either 0 (disabled) or 1 (enabled):
  //!
  //!   - 'case'
  //!   - 'clig'
  //!   - 'cpct'
  //!   - 'cpsp'
  //!   - 'dlig'
  //!   - 'dnom'
  //!   - 'expt'
  //!   - 'falt'
  //!   - 'frac'
  //!   - 'fwid'
  //!   - 'halt'
  //!   - 'hist'
  //!   - 'hwid'
  //!   - 'jalt'
  //!   - 'kern'
  //!   - 'liga'
  //!   - 'lnum'
  //!   - 'onum'
  //!   - 'ordn'
  //!   - 'palt'
  //!   - 'pcap'
  //!   - 'ruby'
  //!   - 'smcp'
  //!   - 'subs'
  //!   - 'sups'
  //!   - 'titl'
  //!   - 'tnam'
  //!   - 'tnum'
  //!   - 'unic'
  //!   - 'valt'
  //!   - 'vkrn'
  //!   - 'zero'
  //!
  //! Trying to use any other value with these tags would fail with \ref BL_ERROR_INVALID_VALUE error.
  BL_INLINE_NODEBUG BLResult set_value(BLTag feature_tag, uint32_t value) noexcept { return bl_font_feature_settings_set_value(this, feature_tag, value); }

  //! Removes the given `feature_tag` and its associated value from the settings.
  //!
  //! Nothing happens if the `feature_tag` is not in the settings (\ref BL_SUCCESS is returned).
  BL_INLINE_NODEBUG BLResult remove_value(BLTag feature_tag) noexcept { return bl_font_feature_settings_remove_value(this, feature_tag); }

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Tests whether this font feature settings is equal to `other` - equality means that it has the same tag/value pairs.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLFontFeatureSettings& other) const noexcept { return bl_font_feature_settings_equals(this, &other); }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_FONTFEATURESETTINGS_H_INCLUDED
