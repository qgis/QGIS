// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/context_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/pattern_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/raster/rastercontext_p.h>

namespace bl::ContextInternal {

// bl::Context - Globals
// =====================

static Wrap<BLContextState> null_state;
static BLObjectEternalVirtualImpl<BLContextImpl, BLContextVirt> default_context;
static const constexpr BLContextCreateInfo no_create_info {};

// bl::Context - Null Context
// ==========================

namespace NullContext {

// NullContext implementation does nothing. These functions consistently return `BL_ERROR_INVALID_STATE` to inform the
// caller that the context is not usable. We don't want to mark every unused parameter by `bl_unused()` in this case so
// the warning is temporarily turned off by BL_DIAGNOSTIC_PUSH/POP.
BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)

static BLResult BL_CDECL destroy_impl(BLObjectImpl* impl) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL flush_impl(BLContextImpl* impl, BLContextFlushFlags flags) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL no_args_impl(BLContextImpl* impl) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_double_impl(BLContextImpl* impl, double) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_comp_op_impl(BLContextImpl* impl, BLCompOp) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_fill_rule_impl(BLContextImpl* impl, BLFillRule) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL save_impl(BLContextImpl* impl, BLContextCookie*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL restore_impl(BLContextImpl* impl, const BLContextCookie*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL get_style_impl(const BLContextImpl* impl, BLContextStyleSlot, bool, BLVarCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_style_impl(BLContextImpl* impl, BLContextStyleSlot, const BLObjectCore*, BLContextStyleTransformMode) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL disable_style_impl(BLContextImpl* impl, BLContextStyleSlot) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_style_rgba_impl(BLContextImpl* impl, BLContextStyleSlot, const BLRgba*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_style_rgba32Impl(BLContextImpl* impl, BLContextStyleSlot, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_style_rgba64Impl(BLContextImpl* impl, BLContextStyleSlot, uint64_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_style_alpha_impl(BLContextImpl* impl, BLContextStyleSlot, double) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL swap_styles_impl(BLContextImpl* impl, BLContextStyleSwapMode mode) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL set_hint_impl(BLContextImpl* impl, BLContextHint, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_hints_impl(BLContextImpl* impl, const BLContextHints*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_flatten_mode_impl(BLContextImpl* impl, BLFlattenMode) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_approximation_options_impl(BLContextImpl* impl, const BLApproximationOptions*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_stroke_transform_order_impl(BLContextImpl* impl, BLStrokeTransformOrder) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_stroke_dash_array_impl(BLContextImpl* impl, const BLArrayCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_stroke_cap_impl(BLContextImpl* impl, BLStrokeCapPosition, BLStrokeCap) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_stroke_caps_impl(BLContextImpl* impl, BLStrokeCap) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_stroke_join_impl(BLContextImpl* impl, BLStrokeJoin) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL set_stroke_options_impl(BLContextImpl* impl, const BLStrokeOptionsCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL apply_transform_op_impl(BLContextImpl* impl, BLTransformOp, const void*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL fill_all_impl(BLContextImpl* impl) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL fill_all_rgba32Impl(BLContextImpl* impl, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL fill_all_ext_impl(BLContextImpl* impl, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL doRectIImpl(BLContextImpl* impl, const BLRectI*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doRectIRgba32Impl(BLContextImpl* impl, const BLRectI*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doRectIExtImpl(BLContextImpl* impl, const BLRectI*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL doRectDImpl(BLContextImpl* impl, const BLRect*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doRectDRgba32Impl(BLContextImpl* impl, const BLRect*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doRectDExtImpl(BLContextImpl* impl, const BLRect*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL doPathDImpl(BLContextImpl* impl, const BLPoint*, const BLPathCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doPathDRgba32Impl(BLContextImpl* impl, const BLPoint*, const BLPathCore*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doPathDExtImpl(BLContextImpl* impl, const BLPoint*, const BLPathCore*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL do_geometry_impl(BLContextImpl* impl, BLGeometryType, const void*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doGeometryRgba32Impl(BLContextImpl* impl, BLGeometryType, const void*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL do_geometry_ext_impl(BLContextImpl* impl, BLGeometryType, const void*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL doTextOpIImpl(BLContextImpl* impl, const BLPointI*, const BLFontCore*, BLContextRenderTextOp, const void*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doTextOpIRgba32Impl(BLContextImpl* impl, const BLPointI*, const BLFontCore*, BLContextRenderTextOp, const void*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doTextOpIExtImpl(BLContextImpl* impl, const BLPointI*, const BLFontCore*, BLContextRenderTextOp, const void*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL doTextOpDImpl(BLContextImpl* impl, const BLPoint*, const BLFontCore*, BLContextRenderTextOp, const void*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doTextOpDRgba32Impl(BLContextImpl* impl, const BLPoint*, const BLFontCore*, BLContextRenderTextOp, const void*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doTextOpDExtImpl(BLContextImpl* impl, const BLPoint*, const BLFontCore*, BLContextRenderTextOp, const void*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL doMaskIImpl(BLContextImpl* impl, const BLPointI*, const BLImageCore*, const BLRectI*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doMaskDRgba32Impl(BLContextImpl* impl, const BLPointI*, const BLImageCore*, const BLRectI*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doMaskDExtImpl(BLContextImpl* impl, const BLPointI*, const BLImageCore*, const BLRectI*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL doMaskDImpl(BLContextImpl* impl, const BLPoint*, const BLImageCore*, const BLRectI*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doMaskDRgba32Impl(BLContextImpl* impl, const BLPoint*, const BLImageCore*, const BLRectI*, uint32_t) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL doMaskDExtImpl(BLContextImpl* impl, const BLPoint*, const BLImageCore*, const BLRectI*, const BLObjectCore*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

static BLResult BL_CDECL blit_image_iImpl(BLContextImpl* impl, const BLPointI*, const BLImageCore*, const BLRectI*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL blit_image_dImpl(BLContextImpl* impl, const BLPoint*, const BLImageCore*, const BLRectI*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL blit_scaled_image_iImpl(BLContextImpl* impl, const BLRectI*, const BLImageCore*, const BLRectI*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }
static BLResult BL_CDECL blit_scaled_image_dImpl(BLContextImpl* impl, const BLRect*, const BLImageCore*, const BLRectI*) noexcept { return bl_make_error(BL_ERROR_INVALID_STATE); }

BL_DIAGNOSTIC_POP

} // {NullContext}

static void init_null_context_virt(BLContextVirt* virt) noexcept {
  virt->base.destroy                = NullContext::destroy_impl;
  virt->base.get_property           = bl_object_impl_get_property;
  virt->base.set_property           = bl_object_impl_set_property;
  virt->flush                       = NullContext::flush_impl;

  virt->save                        = NullContext::save_impl;
  virt->restore                     = NullContext::restore_impl;

  virt->user_to_meta                = NullContext::no_args_impl;
  virt->apply_transform_op          = NullContext::apply_transform_op_impl;

  virt->set_hint                    = NullContext::set_hint_impl;
  virt->set_hints                   = NullContext::set_hints_impl;

  virt->set_flatten_mode            = NullContext::set_flatten_mode_impl;
  virt->set_flatten_tolerance       = NullContext::set_double_impl;
  virt->set_approximation_options   = NullContext::set_approximation_options_impl;

  virt->get_style                   = NullContext::get_style_impl;
  virt->set_style                   = NullContext::set_style_impl;
  virt->disable_style               = NullContext::disable_style_impl;
  virt->set_style_rgba              = NullContext::set_style_rgba_impl;
  virt->set_style_rgba32            = NullContext::set_style_rgba32Impl;
  virt->set_style_rgba64            = NullContext::set_style_rgba64Impl;
  virt->set_style_alpha             = NullContext::set_style_alpha_impl;
  virt->swap_styles                 = NullContext::swap_styles_impl;

  virt->set_global_alpha            = NullContext::set_double_impl;
  virt->set_comp_op                 = NullContext::set_comp_op_impl;

  virt->set_fill_rule               = NullContext::set_fill_rule_impl;

  virt->set_stroke_width            = NullContext::set_double_impl;
  virt->set_stroke_miter_limit      = NullContext::set_double_impl;
  virt->set_stroke_cap              = NullContext::set_stroke_cap_impl;
  virt->set_stroke_caps             = NullContext::set_stroke_caps_impl;
  virt->set_stroke_join             = NullContext::set_stroke_join_impl;
  virt->set_stroke_transform_order  = NullContext::set_stroke_transform_order_impl;
  virt->set_stroke_dash_offset      = NullContext::set_double_impl;
  virt->set_stroke_dash_array       = NullContext::set_stroke_dash_array_impl;
  virt->set_stroke_options          = NullContext::set_stroke_options_impl;

  virt->clip_to_rect_i              = NullContext::doRectIImpl;
  virt->clip_to_rect_d              = NullContext::doRectDImpl;
  virt->restore_clipping            = NullContext::no_args_impl;

  virt->clear_all                   = NullContext::no_args_impl;
  virt->clear_recti                 = NullContext::doRectIImpl;
  virt->clear_rectd                 = NullContext::doRectDImpl;

  virt->fill_all                    = NullContext::fill_all_impl;
  virt->fill_all_rgba32             = NullContext::fill_all_rgba32Impl;
  virt->fill_all_ext                = NullContext::fill_all_ext_impl;

  virt->fill_rect_i                 = NullContext::doRectIImpl;
  virt->fill_rect_i_rgba32          = NullContext::doRectIRgba32Impl;
  virt->fill_rect_i_ext             = NullContext::doRectIExtImpl;

  virt->fill_rect_d                 = NullContext::doRectDImpl;
  virt->fill_rect_d_rgba32          = NullContext::doRectDRgba32Impl;
  virt->fill_rect_d_ext             = NullContext::doRectDExtImpl;

  virt->fill_path_d                 = NullContext::doPathDImpl;
  virt->fill_path_d_rgba32          = NullContext::doPathDRgba32Impl;
  virt->fill_path_d_ext             = NullContext::doPathDExtImpl;

  virt->fill_geometry               = NullContext::do_geometry_impl;
  virt->fill_geometry_rgba32        = NullContext::doGeometryRgba32Impl;
  virt->fill_geometry_ext           = NullContext::do_geometry_ext_impl;

  virt->fill_text_op_i              = NullContext::doTextOpIImpl;
  virt->fill_text_op_i_rgba32       = NullContext::doTextOpIRgba32Impl;
  virt->fill_text_op_i_ext          = NullContext::doTextOpIExtImpl;

  virt->fill_text_op_d              = NullContext::doTextOpDImpl;
  virt->fill_text_op_d_rgba32       = NullContext::doTextOpDRgba32Impl;
  virt->fill_text_op_d_ext          = NullContext::doTextOpDExtImpl;

  virt->fill_mask_i                 = NullContext::doMaskIImpl;
  virt->fill_mask_i_rgba32          = NullContext::doMaskDRgba32Impl;
  virt->fill_mask_i_ext             = NullContext::doMaskDExtImpl;

  virt->fill_mask_d                 = NullContext::doMaskDImpl;
  virt->fill_mask_d_Rgba32          = NullContext::doMaskDRgba32Impl;
  virt->fill_mask_d_ext             = NullContext::doMaskDExtImpl;

  virt->stroke_path_d               = NullContext::doPathDImpl;
  virt->stroke_path_d_rgba32        = NullContext::doPathDRgba32Impl;
  virt->stroke_path_d_ext           = NullContext::doPathDExtImpl;

  virt->stroke_geometry             = NullContext::do_geometry_impl;
  virt->stroke_geometry_rgba32      = NullContext::doGeometryRgba32Impl;
  virt->stroke_geometry_ext         = NullContext::do_geometry_ext_impl;

  virt->stroke_text_op_i            = NullContext::doTextOpIImpl;
  virt->stroke_text_op_i_rgba32     = NullContext::doTextOpIRgba32Impl;
  virt->stroke_text_op_i_ext        = NullContext::doTextOpIExtImpl;

  virt->stroke_text_op_d            = NullContext::doTextOpDImpl;
  virt->stroke_text_op_d_rgba32     = NullContext::doTextOpDRgba32Impl;
  virt->stroke_text_op_d_ext        = NullContext::doTextOpDExtImpl;

  virt->blit_image_i                = NullContext::blit_image_iImpl;
  virt->blit_image_d                = NullContext::blit_image_dImpl;

  virt->blit_scaled_image_i         = NullContext::blit_scaled_image_iImpl;
  virt->blit_scaled_image_d         = NullContext::blit_scaled_image_dImpl;
}

} // {bl::ContextInternal}

// bl::Context - API - Init & Destroy
// ==================================

BL_API_IMPL BLResult bl_context_init(BLContextCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_CONTEXT]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_context_init_move(BLContextCore* self, BLContextCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_context());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_CONTEXT]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_context_init_weak(BLContextCore* self, const BLContextCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_context());

  self->_d = other->_d;
  return bl::ObjectInternal::retain_instance(self);
}

BL_API_IMPL BLResult bl_context_init_as(BLContextCore* self, BLImageCore* image, const BLContextCreateInfo* cci) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_CONTEXT]._d;
  return bl_context_begin(self, image, cci);
}

BL_API_IMPL BLResult bl_context_destroy(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());

  return bl::ObjectInternal::release_virtual_instance(self);
}

// bl::Context - API - Reset
// =========================

BL_API_IMPL BLResult bl_context_reset(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());

  return bl::ObjectInternal::replace_virtual_instance(self, static_cast<BLContextCore*>(&bl_object_defaults[BL_OBJECT_TYPE_CONTEXT]));
}

// bl::Context - API - Assign
// ==========================

BL_API_IMPL BLResult bl_context_assign_move(BLContextCore* self, BLContextCore* other) noexcept {
  BL_ASSERT(self->_d.is_context());
  BL_ASSERT(other->_d.is_context());

  BLContextCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_CONTEXT]._d;
  return bl::ObjectInternal::replace_virtual_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_context_assign_weak(BLContextCore* self, const BLContextCore* other) noexcept {
  BL_ASSERT(self->_d.is_context());
  BL_ASSERT(other->_d.is_context());

  return bl::ObjectInternal::assign_virtual_instance(self, other);
}

// bl::Context - API - Accessors
// =============================

BL_API_IMPL BLContextType bl_context_get_type(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return BLContextType(impl->context_type);
}

BL_API_IMPL BLResult bl_context_get_target_size(const BLContextCore* self, BLSize* target_size_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  *target_size_out = impl->state->target_size;
  return BL_SUCCESS;
}

BL_API_IMPL BLImageCore* bl_context_get_target_image(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->state->target_image;
}

// bl::Context - API - Begin & End
// ===============================

BL_API_IMPL BLResult bl_context_begin(BLContextCore* self, BLImageCore* image, const BLContextCreateInfo* cci) noexcept {
  // Reject empty images.
  if (image->dcast().is_empty())
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (!cci)
    cci = &bl::ContextInternal::no_create_info;

  BLContextCore newO;
  BL_PROPAGATE(bl_raster_context_init_impl(&newO, image, cci));

  return bl::ObjectInternal::replace_virtual_instance(self, &newO);
}

BL_API_IMPL BLResult bl_context_end(BLContextCore* self) noexcept {
  // Currently mapped to `BLContext::reset()`.
  return bl_context_reset(self);
}

// bl::Context - API - Flush
// =========================

BL_API_IMPL BLResult bl_context_flush(BLContextCore* self, BLContextFlushFlags flags) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->flush(impl, flags);
}

// bl::Context - API - Save & Restore
// ==================================

BL_API_IMPL BLResult bl_context_save(BLContextCore* self, BLContextCookie* cookie) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->save(impl, cookie);
}

BL_API_IMPL BLResult bl_context_restore(BLContextCore* self, const BLContextCookie* cookie) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->restore(impl, cookie);
}

// bl::Context - API - Transformations
// ===================================

BL_API_IMPL BLResult bl_context_get_meta_transform(const BLContextCore* self, BLMatrix2D* transform_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  *transform_out = impl->state->meta_transform;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_context_get_user_transform(const BLContextCore* self, BLMatrix2D* transform_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  *transform_out = impl->state->user_transform;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_context_get_final_transform(const BLContextCore* self, BLMatrix2D* transform_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  *transform_out = impl->state->final_transform;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_context_user_to_meta(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->user_to_meta(impl);
}

BL_API_IMPL BLResult bl_context_apply_transform_op(BLContextCore* self, BLTransformOp op_type, const void* op_data) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->apply_transform_op(impl, op_type, op_data);
}

// bl::Context - API - Rendering Hints
// ===================================

BL_API_IMPL uint32_t bl_context_get_hint(const BLContextCore* self, BLContextHint hint_type) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  if (BL_UNLIKELY(uint32_t(hint_type) > uint32_t(BL_CONTEXT_HINT_MAX_VALUE)))
    return 0;

  return impl->state->hints.hints[hint_type];
}

BL_API_IMPL BLResult bl_context_set_hint(BLContextCore* self, BLContextHint hint_type, uint32_t value) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_hint(impl, hint_type, value);
}

BL_API_IMPL BLResult bl_context_get_hints(const BLContextCore* self, BLContextHints* hints_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  *hints_out = impl->state->hints;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_context_set_hints(BLContextCore* self, const BLContextHints* hints) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_hints(impl, hints);
}

// bl::Context - API - Approximation Options
// =========================================

BL_API_IMPL BLResult bl_context_set_flatten_mode(BLContextCore* self, BLFlattenMode mode) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_flatten_mode(impl, mode);
}

BL_API_IMPL BLResult bl_context_set_flatten_tolerance(BLContextCore* self, double tolerance) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_flatten_tolerance(impl, tolerance);
}

BL_API_IMPL BLResult bl_context_set_approximation_options(BLContextCore* self, const BLApproximationOptions* options) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_approximation_options(impl, options);
}

// bl::Context - API - Fill Style & Alpha
// ======================================

BL_API_IMPL BLResult bl_context_get_fill_style(const BLContextCore* self, BLVarCore* style_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->get_style(impl, BL_CONTEXT_STYLE_SLOT_FILL, false, style_out);
}

BL_API_IMPL BLResult bl_context_get_transformed_fill_style(const BLContextCore* self, BLVarCore* style_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->get_style(impl, BL_CONTEXT_STYLE_SLOT_FILL, true, style_out);
}

BL_API_IMPL BLResult bl_context_set_fill_style(BLContextCore* self, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style(impl, BL_CONTEXT_STYLE_SLOT_FILL, static_cast<const BLObjectCore*>(style), BL_CONTEXT_STYLE_TRANSFORM_MODE_USER);
}

BL_API_IMPL BLResult bl_context_set_fill_style_with_mode(BLContextCore* self, const BLUnknown* style, BLContextStyleTransformMode transform_mode) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style(impl, BL_CONTEXT_STYLE_SLOT_FILL, static_cast<const BLObjectCore*>(style), transform_mode);
}

BL_API_IMPL BLResult bl_context_set_fill_style_rgba(BLContextCore* self, const BLRgba* rgba) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_rgba(impl, BL_CONTEXT_STYLE_SLOT_FILL, rgba);
}

BL_API_IMPL BLResult bl_context_set_fill_style_rgba32(BLContextCore* self, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_rgba32(impl, BL_CONTEXT_STYLE_SLOT_FILL, rgba32);
}

BL_API_IMPL BLResult bl_context_set_fill_style_rgba64(BLContextCore* self, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_rgba64(impl, BL_CONTEXT_STYLE_SLOT_FILL, rgba64);
}

BL_API_IMPL BLResult bl_context_disable_fill_style(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->disable_style(impl, BL_CONTEXT_STYLE_SLOT_FILL);
}

BL_API_IMPL double bl_context_get_fill_alpha(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->state->style_alpha[BL_CONTEXT_STYLE_SLOT_FILL];
}

BL_API_IMPL BLResult bl_context_set_fill_alpha(BLContextCore* self, double alpha) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_alpha(impl, BL_CONTEXT_STYLE_SLOT_FILL, alpha);
}

// bl::Context - API - Stroke Style & Alpha
// ========================================

BL_API_IMPL BLResult bl_context_get_stroke_style(const BLContextCore* self, BLVarCore* style_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->get_style(impl, BL_CONTEXT_STYLE_SLOT_STROKE, false, style_out);
}

BL_API_IMPL BLResult bl_context_get_transformed_stroke_style(const BLContextCore* self, BLVarCore* style_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->get_style(impl, BL_CONTEXT_STYLE_SLOT_STROKE, true, style_out);
}

BL_API_IMPL BLResult bl_context_set_stroke_style(BLContextCore* self, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style(impl, BL_CONTEXT_STYLE_SLOT_STROKE, static_cast<const BLObjectCore*>(style), BL_CONTEXT_STYLE_TRANSFORM_MODE_USER);
}

BL_API_IMPL BLResult bl_context_set_stroke_style_with_mode(BLContextCore* self, const BLUnknown* style, BLContextStyleTransformMode transform_mode) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style(impl, BL_CONTEXT_STYLE_SLOT_STROKE, static_cast<const BLObjectCore*>(style), transform_mode);
}

BL_API_IMPL BLResult bl_context_set_stroke_style_rgba(BLContextCore* self, const BLRgba* rgba) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_rgba(impl, BL_CONTEXT_STYLE_SLOT_STROKE, rgba);
}

BL_API_IMPL BLResult bl_context_set_stroke_style_rgba32(BLContextCore* self, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_rgba32(impl, BL_CONTEXT_STYLE_SLOT_STROKE, rgba32);
}

BL_API_IMPL BLResult bl_context_set_stroke_style_rgba64(BLContextCore* self, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_rgba64(impl, BL_CONTEXT_STYLE_SLOT_STROKE, rgba64);
}

BL_API_IMPL BLResult bl_context_disable_stroke_style(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->disable_style(impl, BL_CONTEXT_STYLE_SLOT_STROKE);
}

BL_API_IMPL double bl_context_get_stroke_alpha(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->state->style_alpha[BL_CONTEXT_STYLE_SLOT_STROKE];
}

BL_API_IMPL BLResult bl_context_set_stroke_alpha(BLContextCore* self, double alpha) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_style_alpha(impl, BL_CONTEXT_STYLE_SLOT_STROKE, alpha);
}

BL_API_IMPL BLResult bl_context_swap_styles(BLContextCore* self, BLContextStyleSwapMode mode) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->swap_styles(impl, mode);
}

// bl::Context - API - Composition Options
// =======================================

BL_API_IMPL double bl_context_get_global_alpha(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->state->global_alpha;
}

BL_API_IMPL BLResult bl_context_set_global_alpha(BLContextCore* self, double alpha) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_global_alpha(impl, alpha);
}

BL_API_IMPL BLCompOp bl_context_get_comp_op(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return (BLCompOp)impl->state->comp_op;
}

BL_API_IMPL BLResult bl_context_set_comp_op(BLContextCore* self, BLCompOp comp_op) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_comp_op(impl, comp_op);
}

// bl::Context - API - Fill Options
// ================================

BL_API_IMPL BLFillRule bl_context_get_fill_rule(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return (BLFillRule)impl->state->fill_rule;
}

BL_API_IMPL BLResult bl_context_set_fill_rule(BLContextCore* self, BLFillRule fill_rule) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_fill_rule(impl, fill_rule);
}

// bl::Context - API - Stroke Options
// ==================================

BL_API_IMPL double bl_context_get_stroke_width(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->state->stroke_options.width;
}

BL_API_IMPL BLResult bl_context_set_stroke_width(BLContextCore* self, double width) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_width(impl, width);
}

BL_API_IMPL double bl_context_get_stroke_miter_limit(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->state->stroke_options.miter_limit;
}

BL_API_IMPL BLResult bl_context_set_stroke_miter_limit(BLContextCore* self, double miter_limit) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_miter_limit(impl, miter_limit);
}

BL_API_IMPL BLStrokeCap bl_context_get_stroke_cap(const BLContextCore* self, BLStrokeCapPosition position) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  if (BL_UNLIKELY(uint32_t(position) > uint32_t(BL_STROKE_CAP_POSITION_MAX_VALUE)))
    return (BLStrokeCap)0;

  return (BLStrokeCap)impl->state->stroke_options.caps[position];
}

BL_API_IMPL BLResult bl_context_set_stroke_cap(BLContextCore* self, BLStrokeCapPosition position, BLStrokeCap stroke_cap) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_cap(impl, position, stroke_cap);
}

BL_API_IMPL BLResult bl_context_set_stroke_caps(BLContextCore* self, BLStrokeCap stroke_cap) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_caps(impl, stroke_cap);
}

BL_API_IMPL BLStrokeJoin bl_context_get_stroke_join(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return (BLStrokeJoin)impl->state->stroke_options.join;
}

BL_API_IMPL BLResult bl_context_set_stroke_join(BLContextCore* self, BLStrokeJoin stroke_join) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_join(impl, stroke_join);
}

BL_API_IMPL BLStrokeTransformOrder bl_context_get_stroke_transform_order(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return (BLStrokeTransformOrder)impl->state->stroke_options.transform_order;
}

BL_API_IMPL BLResult bl_context_set_stroke_transform_order(BLContextCore* self, BLStrokeTransformOrder transform_order) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_transform_order(impl, transform_order);
}

BL_API_IMPL double bl_context_get_stroke_dash_offset(const BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->state->stroke_options.dash_offset;
}

BL_API_IMPL BLResult bl_context_set_stroke_dash_offset(BLContextCore* self, double dash_offset) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_dash_offset(impl, dash_offset);
}

BL_API_IMPL BLResult bl_context_get_stroke_dash_array(const BLContextCore* self, BLArrayCore* dash_array_out) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return bl_array_assign_weak(dash_array_out, &impl->state->stroke_options.dash_array);
}

BL_API_IMPL BLResult bl_context_set_stroke_dash_array(BLContextCore* self, const BLArrayCore* dash_array) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_dash_array(impl, dash_array);
}

BL_API_IMPL BLResult bl_context_get_stroke_options(const BLContextCore* self, BLStrokeOptionsCore* options) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return bl_stroke_options_assign_weak(options, &impl->state->stroke_options);
}

BL_API_IMPL BLResult bl_context_set_stroke_options(BLContextCore* self, const BLStrokeOptionsCore* options) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->set_stroke_options(impl, options);
}

// bl::Context - API - Clip Operations
// ===================================

BL_API_IMPL BLResult bl_context_clip_to_rect_i(BLContextCore* self, const BLRectI* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->clip_to_rect_i(impl, rect);
}

BL_API_IMPL BLResult bl_context_clip_to_rect_d(BLContextCore* self, const BLRect* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->clip_to_rect_d(impl, rect);
}

BL_API_IMPL BLResult bl_context_restore_clipping(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->restore_clipping(impl);
}

// bl::Context - API - Clear Geometry Operations
// =============================================

BL_API_IMPL BLResult bl_context_clear_all(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->clear_all(impl);
}

BL_API_IMPL BLResult bl_context_clear_rect_i(BLContextCore* self, const BLRectI* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->clear_recti(impl, rect);
}

BL_API_IMPL BLResult bl_context_clear_rect_d(BLContextCore* self, const BLRect* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->clear_rectd(impl, rect);
}

// bl::Context - API - Fill All Operations
// =======================================

BL_API_IMPL BLResult bl_context_fill_all(BLContextCore* self) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_all(impl);
}

BL_API_IMPL BLResult bl_context_fill_all_rgba32(BLContextCore* self, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_all_rgba32(impl, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_all_rgba64(BLContextCore* self, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_all_ext(impl, &style);
}

BL_API_IMPL BLResult bl_context_fill_all_ext(BLContextCore* self, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_all_ext(impl, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill Rect Operations
// ========================================

BL_API_IMPL BLResult bl_context_fill_rect_i(BLContextCore* self, const BLRectI* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_rect_i(impl, rect);
}

BL_API_IMPL BLResult bl_context_fill_rect_i_rgba32(BLContextCore* self, const BLRectI* rect, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_rect_i_rgba32(impl, rect, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_rect_i_rgba64(BLContextCore* self, const BLRectI* rect, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_rect_i_ext(impl, rect, &style);
}

BL_API_IMPL BLResult bl_context_fill_rect_i_ext(BLContextCore* self, const BLRectI* rect, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_rect_i_ext(impl, rect, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_fill_rect_d(BLContextCore* self, const BLRect* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_rect_d(impl, rect);
}

BL_API_IMPL BLResult bl_context_fill_rect_d_rgba32(BLContextCore* self, const BLRect* rect, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_rect_d_rgba32(impl, rect, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_rect_d_rgba64(BLContextCore* self, const BLRect* rect, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_rect_d_ext(impl, rect, &style);
}

BL_API_IMPL BLResult bl_context_fill_rect_d_ext(BLContextCore* self, const BLRect* rect, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_rect_d_ext(impl, rect, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill Path Operations
// ========================================

BL_API_IMPL BLResult bl_context_fill_path_d(BLContextCore* self, const BLPoint* origin, const BLPathCore* path) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_path_d(impl, origin, path);
}

BL_API_IMPL BLResult bl_context_fill_path_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_path_d_rgba32(impl, origin, path, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_path_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_path_d_ext(impl, origin, path, &style);
}

BL_API_IMPL BLResult bl_context_fill_path_d_ext(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_path_d_ext(impl, origin, path, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill Geometry Operations
// ============================================

BL_API_IMPL BLResult bl_context_fill_geometry(BLContextCore* self, BLGeometryType type, const void* data) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_geometry(impl, type, data);
}

BL_API_IMPL BLResult bl_context_fill_geometry_rgba32(BLContextCore* self, BLGeometryType type, const void* data, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_geometry_rgba32(impl, type, data, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_geometry_rgba64(BLContextCore* self, BLGeometryType type, const void* data, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_geometry_ext(impl, type, data, &style);
}

BL_API_IMPL BLResult bl_context_fill_geometry_ext(BLContextCore* self, BLGeometryType type, const void* data, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_geometry_ext(impl, type, data, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill UTF-8 Text Operations
// ==============================================

BL_API_IMPL BLResult bl_context_fill_utf8_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->fill_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
}

BL_API_IMPL BLResult bl_context_fill_utf8_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->fill_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_utf8_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLStringView view{text, size};
  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, &style);
}

BL_API_IMPL BLResult bl_context_fill_utf8_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_fill_utf8_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->fill_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
}

BL_API_IMPL BLResult bl_context_fill_utf8_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->fill_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_utf8_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLStringView view{text, size};
  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, &style);
}

BL_API_IMPL BLResult bl_context_fill_utf8_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill UTF-16 Text Operations
// ===============================================

BL_API_IMPL BLResult bl_context_fill_utf16_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
}

BL_API_IMPL BLResult bl_context_fill_utf16_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_utf16_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, &style);
}

BL_API_IMPL BLResult bl_context_fill_utf16_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_fill_utf16_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
}

BL_API_IMPL BLResult bl_context_fill_utf16_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_utf16_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, &style);
}

BL_API_IMPL BLResult bl_context_fill_utf16_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill UTF-32 Text Operations
// ===============================================

BL_API_IMPL BLResult bl_context_fill_utf32_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
}

BL_API_IMPL BLResult bl_context_fill_utf32_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_utf32_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, &style);
}

BL_API_IMPL BLResult bl_context_fill_utf32_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_fill_utf32_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
}

BL_API_IMPL BLResult bl_context_fill_utf32_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_utf32_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, &style);
}

BL_API_IMPL BLResult bl_context_fill_utf32_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill Glyph Run Operations
// =============================================

BL_API_IMPL BLResult bl_context_fill_glyph_run_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run);
}

