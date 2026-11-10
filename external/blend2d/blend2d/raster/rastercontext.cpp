// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/compopinfo_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/pathstroke_p.h>
#include <blend2d/core/pattern_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/core/var_p.h>
#include <blend2d/pipeline/piperuntime_p.h>
#include <blend2d/pixelops/scalar_p.h>
#include <blend2d/pipeline/reference/fixedpiperuntime_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rastercontext_p.h>
#include <blend2d/raster/rastercontextops_p.h>
#include <blend2d/raster/rendercommand_p.h>
#include <blend2d/raster/rendercommandprocsync_p.h>
#include <blend2d/raster/rendertargetinfo_p.h>
#include <blend2d/raster/workerproc_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/stringops_p.h>
#include <blend2d/support/traits_p.h>
#include <blend2d/support/zeroallocator_p.h>

#ifndef BL_BUILD_NO_JIT
  #include <blend2d/pipeline/jit/pipegenruntime_p.h>
#endif

namespace bl::RasterEngine {

static constexpr bool kNoBail = false;

static constexpr RenderingMode kSync = RenderingMode::kSync;
static constexpr RenderingMode kAsync = RenderingMode::kAsync;

// bl::RasterEngine - ContextImpl - Globals
// ========================================

static BLContextVirt raster_impl_virt_sync;
static BLContextVirt raster_impl_virt_async;

// bl::RasterEngine - ContextImpl - Tables
// =======================================

struct SolidDataWrapperU8 {
  Pipeline::Signature signature;
  uint32_t dummy1;
  uint64_t dummy2;
  uint32_t prgb32;
  uint32_t padding;
};

struct SolidDataWrapperU16 {
  Pipeline::Signature signature;
  uint32_t dummy1;
  uint64_t dummy2;
  uint64_t prgb64;
};

static const constexpr SolidDataWrapperU8 solid_override_fill_u8[] = {
  { {0u}, 0u, 0u, 0x00000000u, 0u }, // kNotModified.
  { {0u}, 0u, 0u, 0x00000000u, 0u }, // kTransparent.
  { {0u}, 0u, 0u, 0xFF000000u, 0u }, // kOpaqueBlack.
  { {0u}, 0u, 0u, 0xFFFFFFFFu, 0u }, // kOpaqueWhite.
  { {0u}, 0u, 0u, 0x00000000u, 0u }  // kAlwaysNop.
};

static const constexpr SolidDataWrapperU16 solid_override_fill_u16[] = {
  { {0u}, 0u, 0u, 0x0000000000000000u }, // kNotModified.
  { {0u}, 0u, 0u, 0x0000000000000000u }, // kTransparent.
  { {0u}, 0u, 0u, 0xFFFF000000000000u }, // kOpaqueBlack.
  { {0u}, 0u, 0u, 0xFFFFFFFFFFFFFFFFu }, // kOpaqueWhite.
  { {0u}, 0u, 0u, 0x0000000000000000u }  // kAlwaysNop.
};

static const uint8_t text_byte_size_shift_by_encoding[] = { 0, 1, 2, 0 };

// bl::RasterEngine - ContextImpl - Internals - Uncategorized Yet
// ==============================================================

static BL_INLINE FormatExt formatFromRgba32(uint32_t rgba32) noexcept {
  return rgba32 == 0x00000000u ? FormatExt::kZERO32 :
         rgba32 >= 0xFF000000u ? FormatExt::kFRGB32 : FormatExt::kPRGB32;
}

// bl::RasterEngine - ContextImpl - Internals - Dispatch Info / Style
// ==================================================================

// We want to pass some data from the frontend down during the dispatching. Ideally, we just want to pass this data
// as value in registers. To minimize the registers required to pass some values as parameters, we can use 64-bit
// type on 64-bit target, which would save us one register to propagate.
union DispatchInfo {
  //! \name Members
  //! \{

  uint64_t packed;
  struct {
#if BL_BYTE_ORDER == 1234
    Pipeline::Signature signature;
    uint32_t alpha;
#else
    uint32_t alpha;
    Pipeline::Signature signature;
#endif
  };

  //! \}

  //! \name Interface
  //! \{

  BL_INLINE_NODEBUG void init(Pipeline::Signature signature_value, uint32_t alpha_value) noexcept {
    alpha = alpha_value;
    signature = signature_value;
  }

  BL_INLINE_NODEBUG void add_signature(Pipeline::Signature sgn) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
    packed |= sgn.value;
#else
    signature |= sgn;
#endif
  }

  BL_INLINE_NODEBUG void add_fill_type(Pipeline::FillType fill_type) noexcept {
    add_signature(Pipeline::Signature::from_fill_type(fill_type));
  }

  //! \}
};

//! Another data that is passed by value during a render call dispatching.
struct DispatchStyle {
  //! \name Members
  //! \{

  RenderFetchDataHeader* fetch_data;

  //! \}
};

// bl::RasterEngine - ContextImpl - Internals - DirectStateAccessor
// ================================================================

class DirectStateAccessor {
public:
  const BLRasterContextImpl* ctx_impl;

  BL_INLINE_NODEBUG explicit DirectStateAccessor(const BLRasterContextImpl* ctx_impl) noexcept : ctx_impl(ctx_impl) {}

  BL_INLINE_NODEBUG const BLBox& final_clip_box_d() const noexcept { return ctx_impl->final_clip_box_d(); }
  BL_INLINE_NODEBUG const BLBox& final_clip_box_fixed_d() const noexcept { return ctx_impl->final_clip_box_fixed_d(); }

  BL_INLINE_NODEBUG const BLStrokeOptions& stroke_options() const noexcept { return ctx_impl->stroke_options(); }
  BL_INLINE_NODEBUG const BLApproximationOptions& approximation_options() const noexcept { return ctx_impl->approximation_options(); }

  BL_INLINE_NODEBUG BLTransformType meta_transform_fixed_type() const noexcept { return ctx_impl->meta_transform_fixed_type(); }
  BL_INLINE_NODEBUG BLTransformType final_transform_fixed_type() const noexcept { return ctx_impl->final_transform_fixed_type(); }

  BL_INLINE BLMatrix2D user_transform() const noexcept {
    const BLMatrix2D& t = ctx_impl->user_transform();
    return BLMatrix2D(t.m00, t.m01, t.m10, t.m11, 0.0, 0.0);
  }

  BL_INLINE BLMatrix2D final_transform_fixed(const BLPoint& origin_fixed) const noexcept {
    const BLMatrix2D& t = ctx_impl->final_transform_fixed();
    return BLMatrix2D(t.m00, t.m01, t.m10, t.m11, origin_fixed.x, origin_fixed.y);
  }

  BL_INLINE BLMatrix2D meta_transform_fixed(const BLPoint& origin_fixed) const noexcept {
    const BLMatrix2D& t = ctx_impl->meta_transform_fixed();
    return BLMatrix2D(t.m00, t.m01, t.m10, t.m11, origin_fixed.x, origin_fixed.y);
  }
};

// bl::RasterEngine - ContextImpl - Internals - SyncWorkState
// ==========================================================

//! State that is used by the synchronous rendering context when using `sync_work_data` to execute the work
//! in user thread. Some properties of `WorkData` are used as states, and those have to be saved/restored.
class SyncWorkState {
public:
  BLBox _clip_box_d;

  BL_INLINE_NODEBUG void save(const WorkData& work_data) noexcept { _clip_box_d = work_data.edge_builder._clip_box_d; }
  BL_INLINE_NODEBUG void restore(WorkData& work_data) const noexcept { work_data.edge_builder._clip_box_d = _clip_box_d; }
};

// bl::RasterEngine - ContextImpl - Internals - Core State
// =======================================================

static BL_INLINE void on_before_config_change(BLRasterContextImpl* ctx_impl) noexcept {
  if (bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateConfig)) {
    SavedState* state = ctx_impl->saved_state;
    state->approximation_options = ctx_impl->approximation_options();
  }
}

static BL_INLINE void on_after_flatten_tolerance_changed(BLRasterContextImpl* ctx_impl) noexcept {
  ctx_impl->internal_state.toleranceFixedD = ctx_impl->approximation_options().flatten_tolerance * ctx_impl->render_target_info.fp_scale_d;
  ctx_impl->sync_work_data.edge_builder.set_flatten_tolerance_sq(Math::square(ctx_impl->internal_state.toleranceFixedD));
}

static BL_INLINE void on_after_offset_parameter_changed(BLRasterContextImpl* ctx_impl) noexcept {
  bl_unused(ctx_impl);
}

static BL_INLINE void on_after_comp_op_changed(BLRasterContextImpl* ctx_impl) noexcept {
  ctx_impl->comp_op_simplify_info = comp_op_simplify_info_array_of(CompOpExt(ctx_impl->comp_op()), ctx_impl->format());
}

// bl::RasterEngine - ContextImpl - Internals - Style State
// ========================================================

static BL_INLINE void init_style_to_default(BLRasterContextImpl* ctx_impl, BLContextStyleSlot slot) noexcept {
  ctx_impl->internal_state.style_type[slot] = uint8_t(BL_OBJECT_TYPE_RGBA32);

  StyleData& style = ctx_impl->internal_state.style[slot];
  style = StyleData{};
  style.solid.init_header(0, FormatExt(ctx_impl->solid_format_table[BL_RASTER_CONTEXT_SOLID_FORMAT_FRGB]));
  style.solid.pipeline_data = ctx_impl->solid_override_fill_table[size_t(CompOpSolidId::kOpaqueBlack)].pipeline_data;
  style.solid.original.rgba32.value = 0xFF000000u;
  style.make_fetch_data_implicit();
}

static BL_INLINE void destroy_valid_style(BLRasterContextImpl* ctx_impl, StyleData* style) noexcept {
  RenderFetchData* fetch_data = static_cast<RenderFetchData*>(style->fetch_data);
  fetch_data->release(ctx_impl);
}

static BL_INLINE void on_before_style_change(BLRasterContextImpl* ctx_impl, BLContextStyleSlot slot, StyleData& style, ContextFlags context_flags) noexcept {
  if (bl_test_flag(context_flags, ContextFlags::kFetchDataBase << slot)) {
    if (!bl_test_flag(context_flags, ContextFlags::kWeakStateBaseStyle << slot)) {
      RenderFetchData* fetch_data = style.get_render_fetch_data();
      fetch_data->release(ctx_impl);
      return;
    }
  }
  else {
    BL_ASSERT(bl_test_flag(context_flags, ContextFlags::kWeakStateBaseStyle << slot));
  }

  BL_ASSERT(ctx_impl->saved_state != nullptr);
  ctx_impl->saved_state->style[slot].copy_from(style);
}

// bl::RasterEngine - ContextImpl - Internals - Fetch Data Initialization
// ======================================================================

// Recycle means that the FetchData was allocated by the rendering context 'set_style()' function and it's pooled.
static void BL_CDECL recycle_fetch_data_image(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data) noexcept {
  ImageInternal::release_instance(static_cast<BLImageCore*>(&fetch_data->style));
  ctx_impl->free_fetch_data(fetch_data);
}

static void BL_CDECL recycle_fetch_data_pattern(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data) noexcept {
  PatternInternal::release_instance(static_cast<BLPatternCore*>(&fetch_data->style));
  ctx_impl->free_fetch_data(fetch_data);
}

static void BL_CDECL recycle_fetch_data_gradient(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data) noexcept {
  GradientInternal::release_instance(static_cast<BLGradientCore*>(&fetch_data->style));
  ctx_impl->free_fetch_data(fetch_data);
}

// Destroy is used exclusively by the multi-threaded rendering context implementation. This FetchData was allocated
// during a render call dispatch in which a Style has been passed explicitly to the call. This kind of FetchData is
// one-shot: only one reference to it exists.
static void BL_CDECL destroy_fetch_data_image(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data) noexcept {
  bl_unused(ctx_impl);
  ImageInternal::release_instance(static_cast<BLImageCore*>(&fetch_data->style));
}

static void BL_CDECL destroy_fetch_data_gradient(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data) noexcept {
  bl_unused(ctx_impl);
  GradientInternal::release_instance(static_cast<BLGradientCore*>(&fetch_data->style));
}

// Creating FetchData
// ------------------

// There are in general two ways FetchData can be created:
//
//   - using 'BLContext::set_style()'
//   - passing Style explicitly to the frontend function suffixed by 'Ext'
//   - passing an Image to a 'blit_image' frontend function
//
// When FetchData is created by set_style() it will become part of the rendering context State, which means that
// such FetchData can be saved, restored, reused, etc... The rendering context uses a reference count to keep
// track of such FetchData and must maintain additional properties to make get_style() working.
//
// On the other hand, when FetchData is created from an explicitly passed style / image to a render call, it's
// only used once and doesn't need anything to be able to get that style in the future, which means that it's
// much easier to manage.
//
// Applier is used with init_non_solid_fetch_data function to unify both concepts and to share code.

class NonSolidFetchStateApplier {
public:
  static constexpr bool kIsExplicit = false;

  ContextFlags _context_flags;
  ContextFlags _style_flags;
  BLContextStyleSlot _slot;

  BL_INLINE_NODEBUG NonSolidFetchStateApplier(ContextFlags context_flags, BLContextStyleSlot slot) noexcept
    : _context_flags(context_flags),
      _style_flags(ContextFlags::kFetchDataBase),
      _slot(slot) {}

  BL_INLINE void init_style_type(BLRasterContextImpl* ctx_impl, BLObjectType style_type) noexcept {
    ctx_impl->internal_state.style_type[_slot] = uint8_t(style_type);
  }

  BL_INLINE void init_computed_transform(BLRasterContextImpl* ctx_impl, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {
    if (transform_type >= BL_TRANSFORM_TYPE_INVALID)
      mark_as_nop();
    ctx_impl->internal_state.style[_slot].non_solid.adjusted_transform = transform;
  }

  BL_INLINE void mark_as_nop() noexcept {
    _style_flags |= ContextFlags::kNoBaseStyle;
  }

  BL_INLINE bool finalize(BLRasterContextImpl* ctx_impl) noexcept {
    ctx_impl->context_flags = _context_flags | (_style_flags << uint32_t(_slot));
    return true;
  }
};

class NonSolidFetchExplicitApplier {
public:
  static constexpr bool kIsExplicit = true;

  BL_INLINE_NODEBUG NonSolidFetchExplicitApplier() noexcept {}

  BL_INLINE void init_style_type(BLRasterContextImpl* ctx_impl, BLObjectType style_type) noexcept {
    bl_unused(ctx_impl, style_type);
  }

  BL_INLINE void init_computed_transform(BLRasterContextImpl* ctx_impl, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {
    bl_unused(ctx_impl, transform, transform_type);
  }

  BL_INLINE void mark_as_nop() noexcept {
  }

  BL_INLINE bool finalize(BLRasterContextImpl* ctx_impl) noexcept {
    bl_unused(ctx_impl);
    return true;
  }
};

template<typename Applier>
static BL_INLINE bool init_non_solid_fetch_data(
    BLRasterContextImpl* ctx_impl,
    RenderFetchData* fetch_data,
    const BLObjectCore* style, BLObjectType style_type, BLContextStyleTransformMode transform_mode, Applier& applier) noexcept {

  const BLMatrix2D* transform = ctx_impl->transform_ptrs[transform_mode];
  BLTransformType transform_type = BLTransformType(ctx_impl->internal_state.transform_types[transform_mode]);
  BLMatrix2D transform_storage;

  applier.init_style_type(ctx_impl, style_type);
  Pipeline::Signature pending_bit{0};

  switch (style_type) {
    case BL_OBJECT_TYPE_PATTERN: {
      const BLPattern* pattern = static_cast<const BLPattern*>(style);
      BLPatternImpl* pattern_impl = PatternInternal::get_impl(pattern);
      BLImageCore* image = &pattern_impl->image;

      if constexpr (Applier::kIsExplicit) {
        // Reinitialize this style to use the image instead of the pattern if this is an explicit operation.
        // The reason is that we don't need the BLPattern data once the FetchData is initialized, so if the
        // user reinitializes the pattern for multiple calls we would save one memory allocation each time
        // the pattern is reinitialized.
        fetch_data->init_style_object(image);
        fetch_data->init_destroy_func(destroy_fetch_data_image);
      }
      else {
        fetch_data->init_destroy_func(recycle_fetch_data_pattern);
      }

      // NOTE: The area comes from pattern, it means that it's the pattern's responsibility to make sure that it's valid.
      BLRectI area = pattern_impl->area;

      if (!area.w || !area.h) {
        applier.mark_as_nop();
        if constexpr (Applier::kIsExplicit)
          return false;
      }

      BLTransformType style_transform_type = pattern->transform_type();
      if (style_transform_type != BL_TRANSFORM_TYPE_IDENTITY) {
        TransformInternal::multiply(transform_storage, pattern_impl->transform, *transform);
        transform = &transform_storage;
        style_transform_type = transform_storage.type();
      }
      applier.init_computed_transform(ctx_impl, *transform, transform_type);

      BLPatternQuality quality = BLPatternQuality(ctx_impl->hints().pattern_quality);
      BLExtendMode extend_mode = PatternInternal::get_extend_mode(pattern);
      BLImageImpl* image_impl = ImageInternal::get_impl(image);

      fetch_data->extra.format = uint8_t(image_impl->format);
      fetch_data->init_image_source(image_impl, area);

      fetch_data->signature = Pipeline::FetchUtils::init_pattern_affine(
        fetch_data->pipeline_data.pattern, extend_mode, quality, uint32_t(image_impl->depth) / 8u, *transform);
      break;
    }

    case BL_OBJECT_TYPE_GRADIENT: {
      const BLGradient* gradient = static_cast<const BLGradient*>(style);
      BLGradientPrivateImpl* gradient_impl = GradientInternal::get_impl(gradient);

      fetch_data->init_style_object(gradient);
      if constexpr (Applier::kIsExplicit)
        fetch_data->init_destroy_func(destroy_fetch_data_gradient);
      else
        fetch_data->init_destroy_func(recycle_fetch_data_gradient);

      BLTransformType style_transform_type = gradient->transform_type();
      if (style_transform_type != BL_TRANSFORM_TYPE_IDENTITY) {
        TransformInternal::multiply(transform_storage, gradient_impl->transform, *transform);
        transform = &transform_storage;
        style_transform_type = transform_storage.type();
      }
      applier.init_computed_transform(ctx_impl, *transform, transform_type);

      BLGradientInfo gradient_info = GradientInternal::ensure_info(gradient_impl);
      fetch_data->extra.format = uint8_t(gradient_info.format);

      if (gradient_info.is_empty()) {
        applier.mark_as_nop();
        if constexpr (Applier::kIsExplicit)
          return false;
      }
      else if (gradient_info.solid) {
        // Using last color according to the SVG specification.
        uint32_t rgba32 = PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888(RgbaInternal::rgba32FromRgba64(gradient_impl->stops[gradient_impl->size - 1].rgba.value));
        fetch_data->pipeline_data.solid.prgb32 = rgba32;
      }
      else {
        BLGradientType type = GradientInternal::get_gradient_type(gradient);
        BLGradientQuality quality = BLGradientQuality(ctx_impl->hints().gradient_quality);
        BLExtendMode extend_mode = GradientInternal::get_extend_mode(gradient);

        // Do not dither gradients when rendering into A8 targets.
        if (ctx_impl->sync_work_data.ctx_data.dst.format == BL_FORMAT_A8)
          quality = BL_GRADIENT_QUALITY_NEAREST;

        const void* lut_data = nullptr;
        uint32_t lut_size = gradient_info.lut_size(quality >= BL_GRADIENT_QUALITY_DITHER);

        BLGradientLUT* lut = gradient_impl->lut[size_t(quality >= BL_GRADIENT_QUALITY_DITHER)];
        if (lut)
          lut_data = lut->data();

        // We have to store the quality somewhere as if this FetchData would be lazily materialized we have to
        // cache the desired quality and the size of the LUT calculated (to avoid going over GradientInfo again).
        fetch_data->extra.custom[0] = uint8_t(quality);
        pending_bit = Pipeline::Signature::from_pending_flag(!lut);

        fetch_data->signature = Pipeline::FetchUtils::init_gradient(
          fetch_data->pipeline_data.gradient, type, extend_mode, quality, gradient_impl->values, lut_data, lut_size, *transform);
      }
      break;
    }

    default:
      // The caller must ensure that this is not a solid case and the style type is valid.
      BL_NOT_REACHED();
  }

  if (fetch_data->signature.has_pending_flag()) {
    applier.mark_as_nop();
    if constexpr (Applier::kIsExplicit)
      return false;
  }

  fetch_data->signature |= pending_bit;
  return applier.finalize(ctx_impl);
}

// bl::RasterEngine - ContextImpl - Frontend - Fill & Stroke Style
// ===============================================================

static BL_INLINE_NODEBUG uint32_t restricted_index_from_slot(BLContextStyleSlot slot) noexcept {
  return bl_min<uint32_t>(slot, uint32_t(BL_CONTEXT_STYLE_SLOT_MAX_VALUE) + 1);
}

static BLResult BL_CDECL get_style_impl(const BLContextImpl* base_impl, BLContextStyleSlot slot, bool transformed, BLVarCore* var_out) noexcept {
  const BLRasterContextImpl* ctx_impl = static_cast<const BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(slot > BL_CONTEXT_STYLE_SLOT_MAX_VALUE)) {
    bl_var_assign_null(var_out);
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  BLObjectType style_type = BLObjectType(ctx_impl->internal_state.style_type[slot]);
  const StyleData* style = &ctx_impl->internal_state.style[slot];

  if (style_type <= BL_OBJECT_TYPE_NULL) {
    if (style_type == BL_OBJECT_TYPE_RGBA32)
      return bl_var_assign_rgba32(var_out, style->solid.original.rgba32.value);

    if (style_type == BL_OBJECT_TYPE_RGBA64)
      return bl_var_assign_rgba64(var_out, style->solid.original.rgba64.value);

    if (style_type == BL_OBJECT_TYPE_RGBA)
      return bl_var_assign_rgba(var_out, &style->solid.original.rgba);

    return bl_var_assign_null(var_out);
  }

  const RenderFetchData* fetch_data = style->get_render_fetch_data();
  bl_var_assign_weak(var_out, &fetch_data->style_as<BLVarCore>());

  if (!transformed)
    return BL_SUCCESS;

  switch (style_type) {
    case BL_OBJECT_TYPE_PATTERN:
      return var_out->dcast().as<BLPattern>().set_transform(style->non_solid.adjusted_transform);

    case BL_OBJECT_TYPE_GRADIENT:
      return var_out->dcast().as<BLGradient>().set_transform(style->non_solid.adjusted_transform);

    default:
      return bl_make_error(BL_ERROR_INVALID_STATE);
  }
}

static BLResult BL_CDECL disable_style_impl(BLContextImpl* base_impl, BLContextStyleSlot slot) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  ContextFlags context_flags = ctx_impl->context_flags;
  ContextFlags style_flags = (ContextFlags::kWeakStateBaseStyle | ContextFlags::kFetchDataBase) << restricted_index_from_slot(slot);

  if (bl_test_flag(context_flags, style_flags)) {
    if (BL_UNLIKELY(slot > BL_CONTEXT_STYLE_SLOT_MAX_VALUE))
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    on_before_style_change(ctx_impl, slot, ctx_impl->internal_state.style[slot], context_flags);
  }

  ctx_impl->context_flags = (context_flags & ~style_flags) | ContextFlags::kNoBaseStyle << slot;
  ctx_impl->internal_state.style_type[slot] = uint8_t(BL_OBJECT_TYPE_NULL);
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_style_rgba32_impl(BLContextImpl* base_impl, BLContextStyleSlot slot, uint32_t rgba32) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  ContextFlags context_flags = ctx_impl->context_flags;
  ContextFlags style_flags = (ContextFlags::kWeakStateBaseStyle | ContextFlags::kFetchDataBase) << restricted_index_from_slot(slot);

  if (bl_test_flag(context_flags, style_flags)) {
    if (BL_UNLIKELY(slot > BL_CONTEXT_STYLE_SLOT_MAX_VALUE))
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    on_before_style_change(ctx_impl, slot, ctx_impl->internal_state.style[slot], context_flags);
  }

  StyleData& style = ctx_impl->internal_state.style[slot];
  style.solid.original.rgba32.value = rgba32;

  uint32_t premultiplied = PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888(rgba32);
  FormatExt format = formatFromRgba32(rgba32);

  ctx_impl->context_flags = context_flags & ~(style_flags | (ContextFlags::kNoBaseStyle << slot));
  ctx_impl->internal_state.style_type[slot] = uint8_t(BL_OBJECT_TYPE_RGBA32);

  style.solid.init_header(0, format);
  style.solid.pipeline_data.prgb32 = premultiplied;
  style.make_fetch_data_implicit();

  return BL_SUCCESS;
}

static BLResult BL_CDECL set_style_rgba64_impl(BLContextImpl* base_impl, BLContextStyleSlot slot, uint64_t rgba64) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  ContextFlags context_flags = ctx_impl->context_flags;
  ContextFlags style_flags = (ContextFlags::kWeakStateBaseStyle | ContextFlags::kFetchDataBase) << restricted_index_from_slot(slot);

  if (bl_test_flag(context_flags, style_flags)) {
    if (BL_UNLIKELY(slot > BL_CONTEXT_STYLE_SLOT_MAX_VALUE))
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    on_before_style_change(ctx_impl, slot, ctx_impl->internal_state.style[slot], context_flags);
  }

  StyleData& style = ctx_impl->internal_state.style[slot];
  style.solid.original.rgba64.value = rgba64;

  uint32_t rgba32 = RgbaInternal::rgba32FromRgba64(rgba64);
  uint32_t premultiplied = PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888(rgba32);
  FormatExt format = formatFromRgba32(rgba32);

  ctx_impl->context_flags = context_flags & ~(style_flags | (ContextFlags::kNoBaseStyle << slot));
  ctx_impl->internal_state.style_type[slot] = uint8_t(BL_OBJECT_TYPE_RGBA64);

