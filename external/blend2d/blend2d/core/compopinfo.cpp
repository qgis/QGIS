// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/compopinfo_p.h>
#include <blend2d/core/compopsimplifyimpl_p.h>

namespace bl {

struct CompOpInfoGen {
  #define F(VALUE) CompOpFlags::VALUE

  static constexpr CompOpInfo value(size_t op) noexcept {
    return CompOpInfo { uint16_t(
      op == size_t(CompOpExt::kSrcOver)     ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kSrcCopy)     ? F(kTypeB) | F(kNone) | F(kNone) | F(kSc)   | F(kSa)   | F(kNone)       | F(kNone)       :
      op == size_t(CompOpExt::kSrcIn)       ? F(kTypeB) | F(kNone) | F(kDa)   | F(kSc)   | F(kSa)   | F(kNopIfDaEq0) | F(kNone)       :
      op == size_t(CompOpExt::kSrcOut)      ? F(kTypeB) | F(kNone) | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNone)       :
      op == size_t(CompOpExt::kSrcAtop)     ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNopIfDaEq0) | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kDstOver)     ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNopIfDaEq1) | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kDstCopy)     ? F(kTypeC) | F(kDc)   | F(kDa)   | F(kNone) | F(kNone) | F(kNop)        | F(kNop)        :
      op == size_t(CompOpExt::kDstIn)       ? F(kTypeB) | F(kDc)   | F(kDa)   | F(kNone) | F(kSa)   | F(kNone)       | F(kNopIfSaEq1) :
      op == size_t(CompOpExt::kDstOut)      ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kNone) | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kDstAtop)     ? F(kTypeB) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNone)       :
      op == size_t(CompOpExt::kXor)         ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kClear)       ? F(kTypeC) | F(kNone) | F(kNone) | F(kNone) | F(kNone) | F(kNopIfDaEq0) | F(kNone)       :

      op == size_t(CompOpExt::kPlus)        ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kMinus)       ? F(kTypeC) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kModulate)    ? F(kTypeB) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNopIfDaEq0) | F(kNone)       :
      op == size_t(CompOpExt::kMultiply)    ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNopIfDaEq0) | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kScreen)      ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kOverlay)     ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kDarken)      ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kLighten)     ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kColorDodge)  ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kColorBurn)   ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kLinearBurn)  ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kLinearLight) ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kPinLight)    ? F(kTypeC) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kHardLight)   ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kSoftLight)   ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kDifference)  ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :
      op == size_t(CompOpExt::kExclusion)   ? F(kTypeA) | F(kDc)   | F(kDa)   | F(kSc)   | F(kSa)   | F(kNone)       | F(kNopIfSaEq0) :

      op == size_t(CompOpExt::kAlphaInv)    ? F(kTypeC) | F(kNone) | F(kDa)   | F(kNone) | F(kNone) | F(kNone)       | F(kNone)       : F(kNone)
    ) };
  }

  #undef F
};

const LookupTable<CompOpInfo, uint32_t(CompOpExt::kMaxValue) + 1> comp_op_info_table =
  make_lookup_table<CompOpInfo, uint32_t(CompOpExt::kMaxValue) + 1, CompOpInfoGen>();

struct CompOpSimplifyInfoGen {
  // Function called by the table generator, decomposes the parameters and passes them to the simplifier.
  static constexpr CompOpSimplifyInfo value(size_t index) noexcept {
    return CompOpSimplifyInfoImpl::simplify(
      CompOpExt(uint32_t((index / kFormatExtCount)) % kCompOpExtCount),
      FormatExt(index / (kCompOpExtCount * kFormatExtCount)),
      FormatExt(index % kFormatExtCount));
  }
};

template<bl::FormatExt Dst>
struct CompOpSimplifyInfoRecordSetGen {
  // Function called by the table generator, decomposes the parameters and passes them to the simplifier.
  static constexpr CompOpSimplifyInfo value(size_t index) noexcept {
    return CompOpSimplifyInfoImpl::simplify(CompOpExt(index / kFormatExtCount), Dst, FormatExt(index % kFormatExtCount));
  }
};

static_assert(BL_FORMAT_MAX_VALUE == 3u, "Don't forget to add new formats to comp_op_simplify_info_table");

// HACK: MSVC doesn't honor constexpr functions and sometimes outputs initialization
//       code even when the expression can be calculated at compile time. To fix this
//       we go through an additional constexpr to force the compiler to always generate
//       our lookup tables at compile time.
//
// Additionally, if there is a mistake leading to recursion the compiler would catch it
// at compile-time instead of hitting it at runtime during initialization.
static constexpr const CompOpSimplifyInfoTable comp_op_simplify_info_table_ = {{
  make_lookup_table<CompOpSimplifyInfo, kCompOpSimplifyRecordSize, CompOpSimplifyInfoRecordSetGen<FormatExt(0)>>(),
  make_lookup_table<CompOpSimplifyInfo, kCompOpSimplifyRecordSize, CompOpSimplifyInfoRecordSetGen<FormatExt(1)>>(),
  make_lookup_table<CompOpSimplifyInfo, kCompOpSimplifyRecordSize, CompOpSimplifyInfoRecordSetGen<FormatExt(2)>>(),
  make_lookup_table<CompOpSimplifyInfo, kCompOpSimplifyRecordSize, CompOpSimplifyInfoRecordSetGen<FormatExt(3)>>()
}};
const CompOpSimplifyInfoTable comp_op_simplify_info_table = comp_op_simplify_info_table_;

} // {bl}