BL_API_IMPL BLResult bl_context_fill_glyph_run_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_glyph_run_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, &style);
}

BL_API_IMPL BLResult bl_context_fill_glyph_run_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_fill_glyph_run_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run);
}

BL_API_IMPL BLResult bl_context_fill_glyph_run_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_glyph_run_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, &style);
}

BL_API_IMPL BLResult bl_context_fill_glyph_run_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Fill Mask Operations
// ========================================

BL_API_IMPL BLResult bl_context_fill_mask_i(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_mask_i(impl, origin, mask, mask_area);
}

BL_API_IMPL BLResult bl_context_fill_mask_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_mask_i_rgba32(impl, origin, mask, mask_area, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_mask_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_mask_i_ext(impl, origin, mask, mask_area, &style);
}

BL_API_IMPL BLResult bl_context_fill_mask_i_ext(BLContextCore* self, const BLPointI* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_mask_i_ext(impl, origin, mask, mask_area, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_fill_mask_d(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_mask_d(impl, origin, mask, mask_area);
}

BL_API_IMPL BLResult bl_context_fill_mask_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_mask_d_Rgba32(impl, origin, mask, mask_area, rgba32);
}

BL_API_IMPL BLResult bl_context_fill_mask_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->fill_mask_d_ext(impl, origin, mask, mask_area, &style);
}

