// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/compoppart_p.h>
#include <blend2d/pipeline/jit/fetchpart_p.h>
#include <blend2d/pipeline/jit/fetchgradientpart_p.h>
#include <blend2d/pipeline/jit/fetchpixelptrpart_p.h>
#include <blend2d/pipeline/jit/fetchsolidpart_p.h>
#include <blend2d/pipeline/jit/fetchpatternpart_p.h>
#include <blend2d/pipeline/jit/fillpart_p.h>
#include <blend2d/pipeline/jit/pipecomposer_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::PipeComposer - Construction & Destruction
// =======================================================

// TODO: [JIT] There should be a getter on asmjit side that will return the `Arena` instance that can
// be used for these kind of purposes. It doesn't make sense to create another Arena for our use-case.
PipeComposer::PipeComposer(PipeCompiler& pc) noexcept
  : _pc(pc),
    _arena(pc.cc->_builder_arena) {}

// bl::Pipeline::PipeComposer - Pipeline Parts Construction
// ========================================================

FillPart* PipeComposer::new_fill_part(FillType fill_type, FetchPart* dst_part, CompOpPart* comp_op_part) noexcept {
  if (fill_type == FillType::kBoxA)
    return new_part_t<FillBoxAPart>(dst_part->as<FetchPixelPtrPart>(), comp_op_part);

  if (fill_type == FillType::kMask)
    return new_part_t<FillMaskPart>(dst_part->as<FetchPixelPtrPart>(), comp_op_part);

  if (fill_type == FillType::kAnalytic)
    return new_part_t<FillAnalyticPart>(dst_part->as<FetchPixelPtrPart>(), comp_op_part);

  return nullptr;
}

FetchPart* PipeComposer::new_fetch_part(FetchType fetch_type, FormatExt format) noexcept {
  if (fetch_type == FetchType::kSolid)
    return new_part_t<FetchSolidPart>(format);

  if (fetch_type >= FetchType::kGradientLinearFirst && fetch_type <= FetchType::kGradientLinearLast)
    return new_part_t<FetchLinearGradientPart>(fetch_type, format);

  if (fetch_type >= FetchType::kGradientRadialFirst && fetch_type <= FetchType::kGradientRadialLast)
    return new_part_t<FetchRadialGradientPart>(fetch_type, format);

  if (fetch_type >= FetchType::kGradientConicFirst && fetch_type <= FetchType::kGradientConicLast)
    return new_part_t<FetchConicGradientPart>(fetch_type, format);

  if (fetch_type >= FetchType::kPatternSimpleFirst && fetch_type <= FetchType::kPatternSimpleLast)
    return new_part_t<FetchSimplePatternPart>(fetch_type, format);

  if (fetch_type >= FetchType::kPatternAffineFirst && fetch_type <= FetchType::kPatternAffineLast)
    return new_part_t<FetchAffinePatternPart>(fetch_type, format);

  if (fetch_type == FetchType::kPixelPtr)
    return new_part_t<FetchPixelPtrPart>(fetch_type, format);

  return nullptr;
}

CompOpPart* PipeComposer::new_comp_op_part(CompOpExt comp_op, FetchPart* dst_part, FetchPart* src_part) noexcept {
  return new_part_t<CompOpPart>(comp_op, dst_part, src_part);
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT