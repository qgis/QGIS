// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/bitarray_p.h>
#include <blend2d/core/glyphbuffer_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/fontface_p.h>
#include <blend2d/core/fontfeaturesettings_p.h>
#include <blend2d/core/matrix.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otlayout_p.h>

namespace bl {
namespace FontInternal {

// bl::Font - Globals
// ==================

static BLObjectEternalImpl<BLFontPrivateImpl> default_font;

// bl::Font - Internal Utilities
// =============================

static void bl_font_calc_properties(BLFontPrivateImpl* font_impl, const BLFontFacePrivateImpl* face_impl, float size) noexcept {
  const BLFontDesignMetrics& dm = face_impl->design_metrics;

  double y_scale = dm.units_per_em ? double(size) / double(dm.units_per_em) : 0.0;
  double x_scale = y_scale;

  font_impl->metrics.size                   = size;
  font_impl->metrics.ascent                 = float(double(dm.ascent                ) * y_scale);
  font_impl->metrics.descent                = float(double(dm.descent               ) * y_scale);
  font_impl->metrics.line_gap                = float(double(dm.line_gap               ) * y_scale);
  font_impl->metrics.x_height                = float(double(dm.x_height               ) * y_scale);
  font_impl->metrics.cap_height              = float(double(dm.cap_height             ) * y_scale);
  font_impl->metrics.v_ascent                = float(double(dm.v_ascent               ) * y_scale);
  font_impl->metrics.v_descent               = float(double(dm.v_descent              ) * y_scale);
  font_impl->metrics.x_min                   = float(double(dm.glyph_bounding_box.x0   ) * x_scale);
  font_impl->metrics.y_min                   = float(double(dm.glyph_bounding_box.y0   ) * y_scale);
  font_impl->metrics.x_max                   = float(double(dm.glyph_bounding_box.x1   ) * x_scale);
  font_impl->metrics.y_max                   = float(double(dm.glyph_bounding_box.y1   ) * y_scale);
  font_impl->metrics.underline_position      = float(double(dm.underline_position     ) * y_scale);
  font_impl->metrics.underline_thickness     = float(double(dm.underline_thickness    ) * y_scale);
  font_impl->metrics.strikethrough_position  = float(double(dm.strikethrough_position ) * y_scale);
  font_impl->metrics.strikethrough_thickness = float(double(dm.strikethrough_thickness) * y_scale);
  font_impl->matrix.reset(x_scale, 0.0, 0.0, -y_scale);
}

// bl::Font - Internals - Alloc & Free Impl
// ========================================

static BL_INLINE BLResult alloc_impl(BLFontCore* self, const BLFontFaceCore* face, float size) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontPrivateImpl>(self, info));

  BLFontPrivateImpl* impl = get_impl(self);
  bl_call_ctor(impl->face.dcast(), face->dcast());
  bl_call_ctor(impl->feature_settings.dcast());
  bl_call_ctor(impl->variation_settings.dcast());
  impl->weight = 0;
  impl->stretch = 0;
  impl->style = 0;
  bl_font_calc_properties(impl, FontFaceInternal::get_impl(face), size);
  return BL_SUCCESS;
}

BLResult free_impl(BLFontPrivateImpl* impl) noexcept {
  bl_call_dtor(impl->variation_settings.dcast());
  bl_call_dtor(impl->feature_settings.dcast());
  bl_call_dtor(impl->face.dcast());

  return ObjectInternal::free_impl(impl);
}

// bl::Font - Internals - Make Mutable
// ===================================

static BL_NOINLINE BLResult make_mutable_internal(BLFontCore* self) noexcept {
  BLFontPrivateImpl* self_impl = get_impl(self);

  BLFontCore newO;
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontPrivateImpl>(&newO, info));

  BLFontPrivateImpl* new_impl = get_impl(&newO);
  bl_call_ctor(new_impl->face.dcast(), self_impl->face.dcast());
  new_impl->weight = self_impl->weight;
  new_impl->stretch = self_impl->stretch;
  new_impl->style = self_impl->style;
  new_impl->reserved = 0;
  new_impl->metrics = self_impl->metrics;
  new_impl->matrix = self_impl->matrix;
  bl_call_ctor(new_impl->feature_settings.dcast(), self_impl->feature_settings.dcast());
  bl_call_ctor(new_impl->variation_settings.dcast(), self_impl->variation_settings.dcast());

  return replace_instance(self, &newO);
}

