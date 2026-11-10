// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTLAYOUTTABLES_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTLAYOUTTABLES_P_H_INCLUDED

#include <blend2d/opentype/otcore_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

enum class LookupFlags : uint32_t {
  //! Relates only to the correct processing of the cursive attachment lookup type (GPOS lookup type 3).
  kRightToLeft = 0x0001u,
  //! Skips over base glyphs.
  kIgnoreBaseGlyphs = 0x0002u,
  //! Skips over ligatures.
  kIgnoreLigatures = 0x0004u,
  //! Skips over all combining marks.
  kIgnoreMarks = 0x0008u,
  //! Indicates that the lookup table structure is followed by a `mark_filtering_set` field.
  kUseMarkFilteringSet = 0x0010u,
  //! Must be zero.
  kReserved = 0x00E0u,
  //! If non-zero, skips over all marks of attachment type different from specified.
  kMarkAttachmentType = 0xFF00u
};

BL_DEFINE_ENUM_FLAGS(LookupFlags)

//! OpenType coverage table.
struct CoverageTable {
  enum : uint32_t { kBaseSize = 4 };

  struct Range {
    enum : uint32_t { kBaseSize = 6 };

    UInt16 first_glyph;
    UInt16 last_glyph;
    UInt16 start_coverage_index;
  };

  struct Format1 {
    enum : uint32_t { kBaseSize = 4 };

    UInt16 format;
    Array16<UInt16> glyphs;
  };

  struct Format2 {
    enum : uint32_t { kBaseSize = 4 };

    UInt16 format;
    Array16<Range> ranges;
  };

  UInt16 format;
  Array16<void> array;

  BL_INLINE_NODEBUG const Format1* format1() const noexcept { return PtrOps::offset<const Format1>(this, 0); }
  BL_INLINE_NODEBUG const Format2* format2() const noexcept { return PtrOps::offset<const Format2>(this, 0); }

  // Format 1 has 2 byte entries, format 2 has 6 byte entries - other formats don't exist.
  static BL_INLINE_NODEBUG uint32_t entry_size_by_format(uint32_t format) noexcept { return format * 4u - 2u; }
};

class CoverageTableIterator {
public:
  typedef CoverageTable::Range Range;

  const void* _array;
  size_t _size;

  BL_INLINE_IF_NOT_DEBUG uint32_t init(RawTable table) noexcept {
    BL_ASSERT_VALIDATED(table.fits(CoverageTable::kBaseSize));

    uint32_t format = table.data_as<CoverageTable>()->format();
    BL_ASSERT_VALIDATED(format == 1 || format == 2);

    uint32_t size = table.data_as<CoverageTable>()->array.count();
    BL_ASSERT_VALIDATED(table.fits(CoverageTable::kBaseSize + size * CoverageTable::entry_size_by_format(format)));

    _array = table.data_as<CoverageTable>()->array.array();
    _size = size;
    return format;
  }

  template<typename T>
  BL_INLINE_NODEBUG const T& at(size_t index) const noexcept {
    return static_cast<const T*>(_array)[index];
  }

  template<uint32_t Format>
  BL_INLINE uint32_t min_glyph_id() const noexcept {
    if (Format == 1)
      return at<UInt16>(0).value();
    else
      return at<Range>(0).first_glyph();
  }

  template<uint32_t Format>
  BL_INLINE uint32_t max_glyph_id() const noexcept {
    if (Format == 1)
      return at<UInt16>(_size - 1).value();
    else
      return at<Range>(_size - 1).last_glyph();
  }

  template<uint32_t Format>
  BL_INLINE_NODEBUG GlyphRange glyph_range() const noexcept {
    return GlyphRange{min_glyph_id<Format>(), max_glyph_id<Format>()};
  }

