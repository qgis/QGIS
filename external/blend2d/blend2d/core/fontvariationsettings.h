// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTVARIATIONSETTINGS_H_INCLUDED
#define BLEND2D_FONTVARIATIONSETTINGS_H_INCLUDED

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

//! \name BLFontVariationSettings - C API
//! \{

//! Font variation settings [C API].
struct BLFontVariationSettingsCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLFontVariationSettings)
};

//! \cond INTERNAL
//! Font variation settings [C API Impl].
//!
//! \note This Impl's layout is fully compatible with \ref BLArrayImpl.
struct BLFontVariationSettingsImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Pointer to variation items.
  BLFontVariationItem* data;
  //! Number of variation items in `data`.
  size_t size;
  //! Capacity of `data`.
  size_t capacity;
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_font_variation_settings_init(BLFontVariationSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_init_move(BLFontVariationSettingsCore* self, BLFontVariationSettingsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_init_weak(BLFontVariationSettingsCore* self, const BLFontVariationSettingsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_destroy(BLFontVariationSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_reset(BLFontVariationSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_clear(BLFontVariationSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_shrink(BLFontVariationSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_assign_move(BLFontVariationSettingsCore* self, BLFontVariationSettingsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_assign_weak(BLFontVariationSettingsCore* self, const BLFontVariationSettingsCore* other) BL_NOEXCEPT_C;
BL_API size_t BL_CDECL bl_font_variation_settings_get_size(const BLFontVariationSettingsCore* self) BL_NOEXCEPT_C;
BL_API size_t BL_CDECL bl_font_variation_settings_get_capacity(const BLFontVariationSettingsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_get_view(const BLFontVariationSettingsCore* self, BLFontVariationSettingsView* out) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_variation_settings_has_value(const BLFontVariationSettingsCore* self, BLTag variation_tag) BL_NOEXCEPT_C;
BL_API float BL_CDECL bl_font_variation_settings_get_value(const BLFontVariationSettingsCore* self, BLTag variation_tag) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_set_value(BLFontVariationSettingsCore* self, BLTag variation_tag, float value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_variation_settings_remove_value(BLFontVariationSettingsCore* self, BLTag variation_tag) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_variation_settings_equals(const BLFontVariationSettingsCore* a, const BLFontVariationSettingsCore* b) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_text
//! \{

//! \name BLFontVariationSettings - Structs
//! \{

//! Associates a font variation tag with a value.
struct BLFontVariationItem {
  //! \name Members
  //! \{

  //! Variation tag (32-bit).
  BLTag tag;
  //! Variation value.
  //!
  //! \note values outside of [0, 1] range are invalid.
  float value;

  //! \}

#ifdef __cplusplus
  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFontVariationItem{}; }

  //! \}
#endif
};

//! A view unifying the representation of an internal storage used by \ref BLFontVariationSettings.
struct BLFontVariationSettingsView {
  //! Pointer to font variation items, where each item describes a variation tag and its value.
  //!
  //! \note If the container is in SSO mode the `data` member will point to `sso_data`.
  const BLFontVariationItem* data;

  //! Count of items in `data.
  size_t size;

  //! Unpacked SSO items into \ref BLFontVariationItem array.
  //!
  //! \note This member won't be initialized or zeroed in case \ref BLFontVariationSettings is not in
  //! SSO mode. And if the container is in SSO mode only the number of items used will be overwritten
  //! by \ref BLFontVariationSettings::get_view().
  BLFontVariationItem sso_data[3];

#if defined(__cplusplus)
  //! \name C++ Iterator Compatibility
  //! \{

  //! Tests whether the view is empty.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size == 0; }

   //! Returns a const pointer to \ref BLFontVariationSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontVariationItem* begin() const noexcept { return data; }

  //! Returns a const pointer to the end of \ref BLFontVariationSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontVariationItem* end() const noexcept { return data + size; }

   //! Returns a const pointer to \ref BLFontVariationSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontVariationItem* cbegin() const noexcept { return data; }

  //! Returns a const pointer to the end of \ref BLFontVariationSettingsView data (iterator compatibility).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontVariationItem* cend() const noexcept { return data + size; }

  //! \}
#endif
};

//! \}

//! \name BLFontVariationSettings - C++ API
//! \{
#ifdef __cplusplus

//! Font variation settings [C++ API].
class BLFontVariationSettings final : public BLFontVariationSettingsCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  //! SSO capacity of \ref BLFontVariationSettings container.
  static inline constexpr uint32_t kSSOCapacity = 3;

  //! Signature of SSO representation of an empty font variation settings.
  static inline constexpr uint32_t kSSOEmptySignature = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS);

  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontVariationSettingsImpl* _impl() const noexcept { return static_cast<BLFontVariationSettingsImpl*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLFontVariationSettings() noexcept {
    _d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  BL_INLINE_NODEBUG BLFontVariationSettings(BLFontVariationSettings&& other) noexcept {
    _d = other._d;
    other._d.init_static(BLObjectInfo{kSSOEmptySignature});
  }

  BL_INLINE_NODEBUG BLFontVariationSettings(const BLFontVariationSettings& other) noexcept {
    bl_font_variation_settings_init_weak(this, &other);
  }

  BL_INLINE_NODEBUG ~BLFontVariationSettings() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_font_variation_settings_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG BLFontVariationSettings& operator=(BLFontVariationSettings&& other) noexcept { bl_font_variation_settings_assign_move(this, &other); return *this; }
  BL_INLINE_NODEBUG BLFontVariationSettings& operator=(const BLFontVariationSettings& other) noexcept { bl_font_variation_settings_assign_weak(this, &other); return *this; }

  BL_INLINE_NODEBUG bool operator==(const BLFontVariationSettings& other) const noexcept { return  equals(other); }
  BL_INLINE_NODEBUG bool operator!=(const BLFontVariationSettings& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_font_variation_settings_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLFontVariationSettings after reset.
    BL_ASSUME(_d.info.bits == kSSOEmptySignature);

    return result;
  }

  BL_INLINE_NODEBUG BLResult clear() noexcept { return bl_font_variation_settings_clear(this); }
  BL_INLINE_NODEBUG void swap(BLFontVariationSettings& other) noexcept { _d.swap(other._d); }

  BL_INLINE_NODEBUG BLResult assign(BLFontVariationSettings&& other) noexcept { return bl_font_variation_settings_assign_move(this, &other); }
  BL_INLINE_NODEBUG BLResult assign(const BLFontVariationSettings& other) noexcept { return bl_font_variation_settings_assign_weak(this, &other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the container is empty, which means that no tag/value pairs are stored in it.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size() == 0; }

  //! Returns the number of tag/value pairs stored in the container.
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t size() const noexcept { return _d.sso() ? size_t(_d.info.a_field()) : _impl()->size; }

  //! Returns the container capacity.
  //!
  //! \note If the container is in SSO mode, it would return the SSO capacity, however, such capacity can only be used
  //! for simple tag/value pairs (where the tag is known by Blend2D and has associated an internal ID that represents it).
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return _d.sso() ? size_t(kSSOCapacity) : _impl()->capacity; }

  //! Returns a normalized view of tag/value pairs as an iterable \ref BLFontVariationItem array in the output view.
  //!
  //! \note If the container is in SSO mode then all \ref BLFontVariationItem values will be created from the
  //! underlying SSO representation and \ref BLFontVariationSettingsView::data will point to \ref
  //! BLFontVariationSettingsView::sso_data. If the container is dynamic, \ref BLFontVariationSettingsView::sso_data
  //! won't be initialized and \ref BLFontVariationSettingsView::data will point to the container's data. This means
  //! that the view cannot outlive the container, and also during iteration the view the container cannot be modified
  //! as that could invalidate the entire view.
  BL_INLINE_NODEBUG BLResult get_view(BLFontVariationSettingsView* out) const noexcept { return bl_font_variation_settings_get_view(this, out); }

  //! Tests whether the settings contains the given `variation_tag`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_value(BLTag variation_tag) const noexcept { return bl_font_variation_settings_has_value(this, variation_tag); }

  //! Returns the value associated with the given `variation_tag`.
  //!
  //! If the `variation_tag` doesn't exist or is invalid `NaN` is returned.
  [[nodiscard]]
  BL_INLINE_NODEBUG float get_value(BLTag variation_tag) const noexcept { return bl_font_variation_settings_get_value(this, variation_tag); }

  //! Sets or inserts the given `variation_tag` to the settings and associates it with the given `value`.
  BL_INLINE_NODEBUG BLResult set_value(BLTag variation_tag, float value) noexcept { return bl_font_variation_settings_set_value(this, variation_tag, value); }

  //! Removes the given `variation_tag` and its value from the settings.
  //!
  //! Nothing happens if the `variation_tag` is not in the settings (\ref BL_SUCCESS is returned).
  BL_INLINE_NODEBUG BLResult remove_value(BLTag variation_tag) noexcept { return bl_font_variation_settings_remove_value(this, variation_tag); }

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Tests whether this font variation settings is equal to `other` - equality means that it has the same tag/value pairs.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLFontVariationSettings& other) const noexcept { return bl_font_variation_settings_equals(this, &other); }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_FONTVARIATIONSETTINGS_H_INCLUDED