static BL_INLINE BLResult make_mutable(BLFontCore* self) noexcept {
  if (is_instance_mutable(self))
    return BL_SUCCESS;

  return make_mutable_internal(self);
}

} // {FontInternal}
} // {bl}

// bl::Font - Init & Destroy
// =========================

BL_API_IMPL BLResult bl_font_init(BLFontCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_init_move(BLFontCore* self, BLFontCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_init_weak(BLFontCore* self, const BLFontCore* other) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_font_destroy(BLFontCore* self) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  return release_instance(self);
}

// bl::Font - Reset
// ================

BL_API_IMPL BLResult bl_font_reset(BLFontCore* self) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  return replace_instance(self, static_cast<BLFontCore*>(&bl_object_defaults[BL_OBJECT_TYPE_FONT]));
}

// bl::Font - Assign
// =================

BL_API_IMPL BLResult bl_font_assign_move(BLFontCore* self, BLFontCore* other) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(other->_d.is_font());

  BLFontCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT]._d;
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_font_assign_weak(BLFontCore* self, const BLFontCore* other) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(other->_d.is_font());

  retain_instance(other);
  return replace_instance(self, other);
}

// bl::Font - Equality & Comparison
// ================================

BL_API_IMPL bool bl_font_equals(const BLFontCore* a, const BLFontCore* b) noexcept {
  BL_ASSERT(a->_d.is_font());
  BL_ASSERT(b->_d.is_font());

  return a->_d.impl == b->_d.impl;
}

// bl::Font - Create
// =================

BL_API_IMPL BLResult bl_font_create_from_face(BLFontCore* self, const BLFontFaceCore* face, float size) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(face->_d.is_font_face());

  if (!face->dcast().is_valid())
    return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);

  BLFontPrivateImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(face);

    self_impl->feature_settings.dcast().clear();
    self_impl->variation_settings.dcast().clear();
    self_impl->weight = 0;
    self_impl->stretch = 0;
    self_impl->style = 0;
    bl_font_calc_properties(self_impl, face_impl, size);

    return bl::ObjectInternal::assign_virtual_instance(&self_impl->face, face);
  }
  else {
    BLFontCore newO;
    BL_PROPAGATE(alloc_impl(&newO, face, size));
    return replace_instance(self, &newO);
  }
}

BL_API_IMPL BLResult bl_font_create_from_face_with_settings(BLFontCore* self, const BLFontFaceCore* face, float size, const BLFontFeatureSettingsCore* feature_settings, const BLFontVariationSettingsCore* variation_settings) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(face->_d.is_font_face());

  if (feature_settings == nullptr)
    feature_settings = static_cast<BLFontFeatureSettings*>(&bl_object_defaults[BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS]);

  if (variation_settings == nullptr)
    variation_settings = static_cast<BLFontVariationSettings*>(&bl_object_defaults[BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS]);

  BL_ASSERT(feature_settings->_d.is_font_feature_settings());
  BL_ASSERT(variation_settings->_d.is_font_variation_settings());

  if (!face->dcast().is_valid())
    return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);

  BLFontPrivateImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(face);

    self_impl->feature_settings.dcast().assign(feature_settings->dcast());
    self_impl->variation_settings.dcast().assign(variation_settings->dcast());
    self_impl->weight = 0;
    self_impl->stretch = 0;
    self_impl->style = 0;
    bl_font_calc_properties(self_impl, face_impl, size);

    return bl::ObjectInternal::assign_virtual_instance(&self_impl->face, face);
  }
  else {
    BLFontCore newO;
    BL_PROPAGATE(alloc_impl(&newO, face, size));

    BLFontPrivateImpl* new_impl = get_impl(&newO);
    new_impl->feature_settings.dcast().assign(feature_settings->dcast());
    new_impl->variation_settings.dcast().assign(variation_settings->dcast());
    return replace_instance(self, &newO);
  }
}

// bl::Font - Accessors
// ====================

BL_API_IMPL BLResult bl_font_get_face(const BLFontCore* self, BLFontFaceCore* out) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(out->_d.is_font_face());

  BLFontPrivateImpl* self_impl = get_impl(self);
  return bl_font_face_assign_weak(out, &self_impl->face);
}

BL_API_IMPL float bl_font_get_size(const BLFontCore* self) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  return self_impl->metrics.size;
}

