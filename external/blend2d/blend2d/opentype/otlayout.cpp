// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/bitarray_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/fonttagdata_p.h>
#include <blend2d/core/fontfeaturesettings_p.h>
#include <blend2d/core/glyphbuffer_p.h>
#include <blend2d/core/trace_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otlayout_p.h>
#include <blend2d/opentype/otlayoutcontext_p.h>
#include <blend2d/opentype/otlayouttables_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/fixedbitarray_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>

// TODO: [OpenType] This is not complete so we had to disable some warnings here...
BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)

namespace bl::OpenType {
namespace LayoutImpl {

// bl::OpenType::LayoutImpl - Tracing
// ==================================

#if defined(BL_TRACE_OT_ALL) || defined(BL_TRACE_OT_LAYOUT)
  #define Trace BLDebugTrace
#else
  #define Trace BLDummyTrace
#endif

class ValidationContext {
public:
  //! \name Members
  //! \{

  const OTFaceImpl* _ot_face_impl;
  LookupKind _lookup_kind;
  Trace _trace;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG ValidationContext(const OTFaceImpl* ot_face_impl, LookupKind lookup_kind) noexcept
    : _ot_face_impl(ot_face_impl),
      _lookup_kind(lookup_kind) {}

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG const OTFaceImpl* ot_face_impl() const noexcept { return _ot_face_impl; }
  BL_INLINE_NODEBUG LookupKind lookup_kind() const noexcept { return _lookup_kind; }

  //! \}

  //! \name Tracing & Error Handling
  //! \{

  BL_INLINE void indent() noexcept { _trace.indent(); }
  BL_INLINE void deindent() noexcept { _trace.deindent(); }

  template<typename... Args>
  BL_INLINE void out(Args&&... args) noexcept { _trace.out(BLInternal::forward<Args>(args)...); }

  template<typename... Args>
  BL_INLINE void info(Args&&... args) noexcept { _trace.info(BLInternal::forward<Args>(args)...); }

  template<typename... Args>
  BL_INLINE bool warn(Args&&... args) noexcept { return _trace.warn(BLInternal::forward<Args>(args)...); }

  template<typename... Args>
  BL_INLINE bool fail(Args&&... args) noexcept { return _trace.fail(BLInternal::forward<Args>(args)...); }

  BL_NOINLINE bool table_empty(const char* table_name) noexcept {
    return fail("%s cannot be empty", table_name);
  }

  BL_NOINLINE bool invalid_table_size(const char* table_name, uint32_t table_size, uint32_t required_size) noexcept {
    return fail("%s is truncated (size=%u, required=%u)", table_name, table_size, required_size);
  }

  BL_NOINLINE bool invalid_table_format(const char* table_name, uint32_t format) noexcept {
    return fail("%s has invalid format (%u)", table_name, format);
  }

  BL_NOINLINE bool invalid_field_value(const char* table_name, const char* field, uint32_t value) noexcept {
    return fail("%s has invalid %s (%u)", table_name, value);
  }

  BL_NOINLINE bool invalid_field_offset(const char* table_name, const char* field, uint32_t offset, OffsetRange range) noexcept {
    return fail("%s.%s has invalid offset (%u), valid range=[%u:%u]", table_name, field, offset, range.start, range.end);
  }

  BL_NOINLINE bool invalid_offset_array(const char* table_name, uint32_t i, uint32_t offset, OffsetRange range) noexcept {
    return fail("%s has invalid offset at #%u: Offset=%u, ValidRange=[%u:%u]", table_name, i, offset, range.start, range.end);
  }

  BL_NOINLINE bool invalid_offset_entry(const char* table_name, const char* field, uint32_t i, uint32_t offset, OffsetRange range) noexcept {
    return fail("%s has invalid offset of %s at #%u: Offset=%u, ValidRange=[%u:%u]", table_name, field, i, offset, range.start, range.end);
  }

