// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/core/var_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl::ArrayInternal {

// bl::Array - Private - Tables
// ============================

struct ItemSizeGen {
  static constexpr uint8_t value(size_t impl_type) noexcept {
    return impl_type == BL_OBJECT_TYPE_ARRAY_OBJECT    ? uint8_t(sizeof(BLObjectCore)) :
           impl_type == BL_OBJECT_TYPE_ARRAY_INT8      ? uint8_t(1) :
           impl_type == BL_OBJECT_TYPE_ARRAY_UINT8     ? uint8_t(1) :
           impl_type == BL_OBJECT_TYPE_ARRAY_INT16     ? uint8_t(2) :
           impl_type == BL_OBJECT_TYPE_ARRAY_UINT16    ? uint8_t(2) :
           impl_type == BL_OBJECT_TYPE_ARRAY_INT32     ? uint8_t(4) :
           impl_type == BL_OBJECT_TYPE_ARRAY_UINT32    ? uint8_t(4) :
           impl_type == BL_OBJECT_TYPE_ARRAY_INT64     ? uint8_t(8) :
           impl_type == BL_OBJECT_TYPE_ARRAY_UINT64    ? uint8_t(8) :
           impl_type == BL_OBJECT_TYPE_ARRAY_FLOAT32   ? uint8_t(4) :
           impl_type == BL_OBJECT_TYPE_ARRAY_FLOAT64   ? uint8_t(8) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_1  ? uint8_t(1) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_2  ? uint8_t(2) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_3  ? uint8_t(3) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_4  ? uint8_t(4) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_6  ? uint8_t(6) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_8  ? uint8_t(8) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_10 ? uint8_t(10) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_12 ? uint8_t(12) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_16 ? uint8_t(16) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_20 ? uint8_t(20) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_24 ? uint8_t(24) :
           impl_type == BL_OBJECT_TYPE_ARRAY_STRUCT_32 ? uint8_t(32) : uint8_t(0);
  }
};

struct SSOCapacityGen {
  static constexpr uint8_t value(size_t object_type) noexcept {
    return ItemSizeGen::value(object_type) == 0
      ? uint8_t(0)
      : uint8_t(BLObjectDetail::kStaticDataSize / ItemSizeGen::value(object_type));
  }
};

struct MaximumCapacityGen {
  static constexpr size_t value(size_t object_type) noexcept {
    return ItemSizeGen::value(object_type) == 0
      ? size_t(0)
      : (BL_OBJECT_IMPL_MAX_SIZE - sizeof(BLArrayImpl)) / ItemSizeGen::value(object_type);
  }
};

static constexpr const auto item_size_table = make_lookup_table<uint8_t, BL_OBJECT_TYPE_MAX_VALUE + 1, ItemSizeGen>();
static constexpr const auto sso_capacity_table = make_lookup_table<uint8_t, BL_OBJECT_TYPE_MAX_VALUE + 1, SSOCapacityGen>();
static constexpr const auto maximum_capacity_table = make_lookup_table<size_t, BL_OBJECT_TYPE_MAX_VALUE + 1, MaximumCapacityGen>();

// bl::Array - Private - Commons
// =============================

// Only used as a filler
template<size_t N>
struct UInt32xN { uint32_t data[N]; };

static BL_INLINE constexpr bool is_array_type_valid(BLObjectType array_type) noexcept {
  return array_type >= BL_OBJECT_TYPE_MIN_ARRAY &&
         array_type <= BL_OBJECT_TYPE_MAX_ARRAY;
}

static BL_INLINE constexpr bool is_array_type_object_based(BLObjectType array_type) noexcept {
  return array_type == BL_OBJECT_TYPE_ARRAY_OBJECT;
}

static BL_INLINE size_t item_size_from_array_type(BLObjectType array_type) noexcept {
  return item_size_table[array_type];
}

static BL_INLINE constexpr size_t capacity_from_impl_size(BLObjectImplSize impl_size, size_t item_size) noexcept {
  return (impl_size.value() - sizeof(BLArrayImpl)) / item_size;
}

static BL_INLINE constexpr BLObjectImplSize impl_size_from_capacity(size_t capacity, size_t item_size) noexcept {
  return BLObjectImplSize(sizeof(BLArrayImpl) + capacity * item_size);
}

static BL_INLINE BLObjectImplSize expand_impl_size(BLObjectImplSize impl_size) noexcept {
  return bl_object_expand_impl_size(impl_size);
}

static BLObjectImplSize expand_impl_size_with_modify_op(BLObjectImplSize impl_size, BLModifyOp modify_op) noexcept {
  return bl_object_expand_impl_size_with_modify_op(impl_size, modify_op);
}

// bl::Array - Private - Low-Level Operations
// ==========================================

static BL_NOINLINE void init_content_objects(void* dst_, const void* src_, size_t n_bytes) noexcept {
  BL_ASSUME((n_bytes % sizeof(BLObjectCore)) == 0);

  BLObjectCore* dst = static_cast<BLObjectCore*>(dst_);
  BLObjectCore* end = PtrOps::offset(dst, n_bytes);
  const BLObjectCore* src = static_cast<const BLObjectCore*>(src_);

  while (dst != end) {
    bl_object_private_init_weak_unknown(dst, src);

    dst++;
    src++;
  }
}

static BL_INLINE void init_content_by_type(void* dst, const void* src, size_t n_bytes, BLObjectType array_type) noexcept {
  if (is_array_type_object_based(array_type))
    init_content_objects(dst, src, n_bytes);
  else
    memcpy(dst, src, n_bytes);
}