BL_API_IMPL BLResult bl_font_set_size(BLFontCore* self, float size) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  if (get_impl(self)->face.dcast().is_empty())
    return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);

  BL_PROPAGATE(make_mutable(self));
  BLFontPrivateImpl* self_impl = get_impl(self);

  bl_font_calc_properties(self_impl, bl::FontFaceInternal::get_impl(&self_impl->face), size);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_get_metrics(const BLFontCore* self, BLFontMetrics* out) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  *out = self_impl->metrics;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_get_matrix(const BLFontCore* self, BLFontMatrix* out) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  *out = self_impl->matrix;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_get_design_metrics(const BLFontCore* self, BLFontDesignMetrics* out) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);

  *out = face_impl->design_metrics;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_get_feature_settings(const BLFontCore* self, BLFontFeatureSettingsCore* out) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(out->_d.is_font_feature_settings());

  BLFontPrivateImpl* self_impl = get_impl(self);
  return bl_font_feature_settings_assign_weak(out, &self_impl->feature_settings);
}

BL_API_IMPL BLResult bl_font_set_feature_settings(BLFontCore* self, const BLFontFeatureSettingsCore* feature_settings) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(feature_settings->_d.is_font_feature_settings());

  if (get_impl(self)->face.dcast().is_empty())
    return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);

  BL_PROPAGATE(make_mutable(self));
  BLFontPrivateImpl* self_impl = get_impl(self);
  return bl_font_feature_settings_assign_weak(&self_impl->feature_settings, feature_settings);
}

BL_API_IMPL BLResult bl_font_reset_feature_settings(BLFontCore* self) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  // Don't make the font mutable if there are no feature settings set.
  if (get_impl(self)->feature_settings.dcast().is_empty())
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self));
  BLFontPrivateImpl* self_impl = get_impl(self);
  return bl_font_feature_settings_reset(&self_impl->feature_settings);
}

BL_API_IMPL BLResult bl_font_get_variation_settings(const BLFontCore* self, BLFontVariationSettingsCore* out) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(out->_d.is_font_variation_settings());

  BLFontPrivateImpl* self_impl = get_impl(self);
  return bl_font_variation_settings_assign_weak(out, &self_impl->variation_settings);
}

BL_API_IMPL BLResult bl_font_set_variation_settings(BLFontCore* self, const BLFontVariationSettingsCore* variation_settings) noexcept {
  using namespace bl::FontInternal;

  BL_ASSERT(self->_d.is_font());
  BL_ASSERT(variation_settings->_d.is_font_variation_settings());

  if (get_impl(self)->face.dcast().is_empty())
    return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);

  BL_PROPAGATE(make_mutable(self));
  BLFontPrivateImpl* self_impl = get_impl(self);
  return bl_font_variation_settings_assign_weak(&self_impl->variation_settings, variation_settings);
}

BL_API_IMPL BLResult bl_font_reset_variation_settings(BLFontCore* self) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  // Don't make the font mutable if there are no variation settings set.
  if (get_impl(self)->variation_settings.dcast().is_empty())
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self));
  BLFontPrivateImpl* self_impl = get_impl(self);
  return bl_font_variation_settings_reset(&self_impl->variation_settings);
}

// bl::Font - Shaping
// ==================

BL_API_IMPL BLResult bl_font_shape(const BLFontCore* self, BLGlyphBufferCore* gb) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BL_PROPAGATE(bl_font_map_text_to_glyphs(self, gb, nullptr));

  bl::OpenType::OTFaceImpl* ot_face_impl = bl::FontFaceInternal::get_impl<bl::OpenType::OTFaceImpl>(&self->dcast().face());
  if (ot_face_impl->layout.gsub().lookup_count) {
    BLBitArray plan;
    BL_PROPAGATE(bl::OpenType::LayoutImpl::calculate_gsub_plan(ot_face_impl, self->dcast().feature_settings(), &plan));
    BL_PROPAGATE(bl_font_apply_gsub(self, gb, &plan));
  }

  return bl_font_position_glyphs(self, gb);
}