BL_API_IMPL BLResult bl_context_fill_mask_d_ext(BLContextCore* self, const BLPoint* origin, const BLImageCore* mask, const BLRectI* mask_area, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->fill_mask_d_ext(impl, origin, mask, mask_area, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Stroke Rect Operations
// ==========================================

BL_API_IMPL BLResult bl_context_stroke_rect_i(BLContextCore* self, const BLRectI* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry(impl, BL_GEOMETRY_TYPE_RECTI, rect);
}

BL_API_IMPL BLResult bl_context_stroke_rect_i_rgba32(BLContextCore* self, const BLRectI* rect, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry_rgba32(impl, BL_GEOMETRY_TYPE_RECTI, rect, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_rect_i_rgba64(BLContextCore* self, const BLRectI* rect, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->stroke_geometry_ext(impl, BL_GEOMETRY_TYPE_RECTI, rect, &style);
}

BL_API_IMPL BLResult bl_context_stroke_rect_i_ext(BLContextCore* self, const BLRectI* rect, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry_ext(impl, BL_GEOMETRY_TYPE_RECTI, rect, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_stroke_rect_d(BLContextCore* self, const BLRect* rect) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry(impl, BL_GEOMETRY_TYPE_RECTD, rect);
}

BL_API_IMPL BLResult bl_context_stroke_rect_d_rgba32(BLContextCore* self, const BLRect* rect, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry_rgba32(impl, BL_GEOMETRY_TYPE_RECTD, rect, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_rect_d_rgba64(BLContextCore* self, const BLRect* rect, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->stroke_geometry_ext(impl, BL_GEOMETRY_TYPE_RECTD, rect, &style);
}

BL_API_IMPL BLResult bl_context_stroke_rect_d_ext(BLContextCore* self, const BLRect* rect, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry_ext(impl, BL_GEOMETRY_TYPE_RECTD, rect, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Stroke Path Operations
// ==========================================

BL_API_IMPL BLResult bl_context_stroke_path_d(BLContextCore* self, const BLPoint* origin, const BLPathCore* path) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_path_d(impl, origin, path);
}

BL_API_IMPL BLResult bl_context_stroke_path_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_path_d_rgba32(impl, origin, path, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_path_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->stroke_path_d_ext(impl, origin, path, &style);
}

BL_API_IMPL BLResult bl_context_stroke_path_d_ext(BLContextCore* self, const BLPoint* origin, const BLPathCore* path, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_path_d_ext(impl, origin, path, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Stroke Geometry Operations
// ==============================================

BL_API_IMPL BLResult bl_context_stroke_geometry(BLContextCore* self, BLGeometryType type, const void* data) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry(impl, type, data);
}

BL_API_IMPL BLResult bl_context_stroke_geometry_rgba32(BLContextCore* self, BLGeometryType type, const void* data, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry_rgba32(impl, type, data, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_geometry_rgba64(BLContextCore* self, BLGeometryType type, const void* data, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->stroke_geometry_ext(impl, type, data, &style);
}

BL_API_IMPL BLResult bl_context_stroke_geometry_ext(BLContextCore* self, BLGeometryType type, const void* data, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_geometry_ext(impl, type, data, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Stroke UTF-8 Text Operations
// ================================================

BL_API_IMPL BLResult bl_context_stroke_utf8_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->stroke_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
}

BL_API_IMPL BLResult bl_context_stroke_utf8_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->stroke_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_utf8_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLStringView view{text, size};
  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, &style);
}

BL_API_IMPL BLResult bl_context_stroke_utf8_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_stroke_utf8_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->stroke_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view);
}

BL_API_IMPL BLResult bl_context_stroke_utf8_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->stroke_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_utf8_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLStringView view{text, size};
  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, &style);
}

BL_API_IMPL BLResult bl_context_stroke_utf8_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const char* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLStringView view{text, size};
  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF8, &view, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Stroke UTF-16 Text Operations
// =================================================

BL_API_IMPL BLResult bl_context_stroke_utf16_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
}

BL_API_IMPL BLResult bl_context_stroke_utf16_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_utf16_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, &style);
}

BL_API_IMPL BLResult bl_context_stroke_utf16_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_stroke_utf16_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view);
}

BL_API_IMPL BLResult bl_context_stroke_utf16_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_utf16_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, &style);
}

