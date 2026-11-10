// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTVARIATIONSETTINGS_P_H_INCLUDED
#define BLEND2D_FONTVARIATIONSETTINGS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/fontvariationsettings.h>
#include <blend2d/core/object_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace FontVariationSettingsInternal {

//! \name BLFontVariationSettings - Internals - Common Functionality (Container)
//! \{

static BL_INLINE_CONSTEXPR BLObjectImplSize impl_size_from_capacity(size_t capacity) noexcept {
  return BLObjectImplSize(sizeof(BLFontVariationSettingsImpl) + capacity * sizeof(BLFontVariationItem));
}

static BL_INLINE_CONSTEXPR size_t capacity_from_impl_size(BLObjectImplSize impl_size) noexcept {
  return (impl_size.value() - sizeof(BLFontVariationSettingsImpl)) / sizeof(BLFontVariationItem);
}

//! \}

//! \name BLFontVariationSettings - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLFontVariationSettingsImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

static BL_INLINE BLResult free_impl(BLFontVariationSettingsImpl* impl) noexcept {
  return ObjectInternal::free_impl(impl);
}

template<RCMode kRCMode>
static BL_INLINE void retain_impl(BLFontVariationSettingsImpl* impl, size_t n = 1) noexcept {
  ObjectInternal::retain_impl<kRCMode>(impl, n);
}

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLFontVariationSettingsImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLFontVariationSettings - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE_NODEBUG BLFontVariationSettingsImpl* get_impl(const BLFontVariationSettingsCore* self) noexcept {
  return static_cast<BLFontVariationSettingsImpl*>(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLFontVariationSettingsCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLFontVariationSettingsCore* self) noexcept {
  return self->_d.info.is_ref_counted_object() ? release_impl<RCMode::kForce>(get_impl(self)) : BLResult(BL_SUCCESS);
}

static BL_INLINE BLResult replace_instance(BLFontVariationSettingsCore* self, const BLFontVariationSettingsCore* other) noexcept {
  BLFontVariationSettingsImpl* impl = get_impl(self);
  BLObjectInfo info = self->_d.info;

  self->_d = other->_d;
  return info.is_ref_counted_object() ? release_impl<RCMode::kForce>(impl) : BLResult(BL_SUCCESS);
}

//! \}

} // {FontVariationSettingsInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FONTVARIATIONSETTINGS_P_H_INCLUDED
