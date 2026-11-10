// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTDATA_P_H_INCLUDED
#define BLEND2D_FONTDATA_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/fontdata.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name BLFontData - Impl
//! \{

struct BLFontDataPrivateImpl : public BLFontDataImpl {
  BLArray<BLFontFaceImpl*> face_cache;
};

namespace bl {
namespace FontDataInternal {

static BL_INLINE BLFontDataPrivateImpl* get_impl(const BLFontDataCore* self) noexcept {
  return static_cast<BLFontDataPrivateImpl*>(self->_d.impl);
}

static BL_INLINE void init_impl(BLFontDataPrivateImpl* impl, BLFontDataVirt* virt) noexcept {
  impl->virt = virt;
  impl->face_count = 0;
  impl->face_type = BL_FONT_FACE_TYPE_NONE;
  impl->flags = 0;
  bl_call_ctor(impl->face_cache);
}

} // {FontDataInternal}
} // {bl}

//! \}

//! \}
//! \endcond

#endif // BLEND2D_FONTDATA_P_H_INCLUDED
