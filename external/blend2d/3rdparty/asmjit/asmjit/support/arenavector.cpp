// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/support/support.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenavector.h>

ASMJIT_BEGIN_NAMESPACE

// ArenaVector - Item Size Helpers
// ===============================

template<typename ResultType, bool IsPowerOf2>
static ASMJIT_INLINE_NODEBUG ResultType byte_size_from_item_count(size_t item_count, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  if constexpr (IsPowerOf2) {
    return ResultType(item_count) << item_size.n;
  }
  else {
    return ResultType(item_count) * item_size.n;
  }
}

template<bool IsPowerOf2>
static ASMJIT_INLINE_NODEBUG size_t item_count_from_byte_size(size_t byte_size, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  if constexpr (IsPowerOf2) {
    return byte_size >> item_size.n;
  }
  else {
    return byte_size / item_size.n;
  }
}

// ArenaVector - Memory Management
// ===============================

// Rule based growing strategy - 32 bytes, 128 bytes, 512 bytes, and then grow exponentially until `kGrowThreshold`
// is reached.
static constexpr uint8_t ArenaVector_grow_rule(uint8_t log2_size) noexcept {
  return log2_size < 1u ? uint8_t(0) :
         log2_size < 2u ? uint8_t(2) :
         log2_size < 4u ? uint8_t(4) :
         log2_size < 6u ? uint8_t(6) :
         log2_size < 8u ? uint8_t(8) : uint8_t(log2_size);
}

// The table is never used fully, only indexes up to `ctz(Support::kGrowThreshold) + 1`.
static constexpr uint8_t ArenaVector_grow_table[32] = {
  ArenaVector_grow_rule( 0), ArenaVector_grow_rule( 1), ArenaVector_grow_rule( 2), ArenaVector_grow_rule( 3),
  ArenaVector_grow_rule( 4), ArenaVector_grow_rule( 5), ArenaVector_grow_rule( 6), ArenaVector_grow_rule( 7),
  ArenaVector_grow_rule( 8), ArenaVector_grow_rule( 9), ArenaVector_grow_rule(10), ArenaVector_grow_rule(11),
  ArenaVector_grow_rule(12), ArenaVector_grow_rule(13), ArenaVector_grow_rule(14), ArenaVector_grow_rule(15),
  ArenaVector_grow_rule(16), ArenaVector_grow_rule(17), ArenaVector_grow_rule(18), ArenaVector_grow_rule(19),
  ArenaVector_grow_rule(20), ArenaVector_grow_rule(21), ArenaVector_grow_rule(22), ArenaVector_grow_rule(23),
  ArenaVector_grow_rule(24), ArenaVector_grow_rule(25), ArenaVector_grow_rule(26), ArenaVector_grow_rule(27),
  ArenaVector_grow_rule(28), ArenaVector_grow_rule(29), ArenaVector_grow_rule(30), ArenaVector_grow_rule(31)
};

static ASMJIT_INLINE size_t ArenaVector_expand_byte_size(size_t byte_size) noexcept {
  ASMJIT_ASSERT(byte_size > 0u);

  if (ASMJIT_LIKELY(byte_size <= Globals::kGrowThreshold)) {
    uint32_t grow_table_idx = Support::bit_size_of<size_t> - Support::clz((byte_size - 1u) | 1u);
    uint32_t grow_log2_size = ArenaVector_grow_table[grow_table_idx];

    return size_t(1) << grow_log2_size;
  }
  else {
    return Support::align_up(size_t(byte_size) + 1u, Globals::kGrowThreshold);
  }
}

template<bool IsPowerOf2>
static ASMJIT_NOINLINE Error ArenaVector_reserve_with_byte_size(ArenaVectorBase& self, Arena& arena, size_t byte_size, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  size_t allocated_size;
  uint8_t* new_data = static_cast<uint8_t*>(arena.alloc_reusable(byte_size, Out(allocated_size)));

  if (ASMJIT_UNLIKELY(!new_data)) {
    return make_error(Error::kOutOfMemory);
  }

  size_t allocated_capacity = item_count_from_byte_size(allocated_size, item_size);

  void* old_data = self._data;
  uint32_t size = self._size;

  if (old_data) {
    memcpy(new_data, old_data, byte_size_from_item_count<size_t>(size, item_size));
    arena.free_reusable(old_data, byte_size_from_item_count<size_t>(self._capacity, item_size));
  }

  self._data = new_data;
  self._capacity = uint32_t(allocated_capacity);

  return Error::kOk;
}