  //! \}
};

// bl::OpenType::LayoutImpl - GDEF - Init
// ======================================

static BL_NOINLINE BLResult initGDef(OTFaceImpl* ot_face_impl, Table<GDefTable> gdef) noexcept {
  if (!gdef.fits())
    return BL_SUCCESS;

  uint32_t version = gdef->v1_0()->version();
  uint32_t header_size = GDefTable::HeaderV1_0::kBaseSize;

  if (version >= 0x00010002u)
    header_size = GDefTable::HeaderV1_2::kBaseSize;

  if (version >= 0x00010003u)
    header_size = GDefTable::HeaderV1_3::kBaseSize;

  if (BL_UNLIKELY(version < 0x00010000u || version > 0x00010003u))
    return BL_SUCCESS;

  if (BL_UNLIKELY(gdef.size < header_size))
    return BL_SUCCESS;

  uint32_t glyph_class_def_offset = gdef->v1_0()->glyph_class_def_offset();
  uint32_t attach_list_offset = gdef->v1_0()->attach_list_offset();
  uint32_t lig_caret_list_offset = gdef->v1_0()->lig_caret_list_offset();
  uint32_t mark_attach_class_def_offset = gdef->v1_0()->mark_attach_class_def_offset();
  uint32_t mark_glyph_sets_def_offset = version >= 0x00010002u ? uint32_t(gdef->v1_2()->mark_glyph_sets_def_offset()) : uint32_t(0);
  uint32_t item_var_store_offset = version >= 0x00010003u ? uint32_t(gdef->v1_3()->item_var_store_offset()) : uint32_t(0);

  // TODO: [OpenType] Unfinished.
  bl_unused(attach_list_offset, lig_caret_list_offset, mark_glyph_sets_def_offset, item_var_store_offset);

  // Some fonts have incorrect value of `GlyphClassDefOffset` set to 10. This collides with the header which is
  // 12 bytes. It's probably a result of some broken tool used to write such fonts in the past. We simply fix
  // this issue by changing the `header_size` to 10 and ignoring `mark_attach_class_def_offset`.
  if (glyph_class_def_offset == 10 && version == 0x00010000u) {
    header_size = 10;
    mark_attach_class_def_offset = 0;
  }

  if (glyph_class_def_offset) {
    if (glyph_class_def_offset >= header_size && glyph_class_def_offset < gdef.size) {
      ot_face_impl->ot_flags |= OTFaceFlags::kGlyphClassDef;
    }
  }

  if (mark_attach_class_def_offset) {
    if (mark_attach_class_def_offset >= header_size && mark_attach_class_def_offset < gdef.size) {
      ot_face_impl->ot_flags |= OTFaceFlags::kMarkAttachClassDef;
    }
  }

  ot_face_impl->layout.tables[2] = gdef;
  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Constants
// ==================================================

// Artificial format bits that describe either two/three/four Coverage/ClassDef tables having formats in [1-2] range.
enum class FormatBits2X : uint32_t {
  k11 = 0x0,
  k12 = 0x1,
  k21 = 0x2,
  k22 = 0x3
};

enum class FormatBits3X : uint32_t {
  k111 = 0x0,
  k112 = 0x1,
  k121 = 0x2,
  k122 = 0x3,
  k211 = 0x4,
  k212 = 0x5,
  k221 = 0x6,
  k222 = 0x7
};

enum class FormatBits4X : uint32_t {
  k1111 = 0x0,
  k1112 = 0x1,
  k1121 = 0x2,
  k1122 = 0x3,
  k1211 = 0x4,
  k1212 = 0x5,
  k1221 = 0x6,
  k1222 = 0x7,
  k2111 = 0x8,
  k2112 = 0x9,
  k2121 = 0xA,
  k2122 = 0xB,
  k2211 = 0xC,
  k2212 = 0xD,
  k2221 = 0xE,
  k2222 = 0xF
};

// bl::OpenType::LayoutImpl - GSUB & GPOS - Metadata
// =================================================

static BL_NOINLINE const char* gsub_lookup_name(uint32_t lookup_type) noexcept {
  static const char lookup_names[] = {
    "<INVALID>\0"
    "SingleSubst\0"
    "MultipleSubst\0"
    "AlternateSubst\0"
    "LigatureSubst\0"
    "ContextSubst\0"
    "ChainedContextSubst\0"
    "Extension\0"
    "ReverseChainedContextSubst\0"
  };

  static const uint8_t lookup_index[] = {
    0,   // "<INVALID>"
    10,  // "SingleSubst"
    22,  // "MultipleSubst"
    36,  // "AlternateSubst"
    51,  // "LigatureSubst"
    65,  // "ContextSubst"
    78,  // "ChainedContextSubst"
    98,  // "Extension"
    108  // "ReverseChainedContextSubst"
  };

  if (lookup_type >= BL_ARRAY_SIZE(lookup_index))
    lookup_type = 0;

  return lookup_names + lookup_index[lookup_type];
}

static BL_NOINLINE const char* gpos_lookup_name(uint32_t lookup_type) noexcept {
  static const char lookup_names[] = {
    "<INVALID>\0"
    "SingleAdjustment\0"
    "PairAdjustment\0"
    "CursiveAttachment\0"
    "MarkToBaseAttachment\0"
    "MarkToLigatureAttachment\0"
    "MarkToMarkAttachment\0"
    "ContextPositioning\0"
    "ChainedContextPositioning\0"
    "Extension\0"
  };

  static const uint8_t lookup_index[] = {
    0,   // "<INVALID>"
    10,  // "SingleAdjustment"
    27,  // "PairAdjustment"
    42,  // "CursiveAttachment"
    60,  // "MarkToBaseAttachment"
    81,  // "MarkToLigatureAttachment"
    106, // "MarkToMarkAttachment"
    127, // "ContextPositioning"
    146, // "ChainedContextPositioning"
    172  // "Extension"
  };

  if (lookup_type >= BL_ARRAY_SIZE(lookup_index))
    lookup_type = 0;

  return lookup_names + lookup_index[lookup_type];
}

static const GSubGPosLookupInfo gsub_lookup_info_table = {
  // Lookup type maximum value:
  GSubTable::kLookupMaxValue,

  // Lookup extension type:
  GSubTable::kLookupExtension,

  // Lookup type to lookup type & format mapping:
  {
    { 0, uint8_t(GSubLookupAndFormat::kNone)         }, // Invalid.
    { 2, uint8_t(GSubLookupAndFormat::kType1Format1) }, // Lookup Type 1 - Single Substitution.
    { 1, uint8_t(GSubLookupAndFormat::kType2Format1) }, // Lookup Type 2 - Multiple Substitution.
    { 1, uint8_t(GSubLookupAndFormat::kType3Format1) }, // Lookup Type 3 - Alternate Substitution.
    { 1, uint8_t(GSubLookupAndFormat::kType4Format1) }, // Lookup Type 4 - Ligature Substitution.
    { 3, uint8_t(GSubLookupAndFormat::kType5Format1) }, // Lookup Type 5 - Context Substitution.
    { 3, uint8_t(GSubLookupAndFormat::kType6Format1) }, // Lookup Type 6 - Chained Context Substitution.
    { 1, uint8_t(GSubLookupAndFormat::kNone)         }, // Lookup Type 7 - Extension.
    { 1, uint8_t(GSubLookupAndFormat::kType8Format1) }  // Lookup Type 8 - Reverse Chained Context Substitution.
  },

  // Lookup type, format, header_size.
  {
    { 0, 0, uint8_t(0)                                                }, // Lookup Type 0 - Invalid.
    { 1, 1, uint8_t(GSubTable::SingleSubst1::kBaseSize)               }, // Lookup Type 1 - Format 1.
    { 1, 2, uint8_t(GSubTable::SingleSubst2::kBaseSize)               }, // Lookup Type 1 - Format 2.
    { 2, 1, uint8_t(GSubTable::MultipleSubst1::kBaseSize)             }, // Lookup Type 2 - Format 1.
    { 3, 1, uint8_t(GSubTable::AlternateSubst1::kBaseSize)            }, // Lookup Type 3 - Format 1.
    { 4, 1, uint8_t(GSubTable::LigatureSubst1::kBaseSize)             }, // Lookup Type 4 - Format 1.
    { 5, 1, uint8_t(GSubTable::SequenceContext1::kBaseSize)           }, // Lookup Type 5 - Format 1.
    { 5, 2, uint8_t(GSubTable::SequenceContext2::kBaseSize)           }, // Lookup Type 5 - Format 2.
    { 5, 3, uint8_t(GSubTable::SequenceContext3::kBaseSize)           }, // Lookup Type 5 - Format 3.
    { 6, 1, uint8_t(GSubTable::ChainedSequenceContext1::kBaseSize)    }, // Lookup Type 6 - Format 1.
    { 6, 2, uint8_t(GSubTable::ChainedSequenceContext2::kBaseSize)    }, // Lookup Type 6 - Format 2.
    { 6, 3, uint8_t(GSubTable::ChainedSequenceContext3::kBaseSize)    }, // Lookup Type 6 - Format 3.
    { 8, 1, uint8_t(GSubTable::ReverseChainedSingleSubst1::kBaseSize) }  // Lookup Type 8 - Format 1.
  }
};

static const GSubGPosLookupInfo gpos_lookup_info_table = {
  // Lookup type maximum value:
  GPosTable::kLookupMaxValue,

  // Lookup extension type:
  GPosTable::kLookupExtension,

  // Lookup type to lookup type & format mapping:
  {
    { 0, uint8_t(GPosLookupAndFormat::kNone)         }, // Lookup Type 0 - Invalid.
    { 2, uint8_t(GPosLookupAndFormat::kType1Format1) }, // Lookup Type 1 - Single Adjustment.
    { 2, uint8_t(GPosLookupAndFormat::kType2Format1) }, // Lookup Type 2 - Pair Adjustment.
    { 1, uint8_t(GPosLookupAndFormat::kType3Format1) }, // Lookup Type 3 - Cursive Attachment.
    { 1, uint8_t(GPosLookupAndFormat::kType4Format1) }, // Lookup Type 4 - MarkToBase Attachment.
    { 1, uint8_t(GPosLookupAndFormat::kType5Format1) }, // Lookup Type 5 - MarkToLigature Attachment.
    { 1, uint8_t(GPosLookupAndFormat::kType6Format1) }, // Lookup Type 6 - MarkToMark Attachment.
    { 3, uint8_t(GPosLookupAndFormat::kType7Format1) }, // Lookup Type 7 - Context Positioning.
    { 3, uint8_t(GPosLookupAndFormat::kType8Format1) }, // Lookup Type 8 - Chained Context Positioning.
    { 1, uint8_t(GPosLookupAndFormat::kNone)         }  // Lookup Type 9 - Extension.
  },

  // Lookup type, format, header_size.
  {
    { 0, 0, uint8_t(0)                                                }, // Lookup Type 0 - Invalid.
    { 1, 1, uint8_t(GPosTable::SingleAdjustment1::kBaseSize)          }, // Lookup Type 1 - Format 1.
    { 1, 2, uint8_t(GPosTable::SingleAdjustment2::kBaseSize)          }, // Lookup Type 1 - Format 2.
    { 2, 1, uint8_t(GPosTable::PairAdjustment1::kBaseSize)            }, // Lookup Type 2 - Format 1.
    { 2, 2, uint8_t(GPosTable::PairAdjustment2::kBaseSize)            }, // Lookup Type 2 - Format 2.
    { 3, 1, uint8_t(GPosTable::CursiveAttachment1::kBaseSize)         }, // Lookup Type 3 - Format 1.
    { 4, 1, uint8_t(GPosTable::MarkToBaseAttachment1::kBaseSize)      }, // Lookup Type 4 - Format 1.
    { 5, 1, uint8_t(GPosTable::MarkToLigatureAttachment1::kBaseSize)  }, // Lookup Type 5 - Format 1.
    { 6, 1, uint8_t(GPosTable::MarkToMarkAttachment1::kBaseSize)      }, // Lookup Type 6 - Format 1.
    { 7, 1, uint8_t(GPosTable::SequenceContext1::kBaseSize)           }, // Lookup Type 7 - Format 1.
    { 7, 2, uint8_t(GPosTable::SequenceContext2::kBaseSize)           }, // Lookup Type 7 - Format 2.
    { 7, 3, uint8_t(GPosTable::SequenceContext3::kBaseSize)           }, // Lookup Type 7 - Format 3.
    { 8, 1, uint8_t(GPosTable::ChainedSequenceContext1::kBaseSize)    }, // Lookup Type 8 - Format 1.
    { 8, 2, uint8_t(GPosTable::ChainedSequenceContext2::kBaseSize)    }, // Lookup Type 8 - Format 2.
    { 8, 3, uint8_t(GPosTable::ChainedSequenceContext3::kBaseSize)    }  // Lookup Type 8 - Format 3.
  }
};

// bl::OpenType::LayoutImpl - GSUB & GPOS - Validation Helpers
// ===========================================================

// TODO: [OpenType] REMOVE?
/*
static bool validate_raw_offset_array(ValidationContext& validator, RawTable data, const char* table_name) noexcept {
  if (data.size < 2u)
    return validator.invalid_table_size(table_name, data.size, 2u);

  uint32_t count = data.data_as<Array16<UInt16>>()->count();
  uint32_t header_size = 2u + count * 2u;

  if (data.size < header_size)
    return validator.invalid_table_size(table_name, data.size, header_size);

  const UInt16* array = data.data_as<Array16<Offset16>>()->array();
  OffsetRange range{header_size, uint32_t(data.size)};

  for (uint32_t i = 0; i < count; i++) {
    uint32_t offset = array[i].value();
    if (!range.contains(offset))
      return validator.invalid_offset_array(table_name, i, offset, range);
  }

  return true;
}

static bool validateTagRef16Array(ValidationContext& validator, RawTable data, const char* table_name) noexcept {
  if (data.size < 2u)
    return validator.invalid_table_size(table_name, data.size, 2u);

  uint32_t count = data.data_as<Array16<UInt16>>()->count();
  uint32_t header_size = 2u + count * uint32_t(sizeof(TagRef16));

  if (data.size < header_size)
    return validator.invalid_table_size(table_name, data.size, header_size);

  const TagRef16* array = data.data_as<Array16<TagRef16>>()->array();
  OffsetRange range{header_size, uint32_t(data.size)};

  for (uint32_t i = 0; i < count; i++) {
    uint32_t offset = array[i].offset.value();
    if (!range.contains(offset))
      return validator.invalid_offset_array(table_name, i, offset, range);
  }

  return true;
}
*/

// bl::OpenType::LayoutImpl - GSUB & GPOS - Apply Scope
// ====================================================

//! A single index to be applied when processing a lookup.
struct ApplyIndex {
  size_t _index;

  BL_INLINE_CONSTEXPR bool is_range() const noexcept { return false; }
  BL_INLINE_NODEBUG size_t index() const noexcept { return _index; }
  BL_INLINE_NODEBUG size_t end() const noexcept { return _index + 1; }
  BL_INLINE_NODEBUG size_t size() const noexcept { return 1; }
};

//! A range to be applied when processing a lookup.
//!
//! Typically a root lookup would apply the whole range of the work buffer, however, nested lookups only
//! apply a single index, which can still match multiple glyphs, but the match must start at that index.
struct ApplyRange {
  size_t _index;
  size_t _end;

  BL_INLINE_CONSTEXPR bool is_range() const noexcept { return true; }
  BL_INLINE_NODEBUG size_t index() const noexcept { return _index; }
  BL_INLINE_NODEBUG size_t end() const noexcept { return _end; }
  BL_INLINE_NODEBUG size_t size() const noexcept { return _end - _index; }

  BL_INLINE void intersect(size_t index, size_t end) noexcept {
    _index = bl_max(_index, index);
    _end = bl_min(_end, end);
  }
};

// bl::OpenType::LayoutImpl - GSUB & GPOS - ClassDef Validation
// ============================================================

static BL_NOINLINE bool validate_class_def_table(ValidationContext& validator, Table<ClassDefTable> table, const char* table_name) noexcept {
  // Ignore if it doesn't fit.
  if (!table.fits())
    return validator.invalid_table_size(table_name, table.size, ClassDefTable::kBaseSize);

  uint32_t format = table->format();
  switch (format) {
    case 1: {
      uint32_t header_size = ClassDefTable::Format1::kBaseSize;
      if (!table.fits(header_size))
        return validator.invalid_table_size(table_name, table.size, header_size);

      const ClassDefTable::Format1* f = table->format1();
      uint32_t count = f->class_values.count();

      header_size += count * 2u;
      if (!table.fits(header_size))
        return validator.invalid_table_size(table_name, table.size, header_size);

      // We won't fail, but we won't consider we have a ClassDef either.
      // If the ClassDef is required by other tables then we will fail later.
      if (!count)
        return validator.warn("No glyph ids specified, ignoring...");

      return true;
    }

    case 2: {
      uint32_t header_size = ClassDefTable::Format2::kBaseSize;
      if (!table.fits(header_size))
        return validator.invalid_table_size(table_name, table.size, header_size);

      const ClassDefTable::Format2* f = table->format2();
      uint32_t count = f->ranges.count();

      // We won't fail, but we won't consider we have a class definition either.
      if (!count)
        return validator.warn("No range specified, ignoring...");

      header_size = ClassDefTable::Format2::kBaseSize + count * uint32_t(sizeof(ClassDefTable::Range));
      if (!table.fits(header_size))
        return validator.invalid_table_size(table_name, table.size, header_size);

      const ClassDefTable::Range* range_array = f->ranges.array();
      uint32_t last_glyph = range_array[0].last_glyph();

      if (range_array[0].first_glyph() > last_glyph)
        return validator.fail("%s Range[%u] first_glyph (%u) greater than last_glyph (%u)", table_name, 0, range_array[0].first_glyph(), last_glyph);

      for (uint32_t i = 1; i < count; i++) {
        const ClassDefTable::Range& range = range_array[i];
        uint32_t first_glyph = range.first_glyph();

        if (first_glyph <= last_glyph)
          return validator.fail("%s Range[%u] first_glyph (%u) not greater than previous last_flyph (%u)", table_name, i, first_glyph, last_glyph);

        last_glyph = range.last_glyph();
        if (first_glyph > last_glyph)
          return validator.fail("%s Range[%u] first_glyph (%u) greater than last_glyph (%u)", table_name, i, first_glyph, last_glyph);
      }

      return true;
    }

    default:
      return validator.invalid_table_format(table_name, format);
  }
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Coverage Validation
// ============================================================

static BL_NOINLINE bool validate_coverage_table(ValidationContext& validator, Table<CoverageTable> coverage_table, uint32_t& coverage_count) noexcept {
  const char* table_name = "CoverageTable";

  coverage_count = 0;
  if (!coverage_table.fits())
    return validator.invalid_table_size(table_name, coverage_table.size, CoverageTable::kBaseSize);

  uint32_t format = coverage_table->format();
  switch (format) {
    case 1: {
      const CoverageTable::Format1* format1 = coverage_table->format1();

      uint32_t glyph_count = format1->glyphs.count();
      uint32_t header_size = CoverageTable::Format1::kBaseSize + glyph_count * 2u;

      if (!coverage_table.fits(header_size))
        return validator.invalid_table_size(table_name, coverage_table.size, header_size);

      if (!glyph_count)
        return validator.table_empty(table_name);

      coverage_count = glyph_count;
      return true;
    }

    case 2: {
      const CoverageTable::Format2* format2 = coverage_table->format2();

      uint32_t range_count = format2->ranges.count();
      uint32_t header_size = CoverageTable::Format2::kBaseSize + range_count * uint32_t(sizeof(CoverageTable::Range));

      if (!coverage_table.fits(header_size))
        return validator.invalid_table_size(table_name, coverage_table.size, header_size);

      if (!range_count)
        return validator.table_empty(table_name);

      const CoverageTable::Range* range_array = format2->ranges.array();

      uint32_t first_glyph = range_array[0].first_glyph();
      uint32_t last_glyph = range_array[0].last_glyph();
      uint32_t current_coverage_index = range_array[0].start_coverage_index();

      if (first_glyph > last_glyph)
        return validator.fail("Range[%u]: first_glyph (%u) is greater than last_glyph (%u)", 0, first_glyph, last_glyph);

      if (current_coverage_index)
        return validator.fail("Range[%u]: initial start_coverage_index %u must be zero", 0, current_coverage_index);

      current_coverage_index += last_glyph - first_glyph + 1u;
      for (uint32_t i = 1; i < range_count; i++) {
        const CoverageTable::Range& range = range_array[i];

        first_glyph = range.first_glyph();
        if (first_glyph <= last_glyph)
          return validator.fail("Range[%u]: first_glyph (%u) is not greater than previous last_glyph (%u)", i, first_glyph, last_glyph);

        last_glyph = range.last_glyph();
        if (first_glyph > last_glyph)
          return validator.fail("Range[%u]: first_glyph (%u) is greater than last_glyph (%u)", i, first_glyph, last_glyph);

        uint32_t start_coverage_index = range.start_coverage_index();
        if (start_coverage_index != current_coverage_index)
          return validator.fail("Range[%u]: start_coverage_index (%u) doesnt' match current_coverage_index (%u)", i, start_coverage_index, current_coverage_index);

        current_coverage_index += last_glyph - first_glyph + 1u;
      }

      coverage_count = current_coverage_index;
      return true;
    }

    default:
      return validator.invalid_table_format(table_name, format);
  }
}

static BL_NOINLINE bool validate_coverage_tables(
  ValidationContext& validator,
  RawTable table,
  const char* table_name,
  const char* coverage_name,
  const UInt16* offsets, uint32_t count, OffsetRange offset_range) noexcept {

  for (uint32_t i = 0; i < count; i++) {
    uint32_t offset = offsets[i].value();
    if (!offset)
      continue;

    if (!offset_range.contains(offset))
      return validator.fail("%s.%s[%u] offset (%u) is out of range [%u:%u]", table_name, coverage_name, i, offset, offset_range.start, offset_range.end);

    uint32_t unused_coverage_count;
    if (!validate_coverage_table(validator, table.sub_table<CoverageTable>(offset), unused_coverage_count))
      return false;
  }

  return true;
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Lookup Table Validation
// ================================================================

static BL_NOINLINE bool validate_lookup_with_coverage(ValidationContext& validator, RawTable data, const char* table_name, uint32_t header_size, uint32_t& coverage_count) noexcept {
  if (!data.fits(header_size))
    return validator.invalid_table_size(table_name, data.size, header_size);

  uint32_t coverage_offset = data.data_as<GSubGPosTable::LookupHeaderWithCoverage>()->coverage_offset();
  if (coverage_offset < header_size || coverage_offset >= data.size)
    return validator.fail("%s.coverage offset (%u) is out of range [%u:%u]", table_name, coverage_offset, header_size, data.size);

  return validate_coverage_table(validator, data.sub_table(coverage_offset), coverage_count);
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Sequence Context Validation
// ====================================================================

static BL_NOINLINE bool validate_sequence_lookup_record_array(ValidationContext& validator, const GSubGPosTable::SequenceLookupRecord* lookup_record_array, uint32_t lookup_record_count) noexcept {
  const LayoutData& layout_data = validator.ot_face_impl()->layout;
  uint32_t lookup_count = layout_data.by_kind(validator.lookup_kind()).lookup_count;

  for (uint32_t i = 0; i < lookup_record_count; i++) {
    const GSubGPosTable::SequenceLookupRecord& lookup_record = lookup_record_array[i];
    uint32_t lookup_index = lookup_record.lookup_index();

    if (lookup_index >= lookup_count)
      return validator.fail("SequenceLookupRecord[%u] has invalid lookup_index (%u) (lookup_count=%u)", i, lookup_index, lookup_count);
  }

  return true;
}

template<typename SequenceLookupTable>
static BL_NOINLINE bool validate_context_format1_2(ValidationContext& validator, Table<SequenceLookupTable> table, const char* table_name) noexcept {
  typedef GSubGPosTable::SequenceRule SequenceRule;
  typedef GSubGPosTable::SequenceRuleSet SequenceRuleSet;

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, SequenceLookupTable::kBaseSize, coverage_count))
    return false;

  uint32_t rule_set_count = table->rule_set_offsets.count();
  uint32_t header_size = SequenceLookupTable::kBaseSize + rule_set_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  const Offset16* rule_set_offset_array = table->rule_set_offsets.array();
  OffsetRange rule_set_offset_range{header_size, table.size - 4u};

  for (uint32_t i = 0; i < rule_set_count; i++) {
    uint32_t rule_set_offset = rule_set_offset_array[i].value();

    // Offsets are allowed to be null - this means that that SequenceRuleSet must be ignored.
    if (!rule_set_offset)
      continue;

    if (!rule_set_offset_range.contains(rule_set_offset))
      return validator.invalid_offset_entry(table_name, "sequence_rule_set_offset", i, rule_set_offset, rule_set_offset_range);

    Table<SequenceRuleSet> rule_set(table.sub_table(rule_set_offset));
    uint32_t rule_count = rule_set->count();

    if (!rule_count)
      return validator.fail("%s.rule_set[%u] cannot be empty", table_name, i);

    uint32_t rule_set_header_size = 2u + rule_count * 2u;
    if (!rule_set.fits(rule_set_header_size))
      return validator.fail("%s.rule_set[%u] is truncated (size=%u, required=%u)", table_name, i, rule_set.size, rule_set_header_size);

    const Offset16* rule_offset_array = rule_set->array();
    OffsetRange rule_offset_range{rule_set_header_size, rule_set.size - SequenceRule::kBaseSize};

    for (uint32_t rule_index = 0; rule_index < rule_count; rule_index++) {
      uint32_t rule_offset = rule_offset_array[rule_index].value();
      if (!rule_offset_range.contains(rule_offset))
        return validator.fail("%s.rule_set[%u].rule[%u] offset (%u) is out of range [%u:%u]", table_name, i, rule_index, rule_offset, rule_offset_range.start, rule_offset_range.end);

      Table<SequenceRule> rule = rule_set.sub_table(rule_offset);
      uint32_t glyph_count = rule->glyph_count();
      uint32_t lookup_record_count = rule->lookup_record_count();
      uint32_t rule_table_size = 4u + (lookup_record_count + glyph_count - 1) * 2u;

      if (!rule.fits(rule_table_size))
        return validator.fail("%s.rule_set[%u].rule[%u] is truncated (size=%u, required=%u)", table_name, i, rule_index, rule.size, rule_table_size);

      if (glyph_count < 2)
        return validator.fail("%s.rule_set[%u].rule[%u] has invalid glyph_count (%u)", table_name, i, rule_index, glyph_count);

      if (!lookup_record_count)
        return validator.fail("%s.rule_set[%u].rule[%u] has invalid lookup_record_count (%u)", table_name, i, rule_index, lookup_record_count);

      if (!validate_sequence_lookup_record_array(validator, rule->lookup_record_array(glyph_count), lookup_record_count))
        return false;
    }
  }

  return true;
}

static BL_INLINE bool validate_context_format1(ValidationContext& validator, Table<GSubGPosTable::SequenceContext1> table, const char* table_name) noexcept {
  return validate_context_format1_2<GSubGPosTable::SequenceContext1>(validator, table, table_name);
}

static BL_NOINLINE bool validate_context_format2(ValidationContext& validator, Table<GSubGPosTable::SequenceContext2> table, const char* table_name) noexcept {
  if (!table.fits())
    return validator.invalid_table_size(table_name, table.size, GSubGPosTable::SequenceContext2::kBaseSize);

  uint32_t rule_set_count = table->rule_set_offsets.count();
  uint32_t header_size = GSubGPosTable::SequenceContext2::kBaseSize + rule_set_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  uint32_t class_def_offset = table.data_as<GSubGPosTable::SequenceContext2>()->class_def_offset();
  OffsetRange offset_range{header_size, table.size};

  if (!offset_range.contains(class_def_offset))
    return validator.invalid_field_offset(table_name, "class_def_offset", class_def_offset, offset_range);

  if (!validate_class_def_table(validator, table.sub_table_unchecked(class_def_offset), "ClassDef"))
    return false;

  return validate_context_format1_2<GSubGPosTable::SequenceContext2>(validator, table, table_name);
}

static BL_NOINLINE bool validate_context_format3(ValidationContext& validator, Table<GSubGPosTable::SequenceContext3> table, const char* table_name) noexcept {
  if (!table.fits())
    return validator.invalid_table_size(table_name, table.size, GSubGPosTable::SequenceContext3::kBaseSize);

  uint32_t glyph_count = table->glyph_count();
  uint32_t lookup_record_count = table->lookup_record_count();
  uint32_t header_size = GSubGPosTable::SequenceContext3::kBaseSize + glyph_count * 2u + lookup_record_count * GPosTable::SequenceLookupRecord::kBaseSize;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  if (!glyph_count)
    return validator.invalid_field_value(table_name, "glyph_count", glyph_count);

  if (!lookup_record_count)
    return validator.invalid_field_value(table_name, "lookup_record_count", lookup_record_count);

  OffsetRange sub_table_offset_range{header_size, table.size};
  const UInt16* coverage_offset_array = table->coverage_offset_array();

  for (uint32_t coverage_table_index = 0; coverage_table_index < glyph_count; coverage_table_index++) {
    uint32_t coverage_table_offset = coverage_offset_array[coverage_table_index].value();
    if (!sub_table_offset_range.contains(coverage_table_offset))
      return validator.invalid_offset_entry(table_name, "coverage_offset", coverage_table_index, coverage_table_offset, sub_table_offset_range);

    uint32_t coverage_count;
    if (!validate_coverage_table(validator, table.sub_table<CoverageTable>(coverage_table_offset), coverage_count))
      return false;
  }

  return validate_sequence_lookup_record_array(validator, table->lookup_record_array(glyph_count), lookup_record_count);
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Sequence Context Utilities
// ===================================================================

struct SequenceMatch {
  uint32_t glyph_count;
  uint32_t lookup_record_count;
  const GSubGPosTable::SequenceLookupRecord* lookup_records;
};

static BL_INLINE bool match_sequence_rule_format1(Table<Array16<Offset16>> rule_offsets, uint32_t rule_count, const BLGlyphId* glyph_data, size_t max_glyph_count, SequenceMatch* match_out) noexcept {
  size_t maxGlyphCountMinus1 = max_glyph_count - 1u;
  for (uint32_t rule_index = 0; rule_index < rule_count; rule_index++) {
    uint32_t rule_offset = rule_offsets->array()[rule_index].value();
    BL_ASSERT_VALIDATED(rule_offset <= rule_offsets.size - 4u);

    const GSubGPosTable::SequenceRule* rule = PtrOps::offset<const GSubGPosTable::SequenceRule>(rule_offsets.data, rule_offset);
    uint32_t glyph_count = rule->glyph_count();
    uint32_t glyphCountMinus1 = glyph_count - 1u;

    if (glyphCountMinus1 > maxGlyphCountMinus1)
      continue;

    // This is safe - a single SequenceRule is 4 bytes that is followed by `GlyphId[glyph_count - 1]` and then
    // by `SequenceLookupRecord[sequence_lookup_count]`. Since we don't know whether we have a match or not we will
    // only check bounds required by matching and postponing `sequence_lookup_count` until we have an actual match.
    BL_ASSERT_VALIDATED(rule_offset + glyphCountMinus1 * 2u <= rule_offsets.size - 4u);

    uint32_t glyph_index = 0;
    for (;;) {
      BLGlyphId glyphA = rule->input_sequence()[glyph_index].value();
      BLGlyphId glyphB = glyph_data[++glyph_index];

      if (glyphA != glyphB)
        break;

      if (glyph_index < glyphCountMinus1)
        continue;

      BL_ASSERT_VALIDATED(rule->lookup_record_count() > 0);
      BL_ASSERT_VALIDATED(rule_offset + glyphCountMinus1 * 2u + rule->lookup_record_count() * 4u <= rule_offsets.size - 4u);

      *match_out = SequenceMatch{glyph_count, rule->lookup_record_count(), rule->lookup_record_array(glyph_count)};
      return true;
    }
  }

  return false;
}

template<uint32_t kCDFmt>
static BL_INLINE bool match_sequence_rule_format2(Table<Array16<Offset16>> rule_offsets, uint32_t rule_count, const BLGlyphId* glyph_data, size_t max_glyph_count, const ClassDefTableIterator& cd_it, SequenceMatch* match_out) noexcept {
  size_t maxGlyphCountMinus1 = max_glyph_count - 1u;
  for (uint32_t rule_index = 0; rule_index < rule_count; rule_index++) {
    uint32_t rule_offset = rule_offsets->array()[rule_index].value();
    BL_ASSERT_VALIDATED(rule_offset <= rule_offsets.size - 4u);

    const GSubGPosTable::SequenceRule* rule = PtrOps::offset<const GSubGPosTable::SequenceRule>(rule_offsets.data, rule_offset);
    uint32_t glyph_count = rule->glyph_count();
    uint32_t glyphCountMinus1 = glyph_count - 1u;

    if (glyphCountMinus1 > maxGlyphCountMinus1)
      continue;

    // This is safe - a single ClassSequenceRule is 4 bytes that is followed by `GlyphId[glyph_count - 1]` and then
    // by `SequenceLookupRecord[sequence_lookup_count]`. Since we don't know whether we have a match or not we will
    // only check bounds required by matching and postponing `sequence_lookup_count` until we have an actual match.
    BL_ASSERT_VALIDATED(rule_offset + glyphCountMinus1 * 2u <= rule_offsets.size - 4u);

    uint32_t glyph_index = 0;
    for (;;) {
      uint32_t class_value = rule->input_sequence()[glyph_index].value();
      BLGlyphId glyph_id = glyph_data[++glyph_index];

      if (!cd_it.match_glyph_class<kCDFmt>(glyph_id, class_value))
        break;

      if (glyph_index < glyphCountMinus1)
        continue;

      BL_ASSERT_VALIDATED(rule->lookup_record_count() > 0u);
      BL_ASSERT_VALIDATED(rule_offset + glyphCountMinus1 * 2u + rule->lookup_record_count() * 4u <= rule_offsets.size - 4u);

      *match_out = SequenceMatch{glyph_count, rule->lookup_record_count(), rule->lookup_record_array(glyph_count)};
      return true;
    }
  }

  return false;
}

template<uint32_t kCovFmt>
static BL_INLINE bool match_sequence_format1(Table<GSubGPosTable::SequenceContext1> table, uint32_t rule_set_count, GlyphRange first_glyph_range, const CoverageTableIterator& cov_it, const BLGlyphId* glyph_ptr, size_t max_glyph_count, SequenceMatch* match_out) noexcept {
  BLGlyphId glyph_id = glyph_ptr[0];
  if (!first_glyph_range.contains(glyph_id))
    return false;

  uint32_t coverage_index;
  if (!cov_it.find<kCovFmt>(glyph_id, coverage_index) || coverage_index >= rule_set_count)
    return false;

  uint32_t rule_set_offset = table->rule_set_offsets.array()[coverage_index].value();
  BL_ASSERT_VALIDATED(rule_set_offset <= table.size - 2u);

  Table<Array16<Offset16>> rule_offsets(table.sub_table_unchecked(rule_set_offset));
  uint32_t rule_count = rule_offsets->count();
  BL_ASSERT_VALIDATED(rule_count && rule_set_offset + rule_count * 2u <= table.size - 2u);

  return match_sequence_rule_format1(rule_offsets, rule_count, glyph_ptr, max_glyph_count, match_out);
}

template<uint32_t kCovFmt, uint32_t kCDFmt>
static BL_INLINE bool match_sequence_format2(
  Table<GSubGPosTable::SequenceContext2> table,
  uint32_t rule_set_count,
  GlyphRange first_glyph_range,
  const CoverageTableIterator& cov_it,
  const ClassDefTableIterator& cd_it,
  const BLGlyphId* glyph_ptr,
  size_t max_glyph_count,
  SequenceMatch* match_out) noexcept {

  BLGlyphId glyph_id = glyph_ptr[0];
  if (!first_glyph_range.contains(glyph_id))
    return false;

  uint32_t unused_coverage_index;
  if (!cov_it.find<kCovFmt>(glyph_id, unused_coverage_index))
    return false;

  uint32_t class_index = cd_it.class_of_glyph<kCDFmt>(glyph_id);
  if (class_index >= rule_set_count)
    return false;

  uint32_t rule_set_offset = table->rule_set_offsets.array()[class_index].value();
  BL_ASSERT_VALIDATED(rule_set_offset <= table.size - 2u);

  Table<Array16<Offset16>> rule_offsets(table.sub_table_unchecked(rule_set_offset));
  uint32_t rule_count = rule_offsets->count();
  BL_ASSERT_VALIDATED(rule_count && rule_set_offset + rule_count * 2u <= table.size - 2u);

  return match_sequence_rule_format2<kCDFmt>(rule_offsets, rule_count, glyph_ptr, max_glyph_count, cd_it, match_out);
}

static BL_INLINE bool match_sequence_format3(
  Table<GSubGPosTable::SequenceContext3> table,
  const UInt16* coverage_offset_array,
  GlyphRange first_glyph_range,
  const CoverageTableIterator& cov0_it,
  uint32_t cov0_fmt,
  const BLGlyphId* glyph_ptr,
  size_t glyph_count) noexcept {

  BLGlyphId glyph_id = glyph_ptr[0];
  if (!first_glyph_range.contains(glyph_id))
    return false;

  uint32_t unusedCoverageIndex0;
  if (!cov0_it.find_with_format(cov0_fmt, glyph_id, unusedCoverageIndex0))
    return false;

  for (size_t i = 1; i < glyph_count; i++) {
    CoverageTableIterator covItN;
    uint32_t covFmtN = covItN.init(table.sub_table_unchecked(coverage_offset_array[i].value()));
    GlyphRange glyphRangeN = covItN.glyph_range_with_format(covFmtN);

    uint32_t glyphIdN = glyph_ptr[i];
    uint32_t unusedCoverageIndexN;

    if (!glyphRangeN.contains(glyphIdN) || !covItN.find_with_format(covFmtN, glyphIdN, unusedCoverageIndexN))
      return false;
  }

  return true;
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Chained Sequence Context Validation
// ============================================================================

template<typename ChainedSequenceLookupTable>
static BL_NOINLINE bool validate_chained_context_format1_2(ValidationContext& validator, Table<ChainedSequenceLookupTable> table, const char* table_name) noexcept {
  typedef GSubGPosTable::ChainedSequenceRule ChainedSequenceRule;
  typedef GSubGPosTable::ChainedSequenceRuleSet ChainedSequenceRuleSet;

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, ChainedSequenceLookupTable::kBaseSize, coverage_count))
    return false;

  uint32_t rule_set_count = table->rule_set_offsets.count();
  uint32_t header_size = ChainedSequenceLookupTable::kBaseSize + rule_set_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  const Offset16* rule_set_offset_array = table->rule_set_offsets.array();
  OffsetRange rule_set_offset_range{header_size, table.size - 4u};

  for (uint32_t i = 0; i < rule_set_count; i++) {
    uint32_t rule_set_offset = rule_set_offset_array[i].value();

    // Offsets are allowed to be null - this means that that ChainedSequenceRuleSet must be ignored.
    if (!rule_set_offset)
      continue;

    if (!rule_set_offset_range.contains(rule_set_offset))
      return validator.invalid_offset_entry(table_name, "rule_set_offset", i, rule_set_offset, rule_set_offset_range);

    Table<ChainedSequenceRuleSet> rule_set(table.sub_table(rule_set_offset));
    uint32_t rule_count = rule_set->count();

    if (!rule_count)
      return validator.fail("%s.rule_set[%u] cannot be empty", table_name, i);

    uint32_t rule_set_header_size = 2u + rule_count * 2u;
    if (!rule_set.fits(rule_set_header_size))
      return validator.fail("%s.rule_set[%u] is truncated (size=%u, required=%u)", table_name, i, rule_set.size, rule_set_header_size);

    const Offset16* rule_offset_array = rule_set->array();
    OffsetRange rule_offset_range{rule_set_header_size, rule_set.size - ChainedSequenceRule::kBaseSize};

    for (uint32_t rule_index = 0; rule_index < rule_count; rule_index++) {
      uint32_t rule_offset = rule_offset_array[rule_index].value();
      if (!rule_offset_range.contains(rule_offset))
        return validator.fail("%s.rule_set[%u].rule[%u] offset (%u) is out of range [%u:%u]", table_name, i, rule_index, rule_offset, rule_offset_range.start, rule_offset_range.end);

      Table<ChainedSequenceRule> rule = rule_set.sub_table(rule_offset);
      uint32_t backtrack_glyph_count = rule->backtrack_glyph_count();

      // Verify there is a room for `backgrack_glyph_count + backtrack_sequence + input_glyph_count`.
      uint32_t input_glyph_offset = 2u + backtrack_glyph_count * 2u;
      if (!rule.fits(input_glyph_offset + 2u))
        return validator.fail("%s.rule_set[%u].rule[%u] is truncated (size=%u, required=%u)", table_name, i, rule_index, rule.size, input_glyph_offset + 2u);

      uint32_t input_glyph_count = rule.readU16(input_glyph_offset);
      if (!input_glyph_count)
        return validator.fail("%s.rule_set[%u].rule[%u] has invalid input_glyph_count (%u)", table_name, i, rule_index, input_glyph_count);

      // Verify there is a room for `input_glyph_count + input_sequence + lookahead_glyph_count`.
      uint32_t lookahead_offset = input_glyph_offset + 2u + (input_glyph_count - 1u) * 2u;
      if (!rule.fits(lookahead_offset + 2u))
        return validator.fail("%s.rule_set[%u].rule[%u] is truncated (size=%u, required=%u)", table_name, i, rule_index, rule.size, lookahead_offset + 2u);

      // Verify there is a room for `lookahead_sequence + lookup_record_count`.
      uint32_t lookahead_glyph_count = rule.readU16(lookahead_offset);
      uint32_t lookup_record_offset = lookahead_offset + lookahead_glyph_count * 2u;
      if (!rule.fits(lookup_record_offset + 2u))
        return validator.fail("%s.rule_set[%u].rule[%u] is truncated (size=%u, required=%u)", table_name, i, rule_index, rule.size, lookup_record_offset + 2u);

      uint32_t lookup_record_count = rule.readU16(lookup_record_offset);
      if (!lookup_record_count)
        return validator.fail("%s.rule_set[%u].rule[%u] has invalid lookup_record_count (%u)", table_name, i, rule_index, lookup_record_count);

      const GSubGPosTable::SequenceLookupRecord* lookup_record_array = PtrOps::offset<const GSubGPosTable::SequenceLookupRecord>(rule.data, lookup_record_offset + 2u);
      if (!validate_sequence_lookup_record_array(validator, lookup_record_array, lookup_record_count))
        return false;
    }
  }

  return true;
}

static BL_INLINE bool validate_chained_context_format1(ValidationContext& validator, Table<GSubGPosTable::ChainedSequenceContext1> table, const char* table_name) noexcept {
  return validate_chained_context_format1_2(validator, table, table_name);
}

static BL_NOINLINE bool validate_chained_context_format2(ValidationContext& validator, Table<GSubGPosTable::ChainedSequenceContext2> table, const char* table_name) noexcept {
  if (!table.fits())
    return validator.invalid_table_size(table_name, table.size, GSubGPosTable::SequenceContext2::kBaseSize);

  uint32_t rule_set_count = table->rule_set_offsets.count();
  uint32_t header_size = GSubGPosTable::SequenceContext2::kBaseSize + rule_set_count * 2u;
  OffsetRange offset_range{header_size, table.size};

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  uint32_t backtrack_class_def_offset = table->backtrack_class_def_offset();
  uint32_t input_class_def_offset = table->input_class_def_offset();
  uint32_t lookahead_class_def_offset = table->lookahead_class_def_offset();

  if (!offset_range.contains(backtrack_class_def_offset))
    return validator.invalid_field_offset(table_name, "backtrack_class_def_offset", backtrack_class_def_offset, offset_range);

  if (!offset_range.contains(input_class_def_offset))
    return validator.invalid_field_offset(table_name, "input_class_def_offset", input_class_def_offset, offset_range);

  if (!offset_range.contains(lookahead_class_def_offset))
    return validator.invalid_field_offset(table_name, "lookahead_class_def_offset", lookahead_class_def_offset, offset_range);

  if (!validate_class_def_table(validator, table.sub_table_unchecked(backtrack_class_def_offset), "backtrack_class_def"))
    return false;

  if (!validate_class_def_table(validator, table.sub_table_unchecked(input_class_def_offset), "input_class_def"))
    return false;

  if (!validate_class_def_table(validator, table.sub_table_unchecked(lookahead_class_def_offset), "lookahead_class_def"))
    return false;

  return validate_chained_context_format1_2(validator, table, table_name);
}

static BL_NOINLINE bool validate_chained_context_format3(ValidationContext& validator, Table<GSubGPosTable::ChainedSequenceContext3> table, const char* table_name) noexcept {
  if (!table.fits())
    return validator.invalid_table_size(table_name, table.size, GSubGPosTable::ChainedSequenceContext3::kBaseSize);

  uint32_t backtrack_glyph_count = table->backtrack_glyph_count();
  uint32_t input_glyph_count_offset = 4u + backtrack_glyph_count * 2u;

  if (!table.fits(input_glyph_count_offset + 2u))
    return validator.invalid_table_size(table_name, table.size, input_glyph_count_offset + 2u);

  uint32_t input_glyph_count = table.readU16(input_glyph_count_offset);
  uint32_t lookahead_glyph_count_offset = input_glyph_count_offset + 2u + input_glyph_count * 2u;

  if (!table.fits(lookahead_glyph_count_offset + 2u))
    return validator.invalid_table_size(table_name, table.size, lookahead_glyph_count_offset + 2u);

  uint32_t lookahead_glyph_count = table.readU16(lookahead_glyph_count_offset);
  uint32_t lookup_record_count_offset = lookahead_glyph_count_offset + 2u + lookahead_glyph_count * 2u;

  if (!table.fits(lookup_record_count_offset + 2u))
    return validator.invalid_table_size(table_name, table.size, lookup_record_count_offset + 2u);

  uint32_t lookup_record_count = table.readU16(lookup_record_count_offset);
  uint32_t header_size = lookup_record_count_offset + 2u + lookup_record_count * GSubGPosTable::SequenceLookupRecord::kBaseSize;

  if (!lookup_record_count)
    return validator.fail("%s has no lookup records", table_name);

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  OffsetRange offset_range{header_size, table.size - 2u};

  const UInt16* backtrack_coverage_offsets = table->backtrack_coverage_offsets();
  const UInt16* input_glyph_coverage_offsets = PtrOps::offset<const UInt16>(table.data, input_glyph_count_offset + 2u);
  const UInt16* lookahead_coverage_offsets = PtrOps::offset<const UInt16>(table.data, lookahead_glyph_count_offset + 2u);

  if (!validate_coverage_tables(validator, table, table_name, "backtrack", backtrack_coverage_offsets, backtrack_glyph_count, offset_range))
    return false;

  if (!validate_coverage_tables(validator, table, table_name, "input", input_glyph_coverage_offsets, input_glyph_count, offset_range))
    return false;

  if (!validate_coverage_tables(validator, table, table_name, "lookahead", lookahead_coverage_offsets, lookahead_glyph_count, offset_range))
    return false;

  const GSubGPosTable::SequenceLookupRecord* lookup_record_array = PtrOps::offset<const GSubGPosTable::SequenceLookupRecord>(table.data, lookup_record_count_offset + 2u);
  return validate_sequence_lookup_record_array(validator, lookup_record_array, lookup_record_count);
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Chained Sequence Context Lookup
// ========================================================================

struct ChainedMatchContext {
  RawTable table;
  GlyphRange first_glyph_range;

  BLGlyphId* back_glyph_ptr;
  BLGlyphId* ahead_glyph_ptr;

  size_t back_glyph_count;
  size_t ahead_glyph_count;
};

static BL_INLINE bool match_back_glyphs_format1(const BLGlyphId* glyph_ptr, const UInt16* match_sequence, size_t count) noexcept {
  const BLGlyphId* glyph_start = glyph_ptr - count;

  while (glyph_ptr != glyph_start) {
    if (glyph_ptr[-1] != match_sequence[0].value())
      return false;
    glyph_ptr--;
    match_sequence++;
  }

  return true;
}

template<uint32_t kCDFmt>
static BL_INLINE bool match_back_glyphs_format2(const BLGlyphId* glyph_ptr, const UInt16* match_sequence, size_t count, const ClassDefTableIterator& cd_it) noexcept {
  const BLGlyphId* glyph_start = glyph_ptr - count;

  while (glyph_ptr != glyph_start) {
    BLGlyphId glyph_id = glyph_ptr[-1];
    uint32_t class_value = match_sequence[0].value();

    if (!cd_it.match_glyph_class<kCDFmt>(glyph_id, class_value))
      return false;

    glyph_ptr--;
    match_sequence++;
  }

  return true;
}

static BL_INLINE bool match_back_glyphs_format3(RawTable main_table, const BLGlyphId* glyph_ptr, const Offset16* backtrack_coverage_offset_array, size_t count) noexcept {
  for (size_t i = 0; i < count; i++, glyph_ptr--) {
    CoverageTableIterator cov_it;
    uint32_t cov_fmt = cov_it.init(main_table.sub_table_unchecked(backtrack_coverage_offset_array[i].value()));
    BLGlyphId glyph_id = glyph_ptr[0];

    if (cov_fmt == 1) {
      uint32_t unused_coverage_index;
      if (!cov_it.glyph_range<1>().contains(glyph_id) || !cov_it.find<1>(glyph_id, unused_coverage_index))
        return false;
    }
    else {
      uint32_t unused_coverage_index;
      if (!cov_it.glyph_range<2>().contains(glyph_id) || !cov_it.find<2>(glyph_id, unused_coverage_index))
        return false;
    }
  }

  return true;
}

static BL_INLINE bool match_ahead_glyphs_format1(const BLGlyphId* glyph_ptr, const UInt16* match_sequence, size_t count) noexcept {
  for (size_t i = 0; i < count; i++)
    if (glyph_ptr[i] != match_sequence[i].value())
      return false;
  return true;
}

template<uint32_t kCDFmt>
static BL_INLINE bool match_ahead_glyphs_format2(const BLGlyphId* glyph_ptr, const UInt16* match_sequence, size_t count, const ClassDefTableIterator& cd_it) noexcept {
  for (size_t i = 0; i < count; i++) {
    BLGlyphId glyph_id = glyph_ptr[i];
    uint32_t class_value = match_sequence[i].value();

    if (!cd_it.match_glyph_class<kCDFmt>(glyph_id, class_value))
      return false;
  }
  return true;
}

static BL_INLINE bool match_ahead_glyphs_format3(RawTable main_table, const BLGlyphId* glyph_ptr, const Offset16* lookahead_coverage_offset_array, size_t count) noexcept {
  for (size_t i = 0; i < count; i++) {
    CoverageTableIterator cov_it;
    uint32_t cov_fmt = cov_it.init(main_table.sub_table_unchecked(lookahead_coverage_offset_array[i].value()));
    BLGlyphId glyph_id = glyph_ptr[i];

    if (cov_fmt == 1) {
      uint32_t unused_coverage_index;
      if (!cov_it.glyph_range<1>().contains(glyph_id) || !cov_it.find<1>(glyph_id, unused_coverage_index))
        return false;
    }
    else {
      uint32_t unused_coverage_index;
      if (!cov_it.glyph_range<2>().contains(glyph_id) || !cov_it.find<2>(glyph_id, unused_coverage_index))
        return false;
    }
  }

  return true;
}

static BL_INLINE bool match_chained_sequence_rule_format1(
  ChainedMatchContext& mCtx,
  Table<Array16<Offset16>> rule_offsets, uint32_t rule_count,
  SequenceMatch* match_out) noexcept {

  for (uint32_t rule_index = 0; rule_index < rule_count; rule_index++) {
    uint32_t rule_offset = rule_offsets->array()[rule_index].value();
    BL_ASSERT_VALIDATED(rule_offset <= rule_offsets.size - GSubGPosTable::ChainedSequenceRule::kBaseSize);

    Table<GSubGPosTable::ChainedSequenceRule> rule = rule_offsets.sub_table_unchecked(rule_offset);
    uint32_t backtrack_glyph_count = rule->backtrack_glyph_count();

    uint32_t input_glyph_offset = 2u + backtrack_glyph_count * 2u;
    BL_ASSERT_VALIDATED(rule.fits(input_glyph_offset + 2u));

    uint32_t input_glyph_count = rule.readU16(input_glyph_offset);
    BL_ASSERT_VALIDATED(input_glyph_count != 0u);

    uint32_t lookahead_offset = input_glyph_offset + 2u + input_glyph_count * 2u - 2u;
    BL_ASSERT_VALIDATED(rule.fits(lookahead_offset + 2u));

    // Multiple conditions merged into a single one that results in a branch.
    uint32_t lookahead_glyph_count = rule.readU16(lookahead_offset);
    if (unsigned(mCtx.back_glyph_count < backtrack_glyph_count) | unsigned(mCtx.ahead_glyph_count < input_glyph_count + lookahead_glyph_count))
      continue;

    // Match backtrack glyphs, which are stored in reverse order in backtrack array, index 0 describes the last glyph).
    if (!match_back_glyphs_format1(mCtx.back_glyph_ptr + mCtx.back_glyph_count, rule->backtrack_sequence(), backtrack_glyph_count))
      continue;

    // Match input and lookahead glyphs.
    if (!match_ahead_glyphs_format1(mCtx.ahead_glyph_ptr, rule.data_as<UInt16>(input_glyph_offset + 2u), input_glyph_count - 1u))
      continue;

    if (!match_ahead_glyphs_format1(mCtx.ahead_glyph_ptr + input_glyph_count - 1u, rule.data_as<UInt16>(lookahead_offset + 2u), lookahead_glyph_count))
      continue;

    uint32_t lookup_record_offset = lookahead_offset + lookahead_glyph_count * 2u;
    BL_ASSERT_VALIDATED(rule.fits(lookup_record_offset + 2u));

    uint32_t lookup_record_count = rule.readU16(lookup_record_offset);
    BL_ASSERT_VALIDATED(rule.fits(lookup_record_offset + 2u + lookup_record_count * GSubGPosTable::SequenceLookupRecord::kBaseSize));

    *match_out = SequenceMatch{input_glyph_count, lookup_record_count, rule.data_as<GSubGPosTable::SequenceLookupRecord>(lookup_record_offset + 2u)};
    return true;
  }

  return false;
}

template<uint32_t kCD1Fmt, uint32_t kCD2Fmt, uint32_t kCD3Fmt>
static BL_INLINE bool match_chained_sequence_rule_format2(
  ChainedMatchContext& mCtx,
  Table<Array16<Offset16>> rule_offsets, uint32_t rule_count,
  const ClassDefTableIterator& cd1_it, const ClassDefTableIterator& cd2_it, const ClassDefTableIterator& cd3_it,
  SequenceMatch* match_out) noexcept {

  for (uint32_t rule_index = 0; rule_index < rule_count; rule_index++) {
    uint32_t rule_offset = rule_offsets->array()[rule_index].value();
    BL_ASSERT_VALIDATED(rule_offset <= rule_offsets.size - GSubGPosTable::ChainedSequenceRule::kBaseSize);

    Table<GSubGPosTable::ChainedSequenceRule> rule = rule_offsets.sub_table_unchecked(rule_offset);
    uint32_t backtrack_glyph_count = rule->backtrack_glyph_count();

    uint32_t input_glyph_offset = 2u + backtrack_glyph_count * 2u;
    BL_ASSERT_VALIDATED(rule.fits(input_glyph_offset + 2u));

    uint32_t input_glyph_count = rule.readU16(input_glyph_offset);
    BL_ASSERT_VALIDATED(input_glyph_count != 0u);

    uint32_t lookahead_offset = input_glyph_offset + 2u + input_glyph_count * 2u - 2u;
    BL_ASSERT_VALIDATED(rule.fits(lookahead_offset + 2u));

    // Multiple conditions merged into a single one that results in a branch.
    uint32_t lookahead_glyph_count = rule.readU16(lookahead_offset);
    if (unsigned(mCtx.back_glyph_count < backtrack_glyph_count) | unsigned(mCtx.ahead_glyph_count < input_glyph_count + lookahead_glyph_count))
      continue;

    // Match backtrack glyphs, which are stored in reverse order in backtrack array, index 0 describes the last glyph).
    if (!match_back_glyphs_format2<kCD1Fmt>(mCtx.back_glyph_ptr + mCtx.back_glyph_count, rule->backtrack_sequence(), backtrack_glyph_count, cd1_it))
      continue;

    // Match input and lookahead glyphs.
    if (!match_ahead_glyphs_format2<kCD2Fmt>(mCtx.ahead_glyph_ptr, rule.data_as<UInt16>(input_glyph_offset + 2u), input_glyph_count - 1u, cd2_it))
      continue;

    if (!match_ahead_glyphs_format2<kCD3Fmt>(mCtx.ahead_glyph_ptr + input_glyph_count - 1u, rule.data_as<UInt16>(lookahead_offset + 2u), lookahead_glyph_count, cd3_it))
      continue;

    uint32_t lookup_record_offset = lookahead_offset + lookahead_glyph_count * 2u;
    BL_ASSERT_VALIDATED(rule.fits(lookup_record_offset + 2u));

    uint32_t lookup_record_count = rule.readU16(lookup_record_offset);
    BL_ASSERT_VALIDATED(rule.fits(lookup_record_offset + 2u + lookup_record_count * GSubGPosTable::SequenceLookupRecord::kBaseSize));

    *match_out = SequenceMatch{input_glyph_count, lookup_record_count, rule.data_as<GSubGPosTable::SequenceLookupRecord>(lookup_record_offset + 2u)};
    return true;
  }

  return false;
}

template<uint32_t kCovFmt>
static BL_INLINE bool match_chained_sequence_format1(
  ChainedMatchContext& mCtx,
  const Offset16* rule_set_offsets, uint32_t rule_set_count,
  const CoverageTableIterator& cov_it,
  SequenceMatch* match_out) noexcept {

  BLGlyphId glyph_id = mCtx.ahead_glyph_ptr[0];
  if (!mCtx.first_glyph_range.contains(glyph_id))
    return false;

  uint32_t coverage_index;
  if (!cov_it.find<kCovFmt>(glyph_id, coverage_index) || coverage_index >= rule_set_count)
    return false;

  uint32_t rule_set_offset = rule_set_offsets[coverage_index].value();
  BL_ASSERT_VALIDATED(rule_set_offset <= mCtx.table.size - 2u);

  Table<Array16<Offset16>> rule_offsets(mCtx.table.sub_table_unchecked(rule_set_offset));
  uint32_t rule_count = rule_offsets->count();
  BL_ASSERT_VALIDATED(rule_count && rule_set_offset + rule_count * 2u <= mCtx.table.size - 2u);

  return match_chained_sequence_rule_format1(mCtx, rule_offsets, rule_count, match_out);
}

template<uint32_t kCovFmt, uint32_t kCD1Fmt, uint32_t kCD2Fmt, uint32_t kCD3Fmt>
static BL_INLINE bool match_chained_sequence_format2(
  ChainedMatchContext& mCtx,
  const Offset16* rule_set_offsets, uint32_t rule_set_count,
  const CoverageTableIterator& cov_it, const ClassDefTableIterator& cd1_it, const ClassDefTableIterator& cd2_it, const ClassDefTableIterator& cd3_it,
  SequenceMatch* match_out) noexcept {

  BLGlyphId glyph_id = mCtx.ahead_glyph_ptr[0];
  if (!mCtx.first_glyph_range.contains(glyph_id))
    return false;

  uint32_t coverage_index;
  if (!cov_it.find<kCovFmt>(glyph_id, coverage_index) || coverage_index >= rule_set_count)
    return false;

  uint32_t rule_set_offset = rule_set_offsets[coverage_index].value();
  BL_ASSERT_VALIDATED(rule_set_offset <= mCtx.table.size - 2u);

  Table<Array16<Offset16>> rule_offsets(mCtx.table.sub_table_unchecked(rule_set_offset));
  uint32_t rule_count = rule_offsets->count();
  BL_ASSERT_VALIDATED(rule_count && rule_set_offset + rule_count * 2u <= mCtx.table.size - 2u);

  return match_chained_sequence_rule_format2<kCD1Fmt, kCD2Fmt, kCD3Fmt>(mCtx, rule_offsets, rule_count, cd1_it, cd2_it, cd3_it, match_out);
}

static BL_INLINE bool match_chained_sequence_format3(
  ChainedMatchContext& mCtx,
  const UInt16* backtrack_coverage_offset_array, uint32_t backtrack_glyph_count,
  const UInt16* input_coverage_offset_array, uint32_t input_glyph_count,
  const UInt16* lookahead_coverage_offset_array, uint32_t lookahead_glyph_count,
  GlyphRange first_glyph_range,
  CoverageTableIterator& cov0_it, uint32_t cov0_fmt) noexcept {

  BL_ASSERT(mCtx.back_glyph_count >= backtrack_glyph_count);
  BL_ASSERT(mCtx.ahead_glyph_count >= input_glyph_count + lookahead_glyph_count);

  BLGlyphId glyph_id = mCtx.ahead_glyph_ptr[0];
  if (!first_glyph_range.contains(glyph_id))
    return false;

  uint32_t unusedCoverageIndex0;
  if (!cov0_it.find_with_format(cov0_fmt, glyph_id, unusedCoverageIndex0))
    return false;

  for (uint32_t i = 1; i < input_glyph_count; i++) {
    CoverageTableIterator covItN;
    uint32_t covFmtN = covItN.init(mCtx.table.sub_table_unchecked(input_coverage_offset_array[i].value()));
    GlyphRange glyphRangeN = covItN.glyph_range_with_format(covFmtN);

    uint32_t glyphIdN = mCtx.ahead_glyph_ptr[i];
    uint32_t unusedCoverageIndexN;

    if (!glyphRangeN.contains(glyphIdN) || !covItN.find_with_format(covFmtN, glyphIdN, unusedCoverageIndexN))
      return false;
  }

  return match_back_glyphs_format3(mCtx.table, mCtx.back_glyph_ptr + mCtx.back_glyph_count - 1u, backtrack_coverage_offset_array, backtrack_glyph_count) &&
         match_ahead_glyphs_format3(mCtx.table, mCtx.ahead_glyph_ptr + input_glyph_count, lookahead_coverage_offset_array, lookahead_glyph_count);
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #1 - Single Substitution Validation
// =================================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gsub_lookup_type1_format1(ValidationContext& validator, Table<GSubTable::SingleSubst1> table) noexcept {
  const char* table_name = "SingleSubst1";
  uint32_t unused_coverage_count;
  return validate_lookup_with_coverage(validator, table, table_name, GSubTable::SingleSubst1::kBaseSize, unused_coverage_count);
}

static BL_INLINE_IF_NOT_DEBUG bool validate_gsub_lookup_type1_format2(ValidationContext& validator, Table<GSubTable::SingleSubst2> table) noexcept {
  const char* table_name = "SingleSubst2";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GSubTable::SingleSubst2::kBaseSize, coverage_count))
    return false;

  const GSubTable::SingleSubst2* lookup = table.data_as<GSubTable::SingleSubst2>();
  uint32_t glyph_count = lookup->glyphs.count();
  uint32_t header_size = GSubTable::SingleSubst2::kBaseSize + glyph_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  if (glyph_count < coverage_count)
    validator.warn("%s has less glyphs (%u) than coverage entries (%u)", table_name, glyph_count, coverage_count);

  return true;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #1 - Single Substitution Lookup
// =============================================================================

template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type1_format1(GSubContext& ctx, Table<GSubTable::SingleSubst1> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  BLGlyphId* glyph_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_end = ctx.glyph_data() + scope.end();

  // Cannot apply a lookup if there is no data to be processed.
  BL_ASSERT(glyph_ptr != glyph_end);

  uint32_t glyph_delta = uint16_t(table->delta_glyph_id());
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    BLGlyphId glyph_id = glyph_ptr[0];
    if (glyph_range.contains(glyph_id)) {
      uint32_t unused_coverage_index;
      if (cov_it.find<kCovFmt>(glyph_id, unused_coverage_index))
        glyph_ptr[0] = (glyph_id + glyph_delta) & 0xFFFFu;
    }
  } while (scope.is_range() && ++glyph_ptr != glyph_end);

  return BL_SUCCESS;
}

template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type1_format2(GSubContext& ctx, Table<GSubTable::SingleSubst2> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  BLGlyphId* glyph_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_end = ctx.glyph_data() + scope.end();

  // Cannot apply a lookup if there is no data to be processed.
  BL_ASSERT(glyph_ptr != glyph_end);

  uint32_t subst_count = table->glyphs.count();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::SingleSubst2::kBaseSize + subst_count * 2u));