  style.solid.init_header(0, format);
  style.solid.pipeline_data.prgb32 = premultiplied;
  style.make_fetch_data_implicit();

  return BL_SUCCESS;
}

static BLResult BL_CDECL set_style_rgba_impl(BLContextImpl* base_impl, BLContextStyleSlot slot, const BLRgba* rgba) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  ContextFlags context_flags = ctx_impl->context_flags;
  ContextFlags style_flags = (ContextFlags::kWeakStateBaseStyle | ContextFlags::kFetchDataBase) << restricted_index_from_slot(slot);

  BLRgba norm = bl_clamp(*rgba, BLRgba(0.0f, 0.0f, 0.0f, 0.0f), BLRgba(1.0f, 1.0f, 1.0f, 1.0f));
  if (!RgbaInternal::is_valid(*rgba))
    return disable_style_impl(base_impl, slot);

  if (bl_test_flag(context_flags, style_flags)) {
    if (BL_UNLIKELY(slot > BL_CONTEXT_STYLE_SLOT_MAX_VALUE))
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    on_before_style_change(ctx_impl, slot, ctx_impl->internal_state.style[slot], context_flags);
  }

  StyleData& style = ctx_impl->internal_state.style[slot];
  style.solid.original.rgba = norm;

  // Premultiply and convert to RGBA32.
  float a_scale = norm.a * 255.0f;
  uint32_t r = uint32_t(Math::round_to_int(norm.r * a_scale));
  uint32_t g = uint32_t(Math::round_to_int(norm.g * a_scale));
  uint32_t b = uint32_t(Math::round_to_int(norm.b * a_scale));
  uint32_t a = uint32_t(Math::round_to_int(a_scale));
  uint32_t premultiplied = BLRgba32(r, g, b, a).value;
  FormatExt format = formatFromRgba32(premultiplied);

  ctx_impl->context_flags = context_flags & ~(style_flags | (ContextFlags::kNoBaseStyle << slot));
  ctx_impl->internal_state.style_type[slot] = uint8_t(BL_OBJECT_TYPE_RGBA);

  style.solid.init_header(0, format);
  style.solid.pipeline_data.prgb32 = premultiplied;
  style.make_fetch_data_implicit();

  return BL_SUCCESS;
}

static BLResult BL_CDECL set_style_impl(BLContextImpl* base_impl, BLContextStyleSlot slot, const BLObjectCore* style, BLContextStyleTransformMode transform_mode) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLObjectType style_type = style->_d.get_type();

  if (style_type <= BL_OBJECT_TYPE_NULL) {
    if (style_type == BL_OBJECT_TYPE_RGBA32)
      return set_style_rgba32_impl(base_impl, slot, style->_d.rgba32.value);

    if (style_type == BL_OBJECT_TYPE_RGBA64)
      return set_style_rgba64_impl(base_impl, slot, style->_d.rgba64.value);

    if (style_type == BL_OBJECT_TYPE_RGBA)
      return set_style_rgba_impl(base_impl, slot, &style->_d.rgba);

    return disable_style_impl(ctx_impl, slot);
  }

  if (BL_UNLIKELY(slot > BL_CONTEXT_STYLE_SLOT_MAX_VALUE || style_type > BL_OBJECT_TYPE_MAX_STYLE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  ContextFlags context_flags = ctx_impl->context_flags;
  ContextFlags style_flags = (ContextFlags::kFetchDataBase | ContextFlags::kWeakStateBaseStyle) << slot;
  StyleData& style_state = ctx_impl->internal_state.style[slot];

  RenderFetchData* fetch_data = ctx_impl->alloc_fetch_data();
  if (BL_UNLIKELY(!fetch_data))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  if (bl_test_flag(context_flags, style_flags))
    on_before_style_change(ctx_impl, slot, style_state, context_flags);

  fetch_data->init_header(1);
  fetch_data->init_style_object(style);
  ObjectInternal::retain_instance(style);

  style_state.fetch_data = fetch_data;
  context_flags &= ~(style_flags | (ContextFlags::kNoBaseStyle << slot));

  NonSolidFetchStateApplier applier(context_flags, slot);
  init_non_solid_fetch_data(ctx_impl, fetch_data, style, style_type, transform_mode, applier);

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Internals - Stroke State
// =========================================================

static BL_INLINE void on_before_stroke_change(BLRasterContextImpl* ctx_impl) noexcept {
  if (bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateStrokeOptions)) {
    SavedState* state = ctx_impl->saved_state;
    state->stroke_options._copy_from(ctx_impl->stroke_options());
    ArrayInternal::retain_instance(&state->stroke_options.dash_array);
  }
}

static BL_INLINE void on_before_stroke_change_and_destroy_dash_array(BLRasterContextImpl* ctx_impl) noexcept {
  if (bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateStrokeOptions)) {
    SavedState* state = ctx_impl->saved_state;
    state->stroke_options._copy_from(ctx_impl->stroke_options());
  }
  else {
    ArrayInternal::release_instance(&ctx_impl->internal_state.stroke_options.dash_array);
  }
}

// bl::RasterEngine - ContextImpl - Internals - Transform State
// ============================================================

// Called before `user_transform` is changed.
//
// This function is responsible for saving the current user_transform in case that the
// `ContextFlags::kWeakStateUserTransform` flag is set, which means that the user_transform
// must be saved before any modification.
static BL_INLINE void on_before_user_transform_change(BLRasterContextImpl* ctx_impl, Matrix2x2& before2x2) noexcept {
  before2x2.m[0] = ctx_impl->final_transform().m00;
  before2x2.m[1] = ctx_impl->final_transform().m01;
  before2x2.m[2] = ctx_impl->final_transform().m10;
  before2x2.m[3] = ctx_impl->final_transform().m11;

  if (bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateUserTransform)) {
    // Weak MetaTransform state must be set together with weak UserTransform.
    BL_ASSERT(bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateMetaTransform));

    SavedState* state = ctx_impl->saved_state;
    state->alt_transform = ctx_impl->final_transform();
    state->user_transform = ctx_impl->user_transform();
  }
}

static BL_INLINE void update_final_transform(BLRasterContextImpl* ctx_impl) noexcept {
  TransformInternal::multiply(ctx_impl->internal_state.final_transform, ctx_impl->user_transform(), ctx_impl->meta_transform());
}

static BL_INLINE void update_meta_transform_fixed(BLRasterContextImpl* ctx_impl) noexcept {
  ctx_impl->internal_state.meta_transform_fixed = ctx_impl->meta_transform();
  ctx_impl->internal_state.meta_transform_fixed.post_scale(ctx_impl->render_target_info.fp_scale_d);
}

static BL_INLINE void update_final_transform_fixed(BLRasterContextImpl* ctx_impl) noexcept {
  ctx_impl->internal_state.final_transform_fixed = ctx_impl->final_transform();
  ctx_impl->internal_state.final_transform_fixed.post_scale(ctx_impl->render_target_info.fp_scale_d);
}

// Called after `user_transform` has been modified.
//
// Responsible for updating `final_transform` and other matrix information.
static BL_INLINE void on_after_user_transform_changed(BLRasterContextImpl* ctx_impl, const Matrix2x2& before2x2) noexcept {
  ContextFlags context_flags = ctx_impl->context_flags;

  context_flags &= ~(ContextFlags::kNoUserTransform         |
                    ContextFlags::kInfoIntegralTranslation |
                    ContextFlags::kWeakStateUserTransform  );

  update_final_transform(ctx_impl);
  update_final_transform_fixed(ctx_impl);

  const BLMatrix2D& ft = ctx_impl->final_transform_fixed();
  BLTransformType final_transform_type = ctx_impl->final_transform().type();

  ctx_impl->internal_state.final_transform_type = uint8_t(final_transform_type);
  ctx_impl->internal_state.final_transform_fixed_type = uint8_t(bl_max<uint32_t>(final_transform_type, BL_TRANSFORM_TYPE_SCALE));

  if (final_transform_type <= BL_TRANSFORM_TYPE_TRANSLATE) {
    // No scaling - input coordinates have pixel granularity. Check if the translation has pixel granularity as well
    // and setup the `translation_i` data for that case.
    if (ft.m20 >= ctx_impl->fp_min_safe_coord_d && ft.m20 <= ctx_impl->fp_max_safe_coord_d &&
        ft.m21 >= ctx_impl->fp_min_safe_coord_d && ft.m21 <= ctx_impl->fp_max_safe_coord_d) {
      // We need 64-bit ints here as we are already scaled. We also need a `floor` function as we have to handle
      // negative translations which cannot be truncated (the default conversion).
      int64_t tx64 = Math::floor_to_int64(ft.m20);
      int64_t ty64 = Math::floor_to_int64(ft.m21);

      // Pixel to pixel translation is only possible when both fixed points `tx64` and `ty64` have all zeros in
      // their fraction parts.
      if (((tx64 | ty64) & ctx_impl->render_target_info.fpMaskI) == 0) {
        int tx = int(tx64 >> ctx_impl->render_target_info.fpShiftI);
        int ty = int(ty64 >> ctx_impl->render_target_info.fpShiftI);

        ctx_impl->set_translation_i(BLPointI(tx, ty));
        context_flags |= ContextFlags::kInfoIntegralTranslation;
      }
    }
  }

  // Shared states are not invalidated when the transformation is just translated.
  uint32_t invalidate_shared_state = uint32_t(before2x2.m[0] != ctx_impl->final_transform().m00) |
                                   uint32_t(before2x2.m[1] != ctx_impl->final_transform().m01) |
                                   uint32_t(before2x2.m[2] != ctx_impl->final_transform().m10) |
                                   uint32_t(before2x2.m[3] != ctx_impl->final_transform().m11);

  // Mark NoUserTransform in case that the transformation matrix is invalid.
  if (final_transform_type >= BL_TRANSFORM_TYPE_INVALID)
    context_flags |= ContextFlags::kNoUserTransform;

  // Clear shared state flags if the shared state has been invalidated by the transformation.
  if (invalidate_shared_state)
    context_flags &= ~(ContextFlags::kSharedStateFill | ContextFlags::kSharedStateStrokeExt);

  ctx_impl->context_flags = context_flags;
}

// bl::RasterEngine - ContextImpl - Internals - Clip State
// =======================================================

static BL_INLINE void on_before_clip_box_change(BLRasterContextImpl* ctx_impl) noexcept {
  if (bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateClip)) {
    SavedState* state = ctx_impl->saved_state;
    state->final_clip_box_d = ctx_impl->final_clip_box_d();
  }
}

static BL_INLINE void reset_clipping_to_meta_clip_box(BLRasterContextImpl* ctx_impl) noexcept {
  const BLBoxI& meta = ctx_impl->meta_clip_box_i();
  ctx_impl->internal_state.final_clip_box_i.reset(meta.x0, meta.y0, meta.x1, meta.y1);
  ctx_impl->internal_state.final_clip_box_d.reset(meta.x0, meta.y0, meta.x1, meta.y1);
  ctx_impl->set_final_clip_box_fixed_d(ctx_impl->final_clip_box_d() * ctx_impl->render_target_info.fp_scale_d);
}

static BL_INLINE void restore_clipping_from_state(BLRasterContextImpl* ctx_impl, SavedState* saved_state) noexcept {
  // TODO: [Rendering Context] Path-based clipping.
  ctx_impl->internal_state.final_clip_box_d = saved_state->final_clip_box_d;
  ctx_impl->internal_state.final_clip_box_i.reset(
    Math::trunc_to_int(ctx_impl->final_clip_box_d().x0),
    Math::trunc_to_int(ctx_impl->final_clip_box_d().y0),
    Math::ceil_to_int(ctx_impl->final_clip_box_d().x1),
    Math::ceil_to_int(ctx_impl->final_clip_box_d().y1));

  double fp_scale = ctx_impl->render_target_info.fp_scale_d;
  ctx_impl->set_final_clip_box_fixed_d(BLBox(
    ctx_impl->final_clip_box_d().x0 * fp_scale,
    ctx_impl->final_clip_box_d().y0 * fp_scale,
    ctx_impl->final_clip_box_d().x1 * fp_scale,
    ctx_impl->final_clip_box_d().y1 * fp_scale));
}

// bl::RasterEngine - ContextImpl - Internals - Clip Utilities
// ===========================================================

#if 0
// TODO: [Rendering Context] Experiment, not ready yet.
static BL_INLINE bool translateAndClipRectToFillI(const BLRasterContextImpl* ctx_impl, const BLRectI* src_rect, BLBoxI* dst_box_out) noexcept {
  __m128d x0y0 = _mm_cvtepi32_pd(_mm_loadu_si64(&src_rect->x));
  __m128d wh = _mm_cvtepi32_pd(_mm_loadu_si64(&src_rect->w));

  x0y0 = _mm_add_pd(x0y0, *(__m128d*)&ctx_impl->internal_state.final_transform.m20);
  __m128d x1y1 = _mm_add_pd(x0y0, wh);

  x0y0 = _mm_max_pd(x0y0, *(__m128d*)&ctx_impl->internal_state.final_clip_box_d.x0);
  x1y1 = _mm_min_pd(x1y1, *(__m128d*)&ctx_impl->internal_state.final_clip_box_d.x1);

  __m128i a = _mm_cvtpd_epi32(x0y0);
  __m128i b = _mm_cvtpd_epi32(x1y1);
  __m128d msk = _mm_cmpge_pd(x0y0, x1y1);
  _mm_storeu_si128((__m128i*)dst_box_out, _mm_unpacklo_epi64(a, b));

  return _mm_movemask_pd(msk) == 0;
}
#else
static BL_INLINE bool translateAndClipRectToFillI(const BLRasterContextImpl* ctx_impl, const BLRectI* src_rect, BLBoxI* dst_box_out) noexcept {
  int rx = src_rect->x;
  int ry = src_rect->y;
  int rw = src_rect->w;
  int rh = src_rect->h;

  if (BL_TARGET_ARCH_BITS < 64) {
    OverflowFlag of{};

    int x0 = IntOps::add_overflow(rx, ctx_impl->translation_i().x, &of);
    int y0 = IntOps::add_overflow(ry, ctx_impl->translation_i().y, &of);
    int x1 = IntOps::add_overflow(rw, x0, &of);
    int y1 = IntOps::add_overflow(rh, y0, &of);

    if (BL_UNLIKELY(of))
      goto Use64Bit;

    x0 = bl_max(x0, ctx_impl->final_clip_box_i().x0);
    y0 = bl_max(y0, ctx_impl->final_clip_box_i().y0);
    x1 = bl_min(x1, ctx_impl->final_clip_box_i().x1);
    y1 = bl_min(y1, ctx_impl->final_clip_box_i().y1);

    // Clipped out or invalid rect.
    if (BL_UNLIKELY((x0 >= x1) | (y0 >= y1)))
      return false;

    dst_box_out->reset(int(x0), int(y0), int(x1), int(y1));
    return true;
  }
  else {
Use64Bit:
    int64_t x0 = int64_t(rx) + int64_t(ctx_impl->translation_i().x);
    int64_t y0 = int64_t(ry) + int64_t(ctx_impl->translation_i().y);
    int64_t x1 = int64_t(rw) + x0;
    int64_t y1 = int64_t(rh) + y0;

    x0 = bl_max<int64_t>(x0, ctx_impl->final_clip_box_i().x0);
    y0 = bl_max<int64_t>(y0, ctx_impl->final_clip_box_i().y0);
    x1 = bl_min<int64_t>(x1, ctx_impl->final_clip_box_i().x1);
    y1 = bl_min<int64_t>(y1, ctx_impl->final_clip_box_i().y1);

    // Clipped out or invalid rect.
    if (BL_UNLIKELY((x0 >= x1) | (y0 >= y1)))
      return false;

    dst_box_out->reset(int(x0), int(y0), int(x1), int(y1));
    return true;
  }
}
#endif

static BL_INLINE bool translate_and_clip_rect_to_blit_i(const BLRasterContextImpl* ctx_impl, const BLPointI* origin, const BLRectI* area, const BLSizeI* sz, BLResult* result_out, BLBoxI* dst_box_out, BLPointI* src_offset_out) noexcept {
  BLSizeI size(sz->w, sz->h);
  src_offset_out->reset();

  if (area) {
    unsigned max_w = unsigned(size.w) - unsigned(area->x);
    unsigned max_h = unsigned(size.h) - unsigned(area->y);

    if (BL_UNLIKELY((max_w > unsigned(size.w)) | (unsigned(area->w) > max_w) |
                    (max_h > unsigned(size.h)) | (unsigned(area->h) > max_h))) {
      *result_out = bl_make_error(BL_ERROR_INVALID_VALUE);
      return false;
    }

    src_offset_out->reset(area->x, area->y);
    size.reset(area->w, area->h);
  }

  *result_out = BL_SUCCESS;
  if (BL_TARGET_ARCH_BITS < 64) {
    OverflowFlag of{};

    int dx = IntOps::add_overflow(origin->x, ctx_impl->translation_i().x, &of);
    int dy = IntOps::add_overflow(origin->y, ctx_impl->translation_i().y, &of);

    int x0 = dx;
    int y0 = dy;
    int x1 = IntOps::add_overflow(x0, size.w, &of);
    int y1 = IntOps::add_overflow(y0, size.h, &of);

    if (BL_UNLIKELY(of))
      goto Use64Bit;

    x0 = bl_max(x0, ctx_impl->final_clip_box_i().x0);
    y0 = bl_max(y0, ctx_impl->final_clip_box_i().y0);
    x1 = bl_min(x1, ctx_impl->final_clip_box_i().x1);
    y1 = bl_min(y1, ctx_impl->final_clip_box_i().y1);

    // Clipped out.
    if ((x0 >= x1) | (y0 >= y1))
      return false;

    dst_box_out->reset(x0, y0, x1, y1);
    src_offset_out->x += x0 - dx;
    src_offset_out->y += y0 - dy;
    return true;
  }
  else {
Use64Bit:
    int64_t dx = int64_t(origin->x) + ctx_impl->translation_i().x;
    int64_t dy = int64_t(origin->y) + ctx_impl->translation_i().y;

    int64_t x0 = dx;
    int64_t y0 = dy;
    int64_t x1 = x0 + int64_t(unsigned(size.w));
    int64_t y1 = y0 + int64_t(unsigned(size.h));

    x0 = bl_max<int64_t>(x0, ctx_impl->final_clip_box_i().x0);
    y0 = bl_max<int64_t>(y0, ctx_impl->final_clip_box_i().y0);
    x1 = bl_min<int64_t>(x1, ctx_impl->final_clip_box_i().x1);
    y1 = bl_min<int64_t>(y1, ctx_impl->final_clip_box_i().y1);

    // Clipped out.
    if ((x0 >= x1) | (y0 >= y1))
      return false;

    dst_box_out->reset(int(x0), int(y0), int(x1), int(y1));
    src_offset_out->x += int(x0 - dx);
    src_offset_out->y += int(y0 - dy);
    return true;
  }
}

// bl::RasterEngine - ContextImpl - Internals - Async - Render Batch
// =================================================================

static BL_INLINE void release_batch_fetch_data(BLRasterContextImpl* ctx_impl, RenderCommandQueue* queue) noexcept {
  while (queue) {
    RenderCommand* command_data = queue->_data;
    for (size_t i = 0; i < queue->_fetch_data_marks.size_in_words(); i++) {
      BLBitWord bits = queue->_fetch_data_marks.data[i];
      ParametrizedBitOps<BitOrder::kLSB, BLBitWord>::BitIterator it(bits);

      while (it.has_next()) {
        size_t bit_index = it.next();
        RenderCommand& command = command_data[bit_index];

        if (command.retains_style_fetch_data())
          command._source.fetch_data->release(ctx_impl);

        if (command.retains_mask_image_data())
          ImageInternal::release_impl<RCMode::kMaybe>(command._payload.box_mask_a.mask_image_i.ptr);
      }
      command_data += IntOps::bit_size_of<BLBitWord>();
    }
    queue = queue->next();
  }
}