static ASMJIT_INLINE bool ArenaVector_is_valid_size(size_t size) noexcept {
  if constexpr (sizeof(size_t) < 8u) {
    // 32-bit machine - `uint32_t` is the same as `size_t` - there is no need to do any checks
    // as it's impossible to end up having a container, which data uses the whole address space.
    return true;
  }
  else {
    // 64-bit machine - since we store size and capacity as `uint32_t`, we have to check whether
    // the `size_t` argument actually fits `uint32_t`.
    return size < size_t(0xFFFFFFFFu);
  }
}

static ASMJIT_INLINE bool ArenaVector_check_byte_size(uint64_t byte_size) noexcept {
  if constexpr (sizeof(size_t) < 8u) {
    // 32-bit machine.
    return byte_size <= 0x80000000u;
  }
  else {
    return true;
  }
}

template<bool IsPowerOf2>
static ASMJIT_INLINE Error ArenaVector_reserve_fit(ArenaVectorBase& self, Arena& arena, size_t item_count, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  size_t capacity = self._capacity;
  size_t capacity_masked = capacity | Support::bool_as_mask<size_t>(!ArenaVector_is_valid_size(item_count));
  uint64_t byte_size = byte_size_from_item_count<uint64_t>(item_count, item_size);

  if (ASMJIT_UNLIKELY(Support::bool_or(capacity_masked >= item_count, !ArenaVector_check_byte_size(byte_size)))) {
    return capacity >= item_count ? Error::kOk : make_error(Error::kOutOfMemory);
  }

  return ArenaVector_reserve_with_byte_size(self, arena, size_t(byte_size), item_size);
}

template<bool IsPowerOf2>
static ASMJIT_INLINE Error ArenaVector_reserve_grow(ArenaVectorBase& self, Arena& arena, size_t item_count, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  size_t capacity = self._capacity;
  size_t capacity_masked = capacity | Support::bool_as_mask<size_t>(!ArenaVector_is_valid_size(item_count));
  uint64_t byte_size = byte_size_from_item_count<uint64_t>(item_count, item_size);

  if (ASMJIT_UNLIKELY(Support::bool_or(capacity_masked >= item_count, !ArenaVector_check_byte_size(byte_size)))) {
    return capacity >= item_count ? Error::kOk : make_error(Error::kOutOfMemory);
  }

  size_t expanded_byte_size = ArenaVector_expand_byte_size(size_t(byte_size));
  return ArenaVector_reserve_with_byte_size(self, arena, expanded_byte_size, item_size);
}

template<bool IsPowerOf2>
static ASMJIT_INLINE Error ArenaVector_grow(ArenaVectorBase& self, Arena& arena, size_t n, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  Support::FastUInt8 of {};
  size_t after = Support::add_overflow<size_t>(self._size, n, &of);

  if (ASMJIT_UNLIKELY(of)) {
    return make_error(Error::kOutOfMemory);
  }

  return ArenaVector_reserve_grow(self, arena, after, item_size);
}

template<bool IsPowerOf2>
static ASMJIT_INLINE Error ArenaVector_resize_fit(ArenaVectorBase& self, Arena& arena, size_t n, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  size_t size = self._size;
  size_t capacity = self._capacity;

  if (capacity < n) {
    ASMJIT_PROPAGATE(ArenaVector_reserve_fit(self, arena, n, item_size));
  }

  if (size < n) {
    memset(static_cast<uint8_t*>(self._data) + byte_size_from_item_count<size_t>(size, item_size), 0, byte_size_from_item_count<size_t>(n - size, item_size));
  }

  self._size = uint32_t(n);
  return Error::kOk;
}

template<bool IsPowerOf2>
static ASMJIT_INLINE Error ArenaVector_resize_grow(ArenaVectorBase& self, Arena& arena, size_t n, ArenaVectorBase::ItemSize<IsPowerOf2> item_size) noexcept {
  size_t size = self._size;
  size_t capacity = self._capacity;

  if (capacity < n) {
    ASMJIT_PROPAGATE(ArenaVector_reserve_grow(self, arena, n, item_size));
  }

  if (size < n) {
    memset(static_cast<uint8_t*>(self._data) + byte_size_from_item_count<size_t>(size, item_size), 0, byte_size_from_item_count<size_t>(n - size, item_size));
  }

  self._size = uint32_t(n);
  return Error::kOk;
}

// Public API wrappers:
Error ArenaVectorBase::_reserve_fit(Arena& arena, size_t n, ItemSize<false> item_size) noexcept {
  return ArenaVector_reserve_fit(*this, arena, n, item_size);
}

Error ArenaVectorBase::_reserve_fit(Arena& arena, size_t n, ItemSize<true> item_size) noexcept {
  return ArenaVector_reserve_fit(*this, arena, n, item_size);
}