BL_API_IMPL BLResult bl_context_stroke_utf16_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint16_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint16_t> view{text, size};
  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF16, &view, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Stroke UTF-32 Text Operations
// =================================================

BL_API_IMPL BLResult bl_context_stroke_utf32_text_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
}

BL_API_IMPL BLResult bl_context_stroke_utf32_text_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_utf32_text_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, &style);
}

BL_API_IMPL BLResult bl_context_stroke_utf32_text_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_stroke_utf32_text_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view);
}

BL_API_IMPL BLResult bl_context_stroke_utf32_text_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_utf32_text_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, &style);
}

BL_API_IMPL BLResult bl_context_stroke_utf32_text_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const uint32_t* text, size_t size, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLArrayView<uint32_t> view{text, size};
  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_UTF32, &view, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Stroke Glyph Run Operations
// ===============================================

BL_API_IMPL BLResult bl_context_stroke_glyph_run_i(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_text_op_i(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run);
}

BL_API_IMPL BLResult bl_context_stroke_glyph_run_i_rgba32(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_text_op_i_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_glyph_run_i_rgba64(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, &style);
}

BL_API_IMPL BLResult bl_context_stroke_glyph_run_i_ext(BLContextCore* self, const BLPointI* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_text_op_i_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, static_cast<const BLObjectCore*>(style));
}