static BL_NOINLINE void assign_content_objects(void* dst_, const void* src_, size_t n_bytes) noexcept {
  BL_ASSUME((n_bytes % sizeof(BLObjectCore)) == 0);

  BLObjectCore* dst = static_cast<BLObjectCore*>(dst_);
  BLObjectCore* end = PtrOps::offset(dst, n_bytes);
  const BLObjectCore* src = static_cast<const BLObjectCore*>(src_);

  while (dst != end) {
    bl_object_private_assign_weak_unknown(dst, src);
    dst++;
    src++;
  }
}

static BL_INLINE void assign_content_by_type(void* dst, const void* src, size_t n_bytes, BLObjectType array_type) noexcept {
  if (is_array_type_object_based(array_type))
    assign_content_objects(dst, src, n_bytes);
  else
    memcpy(dst, src, n_bytes);
}

static BL_NOINLINE void release_content_objects(void* data, size_t n_bytes) noexcept {
  BL_ASSUME((n_bytes % sizeof(BLObjectCore)) == 0);

  for (size_t i = 0; i < n_bytes; i += sizeof(BLObjectCore))
    ObjectInternal::release_unknown_instance(PtrOps::offset<BLObjectCore>(data, i));
}

static BL_INLINE void release_content_by_type(void* data, size_t n_bytes, BLObjectType array_type) noexcept {
  if (is_array_type_object_based(array_type))
    release_content_objects(data, n_bytes);
}

static BL_INLINE void fill_content_objects(void* dst, size_t n, const void* src, size_t item_size) noexcept {
  // NOTE: This is the best we can do. We can increase the reference count of each item in the item / tuple
  // (in case the array stores pair/tuple of objects) and then just copy the content of BLObjectDetail to the
  // destination.
  BLObjectCore* dst_obj = static_cast<BLObjectCore*>(dst);
  const BLObjectCore* src_obj = static_cast<const BLObjectCore*>(src);

  size_t tuple_size = item_size / sizeof(BLObjectCore);
  BL_ASSERT(tuple_size > 0);

  size_t i, j;
  for (j = 0; j < tuple_size; j++) {
    ObjectInternal::retain_instance(&src_obj[j], n);
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < tuple_size; j++) {
      dst_obj->_d = src_obj[j]._d;
      dst_obj++;
    }
  }
}

static BL_INLINE void fill_content_simple(void* dst, size_t n, const void* src, size_t item_size) noexcept {
  switch (item_size) {
    case  1: MemOps::fill_inline_t(static_cast<uint8_t    *>(dst), *static_cast<const uint8_t    *>(src), n); break;
    case  2: MemOps::fill_inline_t(static_cast<uint16_t   *>(dst), *static_cast<const uint16_t   *>(src), n); break;
    case  4: MemOps::fill_inline_t(static_cast<uint32_t   *>(dst), *static_cast<const uint32_t   *>(src), n); break;
    case  8: MemOps::fill_inline_t(static_cast<UInt32xN<2>*>(dst), *static_cast<const UInt32xN<2>*>(src), n); break;
    case 12: MemOps::fill_inline_t(static_cast<UInt32xN<3>*>(dst), *static_cast<const UInt32xN<3>*>(src), n); break;
    case 16: MemOps::fill_inline_t(static_cast<UInt32xN<4>*>(dst), *static_cast<const UInt32xN<4>*>(src), n); break;

    default: {
      for (size_t i = 0; i < n; i++) {
        memcpy(dst, src, item_size);
        dst = PtrOps::offset(dst, item_size);
      }
      break;
    }
  }
}

static BL_INLINE bool equals_content(const void* a, const void* b, size_t n_bytes, BLObjectType array_type) noexcept {
  if (is_array_type_object_based(array_type)) {
    const BLObjectCore* a_obj = static_cast<const BLObjectCore*>(a);
    const BLObjectCore* b_obj = static_cast<const BLObjectCore*>(b);
    const BLObjectCore* a_end = PtrOps::offset(a_obj, n_bytes);

    while (a_obj != a_end) {
      if (!bl_var_equals(a_obj, b_obj))
        return false;

      a_obj++;
      b_obj++;
    }

    return true;
  }
  else {
    return memcmp(a, b, n_bytes) == 0;
  }
}

// bl::Array - Private - Alloc & Free Impl
// =======================================

static BL_INLINE BLResult init_static(BLArrayCore* self, BLObjectType array_type, size_t size = 0u) noexcept {
  self->_d = bl_object_defaults[array_type]._d;
  // We know the size is default Impl is always zero, so make this faster than `BLObjectInfo::set_a_field()`.
  self->_d.info.bits |= uint32_t(size) << BL_OBJECT_INFO_A_SHIFT;

  return BL_SUCCESS;
}

static BL_INLINE BLResult init_dynamic(BLArrayCore* self, BLObjectType array_type, size_t size, BLObjectImplSize impl_size) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(array_type);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLArrayImpl>(self, info, impl_size));

  BLArrayImpl* impl = get_impl(self);
  uint8_t* data = PtrOps::offset<uint8_t>(impl, sizeof(BLArrayImpl));
  size_t item_size = item_size_table[array_type];

  impl->capacity = capacity_from_impl_size(impl_size, item_size);
  impl->size = size;
  impl->data = data;
  return BL_SUCCESS;
}

