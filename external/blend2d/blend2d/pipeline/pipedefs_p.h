// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_PIPEDEFS_P_H_INCLUDED
#define BLEND2D_PIPELINE_PIPEDEFS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/compop_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/pattern_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/tables/tables_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \namespace bl::Pipeline
//!
//! Blend2D pipeline.

//! \namespace bl::Pipeline::JIT
//!
//! Blend2D JIT pipeline compiler.

//! Global constants used by pipeline and affecting also rasterizers.
enum BLPipeGlobalConsts : uint32_t {
  //! How many pixels are represented by a single bit of a `BLBitWord`.
  //!
  //! This is a hardcoded value as it's required by both rasterizer and compositor. Before establishing `4` the values
  //! [4, 8, 16, 32] were tested. Candidates were `4` and `8` where `8` sometimes surpassed `4` in specific workloads,
  //! but `4` was stable across all tests.
  //!
  //! In general increasing `BL_PIPE_PIXELS_PER_ONE_BIT` would result in less memory consumed by bit vectors, but would
  //! increase the work compositors have to do to process cells produced by analytic rasterizer.
  BL_PIPE_PIXELS_PER_ONE_BIT = 4
};

namespace bl::Pipeline {

struct ContextData;
struct DispatchData;
struct FillData;
struct FetchData;

//! 8-bit alpha constants used by the pipeline and rasterizers.
struct A8Info {
  enum : uint32_t {
    kShift = 8,
    kScale = 1 << kShift,
    kMask = kScale - 1
  };
};

//! Pipeline fill-type.
//!
//! A unique id describing how a mask of each composited pixel is calculated.
enum class FillType : uint8_t {
  //! None or uninitialized.
  kNone = 0,
  //! Fill axis-aligned box.
  kBoxA = 1,
  //! Fill mask command list.
  kMask = 2,
  //! Fill analytic non-zero/even-odd.
  kAnalytic = 3,

  //! Maximum value FillType can have.
  _kMaxValue = 3
};

//! Pipeline extend modes (non-combined).
//!
//! Pipeline sees extend modes a bit differently than the public API in most cases.
enum class ExtendMode : uint8_t {
  //! Pad, same as `BL_EXTEND_MODE_PAD`.
  kPad = 0,
  //! Repeat, same as `BL_EXTEND_MODE_REPEAT`.
  kRepeat = 1,
  //! Reflect, same as `BL_EXTEND_MODE_REFLECT`.
  kReflect = 2,
  //! Repeat-or-reflect (the same code-path for both cases).
  kRoR = 3,

  //! Maximum value the ExtendMode can have.
  _kMaxValue = 3
};

//! Mask command type.
//!
//! CMask must have the value 0.
enum class MaskCommandType : uint32_t {
  //! End or repeat (whether it repeats actually depends on repeat count, which is 1 for end).
  kEndOrRepeat = 0,
  //! Constant mask.
  kCMask = 1,
  //! Variable mask, already multiplied with global alpha.
  kVMaskA8WithGA = 2,
  //! Variable mask, which was not multiplied with global alpha.
  kVMaskA8WithoutGA = 3,

  _kMaxValue = 3
};

//! Fill rule mask used during composition of mask produced by analytic-rasterizer.
enum class FillRuleMask : uint32_t {
  kNonZeroMask = 0xFFFFFFFFu,
  kEvenOddMask = 0x000001FFu
};

//! Pipeline fetch-type.
//!
//! A unique id describing how pixels are fetched - supported fetchers include solid pixels, patterns (sometimes
//! referred as blits), and gradients.
//!
//! \note RoR is a shortcut for repeat-or-reflect - a universal fetcher for both.
enum class FetchType : uint8_t {
  //! Solid fetch.
  kSolid = 0,

  //!< Pattern {aligned} (blit) [Base].
  kPatternAlignedBlit,
  //!< Pattern {aligned} (pad-x) [Base].
  kPatternAlignedPad,
  //!< Pattern {aligned} (repeat-large-x) [Optimized].
  kPatternAlignedRepeat,
  //!< Pattern {aligned} (ror-x) [Base].
  kPatternAlignedRoR,

  //!< Pattern {frac-x} (pad-x) [Optimized].
  kPatternFxPad,
  //!< Pattern {frac-x} (ror-x) [Optimized].
  kPatternFxRoR,
  //!< Pattern {frac-y} (pad-x) [Optimized].
  kPatternFyPad,
  //!< Pattern {frac-x} (ror-x) [Optimized].
  kPatternFyRoR,
  //!< Pattern {frac-xy} (pad-x) [Base].
  kPatternFxFyPad,
  //!< Pattern {frac-xy} (ror-x) [Base].
  kPatternFxFyRoR,

  //!< Pattern {affine-nearest}  (any) [Base].
  kPatternAffineNNAny,
  //!< Pattern {affine-nearest}  (any) [Optimized].
  kPatternAffineNNOpt,
  //!< Pattern {affine-bilinear} (any) [Base].
  kPatternAffineBIAny,
  //!< Pattern {affine-bilinear} (any) [Optimized].
  kPatternAffineBIOpt,

  //!< Linear gradient (pad) [Base].
  kGradientLinearNNPad,
  //!< Linear gradient (repeat or reflect) [Base].
  kGradientLinearNNRoR,

  //!< Linear gradient (pad) [Dither].
  kGradientLinearDitherPad,
  //!< Linear gradient (repeat or reflect) [Dither].
  kGradientLinearDitherRoR,

  //!< Radial gradient (pad) [Base].
  kGradientRadialNNPad,
  //!< Radial gradient (repeat or reflect) [Base].
  kGradientRadialNNRoR,