BL_API_IMPL BLResult bl_context_stroke_glyph_run_d(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_text_op_d(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run);
}

BL_API_IMPL BLResult bl_context_stroke_glyph_run_d_rgba32(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint32_t rgba32) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_text_op_d_rgba32(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, rgba32);
}

BL_API_IMPL BLResult bl_context_stroke_glyph_run_d_rgba64(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, uint64_t rgba64) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  BLVarCore style = BLInternal::make_inline_style(BLRgba64(rgba64));
  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, &style);
}

BL_API_IMPL BLResult bl_context_stroke_glyph_run_d_ext(BLContextCore* self, const BLPoint* origin, const BLFontCore* font, const BLGlyphRun* glyph_run, const BLUnknown* style) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->stroke_text_op_d_ext(impl, origin, font, BL_CONTEXT_RENDER_TEXT_OP_GLYPH_RUN, glyph_run, static_cast<const BLObjectCore*>(style));
}

// bl::Context - API - Blit Operations
// ===================================

BL_API_IMPL BLResult bl_context_blit_image_i(BLContextCore* self, const BLPointI* pt, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->blit_image_i(impl, pt, img, img_area);
}

BL_API_IMPL BLResult bl_context_blit_image_d(BLContextCore* self, const BLPoint* pt, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->blit_image_d(impl, pt, img, img_area);
}