  // Like `glyph_range()`, but used when the coverage table format cannot be templatized.
  BL_INLINE GlyphRange glyph_range_with_format(uint32_t format) const noexcept {
    if (format == 1)
      return GlyphRange{min_glyph_id<1>(), max_glyph_id<1>()};
    else
      return GlyphRange{min_glyph_id<2>(), max_glyph_id<2>()};
  }

  template<uint32_t Format>
  BL_INLINE_IF_NOT_DEBUG bool find(BLGlyphId glyph_id, uint32_t& coverage_index) const noexcept {
    if (Format == 1) {
      const UInt16* base = static_cast<const UInt16*>(_array);
      size_t size = _size;

      while (size_t half = size / 2u) {
        const UInt16* middle = base + half;
        size -= half;
        if (glyph_id >= middle->value())
          base = middle;
      }

      coverage_index = uint32_t(size_t(base - static_cast<const UInt16*>(_array)));
      return base->value() == glyph_id;
    }
    else {
      const Range* base = static_cast<const Range*>(_array);
      size_t size = _size;

      while (size_t half = size / 2u) {
        const Range* middle = base + half;
        size -= half;
        if (glyph_id >= middle->first_glyph())
          base = middle;
      }

      coverage_index = uint32_t(base->start_coverage_index()) + glyph_id - base->first_glyph();
      uint32_t first_glyph = base->first_glyph();
      uint32_t last_glyph = base->last_glyph();
      return glyph_id >= first_glyph && glyph_id <= last_glyph;
    }
  }

  // Like `find()`, but used when the coverage table format cannot be templatized.
  BL_INLINE bool find_with_format(uint32_t format, BLGlyphId glyph_id, uint32_t& coverage_index) const noexcept {
    if (format == 1)
      return find<1>(glyph_id, coverage_index);
    else
      return find<2>(glyph_id, coverage_index);
  }
};

//! OpenType class-definition table.
struct ClassDefTable {
  // Let's assume that Format2 table would contain at least one record.
  enum : uint32_t { kBaseSize = 6 };

  struct Range {
    UInt16 first_glyph;
    UInt16 last_glyph;
    UInt16 class_value;
  };

  struct Format1 {
    enum : uint32_t { kBaseSize = 6 };

    UInt16 format;
    UInt16 first_glyph;
    Array16<UInt16> class_values;
  };

  struct Format2 {
    enum : uint32_t { kBaseSize = 4 };

    UInt16 format;
    Array16<Range> ranges;
  };

  UInt16 format;

  BL_INLINE_NODEBUG const Format1* format1() const noexcept { return PtrOps::offset<const Format1>(this, 0); }
  BL_INLINE_NODEBUG const Format2* format2() const noexcept { return PtrOps::offset<const Format2>(this, 0); }
};

class ClassDefTableIterator {
public:
  typedef ClassDefTable::Range Range;

  const void* _array;
  uint32_t _size;
  uint32_t _first_glyph;

  BL_INLINE_IF_NOT_DEBUG uint32_t init(RawTable table) noexcept {
    const void* array = nullptr;
    uint32_t size = 0;
    uint32_t format = 0;
    uint32_t first_glyph = 0;

    if (BL_LIKELY(table.size >= ClassDefTable::kBaseSize)) {
      uint32_t required_table_size = 0;
      format = table.data_as<ClassDefTable>()->format();

      switch (format) {
        case 1: {
          const ClassDefTable::Format1* fmt1 = table.data_as<ClassDefTable::Format1>();

          size = fmt1->class_values.count();
          array = fmt1->class_values.array();
          first_glyph = fmt1->first_glyph();
          required_table_size = uint32_t(sizeof(ClassDefTable::Format1)) + size * 2u;
          break;
        }

        case 2: {
          const ClassDefTable::Format2* fmt2 = table.data_as<ClassDefTable::Format2>();

          size = fmt2->ranges.count();
          array = fmt2->ranges.array();
          // Minimum table size that we check is 6, so we are still fine here...
          first_glyph = fmt2->ranges.array()[0].first_glyph();
          required_table_size = uint32_t(sizeof(ClassDefTable::Format2)) + size * uint32_t(sizeof(ClassDefTable::Range));
          break;
        }

        default:
          format = 0;
          break;
      }

      if (!size || required_table_size > table.size)
        format = 0;
    }

    _array = array;
    _size = size;
    _first_glyph = first_glyph;

    return format;
  }

