// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPOP_P_H_INCLUDED
#define BLEND2D_COMPOP_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/context.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Composition operator that extends \ref BLCompOp, used internally.
enum class CompOpExt : uint32_t {
  kSrcOver     = BL_COMP_OP_SRC_OVER,
  kSrcCopy     = BL_COMP_OP_SRC_COPY,
  kSrcIn       = BL_COMP_OP_SRC_IN,
  kSrcOut      = BL_COMP_OP_SRC_OUT,
  kSrcAtop     = BL_COMP_OP_SRC_ATOP,
  kDstOver     = BL_COMP_OP_DST_OVER,
  kDstCopy     = BL_COMP_OP_DST_COPY,
  kDstIn       = BL_COMP_OP_DST_IN,
  kDstOut      = BL_COMP_OP_DST_OUT,
  kDstAtop     = BL_COMP_OP_DST_ATOP,
  kXor         = BL_COMP_OP_XOR,
  kClear       = BL_COMP_OP_CLEAR,
  kPlus        = BL_COMP_OP_PLUS,
  kMinus       = BL_COMP_OP_MINUS,
  kModulate    = BL_COMP_OP_MODULATE,
  kMultiply    = BL_COMP_OP_MULTIPLY,
  kScreen      = BL_COMP_OP_SCREEN,
  kOverlay     = BL_COMP_OP_OVERLAY,
  kDarken      = BL_COMP_OP_DARKEN,
  kLighten     = BL_COMP_OP_LIGHTEN,
  kColorDodge  = BL_COMP_OP_COLOR_DODGE,
  kColorBurn   = BL_COMP_OP_COLOR_BURN,
  kLinearBurn  = BL_COMP_OP_LINEAR_BURN,
  kLinearLight = BL_COMP_OP_LINEAR_LIGHT,
  kPinLight    = BL_COMP_OP_PIN_LIGHT,
  kHardLight   = BL_COMP_OP_HARD_LIGHT,
  kSoftLight   = BL_COMP_OP_SOFT_LIGHT,
  kDifference  = BL_COMP_OP_DIFFERENCE,
  kExclusion   = BL_COMP_OP_EXCLUSION,

  kAlphaInv    = BL_COMP_OP_MAX_VALUE + 1,

  kMaxValue    = kAlphaInv
};

static constexpr uint32_t kCompOpExtCount = uint32_t(CompOpExt::kMaxValue) + 1u;

//! Composition operator flags that can be retrieved through CompOpInfo[] table.
enum class CompOpFlags : uint32_t {
  kNone = 0,

  //! TypeA operator - "D*(1-M) + Op(D, S)*M" == "Op(D, S * M)".
  kTypeA = 0x00000001u,
  //! TypeB operator - "D*(1-M) + Op(D, S)*M" == "Op(D, S * M) + D * (1 - M)".
  kTypeB = 0x00000002u,
  //! TypeC operator - cannot be simplified.
  kTypeC = 0x00000004u,

  //! Non-separable operator.
  kNonSeparable = 0x00000008u,

  //! Uses `Dc` (destination color or luminance channel).
  kDc = 0x00000010u,
  //! Uses `Da` (destination alpha channel).
  kDa = 0x00000020u,
  //! Uses both `Dc` and `Da`.
  kDc_Da = 0x00000030u,

  //! Uses `Sc` (source color or luminance channel).
  kSc = 0x00000040u,
  //! Uses `Sa` (source alpha channel).
  kSa = 0x00000080u,
  //! Uses both `Sc` and `Sa`,
  kSc_Sa = 0x000000C0u,

  //! Destination is never changed (NOP).
  kNop = 0x00000800u,
  //! Destination is changed only if `Da != 0`.
  kNopIfDaEq0 = 0x00001000u,
  //! Destination is changed only if `Da != 1`.
  kNopIfDaEq1 = 0x00002000u,
  //! Destination is changed only if `Sa != 0`.
  kNopIfSaEq0 = 0x00004000u,
  //! Destination is changed only if `Sa != 1`.
  kNopIfSaEq1 = 0x00008000u
};
BL_DEFINE_ENUM_FLAGS(CompOpFlags)

//! Simplification of a composition operator that leads to SOLID fill instead.
enum class CompOpSolidId : uint32_t {
  //! Source pixels are used.
  //!
  //! \note This value must be zero as it's usually combined with rendering context flags and then used for decision
  //! making about the whole command.
  kNone = 0,
  //! Source pixels are always treated as transparent zero (all 0).
  kTransparent = 1,
  //! Source pixels are always treated as opaque black (R|G|B=0 A=1).
  kOpaqueBlack = 2,
  //! Source pixels are always treated as opaque white (R|G|B=1 A=1).
  kOpaqueWhite = 3,

  //! Source pixels are always treated as transparent zero (all 0) and this composition operator is also a NOP.
  kAlwaysNop = 4,

  kMaxValue = 4
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_COMPOP_P_H_INCLUDED