  //!< Radial gradient (pad) [Dither].
  kGradientRadialDitherPad,
  //!< Radial gradient (repeat or reflect) [Dither].
  kGradientRadialDitherRoR,

  //!< Conic gradient (any) [Base].
  kGradientConicNN,
  //!< Conic gradient (any) [Dither].
  kGradientConicDither,

  //!< Maximum value of a valid FetchType.
  _kMaxValue = kGradientConicDither,

  //!< Pixel pointer (special value, not a valid fetch type).
  kPixelPtr,

  //!< Invalid fetch type (special value, signalizes error).
  kFailure = 0xFFu,

  kPatternAnyFirst = kPatternAlignedBlit,
  kPatternAnyLast = kPatternAffineBIOpt,

  kPatternAlignedFirst = kPatternAlignedBlit,
  kPatternAlignedLast = kPatternAlignedRoR,

  kPatternUnalignedFirst = kPatternFxPad,
  kPatternUnalignedLast = kPatternFxFyRoR,

  kPatternFxFirst = kPatternFxPad,
  kPatternFxLast = kPatternFxRoR,

  kPatternFyFirst = kPatternFyPad,
  kPatternFyLast = kPatternFyRoR,

  kPatternFxFyFirst = kPatternFxFyPad,
  kPatternFxFyLast = kPatternFxFyRoR,

  kPatternSimpleFirst = kPatternAlignedBlit,
  kPatternSimpleLast = kPatternFxFyRoR,

  kPatternAffineFirst = kPatternAffineNNAny,
  kPatternAffineLast = kPatternAffineBIOpt,

  kGradientAnyFirst = kGradientLinearNNPad,
  kGradientAnyLast = kGradientConicDither,

  kGradientLinearFirst = kGradientLinearNNPad,
  kGradientLinearLast = kGradientLinearDitherRoR,

  kGradientRadialFirst = kGradientRadialNNPad,
  kGradientRadialLast = kGradientRadialDitherRoR,

  kGradientConicFirst = kGradientConicNN,
  kGradientConicLast = kGradientConicDither
};

typedef void (BL_CDECL* FillFunc)(ContextData* ctx_data, const void* fill_data, const void* fetch_data) noexcept;
typedef void (BL_CDECL* FetchFunc)(ContextData* ctx_data, const void* fill_data, const void* fetch_data) noexcept;

//! Pipeline signature packed to a single `uint32_t` value.
//!
//! Can be used to build signatures as well as it offers the required functionality.
struct Signature {
  //! \name Constants
  //! \{

  //! Masks used by the Signature.
  //!
  //! Each mask represents one value in a signature. Each value describes a part in a signature like format,
  //! composition operator, etc. All parts packed together form a 32-bit integer that can be used to uniquely
  //! describe the whole pipeline and can act as a key or hash-code in pipeline function caches.
  enum Masks : uint32_t {
    kMaskDstFormat   = 0x0000000Fu, // (4 bits)
    kMaskSrcFormat   = 0x000000F0u, // (4 bits)
    kMaskCompOp      = 0x00003F00u, // (6 bits)
    kMaskFillType    = 0x0000C000u, // (2 bits)
    kMaskFetchType   = 0x001F0000u, // (5 bits)
    kMaskPendingFlag = 0x80000000u  // (1 bit)
  };

  //! \}

  //! \name Members
  //! \{

  //! Signature as a 32-bit value.
  uint32_t value;

  //! \}

  //! \name Static Construction
  //! \{

  //! Returns a signature only containing a DstFormat.
  static BL_INLINE_CONSTEXPR Signature from_dst_format(FormatExt format) noexcept { return Signature{uint32_t(format) << IntOps::bit_shift_of(kMaskDstFormat)}; }
  //! Returns a signature only containing a SrcFormat.
  static BL_INLINE_CONSTEXPR Signature from_src_format(FormatExt format) noexcept { return Signature{uint32_t(format) << IntOps::bit_shift_of(kMaskSrcFormat)}; }
  //! Returns a signature only containing a CompOp.
  static BL_INLINE_CONSTEXPR Signature from_comp_op(CompOpExt comp_op) noexcept { return Signature{uint32_t(comp_op) << IntOps::bit_shift_of(kMaskCompOp)}; }
  //! Returns a signature only containing a FillType.
  static BL_INLINE_CONSTEXPR Signature from_fill_type(FillType fill_type) noexcept { return Signature{uint32_t(fill_type) << IntOps::bit_shift_of(kMaskFillType)}; }
  //! Returns a signature only containing a FetchType.
  static BL_INLINE_CONSTEXPR Signature from_fetch_type(FetchType fetch_type) noexcept { return Signature{uint32_t(fetch_type) << IntOps::bit_shift_of(kMaskFetchType)}; }
  //! Returns a signature only containing a PendingFlag.
  static BL_INLINE_CONSTEXPR Signature from_pending_flag(uint32_t flag) noexcept { return  Signature{uint32_t(flag) << IntOps::bit_shift_of(kMaskPendingFlag)}; }

  //! \}

  BL_INLINE_NODEBUG bool operator==(const Signature& other) const noexcept { return value == other.value; }
  BL_INLINE_NODEBUG bool operator!=(const Signature& other) const noexcept { return value != other.value; }

  BL_INLINE_NODEBUG Signature operator|(const Signature& other) const noexcept { return Signature{value | other.value}; }
  BL_INLINE_NODEBUG Signature operator^(const Signature& other) const noexcept { return Signature{value ^ other.value}; }

  BL_INLINE_NODEBUG Signature& operator|=(const Signature& other) noexcept { value |= other.value; return *this; }
  BL_INLINE_NODEBUG Signature& operator^=(const Signature& other) noexcept { value ^= other.value; return *this; }

