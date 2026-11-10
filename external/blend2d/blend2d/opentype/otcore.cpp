// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/trace_p.h>
#include <blend2d/opentype/otcore_p.h>
#include <blend2d/opentype/otcmap_p.h>
#include <blend2d/opentype/otface_p.h>

namespace bl::OpenType {
namespace CoreImpl {

// bl::OpenType::CoreImpl - Trace
// ==============================

#if defined(BL_TRACE_OT_ALL) || defined(BL_TRACE_OT_CORE)
#define Trace BLDebugTrace
#else
#define Trace BLDummyTrace
#endif

// bl::OpenType::CoreImpl - Utilities
// ==================================

static BL_INLINE const char* string_from_bool(bool value) noexcept {
  static const char str[] = "False\0\0\0True";
  return str + (size_t(value) * 8u);
}

static BL_INLINE const char* size_check_message(size_t size) noexcept {
  return size ? "Table is truncated" : "Table not found";
}

// bl::OpenType::CoreImpl - Init
// =============================

static BLResult init_head(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  Table<HeadTable> head = tables.head;

  Trace trace;
  trace.info("bl::OpenType::OTFaceImpl::InitHead [Size=%zu]\n", head.size);
  trace.indent();

  if (!head.fits()) {
    trace.fail("%s\n", size_check_message(head.size));
    return bl_make_error(head.size ? BL_ERROR_INVALID_DATA : BL_ERROR_FONT_MISSING_IMPORTANT_TABLE);
  }
  else {
    constexpr uint16_t kMinUnitsPerEm = 16;
    constexpr uint16_t kMaxUnitsPerEm = 16384;

    uint32_t revision = head->revision();
    uint32_t head_flags = head->flags();
    uint16_t units_per_em = head->units_per_em();
    uint16_t lowest_ppem = head->lowestRecPPEM();

    BLBoxI bbox(head->x_min(), -head->y_max(), head->x_max(), -head->y_min());
    if (bbox.x0 > bbox.x1 || bbox.y0 > bbox.y1)
      bbox.reset();

    if (head_flags & HeadTable::kFlagLastResortFont)
      ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_LAST_RESORT_FONT;

    if (head_flags & HeadTable::kFlagBaselineYEquals0)
      ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_BASELINE_Y_EQUALS_0;

    if (head_flags & HeadTable::kFlagLSBPointXEquals0)
      ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_LSB_POINT_X_EQUALS_0;

    trace.info("Revision: %u.%u\n", revision >> 16, revision & 0xFFFFu);
    trace.info("UnitsPerEm: %u\n", units_per_em);
    trace.info("LowestPPEM: %u\n", lowest_ppem);
    trace.info("LastResortFont: %s\n", string_from_bool((ot_face_impl->face_info.face_flags & BL_FONT_FACE_FLAG_LAST_RESORT_FONT) != 0));
    trace.info("BaselineYEquals0: %s\n", string_from_bool((ot_face_impl->face_info.face_flags & BL_FONT_FACE_FLAG_BASELINE_Y_EQUALS_0) != 0));
    trace.info("LSBPointXEquals0: %s\n", string_from_bool((ot_face_impl->face_info.face_flags & BL_FONT_FACE_FLAG_LSB_POINT_X_EQUALS_0) != 0));
    trace.info("BoundingBox: [%d %d %d %d]\n", bbox.x0, bbox.y0, bbox.x1, bbox.y1);

    if (BL_UNLIKELY(units_per_em < kMinUnitsPerEm || units_per_em > kMaxUnitsPerEm)) {
      trace.fail("Invalid UnitsPerEm [%u], must be within [%u:%u] range\n", units_per_em, kMinUnitsPerEm, kMaxUnitsPerEm);
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    uint32_t glyph_data_format = head->glyph_data_format();
    uint32_t index_to_loc_format = head->index_to_loc_format();

    if (glyph_data_format != 0) {
      trace.fail("Invalid GlyphDataFormat [%u], expected 0\n", glyph_data_format);
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    if (index_to_loc_format > 1) {
      trace.fail("Invalid IndexToLocFormat [%u], expected [0:1]\n", index_to_loc_format);
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    ot_face_impl->face_info.revision = revision;
    ot_face_impl->design_metrics.units_per_em = units_per_em;
    ot_face_impl->design_metrics.lowest_ppem = lowest_ppem;
    ot_face_impl->design_metrics.glyph_bounding_box = bbox;
    ot_face_impl->ot_flags |= index_to_loc_format == 0 ? OTFaceFlags::kLocaOffset16 : OTFaceFlags::kLocaOffset32;
  }

  return BL_SUCCESS;
}

static BLResult initMaxP(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  Table<MaxPTable> maxp = tables.maxp;

  Trace trace;
  trace.info("bl::OpenType::OTFaceImpl::InitMaxP [Size=%zu]\n", maxp.size);
  trace.indent();

  if (!maxp.fits()) {
    trace.fail("%s\n", size_check_message(maxp.size));
    return bl_make_error(maxp.size ? BL_ERROR_INVALID_DATA : BL_ERROR_FONT_MISSING_IMPORTANT_TABLE);
  }
  else {
    // We don't know yet if the font is TrueType or OpenType, so only use v0.5 header.
    uint32_t glyph_count = maxp->v0_5()->glyph_count();
    trace.info("GlyphCount: %u\n", glyph_count);

    if (glyph_count == 0) {
      trace.fail("Invalid GlyphCount [%u]\n", glyph_count);
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    ot_face_impl->face_info.glyph_count = uint16_t(glyph_count);
  }

  return BL_SUCCESS;
}

static BLResult initOS_2(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  Table<OS2Table> os2 = tables.os_2;

  Trace trace;
  trace.info("bl::OpenType::OTFaceImpl::InitOS/2 [Size=%zu]\n", os2.size);
  trace.indent();

  if (!os2.fits()) {
    if (os2.size)
      trace.fail("%s\n", size_check_message(os2.size));
  }
  else {
    // Read weight and stretch (width in OS/2 table).
    uint32_t weight = os2->v0a()->weight_class();
    uint32_t stretch = os2->v0a()->width_class();

    // Fix design weight from 1..9 to 100..900 (reported by ~8% of fonts).
    if (weight >= 1 && weight <= 9) weight *= 100;

    // Use defaults if not provided.
    if (!weight) weight = BL_FONT_WEIGHT_NORMAL;
    if (!stretch) stretch = BL_FONT_STRETCH_NORMAL;

    ot_face_impl->weight = uint16_t(bl_clamp<uint32_t>(weight, 1, 999));
    ot_face_impl->stretch = uint8_t(bl_clamp<uint32_t>(stretch, 1, 9));

    trace.info("Weight: %u\n", ot_face_impl->weight);
    trace.info("Stretch: %u\n", ot_face_impl->stretch);

    // Read PANOSE classification.
    memcpy(&ot_face_impl->panose_info, os2->v0a()->panose, sizeof(BLFontPanoseInfo));
    if (!ot_face_impl->panose_info.is_empty())
      ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_PANOSE_INFO;

    // Read unicode coverage.
    ot_face_impl->coverage_info.data[0] = os2->v0a()->unicode_coverage[0].value();
    ot_face_impl->coverage_info.data[1] = os2->v0a()->unicode_coverage[1].value();
    ot_face_impl->coverage_info.data[2] = os2->v0a()->unicode_coverage[2].value();
    ot_face_impl->coverage_info.data[3] = os2->v0a()->unicode_coverage[3].value();
    if (!ot_face_impl->coverage_info.is_empty())
      ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_COVERAGE_INFO;

    // Read strikethrough info.
    int strikeout_thickness = os2->v0a()->yStrikeoutSize();
    int strikeout_position = -(os2->v0a()->yStrikeoutPosition() + strikeout_thickness);
    ot_face_impl->design_metrics.strikethrough_position = strikeout_position;
    ot_face_impl->design_metrics.strikethrough_thickness = strikeout_thickness;

    trace.info("StrikethroughPosition: %d\n", strikeout_position);
    trace.info("StrikethroughThickness: %d\n", strikeout_thickness);

    // Read additional fields provided by newer versions.
    uint32_t version = os2->v0a()->version();
    if (os2.fits(OS2Table::V0B::kBaseSize)) {
      uint32_t selection_flags = os2->v0a()->selection_flags();

      if (selection_flags & OS2Table::kSelectionItalic)
        ot_face_impl->style = BL_FONT_STYLE_ITALIC;
      else if (selection_flags & OS2Table::kSelectionOblique)
        ot_face_impl->style = BL_FONT_STYLE_OBLIQUE;

      if ((selection_flags & OS2Table::kSelectionUseTypoMetrics) != 0)
        ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_TYPOGRAPHIC_METRICS;
      trace.info("HasTypographicMetrics: %s\n", string_from_bool((ot_face_impl->face_info.face_flags & BL_FONT_FACE_FLAG_TYPOGRAPHIC_METRICS) != 0));

      int ascent  = os2->v0b()->typo_ascender();
      int descent = os2->v0b()->typo_descender();
      int line_gap = os2->v0b()->typo_line_gap();

      descent = bl_abs(descent);
      ot_face_impl->design_metrics.ascent = ascent;
      ot_face_impl->design_metrics.descent = descent;
      ot_face_impl->design_metrics.line_gap = line_gap;

      trace.info("Ascent: %d\n", ot_face_impl->design_metrics.ascent);
      trace.info("Descent: %d\n", ot_face_impl->design_metrics.descent);
      trace.info("LineGap: %d\n", ot_face_impl->design_metrics.line_gap);

      if (os2.fits(OS2Table::V2::kBaseSize) && version >= 2) {
        ot_face_impl->design_metrics.x_height = os2->v2()->x_height();
        ot_face_impl->design_metrics.cap_height = os2->v2()->cap_height();

        trace.info("X-Height: %d\n", ot_face_impl->design_metrics.x_height);
        trace.info("Cap-Height: %d\n", ot_face_impl->design_metrics.cap_height);
      }
    }
  }

  return BL_SUCCESS;
}

static BLResult init_post(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  Table<PostTable> post = tables.post;

  Trace trace;
  trace.info("bl::OpenType::OTFaceImpl::InitPost [Size=%zu]\n", post.size);
  trace.indent();

  if (!post.fits()) {
    if (post.size)
      trace.fail("%s\n", size_check_message(post.size));
  }
  else {
    int underline_thickness = post->underline_thickness();
    int underline_position = -(post->underline_position() + underline_thickness);

    trace.info("UnderlinePosition: %d\n", underline_position);
    trace.info("UnderlineThickness: %d\n", underline_thickness);

    ot_face_impl->design_metrics.underline_position = underline_position;
    ot_face_impl->design_metrics.underline_thickness = underline_thickness;
  }

  return BL_SUCCESS;
}

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  BL_PROPAGATE(init_head(ot_face_impl, tables));
  BL_PROPAGATE(initMaxP(ot_face_impl, tables));
  BL_PROPAGATE(initOS_2(ot_face_impl, tables));
  BL_PROPAGATE(init_post(ot_face_impl, tables));

  return BL_SUCCESS;
}

} // {CoreImpl}
} // {bl::OpenType}
