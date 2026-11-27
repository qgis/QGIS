// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PATTERN_P_H_INCLUDED
#define BLEND2D_PATTERN_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/pattern.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name BLPattern - Private Structs
//! \{

//! Private implementation that extends \ref BLPatternImpl.
struct BLPatternPrivateImpl : public BLPatternImpl {
  // Nothing at the moment.
};

//! \}

//! \name BLPattern - Private API
//! \{

namespace bl {
namespace PatternInternal {

//! \name BLPattern - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLPatternImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

BL_HIDDEN BLResult free_impl(BLPatternPrivateImpl* impl) noexcept;

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLPatternPrivateImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLPattern - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE BLPatternPrivateImpl* get_impl(const BLPatternCore* self) noexcept {
  return static_cast<BLPatternPrivateImpl*>(self->_d.impl);
}

static BL_INLINE bool is_instance_mutable(const BLPatternCore* self) noexcept {
  return is_impl_mutable(get_impl(self));
}

static BL_INLINE BLResult retain_instance(const BLPatternCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLPatternCore* self) noexcept {
  return release_impl<RCMode::kMaybe>(get_impl(self));
}

static BL_INLINE BLResult replace_instance(BLPatternCore* self, const BLPatternCore* other) noexcept {
  BLPatternPrivateImpl* impl = get_impl(self);
  self->_d = other->_d;
  return release_impl<RCMode::kMaybe>(impl);
}

//! \}

//! \name BLPattern - Internals - Accessors
//! \{

static BL_INLINE BLExtendMode get_extend_mode(const BLPatternCore* self) noexcept { return (BLExtendMode)self->_d.info.b_field(); }
static BL_INLINE BLTransformType get_transform_type(const BLPatternCore* self) noexcept { return (BLTransformType)self->_d.info.c_field(); }

static BL_INLINE void set_extend_mode(BLPatternCore* self, BLExtendMode extend_mode) noexcept { self->_d.info.set_b_field(uint32_t(extend_mode)); }
static BL_INLINE void set_transform_type(BLPatternCore* self, BLTransformType transform_type) noexcept { self->_d.info.set_c_field(uint32_t(transform_type)); }

static BL_INLINE bool is_area_valid(const BLRectI& area, const BLSizeI& size) noexcept {
  typedef unsigned U;
  return bool((U(area.x) < U(size.w)) &
              (U(area.y) < U(size.h)) &
              (U(area.w) <= U(size.w) - U(area.x)) &
              (U(area.h) <= U(size.h) - U(area.y)));
}

static BL_INLINE bool is_area_valid_and_non_zero(const BLRectI& area, const BLSizeI& size) noexcept {
  typedef unsigned U;
  return bool((U(area.x) < U(size.w)) &
              (U(area.y) < U(size.h)) &
              (U(area.w) - U(1) < U(size.w) - U(area.x)) &
              (U(area.h) - U(1) < U(size.h) - U(area.y)));
}

//! \}

} // {PatternInternal}
} // {bl}

//! \}

//! \}
//! \endcond

#endif // BLEND2D_PATTERN_P_H_INCLUDED
