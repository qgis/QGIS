// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_BITARRAY_P_H
#define BLEND2D_BITARRAY_P_H

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/bitarray.h>
#include <blend2d/core/object_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name BLBitArray - Types
//! \{

typedef ParametrizedBitOps<BitOrder::kMSB, uint32_t> BitArrayOps;

//! \}

namespace BitArrayInternal {

//! \name BLBitArray - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLBitArrayImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

static BL_INLINE_NODEBUG BLResult free_impl(BLBitArrayImpl* impl) noexcept {
  return ObjectInternal::free_impl(impl);
}

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLBitArrayImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLBitArray - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE_NODEBUG BLBitArrayImpl* get_impl(const BLBitArrayCore* self) noexcept {
  return static_cast<BLBitArrayImpl*>(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLBitArrayCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLBitArrayCore* self) noexcept {
  return self->_d.is_ref_counted_object() ? release_impl<RCMode::kForce>(get_impl(self)) : BLResult(BL_SUCCESS);
}

static BL_INLINE BLResult replace_instance(BLBitArrayCore* self, const BLBitArrayCore* other) noexcept {
  // NOTE: UBSAN doesn't like casting the impl in case the BitArray is in SSO mode, so wait with the cast.
  void* impl = static_cast<void*>(self->_d.impl);
  BLObjectInfo info = self->_d.info;

  self->_d = other->_d;
  return info.is_ref_counted_object() ? release_impl<RCMode::kForce>(static_cast<BLBitArrayImpl*>(impl)) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLBitArray - Internals - Accessors
//! \{

struct BitData {
  uint32_t* data;
  size_t size;
};

static BL_INLINE_NODEBUG size_t get_sso_size(const BLBitArrayCore* self) noexcept { return self->_d.p_field(); }
static BL_INLINE_NODEBUG uint32_t* get_sso_data(BLBitArrayCore* self) noexcept { return self->_d.u32_data; }
static BL_INLINE_NODEBUG const uint32_t* get_sso_data(const BLBitArrayCore* self) noexcept { return self->_d.u32_data; }

static BL_INLINE BitData unpack(const BLBitArrayCore* self) noexcept {
  if (self->_d.sso()) {
    return BitData{const_cast<uint32_t*>(self->_d.u32_data), self->_d.p_field()};
  }
  else {
    BLBitArrayImpl* impl = get_impl(self);
    return BitData{impl->data(), impl->size};
  }
}

static BL_INLINE_NODEBUG uint32_t* get_data(BLBitArrayCore* self) noexcept {
  return self->_d.sso() ? self->_d.u32_data : get_impl(self)->data();
}

static BL_INLINE_NODEBUG const uint32_t* get_data(const BLBitArrayCore* self) noexcept {
  return self->_d.sso() ? self->_d.u32_data : get_impl(self)->data();
}

static BL_INLINE_NODEBUG size_t get_size(const BLBitArrayCore* self) noexcept {
  return self->_d.sso() ? self->_d.p_field() : get_impl(self)->size;
}

static BL_INLINE_NODEBUG size_t get_capacity(const BLBitArrayCore* self) noexcept {
  return self->_d.sso() ? size_t(BLBitArray::kSSOWordCount * 32u) : get_impl(self)->capacity;
}

static BL_INLINE void set_size(BLBitArrayCore* self, size_t new_size) noexcept {
  BL_ASSERT(new_size <= get_capacity(self));
  if (self->_d.sso())
    self->_d.info.set_p_field(uint32_t(new_size));
  else
    get_impl(self)->size = uint32_t(new_size);
}

//! \}

} // {BitArrayInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_BITARRAY_P_H
