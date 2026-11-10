// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_IMAGE_P_H_INCLUDED
#define BLEND2D_IMAGE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/image.h>
#include <blend2d/core/object_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name BLImage - Internals - Structs
//! \{

//! Private implementation that extends \ref BLImageImpl.
struct BLImagePrivateImpl : public BLImageImpl {
  //! Count of writers that write to this image.
  //!
  //! Writers don't increase the reference count of the image to keep it mutable. However, we must keep
  //! a counter that would tell the BLImage destructor that it's not the time if `writer_count > 0`.
  size_t writer_count;
};

//! \}

namespace bl {
namespace ImageInternal {

//! \name BLImage - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(const BLImageImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

BL_HIDDEN BLResult free_impl(BLImagePrivateImpl* impl) noexcept;

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLImageImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(static_cast<BLImagePrivateImpl*>(impl)) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLImage - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE_NODEBUG BLImagePrivateImpl* get_impl(const BLImageCore* self) noexcept {
  return static_cast<BLImagePrivateImpl*>(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLImageCore* self, size_t n = 1) noexcept {
  BL_ASSERT(self->_d.is_image());
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLImageCore* self) noexcept {
  BL_ASSERT(self->_d.is_image());
  return release_impl<RCMode::kMaybe>(get_impl(self));
}

static BL_INLINE BLResult replace_instance(BLImageCore* self, const BLImageCore* other) noexcept {
  BLImagePrivateImpl* impl = get_impl(self);
  self->_d = other->_d;
  return release_impl<RCMode::kMaybe>(impl);
}

//! \}

} // {ImageInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_IMAGE_P_H_INCLUDED
