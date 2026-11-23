// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CONTEXT_H_INCLUDED
#define BLEND2D_CONTEXT_H_INCLUDED

#include <blend2d/core/font.h>
#include <blend2d/core/geometry.h>
#include <blend2d/core/glyphrun.h>
#include <blend2d/core/gradient.h>
#include <blend2d/core/image.h>
#include <blend2d/core/matrix.h>
#include <blend2d/core/object.h>
#include <blend2d/core/pattern.h>
#include <blend2d/core/path.h>
#include <blend2d/core/rgba.h>
#include <blend2d/core/var.h>

//! \addtogroup bl_rendering
//! \{

//! \name BLContext - Constants
//! \{

//! Rendering context type.
BL_DEFINE_ENUM(BLContextType) {
  //! No rendering context.
  BL_CONTEXT_TYPE_NONE = 0,
  //! Dummy rendering context.
  BL_CONTEXT_TYPE_DUMMY = 1,

  /*
  //! Proxy rendering context.
  BL_CONTEXT_TYPE_PROXY = 2,
  */

  //! Software-accelerated rendering context.
  BL_CONTEXT_TYPE_RASTER = 3,

  //! Maximum value of `BLContextType`.
  BL_CONTEXT_TYPE_MAX_VALUE = 3

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_TYPE)
};

//! Rendering context hint.
BL_DEFINE_ENUM(BLContextHint) {
  //! Rendering quality.
  BL_CONTEXT_HINT_RENDERING_QUALITY = 0,
  //! Gradient quality.
  BL_CONTEXT_HINT_GRADIENT_QUALITY = 1,
  //! Pattern quality.
  BL_CONTEXT_HINT_PATTERN_QUALITY = 2,

  //! Maximum value of `BLContextHint`.
  BL_CONTEXT_HINT_MAX_VALUE = 7

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_HINT)
};

//! Describes a rendering context style slot - fill or stroke.
BL_DEFINE_ENUM(BLContextStyleSlot) {
  //! Fill operation style slot.
  BL_CONTEXT_STYLE_SLOT_FILL = 0,
  //! Stroke operation style slot.
  BL_CONTEXT_STYLE_SLOT_STROKE = 1,

  //! Maximum value of `BLContextStyleSlot`
  BL_CONTEXT_STYLE_SLOT_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_STYLE_SLOT)
};

//! The type of a text rendering operation.
//!
//! This value specifies the type of the parameter passed to the text rendering API.
//!
//! \note In most cases this should not be required to use by Blend2D users. The C API provides functions that
//! wrap all of the text operations and C++ API provides functions that use `BLContextRenderTextOp` internally.
BL_DEFINE_ENUM(BLContextRenderTextOp) {
  //! UTF-8 text rendering operation - UTF-8 string passed as \ref BLStringView or \ref BLArrayView<uint8_t>.
  BL_CONTEXT_RENDER_TEXT_OP_UTF8 = BL_TEXT_ENCODING_UTF8,
  //! UTF-16 text rendering operation - UTF-16 string passed as \ref BLArrayView<uint16_t>.
  BL_CONTEXT_RENDER_TEXT_OP_UTF16 = BL_TEXT_ENCODING_UTF16,
  //! UTF-32 text rendering operation - UTF-32 string passed as \ref BLArrayView<uint32_t>.
  BL_CONTEXT_RENDER_TEXT_OP_UTF32 = BL_TEXT_ENCODING_UTF32,
  //! LATIN1 text rendering operation - LATIN1 string is passed as \ref BLStringView or \ref BLArrayView<uint8_t>.
  BL_CONTEXT_RENDER_TEXT_OP_LATIN1 = BL_TEXT_ENCODING_LATIN1,
  //! `wchar_t` text rendering operation - wchar_t string is passed as \ref BLArrayView<wchar_t>.
  BL_CONTEXT_RENDER_TEXT_OP_WCHAR = BL_TEXT_ENCODING_WCHAR,
  //! Glyph run text rendering operation - the \ref BLGlyphRun parameter is passed.
  BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN = 4,

  //! Maximum value of `BLContextRenderTextInputType`
  BL_CONTEXT_RENDER_TEXT_OP_MAX_VALUE = 4

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_RENDER_TEXT_OP_TYPE)
};

//! Rendering context flush flags, used by \ref BLContext::flush().
BL_DEFINE_ENUM(BLContextFlushFlags) {
  BL_CONTEXT_FLUSH_NO_FLAGS = 0u,

  //! Flushes the command queue and waits for its completion (will block until done).
  BL_CONTEXT_FLUSH_SYNC = 0x80000000u

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_FLUSH)
};

//! Rendering context creation flags.
BL_DEFINE_ENUM(BLContextCreateFlags) {
  //! No flags.
  BL_CONTEXT_CREATE_NO_FLAGS = 0u,

  //! Disables JIT pipeline generator.
  BL_CONTEXT_CREATE_FLAG_DISABLE_JIT = 0x00000001u,

  //! Fallbacks to a synchronous rendering in case that the rendering engine wasn't able to acquire threads. This
  //! flag only makes sense when the asynchronous mode was specified by having `thread_count` greater than 0. If the
  //! rendering context fails to acquire at least one thread it would fallback to synchronous mode with no worker
  //! threads.
  //!
  //! \note If this flag is specified with `thread_count == 1` it means to immediately fallback to synchronous
  //! rendering. It's only practical to use this flag with 2 or more requested threads.
  BL_CONTEXT_CREATE_FLAG_FALLBACK_TO_SYNC = 0x00100000u,

  //! If this flag is specified and asynchronous rendering is enabled then the context would create its own isolated
  //! thread-pool, which is useful for debugging purposes.
  //!
  //! Do not use this flag in production as rendering contexts with isolated thread-pool have to create and destroy all
  //! threads they use. This flag is only useful for testing, debugging, and isolated benchmarking.
  BL_CONTEXT_CREATE_FLAG_ISOLATED_THREAD_POOL = 0x01000000u,

  //! If this flag is specified and JIT pipeline generation enabled then the rendering context would create its own
  //! isolated JIT runtime. which is useful for debugging purposes. This flag will be ignored if JIT pipeline
  //! compilation is either not supported or was disabled by other flags.
  //!
  //! Do not use this flag in production as rendering contexts with isolated JIT runtime do not use global pipeline
  //! cache, that's it, after the rendering context is destroyed the JIT runtime is destroyed with it with all
  //! compiled pipelines. This flag is only useful for testing, debugging, and isolated benchmarking.
  BL_CONTEXT_CREATE_FLAG_ISOLATED_JIT_RUNTIME = 0x02000000u,

  //! Enables logging to stderr of isolated runtime.
  //!
  //! \note Must be used with \ref BL_CONTEXT_CREATE_FLAG_ISOLATED_JIT_RUNTIME otherwise it would have no effect.
  BL_CONTEXT_CREATE_FLAG_ISOLATED_JIT_LOGGING = 0x04000000u,

  //! Override CPU features when creating isolated context.
  BL_CONTEXT_CREATE_FLAG_OVERRIDE_CPU_FEATURES = 0x08000000u

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_CREATE_FLAG)
};

//! Error flags that are accumulated during the rendering context lifetime and that can be queried through
//! \ref BLContext::accumulated_error_flags(). The reason why these flags exist is that errors can happen during
//! asynchronous rendering, and there is no way the user can catch these errors.
BL_DEFINE_ENUM(BLContextErrorFlags) {
  //! No flags.
  BL_CONTEXT_ERROR_NO_FLAGS = 0u,

  //! The rendering context returned or encountered \ref BL_ERROR_INVALID_VALUE, which is mostly related to
  //! the function argument handling. It's very likely some argument was wrong when calling \ref BLContext API.
  BL_CONTEXT_ERROR_FLAG_INVALID_VALUE = 0x00000001u,

  //! Invalid state describes something wrong, for example a pipeline compilation error.
  BL_CONTEXT_ERROR_FLAG_INVALID_STATE = 0x00000002u,

  //! The rendering context has encountered invalid geometry.
  BL_CONTEXT_ERROR_FLAG_INVALID_GEOMETRY = 0x00000004u,

  //! The rendering context has encountered invalid glyph.
  BL_CONTEXT_ERROR_FLAG_INVALID_GLYPH = 0x00000008u,

  //! The rendering context has encountered invalid or uninitialized font.
  BL_CONTEXT_ERROR_FLAG_INVALID_FONT = 0x00000010u,

  //! Thread pool was exhausted and couldn't acquire the requested number of threads.
  BL_CONTEXT_ERROR_FLAG_THREAD_POOL_EXHAUSTED = 0x20000000u,

  //! Out of memory condition.
  BL_CONTEXT_ERROR_FLAG_OUT_OF_MEMORY = 0x40000000u,

  //! Unknown error, which we don't have flag for.
  BL_CONTEXT_ERROR_FLAG_UNKNOWN_ERROR = 0x80000000u

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_ERROR_FLAG)
};

//! Specifies the behavior of \ref BLContext::swap_styles() operation.
BL_DEFINE_ENUM(BLContextStyleSwapMode) {
  //! Swap only fill and stroke styles without affecting fill and stroke alpha.
  BL_CONTEXT_STYLE_SWAP_MODE_STYLES = 0,

  //! Swap both fill and stroke styles and their alpha values.
  BL_CONTEXT_STYLE_SWAP_MODE_STYLES_WITH_ALPHA = 1,

  //! Maximum value of `BLContextStyleSwapMode`.
  BL_CONTEXT_STYLE_SWAP_MODE_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_STYLE_SWAP_MODE)
};

//! Specifies how style transformation matrix is combined with the rendering context transformation matrix, used by
//! \ref BLContext::set_style() function.
BL_DEFINE_ENUM(BLContextStyleTransformMode) {
  //! Style transformation matrix should be transformed with the rendering context user and meta matrix (default).
  //!
  //! \note This transformation mode is identical to how user geometry is transformed and it's the default
  //! transformation and most likely the behavior expected in most cases.
  BL_CONTEXT_STYLE_TRANSFORM_MODE_USER = 0,

  //! Style transformation matrix should be transformed with the rendering context meta matrix.
  BL_CONTEXT_STYLE_TRANSFORM_MODE_META = 1,

  //! Style transformation matrix is considered absolute, and is not combined with a rendering context transform.
  BL_CONTEXT_STYLE_TRANSFORM_MODE_NONE = 2,

  //! Maximum value of `BLContextStyleTransformMode`.
  BL_CONTEXT_STYLE_TRANSFORM_MODE_MAX_VALUE = 2

  BL_FORCE_ENUM_UINT32(BL_CONTEXT_STYLE_TRANSFORM_MODE)
};

//! Clip mode.
BL_DEFINE_ENUM(BLClipMode) {
  //! Clipping to a rectangle that is aligned to the pixel grid.
  BL_CLIP_MODE_ALIGNED_RECT = 0,
  //! Clipping to a rectangle that is not aligned to pixel grid.
  BL_CLIP_MODE_UNALIGNED_RECT = 1,
  //! Clipping to a non-rectangular area that is defined by using mask.
  BL_CLIP_MODE_MASK = 2,

  //! Count of clip modes.
  BL_CLIP_MODE_COUNT = 3

  BL_FORCE_ENUM_UINT32(BL_CLIP_MODE)
};

//! Composition & blending operator.
BL_DEFINE_ENUM(BLCompOp) {
  //! Source-over [default].
  BL_COMP_OP_SRC_OVER = 0,
  //! Source-copy.
  BL_COMP_OP_SRC_COPY = 1,
  //! Source-in.
  BL_COMP_OP_SRC_IN = 2,
  //! Source-out.
  BL_COMP_OP_SRC_OUT = 3,
  //! Source-atop.
  BL_COMP_OP_SRC_ATOP = 4,
  //! Destination-over.
  BL_COMP_OP_DST_OVER = 5,
  //! Destination-copy [nop].
  BL_COMP_OP_DST_COPY = 6,
  //! Destination-in.
  BL_COMP_OP_DST_IN = 7,
  //! Destination-out.
  BL_COMP_OP_DST_OUT = 8,
  //! Destination-atop.
  BL_COMP_OP_DST_ATOP = 9,
  //! Xor.
  BL_COMP_OP_XOR = 10,
  //! Clear.
  BL_COMP_OP_CLEAR = 11,
  //! Plus.
  BL_COMP_OP_PLUS = 12,
  //! Minus.
  BL_COMP_OP_MINUS = 13,
  //! Modulate.
  BL_COMP_OP_MODULATE = 14,
  //! Multiply.
  BL_COMP_OP_MULTIPLY = 15,
  //! Screen.
  BL_COMP_OP_SCREEN = 16,
  //! Overlay.
  BL_COMP_OP_OVERLAY = 17,
  //! Darken.
  BL_COMP_OP_DARKEN = 18,
  //! Lighten.
  BL_COMP_OP_LIGHTEN = 19,
  //! Color dodge.
  BL_COMP_OP_COLOR_DODGE = 20,
  //! Color burn.
  BL_COMP_OP_COLOR_BURN = 21,
  //! Linear burn.
  BL_COMP_OP_LINEAR_BURN = 22,
  //! Linear light.
  BL_COMP_OP_LINEAR_LIGHT = 23,
  //! Pin light.
  BL_COMP_OP_PIN_LIGHT = 24,
  //! Hard-light.
  BL_COMP_OP_HARD_LIGHT = 25,
  //! Soft-light.
  BL_COMP_OP_SOFT_LIGHT = 26,
  //! Difference.
  BL_COMP_OP_DIFFERENCE = 27,
  //! Exclusion.
  BL_COMP_OP_EXCLUSION = 28,

  //! Count of composition & blending operators.
  BL_COMP_OP_MAX_VALUE = 28

  BL_FORCE_ENUM_UINT32(BL_COMP_OP)
};

//! Rendering quality.
BL_DEFINE_ENUM(BLRenderingQuality) {
  //! Render using anti-aliasing.
  BL_RENDERING_QUALITY_ANTIALIAS = 0,

  //! Maximum value of `BLRenderingQuality`.
  BL_RENDERING_QUALITY_MAX_VALUE = 0

  BL_FORCE_ENUM_UINT32(BL_RENDERING_QUALITY)
};

//! \}

//! \name BLContext - Structs
//! \{

//! Information that can be used to customize the rendering context.
struct BLContextCreateInfo {
  //! Create flags, see \ref BLContextCreateFlags.
  uint32_t flags;

  //! Number of worker threads to use for asynchronous rendering, if non-zero.
  //!
  //! If `thread_count` is zero it means to initialize the context for synchronous rendering. This means that every
  //! operation will take effect immediately. If `thread_count` is `1` it means that the rendering will be asynchronous,
  //! but no thread would be acquired from a thread-pool, because the user thread will be used as a worker. And
  //! finally, if `thread_count` is greater than `1` then total of `thread_count - 1` threads will be acquired from
  //! thread-pool and used as additional workers.
  uint32_t thread_count;

  //! CPU features to use in isolated JIT runtime (if supported), only used when `flags` contains
  //! \ref BL_CONTEXT_CREATE_FLAG_OVERRIDE_CPU_FEATURES.
  uint32_t cpu_features;

  //! Maximum number of commands to be queued.
  //!
  //! If this parameter is zero the queue size will be determined automatically.
  //!
  //! TODO: To be documented, has no effect at the moment.
  uint32_t command_queue_limit;

  //! Maximum number of saved states.
  //!
  //! \note Zero value tells the rendering engine to use the default saved state limit, which currently defaults
  //! to 4096 states. This option allows to even increase or decrease the limit, depending on the use case.
  uint32_t saved_state_limit;

  //! Pixel origin.
  //!
  //! Pixel origin is an offset in pixel units that can be used as an origin for fetchers and effects that use a pixel
  //! X/Y coordinate in the calculation. One example of using pixel origin is dithering, where it's used to shift the
  //! dithering matrix.
  BLPointI pixel_origin;

  //! Reserved for future use, must be zero.
  uint32_t reserved[1];

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLContextCreateInfo{}; }
#endif
};

//! Holds an arbitrary 128-bit value (cookie) that can be used to match other cookies. Blend2D uses cookies in places
//! where it allows to "lock" some state that can only be unlocked by a matching cookie. Please don't confuse cookies
//! with a security of any kind, it's just an arbitrary data that must match to proceed with a certain operation.
//!
//! Cookies can be used with \ref BLContext::save() and \ref BLContext::restore() operations.
struct BLContextCookie {
  uint64_t data[2];

#ifdef __cplusplus
  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLContextCookie& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLContextCookie& other) const noexcept { return !equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept {
    return data[0] == 0 && data[1] == 0;
  }

  BL_INLINE_NODEBUG void reset() noexcept { reset(0, 0); }
  BL_INLINE_NODEBUG void reset(const BLContextCookie& other) noexcept { reset(other.data[0], other.data[1]); }

  BL_INLINE_NODEBUG void reset(uint64_t data0_init, uint64_t data1_init) noexcept {
    data[0] = data0_init;
    data[1] = data1_init;
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLContextCookie& other) const noexcept {
    return BLInternal::bool_and(bl_equals(data[0], other.data[0]),
                                bl_equals(data[1], other.data[1]));
  }

  #endif
};

//! Rendering context hints.
struct BLContextHints {
  union {
    struct {
      uint8_t rendering_quality;
      uint8_t gradient_quality;
      uint8_t pattern_quality;
    };

    uint8_t hints[BL_CONTEXT_HINT_MAX_VALUE + 1];
  };

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLContextHints{}; }
#endif
};

//! \}

//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLContext - C API
//! \{

//! Rendering context [C API].
struct BLContextCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLContext)