static BL_NOINLINE BLResult flush_render_batch(BLRasterContextImpl* ctx_impl) noexcept {
  WorkerManager& mgr = ctx_impl->worker_mgr();
  if (mgr.has_pending_commands()) {
    mgr.finalize_batch();

    WorkerSynchronization* synchronization = &mgr._synchronization;
    RenderBatch* batch = mgr.current_batch();
    uint32_t thread_count = mgr.thread_count();

    for (uint32_t i = 0; i < thread_count; i++) {
      WorkData* work_data = mgr._work_data_storage[i];
      work_data->init_batch(batch);
      work_data->init_context_data(ctx_impl->dst_data, ctx_impl->sync_work_data.ctx_data.pixel_origin);
    }

    // Just to make sure that all the changes are visible to the threads.
    synchronization->before_start(thread_count, batch->job_count() > 0);

    for (uint32_t i = 0; i < thread_count; i++) {
      mgr._worker_threads[i]->run(WorkerProc::worker_thread_entry, mgr._work_data_storage[i]);
    }

    // User thread acts as a worker too.
    {
      synchronization->thread_started();

      WorkData* work_data = &ctx_impl->sync_work_data;
      SyncWorkState work_state;

      work_state.save(*work_data);
      WorkerProc::process_work_data(work_data, batch);
      work_state.restore(*work_data);
    }

    if (thread_count) {
      synchronization->wait_for_threads_to_finish();
      ctx_impl->sync_work_data._accumulated_error_flags |= bl_atomic_fetch_relaxed(&batch->_accumulated_error_flags);
    }

    release_batch_fetch_data(ctx_impl, batch->_command_list.first());

    mgr._allocator.clear();
    mgr.init_first_batch();

    ctx_impl->sync_work_data.start_over();
    ctx_impl->context_flags &= ~ContextFlags::kSharedStateAllFlags;
    ctx_impl->shared_fill_state = nullptr;
    ctx_impl->shared_stroke_state = nullptr;
  }

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Internals - Render Call - Data Allocation
// ==========================================================================

static BL_INLINE void mark_queue_full_or_exhausted(BLRasterContextImpl* ctx_impl, bool flag) noexcept {
  constexpr uint32_t kMTFullOrExhaustedShift = IntOps::bit_shift_of(ContextFlags::kMTFullOrExhausted);
  ctx_impl->context_flags |= ContextFlags(uint32_t(flag) << kMTFullOrExhaustedShift);
}

template<RenderingMode kRM>
struct RenderFetchDataStorage;

template<>
struct RenderFetchDataStorage<kSync> {
  RenderFetchData _fetch_data;

  BL_INLINE_NODEBUG RenderFetchDataStorage() noexcept {}
  BL_INLINE_NODEBUG RenderFetchDataStorage(BLRasterContextImpl* ctx_impl) noexcept { init(ctx_impl); }

  BL_INLINE_NODEBUG void init(BLRasterContextImpl* ctx_impl) noexcept { bl_unused(ctx_impl); }

  BL_INLINE_NODEBUG RenderFetchData* ptr() noexcept { return &_fetch_data; }
  BL_INLINE_NODEBUG RenderFetchData* operator->() noexcept { return &_fetch_data; }
};

template<>
struct RenderFetchDataStorage<kAsync> {
  RenderFetchData* _fetch_data;

  BL_INLINE_NODEBUG RenderFetchDataStorage() noexcept {}
  BL_INLINE_NODEBUG RenderFetchDataStorage(BLRasterContextImpl* ctx_impl) noexcept {
    init(ctx_impl);
  }

  BL_INLINE_NODEBUG void init(BLRasterContextImpl* ctx_impl) noexcept {
    _fetch_data = ctx_impl->worker_mgr()._fetch_data_pool.ptr;
    _fetch_data->init_header(0);
  }

  BL_INLINE_NODEBUG RenderFetchData* ptr() noexcept { return _fetch_data; }
  BL_INLINE_NODEBUG RenderFetchData* operator->() noexcept { return _fetch_data; }
};

static BL_INLINE void advance_fetch_ptr(BLRasterContextImpl* ctx_impl) noexcept {
  ctx_impl->worker_mgr()._fetch_data_pool.advance();
  mark_queue_full_or_exhausted(ctx_impl, ctx_impl->worker_mgr()._fetch_data_pool.exhausted());
}

// bl::RasterEngine - ContextImpl - Internals - Render Call - Fetch And Dispatch Data
// ==================================================================================

// Slow path - if the pipeline is not in cache there is also a chance that FetchData has not been setup yet.
// In that case it would have PendingFlag set to 1, which would indicate the pending setup.
static BL_NOINLINE BLResult ensure_fetch_and_dispatch_data_slow(
    BLRasterContextImpl* ctx_impl,
    Pipeline::Signature signature, RenderFetchDataHeader* fetch_data, Pipeline::DispatchData* out) noexcept {

  if (signature.has_pending_flag()) {
    BL_PROPAGATE(compute_pending_fetch_data(static_cast<RenderFetchData*>(fetch_data)));

    signature.clear_pending_bit();
    auto m = Pipeline::cache_lookup(ctx_impl->pipe_lookup_cache, signature.value);

    if (m.matched()) {
      *out = ctx_impl->pipe_lookup_cache.dispatch_data(m.index());
      return BL_SUCCESS;
    }
  }

  return ctx_impl->pipe_provider.get(signature.value, out, &ctx_impl->pipe_lookup_cache);
}

// Fast path - if the signature is in the cache, which means that we have the dispatch data and that FetchData
// doesn't need initialization (it's either SOLID or it has been already initialized in case that it was used
// multiple times or this render call is a blit).
static BL_INLINE BLResult ensure_fetch_and_dispatch_data(
    BLRasterContextImpl* ctx_impl,
    Pipeline::Signature signature, RenderFetchDataHeader* fetch_data, Pipeline::DispatchData* out) noexcept {

  // Must be inlined for greater performance.
  auto m = Pipeline::cache_lookup(ctx_impl->pipe_lookup_cache, signature.value);

  // Likely if there is not a lot of diverse render commands.
  if (BL_LIKELY(m.matched())) {
    *out = ctx_impl->pipe_lookup_cache.dispatch_data(m.index());
    return BL_SUCCESS;
  }

  return ensure_fetch_and_dispatch_data_slow(ctx_impl, signature, fetch_data, out);
}

// bl::RasterEngine - ContextImpl - Internals - Render Call - Queues and Pools
// ===========================================================================

// This function is called when a command/job queue is full or when pool(s) get exhausted.
//
// The purpose of this function is to make sure that ALL queues are not full and that no pools
// are exhausted, because the dispatching relies on the availability of these resources.
static BL_NOINLINE BLResult handle_queues_full_or_pools_exhausted(BLRasterContextImpl* ctx_impl) noexcept {
  // Should only be called when we know at that least one queue / buffer needs refill.
  BL_ASSERT(bl_test_flag(ctx_impl->context_flags, ContextFlags::kMTFullOrExhausted));

  WorkerManager& mgr = ctx_impl->worker_mgr();

  if (mgr.is_command_queue_full()) {
    mgr.before_grow_command_queue();
    if (mgr.is_batch_full()) {
      BL_PROPAGATE(flush_render_batch(ctx_impl));
      // NOTE: After a successful flush the queues and pools should already be allocated.
      ctx_impl->context_flags &= ~ContextFlags::kMTFullOrExhausted;
      return BL_SUCCESS;
    }

    BL_PROPAGATE(mgr._grow_command_queue());
  }

  if (mgr.is_job_queue_full()) {
    BL_PROPAGATE(mgr._grow_job_queue());
  }

  if (mgr.is_fetch_data_pool_exhausted()) {
    BL_PROPAGATE(mgr._preallocate_fetch_data_pool());
  }

  if (mgr.is_shared_data_pool_exhausted()) {
    BL_PROPAGATE(mgr._preallocate_shared_data_pool());
  }

  ctx_impl->context_flags &= ~ContextFlags::kMTFullOrExhausted;
  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Internals - Render Call - Resolve
// ==================================================================

// These functions are intended to be used by the entry function (frontend). The purpose is to calculate the optimal
// pipeline signature and to perform the necessary initialization of the render command. Sync mode is pretty trivial
// as nothing survives the call, so nothing really needs any accounting. However, async mode is a bit more tricky as
// it's required to allocate the render command, and to make sure we can hold everything it uses.

struct RenderCallResolvedOp {
  //! \name Members
  //! \{

  Pipeline::Signature signature;
  ContextFlags flags;

  //! \}

  //! \name Interface
  //! \{

  BL_INLINE_NODEBUG bool unmodified() const noexcept { return flags == ContextFlags::kNoFlagsSet; }

  //! \}
};

// Resolves a clear operation - clear operation is always solid and always forces SRC_COPY operator on the input.
template<RenderingMode kRM>
static BL_INLINE RenderCallResolvedOp resolve_clear_op(BLRasterContextImpl* ctx_impl, ContextFlags nop_flags) noexcept {
  constexpr ContextFlags kNopExtra = kRM == kSync ? ContextFlags::kNoFlagsSet : ContextFlags::kMTFullOrExhausted;

  CompOpSimplifyInfo simplify_info = comp_op_simplify_info(CompOpExt::kSrcCopy, ctx_impl->format(), FormatExt::kPRGB32);
  CompOpSolidId solid_id = simplify_info.solid_id();

  ContextFlags combined_flags = ctx_impl->context_flags | ContextFlags(solid_id);
  ContextFlags resolved_flags = combined_flags & (nop_flags | kNopExtra);

  return RenderCallResolvedOp{simplify_info.signature(), resolved_flags};
}

// Resolves a fill operation that uses the default fill style (or stroke style if this fill implements a stroke operation).
template<RenderingMode kRM>
static BL_INLINE RenderCallResolvedOp resolve_implicit_style_op(BLRasterContextImpl* ctx_impl, ContextFlags nop_flags, const RenderFetchDataHeader* fetch_data, bool bail) noexcept {
  constexpr ContextFlags kNopExtra = kRM == kSync ? ContextFlags::kNoFlagsSet : ContextFlags::kMTFullOrExhausted;

  CompOpSimplifyInfo simplify_info = ctx_impl->comp_op_simplify_info[fetch_data->extra.format];
  CompOpSolidId solid_id = simplify_info.solid_id();

  ContextFlags bail_flag = ContextFlags(uint32_t(bail) << IntOps::bit_shift_of(uint32_t(ContextFlags::kNoOperation)));
  ContextFlags resolved_flags = (ctx_impl->context_flags | ContextFlags(solid_id) | bail_flag) & (nop_flags | kNopExtra);

  return RenderCallResolvedOp{simplify_info.signature(), resolved_flags};
}

// Resolves a solid operation, which uses a custom Rgba32 color passed by the user to the frontend.
template<RenderingMode kRM>
static BL_INLINE RenderCallResolvedOp resolve_explicit_solid_op(BLRasterContextImpl* ctx_impl, ContextFlags nop_flags, uint32_t rgba32, RenderFetchDataSolid& solid, bool bail) noexcept {
  constexpr ContextFlags kNopExtra = kRM == kSync ? ContextFlags::kNoFlagsSet : ContextFlags::kMTFullOrExhausted;

  FormatExt fmt = formatFromRgba32(rgba32);
  CompOpSimplifyInfo simplify_info = ctx_impl->comp_op_simplify_info[size_t(fmt)];
  CompOpSolidId solid_id = simplify_info.solid_id();

  ContextFlags bail_flag = ContextFlags(uint32_t(bail) << IntOps::bit_shift_of(uint32_t(ContextFlags::kNoOperation)));
  ContextFlags resolved_flags = (ctx_impl->context_flags | ContextFlags(solid_id) | bail_flag) & (nop_flags | kNopExtra);

  solid.signature.reset();
  solid.pipeline_data.prgb32 = PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888(rgba32);
  solid.pipeline_data.reserved32 = 0;

  return RenderCallResolvedOp{simplify_info.signature(), resolved_flags};
}

template<RenderingMode kRM>
static BL_NOINLINE BLResultT<RenderCallResolvedOp> resolve_explicit_style_op(BLRasterContextImpl* ctx_impl, ContextFlags nop_flags, const BLObjectCore* style, RenderFetchDataStorage<kRM>& fetch_data_storage, bool bail) noexcept {
  constexpr RenderCallResolvedOp kNop = {{0u}, ContextFlags::kNoOperation};
  constexpr BLContextStyleTransformMode kTransformMode = BL_CONTEXT_STYLE_TRANSFORM_MODE_USER;

  if constexpr (kRM == kAsync) {
    if (bl_test_flag(ctx_impl->context_flags, ContextFlags::kMTFullOrExhausted)) {
      BLResult result = handle_queues_full_or_pools_exhausted(ctx_impl);
      if (BL_UNLIKELY(result != BL_SUCCESS))
        return BLResultT<RenderCallResolvedOp>{result, kNop};
    }
  }

  fetch_data_storage.init(ctx_impl);
  RenderFetchData* fetch_data = fetch_data_storage.ptr();

  FormatExt format = FormatExt::kNone;
  BLObjectType style_type = style->_d.get_type();
  fetch_data->init_header(0);

  if (style_type <= BL_OBJECT_TYPE_NULL) {
    BLRgba32 rgba32;

    if (style_type == BL_OBJECT_TYPE_RGBA32)
      rgba32.reset(style->_d.rgba32);
    else if (style_type == BL_OBJECT_TYPE_RGBA64)
      rgba32.reset(style->_d.rgba64);
    else if (style_type == BL_OBJECT_TYPE_RGBA)
      rgba32 = style->_d.rgba.to_rgba32();
    else
      return BLResultT<RenderCallResolvedOp>{BL_SUCCESS, kNop};

    format = formatFromRgba32(rgba32.value);
    fetch_data->pipeline_data.solid.prgb32 = PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888(rgba32.value);
  }
  else {
    if (BL_UNLIKELY(style_type > BL_OBJECT_TYPE_MAX_STYLE))
      return BLResultT<RenderCallResolvedOp>{BL_ERROR_INVALID_VALUE, kNop};

    NonSolidFetchExplicitApplier applier;
    if (!init_non_solid_fetch_data(ctx_impl, fetch_data, style, style_type, kTransformMode, applier))
      return BLResultT<RenderCallResolvedOp>{BL_SUCCESS, kNop};
    format = FormatExt(fetch_data->extra.format);
  }

  CompOpSimplifyInfo simplify_info = ctx_impl->comp_op_simplify_info[size_t(format)];
  CompOpSolidId solid_id = simplify_info.solid_id();
  ContextFlags bail_flag = ContextFlags(uint32_t(bail) << IntOps::bit_shift_of(uint32_t(ContextFlags::kNoOperation)));

  ContextFlags resolved_flags = (ctx_impl->context_flags | ContextFlags(solid_id) | bail_flag) & nop_flags;
  return BLResultT<RenderCallResolvedOp>{BL_SUCCESS, {simplify_info.signature(), resolved_flags}};
}

// Resolves a blit operation.
template<RenderingMode kRM>
static BL_INLINE RenderCallResolvedOp resolve_blit_op(BLRasterContextImpl* ctx_impl, ContextFlags nop_flags, uint32_t format, bool bail) noexcept {
  constexpr ContextFlags kNopExtra = kRM == kSync ? ContextFlags::kNoFlagsSet : ContextFlags::kMTFullOrExhausted;

  CompOpSimplifyInfo simplify_info = ctx_impl->comp_op_simplify_info[format];
  CompOpSolidId solid_id = simplify_info.solid_id();

  ContextFlags bail_flag = ContextFlags(uint32_t(bail) << IntOps::bit_shift_of(uint32_t(ContextFlags::kNoOperation)));
  ContextFlags resolved_flags = (ctx_impl->context_flags | ContextFlags(solid_id) | bail_flag) & (nop_flags | kNopExtra);

  return RenderCallResolvedOp{simplify_info.signature(), resolved_flags};
}

// Prepare means to prepare an already resolved and initialized render call. We don't have to worry about memory
// allocations here, just to setup the render call object in the way it can be consumed by all the layers below.

static BL_INLINE void prepare_overridden_fetch(BLRasterContextImpl* ctx_impl, DispatchInfo& di, DispatchStyle& ds, CompOpSolidId solid_id) noexcept {
  bl_unused(di);
  ds.fetch_data = ctx_impl->solid_fetch_data_override_table[size_t(solid_id)];
}

static BL_INLINE void prepare_non_solid_fetch(BLRasterContextImpl* ctx_impl, DispatchInfo& di, DispatchStyle& ds, RenderFetchDataHeader* fetch_data) noexcept {
  bl_unused(ctx_impl);
  di.add_signature(fetch_data->signature);
  ds.fetch_data = fetch_data;
}

// Used by other macros to share code - in general this is the main resolving mechanism that
// can be used for anything except explicit style API, which requires a bit different logic.
#define BL_CONTEXT_RESOLVE_GENERIC_OP(...)                                       \
  RenderCallResolvedOp resolved = __VA_ARGS__;                                   \
                                                                                 \
  if constexpr (kRM == kAsync) {                                              \
    /* ASYNC MODE: more flags are used, so make sure our queue is not full */    \
    /* and our pools are not exhausted before rejecting the render call.   */    \
    if (BL_UNLIKELY(resolved.flags >= ContextFlags::kNoOperation)) {             \
      if (!bl_test_flag(resolved.flags, ContextFlags::kMTFullOrExhausted))         \
        return bail_result;                                                       \
                                                                                 \
      BL_PROPAGATE(handle_queues_full_or_pools_exhausted(ctx_impl));                      \
      resolved.flags &= ~ContextFlags::kMTFullOrExhausted;                       \
                                                                                 \
      /* The same as in SYNC mode - bail if the resolved operation is NOP. */    \
      if (resolved.flags >= ContextFlags::kNoOperation)                          \
        return bail_result;                                                       \
    }                                                                            \
  }                                                                              \
  else {                                                                         \
    /* SYNC MODE: Just bail if the resolved operation is NOP. */                 \
    if (resolved.flags >= ContextFlags::kNoOperation)                            \
      return bail_result;                                                         \
  }

// Resolves a clear operation (always solid).
#define BL_CONTEXT_RESOLVE_CLEAR_OP(NopFlags)                                    \
  BL_CONTEXT_RESOLVE_GENERIC_OP(                                                 \
    resolve_clear_op<kRM>(ctx_impl, NopFlags))                                         \
                                                                                 \
  DispatchInfo di;                                                               \
  di.init(resolved.signature, ctx_impl->render_target_info.fullAlphaI);                \
  DispatchStyle ds{&ctx_impl->solid_override_fill_table[size_t(resolved.flags)]}

// Resolves an operation that uses implicit style (fill or stroke).
#define BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(NopFlags, Slot, Bail)               \
  RenderFetchDataHeader* fetch_data = ctx_impl->internal_state.style[Slot].fetch_data;  \
                                                                                 \
  BL_CONTEXT_RESOLVE_GENERIC_OP(                                                 \
    resolve_implicit_style_op<kRM>(ctx_impl, NopFlags, fetch_data, Bail))                \
                                                                                 \
  RenderFetchDataHeader* overridden_fetch_data =                                   \
    ctx_impl->solid_fetch_data_override_table[size_t(resolved.flags)];                   \
                                                                                 \
  if (resolved.flags != ContextFlags::kNoFlagsSet)                               \
    fetch_data = overridden_fetch_data;                                             \
                                                                                 \
  DispatchInfo di;                                                               \
  di.init(resolved.signature, ctx_impl->internal_state.styleAlphaI[Slot]);            \
  di.add_signature(fetch_data->signature);                                         \
  DispatchStyle ds{fetch_data}

// Resolves an operation that uses explicit color (fill or stroke).
#define BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(NopFlags, Slot, Color, Bail)        \
  RenderFetchDataSolid solid;                                                    \
                                                                                 \
  BL_CONTEXT_RESOLVE_GENERIC_OP(                                                 \
    resolve_explicit_solid_op<kRM>(ctx_impl, NopFlags, Color, solid, Bail))             \
                                                                                 \
  DispatchInfo di;                                                               \
  di.init(resolved.signature, ctx_impl->internal_state.styleAlphaI[Slot]);            \
  DispatchStyle ds{&solid}

// Resolves an operation that uses explicit style (fill or stroke).
#define BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(NopFlags, Slot, Style, Bail)        \
  RenderFetchDataStorage<kRM> fetch_data;                                         \
                                                                                 \
  BLResultT<RenderCallResolvedOp> resolved =                                     \
    resolve_explicit_style_op(ctx_impl, NopFlags, Style, fetch_data, Bail);              \
                                                                                 \
  if (BL_UNLIKELY(resolved.value.flags >= ContextFlags::kNoOperation))           \
    return resolved.code ? resolved.code : bail_result;                           \
                                                                                 \
  DispatchInfo di;                                                               \
  di.init(resolved.value.signature, ctx_impl->internal_state.styleAlphaI[Slot]);      \
                                                                                 \
  RenderFetchDataHeader* overridden_fetch_data =                                   \
    ctx_impl->solid_fetch_data_override_table[size_t(resolved.value.flags)];             \
                                                                                 \
  DispatchStyle ds{fetch_data.ptr()};                                             \
  if (resolved.value.flags != ContextFlags::kNoFlagsSet)                         \
    ds.fetch_data = overridden_fetch_data;                                          \
                                                                                 \
  di.add_signature(ds.fetch_data->signature)

// Resolves a blit operation that uses explicitly passed image.
#define BL_CONTEXT_RESOLVE_BLIT_OP(NopFlags, Format, Bail)                       \
  BL_CONTEXT_RESOLVE_GENERIC_OP(                                                 \
    resolve_blit_op<kRM>(ctx_impl, NopFlags, Format, Bail))                            \
                                                                                 \
  RenderFetchDataStorage<kRM> fetch_data(ctx_impl);                                   \
                                                                                 \
  DispatchInfo di;                                                               \
  DispatchStyle ds;                                                              \
  di.init(resolved.signature, ctx_impl->global_alpha_i())

// bl::RasterEngine - ContextImpl - Internals - Render Call - Finalize
// ===================================================================

template<RenderingMode kRM>
BL_INLINE_NODEBUG BLResult finalize_explicit_op(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data, BLResult result) noexcept;

template<>
BL_INLINE_NODEBUG BLResult finalize_explicit_op<kSync>(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data, BLResult result) noexcept {
  bl_unused(ctx_impl, fetch_data);
  return result;
}

template<>
BL_INLINE_NODEBUG BLResult finalize_explicit_op<kAsync>(BLRasterContextImpl* ctx_impl, RenderFetchData* fetch_data, BLResult result) noexcept {
  // The reference count of FetchData is always incremented when a command using it is enqueued. Initially it's zero, so check for one.
  if (fetch_data->ref_count == 1u) {
    ObjectInternal::retain_instance(&fetch_data->style);
    advance_fetch_ptr(ctx_impl);
  }
  return result;
}

// bl::RasterEngine - ContextImpl - Frontend - Flush
// =================================================

static BLResult BL_CDECL flush_impl(BLContextImpl* base_impl, BLContextFlushFlags flags) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  // Nothing to flush if the rendering context is synchronous.
  if (ctx_impl->is_sync())
    return BL_SUCCESS;

  if (flags & BL_CONTEXT_FLUSH_SYNC) {
    BL_PROPAGATE(flush_render_batch(ctx_impl));
  }

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Properties
// ======================================================

static BLResult BL_CDECL get_property_impl(const BLObjectImpl* impl, const char* name, size_t name_size, BLVarCore* value_out) noexcept {
  const BLRasterContextImpl* ctx_impl = static_cast<const BLRasterContextImpl*>(impl);

  if (bl_match_property(name, name_size, "thread_count")) {
    uint32_t value = ctx_impl->is_sync() ? uint32_t(0) : uint32_t(ctx_impl->worker_mgr().thread_count() + 1u);
    return bl_var_assign_uint64(value_out, value);
  }

  if (bl_match_property(name, name_size, "accumulated_error_flags")) {
    uint32_t value = ctx_impl->sync_work_data.accumulated_error_flags();
    return bl_var_assign_uint64(value_out, value);
  }

  return bl_object_impl_get_property(ctx_impl, name, name_size, value_out);
}

static BLResult BL_CDECL set_property_impl(BLObjectImpl* impl, const char* name, size_t name_size, const BLVarCore* value) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(impl);
  return bl_object_impl_set_property(ctx_impl, name, name_size, value);
}

// bl::RasterEngine - ContextImpl - Save & Restore
// ===============================================

// Returns how many states have to be restored to match the `state_id`. It would return zero if there is no state
// that matches `state_id`.
static BL_INLINE uint32_t get_num_states_to_restore(SavedState* saved_state, uint64_t state_id) noexcept {
  uint32_t n = 1;
  do {
    uint64_t saved_id = saved_state->state_id;
    if (saved_id <= state_id)
      return saved_id == state_id ? n : uint32_t(0);
    n++;
    saved_state = saved_state->prev_state;
  } while (saved_state);

  return 0;
}

// "CoreState" consists of states that are always saved and restored to make the restoration simpler. All states
// saved/restored by CoreState should be cheap to copy.
static BL_INLINE void save_core_state(BLRasterContextImpl* ctx_impl, SavedState* state) noexcept {
  state->hints = ctx_impl->hints();
  state->comp_op = ctx_impl->comp_op();
  state->fill_rule = uint8_t(ctx_impl->fill_rule());
  state->style_type[0] = ctx_impl->internal_state.style_type[0];
  state->style_type[1] = ctx_impl->internal_state.style_type[1];

  state->clip_mode = ctx_impl->clip_mode();
  state->prev_context_flags = ctx_impl->context_flags & ~(ContextFlags::kPreservedFlags);

  state->transform_types_packed = ctx_impl->internal_state.transform_types_packed;
  state->global_alpha_i = ctx_impl->global_alpha_i();
  state->styleAlphaI[0] = ctx_impl->internal_state.styleAlphaI[0];
  state->styleAlphaI[1] = ctx_impl->internal_state.styleAlphaI[1];

  state->global_alpha = ctx_impl->global_alpha_d();
  state->style_alpha[0] = ctx_impl->internal_state.style_alpha[0];
  state->style_alpha[1] = ctx_impl->internal_state.style_alpha[1];

  state->translation_i = ctx_impl->translation_i();
}

static BL_INLINE void restore_core_state(BLRasterContextImpl* ctx_impl, SavedState* state) noexcept {
  ctx_impl->internal_state.hints = state->hints;
  ctx_impl->internal_state.comp_op = state->comp_op;
  ctx_impl->internal_state.fill_rule = state->fill_rule;
  ctx_impl->internal_state.style_type[0] = state->style_type[0];
  ctx_impl->internal_state.style_type[1] = state->style_type[1];
  ctx_impl->sync_work_data.clip_mode = state->clip_mode;
  ctx_impl->context_flags = state->prev_context_flags;

  ctx_impl->internal_state.transform_types_packed = state->transform_types_packed;
  ctx_impl->internal_state.global_alpha_i = state->global_alpha_i;
  ctx_impl->internal_state.styleAlphaI[0] = state->styleAlphaI[0];
  ctx_impl->internal_state.styleAlphaI[1] = state->styleAlphaI[1];

  ctx_impl->internal_state.global_alpha = state->global_alpha;
  ctx_impl->internal_state.style_alpha[0] = state->style_alpha[0];
  ctx_impl->internal_state.style_alpha[1] = state->style_alpha[1];

  ctx_impl->internal_state.translation_i = state->translation_i;

  on_after_comp_op_changed(ctx_impl);
}

static void discard_states(BLRasterContextImpl* ctx_impl, SavedState* top_state) noexcept {
  SavedState* saved_state = ctx_impl->saved_state;
  if (saved_state == top_state)
    return;

  // NOTE: No need to handle parts of states that don't use dynamically allocated memory.
  ContextFlags context_flags = ctx_impl->context_flags;
  do {
    if ((context_flags & (ContextFlags::kFetchDataFill | ContextFlags::kWeakStateFillStyle)) == ContextFlags::kFetchDataFill) {
      constexpr uint32_t kSlot = BL_CONTEXT_STYLE_SLOT_FILL;
      if (saved_state->style[kSlot].has_fetch_data()) {
        RenderFetchData* fetch_data = saved_state->style[kSlot].get_render_fetch_data();
        fetch_data->release(ctx_impl);
      }
    }

    if ((context_flags & (ContextFlags::kFetchDataStroke | ContextFlags::kWeakStateStrokeStyle)) == ContextFlags::kFetchDataStroke) {
      constexpr uint32_t kSlot = BL_CONTEXT_STYLE_SLOT_STROKE;
      if (saved_state->style[kSlot].has_fetch_data()) {
        RenderFetchData* fetch_data = saved_state->style[kSlot].get_render_fetch_data();
        fetch_data->release(ctx_impl);
      }
    }

    if (!bl_test_flag(context_flags, ContextFlags::kWeakStateStrokeOptions)) {
      bl_call_dtor(saved_state->stroke_options.dash_array);
    }

    SavedState* prev_state = saved_state->prev_state;
    context_flags = saved_state->prev_context_flags;

    ctx_impl->free_saved_state(saved_state);
    saved_state = prev_state;
  } while (saved_state != top_state);

  // Make 'top_state' the current state.
  ctx_impl->saved_state = top_state;
  ctx_impl->context_flags = context_flags;
}

static BLResult BL_CDECL save_impl(BLContextImpl* base_impl, BLContextCookie* cookie) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(ctx_impl->internal_state.saved_state_count >= ctx_impl->saved_state_limit))
    return bl_make_error(BL_ERROR_TOO_MANY_SAVED_STATES);

  SavedState* new_state = ctx_impl->alloc_saved_state();
  if (BL_UNLIKELY(!new_state))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  new_state->prev_state = ctx_impl->saved_state;
  new_state->state_id = Traits::max_value<uint64_t>();

  ctx_impl->saved_state = new_state;
  ctx_impl->internal_state.saved_state_count++;

  save_core_state(ctx_impl, new_state);
  ctx_impl->context_flags |= ContextFlags::kWeakStateAllFlags;

  if (!cookie)
    return BL_SUCCESS;

  // Setup the given `cookie` and make the state cookie dependent.
  uint64_t state_id = ++ctx_impl->state_id_counter;
  new_state->state_id = state_id;

  cookie->reset(ctx_impl->context_origin_id, state_id);
  return BL_SUCCESS;
}

