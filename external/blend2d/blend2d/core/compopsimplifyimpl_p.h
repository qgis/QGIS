// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPOPSIMPLIFYIMPL_P_H_INCLUDED
#define BLEND2D_COMPOPSIMPLIFYIMPL_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/compopinfo_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

// Legend:
//
//   - Sca  - Source color, premultiplied: `Sc * Sa`.
//   - Sc   - Source color.
//   - Sa   - Source alpha.
//
//   - Dca  - Destination color, premultiplied: `Dc * Da`.
//   - Dc   - Destination color.
//   - Da   - Destination alpha.
//
//   - Dca' - Resulting color, premultiplied.
//   - Da'  - Resulting alpha.
//
//   - m    - Mask (if used).
//
// Blending function F(Sc, Dc) is used in the following way if destination
// or source contains alpha channel (otherwise it's assumed to be `1.0`):
//
//  - Dca' = Func(Sc, Dc) * Sa.Da + Sca.(1 - Da) + Dca.(1 - Sa)
//  - Da'  = Da + Sa.(1 - Da)
struct CompOpSimplifyInfoImpl {
  // Shorthands of pixel formats.
  using Fmt = bl::FormatExt;

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo make_op(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return CompOpSimplifyInfo::make(comp_op, d, s, CompOpSolidId::kNone);
  }

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo transparent(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return CompOpSimplifyInfo::make(comp_op, d, s, CompOpSolidId::kTransparent);
  }

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo opaque_black(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return CompOpSimplifyInfo::make(comp_op, d, s, CompOpSolidId::kOpaqueBlack);
  }

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo opaque_white(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return CompOpSimplifyInfo::make(comp_op, d, s, CompOpSolidId::kOpaqueWhite);
  }

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo opaque_alpha(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return CompOpSimplifyInfo::make(comp_op, d, s, CompOpSolidId::kOpaqueWhite);
  }

  // Internal Formats:
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo alpha_inv(Fmt d, Fmt s) noexcept {
    return CompOpSimplifyInfo::make(CompOpExt::kAlphaInv, d, s, CompOpSolidId::kOpaqueWhite);
  }

  // Clear
  // -----
  //
  // [Clear PRGBxPRGB]
  //   Dca' = 0                              Dca' = Dca.(1 - m)
  //   Da'  = 0                              Da'  = Da .(1 - m)
  //
  // [Clear XRGBxPRGB]
  //   Dc'  = 0                              Dc'  = Dca.(1 - m)
  //
  // [Clear PRGBxXRGB] ~= [Clear PRGBxPRGB]
  // [Clear XRGBxXRGB] ~= [Clear XRGBxPRGB]
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo clear(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 ? transparent(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 ? opaque_black(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kA8     ? transparent(CompOpExt::kSrcCopy, Fmt::kA8    , Fmt::kPRGB32) :

           make_op(CompOpExt::kClear, d, s);
  }

  // SrcCopy
  // -------
  //
  // [Src PRGBxPRGB]
  //   Dca' = Sca                            Dca' = Sca.m + Dca.(1 - m)
  //   Da'  = Sa                             Da'  = Sa .m + Da .(1 - m)
  //
  // [Src PRGBxXRGB] ~= [Src PRGBxPRGB]
  //   Dca' = Sc                             Dca' = Sc.m + Dca.(1 - m)
  //   Da'  = 1                              Da'  = 1 .m + Da .(1 - m)
  //
  // [Src XRGBxPRGB]
  //   Dc'  = Sca                            Dc'  = Sca.m + Dc.(1 - m)
  //
  // [Src XRGBxXRGB]
  //   Dc'  = Sc                             Dc'  = Sc.m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo src_copy(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? make_op(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? make_op(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? make_op(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kXRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? make_op(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kXRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? make_op(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kXRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? make_op(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kA8 && s == Fmt::kZERO32 ? clear(Fmt::kA8, Fmt::kZERO32) :
           d == Fmt::kA8 && s == Fmt::kXRGB32 ? opaque_alpha(CompOpExt::kSrcCopy, d, Fmt::kPRGB32) :
           d == Fmt::kA8 && s == Fmt::kFRGB32 ? opaque_alpha(CompOpExt::kSrcCopy, d, Fmt::kPRGB32) :

           make_op(CompOpExt::kSrcCopy, d, s);
  }

  // DstCopy
  // -------
  //
  // [DstCopy ANYxANY]
  //   Dca' = Dca
  //   Da   = Da
  BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo dst_copy(Fmt d, Fmt s) noexcept {
    return CompOpSimplifyInfo::dst_copy();
  }
  BL_DIAGNOSTIC_POP

  // SrcOver
  // -------
  //
  // [SrcOver PRGBxPRGB]
  //   Dca' = Sca + Dca.(1 - Sa)             Dca' = Sca.m + Dca.(1 - Sa.m)
  //   Da'  = Sa  + Da .(1 - Sa)             Da'  = Sa .m + Da .(1 - Sa.m)
  //
  // [SrcOver PRGBxXRGB] ~= [Src PRGBxPRGB]
  //   Dca' = Sc                             Dca' = Sc.m + Dca.(1 - m)
  //   Da'  = 1                              Da'  = 1 .m + Da .(1 - m)
  //
  // [SrcOver XRGBxPRGB] ~= [SrcOver PRGBxPRGB]
  //   Dc'  = Sca   + Dc.(1 - Sa  )          Dc'  = Sca.m + Dc.(1 - Sa.m)
  //
  // [SrcOver XRGBxXRGB] ~= [Src PRGBxPRGB]
  //   Dc'  = Sc                             Dc'  = Sc.m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo src_over(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kPRGB32 && s == Fmt::kXRGB32 ? src_copy(Fmt::kPRGB32, Fmt::kXRGB32) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? src_copy(Fmt::kPRGB32, Fmt::kFRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? src_over(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? src_copy(Fmt::kPRGB32, Fmt::kXRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? src_copy(Fmt::kPRGB32, Fmt::kFRGB32) :

           d == Fmt::kA8 && s == Fmt::kZERO32 ? dst_copy(Fmt::kA8, Fmt::kPRGB32) :
           d == Fmt::kA8 && s == Fmt::kXRGB32 ? src_copy(Fmt::kA8, Fmt::kXRGB32) :
           d == Fmt::kA8 && s == Fmt::kFRGB32 ? src_copy(Fmt::kA8, Fmt::kFRGB32) :

           make_op(CompOpExt::kSrcOver, d, s);
  }

