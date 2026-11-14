// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/compopsimplifyimpl_p.h>
#include <blend2d/pipeline/reference/compopgeneric_p.h>
#include <blend2d/pipeline/reference/fillgeneric_p.h>
#include <blend2d/pipeline/reference/fixedpiperuntime_p.h>
#include <blend2d/support/wrap_p.h>

namespace bl::Pipeline {

// FixedPipelineRuntime - Globals
// ==============================

Wrap<PipeStaticRuntime> PipeStaticRuntime::_global;

// FixedPipelineRuntime - Get
// ==========================

template<CompOpExt kCompOp, FormatExt kDstFomat, FormatExt kSrcFomat, FetchType kFetchType>
struct CompOpValid {
  static constexpr bool kCompOpChanged =
    CompOpSimplifyInfoImpl::simplify(kCompOp, kDstFomat, kSrcFomat).comp_op() != kCompOp;

  static constexpr bool kDstFormatChanged =
    CompOpSimplifyInfoImpl::simplify(kCompOp, kDstFomat, kSrcFomat).dst_format() != kDstFomat;

  static constexpr bool kFetchTypeChanged =
    kFetchType != FetchType::kSolid &&
    CompOpSimplifyInfoImpl::simplify(kCompOp, kDstFomat, kSrcFomat).solid_id() != CompOpSolidId::kNone;

  static constexpr bool kValid = !kCompOpChanged && !kFetchTypeChanged;
};

struct FillSolidFuncTable {
  static inline constexpr uint32_t kFillTypeCount = uint32_t(FillType::_kMaxValue);

  FillFunc funcs[kFillTypeCount];
};

struct FillPatternFuncTable {
  static inline constexpr uint32_t kFillTypeCount = uint32_t(FillType::_kMaxValue);
  static inline constexpr uint32_t kPatternTypeCount = uint32_t(FetchType::kPatternAnyLast) - uint32_t(FetchType::kPatternAnyFirst) + 1u;

  FillFunc funcs[kFillTypeCount * kPatternTypeCount];
};

struct FillGradientFuncTable {
  static inline constexpr uint32_t kFillTypeCount = uint32_t(FillType::_kMaxValue);
  static inline constexpr uint32_t kGradientTypeCount = uint32_t(FetchType::kGradientAnyLast) - uint32_t(FetchType::kGradientAnyFirst) + 1u;

