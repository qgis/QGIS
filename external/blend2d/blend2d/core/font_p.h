// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONT_P_H_INCLUDED
#define BLEND2D_FONT_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/bitset_p.h>
#include <blend2d/core/font.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/support/scopedbuffer_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

static constexpr uint32_t BL_FONT_GET_GLYPH_OUTLINE_BUFFER_SIZE = 2048;

struct BLFontPrivateImpl : public BLFontImpl {};

static BL_INLINE void bl_font_matrix_multiply(BLMatrix2D* dst, const BLFontMatrix* a, const BLMatrix2D* b) noexcept {
  dst->reset(a->m00 * b->m00 + a->m01 * b->m10,
             a->m00 * b->m01 + a->m01 * b->m11,
             a->m10 * b->m00 + a->m11 * b->m10,
             a->m10 * b->m01 + a->m11 * b->m11,
             b->m20,
             b->m21);
}

static BL_INLINE void bl_font_matrix_multiply(BLMatrix2D* dst, const BLMatrix2D* a, const BLFontMatrix* b) noexcept {
  dst->reset(a->m00 * b->m00 + a->m01 * b->m10,
             a->m00 * b->m01 + a->m01 * b->m11,
             a->m10 * b->m00 + a->m11 * b->m10,
             a->m10 * b->m01 + a->m11 * b->m11,
             a->m20 * b->m00 + a->m21 * b->m10,
             a->m20 * b->m01 + a->m21 * b->m11);
}


static BL_INLINE void bl_font_impl_ctor(BLFontPrivateImpl* impl) noexcept {
  impl->face._d = bl_object_defaults[BL_OBJECT_TYPE_FONT_FACE]._d;
  bl_call_ctor(impl->feature_settings.dcast());
  bl_call_ctor(impl->variation_settings.dcast());
}

namespace bl {
namespace FontInternal {

//! \name BLFont - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(const BLFontPrivateImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

BL_HIDDEN BLResult free_impl(BLFontPrivateImpl* impl) noexcept;

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLFontPrivateImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLFont - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE BLFontPrivateImpl* get_impl(const BLFontCore* self) noexcept {
  return static_cast<BLFontPrivateImpl*>(self->_d.impl);
}

static BL_INLINE bool is_instance_mutable(const BLFontCore* self) noexcept {
  return ObjectInternal::is_impl_mutable(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLFontCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLFontCore* self) noexcept {
  return release_impl<RCMode::kMaybe>(get_impl(self));
}

static BL_INLINE BLResult replace_instance(BLFontCore* self, const BLFontCore* other) noexcept {
  BLFontPrivateImpl* impl = get_impl(self);
  self->_d = other->_d;
  return release_impl<RCMode::kMaybe>(impl);
}

//! \}

} // {FontInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FONT_P_H_INCLUDED