  BL_INLINE_NODEBUG uint32_t _get(uint32_t mask) const noexcept {
    return (this->value & mask) >> IntOps::bit_shift_of(mask);
  }

  BL_INLINE void _set(uint32_t mask, uint32_t v) noexcept {
    BL_ASSERT(v <= (mask >> IntOps::bit_shift_of(mask)));
    this->value = (this->value & ~mask) | (v << IntOps::bit_shift_of(mask));
  }

  BL_INLINE void _add(uint32_t mask, uint32_t v) noexcept {
    BL_ASSERT(v <= (mask >> IntOps::bit_shift_of(mask)));
    this->value |= (v << IntOps::bit_shift_of(mask));
  }

  //! Reset all values to zero.
  BL_INLINE_NODEBUG void reset() noexcept { this->value = 0; }
  //! Reset all values to `v`.
  BL_INLINE_NODEBUG void reset(uint32_t v) noexcept { this->value = v; }
  //! Reset all values to the `other` signature.
  BL_INLINE_NODEBUG void reset(const Signature& other) noexcept { this->value = other.value; }

  //! Set the signature from a packed 32-bit integer.
  BL_INLINE_NODEBUG void set_value(uint32_t v) noexcept { this->value = v; }
  //! Set the signature from another `Signature`.
  BL_INLINE_NODEBUG void set_value(const Signature& other) noexcept { this->value = other.value; }

  //! Extracts destination pixel format from the signature.
  BL_INLINE_NODEBUG FormatExt dst_format() const noexcept { return FormatExt(_get(kMaskDstFormat)); }
  //! Extracts source pixel format from the signature.
  BL_INLINE_NODEBUG FormatExt src_format() const noexcept { return FormatExt(_get(kMaskSrcFormat)); }
  //! Extracts composition operator from the signature.
  BL_INLINE_NODEBUG CompOpExt comp_op() const noexcept { return CompOpExt(_get(kMaskCompOp)); }
  //! Extracts sweep type from the signature.
  BL_INLINE_NODEBUG FillType fill_type() const noexcept { return FillType(_get(kMaskFillType)); }
  //! Extracts fetch type from the signature.
  BL_INLINE_NODEBUG FetchType fetch_type() const noexcept { return FetchType(_get(kMaskFetchType)); }
  //! Extracts pending flag from the signature.
  BL_INLINE_NODEBUG bool has_pending_flag() const noexcept { return (value & kMaskPendingFlag) != 0u; }

  BL_INLINE_NODEBUG bool is_solid() const noexcept { return (value & kMaskFetchType) == 0u; }

  BL_INLINE_NODEBUG bool is_gradient() const noexcept {
    return fetch_type() >= FetchType::kGradientAnyFirst && fetch_type() <= FetchType::kGradientAnyLast;
  }

  //! Add destination pixel format.
  BL_INLINE_NODEBUG void set_dst_format(FormatExt v) noexcept { _set(kMaskDstFormat, uint32_t(v)); }
  //! Add source pixel format.
  BL_INLINE_NODEBUG void set_src_format(FormatExt v) noexcept { _set(kMaskSrcFormat, uint32_t(v)); }
  //! Add clip mode.
  BL_INLINE_NODEBUG void set_comp_op(CompOpExt v) noexcept { _set(kMaskCompOp, uint32_t(v)); }
  //! Add sweep type.
  BL_INLINE_NODEBUG void set_fill_type(FillType v) noexcept { _set(kMaskFillType, uint32_t(v)); }
  //! Add fetch type.
  BL_INLINE_NODEBUG void set_fetch_type(FetchType v) noexcept { _set(kMaskFetchType, uint32_t(v)); }

  // The following methods are used to build the signature. They use '|' operator
  // which doesn't clear the previous value, each function is expected to be called
  // only once when building a new signature.

  //! Combine with other signature.
  BL_INLINE_NODEBUG void add(uint32_t v) noexcept { this->value |= v; }
  //! Combine with other signature.
  BL_INLINE_NODEBUG void add(const Signature& other) noexcept { this->value |= other.value; }

  //! Add destination pixel format.
  BL_INLINE_NODEBUG void add_dst_format(FormatExt v) noexcept { _add(kMaskDstFormat, uint32_t(v)); }
  //! Add source pixel format.
  BL_INLINE_NODEBUG void add_src_format(FormatExt v) noexcept { _add(kMaskSrcFormat, uint32_t(v)); }
  //! Add clip mode.
  BL_INLINE_NODEBUG void add_comp_op(CompOpExt v) noexcept { _add(kMaskCompOp, uint32_t(v)); }
  //! Add sweep type.
  BL_INLINE_NODEBUG void add_fill_type(FillType v) noexcept { _add(kMaskFillType, uint32_t(v)); }
  //! Add fetch type.
  BL_INLINE_NODEBUG void add_fetch_type(FetchType v) noexcept { _add(kMaskFetchType, uint32_t(v)); }

  BL_INLINE_NODEBUG void add_pending_bit(uint32_t v) noexcept { _add(kMaskPendingFlag, v); }
  BL_INLINE_NODEBUG void clear_pending_bit() noexcept { value &= ~kMaskPendingFlag; }
};

struct DispatchData {
  FillFunc fill_func;
  FetchFunc fetch_func;

  //! Initializes the dispatch data.
  //!
  //! If both `fill_func_init` and `fetch_func_init` are non-null the pipeline would be two-stage, if `fetch_func` is
  //! null the pipeline would be one-stage. Typically JIT compiled pipelines are one-stage only (the fetch phase
  //! is inlined into the pipeline, but it's not a hard requirement).
  BL_INLINE void init(FillFunc fill_func_init, FetchFunc fetch_func_init = nullptr) noexcept {
    fill_func = fill_func_init;
    fetch_func = fetch_func_init;
  }

