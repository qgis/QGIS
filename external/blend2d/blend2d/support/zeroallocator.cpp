// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenalist_p.h>
#include <blend2d/support/arenatree_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/traits_p.h>
#include <blend2d/support/wrap_p.h>
#include <blend2d/support/zeroallocator_p.h>
#include <blend2d/threading/mutex_p.h>

// Define to always check whether the memory has been zeroed before release.
// #define BL_BUILD_DEBUG_ZERO_ALLOCATOR

namespace bl {

#if defined(BL_BUILD_DEBUG) || defined(BL_BUILD_DEBUG_ZERO_ALLOCATOR)
static void ZeroAllocator_checkReleasedMemory(uint8_t* ptr, size_t size) noexcept {
  // Must be aligned.
  BL_ASSERT(IntOps::is_aligned(ptr , sizeof(uintptr_t)));
  BL_ASSERT(IntOps::is_aligned(size, sizeof(uintptr_t)));

  uintptr_t* p = reinterpret_cast<uintptr_t*>(ptr);
  bool zeroed_memory_was_not_zero = false;

  for (size_t i = 0; i < size / sizeof(uintptr_t); i++) {
    if (p[i] != 0) {
      zeroed_memory_was_not_zero = true;
      bl_runtime_message_fmt("bl::ZeroAllocator::check_released_memory(): Found non-zero: %p[%zu] == %zu\n", p, i * sizeof(uintptr_t), size_t(p[i]));
    }
  }

  BL_ASSERT(!zeroed_memory_was_not_zero);
}
#endif

//! Calculate the number of elements that would be required if `base` is granularized by `granularity`.
//! This function can be used to calculate the number of BitWords to represent N bits, for example.
template<typename X, typename Y>
static constexpr X ZeroAllocator_numGranularized(const X& base, const Y& granularity) noexcept {
  using U = BLInternal::UIntByType<X>;
  return X((U(base) + U(granularity) - 1) / U(granularity));
}

//! Based on asmjit's JitAllocator, but modified and enhanced for our own purposes.
class ZeroAllocator {
public:
  BL_NONCOPYABLE(ZeroAllocator)
  typedef PrivateBitWordOps BitOps;

  enum : uint32_t {
    kBlockAlignment = 64,
    kBlockGranularity = 1024,

    kMinBlockSize = 1024 * 1024,     // 1MB.
    kMaxBlockSize = 1024 * 1024 * 16 // 16MB.
  };

  static constexpr size_t bit_word_count_from_area_size(uint32_t area_size) noexcept {
    return IntOps::align_up<size_t>(area_size, IntOps::bit_size_of<BLBitWord>()) / IntOps::bit_size_of<BLBitWord>();
  }

  class Block : public ArenaTreeNode<Block>, public ArenaListNode<Block> {
  public:
    BL_NONCOPYABLE(Block)

    enum Flags : uint32_t {
      //! This is a statically allocated block.
      kFlagStatic = 0x00000001u,
      //! Block is dirty (some members need to be updated).
      kFlagDirty  = 0x80000000u
    };

    //! Zeroed buffer managed by this block.
    uint8_t* _buffer;
    //! Aligned `_buffer` to kBlockAlignment.
    uint8_t* _buffer_aligned;
    //! Size of `buffer` in bytes.
    size_t _block_size;

    //! Block flags.
    uint32_t _flags;
    //! Size of the whole block area (bit-vector size).
    uint32_t _area_size;
    //! Used area (number of bits in bit-vector used).
    uint32_t _area_used;
    //! The largest unused continuous area in the bit-vector (or `_area_size` to initiate rescan).
    uint32_t _largest_unused_area;
    //! Start of a search range (for unused bits).
    uint32_t _search_start;
    //! End of a search range (for unused bits).
    uint32_t _search_end;
    //! Bit vector representing all used areas (0 = unused, 1 = used).
    BLBitWord _bit_vector[1];

