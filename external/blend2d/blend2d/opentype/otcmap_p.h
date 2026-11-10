// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTCMAP_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTCMAP_P_H_INCLUDED

#include <blend2d/opentype/otdefs_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

//! OpenType 'cmap' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/cmap
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
//!
//! Some names inside this table do not match 1:1 specifications of Apple and MS as they diverge as well. In general
//! the naming was normalized to be consistent in the following:
//!   - `first` - First character or glyph included in the set.
//!   - `last`  - Last character or glyph included in the set.
//!   - `end`   - First character or glyph excluded from the set.
//!   - `count` - Count of something, specifies a range of [first, first + count).
struct CMapTable {
  // Header and one encoding record (just to read the header).
  enum : uint32_t { kBaseSize = 4 + 8 };

  struct Encoding {
    UInt16 platform_id;
    UInt16 encoding_id;
    Offset32 offset;
  };

  struct Group {
    UInt32 first;
    UInt32 last;
    UInt32 glyph_id;
  };

  struct Format0 {
    enum : uint32_t { kBaseSize = 262 };

    UInt16 format;
    UInt16 length;
    UInt16 language;
    UInt8 glyph_id_array[256];
  };

  struct Format2 {
    enum : uint32_t { kBaseSize = 518 };

    struct SubHeader {
      UInt16 first_code;
      UInt16 entry_count;
      Int16 id_delta;
      UInt16 id_range_offset;
    };

    UInt16 format;
    UInt16 length;
    UInt16 language;
    UInt16 glyph_index_array[256];
    /*
    SubHeader sub_header_array[num_sub];
    UInt16 glyph_id_array[];
    */

    BL_INLINE const SubHeader* sub_header_array() const noexcept { return PtrOps::offset<const SubHeader>(this, sizeof(Format2)); }
    BL_INLINE const UInt16* glyph_id_array(size_t n_sub) const noexcept { return PtrOps::offset<const UInt16>(this, sizeof(Format2) + n_sub * sizeof(SubHeader)); }
  };

  struct Format4 {
    enum : uint32_t { kBaseSize = 24 };

    UInt16 format;
    UInt16 length;
    UInt16 mac_language_code;
    UInt16 numSegX2;
    UInt16 search_range;
    UInt16 entry_selector;
    UInt16 range_shift;
    /*
    UInt16 last_char_array[num_segs];
    UInt16 pad;
    UInt16 first_char_array[num_segs];
    Int16 id_delta_array[num_segs];
    UInt16 id_offset_array[num_segs];
    UInt16 glyph_id_array[];
    */

    BL_INLINE const UInt16* last_char_array() const noexcept { return PtrOps::offset<const UInt16>(this, sizeof(*this)); }
    BL_INLINE const UInt16* first_char_array(size_t num_seg) const noexcept { return PtrOps::offset<const UInt16>(this, sizeof(Format4) + 2u + num_seg * 2u); }
    BL_INLINE const UInt16* id_delta_array(size_t num_seg) const noexcept { return PtrOps::offset<const UInt16>(this, sizeof(Format4) + 2u + num_seg * 4u); }
    BL_INLINE const UInt16* id_offset_array(size_t num_seg) const noexcept { return PtrOps::offset<const UInt16>(this, sizeof(Format4) + 2u + num_seg * 6u); }
    BL_INLINE const UInt16* glyph_id_array(size_t num_seg) const noexcept { return PtrOps::offset<const UInt16>(this, sizeof(Format4) + 2u + num_seg * 8u); }
  };

  struct Format6 {
    enum : uint32_t { kBaseSize = 12 };

    UInt16 format;
    UInt16 length;
    UInt16 language;
    UInt16 first;
    UInt16 count;
    /*
    UInt16 glyph_id_array[count];
    */

    BL_INLINE const UInt16* glyph_id_array() const noexcept { return PtrOps::offset<const UInt16>(this, sizeof(Format6)); }
  };

  //! This format is dead and it's not supported by Blend2D [only defined for reference].
  struct Format8 {
    enum : uint32_t { kBaseSize = 16 + 8192 };

    UInt16 format;
    UInt16 reserved;
    UInt32 length;
    UInt32 language;
    UInt8 is32[8192];
    Array32<Group> groups;
  };

  struct Format10 {
    enum : uint32_t { kBaseSize = 20 };

    UInt16 format;
    UInt16 reserved;
    UInt32 length;
    UInt32 language;
    UInt32 first;
    Array32<UInt16> glyph_ids;
  };

  struct Format12_13 {
    enum : uint32_t { kBaseSize = 16 };

    UInt16 format;
    UInt16 reserved;
    UInt32 length;
    UInt32 language;
    Array32<Group> groups;
  };

  struct Format14 {
    enum : uint32_t { kBaseSize = 10 };

    struct VarSelector {
      UInt24 var_selector;
      Offset32 defaultUVSOffset;
      Offset32 nonDefaultUVSOffset;
    };

    struct UnicodeRange {
      UInt24 start_unicode_value;
      UInt8 additional_count;
    };

    struct UVSMapping {
      UInt24 unicode_value;
      UInt16 glyph_id;
    };

    struct DefaultUVS {
      Array32<UnicodeRange> ranges;
    };

    struct NonDefaultUVS {
      Array32<UVSMapping> mappings;
    };

    UInt16 format;
    UInt32 length;
    Array32<VarSelector> var_selectors;
  };

  UInt16 version;
  Array16<Encoding> encodings;
};

struct CMapEncoding {
  //! Offset to get the sub-table of this encoding.
  uint32_t offset;
  //! Count of entries in that sub-table (possibly corrected).
  uint32_t entry_count;

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

//! Character to glyph mapping data for making it easier to use `CMapTable`.
struct CMapData {
  //! CMap table.
  RawTable cmap_table;
  //! CMap encoding [selected].
  CMapEncoding encoding;

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

namespace CMapImpl {

//! Validates a CMapTable::Encoding subtable of any format at `sub_table_offset`. On success a valid `CMapEncoding`
//! is written to `encoding_out`, otherwise an error is returned and `encoding_out` is kept unchanged.
BL_HIDDEN BLResult validate_sub_table(RawTable cmap_table, uint32_t sub_table_offset, uint32_t& format_out, CMapEncoding& encoding_out) noexcept;

//! Populates character coverage of the given font face into `out` bit-set.
BL_HIDDEN BLResult populate_character_coverage(const OTFaceImpl* ot_face_impl, BLBitSet* out) noexcept;

//! Tries to find the best encoding in the provided 'cmap' and store this information into the given `ot_face_impl` instance.
//! The function will return `BL_SUCCESS` even if there is no encoding to be used, however, in such case the character
//! to glyph mapping feature will not be available to the users of this font face.
BL_HIDDEN BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept;

} // {CMapImpl}
} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTCMAP_P_H_INCLUDED
