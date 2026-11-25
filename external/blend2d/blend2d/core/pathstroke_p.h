// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PATHSTROKE_P_H_INCLUDED
#define BLEND2D_PATHSTROKE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/path_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace PathInternal {

BL_HIDDEN BLResult stroke_path(
  const BLPathView& input,
  const BLStrokeOptions& options,
  const BLApproximationOptions& approx,
  BLPath& a_path,
  BLPath& b_path,
  BLPath& c_path,
  BLPathStrokeSinkFunc sink, void* user_data) noexcept;

} // {PathInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_PATHSTROKE_P_H_INCLUDED
