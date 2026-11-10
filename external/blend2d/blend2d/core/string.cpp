// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/stringops_p.h>

#include <stdio.h>

namespace bl {
namespace StringInternal {

// bl::String - Private - Preconditions
// ====================================

static_assert(((BL_OBJECT_TYPE_STRING << BL_OBJECT_INFO_TYPE_SHIFT) & 0xFFFFu) == 0,
              "BL_OBJECT_TYPE_STRING must be a value that would not use any bits in the two lowest bytes in the "
              "object info, which can be used by BLString on little endian targets to store 13th and 14th byte.");

// bl::String - Private - Commons
// ==============================

static BL_INLINE constexpr size_t get_maximum_size() noexcept {
  return capacity_from_impl_size(BLObjectImplSize(BL_OBJECT_IMPL_MAX_SIZE));
}

static BL_INLINE BLObjectImplSize expand_impl_size(BLObjectImplSize impl_size) noexcept {
  return bl_object_expand_impl_size(impl_size);
}

static BLObjectImplSize expand_impl_size_with_modify_op(BLObjectImplSize impl_size, BLModifyOp modify_op) noexcept {
  return bl_object_expand_impl_size_with_modify_op(impl_size, modify_op);
}

static BL_INLINE void set_sso_size(BLStringCore* self, size_t new_size) noexcept {
  self->_d.info.set_a_field(uint32_t(new_size) ^ BLString::kSSOCapacity);
}

static BL_INLINE void set_size(BLStringCore* self, size_t new_size) noexcept {
  BL_ASSERT(new_size <= get_capacity(self));
  if (self->_d.sso())
    set_sso_size(self, new_size);
  else
    get_impl(self)->size = new_size;
}

static BL_INLINE void clear_sso_data(BLStringCore* self) noexcept {
  memset(self->_d.char_data, 0, bl_max<size_t>(BLString::kSSOCapacity, BLObjectDetail::kStaticDataSize));
}

// bl::String - Private - Alloc & Free Impl
// ========================================

static BL_INLINE BLResult init_sso(BLStringCore* self, size_t size = 0u) noexcept {
  self->_d.init_static(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_STRING) |
                      BLObjectInfo::from_abcp(uint32_t(size) ^ BLString::kSSOCapacity));
  return BL_SUCCESS;
}

static BL_INLINE BLResult init_dynamic(BLStringCore* self, BLObjectImplSize impl_size, size_t size = 0u) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_STRING);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLStringImpl>(self, info, impl_size));

  BLStringImpl* impl = get_impl(self);
  impl->capacity = capacity_from_impl_size(impl_size);
  impl->size = size;
  impl->data()[size] = '\0';
  return BL_SUCCESS;
}

static BL_NOINLINE BLResult init_string(BLStringCore* self, size_t size, size_t capacity, char** out) noexcept {
  BL_ASSERT(capacity >= size);

  if (capacity <= BLString::kSSOCapacity) {
    init_sso(self, size);
    *out = self->_d.char_data;
    return BL_SUCCESS;
  }
  else {
    BL_PROPAGATE(init_dynamic(self, impl_size_from_capacity(size), size));
    *out = get_impl(self)->data();
    return BL_SUCCESS;
  }
}

static BL_NOINLINE BLResult init_string_and_copy(BLStringCore* self, size_t capacity, const char* str, size_t size) noexcept {
  BL_ASSERT(capacity >= size);
  BL_ASSERT(size != SIZE_MAX);

  char* dst;
  BL_PROPAGATE(init_string(self, size, capacity, &dst));

  memcpy(dst, str, size);
  return BL_SUCCESS;
}

// bl::String - Private - Manipulation
// ===================================

