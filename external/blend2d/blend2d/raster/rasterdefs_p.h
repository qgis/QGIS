// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RASTERDEFS_P_H_INCLUDED
#define BLEND2D_RASTER_RASTERDEFS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/compop_p.h>
#include <blend2d/core/context_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/pattern_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/pipeline/pipedefs_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

class BLRasterContextImpl;

namespace bl::RasterEngine {

struct RenderFetchData;
struct StyleData;
class WorkData;

//! Represents the number of bits per a single color channel, rounded up to 8 or 16.
enum class TargetDepth : uint32_t {
  kU8,
  kU16,
  kF32
};

//! Rendering mode.
enum class RenderingMode : uint32_t {
  kSync = 0,
  kAsync = 1
};

//! Rendering context flags that represent the following states:
//!
//!  - no flags - no flags describe a no-operation conditions used to quickly reject render calls.
//!
//!  - fetch flags - fetch flags describe a fetch data state.
//!
//!  - shared flags - describe shared states used by multithreaded rendering. Some functions in the rendering context
//!    don't care whether the rendering is synchronous or asynchronous and just clear the `kSharedState` flags in case
//!    that something shared was changed. Then before a command is enqueued such flags are checked and the shared state
//!    is created when necessary. If a shared state flag is set then the engine would reuse the state if possible.
//!
//!  - informative flags - other flags like integral translation.
//!
//!  - weak state flags - book keeping flags that describe which states must be saved to `SavedState` before they can
//!    be modified. Used by `save()`, `restore()`, and by all other functions that manipulate the context state. When
//!    the rendering context is created, all WEAK state flags are false, which means that there is no previous state
//!    where to save.
//!
//!  - queue flags - TODO
enum class ContextFlags : uint32_t {
  //! No flags set.
  kNoFlagsSet = 0u,

  //! Reserved for solic color override - set during render call dispatching.
  kSolidOverride = 0x00000003u,

  //! The operation is always NOP regardless of the composition operator, source, etc...
  //!
  //! \note When `CompOpSolidId::kAlwaysNop` is combined with ContextFlags the 3 LSB bits would be modified. This
  //! would always be set when the composition is simplified to \ref BL_COMP_OP_DST_COPY. Since this flag is part of
  //! 'no-' flags family, we don't have to do anything else to reject render calls that would implicitly or explicitly
  //! have this flag set during the call invocation.
  kNoOperation = 0x00000004u,

  //! Global alpha is zero.
  kNoGlobalAlpha = 0x00000008u,

  //! Start of fill/stroke 'no' alpha.
  kNoBaseAlpha = 0x00000010u,
  //! Fill alpha is zero.
  kNoFillAlpha = kNoBaseAlpha << 0,
  //! Stroke alpha is zero.
  kNoStrokeAlpha = kNoBaseAlpha << 1,
  //! Start of fill/stroke 'no' flags.
  kNoBaseStyle = 0x00000040u,
  //! Fill style is invalid or none.
  kNoFillStyle = kNoBaseStyle << 0,
  //! Stroke style is invalid or none.
  kNoStrokeStyle = kNoBaseStyle << 1,

  kNoFillAndStrokeAlpha = kNoFillAlpha | kNoStrokeAlpha,
  kNoFillAndStrokeStyle = kNoFillStyle | kNoStrokeStyle,

  //! User clip-rect is empty.
  kNoClipRect = 0x00000100u,
  //! User clip-mask is empty.
  kNoClipMask = 0x00000200u,

  //! Meta transform is invalid.
  kNoMetaTransform = 0x00000400u,
  //! User transform is invalid.
  kNoUserTransform = 0x00000800u,

  //! One or more stroke parameter is invalid.
  //!
  //! TODO: [Rendering Context] Never set!
  kNoStrokeOptions = 0x00001000u,

  //! All 'no' flags.
  kNoAllFlags = 0x00003FFFu,

  //! A combination of no-clear flags that would prevent a clear all operation.
  kNoClearOpAll = kSolidOverride | kNoClipRect | kNoClipMask | kNoOperation,
  //! A combination of no-clear flags that would prevent a clear geometry operation.
  kNoClearOp = kNoClearOpAll | kNoMetaTransform | kNoUserTransform,

  //! A combination of no-fill flags used by a fill-all render call with an explicit style.
  kNoFillOpAllExplicit = kSolidOverride | kNoGlobalAlpha | kNoFillAlpha | kNoClipRect | kNoClipMask | kNoOperation,
  //! A combination of no-fill flags used by a fill-all render call with a default style.
  kNoFillOpAllImplicit = kNoFillOpAllExplicit | kNoFillStyle,
  //! A combination of no-fill flags used by a geometry render call with an explicit style.
  kNoFillOpExplicit = kNoFillOpAllExplicit | kNoMetaTransform | kNoUserTransform,
  //! A combination of no-fill flags used by a geometry render call with a default style.
  kNoFillOpImplicit = kNoFillOpExplicit | kNoFillStyle,

