// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OBJECT_P_H_INCLUDED
#define BLEND2D_OBJECT_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/api-impl.h>
#include <blend2d/core/object.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/wrap_p.h>
#include <blend2d/threading/atomic_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name BLObject - Internals - Constants
//! \{

//! Default object impl alignment that the Impl allocator honors.
static constexpr size_t BL_OBJECT_IMPL_ALIGNMENT = 16;

//! Maximum impl size: MaximumTheoreticalAddressableMemory / 2 - 4096.
//!
//! \note The reason we divide the theoretical addressable space by 2 is to never allocate anything that would
//! have a sign bit set. In addition, the sign bit then can be used as a flag in \ref BLObjectImplHeader.
static constexpr size_t BL_OBJECT_IMPL_MAX_SIZE = (SIZE_MAX / 2u) - 4096u;

//! \}

//! \name BLObject - Internals - Strong Types
//! \{

//! Strongly typed object impl size to not confuse it with regular size / capacity of containers.
BL_DEFINE_STRONG_TYPE(BLObjectImplSize, size_t)

//! \}

//! \name BLObject - Internals - Structs
//! \{

//! BLObjectImpl header, which precedes BLObjectImpl.
struct BLObjectImplHeader {
  //! \name Members
  //! \{

  //! Reference count.
  size_t ref_count;

  //!   - [0]     - 'R' RefCount flag (if set, the impl data is reference countable, and refcount is not 0).
  //!   - [1]     - 'I' Immutable flag (if set, the impl data is immutable, and the ref_count base is 2).
  //!   - [5:2]   - alignment offset multiplied by 4 to subtract from the impl to get the original allocated pointer.
  //!   - [MSB]   - 'X' External flag (if the impl holds external data and BLDestroyExternalDataFunc pointer + user_data).
  //!
  //! \note Immutable flag can only be set when also RefCount flag is set. By design all Impls are immutable when
  //! `ref_count != 1`, so Immutable flag is only useful when the Impl is RefCounted. When not ref_counted, the Impl
  //! is immutable by design.
  size_t flags;

  //! \}

  //! \name
  //! \{

  enum : uint32_t {
    kRefCountedFlagShift = 0,
    kImmutableFlagShift = 1,
    kExternalFlagShift = bl::IntOps::bit_size_of<size_t>() - 1u,
    kAlignmentMaskShift = 2
  };

  enum : size_t {
    kRefCountedFlag = size_t(0x01u) << kRefCountedFlagShift,
    kImmutableFlag = size_t(0x01u) << kImmutableFlagShift,
    kRefCountedAndImmutableFlags = kRefCountedFlag | kImmutableFlag,

    kExternalFlag = size_t(0x01u) << kExternalFlagShift,
    kAlignmentOffsetMask = size_t(0x1Fu) << kAlignmentMaskShift
  };

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the number of bytes used for alignment of the impl (0, 4, 8, 12, 16, ..., 56).
  BL_INLINE_NODEBUG size_t alignment_offset() const noexcept { return flags & kAlignmentOffsetMask; }

  //! Tests whether this object impl is reference counted.
  BL_INLINE_NODEBUG bool is_ref_counted() const noexcept { return ref_count != 0u; }
  //! Tests whether this object impl is immutable.
  BL_INLINE_NODEBUG bool is_immutable() const noexcept { return (flags & kRefCountedAndImmutableFlags) != kRefCountedFlag; }
  //! Tests whether this object impl holds external data.
  BL_INLINE_NODEBUG bool is_external() const noexcept { return (flags & kExternalFlag) != 0u; }

  //! Returns the base reference count value (if the reference count goes below the object must be freed).
  //!
  //! The returned value describes a reference count of Impl that would signalize that it's not shared with any other
  //! object. The base value is always 1 for mutable Impls and 3 for immutable Impls. This function just uses some
  //! trick to make the extraction of this value as short as possible in the resulting machine code.
  //!
  //! \note Why it works this way? Typically the runtime only check the reference-count to check whether an Impl can
  //! be modified. If the reference count is not 1 the Impl cannot be modified. This makes it simple to check whether
  //! an Impl is mutable.
  BL_INLINE_NODEBUG size_t base_ref_count_value() const noexcept { return flags & kRefCountedAndImmutableFlags; }

