// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/glyphbuffer_p.h>
#include <blend2d/core/filesystem.h>
#include <blend2d/core/fontface_p.h>
#include <blend2d/core/matrix.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/opentype/otcore_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>
#include <blend2d/threading/uniqueidgenerator_p.h>
#include <blend2d/unicode/unicode_p.h>

// bl::FontFace - Globals
// ======================

BLFontFacePrivateFuncs bl_null_font_face_funcs;
static BLObjectEternalVirtualImpl<BLFontFacePrivateImpl, BLFontFaceVirt> bl_font_face_default_impl;

// bl::FontFace - Default Impl
// ===========================

BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)

static BLResult BL_CDECL bl_null_font_face_impl_destroy(BLObjectImpl* impl) noexcept {
  return BL_SUCCESS;
}

static BLResult BL_CDECL bl_null_font_face_map_text_to_glyphs(
  const BLFontFaceImpl* impl,
  uint32_t* content,
  size_t count,
  BLGlyphMappingState* state) noexcept {

  state->reset();
  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

static BLResult BL_CDECL bl_null_font_face_get_glyph_bounds(
  const BLFontFaceImpl* impl,
  const uint32_t* glyph_data,
  intptr_t glyph_advance,
  BLBoxI* boxes,
  size_t count) noexcept {

  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

static BLResult BL_CDECL bl_null_font_face_get_glyph_advances(
  const BLFontFaceImpl* impl,
  const uint32_t* glyph_data,
  intptr_t glyph_advance,
  BLGlyphPlacement* placement_data,
  size_t count) noexcept {

  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

static BLResult BL_CDECL bl_null_font_face_get_glyph_outlines(
  const BLFontFaceImpl* impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* user_transform,
  BLPath* out,
  size_t* contour_count_out,
  bl::ScopedBuffer* tmp_buffer) noexcept {

  *contour_count_out = 0;
  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

static BLResult BL_CDECL bl_null_font_face_apply_kern(
  const BLFontFaceImpl* face_impl,
  uint32_t* glyph_data,
  BLGlyphPlacement* placement_data,
  size_t count) noexcept {

  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

static BLResult BL_CDECL bl_null_font_face_apply_gsub(
  const BLFontFaceImpl* impl,
  BLGlyphBuffer* gb,
  const uint32_t* bit_words,
  size_t bit_word_count) noexcept {

  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

static BLResult BL_CDECL bl_null_font_face_apply_gpos(
  const BLFontFaceImpl* impl,
  BLGlyphBuffer* gb,
  const uint32_t* bit_words,
  size_t bit_word_count) noexcept {

  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

static BLResult BL_CDECL bl_null_font_face_position_glyphs(
  const BLFontFaceImpl* impl,
  uint32_t* glyph_data,
  BLGlyphPlacement* placement_data,
  size_t count) noexcept {

  return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);
}

BL_DIAGNOSTIC_POP

// bl::FontFace - Init & Destroy
// =============================

BLResult bl_font_face_init(BLFontFaceCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_FACE]._d;
  return BL_SUCCESS;
}

BLResult bl_font_face_init_move(BLFontFaceCore* self, BLFontFaceCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_face());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_FACE]._d;

  return BL_SUCCESS;
}

BLResult bl_font_face_init_weak(BLFontFaceCore* self, const BLFontFaceCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_face());

  return bl_object_private_init_weak_tagged(self, other);
}

BLResult bl_font_face_destroy(BLFontFaceCore* self) noexcept {
  BL_ASSERT(self->_d.is_font_face());

  return bl::ObjectInternal::release_virtual_instance(self);
}

// bl::FontFace - Reset
// ====================

BLResult bl_font_face_reset(BLFontFaceCore* self) noexcept {
  BL_ASSERT(self->_d.is_font_face());

  return bl::ObjectInternal::replace_virtual_instance(self, static_cast<BLFontFaceCore*>(&bl_object_defaults[BL_OBJECT_TYPE_FONT_FACE]));
}

// bl::FontFace - Assign
// =====================

BLResult bl_font_face_assign_move(BLFontFaceCore* self, BLFontFaceCore* other) noexcept {
  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(other->_d.is_font_face());

  BLFontFaceCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_FACE]._d;
  return bl::ObjectInternal::replace_virtual_instance(self, &tmp);
}

BLResult bl_font_face_assign_weak(BLFontFaceCore* self, const BLFontFaceCore* other) noexcept {
  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(other->_d.is_font_face());

  return bl::ObjectInternal::assign_virtual_instance(self, other);
}

// bl::FontFace - Equality & Comparison
// ====================================

bool bl_font_face_equals(const BLFontFaceCore* a, const BLFontFaceCore* b) noexcept {
  BL_ASSERT(a->_d.is_font_face());
  BL_ASSERT(b->_d.is_font_face());

  return a->_d.impl == b->_d.impl;
}

// bl::FontFace - Create
// =====================

BLResult bl_font_face_create_from_file(BLFontFaceCore* self, const char* file_name, BLFileReadFlags read_flags) noexcept {
  BL_ASSERT(self->_d.is_font_face());

  BLFontData font_data;
  BL_PROPAGATE(font_data.create_from_file(file_name, read_flags));
  return bl_font_face_create_from_data(self, &font_data, 0);
}

BLResult bl_font_face_create_from_data(BLFontFaceCore* self, const BLFontDataCore* font_data, uint32_t face_index) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(font_data->_d.is_font_data());

  if (BL_UNLIKELY(!font_data->dcast().is_valid()))
    return bl_make_error(BL_ERROR_NOT_INITIALIZED);

  if (face_index >= font_data->dcast().face_count())
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLFontFaceCore newO;
  BL_PROPAGATE(bl::OpenType::create_open_type_face(&newO, static_cast<const BLFontData*>(font_data), face_index));

  // TODO: Move to OTFace?
  get_impl<bl::OpenType::OTFaceImpl>(&newO)->unique_id = BLUniqueIdGenerator::generate_id(BLUniqueIdGenerator::Domain::kAny);

  return bl::ObjectInternal::replace_virtual_instance(self, &newO);
}