  //! Tests whether the dispatch data contains a one-stage pipeline.
  //!
  //! One-stage pipelines have no fetch function as it has been merged with fill function.
  BL_INLINE bool is_one_stage() const noexcept { return fetch_func == nullptr; }
};

union PipeValue32 {
  uint32_t u;
  int32_t i;
  float f;
};

union PipeValue64 {
  uint64_t u64;
  int64_t i64;
  double d;

  int32_t i32[2];
  uint32_t u32[2];

  int16_t i16[4];
  uint16_t u16[4];

#if BL_BYTE_ORDER == 1234 // LITTLE ENDIAN
  struct { int32_t  i32_lo, i32_hi; };
  struct { uint32_t u32_lo, u32_hi; };
#else
  struct { int32_t  i32_hi, i32_lo; };
  struct { uint32_t u32_hi, u32_lo; };
#endif

  BL_INLINE void expand_lo_to_hi() noexcept { u32_hi = u32_lo; }
};

//! Mask command.
struct MaskCommand {
  enum : uint32_t {
    kTypeBits = 3,
    kTypeMask = 0x7
  };

  //! \name Members
  //! \{

  //! Start of the span, inclusive.
  uint32_t _x0;
  //! End of the span combined with command type, exclusive.
  //!
  //! \note Most people would add type into _x0 member, however, it's not good for most micro-architectures
  //! as today's CPUs are speculative and not knowing X0 would cause a lot of frontend cycle stalls due to
  //! not knowing the index on load.
  uint32_t _x1_and_type;

  union {
    uintptr_t data;
    const void* ptr;
  } _value;

  //! Added to `_value.data` each time this command is processed by the filler.
  uintptr_t _mask_advance;

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG MaskCommandType type() const noexcept { return MaskCommandType(_x1_and_type & kTypeMask); }
  BL_INLINE_NODEBUG uint32_t x0() const noexcept { return _x0; }
  BL_INLINE_NODEBUG uint32_t x1() const noexcept { return _x1_and_type >> kTypeBits; }

  BL_INLINE_NODEBUG uint32_t repeat_count() const noexcept { return _x0; }
  BL_INLINE_NODEBUG void update_repeat_count(uint32_t value) noexcept { _x0 = value; }

  BL_INLINE_NODEBUG bool is_const_mask() const noexcept { return type() == MaskCommandType::kCMask; }

  BL_INLINE_NODEBUG uint32_t mask_value() const noexcept { return uint32_t(_value.data); }
  BL_INLINE_NODEBUG const void* mask_data() const noexcept { return _value.ptr; }

  BL_INLINE_NODEBUG intptr_t mask_advance() const noexcept { return intptr_t(_mask_advance); }

  BL_INLINE void init_type_and_span(MaskCommandType type, uint32_t x0, uint32_t x1) noexcept {
    BL_ASSERT(((x1 << kTypeBits) >> kTypeBits) == x1);
    _x0 = x0;
    _x1_and_type = uint32_t(type) | (x1 << kTypeBits);
  }

  BL_INLINE void init_cmask(MaskCommandType cmd_type, uint32_t x0, uint32_t x1, uint32_t mask_value) noexcept {
    init_type_and_span(cmd_type, x0, x1);
    _value.data = mask_value;
    _mask_advance = 0;
  }

  BL_INLINE void init_vmask(MaskCommandType type, uint32_t x0, uint32_t x1, const void* mask_data, intptr_t mask_advance = 0) noexcept {
    init_type_and_span(type, x0, x1);
    _value.ptr = mask_data;
    _mask_advance = uintptr_t(mask_advance);
  }

  BL_INLINE void init_cmask_a8(uint32_t x0, uint32_t x1, uint32_t mask_value) noexcept {
    init_cmask(MaskCommandType::kCMask, x0, x1, mask_value);
  }

  BL_INLINE void init_vmask_a8_with_ga(uint32_t x0, uint32_t x1, const void* mask_data, intptr_t mask_advance = 0) noexcept {
    init_vmask(MaskCommandType::kVMaskA8WithGA, x0, x1, mask_data, mask_advance);
  }

  BL_INLINE void init_vmask_a8_without_ga(uint32_t x0, uint32_t x1, const void* mask_data, intptr_t mask_advance = 0) noexcept {
    init_vmask(MaskCommandType::kVMaskA8WithoutGA, x0, x1, mask_data, mask_advance);
  }

  BL_INLINE void init_end() noexcept {
    init_type_and_span(MaskCommandType::kEndOrRepeat, 1, 0);
  }

  BL_INLINE void init_repeat(uint32_t n_repeat = 0xFFFFFFFFu) noexcept {
    init_type_and_span(MaskCommandType::kEndOrRepeat, n_repeat, 0);
  }

  //! \}
};

//! Contains data that is required to decompose a BoxU fill into mask commands.
struct BoxUToMaskData {
  // At most 4 commands per scanline, at most 3 distinct scanlines.
  MaskCommand mask_cmd[4u * 3u];
  // At most 32 bytes per scanline, at most 3 distinct scanlines.
  uint8_t mask_data[32 * 3u];
};

struct ContextData {
  BLImageData dst;
  BLPointI pixel_origin;