static BLResult modify_and_copy(BLStringCore* self, BLModifyOp op, const char* str, size_t n) noexcept {
  UnpackedData u = unpack_data(self);
  size_t index = bl_modify_op_is_append(op) ? u.size : size_t(0);
  size_t size_after = IntOps::uadd_saturate(index, n);
  size_t immutable_msk = IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((size_after | immutable_msk) > u.capacity) {
    if (BL_UNLIKELY(size_after > get_maximum_size()))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    // Use a temporary object to avoid possible overlaps with both 'self' and 'str'.
    BLStringCore newO;
    char* dst = nullptr;

    if (size_after <= BLString::kSSOCapacity && !bl_modify_op_does_grow(op)) {
      init_sso(self, size_after);
      dst = self->_d.char_data;
    }
    else {
      BLObjectImplSize impl_size = expand_impl_size_with_modify_op(impl_size_from_capacity(size_after), op);
      BL_PROPAGATE(init_dynamic(&newO, impl_size, size_after));

      dst = get_impl(&newO)->data();
    }

    memcpy(dst, u.data, index);
    memcpy(dst + index, str, n);

    return replace_instance(self, &newO);
  }

  memmove(u.data + index, str, n);
  u.data[size_after] = '\0';

  if (self->_d.sso()) {
    set_sso_size(self, size_after);
    if (bl_modify_op_is_assign(op))
      MemOps::fill_inline_t(u.data + size_after, char(0), BLString::kSSOCapacity - size_after);
    return BL_SUCCESS;
  }
  else {
    get_impl(self)->size = size_after;
    return BL_SUCCESS;
  }
}

static BLResult insert_and_copy(BLStringCore* self, size_t index, const char* str, size_t n) noexcept {
  UnpackedData u = unpack_data(self);
  size_t end_index = index + n;
  size_t size_after = IntOps::uadd_saturate(u.size, n);
  size_t immutable_msk = IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((size_after | immutable_msk) > u.capacity) {
    if (BL_UNLIKELY(size_after > get_maximum_size()))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    BLStringCore newO;
    BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(size_after));
    BL_PROPAGATE(init_dynamic(&newO, impl_size, size_after));

    char* dst = get_impl(&newO)->data();
    memcpy(dst, u.data, index);
    memcpy(dst + end_index, u.data +  index, u.size - index);
    memcpy(dst + index, str, n);

    return replace_instance(self, &newO);
  }
  else {
    set_size(self, size_after);

    char* dst = u.data;
    char* dst_end = dst + u.size;

    // The destination would point into the first byte that will be modified. So for example if the
    // data is `[ABCDEF]` and we are inserting at index 1 then the `dst` would point to `[BCDEF]`.
    dst += index;
    dst_end += n;

    // Move the memory in-place making space for items to insert. For example if the destination points
    // to [ABCDEF] and we want to insert 4 items we  would get [____ABCDEF].
    //
    // NOTE: +1 includes a NULL terminator.
    memmove(dst + n, dst, u.size - index + 1);

    // Split the [str:str_end] into LEAD and TRAIL slices and shift TRAIL slice in a way to cancel the `memmove()` if
    // `str` overlaps `dst`. In practice if there is an overlap the [str:str_end] source should be within [dst:dst_end]
    // as it doesn't make sense to insert something which is outside of the current valid area.
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

    // Leading area precedes `dst` - nothing changed in here and if this is the whole area then there was no overlap
    // that we would have to deal with.
    size_t nLeadBytes = 0;
    if (str < dst) {
      nLeadBytes = bl_min<size_t>((size_t)(dst - str), n);
      memcpy(dst, str, nLeadBytes);

      dst += nLeadBytes;
      str += nLeadBytes;
    }

    // Trailing area - we either shift none or all of it.
    if (str < dst_end)
      str += n; // Shift source in case of overlap.

    memcpy(dst, str, n - nLeadBytes);
    return BL_SUCCESS;
  }
}

} // {StringInternal}
} // {bl}

// bl::String - API - Construction & Destruction
// =============================================

BL_API_IMPL BLResult bl_string_init(BLStringCore* self) noexcept {
  using namespace bl::StringInternal;

  init_sso(self);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_string_init_move(BLStringCore* self, BLStringCore* other) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_string());

  self->_d = other->_d;
  init_sso(other);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_string_init_weak(BLStringCore* self, const BLStringCore* other) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_string());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_string_init_with_data(BLStringCore* self, const char* str, size_t size) noexcept {
  using namespace bl::StringInternal;

  if (size == SIZE_MAX)
    size = strlen(str);

  BLResult result = init_string_and_copy(self, size, str, size);
  if (result != BL_SUCCESS)
    init_sso(self);
  return result;
}

BL_API_IMPL BLResult bl_string_destroy(BLStringCore* self) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  return release_instance(self);
}

// bl::String - API - Common Functionality
// =======================================

BL_API_IMPL BLResult bl_string_reset(BLStringCore* self) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  release_instance(self);
  init_sso(self);

  return BL_SUCCESS;
}

// bl::String - API - Accessors
// ============================

BL_API_IMPL const char* bl_string_get_data(const BLStringCore* self) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  return get_data(self);
}