static BL_INLINE BLResult init_external(
  BLArrayCore* self, BLObjectType array_type,
  void* external_data, size_t size, size_t capacity, BLDataAccessFlags access_flags,
  BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {

  BLObjectImplSize impl_size(sizeof(BLArrayImpl));
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(array_type);

  bool immutable = !(access_flags & BL_DATA_ACCESS_WRITE);
  BL_PROPAGATE(ObjectInternal::alloc_impl_external_t<BLArrayImpl>(self, info, impl_size, immutable, destroy_func, user_data));

  BLArrayImpl* impl = get_impl(self);
  impl->data = external_data;
  impl->size = size;
  impl->capacity = capacity;
  return BL_SUCCESS;
}

static BL_NOINLINE BLResult init_array(BLArrayCore* self, BLObjectType array_type, size_t size, size_t capacity, uint8_t** out) noexcept {
  size_t sso_capacity = sso_capacity_table[array_type];
  if (capacity <= sso_capacity) {
    init_static(self, array_type, size);
    *out = self->_d.u8_data;
    return BL_SUCCESS;
  }
  else {
    BL_PROPAGATE(init_dynamic(self, array_type, size, impl_size_from_capacity(capacity, item_size_from_array_type(array_type))));
    *out = get_impl(self)->data_as<uint8_t>();
    return BL_SUCCESS;
  }
}

static BL_NOINLINE BLResult realloc_to_dynamic(BLArrayCore* self, BLObjectType array_type, BLObjectImplSize impl_size) noexcept {
  BL_ASSERT(self->_d.raw_type() == array_type);

  size_t size = get_size(self);
  size_t item_size = item_size_from_array_type(array_type);

  BLArrayCore newO;
  BL_PROPAGATE(init_dynamic(&newO, array_type, size, impl_size));

  uint8_t* dst = get_impl(&newO)->data_as<uint8_t>();
  if (is_instance_dynamic_and_mutable(self)) {
    BLArrayImpl* tmp_impl = get_impl(self);
    memcpy(dst, tmp_impl->data, size * item_size);
    tmp_impl->size = 0;
  }
  else {
    init_content_by_type(dst, get_data(self), size * item_size, array_type);
  }

  return replace_instance(self, &newO);
}

BLResult free_impl(BLArrayImpl* impl) noexcept {
  if (ObjectInternal::is_impl_external(impl))
    ObjectInternal::call_external_destroy_func(impl, impl->data);
  return ObjectInternal::free_impl(impl);
}

// bl::Array - Private - Typed Operations
// ======================================

template<typename T>
static BL_INLINE BLResult append_value_t(BLArrayCore* self, T value) noexcept {
  BL_ASSERT(self->_d.is_array());
  BL_ASSERT(item_size_from_array_type(self->_d.raw_type()) == sizeof(T));

  if (self->_d.sso()) {
    size_t size = self->_d.a_field();
    size_t capacity = self->_d.b_field();

    BL_ASSERT(size <= capacity);
    if (size == capacity)
      return bl_array_append_item(self, &value);

    T* data = self->_d.data_as<T>() + size;
    self->_d.info.set_a_field(uint32_t(size + 1));

    *data = value;
    return BL_SUCCESS;
  }
  else {
    BLArrayImpl* self_impl = get_impl(self);

    size_t size = self_impl->size;
    size_t capacity = self_impl->capacity;
    size_t immutable_msk = IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

    // Not enough capacity or not mutable - bail to the generic implementation.
    if ((size | immutable_msk) >= capacity)
      return bl_array_append_item(self, &value);

    T* dst = self_impl->data_as<T>() + size;
    self_impl->size = size + 1;

    *dst = value;
    return BL_SUCCESS;
  }
}

template<typename T>
static BL_INLINE BLResult insert_value_t(BLArrayCore* self, size_t index, T value) noexcept {
  BL_ASSERT(self->_d.is_array());
  BL_ASSERT(item_size_from_array_type(self->_d.raw_type()) == sizeof(T));

  T* dst;
  BL_PROPAGATE(bl_array_insert_op(self, index, 1, (void**)&dst));

  *dst = value;
  return BL_SUCCESS;
}

template<typename T>
static BL_INLINE BLResult replace_value_t(BLArrayCore* self, size_t index, T value) noexcept {
  BL_ASSERT(self->_d.is_array());
  BL_ASSERT(item_size_from_array_type(self->_d.raw_type()) == sizeof(T));

  if (!self->_d.sso()) {
    BLArrayImpl* self_impl = get_impl(self);
    size_t size = self_impl->size;

    if (BL_UNLIKELY(index >= size))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    // Not mutable - don't inline as this is an expensive case anyway.
    if (!is_impl_mutable(self_impl))
      return bl_array_replace_item(self, index, &value);

    T* data = self_impl->data_as<T>();
    data[index] = value;

    return BL_SUCCESS;
  }
  else {
    size_t size = self->_d.a_field();
    if (BL_UNLIKELY(index >= size))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    T* data = self->_d.data_as<T>();
    data[index] = value;

    return BL_SUCCESS;
  }
}

} // {bl::ArrayInternal}

// bl::Array - API - Init & Destroy
// ================================

BL_API_IMPL BLResult bl_array_init(BLArrayCore* self, BLObjectType array_type) noexcept {
  using namespace bl::ArrayInternal;

  BLResult result = BL_SUCCESS;
  if (BL_UNLIKELY(!is_array_type_valid(array_type))) {
    array_type = BL_OBJECT_TYPE_NULL;
    result = bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  init_static(self, array_type);
  return result;
}

BL_API_IMPL BLResult bl_array_init_move(BLArrayCore* self, BLArrayCore* other) noexcept {
  using namespace bl::ArrayInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_array());

  self->_d = other->_d;
  return init_static(other, other->_d.raw_type());
}

BL_API_IMPL BLResult bl_array_init_weak(BLArrayCore* self, const BLArrayCore* other) noexcept {
  using namespace bl::ArrayInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_array());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_array_destroy(BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  return release_instance(self);
}

// bl::Array - API - Reset
// =======================

BL_API_IMPL BLResult bl_array_reset(BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  BLObjectType array_type = self->_d.raw_type();
  return replace_instance(self, static_cast<const BLArrayCore*>(&bl_object_defaults[array_type]));
}

// bl::Array - API - Accessors
// ===========================

BL_API_IMPL size_t bl_array_get_size(const BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  return get_size(self);
}

BL_API_IMPL size_t bl_array_get_capacity(const BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  return get_capacity(self);
}