  template<typename T>
  BL_INLINE_NODEBUG const T& at(size_t index) const noexcept { return static_cast<const T*>(_array)[index]; }

  template<uint32_t Format>
  BL_INLINE_NODEBUG uint32_t min_glyph_id() const noexcept {
    return _first_glyph;
  }

  template<uint32_t Format>
  BL_INLINE uint32_t max_glyph_id() const noexcept {
    if (Format == 1)
      return _first_glyph + uint32_t(_size) - 1;
    else
      return at<Range>(_size - 1).last_glyph();
  }

  template<uint32_t Format>
  BL_INLINE_IF_NOT_DEBUG uint32_t class_of_glyph(BLGlyphId glyph_id) const noexcept {
    if (Format == 1) {
      uint32_t index = glyph_id - _first_glyph;
      uint32_t fixed_index = bl_min(index, _size - 1);

      uint32_t class_value = at<UInt16>(fixed_index).value();
      if (index >= _size)
        class_value = 0;
      return class_value;
    }
    else {
      const Range* base = static_cast<const Range*>(_array);
      uint32_t size = _size;

      while (uint32_t half = size / 2u) {
        const Range* middle = base + half;
        size -= half;
        if (glyph_id >= middle->first_glyph())
          base = middle;
      }

      uint32_t class_value = base->class_value();
      if (glyph_id < base->first_glyph() || glyph_id > base->last_glyph())
        class_value = 0;
      return class_value;
    }
  }

  template<uint32_t Format>
  BL_INLINE uint32_t match_glyph_class(BLGlyphId glyph_id, uint32_t class_id) const noexcept {
    if (Format == 1) {
      uint32_t index = glyph_id - _first_glyph;
      return index == class_id && class_id < _size;
    }
    else {
      return class_of_glyph<2>(glyph_id) == class_id;
    }
  }
};

//! OpenType condition table.
struct ConditionTable {
  enum : uint32_t { kBaseSize = 2 };

  struct Format1 {
    enum : uint32_t { kBaseSize = 8 };

    UInt16 format;
    UInt16 axis_index;
    F2x14 filter_range_min_value;
    F2x14 filter_range_max_value;
  };

  UInt16 format;

  BL_INLINE_NODEBUG const Format1* format1() const noexcept { return PtrOps::offset<const Format1>(this, 0); }
};

//! OpenType 'GDEF' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/gdef
struct GDefTable {
  enum : uint32_t { kBaseSize = 12 };

  struct HeaderV1_0 {
    enum : uint32_t { kBaseSize = 12 };

    F16x16 version;
    Offset16 glyph_class_def_offset;
    Offset16 attach_list_offset;
    Offset16 lig_caret_list_offset;
    Offset16 mark_attach_class_def_offset;
  };

  struct HeaderV1_2 : public HeaderV1_0 {
    enum : uint32_t { kBaseSize = 14 };

    UInt16 mark_glyph_sets_def_offset;
  };

  struct HeaderV1_3 : public HeaderV1_2 {
    enum : uint32_t { kBaseSize = 18 };

    UInt32 item_var_store_offset;
  };

  HeaderV1_0 header;

  BL_INLINE_NODEBUG const HeaderV1_0* v1_0() const noexcept { return &header; }
  BL_INLINE_NODEBUG const HeaderV1_2* v1_2() const noexcept { return PtrOps::offset<const HeaderV1_2>(this, 0); }
  BL_INLINE_NODEBUG const HeaderV1_3* v1_3() const noexcept { return PtrOps::offset<const HeaderV1_3>(this, 0); }
};

