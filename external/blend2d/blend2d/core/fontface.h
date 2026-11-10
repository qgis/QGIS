// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTFACE_H_INCLUDED
#define BLEND2D_FONTFACE_H_INCLUDED

#include <blend2d/core/array.h>
#include <blend2d/core/bitset.h>
#include <blend2d/core/filesystem.h>
#include <blend2d/core/fontdata.h>
#include <blend2d/core/fontdefs.h>
#include <blend2d/core/geometry.h>
#include <blend2d/core/glyphbuffer.h>
#include <blend2d/core/object.h>
#include <blend2d/core/path.h>
#include <blend2d/core/string.h>

//! \addtogroup bl_text
//! \{

//! \name BLFontFace - Constants
//! \{

//! Flags used by \ref BLFontFace (or \ref BLFontFaceCore)
BL_DEFINE_ENUM(BLFontFaceFlags) {
  //! No flags.
  BL_FONT_FACE_NO_FLAGS = 0u,
  //! Font uses typographic family and subfamily names.
  BL_FONT_FACE_FLAG_TYPOGRAPHIC_NAMES = 0x00000001u,
  //! Font uses typographic metrics.
  BL_FONT_FACE_FLAG_TYPOGRAPHIC_METRICS = 0x00000002u,
  //! Character to glyph mapping is available.
  BL_FONT_FACE_FLAG_CHAR_TO_GLYPH_MAPPING = 0x00000004u,
  //! Horizontal glyph metrics (advances, side bearings) is available.
  BL_FONT_FACE_FLAG_HORIZONTAL_METRICS = 0x00000010u,
  //! Vertical glyph metrics (advances, side bearings) is available.
  BL_FONT_FACE_FLAG_VERTICAL_METRICS = 0x00000020u,
  //! Legacy horizontal kerning feature ('kern' table with horizontal kerning data).
  BL_FONT_FACE_FLAG_HORIZONTAL_KERNING = 0x00000040u,
  //! Legacy vertical kerning feature ('kern' table with vertical kerning data).
  BL_FONT_FACE_FLAG_VERTICAL_KERNING = 0x00000080u,
  //! OpenType features (GDEF, GPOS, GSUB) are available.
  BL_FONT_FACE_FLAG_OPENTYPE_FEATURES = 0x00000100u,
  //! Panose classification is available.
  BL_FONT_FACE_FLAG_PANOSE_INFO = 0x00000200u,
  //! Unicode coverage information is available.
  BL_FONT_FACE_FLAG_COVERAGE_INFO = 0x00000400u,
  //! Baseline for font at `y` equals 0.
  BL_FONT_FACE_FLAG_BASELINE_Y_EQUALS_0 = 0x00001000u,
  //! Left sidebearing point at `x == 0` (TT only).
  BL_FONT_FACE_FLAG_LSB_POINT_X_EQUALS_0 = 0x00002000u,
  //! Unicode variation sequences feature is available.
  BL_FONT_FACE_FLAG_VARIATION_SEQUENCES = 0x10000000u,
  //! OpenType Font Variations feature is available.
  BL_FONT_FACE_FLAG_OPENTYPE_VARIATIONS = 0x20000000u,
  //! This is a symbol font.
  BL_FONT_FACE_FLAG_SYMBOL_FONT = 0x40000000u,
  //! This is a last resort font.
  BL_FONT_FACE_FLAG_LAST_RESORT_FONT = 0x80000000u

  BL_FORCE_ENUM_UINT32(BL_FONT_FACE_FLAG)
};

//! Diagnostic flags offered by \ref BLFontFace (or \ref BLFontFaceCore).
BL_DEFINE_ENUM(BLFontFaceDiagFlags) {
  //! No flags.
  BL_FONT_FACE_DIAG_NO_FLAGS = 0u,
  //! Wrong data in 'name' table.
  BL_FONT_FACE_DIAG_WRONG_NAME_DATA = 0x00000001u,
  //! Fixed data read from 'name' table and possibly fixed font family/subfamily name.
  BL_FONT_FACE_DIAG_FIXED_NAME_DATA = 0x00000002u,

  //! Wrong data in 'kern' table [kerning disabled].
  BL_FONT_FACE_DIAG_WRONG_KERN_DATA = 0x00000004u,
  //! Fixed data read from 'kern' table so it can be used.
  BL_FONT_FACE_DIAG_FIXED_KERN_DATA = 0x00000008u,

  //! Wrong data in 'cmap' table.
  BL_FONT_FACE_DIAG_WRONG_CMAP_DATA = 0x00000010u,
  //! Wrong format in 'cmap' (sub)table.
  BL_FONT_FACE_DIAG_WRONG_CMAP_FORMAT = 0x00000020u

  BL_FORCE_ENUM_UINT32(BL_FONT_FACE_DIAG)
};