BL_API_IMPL size_t bl_array_get_item_size(BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  return item_size_from_array_type(self->_d.raw_type());
}

BL_API_IMPL const void* bl_array_get_data(const BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  return get_data(self);
}

// bl::Array - API - Data Manipulation
// ===================================

BL_API_IMPL BLResult bl_array_clear(BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  if (self->_d.sso()) {
    size_t size = self->_d.a_field();
    if (size) {
      self->_d.clear_static_data();
      self->_d.info.set_a_field(0);
    }
    return BL_SUCCESS;
  }
  else {
    BLArrayImpl* self_impl = get_impl(self);
    BLObjectType array_type = self->_d.raw_type();

    if (!is_impl_mutable(self_impl)) {
      release_instance(self);
      return init_static(self, array_type);
    }

    size_t size = self_impl->size;
    if (!size)
      return BL_SUCCESS;

    size_t item_size = item_size_table[array_type];
    release_content_by_type(self_impl->data, size * item_size, array_type);

    self_impl->size = 0;
    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_array_shrink(BLArrayCore* self) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  BLObjectType array_type = self->_d.raw_type();

  size_t item_size = item_size_from_array_type(array_type);
  uint32_t sso_capacity = sso_capacity_table[array_type];

  // 1. Try to move the content to static storage, if possible.
  if (u.size <= sso_capacity) {
    if (self->_d.sso())
      return BL_SUCCESS;

    BLArrayCore newO;
    newO._d.init_static(BLObjectInfo::from_type_with_marker(array_type) |
                       BLObjectInfo::from_abcp(uint32_t(u.size), sso_capacity));
    memcpy(newO._d.u8_data, u.data, u.size * item_size);
    return replace_instance(self, &newO);
  }

  // 2. Don't touch arrays that hold external data.
  if (bl::ObjectInternal::is_impl_external(self->_d.impl))
    return BL_SUCCESS;

  // 2. Only reallocate if we can save at least a single cache line.
  BLObjectImplSize fitting_impl_size = impl_size_from_capacity(u.size, item_size);
  BLObjectImplSize current_impl_size = impl_size_from_capacity(u.capacity, item_size);

  if (current_impl_size - fitting_impl_size >= BL_OBJECT_IMPL_ALIGNMENT)
    return realloc_to_dynamic(self, array_type, fitting_impl_size);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_array_resize(BLArrayCore* self, size_t n, const void* fill) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);

  // If `n` is smaller than the current `size` then this is a truncation. We only have
  // to cover the BLObjectCore[] case, which means to destroy all variants beyond `n`.
  if (n <= u.size) {
    if (!is_instance_mutable(self)) {
      if (n == u.size)
        return BL_SUCCESS;

      BLArrayCore newO;
      uint8_t* dst;
      BL_PROPAGATE(init_array(&newO, array_type, n, n, &dst));

      init_content_by_type(dst, u.data, n * item_size, array_type);
      return replace_instance(self, &newO);
    }
    else {
      set_size(self, n);
      release_content_by_type(u.data, (u.size - n) * item_size, array_type);
      return BL_SUCCESS;
    }
  }

  // `n` becames the number of items to add to the array.
  n -= u.size;

  void* dst;
  BL_PROPAGATE(bl_array_modify_op(self, BL_MODIFY_OP_APPEND_FIT, n, &dst));

  if (!fill)
    memset(dst, 0, n * item_size);
  else if (is_array_type_object_based(array_type))
    fill_content_objects(dst, n, fill, item_size);
  else
    fill_content_simple(dst, n, fill, item_size);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_array_reserve(BLArrayCore* self, size_t n) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((n | immutable_msk) <= u.capacity)
    return BL_SUCCESS;

  BLObjectType array_type = self->_d.raw_type();
  if (BL_UNLIKELY(n > maximum_capacity_table[array_type]))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  size_t sso_capacity = sso_capacity_table[array_type];
  size_t item_size = item_size_from_array_type(array_type);
  n = bl_max(n, u.size);

  if (n <= sso_capacity) {
    BLArrayCore newO;
    init_static(&newO, array_type, u.size);

    uint8_t* dst = newO._d.u8_data;
    bl::MemOps::copy_forward_inline_t(dst, get_impl(self)->data_as<uint8_t>(), u.size * item_size);
    return replace_instance(self, &newO);
  }
  else {
    return realloc_to_dynamic(self, array_type, impl_size_from_capacity(n, item_size));
  }
}