  do {
    BLGlyphId glyph_id = glyph_ptr[0];
    if (glyph_range.contains(glyph_id)) {
      uint32_t coverage_index;
      if (cov_it.find<kCovFmt>(glyph_id, coverage_index) && coverage_index < subst_count) {
        glyph_ptr[0] = table->glyphs.array()[coverage_index].value();
      }
    }
  } while (scope.is_range() && ++glyph_ptr != glyph_end);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #2 - Multiple Substitution Validation
// ===================================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gsub_lookup_type2_format1(ValidationContext& validator, Table<GSubTable::MultipleSubst1> table) noexcept {
  const char* table_name = "MultipleSubst1";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GSubTable::MultipleSubst1::kBaseSize, coverage_count))
    return false;

  uint32_t sequence_set_count = table->sequence_offsets.count();
  uint32_t header_size = GSubTable::MultipleSubst1::kBaseSize + sequence_set_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  if (sequence_set_count < coverage_count)
    validator.warn("%s has less sequence sets (%u) than coverage entries (%u)", table_name, sequence_set_count, coverage_count);

  // Offsets to glyph sequences.
  const Offset16* offset_array = table->sequence_offsets.array();
  OffsetRange offset_range{header_size, table.size - 4u};

  for (uint32_t i = 0; i < sequence_set_count; i++) {
    uint32_t sequence_offset = offset_array[i].value();
    if (!offset_range.contains(sequence_offset))
      return validator.invalid_offset_entry(table_name, "sequence_offsets", i, sequence_offset, offset_range);

    Table<Array16<UInt16>> sequence(table.sub_table(sequence_offset));

    // NOTE: The OpenType specification explicitly forbids empty sequences (aka removing glyphs), however,
    // this is actually used in practice (actually by fonts from MS), so we just allow it as others do...
    uint32_t sequence_length = sequence->count();
    uint32_t sequence_table_size = 2u + sequence_length * 2u;

    if (sequence.fits(sequence_table_size))
      return validator.fail("%s.sequence[%u] is truncated (size=%u, required=%u)", table_name, i, sequence.size, sequence_table_size);
  }

  return true;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #2 - Multiple Substitution Lookup
