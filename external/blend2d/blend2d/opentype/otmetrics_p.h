// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTMETRICS_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTMETRICS_P_H_INCLUDED

#include <blend2d/opentype/otcore_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

//! OpenType 'hhea' and 'vhea' tables.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/hhea
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/vhea
struct XHeaTable {
  enum : uint32_t { kBaseSize = 36 };

  F16x16 version;
  Int16 ascender;
  Int16 descender;
  Int16 line_gap;
  UInt16 max_advance;
  Int16 min_leading_bearing;
  Int16 min_trailing_bearing;
  Int16 max_extent;
  Int16 caret_slope_rise;
  Int16 caret_slope_run;
  Int16 caret_offset;
  Int16 reserved[4];
  UInt16 long_metric_format;
  UInt16 long_metric_count;
};

//! OpenType 'hmtx' and 'vmtx' tables.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/hmtx
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/vmtx
struct XMtxTable {
  // At least one LongMetric.
  enum : uint32_t { kBaseSize = 4  };

  struct LongMetric {
    UInt16 advance;
    Int16 lsb;
  };

  /*
  LongMetric lm_array[];
  Int16 lsb_array[];
  */

  //! Paired advance width and left side bearing values, indexed by glyph ID.
  BL_INLINE const LongMetric* lm_array() const noexcept { return PtrOps::offset<const LongMetric>(this, 0); }
  //! Leading side bearings for glyph IDs greater than or equal to `metric_count`.
  BL_INLINE const Int16* lsb_array(size_t metric_count) const noexcept { return PtrOps::offset<const Int16>(this, metric_count * sizeof(LongMetric)); }
};

struct MetricsData {
  //! Metrics tables - 'hmtx' and 'vmtx' (if present).
  Table<XMtxTable> xmtx_table[2];
  //! Count of LongMetric entries.
  uint16_t long_metric_count[2];
  //! Count of LSB entries.
  uint16_t lsb_array_size[2];
};

namespace MetricsImpl {
BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept;
} // {MetricsImpl}

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTMETRICS_P_H_INCLUDED