#ifdef __cplusplus
  //! \name Impl Utilities
  //! \{

  //! Returns Impl of the rendering context (only provided for use cases that implement BLContext).
  template<typename T = BLContextImpl>
  [[nodiscard]]
  BL_INLINE_NODEBUG T* _impl() const noexcept { return static_cast<T*>(_d.impl); }

  //! \}
#endif
};

//! \cond INTERNAL
//! Rendering context [Virtual Function Table].
struct BLContextVirt BL_CLASS_INHERITS(BLObjectVirt) {
  BL_DEFINE_VIRT_BASE

  // Interface - Most Used Functions
  // -------------------------------

  // NOTE 1: These functions are called directly by the BLContext C++ API (the dispatch is inlined). So in general
  // on x86 targets the compiler will generate something like `call [base + offset]` instruction to perform the call.
  // We want to have the most used functions first as these would use 8-bit offset instead of 32-bit offset. We have
  // space for 12 functions as 8-bit offset is signed (from -128 to 127) and the BLObjectVirt already uses 3 functions.
  //
  // NOTE 2: On non-X86 platforms such as AArch64 we don't have to worry about offsets as the instruction would be
  // encoded in 32-bits regardless of the offset.

  BLResult (BL_CDECL* apply_transform_op         )(BLContextImpl* impl, BLTransformOp op_type, const void* op_data) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_rect_i                )(BLContextImpl* impl, const BLRectI* rect) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_rect_i_rgba32         )(BLContextImpl* impl, const BLRectI* rect, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_rect_i_ext            )(BLContextImpl* impl, const BLRectI* rect, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_rect_d                )(BLContextImpl* impl, const BLRect* rect) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_rect_d_rgba32         )(BLContextImpl* impl, const BLRect* rect, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_rect_d_ext            )(BLContextImpl* impl, const BLRect* rect, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_path_d                )(BLContextImpl* impl, const BLPoint* origin, const BLPathCore* path) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_path_d_rgba32         )(BLContextImpl* impl, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_path_d_ext            )(BLContextImpl* impl, const BLPoint* origin, const BLPathCore* path, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* blit_image_i               )(BLContextImpl* impl, const BLPointI* origin, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* blit_scaled_image_i        )(BLContextImpl* impl, const BLRectI* rect, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;

  // Interface
  // ---------

  BLResult (BL_CDECL* flush                      )(BLContextImpl* impl, BLContextFlushFlags flags) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* save                       )(BLContextImpl* impl, BLContextCookie* cookie) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* restore                    )(BLContextImpl* impl, const BLContextCookie* cookie) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* user_to_meta               )(BLContextImpl* impl) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* set_hint                   )(BLContextImpl* impl, BLContextHint hint_type, uint32_t value) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_hints                  )(BLContextImpl* impl, const BLContextHints* hints) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_flatten_mode           )(BLContextImpl* impl, BLFlattenMode mode) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_flatten_tolerance      )(BLContextImpl* impl, double tolerance) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_approximation_options  )(BLContextImpl* impl, const BLApproximationOptions* options) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* get_style                  )(const BLContextImpl* impl, BLContextStyleSlot slot, bool transformed, BLVarCore* style_out) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_style                  )(BLContextImpl* impl, BLContextStyleSlot slot, const BLObjectCore* style, BLContextStyleTransformMode transform_mode) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_style_rgba             )(BLContextImpl* impl, BLContextStyleSlot slot, const BLRgba* rgba) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_style_rgba32           )(BLContextImpl* impl, BLContextStyleSlot slot, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_style_rgba64           )(BLContextImpl* impl, BLContextStyleSlot slot, uint64_t rgba64) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* disable_style              )(BLContextImpl* impl, BLContextStyleSlot slot) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_style_alpha            )(BLContextImpl* impl, BLContextStyleSlot slot, double alpha) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* swap_styles                )(BLContextImpl* impl, BLContextStyleSwapMode mode) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* set_global_alpha           )(BLContextImpl* impl, double alpha) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_comp_op                )(BLContextImpl* impl, BLCompOp comp_op) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* set_fill_rule              )(BLContextImpl* impl, BLFillRule fill_rule) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_width           )(BLContextImpl* impl, double width) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_miter_limit     )(BLContextImpl* impl, double miter_limit) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_cap             )(BLContextImpl* impl, BLStrokeCapPosition position, BLStrokeCap stroke_cap) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_caps            )(BLContextImpl* impl, BLStrokeCap stroke_cap) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_join            )(BLContextImpl* impl, BLStrokeJoin stroke_join) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_dash_offset     )(BLContextImpl* impl, double dash_offset) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_dash_array      )(BLContextImpl* impl, const BLArrayCore* dash_array) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_transform_order )(BLContextImpl* impl, BLStrokeTransformOrder transform_order) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* set_stroke_options         )(BLContextImpl* impl, const BLStrokeOptionsCore* options) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* clip_to_rect_i             )(BLContextImpl* impl, const BLRectI* rect) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* clip_to_rect_d             )(BLContextImpl* impl, const BLRect* rect) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* restore_clipping           )(BLContextImpl* impl) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* clear_all                  )(BLContextImpl* impl) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* clear_recti                )(BLContextImpl* impl, const BLRectI* rect) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* clear_rectd                )(BLContextImpl* impl, const BLRect* rect) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_all                   )(BLContextImpl* impl) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_all_rgba32            )(BLContextImpl* impl, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_all_ext               )(BLContextImpl* impl, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_geometry              )(BLContextImpl* impl, BLGeometryType type, const void* data) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_geometry_rgba32       )(BLContextImpl* impl, BLGeometryType type, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_geometry_ext          )(BLContextImpl* impl, BLGeometryType type, const void* data, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_text_op_i             )(BLContextImpl* self, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_text_op_i_rgba32      )(BLContextImpl* self, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_text_op_i_ext         )(BLContextImpl* self, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_text_op_d             )(BLContextImpl* self, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_text_op_d_rgba32      )(BLContextImpl* self, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_text_op_d_ext         )(BLContextImpl* self, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_mask_i                )(BLContextImpl* impl, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_mask_i_rgba32         )(BLContextImpl* impl, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_mask_i_ext            )(BLContextImpl* impl, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* fill_mask_d                )(BLContextImpl* impl, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_mask_d_Rgba32         )(BLContextImpl* impl, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* fill_mask_d_ext            )(BLContextImpl* impl, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* stroke_path_d              )(BLContextImpl* impl, const BLPoint* origin, const BLPathCore* path) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_path_d_rgba32       )(BLContextImpl* impl, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_path_d_ext          )(BLContextImpl* impl, const BLPoint* origin, const BLPathCore* path, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* stroke_geometry            )(BLContextImpl* impl, BLGeometryType type, const void* data) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_geometry_rgba32     )(BLContextImpl* impl, BLGeometryType type, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_geometry_ext        )(BLContextImpl* impl, BLGeometryType type, const void* data, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* stroke_text_op_i           )(BLContextImpl* self, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_text_op_i_rgba32    )(BLContextImpl* self, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_text_op_i_ext       )(BLContextImpl* self, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* stroke_text_op_d           )(BLContextImpl* self, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_text_op_d_rgba32    )(BLContextImpl* self, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* stroke_text_op_d_ext       )(BLContextImpl* self, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op, const void* data, const BLObjectCore* style) BL_NOEXCEPT_C;

  BLResult (BL_CDECL* blit_image_d               )(BLContextImpl* impl, const BLPoint* origin, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* blit_scaled_image_d        )(BLContextImpl* impl, const BLRect* rect, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;
};

//! Rendering context state.
//!
//! This state is not meant to be created by users, it's only provided for users that want to introspect
//! the rendering context state and for C++ API that accesses it directly for performance reasons.
struct BLContextState {
  //! Target image or image object with nullptr impl in case that the rendering context doesn't render to an image.
  BLImageCore* target_image;
  //! Current size of the target in abstract units, pixels if rendering to \ref BLImage.
  BLSize target_size;

  //! Current rendering context hints.
  BLContextHints hints;
  //! Current composition operator.
  uint8_t comp_op;
  //! Current fill rule.
  uint8_t fill_rule;
  //! Current type of a style object of fill and stroke operations indexed by \ref BLContextStyleSlot.
  uint8_t style_type[2];
  //! Count of saved states in the context.
  uint32_t saved_state_count;

  //! Current global alpha value [0, 1].
  double global_alpha;
  //! Current fill or stroke alpha indexed by style slot, see \ref BLContextStyleSlot.
  double style_alpha[2];

  //! Current stroke options.
  BLStrokeOptionsCore stroke_options;

  //! Current approximation options.
  BLApproximationOptions approximation_options;

  //! Current meta transformation matrix.
  BLMatrix2D meta_transform;
  //! Current user transformation matrix.
  BLMatrix2D user_transform;
  //! Current final transformation matrix, which combines all transformation matrices.
  BLMatrix2D final_transform;
};