BL_API_IMPL size_t bl_string_get_size(const BLStringCore* self) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  return get_size(self);
}

BL_API_IMPL size_t bl_string_get_capacity(const BLStringCore* self) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  return get_capacity(self);
}

// bl::String - API - Data Manipulation - Storage Management
// =========================================================

BL_API_IMPL BLResult bl_string_clear(BLStringCore* self) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  if (self->_d.sso()) {
    size_t size = get_sso_size(self);

    if (size) {
      clear_sso_data(self);
      set_sso_size(self, 0);
    }

    return BL_SUCCESS;
  }
  else {
    BLStringImpl* self_impl = get_impl(self);

    if (!is_impl_mutable(self_impl)) {
      release_instance(self);
      init_sso(self);

      return BL_SUCCESS;
    }

    if (self_impl->size) {
      self_impl->size = 0;
      self_impl->data()[0] = '\0';
    }

    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_string_shrink(BLStringCore* self) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  if (self->_d.sso())
    return BL_SUCCESS;

  BLStringImpl* self_impl = get_impl(self);
  if (!bl::ObjectInternal::is_impl_ref_counted(self_impl))
    return BL_SUCCESS;

  const char* data = self_impl->data();
  size_t size = self_impl->size;

  if (size <= BLString::kSSOCapacity || size + BL_OBJECT_IMPL_ALIGNMENT <= self_impl->capacity) {
    // Use static storage if the string is small enough to hold the data. Only try to reduce the capacity if the string
    // is dynamic and reallocating the storage would save at least a single cache line, otherwise we would end up most
    // likely with a similar size of the Impl.
    BLStringCore tmp;
    BL_PROPAGATE(init_string_and_copy(&tmp, size, data, size));
    return replace_instance(self, &tmp);
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_string_reserve(BLStringCore* self, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  UnpackedData u = unpack_data(self);
  size_t immutable_mask = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((n | immutable_mask) <= u.capacity)
    return BL_SUCCESS;

  BLStringCore newO;
  BL_PROPAGATE(init_dynamic(&newO, impl_size_from_capacity(bl_max(u.size, n)), u.size));

  char* dst = get_impl(&newO)->data();
  memcpy(dst, u.data, u.size);
  return replace_instance(self, &newO);
}

BL_API_IMPL BLResult bl_string_resize(BLStringCore* self, size_t n, char fill) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  UnpackedData u = unpack_data(self);
  if (n <= u.size) {
    if (n == u.size)
      return BL_SUCCESS;

    // If `n` is lesser than the current `size` it's a truncation.
    if (!is_instance_mutable(self)) {
      BLStringCore newO;
      BL_PROPAGATE(init_string_and_copy(&newO, n, u.data, n));
      return replace_instance(self, &newO);
    }
    else {
      if (self->_d.sso()) {
        // Clears all unused bytes in the SSO storage.
        bl::MemOps::fill_inline_t<char>(u.data + n, '\0', u.size - n);
        set_sso_size(self, n);
        return BL_SUCCESS;
      }
      else {
        BLStringImpl* impl = get_impl(self);
        impl->size = n;
        impl->data()[n] = '\0';
        return BL_SUCCESS;
      }
    }
  }
  else {
    n -= u.size;
    char* dst;
    BL_PROPAGATE(bl_string_modify_op(self, BL_MODIFY_OP_APPEND_FIT, n, &dst));

    memset(dst, int((unsigned char)fill), n);
    return BL_SUCCESS;
  }
}

// bl::String - API - Data Manipulation - Modify Operations
// ========================================================

BL_API_IMPL BLResult bl_string_make_mutable(BLStringCore* self, char** data_out) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  if (self->_d.sso()) {
    *data_out = self->_d.char_data;
    return BL_SUCCESS;
  }

  BLStringImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    *data_out = self_impl->data();
    return BL_SUCCESS;
  }

  // Temporarily store it here as we need to create a new instance on 'self' to be able to return `data_out` ptr.
  BLStringCore tmp = *self;
  size_t size = self_impl->size;
  BL_PROPAGATE(init_string_and_copy(self, size, self_impl->data(), size));

  *data_out = get_data(self);
  return release_instance(&tmp);
}