//! Base class for 'GSUB' and 'GPOS' tables.
struct GSubGPosTable {
  enum : uint32_t { kBaseSize = 10 };

  //! No feature required, possibly stored in `LangSysTable::required_feature_index`.
  static inline constexpr uint16_t kFeatureNotRequired = 0xFFFFu;

  //! \name GPOS & GSUB - Core Tables
  //! \{

  struct HeaderV1_0 {
    enum : uint32_t { kBaseSize = 10 };

    F16x16 version;
    Offset16 script_list_offset;
    Offset16 feature_list_offset;
    Offset16 lookup_list_offset;
  };

  struct HeaderV1_1 : public HeaderV1_0 {
    enum : uint32_t { kBaseSize = 14 };

    Offset32 feature_variations_offset;
  };

  typedef TagRef16 LangSysRecord;
  struct LangSysTable {
    enum : uint32_t { kBaseSize = 6 };

    Offset16 lookup_order_offset;
    UInt16 required_feature_index;
    Array16<UInt16> feature_indexes;
  };

  struct ScriptTable {
    enum : uint32_t { kBaseSize = 4 };

    UInt16 lang_sys_default;
    Array16<TagRef16> lang_sys_offsets;
  };

  struct FeatureTable {
    enum : uint32_t { kBaseSize = 4 };

    typedef TagRef16 Record;
    typedef Array16<Record> List;

    Offset16 feature_params_offset;
    Array16<UInt16> lookup_list_indexes;
  };

  struct LookupTable {
    enum : uint32_t { kBaseSize = 6 };

    UInt16 lookup_type;
    UInt16 lookup_flags;
    Array16<Offset16> sub_table_offsets;
    /*
    UInt16 mark_filtering_set;
    */
  };

  //! \]

  //! \name GSUB & GPOS - Lookup Headers
  //! \{

  struct LookupHeader {
    enum : uint32_t { kBaseSize = 2 };

    UInt16 format;
  };

  struct LookupHeaderWithCoverage : public LookupHeader {
    enum : uint32_t { kBaseSize = LookupHeader::kBaseSize + 2 };

    Offset16 coverage_offset;
  };

  struct ExtensionLookup : public LookupHeader {
    enum : uint32_t { kBaseSize = LookupHeader::kBaseSize + 6 };

    UInt16 lookup_type;
    Offset32 offset;
  };

  //! \}

  //! \name GSUB & GPOS - Sequence Context Tables
  //! \{

  struct SequenceLookupRecord {
    enum : uint32_t { kBaseSize = 4 };

    UInt16 sequence_index;
    UInt16 lookup_index;
  };

  typedef Array16<UInt16> SequenceRuleSet;

  struct SequenceRule {
    enum : uint32_t { kBaseSize = 4 };

    UInt16 glyph_count;
    UInt16 lookup_record_count;
    /*
    UInt16 input_sequence[glyph_count - 1];
    SequenceLookupRecord lookup_records[lookup_count];
    */

    BL_INLINE_NODEBUG const UInt16* input_sequence() const noexcept {
      return PtrOps::offset<const UInt16>(this, 4);
    }

    BL_INLINE_NODEBUG const SequenceLookupRecord* lookup_record_array(uint32_t glyph_count_) const noexcept {
      return PtrOps::offset<const SequenceLookupRecord>(this, kBaseSize + glyph_count_ * 2u - 2u);
    }
  };