BL_API_IMPL BLResult bl_font_map_text_to_glyphs(const BLFontCore* self, BLGlyphBufferCore* gb, BLGlyphMappingState* state_out) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);
  BLGlyphBufferPrivateImpl* gb_impl = bl_glyph_buffer_get_impl(gb);

  if (!gb_impl->size)
    return BL_SUCCESS;

  if (BL_UNLIKELY(!(gb_impl->flags & BL_GLYPH_RUN_FLAG_UCS4_CONTENT)))
    return bl_make_error(BL_ERROR_INVALID_STATE);

  BLGlyphMappingState state;
  if (!state_out)
    state_out = &state;

  BL_PROPAGATE(face_impl->funcs.map_text_to_glyphs(face_impl, gb_impl->content, gb_impl->size, state_out));

  gb_impl->flags = gb_impl->flags & ~BL_GLYPH_RUN_FLAG_UCS4_CONTENT;
  if (state_out->undefined_count > 0)
    gb_impl->flags |= BL_GLYPH_RUN_FLAG_UNDEFINED_GLYPHS;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_position_glyphs(const BLFontCore* self, BLGlyphBufferCore* gb) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);
  BLGlyphBufferPrivateImpl* gb_impl = bl_glyph_buffer_get_impl(gb);

  if (!gb_impl->size)
    return BL_SUCCESS;

  if (BL_UNLIKELY(gb_impl->flags & BL_GLYPH_RUN_FLAG_UCS4_CONTENT))
    return bl_make_error(BL_ERROR_INVALID_STATE);

  if (!(gb_impl->flags & BL_GLYPH_BUFFER_GLYPH_ADVANCES)) {
    BL_PROPAGATE(gb_impl->ensure_placement());
    face_impl->funcs.get_glyph_advances(face_impl, gb_impl->content, sizeof(uint32_t), gb_impl->placement_data, gb_impl->size);
    gb_impl->glyph_run.placement_type = uint8_t(BL_GLYPH_PLACEMENT_TYPE_ADVANCE_OFFSET);
    gb_impl->flags |= BL_GLYPH_BUFFER_GLYPH_ADVANCES;
  }

  using OTFaceImpl = bl::OpenType::OTFaceImpl;
  using OTFaceFlags = bl::OpenType::OTFaceFlags;

  OTFaceImpl* otFaceI = bl::FontFaceInternal::get_impl<OTFaceImpl>(&self->dcast().face());

  if (bl_test_flag(otFaceI->ot_flags, OTFaceFlags::kGPosLookupList)) {
    BLBitArray plan;
    BL_PROPAGATE(bl::OpenType::LayoutImpl::calculate_gpos_plan(otFaceI, self->dcast().feature_settings(), &plan));
    BL_PROPAGATE(bl_font_apply_gpos(self, gb, &plan));
  }

  constexpr OTFaceFlags kKernFlags = OTFaceFlags::kGPosKernAvailable | OTFaceFlags::kLegacyKernAvailable;
  if ((otFaceI->ot_flags & kKernFlags) == OTFaceFlags::kLegacyKernAvailable) {
    if (self_impl->feature_settings.dcast().get_value(BL_MAKE_TAG('k', 'e', 'r', 'n')) != 0u) {
      BL_PROPAGATE(face_impl->funcs.apply_kern(face_impl, gb_impl->content, gb_impl->placement_data, gb_impl->size));
    }
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_apply_kerning(const BLFontCore* self, BLGlyphBufferCore* gb) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);
  BLGlyphBufferPrivateImpl* gb_impl = bl_glyph_buffer_get_impl(gb);

  if (!gb_impl->size)
    return BL_SUCCESS;

  if (BL_UNLIKELY(!(gb_impl->placement_data)))
    return bl_make_error(BL_ERROR_INVALID_STATE);

  return face_impl->funcs.apply_kern(face_impl, gb_impl->content, gb_impl->placement_data, gb_impl->size);
}

BL_API_IMPL BLResult bl_font_apply_gsub(const BLFontCore* self, BLGlyphBufferCore* gb, const BLBitArrayCore* lookups) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);

  return face_impl->funcs.apply_gsub(face_impl, static_cast<BLGlyphBuffer*>(gb), lookups->dcast().data(), lookups->dcast().word_count());
}

BL_API_IMPL BLResult bl_font_apply_gpos(const BLFontCore* self, BLGlyphBufferCore* gb, const BLBitArrayCore* lookups) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);
  BLGlyphBufferPrivateImpl* gb_impl = bl_glyph_buffer_get_impl(gb);

  if (!gb_impl->size)
    return BL_SUCCESS;

  if (BL_UNLIKELY(!(gb_impl->placement_data)))
    return bl_make_error(BL_ERROR_INVALID_STATE);

  return face_impl->funcs.apply_gpos(face_impl, static_cast<BLGlyphBuffer*>(gb), lookups->dcast().data(), lookups->dcast().word_count());
}