BL_API_IMPL BLResult bl_string_modify_op(BLStringCore* self, BLModifyOp op, size_t n, char** data_out) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  UnpackedData u = unpack_data(self);
  size_t index = bl_modify_op_is_append(op) ? u.size : size_t(0);
  size_t size_after = bl::IntOps::uadd_saturate(index, n);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((size_after | immutable_msk) > u.capacity) {
    BLStringCore tmp = *self;
    char* dst = nullptr;
    const char* src = get_data(&tmp);

    if (size_after <= BLString::kSSOCapacity && !bl_modify_op_does_grow(op)) {
      init_sso(self, size_after);
      dst = self->_d.char_data;
    }
    else {
      *data_out = nullptr;

      if (BL_UNLIKELY(size_after > get_maximum_size()))
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

      BLObjectImplSize impl_size = expand_impl_size_with_modify_op(impl_size_from_capacity(size_after), op);
      BL_PROPAGATE(init_dynamic(self, impl_size, size_after));

      dst = get_impl(self)->data();
    }

    *data_out = dst + index;
    memcpy(dst, src, index);
    dst[size_after] = '\0';

    return release_instance(&tmp);
  }

  *data_out = u.data + index;
  u.data[size_after] = '\0';

  if (self->_d.sso()) {
    set_sso_size(self, size_after);
    if (bl_modify_op_is_assign(op))
      clear_sso_data(self);
    return BL_SUCCESS;
  }
  else {
    get_impl(self)->size = size_after;
    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_string_insert_op(BLStringCore* self, size_t index, size_t n, char** data_out) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  UnpackedData u = unpack_data(self);
  size_t size_after = bl::IntOps::uadd_saturate(u.size, n);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_instance_mutable(self));

  if ((size_after | immutable_msk) > u.capacity) {
    *data_out = nullptr;

    if (BL_UNLIKELY(size_after > get_maximum_size()))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    BLStringCore newO;
    BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(size_after));
    BL_PROPAGATE(init_dynamic(&newO, impl_size, size_after));

    char* dst = get_impl(&newO)->data();
    memcpy(dst, u.data, index);
    memcpy(dst + index + n, u.data + index, u.size - index);

    *data_out = dst + index;
    return replace_instance(self, &newO);
  }
  else {
    set_size(self, size_after);
    memmove(u.data + index + n, u.data + index, u.size - index);
    u.data[size_after] = '\0';

    *data_out = u.data + index;
    return BL_SUCCESS;
  }
}

// bl::String - API - Data Manipulation - Assignment
// =================================================

BL_API_IMPL BLResult bl_string_assign_move(BLStringCore* self, BLStringCore* other) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(self->_d.is_string());
  BL_ASSERT(other->_d.is_string());

  BLStringCore tmp = *other;
  init_sso(other);
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_string_assign_weak(BLStringCore* self, const BLStringCore* other) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(self->_d.is_string());
  BL_ASSERT(other->_d.is_string());

  bl::ObjectInternal::retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_string_assign_deep(BLStringCore* self, const BLStringCore* other) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(self->_d.is_string());
  BL_ASSERT(other->_d.is_string());

  return modify_and_copy(self, BL_MODIFY_OP_ASSIGN_FIT, get_data(other), get_size(other));
}

BL_API_IMPL BLResult bl_string_assign_data(BLStringCore* self, const char* str, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  if (n == SIZE_MAX)
    n = strlen(str);

  return modify_and_copy(self, BL_MODIFY_OP_ASSIGN_FIT, str, n);
}

// bl::String - API - Data Manipulation - ApplyOp
// ==============================================

BL_API_IMPL BLResult bl_string_apply_op_char(BLStringCore* self, BLModifyOp op, char c, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  char* dst;
  BL_PROPAGATE(bl_string_modify_op(self, op, n, &dst));

  memset(dst, int((unsigned char)c), n);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_string_apply_op_data(BLStringCore* self, BLModifyOp op, const char* str, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  if (n == SIZE_MAX)
    n = strlen(str);

  return modify_and_copy(self, op, str, n);
}

BL_API_IMPL BLResult bl_string_apply_op_string(BLStringCore* self, BLModifyOp op, const BLStringCore* other) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  return modify_and_copy(self, op, get_data(other), get_size(other));
}