  struct SequenceContext1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Array16<Offset16> rule_set_offsets;
  };

  struct SequenceContext2 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 4 };

    Offset16 class_def_offset;
    Array16<Offset16> rule_set_offsets;
  };

  struct SequenceContext3 : public LookupHeader {
    enum : uint32_t { kBaseSize = LookupHeader::kBaseSize + 4 };

    UInt16 glyph_count;
    UInt16 lookup_record_count;
    /*
    Offset16 coverage_offset_array[glyph_count];
    SequenceLookupRecord lookup_records[lookup_record_count];
    */

    BL_INLINE_NODEBUG const UInt16* coverage_offset_array() const noexcept { return PtrOps::offset<const UInt16>(this, kBaseSize); }
    BL_INLINE_NODEBUG const SequenceLookupRecord* lookup_record_array(size_t glyph_count_) const noexcept { return PtrOps::offset<const SequenceLookupRecord>(this, kBaseSize + glyph_count_ * 2u); }
  };

  //! \}

  //! \name GSUB & GPOS - Chained Sequence Context Tables
  //! \{

  struct ChainedSequenceRule {
    enum : uint32_t { kBaseSize = 8 };

    UInt16 backtrack_glyph_count;
    /*
    UInt16 backtrack_sequence[backtrack_glyph_count];
    UInt16 input_glyph_count;
    UInt16 input_sequence[input_glyph_count - 1];
    UInt16 lookahead_glyph_count;
    UInt16 lookahead_sequence[lookahead_glyph_count];
    UInt16 lookup_record_count;
    SequenceLookupRecord lookup_records[lookup_record_count];
    */

    BL_INLINE const UInt16* backtrack_sequence() const noexcept { return PtrOps::offset<const UInt16>(this, 2); }
  };

  typedef Array16<UInt16> ChainedSequenceRuleSet;

  struct ChainedSequenceContext1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Array16<Offset16> rule_set_offsets;
  };

  struct ChainedSequenceContext2 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 8 };

    Offset16 backtrack_class_def_offset;
    Offset16 input_class_def_offset;
    Offset16 lookahead_class_def_offset;
    Array16<Offset16> rule_set_offsets;
  };

  struct ChainedSequenceContext3 : public LookupHeader {
    enum : uint32_t { kBaseSize = LookupHeader::kBaseSize + 8 };

    UInt16 backtrack_glyph_count;
    /*
    Offset16 backtrack_coverage_offsets[backtrack_glyph_count];
    UInt16 input_glyph_count;
    Offset16 input_coverage_offsets[input_glyph_count];
    UInt16 lookahead_glyph_count;
    Offset16 lookahead_coverage_offsets[lookahead_glyph_count];
    UInt16 lookup_record_count;
    SequenceLookupRecord lookup_records[subst_count];
    */

    BL_INLINE_NODEBUG const UInt16* backtrack_coverage_offsets() const noexcept { return PtrOps::offset<const UInt16>(this, 4); }
  };

  //! \}

  //! \name Members
  //! \{

  HeaderV1_0 header;

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG const HeaderV1_0* v1_0() const noexcept { return &header; }
  BL_INLINE_NODEBUG const HeaderV1_1* v1_1() const noexcept { return PtrOps::offset<const HeaderV1_1>(this, 0); }

  //! \}
};

//! Glyph Substitution Table 'GSUB'.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/gsub
//!   - https://fontforge.github.io/gposgsub.html
struct GSubTable : public GSubGPosTable {
  enum LookupType : uint8_t {
    kLookupSingle = 1,
    kLookupMultiple = 2,
    kLookupAlternate = 3,
    kLookupLigature = 4,
    kLookupContext = 5,
    kLookupChainedContext = 6,
    kLookupExtension = 7,
    //! Extension - access to lookup tables beyond a 16-bit offset.
    kLookupReverseChainedContext = 8,
    //! Maximum value of LookupType.
    kLookupMaxValue = 8
  };

  // Lookup Type 1 - SingleSubst
  // ---------------------------