//! Format of an outline stored in a font.
BL_DEFINE_ENUM(BLFontOutlineType) {
  //! None.
  BL_FONT_OUTLINE_TYPE_NONE = 0,
  //! Truetype outlines.
  BL_FONT_OUTLINE_TYPE_TRUETYPE = 1,
  //! OpenType (CFF) outlines.
  BL_FONT_OUTLINE_TYPE_CFF = 2,
  //! OpenType (CFF2) outlines with font variations support.
  BL_FONT_OUTLINE_TYPE_CFF2 = 3,

  //! Maximum value of `BLFontOutlineType`.
  BL_FONT_OUTLINE_TYPE_MAX_VALUE = 3

  BL_FORCE_ENUM_UINT32(BL_FONT_OUTLINE_TYPE)
};

//! \}

//! \name BLFontFace - Structs
//! \{

//! Information of \ref BLFontFace.
struct BLFontFaceInfo {
  //! \name Members
  //! \{

  //! Font face type, see \ref BLFontFaceType.
  uint8_t face_type;
  //! Type of outlines used by the font face, see \ref BLFontOutlineType.
  uint8_t outline_type;
  //! Reserved fields.
  uint8_t reserved8[2];
  //! Number of glyphs provided by this font face.
  uint32_t glyph_count;

  //! Revision (read from 'head' table, represented as 16.16 fixed point).
  uint32_t revision;

  //! Face face index in a TTF/OTF collection or zero if not part of a collection.
  uint32_t face_index;
  //! Font face flags, see \ref BLFontFaceFlags
  uint32_t face_flags;

  //! Font face diagnostic flags, see \ref BLFontFaceDiagFlags.
  uint32_t diag_flags;

  //! Reserved for future use, set to zero.
  uint32_t reserved[2];

  //! \}

#ifdef __cplusplus
  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFontFaceInfo{}; }

  //! \}
#endif
};

