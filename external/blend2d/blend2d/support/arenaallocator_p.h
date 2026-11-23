// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_ARENAALLOCATOR_P_H_INCLUDED
#define BLEND2D_SUPPORT_ARENAALLOCATOR_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/support/intops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Arena memory allocator.
//!
//! Arena allocator is an incremental memory allocator that allocates memory by simply incrementing a pointer.
//! It allocates blocks of memory by using standard C library `malloc/free`, but divides these blocks into
//! smaller chunks requested by calling `ArenaAllocator::alloc()` and friends.
//!
//! Arena allocators are designed to either allocate memory for data that has a short lifetime or data in containers
//! where it's expected that many small chunks will be allocated.
//!
//! \note It's not recommended to use `ArenaAllocator` to allocate larger data structures than the initial `block_size`
//! passed to its constructor. The block size should be always greater than the maximum `size` passed to `alloc()`.
//! Arena allocator is designed to handle such cases, but it may allocate new block for each call to `alloc()` that
//! exceeds the default block size.
class ArenaAllocator {
public:
  BL_NONCOPYABLE(ArenaAllocator)

  //! A single block of memory managed by `ArenaAllocator`.
  struct Block {
    //! Link to the previous block.
    Block* prev;
    //! Link to the next block.
    Block* next;
    //! Size of the block.
    size_t size;

    BL_INLINE uint8_t* data() const noexcept {
      return const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(*this));
    }

