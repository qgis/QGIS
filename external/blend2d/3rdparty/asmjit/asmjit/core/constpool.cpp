// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/constpool.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// ConstPool - Construction & Destruction
// ======================================

ConstPool::ConstPool(Arena& arena) noexcept : _arena(arena) {
  reset();
}

ConstPool::~ConstPool() noexcept {}

// ConstPool - Reset
// =================

void ConstPool::reset() noexcept {
  size_t data_size = 1;

  for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_tree); i++) {
    _tree[i].reset();
    _tree[i].set_data_size(data_size);
    _gaps[i] = nullptr;
    data_size <<= 1;
  }

  _gap_pool = nullptr;
  _size = 0;
  _alignment = 0;
  _min_item_size = 0;
}

// ConstPool - Operations
// ======================

static inline ConstPool::Gap* ConstPool_allocGap(ConstPool* self) noexcept {
  ConstPool::Gap* gap = self->_gap_pool;

  if (!gap) {
    return self->_arena.alloc_oneshot<ConstPool::Gap>();
  }

  self->_gap_pool = gap->_next;
  return gap;
}

static inline void ConstPool_freeGap(ConstPool* self, ConstPool::Gap* gap) noexcept {
  gap->_next = self->_gap_pool;
  self->_gap_pool = gap;
}

static void ConstPool_addGap(ConstPool* self, size_t offset, size_t size) noexcept {
  ASMJIT_ASSERT(size > 0);

  while (size > 0) {
    size_t gap_index;
    size_t gap_size;

    if (size >= 32 && Support::is_aligned<size_t>(offset, 32)) {
      gap_index = ConstPool::kIndex32;
      gap_size = 32;
    }
    else if (size >= 16 && Support::is_aligned<size_t>(offset, 16)) {
      gap_index = ConstPool::kIndex16;
      gap_size = 16;
    }
    else if (size >= 8 && Support::is_aligned<size_t>(offset, 8)) {
      gap_index = ConstPool::kIndex8;
      gap_size = 8;
    }
    else if (size >= 4 && Support::is_aligned<size_t>(offset, 4)) {
      gap_index = ConstPool::kIndex4;
      gap_size = 4;
    }
    else if (size >= 2 && Support::is_aligned<size_t>(offset, 2)) {
      gap_index = ConstPool::kIndex2;
      gap_size = 2;
    }
    else {
      gap_index = ConstPool::kIndex1;
      gap_size = 1;
    }

    // We don't have to check for errors here, if this failed nothing really happened (just the gap won't be
    // visible) and it will fail again at place where the same check would generate \ref Error::kOutOfMemory error.
    ConstPool::Gap* gap = ConstPool_allocGap(self);
    if (!gap) {
      return;
    }

    gap->_next = self->_gaps[gap_index];
    self->_gaps[gap_index] = gap;

    gap->_offset = offset;
    gap->_size = gap_size;

    offset += gap_size;
    size -= gap_size;
  }
}

Error ConstPool::add(const void* data, size_t size, Out<size_t> offset_out) noexcept {
  constexpr size_t kMaxSize = size_t(1) << (kIndexCount - 1);

  // Avoid sizes outside of the supported range.
  if (ASMJIT_UNLIKELY(size == 0 || size > kMaxSize)) {
    return make_error(Error::kInvalidArgument);
  }

  size_t tree_index = Support::ctz(size);

  // Avoid sizes, which are not aligned to power of 2.
  if (ASMJIT_UNLIKELY((size_t(1) << tree_index) != size)) {
    return make_error(Error::kInvalidArgument);
  }

  ConstPool::Node* node = _tree[tree_index].get(data);
  if (node) {
    offset_out = node->_offset;
    return Error::kOk;
  }

  // Before incrementing the current offset try if there is a gap that can be used for the requested data.
  size_t offset = ~size_t(0);
  size_t gap_index = tree_index;

  while (gap_index != kIndexCount - 1) {
    ConstPool::Gap* gap = _gaps[tree_index];

    // Check if there is a gap.
    if (gap) {
      size_t gap_offset = gap->_offset;
      size_t gap_size = gap->_size;

      // Destroy the gap for now.
      _gaps[tree_index] = gap->_next;
      ConstPool_freeGap(this, gap);

      offset = gap_offset;
      ASMJIT_ASSERT(Support::is_aligned<size_t>(offset, size));

      gap_size -= size;
      if (gap_size > 0) {
        ConstPool_addGap(this, gap_offset, gap_size);
      }
    }

    gap_index++;
  }

  if (offset == ~size_t(0)) {
    // Get how many bytes have to be skipped so the address is aligned accordingly to the 'size'.
    size_t diff = Support::align_up_diff<size_t>(_size, size);

    if (diff != 0) {
      ConstPool_addGap(this, _size, diff);
      _size += diff;
    }

    offset = _size;
    _size += size;
  }

  // Add the initial node to the right index.
  node = ConstPool::Tree::new_node_t(_arena, data, size, offset, false);
  if (ASMJIT_UNLIKELY(!node)) {
    return make_error(Error::kOutOfMemory);
  }

  _tree[tree_index].insert(node);
  _alignment = Support::max<size_t>(_alignment, size);

  offset_out = offset;

  // Now create a bunch of shared constants that are based on the data pattern. We stop at size 4,
  // it probably doesn't make sense to split constants down to 1 byte.
  size_t p_count = 1;
  size_t smaller_size = size;

  while (smaller_size > 4) {
    p_count <<= 1;
    smaller_size >>= 1;

    ASMJIT_ASSERT(tree_index != 0);
    tree_index--;

    const uint8_t* data_ptr = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < p_count; i++, data_ptr += smaller_size) {
      node = _tree[tree_index].get(data_ptr);
      if (node) {
        continue;
      }

      node = ConstPool::Tree::new_node_t(_arena, data_ptr, smaller_size, offset + (i * smaller_size), true);
      _tree[tree_index].insert(node);
    }
  }

  _min_item_size = !_min_item_size ? size : Support::min(_min_item_size, size);
  return Error::kOk;
}