BL_API_IMPL BLResult bl_context_blit_scaled_image_i(BLContextCore* self, const BLRectI* rect, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->blit_scaled_image_i(impl, rect, img, img_area);
}

BL_API_IMPL BLResult bl_context_blit_scaled_image_d(BLContextCore* self, const BLRect* rect, const BLImageCore* img, const BLRectI* img_area) noexcept {
  BL_ASSERT(self->_d.is_context());
  BLContextImpl* impl = self->_impl();

  return impl->virt->blit_scaled_image_d(impl, rect, img, img_area);
}

// bl::Context - Runtime Registration
// ==================================

void bl_context_rt_init(BLRuntimeContext* rt) noexcept {
  auto& default_context = bl::ContextInternal::default_context;

  // Initialize a NullContextImpl.
  bl::ContextInternal::init_state(&bl::ContextInternal::null_state);
  bl::ContextInternal::init_null_context_virt(&default_context.virt);

  // Initialize a default context object (that points to NullContextImpl).
  default_context.impl->virt = &default_context.virt;
  default_context.impl->state = &bl::ContextInternal::null_state;
  bl_object_defaults[BL_OBJECT_TYPE_CONTEXT]._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_CONTEXT), &default_context.impl);

  // Initialize built-in rendering context implementations.
  bl_raster_context_on_init(rt);
}
