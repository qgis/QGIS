// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/intops_p.h>

namespace bl {

// bl::ArenaAllocator - API
// ========================

//! Zero block, used by a default constructed `ArenaAllocator`, which doesn't hold any allocated block. This block
//! must be properly aligned so when arena allocator aligns its current pointer to check for aligned allocation it
//! would not overflow past the end of the block - which is the same as the beginning of the block as it has no size.
struct alignas(64) ArenaAllocatorZeroBlock {
  uint8_t padding[64 - sizeof(ArenaAllocator::Block)];
  ArenaAllocator::Block block;
};

// Zero size block used by `ArenaAllocator` that doesn't have any memory allocated.
// Should be allocated in read-only memory and should never be modified.
static const ArenaAllocatorZeroBlock kArenaAllocatorZeroBlock = { { 0 }, { nullptr, nullptr, 0 } };

static BL_INLINE void ArenaAllocator_assignZeroBlock(ArenaAllocator* self) noexcept {
  ArenaAllocator::Block* block = const_cast<ArenaAllocator::Block*>(&kArenaAllocatorZeroBlock.block);
  self->_ptr = block->data();
  self->_end = block->data();
  self->_block = block;
}

static BL_INLINE void ArenaAllocator_assignBlock(ArenaAllocator* self, ArenaAllocator::Block* block) noexcept {
  size_t alignment = self->block_alignment();
  self->_ptr = IntOps::align_up(block->data(), alignment);
  self->_end = block->data() + block->size;
  self->_block = block;
}

void ArenaAllocator::_init(size_t block_size, size_t block_alignment, void* static_data, size_t static_size) noexcept {
  BL_ASSERT(block_size >= kMinBlockSize);
  BL_ASSERT(block_size <= kMaxBlockSize);
  BL_ASSERT(block_alignment <= 64);

  ArenaAllocator_assignZeroBlock(this);

  size_t block_size_shift = IntOps::bit_size_of<size_t>() - IntOps::clz(block_size);
  size_t block_alignment_shift = IntOps::bit_size_of<size_t>() - IntOps::clz(block_alignment | (size_t(1) << 3));

  _block_alignment_shift = uint8_t(block_alignment_shift);
  _min_block_size_shift = uint8_t(block_size_shift);
  _max_block_size_shift = uint8_t(25); // (1 << 25) Equals 32 MiB blocks (should be enough for all cases)
  _has_static_block = uint8_t(static_data != nullptr);
  _reserved = uint8_t(0u);
  _block_count = size_t(static_data != nullptr);

  // Setup the first [temporary] block, if necessary.
  if (static_data) {
    Block* block = static_cast<Block*>(static_data);
    block->prev = nullptr;
    block->next = nullptr;

    BL_ASSERT(static_size >= kBlockSize);
    block->size = static_size - kBlockSize;

    ArenaAllocator_assignBlock(this, block);
    _block_count = 1u;
  }
}

void ArenaAllocator::reset() noexcept {
  // Can't be altered.
  Block* cur = _block;
  if (cur == &kArenaAllocatorZeroBlock.block)
    return;

  Block* initial = const_cast<ArenaAllocator::Block*>(&kArenaAllocatorZeroBlock.block);
  _ptr = initial->data();
  _end = initial->data();
  _block = initial;
  _block_count = 0u;

  // Since cur can be in the middle of the double-linked list, we have to traverse both directions separately.
  Block* next = cur->next;
  do {
    Block* prev = cur->prev;

    // If this is the first block and this AllocatorTmp is temporary then the first block is statically allocated.
    // We cannot free it and it makes sense to keep it even when this is hard reset.
    if (prev == nullptr && _has_static_block) {
      cur->prev = nullptr;
      cur->next = nullptr;
      ArenaAllocator_assignBlock(this, cur);
      break;
    }

    ::free(cur);
    cur = prev;
  } while (cur);

  cur = next;
  while (cur) {
    next = cur->next;
    ::free(cur);
    cur = next;
  }
}

void ArenaAllocator::clear() noexcept {
  Block* cur = _block;
  while (cur->prev) {
    cur = cur->prev;
  }
  ArenaAllocator_assignBlock(this, cur);
}

void* ArenaAllocator::_alloc(size_t size, size_t alignment) noexcept {
  Block* cur_block = _block;
  Block* next = cur_block->next;

  size_t default_block_alignment = block_alignment();
  size_t required_block_alignment = bl_max<size_t>(alignment, default_block_alignment);

  // If the `Arena` has been cleared the current block doesn't have to be the last one. Check if there is a block
  // that can be used instead of allocating a new one. If there is a `next` block it's completely unused, we don't
  // have to check for remaining bytes in that case.
  if (next) {
    uint8_t* ptr = IntOps::align_up(next->data(), required_block_alignment);
    uint8_t* end = next->data() + next->size;

    if (size <= (size_t)(end - ptr)) {
      _block = next;
      _ptr = ptr + size;
      _end = end;
      return static_cast<void*>(ptr);
    }
  }

  // Calculates the "default" size of a next block - in most cases this would be enough for the allocation. In
  // general we want to gradually increase block size when more and more blocks are allocated until the maximum
  // block size. Since we use shifts (aka log2(size) sizes) we just need block count and minumum/maximum block
  // size shift to calculate the final size.
  size_t default_block_size_shift = bl_min<size_t>(_block_count + _min_block_size_shift, _max_block_size_shift);
  size_t default_block_size = size_t(1) << default_block_size_shift;

  // Allocate a new block. We have to accommodate all possible overheads so after the memory is allocated and then
  // properly aligned there will be size for the requested memory. In 99.9999% cases this is never a problem, but
  // we must be sure that even rare border cases would allocate properly.
  size_t alignment_overhead = required_block_alignment - bl_min<size_t>(required_block_alignment, BL_ALLOC_ALIGNMENT);
  size_t block_size_overhead = kBlockSize + BL_ALLOC_OVERHEAD + alignment_overhead;

  // If the requested size is larger than a default calculated block size -> increase block size so the allocation
  // would be enough to fit the requested size.
  size_t final_block_size = default_block_size;

  if (BL_UNLIKELY(size > default_block_size - block_size_overhead)) {
    if (BL_UNLIKELY(size > SIZE_MAX - block_size_overhead)) {
      // This would probably never happen in practice - however, it needs to be done to stop malicious cases like
      // `alloc(SIZE_MAX)`.
      return nullptr;
    }
    final_block_size = size + alignment_overhead + kBlockSize;
  }
  else {
    final_block_size -= BL_ALLOC_OVERHEAD;
  }

  // Allocate new block.
  Block* new_block = static_cast<Block*>(::malloc(final_block_size));

  if (BL_UNLIKELY(!new_block)) {
    return nullptr;
  }

  // final_block_size includes the struct size, which must be avoided when assigning the size to a newly allocated block.
  size_t real_block_size = final_block_size - kBlockSize;

  // Align the pointer to `minimum_alignment` and adjust the size of this block accordingly. It's the same as using
  // `minimum_alignment - Support::align_up_diff()`, just written differently.
  new_block->prev = nullptr;
  new_block->next = nullptr;
  new_block->size = real_block_size;

  if (cur_block != &kArenaAllocatorZeroBlock.block) {
    new_block->prev = cur_block;
    cur_block->next = new_block;

    // Does only happen if there is a next block, but the requested memory can't fit into it. In this case a new
    // buffer is allocated and inserted between the current block and the next one.
    if (next) {
      new_block->next = next;
      next->prev = new_block;
    }
  }

  uint8_t* ptr = IntOps::align_up(new_block->data(), required_block_alignment);
  uint8_t* end = new_block->data() + real_block_size;

  _ptr = ptr + size;
  _end = end;
  _block = new_block;
  _block_count++;

  BL_ASSERT(_ptr <= _end);
  return static_cast<void*>(ptr);
}

void* ArenaAllocator::alloc_zeroed(size_t size, size_t alignment) noexcept {
  void* p = alloc(size, alignment);
  if (BL_UNLIKELY(!p))
    return p;
  return memset(p, 0, size);
}

} // {bl}