BL_API_IMPL BLResult bl_font_get_text_metrics(const BLFontCore* self, BLGlyphBufferCore* gb, BLTextMetrics* out) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLGlyphBufferPrivateImpl* gb_impl = bl_glyph_buffer_get_impl(gb);

  out->reset();
  if (!(gb_impl->flags & BL_GLYPH_BUFFER_GLYPH_ADVANCES)) {
    BL_PROPAGATE(bl_font_shape(self, gb));
    gb_impl = bl_glyph_buffer_get_impl(gb);
  }

  size_t size = gb_impl->size;
  if (!size)
    return BL_SUCCESS;

  BLPoint advance {};

  const uint32_t* glyph_data = gb_impl->content;
  const BLGlyphPlacement* placement_data = gb_impl->placement_data;

  for (size_t i = 0; i < size; i++) {
    advance += BLPoint(placement_data[i].advance);
  }

  BLBoxI glyph_bounds[2];
  uint32_t border_glyphs[2] = { glyph_data[0], glyph_data[size - 1] };

  BL_PROPAGATE(bl_font_get_glyph_bounds(self, border_glyphs, intptr_t(sizeof(uint32_t)), glyph_bounds, 2));
  out->advance = advance;

  double lsb = glyph_bounds[0].x0;
  double rsb = placement_data[size - 1].advance.x - glyph_bounds[1].x1;

  out->leading_bearing.reset(lsb, 0);
  out->trailing_bearing.reset(rsb, 0);
  out->bounding_box.reset(glyph_bounds[0].x0, 0.0, advance.x - rsb, 0.0);

  const BLFontMatrix& m = self_impl->matrix;
  BLPoint scale_pt = BLPoint(m.m00, m.m11);

  out->advance *= scale_pt;
  out->leading_bearing *= scale_pt;
  out->trailing_bearing *= scale_pt;
  out->bounding_box *= scale_pt;

  return BL_SUCCESS;
}

// bl::Font - Low-Level API
// ========================

BL_API_IMPL BLResult bl_font_get_glyph_bounds(const BLFontCore* self, const uint32_t* glyph_data, intptr_t glyph_advance, BLBoxI* out, size_t count) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);

  return face_impl->funcs.get_glyph_bounds(face_impl, glyph_data, glyph_advance, out, count);
}

BL_API_IMPL BLResult bl_font_get_glyph_advances(const BLFontCore* self, const uint32_t* glyph_data, intptr_t glyph_advance, BLGlyphPlacement* out, size_t count) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);

  return face_impl->funcs.get_glyph_advances(face_impl, glyph_data, glyph_advance, out, count);
}

// bl::Font - Glyph Outlines
// =========================

static BLResult BL_CDECL bl_font_dummy_path_sink(BLPathCore* path, const void* info, void* user_data) noexcept {
  bl_unused(path, info, user_data);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_get_glyph_outlines(const BLFontCore* self, BLGlyphId glyph_id, const BLMatrix2D* user_transform, BLPathCore* out, BLPathSinkFunc sink, void* user_data) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);

  BLMatrix2D final_transform;
  const BLFontMatrix& fMat = self_impl->matrix;

  if (user_transform)
    bl_font_matrix_multiply(&final_transform, &fMat, user_transform);
  else
    final_transform.reset(fMat.m00, fMat.m01, fMat.m10, fMat.m11, 0.0, 0.0);

  bl::ScopedBufferTmp<BL_FONT_GET_GLYPH_OUTLINE_BUFFER_SIZE> tmp_buffer;
  BLGlyphOutlineSinkInfo sink_info;
  BL_PROPAGATE(face_impl->funcs.get_glyph_outlines(face_impl, glyph_id, &final_transform, static_cast<BLPath*>(out), &sink_info.contour_count, &tmp_buffer));

  if (!sink)
    return BL_SUCCESS;

  sink_info.glyph_index = 0;
  return sink(out, &sink_info, user_data);
}