BL_API_IMPL BLResult bl_array_make_mutable(BLArrayCore* self, void** data_out) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  if (self->_d.sso()) {
    *data_out = self->_d.u8_data;
    return BL_SUCCESS;
  }

  BLArrayImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    *data_out = self_impl->data;
    return BL_SUCCESS;
  }

  BLObjectType array_type = self->_d.raw_type();
  size_t size = self_impl->size;
  size_t item_size = item_size_from_array_type(array_type);

  BLArrayCore tmp = *self;
  uint8_t* dst;

  BL_PROPAGATE(init_array(self, array_type, size, size, &dst));
  init_content_by_type(dst, self_impl->data, size * item_size, array_type);
  release_instance(&tmp);

  *data_out = dst;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_array_modify_op(BLArrayCore* self, BLModifyOp op, size_t n, void** data_out) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_table[array_type];

  UnpackedData u;

  size_t index;
  size_t size_after;

  if (self->_d.sso()) {
    u.data = self->_d.u8_data;
    u.size = self->_d.a_field();
    u.capacity = self->_d.b_field();

    if (bl_modify_op_is_assign(op)) {
      index = 0;
      size_after = n;

      if (size_after <= u.capacity) {
        self->_d.info.set_a_field(uint32_t(size_after));
        self->_d.clear_static_data();

        *data_out = self->_d.u8_data;
        return BL_SUCCESS;
      }
    }
    else {
      bl::OverflowFlag of{};
      index = u.size;
      size_after = bl::IntOps::add_overflow(u.size, n, &of);

      if (BL_UNLIKELY(of))
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

      if (size_after <= u.capacity) {
        self->_d.info.set_a_field(uint32_t(size_after));

        *data_out = self->_d.u8_data + index * item_size;
        return BL_SUCCESS;
      }
    }
  }
  else {
    BLArrayImpl* self_impl = get_impl(self);
    size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

    u.data = self_impl->data_as<uint8_t>();
    u.size = self_impl->size;
    u.capacity = self_impl->capacity;

    if (bl_modify_op_is_assign(op)) {
      index = 0;
      size_after = n;

      if ((size_after | immutable_msk) <= u.capacity) {
        self_impl->size = size_after;
        release_content_by_type(u.data, u.size * item_size, array_type);

        *data_out = u.data;
        return BL_SUCCESS;
      }
    }
    else {
      bl::OverflowFlag of{};
      index = u.size;
      size_after = bl::IntOps::add_overflow(u.size, n, &of);

      if (BL_UNLIKELY(of))
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

      if ((size_after | immutable_msk) <= u.capacity) {
        self_impl->size = size_after;

        *data_out = u.data + index * item_size;
        return BL_SUCCESS;
      }
    }
  }

  // The container is either immutable or doesn't have the capacity required.
  BLArrayCore newO;
  size_t sso_capacity = sso_capacity_table[array_type];

  if (size_after <= sso_capacity) {
    // The new content fits in static storage, which implies that the current content must be dynamic.
    BL_ASSERT(!self->_d.sso());

    newO._d.init_static(BLObjectInfo::from_type_with_marker(array_type) |
                       BLObjectInfo::from_abcp(uint32_t(size_after), uint32_t(sso_capacity)));
    bl::MemOps::copy_forward_inline_t(newO._d.u8_data, u.data, index * item_size);

    *data_out = self->_d.u8_data + index * item_size;
    return replace_instance(self, &newO);
  }
  else {
    BLObjectImplSize impl_size = expand_impl_size_with_modify_op(impl_size_from_capacity(size_after, item_size), op);
    BL_PROPAGATE(init_dynamic(&newO, array_type, size_after, impl_size));

    uint8_t* dst = get_impl(&newO)->data_as<uint8_t>();
    if (self->_d.is_dynamic_object() && bl::ObjectInternal::is_impl_mutable(self->_d.impl)) {
      // Use memcpy() instead of weak copying if the original data is gonna be destroyed.
      memcpy(dst, u.data, index * item_size);
      // We have to patch the source Impl as release_instance() as we have moved the content.
      get_impl(self)->size = 0;
    }
    else {
      init_content_by_type(dst, u.data, index * item_size, array_type);
    }

    *data_out = dst + index * item_size;
    return replace_instance(self, &newO);
  }
}

BL_API_IMPL BLResult bl_array_insert_op(BLArrayCore* self, size_t index, size_t n, void** data_out) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);

  size_t size_after = bl::IntOps::uadd_saturate(u.size, n);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((size_after | immutable_msk) > u.capacity) {
    if (BL_UNLIKELY(size_after > maximum_capacity_table[array_type]))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    BLArrayCore tmp = *self;

    uint8_t* dst = nullptr;
    const uint8_t* src = get_data<uint8_t>(&tmp);

    size_t sso_capacity = sso_capacity_table[array_type];
    if (size_after <= sso_capacity) {
      init_static(self, array_type, size_after);
      dst = self->_d.u8_data;
    }
    else {
      BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(size_after, item_size));
      *data_out = nullptr;

      BL_PROPAGATE(init_dynamic(self, array_type, size_after, impl_size));
      dst = get_impl(self)->data_as<uint8_t>();
    }

    if (!immutable_msk) {
      // Move if `tmp` will be destroyed.
      memcpy(dst, src, index * item_size);
      memcpy(dst + (index + n) * item_size, src + index * item_size, (u.size - index) * item_size);
      set_size(&tmp, 0);
    }
    else {
      init_content_by_type(dst, src, index * item_size, array_type);
      init_content_by_type(dst + (index + n) * item_size, src + index * item_size, (u.size - index) * item_size, array_type);
    }

    *data_out = dst + index * item_size;
    return release_instance(&tmp);
  }
  else {
    set_size(self, size_after);
    memmove(u.data + (index + n) * item_size, u.data + index * item_size, (u.size - index) * item_size);

    *data_out = u.data + index * item_size;
    return BL_SUCCESS;
  }
}

// bl::Array - API - Data Manipulation - Assignment
// ================================================