  // DstOver
  // -------
  //
  // [DstOver PRGBxPRGB]
  //   Dca' = Dca + Sca.(1 - Da)             Dca' = Dca + Sca.m.(1 - Da)
  //   Da'  = Da  + Sa .(1 - Da)             Da'  = Da  + Sa .m.(1 - Da)
  //
  // [DstOver PRGBxXRGB] ~= [DstOver PRGBxPRGB]
  //   Dca' = Dca + Sc.(1 - Da)              Dca' = Dca + Sc.m.(1 - Da)
  //   Da'  = Da  + 1 .(1 - Da)              Da'  = Da  + 1 .m.(1 - Da)
  //
  // [DstOver XRGBxPRGB] ~= [Dst]
  //   Dc'  = Dc
  //
  // [DstOver XRGBxXRGB] ~= [Dst]
  //   Dc'  = Dc
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo dst_over(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? dst_over(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 ? dst_copy(d, s) :

           d == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kDstOver, d, s);
  }

  // SrcIn
  // -----
  //
  // [SrcIn PRGBxPRGB]
  //   Dca' = Sca.Da                         Dca' = Sca.Da.m + Dca.(1 - m)
  //   Da'  = Sa .Da                         Da'  = Sa .Da.m + Da .(1 - m)
  //
  // [SrcIn PRGBxXRGB] ~= [SrcIn PRGBxPRGB]
  //   Dca' = Sc.Da                          Dca' = Sc.Da.m + Dca.(1 - m)
  //   Da'  = 1 .Da                          Da'  = 1 .Da.m + Da .(1 - m)
  //
  // [SrcIn XRGBxPRGB]
  //   Dc'  = Sca                            Dc'  = Sca.m + Dc.(1 - m)
  //
  // [SrcIn XRGBxXRGB] ~= [SrcCopy XRGBxXRGB]
  //   Dc'  = Sc                             Dc'  = Sc.m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo src_in(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? src_in(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? src_in(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 ? src_copy(d, s) :

           d == Fmt::kA8 && s == Fmt::kZERO32 ? clear(d, s) :
           d == Fmt::kA8 && s == Fmt::kXRGB32 ? dst_copy(d, s) :
           d == Fmt::kA8 && s == Fmt::kFRGB32 ? dst_copy(d, s) :