static BLResult BL_CDECL restore_impl(BLContextImpl* base_impl, const BLContextCookie* cookie) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  SavedState* saved_state = ctx_impl->saved_state;

  if (BL_UNLIKELY(!saved_state))
    return bl_make_error(BL_ERROR_NO_STATES_TO_RESTORE);

  // By default there would be only one state to restore if `cookie` was not provided.
  uint32_t n = 1;

  if (cookie) {
    // Verify context origin.
    if (BL_UNLIKELY(cookie->data[0] != ctx_impl->context_origin_id))
      return bl_make_error(BL_ERROR_NO_MATCHING_COOKIE);

    // Verify cookie payload and get the number of states we have to restore (if valid).
    n = get_num_states_to_restore(saved_state, cookie->data[1]);
    if (BL_UNLIKELY(n == 0))
      return bl_make_error(BL_ERROR_NO_MATCHING_COOKIE);
  }
  else {
    // A state that has a `state_id` assigned cannot be restored without a matching cookie.
    if (saved_state->state_id != Traits::max_value<uint64_t>())
      return bl_make_error(BL_ERROR_NO_MATCHING_COOKIE);
  }

  ContextFlags kPreservedFlags = ContextFlags::kPreservedFlags | ContextFlags::kSharedStateAllFlags;
  ContextFlags context_flags_to_keep = ctx_impl->context_flags & kPreservedFlags;
  ctx_impl->internal_state.saved_state_count -= n;

  for (;;) {
    ContextFlags current_flags = ctx_impl->context_flags;
    restore_core_state(ctx_impl, saved_state);

    if (!bl_test_flag(current_flags, ContextFlags::kWeakStateConfig)) {
      ctx_impl->internal_state.approximation_options = saved_state->approximation_options;
      on_after_flatten_tolerance_changed(ctx_impl);
      on_after_offset_parameter_changed(ctx_impl);

      context_flags_to_keep &= ~ContextFlags::kSharedStateFill;
    }

    if (!bl_test_flag(current_flags, ContextFlags::kWeakStateClip)) {
      restore_clipping_from_state(ctx_impl, saved_state);
      context_flags_to_keep &= ~ContextFlags::kSharedStateFill;
    }

    if (!bl_test_flag(current_flags, ContextFlags::kWeakStateFillStyle)) {
      StyleData* dst = &ctx_impl->internal_state.style[BL_CONTEXT_STYLE_SLOT_FILL];
      StyleData* src = &saved_state->style[BL_CONTEXT_STYLE_SLOT_FILL];

      if (bl_test_flag(current_flags, ContextFlags::kFetchDataFill))
        destroy_valid_style(ctx_impl, dst);

      dst->copy_from(*src);
    }

    if (!bl_test_flag(current_flags, ContextFlags::kWeakStateStrokeStyle)) {
      StyleData* dst = &ctx_impl->internal_state.style[BL_CONTEXT_STYLE_SLOT_STROKE];
      StyleData* src = &saved_state->style[BL_CONTEXT_STYLE_SLOT_STROKE];

      if (bl_test_flag(current_flags, ContextFlags::kFetchDataStroke))
        destroy_valid_style(ctx_impl, dst);

      dst->copy_from(*src);
    }

    if (!bl_test_flag(current_flags, ContextFlags::kWeakStateStrokeOptions)) {
      // NOTE: This code is unsafe, but since we know that `BLStrokeOptions` is movable it's just fine. We
      // destroy `BLStrokeOptions` first and then move it into that destroyed instance from the saved state.
      ArrayInternal::release_instance(&ctx_impl->internal_state.stroke_options.dash_array);
      ctx_impl->internal_state.stroke_options._copy_from(saved_state->stroke_options);
      context_flags_to_keep &= ~(ContextFlags::kSharedStateStrokeBase | ContextFlags::kSharedStateStrokeExt);
    }

    // UserTransform state is unset when MetaTransform and/or UserTransform were saved.
    if (!bl_test_flag(current_flags, ContextFlags::kWeakStateUserTransform)) {
      ctx_impl->internal_state.user_transform = saved_state->user_transform;

      if (!bl_test_flag(current_flags, ContextFlags::kWeakStateMetaTransform)) {
        ctx_impl->internal_state.meta_transform = saved_state->alt_transform;
        update_final_transform(ctx_impl);
        update_meta_transform_fixed(ctx_impl);
        update_final_transform_fixed(ctx_impl);
      }
      else {
        ctx_impl->internal_state.final_transform = saved_state->alt_transform;
        update_final_transform_fixed(ctx_impl);
      }

      context_flags_to_keep &= ~(ContextFlags::kSharedStateFill | ContextFlags::kSharedStateStrokeBase | ContextFlags::kSharedStateStrokeExt);
    }

    SavedState* finished_saved_state = saved_state;
    saved_state = saved_state->prev_state;

    ctx_impl->saved_state = saved_state;
    ctx_impl->free_saved_state(finished_saved_state);

    if (--n == 0)
      break;
  }

  ctx_impl->context_flags = (ctx_impl->context_flags & ~kPreservedFlags) | context_flags_to_keep;
  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Transformations
// ===========================================================

static BLResult BL_CDECL apply_transform_op_impl(BLContextImpl* base_impl, BLTransformOp op_type, const void* op_data) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  Matrix2x2 before2x2;
  on_before_user_transform_change(ctx_impl, before2x2);
  BL_PROPAGATE(bl_matrix2d_apply_op(&ctx_impl->internal_state.user_transform, op_type, op_data));

  on_after_user_transform_changed(ctx_impl, before2x2);
  return BL_SUCCESS;
}

static BLResult BL_CDECL user_to_meta_impl(BLContextImpl* base_impl) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  constexpr ContextFlags kUserAndMetaFlags = ContextFlags::kWeakStateMetaTransform | ContextFlags::kWeakStateUserTransform;

  if (bl_test_flag(ctx_impl->context_flags, kUserAndMetaFlags)) {
    SavedState* state = ctx_impl->saved_state;

    // Always save both `meta_transform` and `user_transform` in case we have to save the current state before we
    // change the transform. In this case the `alt_transform` of the state would store the current `meta_transform`
    // and on state restore the final transform would be recalculated in-place.
    state->alt_transform = ctx_impl->meta_transform();

    // Don't copy it if it was already saved, we would have copied an altered `user_transform`.
    if (bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateUserTransform))
      state->user_transform = ctx_impl->user_transform();
  }

  ctx_impl->context_flags &= ~(kUserAndMetaFlags | ContextFlags::kSharedStateStrokeExt);
  ctx_impl->internal_state.user_transform.reset();
  ctx_impl->internal_state.meta_transform = ctx_impl->final_transform();
  ctx_impl->internal_state.meta_transform_fixed = ctx_impl->final_transform_fixed();
  ctx_impl->internal_state.meta_transform_type = uint8_t(ctx_impl->final_transform_type());
  ctx_impl->internal_state.meta_transform_fixed_type = uint8_t(ctx_impl->final_transform_fixed_type());

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Rendering Hints
// ===========================================================

static BLResult BL_CDECL set_hint_impl(BLContextImpl* base_impl, BLContextHint hint_type, uint32_t value) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  switch (hint_type) {
    case BL_CONTEXT_HINT_RENDERING_QUALITY:
      if (BL_UNLIKELY(value > BL_RENDERING_QUALITY_MAX_VALUE))
        return bl_make_error(BL_ERROR_INVALID_VALUE);

      ctx_impl->internal_state.hints.rendering_quality = uint8_t(value);
      return BL_SUCCESS;

    case BL_CONTEXT_HINT_GRADIENT_QUALITY:
      if (BL_UNLIKELY(value > BL_GRADIENT_QUALITY_MAX_VALUE))
        return bl_make_error(BL_ERROR_INVALID_VALUE);

      ctx_impl->internal_state.hints.gradient_quality = uint8_t(value);
      return BL_SUCCESS;

    case BL_CONTEXT_HINT_PATTERN_QUALITY:
      if (BL_UNLIKELY(value > BL_PATTERN_QUALITY_MAX_VALUE))
        return bl_make_error(BL_ERROR_INVALID_VALUE);

      ctx_impl->internal_state.hints.pattern_quality = uint8_t(value);
      return BL_SUCCESS;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }
}

static BLResult BL_CDECL set_hints_impl(BLContextImpl* base_impl, const BLContextHints* hints) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  uint8_t rendering_quality = hints->rendering_quality;
  uint8_t pattern_quality = hints->pattern_quality;
  uint8_t gradient_quality = hints->gradient_quality;

  if (BL_UNLIKELY(rendering_quality > BL_RENDERING_QUALITY_MAX_VALUE ||
                  pattern_quality   > BL_PATTERN_QUALITY_MAX_VALUE   ||
                  gradient_quality  > BL_GRADIENT_QUALITY_MAX_VALUE  ))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  ctx_impl->internal_state.hints.rendering_quality = rendering_quality;
  ctx_impl->internal_state.hints.pattern_quality = pattern_quality;
  ctx_impl->internal_state.hints.gradient_quality = gradient_quality;
  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Approximation Options
// =================================================================

static BLResult BL_CDECL set_flatten_mode_impl(BLContextImpl* base_impl, BLFlattenMode mode) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(uint32_t(mode) > BL_FLATTEN_MODE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_config_change(ctx_impl);
  ctx_impl->context_flags &= ~ContextFlags::kWeakStateConfig;

  ctx_impl->internal_state.approximation_options.flatten_mode = uint8_t(mode);
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_flatten_tolerance_impl(BLContextImpl* base_impl, double tolerance) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(Math::is_nan(tolerance)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_config_change(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kWeakStateConfig | ContextFlags::kSharedStateFill);

  tolerance = bl_clamp(tolerance, ContextInternal::kMinimumTolerance, ContextInternal::kMaximumTolerance);
  BL_ASSERT(Math::is_finite(tolerance));

  ctx_impl->internal_state.approximation_options.flatten_tolerance = tolerance;
  on_after_flatten_tolerance_changed(ctx_impl);

  return BL_SUCCESS;
}

static BLResult BL_CDECL set_approximation_options_impl(BLContextImpl* base_impl, const BLApproximationOptions* options) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  uint32_t flatten_mode = options->flatten_mode;
  uint32_t offset_mode = options->offset_mode;

  double flatten_tolerance = options->flatten_tolerance;
  double offset_parameter = options->offset_parameter;

  if (BL_UNLIKELY(flatten_mode > BL_FLATTEN_MODE_MAX_VALUE ||
                  offset_mode > BL_OFFSET_MODE_MAX_VALUE ||
                  Math::is_nan(flatten_tolerance) ||
                  Math::is_nan(offset_parameter)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_config_change(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kWeakStateConfig | ContextFlags::kSharedStateFill);

  BLApproximationOptions& dst = ctx_impl->internal_state.approximation_options;
  dst.flatten_mode = uint8_t(flatten_mode);
  dst.offset_mode = uint8_t(offset_mode);
  dst.flatten_tolerance = bl_clamp(flatten_tolerance, ContextInternal::kMinimumTolerance, ContextInternal::kMaximumTolerance);
  dst.offset_parameter = offset_parameter;

  on_after_flatten_tolerance_changed(ctx_impl);
  on_after_offset_parameter_changed(ctx_impl);

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Style Alpha
// =======================================================

static BLResult BL_CDECL set_style_alpha_impl(BLContextImpl* base_impl, BLContextStyleSlot slot, double alpha) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(slot > BL_CONTEXT_STYLE_SLOT_MAX_VALUE || Math::is_nan(alpha)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  ContextFlags no_alpha = ContextFlags::kNoBaseAlpha << slot;
  ContextFlags context_flags = ctx_impl->context_flags & ~no_alpha;

  alpha = bl_clamp(alpha, 0.0, 1.0);
  uint32_t alpha_i = uint32_t(Math::round_to_int(ctx_impl->global_alpha_d() * ctx_impl->full_alpha_d() * alpha));

  if (alpha_i)
    no_alpha = ContextFlags::kNoFlagsSet;

  ctx_impl->internal_state.style_alpha[slot] = alpha;
  ctx_impl->internal_state.styleAlphaI[slot] = alpha_i;
  ctx_impl->context_flags = context_flags | no_alpha;
  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Swap Styles
// =======================================================

static BLResult BL_CDECL swap_styles_impl(BLContextImpl* base_impl, BLContextStyleSwapMode mode) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  ContextFlags context_flags = ctx_impl->context_flags;

  if (BL_UNLIKELY(mode > BL_CONTEXT_STYLE_SWAP_MODE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  RasterContextState& state = ctx_impl->internal_state;

  constexpr BLContextStyleSlot kFillSlot = BL_CONTEXT_STYLE_SLOT_FILL;
  constexpr BLContextStyleSlot kStrokeSlot = BL_CONTEXT_STYLE_SLOT_STROKE;
  constexpr ContextFlags kWeakFillAndStrokeStyle = ContextFlags::kWeakStateFillStyle | ContextFlags::kWeakStateStrokeStyle;

  if (bl_test_flag(context_flags, kWeakFillAndStrokeStyle)) {
    BL_ASSERT(ctx_impl->saved_state != nullptr);

    if (bl_test_flag(context_flags, ContextFlags::kWeakStateFillStyle)) {
      ctx_impl->saved_state->style[kFillSlot].copy_from(state.style[kFillSlot]);
      if (bl_test_flag(context_flags, ContextFlags::kFetchDataFill))
        state.style[kFillSlot].get_render_fetch_data()->ref_count++;
    }

    if (bl_test_flag(context_flags, ContextFlags::kWeakStateStrokeStyle)) {
      ctx_impl->saved_state->style[kStrokeSlot].copy_from(state.style[kStrokeSlot]);
      if (bl_test_flag(context_flags, ContextFlags::kFetchDataFill))
        state.style[kStrokeSlot].get_render_fetch_data()->ref_count++;
    }

    context_flags &= ~kWeakFillAndStrokeStyle;
  }

  // Swap fill and stroke styles.
  {
    state.style[kFillSlot].swap(state.style[kStrokeSlot]);
    BLInternal::swap(state.style_type[kFillSlot], state.style_type[kStrokeSlot]);

    constexpr ContextFlags kSwapFlags = ContextFlags::kNoFillAndStrokeStyle | ContextFlags::kFetchDataFillAndStroke;
    context_flags = (context_flags & ~kSwapFlags) | ((context_flags >> 1) & kSwapFlags) | ((context_flags << 1) & kSwapFlags);
  }

  // Swap fill and stroke alphas.
  if (mode == BL_CONTEXT_STYLE_SWAP_MODE_STYLES_WITH_ALPHA) {
    BLInternal::swap(state.style_alpha[kFillSlot], state.style_alpha[kStrokeSlot]);
    BLInternal::swap(state.styleAlphaI[kFillSlot], state.styleAlphaI[kStrokeSlot]);

    constexpr ContextFlags kSwapFlags = ContextFlags::kNoFillAndStrokeAlpha;
    context_flags = (context_flags & ~kSwapFlags) | ((context_flags >> 1) & kSwapFlags) | ((context_flags << 1) & kSwapFlags);
  }

  ctx_impl->context_flags = context_flags;
  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Composition Options
// ===============================================================

static BLResult BL_CDECL set_global_alpha_impl(BLContextImpl* base_impl, double alpha) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(Math::is_nan(alpha)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  alpha = bl_clamp(alpha, 0.0, 1.0);

  double intAlphaD = alpha * ctx_impl->full_alpha_d();
  double fillAlphaD = intAlphaD * ctx_impl->internal_state.style_alpha[BL_CONTEXT_STYLE_SLOT_FILL];
  double strokeAlphaD = intAlphaD * ctx_impl->internal_state.style_alpha[BL_CONTEXT_STYLE_SLOT_STROKE];

  uint32_t global_alpha_i = uint32_t(Math::round_to_int(intAlphaD));
  uint32_t styleAlphaI[2] = { uint32_t(Math::round_to_int(fillAlphaD)), uint32_t(Math::round_to_int(strokeAlphaD)) };

  ctx_impl->internal_state.global_alpha = alpha;
  ctx_impl->internal_state.global_alpha_i = global_alpha_i;
  ctx_impl->internal_state.styleAlphaI[0] = styleAlphaI[0];
  ctx_impl->internal_state.styleAlphaI[1] = styleAlphaI[1];

  ContextFlags context_flags = ctx_impl->context_flags;
  context_flags &= ~(ContextFlags::kNoGlobalAlpha | ContextFlags::kNoFillAlpha | ContextFlags::kNoStrokeAlpha);

  if (!global_alpha_i) context_flags |= ContextFlags::kNoGlobalAlpha;
  if (!styleAlphaI[0]) context_flags |= ContextFlags::kNoFillAlpha;
  if (!styleAlphaI[1]) context_flags |= ContextFlags::kNoStrokeAlpha;

  ctx_impl->context_flags = context_flags;
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_comp_op_impl(BLContextImpl* base_impl, BLCompOp comp_op) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(uint32_t(comp_op) > BL_COMP_OP_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  ctx_impl->internal_state.comp_op = uint8_t(comp_op);
  on_after_comp_op_changed(ctx_impl);

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Fill Options
// ========================================================

static BLResult BL_CDECL set_fill_rule_impl(BLContextImpl* base_impl, BLFillRule fill_rule) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(uint32_t(fill_rule) > BL_FILL_RULE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  ctx_impl->internal_state.fill_rule = uint8_t(fill_rule);
  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Frontend - Stroke Options
// ==========================================================

static BLResult BL_CDECL set_stroke_width_impl(BLContextImpl* base_impl, double width) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kNoStrokeOptions | ContextFlags::kWeakStateStrokeOptions | ContextFlags::kSharedStateStrokeBase);
  ctx_impl->internal_state.stroke_options.width = width;
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_stroke_miter_limit_impl(BLContextImpl* base_impl, double miter_limit) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kNoStrokeOptions | ContextFlags::kWeakStateStrokeOptions | ContextFlags::kSharedStateStrokeBase);
  ctx_impl->internal_state.stroke_options.miter_limit = miter_limit;
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_stroke_cap_impl(BLContextImpl* base_impl, BLStrokeCapPosition position, BLStrokeCap stroke_cap) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(uint32_t(position) > BL_STROKE_CAP_POSITION_MAX_VALUE ||
                  uint32_t(stroke_cap) > BL_STROKE_CAP_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~ContextFlags::kSharedStateStrokeBase;

  ctx_impl->internal_state.stroke_options.caps[position] = uint8_t(stroke_cap);
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_stroke_caps_impl(BLContextImpl* base_impl, BLStrokeCap stroke_cap) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(uint32_t(stroke_cap) > BL_STROKE_CAP_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~ContextFlags::kSharedStateStrokeBase;

  for (uint32_t i = 0; i <= BL_STROKE_CAP_POSITION_MAX_VALUE; i++)
    ctx_impl->internal_state.stroke_options.caps[i] = uint8_t(stroke_cap);
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_stroke_join_impl(BLContextImpl* base_impl, BLStrokeJoin stroke_join) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(uint32_t(stroke_join) > BL_STROKE_JOIN_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~ContextFlags::kSharedStateStrokeBase;

  ctx_impl->internal_state.stroke_options.join = uint8_t(stroke_join);
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_stroke_dash_offset_impl(BLContextImpl* base_impl, double dash_offset) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kNoStrokeOptions | ContextFlags::kWeakStateStrokeOptions | ContextFlags::kSharedStateStrokeBase);

  ctx_impl->internal_state.stroke_options.dash_offset = dash_offset;
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_stroke_dash_array_impl(BLContextImpl* base_impl, const BLArrayCore* dash_array) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(dash_array->_d.raw_type() != BL_OBJECT_TYPE_ARRAY_FLOAT64))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_stroke_change_and_destroy_dash_array(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kNoStrokeOptions | ContextFlags::kWeakStateStrokeOptions | ContextFlags::kSharedStateStrokeBase);

  ctx_impl->internal_state.stroke_options.dash_array._d = dash_array->_d;
  return ArrayInternal::retain_instance(&ctx_impl->internal_state.stroke_options.dash_array);
}

static BLResult BL_CDECL set_stroke_transform_order_impl(BLContextImpl* base_impl, BLStrokeTransformOrder transform_order) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(uint32_t(transform_order) > BL_STROKE_TRANSFORM_ORDER_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kWeakStateStrokeOptions | ContextFlags::kSharedStateStrokeBase);

  ctx_impl->internal_state.stroke_options.transform_order = uint8_t(transform_order);
  return BL_SUCCESS;
}

static BLResult BL_CDECL set_stroke_options_impl(BLContextImpl* base_impl, const BLStrokeOptionsCore* options) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  if (BL_UNLIKELY(options->start_cap > BL_STROKE_CAP_MAX_VALUE ||
                  options->end_cap > BL_STROKE_CAP_MAX_VALUE ||
                  options->join > BL_STROKE_JOIN_MAX_VALUE ||
                  options->transform_order > BL_STROKE_TRANSFORM_ORDER_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  on_before_stroke_change(ctx_impl);
  ctx_impl->context_flags &= ~(ContextFlags::kNoStrokeOptions | ContextFlags::kWeakStateStrokeOptions | ContextFlags::kSharedStateStrokeBase);
  return bl_stroke_options_assign_weak(&ctx_impl->internal_state.stroke_options, options);
}

// bl::RasterEngine - ContextImpl - Frontend - Clip Operations
// ===========================================================

static BLResult clip_to_final_box(BLRasterContextImpl* ctx_impl, const BLBox& input_box) noexcept {
  BLBox b;
  on_before_clip_box_change(ctx_impl);

  if (Geometry::intersect(b, ctx_impl->final_clip_box_d(), input_box)) {
    int fpMaskI = ctx_impl->render_target_info.fpMaskI;
    int fpShiftI = ctx_impl->render_target_info.fpShiftI;

    ctx_impl->set_final_clip_box_fixed_d(b * ctx_impl->fp_scale_d());
    const BLBoxI& clipBoxFixedI = ctx_impl->final_clip_box_fixed_i();

    ctx_impl->internal_state.final_clip_box_d = b;
    ctx_impl->internal_state.final_clip_box_i.reset((clipBoxFixedI.x0 >> fpShiftI),
                                            (clipBoxFixedI.y0 >> fpShiftI),
                                            (clipBoxFixedI.x1 + fpMaskI) >> fpShiftI,
                                            (clipBoxFixedI.y1 + fpMaskI) >> fpShiftI);

    int32_t bits = clipBoxFixedI.x0 | clipBoxFixedI.y0 | clipBoxFixedI.x1 | clipBoxFixedI.y1;

    if ((bits & fpMaskI) == 0)
      ctx_impl->sync_work_data.clip_mode = BL_CLIP_MODE_ALIGNED_RECT;
    else
      ctx_impl->sync_work_data.clip_mode = BL_CLIP_MODE_UNALIGNED_RECT;
  }
  else {
    ctx_impl->internal_state.final_clip_box_d.reset();
    ctx_impl->internal_state.final_clip_box_i.reset();
    ctx_impl->set_final_clip_box_fixed_d(BLBox(0, 0, 0, 0));
    ctx_impl->context_flags |= ContextFlags::kNoClipRect;
    ctx_impl->sync_work_data.clip_mode = BL_CLIP_MODE_ALIGNED_RECT;
  }

  ctx_impl->context_flags &= ~(ContextFlags::kWeakStateClip | ContextFlags::kSharedStateFill);
  return BL_SUCCESS;
}

static BLResult BL_CDECL clip_to_rect_d_impl(BLContextImpl* base_impl, const BLRect* rect) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  // TODO: [Rendering Context] Path-based clipping.
  BLBox input_box = BLBox(rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
  return clip_to_final_box(ctx_impl, TransformInternal::map_box(ctx_impl->final_transform(), input_box));
}

static BLResult BL_CDECL clip_to_rect_i_impl(BLContextImpl* base_impl, const BLRectI* rect) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  // Don't bother if the current ClipBox is not aligned or the translation is not integral.
  if (ctx_impl->sync_work_data.clip_mode != BL_CLIP_MODE_ALIGNED_RECT || !bl_test_flag(ctx_impl->context_flags, ContextFlags::kInfoIntegralTranslation)) {
    BLRect rect_d;
    rect_d.x = double(rect->x);
    rect_d.y = double(rect->y);
    rect_d.w = double(rect->w);
    rect_d.h = double(rect->h);
    return clip_to_rect_d_impl(ctx_impl, &rect_d);
  }

  BLBoxI b;
  on_before_clip_box_change(ctx_impl);

  int tx = ctx_impl->translation_i().x;
  int ty = ctx_impl->translation_i().y;

  if (BL_TARGET_ARCH_BITS < 64) {
    OverflowFlag of{};

    int x0 = IntOps::add_overflow(tx, rect->x, &of);
    int y0 = IntOps::add_overflow(ty, rect->y, &of);
    int x1 = IntOps::add_overflow(x0, rect->w, &of);
    int y1 = IntOps::add_overflow(y0, rect->h, &of);

    if (BL_UNLIKELY(of))
      goto Use64Bit;

    b.x0 = bl_max(x0, ctx_impl->final_clip_box_i().x0);
    b.y0 = bl_max(y0, ctx_impl->final_clip_box_i().y0);
    b.x1 = bl_min(x1, ctx_impl->final_clip_box_i().x1);
    b.y1 = bl_min(y1, ctx_impl->final_clip_box_i().y1);
  }
  else {
Use64Bit:
    // We don't have to worry about overflow in 64-bit mode.
    int64_t x0 = tx + int64_t(rect->x);
    int64_t y0 = ty + int64_t(rect->y);
    int64_t x1 = x0 + int64_t(rect->w);
    int64_t y1 = y0 + int64_t(rect->h);

    b.x0 = int(bl_max<int64_t>(x0, ctx_impl->final_clip_box_i().x0));
    b.y0 = int(bl_max<int64_t>(y0, ctx_impl->final_clip_box_i().y0));
    b.x1 = int(bl_min<int64_t>(x1, ctx_impl->final_clip_box_i().x1));
    b.y1 = int(bl_min<int64_t>(y1, ctx_impl->final_clip_box_i().y1));
  }

  if (b.x0 < b.x1 && b.y0 < b.y1) {
    ctx_impl->internal_state.final_clip_box_i = b;
    ctx_impl->internal_state.final_clip_box_d.reset(b);
    ctx_impl->set_final_clip_box_fixed_d(ctx_impl->final_clip_box_d() * ctx_impl->fp_scale_d());
  }
  else {
    ctx_impl->internal_state.final_clip_box_i.reset();
    ctx_impl->internal_state.final_clip_box_d.reset(b);
    ctx_impl->set_final_clip_box_fixed_d(BLBox(0, 0, 0, 0));
    ctx_impl->context_flags |= ContextFlags::kNoClipRect;
  }

  ctx_impl->context_flags &= ~(ContextFlags::kWeakStateClip | ContextFlags::kSharedStateFill);
  return BL_SUCCESS;
}

static BLResult BL_CDECL restore_clipping_impl(BLContextImpl* base_impl) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  SavedState* state = ctx_impl->saved_state;

  if (!bl_test_flag(ctx_impl->context_flags, ContextFlags::kWeakStateClip)) {
    if (state) {
      restore_clipping_from_state(ctx_impl, state);
      ctx_impl->sync_work_data.clip_mode = state->clip_mode;
      ctx_impl->context_flags &= ~(ContextFlags::kNoClipRect | ContextFlags::kWeakStateClip | ContextFlags::kSharedStateFill);
      ctx_impl->context_flags |= (state->prev_context_flags & ContextFlags::kNoClipRect);
    }
    else {
      // If there is no state saved it means that we have to restore clipping to
      // the initial state, which is accessible through `meta_clip_box_i` member.
      ctx_impl->context_flags &= ~(ContextFlags::kNoClipRect | ContextFlags::kSharedStateFill);
      reset_clipping_to_meta_clip_box(ctx_impl);
    }
  }

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Mask & Blit Utilities
// ======================================================

static BL_INLINE BLResult check_image_area(BLRectI& out, const BLImageImpl* image, const BLRectI* area) noexcept {
  out.reset(0, 0, image->size.w, image->size.h);

  if (area) {
    unsigned max_w = unsigned(out.w) - unsigned(area->x);
    unsigned max_h = unsigned(out.h) - unsigned(area->y);

    if ((max_w > unsigned(out.w)) | (unsigned(area->w) > max_w) |
        (max_h > unsigned(out.h)) | (unsigned(area->h) > max_h))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    out = *area;
  }

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Internals - Asynchronous Rendering - Shared State
// ==================================================================================

static constexpr ContextFlags shared_stroke_state_flags_table[BL_STROKE_TRANSFORM_ORDER_MAX_VALUE + 1] = {
  ContextFlags::kSharedStateStrokeBase,
  ContextFlags::kSharedStateStrokeBase | ContextFlags::kSharedStateStrokeExt
};

static constexpr uint32_t shared_stroke_state_size_table[BL_STROKE_TRANSFORM_ORDER_MAX_VALUE + 1] = {
  uint32_t(sizeof(SharedBaseStrokeState)),
  uint32_t(sizeof(SharedExtendedStrokeState))
};

// NOTE: These functions are named 'getXXX()', because they are not intended to fail. They allocate data from
// shared data pool, which is ALWAYS available once the frontend function checks ContextFlags and refills the
// pool when necessary. There is ALWAYS enough space in the pool to allocate BOTH shared states, so we don't
// have to do any checks in case that shared fill or stroke states were not created yet.
static BL_INLINE SharedFillState* get_shared_fill_state(BLRasterContextImpl* ctx_impl) noexcept {
  SharedFillState* shared_fill_state = ctx_impl->shared_fill_state;

  if (!bl_test_flag(ctx_impl->context_flags, ContextFlags::kSharedStateFill)) {
    shared_fill_state = ctx_impl->worker_mgr().allocate_from_shared_data_pool<SharedFillState>();

    const BLMatrix2D& ft = ctx_impl->final_transform_fixed();
    shared_fill_state->final_clip_box_fixed_d = ctx_impl->final_clip_box_fixed_d();
    shared_fill_state->final_transform_fixed = Matrix2x2{ft.m00, ft.m01, ft.m10, ft.m11};
    shared_fill_state->toleranceFixedD = ctx_impl->internal_state.toleranceFixedD;

    ctx_impl->shared_fill_state = shared_fill_state;
    ctx_impl->context_flags |= ContextFlags::kSharedStateFill;
    mark_queue_full_or_exhausted(ctx_impl, ctx_impl->worker_mgr().is_shared_data_pool_exhausted());
  }

  return shared_fill_state;
}

static BL_INLINE SharedBaseStrokeState* get_shared_stroke_state(BLRasterContextImpl* ctx_impl) noexcept {
  SharedBaseStrokeState* shared_stroke_state = ctx_impl->shared_stroke_state;

  BLStrokeTransformOrder transform_order = BLStrokeTransformOrder(ctx_impl->stroke_options().transform_order);
  ContextFlags shared_flags = shared_stroke_state_flags_table[transform_order];

  if ((ctx_impl->context_flags & shared_flags) != shared_flags) {
    size_t state_size = shared_stroke_state_size_table[transform_order];
    shared_stroke_state = ctx_impl->worker_mgr().allocate_from_shared_data_pool<SharedBaseStrokeState>(state_size);

    bl_call_ctor(*shared_stroke_state, ctx_impl->stroke_options(), ctx_impl->approximation_options());

    if (transform_order != BL_STROKE_TRANSFORM_ORDER_AFTER) {
      const BLMatrix2D& ut = ctx_impl->user_transform();
      const BLMatrix2D& mt = ctx_impl->meta_transform_fixed();
      static_cast<SharedExtendedStrokeState*>(shared_stroke_state)->user_transform = Matrix2x2{ut.m00, ut.m01, ut.m10, ut.m11};
      static_cast<SharedExtendedStrokeState*>(shared_stroke_state)->meta_transform_fixed = Matrix2x2{mt.m00, mt.m01, mt.m10, mt.m11};
    }

    ctx_impl->shared_stroke_state = shared_stroke_state;
    ctx_impl->context_flags |= shared_flags;
    mark_queue_full_or_exhausted(ctx_impl, ctx_impl->worker_mgr().is_shared_data_pool_exhausted());
  }

  return shared_stroke_state;
}

// bl::RasterEngine - ContextImpl - Internals - Asynchronous Rendering - Jobs
// ==========================================================================

template<typename JobType>
static BL_INLINE BLResult new_fill_job(BLRasterContextImpl* ctx_impl, size_t job_data_size, JobType** out) noexcept {
  JobType* job = ctx_impl->worker_mgr()._allocator.template allocNoAlignT<JobType>(job_data_size);
  if (BL_UNLIKELY(!job))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  job->init_states(get_shared_fill_state(ctx_impl));
  *out = job;
  return BL_SUCCESS;
}

template<typename JobType>
static BL_INLINE BLResult new_stroke_job(BLRasterContextImpl* ctx_impl, size_t job_data_size, JobType** out) noexcept {
  JobType* job = ctx_impl->worker_mgr()._allocator.template allocNoAlignT<JobType>(job_data_size);
  if (BL_UNLIKELY(!job))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  job->init_states(get_shared_fill_state(ctx_impl), get_shared_stroke_state(ctx_impl));
  *out = job;
  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Internals - Asynchronous Rendering - Enqueue
// =============================================================================

template<typename CommandFinalizer>
static BL_INLINE BLResult enqueue_command(
    BLRasterContextImpl* ctx_impl,
    RenderCommand* command,
    uint8_t qy0,
    RenderFetchDataHeader* fetch_data,
    const CommandFinalizer& command_finalizer) noexcept {

  WorkerManager& mgr = ctx_impl->worker_mgr();
  constexpr uint32_t kRetainsStyleFetchDataShift = IntOps::bit_shift_of(uint32_t(RenderCommandFlags::kRetainsStyleFetchData));

  if (fetch_data->is_solid()) {
    command->_source.solid = static_cast<RenderFetchDataSolid*>(fetch_data)->pipeline_data;
  }
  else {
    uint32_t batch_id = mgr.current_batch_id();
    uint32_t fd_retain = uint32_t(fetch_data->batch_id != batch_id);

    fetch_data->batch_id = mgr.current_batch_id();
    fetch_data->retain(fd_retain);

    RenderCommandFlags flags = RenderCommandFlags(fd_retain << kRetainsStyleFetchDataShift) | RenderCommandFlags::kHasStyleFetchData;
    command->add_flags(flags);
    command->_source.fetch_data = static_cast<RenderFetchData*>(fetch_data);

    mgr._command_appender.mark_fetch_data(fd_retain);
  }

  command_finalizer(command);
  mgr.command_appender().initQuantizedY0(qy0);
  mgr.command_appender().advance();
  mark_queue_full_or_exhausted(ctx_impl, mgr._command_appender.full());

  return BL_SUCCESS;
}

template<typename JobType, typename JobFinalizer>
static BL_INLINE BLResult enqueue_command_with_fill_job(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    size_t job_size, const BLPoint& origin_fixed, const JobFinalizer& job_finalizer) noexcept {

  constexpr uint8_t kNoCoord = kInvalidQuantizedCoordinate;

  RenderCommand* command = ctx_impl->worker_mgr->current_command();
  JobType* job;

  // TODO: [Rendering Context] FetchData calculation offloading not ready yet - needs more testing:
  // bool was_pending = di.signature.has_pending_flag();
  // di.signature.clear_pending_bit();

  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, command->pipe_dispatch_data()));
  BL_PROPAGATE(new_fill_job(ctx_impl, job_size, &job));

  return enqueue_command(ctx_impl, command, kNoCoord, ds.fetch_data, [&](RenderCommand* command) noexcept {
    command->_payload.analytic.state_slot_index = ctx_impl->worker_mgr().next_state_slot_index();

    WorkerManager& mgr = ctx_impl->worker_mgr();
    job->init_fill_job(mgr._command_appender.queue(), mgr._command_appender.index());

    // TODO: [Rendering Context] FetchData calculation offloading not ready yet - needs more testing:
    // if (was_pending && command->has_flag(RenderCommandFlags::kRetainsStyleFetchData))
    //   job->add_job_flags(RenderJobFlags::kComputePendingFetchData);

    job->set_origin_fixed(origin_fixed);
    job->set_meta_transform_fixed_type(ctx_impl->meta_transform_fixed_type());
    job->set_final_transform_fixed_type(ctx_impl->final_transform_fixed_type());
    job_finalizer(job);
    mgr.add_job(job);
    mark_queue_full_or_exhausted(ctx_impl, mgr._job_appender.full());
  });
}

template<typename JobType, typename JobFinalizer>
static BL_INLINE BLResult enqueue_command_with_stroke_job(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    size_t job_size, const BLPoint& origin_fixed, const JobFinalizer& job_finalizer) noexcept {

  constexpr uint8_t kNoCoord = kInvalidQuantizedCoordinate;

  RenderCommand* command = ctx_impl->worker_mgr->current_command();
  JobType* job;

  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, command->pipe_dispatch_data()));
  BL_PROPAGATE(new_stroke_job(ctx_impl, job_size, &job));

  return enqueue_command(ctx_impl, command, kNoCoord, ds.fetch_data, [&](RenderCommand* command) noexcept {
    command->_payload.analytic.state_slot_index = ctx_impl->worker_mgr().next_state_slot_index();

    WorkerManager& mgr = ctx_impl->worker_mgr();
    job->init_stroke_job(mgr._command_appender.queue(), mgr._command_appender.index());

    job->set_origin_fixed(origin_fixed);
    job->set_meta_transform_fixed_type(ctx_impl->meta_transform_fixed_type());
    job->set_final_transform_fixed_type(ctx_impl->final_transform_fixed_type());
    job_finalizer(job);

    mgr.add_job(job);
    mark_queue_full_or_exhausted(ctx_impl, mgr._job_appender.full());
  });
}

template<uint32_t OpType, typename JobType, typename JobFinalizer>
static BL_INLINE BLResult enqueue_command_with_fill_or_stroke_job(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    size_t job_size, const BLPoint& origin_fixed, const JobFinalizer& job_finalizer) noexcept {

  if (OpType == BL_CONTEXT_STYLE_SLOT_FILL)
    return enqueue_command_with_fill_job<JobType, JobFinalizer>(ctx_impl, di, ds, job_size, origin_fixed, job_finalizer);
  else
    return enqueue_command_with_stroke_job<JobType, JobFinalizer>(ctx_impl, di, ds, job_size, origin_fixed, job_finalizer);
}

// bl::RasterEngine - ContextImpl - Asynchronous Rendering - Enqueue GlyphRun & TextData
// =====================================================================================

struct BLGlyphPlacementRawData {
  uint64_t data[2];
};

BL_STATIC_ASSERT(sizeof(BLGlyphPlacementRawData) == sizeof(BLPoint));
BL_STATIC_ASSERT(sizeof(BLGlyphPlacementRawData) == sizeof(BLGlyphPlacement));

template<uint32_t OpType>
static BL_INLINE BLResult enqueue_fill_or_stroke_glyph_run(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) noexcept {

  size_t size = glyph_run->size;
  size_t glyph_data_size = IntOps::align_up(size * sizeof(uint32_t), WorkerManager::kAllocatorAlignment);
  size_t placement_data_size = IntOps::align_up(size * sizeof(BLGlyphPlacementRawData), WorkerManager::kAllocatorAlignment);

  uint32_t* glyph_data = ctx_impl->worker_mgr()._allocator.template allocNoAlignT<uint32_t>(glyph_data_size);
  BLGlyphPlacementRawData* placement_data = ctx_impl->worker_mgr()._allocator.template allocNoAlignT<BLGlyphPlacementRawData>(placement_data_size);

  if (BL_UNLIKELY(!glyph_data || !placement_data))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  BLGlyphRunIterator it(*glyph_run);
  uint32_t* dst_glyph_data = glyph_data;
  BLGlyphPlacementRawData* dst_placement_data = placement_data;

  while (!it.at_end()) {
    *dst_glyph_data++ = it.glyph_id();
    *dst_placement_data++ = it.placement<BLGlyphPlacementRawData>();
    it.advance();
  }

  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  di.add_fill_type(Pipeline::FillType::kAnalytic);

  RenderCommand* command = ctx_impl->worker_mgr->current_command();
  command->init_command(di.alpha);
  command->init_fill_analytic(nullptr, 0, BL_FILL_RULE_NON_ZERO);

  return enqueue_command_with_fill_or_stroke_job<OpType, RenderJob_TextOp>(
    ctx_impl, di, ds,
    IntOps::align_up(sizeof(RenderJob_TextOp), WorkerManager::kAllocatorAlignment), origin_fixed,
    [&](RenderJob_TextOp* job) {
      job->init_font(*font);
      job->init_glyph_run(glyph_data, placement_data, size, glyph_run->placement_type, glyph_run->flags);
    });
}

template<uint32_t OpType>
static BL_INLINE BLResult enqueue_fill_or_stroke_text(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLPoint* origin, const BLFontCore* font, const void* text, size_t size, BLTextEncoding encoding) noexcept {

  if (size == SIZE_MAX)
    size = StringOps::length_with_encoding(text, encoding);

  if (!size)
    return BL_SUCCESS;

  BLResult result = BL_SUCCESS;
  Wrap<BLGlyphBuffer> gb;

  void* serialized_text_data = nullptr;
  size_t serialized_text_size = size << text_byte_size_shift_by_encoding[encoding];

  if (serialized_text_size > BL_RASTER_CONTEXT_MAXIMUM_EMBEDDED_TEXT_SIZE) {
    gb.init();
    result = gb->set_text(text, size, encoding);
  }
  else {
    serialized_text_data = ctx_impl->worker_mgr->_allocator.alloc(IntOps::align_up(serialized_text_size, 8));
    if (!serialized_text_data)
      result = BL_ERROR_OUT_OF_MEMORY;
    else
      memcpy(serialized_text_data, text, serialized_text_size);
  }

  if (result == BL_SUCCESS) {
    BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
    di.add_fill_type(Pipeline::FillType::kAnalytic);

    RenderCommand* command = ctx_impl->worker_mgr->current_command();
    command->init_command(di.alpha);
    command->init_fill_analytic(nullptr, 0, BL_FILL_RULE_NON_ZERO);

    result = enqueue_command_with_fill_or_stroke_job<OpType, RenderJob_TextOp>(
      ctx_impl, di, ds,
      IntOps::align_up(sizeof(RenderJob_TextOp), WorkerManager::kAllocatorAlignment), origin_fixed,
      [&](RenderJob_TextOp* job) {
        job->init_font(*font);
        if (serialized_text_size > BL_RASTER_CONTEXT_MAXIMUM_EMBEDDED_TEXT_SIZE)
          job->init_glyph_buffer(gb->impl);
        else
          job->init_text_data(serialized_text_data, size, encoding);
      });
  }

  if (result != BL_SUCCESS && serialized_text_size > BL_RASTER_CONTEXT_MAXIMUM_EMBEDDED_TEXT_SIZE)
    gb.destroy();

  return result;
}

// bl::RasterEngine - ContextImpl - Internals - Fill Clipped Box
// =============================================================

template<RenderingMode kRM>
static BL_INLINE BLResult fill_clipped_box_a(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_a) noexcept;

template<>
BL_INLINE BLResult fill_clipped_box_a<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_a) noexcept {
  Pipeline::DispatchData dispatch_data;
  di.add_fill_type(Pipeline::FillType::kBoxA);
  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, &dispatch_data));

  return CommandProcSync::fill_box_a(ctx_impl->sync_work_data, dispatch_data, di.alpha, box_a, ds.fetch_data->get_pipeline_data());
}

template<>
BL_INLINE BLResult fill_clipped_box_a<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_a) noexcept {
  RenderCommand* command = ctx_impl->worker_mgr->current_command();

  di.add_fill_type(Pipeline::FillType::kBoxA);
  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, command->pipe_dispatch_data()));

  command->init_command(di.alpha);
  command->init_fill_box_a(box_a);

  uint8_t qy0 = uint8_t((box_a.y0) >> ctx_impl->command_quantization_shift_aa());
  return enqueue_command(ctx_impl, command, qy0, ds.fetch_data, [&](RenderCommand*) noexcept {});
}