  BL_INLINE void reset() noexcept { *this = ContextData{}; }
};

static BL_INLINE void writeBoxUMaskToMaskBuffer(uint8_t* dst, uint32_t m) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t repeated = uint64_t(m) * 0x0101010101010101u;
  uint64_t mask = BL_BYTE_ORDER == 1234 ? 0xFFFFFFFF00000000u : 0x00000000FFFFFFFFu;
  MemOps::writeU64a(dst + 24, repeated);
  MemOps::writeU64a(dst + 16, repeated);
  MemOps::writeU64a(dst +  8, repeated);
  MemOps::writeU64a(dst +  0, repeated & mask);
#else
  uint32_t repeated = m * 0x01010101u;
  MemOps::writeU32a(dst +  0, 0u);
  MemOps::writeU32a(dst +  4, repeated);
  MemOps::writeU32a(dst +  8, repeated);
  MemOps::writeU32a(dst + 12, repeated);
  MemOps::writeU32a(dst + 16, repeated);
  MemOps::writeU32a(dst + 20, repeated);
  MemOps::writeU32a(dst + 24, repeated);
  MemOps::writeU32a(dst + 28, repeated);
#endif
}

struct FillData {
  struct Common {
    //! Rectangle to fill.
    BLBoxI box;
    //! Alpha value (range depends on target pixel format).
    PipeValue32 alpha;
  };

  //! Rectangle (axis-aligned).
  struct BoxA {
    //! Rectangle to fill.
    BLBoxI box;
    //! Alpha value (range depends on target pixel format).
    PipeValue32 alpha;
  };

  //! Rectangle (axis-unaligned).
  struct BoxU {
    //! Rectangle to fill.
    BLBoxI box;
    //! Alpha value (range depends on target pixel format).
    PipeValue32 alpha;

    //! Masks of top, middle and bottom part of the rect.
    //!
    //! \note The last value `masks[3]` must be zero as it's a sentinel for the pipeline.
    uint32_t masks[4];
    //! Height of the middle (1) and last (2) masks.
    uint32_t heights[2];
    //! Start width (from 1 to 3).
    uint32_t start_width;
    //! Inner width (from 0 to width).
    uint32_t inner_width;
  };

  struct Mask {
    //! Fill boundary.
    BLBoxI box;
    //! Alpha value (range depends on target pixel format).
    PipeValue32 alpha;
    //! Reserved for future use (padding).
    uint32_t reserved;

    //! The first mask command to process.
    MaskCommand* mask_command_data;
  };

  struct Analytic {
    //! Fill boundary.
    BLBoxI box;
    //! Alpha value (range depends on format).
    PipeValue32 alpha;
    //! All ones if NonZero or 0x01FF if EvenOdd.
    uint32_t fill_rule_mask;

    //! Shadow bit-buffer (marks a group of cells which are non-zero).
    BLBitWord* bit_top_ptr;
    //! Bit-buffer stride (in bytes).
    size_t bit_stride;

    //! Cell buffer.
    uint32_t* cell_top_ptr;
    //! Cell stride (in bytes).
    size_t cell_stride;
  };