    BL_INLINE Block(uint8_t* buffer, size_t block_size, uint32_t area_size) noexcept
      : ArenaTreeNode(),
        _buffer(buffer),
        _buffer_aligned(IntOps::align_up(buffer, kBlockAlignment)),
        _block_size(block_size),
        _flags(0),
        _area_size(area_size),
        _area_used(0),
        _largest_unused_area(area_size),
        _search_start(0),
        _search_end(area_size) {}

    BL_INLINE uint8_t* buffer_aligned() const noexcept { return _buffer_aligned; }
    BL_INLINE size_t block_size() const noexcept { return _block_size; }

    BL_INLINE size_t overhead_size() const noexcept {
      return sizeof(Block) - sizeof(BLBitWord) + ZeroAllocator_numGranularized(area_size(), IntOps::bit_size_of<BLBitWord>());
    }

    BL_INLINE uint32_t flags() const noexcept { return _flags; }
    BL_INLINE bool has_flag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
    BL_INLINE void add_flags(uint32_t flags) noexcept { _flags |= flags; }
    BL_INLINE void clear_flags(uint32_t flags) noexcept { _flags &= ~flags; }

    BL_INLINE uint32_t area_size() const noexcept { return _area_size; }
    BL_INLINE uint32_t area_used() const noexcept { return _area_used; }
    BL_INLINE uint32_t area_available() const noexcept { return area_size() - area_used(); }
    BL_INLINE uint32_t largest_unused_area() const noexcept { return _largest_unused_area; }

    BL_INLINE void reset_bit_vector() noexcept {
      memset(_bit_vector, 0, ZeroAllocator_numGranularized(_area_size, IntOps::bit_size_of<BLBitWord>()) * sizeof(BLBitWord));
    }

    // RBTree default CMP uses '<' and '>' operators.
    BL_INLINE bool operator<(const Block& other) const noexcept { return buffer_aligned() < other.buffer_aligned(); }
    BL_INLINE bool operator>(const Block& other) const noexcept { return buffer_aligned() > other.buffer_aligned(); }

    // Special implementation for querying blocks by `key`, which must be in `[buffer, buffer + block_size)` range.
    BL_INLINE bool operator<(const uint8_t* key) const noexcept { return buffer_aligned() + block_size() <= key; }
    BL_INLINE bool operator>(const uint8_t* key) const noexcept { return buffer_aligned() > key; }
  };

  //! Mutex for thread safety.
  mutable BLMutex _mutex;
  //! Tree that contains all blocks.
  ArenaTree<Block> _tree;
  //! Double linked list of blocks.
  ArenaList<Block> _blocks;
  //! Allocated block count.
  size_t _block_count;
  //! Area size of base block.
  size_t _base_area_size;
  //! Number of bits reserved across all blocks.
  size_t _total_area_size;
  //! Number of bits used across all blocks.
  size_t _total_area_used;
  //! A threshold to trigger auto-cleanup.
  size_t _cleanup_threshold;
  //! Memory overhead required to manage blocks.
  size_t _overhead_size;

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ZeroAllocator(Block* base_block) noexcept
    : _tree(),
      _blocks(),
      _block_count(0),
      _base_area_size(0),
      _total_area_size(0),
      _total_area_used(0),
      _cleanup_threshold(0),
      _overhead_size(0) {

    base_block->add_flags(Block::kFlagStatic);
    insert_block(base_block);

    _base_area_size = _total_area_size;
    _cleanup_threshold = _total_area_size;
  }

  BL_INLINE ~ZeroAllocator() noexcept {
    _cleanup_internal();
  }

  //! \}

  //! \name Block Management
  //! \{

  // Allocate a new `ZeroAllocator::Block` for the given `block_size`.
  Block* new_block(size_t block_size) noexcept {
    uint32_t area_size = uint32_t((block_size + kBlockGranularity - 1) / kBlockGranularity);
    uint32_t num_bit_words = (area_size + IntOps::bit_size_of<BLBitWord>() - 1u) / IntOps::bit_size_of<BLBitWord>();

    size_t block_struct_size = sizeof(Block) + size_t(num_bit_words - 1) * sizeof(BLBitWord);
    Block* block = static_cast<Block*>(malloc(block_struct_size));
    uint8_t* buffer = static_cast<uint8_t*>(::calloc(1, block_size + kBlockAlignment));

    // Out of memory.
    if (BL_UNLIKELY(!block || !buffer)) {
      if (buffer)
        free(buffer);

      if (block)
        free(block);

      return nullptr;
    }

    bl_call_ctor(*block, buffer, block_size, area_size);
    block->reset_bit_vector();
    return block;
  }

