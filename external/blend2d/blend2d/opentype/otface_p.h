// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTFACE_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTFACE_P_H_INCLUDED

#include <blend2d/core/fonttagdataids_p.h>
#include <blend2d/opentype/otcff_p.h>
#include <blend2d/opentype/otcmap_p.h>
#include <blend2d/opentype/otdefs_p.h>
#include <blend2d/opentype/otglyf_p.h>
#include <blend2d/opentype/otkern_p.h>
#include <blend2d/opentype/otlayout_p.h>
#include <blend2d/opentype/otmetrics_p.h>
#include <blend2d/opentype/otname_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

enum class OTFaceFlags : uint32_t {
  // Flags related to 'loca' table
  // -----------------------------

  //! Glyph offsets in 'loca' table use 16-bit offsets [must be 0x2].
  kLocaOffset16        = 0x00000002u,
  //! Glyph offsets in 'loca' table use 32-bit offsets [must be 0x4].
  kLocaOffset32        = 0x00000004u,

  // Flags related to 'kern' table
  // -----------------------------

  kLegacyKernAvailable = 0x00000010u,

  // Flags related to 'GDEF' table
  // -----------------------------

  kGlyphClassDef       = 0x00000100u,
  kAttachList          = 0x00000200u,
  kLitCaretList        = 0x00000400u,
  kMarkAttachClassDef  = 0x00000800u,
  kMarkGlyphSetsDef    = 0x00001000u,
  kItemVarStore        = 0x00002000u,

  // Flags related to 'GSUB' table
  // -----------------------------

  kGSubScriptList      = 0x00010000u,
  kGSubFeatureList     = 0x00020000u,
  kGSubLookupList      = 0x00040000u,
  kGSubFVar            = 0x00080000u,

  // Flags related to 'GPOS' table
  // -----------------------------

  kGPosScriptList      = 0x00100000u,
  kGPosFeatureList     = 0x00200000u,
  kGPosLookupList      = 0x00400000u,
  kGPosFVar            = 0x00800000u,
  kGPosKernAvailable   = 0x01000000u
};

BL_DEFINE_ENUM_FLAGS(OTFaceFlags)

//! OpenType & TrueType font face.
//!
//! This class provides extra data required by TrueType / OpenType implementation. It's currently the only
//! implementation of \ref BLFontFaceImpl available in Blend2D and there will probably not be any other
//! implementation as OpenType provides enough features required to render text in general.
struct OTFaceImpl : public BLFontFacePrivateImpl {
  //! OpenType flags, see OTFlags.
  OTFaceFlags ot_flags;

  //! Character mapping format (stored here so we won't misalign `CMapData`.
  uint8_t cmap_format;

  //! Reserved for future use.
  uint8_t reserved_open_type[3];

  //! Character to glyph mapping data.
  CMapData cmap;
  //! Metrics data.
  MetricsData metrics;

  //! Legacy kerning data - 'kern' table and related data.
  KernData kern;
  //! OpenType layout data - 'GDEF', 'GSUB', and 'GPOS' tables.
  LayoutData layout;

  union {
    //! OpenType font data [Compact Font Format] [CFF or CFF2].
    CFFData cff;
    //! TrueType font data [glyf/loca].
    GlyfData glyf;
  };
  //! Array of LSubR indexes used by CID fonts (CFF/CFF2).
  BLArray<CFFData::IndexData> cff_fd_subr_indexes;

  BL_INLINE uint32_t loca_offset_size() const noexcept {
    return uint32_t(ot_flags & (OTFaceFlags::kLocaOffset16 | OTFaceFlags::kLocaOffset32));
  }
};

//! OpenType tables that are used during the initialization of \ref OTFaceImpl.
union OTFaceTables {
  enum : uint32_t { kTableCount = 19 };

  BLFontTable tables[kTableCount];

  struct {
    BLFontTable head;
    BLFontTable maxp;
    BLFontTable os_2;
    BLFontTable post;
    BLFontTable name;
    BLFontTable cmap;

    BLFontTable hhea;
    BLFontTable hmtx;
    BLFontTable vhea;
    BLFontTable vmtx;

    BLFontTable kern;

    BLFontTable base;
    BLFontTable gdef;
    BLFontTable gpos;
    BLFontTable gsub;

    BLFontTable glyf;
    BLFontTable loca;

    BLFontTable cff;
    BLFontTable cff2;
  };

  BL_INLINE void init(OTFaceImpl* ot_face_impl, const BLFontData* font_data) noexcept {
    static const BLTag tags[kTableCount] = {
      BL_MAKE_TAG('h', 'e', 'a', 'd'),
      BL_MAKE_TAG('m', 'a', 'x', 'p'),
      BL_MAKE_TAG('O', 'S', '/', '2'),
      BL_MAKE_TAG('p', 'o', 's', 't'),
      BL_MAKE_TAG('n', 'a', 'm', 'e'),
      BL_MAKE_TAG('c', 'm', 'a', 'p'),

      BL_MAKE_TAG('h', 'h', 'e', 'a'),
      BL_MAKE_TAG('h', 'm', 't', 'x'),
      BL_MAKE_TAG('v', 'h', 'e', 'a'),
      BL_MAKE_TAG('v', 'm', 't', 'x'),

      BL_MAKE_TAG('k', 'e', 'r', 'n'),

      BL_MAKE_TAG('B', 'A', 'S', 'E'),
      BL_MAKE_TAG('G', 'D', 'E', 'F'),
      BL_MAKE_TAG('G', 'P', 'O', 'S'),
      BL_MAKE_TAG('G', 'S', 'U', 'B'),

      BL_MAKE_TAG('g', 'l', 'y', 'f'),
      BL_MAKE_TAG('l', 'o', 'c', 'a'),

      BL_MAKE_TAG('C', 'F', 'F', ' '),
      BL_MAKE_TAG('C', 'F', 'F', '2')
    };

    font_data->get_tables(ot_face_impl->face_info.face_index, tables, tags, kTableCount);
  }
};

BL_HIDDEN BLResult create_open_type_face(BLFontFaceCore* self, const BLFontData* font_data, uint32_t face_index) noexcept;

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTFACE_P_H_INCLUDED
