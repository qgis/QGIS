// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_MATHCONST_P_H_INCLUDED
#define BLEND2D_SUPPORT_MATHCONST_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_support
//! \{

namespace bl {
namespace Math {

//! \name Math Constants
//! \{

static constexpr double kPI            = 3.14159265358979323846;  //!< pi.
static constexpr double kPI_MUL_1p5    = 4.71238898038468985769;  //!< pi * 1.5.
static constexpr double kPI_MUL_2      = 6.28318530717958647692;  //!< pi * 2.
static constexpr double kPI_DIV_2      = 1.57079632679489661923;  //!< pi / 2.
static constexpr double kPI_DIV_3      = 1.04719755119659774615;  //!< pi / 3.
static constexpr double kPI_DIV_4      = 0.78539816339744830962;  //!< pi / 4.
static constexpr double kSQRT_0p5      = 0.70710678118654746172;  //!< sqrt(0.5).
static constexpr double kSQRT_2        = 1.41421356237309504880;  //!< sqrt(2).
static constexpr double kSQRT_3        = 1.73205080756887729353;  //!< sqrt(3).

static constexpr double kAfter0        = 1e-40;                   //!< Safe value after 0.0 for root finding/intervals.
static constexpr double kBefore1       = 0.999999999999999889;    //!< Safe value before 1.0 for root finding/intervals.

static constexpr double kANGLE_EPSILON = 1e-8;

//! Constant that is used to approximate elliptic arcs with cubic curves. Since it's an approximation there are
//! various approaches that can be used to calculate the best value. The most used KAPPA is:
//!
//!   k = (4/3) * (sqrt(2) - 1) ~= 0.55228474983
//!
//! which has a maximum error of 0.00027253. However, according to this post
//!
//!   http://spencermortensen.com/articles/bezier-circle/
//!
//! the maximum error can be further reduced by 28% if we change the approximation constraint to have the maximum
//! radial distance from the circle to the curve as small as possible. The an alternative constant
//!
//!   k = 1/2 +- sqrt(12 - 20*c - 3*c^2)/(4 - 6*c) ~= 0.551915024494.
//!
//! can be used to reduce the maximum error to 0.00019608. We don't use the alternative, because we need to calculate
//! the KAPPA for arcs that are not 90deg, in that case the KAPPA must be calculated for such angles.
static constexpr double kKAPPA = 0.55228474983;

} // {Math}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_MATHCONST_P_H_INCLUDED