  //! \}
};

//! Provides information necessary to release external data that Impl references.
//!
//! \note The `destroy_func` is always non-null - if the user passes `nullptr` as a `destroy_func` to Blend2D API it would
//! be replaced with a built-in "dummy" function that does nothing to make sure we only have a single code-path
struct BLObjectExternalInfo {
  //! Destroy callback to be called when Impl holding the external data is being destroyed.
  BLDestroyExternalDataFunc destroy_func;
  //! Data provided by the user to identify the external data, passed to destroy_func() as `user_data`.
  void* user_data;
};

//! BLObjectImpl having a virtual function table.
struct BLObjectVirtImpl : public BLObjectImpl {
  const BLObjectVirt* virt;
};

struct alignas(16) BLObjectEternalHeader {
#if BL_TARGET_ARCH_BITS == 32
  uint64_t padding;
#endif
  BLObjectImplHeader header;
};

//! Only used for storing built-in default Impls.
template<typename Impl>
struct alignas(16) BLObjectEternalImpl {
  BLObjectEternalHeader header;
  bl::Wrap<Impl> impl;
};

//! Only used for storing built-in default Impls with virtual function table.
template<typename Impl, typename Virt>
struct alignas(16) BLObjectEternalVirtualImpl {
  BLObjectEternalHeader header;
  bl::Wrap<Impl> impl;
  Virt virt;
};

//! \}

//! \name BLObject - Internals - Globals
//! \{

//! Object header used by \ref bl::ObjectInternal::is_instance_mutable() and similar functions to avoid branching in SSO case.
BL_HIDDEN extern const BLObjectImplHeader bl_object_header_with_ref_count_eq_0;

//! Object header used by \ref bl::ObjectInternal::is_instance_mutable() and similar functions to avoid branching in SSO case.
BL_HIDDEN extern const BLObjectImplHeader bl_object_header_with_ref_count_eq_1;

//! A table that contains default constructed objects of each object type.
BL_HIDDEN extern BLObjectCore bl_object_defaults[BL_OBJECT_TYPE_MAX_VALUE + 1];

BL_HIDDEN void BL_CDECL bl_object_destroy_external_data_dummy(void* impl, void* external_data, void* user_data) noexcept;

//! \}

//! \name BLObject - Internals - Property Handling
//! \{

BL_HIDDEN BLResult BL_CDECL bl_object_impl_get_property(const BLObjectImpl* impl, const char* name, size_t name_size, BLVarCore* value_out) BL_NOEXCEPT_C;
BL_HIDDEN BLResult BL_CDECL bl_object_impl_set_property(BLObjectImpl* impl, const char* name, size_t name_size, const BLVarCore* value) BL_NOEXCEPT_C;

static BL_INLINE bool bl_match_property(const char* key, size_t key_size, const char* str) noexcept {
  size_t str_size = strlen(str);
  return key_size == str_size && memcmp(key, str, key_size) == 0;
}

//! \}

//! \name BLObject - Internals - Cast From Unknown
//! \{

//! Casts the given unknown pointer to `BLObjectCore*`.
static BL_INLINE_NODEBUG BLObjectCore* bl_as_object(BLUnknown* unknown) { return static_cast<BLObjectCore*>(unknown); }
//! Casts the given unknown pointer to `BLObjectCore*` (const).
static BL_INLINE_NODEBUG const BLObjectCore* bl_as_object(const BLUnknown* unknown) { return static_cast<const BLObjectCore*>(unknown); }

//! \}

BL_HIDDEN BLResult bl_object_destroy_unknown_impl(BLObjectImpl* impl, BLObjectInfo info) noexcept;