           make_op(CompOpExt::kSrcIn, d, s);
  }

  // DstIn
  // -----
  //
  // [DstIn PRGBxPRGB]
  //   Dca' = Dca.Sa                         Dca' = Dca.Sa.m + Dca.(1 - m)
  //   Da'  = Da .Sa                         Da'  = Da .Sa.m + Da .(1 - m)
  //
  // [DstIn PRGBxXRGB] ~= [Dst]
  //   Dca' = Dca
  //   Da'  = Da
  //
  // [DstIn XRGBxPRGB]
  //   Dc'  = Dc.Sa                          Dc'  = Dc.Sa.m + Dc.(1 - m)
  //
  // [DstIn XRGBxXRGB] ~= [Dst]
  //   Dc'  = Dc
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo dst_in(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? src_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kXRGB32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? dst_copy(d, s) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? dst_in(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_in(Fmt::kPRGB32, Fmt::kFRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? dst_copy(d, s) :

           d == Fmt::kA8 ? src_in(d, s) :

           make_op(CompOpExt::kDstIn, d, s);
  }

  // SrcOut
  // ------
  //
  // [SrcOut PRGBxPRGB]
  //   Dca' = Sca.(1 - Da)                   Dca' = Sca.m.(1 - Da) + Dca.(1 - m)
  //   Da'  = Sa .(1 - Da)                   Da'  = Sa .m.(1 - Da) + Da .(1 - m)
  //
  // [SrcOut PRGBxXRGB] ~= [SrcOut PRGBxPRGB]
  //   Dca' = Sc.(1 - Da)                    Dca' = Sc.m.(1 - Da) + Dca.(1 - m)
  //   Da'  = 1 .(1 - Da)                    Da'  = 1 .m.(1 - Da) + Da .(1 - m)
  //
  // [SrcOut XRGBxPRGB] ~= [Clear XRGBxPRGB]
  //   Dc'  = 0                              Dc'  = Dc.(1 - m)
  //
  // [SrcOut XRGBxXRGB] ~= [Clear XRGBxPRGB]
  //   Dc'  = 0                              Dc'  = Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo src_out(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? clear(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? src_out(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 ? clear(d, s) :

           d == Fmt::kA8 && s == Fmt::kZERO32 ? clear(d, s) :
           d == Fmt::kA8 && s == Fmt::kXRGB32 ? alpha_inv(d, Fmt::kXRGB32) :
           d == Fmt::kA8 && s == Fmt::kFRGB32 ? alpha_inv(d, Fmt::kXRGB32) :

           make_op(CompOpExt::kSrcOut, d, s);
  }

  // DstOut
  // ------
  //
  // [DstOut PRGBxPRGB]
  //   Dca' = Dca.(1 - Sa)                   Dca' = Dca.(1 - Sa.m)
  //   Da'  = Da .(1 - Sa)                   Da'  = Da .(1 - Sa.m)
  //
  // [DstOut PRGBxXRGB] ~= [Clear PRGBxPRGB]
  //   Dca' = 0
  //   Da'  = 0
  //
  // [DstOut XRGBxPRGB]
  //   Dc'  = Dc.(1 - Sa)                    Dc'  = Dc.(1 - Sa.m)
  //
  // [DstOut XRGBxXRGB] ~= [Clear XRGBxPRGB]
  //   Dc'  = 0
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo dst_out(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kXRGB32 ? clear(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? clear(d, s) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? clear(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? clear(d, s) :

           d == Fmt::kA8 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kA8 && s == Fmt::kXRGB32 ? clear(d, s) :
           d == Fmt::kA8 && s == Fmt::kFRGB32 ? clear(d, s) :

           make_op(CompOpExt::kDstOut, d, s);
  }

  // SrcAtop
  // -------
  //
  // [SrcAtop PRGBxPRGB]
  //   Dca' = Sca.Da + Dca.(1 - Sa)          Dca' = Sca.Da.m + Dca.(1 - Sa.m)
  //   Da'  = Sa .Da + Da .(1 - Sa) = Da     Da'  = Sa .Da.m + Da .(1 - Sa.m) = Da
  //
  // [SrcAtop PRGBxXRGB] ~= [SrcIn PRGBxPRGB]
  //   Dca' = Sc.Da                          Dca' = Sc.Da.m + Dca.(1 - m)
  //   Da'  = 1 .Da                          Da'  = 1 .Da.m + Da .(1 - m)
  //
  // [SrcAtop XRGBxPRGB] ~= [SrcOver PRGBxPRGB]
  //   Dc'  = Sca + Dc.(1 - Sa)              Dc'  = Sca.m + Dc.(1 - Sa.m)
  //
  // [SrcAtop XRGBxXRGB] ~= [Src PRGBxPRGB]
  //   Dc'  = Sc                             Dc'  = Sc.m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo src_atop(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kXRGB32 ? src_in(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? src_in(d, s) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? src_over(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? src_over(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? src_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? src_copy(d, s) :

           d == Fmt::kA8 ? dst_copy(d, s) :

           make_op(CompOpExt::kSrcAtop, d, s);
  }

  // DstAtop
  // -------
  //
  // [DstAtop PRGBxPRGB]
  //   Dca' = Dca.Sa + Sca.(1 - Da)          Dca' = Dca.(1 - m.(1 - Sa)) + Sca.m.(1 - Da)
  //   Da'  = Da .Sa + Sa .(1 - Da) = Sa     Da'  = Da .(1 - m.(1 - Sa)) + Sa .m.(1 - Da)
  //
  // [DstAtop PRGBxXRGB] ~= [DstOver PRGBxPRGB]
  //   Dca' = Dca + Sc.(1 - Da)              Dca' = Dca + Sc.m.(1 - Da)
  //   Da'  = Da  + 1 .(1 - Da) = 1          Da'  = Da  + 1 .m.(1 - Da)
  //
  // [DstAtop XRGBxPRGB] ~= [DstIn XRGBxPRGB]
  //   Dc'  = Dc.Sa                          Dc'  = Dc.(1 - m.(1 - Sa)) = Dc.(1 - m) + Dc.Sa.m
  //
  // [DstAtop XRGBxXRGB] ~= [Dst]
  //   Dc'  = Dc
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo dst_atop(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? clear(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kXRGB32 ? dst_over(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? dst_over(d, s) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? dst_in(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? clear(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? dst_copy(d, s) :

           d == Fmt::kA8 ? src_copy(d, s) :

           make_op(CompOpExt::kDstAtop, d, s);
  }

  // Xor
  // ---
  //
  // [Xor PRGBxPRGB]
  //   Dca' = Dca.(1 - Sa) + Sca.(1 - Da)    Dca' = Dca.(1 - Sa.m) + Sca.m.(1 - Da)
  //   Da'  = Da .(1 - Sa) + Sa .(1 - Da)    Da'  = Da .(1 - Sa.m) + Sa .m.(1 - Da)
  //
  // [Xor PRGBxXRGB] ~= [SrcOut PRGBxPRGB]
  //   Dca' = Sca.(1 - Da)                   Dca' = Sca.m.(1 - Da) + Dca.(1 - m)
  //   Da'  = 1  .(1 - Da)                   Da'  = 1  .m.(1 - Da) + Da .(1 - m)
  //
  // [Xor XRGBxPRGB] ~= [DstOut XRGBxPRGB]
  //   Dc'  = Dc.(1 - Sa)                    Dc'  = Dc.(1 - Sa.m)
  //
  // [Xor XRGBxXRGB] ~= [Clear XRGBxPRGB]
  //   Dc'  = 0                              Dc'  = Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo xor_(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kXRGB32 ? src_out(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? src_out(d, s) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? dst_out(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? clear(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? clear(d, s) :

           d == Fmt::kA8 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kA8 && s == Fmt::kXRGB32 ? alpha_inv(d, Fmt::kXRGB32) :
           d == Fmt::kA8 && s == Fmt::kFRGB32 ? alpha_inv(d, Fmt::kXRGB32) :

           make_op(CompOpExt::kXor, d, s);
  }

  // Plus
  // ----
  //
  // [Plus PRGBxPRGB]
  //   Dca' = Clamp(Dca + Sca)               Dca' = Clamp(Dca + Sca.m)
  //   Da'  = Clamp(Da  + Sa )               Da'  = Clamp(Da  + Sa .m)
  //
  // [Plus PRGBxXRGB] ~= [Plus PRGBxPRGB]
  //   Dca' = Clamp(Dca + Sc)                Dca' = Clamp(Dca + Sc.m)
  //   Da'  = Clamp(Da  + 1 )                Da'  = Clamp(Da  + 1 .m)
  //
  // [Plus XRGBxPRGB] ~= [Plus PRGBxPRGB]
  //   Dc'  = Clamp(Dc + Sca)                Dc'  = Clamp(Dc + Sca.m)
  //
  // [Plus XRGBxXRGB] ~= [Plus PRGBxPRGB]
  //   Dc'  = Clamp(Dc + Sc)                 Dc'  = Clamp(Dc + Sc.m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo plus(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? plus(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? plus(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? plus(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? plus(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kA8 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kA8 && s == Fmt::kXRGB32 ? opaque_alpha(CompOpExt::kPlus, d, Fmt::kPRGB32) :
           d == Fmt::kA8 && s == Fmt::kFRGB32 ? opaque_alpha(CompOpExt::kPlus, d, Fmt::kPRGB32) :

           make_op(CompOpExt::kPlus, d, s);
  }

  // Minus
  // -----
  //
  // [Minus PRGBxPRGB]
  //   Dca' = Clamp(Dca - Sca)               Dca' = Clamp(Dca - Sca).m + Dca.(1 - m)
  //   Da'  = Da + Sa.(1 - Da)               Da'  = Da + Sa.m(1 - Da)
  //
  // [Minus PRGBxXRGB] ~= [Minus PRGBxPRGB]
  //   Dca' = Clamp(Dca - Sc)                Dca' = Clamp(Dca - Sc).m + Dca.(1 - m)
  //   Da'  = Da + 1.(1 - Da) = 1            Da'  = Da + 1.m(1 - Da)
  //
  // [Minus XRGBxPRGB]
  //   Dc'  = Clamp(Dc - Sca)                Dc'  = Clamp(Dc - Sca).m + Dc.(1 - m)
  //
  // [Minus XRGBxXRGB] ~= [Minus XRGBxPRGB]
  //   Dc'  = Clamp(Dc - Sc)                 Dc'  = Clamp(Dc - Sc).m + Dc.(1 - m)
  //
  // NOTE:
  //   `Clamp(a - b)` == `Max(a - b, 0)` == `1 - Min(1 - a + b, 1)`
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo minus(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? minus(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? minus(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? minus(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? minus(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kMinus, d, s);
  }

  // Modulate
  // --------
  //
  // [Modulate PRGBxPRGB]
  //   Dca' = Dca.Sca
  //   Da'  = Da .Sa
  //
  //   Dca' = Dca.(Sca.m + 1 - m)
  //   Da'  = Da .(Sa .m + 1 - m)
  //
  // [Modulate PRGBxXRGB]
  //   Dca' = Dca.Sc
  //   Da'  = Da .1
  //
  //   Dca' = Dca.(Sc.m + 1 - m)
  //   Da'  = Da .(1 .m + 1 - m) = Da
  //
  // [Modulate XRGBxPRGB]
  //   Dc' = Dc.Sca
  //   Dc' = Dc.(Sca.m + 1 - m)
  //
  // [Modulate XRGBxXRGB]
  //   Dc' = Dc.Sc
  //   Dc' = Dc.(Sc.m + 1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo modulate(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? transparent(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? modulate(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? opaque_black(CompOpExt::kSrcCopy, Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? modulate(Fmt::kXRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? modulate(Fmt::kXRGB32, Fmt::kPRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? dst_in(d, s) :

           make_op(CompOpExt::kModulate, d, s);
  }

  // Multiply
  // --------
  //
  // [Multiply PRGBxPRGB]
  //   Dca' = Dca.(Sca + 1 - Sa) + Sca.(1 - Da)
  //   Da'  = Da .(Sa  + 1 - Sa) + Sa .(1 - Da) = Da + Sa.(1 - Da)
  //
  //   Dca' = Dca.(Sca.m + 1 - Sa.m) + Sca.m(1 - Da)
  //   Da'  = Da .(Sa .m + 1 - Sa.m) + Sa .m(1 - Da) = Da + Sa.m(1 - Da)
  //
  // [Multiply PRGBxXRGB]
  //   Dca' = Sc.(Dca + 1 - Da)
  //   Da'  = 1 .(Da  + 1 - Da) = 1
  //
  //   Dca' = Dca.(Sc.m + 1 - 1.m) + Sc.m(1 - Da)
  //   Da'  = Da .(1 .m + 1 - 1.m) + 1 .m(1 - Da) = Da + Sa.m(1 - Da)
  //
  // [Multiply XRGBxPRGB] ~= [Modulate XRGBxPRGB]
  //   Dc'  = Dc.(Sca   + 1 - Sa  )
  //   Dc'  = Dc.(Sca.m + 1 - Sa.m)
  //
  // [Multiply XRGBxXRGB] ~= [Modulate XRGBxXRGB]
  //   Dc'  = Dc.Sc
  //   Dc'  = Dc.(Sc.m + 1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo multiply(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? multiply(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? modulate(Fmt::kXRGB32, Fmt::kXRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? modulate(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? dst_over(d, s) :

           make_op(CompOpExt::kMultiply, d, s);
  }

  // Screen
  // ------
  //
  // [Screen PRGBxPRGB]
  //   Dca' = Dca + Sca.(1 - Dca)
  //   Da'  = Da  + Sa .(1 - Da )
  //
  //   Dca' = Dca + Sca.m.(1 - Dca)
  //   Da'  = Da  + Sa .m.(1 - Da )
  //
  // [Screen PRGBxXRGB] ~= [Screen PRGBxPRGB]
  //   Dca' = Dca + Sc.(1 - Dca)
  //   Da'  = Da  + 1 .(1 - Da )
  //
  //   Dca' = Dca + Sc.m.(1 - Dca)
  //   Da'  = Da  + 1 .m.(1 - Da )
  //
  // [Screen XRGBxPRGB] ~= [Screen PRGBxPRGB]
  //   Dc'  = Dc + Sca  .(1 - Dca)
  //   Dc'  = Dc + Sca.m.(1 - Dca)
  //
  // [Screen PRGBxPRGB] ~= [Screen PRGBxPRGB]
  //   Dc'  = Dc + Sc  .(1 - Dc)
  //   Dc'  = Dc + Sc.m.(1 - Dc)

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo screen(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? screen(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kPRGB32 ? screen(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? screen(Fmt::kPRGB32, Fmt::kPRGB32) :
           d == Fmt::kXRGB32 && s == Fmt::kXRGB32 ? screen(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kScreen, d, s);
  }

  // Overlay
  // -------
  //
  // [Overlay PRGBxPRGB]
  //   if (2.Dca < Da)
  //     Dca' = Dca + Sca - (Dca.Sa + Sca.Da - 2.Sca.Dca)
  //     Da'  = Da  + Sa  - Sa.Da
  //   else
  //     Dca' = Dca + Sca + (Dca.Sa + Sca.Da - 2.Sca.Dca) - Sa.Da
  //     Da'  = Da  + Sa  - Sa.Da
  //
  // [Overlay PRGBxXRGB]
  //   if (2.Dca < Da)
  //     Dca' = Sc.(2.Dca - Da + 1)
  //     Da'  = 1
  //   else
  //     Dca' = 2.Dca - Da - Sc.(1 - (2.Dca - Da))
  //     Da'  = 1
  //
  // [Overlay XRGBxPRGB]
  //   if (2.Dca < Da)
  //     Dc'  = Dc - (Dc.Sa - 2.Sca.Dc)
  //   else
  //     Dc'  = Dc + 2.Sca - Sa + (Dca.Sa - 2.Sca.Dc)
  //
  // [Overlay XRGBxXRGB]
  //   if (2.Dc < 1)
  //     Dc'  = 2.Dc.Sc
  //   else
  //     Dc'  = 2.(Dc + Sc) - 2.Sc.Dc - 1
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo overlay(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? overlay(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? overlay(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kOverlay, d, s);
  }

  // Darken
  // ------
  //
  // [Darken PRGBxPRGB]
  //   Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
  //   Da'  = min(Sa .Da, Da .Sa) + Sa .(1 - Da) + Da .(1 - Sa)
  //        = Sa + Da - Sa.Da
  //
  //   Dca' = min(Sca.m.Da, Dca.Sa.m) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //   Da'  = min(Sa .m.Da, Da .Sa.m) + Sa .m.(1 - Da) + Da .(1 - Sa.m)
  //        = Sa.m + Da - Sa.m.Da
  //
  // [Darken PRGBxXRGB]
  //   Dca' = min(Sc.Da, Dca) + Sc.(1 - Da)
  //   Da'  = min(1 .Da, Da ) + 1 .(1 - Da)
  //        = Sa + Da - Sa.Da
  //
  //   Dca' = min(Sc.m.Da, Dca.m) + Sc.m.(1 - Da) + Dca.(1 - 1.m)
  //   Da'  = min(1 .m.Da, Da .m) + 1 .m.(1 - Da) + Da .(1 - 1.m)
  //        = 1.m + Da - 1.m.Da
  //
  // [Darken XRGBxPRGB]
  //   Dc'  = min(Sca  , Dc.Sa  ) + Dc.(1 - Sa  )
  //   Dc'  = min(Sca.m, Dc.Sa.m) + Dc.(1 - Sa.m)
  //
  // [Darken XRGBxXRGB]
  //   Dc'  = min(Sc, Dc)
  //   Dc'  = min(Sc, Dc).m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo darken(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? darken(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? darken(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? dst_over(d, s) :

           make_op(CompOpExt::kDarken, d, s);
  }

  // Lighten
  // -------
  //
  // [Lighten PRGBxPRGB]
  //   Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
  //   Da'  = max(Sa .Da, Da .Sa) + Sa .(1 - Da) + Da .(1 - Sa)
  //        = Sa + Da - Sa.Da
  //
  //   Dca' = max(Sca.m.Da, Dca.Sa.m) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //   Da'  = max(Sa .m.Da, Da .Sa.m) + Sa .m.(1 - Da) + Da .(1 - Sa.m)
  //        = Sa.m + Da - Sa.m.Da
  //
  // [Lighten PRGBxXRGB]
  //   Dca' = max(Sc.Da, Dca) + Sc.(1 - Da)
  //   Da'  = max(1 .Da, Da ) + 1 .(1 - Da)
  //        = Sa + Da - Sa.Da
  //
  //   Dca' = max(Sc.m.Da, Dca.m) + Sc.m.(1 - Da) + Dca.(1 - 1.m)
  //   Da'  = max(1 .m.Da, Da .m) + 1 .m.(1 - Da) + Da .(1 - 1.m)
  //        = 1.m + Da - 1.m.Da
  //
  // [Lighten XRGBxPRGB]
  //   Dc'  = max(Sca  , Dc.Sa  ) + Dc.(1 - Sa  )
  //   Dc'  = max(Sca.m, Dc.Sa.m) + Dc.(1 - Sa.m)
  //
  // [Lighten XRGBxXRGB]
  //   Dc'  = max(Sc, Dc)
  //   Dc'  = max(Sc, Dc).m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo lighten(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? lighten(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? lighten(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kLighten, d, s);
  }

  // ColorDodge
  // ----------
  //
  // [ColorDodge PRGBxPRGB]
  //   Dca' = min(Dca.Sa.Sa / max(Sa - Sca, 0.001), Da.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
  //   Da'  = Sa + Da - Sa.Da
  //
  //   Dca' = min(Dca.Sa.m.Sa.m / max(Sa.m - Sca.m, 0.001), Da.Sa.m) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //   Da'  = Sa.m + Da - Sa.m.Da
  //
  // [ColorDodge PRGBxXRGB]
  //   Dca' = min(Dca / max(1 - Sc, 0.001), Da) + Sc.(1 - Da)
  //   Da'  = 1
  //
  //   Dca' = min(Dca.1.m.1.m / max(1.m - Sc.m, 0.001), Da.1.m) + Sc.m.(1 - Da) + Dca.(1 - 1.m)
  //   Da'  = 1.m + Da - 1.m.Da
  //
  // [ColorDodge XRGBxPRGB]
  //   Dc'  = min(Dc.Sa  .Sa   / max(Sa   - Sca  , 0.001), Sa)   + Dc.(1 - Sa)
  //   Dc'  = min(Dc.Sa.m.Sa.m / max(Sa.m - Sca.m, 0.001), Sa.m) + Dc.(1 - Sa.m)
  //
  // [ColorDodge XRGBxXRGB]
  //   Dc'  = min(Dc / max(1 - Sc, 0.001), 1)
  //   Dc'  = min(Dc / max(1 - Sc, 0.001), 1).m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo color_dodge(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? color_dodge(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? color_dodge(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kColorDodge, d, s);
  }

  // ColorBurn
  // ---------
  //
  // [ColorBurn PRGBxPRGB]
  //   Dca' = Sa.Da - min(Sa.Da, (Da - Dca).Sa.Sa / max(Sca, 0.001)) + Sca.(1 - Da) + Dca.(1 - Sa)
  //   Da'  = Sa + Da - Sa.Da
  //
  //   Dca' = Sa.m.Da - min(Sa.m.Da, (Da - Dca).Sa.m.Sa.m / max(Sca.m, 0.001)) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //   Da'  = Sa.m + Da - Sa.m.Da
  //
  // [ColorBurn PRGBxXRGB]
  //   Dca' = 1.Da - min(Da, (Da - Dca) / max(Sc, 0.001)) + Sc.(1 - Da)
  //   Da'  = 1
  //
  //   Dca' = m.Da - min(1.m.Da, (Da - Dca).1.m.1.m / max(Sc.m, 0.001)) + Sc.m.(1 - Da) + Dca.(1 - 1.m)
  //   Da'  = 1.m + Da - 1.m.Da
  //
  // [ColorBurn XRGBxPRGB]
  //   Dc'  = Sa   - min(Sa  , (1 - Dc).Sa  .Sa   / max(Sca  , 0.001)) + Dc.(1 - Sa)
  //   Dc'  = Sa.m - min(Sa.m, (1 - Dc).Sa.m.Sa.m / max(Sca.m, 0.001)) + Dc.(1 - Sa.m)
  //
  // [ColorBurn XRGBxXRGB]
  //   Dc'  = (1 - min(1, (1 - Dc) / max(Sc, 0.001)))
  //   Dc'  = (1 - min(1, (1 - Dc) / max(Sc, 0.001))).m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo color_burn(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? color_burn(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? color_burn(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? dst_over(d, s) :

           make_op(CompOpExt::kColorBurn, d, s);
  }

  // LinearBurn
  // ----------
  //
  // [LinearBurn PRGBxPRGB]
  //   Dca' = Clamp(Dca + Sca - Sa.Da)
  //   Da'  = Da + Sa - Sa.Da
  //
  //   Dca' = Clamp(Dca + Sca - Sa.Da).m + Dca.(1 - m)
  //   Da'  = Sa.m.(1 - Da) + Da
  //
  // [LinearBurn PRGBxXRGB]
  //   Dca' = Clamp(Dca + Sc - Da)
  //   Da'  = 1
  //
  //   Dca' = Clamp(Dca + Sc - Da).m + Dca.(1 - m)
  //   Da'  = Da + Sa - Sa.Da
  //
  // [LinearBurn XRGBxPRGB]
  //   Dc'  = Clamp(Dc + Sca - Sa)
  //   Dc'  = Clamp(Dc + Sca - Sa).m + Dc.(1 - m)
  //
  // [LinearBurn XRGBxXRGB]
  //   Dc'  = Clamp(Dc + Sc - 1)
  //   Dc'  = Clamp(Dc + Sc - 1).m + Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo linear_burn(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? linear_burn(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? linear_burn(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? dst_over(d, s) :

           make_op(CompOpExt::kLinearBurn, d, s);
  }

  // LinearLight
  // -----------
  //
  // [LinearLight PRGBxPRGB]
  //   Dca' = min(max(Dca.Sa + 2.Sca.Da - Sa.Da, 0), Sa.Da) + Sca.(1 - Da) + Dca.(1 - Sa)
  //   Da'  = Da + Sa - Sa.Da
  //
  //   Dca' = min(max((Dca.Sa.m + 2.Sca.m.Da - Sa.m.Da), 0), Sa.m.Da) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //   Da'  = Da + Sa.m - Sa.m.Da
  //
  // [LinearLight PRGBxXRGB]
  //   Dca' = min(max((Dca + 2.Sc.Da - Da), 0), Da) + Sc.(1 - Da)
  //   Da'  = 1
  //
  //   Dca' = min(max((Dca.1.m + 2.Sc.m.Da - 1.m.Da), 0), 1.m.Da) + Sc.m.(1 - Da) + Dca.(1 - m)
  //   Da'  = Da + Sa.m - Sa.m.Da
  //
  // [LinearLight XRGBxPRGB]
  //   Dca' = min(max((Dc.Sa   + 2.Sca   - Sa  ), 0), Sa  ) + Dca.(1 - Sa)
  //   Dca' = min(max((Dc.Sa.m + 2.Sca.m - Sa.m), 0), Sa.m) + Dca.(1 - Sa.m)
  //
  // [LinearLight XRGBxXRGB]
  //   Dc'  = min(max((Dc + 2.Sc - 1), 0), 1)
  //   Dc'  = min(max((Dc + 2.Sc - 1), 0), 1).m + Dca.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo linear_light(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? linear_light(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? linear_light(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kLinearLight, d, s);
  }

  // PinLight
  // --------
  //
  // [PinLight PRGBxPRGB]
  //   if 2.Sca <= Sa
  //     Dca' = min(Dca + Sca - Sca.Da, Dca + Sca + Sca.Da - Dca.Sa)
  //     Da'  = min(Da  + Sa  - Sa .Da, Da  + Sa  + Sa .Da - Da .Sa) = Da + Sa.(1 - Da)
  //   else
  //     Dca' = max(Dca + Sca - Sca.Da, Dca + Sca + Sca.Da - Dca.Sa - Da.Sa)
  //     Da'  = max(Da  + Sa  - Sa .Da, Da  + Sa  + Sa .Da - Da .Sa - Da.Sa) = Da + Sa.(1 - Da)
  //
  //   if 2.Sca.m <= Sa.m
  //     Dca' = min(Dca.Sa.m, 2.Sca.m.Da) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //     Da'  = Da + Sa.m.(1 - Da)
  //   else
  //     Dca' = max(Dca.Sa.m, 2.Sca.m.Da - Sa.m.Da) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //     Da'  = Da + Sa.m.(1 - Da)
  //
  // [PinLight PRGBxXRGB]
  //   if 2.Sc <= 1
  //     Dca' = min(Dca, 2.Sc.Da) + Sc.(1 - Da)
  //     Da'  = 1
  //   else
  //     Dca' = max(Dca, 2.Sc.Da - Da) + Sc.(1 - Da)
  //     Da'  = 1
  //
  //   if 2.Sc.m <= 1.m
  //     Dca' = min(Dca.m, 2.Sc.m.Da) + Sc.m.(1 - Da) + Dca.(1 - m)
  //     Da'  = Da + m.(1 - Da)
  //   else
  //     Dca' = max(Dca.m, 2.Sc.m.Da - m.Da) + Sc.m.(1 - Da) + Dc.(1 - m)
  //     Da'  = Da + m.(1 - Da)
  //
  // [PinLight XRGBxPRGB]
  //   if 2.Sca <= Sa
  //     Dc'  = min(Dc.Sa, 2.Sca) + Dc.(1 - Sa)
  //   else
  //     Dc'  = max(Dc.Sa, 2.Sca - Sa) + Dc.(1 - Sa)
  //
  //   if 2.Sca.m <= Sa.m
  //     Dc'  = min(Dc.Sa.m, 2.Sca.m) + Dc.(1 - Sa.m)
  //   else
  //     Dc'  = max(Dc.Sa.m, 2.Sca.m - Sa.m) + Dc.(1 - Sa.m)
  //
  // [PinLight XRGBxXRGB]
  //   if 2.Sc <= 1
  //     Dc'  = min(Dc, 2.Sc)
  //   else
  //     Dc'  = max(Dc, 2.Sc - 1)
  //
  //   if 2.Sca.m <= Sa.m
  //     Dc'  = min(Dc, 2.Sc).m + Dca.(1 - m)
  //   else
  //     Dc'  = max(Dc, 2.Sc - 1).m + Dca.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo pin_light(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? pin_light(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? pin_light(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kPinLight, d, s);
  }

  // HardLight
  // ---------
  //
  // [HardLight PRGBxPRGB]
  //   if (2.Sca <= Sa)
  //     Dca' = 2.Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
  //     Da'  = Sa + Da - Sa.Da
  //   else
  //     Dca' = Sa.Da - 2.(Da - Dca).(Sa - Sca) + Sca.(1 - Da) + Dca.(1 - Sa)
  //     Da'  = Sa + Da - Sa.Da
  //
  //   if (2.Sca.m <= Sa.m)
  //     Dca' = 2.Sca.m.Dca + Sca.m(1 - Da) + Dca.(1 - Sa.m)
  //     Da'  = Sa.m + Da - Sa.m.Da
  //   else
  //     Dca' = Sa.m.Da - 2.(Da - Dca).(Sa.m - Sca.m) + Sca.m.(1 - Da) + Dca.(1 - Sa.m)
  //     Da'  = Sa.m + Da - Sa.m.Da
  //
  // [HardLight PRGBxXRGB]
  //   if (2.Sc <= 1)
  //     Dca' = 2.Sc.Dca + Sc.(1 - Da)
  //     Da'  = 1
  //   else
  //     Dca' = Da - 2.(Da - Dca).(1 - Sc) + Sc.(1 - Da)
  //     Da'  = 1
  //
  //   if (2.Sc.m <= m)
  //     Dca' = 2.Sc.m.Dca + Sc.m(1 - Da) + Dca.(1 - m)
  //     Da'  = Da + m.(1 - Da)
  //   else
  //     Dca' = 1.m.Da - 2.(Da - Dca).((1 - Sc).m) + Sc.m.(1 - Da) + Dca.(1 - m)
  //     Da'  = Da + m.(1 - Da)
  //
  // [HardLight XRGBxPRGB]
  //   if (2.Sca <= Sa)
  //     Dc'  = 2.Sca.Dc + Dc.(1 - Sa)
  //   else
  //     Dc'  = Sa - 2.(1 - Dc).(Sa - Sca) + Dc.(1 - Sa)
  //
  //   if (2.Sca.m <= Sa.m)
  //     Dc'  = 2.Sca.m.Dc + Dc.(1 - Sa.m)
  //   else
  //     Dc'  = Sa.m - 2.(1 - Dc).(Sa.m - Sca.m) + Dc.(1 - Sa.m)
  //
  // [HardLight XRGBxXRGB]
  //   if (2.Sc <= 1)
  //     Dc'  = 2.Sc.Dc
  //   else
  //     Dc'  = 1 - 2.(1 - Dc).(1 - Sc)
  //
  //   if (2.Sc.m <= 1.m)
  //     Dc'  = 2.Sc.Dc.m + Dc.(1 - m)
  //   else
  //     Dc'  = (1 - 2.(1 - Dc).(1 - Sc)).m - Dc.(1 - m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo hard_light(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? hard_light(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? hard_light(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 || s == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kHardLight, d, s);
  }

  // SoftLight
  // ---------
  //
  // [SoftLight PRGBxPRGB]
  //   Dc = Dca/Da
  //   if 2.Sca - Sa <= 0
  //     Dca' = Dca + Sca.(1 - Da) + (2.Sca - Sa).Da.[[              Dc.(1 - Dc)           ]]
  //     Da'  = Da + Sa - Sa.Da
  //   else if 2.Sca - Sa > 0 and 4.Dc <= 1
  //     Dca' = Dca + Sca.(1 - Da) + (2.Sca - Sa).Da.[[ 4.Dc.(4.Dc.Dc + Dc - 4.Dc + 1) - Dc]]
  //     Da'  = Da + Sa - Sa.Da
  //   else
  //     Dca' = Dca + Sca.(1 - Da) + (2.Sca - Sa).Da.[[             sqrt(Dc) - Dc          ]]
  //     Da'  = Da + Sa - Sa.Da
  //
  // [SoftLight XRGBxXRGB]
  //   if 2.Sc <= 1
  //     Dc' = Dc + (2.Sc - 1).[[              Dc.(1 - Dc)           ]]
  //   else if 2.Sc > 1 and 4.Dc <= 1
  //     Dc' = Dc + (2.Sc - 1).[[ 4.Dc.(4.Dc.Dc + Dc - 4.Dc + 1) - Dc]]
  //   else
  //     Dc' = Dc + (2.Sc - 1).[[             sqrt(Dc) - Dc          ]]
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo soft_light(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? soft_light(Fmt::kPRGB32, Fmt::kXRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? soft_light(Fmt::kXRGB32, Fmt::kXRGB32) :

           d == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kSoftLight, d, s);
  }

  // Difference
  // ----------
  //
  // [Difference PRGBxPRGB]
  //   Dca' = Dca + Sca - 2.min(Sca.Da, Dca.Sa)
  //   Da'  = Sa + Da - Sa.Da
  //
  //   Dca' = Dca + Sca.m - 2.min(Sca.m.Da, Dca.Sa.m)
  //   Da'  = Sa.m + Da - Sa.m.Da
  //
  // [Difference PRGBxXRGB]
  //   Dca' = Dca + Sc - 2.min(Sc.Da, Dca)
  //   Da'  = 1
  //
  //   Dca' = Dca + Sc.m - 2.min(Sc.m.Da, Dca)
  //   Da'  = Da + 1.m - m.Da
  //
  // [Difference XRGBxPRGB]
  //   Dc'  = Dc + Sca   - 2.min(Sca  , Dc.Sa)
  //   Dc'  = Dc + Sca.m - 2.min(Sca.m, Dc.Sa.m)
  //
  // [Difference XRGBxXRGB]
  //   Dc'  = Dc + Sc   - 2.min(Sc  , Dc  )
  //   Dc'  = Dc + Sc.m - 2.min(Sc.m, Dc.m)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo difference(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? difference(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? difference(Fmt::kXRGB32, Fmt::kPRGB32) :

           d == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kDifference, d, s);
  }

  // Exclusion
  // ---------
  //
  // [Exclusion PRGBxPRGB]
  //   Dca' = Dca + Sca.(Da - 2.Dca)
  //   Da'  = Da  + Sa - Sa.Da
  //
  //   Dca' = Dca + Sca.m.(Da - 2.Dca)
  //   Da'  = Da  + Sa.m - Sa.m.Da
  //
  // [Exclusion PRGBxXRGB] ~= [Exclusion PRGBxPRGB]
  //   Dca' = Dca + Sc.(Da - 2.Dca)
  //   Da'  = Da  + 1 - 1.Da
  //
  //   Dca' = Dca + Sc.m.(Da - 2.Dca)
  //   Da'  = Da  + 1.m - 1.m.Da
  //
  // [Exclusion XRGBxPRGB]
  //   Dc'  = Dc + Sca  .(1 - 2.Dc)
  //   Dc'  = Dc + Sca.m.(1 - 2.Dc)
  //
  // [Exclusion XRGBxXRGB] ~= [Exclusion XRGBxPRGB]
  //   Dc'  = Dc + Sc  .(1 - 2.Dc)
  //   Dc'  = Dc + Sc.m.(1 - 2.Dc)
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo exclusion(Fmt d, Fmt s) noexcept {
    return d == Fmt::kPRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kPRGB32 && s == Fmt::kFRGB32 ? exclusion(Fmt::kPRGB32, Fmt::kPRGB32) :

           d == Fmt::kXRGB32 && s == Fmt::kZERO32 ? dst_copy(d, s) :
           d == Fmt::kXRGB32 && s == Fmt::kFRGB32 ? exclusion(Fmt::kXRGB32, Fmt::kPRGB32) :

           d == Fmt::kA8 ? src_over(d, s) :

           make_op(CompOpExt::kExclusion, d, s);
  }

  // HACK: MSVC has a problem with code that does multiple ternary operations (? :)
  //       so we had to split it so Blend2D can compile. So please don't be active
  //       here and don't try to join these functions as you would break MSVC builds.
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo simplify_1(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return comp_op == CompOpExt::kSrcCopy     ? src_copy(d, s)     :
           comp_op == CompOpExt::kSrcOver     ? src_over(d, s)     :
           comp_op == CompOpExt::kSrcIn       ? src_in(d, s)       :
           comp_op == CompOpExt::kSrcOut      ? src_out(d, s)      :
           comp_op == CompOpExt::kSrcAtop     ? src_atop(d, s)     :
           comp_op == CompOpExt::kDstCopy     ? dst_copy(d, s)     :
           comp_op == CompOpExt::kDstOver     ? dst_over(d, s)     :
           comp_op == CompOpExt::kDstIn       ? dst_in(d, s)       :
           comp_op == CompOpExt::kDstOut      ? dst_out(d, s)      :
           comp_op == CompOpExt::kDstAtop     ? dst_atop(d, s)     : dst_copy(d, s);
  }

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo simplify_2(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return comp_op == CompOpExt::kXor         ? xor_(d, s)        :
           comp_op == CompOpExt::kClear       ? clear(d, s)       :
           comp_op == CompOpExt::kPlus        ? plus(d, s)        :
           comp_op == CompOpExt::kMinus       ? minus(d, s)       :
           comp_op == CompOpExt::kModulate    ? modulate(d, s)    :
           comp_op == CompOpExt::kMultiply    ? multiply(d, s)    :
           comp_op == CompOpExt::kScreen      ? screen(d, s)      :
           comp_op == CompOpExt::kOverlay     ? overlay(d, s)     :
           comp_op == CompOpExt::kDarken      ? darken(d, s)      :
           comp_op == CompOpExt::kLighten     ? lighten(d, s)     : simplify_1(comp_op, d, s);
  }

  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo simplify_3(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return comp_op == CompOpExt::kColorDodge  ? color_dodge(d, s)  :
           comp_op == CompOpExt::kColorBurn   ? color_burn(d, s)   :
           comp_op == CompOpExt::kLinearBurn  ? linear_burn(d, s)  :
           comp_op == CompOpExt::kLinearLight ? linear_light(d, s) :
           comp_op == CompOpExt::kPinLight    ? pin_light(d, s)    :
           comp_op == CompOpExt::kHardLight   ? hard_light(d, s)   :
           comp_op == CompOpExt::kSoftLight   ? soft_light(d, s)   :
           comp_op == CompOpExt::kDifference  ? difference(d, s)  :
           comp_op == CompOpExt::kExclusion   ? exclusion(d, s)   :

           // Extended operators, only used to simplify others.
           comp_op == CompOpExt::kAlphaInv    ? alpha_inv(d, s)    : simplify_2(comp_op, d, s);
  }

  // Just dispatches to the respective composition operator.
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo simplify(CompOpExt comp_op, Fmt d, Fmt s) noexcept {
    return simplify_3(comp_op, d, s);
  }
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_COMPOPSIMPLIFYIMPL_P_H_INCLUDED