//! \}
//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLFontFace - C API
//! \{

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_font_face_init(BLFontFaceCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_init_move(BLFontFaceCore* self, BLFontFaceCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_init_weak(BLFontFaceCore* self, const BLFontFaceCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_destroy(BLFontFaceCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_reset(BLFontFaceCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_assign_move(BLFontFaceCore* self, BLFontFaceCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_assign_weak(BLFontFaceCore* self, const BLFontFaceCore* other) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_face_equals(const BLFontFaceCore* a, const BLFontFaceCore* b) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_create_from_file(BLFontFaceCore* self, const char* file_name, BLFileReadFlags read_flags) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_create_from_data(BLFontFaceCore* self, const BLFontDataCore* font_data, uint32_t face_index) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_full_name(const BLFontFaceCore* self, BLStringCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_family_name(const BLFontFaceCore* self, BLStringCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_subfamily_name(const BLFontFaceCore* self, BLStringCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_post_script_name(const BLFontFaceCore* self, BLStringCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_face_info(const BLFontFaceCore* self, BLFontFaceInfo* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_design_metrics(const BLFontFaceCore* self, BLFontDesignMetrics* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_coverage_info(const BLFontFaceCore* self, BLFontCoverageInfo* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_panose_info(const BLFontFaceCore* self, BLFontPanoseInfo* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_character_coverage(const BLFontFaceCore* self, BLBitSetCore* out) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_face_has_script_tag(const BLFontFaceCore* self, BLTag script_tag) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_face_has_feature_tag(const BLFontFaceCore* self, BLTag feature_tag) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_font_face_has_variation_tag(const BLFontFaceCore* self, BLTag variation_tag) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_script_tags(const BLFontFaceCore* self, BLArrayCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_feature_tags(const BLFontFaceCore* self, BLArrayCore* out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_font_face_get_variation_tags(const BLFontFaceCore* self, BLArrayCore* out) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! Font face [C API].
struct BLFontFaceCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLFontFace)
};

//! \cond INTERNAL
//! Font face [C API Virtual Function Table].
struct BLFontFaceVirt BL_CLASS_INHERITS(BLObjectVirt) {
  BL_DEFINE_VIRT_BASE
};

//! Font face [C API Impl].
struct BLFontFaceImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Virtual function table.
  const BLFontFaceVirt* virt;

  //! Font face default weight (1..1000) [0 if font face is not initialized].
  uint16_t weight;
  //! Font face default stretch (1..9) [0 if font face is not initialized].
  uint8_t stretch;
  //! Font face default style.
  uint8_t style;

  //! Font face information.
  BLFontFaceInfo face_info;
  //! Unique identifier assigned by Blend2D that can be used for caching.
  BLUniqueId unique_id;

  //! Font data.
  BLFontDataCore data;
  //! Full name.
  BLStringCore full_name;
  //! Family name.
  BLStringCore family_name;
  //! Subfamily name.
  BLStringCore subfamily_name;
  //! PostScript name.
  BLStringCore post_script_name;

  //! Font face metrics in design units.
  BLFontDesignMetrics design_metrics;
  //! Font face unicode coverage information as specified in the "OS/2" header.
  BLFontCoverageInfo coverage_info;
  //! Font face panose classification.
  BLFontPanoseInfo panose_info;
};
//! \endcond

//! \}
//! \}

//! \addtogroup bl_text
//! \{

//! \name BLFontFace - C++ API
//! \{

#ifdef __cplusplus
//! Font face [C++ API].
class BLFontFace final : public BLFontFaceCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontFaceImpl* _impl() const noexcept { return static_cast<BLFontFaceImpl*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLFontFace() noexcept {
    bl_font_face_init(this);
  }

  BL_INLINE_NODEBUG BLFontFace(BLFontFace&& other) noexcept {
    bl_font_face_init_move(this, &other);
  }

  BL_INLINE_NODEBUG BLFontFace(const BLFontFace& other) noexcept {
    bl_font_face_init_weak(this, &other);
  }

  BL_INLINE_NODEBUG ~BLFontFace() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_font_face_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return is_valid(); }

  BL_INLINE_NODEBUG BLFontFace& operator=(BLFontFace&& other) noexcept { bl_font_face_assign_move(this, &other); return *this; }
  BL_INLINE_NODEBUG BLFontFace& operator=(const BLFontFace& other) noexcept { bl_font_face_assign_weak(this, &other); return *this; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLFontFace& other) const noexcept { return equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLFontFace& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult reset() noexcept {
    return bl_font_face_reset(this);
  }

  BL_INLINE_NODEBUG void swap(BLFontFace& other) noexcept { _d.swap(other._d); }

  BL_INLINE_NODEBUG BLResult assign(BLFontFace&& other) noexcept { return bl_font_face_assign_move(this, &other); }
  BL_INLINE_NODEBUG BLResult assign(const BLFontFace& other) noexcept { return bl_font_face_assign_weak(this, &other); }

  //! Tests whether the font face is valid.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_valid() const noexcept { return _impl()->face_info.face_type != BL_FONT_FACE_TYPE_NONE; }

  //! Tests whether the font face is empty, which is the same as `!is_valid()`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return !is_valid(); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLFontFace& other) const noexcept { return bl_font_face_equals(this, &other); }

  //! \}

  //! \name Create Functionality
  //! \{

  //! Creates a new \ref BLFontFace from a file specified by `file_name`.
  //!
  //! This is a utility function that first creates a \ref BLFontData` and then calls `create_from_data(font_data, 0)`.
  //! See \ref BLFontData::create_from_file() for more details, especially the use of `read_flags` is important for
  //! system fonts.
  //!
  //! \note This function offers a simplified creation of \ref BLFontFace directly from a file, but doesn't provide
  //! as much flexibility as \ref create_from_data() as it allows to specify a `face_index`, which can be used to load
  //! multiple font faces from a TrueType/OpenType collection. The use of \ref create_from_data() is recommended for
  //! any serious font handling.
  BL_INLINE_NODEBUG BLResult create_from_file(const char* file_name, BLFileReadFlags read_flags = BL_FILE_READ_NO_FLAGS) noexcept {
    return bl_font_face_create_from_file(this, file_name, read_flags);
  }

  //! Creates a new \ref BLFontFace from \ref BLFontData at the given `face_index`.
  //!
  //! On success the existing \ref BLFontFace is completely replaced by a new one, on failure an error is returned
  //! in \ref BLResult and the existing \ref BLFontFace is kept as is.
  BL_INLINE_NODEBUG BLResult create_from_data(const BLFontDataCore& font_data, uint32_t face_index) noexcept {
    return bl_font_face_create_from_data(this, &font_data, face_index);
  }

  //! \}

  //! \name Properties
  //! \{

  //! Returns font weight (returns default weight in case this is a variable font).
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t weight() const noexcept { return _impl()->weight; }

  //! Returns font stretch (returns default weight in case this is a variable font).
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t stretch() const noexcept { return _impl()->stretch; }

  //! Returns font style.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t style() const noexcept { return _impl()->style; }

  //! Returns font face information as \ref BLFontFaceInfo.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontFaceInfo& face_info() const noexcept { return _impl()->face_info; }

  //! Returns the font face type.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontFaceType face_type() const noexcept { return (BLFontFaceType)_impl()->face_info.face_type; }

  //! Returns the font face type.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontOutlineType outline_type() const noexcept { return (BLFontOutlineType)_impl()->face_info.outline_type; }

  //! Returns the number of glyphs this font face provides.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t glyph_count() const noexcept { return _impl()->face_info.glyph_count; }

  //! Returns a zero-based index of this font face.
  //!
  //! \note Face index does only make sense if this face is part of a TrueType or OpenType font collection. In that
  //! case the returned value would be the index of this face in that collection. If the face is not part of a
  //! collection then the returned value would always be zero.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t face_index() const noexcept { return _impl()->face_info.face_index; }

  //! Returns font face flags.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontFaceFlags face_flags() const noexcept { return (BLFontFaceFlags)_impl()->face_info.face_flags; }

  //! Tests whether the font face has a given `flag` set.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_face_flag(BLFontFaceFlags flag) const noexcept { return (_impl()->face_info.face_flags & flag) != 0; }

  //! Tests whether the font face uses typographic family and subfamily names.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_typographic_names() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_TYPOGRAPHIC_NAMES); }

  //! Tests whether the font face uses typographic metrics.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_typographic_metrics() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_TYPOGRAPHIC_METRICS); }

  //! Tests whether the font face provides character to glyph mapping.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_char_to_glyph_mapping() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_CHAR_TO_GLYPH_MAPPING); }

  //! Tests whether the font face has horizontal glyph metrics (advances, side bearings).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_horizontal_metrics() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_HORIZONTAL_METRICS); }

  //! Tests whether the font face has vertical glyph metrics (advances, side bearings).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_vertical_metrics() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_VERTICAL_METRICS); }

  //! Tests whether the font face has a legacy horizontal kerning feature ('kern' table with horizontal kerning data).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_horizontal_kerning() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_HORIZONTAL_KERNING); }

  //! Tests whether the font face has a legacy vertical kerning feature ('kern' table with vertical kerning data).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_vertical_kerning() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_VERTICAL_KERNING); }

  //! Tests whether the font face has OpenType features (GDEF, GPOS, GSUB).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_open_type_features() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_OPENTYPE_FEATURES); }

  //! Tests whether the font face has panose classification.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_panose_info() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_PANOSE_INFO); }

  //! Tests whether the font face has unicode coverage information (this is reported by the font-face itself, not calculated).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_coverage_info() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_COVERAGE_INFO); }

  //! Tests whether the font face's baseline equals 0.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_baseline_y_at_0() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_BASELINE_Y_EQUALS_0); }

  //! Tests whether the font face's left sidebearing point at `x` equals 0.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_lsb_point_x_at_0() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_LSB_POINT_X_EQUALS_0); }

  //! Tests whether the font face has unicode variation sequences feature.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_variation_sequences() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_VARIATION_SEQUENCES); }

  //! Tests whether the font face has OpenType Font Variations feature.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_open_type_variations() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_OPENTYPE_VARIATIONS); }

  //! This is a symbol font.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_symbol_font() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_SYMBOL_FONT); }

  //! This is a last resort font.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_last_resort_font() const noexcept { return has_face_flag(BL_FONT_FACE_FLAG_LAST_RESORT_FONT); }

  //! Returns font face diagnostics flags.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFontFaceDiagFlags diag_flags() const noexcept { return BLFontFaceDiagFlags(_impl()->face_info.diag_flags); }

  //! Returns a unique identifier describing this \ref BLFontFace.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLUniqueId unique_id() const noexcept { return _impl()->unique_id; }

  //! Returns \ref BLFontData associated with this font face.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontData& data() const noexcept { return _impl()->data.dcast(); }

  //! Returns a full of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& full_name() const noexcept { return _impl()->full_name.dcast(); }

  //! Returns a family name of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& family_name() const noexcept { return _impl()->family_name.dcast(); }

  //! Returns a subfamily name of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& subfamily_name() const noexcept { return _impl()->subfamily_name.dcast(); }

  //! Returns a PostScript name of the font.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& post_script_name() const noexcept { return _impl()->post_script_name.dcast(); }

  //! Returns design metrics of this \ref BLFontFace.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontDesignMetrics& design_metrics() const noexcept { return _impl()->design_metrics; }

  //! Returns units per em, which are part of font's design metrics.
  [[nodiscard]]
  BL_INLINE_NODEBUG int units_per_em() const noexcept { return _impl()->design_metrics.units_per_em; }

  //! Returns PANOSE classification of this \ref BLFontFace.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontPanoseInfo& panose_info() const noexcept { return _impl()->panose_info; }

  //! Returns unicode coverage of this \ref BLFontFace.
  //!
  //! \note The returned unicode-coverage is not calculated by Blend2D so in general the value doesn't have to be
  //! correct. Consider `get_character_coverage()` to get a coverage calculated by Blend2D at character granularity.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLFontCoverageInfo& coverage_info() const noexcept { return _impl()->coverage_info; }

  //! Calculates the character coverage of this \ref BLFontFace.
  //!
  //! Each unicode character is represented by a single bit in the given BitSet.
  BL_INLINE_NODEBUG BLResult get_character_coverage(BLBitSetCore* out) const noexcept { return bl_font_face_get_character_coverage(this, out); }

  //! Tests whether the font face provides the given OpenType `script_tag`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_script_tag(BLTag script_tag) const noexcept { return bl_font_face_has_script_tag(this, script_tag); }

  //! Tests whether the font face provides the given OpenType `feature_tag`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_feature_tag(BLTag feature_tag) const noexcept { return bl_font_face_has_feature_tag(this, feature_tag); }

  //! Tests whether the font face provides the given OpenType `variation_tag`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_variation_tag(BLTag variation_tag) const noexcept { return bl_font_face_has_variation_tag(this, variation_tag); }

  //! Retrieves OpenType script tags provided by this \ref BLFontFace.
  //!
  //! Each script tag is represented by 4 characters encoded in \ref BLTag.
  BL_INLINE_NODEBUG BLResult get_script_tags(BLArray<BLTag>* out) const noexcept { return bl_font_face_get_script_tags(this, out); }

  //! Retrieves OpenType feature tags provided by this \ref BLFontFace.
  //!
  //! Each feature tag is represented by 4 characters encoded in \ref BLTag.
  //!
  //! Feature tag registry:
  //!   - Microsoft <https://docs.microsoft.com/en-us/typography/opentype/spec/featurelist>
  BL_INLINE_NODEBUG BLResult get_feature_tags(BLArray<BLTag>* out) const noexcept { return bl_font_face_get_feature_tags(this, out); }

  //! Retrieves OpenType variation tags provided by this \ref BLFontFace.
  //!
  //! Each variation tag is represented by 4 characters encoded in \ref BLTag.
  //!
  //! Variation tag registry:
  //!   - Microsoft <https://docs.microsoft.com/en-us/typography/opentype/spec/dvaraxisreg>
  BL_INLINE_NODEBUG BLResult get_variation_tags(BLArray<BLTag>* out) const noexcept { return bl_font_face_get_variation_tags(this, out); }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_FONTFACE_H_INCLUDED