  union {
    Common common;
    BoxA boxAA;
    BoxU boxAU;
    Mask mask;
    Analytic analytic;
  };

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }

  //! \name Init
  //! \{

  BL_INLINE bool init_box_a_8bpc(uint32_t alpha, int x0, int y0, int x1, int y1) noexcept {
    // The rendering engine should never pass out-of-range alpha.
    BL_ASSERT(alpha <= 255);

    // The rendering engine should never pass invalid box to the pipeline.
    BL_ASSERT(x0 < x1);
    BL_ASSERT(y0 < y1);

    boxAA.alpha.u = alpha;
    boxAA.box.reset(int(x0), int(y0), int(x1), int(y1));

    return true;
  }

  template<typename T>
  BL_INLINE bool init_box_u_8bpc_t(uint32_t alpha, T x0, T y0, T x1, T y1, BoxUToMaskData& mask_data) noexcept {
    return init_box_u_8bpc_24x8(alpha,
      Math::trunc_to_int(x0 * T(256)),
      Math::trunc_to_int(y0 * T(256)),
      Math::trunc_to_int(x1 * T(256)),
      Math::trunc_to_int(y1 * T(256)),
      mask_data);
  }

  bool init_box_u_8bpc_24x8(uint32_t alpha, int x0, int y0, int x1, int y1, BoxUToMaskData& mask_data) noexcept {
    // The rendering engine should never pass out-of-range alpha.
    BL_ASSERT(alpha <= 255);

    // The rendering engine should never pass invalid box to the pipeline.
    BL_ASSERT(x0 < x1);
    BL_ASSERT(y0 < y1);

    constexpr uint32_t kInnerAlignment = 8;
    constexpr uint32_t kMaskScanlineWidth = 32;
    constexpr uint32_t kMaxMaskOnlyWidth = 20;

    uint32_t ax0 = uint32_t(x0) >> 8;
    uint32_t ay0 = uint32_t(y0) >> 8;
    uint32_t ax1 = uint32_t(x1 + 0xFF) >> 8;
    uint32_t ay1 = uint32_t(y1 + 0xFF) >> 8;

    uint32_t fx0 = uint32_t(x0) & 0xFFu;
    uint32_t fy0 = uint32_t(y0) & 0xFFu;
    uint32_t fx1 = (uint32_t(x1 - 1) & 0xFFu) + 1u;
    uint32_t fy1 = (uint32_t(y1 - 1) & 0xFFu) + 1u;

    uint32_t w = ax1 - ax0;
    uint32_t h = ay1 - ay0;

    fy0 = (h == 1 ? fy1 : 256u) - fy0;

    uint32_t fy0_a = fy0 * alpha;
    uint32_t fy1_a = fy1 * alpha;

    MaskCommand* mask_cmd = mask_data.mask_cmd;
    uint8_t* mask_ptr = mask_data.mask_data;

    mask.alpha.u = 0xFF;
    mask.box.reset(int(ax0), int(ay0), int(ax0 + w), int(ay0 + h));
    mask.mask_command_data = mask_cmd;

    // Special cases first - smaller the rectangle => greater the overhead per pixel if we do unnecessary work.
    if (w == 1) {
      // If the rectangle has 1 pixel width, we have to sum fx0 and fx1 to calculate the mask value. This is
      // not needed for a regular case in which the width is greater than 1 - in that case there are always
      // two bordering pixels, which masks are calculated separately.
      fx0 = fx1 - fx0;

      uint32_t m0 = (fx0 * fy0_a) >> 16u;
      mask_cmd[0].init_cmask_a8(ax0, ax1, m0);
      mask_cmd[1].init_end();

      if (h == 1)
        return m0 != 0;

      mask_cmd += m0 ? 2u : 0u;
      mask.box.y0 += int(m0 == 0);

      uint32_t m1 = (fx0 * alpha) >> 8u;
      mask_cmd[0].init_cmask_a8(ax0, ax1, m1);
      mask_cmd[1].init_repeat(h - 2);
      mask_cmd += h > 2 ? 2u : 0u;

      uint32_t m2 = (fx0 * fy1_a) >> 16u;
      mask_cmd[0].init_cmask_a8(ax0, ax1, m2);
      mask_cmd[1].init_end();

      mask.box.y1 -= int(m2 == 0);
      return mask.box.y0 < mask.box.y1 && m1 != 0;
    }

    // Common case - if width > 1 then we don't have to worry about fx0 and fx1 - both represent a different pixel.
    uint32_t m0x1 = fy0_a >> 8u;
    uint32_t m1x1 = alpha;
    uint32_t m2x1 = fy1_a >> 8u;

    fx0 = 256 - fx0;

    if ((fx0 & fx1) == 256) {
      // If the rectangle doesn't have a fractional X0/X1 then each scanline would only need a single CMask
      // command instead of either VMask or [VMask, CMask, VMask] sequence.
      mask_cmd[0].init_cmask_a8(ax0, ax1, m0x1);
      mask_cmd[1].init_end();
      mask_cmd += m0x1 ? 2u : 0u;
      mask.box.y0 += int(m0x1 == 0);

      mask_cmd[0].init_cmask_a8(ax0, ax1, m1x1);
      mask_cmd[1].init_repeat(h - 2);
      mask_cmd += h > 2 ? 2u : 0u;

      mask_cmd[0].init_cmask_a8(ax0, ax1, m2x1);
      mask_cmd[1].init_end();
      mask.box.y1 -= int(m2x1 == 0);

      return mask.box.y0 < mask.box.y1;
    }

    uint32_t m0x0 = (fx0 * fy0_a) >> 16u;
    uint32_t m0x2 = (fx1 * fy0_a) >> 16u;
    writeBoxUMaskToMaskBuffer(mask_ptr + kMaskScanlineWidth * 0u, m0x1);
    mask_ptr[kMaskScanlineWidth * 0u + 4] = uint8_t(m0x0);

    uint32_t m1x0 = (fx0 * alpha) >> 8u;
    uint32_t m1x2 = (fx1 * alpha) >> 8u;
    writeBoxUMaskToMaskBuffer(mask_ptr + kMaskScanlineWidth * 1u, m1x1);
    mask_ptr[kMaskScanlineWidth * 1u + 4] = uint8_t(m1x0);

    uint32_t m2x0 = (fx0 * fy1_a) >> 16u;
    uint32_t m2x2 = (fx1 * fy1_a) >> 16u;
    writeBoxUMaskToMaskBuffer(mask_ptr + kMaskScanlineWidth * 2u, m2x1);
    mask_ptr[kMaskScanlineWidth * 2u + 4] = uint8_t(m2x0);

    mask_ptr += 4;
    uint32_t wAlign = IntOps::align_up_diff(w, 4);

    if (wAlign > ax0)
      wAlign = 0;

    ax0 -= wAlign;
    w += wAlign;
    mask_ptr -= wAlign;

    if (w <= kMaxMaskOnlyWidth) {
      mask_ptr[kMaskScanlineWidth * 0u + w - 1u] = uint8_t(m0x2);
      mask_ptr[kMaskScanlineWidth * 1u + w - 1u] = uint8_t(m1x2);
      mask_ptr[kMaskScanlineWidth * 2u + w - 1u] = uint8_t(m2x2);

      mask_cmd[0].init_vmask_a8_with_ga(ax0, ax1, mask_ptr + kMaskScanlineWidth * 0u, 0);
      mask_cmd[1].init_end();
      mask_cmd += m0x1 ? 2u : 0u;

      mask.box.y0 += int(m0x1 == 0);

      mask_cmd[0].init_vmask_a8_with_ga(ax0, ax1, mask_ptr + kMaskScanlineWidth * 1u, 0);
      mask_cmd[1].init_repeat(h - 2);
      mask_cmd += h > 2 ? 2u : 0u;

      mask_cmd[0].init_vmask_a8_with_ga(ax0, ax1, mask_ptr + kMaskScanlineWidth * 2u, 0);
      mask_cmd[1].init_end();
      mask.box.y1 -= int(m2x1 == 0);

      return mask.box.y0 < mask.box.y1;
    }
    else {
      uint32_t inner_width = IntOps::align_down(w - 5, kInnerAlignment);
      uint32_t inner_end = ax0 + 4u + inner_width;
      uint32_t tail_width = ax1 - inner_end;

      const uint8_t* mask_tail = mask_ptr + 16 - tail_width;

      mask_ptr[kMaskScanlineWidth * 0u + 15u] = uint8_t(m0x2);
      mask_ptr[kMaskScanlineWidth * 1u + 15u] = uint8_t(m1x2);
      mask_ptr[kMaskScanlineWidth * 2u + 15u] = uint8_t(m2x2);

      mask_cmd[0].init_vmask_a8_with_ga(ax0, ax0 + 4u, mask_ptr + kMaskScanlineWidth * 0u, 0);
      mask_cmd[1].init_cmask_a8(ax0 + 4u, inner_end, m0x1);
      mask_cmd[2].init_vmask_a8_with_ga(inner_end, ax1, mask_tail + kMaskScanlineWidth * 0u, 0);
      mask_cmd[3].init_end();
      mask_cmd += m0x1 ? 4u : 0u;
      mask.box.y0 += int(m0x1 == 0);

      mask_cmd[0].init_vmask_a8_with_ga(ax0, ax0 + 4u, mask_ptr + kMaskScanlineWidth * 1u, 0);
      mask_cmd[1].init_cmask_a8(ax0 + 4u, inner_end, m1x1);
      mask_cmd[2].init_vmask_a8_with_ga(inner_end, ax1, mask_tail + kMaskScanlineWidth * 1u, 0);
      mask_cmd[3].init_repeat(h - 2);
      mask_cmd += h > 2 ? 4u : 0u;

      mask_cmd[0].init_vmask_a8_with_ga(ax0, ax0 + 4u, mask_ptr + kMaskScanlineWidth * 2u, 0);
      mask_cmd[1].init_cmask_a8(ax0 + 4u, inner_end, m2x1);
      mask_cmd[2].init_vmask_a8_with_ga(inner_end, ax1, mask_tail + kMaskScanlineWidth * 2u, 0);
      mask_cmd[3].init_end();
      mask.box.y1 -= int(m2x1 == 0);

      return mask.box.y0 < mask.box.y1;
    }
  }

  BL_INLINE void init_mask_a(uint32_t alpha, int x0, int y0, int x1, int y1, MaskCommand* mask_command_data) noexcept {
    mask.alpha.u = alpha;
    mask.box.reset(x0, y0, x1, y1);
    mask.mask_command_data = mask_command_data;
  }

  BL_INLINE bool init_analytic(uint32_t alpha, uint32_t fill_rule, BLBitWord* bit_top_ptr, size_t bit_stride, uint32_t* cell_top_ptr, size_t cell_stride) noexcept {
    analytic.alpha.u = alpha;
    analytic.fill_rule_mask = uint32_t(fill_rule == BL_FILL_RULE_NON_ZERO ? FillRuleMask::kNonZeroMask : FillRuleMask::kEvenOddMask);
    analytic.bit_top_ptr = bit_top_ptr;
    analytic.bit_stride = bit_stride;
    analytic.cell_top_ptr = cell_top_ptr;
    analytic.cell_stride = cell_stride;

    return true;
  }

  //! \}
};