// ===============================================================================

// TODO: [OpenType] [SECURITY] What if the glyph contains kSeqMask???
template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type2_format1(GSubContext& ctx, Table<GSubTable::MultipleSubst1> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  constexpr BLGlyphId kSequenceMarker = 0x80000000u;

  BLGlyphId* glyph_in_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_in_end = ctx.glyph_data() + scope.end();

  // Cannot apply a lookup if there is no data to be processed.
  BL_ASSERT(glyph_in_ptr != glyph_in_end);

  uint32_t sequence_set_count = table->sequence_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::MultipleSubst1::kBaseSize + sequence_set_count * 2u));

  size_t replaced_glyph_count = 0;
  size_t replaced_sequence_size = 0;
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    BLGlyphId glyph_id = glyph_in_ptr[0];
    if (glyph_range.contains(glyph_id)) {
      uint32_t coverage_index;
      if (cov_it.find<kCovFmt>(glyph_id, coverage_index) && coverage_index < sequence_set_count) {
        uint32_t sequence_offset = table->sequence_offsets.array()[coverage_index].value();
        BL_ASSERT_VALIDATED(sequence_offset <= table.size - 2u);

        uint32_t sequence_length = MemOps::readU16uBE(table.data + sequence_offset);
        BL_ASSERT_VALIDATED(sequence_offset + sequence_length * 2u <= table.size - 2u);

        glyph_in_ptr[0] = sequence_offset | kSequenceMarker;
        replaced_glyph_count++;
        replaced_sequence_size += sequence_length;
      }
    }
  } while (scope.is_range() && ++glyph_in_ptr != glyph_in_end);

  // Not a single match if zero.
  if (!replaced_glyph_count)
    return BL_SUCCESS;

  // We could be only processing a range withing the work buffer, thus it's not safe to reuse `glyph_in_ptr`.
  glyph_in_ptr = ctx.glyph_data() + ctx.size();

  BLGlyphId* glyph_in_start = ctx.glyph_data();
  BLGlyphInfo* info_in_ptr = ctx.info_data() + ctx.size();

  size_t size_after = ctx.size() - replaced_glyph_count + replaced_sequence_size;
  BL_PROPAGATE(ctx.ensure_work_buffer(size_after));

  BLGlyphId* glyph_out_ptr = ctx.glyph_data() + size_after;
  BLGlyphInfo* info_out_ptr = ctx.info_data() + size_after;

  // Second loop applies all matches that were found and marked.
  do {
    BLGlyphId glyph_id = *--glyph_in_ptr;
    *--glyph_out_ptr = glyph_id;
    *--info_out_ptr = *--info_in_ptr;

    if (glyph_id & kSequenceMarker) {
      size_t sequence_offset = glyph_id & ~kSequenceMarker;
      size_t sequence_length = MemOps::readU16uBE(table.data + sequence_offset);
      const UInt16* sequence_data = table.data_as<UInt16>(sequence_offset + 2u);

      glyph_out_ptr -= sequence_length;
      info_out_ptr -= sequence_length;

      while (sequence_length) {
        sequence_length--;
        glyph_out_ptr[sequence_length] = sequence_data[sequence_length].value();
        info_out_ptr[sequence_length] = *info_in_ptr;
      }
    }
  } while (glyph_in_ptr != glyph_in_start);

  BL_ASSERT(glyph_out_ptr == ctx.glyph_data());
  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #3 - Alternate Substitution Validation
// ====================================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gsub_lookup_type3_format1(ValidationContext& validator, RawTable table) noexcept {
  const char* table_name = "AlternateSubst1";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GSubTable::AlternateSubst1::kBaseSize, coverage_count))
    return false;

  const GSubTable::AlternateSubst1* lookup = table.data_as<GSubTable::AlternateSubst1>();
  uint32_t alternate_set_count = lookup->alternate_set_offsets.count();
  uint32_t header_size = GSubTable::AlternateSubst1::kBaseSize + alternate_set_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  // Offsets to AlternateSet tables.
  const Offset16* offset_array = lookup->alternate_set_offsets.array();
  OffsetRange offset_range{header_size, table.size - 4u};

  if (alternate_set_count < coverage_count)
    validator.warn("%s has less AlternateSet records (%u) than coverage entries (%u)", table_name, alternate_set_count, coverage_count);

  for (uint32_t i = 0; i < alternate_set_count; i++) {
    uint32_t offset = offset_array[i].value();
    if (!offset_range.contains(offset))
      return validator.invalid_offset_entry(table_name, "alternate_set_offsets", i, offset, offset_range);

    const Array16<UInt16>* alternate_set = PtrOps::offset<const Array16<UInt16>>(table.data, offset);
    uint32_t alternate_set_length = alternate_set->count();

    // Specification forbids an empty AlternateSet.
    if (!alternate_set_length)
      return validator.fail("%s.alternate_set[%u] cannot be empty", table_name, i);

    uint32_t alternate_set_table_end = offset + 2u + alternate_set_length * 2u;
    if (alternate_set_table_end > table.size)
      return validator.fail("%s.alternate_set[%u] overflows table size by %u bytes", table_name, i, table.size - alternate_set_table_end);
  }

  return true;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #3 - Alternate Substitution Lookup
// ================================================================================

template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type3_format1(GSubContext& ctx, Table<GSubTable::AlternateSubst1> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  BLGlyphId* glyph_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_end = ctx.glyph_data() + scope.end();

  // Cannot apply a lookup if there is no data to be processed.
  BL_ASSERT(glyph_ptr != glyph_end);

  uint32_t alternate_set_count = table->alternate_set_offsets.count();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::AlternateSubst1::kBaseSize + alternate_set_count * 2u));

  // TODO: [OpenType] Not sure how the index should be selected (AlternateSubst1).
  uint32_t selected_index = 0u;

  do {
    BLGlyphId glyph_id = glyph_ptr[0];
    if (glyph_range.contains(glyph_id)) {
      uint32_t coverage_index;
      if (cov_it.find<kCovFmt>(glyph_id, coverage_index) && coverage_index < alternate_set_count) {
        uint32_t alternate_set_offset = table->alternate_set_offsets.array()[coverage_index].value();
        BL_ASSERT_VALIDATED(alternate_set_offset <= table.size - 2u);

        const UInt16* alts = reinterpret_cast<const UInt16*>(table.data + alternate_set_offset + 2u);
        uint32_t alt_glyph_count = alts[-1].value();
        BL_ASSERT_VALIDATED(alt_glyph_count != 0u && alternate_set_offset + alt_glyph_count * 2u <= table.size - 2u);

        uint32_t alt_glyph_index = (selected_index % alt_glyph_count);
        glyph_ptr[0] = alts[alt_glyph_index].value();
      }
    }
  } while (scope.is_range() && ++glyph_ptr != glyph_end);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #4 - Ligature Substitution Validation
// ===================================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gsub_lookup_type4_format1(ValidationContext& validator, Table<GSubTable::LigatureSubst1> table) noexcept {
  const char* table_name = "LigatureSubst1";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GSubTable::LigatureSubst1::kBaseSize, coverage_count))
    return false;

  const GSubTable::LigatureSubst1* lookup = table.data_as<GSubTable::LigatureSubst1>();
  uint32_t ligature_set_count = lookup->ligature_set_offsets.count();
  uint32_t header_size = GSubTable::LigatureSubst1::kBaseSize + ligature_set_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  if (ligature_set_count < coverage_count)
    validator.warn("%s has less LigatureSet records (%u) than coverage entries (%u)", table_name, ligature_set_count, coverage_count);

  // Offsets to LigatureSet tables.
  const Offset16* ligature_set_offset_array = lookup->ligature_set_offsets.array();
  OffsetRange ligature_set_offset_range{header_size, table.size-4u};

  for (uint32_t i = 0; i < ligature_set_count; i++) {
    uint32_t ligature_set_offset = ligature_set_offset_array[i].value();
    if (!ligature_set_offset_range.contains(ligature_set_offset))
      return validator.invalid_offset_entry(table_name, "ligature_set_offsets", i, ligature_set_offset, ligature_set_offset_range);

    Table<Array16<UInt16>> ligature_set(table.sub_table(ligature_set_offset));

    uint32_t ligature_count = ligature_set->count();
    if (!ligature_count)
      return validator.fail("%s.ligature_set[%u] cannot be empty", table_name, i);

    uint32_t ligature_set_header_size = 2u + ligature_count * 2u;
    if (!ligature_set.fits(ligature_set_header_size))
      return validator.fail("%s.ligature_set[%u] overflows the table size by [%u] bytes", table_name, i, ligature_set_header_size - ligature_set.size);

    const Offset16* ligature_offset_array = ligature_set->array();
    OffsetRange ligature_offset_range{ligature_set_header_size, ligature_set.size - 6u};

    for (uint32_t ligature_index = 0; ligature_index < ligature_count; ligature_index++) {
      uint32_t ligature_offset = ligature_offset_array[ligature_index].value();
      if (!ligature_offset_range.contains(ligature_offset))
        return validator.fail("%s.ligature_set[%u] ligature[%u] offset (%u) is out of range [%u:%u]", table_name, i, ligature_index, ligature_offset, header_size, table.size);

      Table<GSubTable::Ligature> ligature = ligature_set.sub_table(ligature_offset);
      uint32_t component_count = ligature->glyphs.count();
      if (component_count < 2u)
        return validator.fail("%s.ligature_set[%u].ligature[%u] must have at least 2 glyphs, not %u", table_name, i, ligature_index, component_count);

      uint32_t ligature_table_size = 2u + component_count * 2u;
      if (!ligature.fits(ligature_table_size))
        return validator.fail("%s.ligature_set[%u].ligature[%u] is truncated (size=%u, required=%u)", table_name, i, ligature_index, ligature.size, ligature_table_size);
    }
  }

  return true;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #4 - Ligature Substitution Lookup
// ===============================================================================

static BL_INLINE bool match_ligature(
  Table<Array16<Offset16>> ligature_offsets,
  uint32_t ligature_count,
  const BLGlyphId* in_glyph_data,
  size_t max_glyph_count,
  uint32_t& ligature_glyph_id_out,
  uint32_t& ligature_glyph_count) noexcept {

  // Ligatures are ordered by preference. This means we have to go one by one.
  for (uint32_t i = 0; i < ligature_count; i++) {
    uint32_t ligature_offset = ligature_offsets->array()[i].value();
    BL_ASSERT_VALIDATED(ligature_offset <= ligature_offsets.size - 4u);

    const GSubTable::Ligature* ligature = PtrOps::offset<const GSubTable::Ligature>(ligature_offsets.data, ligature_offset);
    ligature_glyph_count = uint32_t(ligature->glyphs.count());
    if (ligature_glyph_count > max_glyph_count)
      continue;

    // This is safe - a single Ligature is 4 bytes + GlyphId[ligature_glyph_count - 1]. MaxLigOffset is 4 bytes less than
    // the end to include the header, so we only have to include `ligature_glyph_count * 2u` to verify we won't read beyond.
    BL_ASSERT_VALIDATED(ligature_offset + ligature_glyph_count * 2u <= ligature_offsets.size - 4u);

    uint32_t glyph_index = 1;
    for (;;) {
      BLGlyphId glyphA = ligature->glyphs.array()[glyph_index-1].value();
      BLGlyphId glyphB = in_glyph_data[glyph_index];

      if (glyphA != glyphB)
        break;

      if (++glyph_index < ligature_glyph_count)
        continue;

      ligature_glyph_id_out = ligature->ligature_glyph_id();
      return true;
    }
  }

  return false;
}

template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type4_format1(GSubContext& ctx, Table<GSubTable::LigatureSubst1> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  BLGlyphId* glyph_in_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_in_end = ctx.glyph_data() + ctx.size();
  BLGlyphId* glyph_in_end_scope = ctx.glyph_data() + scope.end();

  // Cannot apply a lookup if there is no data to be processed.
  BL_ASSERT(glyph_in_ptr != glyph_in_end_scope);

  uint32_t ligature_set_count = table->ligature_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::LigatureSubst1::kBaseSize + ligature_set_count * 2u));

  // Find the first ligature - if no ligature is matched, no buffer operation will be performed.
  BLGlyphId* glyph_out_ptr = nullptr;
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  for (;;) {
    BLGlyphId glyph_id = glyph_in_ptr[0];
    if (glyph_range.contains(glyph_id)) {
      uint32_t coverage_index;
      if (cov_it.find<kCovFmt>(glyph_id, coverage_index) && coverage_index < ligature_set_count) {
        uint32_t ligature_set_offset = table->ligature_set_offsets.array()[coverage_index].value();
        BL_ASSERT_VALIDATED(ligature_set_offset <= table.size - 2u);

        Table<Array16<Offset16>> ligature_offsets(table.sub_table_unchecked(ligature_set_offset));
        uint32_t ligature_count = ligature_offsets->count();
        BL_ASSERT_VALIDATED(ligature_count && ligature_set_offset + ligature_count * 2u <= table.size - 2u);

        BLGlyphId ligature_glyph_id;
        uint32_t ligature_glyph_count;

        if (match_ligature(ligature_offsets, ligature_count, glyph_in_ptr, (size_t)(glyph_in_end - glyph_in_ptr), ligature_glyph_id, ligature_glyph_count)) {
          *glyph_in_ptr = ligature_glyph_id;
          glyph_out_ptr = glyph_in_ptr + 1u;

          glyph_in_ptr += ligature_glyph_count;
          break;
        }
      }
    }

    if (++glyph_in_ptr == glyph_in_end_scope)
      return BL_SUCCESS;
  }

  // Secondary loop - applies the replacement in-place - the buffer will end up having less glyphs.
  size_t in_index = size_t(glyph_in_ptr - ctx.glyph_data());
  size_t out_index = size_t(glyph_out_ptr - ctx.glyph_data());

  BLGlyphInfo* info_in_ptr = ctx.info_data() + in_index;
  BLGlyphInfo* info_out_ptr = ctx.info_data() + out_index;

  // These is only a single possible match if the scope is only a single index (nested lookups).
  if (scope.is_range()) {
    while (glyph_in_ptr != glyph_in_end_scope) {
      BLGlyphId glyph_id = glyph_in_ptr[0];
      if (glyph_range.contains(glyph_id)) {
        uint32_t coverage_index;
        if (cov_it.find<kCovFmt>(glyph_id, coverage_index) && coverage_index < ligature_set_count) {
          uint32_t ligature_set_offset = table->ligature_set_offsets.array()[coverage_index].value();
          BL_ASSERT_VALIDATED(ligature_set_offset <= table.size - 2u);

          Table<Array16<Offset16>> ligature_offsets(table.sub_table_unchecked(ligature_set_offset));
          uint32_t ligature_count = ligature_offsets->count();
          BL_ASSERT_VALIDATED(ligature_count && ligature_set_offset + ligature_count * 2u <= table.size - 2u);

          BLGlyphId ligature_glyph_id;
          uint32_t ligature_glyph_count;

          if (match_ligature(ligature_offsets, ligature_count, glyph_in_ptr, (size_t)(glyph_in_end - glyph_in_ptr), ligature_glyph_id, ligature_glyph_count)) {
            *glyph_out_ptr++ = ligature_glyph_id;
            *info_out_ptr++ = *info_in_ptr;

            glyph_in_ptr += ligature_glyph_count;
            info_in_ptr += ligature_glyph_count;
            continue;
          }
        }
      }

      *glyph_out_ptr++ = glyph_id;
      *info_out_ptr++ = *info_in_ptr;

      glyph_in_ptr++;
      info_in_ptr++;
    }
  }

  while (glyph_in_ptr != glyph_in_end) {
    *glyph_out_ptr++ = *glyph_in_ptr++;
    *info_out_ptr++ = *info_in_ptr++;
  }

  ctx.truncate(size_t(glyph_out_ptr - ctx._work_buffer.glyph_data));
  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB - Nested Lookups