BL_API_IMPL BLResult bl_array_assign_move(BLArrayCore* self, BLArrayCore* other) noexcept {
  using namespace bl::ArrayInternal;

  BL_ASSERT(self->_d.is_array());
  BL_ASSERT(other->_d.is_array());
  BL_ASSERT(self->_d.raw_type() == other->_d.raw_type());

  BLObjectType array_type = self->_d.raw_type();
  BLArrayCore tmp = *other;

  init_static(other, array_type);
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_array_assign_weak(BLArrayCore* self, const BLArrayCore* other) noexcept {
  using namespace bl::ArrayInternal;

  BL_ASSERT(self->_d.is_array());
  BL_ASSERT(other->_d.is_array());
  BL_ASSERT(self->_d.raw_type() == other->_d.raw_type());

  retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_array_assign_deep(BLArrayCore* self, const BLArrayCore* other) noexcept {
  using namespace bl::ArrayInternal;

  BL_ASSERT(self->_d.is_array());
  BL_ASSERT(other->_d.is_array());
  BL_ASSERT(self->_d.raw_type() == other->_d.raw_type());

  return bl_array_assign_data(self, get_data(other), get_size(other));
}

BL_API_IMPL BLResult bl_array_assign_data(BLArrayCore* self, const void* items, size_t n) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((n | immutable_msk) > u.capacity) {
    if (BL_UNLIKELY(n > maximum_capacity_table[array_type]))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    BLObjectImplSize impl_size = impl_size_from_capacity(u.size, item_size);
    BLArrayCore newO;
    BL_PROPAGATE(init_dynamic(&newO, array_type, n, impl_size));

    uint8_t* dst = get_impl(&newO)->data_as<uint8_t>();
    init_content_by_type(dst, items, n * item_size, array_type);
    return replace_instance(self, &newO);
  }

  if (!n)
    return bl_array_clear(self);

  set_size(self, n);

  if (is_array_type_object_based(array_type)) {
    size_t replace_size = bl_min(u.size, n);
    const uint8_t* src = static_cast<const uint8_t*>(items);

    assign_content_objects(u.data, src, replace_size * item_size);
    release_content_objects(u.data + replace_size * item_size, (u.size - replace_size) * item_size);

    return BL_SUCCESS;
  }
  else {
    // Memory move is required in case of overlap between `data` and `items`.
    memmove(u.data, items, n * item_size);
    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_array_assign_external_data(BLArrayCore* self, void* external_data, size_t size, size_t capacity, BLDataAccessFlags access_flags, BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);

  bl::OverflowFlag of{};
  bl::IntOps::mul_overflow(capacity, item_size, &of);

  if (BL_UNLIKELY(!capacity || capacity < size || !bl_data_access_flags_is_valid(access_flags) || of))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLArrayCore newO;
  BL_PROPAGATE(init_external(&newO, array_type, external_data, size, capacity, access_flags, destroy_func, user_data));

  return replace_instance(self, &newO);
}

// bl::Array - API - Data Manipulation - Append
// ============================================

BL_API_IMPL BLResult bl_array_append_u8(BLArrayCore* self, uint8_t value) noexcept { return bl::ArrayInternal::append_value_t<uint8_t>(self, value); }
BL_API_IMPL BLResult bl_array_append_u16(BLArrayCore* self, uint16_t value) noexcept { return bl::ArrayInternal::append_value_t<uint16_t>(self, value); }
BL_API_IMPL BLResult bl_array_append_u32(BLArrayCore* self, uint32_t value) noexcept { return bl::ArrayInternal::append_value_t<uint32_t>(self, value); }
BL_API_IMPL BLResult bl_array_append_u64(BLArrayCore* self, uint64_t value) noexcept { return bl::ArrayInternal::append_value_t<uint64_t>(self, value); }
BL_API_IMPL BLResult bl_array_append_f32(BLArrayCore* self, float value) noexcept { return bl::ArrayInternal::append_value_t<float>(self, value); }
BL_API_IMPL BLResult bl_array_append_f64(BLArrayCore* self, double value) noexcept { return bl::ArrayInternal::append_value_t<double>(self, value); }

BL_API_IMPL BLResult bl_array_append_item(BLArrayCore* self, const void* item) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if (BL_UNLIKELY((u.size | immutable_msk) >= u.capacity)) {
    if (BL_UNLIKELY(u.size >= maximum_capacity_table[array_type]))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    BLArrayCore newO;
    BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(u.size + 1u, item_size));
    BL_PROPAGATE(init_dynamic(&newO, array_type, u.size + 1, impl_size));

    // Copy the existing data to a new place / move if the data will be destroyed.
    uint8_t* dst = get_impl(&newO)->data_as<uint8_t>();
    if (!immutable_msk) {
      set_size(self, 0);
      memcpy(dst, u.data, u.size * item_size);
    }
    else {
      init_content_by_type(dst, u.data, u.size * item_size, array_type);
    }

    init_content_by_type(dst + u.size * item_size, item, item_size, array_type);
    return replace_instance(self, &newO);
  }
  else {
    init_content_by_type(u.data + u.size * item_size, item, item_size, array_type);
    set_size(self, u.size + 1);

    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_array_append_data(BLArrayCore* self, const void* items, size_t n) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  size_t size_after = bl::IntOps::uadd_saturate(u.size, n);

  if (BL_UNLIKELY((size_after | immutable_msk) > u.capacity)) {
    if (BL_UNLIKELY(size_after > maximum_capacity_table[array_type]))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    BLArrayCore newO;
    BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(size_after, item_size));
    BL_PROPAGATE(init_dynamic(&newO, array_type, size_after, impl_size));

    // Copy the existing data to a new place / move if the data will be destroyed.
    uint8_t* dst = get_impl(&newO)->data_as<uint8_t>();
    if (!immutable_msk) {
      set_size(self, 0);
      memcpy(dst, u.data, u.size * item_size);
    }
    else {
      init_content_by_type(dst, u.data, u.size * item_size, array_type);
    }

    init_content_by_type(dst + u.size * item_size, items, n * item_size, array_type);
    return replace_instance(self, &newO);
  }
  else {
    init_content_by_type(u.data + u.size * item_size, items, n * item_size, array_type);
    set_size(self, size_after);

    return BL_SUCCESS;
  }
}

// bl::Array - API - Data Manipulation - Insert
// ============================================