  void delete_block(Block* block) noexcept {
    BL_ASSERT(!(block->has_flag(Block::kFlagStatic)));

    free(block->_buffer);
    free(block);
  }

  void insert_block(Block* block) noexcept {
    // Add to RBTree and List.
    _tree.insert(block);
    _blocks.append(block);

    // Update statistics.
    _block_count++;
    _total_area_size += block->area_size();
    _overhead_size += block->overhead_size();
  }

  void remove_block(Block* block) noexcept {
    // Remove from RBTree and List.
    _tree.remove(block);
    _blocks.unlink(block);

    // Update statistics.
    _block_count--;
    _total_area_size -= block->area_size();
    _overhead_size -= block->overhead_size();
  }

  BL_INLINE size_t calculate_ideal_block_size(size_t allocation_size) noexcept {
    uint32_t kMaxSizeShift = IntOps::ctz_static(kMaxBlockSize) -
                             IntOps::ctz_static(kMinBlockSize) ;

    size_t block_size = size_t(kMinBlockSize) << bl_min<size_t>(_block_count, kMaxSizeShift);
    if (block_size < allocation_size)
      block_size = IntOps::align_up(allocation_size, block_size);
    return block_size;
  }

  BL_INLINE size_t calculate_cleanup_threshold() const noexcept {
    if (_block_count <= 6)
      return 0;

    size_t area = _total_area_size - _base_area_size;
    size_t threshold = area / 5u;
    return _base_area_size + threshold;
  }

  //! \}

  //! \name Cleanup
  //! \{

  void _cleanup_internal(size_t n = SIZE_MAX) noexcept {
    Block* block = _blocks.last();

    while (block && n) {
      Block* prev = block->prev();
      if (block->area_used() == 0 && !(block->has_flag(Block::kFlagStatic))) {
        remove_block(block);
        delete_block(block);
        n--;
      }
      block = prev;
    }

    _cleanup_threshold = calculate_cleanup_threshold();
  }

  //! \}

  //! \name Alloc & Release
  //! \{