//! Rendering context [C API Impl].
struct BLContextImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Virtual function table.
  const BLContextVirt* virt;
  //! Current state of the context.
  const BLContextState* state;

  //! Type of the rendering context, see \ref BLContextType.
  uint32_t context_type;
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_context_init(BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_init_move(BLContextCore* self, BLContextCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_init_weak(BLContextCore* self, const BLContextCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_init_as(BLContextCore* self, BLImageCore* image, const BLContextCreateInfo* cci) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_destroy(BLContextCore* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_reset(BLContextCore* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_assign_move(BLContextCore* self, BLContextCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_assign_weak(BLContextCore* self, const BLContextCore* other) BL_NOEXCEPT_C;

BL_API BLContextType BL_CDECL bl_context_get_type(const BLContextCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_context_get_target_size(const BLContextCore* self, BLSize* target_size_out) BL_NOEXCEPT_C;
BL_API BLImageCore* BL_CDECL bl_context_get_target_image(const BLContextCore* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_begin(BLContextCore* self, BLImageCore* image, const BLContextCreateInfo* cci) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_end(BLContextCore* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_flush(BLContextCore* self, BLContextFlushFlags flags) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_save(BLContextCore* self, BLContextCookie* cookie) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_restore(BLContextCore* self, const BLContextCookie* cookie) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_get_meta_transform(const BLContextCore* self, BLMatrix2D* transform_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_get_user_transform(const BLContextCore* self, BLMatrix2D* transform_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_get_final_transform(const BLContextCore* self, BLMatrix2D* transform_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_user_to_meta(BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_apply_transform_op(BLContextCore* self, BLTransformOp op_type, const void* op_data) BL_NOEXCEPT_C;

BL_API uint32_t BL_CDECL bl_context_get_hint(const BLContextCore* self, BLContextHint hint_type) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_hint(BLContextCore* self, BLContextHint hint_type, uint32_t value) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_get_hints(const BLContextCore* self, BLContextHints* hints_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_hints(BLContextCore* self, const BLContextHints* hints) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_set_flatten_mode(BLContextCore* self, BLFlattenMode mode) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_flatten_tolerance(BLContextCore* self, double tolerance) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_approximation_options(BLContextCore* self, const BLApproximationOptions* options) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_get_fill_style(const BLContextCore* self, BLVarCore* style_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_get_transformed_fill_style(const BLContextCore* self, BLVarCore* style_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_fill_style(BLContextCore* self, const BLUnknown* style) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_fill_style_with_mode(BLContextCore* self, const BLUnknown* style, BLContextStyleTransformMode transform_mode) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_fill_style_rgba(BLContextCore* self, const BLRgba* rgba) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_fill_style_rgba32(BLContextCore* self, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_fill_style_rgba64(BLContextCore* self, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_disable_fill_style(BLContextCore* self) BL_NOEXCEPT_C;
BL_API double BL_CDECL bl_context_get_fill_alpha(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_fill_alpha(BLContextCore* self, double alpha) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_get_stroke_style(const BLContextCore* self, BLVarCore* style_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_get_transformed_stroke_style(const BLContextCore* self, BLVarCore* style_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_style(BLContextCore* self, const BLUnknown* style) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_style_with_mode(BLContextCore* self, const BLUnknown* style, BLContextStyleTransformMode transform_mode) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_style_rgba(BLContextCore* self, const BLRgba* rgba) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_style_rgba32(BLContextCore* self, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_style_rgba64(BLContextCore* self, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_disable_stroke_style(BLContextCore* self) BL_NOEXCEPT_C;
BL_API double BL_CDECL bl_context_get_stroke_alpha(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_alpha(BLContextCore* self, double alpha) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_swap_styles(BLContextCore* self, BLContextStyleSwapMode mode) BL_NOEXCEPT_C;

BL_API double BL_CDECL bl_context_get_global_alpha(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_global_alpha(BLContextCore* self, double alpha) BL_NOEXCEPT_C;

BL_API BLCompOp BL_CDECL bl_context_get_comp_op(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_comp_op(BLContextCore* self, BLCompOp comp_op) BL_NOEXCEPT_C;

BL_API BLFillRule BL_CDECL bl_context_get_fill_rule(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_fill_rule(BLContextCore* self, BLFillRule fill_rule) BL_NOEXCEPT_C;

BL_API double BL_CDECL bl_context_get_stroke_width(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_width(BLContextCore* self, double width) BL_NOEXCEPT_C;

BL_API double BL_CDECL bl_context_get_stroke_miter_limit(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_miter_limit(BLContextCore* self, double miter_limit) BL_NOEXCEPT_C;

BL_API BLStrokeCap BL_CDECL bl_context_get_stroke_cap(const BLContextCore* self, BLStrokeCapPosition position) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_cap(BLContextCore* self, BLStrokeCapPosition position, BLStrokeCap stroke_cap) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_caps(BLContextCore* self, BLStrokeCap stroke_cap) BL_NOEXCEPT_C;

BL_API BLStrokeJoin BL_CDECL bl_context_get_stroke_join(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_join(BLContextCore* self, BLStrokeJoin stroke_join) BL_NOEXCEPT_C;

BL_API BLStrokeTransformOrder BL_CDECL bl_context_get_stroke_transform_order(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_transform_order(BLContextCore* self, BLStrokeTransformOrder transform_order) BL_NOEXCEPT_C;

BL_API double BL_CDECL bl_context_get_stroke_dash_offset(const BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_dash_offset(BLContextCore* self, double dash_offset) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_get_stroke_dash_array(const BLContextCore* self, BLArrayCore* dash_array_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_dash_array(BLContextCore* self, const BLArrayCore* dash_array) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_get_stroke_options(const BLContextCore* self, BLStrokeOptionsCore* options) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_set_stroke_options(BLContextCore* self, const BLStrokeOptionsCore* options) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_clip_to_rect_i(BLContextCore* self, const BLRectI* rect) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_clip_to_rect_d(BLContextCore* self, const BLRect* rect) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_restore_clipping(BLContextCore* self) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_clear_all(BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_clear_rect_i(BLContextCore* self, const BLRectI* rect) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_clear_rect_d(BLContextCore* self, const BLRect* rect) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_all(BLContextCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_all_rgba32(BLContextCore* self, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_all_rgba64(BLContextCore* self, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_all_ext(BLContextCore* self, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_rect_i(BLContextCore* self, const BLRectI* rect) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_rect_i_rgba32(BLContextCore* self, const BLRectI* rect, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_rect_i_rgba64(BLContextCore* self, const BLRectI* rect, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_rect_i_ext(BLContextCore* self, const BLRectI* rect, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_rect_d(BLContextCore* self, const BLRect* rect) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_rect_d_rgba32(BLContextCore* self, const BLRect* rect, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_rect_d_rgba64(BLContextCore* self, const BLRect* rect, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_rect_d_ext(BLContextCore* self, const BLRect* rect, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_path_d(BLContextCore* self, const BLPoint* origin, const BLPathCore* path) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_path_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_path_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_path_d_ext(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_geometry(BLContextCore* self, BLGeometryType type, const void* data) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_geometry_rgba32(BLContextCore* self, BLGeometryType type, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_geometry_rgba64(BLContextCore* self, BLGeometryType type, const void* data, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_geometry_ext(BLContextCore* self, BLGeometryType type, const void* data, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_utf8_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf8_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf8_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf8_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_utf8_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf8_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf8_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf8_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_utf16_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf16_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf16_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf16_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_utf16_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf16_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf16_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf16_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_utf32_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf32_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf32_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf32_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_utf32_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf32_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf32_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_utf32_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_glyph_run_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_glyph_run_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_glyph_run_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_glyph_run_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_glyph_run_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_glyph_run_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_glyph_run_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_glyph_run_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_mask_i(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_mask_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_mask_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_mask_i_ext(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_fill_mask_d(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_mask_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_mask_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_fill_mask_d_ext(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_rect_i(BLContextCore* self, const BLRectI* rect) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_rect_i_rgba32(BLContextCore* self, const BLRectI* rect, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_rect_i_rgba64(BLContextCore* self, const BLRectI* rect, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_rect_i_ext(BLContextCore* self, const BLRectI* rect, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_rect_d(BLContextCore* self, const BLRect* rect) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_rect_d_rgba32(BLContextCore* self, const BLRect* rect, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_rect_d_rgba64(BLContextCore* self, const BLRect* rect, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_rect_d_ext(BLContextCore* self, const BLRect* rect, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_path_d(BLContextCore* self, const BLPoint* origin, const BLPathCore* path) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_path_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_path_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_path_d_ext(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_geometry(BLContextCore* self, BLGeometryType type, const void* data) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_geometry_rgba32(BLContextCore* self, BLGeometryType type, const void* data, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_geometry_rgba64(BLContextCore* self, BLGeometryType type, const void* data, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_geometry_ext(BLContextCore* self, BLGeometryType type, const void* data, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf8_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf16_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_utf32_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_stroke_glyph_run_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_context_blit_image_i(BLContextCore* self, const BLPointI* origin, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_blit_image_d(BLContextCore* self, const BLPoint* origin, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_blit_scaled_image_i(BLContextCore* self, const BLRectI* rect, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_context_blit_scaled_image_d(BLContextCore* self, const BLRect* rect, const BLImageCore* img, const BLRectI* img_area) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_rendering
//! \{

#ifdef __cplusplus

//! \cond INTERNAL

namespace BLInternal {

static BL_INLINE BLVarCore make_inline_style(const BLRgba32& rgba32) noexcept {
  BLVarCore style;
  style._d.init_rgba32(rgba32.value);
  return style;
}

static BL_INLINE BLVarCore make_inline_style(const BLRgba64& rgba64) noexcept {
  BLVarCore style;
  style._d.init_rgba64(rgba64.value);
  return style;
}

static BL_INLINE BLVarCore make_inline_style(const BLRgba& rgba) noexcept {
  uint32_t r = bl_bit_cast<uint32_t>(rgba.r);
  uint32_t g = bl_bit_cast<uint32_t>(rgba.g);
  uint32_t b = bl_bit_cast<uint32_t>(rgba.b);
  uint32_t a = bl_bit_cast<uint32_t>(bl_max(0.0f, rgba.a)) & 0x7FFFFFFFu;

  BLVarCore style;
  style._d.init_u32x4(r, g, b, a);
  return style;
}

template<typename StyleT> struct ForwardedStyleOf { typedef const StyleT& Type; };
template<> struct ForwardedStyleOf<BLRgba> { typedef BLVarCore Type; };
template<> struct ForwardedStyleOf<BLRgba32> { typedef BLVarCore Type; };
template<> struct ForwardedStyleOf<BLRgba64> { typedef BLVarCore Type; };

static BL_INLINE_NODEBUG BLVarCore forward_style(const BLRgba& rgba) { return make_inline_style(rgba); }
static BL_INLINE_NODEBUG BLVarCore forward_style(const BLRgba32& rgba) { return make_inline_style(rgba); }
static BL_INLINE_NODEBUG BLVarCore forward_style(const BLRgba64& rgba) { return make_inline_style(rgba); }

static BL_INLINE_NODEBUG const BLVarCore& forward_style(const BLVarCore& var) { return var; }
static BL_INLINE_NODEBUG const BLPatternCore& forward_style(const BLPatternCore& pattern) { return pattern; }
static BL_INLINE_NODEBUG const BLGradientCore& forward_style(const BLGradientCore& gradient) { return gradient; }

static BL_INLINE_NODEBUG const BLVar& forward_style(const BLVar& var) { return var; }
static BL_INLINE_NODEBUG const BLPattern& forward_style(const BLPattern& pattern) { return pattern; }
static BL_INLINE_NODEBUG const BLGradient& forward_style(const BLGradient& gradient) { return gradient; }

} // {BLInternal}

//! \endcond

//! \name BLContext - C++ API
//! \{

//! Rendering context [C++ API].
class BLContext /* final */ : public BLContextCore {
public:
  //! \cond INTERNAL

  static inline constexpr uint32_t kDefaultSignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_CONTEXT) | BL_OBJECT_INFO_D_FLAG;

  // This macro is here so stepping-in into BLContext API doesn't step into _impl() method.
  #define BL_CONTEXT_IMPL() static_cast<BLContextImpl*>(_d.impl)

  // Optimize the virtual call dispatch (only clang currently understands this optimization).
#if defined(__clang__)
  #define BL_CONTEXT_CALL_RETURN(fn, ...)          \
    BLContextImpl* impl = BL_CONTEXT_IMPL();       \
    const BLContextVirt* virt = impl->virt;        \
    const BLContextState* state = impl->state;     \
                                                   \
    BLResult result = virt->fn(__VA_ARGS__);       \
                                                   \
    BL_ASSUME(impl == BL_CONTEXT_IMPL());          \
    BL_ASSUME(virt == BL_CONTEXT_IMPL()->virt);    \
    BL_ASSUME(state == BL_CONTEXT_IMPL()->state);  \
                                                   \
    return result
#else
  #define BL_CONTEXT_CALL_RETURN(fn, ...)          \
    BLContextImpl* impl = BL_CONTEXT_IMPL();       \
    return impl->virt->fn(__VA_ARGS__)
#endif
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed rendering context.
  //!
  //! Default constructed means that the instance is valid, but uninitialized,  which means the rendering context does
  //! not have attached any target. Any attempt to use uninitialized context results in \ref BL_ERROR_NOT_INITIALIZED
  //! error.
  BL_INLINE_NODEBUG BLContext() noexcept {
    bl_context_init(this);

    BL_ASSUME(_d.info.bits == kDefaultSignature);
  }

  //! Move constructor.
  //!
  //! Moves the `other` rendering context into this one and resets the `other` rendering context to
  //! a default-constructed state. This is an efficient way of "moving" the rendering context as it doesn't touch its
  //! internal reference count, which is atomic, because moving is internally implemented as a trivial memory copy.
  BL_INLINE_NODEBUG BLContext(BLContext&& other) noexcept {
    bl_context_init_move(this, &other);
  }

  //! Copy constructor.
  //!
  //! Creates a weak-copy of the `other` rendering context by increasing it's internal reference counter. This context
  //! and `other` would point to the same data and would be otherwise identical. Any change to `other` would also
  //! affect this context.
  //!
  //! This function is mostly provided for C++ users that may keep a global reference to the same rendering context,
  //! for example, otherwise sharing is not that useful as the rendering context has states that are manipulated during
  //! rendering.
  //!
  //! Two weak copies of the same rendering context cannot be used by different threads simultaneously.
  BL_INLINE_NODEBUG BLContext(const BLContext& other) noexcept {
    bl_context_init_weak(this, &other);
  }

  //! Creates a new rendering context for rendering to the image `target`.
  //!
  //! This is a simplified constructor that can be used to create a rendering context without any additional parameters,
  //! which means that the rendering context will use a single-threaded synchronous rendering.
  //!
  //! \note Since Blend2D doesn't use exceptions in C++ this function always succeeds even when an error happened.
  //! The underlying C-API function \ref bl_context_init_as() returns an error, but it just cannot be propagated back.
  //! Use \ref begin() function, which returns a \ref BLResult to check the status of the call immediately.
  BL_INLINE_NODEBUG explicit BLContext(BLImageCore& target) noexcept {
    bl_context_init_as(this, &target, nullptr);
  }

  //! Creates a new rendering context for rendering to the image `target`.
  //!
  //! This is an advanced constructor that can be used to create a rendering context with additional parameters. These
  //! parameters can be used to specify the number of threads to be used during rendering and to select other features.
  //!
  //! \note Since Blend2D doesn't use exceptions in C++ this function always succeeds even when an error happened.
  //! The underlying C-API function \ref bl_context_init_as() returns an error, but it just cannot be propagated back.
  //! Use \ref begin() function, which returns a \ref BLResult to check the status of the call immediately.
  BL_INLINE_NODEBUG BLContext(BLImageCore& target, const BLContextCreateInfo& create_info) noexcept {
    bl_context_init_as(this, &target, &create_info);
  }

  //! \overload
  BL_INLINE_NODEBUG BLContext(BLImageCore& target, const BLContextCreateInfo* create_info) noexcept {
    bl_context_init_as(this, &target, create_info);
  }

  //! Destroys the rendering context.
  //!
  //! Waits for all operations, detaches the target from the rendering context and then destroys it. Does nothing if
  //! the context is not initialized.
  //!
  //! \note Destroying the rendering context would always internally call `flush(BL_CONTEXT_FLUSH_SYNC)`, which would
  //! flush the render calls queue in case multi-threaded rendering is used.
  BL_INLINE_NODEBUG ~BLContext() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_context_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Returns true if the rendering context is initialized (has target attached).
  //!
  //! Provided for users that want to use bool idiom in C++ to check for the status of the object.
  //!
  //! ```
  //! BLContext ctx(some_image);
  //! if (ctx) {
  //!   // Rendering context is initialized.
  //! }
  //! ```
  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return is_valid(); }

  //! Implements a copy-assignment operator.
  //!
  //! Copying a rendering context creates a weak-copy only, which means that all copied instances share the same
  //! underlying rendering context. The rendering context internally uses atomic reference counting to manage ots
  //! lifetime.
  BL_INLINE_NODEBUG BLContext& operator=(const BLContext& other) noexcept { bl_context_assign_weak(this, &other); return *this; }

  //! Implements a move-assignment operator.
  //!
  //! Moves the rendering context of `other` to this rendering context and makes the `other` rendering context
  //! default constructed (which uses an internal "null" implementation that renders to nothing).
  BL_INLINE_NODEBUG BLContext& operator=(BLContext&& other) noexcept { bl_context_assign_move(this, &other); return *this; }

  //! Returns whether this and `other` point to the same rendering context.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLContext& other) const noexcept { return equals(other); }

  //! Returns whether this and `other` are different rendering contexts.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLContext& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Target Information
  //! \{

  //! Returns the target size in abstract units (pixels in case of \ref BLImage).
  [[nodiscard]]
  BL_INLINE_NODEBUG BLSize target_size() const noexcept { return BL_CONTEXT_IMPL()->state->target_size; }

  //! Returns the target width in abstract units (pixels in case of \ref BLImage).
  [[nodiscard]]
  BL_INLINE_NODEBUG double target_width() const noexcept { return BL_CONTEXT_IMPL()->state->target_size.w; }

  //! Returns the target height in abstract units (pixels in case of \ref BLImage).
  [[nodiscard]]
  BL_INLINE_NODEBUG double target_height() const noexcept { return BL_CONTEXT_IMPL()->state->target_size.h; }

  //! Returns the target image or null if there is no target image.
  //!
  //! \note The rendering context doesn't own the image, but it increases its writer count, which means that the image
  //! will not be destroyed even when user destroys it during the rendering (in such case it will be destroyed after
  //! the rendering ends when the writer count goes to zero). This means that the rendering context must hold the image
  //! and not the pointer to the \ref BLImage passed to either the constructor or `begin()` function. So the returned
  //! pointer is not the same as the pointer passed to `begin()`, but it points to the same underlying data.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLImage* target_image() const noexcept {
    return static_cast<BLImage*>(BL_CONTEXT_IMPL()->state->target_image);
  }

  //! \}

  //! \name Context Lifetime and Others
  //! \{

  //! Returns the type of this context, see \ref BLContextType.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLContextType context_type() const noexcept {
    return BLContextType(BL_CONTEXT_IMPL()->context_type);
  }

  //! Tests whether the context is a valid rendering context that has attached target to it.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_valid() const noexcept {
    return context_type() != BL_CONTEXT_TYPE_NONE;
  }

  //! Returns whether this and `other` point to the same rendering context.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLContext& other) const noexcept {
    return this->_d.impl == other._d.impl;
  }

  //! Resets this rendering context to the default constructed one.
  //!
  //! Similar behavior to the destructor, but the rendering context will still be a valid object after the call to
  //! `reset()` and would behave like a default constructed context.
  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_context_reset(this);
    BL_ASSUME(result == BL_SUCCESS);
    BL_ASSUME(_d.info.bits == kDefaultSignature);
    return result;
  }

  //! Assigns the `other` rendering context to this rendering context.
  //!
  //! This is the same as using C++ copy-assignment operator, see \ref operator=().
  BL_INLINE_NODEBUG BLResult assign(const BLContext& other) noexcept {
    return bl_context_assign_weak(this, &other);
  }

  //! Moves the `other` rendering context to this rendering context, which would make ther `other` rendering context
  //! default initialized.
  //!
  //! This is the same as using C++ move-assignment operator, see \ref operator=().
  BL_INLINE_NODEBUG BLResult assign(BLContext&& other) noexcept {
    return bl_context_assign_move(this, &other);
  }

  //! Begins rendering to the given `image`.
  //!
  //! This is a simplified `begin()` function that can be used to create a rendering context without any additional
  //! parameters, which means that the rendering context will use a single-threaded synchronous rendering.
  //!
  //! If this operation succeeds then the rendering context will have exclusive access to the image data. This means
  //! that no other renderer can use it during rendering.
  BL_INLINE_NODEBUG BLResult begin(BLImageCore& image) noexcept {
    return bl_context_begin(this, &image, nullptr);
  }

  //! Begins rendering to the given `image`.
  //!
  //! This is an advanced `begin()` function that can be used to create a rendering context with additional parameters.
  //! These parameters can be used to specify the number of threads to be used during rendering and to select other
  //! features.
  //!
  //! If this operation succeeds then the rendering context will have exclusive access to the image data. This means
  //! that no other renderer can use it during rendering.
  BL_INLINE_NODEBUG BLResult begin(BLImageCore& image, const BLContextCreateInfo& create_info) noexcept {
    return bl_context_begin(this, &image, &create_info);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult begin(BLImageCore& image, const BLContextCreateInfo* create_info) noexcept {
    return bl_context_begin(this, &image, create_info);
  }

  //! Waits for completion of all render commands and detaches the rendering context from the rendering target.
  //! After `end()` completes the rendering context implementation would be released and replaced by a built-in
  //! null instance (no context).
  //!
  //! \note Calling `end()` would implicitly call `flush(BL_CONTEXT_FLUSH_SYNC)`, which would flush the render calls
  //! queue in case multi-threaded rendering is used.
  BL_INLINE_NODEBUG BLResult end() noexcept {
    BLResult result = bl_context_end(this);
    BL_ASSUME(_d.info.bits == kDefaultSignature);
    return result;
  }

  //! Flushes the context, see \ref BLContextFlushFlags.
  BL_INLINE_NODEBUG BLResult flush(BLContextFlushFlags flags) noexcept {
    BL_CONTEXT_CALL_RETURN(flush, impl, flags);
  }

  //! \}

  //! \name Properties
  //! \{

  BL_DEFINE_OBJECT_PROPERTY_API

  //! Queries the number of threads that the rendering context uses.
  //!
  //! If the returned value is zero it means that the rendering is synchronous, otherwise it describes the number of
  //! threads used for asynchronous rendering which include the user thread. For example if the returned value is `2`
  //! it means that the rendering context uses the user thread and one more worker.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t thread_count() const noexcept {
    static const char property_name[] = "thread_count";

    uint32_t value;
    bl_object_get_property_uint32(this, property_name, sizeof(property_name) - 1u, &value);
    return value;
  }

  //! Queries accumulated errors as flags, see \ref BLContextErrorFlags.
  //!
  //! Errors may accumulate during the lifetime of the rendering context.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLContextErrorFlags accumulated_error_flags() const noexcept {
    static const char property_name[] = "accumulated_error_flags";

    uint32_t value;
    bl_object_get_property_uint32(this, property_name, sizeof(property_name) - 1u, &value);
    return BLContextErrorFlags(value);
  }

  //! \}

  //! \name State Management
  //! \{

  //! Returns the number of saved states in the context (0 means no saved states).
  //!
  //! \note Each successful call to \ref save() increments the saved-state counter and each successful call to
  //! `restore()` decrements it. However, the calls must be successful as the rendering context allows to restrict
  //! the number of save states, for example, or to use a \ref BLContextCookie to guard state save and restoration.
  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t saved_state_count() const noexcept {
    return BL_CONTEXT_IMPL()->state->saved_state_count;
  }

  //! Saves the current rendering context state.
  //!
  //! Blend2D uses optimizations that make \ref save() a cheap operation. Only core values are actually saved in
  //! \ref save(), others will only be saved if they are modified. This means that consecutive calls to \ref save()
  //! and \ref restore() do almost nothing.
  BL_INLINE_NODEBUG BLResult save() noexcept {
    BL_CONTEXT_CALL_RETURN(save, impl, nullptr);
  }

  //! Saves the current rendering context state and creates a restoration `cookie`.
  //!
  //! If you use a `cookie` to save a state you have to use the same cookie to restore it otherwise the \ref restore()
  //! would fail. Please note that cookies are not a means of security, they are provided for making it easier to
  //! guarantee that a code that you may not control won't break your context.
  BL_INLINE_NODEBUG BLResult save(BLContextCookie& cookie) noexcept {
    BL_CONTEXT_CALL_RETURN(save, impl, &cookie);
  }

  //! Restores the top-most saved context-state.
  //!
  //! Possible return conditions:
  //!
  //!   - \ref BL_SUCCESS - State was restored successfully.
  //!   - \ref BL_ERROR_NO_STATES_TO_RESTORE - There are no saved states to restore.
  //!   - \ref BL_ERROR_NO_MATCHING_COOKIE - Previous state was saved with cookie, which was not provided. You would need
  //!     the correct cookie to restore such state.
  BL_INLINE_NODEBUG BLResult restore() noexcept {
    BL_CONTEXT_CALL_RETURN(restore, impl, nullptr);
  }

  //! Restores to the point that matches the given `cookie`.
  //!
  //! More than one state can be restored in case that the `cookie` points to some previous state in the list.
  //!
  //! Possible return conditions:
  //!
  //!   - \ref BL_SUCCESS - Matching state was restored successfully.
  //!   - \ref BL_ERROR_NO_STATES_TO_RESTORE - There are no saved states to restore.
  //!   - \ref BL_ERROR_NO_MATCHING_COOKIE - The cookie did't match any saved state.
  BL_INLINE_NODEBUG BLResult restore(const BLContextCookie& cookie) noexcept {
    BL_CONTEXT_CALL_RETURN(restore, impl, &cookie);
  }

  //! \}

  //! \cond INTERNAL
  //! \name Transformations (Internal)
  //! \{

  //! Applies a matrix operation to the current transformation matrix (internal).
  BL_INLINE_NODEBUG BLResult _apply_transform_op(BLTransformOp op_type, const void* op_data) noexcept {
    BL_CONTEXT_CALL_RETURN(apply_transform_op, impl, op_type, op_data);
  }

  //! Applies a matrix operation to the current transformation matrix (internal).
  template<typename... Args>
  BL_INLINE_NODEBUG BLResult _apply_transform_op_v(BLTransformOp op_type, Args&&... args) noexcept {
    double op_data[] = { double(args)... };
    BL_CONTEXT_CALL_RETURN(apply_transform_op, impl, op_type, op_data);
  }

  //! \}
  //! \endcond

  //! \name Transformations
  //! \{

  //! Returns a meta transformation matrix.
  //!
  //! Meta matrix is a core transformation matrix that is normally not changed by transformations applied to the
  //! context. Instead it acts as a secondary matrix used to create the final transformation matrix from meta and
  //! user matrices.
  //!
  //! Meta matrix can be used to scale the whole context for HI-DPI rendering or to change the orientation of the
  //! image being rendered, however, the number of use-cases is unlimited.
  //!
  //! To change the meta-matrix you must first change user-matrix and then call `user_to_meta()`, which would update
  //! meta-matrix and clear user-matrix.
  //!
  //! See `user_transform()` and `user_to_meta()`.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLMatrix2D& meta_transform() const noexcept { return BL_CONTEXT_IMPL()->state->meta_transform; }

  //! Returns a user transformation matrix.
  //!
  //! User matrix contains all transformations that happened to the rendering context unless the context was restored
  //! or `user_to_meta()` was called.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLMatrix2D& user_transform() const noexcept { return BL_CONTEXT_IMPL()->state->user_transform; }

  //! Returns a final transformation matrix.
  //!
  //! Final transformation matrix is a combination of meta and user transformation matrices. It's the final
  //! transformation that the rendering context applies to all input coordinates.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLMatrix2D& final_transform() const noexcept { return BL_CONTEXT_IMPL()->state->final_transform; }

  //! Sets user transformation matrix to `m`.
  //!
  //! \note This only assigns the user transformation matrix, which means that the meta transformation matrix is kept
  //! as is. This means that the final transformation matrix will be recalculated based on the given `transform`.
  BL_INLINE_NODEBUG BLResult set_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_ASSIGN, &transform); }

  //! Resets user transformation matrix to identity.
  //!
  //! \note This only resets the user transformation matrix, which means that the meta transformation matrix is kept
  //! as is. This means that the final transformation matrix after \ref reset_transform() would be the same as meta
  //! transformation matrix.
  BL_INLINE_NODEBUG BLResult reset_transform() noexcept { return _apply_transform_op(BL_TRANSFORM_OP_RESET, nullptr); }

  //! Translates the used transformation matrix by `[x, y]`.
  BL_INLINE_NODEBUG BLResult translate(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_TRANSLATE, x, y); }

  //! Translates the used transformation matrix by `[p]` (integer).
  BL_INLINE_NODEBUG BLResult translate(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_TRANSLATE, p.x, p.y); }

  //! Translates the used transformation matrix by `[p]` (floating-point).
  BL_INLINE_NODEBUG BLResult translate(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_TRANSLATE, &p); }

  //! Scales the user transformation matrix by `xy` (both X and Y is scaled by `xy`).
  BL_INLINE_NODEBUG BLResult scale(double xy) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, xy, xy); }

  //! Scales the user transformation matrix by `[x, y]`.
  BL_INLINE_NODEBUG BLResult scale(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, x, y); }

  //! Scales the user transformation matrix by `[p]` (integer).
  BL_INLINE_NODEBUG BLResult scale(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SCALE, p.x, p.y); }

  //! Scales the user transformation matrix by `[p]` (floating-point).
  BL_INLINE_NODEBUG BLResult scale(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_SCALE, &p); }

  //! Skews the user transformation matrix by `[x, y]`.
  BL_INLINE_NODEBUG BLResult skew(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_SKEW, x, y); }

  //! Skews the user transformation matrix by `[p]` (floating-point).
  BL_INLINE_NODEBUG BLResult skew(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_SKEW, &p); }

  //! Rotates the user transformation matrix by `angle`.
  BL_INLINE_NODEBUG BLResult rotate(double angle) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_ROTATE, &angle); }

  //! Rotates the user transformation matrix at `[x, y]` by `angle`.
  BL_INLINE_NODEBUG BLResult rotate(double angle, double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, x, y); }

  //! Rotates the user transformation matrix at `origin` (integer) by `angle`.
  BL_INLINE_NODEBUG BLResult rotate(double angle, const BLPoint& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, origin.x, origin.y); }

  //! Rotates the user transformation matrix at `origin` (floating-point) by `angle`.
  BL_INLINE_NODEBUG BLResult rotate(double angle, const BLPointI& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_ROTATE_PT, angle, origin.x, origin.y); }

  //! Transforms the user transformation matrix by `transform`.
  BL_INLINE_NODEBUG BLResult apply_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_TRANSFORM, &transform); }

  //! Post-translates the used transformation matrix by `[x, y]`.
  //!
  //! \note Post-translation uses a reversed order of matrix multiplication when compared to \ref translate().
  BL_INLINE_NODEBUG BLResult post_translate(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_TRANSLATE, x, y); }

  //! Post-Translates the used transformation matrix by `[p]` (integer).
  //!
  //! \note Post-translation uses a reversed order of matrix multiplication when compared to \ref translate().
  BL_INLINE_NODEBUG BLResult post_translate(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_TRANSLATE, p.x, p.y); }

  //! Post-translates the used transformation matrix by `[p]` (floating-point).
  //!
  //! \note Post-translation uses a reversed order of matrix multiplication when compared to \ref translate().
  BL_INLINE_NODEBUG BLResult post_translate(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_TRANSLATE, &p); }

  //! Post-scales the user transformation matrix by `xy` (both X and Y is scaled by `xy`).
  //!
  //! \note Post-scale uses a reversed order of matrix multiplication when compared to \ref scale().
  BL_INLINE_NODEBUG BLResult post_scale(double xy) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, xy, xy); }

  //! Post-scales the user transformation matrix by `[x, y]`.
  //!
  //! \note Post-scale uses a reversed order of matrix multiplication when compared to \ref scale().
  BL_INLINE_NODEBUG BLResult post_scale(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, x, y); }

  //! Post-scales the user transformation matrix by `[p]` (integer).
  //!
  //! \note Post-scale uses a reversed order of matrix multiplication when compared to \ref scale().
  BL_INLINE_NODEBUG BLResult post_scale(const BLPointI& p) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SCALE, p.x, p.y); }

  //! Post-scales the user transformation matrix by `[p]` (floating-point).
  //!
  //! \note Post-scale uses a reversed order of matrix multiplication when compared to \ref scale().
  BL_INLINE_NODEBUG BLResult post_scale(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_SCALE, &p); }

  //! Skews the user transformation matrix by `[x, y]`.
  //!
  //! \note Post-skew uses a reversed order of matrix multiplication when compared to \ref skew().
  BL_INLINE_NODEBUG BLResult post_skew(double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_SKEW, x, y); }

  //! Skews the user transformation matrix by `[p]` (floating-point).
  //!
  //! \note Post-skew uses a reversed order of matrix multiplication when compared to \ref skew().
  BL_INLINE_NODEBUG BLResult post_skew(const BLPoint& p) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_SKEW, &p); }

  //! Rotates the user transformation matrix by `angle`.
  //!
  //! \note Post-rotation uses a reversed order of matrix multiplication when compared to \ref rotate().
  BL_INLINE_NODEBUG BLResult post_rotate(double angle) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_ROTATE, &angle); }

  //! Rotates the user transformation matrix at `[x, y]` by `angle`.
  //!
  //! \note Post-rotation uses a reversed order of matrix multiplication when compared to \ref rotate().
  BL_INLINE_NODEBUG BLResult post_rotate(double angle, double x, double y) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, x, y); }

  //! Rotates the user transformation matrix at `origin` (integer) by `angle`.
  //!
  //! \note Post-rotation uses a reversed order of matrix multiplication when compared to \ref rotate().
  BL_INLINE_NODEBUG BLResult post_rotate(double angle, const BLPoint& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, origin.x, origin.y); }

  //! Rotates the user transformation matrix at `origin` (floating-point) by `angle`.
  //!
  //! \note Post-rotation uses a reversed order of matrix multiplication when compared to \ref rotate().
  BL_INLINE_NODEBUG BLResult post_rotate(double angle, const BLPointI& origin) noexcept { return _apply_transform_op_v(BL_TRANSFORM_OP_POST_ROTATE_PT, angle, origin.x, origin.y); }

  //! Transforms the user transformation matrix by `transform`.
  //!
  //! \note Post-transform uses a reversed order of matrix multiplication when compared to \ref apply_transform().
  BL_INLINE_NODEBUG BLResult post_transform(const BLMatrix2D& transform) noexcept { return _apply_transform_op(BL_TRANSFORM_OP_POST_TRANSFORM, &transform); }

  //! Stores the result of combining the current `MetaTransform` and `UserTransform` to `MetaTransform` and resets
  //! `UserTransform` to identity as shown below:
  //!
  //! ```
  //! MetaTransform = MetaTransform x UserTransform
  //! UserTransform = Identity
  //! ```
  //!
  //! Please note that this operation is irreversible. The only way to restore a meta-matrix is to \ref save() the
  //! rendering context state, then to use \ref user_to_meta(), and then restored by \ref restore() when needed.
  BL_INLINE_NODEBUG BLResult user_to_meta() noexcept {
    BL_CONTEXT_CALL_RETURN(user_to_meta, impl);
  }

  //! \}

  //! \name Rendering Hints
  //! \{

  //! Returns rendering context hints.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLContextHints& hints() const noexcept {
    return BL_CONTEXT_IMPL()->state->hints;
  }

  //! Sets the given rendering hint `hint_type` to `value`.
  BL_INLINE_NODEBUG BLResult set_hint(BLContextHint hint_type, uint32_t value) noexcept {
    BL_CONTEXT_CALL_RETURN(set_hint, impl, hint_type, value);
  }

  //! Sets all rendering hints of this context to `hints`.
  BL_INLINE_NODEBUG BLResult set_hints(const BLContextHints& hints) noexcept {
    BL_CONTEXT_CALL_RETURN(set_hints, impl, &hints);
  }

  //! Returns the rendering quality hint.
  //!
  //! \note This is the same as calling \ref hints() and extracting the rendering quality from it.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLRenderingQuality rendering_quality() const noexcept {
    return BLRenderingQuality(hints().rendering_quality);
  }

  //! Sets rendering quality hint to `value`.
  BL_INLINE_NODEBUG BLResult set_rendering_quality(BLRenderingQuality value) noexcept {
    return set_hint(BL_CONTEXT_HINT_RENDERING_QUALITY, uint32_t(value));
  }

  //! Returns the gradient quality hint.
  //!
  //! \note This is the same as calling \ref hints() and extracting the gradient quality from it.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLGradientQuality gradient_quality() const noexcept {
    return BLGradientQuality(hints().gradient_quality);
  }

  //! Sets gradient quality hint to `value`.
  BL_INLINE_NODEBUG BLResult set_gradient_quality(BLGradientQuality value) noexcept {
    return set_hint(BL_CONTEXT_HINT_GRADIENT_QUALITY, uint32_t(value));
  }

  //! Returns the pattern quality hint.
  //!
  //! \note This is the same as calling \ref hints() and extracting the pattern quality from it.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLPatternQuality pattern_quality() const noexcept {
    return BLPatternQuality(hints().pattern_quality);
  }

  //! Sets pattern quality hint to `value`.
  BL_INLINE_NODEBUG BLResult set_pattern_quality(BLPatternQuality value) noexcept {
    return set_hint(BL_CONTEXT_HINT_PATTERN_QUALITY, uint32_t(value));
  }

  //! \}

  //! \name Approximation Options
  //! \{

  //! Returns approximation options.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLApproximationOptions& approximation_options() const noexcept {
    return BL_CONTEXT_IMPL()->state->approximation_options;
  }

  //! Sets approximation options to `options`.
  BL_INLINE_NODEBUG BLResult set_approximation_options(const BLApproximationOptions& options) noexcept {
    BL_CONTEXT_CALL_RETURN(set_approximation_options, impl, &options);
  }

  //! Returns flatten mode (how curves are flattened).
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFlattenMode flatten_mode() const noexcept {
    return BLFlattenMode(BL_CONTEXT_IMPL()->state->approximation_options.flatten_mode);
  }

  //! Sets flatten `mode` (how curves are flattened).
  BL_INLINE_NODEBUG BLResult set_flatten_mode(BLFlattenMode mode) noexcept {
    BL_CONTEXT_CALL_RETURN(set_flatten_mode, impl, mode);
  }

  //! Returns tolerance used for curve flattening.
  [[nodiscard]]
  BL_INLINE_NODEBUG double flatten_tolerance() const noexcept {
    return BL_CONTEXT_IMPL()->state->approximation_options.flatten_tolerance;
  }

  //! Sets tolerance used for curve flattening.
  BL_INLINE_NODEBUG BLResult set_flatten_tolerance(double tolerance) noexcept {
    BL_CONTEXT_CALL_RETURN(set_flatten_tolerance, impl, tolerance);
  }

  //! \}

  //! \name Composition Options
  //! \{

  //! Returns a composition operator.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLCompOp comp_op() const noexcept { return BLCompOp(BL_CONTEXT_IMPL()->state->comp_op); }

  //! Sets the composition operator to `comp_op`, see \ref BLCompOp.
  //!
  //! The composition operator is part of the rendering context state and is subject to \ref save() and
  //! \ref restore(). The default composition operator is \ref BL_COMP_OP_SRC_OVER, which would be returned
  //! immediately after the rendering context is created.
  BL_INLINE_NODEBUG BLResult set_comp_op(BLCompOp comp_op) noexcept { BL_CONTEXT_CALL_RETURN(set_comp_op, impl, comp_op); }

  //! Returns a global alpha value.
  [[nodiscard]]
  BL_INLINE_NODEBUG double global_alpha() const noexcept { return BL_CONTEXT_IMPL()->state->global_alpha; }

  //! Sets the global alpha value.
  //!
  //! The global alpha value is part of the rendering context state and is subject to \ref save() and
  //! \ref restore(). The default value is `1.0`, which would be returned immediately after the rendering
  //! context is created.
  BL_INLINE_NODEBUG BLResult set_global_alpha(double alpha) noexcept { BL_CONTEXT_CALL_RETURN(set_global_alpha, impl, alpha); }

  //! \}

  //! \cond INTERNAL
  //! \name Style Options (Internal)
  //! \{

private:

  BL_INLINE_NODEBUG BLResult _set_style_internal(BLContextStyleSlot slot, const BLRgba& rgba) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style_rgba, impl, slot, &rgba);
  }

  BL_INLINE_NODEBUG BLResult _set_style_internal(BLContextStyleSlot slot, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style_rgba32, impl, slot, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _set_style_internal(BLContextStyleSlot slot, const BLRgba64& rgba64) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style_rgba64, impl, slot, rgba64.value);
  }

  BL_INLINE_NODEBUG BLResult _set_style_internal(BLContextStyleSlot slot, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style, impl, slot, &style, BL_CONTEXT_STYLE_TRANSFORM_MODE_USER);
  }

  BL_INLINE_NODEBUG BLResult _set_style_internal(BLContextStyleSlot slot, const BLPatternCore& pattern) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style, impl, slot, &pattern, BL_CONTEXT_STYLE_TRANSFORM_MODE_USER);
  }

  BL_INLINE_NODEBUG BLResult _set_style_internal(BLContextStyleSlot slot, const BLGradientCore& gradient) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style, impl, slot, &gradient, BL_CONTEXT_STYLE_TRANSFORM_MODE_USER);
  }

  BL_INLINE_NODEBUG BLResult _set_style_with_mode(BLContextStyleSlot slot, const BLVarCore& style, BLContextStyleTransformMode transform_mode) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style, impl, slot, &style, transform_mode);
  }

  BL_INLINE_NODEBUG BLResult _set_style_with_mode(BLContextStyleSlot slot, const BLPatternCore& pattern, BLContextStyleTransformMode transform_mode) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style, impl, slot, &pattern, transform_mode);
  }

  BL_INLINE_NODEBUG BLResult _set_style_with_mode(BLContextStyleSlot slot, const BLGradientCore& gradient, BLContextStyleTransformMode transform_mode) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style, impl, slot, &gradient, transform_mode);
  }

public:

  //! \}
  //! \endcond

  //! \name Style Options
  //! \{

  //! Returns the current style type associated with the given style `slot`.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLObjectType style_type(BLContextStyleSlot slot) const noexcept {
    return uint32_t(slot) <= BL_CONTEXT_STYLE_SLOT_MAX_VALUE ? BLObjectType(BL_CONTEXT_IMPL()->state->style_type[slot]) : BL_OBJECT_TYPE_NULL;
  }

  //! Reads a style state associated with the given style `slot` and writes it into `style_out`.
  //!
  //! \note This function returns the original style passed to the rendering context with its original transformation
  //! matrix if it's not a solid color. Consider using \ref get_transformed_style() if you want to get a style with the
  //! transformation matrix that the rendering context actually uses to render it.
  //!
  //! \warning This function requires an initialized variant. Use a C++ provided `BLVar` class instead of C-API
  //! `BLVarCore` to retrieve a style. `BLVar` is always initialized and properly destroyed.
  BL_INLINE_NODEBUG BLResult get_style(BLContextStyleSlot slot, BLVarCore& style_out) const noexcept {
    BL_CONTEXT_CALL_RETURN(get_style, impl, slot, false, &style_out);
  }

  //! Reads a style state associated with the given style `slot` and writes it into `style_out`.
  //!
  //! The retrieved style uses a transformation matrix that is a combination of style transformation matrix and
  //! the rendering context matrix at a time \ref set_style() was called.
  //!
  //! \warning This function requires an initialized variant. Use a C++ provided `BLVar` class instead of C-API
  //! `BLVarCore` to retrieve a style. `BLVar` is always initialized and properly destroyed.
  BL_INLINE_NODEBUG BLResult get_transformed_style(BLContextStyleSlot slot, BLVarCore& style_out) const noexcept {
    BL_CONTEXT_CALL_RETURN(get_style, impl, slot, true, &style_out);
  }

  //! Sets `style` to be used with the given style `slot` operation.
  //!
  //! \note The `style` argument could be \ref BLRgba, \ref BLRgba32, \ref BLRgba64, \ref BLGradient, \ref BLPattern, and \ref BLVar.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult set_style(BLContextStyleSlot slot, const StyleT& style) noexcept {
    return _set_style_internal(slot, style);
  }

  //! Sets `style` to be used with the given style `slot` operation and applied `transform_mode`.
  //!
  //! This is a convenience function that allows to control how the given `style` is transformed. By default, if `transform_mode` is not
  //! provided, the rendering context combines the style transformation matrix with user transformation matrix, which is compatible with
  //! how it transforms geometry. However, if that' undesired, a `transform_mode` can override the default operation.
  //!
  //! \note The `style` argument could be \ref BLGradient, \ref BLPattern, and \ref BLVar.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult set_style(BLContextStyleSlot slot, const StyleT& style, BLContextStyleTransformMode transform_mode) noexcept {
    return _set_style_with_mode(slot, style, transform_mode);
  }

  //! Sets the given style `slot` to NULL, which disables it.
  //!
  //! Styles set to NULL would reject all rendering operations that would otherwise use that style.
  BL_INLINE_NODEBUG BLResult disable_style(BLContextStyleSlot slot) noexcept {
    BL_CONTEXT_CALL_RETURN(disable_style, impl, slot);
  }

  //! Returns fill or alpha value associated with the given style `slot`.
  //!
  //! The function behaves like `fill_alpha()` or `stroke_alpha()` depending on style `slot`, see \ref BLContextStyleSlot.
  [[nodiscard]]
  BL_INLINE_NODEBUG double style_alpha(BLContextStyleSlot slot) const noexcept {
    return slot <= BL_CONTEXT_STYLE_SLOT_MAX_VALUE ? BL_CONTEXT_IMPL()->state->style_alpha[slot] : 0.0;
  }

  //! Set fill or stroke `alpha` value associated with the given style `slot`.
  //!
  //! The function behaves like `set_fill_alpha()` or `set_stroke_alpha()` depending on style `slot`, see \ref BLContextStyleSlot.
  BL_INLINE_NODEBUG BLResult set_style_alpha(BLContextStyleSlot slot, double alpha) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style_alpha, impl, slot, alpha);
  }

  //! Swaps fill and stroke styles, see \ref BLContextStyleSwapMode for options.
  BL_INLINE_NODEBUG BLResult swap_styles(BLContextStyleSwapMode mode) noexcept {
    BL_CONTEXT_CALL_RETURN(swap_styles, impl, mode);
  }

  //! \}

  //! \name Fill Style & Options
  //! \{


  //! Returns the current fill style type.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLObjectType fill_style_type() const noexcept {
    return BLObjectType(BL_CONTEXT_IMPL()->state->style_type[BL_CONTEXT_STYLE_SLOT_FILL]);
  }

  //! Reads a fill style state and writes it into `style_out` variant.
  //!
  //! \note This function returns the original style passed to the rendering context with its original transformation
  //! matrix if it's not a solid color. Consider using \ref get_transformed_fill_style() if you want to get a fill style
  //! with the transformation matrix that the rendering context actually uses to render it.
  //!
  //! \warning This function requires an initialized variant. Use a C++ provided `BLVar` class instead of C-API
  //! `BLVarCore` to retrieve a style. `BLVar` is always initialized and properly destroyed.
  BL_INLINE_NODEBUG BLResult get_fill_style(BLVarCore& out) const noexcept {
    BL_CONTEXT_CALL_RETURN(get_style, impl, BL_CONTEXT_STYLE_SLOT_FILL, false, &out);
  }

  //! Reads a fill style state and writes it into `styleOut` variant.
  //!
  //! \note Unlike `getFillStyle()` this function returns a style, which was transformed by the rendering context
  //! at the time it was set (using a past transformation matrix). When this style is set to the rendering context
  //! again with all transformations bypassed (by using \ref BL_CONTEXT_STYLE_TRANSFORM_MODE_NONE option) it would
  //! make the style identical.
  //!
  //! \warning This function requires an initialized variant. Use a C++ provided `BLVar` class instead of C-API
  //! `BLVarCore` to retrieve a style. `BLVar` is always initialized and properly destroyed.
  BL_INLINE_NODEBUG BLResult get_transformed_fill_style(BLVarCore& out) const noexcept {
    BL_CONTEXT_CALL_RETURN(get_style, impl, BL_CONTEXT_STYLE_SLOT_FILL, true, &out);
  }

  //! Sets fill style.
  //!
  //! \note The `style` argument could be \ref BLRgba, \ref BLRgba32, \ref BLRgba64, \ref BLGradient, \ref BLPattern, and \ref BLVar.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult set_fill_style(const StyleT& style) noexcept {
    return _set_style_internal(BL_CONTEXT_STYLE_SLOT_FILL, style);
  }

  //! Sets fill style to `style`.
  //!
  //! \note The `style` argument could be \ref BLGradient, \ref BLPattern, and \ref BLVar.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult set_fill_style(const StyleT& style, BLContextStyleTransformMode transform_mode) noexcept {
    return _set_style_with_mode(BL_CONTEXT_STYLE_SLOT_FILL, style, transform_mode);
  }

  //! Sets fill style to NULL, which disables it.
  BL_INLINE_NODEBUG BLResult disable_fill_style() noexcept {
    BL_CONTEXT_CALL_RETURN(disable_style, impl, BL_CONTEXT_STYLE_SLOT_FILL);
  }

  //! Returns fill alpha value.
  [[nodiscard]]
  BL_INLINE_NODEBUG double fill_alpha() const noexcept {
    return BL_CONTEXT_IMPL()->state->style_alpha[BL_CONTEXT_STYLE_SLOT_FILL];
  }

  //! Sets fill `alpha` value.
  BL_INLINE_NODEBUG BLResult set_fill_alpha(double alpha) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style_alpha, impl, BL_CONTEXT_STYLE_SLOT_FILL, alpha);
  }

  //! Returns fill-rule, see \ref BLFillRule.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLFillRule fill_rule() const noexcept {
    return BLFillRule(BL_CONTEXT_IMPL()->state->fill_rule);
  }

  //! Sets fill-rule, see \ref BLFillRule.
  BL_INLINE_NODEBUG BLResult set_fill_rule(BLFillRule fill_rule) noexcept {
    BL_CONTEXT_CALL_RETURN(set_fill_rule, impl, fill_rule);
  }

  //! \}

  //! \name Stroke Style & Options
  //! \{

  //! Returns the current stroke style type.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLObjectType stroke_style_type() const noexcept {
    return BLObjectType(BL_CONTEXT_IMPL()->state->style_type[BL_CONTEXT_STYLE_SLOT_STROKE]);
  }

  //! Reads a stroke style state and writes it into `style_out` variant.
  //!
  //! \note This function returns the original style passed to the rendering context with its original transformation
  //! matrix if it's not a solid color. Consider using \ref get_transformed_stroke_style() if you want to get a stroke
  //! style with the transformation matrix that the rendering context actually uses to render it.
  //!
  //! \warning This function requires an initialized variant. Use a C++ provided `BLVar` class instead of C-API
  //! `BLVarCore` to retrieve a style. `BLVar` is always initialized and properly destroyed.
  BL_INLINE_NODEBUG BLResult get_stroke_style(BLVarCore& out) const noexcept {
    BL_CONTEXT_CALL_RETURN(get_style, impl, BL_CONTEXT_STYLE_SLOT_STROKE, false, &out);
  }

  //! Reads a stroke style state and writes it into `styleOut` variant.
  //!
  //! \note Unlike `getStrokeStyle()` this function returns a style, which was transformed by the rendering context
  //! at the time it was set (using a past transformation matrix). When this style is set to the rendering context
  //! again with all transformations bypassed (by using \ref BL_CONTEXT_STYLE_TRANSFORM_MODE_NONE option) it would
  //! make the style identical.
  //!
  //! \warning This function requires an initialized variant. Use a C++ provided `BLVar` class instead of C-API
  //! `BLVarCore` to retrieve a style. `BLVar` is always initialized and properly destroyed.
  BL_INLINE_NODEBUG BLResult get_transformed_stroke_style(BLVarCore& out) const noexcept {
    BL_CONTEXT_CALL_RETURN(get_style, impl, BL_CONTEXT_STYLE_SLOT_STROKE, true, &out);
  }

  //! Sets stroke style.
  //!
  //! \note The `style` argument could be \ref BLRgba, \ref BLRgba32, \ref BLRgba64, \ref BLGradient, \ref BLPattern, and \ref BLVar.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult set_stroke_style(const StyleT& style) noexcept {
    return _set_style_internal(BL_CONTEXT_STYLE_SLOT_STROKE, style);
  }

  //! Sets fill style to `style`.
  //!
  //! \note The `style` argument could be \ref BLGradient, \ref BLPattern, and \ref BLVar.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult set_stroke_style(const StyleT& style, BLContextStyleTransformMode transform_mode) noexcept {
    return _set_style_with_mode(BL_CONTEXT_STYLE_SLOT_STROKE, style, transform_mode);
  }

  //! Sets stroke style to NULL, which disables it.
  BL_INLINE_NODEBUG BLResult disable_stroke_style() noexcept {
    BL_CONTEXT_CALL_RETURN(disable_style, impl, BL_CONTEXT_STYLE_SLOT_STROKE);
  }

  //! Returns stroke width.
  [[nodiscard]]
  BL_INLINE_NODEBUG double stroke_width() const noexcept {
    return BL_CONTEXT_IMPL()->state->stroke_options.width;
  }

  //! Returns stroke miter-limit.
  [[nodiscard]]
  BL_INLINE_NODEBUG double stroke_miter_limit() const noexcept {
    return BL_CONTEXT_IMPL()->state->stroke_options.miter_limit;
  }

  //! Returns stroke join, see \ref BLStrokeJoin.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLStrokeJoin stroke_join() const noexcept {
    return BLStrokeJoin(BL_CONTEXT_IMPL()->state->stroke_options.join);
  }

  //! Returns stroke start-cap, see \ref BLStrokeCap.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLStrokeCap stroke_start_cap() const noexcept {
    return BLStrokeCap(BL_CONTEXT_IMPL()->state->stroke_options.start_cap);
  }

  //! Returns stroke end-cap, see \ref BLStrokeCap.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLStrokeCap stroke_end_cap() const noexcept {
    return BLStrokeCap(BL_CONTEXT_IMPL()->state->stroke_options.end_cap);
  }

  //! Returns stroke transform order, see \ref BLStrokeTransformOrder.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLStrokeTransformOrder stroke_transform_order() const noexcept {
    return BLStrokeTransformOrder(BL_CONTEXT_IMPL()->state->stroke_options.transform_order);
  }

  //! Returns stroke dash-offset.
  [[nodiscard]]
  BL_INLINE_NODEBUG double stroke_dash_offset() const noexcept {
    return BL_CONTEXT_IMPL()->state->stroke_options.dash_offset;
  }

  //! Returns stroke dash-array.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLArray<double>& stroke_dash_array() const noexcept {
    return BL_CONTEXT_IMPL()->state->stroke_options.dash_array;
  }

  //! Returns stroke options as a reference to \ref BLStrokeOptions.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLStrokeOptions& stroke_options() const noexcept {
    return BL_CONTEXT_IMPL()->state->stroke_options.dcast();
  }

  //! Sets stroke width to `width`.
  BL_INLINE_NODEBUG BLResult set_stroke_width(double width) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_width, impl, width);
  }

  //! Sets miter limit to `miter_limit`.
  BL_INLINE_NODEBUG BLResult set_stroke_miter_limit(double miter_limit) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_miter_limit, impl, miter_limit);
  }

  //! Sets stroke join to `stroke_join`, see \ref BLStrokeJoin.
  BL_INLINE_NODEBUG BLResult set_stroke_join(BLStrokeJoin stroke_join) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_join, impl, stroke_join);
  }

  //! Sets stroke cap of the specified `type` to `stroke_cap`, see \ref BLStrokeCap.
  BL_INLINE_NODEBUG BLResult set_stroke_cap(BLStrokeCapPosition position, BLStrokeCap stroke_cap) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_cap, impl, position, stroke_cap);
  }

  //! Sets stroke start cap to `stroke_cap`, see \ref BLStrokeCap.
  BL_INLINE_NODEBUG BLResult set_stroke_start_cap(BLStrokeCap stroke_cap) noexcept {
    return set_stroke_cap(BL_STROKE_CAP_POSITION_START, stroke_cap);
  }

  //! Sets stroke end cap to `stroke_cap`, see \ref BLStrokeCap.
  BL_INLINE_NODEBUG BLResult set_stroke_end_cap(BLStrokeCap stroke_cap) noexcept {
    return set_stroke_cap(BL_STROKE_CAP_POSITION_END, stroke_cap);
  }

  //! Sets all stroke caps to `stroke_cap`, see \ref BLStrokeCap.
  BL_INLINE_NODEBUG BLResult set_stroke_caps(BLStrokeCap stroke_cap) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_caps, impl, stroke_cap);
  }

  //! Sets stroke transformation order to `transform_order`, see \ref BLStrokeTransformOrder.
  BL_INLINE_NODEBUG BLResult set_stroke_transform_order(BLStrokeTransformOrder transform_order) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_transform_order, impl, transform_order);
  }

  //! Sets stroke dash-offset to `dash_offset`.
  BL_INLINE_NODEBUG BLResult set_stroke_dash_offset(double dash_offset) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_dash_offset, impl, dash_offset);
  }

  //! Sets stroke dash-array to `dash_array`.
  BL_INLINE_NODEBUG BLResult set_stroke_dash_array(const BLArray<double>& dash_array) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_dash_array, impl, &dash_array);
  }

  //! Sets all stroke `options`.
  BL_INLINE_NODEBUG BLResult set_stroke_options(const BLStrokeOptions& options) noexcept {
    BL_CONTEXT_CALL_RETURN(set_stroke_options, impl, &options);
  }

  //! Returns stroke alpha value.
  [[nodiscard]]
  BL_INLINE_NODEBUG double stroke_alpha() const noexcept {
    return BL_CONTEXT_IMPL()->state->style_alpha[BL_CONTEXT_STYLE_SLOT_STROKE];
  }

  //! Sets stroke alpha value to `alpha`.
  BL_INLINE_NODEBUG BLResult set_stroke_alpha(double alpha) noexcept {
    BL_CONTEXT_CALL_RETURN(set_style_alpha, impl, BL_CONTEXT_STYLE_SLOT_STROKE, alpha);
  }

  //! \}

  //! \name Clip Operations
  //! \{

  //! Restores clipping to the last saved state or to the context default clipping if there is no saved state.
  //!
  //! If there are no saved states then it resets clipping completely to the initial state that was used when
  //! the rendering context was created.
  BL_INLINE_NODEBUG BLResult restore_clipping() noexcept {
    BL_CONTEXT_CALL_RETURN(restore_clipping, impl);
  }

  BL_INLINE_NODEBUG BLResult clip_to_rect(const BLRectI& rect) noexcept {
    BL_CONTEXT_CALL_RETURN(clip_to_rect_i, impl, &rect);
  }

  BL_INLINE_NODEBUG BLResult clip_to_rect(const BLRect& rect) noexcept {
    BL_CONTEXT_CALL_RETURN(clip_to_rect_d, impl, &rect);
  }

  BL_INLINE_NODEBUG BLResult clip_to_rect(double x, double y, double w, double h) noexcept {
    return clip_to_rect(BLRect(x, y, w, h));
  }

  //! \}

  //! \name Clear Geometry Operations
  //! \{

  //! Clear everything to a transparent black, which is the same operation as temporarily setting the composition
  //! operator to \ref BL_COMP_OP_CLEAR and then filling everything by `fill_all()`.
  //!
  //! \note If the target surface doesn't have alpha, but has X component, like \ref BL_FORMAT_XRGB32, the `X`
  //! component would be set to `1.0`, which would translate to `0xFF` in case of \ref BL_FORMAT_XRGB32.
  BL_INLINE_NODEBUG BLResult clear_all() noexcept {
    BL_CONTEXT_CALL_RETURN(clear_all, impl);
  }

  //! Clears a rectangle `rect` (integer coordinates) to a transparent black, which is the same operation as
  //! temporarily setting the composition operator to \ref BL_COMP_OP_CLEAR and then calling `fill_rect(rect)`.
  //!
  //! \note If the target surface doesn't have alpha, but has X component, like \ref BL_FORMAT_XRGB32, the `X`
  //! component would be set to `1.0`, which would translate to `0xFF` in case of \ref BL_FORMAT_XRGB32.
  BL_INLINE_NODEBUG BLResult clear_rect(const BLRectI& rect) noexcept {
    BL_CONTEXT_CALL_RETURN(clear_recti, impl, &rect);
  }

  //! Clears a rectangle `rect` (floating-point coordinates) to a transparent black, which is the same operation
  //! as temporarily setting the composition operator to \ref BL_COMP_OP_CLEAR and then calling `fill_rect(rect)`.
  //!
  //! \note If the target surface doesn't have alpha, but has X component, like \ref BL_FORMAT_XRGB32, the `X`
  //! component would be set to `1.0`, which would translate to `0xFF` in case of \ref BL_FORMAT_XRGB32.
  BL_INLINE_NODEBUG BLResult clear_rect(const BLRect& rect) noexcept {
    BL_CONTEXT_CALL_RETURN(clear_rectd, impl, &rect);
  }

  //! Clears a rectangle `[x, y, w, h]` (floating-point coordinates) to a transparent black, which is the same
  //! operation as temporarily setting the composition operator to \ref BL_COMP_OP_CLEAR and then calling
  //! `fill_rect(x, y, w, h)`.
  //!
  //! \note If the target surface doesn't have alpha, but has X component, like \ref BL_FORMAT_XRGB32, the `X`
  //! component would be set to `1.0`, which would translate to `0xFF` in case of \ref BL_FORMAT_XRGB32.
  BL_INLINE_NODEBUG BLResult clear_rect(double x, double y, double w, double h) noexcept {
    return clear_rect(BLRect(x, y, w, h));
  }

  //! \}

  //! \cond INTERNAL
  //! \name Fill Wrappers (Internal)
  //! \{