// bl::FontFace - Accessors
// ========================

BLResult bl_font_face_get_full_name(const BLFontFaceCore* self, BLStringCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(out->_d.is_string());

  BLFontFacePrivateImpl* self_impl = get_impl(self);
  return bl_string_assign_weak(out, &self_impl->full_name);
}

BLResult bl_font_face_get_family_name(const BLFontFaceCore* self, BLStringCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(out->_d.is_string());

  BLFontFacePrivateImpl* self_impl = get_impl(self);
  return bl_string_assign_weak(out, &self_impl->family_name);
}

BLResult bl_font_face_get_subfamily_name(const BLFontFaceCore* self, BLStringCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(out->_d.is_string());

  BLFontFacePrivateImpl* self_impl = get_impl(self);
  return bl_string_assign_weak(out, &self_impl->subfamily_name);
}

BLResult bl_font_face_get_post_script_name(const BLFontFaceCore* self, BLStringCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(out->_d.is_string());

  BLFontFacePrivateImpl* self_impl = get_impl(self);
  return bl_string_assign_weak(out, &self_impl->post_script_name);
}

BLResult bl_font_face_get_face_info(const BLFontFaceCore* self, BLFontFaceInfo* out) noexcept {
  BL_ASSERT(self->_d.is_font_face());

  *out = self->dcast().face_info();
  return BL_SUCCESS;
}

BLResult bl_font_face_get_design_metrics(const BLFontFaceCore* self, BLFontDesignMetrics* out) noexcept {
  BL_ASSERT(self->_d.is_font_face());

  *out = self->dcast().design_metrics();
  return BL_SUCCESS;
}

BLResult bl_font_face_get_coverage_info(const BLFontFaceCore* self, BLFontCoverageInfo* out) noexcept {
  BL_ASSERT(self->_d.is_font_face());

  *out = self->dcast().coverage_info();
  return BL_SUCCESS;
}

BLResult bl_font_face_get_panose_info(const BLFontFaceCore* self, BLFontPanoseInfo* out) noexcept {
  BL_ASSERT(self->_d.is_font_face());

  *out = self->dcast().panose_info();
  return BL_SUCCESS;
}

