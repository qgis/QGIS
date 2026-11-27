// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTCORE_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTCORE_P_H_INCLUDED

#include <blend2d/opentype/otdefs_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

//! OpenType 'SFNT' header.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/font-file
struct SFNTHeader {
  enum : uint32_t { kBaseSize = 12 };

  enum VersionTag : uint32_t {
    kVersionTagOpenType   = BL_MAKE_TAG('O', 'T', 'T', 'O'),
    kVersionTagTrueTypeA  = BL_MAKE_TAG( 0,   1 ,  0 ,  0 ),
    kVersionTagTrueTypeB  = BL_MAKE_TAG('t', 'r', 'u', 'e'),
    kVersionTagType1      = BL_MAKE_TAG('t', 'y', 'p', '1')
  };

  struct TableRecord {
    UInt32 tag;
    CheckSum check_sum;
    UInt32 offset;
    UInt32 length;
  };

  UInt32 version_tag;
  UInt16 num_tables;
  UInt16 search_range;
  UInt16 entry_selector;
  UInt16 range_shift;

  BL_INLINE const TableRecord* table_records() const noexcept { return PtrOps::offset<const TableRecord>(this, sizeof(SFNTHeader)); }
};

//! OpenType 'TTCF' header.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/font-file
struct TTCFHeader {
  enum : uint32_t { kBaseSize = 12 };
  enum : uint32_t { kMaxFonts = 65536 };

  BL_INLINE size_t calc_size(uint32_t num_fonts) const noexcept {
    uint32_t header_size = uint32_t(sizeof(TTCFHeader));

    if (num_fonts > kMaxFonts)
      return 0;

    if (version() >= 0x00020000u)
      header_size += 12;

    return header_size + num_fonts * 4;
  }

  // Version 1.
  UInt32 ttc_tag;
  F16x16 version;
  Array32<UInt32> fonts;

  /*
  // Version 2.
  UInt32 dsig_tag;
  UInt32 dsig_length;
  UInt32 dsig_offset;
  */
};

//! OpenType 'head' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/head
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6head.html
struct HeadTable {
  enum : uint32_t { kBaseSize = 54 };

  enum : uint32_t {
    kCheckSumAdjustment      = BL_MAKE_TAG(0xB1, 0xB0, 0xAF, 0xBA),
    kMagicNumber             = BL_MAKE_TAG(0x5F, 0x0F, 0x3C, 0xF5)
  };

  enum Flags : uint16_t {
    kFlagBaselineYEquals0    = 0x0001u,
    kFlagLSBPointXEquals0    = 0x0002u,
    kFlagInstDependOnPtSize  = 0x0004u,
    kFlagForcePPEMToInteger  = 0x0008u,
    kFlagInstMayAlterAW      = 0x0010u,
    kFlagLossLessData        = 0x0800u,
    kFlagConvertedFont       = 0x1000u,
    kFlagClearTypeOptimized  = 0x2000u,
    kFlagLastResortFont      = 0x4000u
  };

  enum MacStyle : uint16_t {
    kMacStyleBold            = 0x0001u,
    kMacStyleItalic          = 0x0002u,
    kMacStyleUnderline       = 0x0004u,
    kMacStyleOutline         = 0x0008u,
    kMacStyleShadow          = 0x0010u,
    kMacStyleCondensed       = 0x0020u,
    kMacStyleExtended        = 0x0040u,
    kMacStyleReservedBits    = 0xFF70u
  };

  enum IndexToLocFormat : uint16_t {
    kIndexToLocUInt16        = 0,
    kIndexToLocUInt32        = 1
  };

  F16x16 version;
  F16x16 revision;

  UInt32 check_sum_adjustment;
  UInt32 magic_number;
  UInt16 flags;
  UInt16 units_per_em;

  DateTime created;
  DateTime modified;

  Int16 x_min;
  Int16 y_min;
  Int16 x_max;
  Int16 y_max;

  UInt16 mac_style;
  UInt16 lowestRecPPEM;

  Int16 font_direction_hint;
  UInt16 index_to_loc_format;
  UInt16 glyph_data_format;
};

//! OpenType 'maxp' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/maxp
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6maxp.html
struct MaxPTable {
  enum : uint32_t { kBaseSize = 6 };

  // V0.5 - Must be used with CFF Glyphs (OpenType).
  struct V0_5 {
    F16x16 version;
    UInt16 glyph_count;
  };