  struct SingleSubst1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Int16 delta_glyph_id;
  };

  struct SingleSubst2 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Array16<UInt16> glyphs;
  };

  // Lookup Type 2 - MultipleSubst
  // -----------------------------

  typedef Array16<UInt16> Sequence;

  struct MultipleSubst1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Array16<Offset16> sequence_offsets;
  };

  // Lookup Type 3 - AlternateSubst
  // ------------------------------

  typedef Array16<UInt16> AlternateSet;

  struct AlternateSubst1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Array16<Offset16> alternate_set_offsets;
  };

  // Lookup Type 4 - LigatureSubst
  // -----------------------------

  struct Ligature {
    UInt16 ligature_glyph_id;
    Array16<UInt16> glyphs;
  };

  typedef Array16<UInt16> LigatureSet;

  struct LigatureSubst1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Array16<Offset16> ligature_set_offsets;
  };

  // Lookup Type 5 - ContextSubst
  // ----------------------------

  // Uses SequenceContext[1|2|3]

  // Lookup Type 6 - ChainedContextSubst
  // -----------------------------------

  // Uses ChainedSequenceContext[1|2|3]

  // Lookup Type 7 - Extension
  // -------------------------

  // Use `ExtensionLookup` to handle this lookup type.

  // Lookup Type 8 - ReverseChainedSingleSubst
  // -----------------------------------------

  struct ReverseChainedSingleSubst1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    UInt16 backtrack_glyph_count;
    /*
    Offset16 backtrack_coverage_offsets[backtrack_glyph_count];
    UInt16 lookahead_glyph_count;
    Offset16 lookahead_coverage_offsets[lookahead_glyph_count];
    UInt16 subst_glyph_count;
    UInt16 subst_glyph_array[subst_glyph_count];
    */

    BL_INLINE_NODEBUG const UInt16* backtrack_coverage_offsets() const noexcept { return PtrOps::offset<const UInt16>(this, 6); }
  };
};

//! OpenType 'GPOS' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/gpos
//!   - https://fontforge.github.io/gposgsub.html
struct GPosTable : public GSubGPosTable {
  enum LookupType : uint8_t {
    //! Adjust position of a single glyph.
    kLookupSingle = 1,
    //! Adjust position of a pair of glyphs.
    kLookupPair = 2,
    //! Attach cursive glyphs.
    kLookupCursive = 3,
    //! Attach a combining mark to a base glyph.
    kLookupMarkToBase = 4,
    //! Attach a combining mark to a ligature.
    kLookupMarkToLigature = 5,
    //! Attach a combining mark to another mark.
    kLookupMarkToMark = 6,
    //! Position one or more glyphs in context.
    kLookupContext = 7,
    //! Position one or more glyphs in chained context.
    kLookupChainedContext = 8,
    //! Extension - access to lookup tables beyond a 16-bit offset.
    kLookupExtension = 9,
    //! Maximum value of LookupType.
    kLookupMaxValue = 9
  };

  enum ValueFlags : uint16_t {
    kValueXPlacement = 0x0001u,
    kValueYPlacement = 0x0002u,
    kValueXAdvance = 0x0004u,
    kValueYAdvance = 0x0008u,
    kValueXPlacementDevice = 0x0010u,
    kValueYPlacementDevice = 0x0020u,
    kValueXAdvanceDevice = 0x0040u,
    kValueYAdvanceDevice = 0x0080u,
    kValurReservedFlags = 0xFF00u
  };

  // Anchor Table
  // ------------

  struct Anchor1 {
    enum : uint32_t { kBaseSize = 6 };

    UInt16 anchor_format;
    Int16 x_coordinate;
    Int16 y_coordinate;
  };

  struct Anchor2 {
    enum : uint32_t { kBaseSize = 8 };

    UInt16 anchor_format;
    Int16 x_coordinate;
    Int16 y_coordinate;
    UInt16 anchor_point;
  };

  struct Anchor3 {
    enum : uint32_t { kBaseSize = 10 };

    UInt16 anchor_format;
    Int16 x_coordinate;
    Int16 y_coordinate;
    UInt16 xDeviceOffset;
    UInt16 yDeviceOffset;
  };

  // Mark
  // ----

  struct Mark {
    UInt16 mark_class;
    UInt16 mark_anchor_offset;
  };