template<RenderingMode kRM>
static BL_INLINE BLResult fill_clipped_box_u(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_u) noexcept;

template<>
BL_INLINE BLResult fill_clipped_box_u<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_u) noexcept {
  Pipeline::DispatchData dispatch_data;
  di.add_fill_type(Pipeline::FillType::kMask);
  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, &dispatch_data));

  return CommandProcSync::fill_box_u(ctx_impl->sync_work_data, dispatch_data, di.alpha, box_u, ds.fetch_data->get_pipeline_data());
}

template<>
BL_INLINE BLResult fill_clipped_box_u<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_u) noexcept {
  RenderCommand* command = ctx_impl->worker_mgr->current_command();

  di.add_fill_type(Pipeline::FillType::kMask);
  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, command->pipe_dispatch_data()));

  command->init_command(di.alpha);
  command->init_fill_box_u(box_u);

  uint8_t qy0 = uint8_t((box_u.y0) >> ctx_impl->command_quantization_shift_fp());
  return enqueue_command(ctx_impl, command, qy0, ds.fetch_data, [&](RenderCommand*) noexcept {});
}

template<RenderingMode kRM>
static BL_INLINE BLResult fill_clipped_box_f(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_u) noexcept;

template<>
BL_INLINE BLResult fill_clipped_box_f<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_u) noexcept {
  if (isBoxAligned24x8(box_u))
    return fill_clipped_box_a<kSync>(ctx_impl, di, ds, BLBoxI(box_u.x0 >> 8, box_u.y0 >> 8, box_u.x1 >> 8, box_u.y1 >> 8));
  else
    return fill_clipped_box_u<kSync>(ctx_impl, di, ds, box_u);
}

template<>
BL_INLINE BLResult fill_clipped_box_f<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBoxI& box_u) noexcept {
  RenderCommand* command = ctx_impl->worker_mgr->current_command();
  command->init_command(di.alpha);

  uint8_t qy0 = uint8_t(box_u.y0 >> ctx_impl->command_quantization_shift_fp());

  if (isBoxAligned24x8(box_u)) {
    di.add_fill_type(Pipeline::FillType::kBoxA);
    command->init_fill_box_a(BLBoxI(box_u.x0 >> 8, box_u.y0 >> 8, box_u.x1 >> 8, box_u.y1 >> 8));
  }
  else {
    di.add_fill_type(Pipeline::FillType::kMask);
    command->init_fill_box_u(box_u);
  }

  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, command->pipe_dispatch_data()));
  return enqueue_command(ctx_impl, command, qy0, ds.fetch_data, [&](RenderCommand*) noexcept {});
}

// bl::RasterEngine - ContextImpl - Internals - Fill All
// =====================================================

template<RenderingMode kRM>
static BL_NOINLINE BLResult fill_all(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds) noexcept {
  return ctx_impl->clip_mode() == BL_CLIP_MODE_ALIGNED_RECT
    ? fill_clipped_box_a<kRM>(ctx_impl, di, ds, ctx_impl->final_clip_box_i())
    : fill_clipped_box_u<kRM>(ctx_impl, di, ds, ctx_impl->final_clip_box_fixed_i());
}

// bl::RasterEngine - ContextImpl - Internals - Fill Clipped Edges
// ===============================================================

template<RenderingMode kRM>
static BLResult fill_clipped_edges(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLFillRule fill_rule) noexcept;

template<>
BL_NOINLINE BLResult fill_clipped_edges<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLFillRule fill_rule) noexcept {
  WorkData& work_data = ctx_impl->sync_work_data;
  EdgeStorage<int>& edge_storage = work_data.edge_storage;

  // NOTE: This doesn't happen often, but it's possible if for example the data in bands is all horizontal lines or no data at all.
  if (edge_storage.is_empty() || edge_storage.bounding_box().y0 >= edge_storage.bounding_box().y1)
    return BL_SUCCESS;

  Pipeline::DispatchData dispatch_data;
  di.add_fill_type(Pipeline::FillType::kAnalytic);
  BLResult result = ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, &dispatch_data);
  if (BL_UNLIKELY(result != BL_SUCCESS)) {
    // Must revert the edge builder if we have failed here as we cannot execute the render call.
    work_data.revert_edge_builder();
    return result;
  }

  return CommandProcSync::fill_analytic(work_data, dispatch_data, di.alpha, &edge_storage, fill_rule, ds.fetch_data->get_pipeline_data());
}

template<>
BL_NOINLINE BLResult fill_clipped_edges<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLFillRule fill_rule) noexcept {
  RenderCommand* command = ctx_impl->worker_mgr->current_command();

  WorkData& work_data = ctx_impl->sync_work_data;
  EdgeStorage<int>& edge_storage = work_data.edge_storage;

  // NOTE: This doesn't happen often, but it's possible if for example the data in bands is all horizontal lines or no data at all.
  if (edge_storage.is_empty() || edge_storage.bounding_box().y0 >= edge_storage.bounding_box().y1)
    return BL_SUCCESS;

  uint8_t qy0 = uint8_t(edge_storage.bounding_box().y0 >> ctx_impl->command_quantization_shift_fp());

  di.add_fill_type(Pipeline::FillType::kAnalytic);
  command->init_command(di.alpha);
  command->init_fill_analytic(edge_storage.flatten_edge_links(), edge_storage.bounding_box().y0, fill_rule);
  edge_storage.reset_bounding_box();

  BLResult result = ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, command->pipe_dispatch_data());
  if (BL_UNLIKELY(result != BL_SUCCESS)) {
    // Must revert the edge builder if we have failed here as we cannot execute the render call.
    work_data.revert_edge_builder();
    return result;
  }

  return enqueue_command(ctx_impl, command, qy0, ds.fetch_data, [&](RenderCommand* command) noexcept {
    command->_payload.analytic.state_slot_index = ctx_impl->worker_mgr().next_state_slot_index();
  });
}

// bl::RasterEngine - ContextImpl - Internals - Fill Unclipped Path
// ================================================================