  // V1.0 - Must be used with TT Glyphs (TrueType).
  struct V1_0 : public V0_5 {
    UInt16 max_points;
    UInt16 max_contours;
    UInt16 max_component_points;
    UInt16 max_component_contours;
    UInt16 max_zones;
    UInt16 max_twilight_points;
    UInt16 max_storage;
    UInt16 max_function_defs;
    UInt16 max_instruction_defs;
    UInt16 max_stack_elements;
    UInt16 max_size_of_instructions;
    UInt16 max_component_elements;
    UInt16 max_component_depth;
  };

  V0_5 header;

  BL_INLINE const V0_5* v0_5() const noexcept { return PtrOps::offset<const V0_5>(this, 0); }
  BL_INLINE const V1_0* v1_0() const noexcept { return PtrOps::offset<const V1_0>(this, 0); }
};

//! OpenType 'OS/2' table.
//!
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/os2
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6OS2.html
struct OS2Table {
  enum : uint32_t { kBaseSize = 68 };

  //! OS/2 selection flags used by `OS2::selection_flags` field.
  enum SelectionFlags : uint32_t {
    kSelectionItalic         = 0x0001u,
    kSelectionUnderscore     = 0x0002u,
    kSelectionNegative       = 0x0004u,
    kSelectionOutlined       = 0x0008u,
    kSelectionStrikeout      = 0x0010u,
    kSelectionBold           = 0x0020u,
    kSelectionRegular        = 0x0040u,
    kSelectionUseTypoMetrics = 0x0080u,
    kSelectionWWS            = 0x0100u,
    kSelectionOblique        = 0x0200u
  };

  struct V0A {
    enum : uint32_t { kBaseSize = 68 };

    UInt16 version;
    Int16 xAverageWidth;
    UInt16 weight_class;
    UInt16 width_class;
    UInt16 embedding_flags;
    Int16 ySubscriptXSize;
    Int16 ySubscriptYSize;
    Int16 ySubscriptXOffset;
    Int16 ySubscriptYOffset;
    Int16 ySuperscriptXSize;
    Int16 ySuperscriptYSize;
    Int16 ySuperscriptXOffset;
    Int16 ySuperscriptYOffset;
    Int16 yStrikeoutSize;
    Int16 yStrikeoutPosition;
    Int16 family_class;
    UInt8 panose[10];
    UInt32 unicode_coverage[4];
    UInt8 vendor_id[4];
    UInt16 selection_flags;
    UInt16 first_char;
    UInt16 last_char;
  };

  struct V0B : public V0A {
    enum : uint32_t { kBaseSize = 78 };

    Int16 typo_ascender;
    Int16 typo_descender;
    Int16 typo_line_gap;
    UInt16 win_ascent;
    UInt16 win_descent;
  };

  struct V1 : public V0B {
    enum : uint32_t { kBaseSize = 86 };

    UInt32 code_page_range[2];
  };

  struct V2 : public V1 {
    enum : uint32_t { kBaseSize = 96 };

    Int16 x_height;
    Int16 cap_height;
    UInt16 default_char;
    UInt16 break_char;
    UInt16 max_context;
  };

  struct V5 : public V2 {
    enum : uint32_t { kBaseSize = 100 };

    UInt16 lower_optical_point_size;
    UInt16 upper_optical_point_size;
  };

  V0A header;

  BL_INLINE const V0A* v0a() const noexcept { return PtrOps::offset<const V0A>(this, 0); }
  BL_INLINE const V0B* v0b() const noexcept { return PtrOps::offset<const V0B>(this, 0); }
  BL_INLINE const V1* v1() const noexcept { return PtrOps::offset<const V1>(this, 0); }
  BL_INLINE const V2* v2() const noexcept { return PtrOps::offset<const V2>(this, 0); }
  BL_INLINE const V5* v5() const noexcept { return PtrOps::offset<const V5>(this, 0); }
};

//! OpenType 'post' table.
//!
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/post
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6Post.html
struct PostTable {
  enum : uint32_t { kBaseSize = 32 };

  F16x16 version;
  F16x16 italic_angle;
  Int16 underline_position;
  Int16 underline_thickness;
  UInt32 is_fixed_pitch;
  UInt32 minMemType42;
  UInt32 maxMemType42;
  UInt32 minMemType1;
  UInt32 maxMemType1;
};


namespace CoreImpl {
BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept;
} // {CoreImpl}

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTCORE_P_H_INCLUDED