  void* _alloc_internal(size_t size, size_t* allocated_size) noexcept {
    constexpr uint32_t kNoIndex = Traits::max_value<uint32_t>();

    // Align to minimum granularity by default.
    size = IntOps::align_up<size_t>(size, kBlockGranularity);
    *allocated_size = 0;

    if (BL_UNLIKELY(size == 0 || size > Traits::max_value<uint32_t>() / 2))
      return nullptr;

    Block* block = _blocks.first();
    uint32_t area_index = kNoIndex;
    uint32_t area_size = uint32_t(ZeroAllocator_numGranularized(size, kBlockGranularity));

    // Try to find the requested memory area in existing blocks.
    if (block) {
      Block* initial = block;
      do {
        Block* next = block->has_next() ? block->next() : _blocks.first();
        if (block->area_available() >= area_size) {
          if (block->has_flag(Block::kFlagDirty) || block->largest_unused_area() >= area_size) {
            uint32_t block_area_size = block->area_size();
            uint32_t search_start = block->_search_start;
            uint32_t search_end = block->_search_end;

            BitOps::BitVectorFlipIterator it(
              block->_bit_vector, ZeroAllocator_numGranularized(search_end, BitOps::kNumBits), search_start, BitOps::ones());

            // If there is unused area available then there has to be at least one match.
            BL_ASSERT(it.has_next());

            uint32_t best_area = block_area_size;
            uint32_t largest_area = 0;

            uint32_t hole_index = uint32_t(it.peek_next());
            uint32_t hole_end = hole_index;

            search_start = hole_index;
            do {
              hole_index = uint32_t(it.next_and_flip());
              if (hole_index >= search_end)
                break;

              hole_end = it.has_next() ? bl_min(search_end, uint32_t(it.next_and_flip())) : search_end;
              uint32_t hole_size = hole_end - hole_index;

              if (hole_size >= area_size && best_area >= hole_size) {
                largest_area = bl_max(largest_area, best_area);
                best_area = hole_size;
                area_index = hole_index;
              }
              else {
                largest_area = bl_max(largest_area, hole_size);
              }
            } while (it.has_next());
            search_end = hole_end;

            // Because we have traversed the entire block, we can now mark the
            // largest unused area that can be used to cache the next traversal.
            block->_search_start = search_start;
            block->_search_end = search_end;
            block->_largest_unused_area = largest_area;
            block->clear_flags(Block::kFlagDirty);

            if (area_index != kNoIndex) {
              if (search_start == area_index)
                block->_search_start += area_size;
              break;
            }
          }
        }

        block = next;
      } while (block != initial);
    }

    // Allocate a new block if there is no region of a required width.
    if (area_index == kNoIndex) {
      size_t block_size = calculate_ideal_block_size(size);
      block = new_block(block_size);

      if (BL_UNLIKELY(!block))
        return nullptr;

      insert_block(block);
      _cleanup_threshold = calculate_cleanup_threshold();

      area_index = 0;
      block->_search_start = area_size;
      block->_largest_unused_area = block->area_size() - area_size;
    }

    // Update statistics.
    _total_area_used += area_size;
    block->_area_used += area_size;

    // Handle special cases.
    if (block->area_available() == 0) {
      // The whole block is filled.
      block->_search_start = block->area_size();
      block->_search_end = 0;
      block->_largest_unused_area = 0;
      block->clear_flags(Block::kFlagDirty);
    }

    // Mark the newly allocated space as occupied and also the sentinel.
    BitOps::bit_array_fill(block->_bit_vector, area_index, area_size);

    // Return a pointer to allocated memory.
    uint8_t* result = block->buffer_aligned() + area_index * kBlockGranularity;
    BL_ASSERT(result >= block->buffer_aligned());
    BL_ASSERT(result <= block->buffer_aligned() + block->block_size() - size);

    *allocated_size = size;
    return result;
  }

  void _release_internal(void* ptr, size_t size) noexcept {
    BL_ASSERT(ptr != nullptr);
    BL_ASSERT(size != 0);

    Block* block = _tree.get(static_cast<uint8_t*>(ptr));
    BL_ASSERT(block != nullptr);

#if defined(BL_BUILD_DEBUG) || defined(BL_BUILD_DEBUG_ZERO_ALLOCATOR)
    ZeroAllocator_checkReleasedMemory(static_cast<uint8_t*>(ptr), size);
#endif

    // Offset relative to the start of the block.
    size_t byte_offset = (size_t)((uint8_t*)ptr - block->buffer_aligned());

    // The first bit representing the allocated area and its size.
    uint32_t area_index = uint32_t(byte_offset / kBlockGranularity);
    uint32_t area_size = uint32_t(ZeroAllocator_numGranularized(size, kBlockGranularity));

    // Update the search region and statistics.
    block->_search_start = bl_min(block->_search_start, area_index);
    block->_search_end = bl_max(block->_search_end, area_index + area_size);
    block->add_flags(Block::kFlagDirty);

    block->_area_used -= area_size;
    _total_area_used -= area_size;

    // Clear bits used to mark this area as occupied.
    BitOps::bit_array_clear(block->_bit_vector, area_index, area_size);

    if (_total_area_used < _cleanup_threshold)
      _cleanup_internal(1);
  }

  BL_INLINE void* _resize_internal(void* prev_ptr, size_t prev_size, size_t size, size_t* allocated_size) noexcept {
    if (prev_ptr != nullptr)
      _release_internal(prev_ptr, prev_size);
    return _alloc_internal(size, allocated_size);
  }

  //! \}

  //! \name API
  //! \{

  BL_INLINE void* alloc(size_t size, size_t* allocated_size) noexcept {
    return _mutex.protect([&] { return _alloc_internal(size, allocated_size); });
  }