BL_API_IMPL BLResult bl_array_insert_u8(BLArrayCore* self, size_t index, uint8_t value) noexcept { return bl::ArrayInternal::insert_value_t<uint8_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_insert_u16(BLArrayCore* self, size_t index, uint16_t value) noexcept { return bl::ArrayInternal::insert_value_t<uint16_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_insert_u32(BLArrayCore* self, size_t index, uint32_t value) noexcept { return bl::ArrayInternal::insert_value_t<uint32_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_insert_u64(BLArrayCore* self, size_t index, uint64_t value) noexcept { return bl::ArrayInternal::insert_value_t<uint64_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_insert_f32(BLArrayCore* self, size_t index, float value) noexcept { return bl::ArrayInternal::insert_value_t<float>(self, index, value); }
BL_API_IMPL BLResult bl_array_insert_f64(BLArrayCore* self, size_t index, double value) noexcept { return bl::ArrayInternal::insert_value_t<double>(self, index, value); }
BL_API_IMPL BLResult bl_array_insert_item(BLArrayCore* self, size_t index, const void* item) noexcept { return bl_array_insert_data(self, index, item, 1); }

BL_API_IMPL BLResult bl_array_insert_data(BLArrayCore* self, size_t index, const void* items, size_t n) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  size_t end_index = index + n;
  size_t size_after = bl::IntOps::uadd_saturate(u.size, n);

  if ((size_after | immutable_msk) > u.capacity) {
    if (BL_UNLIKELY(size_after > maximum_capacity_table[array_type]))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(size_after, item_size));
    BLArrayCore newO;
    BL_PROPAGATE(init_dynamic(&newO, array_type, size_after, impl_size));

    uint8_t* dst = get_impl(&newO)->data_as<uint8_t>();
    if (!immutable_msk) {
      set_size(self, 0);
      memcpy(dst, u.data, index * item_size);
      memcpy(dst + end_index * item_size, u.data + index * item_size, (u.size - index) * item_size);
    }
    else {
      init_content_by_type(dst, u.data, index * item_size, array_type);
      init_content_by_type(dst + end_index * item_size, u.data + index * item_size, (u.size - index) * item_size, array_type);
    }

    init_content_by_type(dst + index * item_size, items, n * item_size, array_type);
    return replace_instance(self, &newO);
  }
  else {
    size_t nInBytes = n * item_size;

    uint8_t* dst = u.data;
    uint8_t* dst_end = dst + u.size * item_size;
    const uint8_t* src = static_cast<const uint8_t*>(items);

    // The destination would point into the first byte that will be modified. So for example if the
    // data is `[ABCDEF]` and we are inserting at index 1 then the `dst` would point to `[BCDEF]`.
    dst += index * item_size;
    dst_end += nInBytes;

    // Move the memory in-place making space for items to insert. For example if the destination points
    // to [ABCDEF] and we want to insert 4 items we would get [____ABCDEF].
    memmove(dst + nInBytes, dst, (u.size - index) * item_size);

    // Split the [src:src_end] into LEAD and TRAIL slices and shift TRAIL slice in a way to cancel the
    // `memmove()` if `src` overlaps `dst`. In practice if there is an overlap the [src:src_end] source
    // should be within [dst:dst_end] as it doesn't make sense to insert something which is outside of
    // the current valid area.
    //
    // This illustrates how the input is divided into leading and traling data.
    //
    //   BCDEFGH    <- Insert This
    // [abcdefghi]
    //      ^       <- Here
    //
    // [abcd_______efgh]
    //              <- memmove()
    //
    //      |-|     <- Copy leading data
    // [abcdBCD____efgh]
    //
    //         |--| <- Copy shifted trailing data.
    // [abcdBCDEFGHefgh]

    // Leading area precedes `dst` - nothing changed in here and if this is the whole area then there
    // was no overlap that we would have to deal with.
    size_t nLeadBytes = 0;
    if (src < dst) {
      nLeadBytes = bl_min<size_t>((size_t)(dst - src), nInBytes);
      init_content_by_type(dst, src, nLeadBytes, array_type);

      dst += nLeadBytes;
      src += nLeadBytes;
    }

    // Trailing area - we either shift none or all of it.
    if (src < dst_end)
      src += nInBytes; // Shift source in case of overlap.

    init_content_by_type(dst, src, nInBytes - nLeadBytes, array_type);
    set_size(self, size_after);

    return BL_SUCCESS;
  }
}

// bl::Array - API - Data Manipulation - Replace
// =============================================

BL_API_IMPL BLResult bl_array_replace_u8(BLArrayCore* self, size_t index, uint8_t value) noexcept { return bl::ArrayInternal::replace_value_t<uint8_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_replace_u16(BLArrayCore* self, size_t index, uint16_t value) noexcept { return bl::ArrayInternal::replace_value_t<uint16_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_replace_u32(BLArrayCore* self, size_t index, uint32_t value) noexcept { return bl::ArrayInternal::replace_value_t<uint32_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_replace_u64(BLArrayCore* self, size_t index, uint64_t value) noexcept { return bl::ArrayInternal::replace_value_t<uint64_t>(self, index, value); }
BL_API_IMPL BLResult bl_array_replace_f32(BLArrayCore* self, size_t index, float value) noexcept { return bl::ArrayInternal::replace_value_t<float>(self, index, value); }
BL_API_IMPL BLResult bl_array_replace_f64(BLArrayCore* self, size_t index, double value) noexcept { return bl::ArrayInternal::replace_value_t<double>(self, index, value); }

BL_API_IMPL BLResult bl_array_replace_item(BLArrayCore* self, size_t index, const void* item) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  if (BL_UNLIKELY(index >= u.size))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);

  if (!is_instance_mutable(self)) {
    BLArrayCore newO;
    BLObjectImplSize impl_size = impl_size_from_capacity(u.size, item_size);
    BL_PROPAGATE(init_dynamic(&newO, array_type, u.size, impl_size));

    uint8_t* dst = get_impl(&newO)->data_as<uint8_t>();
    const uint8_t* src = u.data;

    init_content_by_type(dst, src, index * item_size, array_type);
    dst += index * item_size;
    src += index * item_size;

    init_content_by_type(dst, item, item_size, array_type);
    dst += item_size;
    src += item_size;

    init_content_by_type(dst, src, (u.size - index - 1) * item_size, array_type);
    return replace_instance(self, &newO);
  }
  else {
    assign_content_by_type(u.data + index * item_size, item, item_size, array_type);
    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_array_replace_data(BLArrayCore* self, size_t r_start, size_t r_end, const void* items, size_t n) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  size_t end = bl_min(r_end, u.size);
  size_t index = bl_min(r_start, end);
  size_t range_size = end - index;

  if (!range_size)
    return bl_array_insert_data(self, index, items, n);

  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);

  size_t tail_size = u.size - end;
  size_t size_after = u.size - range_size + n;

  if (is_instance_mutable(self)) {
    // 0           |<-Start   End->|          | <- Size
    // ^***********^***************^**********^
    // | Unchanged |  Replacement  | TailSize |
    //
    // <  Less     |+++++++| <- MidEnd
    // == Equal    |+++++++++++++++| <- MidEnd
    // >  Greater  |++++++++++++++++++++++| <- MidEnd
    const uint8_t* items_ptr = static_cast<const uint8_t*>(items);
    const uint8_t* items_end = items_ptr + item_size * n;

    if (BL_LIKELY(items_ptr >= u.data + u.size * n || items_end <= u.data)) {
      // Non-overlaping case (the expected one).
      if (range_size == n) {
        assign_content_by_type(u.data + index * item_size, items, n * item_size, array_type);
      }
      else {
        release_content_by_type(u.data + index * item_size, range_size * item_size, array_type);
        memmove(u.data + (index + range_size) * item_size, u.data + end * item_size, tail_size * item_size);
        init_content_by_type(u.data + index * item_size, items, n * item_size, array_type);
        set_size(self, size_after);
      }
      return BL_SUCCESS;
    }
  }

  // Array is either immmutable or its data overlaps with `items`.
  BLArrayCore newO;
  uint8_t* dst;
  BL_PROPAGATE(init_array(&newO, array_type, size_after, size_after, &dst));

  const uint8_t* src = u.data;
  init_content_by_type(dst, src, index * item_size, array_type);

  dst += index * item_size;
  src += (index + range_size) * item_size;

  init_content_by_type(dst, items, n * item_size, array_type);
  dst += n * item_size;

  init_content_by_type(dst, src, tail_size * item_size, array_type);
  return replace_instance(self, &newO);
}

// bl::Array - API - Data Manipulation - Remove
// ============================================

BL_API_IMPL BLResult bl_array_remove_index(BLArrayCore* self, size_t index) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  return bl_array_remove_range(self, index, index + 1);
}

BL_API_IMPL BLResult bl_array_remove_range(BLArrayCore* self, size_t r_start, size_t r_end) noexcept {
  using namespace bl::ArrayInternal;
  BL_ASSERT(self->_d.is_array());

  UnpackedData u = unpack(self);
  size_t end = bl_min(r_end, u.size);
  size_t index = bl_min(r_start, end);

  size_t n = end - index;
  size_t size_after = u.size - n;

  if (!n)
    return BL_SUCCESS;

  BLObjectType array_type = self->_d.raw_type();
  size_t item_size = item_size_from_array_type(array_type);

  if (self->_d.sso()) {
    size_t sso_capacity = self->_d.b_field();

    bl::MemOps::copy_small(u.data + index * item_size, u.data + (index + n) * item_size, (u.size - end) * item_size);
    bl::MemOps::fill_small_t(u.data + size_after * item_size, uint8_t(0), (sso_capacity - size_after) * item_size);

    self->_d.info.set_a_field(uint32_t(size_after));
    return BL_SUCCESS;
  }

  BLArrayImpl* self_impl = get_impl(self);
  if (!is_impl_mutable(self_impl)) {
    BLArrayCore newO;
    uint8_t* dst;
    BL_PROPAGATE(init_array(&newO, array_type, size_after, size_after, &dst));

    init_content_by_type(dst, u.data, index * item_size, array_type);
    init_content_by_type(dst + index * item_size, u.data + end * item_size, (u.size - end) * item_size, array_type);

    return replace_instance(self, &newO);
  }
  else {
    uint8_t* data = get_data<uint8_t>(self) + index * item_size;

    release_content_by_type(data, n * item_size, array_type);
    memmove(data, data + n * item_size, (u.size - end) * item_size);

    self_impl->size = size_after;
    return BL_SUCCESS;
  }
}

// bl::Array - API - Equality & Comparison
// =======================================

BL_API_IMPL bool bl_array_equals(const BLArrayCore* a, const BLArrayCore* b) noexcept {
  using namespace bl::ArrayInternal;

  BL_ASSERT(a->_d.is_array());
  BL_ASSERT(b->_d.is_array());

  if (a->_d == b->_d)
    return true;

  // NOTE: This should never happen. Mixing array types is not supported.
  BLObjectType array_type = a->_d.raw_type();
  BL_ASSERT(array_type == b->_d.raw_type());

  // However, if it happens, we want the comparison to return false in release builds.
  if (array_type != b->_d.raw_type())
    return false;

  UnpackedData au = unpack(a);
  UnpackedData bu = unpack(b);

  if (au.size != bu.size)
    return false;

  size_t item_size = item_size_from_array_type(array_type);
  return equals_content(au.data, bu.data, au.size * item_size, array_type);
}

// bl::Array - Runtime Registration
// ================================

void bl_array_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  for (uint32_t object_type = BL_OBJECT_TYPE_MIN_ARRAY; object_type <= BL_OBJECT_TYPE_MAX_ARRAY; object_type++) {
    bl_object_defaults[object_type]._d.init_static(
      BLObjectInfo::from_type_with_marker(BLObjectType(object_type)) |
      BLObjectInfo::from_abcp(0, bl::ArrayInternal::sso_capacity_table[object_type]));
  }
}