BL_API_IMPL BLResult bl_string_apply_op_format_v(BLStringCore* self, BLModifyOp op, const char* fmt, va_list ap) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  UnpackedData u = unpack_data(self);
  size_t index = bl_modify_op_is_append(op) ? u.size : size_t(0);
  size_t remaining = u.capacity - index;
  size_t mutable_msk = bl::IntOps::bool_as_mask<size_t>(is_instance_mutable(self));

  char buf[1024];
  int fmt_result;
  size_t output_size;

  va_list ap_copy;
  va_copy(ap_copy, ap);

  if ((remaining & mutable_msk) >= 64) {
    // vsnprintf() expects null terminator to be included in the size of the buffer.
    char* dst = u.data;
    fmt_result = vsnprintf(dst + index, remaining + 1, fmt, ap);

    if (BL_UNLIKELY(fmt_result < 0))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    output_size = size_t(unsigned(fmt_result));
    if (BL_LIKELY(output_size <= remaining)) {
      // `vsnprintf` must write a null terminator, verify it's true.
      BL_ASSERT(dst[index + output_size] == '\0');

      set_size(self, index + output_size);
      return BL_SUCCESS;
    }
  }
  else {
    fmt_result = vsnprintf(buf, BL_ARRAY_SIZE(buf), fmt, ap);
    if (BL_UNLIKELY(fmt_result < 0))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    // If the `output_size` is less than our buffer size then we are fine and the formatted text is already in the
    // buffer. Since `vsnprintf` doesn't include null-terminator in the returned size we cannot use '<=' as that
    // would mean that the last character written by `vsnprintf` was truncated.
    output_size = size_t(unsigned(fmt_result));
    if (BL_LIKELY(output_size < BL_ARRAY_SIZE(buf)))
      return bl_string_apply_op_data(self, op, buf, output_size);
  }

  // If we are here it means that the string is either not large enough to hold the formatted text or it's not
  // mutable. In both cases we have to allocate a new buffer and call `vsnprintf` again.
  size_t size_after = bl::IntOps::uadd_saturate(index, output_size);
  if (BL_UNLIKELY(size_after > get_maximum_size()))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  BLStringCore newO;
  BLObjectImplSize impl_size = expand_impl_size_with_modify_op(impl_size_from_capacity(size_after), op);
  BL_PROPAGATE(init_dynamic(&newO, impl_size, size_after));

  char* dst = get_impl(&newO)->data();

  // This should always match. If it doesn't then it means that some other thread must have changed some value where
  // `ap_copy` points and it caused `vsnprintf` to format a different string. If this happens we fail as there is no
  // reason to try again...
  fmt_result = vsnprintf(dst + index, output_size + 1, fmt, ap_copy);

  if (BL_UNLIKELY(size_t(unsigned(fmt_result)) != output_size)) {
    release_instance(&newO);
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  memcpy(dst, u.data, index);
  return replace_instance(self, &newO);
}

BL_API_IMPL BLResult bl_string_apply_op_format(BLStringCore* self, BLModifyOp op, const char* fmt, ...) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  BLResult result;
  va_list ap;

  va_start(ap, fmt);
  result = bl_string_apply_op_format_v(self, op, fmt, ap);
  va_end(ap);

  return result;
}

// bl::String - API - Data Manipulation - Insert
// =============================================

BL_API_IMPL BLResult bl_string_insert_char(BLStringCore* self, size_t index, char c, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  char* dst;
  BL_PROPAGATE(bl_string_insert_op(self, index, n, &dst));

  memset(dst, int((unsigned char)c), n);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_string_insert_data(BLStringCore* self, size_t index, const char* str, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  if (n == SIZE_MAX)
    n = strlen(str);

  return insert_and_copy(self, index, str, n);
}

BL_API_IMPL BLResult bl_string_insert_string(BLStringCore* self, size_t index, const BLStringCore* other) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(self->_d.is_string());
  BL_ASSERT(other->_d.is_string());

  if (self != other) {
    return insert_and_copy(self, index, get_data(other), get_size(other));
  }
  else {
    BLStringCore copy(*other);
    return insert_and_copy(self, index, get_data(&copy), get_size(&copy));
  }
}

// bl::String - API - Data Manipulation - Remove
// =============================================

BL_API_IMPL BLResult bl_string_remove_index(BLStringCore* self, size_t index) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  return bl_string_remove_range(self, index, index + 1);
}

