// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_GEOMETRY_SIZETABLE_P_H_INCLUDED
#define BLEND2D_GEOMETRY_SIZETABLE_P_H_INCLUDED

#include <blend2d/core/geometry.h>
#include <blend2d/support/lookuptable_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_geometry
//! \{

namespace bl::Geometry {

//! \name Geometry Type Size
//! \{

static BL_INLINE bool is_simple_geometry_type(uint32_t geometry_type) noexcept {
  return geometry_type <= BL_GEOMETRY_TYPE_SIMPLE_LAST;
}

BL_HIDDEN extern const LookupTable<uint8_t, BL_GEOMETRY_TYPE_SIMPLE_LAST + 1> geometry_type_size_table;

//! \}


} // {bl::Geometry}

//! \}
//! \endcond

#endif // BLEND2D_GEOMETRY_SIZETABLE_P_H_INCLUDED
