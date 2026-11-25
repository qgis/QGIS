// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_ARRAY_P_H_INCLUDED
#define BLEND2D_ARRAY_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array.h>
#include <blend2d/core/object_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl::ArrayInternal {

//! \name BLArray - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(const BLArrayImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

BL_HIDDEN BLResult free_impl(BLArrayImpl* impl) noexcept;

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLArrayImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLArray - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE BLArrayImpl* get_impl(const BLArrayCore* self) noexcept {
  return static_cast<BLArrayImpl*>(self->_d.impl);
}

static BL_INLINE bool is_instance_mutable(const BLArrayCore* self) noexcept {
  return ObjectInternal::is_instance_mutable(self);
}

static BL_INLINE bool is_instance_dynamic_and_mutable(const BLArrayCore* self) noexcept {
  return ObjectInternal::is_instance_dynamic_and_mutable(self);
}

static BL_INLINE bool is_dynamic_instance_mutable(const BLArrayCore* self) noexcept {
  return ObjectInternal::is_dynamic_instance_mutable(self);
}

static BL_INLINE BLResult retain_instance(const BLArrayCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLArrayCore* self) noexcept {
  return self->_d.is_ref_counted_object() ? release_impl<RCMode::kForce>(get_impl(self)) : BLResult(BL_SUCCESS);
}

static BL_INLINE BLResult replace_instance(BLArrayCore* self, const BLArrayCore* other) noexcept {
  // NOTE: UBSAN doesn't like casting the impl in case the Array is in SSO mode, so wait with the cast.
  void* impl = static_cast<void*>(self->_d.impl);
  BLObjectInfo info = self->_d.info;

  self->_d = other->_d;
  return info.is_ref_counted_object() ? release_impl<RCMode::kForce>(static_cast<BLArrayImpl*>(impl)) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLArray - Internals - Accessors
//! \{

struct UnpackedData {
  uint8_t* data;
  size_t size;
  size_t capacity;
};

static BL_INLINE UnpackedData unpack(const BLArrayCore* self) noexcept {
  if (self->_d.sso()) {
    return UnpackedData { const_cast<uint8_t*>(self->_d.u8_data), self->_d.a_field(), self->_d.b_field() };
  }
  else {
    BLArrayImpl* impl = get_impl(self);
    return UnpackedData { impl->data_as<uint8_t>(), impl->size, impl->capacity };
  }
}

template<typename T = void>
static BL_INLINE T* get_data(const BLArrayCore* self) noexcept {
  if (self->_d.sso())
    return (T*)(self->_d.char_data);
  else
    return get_impl(self)->data_as<T>();
}

static BL_INLINE size_t get_size(const BLArrayCore* self) noexcept {
  if (self->_d.sso())
    return size_t(self->_d.a_field());
  else
    return get_impl(self)->size;
}

static BL_INLINE size_t get_capacity(const BLArrayCore* self) noexcept {
  if (self->_d.sso())
    return size_t(self->_d.b_field());
  else
    return get_impl(self)->capacity;
}

static BL_INLINE void set_size(BLArrayCore* self, size_t new_size) noexcept {
  BL_ASSERT(new_size <= get_capacity(self));
  if (self->_d.sso())
    self->_d.info.set_a_field(uint32_t(new_size));
  else
    get_impl(self)->size = new_size;
}

//! \}

} // {bl::ArrayInternal}

//! \}
//! \endcond

#endif // BLEND2D_ARRAY_P_H_INCLUDED