    BL_INLINE uint8_t* end() const noexcept {
      return data() + size;
    }
  };

  typedef uint8_t* StatePtr;

  static inline constexpr size_t kMinBlockSize = 1024; // Safe bet - it must be greater than `kMaxAlignment`.
  static inline constexpr size_t kMaxBlockSize = size_t(1) << (sizeof(size_t) * 8 - 1);

  static inline constexpr size_t kMinAlignment = 1;
  static inline constexpr size_t kMaxAlignment = 64;

  static inline constexpr size_t kBlockSize = sizeof(Block);
  static inline constexpr size_t kBlockOverhead = kBlockSize + kMaxAlignment + size_t(BL_ALLOC_OVERHEAD);

  //! Pointer in the current block.
  uint8_t* _ptr;
  //! End of the current block.
  uint8_t* _end;
  //! Current block.
  Block* _block;

  //! Block alignment shift
  uint8_t _block_alignment_shift;
  //! Minimum log2(block_size) to allocate.
  uint8_t _min_block_size_shift;
  //! Maximum log2(block_size) to allocate.
  uint8_t _max_block_size_shift;
  //! True when the Arena is actually ArenaTmp.
  uint8_t _has_static_block;
  //! Reserved for future use, must be zero.
  uint32_t _reserved;
  //! Count of allocated blocks.
  size_t _block_count;

  //! \name Construction & Destruction
  //! \{

  //! Create a new `ArenaAllocator`.
  //!
  //! The `block_size` parameter describes the default size of the block. If the `size` parameter passed to
  //! `alloc()` is greater than the default size `ArenaAllocator` will allocate and use a larger block, but
  //! it will not change the default `block_size`.
  //!
  //! It's not required, but it's good practice to set `block_size` to a reasonable value that depends on the
  //! usage of `ArenaAllocator`. Greater block sizes are generally safer and perform better than unreasonably
  //! low block sizes.
  BL_INLINE explicit ArenaAllocator(size_t block_size, size_t block_alignment = 1) noexcept {
    _init(block_size, block_alignment, nullptr, 0);
  }

  BL_INLINE ArenaAllocator(size_t block_size, size_t block_alignment, void* static_data, size_t static_size) noexcept {
    _init(block_size, block_alignment, static_data, static_size);
  }

  //! Destroy the `ArenaAllocator` instance.
  //!
  //! This will destroy the `ArenaAllocator` instance and release all blocks of memory allocated by it. It
  //! performs implicit `reset()`.
  BL_INLINE ~ArenaAllocator() noexcept { reset(); }

  BL_HIDDEN void _init(size_t block_size, size_t block_alignment, void* static_data, size_t static_size) noexcept;

  //! Resets the `ArenaAllocator` and invalidates all blocks it has allocated.
  BL_HIDDEN void reset() noexcept;

  //! \}

  //! \name Basic Operations
  //! \{

  //! Invalidates all allocations and moves the current block pointer to the first block. It's similar to
  //! `reset()`, however, it doesn't free blocks of memory it holds.
  BL_HIDDEN void clear() noexcept;

  BL_INLINE void swap(ArenaAllocator& other) noexcept {
    // This could lead to a disaster.
    BL_ASSERT(!this->has_static_block());
    BL_ASSERT(!other.has_static_block());

    BLInternal::swap(_ptr, other._ptr);
    BLInternal::swap(_end, other._end);
    BLInternal::swap(_block, other._block);

    BLInternal::swap(_block_alignment_shift, other._block_alignment_shift);
    BLInternal::swap(_min_block_size_shift, other._min_block_size_shift);
    BLInternal::swap(_max_block_size_shift, other._max_block_size_shift);
    BLInternal::swap(_has_static_block, other._has_static_block);
    BLInternal::swap(_reserved, other._reserved);
    BLInternal::swap(_block_count, other._block_count);
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether this `ArenaAllocator` is actually a `ArenaAllocatorTmp` that uses temporary memory.
  [[nodiscard]]
  BL_INLINE bool has_static_block() const noexcept { return _has_static_block != 0; }

  //! Returns the minimum block size.
  [[nodiscard]]
  BL_INLINE size_t min_block_size() const noexcept { return size_t(1) << _min_block_size_shift; }

  //! Returns the maximum block size.
  [[nodiscard]]
  BL_INLINE size_t max_block_size() const noexcept { return size_t(1) << _max_block_size_shift; }

  //! Returns the default block alignment.
  [[nodiscard]]
  BL_INLINE size_t block_alignment() const noexcept { return size_t(1) << _block_alignment_shift; }

  //! Returns the remaining size of the current block.
  [[nodiscard]]
  BL_INLINE size_t remaining_size() const noexcept { return (size_t)(_end - _ptr); }

  //! Returns the current arena allocator cursor (dangerous).
  //!
  //! This is a function that can be used to get exclusive access to the current block's memory buffer.
  template<typename T = uint8_t>
  [[nodiscard]]
  BL_INLINE T* ptr() noexcept { return reinterpret_cast<T*>(_ptr); }

  //! Returns the end of the current arena allocator block, only useful if you use `ptr()`.
  template<typename T = uint8_t>
  [[nodiscard]]
  BL_INLINE T* end() noexcept { return reinterpret_cast<T*>(_end); }

  // NOTE: The following two functions `set_ptr()` and `set_end()` can be used to perform manual memory allocation
  // in case that an incremental allocation is needed - for example you build some data structure without knowing
  // the final size. This is used for example by AnalyticRasterizer to build list of edges.

  //! Sets the current arena allocator pointer to `ptr` (must be within the current block).
  template<typename T>
  BL_INLINE void set_ptr(T* ptr) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(ptr);
    BL_ASSERT(p >= _ptr && p <= _end);
    _ptr = p;
  }

  //! Sets the end arena allocator pointer to `end` (must be within the current block).
  template<typename T>
  BL_INLINE void set_end(T* end) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(end);
    BL_ASSERT(p >= _ptr && p <= _end);
    _end = p;
  }

  //! Align the current pointer to `alignment`.
  BL_INLINE void align(size_t alignment) noexcept {
    _ptr = bl_min(IntOps::align_up(_ptr, alignment), _end);
  }

  //! Ensures the remaining size is at least equal or greater than `size`.
  //!
  //! \note This function doesn't respect any alignment. If you need to ensure there is enough room for an aligned
  //! allocation you need to call `align()` before calling `ensure()`.
  [[nodiscard]]
  BL_INLINE BLResult ensure(size_t size) noexcept {
    if (size <= remaining_size())
      return BL_SUCCESS;
    else
      return _alloc(0, 1) ? BL_SUCCESS : bl_make_error(BL_ERROR_OUT_OF_MEMORY);
  }

  //! \}

  //! \name Allocation
  //! \{

  //! Internal alloc function.
  [[nodiscard]]
  BL_HIDDEN void* _alloc(size_t size, size_t alignment) noexcept;

  //! Allocates the requested memory specified by `size`.
  //!
  //! Pointer returned is valid until the `ArenaAllocator` instance is destroyed or reset by calling `reset()`.
  //! If you plan to make an instance of C++ from the given pointer use placement `new` and `delete` operators:
  //!
  //! ```
  //! class Object { ... };
  //!
  //! // Create a new arena with default block size of approximately 65536 bytes.
  //! ArenaAllocator arena(65536);
  //!
  //! // Create your objects using arena object allocating, for example:
  //! Object* obj = static_cast<Object*>(arena.alloc(sizeof(Object)));
  //
  //! if (!obj) {
  //!   // Handle out of memory error.
  //! }
  //!
  //! // Placement `new` and `delete` operators can be used to instantiate it.
  //! new(obj) Object();
  //!
  //! // ... lifetime of your objects ...
  //!
  //! // To destroy the instance (if required).
  //! obj->~Object();
  //!
  //! // Reset or destroy `ArenaAllocator`.
  //! arena.reset();
  //! ```
  [[nodiscard]]
  BL_INLINE void* alloc(size_t size) noexcept {
    if (BL_UNLIKELY(size > remaining_size()))
      return _alloc(size, 1);

    uint8_t* ptr = _ptr;
    _ptr += size;
    return static_cast<void*>(ptr);
  }

  //! Allocates the requested memory specified by `size` and `alignment`.
  //!
  //! Performs the same operation as `ArenaAllocator::alloc(size)` with `alignment` applied.
  [[nodiscard]]
  BL_INLINE void* alloc(size_t size, size_t alignment) noexcept {
    BL_ASSERT(IntOps::is_power_of_2(alignment));
    uint8_t* ptr = IntOps::align_up(_ptr, alignment);

    if (size > (size_t)(_end - ptr))
      return _alloc(size, alignment);

    _ptr = ptr + size;
    return static_cast<void*>(ptr);
  }

  //! Allocates the requested memory specified by `size` without doing any checks.
  //!
  //! Can only be called if `remaining_size()` returns size at least equal to `size`.
  [[nodiscard]]
  BL_INLINE void* alloc_no_check(size_t size) noexcept {
    BL_ASSERT(remaining_size() >= size);

    uint8_t* ptr = _ptr;
    _ptr += size;
    return static_cast<void*>(ptr);
  }

  //! Allocates the requested memory specified by `size` and `alignment` without doing any checks.
  //!
  //! Performs the same operation as `ArenaAllocator::alloc_no_check(size)` with `alignment` applied.
  [[nodiscard]]
  BL_INLINE void* alloc_no_check(size_t size, size_t alignment) noexcept {
    BL_ASSERT(IntOps::is_power_of_2(alignment));

    uint8_t* ptr = IntOps::align_up(_ptr, alignment);
    BL_ASSERT(size <= (size_t)(_end - ptr));

    _ptr = ptr + size;
    return static_cast<void*>(ptr);
  }

  //! Allocates the requested memory specified by `size` and `alignment` and clears it before returning its pointer.
  //!
  //! See `alloc()` for more details.
  [[nodiscard]]
  BL_HIDDEN void* alloc_zeroed(size_t size, size_t alignment = 1) noexcept;

  //! Like `alloc()`, but the return pointer is casted to `T*`.
  template<typename T>
  [[nodiscard]]
  BL_INLINE T* allocT(size_t size = sizeof(T), size_t alignment = alignof(T)) noexcept {
    return static_cast<T*>(alloc(size, alignment));
  }

  template<typename T>
  [[nodiscard]]
  BL_INLINE T* allocNoAlignT(size_t size = sizeof(T)) noexcept {
    T* ptr = static_cast<T*>(alloc(size));
    BL_ASSERT(IntOps::is_aligned(ptr, alignof(T)));
    return ptr;
  }

  //! Like `alloc_no_check()`, but the return pointer is casted to `T*`.
  template<typename T>
  [[nodiscard]]
  BL_INLINE T* allocNoCheckT(size_t size = sizeof(T), size_t alignment = alignof(T)) noexcept {
    return static_cast<T*>(alloc_no_check(size, alignment));
  }

  //! Like `alloc_zeroed()`, but the return pointer is casted to `T*`.
  template<typename T>
  [[nodiscard]]
  BL_INLINE T* alloc_zeroedT(size_t size = sizeof(T), size_t alignment = alignof(T)) noexcept {
    return static_cast<T*>(alloc_zeroed(size, alignment));
  }

  //! Like `new(std::nothrow) T(...)`, but allocated by `ArenaAllocator`.
  template<typename T>
  [[nodiscard]]
  BL_INLINE T* new_t() noexcept {
    void* p = alloc(sizeof(T), alignof(T));
    if (BL_UNLIKELY(!p))
      return nullptr;
    return new(BLInternal::PlacementNew{p}) T();
  }

  //! Like `new(std::nothrow) T(...)`, but allocated by `ArenaAllocator`.
  template<typename T, typename... Args>
  [[nodiscard]]
  BL_INLINE T* new_t(Args&&... args) noexcept {
    void* p = alloc(sizeof(T), alignof(T));
    if (BL_UNLIKELY(!p))
      return nullptr;
    return new(BLInternal::PlacementNew{p}) T(BLInternal::forward<Args>(args)...);
  }

  BL_INLINE void release(void* ptr, size_t size) noexcept {
    // TODO: This should work by creating an invisible block.
    bl_unused(ptr, size);
  }

  //! \}

  //! \name State Management
  //! \{

  //! Stores the current state to `state`.
  [[nodiscard]]
  BL_INLINE StatePtr save_state() noexcept {
    return _ptr;
  }

  //! Restores the state of `ArenaAllocator` from the previously saved `state`.
  BL_INLINE void restore_state(StatePtr p) noexcept {
    Block* block = _block;
    size_t alignment = block_alignment();

    while (p < block->data() || p >= block->end()) {
      if (!block->prev) {
        // Special case - can happen in case that the allocator didn't have allocated any block when `save_state()`
        // was called. In that case we won't restore to the shared null block, instead we restore to the first block
        // the allocator has.
        p = IntOps::align_up(block->data(), alignment);
        break;
      }
      block = block->prev;
    }

    _block = block;
    _ptr = p;
    _end = block->end();
  }

  //! \}

  //! \name Block Management
  //! \{

  //! Returns a past block - a block used before the current one, or null if this is the first block. Use together
  //! with `reuse_past_block()`.
  [[nodiscard]]
  BL_INLINE Block* past_block() const noexcept { return _block->prev; }

  //! Moves the passed block after the current block and makes the block after the given `block` first.
  BL_INLINE void reuse_past_block(Block* past_last) noexcept {
    BL_ASSERT(past_last != nullptr); // Cannot be null, check for null block before.
    BL_ASSERT(past_last != _block);  // Cannot be the current block, must be past that.

    Block* past_first = past_last;
    while (past_first->prev)
      past_first = past_first->prev;

    // Makes `past_next` the first block.
    Block* past_next = past_last->next;
    past_next->prev = nullptr;

    // Link [past_first:past_last] between `_block` and next.
    Block* next = _block->next;

    _block->next = past_first;
    past_first->prev = _block;

    next->prev = past_last;
    past_last->next = next;
  }

  //! \}
};