namespace bl {

//! Reference counting mode.
enum class RCMode : uint32_t {
  //! It's not known whether the Impl is reference counted (useful for "always-dynamic" objects that don't check object info).
  kMaybe,
  //! It's guaranteed that the Impl is reference counted (for example BLObjectInfo::is_ref_counted_object() returned true).
  kForce
};

} // {bl}

namespace bl {
namespace ObjectInternal {

//! \name BLObject - Internals - Impl - Header
//! \{

//! Returns a pointer to the header of `impl`.
static BL_INLINE_NODEBUG BLObjectImplHeader* get_impl_header(BLObjectImpl* impl) noexcept {
  return PtrOps::deoffset<BLObjectImplHeader>(impl, sizeof(BLObjectImplHeader));
}

//! Returns a pointer to the header of `impl` (const).
static BL_INLINE_NODEBUG const BLObjectImplHeader* get_impl_header(const BLObjectImpl* impl) noexcept {
  return PtrOps::deoffset<const BLObjectImplHeader>(impl, sizeof(BLObjectImplHeader));
}

//! \}

//! \name BLObject - Internals - Impl - Alloc / Free
//! \{

static BL_INLINE void* get_allocated_ptr(BLObjectImpl* impl) noexcept {
  const BLObjectImplHeader* header = get_impl_header(impl);
  size_t offset = sizeof(BLObjectImplHeader);

  if (header->is_external())
    offset = sizeof(BLObjectExternalInfo) + sizeof(BLObjectImplHeader);

  return PtrOps::deoffset(impl, offset + header->alignment_offset());
}

template<typename T>
static BL_INLINE BLResult alloc_impl_t(BLObjectCore* self, BLObjectInfo info, BLObjectImplSize impl_size = BLObjectImplSize{sizeof(T)}) noexcept {
  return bl_object_alloc_impl(self, info.bits, impl_size.value());
}

template<typename T>
static BL_INLINE BLResult alloc_impl_aligned_t(BLObjectCore* self, BLObjectInfo info, BLObjectImplSize impl_size, size_t impl_alignment) noexcept {
  return bl_object_alloc_impl_aligned(self, info.bits, impl_size.value(), impl_alignment);
}

template<typename T>
static BL_INLINE BLResult alloc_impl_external_t(BLObjectCore* self, BLObjectInfo info, bool immutable, BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {
  return bl_object_alloc_impl_external(self, info.bits, sizeof(T), immutable, destroy_func, user_data);
}

template<typename T>
static BL_INLINE BLResult alloc_impl_external_t(BLObjectCore* self, BLObjectInfo info, BLObjectImplSize impl_size, bool immutable, BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {
  return bl_object_alloc_impl_external(self, info.bits, impl_size.value(), immutable, destroy_func, user_data);
}

static BL_INLINE BLResult free_impl(BLObjectImpl* impl) noexcept {
  void* ptr = get_allocated_ptr(impl);
  free(ptr);
  return BL_SUCCESS;
}

static BL_INLINE BLResult free_virtual_impl(BLObjectImpl* impl) noexcept {
  return static_cast<BLObjectVirtImpl*>(impl)->virt->base.destroy(static_cast<BLObjectImpl*>(impl));
}

//! \}

//! \name BLObject - Internals - Impl - External
//! \{

//! Tests whether the Impl uses external data.
static BL_INLINE bool is_impl_external(const BLObjectImpl* impl) noexcept {
  const BLObjectImplHeader* header = get_impl_header(impl);
  return header->is_external();
}

//! Returns a pointer to the header of `impl`.
static BL_INLINE_NODEBUG BLObjectExternalInfo* get_external_info(BLObjectImpl* impl) noexcept {
  return PtrOps::deoffset<BLObjectExternalInfo>(impl, sizeof(BLObjectExternalInfo) + sizeof(BLObjectImplHeader));
}

//! Returns a pointer to the header of `impl` (const).
static BL_INLINE_NODEBUG const BLObjectExternalInfo* get_external_info(const BLObjectImpl* impl) noexcept {
  return PtrOps::deoffset<const BLObjectExternalInfo>(impl, sizeof(BLObjectExternalInfo) + sizeof(BLObjectImplHeader));
}

static BL_INLINE void init_external_destroy_func(BLObjectImpl* impl, BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {
  BLObjectExternalInfo* external_info = get_external_info(impl);
  external_info->destroy_func = destroy_func ? destroy_func : bl_object_destroy_external_data_dummy;
  external_info->user_data = user_data;

}

static BL_INLINE void call_external_destroy_func(BLObjectImpl* impl, void* external_data) noexcept {
  BLObjectExternalInfo* external_info = get_external_info(impl);
  external_info->destroy_func(impl, external_data, external_info->user_data);
}

//! \}

//! \name BLObject - Internals - Impl - Reference Counting
//! \{

//! Tests whether the `impl` is mutable.
static BL_INLINE bool is_impl_mutable(const BLObjectImpl* impl) noexcept {
  const BLObjectImplHeader* header = get_impl_header(impl);
  return header->ref_count == 1;
}

//! Tests whether the `impl` is reference counted.
static BL_INLINE bool is_impl_ref_counted(const BLObjectImpl* impl) noexcept {
  const BLObjectImplHeader* header = get_impl_header(impl);
  return header->is_ref_counted();
}

//! Tests whether the `impl` reference count is the same as its initial value.
//!
//! This check essentially checks whether these is only a single remaining reference to the `impl`.
static BL_INLINE bool is_impl_ref_count_equal_to_base(const BLObjectImpl* impl) noexcept {
  const BLObjectImplHeader* header = get_impl_header(impl);
  return header->ref_count == header->base_ref_count_value();
}

//! Initializes the reference count of `impl` to its base value, considering the passed `immediate` flag.
//!
//! The base value is either 1 if the impl is mutable, or 3, if the impl is immutable.
static BL_INLINE void init_ref_count_to_base(BLObjectImpl* impl, bool immutable) noexcept {
  size_t ri_flags = BLObjectImplHeader::kRefCountedFlag | (size_t(immutable) << BLObjectImplHeader::kImmutableFlagShift);

  BLObjectImplHeader* header = get_impl_header(impl);
  header->flags = (header->flags & ~BLObjectImplHeader::kImmutableFlag) | ri_flags;
}

//! Returns a reference count of `impl`.
static BL_INLINE size_t get_impl_ref_count(const BLObjectImpl* impl) noexcept {
  return get_impl_header(impl)->ref_count;
}

template<RCMode kRCMode>
static BL_INLINE void retain_impl(BLObjectImpl* impl, size_t n = 1u) noexcept {
  if (kRCMode == RCMode::kMaybe && !is_impl_ref_counted(impl))
    return;
  bl_atomic_fetch_add_relaxed(&get_impl_header(impl)->ref_count, n);
}

template<RCMode kRCMode>
static BL_INLINE bool deref_impl_and_test(BLObjectImpl* impl) noexcept {
  BLObjectImplHeader* header = get_impl_header(impl);
  size_t base_ref_count = header->base_ref_count_value();

  if (kRCMode == RCMode::kMaybe && !base_ref_count)
    return false;

  return bl_atomic_fetch_sub_strong(&header->ref_count) == base_ref_count;
}

template<RCMode kRCMode>
static BL_INLINE BLResult release_virtual_impl(BLObjectImpl* impl) noexcept {
  return deref_impl_and_test<kRCMode>(impl) ? free_virtual_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}
//! \name BLObject - Internals - Object Utilities
//! \{

//! Tests whether an untyped object is mutable.
//!
//! \note This function supports both SSO and dynamic objects. SSO object always returns true. If you want to check
//! whether the object is dynamic and that dynamic object has a mutable impl, use `is_instance_impl_mutable()` instead.
static BL_INLINE bool is_instance_mutable(const BLObjectCore* self) noexcept {
  const BLObjectImplHeader* header = self->_d.sso() ? &bl_object_header_with_ref_count_eq_1 : get_impl_header(self->_d.impl);
  return header->ref_count == 1u;
}

//! Tests whether an untyped object is dynamic and has a mutable Impl.
static BL_INLINE bool is_instance_dynamic_and_mutable(const BLObjectCore* self) noexcept {
  const BLObjectImplHeader* header = self->_d.sso() ? &bl_object_header_with_ref_count_eq_0 : get_impl_header(self->_d.impl);
  return header->ref_count == 1u;
}

//! Tests whether an object that always has a dynamic Impl is mutable.
static BL_INLINE bool is_dynamic_instance_mutable(const BLObjectCore* self) noexcept {
  BL_ASSERT(self->_d.is_dynamic_object());

  const BLObjectImplHeader* header = get_impl_header(self->_d.impl);
  return header->ref_count == 1u;
}

template<typename T>
static BL_INLINE BLResult retain_instance(const T* self, size_t n = 1) noexcept {
  if (self->_d.is_ref_counted_object())
    retain_impl<RCMode::kForce>(self->_d.impl, n);
  return BL_SUCCESS;
}

template<typename T>
static BL_INLINE BLResult release_unknown_instance(T* self) noexcept {
  BLObjectInfo info = self->_d.info;
  if (info.is_dynamic_object() && deref_impl_and_test<RCMode::kMaybe>(self->_d.impl))
    return bl_object_destroy_unknown_impl(self->_d.impl, info);
  return BL_SUCCESS;
}

template<typename T>
static BL_INLINE BLResult release_virtual_instance(T* self) noexcept {
  BL_ASSERT(self->_d.is_virtual_object());
  return release_virtual_impl<RCMode::kMaybe>(self->_d.impl);
}

template<typename T>
static BL_INLINE BLResult replace_virtual_instance(T* self, const T* other) noexcept {
  BL_ASSERT(self->_d.is_virtual_object());
  BL_ASSERT(other->_d.is_virtual_object());

  BLObjectImpl* impl = self->_d.impl;
  self->_d = other->_d;
  return release_virtual_impl<RCMode::kMaybe>(impl);
}

template<typename T>
static BL_INLINE BLResult assign_virtual_instance(T* dst, const T* src) noexcept {
  retain_instance(src);
  release_virtual_instance(dst);

  dst->_d = src->_d;
  return BL_SUCCESS;
}

//! \}

} // {ObjectInternal}
} // {bl}

//! \name BLObject - Internals - Reference Counting and Object Lifetime
//! \{

template<typename T>
static BL_INLINE BLResult bl_object_private_init_move_tagged(T* dst, T* src) noexcept {
  dst->_d = src->_d;
  src->_d = bl_object_defaults[src->_d.raw_type()]._d;
  return BL_SUCCESS;
}

template<typename T>
static BL_INLINE BLResult bl_object_private_init_move_unknown(T* dst, T* src) noexcept {
  dst->_d = src->_d;
  src->_d = bl_object_defaults[dst->_d.get_type()]._d;
  return BL_SUCCESS;
}

template<typename T>
static BL_INLINE BLResult bl_object_private_init_weak_tagged(T* dst, const T* src) noexcept {
  dst->_d = src->_d;
  return bl::ObjectInternal::retain_instance(dst);
}

template<typename T>
static BL_INLINE BLResult bl_object_private_init_weak_unknown(T* dst, const T* src) noexcept {
  dst->_d = src->_d;
  return bl::ObjectInternal::retain_instance(dst);
}

template<typename T>
static BL_INLINE BLResult bl_object_private_assign_weak_unknown(T* dst, const T* src) noexcept {
  bl::ObjectInternal::retain_instance(src);
  bl::ObjectInternal::release_unknown_instance(dst);

  dst->_d = src->_d;
  return BL_SUCCESS;
}

//! \}

//! \name BLObject - Internals - Expanding Utilities (Containers)
//! \{

static BL_INLINE size_t bl_object_grow_impl_size_to_power_of_2(size_t x) noexcept {
  return size_t(1u) << (bl::IntOps::bit_size_of<size_t>() - bl::IntOps::clz(x + 1u));
}

static BL_INLINE BLObjectImplSize bl_object_align_impl_size(BLObjectImplSize impl_size) noexcept {
  return BLObjectImplSize(bl::IntOps::align_up(impl_size.value(), 64u));
}

static BL_INLINE BLObjectImplSize bl_object_expand_impl_size(BLObjectImplSize impl_size) noexcept {
  size_t n = impl_size.value();

  if (n >= BL_ALLOC_GROW_LIMIT)
    n = n + (n >> 2) + (n >> 3); // Makes the capacity 37.5% greater.
  else
    n = bl_object_grow_impl_size_to_power_of_2(n); // Doubles the capacity.

  // If an overflow happened during any of the computation above `bl_max()` would cancel it and make it fitting.
  return BLObjectImplSize(bl_max(n, impl_size.value()));
}

static BLObjectImplSize bl_object_expand_impl_size_with_modify_op(BLObjectImplSize impl_size, BLModifyOp modify_op) noexcept {
  if (bl_modify_op_does_grow(modify_op))
    return bl_object_expand_impl_size(impl_size);
  else
    return impl_size;
}

//! \}

//! \name BLObject - Internals - Atomic Content Utilities
//! \{

//! Initializes an object to a representation suitable for using `bl_object_atomic_assign_move()` on it.
static BL_INLINE void bl_object_atomic_content_init(BLObjectCore* self) noexcept {
  self->_d.u64_data[0] = 0;
  self->_d.u64_data[1] = 0;
}

//! Tests whether the object contains a valid instance.
//!
//! Freshly initialized object by `bl_object_atomic_content_init()` returns false. When a moving into the object is still
//! in progress `false` is returned as well. When the first called `bl_object_atomic_assign_move()` finishes, `true` is
//! returned.
static BL_INLINE bool bl_object_atomic_content_test(const BLObjectCore* self) noexcept {
  return bl_atomic_fetch_strong(&self->_d.info.bits) > 1u;
}

//! Moves `other` to `self` atomically.
//!
//! The `self` object must have been initialized by `bl_object_atomic_content_init()` or assigned by
//! `bl_object_atomic_assign_move()` - the later case would be detected by the implementation.
//!
//! Returns `true` when the object was successfully moved, `false` otherwise.
//!
//! \note If `false` was returned it doesn't mean that `self` has been successfully initialized by other thread. It
//! means that the implementation failed to move `other` to `self`, because some other thread started moving into
//! that object first, however, it could be still moving the object when `bl_object_atomic_assign_move()` returns.
static BL_NOINLINE bool bl_object_atomic_content_move(BLObjectCore* self, BLObjectCore* other) noexcept {
  // TODO: This should use CMPXCHG16B on X86_64 when available.
  BL_ASSERT(self != other);

  // Maximum number of spins to wait for another thread in case of high contention.
  constexpr size_t kMaxSpins = 100;

  BLObjectDetail other_d = other->_d;
  uint32_t self_info = 0;

  if (bl_atomic_compare_exchange(&self->_d.info.bits, &self_info, 1u)) {
    // We have successfully acquired the info so we can perform the move.
    self->_d = other_d;
    bl_atomic_thread_fence();

    other->_d = bl_object_defaults[other_d.raw_type()]._d;
    return true;
  }
  else {
    // Other thread is either moving at the moment or did already move.
    size_t spin_count = kMaxSpins;

    // Wait for a bit for another thread to finish the atomic assignment.
    for (;;) {
      if (self_info > 1u) {
        // `self` is now a valid object, however, `other` was not moved, so reset it.
        bl_object_reset(other);
        return true;
      }

      if (--spin_count == 0)
        return false;

      self_info = bl_atomic_fetch_strong(&self->_d.info.bits);
    }
  }
}

//! \}

//! \}
//! \endcond

#endif // BLEND2D_OBJECT_P_H_INCLUDED