private:
  BL_INLINE_NODEBUG BLResult _fill_all(const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_all_ext, impl, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_all(const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_all_rgba32, impl, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_all(const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_all_ext, impl, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_all(const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_all_ext, impl, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_all(const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_all_ext, impl, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_all(const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_all_ext, impl, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_i(const BLRectI& rect, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_rect_i_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_i(const BLRectI& rect, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_i_rgba32, impl, &rect, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_i(const BLRectI& rect, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_rect_i_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_i(const BLRectI& rect, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_i_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_i(const BLRectI& rect, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_i_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_i(const BLRectI& rect, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_i_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_d(const BLRect& rect, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_rect_d_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_d(const BLRect& rect, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_d_rgba32, impl, &rect, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_d(const BLRect& rect, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_rect_d_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_d(const BLRect& rect, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_d_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_d(const BLRect& rect, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_d_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_rect_d(const BLRect& rect, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_d_ext, impl, &rect, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_geometry_op(BLGeometryType type, const void* data) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_geometry, impl, type, data);
  }

  BL_INLINE_NODEBUG BLResult _fill_geometry_op(BLGeometryType type, const void* data, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_geometry_op(BLGeometryType type, const void* data, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_geometry_rgba32, impl, type, data, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_geometry_op(BLGeometryType type, const void* data, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_geometry_op(BLGeometryType type, const void* data, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_geometry_op(BLGeometryType type, const void* data, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_geometry_op(BLGeometryType type, const void* data, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_path_d(const BLPoint& origin, const BLPathCore& path, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_path_d(const BLPoint& origin, const BLPathCore& path, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_path_d_rgba32, impl, &origin, &path, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_path_d(const BLPoint& origin, const BLPathCore& path, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_path_d(const BLPoint& origin, const BLPathCore& path, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_path_d(const BLPoint& origin, const BLPathCore& path, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_path_d(const BLPoint& origin, const BLPathCore& path, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_i, impl, &origin, &font, op, data);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_i_rgba32, impl, &origin, &font, op, data, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_d, impl, &origin, &font, op, data);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_d_rgba32, impl, &origin, &font, op, data, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_i(const BLPointI& origin, const BLImage& mask, const BLRectI* mask_area) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_i, impl, &origin, &mask, mask_area);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_i(const BLPointI& origin, const BLImage& mask, const BLRectI* mask_area, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_mask_i_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_i(const BLPointI& origin, const BLImage& mask, const BLRectI* mask_area, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_i_rgba32, impl, &origin, &mask, mask_area, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_i(const BLPointI& origin, const BLImage& mask, const BLRectI* mask_area, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_mask_i_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_i(const BLPointI& origin, const BLImage& mask, const BLRectI* mask_area, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_i_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_i(const BLPointI& origin, const BLImage& mask, const BLRectI* mask_area, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_i_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_i(const BLPointI& origin, const BLImage& mask, const BLRectI* mask_area, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_i_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_d(const BLPoint& origin, const BLImage& mask, const BLRectI* mask_area) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_d, impl, &origin, &mask, mask_area);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_d(const BLPoint& origin, const BLImage& mask, const BLRectI* mask_area, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(fill_mask_d_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_d(const BLPoint& origin, const BLImage& mask, const BLRectI* mask_area, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_d_Rgba32, impl, &origin, &mask, mask_area, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_d(const BLPoint& origin, const BLImage& mask, const BLRectI* mask_area, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(fill_mask_d_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_d(const BLPoint& origin, const BLImage& mask, const BLRectI* mask_area, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_d_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_d(const BLPoint& origin, const BLImage& mask, const BLRectI* mask_area, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_d_ext, impl, &origin, &mask, mask_area, &style);
  }

  BL_INLINE_NODEBUG BLResult _fill_mask_d(const BLPoint& origin, const BLImage& mask, const BLRectI* mask_area, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_mask_d_ext, impl, &origin, &mask, mask_area, &style);
  }

public:

  //! \}
  //! \endcond

  //! \name Fill Geometry Operations
  //! \{

  //! Fills everything non-clipped with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_all() noexcept {
    BL_CONTEXT_CALL_RETURN(fill_all, impl);
  }

  //! Fills everything non-clipped with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_all(const StyleT& style) noexcept {
    return _fill_all(style);
  }

  //! Fills a `box` (floating point coordinates) with the current fill style.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref fill_rect() instead.
  BL_INLINE_NODEBUG BLResult fill_box(const BLBox& box) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_BOXD, &box);
  }

  //! Fills a `box` (floating point coordinates) with an explicit fill `style`.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref fill_rect() instead.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_box(const BLBox& box, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_BOXD, &box, style);
  }

  //! Fills a `box` (integer coordinates) with the current fill style.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref fill_rect() instead.
  BL_INLINE_NODEBUG BLResult fill_box(const BLBoxI& box) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_BOXI, &box);
  }

  //! Fills a `box` (integer coordinates) with an explicit fill `style`.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref fill_rect() instead.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_box(const BLBoxI& box, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_BOXI, &box, style);
  }

  //! Fills a box `[x0, y0, x1, y1]` (floating point coordinates) with the current fill style.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref fill_rect() instead.
  BL_INLINE_NODEBUG BLResult fill_box(double x0, double y0, double x1, double y1) noexcept {
    return fill_box(BLBox(x0, y0, x1, y1));
  }

  //! Fills a box `[x0, y0, x1, y1]` (floating point coordinates) with an explicit fill `style`.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref fill_rect() instead.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_box(double x0, double y0, double x1, double y1, const StyleT& style) noexcept {
    return fill_box(BLBox(x0, y0, x1, y1), style);
  }

  //! Fills a rectangle `rect` (integer coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_rect(const BLRectI& rect) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_i, impl, &rect);
  }

  //! Fills a rectangle `rect` (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_rect(const BLRectI& rect, const StyleT& style) noexcept {
    return _fill_rect_i(rect, style);
  }

  //! Fills a rectangle `rect` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_rect(const BLRect& rect) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_rect_d, impl, &rect);
  }

  //! Fills a rectangle `rect` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_rect(const BLRect& rect, const StyleT& style) noexcept {
    return _fill_rect_d(rect, style);
  }

  //! Fills a rectangle `[x, y, w, h]` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_rect(double x, double y, double w, double h) noexcept {
    return fill_rect(BLRect(x, y, w, h));
  }

  //! Fills a rectangle `[x, y, w, h]` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_rect(double x, double y, double w, double h, const StyleT& style) noexcept {
    return _fill_rect_d(BLRect(x, y, w, h), style);
  }

  //! Fills a `circle` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_circle(const BLCircle& circle) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_CIRCLE, &circle);
  }

  //! Fills a `circle` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_circle(const BLCircle& circle, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_CIRCLE, &circle, style);
  }

  //! Fills a circle at `[cx, cy]` and radius `r` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_circle(double cx, double cy, double r) noexcept {
    return fill_circle(BLCircle(cx, cy, r));
  }

  //! Fills a circle at `[cx, cy]` and radius `r` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_circle(double cx, double cy, double r, const StyleT& style) noexcept {
    return fill_circle(BLCircle(cx, cy, r), style);
  }

  //! Fills an `ellipse` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_ellipse(const BLEllipse& ellipse) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse);
  }

  //! Fills an `ellipse` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_ellipse(const BLEllipse& ellipse, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse, style);
  }

  //! Fills an ellipse at `[cx, cy]` with radius `[rx, ry]` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_ellipse(double cx, double cy, double rx, double ry) noexcept {
    return fill_ellipse(BLEllipse(cx, cy, rx, ry));
  }

  //! Fills an ellipse at `[cx, cy]` with radius `[rx, ry]` (floating point coordinates) with en explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_ellipse(double cx, double cy, double rx, double ry, const StyleT& style) noexcept {
    return fill_ellipse(BLEllipse(cx, cy, rx, ry), style);
  }

  //! Fills a rounded rectangle `rr` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_round_rect(const BLRoundRect& rr) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ROUND_RECT, &rr);
  }

  //! Fills a rounded rectangle `rr` (floating point coordinates) with en explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_round_rect(const BLRoundRect& rr, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ROUND_RECT, &rr, style);
  }

  //! Fills a rounded rectangle bounded by `rect` with radius `r` with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_round_rect(const BLRect& rect, double r) noexcept {
    return fill_round_rect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, r));
  }

  //! Fills a rounded rectangle bounded by `rect` with radius `[rx, ry]` with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_round_rect(const BLRect& rect, double rx, double ry) noexcept {
    return fill_round_rect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, rx, ry));
  }

  //! Fills a rounded rectangle bounded by `rect` with radius `[rx, ry]` with en explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_round_rect(const BLRect& rect, double rx, double ry, const StyleT& style) noexcept {
    return fill_round_rect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, rx, ry), style);
  }

  //! Fills a rounded rectangle bounded by `[x, y, w, h]` with radius `r` with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_round_rect(double x, double y, double w, double h, double r) noexcept {
    return fill_round_rect(BLRoundRect(x, y, w, h, r));
  }

  //! Fills a rounded rectangle bounded as `[x, y, w, h]` with radius `[rx, ry]` with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_round_rect(double x, double y, double w, double h, double rx, double ry) noexcept {
    return fill_round_rect(BLRoundRect(x, y, w, h, rx, ry));
  }

  //! Fills a rounded rectangle bounded as `[x, y, w, h]` with radius `[rx, ry]` with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_round_rect(double x, double y, double w, double h, double rx, double ry, const StyleT& style) noexcept {
    return fill_round_rect(BLRoundRect(x, y, w, h, rx, ry), style);
  }

  //! Fills a `chord` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_chord(const BLArc& chord) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_CHORD, &chord);
  }

  //! Fills a `chord` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_chord(const BLArc& chord, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_CHORD, &chord, style);
  }

  //! Fills a chord at `[cx, cy]` with radius `r` at `start` of `sweep` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_chord(double cx, double cy, double r, double start, double sweep) noexcept {
    return fill_chord(BLArc(cx, cy, r, r, start, sweep));
  }

  //! Fills a chord at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_chord(double cx, double cy, double rx, double ry, double start, double sweep) noexcept {
    return fill_chord(BLArc(cx, cy, rx, ry, start, sweep));
  }

  //! Fills a chord at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_chord(double cx, double cy, double rx, double ry, double start, double sweep, const StyleT& style) noexcept {
    return fill_chord(BLArc(cx, cy, rx, ry, start, sweep), style);
  }

  //! Fills a `pie` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_pie(const BLArc& pie) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_PIE, &pie);
  }

  //! Fills a `pie` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_pie(const BLArc& pie, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_PIE, &pie, style);
  }

  //! Fills a pie at `[cx, cy]` with radius `r` at `start` of `sweep` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_pie(double cx, double cy, double r, double start, double sweep) noexcept {
    return fill_pie(BLArc(cx, cy, r, r, start, sweep));
  }

  //! Fills a pie at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_pie(double cx, double cy, double rx, double ry, double start, double sweep) noexcept {
    return fill_pie(BLArc(cx, cy, rx, ry, start, sweep));
  }

  //! Fills a pie at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_pie(double cx, double cy, double rx, double ry, double start, double sweep, const StyleT& style) noexcept {
    return fill_pie(BLArc(cx, cy, rx, ry, start, sweep), style);
  }

  //! Fills a `triangle` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_triangle(const BLTriangle& triangle) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_TRIANGLE, &triangle);
  }

  //! Fills a `triangle` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_triangle(const BLTriangle& triangle, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_TRIANGLE, &triangle, style);
  }

  //! Fills a triangle defined by `[x0, y0]`, `[x1, y1]`, `[x2, y2]` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_triangle(double x0, double y0, double x1, double y1, double x2, double y2) noexcept {
    return fill_triangle(BLTriangle(x0, y0, x1, y1, x2, y2));
  }

  //! Fills a triangle defined by `[x0, y0]`, `[x1, y1]`, `[x2, y2]` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_triangle(double x0, double y0, double x1, double y1, double x2, double y2, const StyleT& style) noexcept {
    return fill_triangle(BLTriangle(x0, y0, x1, y1, x2, y2), style);
  }

  //! Fills a polygon `poly` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLArrayView<BLPoint>& poly) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_POLYGOND, &poly);
  }

  //! Fills a polygon `poly` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLArrayView<BLPoint>& poly, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_POLYGOND, &poly, style);
  }

  //! Fills a polygon `poly` having `n` vertices (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLPoint* poly, size_t n) noexcept {
    return fill_polygon(BLArrayView<BLPoint>{poly, n});
  }

  //! Fills a polygon `poly` having `n` vertices (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLPoint* poly, size_t n, const StyleT& style) noexcept {
    return fill_polygon(BLArrayView<BLPoint>{poly, n}, style);
  }

  //! Fills a polygon `poly` (integer coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLArrayView<BLPointI>& poly) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_POLYGONI, &poly);
  }

  //! Fills a polygon `poly` (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLArrayView<BLPointI>& poly, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_POLYGONI, &poly, style);
  }

  //! Fills a polygon `poly` having `n` vertices (integer coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLPointI* poly, size_t n) noexcept {
    return fill_polygon(BLArrayView<BLPointI>{poly, n});
  }

  //! Fills a polygon `poly` having `n` vertices (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_polygon(const BLPointI* poly, size_t n, const StyleT& style) noexcept {
    return fill_polygon(BLArrayView<BLPointI>{poly, n}, style);
  }

  //! Fills an `array` of boxes (floating point coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLArrayView<BLBox>& array) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array);
  }

  //! Fills an `array` of boxes (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLArrayView<BLBox>& array, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array, style);
  }

  //! Fills an `array` of boxes of size `n` (floating point coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLBox* array, size_t n) noexcept {
    return fill_box_array(BLArrayView<BLBox>{array, n});
  }

  //! Fills an `array` of boxes of size `n` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLBox* array, size_t n, const StyleT& style) noexcept {
    return fill_box_array(BLArrayView<BLBox>{array, n}, style);
  }

  //! Fills an `array` of boxes (integer coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLArrayView<BLBoxI>& array) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array);
  }

  //! Fills an `array` of boxes (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLArrayView<BLBoxI>& array, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array, style);
  }

  //! Fills an `array` of boxes of size `n` (integer coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLBoxI* array, size_t n) noexcept {
    return fill_box_array(BLArrayView<BLBoxI>{array, n});
  }

  //! Fills an array of boxes of size `n` (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_box_array(const BLBoxI* array, size_t n, const StyleT& style) noexcept {
    return fill_box_array(BLArrayView<BLBoxI>{array, n}, style);
  }

  //! Fills an `array` of rectangles (floating point coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLArrayView<BLRect>& array) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array);
  }

  //! Fills an `array` of rectangles (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLArrayView<BLRect>& array, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array, style);
  }

  //! Fills an `array` of rectangles of size `n` (floating point coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLRect* array, size_t n) noexcept {
    return fill_rect_array(BLArrayView<BLRect>{array, n});
  }

  //! Fills an `array` of rectangles of size `n` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLRect* array, size_t n, const StyleT& style) noexcept {
    return fill_rect_array(BLArrayView<BLRect>{array, n}, style);
  }

  //! Fills an array of rectangles (integer coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLArrayView<BLRectI>& array) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array);
  }

  //! Fills an array of rectangles (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLArrayView<BLRectI>& array, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array, style);
  }

  //! Fills an `array` of rectangles of size `n` (integer coordinates) with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLRectI* array, size_t n) noexcept {
    return fill_rect_array(BLArrayView<BLRectI>{array, n});
  }

  //! Fills an `array` of rectangles of size `n` (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_rect_array(const BLRectI* array, size_t n, const StyleT& style) noexcept {
    return fill_rect_array(BLArrayView<BLRectI>{array, n}, style);
  }

  //! Fills the given `path` with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_path(const BLPathCore& path) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_PATH, &path);
  }

  //! Fills the given `path` with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_path(const BLPathCore& path, const StyleT& style) noexcept {
    return _fill_geometry_op(BL_GEOMETRY_TYPE_PATH, &path, style);
  }

  //! Fills the given `path` translated by `origin` with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_path(const BLPoint& origin, const BLPathCore& path) noexcept {
    BL_CONTEXT_CALL_RETURN(fill_path_d, impl, &origin, &path);
  }

  //! Fills the given `path` translated by `origin` with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_path(const BLPoint& origin, const BLPathCore& path, const StyleT& style) noexcept {
    return _fill_path_d(origin, path, style);
  }

  //! Fills the passed geometry specified by geometry `type` and `data` with the default fill style.
  //!
  //! \note This function provides a low-level interface that can be used in cases in which geometry `type` and `data`
  //! parameters are passed to a wrapper function that just passes them to the rendering context. It's a good way of
  //! creating wrappers, but generally low-level for a general purpose use, so please use this with caution.
  BL_INLINE_NODEBUG BLResult fill_geometry(BLGeometryType type, const void* data) noexcept {
    return _fill_geometry_op(type, data);
  }

  //! Fills the passed geometry specified by geometry `type` and `data` with an explicit fill `style`.
  //!
  //! \note This function provides a low-level interface that can be used in cases in which geometry `type` and `data`
  //! parameters are passed to a wrapper function that just passes them to the rendering context. It's a good way of
  //! creating wrappers, but generally low-level for a general purpose use, so please use this with caution.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_geometry(BLGeometryType type, const void* data, const StyleT& style) noexcept {
    return _fill_geometry_op(type, data, style);
  }

  //! \}

  //! \name Fill Text & Glyphs Operations
  //! \{

  //! Fills UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default fill style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated string.
  //! If you want to pass a non-null terminated string or a substring of an existing string, use either this function with
  //! a `size` parameter set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters)
  //! or an overloaded function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPointI& origin, const BLFontCore& font, const char* text, size_t size = SIZE_MAX) noexcept {
    BLStringView view{text, size};
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }

  //! Fills UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit fill `style`.
  //!
  //! \note Pass `SIZE_MAX` to `size` to inform Blend2D that the input is a null terminated string. If you want to pass
  //! a non-null terminated string or a substring of an existing string, use either this function with a `size` parameter
  //! set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters) or an overloaded
  //! function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPointI& origin, const BLFontCore& font, const char* text, size_t size, const StyleT& style) noexcept {
    BLStringView view{text, size};
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Fills UTF-8 encoded string passed as string `view` by using the given `font` at `origin` (floating point coordinates)
  //! with the default fill style.
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPointI& origin, const BLFontCore& font, const BLStringView& view) noexcept {
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }

  //! Fills UTF-8 encoded string passed as string `view` by using the given `font` at `origin` (floating point coordinates)
  //! with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPointI& origin, const BLFontCore& font, const BLStringView& view, const StyleT& style) noexcept {
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Fills UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default fill style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated string.
  //! If you want to pass a non-null terminated string or a substring of an existing string, use either this function with
  //! a `size` parameter set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters)
  //! or an overloaded function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPoint& origin, const BLFontCore& font, const char* text, size_t size = SIZE_MAX) noexcept {
    BLStringView view{text, size};
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }

  //! Fills UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit fill `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass
  //! a non-null terminated string or a substring of an existing string, use either this function with a `size` parameter
  //! set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters) or an overloaded
  //! function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPoint& origin, const BLFontCore& font, const char* text, size_t size, const StyleT& style) noexcept {
    BLStringView view{text, size};
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Fills UTF-8 encoded string passed as string `view` by using the given `font` at `origin` (floating point coordinates).
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPoint& origin, const BLFontCore& font, const BLStringView& view) noexcept {
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }
  //! \overload
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf8_text(const BLPoint& origin, const BLFontCore& font, const BLStringView& view, const StyleT& style) noexcept {
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Fills UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default fill style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-16
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 16-bit values).
  BL_INLINE_NODEBUG BLResult fill_utf16_text(const BLPointI& origin, const BLFontCore& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
  }

  //! Fills UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit fill `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 16-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf16_text(const BLPointI& origin, const BLFontCore& font, const uint16_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, style);
  }

  //! Fills UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with the default fill style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-16
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 16-bit values).
  BL_INLINE_NODEBUG BLResult fill_utf16_text(const BLPoint& origin, const BLFontCore& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
  }

  //! Fills UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with an explicit fill `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 16-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf16_text(const BLPoint& origin, const BLFontCore& font, const uint16_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, style);
  }

  //! Fills UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default fill style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-32
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 32-bit values).
  BL_INLINE_NODEBUG BLResult fill_utf32_text(const BLPointI& origin, const BLFontCore& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
  }

  //! Fills UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit fill `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 32-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf32_text(const BLPointI& origin, const BLFontCore& font, const uint32_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, style);
  }

  //! Fills UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with the default fill style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-32
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 32-bit values).
  BL_INLINE_NODEBUG BLResult fill_utf32_text(const BLPoint& origin, const BLFontCore& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
  }

  //! Fills UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with an explicit fill `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 32-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_utf32_text(const BLPoint& origin, const BLFontCore& font, const uint32_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, style);
  }

  //! Fills a `glyph_run` by using the given `font` at `origin` (integer coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_glyph_run(const BLPointI& origin, const BLFontCore& font, const BLGlyphRun& glyph_run) noexcept {
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run);
  }

  //! Fills a `glyph_run` by using the given `font` at `origin` (integer coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_glyph_run(const BLPointI& origin, const BLFontCore& font, const BLGlyphRun& glyph_run, const StyleT& style) noexcept {
    return _fill_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run, style);
  }

  //! Fills the passed `glyph_run` by using the given `font` at `origin` (floating point coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_glyph_run(const BLPoint& origin, const BLFontCore& font, const BLGlyphRun& glyph_run) noexcept {
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run);
  }

  //! Fills the passed `glyph_run` by using the given `font` at `origin` (floating point coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_glyph_run(const BLPoint& origin, const BLFontCore& font, const BLGlyphRun& glyph_run, const StyleT& style) noexcept {
    return _fill_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run, style);
  }

  //! \}

  //! \name Fill Mask Operations
  //! \{

  //! Fills a source `mask` image at coordinates specified by `origin` (int coordinates) with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPointI& origin, const BLImage& mask) noexcept {
    return _fill_mask_i(origin, mask, nullptr);
  }

  //! Fills a source `mask` image at coordinates specified by `origin` (int coordinates) with an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPointI& origin, const BLImage& mask, const StyleT& style) noexcept {
    return _fill_mask_i(origin, mask, nullptr, style);
  }

  //! Fills a source `mask` image specified by `mask_area` at coordinates specified by `origin` (int coordinates) with
  //! the current fill style.
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPointI& origin, const BLImage& mask, const BLRectI& mask_area) noexcept {
    return _fill_mask_i(origin, mask, &mask_area);
  }

  //! Fills a source `mask` image specified by `mask_area` at coordinates specified by `origin` (int coordinates) with
  //! an explicit fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPointI& origin, const BLImage& mask, const BLRectI& mask_area, const StyleT& style) noexcept {
    return _fill_mask_i(origin, mask, &mask_area, style);
  }

  //! Fills a source `mask` image at coordinates specified by `origin` (floating point coordinates) with the current
  //! fill style.
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPoint& origin, const BLImage& mask) noexcept {
    return _fill_mask_d(origin, mask, nullptr);
  }

  //! Fills a source `mask` image at coordinates specified by `origin` (floating point coordinates) with an explicit
  //! fill `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPoint& origin, const BLImage& mask, const StyleT& style) noexcept {
    return _fill_mask_d(origin, mask, nullptr, style);
  }

  //! Fills a source `mask` image specified by `mask_area` at coordinates specified by `origin` (floating point coordinates)
  //! with the current fill style.
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPoint& origin, const BLImage& mask, const BLRectI& mask_area) noexcept {
    return _fill_mask_d(origin, mask, &mask_area);
  }

  //! Fills a source `mask` image specified by `mask_area` at coordinates specified by `origin` (floating point coordinates)
  //! with an explicit fill style.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult fill_mask(const BLPoint& origin, const BLImage& mask, const BLRectI& mask_area, const StyleT& style) noexcept {
    return _fill_mask_d(origin, mask, &mask_area, style);
  }

  //! \}

  //! \cond INTERNAL
  //! \name Stroke Wrappers (Internal)
  //! \{

