// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/trace_p.h>
#include <blend2d/opentype/otcff_p.h>
#include <blend2d/opentype/otcmap_p.h>
#include <blend2d/opentype/otcore_p.h>
#include <blend2d/opentype/otdefs_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otglyf_p.h>
#include <blend2d/opentype/otkern_p.h>
#include <blend2d/opentype/otlayout_p.h>
#include <blend2d/opentype/otmetrics_p.h>
#include <blend2d/opentype/otname_p.h>

namespace bl::OpenType {

// bl::OpenType - OTFaceImpl - Tracing
// ===================================

#if defined(BL_TRACE_OT_ALL) || defined(BL_TRACE_OT_CORE)
#define Trace BLDebugTrace
#else
#define Trace BLDummyTrace
#endif

// bl::OpenType - OTFaceImpl - Globals
// ===================================

static BLFontFaceVirt bl_ot_face_virt;

// bl::OpenType - OTFaceImpl - Init & Destroy
// ==========================================

static BLResult init_open_type_face(OTFaceImpl* ot_face_impl, const BLFontData* font_data) noexcept {
  OTFaceTables tables;
  tables.init(ot_face_impl, font_data);

  BL_PROPAGATE(CoreImpl::init(ot_face_impl, tables));
  BL_PROPAGATE(NameImpl::init(ot_face_impl, tables));
  BL_PROPAGATE(CMapImpl::init(ot_face_impl, tables));

  // Glyph outlines require either 'CFF2', 'CFF ', or 'glyf/loca' tables. Based on these
  // tables we can initialize `outline_type` and select either CFF or GLYF implementation.
  if (tables.cff || tables.cff2) {
    BL_STATIC_ASSERT(CFFData::kVersion1 == 0);
    BL_STATIC_ASSERT(CFFData::kVersion2 == 1);

    uint32_t cff_version = tables.cff2 ? CFFData::kVersion2 : CFFData::kVersion1;
    BL_PROPAGATE(CFFImpl::init(ot_face_impl, tables, cff_version));
  }
  else if (tables.glyf && tables.loca) {
    BL_PROPAGATE(GlyfImpl::init(ot_face_impl, tables));
  }
  else {
    // The font has no outlines that we can use.
    return bl_make_error(BL_ERROR_FONT_MISSING_IMPORTANT_TABLE);
  }

  BL_PROPAGATE(MetricsImpl::init(ot_face_impl, tables));
  BL_PROPAGATE(LayoutImpl::init(ot_face_impl, tables));

  // Only setup legacy kerning if 'kern' feature is not provided by 'GPOS' table.
  if (!bl_test_flag(ot_face_impl->ot_flags, OTFaceFlags::kGPosKernAvailable)) {
    BL_PROPAGATE(KernImpl::init(ot_face_impl, tables));
  }

  BL_PROPAGATE(ot_face_impl->script_tag_set.finalize());
  BL_PROPAGATE(ot_face_impl->feature_tag_set.finalize());
  BL_PROPAGATE(ot_face_impl->variation_tag_set.finalize());

  return BL_SUCCESS;
}

static BLResult BL_CDECL destroy_open_type_face(BLObjectImpl* impl) noexcept {
  OTFaceImpl* ot_face_impl = static_cast<OTFaceImpl*>(impl);

  bl_call_dtor(ot_face_impl->kern);
  bl_call_dtor(ot_face_impl->layout);
  bl_call_dtor(ot_face_impl->cff_fd_subr_indexes);
  bl_font_face_impl_dtor(ot_face_impl);

  return bl_object_free_impl(ot_face_impl);
}

BLResult create_open_type_face(BLFontFaceCore* self, const BLFontData* font_data, uint32_t face_index) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_FACE);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<OTFaceImpl>(self, info));

  // Zero everything so we don't have to initialize features not provided by the font.
  OTFaceImpl* ot_face_impl = static_cast<OTFaceImpl*>(self->_d.impl);
  memset(static_cast<void*>(ot_face_impl), 0, sizeof(OTFaceImpl));

  bl_font_face_impl_ctor(ot_face_impl, &bl_ot_face_virt, bl_null_font_face_funcs);

  ot_face_impl->face_info.face_type = uint8_t(BL_FONT_FACE_TYPE_OPENTYPE);
  ot_face_impl->face_info.face_index = face_index;
  ot_face_impl->data.dcast() = *font_data;
  ot_face_impl->cmap_format = uint8_t(0xFF);

  bl_call_ctor(ot_face_impl->kern);
  bl_call_ctor(ot_face_impl->layout);
  bl_call_ctor(ot_face_impl->cff_fd_subr_indexes);

  BLResult result = init_open_type_face(ot_face_impl, font_data);

  if (BL_UNLIKELY(result != BL_SUCCESS)) {
    destroy_open_type_face(ot_face_impl);
    return result;
  }

  return BL_SUCCESS;
}

} // {bl::OpenType}

// bl::OpenType - Runtime Registration
// ===================================

void bl_open_type_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  bl::OpenType::bl_ot_face_virt.base.destroy = bl::OpenType::destroy_open_type_face;
  bl::OpenType::bl_ot_face_virt.base.get_property = bl_object_impl_get_property;
  bl::OpenType::bl_ot_face_virt.base.set_property = bl_object_impl_set_property;
}