Error ArenaVectorBase::_reserve_grow(Arena& arena, size_t n, ItemSize<false> item_size) noexcept {
  return ArenaVector_reserve_grow(*this, arena, n, item_size);
}

Error ArenaVectorBase::_reserve_grow(Arena& arena, size_t n, ItemSize<true> item_size) noexcept {
  return ArenaVector_reserve_grow(*this, arena, n, item_size);
}

Error ArenaVectorBase::_reserve_additional(Arena& arena, size_t n, ItemSize<false> item_size) noexcept {
  return ArenaVector_grow(*this, arena, n, item_size);
}

Error ArenaVectorBase::_reserve_additional(Arena& arena, size_t n, ItemSize<true> item_size) noexcept {
  return ArenaVector_grow(*this, arena, n, item_size);
}

Error ArenaVectorBase::_resize_fit(Arena& arena, size_t n, ItemSize<false> item_size) noexcept {
  return ArenaVector_resize_fit(*this, arena, n, item_size);
}

Error ArenaVectorBase::_resize_fit(Arena& arena, size_t n, ItemSize<true> item_size) noexcept {
  return ArenaVector_resize_fit(*this, arena, n, item_size);
}

Error ArenaVectorBase::_resize_grow(Arena& arena, size_t n, ItemSize<false> item_size) noexcept {
  return ArenaVector_resize_grow(*this, arena, n, item_size);
}

Error ArenaVectorBase::_resize_grow(Arena& arena, size_t n, ItemSize<true> item_size) noexcept {
  return ArenaVector_resize_grow(*this, arena, n, item_size);
}

// ArenaVector - Tests
// ===================

#if defined(ASMJIT_TEST)
template<typename T>
static void test_arena_vector(Arena& arena, const char* type_name) {
  constexpr uint32_t kMiB = 1024 * 1024;

  size_t i;
  size_t kMax = 100000;

  ArenaVector<T> vec;

  INFO("ArenaVector<%s> basic tests", type_name);
  EXPECT_EQ(vec.append(arena, 0), Error::kOk);
  EXPECT_FALSE(vec.is_empty());
  EXPECT_EQ(vec.size(), 1u);
  EXPECT_GE(vec.capacity(), 1u);
  EXPECT_EQ(vec.index_of(0), size_t(0));
  EXPECT_TRUE(Globals::is_npos(vec.index_of(-11)));

  vec.clear();
  EXPECT_TRUE(vec.is_empty());
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_TRUE(Globals::is_npos(vec.index_of(0)));

  for (i = 0; i < kMax; i++) {
    EXPECT_EQ(vec.append(arena, T(i)), Error::kOk);
  }
  EXPECT_FALSE(vec.is_empty());
  EXPECT_EQ(vec.size(), size_t(kMax));
  EXPECT_EQ(vec.index_of(T(0)), size_t(0));
  EXPECT_EQ(vec.index_of(T(kMax - 1)), uint32_t(kMax - 1));

  EXPECT_EQ(vec.begin()[0], 0);
  EXPECT_EQ(vec.end()[-1], T(kMax - 1));

  INFO("ArenaVector<%s>::operator=(ArenaVector<%s>&&)", type_name, type_name);
  ArenaVector<T> moved_vec(std::move(vec));
  EXPECT_EQ(vec.data(), nullptr);
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_EQ(vec.capacity(), 0u);

  moved_vec.release(arena);

  INFO("ArenaVector<%s>::reserve_grow()", type_name);
  for (uint32_t j = 8; j < 40 / sizeof(T); j += 8) {
    EXPECT_EQ(vec.reserve_grow(arena, j * kMiB), Error::kOk);
    EXPECT_GE(vec.capacity(), j * kMiB);
  }
}

template<typename T>
static void test_arena_vector_capacity(Arena& arena, const char* type_name) {
  ArenaVector<T> vec;

  INFO("ArenaVector<%s> capacity (growing) test", type_name);

  for (size_t i = 0; i < 10000000; i++) {
    size_t old_capacity = vec.capacity();
    EXPECT_EQ(vec.append(arena, T(i)), Error::kOk);

    if (vec.capacity() != old_capacity) {
      INFO("  Increasing capacity from %zu to %zu (vector size=%zu)\n", old_capacity, vec.capacity(), vec.size());
    }
  }
}

UNIT(arena_vector, -1) {
  Arena arena(8192);

  test_arena_vector<int32_t>(arena, "int32_t");
  test_arena_vector_capacity<int32_t>(arena, "int32_t");

  test_arena_vector<int64_t>(arena, "int64_t");
  test_arena_vector_capacity<int64_t>(arena, "int64_t");
}
#endif

ASMJIT_END_NAMESPACE