template<RenderingMode kRM>
static BL_INLINE BLResult fill_unclipped_path(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLPath& path, BLFillRule fill_rule, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {

  if constexpr (kRM == kAsync)
    ctx_impl->sync_work_data.save_state();

  BL_PROPAGATE(add_filled_path_edges(&ctx_impl->sync_work_data, path.view(), transform, transform_type));
  return fill_clipped_edges<kRM>(ctx_impl, di, ds, fill_rule);
}

template<RenderingMode kRM>
static BL_INLINE BLResult fill_unclipped_path(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLPath& path, BLFillRule fill_rule) noexcept {

  return fill_unclipped_path<kRM>(ctx_impl, di, ds, path, fill_rule, ctx_impl->final_transform_fixed(), ctx_impl->final_transform_fixed_type());
}

template<RenderingMode kRM>
static BL_INLINE BLResult fill_unclipped_path_with_origin(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLPoint& origin_fixed, const BLPath& path, BLFillRule fill_rule) noexcept;

template<>
BL_INLINE BLResult fill_unclipped_path_with_origin<kSync>(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLPoint& origin_fixed, const BLPath& path, BLFillRule fill_rule) noexcept {
  const BLMatrix2D& ft = ctx_impl->final_transform_fixed();
  BLMatrix2D transform(ft.m00, ft.m01, ft.m10, ft.m11, origin_fixed.x, origin_fixed.y);

  BLTransformType transform_type = bl_max<BLTransformType>(ctx_impl->final_transform_fixed_type(), BL_TRANSFORM_TYPE_TRANSLATE);
  return fill_unclipped_path<kSync>(ctx_impl, di, ds, path, fill_rule, transform, transform_type);
}

template<>
BL_INLINE BLResult fill_unclipped_path_with_origin<kAsync>(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLPoint& origin_fixed, const BLPath& path, BLFillRule fill_rule) noexcept {

  if (path.size() <= BL_RASTER_CONTEXT_MINIMUM_ASYNC_PATH_SIZE) {
    const BLMatrix2D& ft = ctx_impl->final_transform_fixed();
    BLMatrix2D transform(ft.m00, ft.m01, ft.m10, ft.m11, origin_fixed.x, origin_fixed.y);

    BLTransformType transform_type = bl_max<BLTransformType>(ctx_impl->final_transform_fixed_type(), BL_TRANSFORM_TYPE_TRANSLATE);
    return fill_unclipped_path<kAsync>(ctx_impl, di, ds, path, fill_rule, transform, transform_type);
  }

  size_t job_size = sizeof(RenderJob_GeometryOp) + sizeof(BLPathCore);
  di.add_fill_type(Pipeline::FillType::kAnalytic);

  RenderCommand* command = ctx_impl->worker_mgr->current_command();
  command->init_command(di.alpha);
  command->init_fill_analytic(nullptr, 0, fill_rule);
  return enqueue_command_with_fill_job<RenderJob_GeometryOp>(ctx_impl, di, ds, job_size, origin_fixed, [&](RenderJob_GeometryOp* job) noexcept { job->set_geometry_with_path(&path); });
}

// bl::RasterEngine - ContextImpl - Internals - Fill Unclipped Polygon
// ===================================================================

template<RenderingMode kRM, typename PointType>
static BL_INLINE BLResult fill_unclipped_polygon_t(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const PointType* pts, size_t size, BLFillRule fill_rule, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {

  if constexpr (kRM == kAsync)
    ctx_impl->sync_work_data.save_state();

  BL_PROPAGATE(add_filled_polygon_edges(&ctx_impl->sync_work_data, pts, size, transform, transform_type));
  return fill_clipped_edges<kRM>(ctx_impl, di, ds, fill_rule);
}

template<RenderingMode kRM, typename PointType>
static BL_INLINE BLResult fill_unclipped_polygon_t(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const PointType* pts, size_t size, BLFillRule fill_rule) noexcept {

  return fill_unclipped_polygon_t<kRM>(ctx_impl, di, ds, pts, size, fill_rule, ctx_impl->final_transform_fixed(), ctx_impl->final_transform_fixed_type());
}

// bl::RasterEngine - ContextImpl - Internals - Fill Unclipped Box & Rect
// ======================================================================

template<RenderingMode kRM>
static BL_INLINE BLResult fill_unclipped_box_d(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLBox& box_d, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {

  if (transform_type <= BL_TRANSFORM_TYPE_SWAP) {
    BLBox finalBoxD;
    if (!Geometry::intersect(finalBoxD, TransformInternal::map_box_scaled_swapped(transform, box_d), ctx_impl->final_clip_box_fixed_d()))
      return BL_SUCCESS;

    BLBoxI box_u = Math::trunc_to_int(finalBoxD);
    if (box_u.x0 >= box_u.x1 || box_u.y0 >= box_u.y1)
      return BL_SUCCESS;

    return fill_clipped_box_f<kRM>(ctx_impl, di, ds, box_u);
  }
  else {
    BLPoint poly_d[] = {BLPoint(box_d.x0, box_d.y0), BLPoint(box_d.x1, box_d.y0), BLPoint(box_d.x1, box_d.y1), BLPoint(box_d.x0, box_d.y1)};
    return fill_unclipped_polygon_t<kRM>(ctx_impl, di, ds, poly_d, BL_ARRAY_SIZE(poly_d), BL_RASTER_CONTEXT_PREFERRED_FILL_RULE, transform, transform_type);
  }
}

template<RenderingMode kRM>
static BL_INLINE BLResult fill_unclipped_box_d(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLBox& box_d) noexcept {
  return fill_unclipped_box_d<kRM>(ctx_impl, di, ds, box_d, ctx_impl->final_transform_fixed(), ctx_impl->final_transform_fixed_type());
}

template<RenderingMode kRM>
static BL_INLINE BLResult fillUnclippedRectI(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLRectI& rect_i) noexcept {
  int rw = rect_i.w;
  int rh = rect_i.h;

  if (!bl_test_flag(ctx_impl->context_flags, ContextFlags::kInfoIntegralTranslation)) {
    // Clipped out.
    if ((rw <= 0) | (rh <= 0))
      return BL_SUCCESS;

    BLBox box_d(double(rect_i.x), double(rect_i.y), double(rect_i.x) + double(rect_i.w), double(rect_i.y) + double(rect_i.h));
    return fill_unclipped_box_d<kRM>(ctx_impl, di, ds, box_d);
  }

  BLBoxI dstBoxI;
  if (!translateAndClipRectToFillI(ctx_impl, &rect_i, &dstBoxI))
    return BL_SUCCESS;

  return fill_clipped_box_a<kRM>(ctx_impl, di, ds, dstBoxI);
}

// bl::RasterEngine - ContextImpl - Internals - Fill Unclipped Geometry
// ====================================================================

template<RenderingMode kRM>
static BLResult fill_unclipped_geometry(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLGeometryType type, const void* data) noexcept;

template<>
BLResult BL_NOINLINE fill_unclipped_geometry<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLGeometryType type, const void* data) noexcept {
  // The most common primary geometry operation would be rendering rectangles - so check these first.
  if (type <= BL_GEOMETRY_TYPE_RECTD) {
    BLBox temporary_box;

    if (type == BL_GEOMETRY_TYPE_RECTI)
      return fillUnclippedRectI<kSync>(ctx_impl, di, ds, *static_cast<const BLRectI*>(data));

    if (type == BL_GEOMETRY_TYPE_RECTD) {
      const BLRect* r = static_cast<const BLRect*>(data);
      temporary_box.reset(r->x, r->y, r->x + r->w, r->y + r->h);
      data = &temporary_box;
    }
    else if (type == BL_GEOMETRY_TYPE_BOXI) {
      const BLBoxI* box_i = static_cast<const BLBoxI*>(data);
      temporary_box.reset(double(box_i->x0), double(box_i->y0), double(box_i->x1), double(box_i->y1));
      data = &temporary_box;
    }
    else if (type == BL_GEOMETRY_TYPE_NONE) {
      return BL_SUCCESS;
    }

    return fill_unclipped_box_d<kSync>(ctx_impl, di, ds, *static_cast<const BLBox*>(data));
  }

  // The most common second geometry operation would be rendering paths.
  if (type != BL_GEOMETRY_TYPE_PATH) {
    if (type == BL_GEOMETRY_TYPE_POLYGONI || type == BL_GEOMETRY_TYPE_POLYLINEI) {
      const BLArrayView<BLPointI>* array = static_cast<const BLArrayView<BLPointI>*>(data);
      if (BL_UNLIKELY(array->size < 3))
        return BL_SUCCESS;

      return fill_unclipped_polygon_t<kSync>(ctx_impl, di, ds, array->data, array->size, ctx_impl->fill_rule());
    }

    if (type == BL_GEOMETRY_TYPE_POLYGOND || type == BL_GEOMETRY_TYPE_POLYLINED) {
      const BLArrayView<BLPoint>* array = static_cast<const BLArrayView<BLPoint>*>(data);
      if (BL_UNLIKELY(array->size < 3))
        return BL_SUCCESS;

      return fill_unclipped_polygon_t<kSync>(ctx_impl, di, ds, array->data, array->size, ctx_impl->fill_rule());
    }

    BLPath* temporary_path = &ctx_impl->sync_work_data.tmp_path[3];
    temporary_path->clear();
    BL_PROPAGATE(temporary_path->add_geometry(type, data));
    data = temporary_path;
  }

  const BLPath* path = static_cast<const BLPath*>(data);
  if (BL_UNLIKELY(path->is_empty()))
    return BL_SUCCESS;

  return fill_unclipped_path<kSync>(ctx_impl, di, ds, *path, ctx_impl->fill_rule());
}

template<>
BLResult BL_NOINLINE fill_unclipped_geometry<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLGeometryType type, const void* data) noexcept {
  if (type <= BL_GEOMETRY_TYPE_RECTD) {
    BLBox temporary_box;

    if (type == BL_GEOMETRY_TYPE_RECTI)
      return fillUnclippedRectI<kAsync>(ctx_impl, di, ds, *static_cast<const BLRectI*>(data));

    if (type == BL_GEOMETRY_TYPE_RECTD) {
      const BLRect* r = static_cast<const BLRect*>(data);
      temporary_box.reset(r->x, r->y, r->x + r->w, r->y + r->h);
      data = &temporary_box;
    }
    else if (type == BL_GEOMETRY_TYPE_BOXI) {
      const BLBoxI* box_i = static_cast<const BLBoxI*>(data);
      temporary_box.reset(double(box_i->x0), double(box_i->y0), double(box_i->x1), double(box_i->y1));
      data = &temporary_box;
    }
    else if (type == BL_GEOMETRY_TYPE_NONE) {
      return BL_SUCCESS;
    }

    return fill_unclipped_box_d<kAsync>(ctx_impl, di, ds, *static_cast<const BLBox*>(data));
  }

  BLFillRule fill_rule = ctx_impl->fill_rule();

  switch (type) {
    case BL_GEOMETRY_TYPE_POLYGONI:
    case BL_GEOMETRY_TYPE_POLYLINEI: {
      const BLArrayView<BLPointI>* array = static_cast<const BLArrayView<BLPointI>*>(data);
      if (BL_UNLIKELY(array->size < 3))
        return BL_SUCCESS;
      return fill_unclipped_polygon_t<kAsync>(ctx_impl, di, ds, array->data, array->size, fill_rule);
    }

    case BL_GEOMETRY_TYPE_POLYGOND:
    case BL_GEOMETRY_TYPE_POLYLINED: {
      const BLArrayView<BLPoint>* array = static_cast<const BLArrayView<BLPoint>*>(data);
      if (BL_UNLIKELY(array->size < 3))
        return BL_SUCCESS;
      return fill_unclipped_polygon_t<kAsync>(ctx_impl, di, ds, array->data, array->size, fill_rule);
    }

    case BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI:
    case BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD:
    case BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI:
    case BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD: {
      BLPath* temporary_path = &ctx_impl->sync_work_data.tmp_path[3];
      temporary_path->clear();
      BL_PROPAGATE(temporary_path->add_geometry(type, data));

      type = BL_GEOMETRY_TYPE_PATH;
      data = temporary_path;

      [[fallthrough]];
    }

    case BL_GEOMETRY_TYPE_PATH: {
      const BLPath* path = static_cast<const BLPath*>(data);
      if (path->size() <= BL_RASTER_CONTEXT_MINIMUM_ASYNC_PATH_SIZE)
        return fill_unclipped_path<kAsync>(ctx_impl, di, ds, *path, fill_rule);

      size_t job_size = sizeof(RenderJob_GeometryOp) + sizeof(BLPathCore);
      BLPoint origin_fixed(ctx_impl->final_transform_fixed().m20, ctx_impl->final_transform_fixed().m21);

      di.add_fill_type(Pipeline::FillType::kAnalytic);

      RenderCommand* command = ctx_impl->worker_mgr->current_command();
      command->init_command(di.alpha);
      command->init_fill_analytic(nullptr, 0, fill_rule);
      return enqueue_command_with_fill_job<RenderJob_GeometryOp>(ctx_impl, di, ds, job_size, origin_fixed, [&](RenderJob_GeometryOp* job) noexcept { job->set_geometry_with_path(path); });
    }

    default: {
      if (!Geometry::is_simple_geometry_type(type))
        return bl_make_error(BL_ERROR_INVALID_VALUE);

      size_t geometry_size = Geometry::geometry_type_size_table[type];
      size_t job_size = sizeof(RenderJob_GeometryOp) + geometry_size;
      BLPoint origin_fixed(ctx_impl->final_transform_fixed().m20, ctx_impl->final_transform_fixed().m21);

      di.add_fill_type(Pipeline::FillType::kAnalytic);

      RenderCommand* command = ctx_impl->worker_mgr->current_command();
      command->init_command(di.alpha);
      command->init_fill_analytic(nullptr, 0, fill_rule);

      return enqueue_command_with_fill_job<RenderJob_GeometryOp>(ctx_impl, di, ds, job_size, origin_fixed, [&](RenderJob_GeometryOp* job) noexcept { job->set_geometry_with_shape(type, data, geometry_size); });
    }
  }
}

// bl::RasterEngine - ContextImpl - Internals - Fill Unclipped Text
// ================================================================

template<RenderingMode kRM>
static BLResult fill_unclipped_text(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* data) noexcept;

template<>
BL_NOINLINE BLResult fill_unclipped_text<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* data) noexcept {
  const BLGlyphRun* glyph_run = nullptr;

  if (op_type <= BLContextRenderTextOp(BL_TEXT_ENCODING_MAX_VALUE)) {
    BLTextEncoding encoding = static_cast<BLTextEncoding>(op_type);
    const BLDataView* view = static_cast<const BLDataView*>(data);

    BLGlyphBuffer& gb = ctx_impl->sync_work_data.glyph_buffer;
    BL_PROPAGATE(gb.set_text(view->data, view->size, encoding));
    BL_PROPAGATE(font->dcast().shape(gb));
    glyph_run = &gb.glyph_run();
  }
  else if (op_type == BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN) {
    glyph_run = static_cast<const BLGlyphRun*>(data);
  }
  else {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  if (glyph_run->is_empty())
    return BL_SUCCESS;

  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  WorkData* work_data = &ctx_impl->sync_work_data;

  BL_PROPAGATE(add_filled_glyph_run_edges(work_data, DirectStateAccessor(ctx_impl), origin_fixed, font, glyph_run));
  return fill_clipped_edges<kSync>(ctx_impl, di, ds, BL_FILL_RULE_NON_ZERO);
}

template<>
BL_NOINLINE BLResult fill_unclipped_text<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* data) noexcept {
  if (op_type <= BLContextRenderTextOp(BL_TEXT_ENCODING_MAX_VALUE)) {
    const BLDataView* view = static_cast<const BLDataView*>(data);
    BLTextEncoding encoding = static_cast<BLTextEncoding>(op_type);

    if (view->size == 0)
      return BL_SUCCESS;

    return enqueue_fill_or_stroke_text<BL_CONTEXT_STYLE_SLOT_FILL>(ctx_impl, di, ds, origin, font, view->data, view->size, encoding);
  }
  else if (op_type == BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN) {
    const BLGlyphRun* glyph_run = static_cast<const BLGlyphRun*>(data);
    return enqueue_fill_or_stroke_glyph_run<BL_CONTEXT_STYLE_SLOT_FILL>(ctx_impl, di, ds, origin, font, glyph_run);
  }
  else {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }
}

// bl::RasterEngine - ContextImpl - Internals - Fill Mask
// ======================================================

template<RenderingMode kRM>
static BLResult fill_clipped_box_masked_a(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLBoxI& box_a, const BLImageCore* mask, const BLPointI& mask_offset_i) noexcept;

template<>
BL_NOINLINE BLResult fill_clipped_box_masked_a<kSync>(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLBoxI& box_a, const BLImageCore* mask, const BLPointI& mask_offset_i) noexcept {

  Pipeline::DispatchData dispatch_data;

  di.add_fill_type(Pipeline::FillType::kMask);
  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, &dispatch_data));

  RenderCommand::FillBoxMaskA payload;
  payload.mask_image_i.ptr = ImageInternal::get_impl(mask);
  payload.mask_offset_i = mask_offset_i;
  payload.box_i = box_a;
  return CommandProcSync::fill_box_masked_a(ctx_impl->sync_work_data, dispatch_data, di.alpha, payload, ds.fetch_data->get_pipeline_data());
}

template<>
BL_NOINLINE BLResult fill_clipped_box_masked_a<kAsync>(
    BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds,
    const BLBoxI& box_a, const BLImageCore* mask, const BLPointI& mask_offset_i) noexcept {

  RenderCommand* command = ctx_impl->worker_mgr->current_command();

  di.add_fill_type(Pipeline::FillType::kMask);
  BL_PROPAGATE(ensure_fetch_and_dispatch_data(ctx_impl, di.signature, ds.fetch_data, command->pipe_dispatch_data()));

  command->init_command(di.alpha);
  command->init_fill_box_mask_a(box_a, mask, mask_offset_i);

  uint8_t qy0 = uint8_t(box_a.y0 >> ctx_impl->command_quantization_shift_aa());

  return enqueue_command(ctx_impl, command, qy0, ds.fetch_data, [&](RenderCommand* command) noexcept {
    ObjectInternal::retain_impl<RCMode::kMaybe>(command->_payload.box_mask_a.mask_image_i.ptr);
  });
}

// bl::RasterEngine - ContextImpl - Internals - Stroke Unclipped Path
// ==================================================================

template<RenderingMode kRM>
static BLResult stroke_unclipped_path(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint& origin_fixed, const BLPath& path) noexcept;

template<>
BL_NOINLINE BLResult stroke_unclipped_path<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint& origin_fixed, const BLPath& path) noexcept {
  WorkData* work_data = &ctx_impl->sync_work_data;
  BL_PROPAGATE(add_stroked_path_edges(work_data, DirectStateAccessor(ctx_impl), origin_fixed, &path));

  return fill_clipped_edges<kSync>(ctx_impl, di, ds, BL_FILL_RULE_NON_ZERO);
}

template<>
BL_NOINLINE BLResult stroke_unclipped_path<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint& origin_fixed, const BLPath& path) noexcept {
  size_t job_size = sizeof(RenderJob_GeometryOp) + sizeof(BLPathCore);
  di.add_fill_type(Pipeline::FillType::kAnalytic);

  RenderCommand* command = ctx_impl->worker_mgr->current_command();
  command->init_command(di.alpha);
  command->init_fill_analytic(nullptr, 0, BL_FILL_RULE_NON_ZERO);

  return enqueue_command_with_stroke_job<RenderJob_GeometryOp>(ctx_impl, di, ds, job_size, origin_fixed, [&](RenderJob_GeometryOp* job) noexcept {
    job->set_geometry_with_path(&path);
  });
}

// bl::RasterEngine - ContextImpl - Internals - Stroke Unclipped Geometry
// ======================================================================

template<RenderingMode kRM>
static BLResult stroke_unclipped_geometry(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLGeometryType type, const void* data) noexcept;

template<>
BL_NOINLINE BLResult stroke_unclipped_geometry<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLGeometryType type, const void* data) noexcept {
  WorkData* work_data = &ctx_impl->sync_work_data;
  BLPath* path = const_cast<BLPath*>(static_cast<const BLPath*>(data));

  if (type != BL_GEOMETRY_TYPE_PATH) {
    path = &work_data->tmp_path[3];
    path->clear();
    BL_PROPAGATE(path->add_geometry(type, data));
  }

  BLPoint origin_fixed(ctx_impl->final_transform_fixed().m20, ctx_impl->final_transform_fixed().m21);
  BL_PROPAGATE(add_stroked_path_edges(work_data, DirectStateAccessor(ctx_impl), origin_fixed, path));

  return fill_clipped_edges<kSync>(ctx_impl, di, ds, BL_FILL_RULE_NON_ZERO);
}

template<>
BL_NOINLINE BLResult stroke_unclipped_geometry<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLGeometryType type, const void* data) noexcept {
  size_t geometry_size = sizeof(BLPathCore);
  if (Geometry::is_simple_geometry_type(type)) {
    geometry_size = Geometry::geometry_type_size_table[type];
  }
  else if (type != BL_GEOMETRY_TYPE_PATH) {
    BLPath* temporary_path = &ctx_impl->sync_work_data.tmp_path[3];

    temporary_path->clear();
    BL_PROPAGATE(temporary_path->add_geometry(type, data));

    type = BL_GEOMETRY_TYPE_PATH;
    data = temporary_path;
  }

  size_t job_size = sizeof(RenderJob_GeometryOp) + geometry_size;
  BLPoint origin_fixed(ctx_impl->final_transform_fixed().m20, ctx_impl->final_transform_fixed().m21);

  di.add_fill_type(Pipeline::FillType::kAnalytic);

  RenderCommand* command = ctx_impl->worker_mgr->current_command();
  command->init_command(di.alpha);
  command->init_fill_analytic(nullptr, 0, BL_FILL_RULE_NON_ZERO);

  return enqueue_command_with_stroke_job<RenderJob_GeometryOp>(ctx_impl, di, ds, job_size, origin_fixed, [&](RenderJob_GeometryOp* job) noexcept {
    job->set_geometry(type, data, geometry_size);
  });
}

// bl::RasterEngine - ContextImpl - Internals - Stroke Unclipped Text
// ==================================================================

template<RenderingMode kRM>
static BLResult stroke_unclipped_text(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* data) noexcept;

template<>
BL_NOINLINE BLResult stroke_unclipped_text<kSync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* data) noexcept {
  const BLGlyphRun* glyph_run = nullptr;

  if (op_type <= BLContextRenderTextOp(BL_TEXT_ENCODING_MAX_VALUE)) {
    BLTextEncoding encoding = static_cast<BLTextEncoding>(op_type);
    const BLDataView* view = static_cast<const BLDataView*>(data);

    BLGlyphBuffer& gb = ctx_impl->sync_work_data.glyph_buffer;
    BL_PROPAGATE(gb.set_text(view->data, view->size, encoding));
    BL_PROPAGATE(font->dcast().shape(gb));
    glyph_run = &gb.glyph_run();
  }
  else if (op_type == BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN) {
    glyph_run = static_cast<const BLGlyphRun*>(data);
  }
  else {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  if (glyph_run->is_empty())
    return BL_SUCCESS;

  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  WorkData* work_data = &ctx_impl->sync_work_data;

  BL_PROPAGATE(add_stroked_glyph_run_edges(work_data, DirectStateAccessor(ctx_impl), origin_fixed, font, glyph_run));
  return fill_clipped_edges<kSync>(ctx_impl, di, ds, BL_FILL_RULE_NON_ZERO);
}

template<>
BL_NOINLINE BLResult stroke_unclipped_text<kAsync>(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* data) noexcept {
  if (op_type <= BLContextRenderTextOp(BL_TEXT_ENCODING_MAX_VALUE)) {
    const BLDataView* view = static_cast<const BLDataView*>(data);
    BLTextEncoding encoding = static_cast<BLTextEncoding>(op_type);

    if (view->size == 0)
      return BL_SUCCESS;

    return enqueue_fill_or_stroke_text<BL_CONTEXT_STYLE_SLOT_STROKE>(ctx_impl, di, ds, origin, font, view->data, view->size, encoding);
  }
  else if (op_type == BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN) {
    const BLGlyphRun* glyph_run = static_cast<const BLGlyphRun*>(data);
    return enqueue_fill_or_stroke_glyph_run<BL_CONTEXT_STYLE_SLOT_STROKE>(ctx_impl, di, ds, origin, font, glyph_run);
  }
  else {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }
}

// bl::RasterEngine - ContextImpl - Frontend - Clear All
// =====================================================

template<RenderingMode kRM>
static BLResult BL_CDECL clear_all_impl(BLContextImpl* base_impl) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_CLEAR_OP(ContextFlags::kNoClearOpAll);
  return fill_all<kRM>(ctx_impl, di, ds);
}

// bl::RasterEngine - ContextImpl - Frontend - Clear Rect
// ======================================================

template<RenderingMode kRM>
static BLResult BL_CDECL clear_rect_i_impl(BLContextImpl* base_impl, const BLRectI* rect) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_CLEAR_OP(ContextFlags::kNoClearOp);
  return fillUnclippedRectI<kRM>(ctx_impl, di, ds, *rect);
}

template<RenderingMode kRM>
static BLResult BL_CDECL clear_rect_d_impl(BLContextImpl* base_impl, const BLRect* rect) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_CLEAR_OP(ContextFlags::kNoClearOp);
  BLBox box_d(rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
  return fill_unclipped_box_d<kRM>(ctx_impl, di, ds, box_d);
}

// bl::RasterEngine - ContextImpl - Frontend - Fill All
// ====================================================

template<RenderingMode kRM>
static BLResult BL_CDECL fill_all_impl(BLContextImpl* base_impl) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpAllImplicit, BL_CONTEXT_STYLE_SLOT_FILL, kNoBail);
  return fill_all<kRM>(ctx_impl, di, ds);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_all_rgba32_impl(BLContextImpl* base_impl, uint32_t rgba32) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpAllExplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, kNoBail);
  return fill_all<kRM>(ctx_impl, di, ds);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_all_ext_impl(BLContextImpl* base_impl, const BLObjectCore* style) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpAllExplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, kNoBail);
  BLResult result = fill_all<kRM>(ctx_impl, di, ds);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

// bl::RasterEngine - ContextImpl - Frontend - Fill Rect
// =====================================================

template<RenderingMode kRM>
static BLResult BL_CDECL fill_rect_i_impl(BLContextImpl* base_impl, const BLRectI* rect) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, kNoBail);
  return fillUnclippedRectI<kRM>(ctx_impl, di, ds, *rect);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_rect_i_rgba32_impl(BLContextImpl* base_impl, const BLRectI* rect, uint32_t rgba32) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, kNoBail);
  return fillUnclippedRectI<kRM>(ctx_impl, di, ds, *rect);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_rect_i_ext_impl(BLContextImpl* base_impl, const BLRectI* rect, const BLObjectCore* style) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, kNoBail);
  BLResult result = fillUnclippedRectI<kRM>(ctx_impl, di, ds, *rect);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_rect_d_impl(BLContextImpl* base_impl, const BLRect* rect) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, kNoBail);
  BLBox box_d(rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
  return fill_unclipped_box_d<kRM>(ctx_impl, di, ds, box_d);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_rect_d_rgba32_impl(BLContextImpl* base_impl, const BLRect* rect, uint32_t rgba32) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, kNoBail);
  BLBox box_d(rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
  return fill_unclipped_box_d<kRM>(ctx_impl, di, ds, box_d);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_rect_d_ext_impl(BLContextImpl* base_impl, const BLRect* rect, const BLObjectCore* style) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, kNoBail);
  BLBox box_d(rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
  BLResult result = fill_unclipped_box_d<kRM>(ctx_impl, di, ds, box_d);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

// bl::RasterEngine - ContextImpl - Frontend - Fill Path
// =====================================================

template<RenderingMode kRM>
static BLResult BL_CDECL fill_path_d_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLPathCore* path) noexcept {
  BL_ASSERT(path->_d.is_path());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = path->dcast().is_empty();
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, bail);
  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  return fill_unclipped_path_with_origin<kRM>(ctx_impl, di, ds, origin_fixed, path->dcast(), ctx_impl->fill_rule());
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_path_d_rgba32_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) noexcept {
  BL_ASSERT(path->_d.is_path());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = path->dcast().is_empty();
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, bail);
  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  return fill_unclipped_path_with_origin<kRM>(ctx_impl, di, ds, origin_fixed, path->dcast(), ctx_impl->fill_rule());
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_path_d_ext_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLPathCore* path, const BLObjectCore* style) noexcept {
  BL_ASSERT(path->_d.is_path());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = path->dcast().is_empty();
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, bail);
  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  BLResult result = fill_unclipped_path_with_origin<kRM>(ctx_impl, di, ds, origin_fixed, path->dcast(), ctx_impl->fill_rule());

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

// bl::RasterEngine - ContextImpl - Frontend - Fill Geometry
// =========================================================

template<RenderingMode kRM>
static BLResult BL_CDECL fill_geometry_impl(BLContextImpl* base_impl, BLGeometryType type, const void* data) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, kNoBail);
  return fill_unclipped_geometry<kRM>(ctx_impl, di, ds, type, data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_geometry_rgba32_impl(BLContextImpl* base_impl, BLGeometryType type, const void* data, uint32_t rgba32) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, kNoBail);
  return fill_unclipped_geometry<kRM>(ctx_impl, di, ds, type, data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_geometry_ext_impl(BLContextImpl* base_impl, BLGeometryType type, const void* data, const BLObjectCore* style) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, kNoBail);
  BLResult result = fill_unclipped_geometry<kRM>(ctx_impl, di, ds, type, data);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

