// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_GEOMETRY_TOLERANCE_P_H_INCLUDED
#define BLEND2D_GEOMETRY_TOLERANCE_P_H_INCLUDED

#include <blend2d/geometry/commons_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_geometry
//! \{

namespace bl::Geometry {

struct Tolerance {
  //! Tolerance parameter.
  double tolerance;
  //! Tolerance used to simplify conic curves.
  double toleranceMul4;
  //! Tolerance used to simplify cubic curves.
  double toleranceMul54;
};

static BL_INLINE Tolerance make_tolerance(double tolerance) noexcept {
  double mul4 = tolerance * 4.0;
  double mul54 = tolerance * 54.0;
  return Tolerance{tolerance, mul4, mul54};
}

} // {bl::Geometry}

//! \}
//! \endcond

#endif // BLEND2D_GEOMETRY_TOLERANCE_P_H_INCLUDED