BL_API_IMPL BLResult bl_font_get_glyph_run_outlines(const BLFontCore* self, const BLGlyphRun* glyph_run, const BLMatrix2D* user_transform, BLPathCore* out, BLPathSinkFunc sink, void* user_data) noexcept {
  using namespace bl::FontInternal;
  BL_ASSERT(self->_d.is_font());

  BLFontPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(&self_impl->face);

  if (!glyph_run->size)
    return BL_SUCCESS;

  BLMatrix2D final_transform;
  const BLFontMatrix& fMat = self_impl->matrix;

  if (user_transform) {
    bl_font_matrix_multiply(&final_transform, &fMat, user_transform);
  }
  else {
    user_transform = &bl::TransformInternal::identity_transform;
    final_transform.reset(fMat.m00, fMat.m01, fMat.m10, fMat.m11, 0.0, 0.0);
  }

  if (!sink)
    sink = bl_font_dummy_path_sink;

  bl::ScopedBufferTmp<BL_FONT_GET_GLYPH_OUTLINE_BUFFER_SIZE> tmp_buffer;
  BLGlyphOutlineSinkInfo sink_info;

  uint32_t placement_type = glyph_run->placement_type;
  BLGlyphRunIterator it(*glyph_run);
  auto get_glyph_outlines_func = face_impl->funcs.get_glyph_outlines;

  if (it.has_placement() && placement_type != BL_GLYPH_PLACEMENT_TYPE_NONE) {
    BLMatrix2D offset_transform(1.0, 0.0, 0.0, 1.0, final_transform.m20, final_transform.m21);

    switch (placement_type) {
      case BL_GLYPH_PLACEMENT_TYPE_ADVANCE_OFFSET:
      case BL_GLYPH_PLACEMENT_TYPE_DESIGN_UNITS:
        offset_transform.m00 = final_transform.m00;
        offset_transform.m01 = final_transform.m01;
        offset_transform.m10 = final_transform.m10;
        offset_transform.m11 = final_transform.m11;
        break;

      case BL_GLYPH_PLACEMENT_TYPE_USER_UNITS:
        offset_transform.m00 = user_transform->m00;
        offset_transform.m01 = user_transform->m01;
        offset_transform.m10 = user_transform->m10;
        offset_transform.m11 = user_transform->m11;
        break;
    }

    if (placement_type == BL_GLYPH_PLACEMENT_TYPE_ADVANCE_OFFSET) {
      double ox = final_transform.m20;
      double oy = final_transform.m21;
      double px;
      double py;

      while (!it.at_end()) {
        const BLGlyphPlacement& pos = it.placement<BLGlyphPlacement>();

        px = pos.placement.x;
        py = pos.placement.y;
        final_transform.m20 = px * offset_transform.m00 + py * offset_transform.m10 + ox;
        final_transform.m21 = px * offset_transform.m01 + py * offset_transform.m11 + oy;

        sink_info.glyph_index = it.index;
        BL_PROPAGATE(get_glyph_outlines_func(face_impl, it.glyph_id(), &final_transform, static_cast<BLPath*>(out), &sink_info.contour_count, &tmp_buffer));
        BL_PROPAGATE(sink(out, &sink_info, user_data));
        it.advance();

        px = pos.advance.x;
        py = pos.advance.y;
        ox += px * offset_transform.m00 + py * offset_transform.m10;
        oy += px * offset_transform.m01 + py * offset_transform.m11;
      }
    }
    else {
      while (!it.at_end()) {
        const BLPoint& placement = it.placement<BLPoint>();
        final_transform.m20 = placement.x * offset_transform.m00 + placement.y * offset_transform.m10 + offset_transform.m20;
        final_transform.m21 = placement.x * offset_transform.m01 + placement.y * offset_transform.m11 + offset_transform.m21;

        sink_info.glyph_index = it.index;
        BL_PROPAGATE(get_glyph_outlines_func(face_impl, it.glyph_id(), &final_transform, static_cast<BLPath*>(out), &sink_info.contour_count, &tmp_buffer));
        BL_PROPAGATE(sink(out, &sink_info, user_data));
        it.advance();
      }
    }
  }
  else {
    while (!it.at_end()) {
      sink_info.glyph_index = it.index;
      BL_PROPAGATE(get_glyph_outlines_func(face_impl, it.glyph_id(), &final_transform, static_cast<BLPath*>(out), &sink_info.contour_count, &tmp_buffer));
      BL_PROPAGATE(sink(out, &sink_info, user_data));
      it.advance();
    }
  }

  return BL_SUCCESS;
}

// bl::Font - Runtime Registration
// ===============================

void bl_font_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  // Initialize BLFont built-ins.
  bl_font_impl_ctor(&bl::FontInternal::default_font.impl);

  bl_object_defaults[BL_OBJECT_TYPE_FONT]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT),
    &bl::FontInternal::default_font.impl);
}