//! A temporary `ArenaAllocator`.
template<size_t N>
class ArenaAllocatorTmp : public ArenaAllocator {
public:
  BL_NONCOPYABLE(ArenaAllocatorTmp)

  BL_INLINE explicit ArenaAllocatorTmp(size_t block_size, size_t block_alignment = 1) noexcept
    : ArenaAllocator(block_size, block_alignment, _storage.data, N) {}

  struct Storage {
    char data[N];
  } _storage;
};

//! Helper class for implementing pooling of arena-allocated objects.
template<typename T, size_t SizeOfT = sizeof(T)>
class ArenaPool {
public:
  BL_NONCOPYABLE(ArenaPool)

  struct Link { Link* next; };
  Link* _pool;

  BL_INLINE ArenaPool() noexcept
    : _pool(nullptr) {}

  //! Resets the arena pool.
  //!
  //! Reset must be called after the associated `ArenaAllocator` has been reset, otherwise the existing pool will
  //! collide with possible allocations made on the `ArenaAllocator` object after the reset.
  BL_INLINE void reset() noexcept { _pool = nullptr; }

  //! Ensures that there is at least one object in the pool.
  [[nodiscard]]
  BL_INLINE bool ensure(ArenaAllocator& arena) noexcept {
    if (_pool) return true;

    Link* p = static_cast<Link*>(arena.alloc(SizeOfT));
    if (p == nullptr) return false;

    p->next = nullptr;
    _pool = p;
    return true;
  }

  //! Allocates a memory (or reuses the existing allocation) of `SizeOfT` (in bytes).
  [[nodiscard]]
  BL_INLINE T* alloc(ArenaAllocator& arena) noexcept {
    Link* p = _pool;
    if (BL_UNLIKELY(p == nullptr))
      return static_cast<T*>(arena.alloc(SizeOfT));
    _pool = p->next;
    return static_cast<T*>(static_cast<void*>(p));
  }

  //! Like `alloc()`, but can be only called after `ensure()` returned `true`.
  [[nodiscard]]
  BL_INLINE T* alloc_ensured() noexcept {
    Link* p = _pool;
    BL_ASSERT(p != nullptr);

    _pool = p->next;
    return static_cast<T*>(static_cast<void*>(p));
  }

  //! Pools the previously allocated memory.
  BL_INLINE void free(T* _p) noexcept {
    BL_ASSERT(_p != nullptr);
    Link* p = reinterpret_cast<Link*>(_p);

    p->next = _pool;
    _pool = p;
  }
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_ARENAALLOCATOR_P_H_INCLUDED