BL_API_IMPL BLResult bl_string_remove_range(BLStringCore* self, size_t r_start, size_t r_end) noexcept {
  BL_ASSERT(self->_d.is_string());
  using namespace bl::StringInternal;

  size_t size = get_size(self);
  size_t end = bl_min(r_end, size);
  size_t index = bl_min(r_start, end);

  size_t n = end - index;
  size_t size_after = size - n;

  if (!n)
    return BL_SUCCESS;

  if (self->_d.sso()) {
    char* data = self->_d.char_data;
    bl::MemOps::copy_small(data + index, data + index + n, size - end);
    bl::MemOps::fill_small_t(data + size_after, char(0), BLString::kSSOCapacity - size_after);

    set_sso_size(self, size_after);
    return BL_SUCCESS;
  }

  BLStringImpl* self_impl = get_impl(self);
  if (is_impl_mutable(self_impl)) {
    // Copy one more byte that acts as a NULL terminator.
    char* data = self_impl->data();
    memmove(data + index, data + index + n, size - end + 1);

    self_impl->size = size_after;
    return BL_SUCCESS;
  }

  BLStringCore tmp = *self;
  char* dst;
  BL_PROPAGATE(init_string(self, size_after, size_after, &dst));

  const char* src = get_data(&tmp);
  memcpy(dst, src, index);
  memcpy(dst + index, src + end, size - end);

  return release_instance(&tmp);
}

// bl::String - API - Equality / Comparison
// ========================================

BL_API_IMPL bool bl_string_equals(const BLStringCore* a, const BLStringCore* b) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(a->_d.is_string());
  BL_ASSERT(b->_d.is_string());

  UnpackedData au = unpack_data(a);
  UnpackedData bu = unpack_data(b);

  if (au.size != bu.size)
    return false;

  return memcmp(au.data, bu.data, au.size) == 0;
}

BL_API_IMPL bool bl_string_equals_data(const BLStringCore* self, const char* str, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  const char* a_data = get_data(self);
  const char* b_data = str;

  size_t size = get_size(self);
  if (n == SIZE_MAX) {
    // Null terminated, we don't know the size of `str`.
    size_t i;
    for (i = 0; i < size; i++)
      if ((a_data[i] != b_data[i]) | (b_data[i] == 0))
        return false;
    return b_data[i] == 0;
  }
  else {
    if (size != n)
      return false;
    return memcmp(a_data, b_data, size) == 0;
  }
}

BL_API_IMPL int bl_string_compare(const BLStringCore* a, const BLStringCore* b) noexcept {
  using namespace bl::StringInternal;

  BL_ASSERT(a->_d.is_string());
  BL_ASSERT(b->_d.is_string());

  UnpackedData au = unpack_data(a);
  UnpackedData bu = unpack_data(b);

  size_t min_size = bl_min(au.size, bu.size);
  int c = memcmp(au.data, bu.data, min_size);

  if (c)
    return c;

  return au.size < bu.size ? -1 : int(au.size > bu.size);
}

BL_API_IMPL int bl_string_compare_data(const BLStringCore* self, const char* str, size_t n) noexcept {
  using namespace bl::StringInternal;
  BL_ASSERT(self->_d.is_string());

  UnpackedData u = unpack_data(self);
  size_t a_size = u.size;

  const char* a_data = u.data;
  const char* b_data = str;

  if (n == SIZE_MAX) {
    // Null terminated: We don't know the size of `str`, thus we cannot use strcmp() as BLString content can be
    // arbitrary, so strcmp() won't work if the string holds zeros (aka null terminators).
    size_t i;

    for (i = 0; i < a_size; i++) {
      int a = uint8_t(a_data[i]);
      int b = uint8_t(b_data[i]);

      int c = a - b;

      // If we found a null terminator in 'b' it means that so far the strings were equal, but now we are at the end
      // of 'b', however, there is still some content in 'a'. This would mean that `a > b` like "abc?" > "abc".
      if (b == 0)
        c = 1;

      if (c)
        return c;
    }

    // We are at the end of 'a'. If this is also the end of 'b' then these strings are equal and we return zero. If
    // 'b' doesn't point to a null terminator then `a < b`.
    return -int(b_data[i] != 0);
  }
  else {
    size_t b_size = n;
    size_t min_size = bl_min(a_size, b_size);

    int c = memcmp(a_data, b_data, min_size);
    if (c)
      return c;

    return a_size < b_size ? -1 : int(a_size > b_size);
  }
}

// bl::String - Runtime Registration
// =================================

void bl_string_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);
  bl::StringInternal::init_sso(static_cast<BLStringCore*>(&bl_object_defaults[BL_OBJECT_TYPE_STRING]));
}
