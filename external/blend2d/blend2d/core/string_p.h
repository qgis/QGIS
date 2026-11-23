// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_STRING_P_H_INCLUDED
#define BLEND2D_STRING_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/string.h>
#include <blend2d/unicode/unicode_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace StringInternal {

//! \name BLString - Internals - Common Functionality (Container)
//! \{

static BL_INLINE constexpr BLObjectImplSize impl_size_from_capacity(size_t capacity) noexcept {
  return BLObjectImplSize(sizeof(BLStringImpl) + 1 + capacity);
}

static BL_INLINE constexpr size_t capacity_from_impl_size(BLObjectImplSize impl_size) noexcept {
  return impl_size.value() - sizeof(BLStringImpl) - 1;
}

//! \}

//! \name BLString - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLStringImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}


static BL_INLINE BLResult free_impl(BLStringImpl* impl) noexcept {
  return ObjectInternal::free_impl(impl);
}

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLStringImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLString - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE BLStringImpl* get_impl(const BLStringCore* self) noexcept {
  return static_cast<BLStringImpl*>(self->_d.impl);
}

static BL_INLINE bool is_instance_mutable(const BLStringCore* self) noexcept {
  return ObjectInternal::is_instance_mutable(self);
}

static BL_INLINE BLResult retain_instance(const BLStringCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLStringCore* self) noexcept {
  return self->_d.is_ref_counted_object() ? release_impl<RCMode::kForce>(get_impl(self)) : BLResult(BL_SUCCESS);
}

static BL_INLINE BLResult replace_instance(BLStringCore* self, const BLStringCore* other) noexcept {
  // NOTE: UBSAN doesn't like casting the impl in case the String is in SSO mode, so wait with the cast.
  void* impl = static_cast<void*>(self->_d.impl);
  BLObjectInfo info = self->_d.info;

  self->_d = other->_d;
  return info.is_ref_counted_object() ? release_impl<RCMode::kForce>(static_cast<BLStringImpl*>(impl)) : BLResult(BL_SUCCESS);
}

//! \}

//! \name String - Private - Accessors
//! \{

struct UnpackedData {
  char* data;
  size_t size;
  size_t capacity;
};

static BL_INLINE size_t get_sso_size(const BLStringCore* self) noexcept {
  return (self->_d.info.bits ^ BLString::kSSOEmptySignature) >> BL_OBJECT_INFO_A_SHIFT;
}

static BL_INLINE UnpackedData unpack_data(const BLStringCore* self) noexcept {
  if (self->_d.sso()) {
    return UnpackedData{const_cast<char*>(self->_d.char_data), get_sso_size(self), BLString::kSSOCapacity};
  }
  else {
    BLStringImpl* impl = get_impl(self);
    return UnpackedData{impl->data(), impl->size, impl->capacity};
  }
}

static BL_INLINE char* get_data(const BLStringCore* self) noexcept {
  return self->_d.sso() ? const_cast<char*>(self->_d.char_data) : get_impl(self)->data();
}

static BL_INLINE size_t get_size(const BLStringCore* self) noexcept {
  return self->_d.sso() ? get_sso_size(self) : get_impl(self)->size;
}

static BL_INLINE size_t get_capacity(const BLStringCore* self) noexcept {
  return self->_d.sso() ? size_t(BLString::kSSOCapacity) : get_impl(self)->capacity;
}

//! \}

//! \name String - Private - Static String
//! \{

struct BL_MAY_ALIAS StaticStringImpl {
  size_t size;
  size_t capacity;
};

template<size_t kSize>
struct StaticStringData {
  BLObjectEternalHeader header;
  StaticStringImpl impl;
  char data[kSize + 1];
};

#define BL_DEFINE_STATIC_STRING(name, content)                                                 \
  static const constexpr ::bl::StringInternal::StaticStringData<sizeof(content) - 1u> name = { \
    {},                                                                                        \
    {sizeof(content) - 1u, sizeof(content) - 1u},                                              \
    content                                                                                    \
  };

template<size_t kSize>
static BL_INLINE void init_static(BLStringCore* self, const StaticStringData<kSize>& data) noexcept {
  self->_d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_STRING), (BLObjectImpl*)&data.impl);
}

//! \}

} // {StringInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_STRING_P_H_INCLUDED
