// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTNAME_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTNAME_P_H_INCLUDED

#include <blend2d/opentype/otdefs_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

//! OpenType 'name' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/name
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6name.html
struct NameTable {
  enum : uint32_t { kBaseSize = 6 };

  struct NameRecord {
    UInt16 platform_id;
    UInt16 specific_id;
    UInt16 language_id;
    UInt16 name_id;
    UInt16 length;
    Offset16 offset;
  };

  struct LangTagRecord {
    UInt16 length;
    Offset16 offset;
  };

  UInt16 format;
  UInt16 record_count;
  Offset16 string_offset;
  /*
  NameRecord name_records[count];
  UInt16 lang_tag_count;
  LangTagRecord lang_tag_records[lang_tag_count];
  */

  BL_INLINE bool has_lang_tags() const noexcept { return format() >= 1; }

  //! The name records where count is the number of records.
  BL_INLINE const NameRecord* name_records() const noexcept {
    return PtrOps::offset<const NameRecord>(this, 6);
  }

  BL_INLINE uint16_t lang_tag_count(size_t record_count_) const noexcept {
    return PtrOps::offset<const UInt16>(this, 6 + record_count_ * sizeof(NameRecord))->value();
  }

  BL_INLINE const LangTagRecord* lang_tag_records(size_t record_count_) const noexcept {
    return PtrOps::offset<const LangTagRecord>(this, 6 + record_count_ * sizeof(NameRecord) + 2);
  }
};

namespace NameImpl {
BL_HIDDEN BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept;
} // {NameImpl}

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTNAME_P_H_INCLUDED