// ================================================

static void apply_gsubNestedLookup(GSubContext& ctx) noexcept {
  // TODO: [OpenType] GSUB nested lookups
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #5 - Context Substitution Validation
// ==================================================================================

static BL_INLINE bool validate_gsub_lookup_type5_format1(ValidationContext& validator, Table<GSubTable::SequenceContext1> table) noexcept {
  return validate_context_format1(validator, table, "ContextSubst1");
}

static BL_INLINE bool validate_gsub_lookup_type5_format2(ValidationContext& validator, Table<GSubTable::SequenceContext2> table) noexcept {
  return validate_context_format2(validator, table, "ContextSubst2");
}

static BL_INLINE bool validate_gsub_lookup_type5_format3(ValidationContext& validator, Table<GSubTable::SequenceContext3> table) noexcept {
  return validate_context_format3(validator, table, "ContextSubst3");
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #5 - Context Substitution Lookup
// ==============================================================================

template<uint32_t kCovFmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type5_format1(GSubContext& ctx, Table<GSubTable::SequenceContext1> table, ApplyRange scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::SequenceContext1::kBaseSize + rule_set_count * 2u));

  BLGlyphId* glyph_in_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_in_end = ctx.glyph_data() + scope.end();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  while (glyph_in_ptr != glyph_in_end) {
    SequenceMatch match;
    if (match_sequence_format1<kCovFmt>(table, rule_set_count, glyph_range, cov_it, glyph_in_ptr, size_t(glyph_in_end - glyph_in_ptr), &match)) {
      // TODO: [OpenType] Context MATCH
    }

    glyph_in_ptr++;
  }

  return BL_SUCCESS;
}

template<uint32_t kCovFmt, uint32_t kCDFmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type5_format2(GSubContext& ctx, Table<GSubTable::SequenceContext2> table, ApplyRange scope, LookupFlags flags, const CoverageTableIterator& cov_it, const ClassDefTableIterator& cd_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::SequenceContext2::kBaseSize + rule_set_count * 2u));

  BLGlyphId* glyph_in_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_in_end = ctx.glyph_data() + scope.end();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  while (glyph_in_ptr != glyph_in_end) {
    SequenceMatch match;
    if (match_sequence_format2<kCovFmt, kCDFmt>(table, rule_set_count, glyph_range, cov_it, cd_it, glyph_in_ptr, size_t(glyph_in_end - glyph_in_ptr), &match)) {
      // TODO: [OpenType] Context MATCH
    }

    glyph_in_end++;
  }

  return BL_SUCCESS;
}

static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type5_format3(GSubContext& ctx, Table<GSubTable::SequenceContext3> table, ApplyRange scope, LookupFlags flags) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t glyph_count = table->glyph_count();
  if (glyph_count < scope.size())
    return BL_SUCCESS;

  uint32_t lookup_record_count = table->lookup_record_count();
  const UInt16* coverage_offset_array = table->coverage_offset_array();

  BL_ASSERT_VALIDATED(glyph_count > 0);
  BL_ASSERT_VALIDATED(lookup_record_count > 0);
  BL_ASSERT_VALIDATED(table.fits(GSubTable::SequenceContext3::kBaseSize + glyph_count * 2u + lookup_record_count * GPosTable::SequenceLookupRecord::kBaseSize));

  CoverageTableIterator cov0_it;
  uint32_t cov0_fmt = cov0_it.init(table.sub_table_unchecked(coverage_offset_array[0].value()));
  GlyphRange glyph_range = cov0_it.glyph_range_with_format(cov0_fmt);
  const GSubTable::SequenceLookupRecord* lookup_record_array = table->lookup_record_array(glyph_count);

  BLGlyphId* glyph_in_ptr = ctx.glyph_data() + scope.index();
  BLGlyphId* glyph_in_end = ctx.glyph_data() + scope.end();
  BLGlyphId* glyphInEndMinusN = glyph_in_end - glyph_count;

  do {
    if (match_sequence_format3(table, coverage_offset_array, glyph_range, cov0_it, cov0_fmt, glyph_in_ptr, glyph_count)) {
      // TODO: [OpenType] Context MATCH
      bl_unused(lookup_record_array, lookup_record_count);
    }
    glyph_in_ptr++;
  } while (glyph_in_ptr != glyphInEndMinusN);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #6 - Chained Context Substitution Validation
// ==========================================================================================

static BL_INLINE bool validate_gsub_lookup_type6_format1(ValidationContext& validator, Table<GSubTable::ChainedSequenceContext1> table) noexcept {
  return validate_chained_context_format1(validator, table, "ChainedContextSubst1");
}

static BL_INLINE bool validate_gsub_lookup_type6_format2(ValidationContext& validator, Table<GSubTable::ChainedSequenceContext2> table) noexcept {
  return validate_chained_context_format2(validator, table, "ChainedContextSubst2");
}

static BL_INLINE bool validate_gsub_lookup_type6_format3(ValidationContext& validator, Table<GSubTable::ChainedSequenceContext3> table) noexcept {
  return validate_chained_context_format3(validator, table, "ChainedContextSubst3");
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #6 - Chained Context Substitution Lookup
// ======================================================================================

template<uint32_t kCovFmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type6_format1(GSubContext& ctx, Table<GSubTable::ChainedSequenceContext1> table, ApplyRange scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT(scope.index() < scope.end());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::ChainedSequenceContext1::kBaseSize + rule_set_count * 2u));

  ChainedMatchContext mCtx;
  mCtx.table = table;
  mCtx.first_glyph_range = cov_it.glyph_range<kCovFmt>();
  mCtx.back_glyph_ptr = ctx.glyph_data();
  mCtx.ahead_glyph_ptr = ctx.glyph_data() + scope.index();
  mCtx.back_glyph_count = scope.index();
  mCtx.ahead_glyph_count = scope.size();

  const Offset16* rule_set_offsets = table->rule_set_offsets.array();
  do {
    SequenceMatch match;
    if (match_chained_sequence_format1<kCovFmt>(mCtx, rule_set_offsets, rule_set_count, cov_it, &match)) {
      // TODO: [OpenType] Context MATCH
    }
    mCtx.ahead_glyph_ptr++;
    mCtx.back_glyph_count++;
  } while (--mCtx.ahead_glyph_count != 0);

  return BL_SUCCESS;
}

template<uint32_t kCovFmt, uint32_t kCD1Fmt, uint32_t kCD2Fmt, uint32_t kCD3Fmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type6_format2(
  GSubContext& ctx,
  Table<GSubTable::ChainedSequenceContext2> table,
  ApplyRange scope,
  LookupFlags flags,
  const CoverageTableIterator& cov_it, const ClassDefTableIterator& cd1_it, const ClassDefTableIterator& cd2_it, const ClassDefTableIterator& cd3_it) noexcept {

  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT(scope.index() < scope.end());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GSubTable::ChainedSequenceContext2::kBaseSize + rule_set_count * 2u));

  ChainedMatchContext mCtx;
  mCtx.table = table;
  mCtx.first_glyph_range = cov_it.glyph_range<kCovFmt>();
  mCtx.back_glyph_ptr = ctx.glyph_data();
  mCtx.ahead_glyph_ptr = ctx.glyph_data() + scope.index();
  mCtx.back_glyph_count = scope.index();
  mCtx.ahead_glyph_count = scope.size();

  const Offset16* rule_set_offsets = table->rule_set_offsets.array();
  do {
    SequenceMatch match;
    if (match_chained_sequence_format2<kCovFmt, kCD1Fmt, kCD2Fmt, kCD3Fmt>(mCtx, rule_set_offsets, rule_set_count, cov_it, cd1_it, cd2_it, cd3_it, &match)) {
      // TODO: [OpenType] Context MATCH
    }
    mCtx.ahead_glyph_ptr++;
    mCtx.back_glyph_count++;
  } while (--mCtx.ahead_glyph_count != 0);

  return BL_SUCCESS;
}

static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type6_format3(GSubContext& ctx, Table<GSubTable::ChainedSequenceContext3> table, ApplyRange scope, LookupFlags flags) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t backtrack_glyph_count = table->backtrack_glyph_count();
  uint32_t input_offset = 4u + backtrack_glyph_count * 2u;
  BL_ASSERT_VALIDATED(table.fits(input_offset + 2u));

  uint32_t input_glyph_count = table.readU16(input_offset);
  uint32_t lookahead_offset = input_offset + 2u + input_glyph_count * 2u;
  BL_ASSERT_VALIDATED(input_glyph_count > 0);
  BL_ASSERT_VALIDATED(table.fits(lookahead_offset + 2u));

  uint32_t lookahead_glyph_count = table.readU16(lookahead_offset);
  uint32_t lookup_offset = lookahead_offset + 2u + lookahead_glyph_count * 2u;
  BL_ASSERT_VALIDATED(table.fits(lookup_offset + 2u));

  uint32_t lookup_record_count = table.readU16(lookup_offset);
  BL_ASSERT_VALIDATED(lookup_record_count > 0);
  BL_ASSERT_VALIDATED(table.fits(lookup_offset + 2u + lookup_record_count * GSubGPosTable::SequenceLookupRecord::kBaseSize));

  // Restrict scope in a way so we would never underflow/overflow glyph buffer when matching backtrack/lookahead glyphs.
  uint32_t input_and_lookahead_glyph_count = input_glyph_count + lookahead_glyph_count;
  scope.intersect(backtrack_glyph_count, ctx.size() - input_and_lookahead_glyph_count);

  // Bail if the buffer or the scope is too small for this chained context substitution.
  if (scope.size() < input_and_lookahead_glyph_count || scope.index() >= scope.end())
    return BL_SUCCESS;

  const Offset16* backtrack_coverage_offsets = table->backtrack_coverage_offsets();
  const Offset16* input_coverage_offsets = table.data_as<Offset16>(input_offset + 2u);
  const Offset16* lookahead_coverage_offsets = table.data_as<Offset16>(lookahead_offset + 2u);

  CoverageTableIterator cov0_it;
  uint32_t cov0_fmt = cov0_it.init(table.sub_table_unchecked(input_coverage_offsets[0].value()));
  GlyphRange first_glyph_range = cov0_it.glyph_range_with_format(cov0_fmt);

  ChainedMatchContext mCtx;
  mCtx.table = table;
  mCtx.first_glyph_range = cov0_it.glyph_range_with_format(cov0_fmt);
  mCtx.back_glyph_ptr = ctx.glyph_data();
  mCtx.ahead_glyph_ptr = ctx.glyph_data() + scope.index();
  mCtx.back_glyph_count = scope.index();
  mCtx.ahead_glyph_count = scope.size();

  do {
    if (match_chained_sequence_format3(mCtx,
        backtrack_coverage_offsets, backtrack_glyph_count,
        input_coverage_offsets, input_glyph_count,
        lookahead_coverage_offsets, lookahead_glyph_count,
        first_glyph_range,
        cov0_it, cov0_fmt)) {
      const GSubTable::SequenceLookupRecord* lookup_record_array = table.data_as<GSubTable::SequenceLookupRecord>(lookup_offset + 2u);
      // TODO: [OpenType] Context MATCH
      bl_unused(lookup_record_array, lookup_record_count);
    }
    mCtx.back_glyph_count++;
    mCtx.ahead_glyph_ptr++;
  } while (--mCtx.ahead_glyph_count >= input_and_lookahead_glyph_count);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #8 - Reverse Chained Context Validation
// =====================================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gsub_lookup_type8_format1(ValidationContext& validator, Table<GSubTable::ReverseChainedSingleSubst1> table) noexcept {
  const char* table_name = "ReverseChainedSingleSubst1";

  if (!table.fits())
    return validator.invalid_table_size(table_name, table.size, GSubTable::ReverseChainedSingleSubst1::kBaseSize);

  uint32_t backtrack_glyph_count = table->backtrack_glyph_count();
  uint32_t lookahead_offset = 6u + backtrack_glyph_count * 2u;

  if (!table.fits(lookahead_offset + 2u))
    return validator.invalid_table_size(table_name, table.size, lookahead_offset + 2u);

  uint32_t lookahead_glyph_count = table.readU16(lookahead_offset);
  uint32_t subst_offset = lookahead_offset + 2u + lookahead_glyph_count * 2u;

  if (!table.fits(subst_offset + 2u))
    return validator.invalid_table_size(table_name, table.size, subst_offset + 2u);

  uint32_t subst_glyph_count = table.readU16(subst_offset);
  uint32_t header_size = subst_offset + 2u + subst_glyph_count * 2u;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  OffsetRange sub_table_offset_range{header_size, table.size};
  uint32_t coverage_offset = table->coverage_offset();

  if (!sub_table_offset_range.contains(coverage_offset))
    return validator.invalid_field_offset(table_name, "coverage_table", coverage_offset, sub_table_offset_range);

  uint32_t coverage_count;
  if (!validate_coverage_table(validator, table.sub_table(coverage_offset), coverage_count))
    return false;

  if (coverage_count != subst_glyph_count)
    return validator.fail("%s must have coverage_count (%u) equal to subst_glyph_count (%u)", table_name, coverage_count, subst_glyph_count);

  if (!validate_coverage_tables(validator, table, table_name, "backtrack_coverages", table->backtrack_coverage_offsets(), backtrack_glyph_count, sub_table_offset_range))
    return false;

  if (!validate_coverage_tables(validator, table, table_name, "lookahead_coverages", table.data_as<Offset16>(lookahead_offset + 2u), lookahead_glyph_count, sub_table_offset_range))
    return false;

  return true;
}

// bl::OpenType::LayoutImpl - GSUB - Lookup Type #8 - Reverse Chained Context Lookup
// =================================================================================

static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup_type8_format1(GSubContext& ctx, Table<GSubTable::ReverseChainedSingleSubst1> table, ApplyRange scope, LookupFlags flags) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t backtrack_glyph_count = table->backtrack_glyph_count();
  uint32_t lookahead_offset = 6u + backtrack_glyph_count * 2u;
  BL_ASSERT_VALIDATED(table.fits(lookahead_offset + 2u));

  uint32_t lookahead_glyph_count = table.readU16(lookahead_offset);
  uint32_t subst_offset = lookahead_offset + 2u + lookahead_glyph_count * 2u;
  BL_ASSERT_VALIDATED(table.fits(subst_offset + 2u));

  // Restrict scope in a way so we would never underflow/overflow glyph buffer when matching backtrack/lookahead glyphs.
  scope.intersect(backtrack_glyph_count, ctx.size() - lookahead_glyph_count - 1u);

  // Bail if the buffer or the scope is too small for this chained context substitution.
  if (ctx.size() < lookahead_glyph_count || scope.index() >= scope.end())
    return BL_SUCCESS;

  uint32_t subst_glyph_count = table.readU16(subst_offset);
  BL_ASSERT_VALIDATED(table.fits(subst_offset + 2u + subst_glyph_count * 2u));

  const Offset16* backtrack_coverage_offsets = table->backtrack_coverage_offsets();
  const Offset16* lookahead_coverage_offsets = table.data_as<Offset16>(lookahead_offset + 2u);
  const UInt16* subst_glyph_ids = table.data_as<UInt16>(subst_offset + 2u);

  CoverageTableIterator cov_it;
  uint32_t cov_fmt = cov_it.init(table.sub_table_unchecked(table->coverage_offset()));
  GlyphRange glyph_range = cov_it.glyph_range_with_format(cov_fmt);

  BLGlyphId* glyph_data = ctx.glyph_data();
  size_t i = scope.end();
  size_t scope_begin = scope.index();

  do {
    BLGlyphId glyph_id = glyph_data[--i];
    uint32_t coverage_index;

    if (!glyph_range.contains(glyph_id) || !cov_it.find_with_format(cov_fmt, glyph_id, coverage_index) || coverage_index >= subst_glyph_count)
      continue;

    if (!match_back_glyphs_format3(table, glyph_data + i - 1u, backtrack_coverage_offsets, backtrack_glyph_count))
      continue;

    if (!match_ahead_glyphs_format3(table, glyph_data + i + 1u, lookahead_coverage_offsets, lookahead_glyph_count))
      continue;

    glyph_data[i] = subst_glyph_ids[coverage_index].value();
  } while (i != scope_begin);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB - Dispatch
// ==========================================

static BL_NOINLINE bool validateGSubLookup(ValidationContext& validator, RawTable table, GSubLookupAndFormat type_and_format) noexcept {
  switch (type_and_format) {
    case GSubLookupAndFormat::kType1Format1: return validate_gsub_lookup_type1_format1(validator, table);
    case GSubLookupAndFormat::kType1Format2: return validate_gsub_lookup_type1_format2(validator, table);
    case GSubLookupAndFormat::kType2Format1: return validate_gsub_lookup_type2_format1(validator, table);
    case GSubLookupAndFormat::kType3Format1: return validate_gsub_lookup_type3_format1(validator, table);
    case GSubLookupAndFormat::kType4Format1: return validate_gsub_lookup_type4_format1(validator, table);
    case GSubLookupAndFormat::kType5Format1: return validate_gsub_lookup_type5_format1(validator, table);
    case GSubLookupAndFormat::kType5Format2: return validate_gsub_lookup_type5_format2(validator, table);
    case GSubLookupAndFormat::kType5Format3: return validate_gsub_lookup_type5_format3(validator, table);
    case GSubLookupAndFormat::kType6Format1: return validate_gsub_lookup_type6_format1(validator, table);
    case GSubLookupAndFormat::kType6Format2: return validate_gsub_lookup_type6_format2(validator, table);
    case GSubLookupAndFormat::kType6Format3: return validate_gsub_lookup_type6_format3(validator, table);
    case GSubLookupAndFormat::kType8Format1: return validate_gsub_lookup_type8_format1(validator, table);
    default:
      return validator.fail("Unknown lookup type+format (%u)", uint32_t(type_and_format));
  }
}

static BL_INLINE_IF_NOT_DEBUG BLResult apply_gsub_lookup(GSubContext& ctx, RawTable table, GSubLookupAndFormat type_and_format, ApplyRange scope, LookupFlags flags) noexcept {
  BL_ASSERT_VALIDATED(table.fits(gsub_lookup_info_table.lookup_info[size_t(type_and_format)].header_size));

  #define BL_APPLY_WITH_COVERAGE(FN, TABLE)                                        \
    CoverageTableIterator cov_it;                                                   \
    if (cov_it.init(table.sub_table(table.data_as<TABLE>()->coverage_offset())) == 1u) \
      result = FN<1>(ctx, table, scope, flags, cov_it);                             \
    else                                                                           \
      result = FN<2>(ctx, table, scope, flags, cov_it);

  BLResult result = BL_SUCCESS;

  switch (type_and_format) {
    case GSubLookupAndFormat::kType1Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gsub_lookup_type1_format1, GSubTable::SingleSubst1)
      break;
    }

    case GSubLookupAndFormat::kType1Format2: {
      BL_APPLY_WITH_COVERAGE(apply_gsub_lookup_type1_format2, GSubTable::MultipleSubst1)
      break;
    }

    case GSubLookupAndFormat::kType2Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gsub_lookup_type2_format1, GSubTable::MultipleSubst1)
      break;
    }

    case GSubLookupAndFormat::kType3Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gsub_lookup_type3_format1, GSubTable::AlternateSubst1)
      break;
    }

    case GSubLookupAndFormat::kType4Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gsub_lookup_type4_format1, GSubTable::LigatureSubst1)
      break;
    }

    case GSubLookupAndFormat::kType5Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gsub_lookup_type5_format1, GSubTable::SequenceContext1)
      break;
    }

    case GSubLookupAndFormat::kType5Format2: {
      CoverageTableIterator cov_it;
      ClassDefTableIterator cd_it;

      FormatBits2X fmt_bits = FormatBits2X(
        ((cov_it.init(table.sub_table(table.data_as<GSubTable::SequenceContext2>()->coverage_offset())) - 1u) << 1) |
        ((cd_it.init(table.sub_table(table.data_as<GSubTable::SequenceContext2>()->class_def_offset()))  - 1u) << 0));

      switch (fmt_bits) {
        case FormatBits2X::k11: return apply_gsub_lookup_type5_format2<1, 1>(ctx, table, scope, flags, cov_it, cd_it);
        case FormatBits2X::k12: return apply_gsub_lookup_type5_format2<1, 2>(ctx, table, scope, flags, cov_it, cd_it);
        case FormatBits2X::k21: return apply_gsub_lookup_type5_format2<2, 1>(ctx, table, scope, flags, cov_it, cd_it);
        case FormatBits2X::k22: return apply_gsub_lookup_type5_format2<2, 2>(ctx, table, scope, flags, cov_it, cd_it);
      }
      break;
    }

    case GSubLookupAndFormat::kType5Format3: {
      result = apply_gsub_lookup_type5_format3(ctx, table, scope, flags);
      break;
    }

    case GSubLookupAndFormat::kType6Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gsub_lookup_type6_format1, GSubTable::ChainedSequenceContext1)
      break;
    }

    case GSubLookupAndFormat::kType6Format2: {
      CoverageTableIterator cov_it;
      ClassDefTableIterator cd1_it;
      ClassDefTableIterator cd2_it;
      ClassDefTableIterator cd3_it;

      FormatBits4X fmt_bits = FormatBits4X(
        ((cov_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->coverage_offset()         )) - 1u) << 3) |
        ((cd1_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->backtrack_class_def_offset())) - 1u) << 2) |
        ((cd2_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->input_class_def_offset()    )) - 1u) << 1) |
        ((cd3_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->lookahead_class_def_offset())) - 1u) << 0));

      switch (fmt_bits) {
        case FormatBits4X::k1111: result = apply_gsub_lookup_type6_format2<1, 1, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1112: result = apply_gsub_lookup_type6_format2<1, 1, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1121: result = apply_gsub_lookup_type6_format2<1, 1, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1122: result = apply_gsub_lookup_type6_format2<1, 1, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1211: result = apply_gsub_lookup_type6_format2<1, 2, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1212: result = apply_gsub_lookup_type6_format2<1, 2, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1221: result = apply_gsub_lookup_type6_format2<1, 2, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1222: result = apply_gsub_lookup_type6_format2<1, 2, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2111: result = apply_gsub_lookup_type6_format2<2, 1, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2112: result = apply_gsub_lookup_type6_format2<2, 1, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2121: result = apply_gsub_lookup_type6_format2<2, 1, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2122: result = apply_gsub_lookup_type6_format2<2, 1, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2211: result = apply_gsub_lookup_type6_format2<2, 2, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2212: result = apply_gsub_lookup_type6_format2<2, 2, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2221: result = apply_gsub_lookup_type6_format2<2, 2, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2222: result = apply_gsub_lookup_type6_format2<2, 2, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
      }
      break;
    }

    case GSubLookupAndFormat::kType6Format3: {
      result = apply_gsub_lookup_type6_format3(ctx, table, scope, flags);
      break;
    }

    case GSubLookupAndFormat::kType8Format1: {
      result = apply_gsub_lookup_type8_format1(ctx, table, scope, flags);
      break;
    }

    default:
      break;
  }

  #undef BL_APPLY_WITH_COVERAGE

  return result;
}

