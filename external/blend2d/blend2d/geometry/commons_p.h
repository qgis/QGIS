// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_GEOMETRY_COMMONS_P_H_INCLUDED
#define BLEND2D_GEOMETRY_COMMONS_P_H_INCLUDED

#include <blend2d/core/geometry.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/span_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_geometry
//! \{

namespace bl::Geometry {

using Math::lerp;

static BL_INLINE_NODEBUG bool is_zero(const BLPoint& p) noexcept { return (p.x == 0) & (p.y == 0); }


//! \name Geometry Type Size
//! \{

static BL_INLINE bool is_simple_geometry_type(uint32_t geometry_type) noexcept {
  return geometry_type <= BL_GEOMETRY_TYPE_SIMPLE_LAST;
}

BL_HIDDEN extern const LookupTable<uint8_t, BL_GEOMETRY_TYPE_SIMPLE_LAST + 1> geometry_type_size_table;

//! \}

//! \name Validity Checks
//! \{

static BL_INLINE bool is_valid(const BLSizeI& size) noexcept { return BLInternal::bool_and(size.w > 0, size.h > 0); }
static BL_INLINE bool is_valid(const BLSize& size) noexcept { return BLInternal::bool_and(size.w > 0, size.h > 0); }

static BL_INLINE bool is_valid(const BLBoxI& box) noexcept { return BLInternal::bool_and(box.x0 < box.x1, box.y0 < box.y1); }
static BL_INLINE bool is_valid(const BLBox& box) noexcept { return BLInternal::bool_and(box.x0 < box.x1, box.y0 < box.y1); }

static BL_INLINE bool is_valid(const BLRectI& rect) noexcept {
  OverflowFlag of = 0;
  int x1 = IntOps::add_overflow(rect.x, rect.w, &of);
  int y1 = IntOps::add_overflow(rect.y, rect.h, &of);
  return BLInternal::bool_and(rect.x < x1, rect.y < y1, !of);
}

static BL_INLINE bool is_valid(const BLRect& rect) noexcept {
  double x1 = rect.x + rect.w;
  double y1 = rect.y + rect.h;
  return BLInternal::bool_and(rect.x < x1, rect.y < y1);
}

//! \}

//! \name Vector Operations
//! \{

static BL_INLINE double dot(const BLPoint& a, const BLPoint& b) noexcept { return Math::madd(a.x, b.x, a.y * b.y); }
static BL_INLINE double cross(const BLPoint& a, const BLPoint& b) noexcept { return Math::msub(a.x, b.y, a.y * b.x); }

static BL_INLINE double magnitude_squared(const BLPoint& v) noexcept { return Math::madd(v.x, v.x, v.y * v.y); }
static BL_INLINE double magnitude(const BLPoint& v) noexcept { return Math::sqrt(magnitude_squared(v)); }

static BL_INLINE double length_squared(const BLPoint& a, const BLPoint& b) noexcept { return magnitude_squared(b - a); }
static BL_INLINE double length(const BLPoint& a, const BLPoint& b) noexcept { return Math::sqrt(length_squared(a, b)); }

static BL_INLINE BLPoint normal(const BLPoint& v) noexcept { return BLPoint(-v.y, v.x); }
static BL_INLINE BLPoint unit_vector(const BLPoint& v) noexcept { return v / magnitude(v); }

static BL_INLINE BLPoint line_vector_intersection(const BLPoint& p0, const BLPoint& v0, const BLPoint& p1, const BLPoint& v1) noexcept {
  return p0 + cross(p1 - p0, v1) / cross(v0, v1) * v0;
}

//! \}

//! \name Box/Rect Operations
//! \{

static BL_INLINE void bound(BLBox& box, const BLPoint& p) noexcept {
  box.reset(bl_min(box.x0, p.x), bl_min(box.y0, p.y),
            bl_max(box.x1, p.x), bl_max(box.y1, p.y));
}

static BL_INLINE void bound(BLBox& box, const BLBox& other) noexcept {
  box.reset(bl_min(box.x0, other.x0), bl_min(box.y0, other.y0),
            bl_max(box.x1, other.x1), bl_max(box.y1, other.y1));
}

static BL_INLINE void bound(BLBoxI& box, const BLBoxI& other) noexcept {
  box.reset(bl_min(box.x0, other.x0), bl_min(box.y0, other.y0),
            bl_max(box.x1, other.x1), bl_max(box.y1, other.y1));
}

static BL_INLINE bool intersect(BLBoxI& dst, const BLBoxI& a, const BLBoxI& b) noexcept {
  dst.reset(bl_max(a.x0, b.x0), bl_max(a.y0, b.y0),
            bl_min(a.x1, b.x1), bl_min(a.y1, b.y1));
  return BLInternal::bool_and(dst.x0 < dst.x1,dst.y0 < dst.y1);
}

static BL_INLINE bool intersect(BLBox& dst, const BLBox& a, const BLBox& b) noexcept {
  dst.reset(bl_max(a.x0, b.x0), bl_max(a.y0, b.y0),
            bl_min(a.x1, b.x1), bl_min(a.y1, b.y1));
  return BLInternal::bool_and(dst.x0 < dst.x1, dst.y0 < dst.y1);
}

static BL_INLINE bool subsumes(const BLBoxI& a, const BLBoxI& b) noexcept {
  return BLInternal::bool_and(a.x0 <= b.x0, a.y0 <= b.y0, a.x1 >= b.x1, a.y1 >= b.y1);
}

static BL_INLINE bool subsumes(const BLBox& a, const BLBox& b) noexcept {
  return BLInternal::bool_and(a.x0 <= b.x0, a.y0 <= b.y0, a.x1 >= b.x1, a.y1 >= b.y1);
}

static BL_INLINE bool overlaps(const BLBoxI& a, const BLBoxI& b) noexcept {
  return BLInternal::bool_and(a.x1 > b.x0, a.y1 > b.y0, a.x0 < b.x1, a.y0 < b.y1);
}

static BL_INLINE bool overlaps(const BLBox& a, const BLBox& b) noexcept {
  return BLInternal::bool_and(a.x1 > b.x0, a.y1 > b.y0, a.x0 < b.x1, a.y0 < b.y1);
}

//! \}

} // {bl::Geometry}

//! \}
//! \endcond

#endif // BLEND2D_GEOMETRY_COMMONS_P_H_INCLUDED