  BL_INLINE void* resize(void* prev_ptr, size_t prev_size, size_t size, size_t* allocated_size) noexcept {
    return _mutex.protect([&] { return _resize_internal(prev_ptr, prev_size, size, allocated_size); });
  }

  BL_INLINE void release(void* ptr, size_t size) noexcept {
    _mutex.protect([&] { _release_internal(ptr, size); });
  }

  BL_INLINE void cleanup() noexcept {
    _mutex.protect([&] { _cleanup_internal(); });
  }

  BL_INLINE void on_resource_info(BLRuntimeResourceInfo* resource_info) noexcept {
    _mutex.protect([&] {
      resource_info->zm_used = _total_area_used * kBlockGranularity;
      resource_info->zm_reserved = _total_area_size * kBlockGranularity;
      resource_info->zm_overhead = _overhead_size;
      resource_info->zm_block_count = _block_count;
    });
  }

  //! \}
};

static Wrap<ZeroAllocator> zero_allocator_global;

// ZeroAllocator - Static Buffer
// ===============================

// Base memory is a zeroed memory allocated by the linker. By default we use 1MB of memory that we will use
// as a base before obtaining more from the system if that's not enough.

struct ZeroAllocatorStaticBlock {
  enum : uint32_t {
    kBlockSize = 1024u * 1024u,
    kAreaSize = ZeroAllocator_numGranularized(kBlockSize, ZeroAllocator::kBlockGranularity),
    kBitWordCount = ZeroAllocator_numGranularized(kAreaSize, IntOps::bit_size_of<BLBitWord>())
  };

  Wrap<ZeroAllocator::Block> block;
  BLBitWord bit_words[kBitWordCount];
};

struct alignas(64) ZeroAllocatorStaticBuffer {
  uint8_t buffer[ZeroAllocatorStaticBlock::kBlockSize];
};

static ZeroAllocatorStaticBlock zero_allocator_static_block;
static ZeroAllocatorStaticBuffer zero_allocator_static_buffer;

} // {bl}

// bl::ZeroAllocator - API
// =======================

void* bl_zero_allocator_alloc(size_t size, size_t* allocated_size) noexcept {
  return bl::zero_allocator_global->alloc(size, allocated_size);
}

void* bl_zero_allocator_resize(void* prev_ptr, size_t prev_size, size_t size, size_t* allocated_size) noexcept {
  return bl::zero_allocator_global->resize(prev_ptr, prev_size, size, allocated_size);
}

void bl_zero_allocator_release(void* ptr, size_t size) noexcept {
  bl::zero_allocator_global->release(ptr, size);
}

// bl::ZeroAllocator - Runtime
// ===========================

static void BL_CDECL bl_zero_allocator_rt_shutdown(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);
  bl::zero_allocator_global.destroy();
}

static void BL_CDECL bl_zero_allocator_rt_cleanup(BLRuntimeContext* rt, BLRuntimeCleanupFlags cleanup_flags) noexcept {
  bl_unused(rt);
  if (cleanup_flags & BL_RUNTIME_CLEANUP_ZEROED_POOL)
    bl::zero_allocator_global->cleanup();
}

static void BL_CDECL bl_zero_allocator_rt_resource_info(BLRuntimeContext* rt, BLRuntimeResourceInfo* resource_info) noexcept {
  bl_unused(rt);
  bl::zero_allocator_global->on_resource_info(resource_info);
}

void bl_zero_allocator_rt_init(BLRuntimeContext* rt) noexcept {
  bl::ZeroAllocator::Block* block =
    bl::zero_allocator_static_block.block.init(
      bl::zero_allocator_static_buffer.buffer,
      bl::ZeroAllocatorStaticBlock::kBlockSize,
      bl::ZeroAllocatorStaticBlock::kAreaSize);

  bl::zero_allocator_global.init(block);

  rt->shutdown_handlers.add(bl_zero_allocator_rt_shutdown);
  rt->cleanup_handlers.add(bl_zero_allocator_rt_cleanup);
  rt->resource_info_handlers.add(bl_zero_allocator_rt_resource_info);
}
