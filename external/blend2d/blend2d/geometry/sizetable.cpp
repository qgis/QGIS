// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/geometry/sizetable_p.h>

namespace bl::Geometry {

// bl::Geometry - Size Table
// =========================

struct GeometryTypeSizeTableGen {
  static BL_INLINE constexpr uint8_t value(size_t i) noexcept {
    return i == BL_GEOMETRY_TYPE_BOXI       ? uint8_t(sizeof(BLBoxI))      :
           i == BL_GEOMETRY_TYPE_BOXD       ? uint8_t(sizeof(BLBox))       :
           i == BL_GEOMETRY_TYPE_RECTI      ? uint8_t(sizeof(BLRectI))     :
           i == BL_GEOMETRY_TYPE_RECTD      ? uint8_t(sizeof(BLRect))      :
           i == BL_GEOMETRY_TYPE_CIRCLE     ? uint8_t(sizeof(BLCircle))    :
           i == BL_GEOMETRY_TYPE_ELLIPSE    ? uint8_t(sizeof(BLEllipse))   :
           i == BL_GEOMETRY_TYPE_ROUND_RECT ? uint8_t(sizeof(BLRoundRect)) :
           i == BL_GEOMETRY_TYPE_ARC        ? uint8_t(sizeof(BLArc))       :
           i == BL_GEOMETRY_TYPE_CHORD      ? uint8_t(sizeof(BLArc))       :
           i == BL_GEOMETRY_TYPE_PIE        ? uint8_t(sizeof(BLArc))       :
           i == BL_GEOMETRY_TYPE_LINE       ? uint8_t(sizeof(BLLine))      :
           i == BL_GEOMETRY_TYPE_TRIANGLE   ? uint8_t(sizeof(BLTriangle))  : 0;
  }
};

static constexpr auto geometry_type_size_table_ = make_lookup_table<uint8_t, BL_GEOMETRY_TYPE_SIMPLE_LAST + 1, GeometryTypeSizeTableGen>();
const LookupTable<uint8_t, BL_GEOMETRY_TYPE_SIMPLE_LAST + 1> geometry_type_size_table = geometry_type_size_table_;

} // {bl::Geometry}