  FillFunc funcs[kFillTypeCount * kGradientTypeCount];
};

template<FillType kFillType, FormatExt kDstFormat, uint32_t kDstBPP, typename CompOp>
static constexpr FillFunc get_fill_solid_func() noexcept {
  return Reference::FillDispatch<
    kFillType,
    Reference::CompOp_Base<
      CompOp,
      typename CompOp::PixelType,
      typename Reference::FetchSolid<typename CompOp::PixelType>,
      kDstBPP
    >
  >::Fill::fill_func;
}

template<FillType kFillType, FormatExt kDstFormat, uint32_t kDstBPP, typename CompOp, FetchType kFetchType, FormatExt kSrcFormat>
static constexpr FillFunc get_fill_pattern_func() noexcept {
  return CompOpValid<CompOpExt(CompOp::kCompOp), kDstFormat, kSrcFormat, kFetchType>::kValid
    ? Reference::FillDispatch<
        kFillType,
        Reference::CompOp_Base<
          CompOp,
          typename CompOp::PixelType,
          typename Reference::FetchPatternDispatch<kFetchType, typename CompOp::PixelType, kSrcFormat>::Fetch,
          kDstBPP
        >
      >::Fill::fill_func
    : nullptr;
}

template<FillType kFillType, FormatExt kDstFormat, uint32_t kDstBPP, typename CompOp, FetchType kFetchType>
static constexpr FillFunc get_fill_gradient_func() noexcept {
  return CompOpValid<CompOpExt(CompOp::kCompOp), kDstFormat, FormatExt::kPRGB32, kFetchType>::kValid
    ? Reference::FillDispatch<
        kFillType,
        Reference::CompOp_Base<
          CompOp,
          typename CompOp::PixelType,
          typename Reference::FetchGradientDispatch<kFetchType, typename CompOp::PixelType>::Fetch,
          kDstBPP
        >
      >::Fill::fill_func
    : nullptr;
}

template<FormatExt kDstFormat, uint32_t kDstBPP, typename CompOp>
static constexpr FillSolidFuncTable get_fill_solid_func_table() noexcept {
  return FillSolidFuncTable{{
    get_fill_solid_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp>(),
    get_fill_solid_func<FillType::kMask, kDstFormat, kDstBPP, CompOp>(),
    get_fill_solid_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp>(),
  }};
}

template<FormatExt kDstFormat, uint32_t kDstBPP, typename CompOp, FormatExt kSrcFormat>
static constexpr FillPatternFuncTable get_fill_pattern_func_table() noexcept {
  return FillPatternFuncTable{{
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedBlit  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedPad   , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedRepeat, kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedRoR   , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxPad        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxRoR        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFyPad        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFyRoR        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxFyPad      , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxFyRoR      , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineNNAny  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineNNOpt  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineBIAny  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineBIOpt  , kSrcFormat>(),

    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedBlit  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedPad   , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedRepeat, kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedRoR   , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxPad        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxRoR        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFyPad        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFyRoR        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxFyPad      , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxFyRoR      , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineNNAny  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineNNOpt  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineBIAny  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineBIOpt  , kSrcFormat>(),

    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedBlit  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedPad   , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedRepeat, kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAlignedRoR   , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxPad        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxRoR        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFyPad        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFyRoR        , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxFyPad      , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternFxFyRoR      , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineNNAny  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineNNOpt  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineBIAny  , kSrcFormat>(),
    get_fill_pattern_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kPatternAffineBIOpt  , kSrcFormat>()
  }};
}

template<FormatExt kDstFormat, uint32_t kDstBPP, typename CompOp>
static constexpr FillGradientFuncTable get_fill_gradient_func_table() noexcept {
  return FillGradientFuncTable{{
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearNNPad    >(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearNNRoR    >(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearDitherPad>(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearDitherRoR>(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialNNPad    >(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialNNRoR    >(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialDitherPad>(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialDitherRoR>(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientConicNN        >(),
    get_fill_gradient_func<FillType::kBoxA, kDstFormat, kDstBPP, CompOp, FetchType::kGradientConicDither    >(),

    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearNNPad    >(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearNNRoR    >(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearDitherPad>(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearDitherRoR>(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialNNPad    >(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialNNRoR    >(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialDitherPad>(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialDitherRoR>(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientConicNN        >(),
    get_fill_gradient_func<FillType::kMask, kDstFormat, kDstBPP, CompOp, FetchType::kGradientConicDither    >(),

    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearNNPad    >(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearNNRoR    >(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearDitherPad>(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientLinearDitherRoR>(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialNNPad    >(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialNNRoR    >(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialDitherPad>(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientRadialDitherRoR>(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientConicNN        >(),
    get_fill_gradient_func<FillType::kAnalytic, kDstFormat, kDstBPP, CompOp, FetchType::kGradientConicDither    >()
  }};
}

static const constexpr FillSolidFuncTable prgb32_fill_solid_funcs[2] = {
  get_fill_solid_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcOver_Op<Reference::Pixel::P32_A8R8G8B8>>(),
  get_fill_solid_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P32_A8R8G8B8>>()
};

static const constexpr FillPatternFuncTable prgb32_fill_pattern_prgb32_funcs[2] = {
  get_fill_pattern_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcOver_Op<Reference::Pixel::P32_A8R8G8B8>, FormatExt::kPRGB32>(),
  get_fill_pattern_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P32_A8R8G8B8>, FormatExt::kPRGB32>()
};

static const constexpr FillPatternFuncTable prgb32_fill_pattern_xrgb32_funcs[2] = {
  get_fill_pattern_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcOver_Op<Reference::Pixel::P32_A8R8G8B8>, FormatExt::kXRGB32>(),
  get_fill_pattern_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P32_A8R8G8B8>, FormatExt::kXRGB32>()
};

static const constexpr FillPatternFuncTable prgb32_fill_pattern_a8_funcs[2] = {
  get_fill_pattern_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcOver_Op<Reference::Pixel::P32_A8R8G8B8>, FormatExt::kA8>(),
  get_fill_pattern_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P32_A8R8G8B8>, FormatExt::kA8>()
};

static const constexpr FillGradientFuncTable prgb32_fill_gradient_funcs[2] = {
  get_fill_gradient_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcOver_Op<Reference::Pixel::P32_A8R8G8B8>>(),
  get_fill_gradient_func_table<FormatExt::kPRGB32, 4, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P32_A8R8G8B8>>()
};

static const constexpr FillSolidFuncTable a8_fill_solid_funcs[2] = {
  get_fill_solid_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcOver_Op<Reference::Pixel::P8_Alpha>>(),
  get_fill_solid_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P8_Alpha>>()
};

static const constexpr FillPatternFuncTable a8_fill_pattern_prgb32_funcs[2] = {
  get_fill_pattern_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcOver_Op<Reference::Pixel::P8_Alpha>, FormatExt::kPRGB32>(),
  get_fill_pattern_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P8_Alpha>, FormatExt::kPRGB32>()
};

static const constexpr FillPatternFuncTable a8_fill_pattern_a8_funcs[2] = {
  get_fill_pattern_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcOver_Op<Reference::Pixel::P8_Alpha>, FormatExt::kA8>(),
  get_fill_pattern_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P8_Alpha>, FormatExt::kA8>()
};

static const constexpr FillGradientFuncTable a8_fill_gradient_funcs[2] = {
  get_fill_gradient_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcOver_Op<Reference::Pixel::P8_Alpha>>(),
  get_fill_gradient_func_table<FormatExt::kA8, 1, Reference::CompOp_SrcCopy_Op<Reference::Pixel::P8_Alpha>>()
};

static BLResult BL_CDECL bl_pipe_gen_runtime_get(PipeRuntime* self_, uint32_t signature, DispatchData* dispatch_data, PipeLookupCache* cache) noexcept {
  bl_unused(self_);

  Signature s{signature};
  CompOpExt comp_op = s.comp_op();
  FetchType fetch_type = s.fetch_type();
  uint32_t fill_type_idx = uint32_t(s.fill_type()) - 1u;

  FillFunc fill_func = nullptr;
  FetchFunc fetch_func = nullptr;

  if (comp_op == CompOpExt::kSrcCopy || comp_op == CompOpExt::kSrcOver) {
    uint32_t comp_op_index = uint32_t(comp_op);
    switch (s.dst_format()) {
      case FormatExt::kPRGB32:
      case FormatExt::kXRGB32: {
        if (fetch_type == FetchType::kSolid) {
          fill_func = prgb32_fill_solid_funcs[comp_op_index].funcs[fill_type_idx];
        }
        else if (fetch_type >= FetchType::kPatternAnyFirst && fetch_type <= FetchType::kPatternAnyLast) {
          uint32_t pattern_index = uint32_t(fetch_type) - uint32_t(FetchType::kPatternAnyFirst);
          switch (s.src_format()) {
            case FormatExt::kPRGB32:
              fill_func = prgb32_fill_pattern_prgb32_funcs[comp_op_index].funcs[fill_type_idx * FillPatternFuncTable::kPatternTypeCount + pattern_index];
              break;
            case FormatExt::kXRGB32:
              fill_func = prgb32_fill_pattern_xrgb32_funcs[comp_op_index].funcs[fill_type_idx * FillPatternFuncTable::kPatternTypeCount + pattern_index];
              break;
            case FormatExt::kA8:
              fill_func = prgb32_fill_pattern_a8_funcs[comp_op_index].funcs[fill_type_idx * FillPatternFuncTable::kPatternTypeCount + pattern_index];
              break;
            default:
              break;
          }
        }
        else if (fetch_type >= FetchType::kGradientAnyFirst && fetch_type <= FetchType::kGradientAnyLast) {
          uint32_t gradient_index = uint32_t(fetch_type) - uint32_t(FetchType::kGradientAnyFirst);
          fill_func = prgb32_fill_gradient_funcs[comp_op_index].funcs[fill_type_idx * FillGradientFuncTable::kGradientTypeCount + gradient_index];
        }
        break;
      }

      case FormatExt::kA8: {
        if (fetch_type == FetchType::kSolid) {
          fill_func = a8_fill_solid_funcs[comp_op_index].funcs[fill_type_idx];
        }
        else if (fetch_type >= FetchType::kPatternAnyFirst && fetch_type <= FetchType::kPatternAnyLast) {
          uint32_t pattern_index = uint32_t(fetch_type) - uint32_t(FetchType::kPatternAnyFirst);
          switch (s.src_format()) {
            case FormatExt::kPRGB32:
              fill_func = a8_fill_pattern_prgb32_funcs[comp_op_index].funcs[fill_type_idx * FillPatternFuncTable::kPatternTypeCount + pattern_index];
              break;
            case FormatExt::kA8:
              fill_func = a8_fill_pattern_a8_funcs[comp_op_index].funcs[fill_type_idx * FillPatternFuncTable::kPatternTypeCount + pattern_index];
              break;
            default:
              break;
          }
        }
        else if (fetch_type >= FetchType::kGradientAnyFirst && fetch_type <= FetchType::kGradientAnyLast) {
          uint32_t gradient_index = uint32_t(fetch_type) - uint32_t(FetchType::kGradientAnyFirst);
          fill_func = a8_fill_gradient_funcs[comp_op_index].funcs[fill_type_idx * FillGradientFuncTable::kGradientTypeCount + gradient_index];
        }
        break;
      }

      default:
        break;
    }
  }

  if (!fill_func)
    return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);

  dispatch_data->init(fill_func, fetch_func);

  if (cache)
    cache->store(signature, dispatch_data);

  return BL_SUCCESS;
}

PipeStaticRuntime::PipeStaticRuntime() noexcept {
  // Setup the `PipeRuntime` base.
  _runtime_type = PipeRuntimeType::kStatic;
  _runtime_flags = PipeRuntimeFlags::kNone;
  _runtime_size = uint16_t(sizeof(PipeStaticRuntime));

  // PipeStaticRuntime destructor - never called.
  _destroy = nullptr;

  // PipeStaticRuntime interface - used by the rendering context and `PipeProvider`.
  _funcs.test = bl_pipe_gen_runtime_get;
  _funcs.get = bl_pipe_gen_runtime_get;
}

PipeStaticRuntime::~PipeStaticRuntime() noexcept {}

} // {bl::Pipeline}

// FixedPipelineRuntime - Runtime Registration
// ===========================================

void bl_static_pipeline_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  bl::Pipeline::PipeStaticRuntime::_global.init();
}
