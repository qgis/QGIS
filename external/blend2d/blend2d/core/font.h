// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONT_H_INCLUDED
#define BLEND2D_FONT_H_INCLUDED

#include <blend2d/core/array.h>
#include <blend2d/core/bitset.h>
#include <blend2d/core/filesystem.h>
#include <blend2d/core/fontdata.h>
#include <blend2d/core/fontdefs.h>
#include <blend2d/core/fontface.h>
#include <blend2d/core/fontfeaturesettings.h>
#include <blend2d/core/fontvariationsettings.h>
#include <blend2d/core/geometry.h>
#include <blend2d/core/glyphbuffer.h>
#include <blend2d/core/glyphrun.h>
#include <blend2d/core/object.h>
#include <blend2d/core/path.h>
#include <blend2d/core/string.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLFont - C API
//! \{

//! Font [C API].
struct BLFontCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLFont)
};

BL_BEGIN_C_DECLS

//! \cond INTERNAL
//! Font [C API Impl].
struct BLFontImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Font face used by this font.
  BLFontFaceCore face;

  //! Font width (1..1000) [0 if the font is not initialized].
  uint16_t weight;
  //! Font stretch (1..9) [0 if the font is not initialized].
  uint8_t stretch;
  //! Font style.
  uint8_t style;
  //! Reserved for future use.
  uint32_t reserved;
  //! Font metrics.
  BLFontMetrics metrics;
  //! Font matrix.
  BLFontMatrix matrix;

  //! Assigned font features (key/value pairs).
  BLFontFeatureSettingsCore feature_settings;
  //! Assigned font variations (key/value pairs).
  BLFontVariationSettingsCore variation_settings;
};
//! \endcond