// bl::RasterEngine - ContextImpl - Frontend - Fill Unclipped Text
// ===============================================================

template<RenderingMode kRM>
static BLResult BL_CDECL fill_text_op_d_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = !font->dcast().is_valid();
  BLResult bail_result = bail ? bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED) : BLResult(BL_SUCCESS);

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, bail);
  return fill_unclipped_text<kRM>(ctx_impl, di, ds, origin, font, op_type, op_data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_text_op_i_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLPoint origin_d(*origin);
  return fill_text_op_d_impl<kRM>(base_impl, &origin_d, font, op_type, op_data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_text_op_d_rgba32_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, uint32_t rgba32) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = !font->dcast().is_valid();
  BLResult bail_result = bail ? bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED) : BLResult(BL_SUCCESS);

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, bail);
  return fill_unclipped_text<kRM>(ctx_impl, di, ds, origin, font, op_type, op_data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_text_op_d_ext_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, const BLObjectCore* style) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = !font->dcast().is_valid();
  BLResult bail_result = bail ? bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED) : BLResult(BL_SUCCESS);

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpExplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, bail);
  BLResult result = fill_unclipped_text<kRM>(ctx_impl, di, ds, origin, font, op_type, op_data);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_text_op_i_rgba32_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, uint32_t rgba32) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLPoint origin_d(*origin);
  return fill_text_op_d_rgba32_impl<kRM>(base_impl, &origin_d, font, op_type, op_data, rgba32);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_text_op_i_ext_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, const BLObjectCore* style) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLPoint origin_d(*origin);
  return fill_text_op_d_ext_impl<kRM>(base_impl, &origin_d, font, op_type, op_data, style);
}

// bl::RasterEngine - ContextImpl - Frontend - Fill Mask
// =====================================================

template<RenderingMode kRM>
static BL_INLINE BLResult fill_unclipped_mask_d(BLRasterContextImpl* ctx_impl, DispatchInfo di, DispatchStyle ds, BLPoint dst, const BLImageCore* mask, BLRectI mask_rect) noexcept {
  if (ctx_impl->final_transform_type() <= BL_TRANSFORM_TYPE_TRANSLATE) {
    double startX = dst.x * ctx_impl->final_transform_fixed().m00 + ctx_impl->final_transform_fixed().m20;
    double startY = dst.y * ctx_impl->final_transform_fixed().m11 + ctx_impl->final_transform_fixed().m21;

    BLBox dst_box_d(bl_max(startX, ctx_impl->final_clip_box_fixed_d().x0),
                    bl_max(startY, ctx_impl->final_clip_box_fixed_d().y0),
                    bl_min(startX + double(mask_rect.w) * ctx_impl->final_transform_fixed().m00, ctx_impl->final_clip_box_fixed_d().x1),
                    bl_min(startY + double(mask_rect.h) * ctx_impl->final_transform_fixed().m11, ctx_impl->final_clip_box_fixed_d().y1));

    // Clipped out, invalid coordinates, or empty `mask_area`.
    if (!((dst_box_d.x0 < dst_box_d.x1) & (dst_box_d.y0 < dst_box_d.y1)))
      return BL_SUCCESS;

    int64_t start_fx = Math::floor_to_int64(startX);
    int64_t start_fy = Math::floor_to_int64(startY);

    BLBoxI dst_box_u = Math::trunc_to_int(dst_box_d);

    if (!((start_fx | start_fy) & ctx_impl->render_target_info.fpMaskI)) {
      // Pixel aligned mask.
      int x0 = dst_box_u.x0 >> ctx_impl->render_target_info.fpShiftI;
      int y0 = dst_box_u.y0 >> ctx_impl->render_target_info.fpShiftI;
      int x1 = (dst_box_u.x1 + ctx_impl->render_target_info.fpMaskI) >> ctx_impl->render_target_info.fpShiftI;
      int y1 = (dst_box_u.y1 + ctx_impl->render_target_info.fpMaskI) >> ctx_impl->render_target_info.fpShiftI;

      int tx = int(start_fx >> ctx_impl->render_target_info.fpShiftI);
      int ty = int(start_fy >> ctx_impl->render_target_info.fpShiftI);

      mask_rect.x += x0 - tx;
      mask_rect.y += y0 - ty;
      mask_rect.w = x1 - x0;
      mask_rect.h = y1 - y0;

      // Pixel aligned fill with a pixel aligned mask.
      if (isBoxAligned24x8(dst_box_u)) {
        return fill_clipped_box_masked_a<kRM>(ctx_impl, di, ds, BLBoxI(x0, y0, x1, y1), mask, BLPointI(mask_rect.x, mask_rect.y));
      }

      // TODO: [Rendering Context] Masking support.
      /*
      BL_PROPAGATE(serializer.init_fetch_data_for_mask(ctx_impl));
      serializer.mask_fetch_data()->init_image_source(mask, mask_rect);
      if (!serializer.mask_fetch_data()->setup_pattern_blit(x0, y0))
        return BL_SUCCESS;
      */
    }
    else {
      // TODO: [Rendering Context] Masking support.
      /*
      BL_PROPAGATE(serializer.init_fetch_data_for_mask(ctx_impl));
      serializer.mask_fetch_data()->init_image_source(mask, mask_rect);
      if (!serializer.mask_fetch_data()->setup_pattern_fx_fy(BL_RASTER_CONTEXT_PREFERRED_BLIT_EXTEND, ctx_impl->hints().pattern_quality, start_fx, start_fy))
        return BL_SUCCESS;
      */
    }

    /*
    return fill_clipped_box_u<RenderCommandSerializerFlags::kMask>(ctx_impl, serializer, dst_box_u);
    */
  }

  return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);

  // TODO: [Rendering Context] Masking support.
  /*
  else {
    BLMatrix2D m(ctx_impl->final_transform());
    m.translate(dst.x, dst.y);

    BL_PROPAGATE(serializer.init_fetch_data_for_mask(ctx_impl));
    serializer.mask_fetch_data()->init_image_source(mask, mask_rect);
    if (!serializer.mask_fetch_data()->setup_pattern_affine(BL_RASTER_CONTEXT_PREFERRED_BLIT_EXTEND, ctx_impl->hints().pattern_quality, m)) {
      serializer.rollback_fetch_data(ctx_impl);
      return BL_SUCCESS;
    }
  }

  BLBox final_box(dst.x, dst.y, dst.x + double(mask_rect.w), dst.y + double(mask_rect.h));
  return bl_raster_context_impl_finalize_blit(ctx_impl, serializer,
         fill_unclipped_box<RenderCommandSerializerFlags::kMask>(ctx_impl, serializer, final_box));
  */
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_mask_d_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area) noexcept {
  BL_ASSERT(mask->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  BLRectI mask_rect;
  BLResult bail_result = check_image_area(mask_rect, ImageInternal::get_impl(mask), mask_area);
  bool bail = bail_result != BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, bail);
  return fill_unclipped_mask_d<kRM>(ctx_impl, di, ds, *origin, mask, mask_rect);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_mask_d_rgba32_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) noexcept {
  BL_ASSERT(mask->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  BLRectI mask_rect;
  BLResult bail_result = check_image_area(mask_rect, ImageInternal::get_impl(mask), mask_area);
  bool bail = bail_result != BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, bail);
  return fill_unclipped_mask_d<kRM>(ctx_impl, di, ds, *origin, mask, mask_rect);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_mask_d_ext_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLObjectCore* style) noexcept {
  BL_ASSERT(mask->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  BLRectI mask_rect;
  BLResult bail_result = check_image_area(mask_rect, ImageInternal::get_impl(mask), mask_area);
  bool bail = bail_result != BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, bail);
  BLResult result = fill_unclipped_mask_d<kRM>(ctx_impl, di, ds, *origin, mask, mask_rect);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_mask_i_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area) noexcept {
  BL_ASSERT(mask->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  if (!bl_test_flag(ctx_impl->context_flags, ContextFlags::kInfoIntegralTranslation)) {
    BLPoint origin_d(*origin);
    return fill_mask_d_impl<kRM>(ctx_impl, &origin_d, mask, mask_area);
  }

  BLImageImpl* mask_impl = ImageInternal::get_impl(mask);

  BLBoxI dst_box;
  BLPointI src_offset;

  BLResult bail_result;
  bool bail = !translate_and_clip_rect_to_blit_i(ctx_impl, origin, mask_area, &mask_impl->size, &bail_result, &dst_box, &src_offset);

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, bail);
  return fill_clipped_box_masked_a<kRM>(ctx_impl, di, ds, dst_box, mask, src_offset);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_mask_i_rgba32_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) noexcept {
  BL_ASSERT(mask->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  if (!bl_test_flag(ctx_impl->context_flags, ContextFlags::kInfoIntegralTranslation)) {
    BLPoint origin_d(*origin);
    return fill_mask_d_rgba32_impl<kRM>(ctx_impl, &origin_d, mask, mask_area, rgba32);
  }

  BLImageImpl* mask_impl = ImageInternal::get_impl(mask);

  BLBoxI dst_box;
  BLPointI src_offset;

  BLResult bail_result;
  bool bail = !translate_and_clip_rect_to_blit_i(ctx_impl, origin, mask_area, &mask_impl->size, &bail_result, &dst_box, &src_offset);

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, rgba32, bail);
  return fill_clipped_box_masked_a<kRM>(ctx_impl, di, ds, dst_box, mask, src_offset);
}

template<RenderingMode kRM>
static BLResult BL_CDECL fill_mask_i_ext_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLObjectCore* style) noexcept {
  BL_ASSERT(mask->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  if (!bl_test_flag(ctx_impl->context_flags, ContextFlags::kInfoIntegralTranslation)) {
    BLPoint origin_d(*origin);
    return fill_mask_d_ext_impl<kRM>(ctx_impl, &origin_d, mask, mask_area, style);
  }

  BLImageImpl* mask_impl = ImageInternal::get_impl(mask);

  BLBoxI dst_box;
  BLPointI src_offset;

  BLResult bail_result;
  bool bail = !translate_and_clip_rect_to_blit_i(ctx_impl, origin, mask_area, &mask_impl->size, &bail_result, &dst_box, &src_offset);

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoFillOpImplicit, BL_CONTEXT_STYLE_SLOT_FILL, style, bail);
  BLResult result = fill_clipped_box_masked_a<kRM>(ctx_impl, di, ds, dst_box, mask, src_offset);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

// bl::RasterEngine - ContextImpl - Frontend - Stroke Geometry
// ===========================================================

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_geometry_impl(BLContextImpl* base_impl, BLGeometryType type, const void* data) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoStrokeOpImplicit, BL_CONTEXT_STYLE_SLOT_STROKE, kNoBail);
  return stroke_unclipped_geometry<kRM>(ctx_impl, di, ds, type, data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_geometry_rgba32_impl(BLContextImpl* base_impl, BLGeometryType type, const void* data, uint32_t rgba32) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoStrokeOpExplicit, BL_CONTEXT_STYLE_SLOT_STROKE, rgba32, kNoBail);
  return stroke_unclipped_geometry<kRM>(ctx_impl, di, ds, type, data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_geometry_ext_impl(BLContextImpl* base_impl, BLGeometryType type, const void* data, const BLObjectCore* style) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoStrokeOpExplicit, BL_CONTEXT_STYLE_SLOT_STROKE, style, kNoBail);
  BLResult result = stroke_unclipped_geometry<kRM>(ctx_impl, di, ds, type, data);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

// bl::RasterEngine - ContextImpl - Frontend - Stroke Path
// =======================================================

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_path_d_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLPathCore* path) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  BL_ASSERT(path->_d.is_path());

  bool bail = path->dcast().is_empty();
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoStrokeOpImplicit, BL_CONTEXT_STYLE_SLOT_STROKE, bail);
  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  return stroke_unclipped_path<kRM>(ctx_impl, di, ds, origin_fixed, path->dcast());
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_path_d_rgba32_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  BL_ASSERT(path->_d.is_path());

  bool bail = path->dcast().is_empty();
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoStrokeOpExplicit, BL_CONTEXT_STYLE_SLOT_STROKE, rgba32, bail);
  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  return stroke_unclipped_path<kRM>(ctx_impl, di, ds, origin_fixed, path->dcast());
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_path_d_ext_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLPathCore* path, const BLObjectCore* style) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  BL_ASSERT(path->_d.is_path());

  bool bail = path->dcast().is_empty();
  BLResult bail_result = BL_SUCCESS;

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoStrokeOpExplicit, BL_CONTEXT_STYLE_SLOT_STROKE, style, bail);
  BLPoint origin_fixed = ctx_impl->final_transform_fixed().map_point(*origin);
  BLResult result = stroke_unclipped_path<kRM>(ctx_impl, di, ds, origin_fixed, path->dcast());

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

// bl::RasterEngine - ContextImpl - Frontend - Stroke Text
// =======================================================

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_text_op_d_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = !font->dcast().is_valid();
  BLResult bail_result = bail ? bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED) : BLResult(BL_SUCCESS);

  BL_CONTEXT_RESOLVE_IMPLICIT_STYLE_OP(ContextFlags::kNoStrokeOpImplicit, BL_CONTEXT_STYLE_SLOT_STROKE, bail);
  return stroke_unclipped_text<kRM>(ctx_impl, di, ds, origin, font, op_type, op_data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_text_op_i_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLPoint origin_d(*origin);
  return stroke_text_op_d_impl<kRM>(base_impl, &origin_d, font, op_type, op_data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_text_op_d_rgba32_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, uint32_t rgba32) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = !font->dcast().is_valid();
  BLResult bail_result = bail ? bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED) : BLResult(BL_SUCCESS);

  BL_CONTEXT_RESOLVE_EXPLICIT_SOLID_OP(ContextFlags::kNoStrokeOpExplicit, BL_CONTEXT_STYLE_SLOT_STROKE, rgba32, bail);
  return stroke_unclipped_text<kRM>(ctx_impl, di, ds, origin, font, op_type, op_data);
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_text_op_d_ext_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, const BLObjectCore* style) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);

  bool bail = !font->dcast().is_valid();
  BLResult bail_result = bail ? bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED) : BLResult(BL_SUCCESS);

  BL_CONTEXT_RESOLVE_EXPLICIT_STYLE_OP(ContextFlags::kNoStrokeOpExplicit, BL_CONTEXT_STYLE_SLOT_STROKE, style, bail);
  BLResult result = stroke_unclipped_text<kRM>(ctx_impl, di, ds, origin, font, op_type, op_data);

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), result);
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_text_op_i_rgba32_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, uint32_t rgba32) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLPoint origin_d(*origin);
  return stroke_text_op_d_rgba32_impl<kRM>(base_impl, &origin_d, font, op_type, op_data, rgba32);
}

template<RenderingMode kRM>
static BLResult BL_CDECL stroke_text_op_i_ext_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLFontCore* font, BLContextRenderTextOp op_type, const void* op_data, const BLObjectCore* style) noexcept {
  BL_ASSERT(font->_d.is_font());

  BLPoint origin_d(*origin);
  return stroke_text_op_d_ext_impl<kRM>(base_impl, &origin_d, font, op_type, op_data, style);
}

// bl::RasterEngine - ContextImpl - Frontend - Blit Image
// ======================================================

template<RenderingMode kRM>
static BLResult BL_CDECL blit_image_d_impl(BLContextImpl* base_impl, const BLPoint* origin, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(img->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLImageImpl* image_impl = ImageInternal::get_impl(img);

  BLPoint dst(*origin);
  BLRectI src_rect;

  BLResult bail_result = check_image_area(src_rect, image_impl, img_area);
  bool bail = bail_result != BL_SUCCESS;
  BL_CONTEXT_RESOLVE_BLIT_OP(ContextFlags::kNoBlitFlags, image_impl->format, bail);

  BLBox final_box;

  if (BL_LIKELY(resolved.unmodified())) {
    uint32_t img_bytes_per_pixel = image_impl->depth / 8u;

    if constexpr (kRM == RenderingMode::kAsync)
      fetch_data->init_style_object_and_destroy_func(img, destroy_fetch_data_image);

    if (ctx_impl->final_transform_type() <= BL_TRANSFORM_TYPE_TRANSLATE) {
      double startX = dst.x * ctx_impl->final_transform_fixed().m00 + ctx_impl->final_transform_fixed().m20;
      double startY = dst.y * ctx_impl->final_transform_fixed().m11 + ctx_impl->final_transform_fixed().m21;

      double dx0 = bl_max(startX, ctx_impl->final_clip_box_fixed_d().x0);
      double dy0 = bl_max(startY, ctx_impl->final_clip_box_fixed_d().y0);
      double dx1 = bl_min(startX + double(src_rect.w) * ctx_impl->final_transform_fixed().m00, ctx_impl->final_clip_box_fixed_d().x1);
      double dy1 = bl_min(startY + double(src_rect.h) * ctx_impl->final_transform_fixed().m11, ctx_impl->final_clip_box_fixed_d().y1);

      // Clipped out, invalid coordinates, or empty `img_area`.
      if (!(unsigned(dx0 < dx1) & unsigned(dy0 < dy1)))
        return BL_SUCCESS;

      int ix0 = Math::trunc_to_int(dx0);
      int iy0 = Math::trunc_to_int(dy0);
      int ix1 = Math::trunc_to_int(dx1);
      int iy1 = Math::trunc_to_int(dy1);

      // Clipped out - this is required as the difference between x0 & x1 and y0 & y1 could be smaller than our fixed point.
      if (!(unsigned(ix0 < ix1) & unsigned(iy0 < iy1)))
        return BL_SUCCESS;

      int64_t start_fx = Math::floor_to_int64(startX);
      int64_t start_fy = Math::floor_to_int64(startY);

      if (!((start_fx | start_fy) & ctx_impl->render_target_info.fpMaskI)) {
        // Pixel aligned blit. At this point we still don't know whether the area where the pixels will be composited
        // is aligned, but we know for sure that the pixels of `src` image don't require any interpolation.
        int x0 = ix0 >> ctx_impl->render_target_info.fpShiftI;
        int y0 = iy0 >> ctx_impl->render_target_info.fpShiftI;
        int x1 = (ix1 + ctx_impl->render_target_info.fpMaskI) >> ctx_impl->render_target_info.fpShiftI;
        int y1 = (iy1 + ctx_impl->render_target_info.fpMaskI) >> ctx_impl->render_target_info.fpShiftI;

        int tx = int(start_fx >> ctx_impl->render_target_info.fpShiftI);
        int ty = int(start_fy >> ctx_impl->render_target_info.fpShiftI);

        src_rect.x += x0 - tx;
        src_rect.y += y0 - ty;
        src_rect.w = x1 - x0;
        src_rect.h = y1 - y0;

        fetch_data->init_image_source(image_impl, src_rect);
        fetch_data->setup_pattern_blit(x0, y0);
      }
      else {
        fetch_data->init_image_source(image_impl, src_rect);
        fetch_data->setup_pattern_fx_fy(BL_RASTER_CONTEXT_PREFERRED_BLIT_EXTEND, BLPatternQuality(ctx_impl->hints().pattern_quality), img_bytes_per_pixel, start_fx, start_fy);
      }

      prepare_non_solid_fetch(ctx_impl, di, ds, fetch_data.ptr());
      return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), fill_clipped_box_f<kRM>(ctx_impl, di, ds, BLBoxI(ix0, iy0, ix1, iy1)));
    }

    BLMatrix2D ft(ctx_impl->final_transform());
    ft.translate(dst.x, dst.y);

    fetch_data->init_image_source(image_impl, src_rect);
    if (!fetch_data->setup_pattern_affine(BL_RASTER_CONTEXT_PREFERRED_BLIT_EXTEND, BLPatternQuality(ctx_impl->hints().pattern_quality), img_bytes_per_pixel, ft))
      return BL_SUCCESS;

    prepare_non_solid_fetch(ctx_impl, di, ds, fetch_data.ptr());
    final_box = BLBox(dst.x, dst.y, dst.x + double(src_rect.w), dst.y + double(src_rect.h));
  }
  else {
    prepare_overridden_fetch(ctx_impl, di, ds, CompOpSolidId(resolved.flags));
    final_box = BLBox(dst.x, dst.y, dst.x + double(src_rect.w), dst.y + double(src_rect.h));
  }

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), fill_unclipped_box_d<kRM>(ctx_impl, di, ds, final_box));
}

template<RenderingMode kRM>
static BLResult BL_CDECL blit_image_i_impl(BLContextImpl* base_impl, const BLPointI* origin, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(img->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLImageImpl* image_impl = ImageInternal::get_impl(img);

  if (!bl_test_flag(ctx_impl->context_flags, ContextFlags::kInfoIntegralTranslation)) {
    BLPoint origin_d(*origin);
    return blit_image_d_impl<kRM>(ctx_impl, &origin_d, img, img_area);
  }

  BLBoxI dst_box;
  BLPointI src_offset;

  BLResult bail_result;
  bool bail = !translate_and_clip_rect_to_blit_i(ctx_impl, origin, img_area, &image_impl->size, &bail_result, &dst_box, &src_offset);

  BL_CONTEXT_RESOLVE_BLIT_OP(ContextFlags::kNoBlitFlags, image_impl->format, bail);

  if (BL_LIKELY(resolved.unmodified())) {
    if constexpr (kRM == RenderingMode::kAsync)
      fetch_data->init_style_object_and_destroy_func(img, destroy_fetch_data_image);

    fetch_data->init_image_source(image_impl, BLRectI(src_offset.x, src_offset.y, dst_box.x1 - dst_box.x0, dst_box.y1 - dst_box.y0));
    fetch_data->setup_pattern_blit(dst_box.x0, dst_box.y0);

    prepare_non_solid_fetch(ctx_impl, di, ds, fetch_data.ptr());
  }
  else {
    prepare_overridden_fetch(ctx_impl, di, ds, CompOpSolidId(resolved.flags));
  }

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), fill_clipped_box_a<kRM>(ctx_impl, di, ds, dst_box));
}

// bl::RasterEngine - ContextImpl - Frontend - Blit Scaled Image
// =============================================================

template<RenderingMode kRM>
static BLResult BL_CDECL blit_scaled_image_d_impl(BLContextImpl* base_impl, const BLRect* rect, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(img->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLImageImpl* image_impl = ImageInternal::get_impl(img);

  BLRectI src_rect;
  BL_PROPAGATE(check_image_area(src_rect, image_impl, img_area));

  // OPTIMIZATION: Don't go over all the transformations if the destination and source rects have the same size.
  if (bool_and(rect->w == double(src_rect.w), rect->h == double(src_rect.h)))
    return blit_image_d_impl<kRM>(ctx_impl, reinterpret_cast<const BLPoint*>(rect), img, img_area);

  BLResult bail_result = BL_SUCCESS;
  BL_CONTEXT_RESOLVE_BLIT_OP(ContextFlags::kNoBlitFlags, image_impl->format, kNoBail);

  BLBox final_box(rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
  if (BL_LIKELY(resolved.unmodified())) {
    if constexpr (kRM == RenderingMode::kAsync)
      fetch_data->init_style_object_and_destroy_func(img, destroy_fetch_data_image);

    BLMatrix2D ft(rect->w / double(src_rect.w), 0.0, 0.0, rect->h / double(src_rect.h), rect->x, rect->y);
    TransformInternal::multiply(ft, ft, ctx_impl->final_transform());

    uint32_t img_bytes_per_pixel = image_impl->depth / 8u;
    fetch_data->init_image_source(image_impl, src_rect);

    if (!fetch_data->setup_pattern_affine(BL_RASTER_CONTEXT_PREFERRED_BLIT_EXTEND, BLPatternQuality(ctx_impl->hints().pattern_quality), img_bytes_per_pixel, ft))
      return BL_SUCCESS;

    prepare_non_solid_fetch(ctx_impl, di, ds, fetch_data.ptr());
  }
  else {
    prepare_overridden_fetch(ctx_impl, di, ds, CompOpSolidId(resolved.flags));
  }

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), fill_unclipped_box_d<kRM>(ctx_impl, di, ds, final_box));
}

template<RenderingMode kRM>
static BLResult BL_CDECL blit_scaled_image_i_impl(BLContextImpl* base_impl, const BLRectI* rect, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(img->_d.is_image());

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(base_impl);
  BLImageImpl* image_impl = ImageInternal::get_impl(img);

  BLRectI src_rect;
  BL_PROPAGATE(check_image_area(src_rect, image_impl, img_area));

  // OPTIMIZATION: Don't go over all the transformations if the destination and source rects have the same size.
  if (bool_and(rect->w == src_rect.w, rect->h == src_rect.h))
    return blit_image_i_impl<kRM>(ctx_impl, reinterpret_cast<const BLPointI*>(rect), img, img_area);

  BLResult bail_result = BL_SUCCESS;
  BL_CONTEXT_RESOLVE_BLIT_OP(ContextFlags::kNoBlitFlags, image_impl->format, kNoBail);

  BLBox final_box(double(rect->x), double(rect->y), double(rect->x) + double(rect->w), double(rect->y) + double(rect->h));
  if (BL_LIKELY(resolved.unmodified())) {
    if constexpr (kRM == RenderingMode::kAsync)
      fetch_data->init_style_object_and_destroy_func(img, destroy_fetch_data_image);

    BLMatrix2D transform(double(rect->w) / double(src_rect.w), 0.0, 0.0, double(rect->h) / double(src_rect.h), double(rect->x), double(rect->y));
    TransformInternal::multiply(transform, transform, ctx_impl->final_transform());

    uint32_t img_bytes_per_pixel = image_impl->depth / 8u;
    fetch_data->init_image_source(image_impl, src_rect);
    if (!fetch_data->setup_pattern_affine(BL_RASTER_CONTEXT_PREFERRED_BLIT_EXTEND, BLPatternQuality(ctx_impl->hints().pattern_quality), img_bytes_per_pixel, transform))
      return BL_SUCCESS;

    prepare_non_solid_fetch(ctx_impl, di, ds, fetch_data.ptr());
  }
  else {
    prepare_overridden_fetch(ctx_impl, di, ds, CompOpSolidId(resolved.flags));
  }

  return finalize_explicit_op<kRM>(ctx_impl, fetch_data.ptr(), fill_unclipped_box_d<kRM>(ctx_impl, di, ds, final_box));
}