// bl::OpenType::LayoutImpl - GPOS - Utilities
// ===========================================

// struct ValueRecords {
//   ?[Int16 x_placement]
//   ?[Int16 y_placement]
//   ?[Int16 x_advance]
//   ?[Int16 y_advance]
//   ?[UInt16 xPlacementDeviceOffset]
//   ?[UInt16 yPlacementDeviceOffset]
//   ?[UInt16 xAdvanceDeviceOffset]
//   ?[UInt16 yAdvanceDeviceOffset]
// }
static BL_INLINE uint32_t size_of_value_record_by_format(uint32_t value_format) noexcept {
  return uint32_t(bit_count_byte_table[value_format & 0xFFu]) * 2u;
}

template<typename T>
static BL_INLINE const uint8_t* binary_search_glyph_id_in_var_struct(const uint8_t* array, size_t item_size, size_t array_size, BLGlyphId glyph_id, size_t offset = 0) noexcept {
  if (!array_size)
    return nullptr;

  const uint8_t* ptr = array;
  while (size_t half = array_size / 2u) {
    const uint8_t* middle_ptr = ptr + half * item_size;
    array_size -= half;
    if (glyph_id >= reinterpret_cast<const T*>(middle_ptr + offset)->value())
      ptr = middle_ptr;
  }

  if (glyph_id != reinterpret_cast<const T*>(ptr + offset)->value())
    ptr = nullptr;

  return ptr;
}

static BL_INLINE const Int16* apply_gposValue(const Int16* p, uint32_t value_format, BLGlyphPlacement* glyph_placement) noexcept {
  int32_t v;
  if (value_format & GPosTable::kValueXPlacement      ) { v = p->value(); p++; glyph_placement->placement.x += v; }
  if (value_format & GPosTable::kValueYPlacement      ) { v = p->value(); p++; glyph_placement->placement.y += v; }
  if (value_format & GPosTable::kValueXAdvance        ) { v = p->value(); p++; glyph_placement->advance.x += v; }
  if (value_format & GPosTable::kValueYAdvance        ) { v = p->value(); p++; glyph_placement->advance.y += v; }
  if (value_format & GPosTable::kValueXPlacementDevice) { v = p->value(); p++; }
  if (value_format & GPosTable::kValueYPlacementDevice) { v = p->value(); p++; }
  if (value_format & GPosTable::kValueXAdvanceDevice  ) { v = p->value(); p++; }
  if (value_format & GPosTable::kValueYAdvanceDevice  ) { v = p->value(); p++; }
  return p;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #1 - Single Adjustment Validation
// ===============================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gpos_lookup_type1_format1(ValidationContext& validator, Table<GPosTable::SingleAdjustment1> table) noexcept {
  const char* table_name = "SingleAdjustment1";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GPosTable::SingleAdjustment1::kBaseSize, coverage_count))
    return false;

  uint32_t value_format = table->value_format();
  if (!value_format)
    return validator.invalid_field_value(table_name, "value_format", value_format);

  uint32_t record_size = size_of_value_record_by_format(value_format);
  uint32_t header_size = GPosTable::SingleAdjustment1::kBaseSize + record_size;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  return true;
}

static BL_INLINE_IF_NOT_DEBUG bool validate_gpos_lookup_type1_format2(ValidationContext& validator, Table<GPosTable::SingleAdjustment2> table) noexcept {
  const char* table_name = "SingleAdjustment2";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GPosTable::SingleAdjustment2::kBaseSize, coverage_count))
    return false;

  uint32_t value_format = table->value_format();
  if (!value_format)
    return validator.invalid_field_value(table_name, "value_format", value_format);

  uint32_t value_count = table->value_count();
  if (!value_count)
    return validator.invalid_field_value(table_name, "value_count", value_count);

  uint32_t record_size = size_of_value_record_by_format(value_format);
  uint32_t header_size = GPosTable::SingleAdjustment2::kBaseSize + record_size * value_count;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  return true;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #1 - Single Adjustment Lookup
// ===========================================================================

template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type1_format1(GPosContext& ctx, Table<GPosTable::SingleAdjustment1> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t value_format = table->value_format();
  BL_ASSERT_VALIDATED(value_format != 0);
  BL_ASSERT_VALIDATED(table.fits(GPosTable::SingleAdjustment1::kBaseSize + size_of_value_record_by_format(value_format)));

  size_t i = scope.index();
  size_t end = scope.end();

  BLGlyphId* glyph_data = ctx.glyph_data();
  BLGlyphPlacement* placement_data = ctx.placement_data();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    BLGlyphId glyph_id = glyph_data[i];
    if (glyph_range.contains(glyph_id)) {
      uint32_t unused_coverage_index;
      if (cov_it.find<kCovFmt>(glyph_id, unused_coverage_index)) {
        const Int16* p = reinterpret_cast<const Int16*>(table.data + GPosTable::SingleAdjustment1::kBaseSize);
        apply_gposValue(p, value_format, &placement_data[i]);
      }
    }
  } while (++i < end);

  return BL_SUCCESS;
}

template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type1_format2(GPosContext& ctx, Table<GPosTable::SingleAdjustment2> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t value_format = table->value_format();
  uint32_t value_count = table->value_count();
  uint32_t record_size = size_of_value_record_by_format(value_format);

  BL_ASSERT_VALIDATED(value_format != 0);
  BL_ASSERT_VALIDATED(table.fits(GPosTable::SingleAdjustment2::kBaseSize + value_count * record_size));

  size_t i = scope.index();
  size_t end = scope.end();

  BLGlyphId* glyph_data = ctx.glyph_data();
  BLGlyphPlacement* placement_data = ctx.placement_data();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    BLGlyphId glyph_id = glyph_data[i];
    if (glyph_range.contains(glyph_id)) {
      uint32_t coverage_index;
      if (cov_it.find<kCovFmt>(glyph_id, coverage_index) && coverage_index < value_count) {
        const Int16* p = reinterpret_cast<const Int16*>(table.data + GPosTable::SingleAdjustment2::kBaseSize + coverage_index * record_size);
        apply_gposValue(p, value_format, &placement_data[i]);
      }
    }
  } while (++i < end);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #2 - Pair Adjustment Validation
// =============================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gpos_lookup_type2_format1(ValidationContext& validator, Table<GPosTable::PairAdjustment1> table) noexcept {
  const char* table_name = "PairAdjustment1";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GPosTable::PairAdjustment1::kBaseSize, coverage_count))
    return false;

  uint32_t pair_set_count = table->pair_set_offsets.count();
  uint32_t value_record_size = 2u + size_of_value_record_by_format(table->value_format1()) +
                                  size_of_value_record_by_format(table->value_format2()) ;

  uint32_t header_size = GPosTable::PairAdjustment1::kBaseSize + pair_set_count * 2u;
  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  const Offset16* offset_array = table->pair_set_offsets.array();
  OffsetRange pair_set_offset_range{header_size, table.size - 2u};

  for (uint32_t i = 0; i < pair_set_count; i++) {
    uint32_t pair_set_offset = offset_array[i].value();
    if (!pair_set_offset_range.contains(pair_set_offset))
      return validator.invalid_offset_entry(table_name, "pair_set_offset", i, pair_set_offset, pair_set_offset_range);

    Table<GPosTable::PairSet> pair_set(table.sub_table(pair_set_offset));

    uint32_t pair_value_count = pair_set->pair_value_count();
    uint32_t pair_set_size = pair_value_count * value_record_size;

    if (!pair_set.fits(pair_set_size))
      return validator.invalid_table_size("PairSet", pair_set.size, pair_set_size);
  }

  return true;
}

static BL_INLINE_IF_NOT_DEBUG bool validate_gpos_lookup_type2_format2(ValidationContext& validator, Table<GPosTable::PairAdjustment2> table) noexcept {
  const char* table_name = "PairAdjustment2";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GPosTable::PairAdjustment2::kBaseSize, coverage_count))
    return false;

  uint32_t class1_count = table->class1_count();
  uint32_t class2_count = table->class2_count();
  uint32_t value_record_count = class1_count * class2_count;

  uint32_t value1_format = table->value1_format();
  uint32_t value2_format = table->value2_format();
  uint32_t value_record_size = size_of_value_record_by_format(value1_format) + size_of_value_record_by_format(value2_format);

  uint64_t calculated_table_size = uint64_t(value_record_count) * uint64_t(value_record_size);
  if (calculated_table_size > table.size - GPosTable::PairAdjustment2::kBaseSize)
    calculated_table_size = 0xFFFFFFFFu;

  if (!table.fits(calculated_table_size))
    return validator.invalid_table_size(table_name, table.size, uint32_t(calculated_table_size));

  return true;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #2 - Pair Adjustment Lookup
// =========================================================================

template<uint32_t kCovFmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type2_format1(GPosContext& ctx, Table<GPosTable::PairAdjustment1> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  size_t i = scope.index();
  size_t end = scope.is_range() ? scope.end() : ctx.size();

  // We always want to access the current and next glyphs, so bail if there is no next glyph...
  if (scope.is_range()) {
    if (i >= --end)
      return BL_SUCCESS;
  }
  else {
    if (i + 1u > ctx.size())
      return BL_SUCCESS;
  }

  uint32_t value_format1 = table->value_format1();
  uint32_t value_format2 = table->value_format2();
  uint32_t pair_set_offsets_count = table->pair_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GPosTable::PairAdjustment1::kBaseSize + pair_set_offsets_count * 2u));

  uint32_t value_record_size = 2u + size_of_value_record_by_format(value_format1) +
                                  size_of_value_record_by_format(value_format2) ;

  BLGlyphId* glyph_data = ctx.glyph_data();
  BLGlyphPlacement* placement_data = ctx.placement_data();

  BLGlyphId left_glyph_id = glyph_data[i];
  BLGlyphId right_glyph_id = 0;
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    right_glyph_id = glyph_data[i + 1];
    if (glyph_range.contains(left_glyph_id)) {
      uint32_t coverage_index;
      if (cov_it.find<kCovFmt>(left_glyph_id, coverage_index) && coverage_index < pair_set_offsets_count) {
        uint32_t pair_set_offset = table->pair_set_offsets.array()[coverage_index].value();
        BL_ASSERT_VALIDATED(pair_set_offset <= table.size - 2u);

        const GPosTable::PairSet* pair_set = PtrOps::offset<const GPosTable::PairSet>(table.data, pair_set_offset);
        uint32_t pair_set_count = pair_set->pair_value_count();
        BL_ASSERT_VALIDATED(pair_set_count * value_record_size <= table.size - pair_set_offset);

        const Int16* p = reinterpret_cast<const Int16*>(
          binary_search_glyph_id_in_var_struct<UInt16>(
            reinterpret_cast<const uint8_t*>(pair_set->pair_value_records()), value_record_size, pair_set_count, right_glyph_id));

        if (p) {
          p++;
          if (value_format1) p = apply_gposValue(p, value_format1, &placement_data[i + 0]);
          if (value_format2) p = apply_gposValue(p, value_format2, &placement_data[i + 1]);
        }
      }
    }

    left_glyph_id = right_glyph_id;
  } while (scope.is_range() && ++i < end);

  return BL_SUCCESS;
}

template<uint32_t kCovFmt, uint32_t kCD1Fmt, uint32_t kCD2Fmt, typename ApplyScope>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type2_format2(GPosContext& ctx, Table<GPosTable::PairAdjustment2> table, ApplyScope scope, LookupFlags flags, const CoverageTableIterator& cov_it, const ClassDefTableIterator& cd1_it, ClassDefTableIterator& cd2_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  size_t i = scope.index();
  size_t end = scope.is_range() ? scope.end() : ctx.size();

  // We always want to access the current and next glyphs, so bail if there is no next glyph...
  if (scope.is_range()) {
    if (i >= --end)
      return BL_SUCCESS;
  }
  else {
    if (i + 1u > ctx.size())
      return BL_SUCCESS;
  }

  uint32_t value1_format = table->value1_format();
  uint32_t value2_format = table->value2_format();
  uint32_t value_record_size = size_of_value_record_by_format(value1_format) + size_of_value_record_by_format(value2_format);

  uint32_t class1_count = table->class1_count();
  uint32_t class2_count = table->class2_count();
  uint32_t value_record_count = class1_count * class2_count;
  BL_ASSERT_VALIDATED(table.fits(GPosTable::PairAdjustment2::kBaseSize + uint64_t(value_record_count) * uint64_t(value_record_size)));

  const uint8_t* value_base_ptr = table.data + GPosTable::PairAdjustment2::kBaseSize;

  BLGlyphId* glyph_data = ctx.glyph_data();
  BLGlyphPlacement* placement_data = ctx.placement_data();

  BLGlyphId left_glyph_id = glyph_data[i];
  BLGlyphId right_glyph_id = 0;
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    right_glyph_id = glyph_data[i + 1];
    if (glyph_range.contains(left_glyph_id)) {
      uint32_t coverage_index;
      if (cov_it.find<kCovFmt>(left_glyph_id, coverage_index)) {
        uint32_t c1 = cd1_it.class_of_glyph<kCD1Fmt>(left_glyph_id);
        uint32_t c2 = cd2_it.class_of_glyph<kCD2Fmt>(right_glyph_id);
        uint32_t c_index = c1 * class2_count + c2;

        if (c_index < value_record_count) {
          const Int16* p = PtrOps::offset<const Int16>(value_base_ptr, c_index * value_record_size);
          if (value1_format) p = apply_gposValue(p, value1_format, &placement_data[i + 0]);
          if (value2_format) p = apply_gposValue(p, value2_format, &placement_data[i + 1]);
        }
      }
    }

    left_glyph_id = right_glyph_id;
  } while (scope.is_range() && ++i < end);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #3 - Cursive Attachment Validation
// ================================================================================

static BL_INLINE_IF_NOT_DEBUG bool validate_gpos_lookup_type3_format1(ValidationContext& validator, Table<GPosTable::CursiveAttachment1> table) noexcept {
  const char* table_name = "CursiveAttachment1";

  uint32_t coverage_count;
  if (!validate_lookup_with_coverage(validator, table, table_name, GPosTable::CursiveAttachment1::kBaseSize, coverage_count))
    return false;

  uint32_t entry_exit_count = table->entry_exits.count();
  uint32_t header_size = GPosTable::CursiveAttachment1::kBaseSize + entry_exit_count * GPosTable::EntryExit::kBaseSize;

  if (!table.fits(header_size))
    return validator.invalid_table_size(table_name, table.size, header_size);

  // TODO: [OpenType] GPOS Cursive attachment validation.
  return false;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #4 - MarkToBase Attachment Validation
// ===================================================================================

// TODO: [OpenType] GPOS MarkToBase attachment

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #5 - MarkToLigature Attachment Validation
// =======================================================================================

// TODO: [OpenType] GPOS MarkToLigature attachment

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #6 - MarkToMark Attachment Validation
// ===================================================================================

// TODO: [OpenType] GPOS MarkToMark attachment

// bl::OpenType::LayoutImpl - GPOS - Nested Lookups
// ================================================

static BL_NOINLINE BLResult apply_gpos_nested_lookups(GPosContext& ctx, size_t index, const SequenceMatch& match) noexcept {
  // TODO: [OpenType] GPOS nested lookups
  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #7 - Contextual Positioning Validation
// ====================================================================================

static BL_INLINE bool validate_gpos_lookup_type7_format1(ValidationContext& validator, Table<GSubTable::SequenceContext1> table) noexcept {
  return validate_context_format1(validator, table, "ContextPositioning1");
}

static BL_INLINE bool validate_gpos_lookup_type7_format2(ValidationContext& validator, Table<GSubTable::SequenceContext2> table) noexcept {
  return validate_context_format2(validator, table, "ContextPositioning2");
}

static BL_INLINE bool validate_gpos_lookup_type7_format3(ValidationContext& validator, Table<GSubTable::SequenceContext3> table) noexcept {
  return validate_context_format3(validator, table, "ContextPositioning3");
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #7 - Contextual Positioning Lookup
// ================================================================================

template<uint32_t kCovFmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type7_format1(GPosContext& ctx, Table<GPosTable::SequenceContext1> table, ApplyRange scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GPosTable::SequenceContext1::kBaseSize + rule_set_count * 2u));

  size_t index = scope.index();
  size_t end = scope.end();
  BL_ASSERT(index < end);

  const BLGlyphId* glyph_ptr = ctx.glyph_data();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    SequenceMatch match;
    if (match_sequence_format1<kCovFmt>(table, rule_set_count, glyph_range, cov_it, glyph_ptr, end - index, &match)) {
      BL_PROPAGATE(apply_gpos_nested_lookups(ctx, index, match));
    }
  } while (++index != end);

  return BL_SUCCESS;
}

template<uint32_t kCovFmt, uint32_t kCDFmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type7_format2(GPosContext& ctx, Table<GPosTable::SequenceContext2> table, ApplyRange scope, LookupFlags flags, const CoverageTableIterator& cov_it, const ClassDefTableIterator& cd_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GPosTable::SequenceContext2::kBaseSize + rule_set_count * 2u));

  size_t index = scope.index();
  size_t end = scope.end();
  BL_ASSERT(index < end);

  const BLGlyphId* glyph_ptr = ctx.glyph_data() + scope.index();
  GlyphRange glyph_range = cov_it.glyph_range<kCovFmt>();

  do {
    SequenceMatch match;
    if (match_sequence_format2<kCovFmt, kCDFmt>(table, rule_set_count, glyph_range, cov_it, cd_it, glyph_ptr, end - index, &match)) {
      BL_PROPAGATE(apply_gpos_nested_lookups(ctx, index, match));
    }
  } while (++index != end);

  return BL_SUCCESS;
}

static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type7_format3(GPosContext& ctx, Table<GPosTable::SequenceContext3> table, ApplyRange scope, LookupFlags flags) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t glyph_count = table->glyph_count();
  uint32_t lookup_record_count = table->lookup_record_count();

  if (scope.size() < glyph_count)
    return BL_SUCCESS;

  BL_ASSERT_VALIDATED(glyph_count > 0);
  BL_ASSERT_VALIDATED(lookup_record_count > 0);
  BL_ASSERT_VALIDATED(table.fits(GPosTable::SequenceContext3::kBaseSize + glyph_count * 2u + lookup_record_count * GPosTable::SequenceLookupRecord::kBaseSize));

  const UInt16* coverage_offset_array = table->coverage_offset_array();
  const GPosTable::SequenceLookupRecord* lookup_record_array = table->lookup_record_array(glyph_count);

  CoverageTableIterator cov0_it;
  uint32_t cov0_fmt = cov0_it.init(table.sub_table_unchecked(coverage_offset_array[0].value()));

  size_t index = scope.index();
  size_t end = scope.end();
  BL_ASSERT(index < end);

  const BLGlyphId* glyph_ptr = ctx.glyph_data() + scope.index();
  GlyphRange glyph_range = cov0_it.glyph_range_with_format(cov0_fmt);
  SequenceMatch match{glyph_count, lookup_record_count, lookup_record_array};

  size_t end_minus_glyph_count = end - glyph_count;
  do {
    if (match_sequence_format3(table, coverage_offset_array, glyph_range, cov0_it, cov0_fmt, glyph_ptr, glyph_count)) {
      BL_PROPAGATE(apply_gpos_nested_lookups(ctx, index, match));
    }
  } while (++index != end_minus_glyph_count);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #8 - Chained Context Positioning Validation
// =========================================================================================

static BL_INLINE bool validate_gpos_lookup_type8_format1(ValidationContext& validator, Table<GSubTable::ChainedSequenceContext1> table) noexcept {
  return validate_chained_context_format1(validator, table, "ChainedContextPositioning1");
}

static BL_INLINE bool validate_gpos_lookup_type8_format2(ValidationContext& validator, Table<GSubTable::ChainedSequenceContext2> table) noexcept {
  return validate_chained_context_format2(validator, table, "ChainedContextPositioning2");
}