// ConstPool - Reset
// =================

struct ConstPoolFill {
  inline ConstPoolFill(uint8_t* dst, size_t data_size) noexcept :
    _dst(dst),
    _data_size(data_size) {}

  inline void operator()(const ConstPool::Node* node) noexcept {
    if (!node->_shared) {
      memcpy(_dst + node->_offset, node->data(), _data_size);
    }
  }

  uint8_t* _dst;
  size_t _data_size;
};

void ConstPool::fill(void* dst) const noexcept {
  // Clears possible gaps, asmjit should never emit garbage to the output.
  memset(dst, 0, _size);

  ConstPoolFill filler(static_cast<uint8_t*>(dst), 1);
  for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_tree); i++) {
    _tree[i].for_each(filler);
    filler._data_size <<= 1;
  }
}

// ConstPool - Tests
// =================

#if defined(ASMJIT_TEST)
UNIT(const_pool) {
  Arena arena(32u * 1024u);
  ConstPool pool(arena);

  uint32_t i;
  uint32_t kCount = BrokenAPI::has_arg("--quick") ? 1000 : 1000000;

  INFO("Adding %u constants to the pool", kCount);
  {
    size_t prev_offset;
    size_t cur_offset;
    uint64_t c = 0x0101010101010101u;

    EXPECT_EQ(pool.add(&c, 8, Out(prev_offset)), Error::kOk);
    EXPECT_EQ(prev_offset, 0u);

    for (i = 1; i < kCount; i++) {
      c++;
      EXPECT_EQ(pool.add(&c, 8, Out(cur_offset)), Error::kOk);
      EXPECT_EQ(prev_offset + 8, cur_offset);
      EXPECT_EQ(pool.size(), (i + 1) * 8);
      prev_offset = cur_offset;
    }

    EXPECT_EQ(pool.alignment(), 8u);
  }

  INFO("Retrieving %u constants from the pool", kCount);
  {
    uint64_t c = 0x0101010101010101u;

    for (i = 0; i < kCount; i++) {
      size_t offset;
      EXPECT_EQ(pool.add(&c, 8, Out(offset)), Error::kOk);
      EXPECT_EQ(offset, i * 8);
      c++;
    }
  }

  INFO("Checking if the constants were split into 4-byte patterns");
  {
    uint32_t c = 0x01010101u;
    size_t offset;

    EXPECT_EQ(pool.add(&c, 4, Out(offset)), Error::kOk);
    EXPECT_EQ(offset, 0u);

    // NOTE: We have to adjust the offset to successfully test this on big endian architectures.
    size_t base_offset = size_t(Support::ByteOrder::kNative == Support::ByteOrder::kBE ? 4 : 0);

    for (i = 1; i < kCount; i++) {
      c++;
      EXPECT_EQ(pool.add(&c, 4, Out(offset)), Error::kOk);
      EXPECT_EQ(offset, base_offset + i * 8);
    }
  }

  INFO("Adding 2 byte constant to misalign the current offset");
  {
    uint16_t c = 0xFFFF;
    size_t offset;

    EXPECT_EQ(pool.add(&c, 2, Out(offset)), Error::kOk);
    EXPECT_EQ(offset, kCount * 8);
    EXPECT_EQ(pool.alignment(), 8u);
  }

  INFO("Adding 8 byte constant to check if pool gets aligned again");
  {
    uint64_t c = 0xFFFFFFFFFFFFFFFFu;
    size_t offset;

    EXPECT_EQ(pool.add(&c, 8, Out(offset)), Error::kOk);
    EXPECT_EQ(offset, kCount * 8 + 8u);
  }

  INFO("Adding 2 byte constant to verify the gap is filled");
  {
    uint16_t c = 0xFFFE;
    size_t offset;

    EXPECT_EQ(pool.add(&c, 2, Out(offset)), Error::kOk);
    EXPECT_EQ(offset, kCount * 8 + 2);
    EXPECT_EQ(pool.alignment(), 8u);
  }

  INFO("Checking reset functionality");
  {
    pool.reset();
    arena.reset();

    EXPECT_EQ(pool.size(), 0u);
    EXPECT_EQ(pool.alignment(), 0u);
  }

  INFO("Checking pool alignment when combined constants are added");
  {
    uint8_t bytes[32] = { 0 };
    size_t offset;

    pool.add(bytes, 1, Out(offset));
    EXPECT_EQ(pool.size(), 1u);
    EXPECT_EQ(pool.alignment(), 1u);
    EXPECT_EQ(offset, 0u);

    pool.add(bytes, 2, Out(offset));
    EXPECT_EQ(pool.size(), 4u);
    EXPECT_EQ(pool.alignment(), 2u);
    EXPECT_EQ(offset, 2u);

    pool.add(bytes, 4, Out(offset));
    EXPECT_EQ(pool.size(), 8u);
    EXPECT_EQ(pool.alignment(), 4u);
    EXPECT_EQ(offset, 4u);

    pool.add(bytes, 4, Out(offset));
    EXPECT_EQ(pool.size(), 8u);
    EXPECT_EQ(pool.alignment(), 4u);
    EXPECT_EQ(offset, 4u);

    pool.add(bytes, 32, Out(offset));
    EXPECT_EQ(pool.size(), 64u);
    EXPECT_EQ(pool.alignment(), 32u);
    EXPECT_EQ(offset, 32u);
  }
}
#endif

ASMJIT_END_NAMESPACE