// bl::RasterEngine - ContextImpl - Attach & Detach
// ================================================

static BL_INLINE uint32_t calculate_band_height(uint32_t format, const BLSizeI& size, const BLContextCreateInfo* options) noexcept {
  // TODO: [Rendering Context] We should use the format and calculate how many bytes are used by raster storage per band.
  bl_unused(format);

  // Maximum band height we start at is 64, then decrease to 16.
  constexpr uint32_t kMinBandHeight = 8;
  constexpr uint32_t kMaxBandHeight = 64;

  uint32_t band_height = kMaxBandHeight;

  // TODO: [Rendering Context] We should read this number from the CPU and adjust.
  size_t cache_size_limit = 1024 * 256;
  size_t pixel_count = size_t(uint32_t(size.w)) * band_height;

  do {
    size_t cell_storage = pixel_count * sizeof(uint32_t);
    if (cell_storage <= cache_size_limit)
      break;

    band_height >>= 1;
    pixel_count >>= 1;
  } while (band_height > kMinBandHeight);

  uint32_t thread_count = options->thread_count;
  if (band_height > kMinBandHeight && thread_count > 1) {
    uint32_t band_height_shift = IntOps::ctz(band_height);
    uint32_t minimum_band_count = thread_count;

    do {
      uint32_t band_count = (uint32_t(size.h) + band_height - 1) >> band_height_shift;
      if (band_count >= minimum_band_count)
        break;

      band_height >>= 1;
      band_height_shift--;
    } while (band_height > kMinBandHeight);
  }

  return band_height;
}

static BL_INLINE uint32_t calculate_command_quantization_shift(uint32_t band_height, uint32_t band_count) noexcept {
  uint32_t band_quantization = IntOps::ctz(band_height);
  uint32_t coordinate_quantization = bl_max<uint32_t>(32 - IntOps::clz(band_height * band_count), 8) - 8u;

  // We should never quantize to less than a band height.
  return bl_max(band_quantization, coordinate_quantization);
}

static BL_INLINE size_t calculate_zeroed_memory_size(uint32_t width, uint32_t height) noexcept {
  size_t aligned_width = IntOps::align_up(size_t(width) + 1u + BL_PIPE_PIXELS_PER_ONE_BIT, 16);

  size_t bit_stride = IntOps::word_count_from_bit_count<BLBitWord>(aligned_width / BL_PIPE_PIXELS_PER_ONE_BIT) * sizeof(BLBitWord);
  size_t cell_stride = aligned_width * sizeof(uint32_t);

  size_t minimum_size = bit_stride * size_t(height) + cell_stride * size_t(height);
  return IntOps::align_up(minimum_size + sizeof(BLBitWord) * 16, BL_CACHE_LINE_SIZE);
}

static BLResult attach(BLRasterContextImpl* ctx_impl, BLImageCore* image, const BLContextCreateInfo* options) noexcept {
  BL_ASSERT(image != nullptr);
  BL_ASSERT(options != nullptr);

  uint32_t format = ImageInternal::get_impl(image)->format;
  BLSizeI size = ImageInternal::get_impl(image)->size;

  // TODO: [Rendering Context] Hardcoded for 8bpc.
  uint32_t target_component_type = RenderTargetInfo::kPixelComponentUInt8;

  uint32_t band_height = calculate_band_height(format, size, options);
  uint32_t band_count = (uint32_t(size.h) + band_height - 1) >> IntOps::ctz(band_height);
  uint32_t command_quantization_shift = calculate_command_quantization_shift(band_height, band_count);

  size_t zeroed_memory_size = calculate_zeroed_memory_size(uint32_t(size.w), band_height);

  // Initialization.
  BLResult result = BL_SUCCESS;
  Pipeline::PipeRuntime* pipe_runtime = nullptr;

  // If anything fails we would restore the zone state to match this point.
  ArenaAllocator& base_zone = ctx_impl->base_zone;
  ArenaAllocator::StatePtr zone_state = base_zone.save_state();

  // Not a real loop, just a scope we can escape early via 'break'.
  do {
    // Step 1: Initialize edge storage of the sync worker.
    result = ctx_impl->sync_work_data.init_band_data(band_height, band_count, command_quantization_shift);
    if (result != BL_SUCCESS)
      break;

    // Step 2: Initialize the thread manager if multi-threaded rendering is enabled.
    if (options->thread_count) {
      ctx_impl->ensure_worker_mgr();
      result = ctx_impl->worker_mgr->init(ctx_impl, options);

      if (result != BL_SUCCESS)
        break;

      if (ctx_impl->worker_mgr->is_active())
        ctx_impl->rendering_mode = uint8_t(RenderingMode::kAsync);
    }

    // Step 3: Initialize pipeline runtime (JIT or fixed).
#if !defined(BL_BUILD_NO_JIT)
    if (!(options->flags & BL_CONTEXT_CREATE_FLAG_DISABLE_JIT)) {
      pipe_runtime = &Pipeline::JIT::PipeDynamicRuntime::_global;

      if (options->flags & BL_CONTEXT_CREATE_FLAG_ISOLATED_JIT_RUNTIME) {
        // Create an isolated `BLPipeGenRuntime` if specified. It will be used to store all functions
        // generated during the rendering and will be destroyed together with the context.
        Pipeline::JIT::PipeDynamicRuntime* isolatedRT =
          base_zone.new_t<Pipeline::JIT::PipeDynamicRuntime>(Pipeline::PipeRuntimeFlags::kIsolated);

        // This should not really happen as the first block is allocated with the impl.
        if (BL_UNLIKELY(!isolatedRT)) {
          result = bl_make_error(BL_ERROR_OUT_OF_MEMORY);
          break;
        }

        // Enable logger if required.
        if (options->flags & BL_CONTEXT_CREATE_FLAG_ISOLATED_JIT_LOGGING) {
          isolatedRT->set_logger_enabled(true);
        }

        // Feature restrictions are related to JIT compiler - it allows us to test the code generated by JIT
        // with less features than the current CPU has, to make sure that we support older hardware or to
        // compare between implementations.
        if (options->flags & BL_CONTEXT_CREATE_FLAG_OVERRIDE_CPU_FEATURES) {
          isolatedRT->_restrict_features(options->cpu_features);
        }

        pipe_runtime = isolatedRT;
        base_zone.align(base_zone.block_alignment());
      }
    }
#endif

    if (!pipe_runtime) {
      pipe_runtime = &Pipeline::PipeStaticRuntime::_global;
    }

    // Step 4: Allocate zeroed memory for the user thread and all worker threads.
    result = ctx_impl->sync_work_data.zero_buffer.ensure(zeroed_memory_size);
    if (result != BL_SUCCESS)
      break;

    if (!ctx_impl->is_sync()) {
      result = ctx_impl->worker_mgr->init_work_memory(zeroed_memory_size);
      if (result != BL_SUCCESS)
        break;
    }

    // Step 5: Make the destination image mutable.
    result = bl_image_make_mutable(image, &ctx_impl->dst_data);
    if (result != BL_SUCCESS)
      break;
  } while (0);

  // Handle a possible initialization failure.
  if (result != BL_SUCCESS) {
    // Switch back to a synchronous rendering mode if asynchronous rendering was already setup.
    // We have already acquired worker threads that must be released now.
    if (ctx_impl->rendering_mode == uint8_t(RenderingMode::kAsync)) {
      ctx_impl->worker_mgr->reset();
      ctx_impl->rendering_mode = uint8_t(RenderingMode::kSync);
    }

    // If we failed we don't want the pipeline runtime associated with the
    // context so we simply destroy it and pretend like nothing happened.
    if (pipe_runtime) {
      if (bl_test_flag(pipe_runtime->runtime_flags(), Pipeline::PipeRuntimeFlags::kIsolated))
        pipe_runtime->destroy();
    }

    base_zone.restore_state(zone_state);
    return result;
  }

  ctx_impl->context_flags = ContextFlags::kInfoIntegralTranslation;

  if (!ctx_impl->is_sync()) {
    ctx_impl->virt = &raster_impl_virt_async;
    ctx_impl->sync_work_data.synchronization = &ctx_impl->worker_mgr->_synchronization;
  }

  // Increase `writer_count` of the image, will be decreased by `detach()`.
  BLImagePrivateImpl* image_impl = ImageInternal::get_impl(image);
  bl_atomic_fetch_add_relaxed(&image_impl->writer_count);
  ctx_impl->dst_image._d = image->_d;

  // Initialize the pipeline runtime and pipeline lookup cache.
  ctx_impl->pipe_provider.init(pipe_runtime);
  ctx_impl->pipe_lookup_cache.reset();

  // Initialize the sync work data.
  ctx_impl->sync_work_data.init_context_data(ctx_impl->dst_data, options->pixel_origin);

  // Initialize destination image information available in a public rendering context state.
  ctx_impl->internal_state.target_size.reset(size.w, size.h);
  ctx_impl->internal_state.target_image = &ctx_impl->dst_image;

  // Initialize members that are related to target precision.
  ctx_impl->render_target_info = render_target_info_by_component_type[target_component_type];
  ctx_impl->fp_min_safe_coord_d = Math::floor(double(Traits::min_value<int32_t>() + 1) * ctx_impl->fp_scale_d());
  ctx_impl->fp_max_safe_coord_d = Math::floor(double(Traits::max_value<int32_t>() - 1 - bl_max(size.w, size.h)) * ctx_impl->fp_scale_d());

  // Initialize members that are related to alpha blending and composition.
  ctx_impl->solid_format_table[BL_RASTER_CONTEXT_SOLID_FORMAT_ARGB] = uint8_t(FormatExt::kPRGB32);
  ctx_impl->solid_format_table[BL_RASTER_CONTEXT_SOLID_FORMAT_FRGB] = uint8_t(FormatExt::kFRGB32);
  ctx_impl->solid_format_table[BL_RASTER_CONTEXT_SOLID_FORMAT_ZERO] = uint8_t(FormatExt::kZERO32);

  // Const-casted, because this would replace fetch_data, which is non-const, but guaranteed to not modify solid styles.
  RenderFetchDataSolid* solid_override_fill_table =
    target_component_type == RenderTargetInfo::kPixelComponentUInt8
      ? (RenderFetchDataSolid*)solid_override_fill_u8
      : (RenderFetchDataSolid*)solid_override_fill_u16;

  ctx_impl->solid_override_fill_table = solid_override_fill_table;
  ctx_impl->solid_fetch_data_override_table[size_t(CompOpSolidId::kNone       )] = nullptr;
  ctx_impl->solid_fetch_data_override_table[size_t(CompOpSolidId::kTransparent)] = &solid_override_fill_table[size_t(CompOpSolidId::kTransparent)];
  ctx_impl->solid_fetch_data_override_table[size_t(CompOpSolidId::kOpaqueBlack)] = &solid_override_fill_table[size_t(CompOpSolidId::kOpaqueBlack)];
  ctx_impl->solid_fetch_data_override_table[size_t(CompOpSolidId::kOpaqueWhite)] = &solid_override_fill_table[size_t(CompOpSolidId::kOpaqueWhite)];
  ctx_impl->solid_fetch_data_override_table[size_t(CompOpSolidId::kAlwaysNop  )] = &solid_override_fill_table[size_t(CompOpSolidId::kAlwaysNop  )];

  // Initialize the rendering state to defaults.
  ctx_impl->state_id_counter = 0;
  ctx_impl->saved_state = nullptr;
  ctx_impl->shared_fill_state = nullptr;
  ctx_impl->shared_stroke_state = nullptr;

  // Initialize public state.
  ctx_impl->internal_state.hints.reset();
  ctx_impl->internal_state.hints.pattern_quality = BL_PATTERN_QUALITY_BILINEAR;
  ctx_impl->internal_state.comp_op = uint8_t(BL_COMP_OP_SRC_OVER);
  ctx_impl->internal_state.fill_rule = uint8_t(BL_FILL_RULE_NON_ZERO);
  ctx_impl->internal_state.style_type[BL_CONTEXT_STYLE_SLOT_FILL] = uint8_t(BL_OBJECT_TYPE_RGBA);
  ctx_impl->internal_state.style_type[BL_CONTEXT_STYLE_SLOT_STROKE] = uint8_t(BL_OBJECT_TYPE_RGBA);
  ctx_impl->internal_state.saved_state_count = 0;
  ctx_impl->internal_state.approximation_options = PathInternal::make_default_approximation_options();
  ctx_impl->internal_state.global_alpha = 1.0;
  ctx_impl->internal_state.style_alpha[0] = 1.0;
  ctx_impl->internal_state.style_alpha[1] = 1.0;
  ctx_impl->internal_state.styleAlphaI[0] = uint32_t(ctx_impl->render_target_info.fullAlphaI);
  ctx_impl->internal_state.styleAlphaI[1] = uint32_t(ctx_impl->render_target_info.fullAlphaI);
  bl_call_ctor(ctx_impl->internal_state.stroke_options.dcast());
  ctx_impl->internal_state.meta_transform.reset();
  ctx_impl->internal_state.user_transform.reset();

  // Initialize private state.
  ctx_impl->internal_state.final_transform_fixed_type = BL_TRANSFORM_TYPE_SCALE;
  ctx_impl->internal_state.meta_transform_fixed_type = BL_TRANSFORM_TYPE_SCALE;
  ctx_impl->internal_state.meta_transform_type = BL_TRANSFORM_TYPE_TRANSLATE;
  ctx_impl->internal_state.final_transform_type = BL_TRANSFORM_TYPE_TRANSLATE;
  ctx_impl->internal_state.identity_transform_type = BL_TRANSFORM_TYPE_IDENTITY;
  ctx_impl->internal_state.global_alpha_i = uint32_t(ctx_impl->render_target_info.fullAlphaI);

  ctx_impl->internal_state.final_transform.reset();
  ctx_impl->internal_state.meta_transform_fixed.reset_to_scaling(ctx_impl->render_target_info.fp_scale_d);
  ctx_impl->internal_state.final_transform_fixed.reset_to_scaling(ctx_impl->render_target_info.fp_scale_d);
  ctx_impl->internal_state.translation_i.reset(0, 0);

  ctx_impl->internal_state.meta_clip_box_i.reset(0, 0, size.w, size.h);
  // `final_clip_box_i` and `final_clip_box_d` are initialized by `reset_clipping_to_meta_clip_box()`.

  if (options->saved_state_limit)
    ctx_impl->saved_state_limit = options->saved_state_limit;
  else
    ctx_impl->saved_state_limit = BL_RASTER_CONTEXT_DEFAULT_SAVED_STATE_LIMIT;

  // Make sure the state is initialized properly.
  on_after_comp_op_changed(ctx_impl);
  on_after_flatten_tolerance_changed(ctx_impl);
  on_after_offset_parameter_changed(ctx_impl);
  reset_clipping_to_meta_clip_box(ctx_impl);

  // Initialize styles.
  init_style_to_default(ctx_impl, BL_CONTEXT_STYLE_SLOT_FILL);
  init_style_to_default(ctx_impl, BL_CONTEXT_STYLE_SLOT_STROKE);

  return BL_SUCCESS;
}

static BLResult detach(BLRasterContextImpl* ctx_impl) noexcept {
  // Release the ImageImpl.
  BLImagePrivateImpl* image_impl = ImageInternal::get_impl(&ctx_impl->dst_image);
  BL_ASSERT(image_impl != nullptr);

  flush_impl(ctx_impl, BL_CONTEXT_FLUSH_SYNC);

  // Release Threads/WorkerContexts used by asynchronous rendering.
  if (ctx_impl->worker_mgr_initialized)
    ctx_impl->worker_mgr->reset();

  // Release PipeRuntime.
  if (bl_test_flag(ctx_impl->pipe_provider.runtime()->runtime_flags(), Pipeline::PipeRuntimeFlags::kIsolated))
    ctx_impl->pipe_provider.runtime()->destroy();
  ctx_impl->pipe_provider.reset();

  // Release all states.
  //
  // Important as the user doesn't have to restore all states, in that case we basically need to iterate
  // over all of them and release resources they hold.
  discard_states(ctx_impl, nullptr);
  bl_call_dtor(ctx_impl->internal_state.stroke_options);

  ContextFlags context_flags = ctx_impl->context_flags;
  if (bl_test_flag(context_flags, ContextFlags::kFetchDataFill))
    destroy_valid_style(ctx_impl, &ctx_impl->internal_state.style[BL_CONTEXT_STYLE_SLOT_FILL]);

  if (bl_test_flag(context_flags, ContextFlags::kFetchDataStroke))
    destroy_valid_style(ctx_impl, &ctx_impl->internal_state.style[BL_CONTEXT_STYLE_SLOT_STROKE]);

  // Clear other important members. We don't have to clear everything as if we re-attach an image again
  // all members will be overwritten anyway.
  ctx_impl->context_flags = ContextFlags::kNoFlagsSet;

  ctx_impl->base_zone.clear();
  ctx_impl->fetch_data_pool.reset();
  ctx_impl->saved_state_pool.reset();
  ctx_impl->sync_work_data.ctx_data.reset();
  ctx_impl->sync_work_data.work_zone.clear();

  // If the image was dereferenced during rendering it's our responsibility to destroy it. This is not useful
  // from the consumer's perspective as the resulting image can never be used again, but it can happen in some
  // cases (for example when an asynchronous rendering is terminated and the target image released with it).
  if (bl_atomic_fetch_sub_strong(&image_impl->writer_count) == 1)
    if (ObjectInternal::get_impl_ref_count(image_impl) == 0)
      ImageInternal::free_impl(image_impl);

  ctx_impl->dst_image._d.impl = nullptr;
  ctx_impl->dst_data.reset();

  return BL_SUCCESS;
}

// bl::RasterEngine - ContextImpl - Destroy
// ========================================

static BLResult BL_CDECL destroy_impl(BLObjectImpl* impl) noexcept {
  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(impl);

  if (ctx_impl->dst_image._d.impl)
    detach(ctx_impl);

  ctx_impl->~BLRasterContextImpl();
  return bl_object_free_impl(ctx_impl);
}

// bl::RasterEngine - ContextImpl - Virtual Function Table
// =======================================================

template<RenderingMode kRM>
static void init_virt(BLContextVirt* virt) noexcept {
  virt->base.destroy                = destroy_impl;
  virt->base.get_property           = get_property_impl;
  virt->base.set_property           = set_property_impl;
  virt->flush                       = flush_impl;

  virt->save                        = save_impl;
  virt->restore                     = restore_impl;

  virt->apply_transform_op          = apply_transform_op_impl;
  virt->user_to_meta                = user_to_meta_impl;

  virt->set_hint                    = set_hint_impl;
  virt->set_hints                   = set_hints_impl;

  virt->set_flatten_mode            = set_flatten_mode_impl;
  virt->set_flatten_tolerance       = set_flatten_tolerance_impl;
  virt->set_approximation_options   = set_approximation_options_impl;

  virt->get_style                   = get_style_impl;
  virt->set_style                   = set_style_impl;
  virt->disable_style               = disable_style_impl;
  virt->set_style_rgba              = set_style_rgba_impl;
  virt->set_style_rgba32            = set_style_rgba32_impl;
  virt->set_style_rgba64            = set_style_rgba64_impl;
  virt->set_style_alpha             = set_style_alpha_impl;
  virt->swap_styles                 = swap_styles_impl;

  virt->set_global_alpha            = set_global_alpha_impl;
  virt->set_comp_op                 = set_comp_op_impl;

  virt->set_fill_rule               = set_fill_rule_impl;
  virt->set_stroke_width            = set_stroke_width_impl;
  virt->set_stroke_miter_limit      = set_stroke_miter_limit_impl;
  virt->set_stroke_cap              = set_stroke_cap_impl;
  virt->set_stroke_caps             = set_stroke_caps_impl;
  virt->set_stroke_join             = set_stroke_join_impl;
  virt->set_stroke_transform_order  = set_stroke_transform_order_impl;
  virt->set_stroke_dash_offset      = set_stroke_dash_offset_impl;
  virt->set_stroke_dash_array       = set_stroke_dash_array_impl;
  virt->set_stroke_options          = set_stroke_options_impl;

  virt->clip_to_rect_i              = clip_to_rect_i_impl;
  virt->clip_to_rect_d              = clip_to_rect_d_impl;
  virt->restore_clipping            = restore_clipping_impl;

  virt->clear_all                   = clear_all_impl<kRM>;
  virt->clear_recti                 = clear_rect_i_impl<kRM>;
  virt->clear_rectd                 = clear_rect_d_impl<kRM>;

  virt->fill_all                    = fill_all_impl<kRM>;
  virt->fill_all_rgba32             = fill_all_rgba32_impl<kRM>;
  virt->fill_all_ext                = fill_all_ext_impl<kRM>;

  virt->fill_rect_i                 = fill_rect_i_impl<kRM>;
  virt->fill_rect_i_rgba32          = fill_rect_i_rgba32_impl<kRM>;
  virt->fill_rect_i_ext             = fill_rect_i_ext_impl<kRM>;

  virt->fill_rect_d                 = fill_rect_d_impl<kRM>;
  virt->fill_rect_d_rgba32          = fill_rect_d_rgba32_impl<kRM>;
  virt->fill_rect_d_ext             = fill_rect_d_ext_impl<kRM>;

  virt->fill_path_d                 = fill_path_d_impl<kRM>;
  virt->fill_path_d_rgba32          = fill_path_d_rgba32_impl<kRM>;
  virt->fill_path_d_ext             = fill_path_d_ext_impl<kRM>;

  virt->fill_geometry               = fill_geometry_impl<kRM>;
  virt->fill_geometry_rgba32        = fill_geometry_rgba32_impl<kRM>;
  virt->fill_geometry_ext           = fill_geometry_ext_impl<kRM>;

  virt->fill_text_op_i              = fill_text_op_i_impl<kRM>;
  virt->fill_text_op_i_rgba32       = fill_text_op_i_rgba32_impl<kRM>;
  virt->fill_text_op_i_ext          = fill_text_op_i_ext_impl<kRM>;

  virt->fill_text_op_d              = fill_text_op_d_impl<kRM>;
  virt->fill_text_op_d_rgba32       = fill_text_op_d_rgba32_impl<kRM>;
  virt->fill_text_op_d_ext          = fill_text_op_d_ext_impl<kRM>;

  virt->fill_mask_i                 = fill_mask_i_impl<kRM>;
  virt->fill_mask_i_rgba32          = fill_mask_i_rgba32_impl<kRM>;
  virt->fill_mask_i_ext             = fill_mask_i_ext_impl<kRM>;

  virt->fill_mask_d                 = fill_mask_d_impl<kRM>;
  virt->fill_mask_d_Rgba32          = fill_mask_d_rgba32_impl<kRM>;
  virt->fill_mask_d_ext             = fill_mask_d_ext_impl<kRM>;

  virt->stroke_path_d               = stroke_path_d_impl<kRM>;
  virt->stroke_path_d_rgba32        = stroke_path_d_rgba32_impl<kRM>;
  virt->stroke_path_d_ext           = stroke_path_d_ext_impl<kRM>;

  virt->stroke_geometry             = stroke_geometry_impl<kRM>;
  virt->stroke_geometry_rgba32      = stroke_geometry_rgba32_impl<kRM>;
  virt->stroke_geometry_ext         = stroke_geometry_ext_impl<kRM>;

  virt->stroke_text_op_i            = stroke_text_op_i_impl<kRM>;
  virt->stroke_text_op_i_rgba32     = stroke_text_op_i_rgba32_impl<kRM>;
  virt->stroke_text_op_i_ext        = stroke_text_op_i_ext_impl<kRM>;

  virt->stroke_text_op_d            = stroke_text_op_d_impl<kRM>;
  virt->stroke_text_op_d_rgba32     = stroke_text_op_d_rgba32_impl<kRM>;
  virt->stroke_text_op_d_ext        = stroke_text_op_d_ext_impl<kRM>;

  virt->blit_image_i                = blit_image_i_impl<kRM>;
  virt->blit_image_d                = blit_image_d_impl<kRM>;

  virt->blit_scaled_image_i         = blit_scaled_image_i_impl<kRM>;
  virt->blit_scaled_image_d         = blit_scaled_image_d_impl<kRM>;
}

} // {bl::RasterEngine}

// bl::RasterEngine - ContextImpl - Runtime Registration
// =====================================================

BLResult bl_raster_context_init_impl(BLContextCore* self, BLImageCore* image, const BLContextCreateInfo* options) noexcept {
  // NOTE: Initially static data was part of `BLRasterContextImpl`, however, that doesn't work with MSAN
  // as it would consider it destroyed when `bl::ArenaAllocator` iterates that block during destruction.
  constexpr size_t kStaticDataSize = 2048;
  constexpr size_t kContextImplSize = sizeof(BLRasterContextImpl) + kStaticDataSize;

  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_CONTEXT);
  BL_PROPAGATE(bl::ObjectInternal::alloc_impl_aligned_t<BLRasterContextImpl>(self, info, BLObjectImplSize{kContextImplSize}, 64));

  BLRasterContextImpl* ctx_impl = static_cast<BLRasterContextImpl*>(self->_d.impl);
  void* static_data = static_cast<void*>(reinterpret_cast<uint8_t*>(self->_d.impl) + sizeof(BLRasterContextImpl));

  bl_call_ctor(*ctx_impl, &bl::RasterEngine::raster_impl_virt_sync, static_data, kStaticDataSize);
  BLResult result = bl::RasterEngine::attach(ctx_impl, image, options);

  if (result != BL_SUCCESS)
    ctx_impl->virt->base.destroy(ctx_impl);

  return result;
}

void bl_raster_context_on_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  bl::RasterEngine::init_virt<bl::RasterEngine::RenderingMode::kSync>(&bl::RasterEngine::raster_impl_virt_sync);
  bl::RasterEngine::init_virt<bl::RasterEngine::RenderingMode::kAsync>(&bl::RasterEngine::raster_impl_virt_async);
}