static BL_INLINE bool validate_gpos_lookup_type8_format3(ValidationContext& validator, Table<GSubTable::ChainedSequenceContext3> table) noexcept {
  return validate_chained_context_format3(validator, table, "ChainedContextPositioning3");
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Type #8 - Chained Context Positioning Lookup
// =====================================================================================

template<uint32_t kCovFmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type8_format1(GPosContext& ctx, Table<GPosTable::ChainedSequenceContext1> table, ApplyRange scope, LookupFlags flags, const CoverageTableIterator& cov_it) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT(scope.index() < scope.end());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GPosTable::ChainedSequenceContext1::kBaseSize + rule_set_count * 2u));

  ChainedMatchContext mCtx;
  mCtx.table = table;
  mCtx.first_glyph_range = cov_it.glyph_range<kCovFmt>();
  mCtx.back_glyph_ptr = ctx.glyph_data();
  mCtx.ahead_glyph_ptr = ctx.glyph_data() + scope.index();
  mCtx.back_glyph_count = scope.index();
  mCtx.ahead_glyph_count = scope.size();

  const Offset16* rule_set_offsets = table->rule_set_offsets.array();
  do {
    SequenceMatch match;
    if (match_chained_sequence_format1<kCovFmt>(mCtx, rule_set_offsets, rule_set_count, cov_it, &match)) {
      BL_PROPAGATE(apply_gpos_nested_lookups(ctx, size_t(mCtx.ahead_glyph_ptr - ctx.glyph_data()), match));
    }
    mCtx.ahead_glyph_ptr++;
    mCtx.back_glyph_count++;
  } while (--mCtx.ahead_glyph_count != 0);

  return BL_SUCCESS;
}

template<uint32_t kCovFmt, uint32_t kCD1Fmt, uint32_t kCD2Fmt, uint32_t kCD3Fmt>
static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type8_format2(
  GPosContext& ctx,
  Table<GPosTable::ChainedSequenceContext2> table,
  ApplyRange scope,
  LookupFlags flags,
  const CoverageTableIterator& cov_it, const ClassDefTableIterator& cd1_it, const ClassDefTableIterator& cd2_it, const ClassDefTableIterator& cd3_it) noexcept {

  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT(scope.index() < scope.end());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t rule_set_count = table->rule_set_offsets.count();
  BL_ASSERT_VALIDATED(table.fits(GPosTable::ChainedSequenceContext2::kBaseSize + rule_set_count * 2u));

  ChainedMatchContext mCtx;
  mCtx.table = table;
  mCtx.first_glyph_range = cov_it.glyph_range<kCovFmt>();
  mCtx.back_glyph_ptr = ctx.glyph_data();
  mCtx.ahead_glyph_ptr = ctx.glyph_data() + scope.index();
  mCtx.back_glyph_count = scope.index();
  mCtx.ahead_glyph_count = scope.size();

  const Offset16* rule_set_offsets = table->rule_set_offsets.array();
  do {
    SequenceMatch match;
    if (match_chained_sequence_format2<kCovFmt, kCD1Fmt, kCD2Fmt, kCD3Fmt>(mCtx, rule_set_offsets, rule_set_count, cov_it, cd1_it, cd2_it, cd3_it, &match)) {
      BL_PROPAGATE(apply_gpos_nested_lookups(ctx, size_t(mCtx.ahead_glyph_ptr - ctx.glyph_data()), match));
    }
    mCtx.ahead_glyph_ptr++;
    mCtx.back_glyph_count++;
  } while (--mCtx.ahead_glyph_count != 0);

  return BL_SUCCESS;
}

static BL_INLINE_IF_NOT_DEBUG BLResult apply_gpos_lookup_type8_format3(GPosContext& ctx, Table<GPosTable::ChainedSequenceContext3> table, ApplyRange scope, LookupFlags flags) noexcept {
  BL_ASSERT(scope.end() <= ctx.size());
  BL_ASSERT_VALIDATED(table.fits());

  uint32_t backtrack_glyph_count = table->backtrack_glyph_count();
  uint32_t input_offset = 4u + backtrack_glyph_count * 2u;
  BL_ASSERT_VALIDATED(table.fits(input_offset + 2u));

  uint32_t input_glyph_count = table.readU16(input_offset);
  uint32_t lookahead_offset = input_offset + 2u + input_glyph_count * 2u;
  BL_ASSERT_VALIDATED(input_glyph_count > 0);
  BL_ASSERT_VALIDATED(table.fits(lookahead_offset + 2u));

  uint32_t lookahead_glyph_count = table.readU16(lookahead_offset);
  uint32_t lookup_offset = lookahead_offset + 2u + lookahead_glyph_count * 2u;
  BL_ASSERT_VALIDATED(table.fits(lookup_offset + 2u));

  uint32_t lookup_record_count = table.readU16(lookup_offset);
  BL_ASSERT_VALIDATED(lookup_record_count > 0);
  BL_ASSERT_VALIDATED(table.fits(lookup_offset + 2u + lookup_record_count * GSubGPosTable::SequenceLookupRecord::kBaseSize));

  // Restrict scope in a way so we would never underflow/overflow glyph buffer when matching backtrack/lookahead glyphs.
  uint32_t input_and_lookahead_glyph_count = input_glyph_count + lookahead_glyph_count;
  scope.intersect(backtrack_glyph_count, ctx.size() - input_and_lookahead_glyph_count);

  // Bail if the buffer or the scope is too small for this chained context substitution.
  if (scope.size() < input_and_lookahead_glyph_count || scope.index() >= scope.end())
    return BL_SUCCESS;

  const Offset16* backtrack_coverage_offsets = table->backtrack_coverage_offsets();
  const Offset16* input_coverage_offsets = table.data_as<Offset16>(input_offset + 2u);
  const Offset16* lookahead_coverage_offsets = table.data_as<Offset16>(lookahead_offset + 2u);

  CoverageTableIterator cov0_it;
  uint32_t cov0_fmt = cov0_it.init(table.sub_table_unchecked(input_coverage_offsets[0].value()));
  GlyphRange first_glyph_range = cov0_it.glyph_range_with_format(cov0_fmt);

  ChainedMatchContext mCtx;
  mCtx.table = table;
  mCtx.first_glyph_range = cov0_it.glyph_range_with_format(cov0_fmt);
  mCtx.back_glyph_ptr = ctx.glyph_data();
  mCtx.ahead_glyph_ptr = ctx.glyph_data() + scope.index();
  mCtx.back_glyph_count = scope.index();
  mCtx.ahead_glyph_count = scope.size();

  SequenceMatch match;
  match.glyph_count = input_glyph_count;
  match.lookup_records = table.data_as<GSubTable::SequenceLookupRecord>(lookup_offset + 2u);
  match.lookup_record_count = lookup_record_count;

  do {
    if (match_chained_sequence_format3(mCtx,
        backtrack_coverage_offsets, backtrack_glyph_count,
        input_coverage_offsets, input_glyph_count,
        lookahead_coverage_offsets, lookahead_glyph_count,
        first_glyph_range,
        cov0_it, cov0_fmt)) {
      BL_PROPAGATE(apply_gpos_nested_lookups(ctx, size_t(mCtx.ahead_glyph_ptr - ctx.glyph_data()), match));
    }
    mCtx.back_glyph_count++;
    mCtx.ahead_glyph_ptr++;
  } while (--mCtx.ahead_glyph_count >= input_and_lookahead_glyph_count);

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GPOS - Lookup Dispatch
// =================================================

static BL_INLINE_IF_NOT_DEBUG bool validateGPosLookup(ValidationContext& validator, RawTable table, GPosLookupAndFormat type_and_format) noexcept {
  switch (type_and_format) {
    case GPosLookupAndFormat::kType1Format1: return validate_gpos_lookup_type1_format1(validator, table);
    case GPosLookupAndFormat::kType1Format2: return validate_gpos_lookup_type1_format2(validator, table);
    case GPosLookupAndFormat::kType2Format1: return validate_gpos_lookup_type2_format1(validator, table);
    case GPosLookupAndFormat::kType2Format2: return validate_gpos_lookup_type2_format2(validator, table);
    case GPosLookupAndFormat::kType3Format1: return validate_gpos_lookup_type3_format1(validator, table);
    /*
    case GPosLookupAndFormat::kType4Format1: return validate_gpos_lookup_type4_format1(validator, table);
    case GPosLookupAndFormat::kType5Format1: return validate_gpos_lookup_type5_format1(validator, table);
    case GPosLookupAndFormat::kType6Format1: return validate_gpos_lookup_type6_format1(validator, table);
    */
    case GPosLookupAndFormat::kType7Format1: return validate_gpos_lookup_type7_format1(validator, table);
    case GPosLookupAndFormat::kType7Format2: return validate_gpos_lookup_type7_format2(validator, table);
    case GPosLookupAndFormat::kType7Format3: return validate_gpos_lookup_type7_format3(validator, table);
    case GPosLookupAndFormat::kType8Format1: return validate_gpos_lookup_type8_format1(validator, table);
    case GPosLookupAndFormat::kType8Format2: return validate_gpos_lookup_type8_format2(validator, table);
    case GPosLookupAndFormat::kType8Format3: return validate_gpos_lookup_type8_format3(validator, table);
    default:
      return false;
  }
}

static BLResult apply_gpos_lookup(GPosContext& ctx, RawTable table, GPosLookupAndFormat type_and_format, ApplyRange scope, LookupFlags flags) noexcept {
  BL_ASSERT_VALIDATED(table.fits(gpos_lookup_info_table.lookup_info[size_t(type_and_format)].header_size));

  #define BL_APPLY_WITH_COVERAGE(FN, TABLE)                                        \
    CoverageTableIterator cov_it;                                                   \
    if (cov_it.init(table.sub_table(table.data_as<TABLE>()->coverage_offset())) == 1u) \
      result = FN<1>(ctx, table, scope, flags, cov_it);                             \
    else                                                                           \
      result = FN<2>(ctx, table, scope, flags, cov_it);

  BLResult result = BL_SUCCESS;
  switch (type_and_format) {
    case GPosLookupAndFormat::kType1Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gpos_lookup_type1_format1, GPosTable::SingleAdjustment1)
      break;
    }

    case GPosLookupAndFormat::kType1Format2: {
      BL_APPLY_WITH_COVERAGE(apply_gpos_lookup_type1_format2, GPosTable::SingleAdjustment2)
      break;
    }

    case GPosLookupAndFormat::kType2Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gpos_lookup_type2_format1, GPosTable::PairAdjustment1)
      break;
    }

    case GPosLookupAndFormat::kType2Format2: {
      CoverageTableIterator cov_it;
      ClassDefTableIterator cd1_it;
      ClassDefTableIterator cd2_it;

      FormatBits3X fmt_bits = FormatBits3X(
        ((cov_it.init(table.sub_table(table.data_as<GPosTable::PairAdjustment2>()->coverage_offset()))  - 1u) << 2) |
        ((cd1_it.init(table.sub_table(table.data_as<GPosTable::PairAdjustment2>()->classDef1Offset())) - 1u) << 1) |
        ((cd2_it.init(table.sub_table(table.data_as<GPosTable::PairAdjustment2>()->classDef2Offset())) - 1u) << 0));

      switch (fmt_bits) {
        case FormatBits3X::k111: result = apply_gpos_lookup_type2_format2<1, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
        case FormatBits3X::k112: result = apply_gpos_lookup_type2_format2<1, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
        case FormatBits3X::k121: result = apply_gpos_lookup_type2_format2<1, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
        case FormatBits3X::k122: result = apply_gpos_lookup_type2_format2<1, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
        case FormatBits3X::k211: result = apply_gpos_lookup_type2_format2<2, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
        case FormatBits3X::k212: result = apply_gpos_lookup_type2_format2<2, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
        case FormatBits3X::k221: result = apply_gpos_lookup_type2_format2<2, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
        case FormatBits3X::k222: result = apply_gpos_lookup_type2_format2<2, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it); break;
      }

      break;
    }

    case GPosLookupAndFormat::kType3Format1:
    case GPosLookupAndFormat::kType4Format1:
    case GPosLookupAndFormat::kType5Format1:
    case GPosLookupAndFormat::kType6Format1:
      // TODO: [OpenType] GPOS missing lookups.
      break;

    case GPosLookupAndFormat::kType7Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gpos_lookup_type7_format1, GPosTable::LookupHeaderWithCoverage)
      break;
    }

    case GPosLookupAndFormat::kType7Format2: {
      CoverageTableIterator cov_it;
      ClassDefTableIterator cd_it;

      FormatBits2X fmt_bits = FormatBits2X(
        ((cov_it.init(table.sub_table(table.data_as<GPosTable::SequenceContext2>()->coverage_offset())) - 1u) << 1) |
        ((cd_it.init(table.sub_table(table.data_as<GPosTable::SequenceContext2>()->class_def_offset()))  - 1u) << 0));

      switch (fmt_bits) {
        case FormatBits2X::k11: result = apply_gpos_lookup_type7_format2<1, 1>(ctx, table, scope, flags, cov_it, cd_it); break;
        case FormatBits2X::k12: result = apply_gpos_lookup_type7_format2<1, 2>(ctx, table, scope, flags, cov_it, cd_it); break;
        case FormatBits2X::k21: result = apply_gpos_lookup_type7_format2<2, 1>(ctx, table, scope, flags, cov_it, cd_it); break;
        case FormatBits2X::k22: result = apply_gpos_lookup_type7_format2<2, 2>(ctx, table, scope, flags, cov_it, cd_it); break;
      }

      break;
    }

    case GPosLookupAndFormat::kType7Format3: {
      result = apply_gpos_lookup_type7_format3(ctx, table, scope, flags);
      break;
    }

    case GPosLookupAndFormat::kType8Format1: {
      BL_APPLY_WITH_COVERAGE(apply_gpos_lookup_type8_format1, GPosTable::LookupHeaderWithCoverage)
      break;
    }

    case GPosLookupAndFormat::kType8Format2: {
      CoverageTableIterator cov_it;
      ClassDefTableIterator cd1_it;
      ClassDefTableIterator cd2_it;
      ClassDefTableIterator cd3_it;

      FormatBits4X fmt_bits = FormatBits4X(
        ((cov_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->coverage_offset()         )) - 1u) << 3) |
        ((cd1_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->backtrack_class_def_offset())) - 1u) << 2) |
        ((cd2_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->input_class_def_offset()    )) - 1u) << 1) |
        ((cd3_it.init(table.sub_table(table.data_as<GPosTable::ChainedSequenceContext2>()->lookahead_class_def_offset())) - 1u) << 0));

      switch (fmt_bits) {
        case FormatBits4X::k1111: result = apply_gpos_lookup_type8_format2<1, 1, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1112: result = apply_gpos_lookup_type8_format2<1, 1, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1121: result = apply_gpos_lookup_type8_format2<1, 1, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1122: result = apply_gpos_lookup_type8_format2<1, 1, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1211: result = apply_gpos_lookup_type8_format2<1, 2, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1212: result = apply_gpos_lookup_type8_format2<1, 2, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1221: result = apply_gpos_lookup_type8_format2<1, 2, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k1222: result = apply_gpos_lookup_type8_format2<1, 2, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2111: result = apply_gpos_lookup_type8_format2<2, 1, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2112: result = apply_gpos_lookup_type8_format2<2, 1, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2121: result = apply_gpos_lookup_type8_format2<2, 1, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2122: result = apply_gpos_lookup_type8_format2<2, 1, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2211: result = apply_gpos_lookup_type8_format2<2, 2, 1, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2212: result = apply_gpos_lookup_type8_format2<2, 2, 1, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2221: result = apply_gpos_lookup_type8_format2<2, 2, 2, 1>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
        case FormatBits4X::k2222: result = apply_gpos_lookup_type8_format2<2, 2, 2, 2>(ctx, table, scope, flags, cov_it, cd1_it, cd2_it, cd3_it); break;
      }
      break;
    }

    case GPosLookupAndFormat::kType8Format3: {
      result = apply_gpos_lookup_type8_format3(ctx, table, scope, flags);
      break;
    }

    default:
      break;
  }

  #undef BL_APPLY_WITH_COVERAGE

  return result;
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Validate
// =================================================

static bool validate_lookup(ValidationContext& validator, Table<GSubGPosTable> table, uint32_t lookup_index) noexcept {
  const char* table_name = "LookupList";
  const OTFaceImpl* ot_face_impl = validator.ot_face_impl();

  if (!table.fits())
    return validator.invalid_table_size(table_name, table.size, GSubGPosTable::kBaseSize);

  LookupKind lookup_kind = validator.lookup_kind();
  bool isGSub = lookup_kind == LookupKind::kGSUB;

  const GSubGPosLookupInfo& lookup_info = isGSub ? gsub_lookup_info_table : gpos_lookup_info_table;
  const LayoutData::GSubGPos& layout_data = ot_face_impl->layout.kinds[size_t(lookup_kind)];
  Table<Array16<UInt16>> lookup_list(table.sub_table(layout_data.lookup_list_offset));

  uint32_t lookup_count = layout_data.lookup_count;
  if (lookup_index >= lookup_count)
    return validator.fail("%s[%u] doesn't exist (lookup_count=%u)", table_name, lookup_index, lookup_count);

  uint32_t lookup_list_size = 2u + lookup_count * 2u;
  if (!lookup_list.fits(lookup_list_size))
    return validator.invalid_table_size(table_name, lookup_list.size, lookup_list_size);

  uint32_t lookup_offset = lookup_list->array()[lookup_index].value();
  OffsetRange lookup_offset_range{lookup_list_size, lookup_list.size - GSubGPosTable::LookupTable::kBaseSize};

  if (!lookup_offset_range.contains(lookup_offset))
    return validator.fail("%s[%u] has invalid offset (%u), valid range=[%u:%u]",
      table_name, lookup_index, lookup_offset, lookup_offset_range.start, lookup_offset_range.end);

  Table<GSubGPosTable::LookupTable> lookup_table(lookup_list.sub_table(lookup_offset));
  uint32_t lookup_type = lookup_table->lookup_type();

  // Reject unknown lookup Type+Format combinations (lookup_type 0 and values above lookup_max_value are invalid).
  if (lookup_type - 1u > uint32_t(lookup_info.lookup_max_value))
    return validator.fail("%s[%u] invalid lookup type (%u)", table_name, lookup_index, lookup_type);

  uint32_t sub_table_count = lookup_table->sub_table_offsets.count();
  const Offset16* sub_table_offsets = lookup_table->sub_table_offsets.array();

  uint32_t lookup_table_size = GSubGPosTable::LookupTable::kBaseSize + sub_table_count * 2u;
  if (!lookup_table.fits(lookup_table_size))
    return validator.fail("%s[%u] truncated (size=%u, required=%u)", table_name, lookup_index, lookup_table.size, lookup_table_size);

  uint32_t sub_table_min_size = lookup_type == lookup_info.extension_type ? 8u : 6u;
  OffsetRange sub_table_offset_range{lookup_table_size, lookup_table.size - sub_table_min_size};

  uint32_t ext_previous_lookup_type = 0u;

  for (uint32_t sub_table_index = 0; sub_table_index < sub_table_count; sub_table_index++) {
    uint32_t sub_table_offset = sub_table_offsets[sub_table_index].value();
    if (!sub_table_offset_range.contains(sub_table_offset))
      return validator.fail("%s[%u].sub_table[%u] has invalid offset (%u), valid range=[%u:%u]",
        table_name, lookup_index, sub_table_index, sub_table_offset, sub_table_offset_range.start, sub_table_offset_range.end);

    Table<GSubGPosTable::LookupHeader> sub_table(lookup_table.sub_table(sub_table_offset));
    uint32_t lookup_format = sub_table->format();

    uint32_t lookup_type_and_format = lookup_info.type_info[lookup_type].type_and_format + lookup_format - 1u;
    if (lookup_type == lookup_info.extension_type) {
      Table<GSubGPosTable::ExtensionLookup> ext_sub_table(sub_table);

      lookup_type = ext_sub_table->lookup_type();
      uint32_t ext_lookup_format = ext_sub_table->format();
      uint32_t ext_sub_table_offset = ext_sub_table->offset();

      if (!ext_previous_lookup_type && lookup_type != ext_previous_lookup_type)
        return validator.fail("%s[%u].sub_table[%u] has a different type (%u) than a previous extension (%u)",
          table_name, lookup_index, sub_table_index, lookup_type, ext_previous_lookup_type);

      ext_previous_lookup_type = lookup_type;
      bool valid_type = lookup_type != lookup_info.extension_type && lookup_type - 1u < uint32_t(lookup_info.lookup_max_value);

      if (!valid_type || ext_lookup_format != 1)
        return validator.fail("%s[%u].sub_table[%u] has invalid extension type (%u) & format (%u) combination",
          table_name, lookup_index, sub_table_index, lookup_type, ext_lookup_format);

      sub_table = ext_sub_table.sub_table(ext_sub_table_offset);
      if (!sub_table.fits())
        return validator.fail("%s[%u].sub_table[%u] of extension type points to a truncated table (size=%u required=%u)",
          table_name, lookup_index, sub_table_index);

      lookup_format = sub_table->format();
      lookup_type_and_format = lookup_info.type_info[lookup_type].type_and_format + lookup_format - 1u;
    }

    if (lookup_format - 1u >= lookup_info.type_info[lookup_type].format_count)
      return validator.fail("%s[%u].sub_table[%u] has invalid type (%u) & format (%u) combination",
        table_name, lookup_index, sub_table_index, lookup_type, lookup_format);

    bool valid = isGSub ? validateGSubLookup(validator, sub_table, GSubLookupAndFormat(lookup_type_and_format))
                        : validateGPosLookup(validator, sub_table, GPosLookupAndFormat(lookup_type_and_format));
    if (!valid)
      return false;
  }

  return true;
}