BL_API BLResult BL_CDECL bl_font_init(BLFontCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_init_move(BLFontCore* self, BLFontCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_init_weak(BLFontCore* self, const BLFontCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_destroy(BLFontCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_reset(BLFontCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_assign_move(BLFontCore* self, BLFontCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_assign_weak(BLFontCore* self, const BLFontCore* other) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_equals(const BLFontCore* a, const BLFontCore* b) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_create_from_face(BLFontCore* self, const BLFontFaceCore* face, float size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_create_from_face_with_settings(BLFontCore* self, const BLFontFaceCore* face, float size, const BLFontFeatureSettingsCore* feature_settings, const BLFontVariationSettingsCore* variation_settings) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_face(const BLFontCore* self, BLFontFaceCore* out) BL_NOEXCEPT_C;
BL_API float BL_CDECL bl_font_get_size(const BLFontCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_set_size(BLFontCore* self, float size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_metrics(const BLFontCore* self, BLFontMetrics* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_matrix(const BLFontCore* self, BLFontMatrix* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_design_metrics(const BLFontCore* self, BLFontDesignMetrics* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_feature_settings(const BLFontCore* self, BLFontFeatureSettingsCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_set_feature_settings(BLFontCore* self, const BLFontFeatureSettingsCore* feature_settings) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_reset_feature_settings(BLFontCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_variation_settings(const BLFontCore* self, BLFontVariationSettingsCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_set_variation_settings(BLFontCore* self, const BLFontVariationSettingsCore* variation_settings) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_reset_variation_settings(BLFontCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_shape(const BLFontCore* self, BLGlyphBufferCore* gb) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_map_text_to_glyphs(const BLFontCore* self, BLGlyphBufferCore* gb, BLGlyphMappingState* state_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_position_glyphs(const BLFontCore* self, BLGlyphBufferCore* gb) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_apply_kerning(const BLFontCore* self, BLGlyphBufferCore* gb) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_apply_gsub(const BLFontCore* self, BLGlyphBufferCore* gb, const BLBitArrayCore* lookups) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_apply_gpos(const BLFontCore* self, BLGlyphBufferCore* gb, const BLBitArrayCore* lookups) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_text_metrics(const BLFontCore* self, BLGlyphBufferCore* gb, BLTextMetrics* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_glyph_bounds(const BLFontCore* self, const uint32_t* glyph_data, intptr_t glyph_advance, BLBoxI* out, size_t count) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_glyph_advances(const BLFontCore* self, const uint32_t* glyph_data, intptr_t glyph_advance, BLGlyphPlacement* out, size_t count) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_glyph_outlines(const BLFontCore* self, BLGlyphId glyph_id, const BLMatrix2D* user_transform, BLPathCore* out, BLPathSinkFunc sink, void* user_data) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_get_glyph_run_outlines(const BLFontCore* self, const BLGlyphRun* glyph_run, const BLMatrix2D* user_transform, BLPathCore* out, BLPathSinkFunc sink, void* user_data) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_text
//! \{

//! \name BLFont - C++ API
//! \{

#ifdef __cplusplus

//! Font [C++ API].
class BLFont final : public BLFontCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  //! Object info values of a default constructed BLFont.
  static inline constexpr uint32_t kDefaultSignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_FONT) | BL_OBJECT_INFO_D_FLAG;

  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontImpl* _impl() const noexcept { return static_cast<BLFontImpl*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Creates a default initialized font
  //!
  //! A default initialized font is not a valid font that could be used for rendering. It can be considered an empty
  //! or null font, which has no family, no glyphs, no tables, it's essentially empty.
  BL_INLINE_NODEBUG BLFont() noexcept {
    bl_font_init(this);

    // Assume a default constructed BLFont.
    BL_ASSUME(_d.info.bits == kDefaultSignature);
  }

  //! Move constructor moves the underlying representation of the `other` font into this newly created instance and
  //! resets the `other` font to a default constructed state.
  BL_INLINE_NODEBUG BLFont(BLFont&& other) noexcept {
    bl_font_init_move(this, &other);

    // Assume a default initialized `other`.
    BL_ASSUME(other._d.info.bits == kDefaultSignature);
  }

  //! Copy constructor makes a weak copy of the underlying representation of the `other` font.
  BL_INLINE_NODEBUG BLFont(const BLFont& other) noexcept {
    bl_font_init_weak(this, &other);
  }

  //! Destroys the font.
  BL_INLINE_NODEBUG ~BLFont() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_font_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Returns whether the font is valid, which means that it was constructed from a valid \ref BLFontFace.
  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return is_valid(); }

  BL_INLINE_NODEBUG BLFont& operator=(const BLFont& other) noexcept { bl_font_assign_weak(this, &other); return *this; }
  BL_INLINE_NODEBUG BLFont& operator=(BLFont&& other) noexcept { bl_font_assign_move(this, &other); return *this; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLFont& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLFont& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  //! Resets the font to a default constructed state.
  //!
  //! \note This operation always succeeds and returns \ref BL_SUCCESS.
  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_font_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLFont after reset.
    BL_ASSUME(_d.info.bits == kDefaultSignature);

    return result;
  }

  //! Swaps the underlying representation of this font with the `other` font.
  BL_INLINE_NODEBUG void swap(BLFont& other) noexcept { _d.swap(other._d); }

  //! Copy assignment creates a weak copy of the underlying representation of the `other` font and stores it in this
  //! font.
  BL_INLINE_NODEBUG BLResult assign(const BLFont& other) noexcept { return bl_font_assign_weak(this, &other); }

  //! Move assignment moves the underlying representation of the `other` font into this font and then resets the
  //! `other` font to a default constructed state.
  BL_INLINE_NODEBUG BLResult assign(BLFont&& other) noexcept { return bl_font_assign_move(this, &other); }

  //! Tests whether the font is a valid instance.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_valid() const noexcept { return _impl()->face.dcast().is_valid(); }

  //! Tests whether the font is empty, which is the same as `!is_valid()`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return !is_valid(); }

  //! Tests whether this and `other` fonts are equal.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLFontCore& other) const noexcept { return bl_font_equals(this, &other); }

  //! \}

  //! \name Create Functionality
  //! \{

  //! Creates a new font from the existing font `face` scaled to the given `size`.
  BL_INLINE_NODEBUG BLResult create_from_face(const BLFontFaceCore& face, float size) noexcept {
    return bl_font_create_from_face(this, &face, size);
  }

  //! Creates a new font from the existing font `face` scaled to the given `size`.
  //!
  //! This is an overloaded function that takes additional argument `feature_settings.
  BL_INLINE_NODEBUG BLResult create_from_face(const BLFontFaceCore& face, float size, const BLFontFeatureSettingsCore& feature_settings) noexcept {
    return bl_font_create_from_face_with_settings(this, &face, size, &feature_settings, nullptr);
  }

  //! Creates a new font from the existing font `face` scaled to the given `size`.
  //!
  //! This is an overloaded function that takes additional arguments, which are used to override font `feature_settings`
  //! and font `variation_settings`.
  BL_INLINE_NODEBUG BLResult create_from_face(const BLFontFaceCore& face, float size, const BLFontFeatureSettingsCore& feature_settings, const BLFontVariationSettingsCore& variation_settings) noexcept {
    return bl_font_create_from_face_with_settings(this, &face, size, &feature_settings, &variation_settings);
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the type of the font's associated font face.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontFaceType face_type() const noexcept { return face().face_type(); }
  //! Returns the flags of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontFaceFlags face_flags() const noexcept { return face().face_flags(); }

  //! Returns the size of the font (as float).
  [[nodiscard]]
  BL_INLINE_NODEBUG float size() const noexcept { return _impl()->metrics.size; }
  //! Sets the font size to `size`.
  BL_INLINE_NODEBUG BLResult set_size(float size) noexcept { return bl_font_set_size(this, size); }

  //! Returns the font's associated font face.
  //!
  //! Returns the same font face, which was passed to `create_from_face()`.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontFace& face() const noexcept { return _impl()->face.dcast(); }

  //! Returns the weight of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t weight() const noexcept { return _impl()->weight; }
  //! Returns the stretch of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t stretch() const noexcept { return _impl()->stretch; }
  //! Returns the style of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t style() const noexcept { return _impl()->style; }

  //! Returns the "units per em" (UPEM) of the font's associated font face.
  [[nodiscard]]
  BL_INLINE_NODEBUG int units_per_em() const noexcept { return face().units_per_em(); }

  //! Returns a 2x2 matrix of the font.
  //!
  //! The returned \ref BLFontMatrix is used to scale fonts from design units into user units. The matrix
  //! usually has a negative `m11` member as fonts use a different coordinate system than Blend2D.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontMatrix& matrix() const noexcept { return _impl()->matrix; }

  //! Returns the scaled metrics of the font.
  //!
  //! The returned metrics is a scale of design metrics that match the font size and its options.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontMetrics& metrics() const noexcept { return _impl()->metrics; }

  //! Returns the design metrics of the font.
  //!
  //! The returned metrics is compatible with the metrics of \ref BLFontFace associated with this font.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontDesignMetrics& design_metrics() const noexcept { return face().design_metrics(); }

  //! Returns font feature settings.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontFeatureSettings& feature_settings() const noexcept { return _impl()->feature_settings.dcast(); }
  //! Sets font feature settings to `feature_settings`.
  BL_INLINE_NODEBUG BLResult set_feature_settings(const BLFontFeatureSettingsCore& feature_settings) noexcept { return bl_font_set_feature_settings(this, &feature_settings); }
  //! Resets font feature settings.
  BL_INLINE_NODEBUG BLResult reset_feature_settings() noexcept { return bl_font_reset_feature_settings(this); }

  //! Returns font variation settings.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontVariationSettings& variation_settings() const noexcept { return _impl()->variation_settings.dcast(); }
  //! Sets font variation settings to `variation_settings`.
  BL_INLINE_NODEBUG BLResult set_variation_settings(const BLFontVariationSettingsCore& variation_settings) noexcept { return bl_font_set_variation_settings(this, &variation_settings); }
  //! Resets font variation settings.
  BL_INLINE_NODEBUG BLResult reset_variation_settings() noexcept { return bl_font_reset_variation_settings(this); }

  //! \}

  //! \name Glyphs & Text
  //! \{

  BL_INLINE_NODEBUG BLResult shape(BLGlyphBufferCore& gb) const noexcept {
    return bl_font_shape(this, &gb);
  }

  BL_INLINE_NODEBUG BLResult map_text_to_glyphs(BLGlyphBufferCore& gb) const noexcept {
    return bl_font_map_text_to_glyphs(this, &gb, nullptr);
  }

  BL_INLINE_NODEBUG BLResult map_text_to_glyphs(BLGlyphBufferCore& gb, BLGlyphMappingState& state_out) const noexcept {
    return bl_font_map_text_to_glyphs(this, &gb, &state_out);
  }

  BL_INLINE_NODEBUG BLResult position_glyphs(BLGlyphBufferCore& gb) const noexcept {
    return bl_font_position_glyphs(this, &gb);
  }

  BL_INLINE_NODEBUG BLResult apply_kerning(BLGlyphBufferCore& gb) const noexcept {
    return bl_font_apply_kerning(this, &gb);
  }

  BL_INLINE_NODEBUG BLResult apply_gsub(BLGlyphBufferCore& gb, const BLBitArrayCore& lookups) const noexcept {
    return bl_font_apply_gsub(this, &gb, &lookups);
  }

  BL_INLINE_NODEBUG BLResult apply_gpos(BLGlyphBufferCore& gb, const BLBitArrayCore& lookups) const noexcept {
    return bl_font_apply_gpos(this, &gb, &lookups);
  }

  BL_INLINE_NODEBUG BLResult get_text_metrics(BLGlyphBufferCore& gb, BLTextMetrics& out) const noexcept {
    return bl_font_get_text_metrics(this, &gb, &out);
  }

  BL_INLINE_NODEBUG BLResult get_glyph_bounds(const uint32_t* glyph_data, intptr_t glyph_advance, BLBoxI* out, size_t count) const noexcept {
    return bl_font_get_glyph_bounds(this, glyph_data, glyph_advance, out, count);
  }

  BL_INLINE_NODEBUG BLResult get_glyph_advances(const uint32_t* glyph_data, intptr_t glyph_advance, BLGlyphPlacement* out, size_t count) const noexcept {
    return bl_font_get_glyph_advances(this, glyph_data, glyph_advance, out, count);
  }

  //! Retrieves outlines of a single glyph into the `out` path.
  //!
  //! Optionally, a user can provide a `sink` function with `user_data`, which will be called periodically by the
  //! glyph outline decoder. The `sink` can be used to immediately process the outline to prevent accumulating a
  //! large path in `out`.
  BL_INLINE_NODEBUG BLResult get_glyph_outlines(BLGlyphId glyph_id, BLPathCore& out, BLPathSinkFunc sink = nullptr, void* user_data = nullptr) const noexcept {
    return bl_font_get_glyph_outlines(this, glyph_id, nullptr, &out, sink, user_data);
  }

  //! Retrieves outlines of a single glyph into the `out` path transformed by `user_transform`.
  //!
  //! Optionally, a user can provide a `sink` function with `user_data`, which will be called periodically by the
  //! glyph outline decoder. The `sink` can be used to immediately process the outline to prevent accumulating a
  //! large path in `out`.
  BL_INLINE_NODEBUG BLResult get_glyph_outlines(BLGlyphId glyph_id, const BLMatrix2D& user_transform, BLPathCore& out, BLPathSinkFunc sink = nullptr, void* user_data = nullptr) const noexcept {
    return bl_font_get_glyph_outlines(this, glyph_id, &user_transform, &out, sink, user_data);
  }

  //! Retrieves outlines of a glyph run into the `out` path.
  //!
  //! Optionally, a user can provide a `sink` function with `user_data`, which will be called periodically by the
  //! glyph outline decoder. The `sink` can be used to immediately process the outline to prevent accumulating a
  //! large path in `out`.
  BL_INLINE_NODEBUG BLResult get_glyph_run_outlines(const BLGlyphRun& glyph_run, BLPathCore& out, BLPathSinkFunc sink = nullptr, void* user_data = nullptr) const noexcept {
    return bl_font_get_glyph_run_outlines(this, &glyph_run, nullptr, &out, sink, user_data);
  }

  //! Retrieves outlines of a glyph run into the `out` path transformed by `user_transform`.
  //!
  //! Optionally, a user can provide a `sink` function with `user_data`, which will be called periodically by the
  //! glyph outline decoder. The `sink` can be used to immediately process the outline to prevent accumulating a
  //! large path in `out`.
  BL_INLINE_NODEBUG BLResult get_glyph_run_outlines(const BLGlyphRun& glyph_run, const BLMatrix2D& user_transform, BLPathCore& out, BLPathSinkFunc sink = nullptr, void* user_data = nullptr) const noexcept {
    return bl_font_get_glyph_run_outlines(this, &glyph_run, &user_transform, &out, sink, user_data);
  }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_FONT_H_INCLUDED