  //! A combination of no-stroke flags used by a geometry render call with an explicit style.
  kNoStrokeOpExplicit = kSolidOverride | kNoGlobalAlpha | kNoStrokeAlpha | kNoStrokeOptions | kNoClipRect | kNoClipMask | kNoOperation | kNoMetaTransform | kNoUserTransform,
  //! A combination of no-stroke flags used by a geometry render call with a default style.
  kNoStrokeOpImplicit = kNoStrokeOpExplicit | kNoStrokeStyle,

  //! A combination of no-blit flags.
  kNoBlitFlags = kSolidOverride | kNoGlobalAlpha | kNoClipRect | kNoClipMask | kNoMetaTransform | kNoUserTransform | kNoOperation,

  //! Start of non-solid fill/stroke flag
  kFetchDataBase = 0x00004000u,
  //! Fill style is not solid nor none.
  kFetchDataFill = kFetchDataBase << 0,
  //! Stroke style is not solid nor none.
  kFetchDataStroke = kFetchDataBase << 1,
  //! Invalid style (only used for error checking).
  kFetchDataInvalidStyle = kFetchDataBase << 2,

  kFetchDataFillAndStroke = kFetchDataFill | kFetchDataStroke,

  //! Configuration (tolerance).
  kWeakStateConfig = 0x001000000,
  //! Clip state.
  kWeakStateClip = 0x00200000u,
  //! Start of fill/stroke style flags.
  kWeakStateBaseStyle = 0x00400000u,
  //! Fill style state.
  kWeakStateFillStyle = kWeakStateBaseStyle << 0,
  //! Stroke style state.
  kWeakStateStrokeStyle = kWeakStateBaseStyle << 1,
  //! Invalid style state (only used for error checking).
  kWeakStateInvalidStyle = kWeakStateBaseStyle << 2,
  //! Stroke params state.
  kWeakStateStrokeOptions = 0x02000000u,
  //! Meta transform state.
  kWeakStateMetaTransform = 0x04000000u,
  //! User transform state.
  kWeakStateUserTransform = 0x08000000u,
  //! All states' flags.
  kWeakStateAllFlags = 0x0FF00000u,

  //! Final translation matrix is just a scale of `fp_scale_d()` and integral translation.
  kInfoIntegralTranslation = 0x10000000u,

  //! Shared fill-state has valid data.
  kSharedStateFill = 0x00020000u,
  //! Shared stroke-state has valid base-stroke data.
  kSharedStateStrokeBase = 0x00040000u,
  //! Shared stroke-state has valid extended-stroke data.
  kSharedStateStrokeExt = 0x00080000u,
  //! A combination of all `kShared...` flags.
  kSharedStateAllFlags = kSharedStateFill | kSharedStateStrokeBase | kSharedStateStrokeExt,

  //! This flag has multiple meanings - it can be used to mark command/job queues being full or it can mark pools
  //! exhausted. It should be checked at the beginning of a render call by the frontend to ensure that that the queue
  //! is not full, and if full, the queue should either grow or the rendering engine should execute it if it cannot
  //! grow anymore (for example it reaches a limit). Additionally, this flag can mark exhausted pools that need to be
  //! refilled.
  kMTFullOrExhausted = 0x80000000u,

  //! Flags that must always be preserved during state switching.
  kPreservedFlags = kMTFullOrExhausted
};

BL_DEFINE_ENUM_FLAGS(ContextFlags)

template<typename ShiftT>
static BL_INLINE_CONSTEXPR ContextFlags operator<<(ContextFlags a, const ShiftT& n) noexcept {
  return ContextFlags(std::underlying_type_t<ContextFlags>(a) << n);
}

template<typename ShiftT>
static BL_INLINE_CONSTEXPR ContextFlags operator>>(ContextFlags a, const ShiftT& n) noexcept {
  return ContextFlags(std::underlying_type_t<ContextFlags>(a) >> n);
}

} // {bl::RasterEngine}

//! Indexes to a `BLRasterContextImpl::solid_format_table`, which describes pixel
//! formats used by solid fills. There are in total 3 choices that are selected
//! based on properties of the solid color.
enum BLRasterContextSolidFormatId : uint32_t {
  BL_RASTER_CONTEXT_SOLID_FORMAT_ARGB = 0,
  BL_RASTER_CONTEXT_SOLID_FORMAT_FRGB = 1,
  BL_RASTER_CONTEXT_SOLID_FORMAT_ZERO = 2,

  BL_RASTER_CONTEXT_SOLID_FORMAT_COUNT = 3
};

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RASTERDEFS_P_H_INCLUDED