static BL_NOINLINE LayoutData::LookupStatusBits validate_lookups(const OTFaceImpl* ot_face_impl, LookupKind lookup_kind, uint32_t word_index, uint32_t lookup_bits) noexcept {
  Table<GSubGPosTable> table(ot_face_impl->layout.tables[size_t(lookup_kind)]);
  const LayoutData::GSubGPos& layout_data = ot_face_impl->layout.kinds[size_t(lookup_kind)];

  uint32_t base_index = word_index * 32u;
  uint32_t lookup_count = layout_data.lookup_count;

  ValidationContext validator(ot_face_impl, lookup_kind);
  uint32_t analyzed_bits = lookup_bits;
  uint32_t valid_bits = 0;

  BitSetOps::BitIterator it(analyzed_bits);
  while (it.has_next()) {
    uint32_t bit_index = it.next();
    uint32_t lookup_index = base_index + bit_index;

    if (lookup_index >= lookup_count)
      break;

    if (validate_lookup(validator, table, lookup_index))
      valid_bits |= BitSetOps::index_as_mask(bit_index);
  }

  return ot_face_impl->layout.commit_lookup_status_bits(lookup_kind, word_index, LayoutData::LookupStatusBits::make(analyzed_bits, valid_bits));
}

// bl::OpenType::LayoutImpl - Apply
// ================================

static BL_INLINE BLResult apply_lookup(GSubContext& ctx, RawTable table, uint32_t type_and_format, ApplyRange scope, LookupFlags flags) noexcept {
  return apply_gsub_lookup(ctx, table, GSubLookupAndFormat(type_and_format), scope, flags);
}

static BL_INLINE BLResult apply_lookup(GPosContext& ctx, RawTable table, uint32_t type_and_format, ApplyRange scope, LookupFlags flags) noexcept {
  return apply_gpos_lookup(ctx, table, GPosLookupAndFormat(type_and_format), scope, flags);
}

static void debug_glyph_and_clusters_to_message_sink(DebugSink debug_sink, const BLGlyphId* glyph_data, const BLGlyphInfo* info_data, size_t size) noexcept {
  BLString s;

  s.append('[');
  for (size_t i = 0; i < size; i++) {
    s.append_format("%u@%u", glyph_data[i], info_data[i].cluster);
    if (i != size - 1)
      s.append_format(", ");
  }
  s.append(']');

  debug_sink.message(s);
}

static void debug_context_to_message_sink(GSubContext& ctx) noexcept {
  debug_glyph_and_clusters_to_message_sink(ctx._debug_sink, ctx.glyph_data(), ctx.info_data(), ctx.size());
}

static void debug_context_to_message_sink(GPosContext& ctx) noexcept {
}

template<LookupKind kLookupKind, typename Context>
static BLResult BL_CDECL apply_lookups(const BLFontFaceImpl* face_impl, BLGlyphBuffer* gb, const uint32_t* bit_words, size_t bit_word_count) noexcept {
  constexpr uint32_t kLookupExtension = kLookupKind == LookupKind::kGSUB ? uint32_t(GSubTable::kLookupExtension) : uint32_t(GPosTable::kLookupExtension);

  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  const GSubGPosLookupInfo& lookup_info = kLookupKind == LookupKind::kGSUB ? gsub_lookup_info_table : gpos_lookup_info_table;

  Context ctx;
  ctx.init(bl_glyph_buffer_get_impl(gb));

  if (ctx.is_empty())
    return BL_SUCCESS;

  RawTable table(ot_face_impl->layout.tables[size_t(kLookupKind)]);
  const LayoutData::GSubGPos& layout_data = ot_face_impl->layout.kinds[size_t(kLookupKind)];
  Table<Array16<UInt16>> lookup_list_table(table.sub_table_unchecked(layout_data.lookup_list_offset));

  bool did_process_lookup = false;
  uint32_t word_count = uint32_t(bl_min<size_t>(bit_word_count, layout_data.lookup_status_data_size));

  for (uint32_t word_index = 0; word_index < word_count; word_index++) {
    uint32_t lookup_bits = bit_words[word_index];
    if (!lookup_bits)
      continue;

    // In order to process a GSUB/GPOS lookup we first validate them so the processing pipeline does not have
    // to perform validation every time it processes a lookup. Validation is lazy so only lookups that need to
    // be processed are validated.
    //
    // We have to first check whether the lookups represented by `bits` were already analyzed. If so, then we
    // can just flip bits describing lookups where validation failed and only process lookups that are valid.
    LayoutData::LookupStatusBits status_bits = ot_face_impl->layout.get_lookup_status_bits(kLookupKind, word_index);
    uint32_t non_analyzed_bits = lookup_bits & ~status_bits.analyzed;

    if (BL_UNLIKELY(non_analyzed_bits))
      status_bits = validate_lookups(ot_face_impl, kLookupKind, word_index, non_analyzed_bits);

    // Remove lookups from bits that are invalid, and thus won't be processed. Note that conforming fonts will
    // never have invalid lookups, but it's possible that a font is corrupted or malformed on purpose.
    lookup_bits &= status_bits.valid;

    uint32_t bit_offset = word_index * 32u;
    BitSetOps::BitIterator it(lookup_bits);

    while (it.has_next()) {
      uint32_t lookup_table_index = it.next() + bit_offset;
      BL_ASSERT_VALIDATED(lookup_table_index < layout_data.lookup_count);

      uint32_t lookup_table_offset = lookup_list_table->array()[lookup_table_index].value();
      BL_ASSERT_VALIDATED(lookup_table_offset <= lookup_list_table.size - 6u);

      Table<GSubGPosTable::LookupTable> lookup_table(lookup_list_table.sub_table_unchecked(lookup_table_offset));
      uint32_t lookup_type = lookup_table->lookup_type();
      uint32_t lookup_flags = lookup_table->lookup_flags();
      BL_ASSERT_VALIDATED(lookup_type - 1u < uint32_t(lookup_info.lookup_max_value));

      uint32_t lookup_entry_count = lookup_table->sub_table_offsets.count();
      const Offset16* lookup_entry_offsets = lookup_table->sub_table_offsets.array();

      const GSubGPosLookupInfo::TypeInfo& lookup_type_info = lookup_info.type_info[lookup_type];
      uint32_t lookup_table_min_size = lookup_type == kLookupExtension ? 8u : 6u;
      BL_ASSERT_VALIDATED(lookup_table.fits(lookup_table_min_size + lookup_entry_count * 2u));

      // Only used in Debug builds when `BL_ASSERT_VALIDATED` is enabled.
      bl_unused(lookup_table_min_size);

      for (uint32_t j = 0; j < lookup_entry_count; j++) {
        uint32_t lookup_offset = lookup_entry_offsets[j].value();
        BL_ASSERT_VALIDATED(lookup_offset <= lookup_table.size - lookup_table_min_size);

        Table<GSubGPosTable::LookupHeader> lookup_header(lookup_table.sub_table_unchecked(lookup_offset));
        uint32_t lookup_format = lookup_header->format();
        BL_ASSERT_VALIDATED(lookup_format - 1u < lookup_type_info.format_count);

        uint32_t lookup_type_and_format = lookup_type_info.type_and_format + lookup_format - 1u;
        if (lookup_type == kLookupExtension) {
          Table<GSubGPosTable::ExtensionLookup> extension_table(lookup_table.sub_table_unchecked(lookup_offset));

          uint32_t extension_lookup_type = extension_table->lookup_type();
          uint32_t extension_offset = extension_table->offset();

          BL_ASSERT_VALIDATED(extension_lookup_type - 1u < lookup_info.lookup_max_value);
          BL_ASSERT_VALIDATED(extension_offset <= extension_table.size - 6u);

          lookup_header = extension_table.sub_table_unchecked(extension_offset);
          lookup_format = lookup_header->format();

          const GSubGPosLookupInfo::TypeInfo& extension_lookup_type_info = lookup_info.type_info[extension_lookup_type];
          BL_ASSERT_VALIDATED(lookup_format - 1u < extension_lookup_type_info.format_count);

          lookup_type_and_format = extension_lookup_type_info.type_and_format + lookup_format - 1u;
        }

        if (ctx._debug_sink.enabled()) {
          debug_context_to_message_sink(ctx);
          BLString s;
          if (kLookupKind == LookupKind::kGSUB)
            s.assign_format("Applying GSUB Lookup[%u].%s%u[%u]", lookup_table_index, gsub_lookup_name(lookup_type), lookup_format, j);
          else
            s.assign_format("Applying GPOS Lookup[%u].%s%u[%u]", lookup_table_index, gpos_lookup_name(lookup_type), lookup_format, j);

          ctx._debug_sink.message(s);
          did_process_lookup = true;
        }

        BL_PROPAGATE(apply_lookup(ctx, lookup_header, lookup_type_and_format, ApplyRange{0, ctx.size()}, LookupFlags(lookup_flags)));

        if (ctx.is_empty())
          goto done;
      }
    }
  }

done:
  if (ctx._debug_sink.enabled()) {
    if (did_process_lookup)
      debug_context_to_message_sink(ctx);
  }

  ctx.done();

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - GSUB & GPOS - Init
// =============================================

static BLResult initGSubGPos(OTFaceImpl* ot_face_impl, Table<GSubGPosTable> table, LookupKind lookup_kind) noexcept {
  if (BL_UNLIKELY(!table.fits())) {
    return BL_SUCCESS;
  }

  uint32_t version = table->v1_0()->version();
  if (BL_UNLIKELY(version < 0x00010000u || version > 0x00010001u)) {
    return BL_SUCCESS;
  }

  uint32_t header_size = GPosTable::HeaderV1_0::kBaseSize;
  if (version >= 0x00010001u) {
    header_size = GPosTable::HeaderV1_1::kBaseSize;
  }

  if (BL_UNLIKELY(!table.fits(header_size))) {
    return BL_SUCCESS;
  }

  uint32_t lookup_list_offset = table->v1_0()->lookup_list_offset();
  uint32_t feature_list_offset = table->v1_0()->feature_list_offset();
  uint32_t script_list_offset = table->v1_0()->script_list_offset();

  // Some fonts have these set to a table size to indicate that there are no lookups, fix this...
  if (lookup_list_offset == table.size) {
    lookup_list_offset = 0;
  }

  if (feature_list_offset == table.size) {
    feature_list_offset = 0;
  }

  if (script_list_offset == table.size) {
    script_list_offset = 0;
  }

  OffsetRange offset_range{header_size, table.size - 2u};

  // First validate core offsets - if a core offset is wrong we won't use GSUB/GPOS at all.
  if (lookup_list_offset) {
    if (BL_UNLIKELY(!offset_range.contains(lookup_list_offset))) {
      return BL_SUCCESS;
    }
  }

  if (feature_list_offset) {
    if (BL_UNLIKELY(!offset_range.contains(feature_list_offset))) {
      return BL_SUCCESS;
    }
  }

  if (script_list_offset) {
    if (BL_UNLIKELY(!offset_range.contains(script_list_offset))) {
      return BL_SUCCESS;
    }
  }

  LayoutData::GSubGPos& d = ot_face_impl->layout.kinds[size_t(lookup_kind)];

  if (lookup_list_offset) {
    Table<Array16<Offset16>> lookup_list_offsets(table.sub_table_unchecked(lookup_list_offset));
    uint32_t count = lookup_list_offsets->count();

    if (count) {
      d.lookup_list_offset = uint16_t(lookup_list_offset);
      d.lookup_count = uint16_t(count);
      ot_face_impl->ot_flags |= (lookup_kind == LookupKind::kGPOS) ? OTFaceFlags::kGPosLookupList : OTFaceFlags::kGSubLookupList;
    }
  }

  if (feature_list_offset) {
    Table<Array16<TagRef16>> feature_list_offsets(table.sub_table_unchecked(feature_list_offset));
    uint32_t count = feature_list_offsets->count();

    if (count) {
      const TagRef16* array = feature_list_offsets->array();
      for (uint32_t i = 0; i < count; i++) {
        BLTag feature_tag = array[i].tag();
        BL_PROPAGATE(ot_face_impl->feature_tag_set.add_tag(feature_tag));
      }

      d.feature_count = uint16_t(count);
      d.feature_list_offset = uint16_t(feature_list_offset);
      ot_face_impl->ot_flags |= (lookup_kind == LookupKind::kGPOS) ? OTFaceFlags::kGPosFeatureList : OTFaceFlags::kGSubFeatureList;
    }
  }

  if (script_list_offset) {
    Table<Array16<TagRef16>> script_list_offsets(table.sub_table_unchecked(script_list_offset));
    uint32_t count = script_list_offsets->count();

    if (count) {
      const TagRef16* array = script_list_offsets->array();
      for (uint32_t i = 0; i < count; i++) {
        BLTag script_tag = array[i].tag();
        BL_PROPAGATE(ot_face_impl->script_tag_set.add_tag(script_tag));
      }

      d.script_list_offset = uint16_t(script_list_offset);
      ot_face_impl->ot_flags |= (lookup_kind == LookupKind::kGPOS) ? OTFaceFlags::kGPosScriptList : OTFaceFlags::kGSubScriptList;
    }
  }

  if (d.lookup_count) {
    if (lookup_kind == LookupKind::kGSUB)
      ot_face_impl->funcs.apply_gsub = apply_lookups<LookupKind::kGSUB, GSubContextPrimary>;
    else
      ot_face_impl->funcs.apply_gpos = apply_lookups<LookupKind::kGPOS, GPosContext>;
    ot_face_impl->layout.tables[size_t(lookup_kind)] = table;
  }

  return BL_SUCCESS;
}

// bl::OpenType::LayoutImpl - Plan
// ===============================

static Table<GSubGPosTable::ScriptTable> find_script_in_script_list(Table<Array16<TagRef16>> script_list_offsets, BLTag script_tag, BLTag default_script_tag) noexcept {
  const TagRef16* script_list_array = script_list_offsets->array();
  uint32_t script_count = script_list_offsets->count();

  Table<GSubGPosTable::ScriptTable> table_out{};

  if (script_list_offsets.size >= 2u + script_count * uint32_t(sizeof(TagRef16))) {
    for (uint32_t i = 0; i < script_count; i++) {
      BLTag record_tag = script_list_array[i].tag();
      if (record_tag == script_tag || record_tag == default_script_tag) {
        table_out = script_list_offsets.sub_table_unchecked(script_list_array[i].offset());
        if (record_tag == script_tag) {
          break;
        }
      }
    }
  }

  return table_out;
}

template<bool kSSO>
static BL_INLINE void populate_gsub_gpoos_lookup_bits(
  Table<GSubGPosTable::LangSysTable> lang_sys_table,
  Table<Array16<TagRef16>> feature_list_offsets,
  uint32_t feature_index_count,
  uint32_t feature_count,
  uint32_t lookup_count,
  const BLFontFeatureSettings& settings,
  uint32_t* plan_bits) noexcept {

  BL_ASSERT(settings._d.sso() == kSSO);

  // We need to process required_feature_index as well as all the features from the list. To not duplicate the
  // code inside, we setup the `required_feature_index` here and just continue using the list after it's processed.
  uint32_t i = uint32_t(0) - 1u;
  uint32_t feature_index = lang_sys_table->required_feature_index();

  for (;;) {
    if (BL_LIKELY(feature_index < feature_count)) {
      BLTag feature_tag = feature_list_offsets->array()[feature_index].tag();
      if (FontFeatureSettingsInternal::is_feature_enabled_for_plan<kSSO>(&settings, feature_tag)) {
        uint32_t feature_offset = feature_list_offsets->array()[feature_index].offset();
        Table<GSubGPosTable::FeatureTable> feature_table(feature_list_offsets.sub_table_unchecked(feature_offset));

        // Don't use a feature if its offset is out of bounds.
        if (BL_LIKELY(blFontTableFitsT<GSubGPosTable::FeatureTable>(feature_table))) {
          // Don't use a lookup if its offset is out of bounds.
          uint32_t lookup_index_count = feature_table->lookup_list_indexes.count();
          if (BL_LIKELY(feature_table.size >= GSubGPosTable::FeatureTable::kBaseSize + lookup_index_count * 2u)) {
            for (uint32_t j = 0; j < lookup_index_count; j++) {
              uint32_t lookup_index = feature_table->lookup_list_indexes.array()[j].value();
              if (BL_LIKELY(lookup_index < lookup_count)) {
                BitArrayOps::bit_array_set_bit(plan_bits, lookup_index);
              }
            }
          }
        }
      }
    }

    if (++i >= feature_index_count) {
      break;
    }

    feature_index = lang_sys_table->feature_indexes.array()[i].value();
  }
}

static BLResult calculateGSubGPosPlan(const OTFaceImpl* ot_face_impl, const BLFontFeatureSettings& settings, LookupKind lookup_kind, BLBitArrayCore* plan) noexcept {
  BLTag script_tag = BL_MAKE_TAG('l', 'a', 't', 'n');
  BLTag dflt_script_tag = BL_MAKE_TAG('D', 'F', 'L', 'T');

  const LayoutData::GSubGPos& d = ot_face_impl->layout.kinds[size_t(lookup_kind)];
  Table<GSubGPosTable> table = ot_face_impl->layout.tables[size_t(lookup_kind)];

  if (!table) {
    return BL_SUCCESS;
  }

  Table<Array16<TagRef16>> script_list_offsets(table.sub_table_unchecked(d.script_list_offset));
  Table<Array16<TagRef16>> feature_list_offsets(table.sub_table_unchecked(d.feature_list_offset));
  Table<GSubGPosTable::ScriptTable> script_table(find_script_in_script_list(script_list_offsets, script_tag, dflt_script_tag));

  if (script_table.is_empty()) {
    return BL_SUCCESS;
  }

  if (BL_UNLIKELY(!blFontTableFitsT<GSubGPosTable::ScriptTable>(script_table))) {
    return BL_SUCCESS;
  }

  uint32_t lang_sys_offset = script_table->lang_sys_default();

  /*
  {
    uint32_t lang_sys_count = script_table->lang_sys_offsets.count();
    for (uint32_t i = 0; i < lang_sys_count; i++) {
      uint32_t tag = script_table->lang_sys_offsets.array()[i].tag();
      if (tag == BL_MAKE_TAG('D', 'F', 'L', 'T')) {
        lang_sys_offset = script_table->lang_sys_offsets.array()[i].offset();
      }
    }
  }
  */

  if (lang_sys_offset == 0) {
    return BL_SUCCESS;
  }

  Table<GSubGPosTable::LangSysTable> lang_sys_table(script_table.sub_table_unchecked(lang_sys_offset));
  if (BL_UNLIKELY(!blFontTableFitsT<GSubGPosTable::LangSysTable>(lang_sys_table))) {
    return BL_SUCCESS;
  }

  uint32_t feature_index_count = lang_sys_table->feature_indexes.count();
  uint32_t required_lang_sys_table_size = (GSubGPosTable::LangSysTable::kBaseSize) + feature_index_count * 2u;

  if (lang_sys_table.size < required_lang_sys_table_size) {
    return BL_SUCCESS;
  }

  uint32_t feature_count = feature_list_offsets->count();
  if (feature_list_offsets.size < 2u + feature_count * 2u) {
    return BL_SUCCESS;
  }

  uint32_t lookup_count = ot_face_impl->layout.by_kind(lookup_kind).lookup_count;

  uint32_t* plan_bits;
  BL_PROPAGATE(bl_bit_array_replace_op(plan, lookup_count, &plan_bits));

  if (settings._d.sso())
    populate_gsub_gpoos_lookup_bits<true>(lang_sys_table, feature_list_offsets, feature_index_count, feature_count, lookup_count, settings, plan_bits);
  else
    populate_gsub_gpoos_lookup_bits<false>(lang_sys_table, feature_list_offsets, feature_index_count, feature_count, lookup_count, settings, plan_bits);

  return BL_SUCCESS;
}

BLResult calculate_gsub_plan(const OTFaceImpl* ot_face_impl, const BLFontFeatureSettings& settings, BLBitArrayCore* plan) noexcept {
  return calculateGSubGPosPlan(ot_face_impl, settings, LookupKind::kGSUB, plan);
}

BLResult calculate_gpos_plan(const OTFaceImpl* ot_face_impl, const BLFontFeatureSettings& settings, BLBitArrayCore* plan) noexcept {
  return calculateGSubGPosPlan(ot_face_impl, settings, LookupKind::kGPOS, plan);
}

// bl::OpenType::LayoutImpl - Init
// ===============================

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  if (tables.gdef) {
    BL_PROPAGATE(initGDef(ot_face_impl, tables.gdef));
  }

  if (tables.gsub) {
    BL_PROPAGATE(initGSubGPos(ot_face_impl, tables.gsub, LookupKind::kGSUB));
  }

  if (tables.gpos) {
    BL_PROPAGATE(initGSubGPos(ot_face_impl, tables.gpos, LookupKind::kGPOS));
  }

  BL_PROPAGATE(ot_face_impl->layout.allocate_lookup_status_bits());

  // Some fonts have both 'GPOS' and 'kern' tables, but 'kern' feature is not provided by GPOS. The practice
  // is to use both GPOS and kern tables in this case, basically breaking the rule of not using legacy tables
  // when GSUB/GPOS are provided. We use `OTFaceFlags::kGPosKernAvailable` flag to decide which table to use.
  if (bl_test_flag(ot_face_impl->ot_flags, OTFaceFlags::kGPosLookupList)) {
    if (ot_face_impl->feature_tag_set.has_known_tag(FontTagData::FeatureId::kKERN)) {
      ot_face_impl->ot_flags |= OTFaceFlags::kGPosKernAvailable;
    }
  }

  return BL_SUCCESS;
}

} // {LayoutImpl}
} // {bl::OpenType}

BL_DIAGNOSTIC_POP
