// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/random_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenalist_p.h>
#include <blend2d/support/arenatree_p.h>
#include <blend2d/support/traits_p.h>
#include <blend2d/support/zeroallocator_p.h>

// bl::ZeroAllocator - Tests
// =========================

namespace bl {
namespace Tests {

// A helper class to verify that BLZeroAllocator doesn't return addresses that overlap.
class ZeroAllocatorWrapper {
public:
  BL_INLINE ZeroAllocatorWrapper() noexcept {}

  // Address to a memory region of a given size.
  class Range {
  public:
    BL_INLINE Range(uint8_t* addr, size_t size) noexcept
      : addr(addr),
        size(size) {}
    uint8_t* addr;
    size_t size;
  };

  // Based on BLZeroAllocator::Block, serves our purpose well...
  class Record : public ArenaTreeNode<Record>,
                 public Range {
  public:
    BL_INLINE Record(uint8_t* addr, size_t size)
      : ArenaTreeNode<Record>(),
        Range(addr, size) {}

    BL_INLINE bool operator<(const Record& other) const noexcept { return addr < other.addr; }
    BL_INLINE bool operator>(const Record& other) const noexcept { return addr > other.addr; }

    BL_INLINE bool operator<(const uint8_t* key) const noexcept { return addr + size <= key; }
    BL_INLINE bool operator>(const uint8_t* key) const noexcept { return addr > key; }
  };

  ArenaTree<Record> _records;

  void _insert(void* p_, size_t size) noexcept {
    uint8_t* p = static_cast<uint8_t*>(p_);
    uint8_t* pEnd = p + size - 1;

    Record* record = _records.get(p);
    if (record)
      EXPECT_EQ(record, nullptr)
        .message("Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, p, p + size);

    record = _records.get(pEnd);
    if (record)
      EXPECT_EQ(record, nullptr)
        .message("Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, p, p + size);

    void* r_ptr = malloc(sizeof(Record));
    EXPECT_NE(r_ptr, nullptr).message("Out of memory, cannot allocate 'Record'");
    _records.insert(new(BLInternal::PlacementNew{r_ptr}) Record(p, size));
  }

  void _remove(void* p) noexcept {
    Record* record = _records.get(static_cast<uint8_t*>(p));
    EXPECT_NE(record, nullptr).message("Address [%p] doesn't exist\n", p);

    _records.remove(record);
    free(record);
  }

  void* alloc(size_t size) noexcept {
    size_t allocated_size = 0;
    void* p = bl_zero_allocator_alloc(size, &allocated_size);
    EXPECT_NE(p, nullptr).message("BLZeroAllocator failed to allocate '%u' bytes\n", unsigned(size));

    for (size_t i = 0; i < allocated_size; i++) {
      EXPECT_EQ(static_cast<const uint8_t*>(p)[i], 0)
        .message("The returned pointer doesn't point to a zeroed memory %p[%u]\n", p, int(size));
    }

    _insert(p, allocated_size);
    return p;
  }

  size_t get_size_of_ptr(void* p) noexcept {
    Record* record = _records.get(static_cast<uint8_t*>(p));
    return record ? record->size : size_t(0);
  }

  void release(void* p) noexcept {
    size_t size = get_size_of_ptr(p);
    _remove(p);
    bl_zero_allocator_release(p, size);
  }
};

static void bl_zero_allocator_test_shuffle(void** ptr_array, size_t count, BLRandom& prng) noexcept {
  for (size_t i = 0; i < count; ++i)
    BLInternal::swap(ptr_array[i], ptr_array[size_t(prng.next_uint32() % count)]);
}

static void bl_zero_allocator_test_usage() noexcept {
  BLRuntimeResourceInfo info;
  BLRuntime::query_resource_info(&info);

  INFO("  NumBlocks: %9llu"         , (unsigned long long)(info.zm_block_count));
  INFO("  UsedSize : %9llu [Bytes]" , (unsigned long long)(info.zm_used));
  INFO("  Reserved : %9llu [Bytes]" , (unsigned long long)(info.zm_reserved));
  INFO("  Overhead : %9llu [Bytes]" , (unsigned long long)(info.zm_overhead));
}

UNIT(zero_allocator, BL_TEST_GROUP_CORE_UTILITIES) {
  ZeroAllocatorWrapper wrapper;
  BLRandom prng(0);

  size_t i;
  size_t kCount = 50000;

  INFO("Memory alloc/release test - %d allocations", kCount);

  void** ptr_array = (void**)malloc(sizeof(void*) * size_t(kCount));
  EXPECT_NE(ptr_array, nullptr)
    .message("Couldn't allocate '%u' bytes for pointer-array", unsigned(sizeof(void*) * size_t(kCount)));

  INFO("Allocating zeroed memory...");
  for (i = 0; i < kCount; i++)
    ptr_array[i] = wrapper.alloc((prng.next_uint32() % 8000) + 128);
  bl_zero_allocator_test_usage();

  INFO("Releasing zeroed memory...");
  for (i = 0; i < kCount; i++)
    wrapper.release(ptr_array[i]);
  bl_zero_allocator_test_usage();

  INFO("Submitting manual cleanup...");
  BLRuntime::cleanup(BL_RUNTIME_CLEANUP_ZEROED_POOL);
  bl_zero_allocator_test_usage();

  INFO("Allocating zeroed memory...", kCount);
  for (i = 0; i < kCount; i++)
    ptr_array[i] = wrapper.alloc((prng.next_uint32() % 8000) + 128);
  bl_zero_allocator_test_usage();

  INFO("Shuffling...");
  bl_zero_allocator_test_shuffle(ptr_array, unsigned(kCount), prng);

  INFO("Releasing 50%% blocks...");
  for (i = 0; i < kCount / 2; i++)
    wrapper.release(ptr_array[i]);
  bl_zero_allocator_test_usage();

  INFO("Allocating 50%% blocks again...");
  for (i = 0; i < kCount / 2; i++)
    ptr_array[i] = wrapper.alloc((prng.next_uint32() % 8000) + 128);
  bl_zero_allocator_test_usage();

  INFO("Releasing zeroed memory...");
  for (i = 0; i < kCount; i++)
    wrapper.release(ptr_array[i]);
  bl_zero_allocator_test_usage();

  free(ptr_array);
}

} // {Tests}
} // {bl}

#endif // BL_TEST