BLResult bl_font_face_get_character_coverage(const BLFontFaceCore* self, BLBitSetCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());

  // Don't calculate the `character_coverage` again if it was already calculated. We don't need atomics here as it
  // is set only once, atomics will be used only if it hasn't been calculated yet or if there is a race (already
  // calculated by another thread, but nullptr at this exact moment here).
  BLFontFacePrivateImpl* self_impl = get_impl(self);
  if (!bl_object_atomic_content_test(&self_impl->character_coverage)) {
    if (self_impl->face_info.face_type != BL_FONT_FACE_TYPE_OPENTYPE)
      return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);

    BLBitSet tmp_bit_set;
    BL_PROPAGATE(bl::OpenType::CMapImpl::populate_character_coverage(static_cast<bl::OpenType::OTFaceImpl*>(self_impl), &tmp_bit_set.dcast()));

    tmp_bit_set.shrink();
    if (!bl_object_atomic_content_move(&self_impl->character_coverage, &tmp_bit_set))
      return bl_bit_set_assign_move(out, &tmp_bit_set);
  }

  return bl_bit_set_assign_weak(out, &self_impl->character_coverage);
}

bool bl_font_face_has_script_tag(const BLFontFaceCore* self, BLTag script_tag) noexcept {
  using namespace bl::FontFaceInternal;
  BL_ASSERT(self->_d.is_font_face());

  const BLFontFacePrivateImpl* self_impl = get_impl(self);
  return self_impl->script_tag_set.has_tag(script_tag);
}

bool bl_font_face_has_feature_tag(const BLFontFaceCore* self, BLTag feature_tag) noexcept {
  using namespace bl::FontFaceInternal;
  BL_ASSERT(self->_d.is_font_face());

  const BLFontFacePrivateImpl* self_impl = get_impl(self);
  return self_impl->feature_tag_set.has_tag(feature_tag);
}

bool bl_font_face_has_variation_tag(const BLFontFaceCore* self, BLTag variation_tag) noexcept {
  using namespace bl::FontFaceInternal;
  BL_ASSERT(self->_d.is_font_face());

  const BLFontFacePrivateImpl* self_impl = get_impl(self);
  return self_impl->variation_tag_set.has_tag(variation_tag);
}

BLResult bl_font_face_get_script_tags(const BLFontFaceCore* self, BLArrayCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(out->_d.is_array());

  const BLFontFacePrivateImpl* self_impl = get_impl(self);
  return self_impl->script_tag_set.flatten_to(out->dcast<BLArray<BLTag>>());
}

BLResult bl_font_face_get_feature_tags(const BLFontFaceCore* self, BLArrayCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(out->_d.is_array());

  const BLFontFacePrivateImpl* self_impl = get_impl(self);
  return self_impl->feature_tag_set.flatten_to(out->dcast<BLArray<BLTag>>());
}

BLResult bl_font_face_get_variation_tags(const BLFontFaceCore* self, BLArrayCore* out) noexcept {
  using namespace bl::FontFaceInternal;

  BL_ASSERT(self->_d.is_font_face());
  BL_ASSERT(out->_d.is_array());

  const BLFontFacePrivateImpl* self_impl = get_impl(self);
  return self_impl->variation_tag_set.flatten_to(out->dcast<BLArray<BLTag>>());
}

// bl::FontFace - Runtime Registration
// ===================================

void bl_font_face_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  // Initialize BLFontFace built-ins.
  bl_null_font_face_funcs.map_text_to_glyphs = bl_null_font_face_map_text_to_glyphs;
  bl_null_font_face_funcs.get_glyph_bounds = bl_null_font_face_get_glyph_bounds;
  bl_null_font_face_funcs.get_glyph_advances = bl_null_font_face_get_glyph_advances;
  bl_null_font_face_funcs.get_glyph_outlines = bl_null_font_face_get_glyph_outlines;
  bl_null_font_face_funcs.apply_kern = bl_null_font_face_apply_kern;
  bl_null_font_face_funcs.apply_gsub = bl_null_font_face_apply_gsub;
  bl_null_font_face_funcs.apply_gpos = bl_null_font_face_apply_gpos;
  bl_null_font_face_funcs.position_glyphs = bl_null_font_face_position_glyphs;

  bl_font_face_default_impl.virt.base.destroy = bl_null_font_face_impl_destroy;
  bl_font_face_default_impl.virt.base.get_property = bl_object_impl_get_property;
  bl_font_face_default_impl.virt.base.set_property = bl_object_impl_set_property;
  bl_font_face_impl_ctor(&bl_font_face_default_impl.impl, &bl_font_face_default_impl.virt, bl_null_font_face_funcs);

  bl_object_defaults[BL_OBJECT_TYPE_FONT_FACE]._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_FACE), &bl_font_face_default_impl.impl);
}