private:
  BL_INLINE_NODEBUG BLResult _stroke_geometry_op(BLGeometryType type, const void* data) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_geometry, impl, type, data);
  }

  BL_INLINE_NODEBUG BLResult _stroke_geometry_op(BLGeometryType type, const void* data, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(stroke_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_geometry_op(BLGeometryType type, const void* data, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_geometry_rgba32, impl, type, data, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _stroke_geometry_op(BLGeometryType type, const void* data, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(stroke_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_geometry_op(BLGeometryType type, const void* data, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_geometry_op(BLGeometryType type, const void* data, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_geometry_op(BLGeometryType type, const void* data, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_geometry_ext, impl, type, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_path_d(const BLPoint& origin, const BLPathCore& path, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(stroke_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_path_d(const BLPoint& origin, const BLPathCore& path, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_path_d_rgba32, impl, &origin, &path, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _stroke_path_d(const BLPoint& origin, const BLPathCore& path, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(stroke_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_path_d(const BLPoint& origin, const BLPathCore& path, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_path_d(const BLPoint& origin, const BLPathCore& path, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_path_d(const BLPoint& origin, const BLPathCore& path, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_path_d_ext, impl, &origin, &path, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_i, impl, &origin, &font, op, data);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(stroke_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_i_rgba32, impl, &origin, &font, op, data, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(stroke_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_i(const BLPointI& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_i_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_d, impl, &origin, &font, op, data);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba& rgba) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba);
    BL_CONTEXT_CALL_RETURN(stroke_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba32& rgba32) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_d_rgba32, impl, &origin, &font, op, data, rgba32.value);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLRgba64& rgba64) noexcept {
    BLVarCore style = BLInternal::make_inline_style(rgba64);
    BL_CONTEXT_CALL_RETURN(stroke_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLVarCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLPatternCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

  BL_INLINE_NODEBUG BLResult _stroke_text_op_d(const BLPoint& origin, const BLFontCore& font, BLContextRenderTextOp op, const void* data, const BLGradientCore& style) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_text_op_d_ext, impl, &origin, &font, op, data, &style);
  }

public:

  //! \}
  //! \endcond

  //! \name Stroke Geometry Operations
  //! \{

  //! Strokes a `box` (floating point coordinates) with the current stroke style.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref stroke_rect() instead.
  BL_INLINE_NODEBUG BLResult stroke_box(const BLBox& box) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_BOXD, &box);
  }

  //! Strokes a `box` (floating point coordinates) with an explicit stroke `style`.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref stroke_rect() instead.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_box(const BLBox& box, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_BOXD, &box, style);
  }

  //! Strokes a `box` (integer coordinates) with the current stroke style.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref stroke_rect() instead.
  BL_INLINE_NODEBUG BLResult stroke_box(const BLBoxI& box) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_BOXI, &box);
  }

  //! Strokes a `box` (integer coordinates) with an explicit stroke `style`.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref stroke_rect() instead.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_box(const BLBoxI& box, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_BOXI, &box, style);
  }

  //! Strokes a box `[x0, y0, x1, y1]` (floating point coordinates) with the current stroke style.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref stroke_rect() instead.
  BL_INLINE_NODEBUG BLResult stroke_box(double x0, double y0, double x1, double y1) noexcept {
    return stroke_box(BLBox(x0, y0, x1, y1));
  }

  //! Strokes a box `[x0, y0, x1, y1]` (floating point coordinates) with an explicit stroke `style`.
  //!
  //! \note Box is defined as `[x0, y0, x1, y1]`, if you need `[x, y, w, h]`, use \ref stroke_rect() instead.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_box(double x0, double y0, double x1, double y1, const StyleT& style) noexcept {
    return stroke_box(BLBox(x0, y0, x1, y1), style);
  }

  //! Strokes a rectangle `rect` (integer coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_rect(const BLRectI& rect) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_RECTI, &rect);
  }

  //! Strokes a rectangle `rect` (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_rect(const BLRectI& rect, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_RECTI, &rect, style);
  }

  //! Strokes a rectangle `rect` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_rect(const BLRect& rect) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_RECTD, &rect);
  }

  //! Strokes a rectangle `rect` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_rect(const BLRect& rect, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_RECTD, &rect, style);
  }

  //! Strokes a rectangle `[x, y, w, h]` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_rect(double x, double y, double w, double h) noexcept {
    return stroke_rect(BLRect(x, y, w, h));
  }

  //! Strokes a rectangle `[x, y, w, h]` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_rect(double x, double y, double w, double h, const StyleT& style) noexcept {
    return stroke_rect(BLRect(x, y, w, h), style);
  }

  //! Strokes a line specified as `line` (floating point coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_line(const BLLine& line) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_LINE, &line);
  }

  //! Strokes a line specified as `line` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_line(const BLLine& line, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_LINE, &line, style);
  }

  //! Strokes a line starting at `p0` and ending at `p1` (floating point coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_line(const BLPoint& p0, const BLPoint& p1) noexcept {
    return stroke_line(BLLine(p0.x, p0.y, p1.x, p1.y));
  }

  //! Strokes a line starting at `p0` and ending at `p1` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_line(const BLPoint& p0, const BLPoint& p1, const StyleT& style) noexcept {
    return stroke_line(BLLine(p0.x, p0.y, p1.x, p1.y), style);
  }

  //! Strokes a line starting at `[x0, y0]` and ending at `[x1, y1]` (floating point coordinates) with the default
  //! stroke style.
  BL_INLINE_NODEBUG BLResult stroke_line(double x0, double y0, double x1, double y1) noexcept {
    return stroke_line(BLLine(x0, y0, x1, y1));
  }

  //! Strokes a line starting at `[x0, y0]` and ending at `[x1, y1]` (floating point coordinates) with an explicit
  //! stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_line(double x0, double y0, double x1, double y1, const StyleT& style) noexcept {
    return stroke_line(BLLine(x0, y0, x1, y1), style);
  }

  //! Strokes a `circle` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_circle(const BLCircle& circle) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_CIRCLE, &circle);
  }

  //! Strokes a `circle` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_circle(const BLCircle& circle, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_CIRCLE, &circle, style);
  }

  //! Strokes a circle at `[cx, cy]` and radius `r` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_circle(double cx, double cy, double r) noexcept {
    return stroke_circle(BLCircle(cx, cy, r));
  }

  //! Strokes a circle at `[cx, cy]` and radius `r` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_circle(double cx, double cy, double r, const StyleT& style) noexcept {
    return stroke_circle(BLCircle(cx, cy, r), style);
  }

  //! Strokes an `ellipse` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_ellipse(const BLEllipse& ellipse) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse);
  }

  //! Strokes an `ellipse` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_ellipse(const BLEllipse& ellipse, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse, style);
  }

  //! Strokes an ellipse at `[cx, cy]` with radius `[rx, ry]` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_ellipse(double cx, double cy, double rx, double ry) noexcept {
    return stroke_ellipse(BLEllipse(cx, cy, rx, ry));
  }

  //! Strokes an ellipse at `[cx, cy]` with radius `[rx, ry]` (floating point coordinates) with en explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_ellipse(double cx, double cy, double rx, double ry, const StyleT& style) noexcept {
    return stroke_ellipse(BLEllipse(cx, cy, rx, ry), style);
  }

  //! Strokes a rounded rectangle `rr` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_round_rect(const BLRoundRect& rr) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ROUND_RECT, &rr);
  }

  //! Strokes a rounded rectangle `rr` (floating point coordinates) with en explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_round_rect(const BLRoundRect& rr, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ROUND_RECT, &rr, style);
  }

  //! Strokes a rounded rectangle bounded by `rect` with radius `r` with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_round_rect(const BLRect& rect, double r) noexcept {
    return stroke_round_rect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, r));
  }

  //! Strokes a rounded rectangle bounded by `rect` with radius `[rx, ry]` with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_round_rect(const BLRect& rect, double rx, double ry) noexcept {
    return stroke_round_rect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, rx, ry));
  }

  //! Strokes a rounded rectangle bounded by `rect` with radius `[rx, ry]` with en explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_round_rect(const BLRect& rect, double rx, double ry, const StyleT& style) noexcept {
    return stroke_round_rect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, rx, ry), style);
  }

  //! Strokes a rounded rectangle bounded by `[x, y, w, h]` with radius `r` with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_round_rect(double x, double y, double w, double h, double r) noexcept {
    return stroke_round_rect(BLRoundRect(x, y, w, h, r));
  }

  //! Strokes a rounded rectangle bounded as `[x, y, w, h]` with radius `[rx, ry]` with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_round_rect(double x, double y, double w, double h, double rx, double ry) noexcept {
    return stroke_round_rect(BLRoundRect(x, y, w, h, rx, ry));
  }

  //! Strokes a rounded rectangle bounded as `[x, y, w, h]` with radius `[rx, ry]` with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_round_rect(double x, double y, double w, double h, double rx, double ry, const StyleT& style) noexcept {
    return stroke_round_rect(BLRoundRect(x, y, w, h, rx, ry), style);
  }

  //! Strokes an `arc` with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_arc(const BLArc& arc) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARC, &arc);
  }

  //! Strokes an `arc` with an explicit stroke `style.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_arc(const BLArc& arc, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARC, &arc, style);
  }

  //! Strokes a chord at `[cx, cy]` with radius `r` at `start` of `sweep` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_arc(double cx, double cy, double r, double start, double sweep) noexcept {
    return stroke_arc(BLArc(cx, cy, r, r, start, sweep));
  }

  //! Strokes a chord at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_arc(double cx, double cy, double rx, double ry, double start, double sweep) noexcept {
    return stroke_arc(BLArc(cx, cy, rx, ry, start, sweep));
  }

  //! Strokes a chord at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_arc(double cx, double cy, double rx, double ry, double start, double sweep, const StyleT& style) noexcept {
    return stroke_arc(BLArc(cx, cy, rx, ry, start, sweep), style);
  }

  //! Strokes a `chord` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_chord(const BLArc& chord) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_CHORD, &chord);
  }

  //! Strokes a `chord` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_chord(const BLArc& chord, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_CHORD, &chord, style);
  }

  //! Strokes a chord at `[cx, cy]` with radius `r` at `start` of `sweep` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_chord(double cx, double cy, double r, double start, double sweep) noexcept {
    return stroke_chord(BLArc(cx, cy, r, r, start, sweep));
  }

  //! Strokes a chord at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_chord(double cx, double cy, double rx, double ry, double start, double sweep) noexcept {
    return stroke_chord(BLArc(cx, cy, rx, ry, start, sweep));
  }

  //! Strokes a chord at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_chord(double cx, double cy, double rx, double ry, double start, double sweep, const StyleT& style) noexcept {
    return stroke_chord(BLArc(cx, cy, rx, ry, start, sweep), style);
  }

  //! Strokes a `pie` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_pie(const BLArc& pie) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_PIE, &pie);
  }

  //! Strokes a `pie` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_pie(const BLArc& pie, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_PIE, &pie, style);
  }

  //! Strokes a pie at `[cx, cy]` with radius `r` at `start` of `sweep` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_pie(double cx, double cy, double r, double start, double sweep) noexcept {
    return stroke_pie(BLArc(cx, cy, r, r, start, sweep));
  }

  //! Strokes a pie at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_pie(double cx, double cy, double rx, double ry, double start, double sweep) noexcept {
    return stroke_pie(BLArc(cx, cy, rx, ry, start, sweep));
  }

  //! Strokes a pie at `[cx, cy]` with radius `[rx, ry]` at `start` of `sweep` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_pie(double cx, double cy, double rx, double ry, double start, double sweep, const StyleT& style) noexcept {
    return stroke_pie(BLArc(cx, cy, rx, ry, start, sweep), style);
  }

  //! Strokes a `triangle` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_triangle(const BLTriangle& triangle) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_TRIANGLE, &triangle);
  }

  //! Strokes a `triangle` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_triangle(const BLTriangle& triangle, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_TRIANGLE, &triangle, style);
  }

  //! Strokes a triangle defined by `[x0, y0]`, `[x1, y1]`, `[x2, y2]` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_triangle(double x0, double y0, double x1, double y1, double x2, double y2) noexcept {
    return stroke_triangle(BLTriangle(x0, y0, x1, y1, x2, y2));
  }

  //! Strokes a triangle defined by `[x0, y0]`, `[x1, y1]`, `[x2, y2]` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_triangle(double x0, double y0, double x1, double y1, double x2, double y2, const StyleT& style) noexcept {
    return stroke_triangle(BLTriangle(x0, y0, x1, y1, x2, y2), style);
  }

  //! Strokes a polyline `poly` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLArrayView<BLPoint>& poly) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYLINED, &poly);
  }

  //! Strokes a polyline `poly` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLArrayView<BLPoint>& poly, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYLINED, &poly, style);
  }

  //! Strokes a polyline `poly` having `n` vertices (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLPoint* poly, size_t n) noexcept {
    return stroke_polyline(BLArrayView<BLPoint>{poly, n});
  }

  //! Strokes a polyline `poly` having `n` vertices (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLPoint* poly, size_t n, const StyleT& style) noexcept {
    return stroke_polyline(BLArrayView<BLPoint>{poly, n}, style);
  }

  //! Strokes a polyline `poly` (integer coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLArrayView<BLPointI>& poly) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYLINEI, &poly);
  }

  //! Strokes a polyline `poly` (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLArrayView<BLPointI>& poly, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYLINEI, &poly, style);
  }

  //! Strokes a polyline `poly` having `n` vertices (integer coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLPointI* poly, size_t n) noexcept {
    return stroke_polyline(BLArrayView<BLPointI>{poly, n});
  }

  //! Strokes a polyline `poly` having `n` vertices (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polyline(const BLPointI* poly, size_t n, const StyleT& style) noexcept {
    return stroke_polyline(BLArrayView<BLPointI>{poly, n}, style);
  }

  //! Strokes a polygon `poly` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLArrayView<BLPoint>& poly) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYGOND, &poly);
  }

  //! Strokes a polygon `poly` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLArrayView<BLPoint>& poly, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYGOND, &poly, style);
  }

  //! Strokes a polygon `poly` having `n` vertices (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLPoint* poly, size_t n) noexcept {
    return stroke_polygon(BLArrayView<BLPoint>{poly, n});
  }

  //! Strokes a polygon `poly` having `n` vertices (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLPoint* poly, size_t n, const StyleT& style) noexcept {
    return stroke_polygon(BLArrayView<BLPoint>{poly, n}, style);
  }

  //! Strokes a polygon `poly` (integer coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLArrayView<BLPointI>& poly) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYGONI, &poly);
  }

  //! Strokes a polygon `poly` (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLArrayView<BLPointI>& poly, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_POLYGONI, &poly, style);
  }

  //! Strokes a polygon `poly` having `n` vertices (integer coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLPointI* poly, size_t n) noexcept {
    return stroke_polygon(BLArrayView<BLPointI>{poly, n});
  }

  //! Strokes a polygon `poly` having `n` vertices (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_polygon(const BLPointI* poly, size_t n, const StyleT& style) noexcept {
    return stroke_polygon(BLArrayView<BLPointI>{poly, n}, style);
  }

  //! Strokes an `array` of boxes (floating point coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLArrayView<BLBox>& array) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array);
  }

  //! Strokes an `array` of boxes (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLArrayView<BLBox>& array, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array, style);
  }

  //! Strokes an `array` of boxes of size `n` (floating point coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLBox* array, size_t n) noexcept {
    return stroke_box_array(BLArrayView<BLBox>{array, n});
  }

  //! Strokes an `array` of boxes of size `n` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLBox* array, size_t n, const StyleT& style) noexcept {
    return stroke_box_array(BLArrayView<BLBox>{array, n}, style);
  }

  //! Strokes an `array` of boxes (integer coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLArrayView<BLBoxI>& array) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array);
  }

  //! Strokes an `array` of boxes (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLArrayView<BLBoxI>& array, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array, style);
  }

  //! Strokes an `array` of boxes of size `n` (integer coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLBoxI* array, size_t n) noexcept {
    return stroke_box_array(BLArrayView<BLBoxI>{array, n});
  }

  //! Strokes an array of boxes of size `n` (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_box_array(const BLBoxI* array, size_t n, const StyleT& style) noexcept {
    return stroke_box_array(BLArrayView<BLBoxI>{array, n}, style);
  }

  //! Strokes an `array` of rectangles (floating point coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLArrayView<BLRect>& array) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array);
  }

  //! Strokes an `array` of rectangles (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLArrayView<BLRect>& array, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array, style);
  }

  //! Strokes an `array` of rectangles of size `n` (floating point coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLRect* array, size_t n) noexcept {
    return stroke_rect_array(BLArrayView<BLRect>{array, n});
  }

  //! Strokes an `array` of rectangles of size `n` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLRect* array, size_t n, const StyleT& style) noexcept {
    return stroke_rect_array(BLArrayView<BLRect>{array, n}, style);
  }

  //! Strokes an array of rectangles (integer coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLArrayView<BLRectI>& array) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array);
  }

  //! Strokes an array of rectangles (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLArrayView<BLRectI>& array, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array, style);
  }

  //! Strokes an `array` of rectangles of size `n` (integer coordinates) with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLRectI* array, size_t n) noexcept {
    return stroke_rect_array(BLArrayView<BLRectI>{array, n});
  }

  //! Strokes an `array` of rectangles of size `n` (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_rect_array(const BLRectI* array, size_t n, const StyleT& style) noexcept {
    return stroke_rect_array(BLArrayView<BLRectI>{array, n}, style);
  }

  //! Strokes the given `path` with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_path(const BLPathCore& path) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_PATH, &path);
  }

  //! Strokes the given `path` with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_path(const BLPathCore& path, const StyleT& style) noexcept {
    return _stroke_geometry_op(BL_GEOMETRY_TYPE_PATH, &path, style);
  }

  //! Strokes the given `path` translated by `origin` with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_path(const BLPoint& origin, const BLPathCore& path) noexcept {
    BL_CONTEXT_CALL_RETURN(stroke_path_d, impl, &origin, &path);
  }

  //! Strokes the given `path` translated by `origin` with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_path(const BLPoint& origin, const BLPathCore& path, const StyleT& style) noexcept {
    return _stroke_path_d(origin, path, style);
  }

  //! Strokes the passed geometry specified by geometry `type` and `data` with the default stroke style.
  //!
  //! \note This function provides a low-level interface that can be used in cases in which geometry `type` and `data`
  //! parameters are passed to a wrapper function that just passes them to the rendering context. It's a good way of
  //! creating wrappers, but generally low-level for a general purpose use, so please use this with caution.
  BL_INLINE_NODEBUG BLResult stroke_geometry(BLGeometryType type, const void* data) noexcept {
    return _stroke_geometry_op(type, data);
  }

  //! Strokes the passed geometry specified by geometry `type` and `data` with an explicit stroke `style`.
  //!
  //! \note This function provides a low-level interface that can be used in cases in which geometry `type` and `data`
  //! parameters are passed to a wrapper function that just passes them to the rendering context. It's a good way of
  //! creating wrappers, but generally low-level for a general purpose use, so please use this with caution.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_geometry(BLGeometryType type, const void* data, const StyleT& style) noexcept {
    return _stroke_geometry_op(type, data, style);
  }

  //! \}

  //! \name Stroke Text & Glyphs Operations
  //! \{

  //! Strokes UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default stroke style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated string.
  //! If you want to pass a non-null terminated string or a substring of an existing string, use either this function with
  //! a `size` parameter set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters)
  //! or an overloaded function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPointI& origin, const BLFontCore& font, const char* text, size_t size = SIZE_MAX) noexcept {
    BLStringView view{text, size};
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }

  //! Strokes UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit stroke `style`.
  //!
  //! \note Pass `SIZE_MAX` to `size` to inform Blend2D that the input is a null terminated string. If you want to pass
  //! a non-null terminated string or a substring of an existing string, use either this function with a `size` parameter
  //! set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters) or an overloaded
  //! function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPointI& origin, const BLFontCore& font, const char* text, size_t size, const StyleT& style) noexcept {
    BLStringView view{text, size};
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Strokes UTF-8 encoded string passed as string `view` by using the given `font` at `origin` (floating point coordinates)
  //! with the default stroke style.
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPointI& origin, const BLFontCore& font, const BLStringView& view) noexcept {
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }

  //! Strokes UTF-8 encoded string passed as string `view` by using the given `font` at `origin` (floating point coordinates)
  //! with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPointI& origin, const BLFontCore& font, const BLStringView& view, const StyleT& style) noexcept {
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Strokes UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default stroke style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated string.
  //! If you want to pass a non-null terminated string or a substring of an existing string, use either this function with
  //! a `size` parameter set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters)
  //! or an overloaded function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPoint& origin, const BLFontCore& font, const char* text, size_t size = SIZE_MAX) noexcept {
    BLStringView view{text, size};
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }

  //! Strokes UTF-8 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit stroke `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass
  //! a non-null terminated string or a substring of an existing string, use either this function with a `size` parameter
  //! set (which contains the number of bytes of `text` string, and not the number of UTF-8 characters) or an overloaded
  //! function that accepts a convenience \ref BLStringView parameter instead of `text` and `size`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPoint& origin, const BLFontCore& font, const char* text, size_t size, const StyleT& style) noexcept {
    BLStringView view{text, size};
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Strokes UTF-8 encoded string passed as string `view` by using the given `font` at `origin` (floating point coordinates).
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPoint& origin, const BLFontCore& font, const BLStringView& view) noexcept {
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
  }
  //! \overload
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf8_text(const BLPoint& origin, const BLFontCore& font, const BLStringView& view, const StyleT& style) noexcept {
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, style);
  }

  //! Strokes UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default stroke style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-16
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 16-bit values).
  BL_INLINE_NODEBUG BLResult stroke_utf16_text(const BLPointI& origin, const BLFontCore& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
  }

  //! Strokes UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit stroke `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 16-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf16_text(const BLPointI& origin, const BLFontCore& font, const uint16_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, style);
  }

  //! Strokes UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with the default stroke style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-16
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 16-bit values).
  BL_INLINE_NODEBUG BLResult stroke_utf16_text(const BLPoint& origin, const BLFontCore& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
  }

  //! Strokes UTF-16 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with an explicit stroke `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 16-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf16_text(const BLPoint& origin, const BLFontCore& font, const uint16_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint16_t> view{text, size};
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, style);
  }

  //! Strokes UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with the default stroke style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-32
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 32-bit values).
  BL_INLINE_NODEBUG BLResult stroke_utf32_text(const BLPointI& origin, const BLFontCore& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
  }

  //! Strokes UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (integer coordinates)
  //! with an explicit stroke `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 32-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf32_text(const BLPointI& origin, const BLFontCore& font, const uint32_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, style);
  }

  //! Strokes UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with the default stroke style.
  //!
  //! \note The `size` parameter defaults to `SIZE_MAX`, which informs Blend2D that the input is a null terminated UTF-32
  //! string. If you want to pass a non-null terminated string or a substring of an existing string, specify `size`
  //! parameter to the size of the `text` buffer in code units (the number of 32-bit values).
  BL_INLINE_NODEBUG BLResult stroke_utf32_text(const BLPoint& origin, const BLFontCore& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
  }

  //! Strokes UTF-32 encoded string passed as `text` and `size` by using the given `font` at `origin` (floating point coordinates)
  //! with an explicit stroke `style`.
  //!
  //! \note Pass `SIZE_MAX` in `size` to inform Blend2D that the input is a null terminated string. If you want to pass a
  //! non-null terminated string or a substring of an existing string, specify `size` parameter to the size of the `text`
  //! buffer in code units (the number of 32-bit values).
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_utf32_text(const BLPoint& origin, const BLFontCore& font, const uint32_t* text, size_t size, const StyleT& style) noexcept {
    BLArrayView<uint32_t> view{text, size};
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, style);
  }

  //! Strokes a `glyph_run` by using the given `font` at `origin` (integer coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_glyph_run(const BLPointI& origin, const BLFontCore& font, const BLGlyphRun& glyph_run) noexcept {
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run);
  }

  //! Strokes a `glyph_run` by using the given `font` at `origin` (integer coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_glyph_run(const BLPointI& origin, const BLFontCore& font, const BLGlyphRun& glyph_run, const StyleT& style) noexcept {
    return _stroke_text_op_i(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run, style);
  }

  //! Strokes the passed `glyph_run` by using the given `font` at `origin` (floating point coordinates) with the current stroke style.
  BL_INLINE_NODEBUG BLResult stroke_glyph_run(const BLPoint& origin, const BLFontCore& font, const BLGlyphRun& glyph_run) noexcept {
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run);
  }

  //! Strokes the passed `glyph_run` by using the given `font` at `origin` (floating point coordinates) with an explicit stroke `style`.
  template<typename StyleT>
  BL_INLINE_NODEBUG BLResult stroke_glyph_run(const BLPoint& origin, const BLFontCore& font, const BLGlyphRun& glyph_run, const StyleT& style) noexcept {
    return _stroke_text_op_d(origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, &glyph_run, style);
  }

  //! \}

  //! \name Image Blit Operations
  //! \{

  //! Blits source image `src` at coordinates specified by `origin` (int coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLPointI& origin, const BLImageCore& src) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_image_i, impl, &origin, &src, nullptr);
  }

  //! Blits an area in source image `src` specified by `src_area` at coordinates specified by `origin` (int coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLPointI& origin, const BLImageCore& src, const BLRectI& src_area) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_image_i, impl, &origin, &src, &src_area);
  }

  //! Blits source image `src` at coordinates specified by `origin` (floating point coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLPoint& origin, const BLImageCore& src) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_image_d, impl, &origin, &src, nullptr);
  }

  //! Blits an area of source image `src` specified by `src_area` at coordinates specified by `origin` (floating point coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLPoint& origin, const BLImageCore& src, const BLRectI& src_area) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_image_d, impl, &origin, &src, &src_area);
  }

  //! Blits a source image `src` scaled to fit into `rect` rectangle (int coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLRectI& rect, const BLImageCore& src) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_scaled_image_i, impl, &rect, &src, nullptr);
  }

  //! Blits an area of source image `src` specified by `src_area` scaled to fit into `rect` rectangle (int coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLRectI& rect, const BLImageCore& src, const BLRectI& src_area) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_scaled_image_i, impl, &rect, &src, &src_area);
  }

  //! Blits a source image `src` scaled to fit into `rect` rectangle (floating point coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLRect& rect, const BLImageCore& src) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_scaled_image_d, impl, &rect, &src, nullptr);
  }

  //! Blits an area of source image `src` specified by `src_area` scaled to fit into `rect` rectangle (floating point coordinates).
  BL_INLINE_NODEBUG BLResult blit_image(const BLRect& rect, const BLImageCore& src, const BLRectI& src_area) noexcept {
    BL_CONTEXT_CALL_RETURN(blit_scaled_image_d, impl, &rect, &src, &src_area);
  }

  //! \}

  #undef BL_CONTEXT_CALL_RETURN
  #undef BL_CONTEXT_IMPL
};

//! \}
#endif

//! \}

#endif // BLEND2D_CONTEXT_H_INCLUDED
