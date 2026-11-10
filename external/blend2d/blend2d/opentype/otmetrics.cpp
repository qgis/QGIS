// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/trace_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otmetrics_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl::OpenType {
namespace MetricsImpl {

// bl::OpenType::MetricsImpl - Trace
// =================================

#if defined(BL_TRACE_OT_ALL) || defined(BL_TRACE_OT_CORE)
#define Trace BLDebugTrace
#else
#define Trace BLDummyTrace
#endif

// bl::OpenType::MetricsImpl - Utilities
// =====================================

static BL_INLINE const char* string_from_bool(bool value) noexcept {
  static const char str[] = "False\0\0\0True";
  return str + (size_t(value) * 8u);
}

static BL_INLINE const char* size_check_message(size_t size) noexcept {
  return size ? "Table is truncated" : "Table not found";
}

// bl::OpenType::MetricsImpl - GetGlyphAdvances
// ============================================

static BLResult BL_CDECL get_glyph_advances(const BLFontFaceImpl* face_impl, const uint32_t* glyph_data, intptr_t glyph_advance, BLGlyphPlacement* placement_data, size_t count) noexcept {
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  const XMtxTable* mtx_table = ot_face_impl->metrics.xmtx_table[BL_ORIENTATION_HORIZONTAL].data_as<XMtxTable>();

  // Sanity check.
  uint32_t long_metric_count = ot_face_impl->metrics.long_metric_count[BL_ORIENTATION_HORIZONTAL];
  uint32_t long_metric_max = long_metric_count - 1u;

  if (BL_UNLIKELY(!long_metric_count))
    return bl_make_error(BL_ERROR_INVALID_DATA);

  for (size_t i = 0; i < count; i++) {
    BLGlyphId glyph_id = glyph_data[0];
    glyph_data = PtrOps::offset(glyph_data, glyph_advance);

    uint32_t metric_index = bl_min(glyph_id, long_metric_max);
    int32_t advance = mtx_table->lm_array()[metric_index].advance.value();

    placement_data[i].placement.reset(0, 0);
    placement_data[i].advance.reset(advance, 0);
  }

  return BL_SUCCESS;
}

// bl::OpenType::MetricsImpl - Init
// ================================

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  BLFontDesignMetrics& dm = ot_face_impl->design_metrics;

  Table<XHeaTable> hhea(tables.hhea);
  Table<XMtxTable> hmtx(tables.hmtx);
  Table<XHeaTable> vhea(tables.vhea);
  Table<XMtxTable> vmtx(tables.vmtx);

  if (hhea) {
    if (!hhea.fits())
      return bl_make_error(BL_ERROR_INVALID_DATA);

    if (!(ot_face_impl->face_info.face_flags & BL_FONT_FACE_FLAG_TYPOGRAPHIC_METRICS)) {
      int ascent  = hhea->ascender();
      int descent = hhea->descender();
      int line_gap = hhea->line_gap();

      dm.ascent = ascent;
      dm.descent = bl_abs(descent);
      dm.line_gap = line_gap;
    }

    dm.h_min_lsb = hhea->min_leading_bearing();
    dm.h_min_tsb = hhea->min_trailing_bearing();
    dm.h_max_advance = hhea->max_advance();

    if (hmtx) {
      uint32_t long_metric_count = bl_min<uint32_t>(hhea->long_metric_count(), ot_face_impl->face_info.glyph_count);
      size_t long_metric_data_size = size_t(long_metric_count) * sizeof(XMtxTable::LongMetric);

      if (!hmtx.fits(long_metric_data_size))
        return bl_make_error(BL_ERROR_INVALID_DATA);

      size_t lsb_count = bl_min<size_t>((hmtx.size - long_metric_data_size) / 2u, long_metric_count - ot_face_impl->face_info.glyph_count);
      ot_face_impl->metrics.xmtx_table[BL_ORIENTATION_HORIZONTAL] = hmtx;
      ot_face_impl->metrics.long_metric_count[BL_ORIENTATION_HORIZONTAL] = uint16_t(long_metric_count);
      ot_face_impl->metrics.lsb_array_size[BL_ORIENTATION_HORIZONTAL] = uint16_t(lsb_count);
    }

    ot_face_impl->funcs.get_glyph_advances = get_glyph_advances;
  }

  if (vhea) {
    if (!vhea.fits())
      return bl_make_error(BL_ERROR_INVALID_DATA);

    dm.v_ascent = vhea->ascender();
    dm.v_descent = vhea->descender();
    dm.v_min_lsb = vhea->min_leading_bearing();
    dm.v_min_tsb = vhea->min_trailing_bearing();
    dm.v_max_advance = vhea->max_advance();

    if (vmtx) {
      uint32_t long_metric_count = bl_min<uint32_t>(vhea->long_metric_count(), ot_face_impl->face_info.glyph_count);
      size_t long_metric_data_size = size_t(long_metric_count) * sizeof(XMtxTable::LongMetric);

      if (!vmtx.fits(long_metric_data_size))
        return bl_make_error(BL_ERROR_INVALID_DATA);

      size_t lsb_count = bl_min<size_t>((vmtx.size - long_metric_data_size) / 2u, long_metric_count - ot_face_impl->face_info.glyph_count);
      ot_face_impl->metrics.xmtx_table[BL_ORIENTATION_VERTICAL] = vmtx;
      ot_face_impl->metrics.long_metric_count[BL_ORIENTATION_VERTICAL] = uint16_t(long_metric_count);
      ot_face_impl->metrics.lsb_array_size[BL_ORIENTATION_VERTICAL] = uint16_t(lsb_count);
    }
  }

  return BL_SUCCESS;
}

} // {MetricsImpl}
} // {bl::OpenType}