  // Lookup Type 1 - Single Adjustment
  // ---------------------------------

  struct SingleAdjustment1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    UInt16 value_format;

    BL_INLINE_NODEBUG const UInt16* value_records() const noexcept { return PtrOps::offset<const UInt16>(this, 6u); }
  };

  struct SingleAdjustment2 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 4 };

    UInt16 value_format;
    UInt16 value_count;

    BL_INLINE_NODEBUG const UInt16* value_records() const noexcept { return PtrOps::offset<const UInt16>(this, 8u); }
  };

  // Lookup Type 2 - Pair Adjustment
  // -------------------------------

  struct PairSet {
    UInt16 pair_value_count;

    BL_INLINE_NODEBUG const UInt16* pair_value_records() const noexcept { return PtrOps::offset<const UInt16>(this, 2); }
  };

  struct PairValueRecord {
    UInt16 second_glyph;

    BL_INLINE_NODEBUG const UInt16* value_records() const noexcept { return PtrOps::offset<const UInt16>(this, 2); }
  };

  struct PairAdjustment1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 6 };

    UInt16 value_format1;
    UInt16 value_format2;
    Array16<UInt16> pair_set_offsets;
  };

  struct PairAdjustment2 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 12 };

    UInt16 value1_format;
    UInt16 value2_format;
    Offset16 classDef1Offset;
    Offset16 classDef2Offset;
    UInt16 class1_count;
    UInt16 class2_count;
    /*
    struct ClassRecord {
      ValueRecord value1;
      ValueRecord value2;
    };
    ClassRecord class_records[class1_count * class2_count];
    */
  };

  // Lookup Type 3 - Cursive Attachment
  // ----------------------------------

  struct EntryExit {
    enum : uint32_t { kBaseSize = 4 };

    Offset16 entry_anchor_offset;
    Offset16 exit_anchor_offset;
  };

  struct CursiveAttachment1 : public LookupHeaderWithCoverage {
    enum : uint32_t { kBaseSize = LookupHeaderWithCoverage::kBaseSize + 2 };

    Array16<EntryExit> entry_exits;
  };

  // Lookup Type 4 - MarkToBase Attachment
  // -------------------------------------

  struct MarkToBaseAttachment1 : public LookupHeader {
    enum : uint32_t { kBaseSize = LookupHeader::kBaseSize + 10 };

    Offset16 mark_coverage_offset;
    Offset16 base_coverage_offset;
    UInt16 mark_class_count;
    Offset16 mark_array_offset;
    Offset16 base_array_offset;
  };

  // Lookup Type 5 - MarkToLigature Attachment
  // -----------------------------------------

  struct MarkToLigatureAttachment1 : public LookupHeader {
    enum : uint32_t { kBaseSize = LookupHeader::kBaseSize + 10 };

    Offset16 mark_coverage_offset;
    Offset16 ligature_coverage_offset;
    UInt16 mark_class_count;
    Offset16 mark_array_offset;
    Offset16 ligature_array_offset;
  };

  // Lookup Type 6 - MarkToMark Attachment
  // -------------------------------------

  struct MarkToMarkAttachment1 : public LookupHeader {
    enum : uint32_t { kBaseSize = LookupHeader::kBaseSize + 10 };

    Offset16 mark1_coverage_offset;
    Offset16 mark2_coverage_offset;
    UInt16 mark_class_count;
    Offset16 mark1_array_offset;
    Offset16 mark2_array_offset;
  };

  // Lookup Type 7 - Context Positioning
  // -----------------------------------

  // Uses SequenceContext[1|2|3]

  // Lookup Type 8 - Chained Contextual Positioning
  // ----------------------------------------------

  // Uses ChainedSequenceContext[1|2|3]

  // Lookup Type 9 - Extension
  // -------------------------

  // Use `ExtensionLookup` to handle this lookup type.
};

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTLAYOUTTABLES_P_H_INCLUDED