//! Pipeline fetch data.
struct alignas(16) FetchData {
  //! Solid fetch data.
  struct Solid {
    union {
      //! 64-bit ARGB, premultiplied.
      uint64_t prgb64;

      struct {
#if BL_BYTE_ORDER == 1234
        //! 32-bit ARGB, premultiplied.
        uint32_t prgb32;
        //! Reserved in case 32-bit data is used.
        uint32_t reserved32;
#else
        //! Reserved in case 32-bit data is used.
        uint32_t reserved32;
        //! 32-bit ARGB, premultiplied.
        uint32_t prgb32;
#endif
      };
    };
  };

  //! Pattern fetch data.
  struct alignas(16) Pattern {
    //! Source image data.
    struct SourceData {
      const uint8_t* pixel_data;
      intptr_t stride;
      BLSizeI size;
    };

    struct AlignedBlit {
      //! Translate by x/y (inverted).
      int32_t tx, ty;
    };

    //! Extend data used by pipelines to handle vertical PAD, REPEAT, and REFLECT extend modes dynamically.
    struct VertExtendData {
      //! Stride and alternative stride:
      //!
      //!   - PAD    : [src.stride, 0]
      //!   - REPEAT : [src.stride, src.stride]
      //!   - REFLECT: [src.stride,-src.stride]
      intptr_t stride[2];

      //! Y-stop and alternative y-stop:
      //!
      //!   - PAD    : [src.size.h, 0]
      //!   - REPEAT : [src.size.h, src.size.h]
      //!   - REFLECT: [src.size.h, src.size.h]
      uintptr_t y_stop[2];

      //! Offset that is applied to Y variable when the scanline reaches a local y-stop.
      //!
      //! This value must be 0 in PAD case and `src.size.h` in REPEAT or REFLECT case.
      uintptr_t y_rewind_offset;

      //! Offset that is applied to pixel data when the scanline reaches a local y-stop.
      //!
      //! This value must be 0 in PAD or REFLECT case, and `src.size.h - 1 * stride` in REPEAT case.
      intptr_t pixel_ptr_rewind_offset;
    };

    //! Simple pattern data (only identity or translation matrix).
    struct alignas(16) Simple {
      //! Translate by x/y (inverted).
      int32_t tx, ty;
      //! Repeat/Reflect w/h.
      int32_t rx, ry;
      //! Safe X increments by 1..16 (fetchN).
      ModuloTable ix;
      //! 9-bit or 17-bit weight at [0, 0] (A).
      uint32_t wa;
      //! 9-bit or 17-bit weight at [1, 0] (B).
      uint32_t wb;
      //! 9-bit or 17-bit weight at [0, 1] (C).
      uint32_t wc;
      //! 9-bit or 17-bit weight at [1, 1] (D).
      uint32_t wd;

      //! Vertical extend data.
      VertExtendData v_extend_data;
    };

