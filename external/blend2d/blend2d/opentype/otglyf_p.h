// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTGLYF_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTGLYF_P_H_INCLUDED

#include <blend2d/core/font_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/opentype/otdefs_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

//! OpenType 'loca' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/loca
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6loca.html
struct LocaTable {
  // Minimum size would be 2 records (4 bytes) if the font has only 1 glyph and uses 16-bit LOCA.
  enum : uint32_t { kBaseSize = 4 };

  /*
  union {
    Offset16 offset_array16[...];
    Offset32 offset_array32[...];
  };
  */

  BL_INLINE const Offset16* offset_array16() const noexcept { return PtrOps::offset<const Offset16>(this, 0); }
  BL_INLINE const Offset32* offset_array32() const noexcept { return PtrOps::offset<const Offset32>(this, 0); }
};

//! OpenType 'glyf' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/glyf
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6glyf.html
struct GlyfTable {
  enum : uint32_t { kBaseSize = 10 };

  struct Simple {
    enum Flags : uint8_t {
      kOnCurvePoint             = 0x01u,
      kXIsByte                  = 0x02u,
      kYIsByte                  = 0x04u,
      kRepeatFlag               = 0x08u,
      kXIsSameOrXByteIsPositive = 0x10u,
      kYIsSameOrYByteIsPositive = 0x20u,

      // We internally only use flags within this mask.
      kImportantFlagsMask       = 0x3Fu
    };

    /*
    UInt16 end_pts_of_contours[number_of_contours];
    UInt16 instruction_length;
    UInt8 instructions[instruction_length];
    UInt8 flags[...];
    UInt8/UInt16 x_coordinates[...];
    UInt8/UInt16 y_coordinates[...];
    */
  };

  struct Compound {
    enum Flags : uint16_t {
      kArgsAreWords             = 0x0001u,
      kArgsAreXYValues          = 0x0002u,
      kRoundXYToGrid            = 0x0004u,
      kWeHaveScale              = 0x0008u,
      kMoreComponents           = 0x0020u,
      kWeHaveScaleXY            = 0x0040u,
      kWeHave2x2                = 0x0080u,
      kWeHaveInstructions       = 0x0100u,
      kUseMyMetrics             = 0x0200u,
      kOverlappedCompound       = 0x0400u,
      kScaledComponentOffset    = 0x0800u,
      kUnscaledComponentOffset  = 0x1000u,
      kAnyCompoundScale         = kWeHaveScale | kWeHaveScaleXY | kWeHave2x2,
      kAnyCompoundOffset        = kScaledComponentOffset | kUnscaledComponentOffset
    };

    UInt16 flags;
    UInt16 glyph_id;
    /*
    Var arguments[...];
    Var transformations[...];
    */
  };

  struct GlyphData {
    Int16 number_of_contours;
    FWord x_min;
    FWord y_min;
    FWord x_max;
    FWord y_max;

    const Simple* simple() const noexcept { return PtrOps::offset<const Simple>(this, sizeof(GlyphData)); }
    const Compound* compound() const noexcept { return PtrOps::offset<const Compound>(this, sizeof(GlyphData)); }
  };

  /*
  GlyphData glyph_data[...] // Indexed by LOCA.
  */
};

struct GlyfData {
  //! Content of 'glyf' table.
  RawTable glyf_table;
  //! Content of 'loca' table.
  RawTable loca_table;
};

namespace {

// Used by get_glyph_outline() implementation.
struct CompoundEntry {
  enum : uint32_t { kMaxLevel = 16 };

  const uint8_t* gPtr;
  size_t remaining_size;
  uint32_t compound_flags;
  BLMatrix2D transform;
};

} // {anonymous}

namespace GlyfImpl {

BL_HIDDEN extern const LookupTable<uint32_t, ((GlyfTable::Simple::kImportantFlagsMask + 1) >> 1)> vertex_size_table;

#if defined(BL_BUILD_OPT_SSE4_2)
BL_HIDDEN BLResult BL_CDECL get_glyph_outlines_sse4_2(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  BLPath* out,
  size_t* contour_count_out,
  ScopedBuffer* tmp_buffer) noexcept;
#endif // BL_BUILD_OPT_SSE4_2

#if defined(BL_BUILD_OPT_AVX2)
BL_HIDDEN BLResult BL_CDECL get_glyph_outlines_avx2(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  BLPath* out,
  size_t* contour_count_out,
  ScopedBuffer* tmp_buffer) noexcept;
#endif // BL_BUILD_OPT_AVX2

#if BL_TARGET_ARCH_ARM >= 64 && defined(BL_BUILD_OPT_ASIMD)
BL_HIDDEN BLResult BL_CDECL get_glyph_outlines_asimd(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  BLPath* out,
  size_t* contour_count_out,
  ScopedBuffer* tmp_buffer) noexcept;
#endif // BL_BUILD_OPT_ASIMD

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept;

} // {GlyfImpl}
} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTGLYF_P_H_INCLUDED