    //! Affine pattern data.
    struct alignas(16) Affine {
      //! Single X/Y step in X direction.
      PipeValue64 xx, xy;
      //! Single X/Y step in Y direction.
      PipeValue64 yx, yy;
      //! Pattern offset at [0, 0].
      PipeValue64 tx, ty;
      //! Pattern overflow check.
      PipeValue64 ox, oy;
      //! Pattern overflow correction (repeat/reflect).
      PipeValue64 rx, ry;
      //! Two X/Y steps in X direction, used by `fetch4()`.
      PipeValue64 xx2, xy2;
      //! Pattern padding minimum (0 for PAD, INT32_MIN for other modes).
      int32_t min_x, min_y;
      //! Pattern padding maximum (width-1 and height-1).
      int32_t max_x, max_y;
      //! Correction X/Y values in case that max_x/max_y was exceeded (PAD, BILINEAR)
      int32_t cor_x, cor_y;
      //! Repeated tile width/height (doubled if reflected).
      double tw, th;

      union {
        //! 16-bit multipliers to be used by [V]PMADDWD instruction to calculate address from Y/X pairs.
        int16_t addr_mul16[2];
        //! 32-bit multipliers for X and Y coordinates.
        int32_t addr_mul32[2];
      };
    };

    //! Source image data.
    SourceData src;

    union {
      //! Simple pattern data.
      Simple simple;
      //! Affine pattern data.
      Affine affine;
    };
  };

  //! Gradient fetch data.
  struct alignas(16) Gradient {
    //! Precomputed lookup table, used by all gradient fetchers.
    struct LUT {
      //! Pixel data, array of either 32-bit or 64-bit pixels.
      const void* data;
      //! Number of pixels stored in `data`, must be a power of 2.
      uint32_t size;
    };

    //! Linear gradient data.
    struct alignas(16) Linear {
      //! Gradient offset of the pixel at [0, 0].
      PipeValue64 pt[2];
      //! One Y step.
      PipeValue64 dy;
      //! One X step.
      PipeValue64 dt;

      //! Maximum index value taking into account pad, repeat, and reflection - `(repeated_or_reflected_size - 1)`.
      uint32_t maxi;
      //! Repeat/Reflect mask to apply to index (either `reflected_size - 1` or `zero`).
      uint32_t rori;
    };

    //! Radial gradient data.
    struct alignas(16) Radial {
      //! Gradient X/Y offsets at [0, 0].
      double tx, ty;
      //! Gradient X/Y increments (vertical).
      double yx, yy;

      double amul4, inv2a;
      double sq_fr, sq_inv2a;

      double b0, dd0;
      double by, ddy;

      float f32_ddd;
      float f32_bd;

      //! Maximum index value taking into account pad, repeat, and reflection - `(repeated_or_reflected_size - 1)`.
      uint32_t maxi;
      //! Repeat/Reflect mask to apply to index (either `reflected_size - 1` or `zero`).
      uint32_t rori;
    };

    //! Conic gradient data.
    struct alignas(16) Conic {
      //! Gradient X/Y offsets of the pixel at [0, 0].
      double tx, ty;
      //! Gradient X/Y increments (vertical).
      double yx, yy;

      //! Atan approximation coefficients.
      float q_coeff[4];
      //! Table size divided by 1, 2, and 4.
      float n_div_1_2_4[3];
      //! Angle offset.
      float offset;

      //! Gradient X increment (horizontal)
      //!
      //! \note There is no Y increment in X direction as the transformation matrix has been rotated in a way to
      //! make it zero, which simplifies computation requirements per pixel.
      float xx;

      //! Maximum index value - `lut.size - 1`.
      uint32_t maxi;
      //! Repeat mask to apply to index.
      uint32_t rori;
    };

    //! Precomputed lookup table.
    LUT lut;
    //! Union of all possible gradient data types.
    union {
      //! Linear gradient specific data.
      Linear linear;
      //! Radial gradient specific data.
      Radial radial;
      //! Conic gradient specific data.
      Conic conic;
    };
  };

  //! Union of all possible fetch data types.
  union {
    //! Solid fetch data.
    Solid solid;
    //! Pattern fetch data.
    Pattern pattern;
    //! Gradient fetch data.
    Gradient gradient;
  };
};

namespace FetchUtils {

static BL_INLINE void init_image_source(FetchData::Pattern& fetch_data, const uint8_t* pixel_data, intptr_t stride, int w, int h) noexcept {
  fetch_data.src.pixel_data = pixel_data;
  fetch_data.src.stride = stride;
  fetch_data.src.size.reset(w, h);
}

static BL_INLINE Signature init_pattern_blit(FetchData::Pattern& fetch_data, int x, int y) noexcept {
  fetch_data.simple.tx = x;
  fetch_data.simple.ty = y;
  fetch_data.simple.rx = 0;
  fetch_data.simple.ry = 0;
  return Signature::from_fetch_type(FetchType::kPatternAlignedBlit);
}

Signature init_pattern_ax_ay(
  FetchData::Pattern& fetch_data,
  BLExtendMode extend_mode,
  int x, int y) noexcept;

Signature init_pattern_fx_fy(
  FetchData::Pattern& fetch_data,
  BLExtendMode extend_mode,
  BLPatternQuality quality,
  uint32_t bytes_per_pixel,
  int64_t tx64, int64_t ty64) noexcept;

Signature init_pattern_affine(
  FetchData::Pattern& fetch_data,
  BLExtendMode extend_mode,
  BLPatternQuality quality,
  uint32_t bytes_per_pixel,
  const BLMatrix2D& transform) noexcept;

Signature init_gradient(
  FetchData::Gradient& fetch_data,
  BLGradientType gradient_type,
  BLExtendMode extend_mode,
  BLGradientQuality quality,
  const void* values,
  const void* lut_data, uint32_t lut_size,
  const BLMatrix2D& transform) noexcept;

} // {FetchUtils}
} // {bl::Pipeline}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_PIPEDEFS_P_H_INCLUDED
