// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_SUPPORT_ARENA_H_INCLUDED
#define ASMJIT_SUPPORT_ARENA_H_INCLUDED

#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_support
//! \{

//! Arena allocation statistics.
struct ArenaStatistics {
  //! \name Members
  //! \{

  //! Number of blocks maintained.
  //!
  //! A block is a bigger chunk of memory that is used by \ref Arena.
  size_t _block_count;
  //! Number of bytes allocated and in use.
  size_t _used_size;
  //! Number of bytes reserved.
  size_t _reserved_size;
  //! Overhead describes
  size_t _overhead_size;
  //! Number of bytes pooled by \ref Arena reusable pools and eventually \ref ArenaPool if aggregated.
  size_t _pooled_size;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the number of blocks maintained by \ref Arena (or multiple Arenas if aggregated).
  ASMJIT_INLINE_NODEBUG size_t block_count() const noexcept { return _block_count; }

  //! Returns the number or bytes used by \ref Arena (or multiple Arenas if aggregated).
  //!
  //! Used bytes represent the number of bytes successfully allocated by \ref Arena regardless of how these bytes
  //! are actually used. For example if \ref Arena is used with \ref ArenaPool, the number of used bytes pooled
  //! by \ref ArenaPool is included in used_size, because it was already returned by \ref Arena.
  ASMJIT_INLINE_NODEBUG size_t used_size() const noexcept { return _used_size; }

  //! Returns the number of bytes reserved by \ref Arena (or multiple Arenas if aggregated).
  ASMJIT_INLINE_NODEBUG size_t reserved_size() const noexcept { return _reserved_size; }

  //! Returns the number of bytes that were allocated, but couldn't be used by allocations because of size
  //! requests, alignment, or other reasons. The overhead should be relatively small with \ref Arena, but still
  //! can be used to find pathological cases if they happen for some reason.
  ASMJIT_INLINE_NODEBUG size_t overhead_size() const noexcept { return _overhead_size; }

  //! Returns the number of bytes pooled by \ref Arena reusable pools and eventually \ref ArenaPool if aggregated.
  ASMJIT_INLINE_NODEBUG size_t pooled_size() const noexcept { return _pooled_size; }

  //! \}

  //! \name Aggregation
  //! \{

  ASMJIT_INLINE void aggregate(const ArenaStatistics& other) noexcept {
    _block_count += other._block_count;
    _used_size += other._used_size;
    _reserved_size += other._reserved_size;
    _overhead_size += other._overhead_size;
    _pooled_size += other._pooled_size;
  }

  ASMJIT_INLINE ArenaStatistics& operator+=(const ArenaStatistics& other) noexcept {
    aggregate(other);
    return *this;
  }

  //! \}
};

//! Arena allocator is an incremental memory allocator that allocates memory by simply incrementing a pointer. It
//! allocates blocks of memory by using C's `malloc()`, but divides these blocks into smaller segments requested by
//! calling `Arena::alloc()` and friends.
class Arena {
public:
  ASMJIT_NONCOPYABLE(Arena)

  //! Default alignment of allocation requests to use when using Arena.
  static inline constexpr size_t kAlignment = 8u;

  //! \cond INTERNAL

  //! Minimum managed block size.
  static inline constexpr size_t kMinManagedBlockSize = 1024;
  //! Maximum managed block size.
  static inline constexpr size_t kMaxManagedBlockSize = size_t(1) << (sizeof(size_t) * 8 - 2);

  //! Number of slots.
  static inline constexpr size_t kReusableSlotCount = 8;
  //! How many bytes are in the first slot.
  static inline constexpr size_t kMinReusableSlotSize = 16;
  //! How many bytes are in the last slot.
  static inline constexpr size_t kMaxReusableSlotSize = kMinReusableSlotSize << (kReusableSlotCount - 1u);

  //! A single block of memory managed by `Arena`.
  struct alignas(kAlignment) ManagedBlock {
    //! Link to the next managed block (single-linked list).
    ManagedBlock* next;
    //! Size represents the number of bytes that can be allocated (it doesn't include block overhead).
    size_t size;

    ASMJIT_INLINE_NODEBUG uint8_t* data() const noexcept {
      return const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(*this));
    }

    ASMJIT_INLINE_NODEBUG uint8_t* end() const noexcept {
      return data() + size;
    }
  };

  //! Single-linked list used to store unused reusable chunks.
  struct ReusableSlot {
    //! Link to a next slot in a single-linked list.
    ReusableSlot* next;
  };

  //! A large block of memory that has been allocated dynamically and is not part of managed blocks used by the
  //! allocator. Arena keeps track of these blocks and always releases them when the Arena is destroyed or reset.
  struct DynamicBlock {
    DynamicBlock* prev;
    DynamicBlock* next;
  };

  //! Returns the slot index to be used for the given `size`. Returns `true` if a valid slot has been written to `slot`.
  [[nodiscard]]
  static ASMJIT_INLINE bool _get_reusable_slot_index(size_t size, Out<size_t> slot) noexcept {
    slot = Support::bit_size_of<size_t> - 4u - Support::clz((size - 1u) | 0xF);
    return *slot < kReusableSlotCount;
  }

  //! Returns the slot index to be used for the given `size`. Returns `true` if a valid slot has been written to `slot`
  //! and `allocated_size` has been filled with slot exact size (`allocated_size` can be equal or slightly greater than
  //! `size`).
  [[nodiscard]]
  static ASMJIT_INLINE bool _get_reusable_slot_index(size_t size, Out<size_t> slot, Out<size_t> allocated_size) noexcept {
    slot = Support::bit_size_of<size_t> - 4u - Support::clz((size - 1u) | 0xF);
    allocated_size = kMinReusableSlotSize << *slot;
    return *slot < kReusableSlotCount;
  }

  //! \endcond

  template<typename T>
  static ASMJIT_INLINE_CONSTEXPR size_t aligned_size_of() noexcept { return Support::align_up(sizeof(T), kAlignment); }

  static ASMJIT_INLINE_CONSTEXPR size_t aligned_size(size_t size) noexcept { return Support::align_up(size, kAlignment); }

  //! \name Members
  //! \{

  //! Pointer in the current block.
  uint8_t* _ptr {};
  //! End of the current block.
  uint8_t* _end {};

  //! Current block.
  ManagedBlock* _current_block {};
  //! First block (single-linked list).
  ManagedBlock* _first_block {};

  //! Current block size shift - reverted to _min_block_size_shift every time the Arena is `reset(ResetPolicy::kHard)`.
  uint8_t _current_block_size_shift {};
  //! Minimum log2(block_size) to allocate.
  uint8_t _min_block_size_shift {};
  //! Maximum log2(block_size) to allocate.
  uint8_t _max_block_size_shift {};
  //! True when the Arena has a static block (static blocks are used by ArenaTmp).
  uint8_t _has_static_block {};
  //! Unused bytes (remaining bytes in blocks that couldn't be used because of size requests).
  uint32_t _unused_byte_count {};

  //! Slots that contain reusable memory chunks.
  ReusableSlot* _reusable_slots[kReusableSlotCount] {};
  //! Large blocks for allocations that either couldn't use slots or one-shot allocation.
  DynamicBlock* _dynamic_blocks {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new Arena.
  //!
  //! The `min_block_size` parameter describes the default size of the block. If the `size` parameter passed to
  //! `alloc()` is greater than the default size `Arena` will allocate and use a larger block, but it will not change
  //! the default `min_block_size`.
  //!
  //! It's not required, but it's good practice to set `min_block_size` to a reasonable value that depends on the
  //! usage of `Arena`. Greater block sizes are generally safer and perform better than unreasonably low block sizes.
  ASMJIT_INLINE_NODEBUG explicit Arena(size_t min_block_size) noexcept {
    _init(min_block_size, Span<uint8_t>{});
  }

  //! Creates a new Arena with a first block pointing to `static_arena_memory`.
  ASMJIT_INLINE_NODEBUG Arena(size_t min_block_size, Span<uint8_t> static_arena_memory) noexcept {
    _init(min_block_size, static_arena_memory);
  }

  //! Destroys the `Arena` instance.
  //!
  //! This will destroy the `Arena` instance and release all blocks of memory allocated by it. It performs implicit
  //! `reset(ResetPolicy::kHard)`.
  ASMJIT_INLINE_NODEBUG ~Arena() noexcept { reset(ResetPolicy::kHard); }

  ASMJIT_API void _init(size_t min_block_size, Span<uint8_t> static_arena_memory) noexcept;

  //! Resets the `Arena` invalidating all blocks allocated.
  //!
  //! See `ResetPolicy` for more details.
  ASMJIT_API void reset(ResetPolicy reset_policy = ResetPolicy::kSoft) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns a minimum block size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t min_block_size() const noexcept { return size_t(1) << _min_block_size_shift; }

  //! Returns a maximum block size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t max_block_size() const noexcept { return size_t(1) << _max_block_size_shift; }

  //! Tests whether this `Arena` is actually a `ArenaTmp` that uses temporary memory.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t has_static_block() const noexcept { return _has_static_block; }

  //! Returns remaining size of the current block.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t remaining_size() const noexcept { return (size_t)(_end - _ptr); }

  //! Returns the current arena cursor (dangerous).
  //!
  //! This is a function that can be used to get exclusive access to the current block's memory buffer.
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* ptr() noexcept { return reinterpret_cast<T*>(_ptr); }

  //! Returns the end of the current arena block, only useful if you use `ptr()`.
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* end() noexcept { return reinterpret_cast<T*>(_end); }

  //! Sets the current arena pointer to `ptr` (must be within the current block).
  template<typename T>
  ASMJIT_INLINE void set_ptr(T* ptr) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(ptr);
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _ptr = p;
  }

  //! Sets the end arena pointer to `end` (must be within the current block).
  template<typename T>
  ASMJIT_INLINE void set_end(T* end) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(end);
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _end = p;
  }

  //! \}

  //! \name Oneshot Allocation
  //! \{

  //! \cond INTERNAL

  //! Internal alloc function used by inline wrappers.
  [[nodiscard]]
  ASMJIT_API void* _alloc_oneshot(size_t size) noexcept;

  //! \endcond

  //! Allocates the requested memory specified by `size` and optionally casts the returned value to `T*`.
  //!
  //! Pointer returned is valid until the `Arena` instance is destroyed or reset by calling `reset()`. If you plan to
  //! make an instance of C++ from the given pointer use placement `new` and `delete` operators:
  //!
  //! ```
  //! using namespace asmjit;
  //!
  //! class Object { ... };
  //!
  //! // Create Arena with default block size of 65536 bytes (the maximum size per alloc() would be slightly less).
  //! Arena arena(65536);
  //!
  //! // Create your objects using arena object allocating, for example:
  //! Object* obj = static_cast<Object*>( arena.alloc(Arena::aligned_size_of<Object>() );
  //!
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
  //! // Reset or destroy `Arena`.
  //! arena.reset();
  //! ```
  template<typename T = void>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc_oneshot(size_t size) noexcept {
    ASMJIT_ASSERT(Support::is_aligned(size, kAlignment));

#if defined(__GNUC__)
    // We can optimize this function a little bit if we know that `size` is relatively small - which would mean
    // that we cannot possibly overflow `_ptr`. Since most of the time `alloc()` is used for known types (which
    // implies their size is known as well) this optimization is worth it as it may save us 1 or 2 instructions.
    if (__builtin_constant_p(size) && size <= 1024u) {
      uint8_t* after = _ptr + size;

      if (ASMJIT_UNLIKELY(after > _end)) {
        return static_cast<T*>(_alloc_oneshot(size));
      }

      void* p = static_cast<void*>(_ptr);
      _ptr = after;
      return static_cast<T*>(p);
    }
#endif

    if (ASMJIT_UNLIKELY(size > remaining_size())) {
      return static_cast<T*>(_alloc_oneshot(size));
    }

    void* p = static_cast<void*>(_ptr);
    _ptr += size;
    return static_cast<T*>(p);
  }

  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc_oneshot() noexcept {
    return alloc_oneshot<T>(aligned_size_of<T>());
  }

  //! Allocates `size` bytes of zeroed memory. See `alloc()` for more details.
  [[nodiscard]]
  ASMJIT_API void* _alloc_oneshot_zeroed(size_t size) noexcept;

  //! Allocates `size` bytes of zeroed memory. See `alloc()` for more details.
  template<typename T = void>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc_oneshot_zeroed(size_t size) noexcept {
    return static_cast<T*>(_alloc_oneshot_zeroed(size));
  }

  //! Like `new(std::nothrow) T(...)`, but allocated by `Arena`.
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE T* new_oneshot() noexcept {
    void* p = alloc_oneshot(aligned_size_of<T>());
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) T();
  }

  //! Like `new(std::nothrow) T(...)`, but allocated by `Arena`.
  template<typename T, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE T* new_oneshot(Args&&... args) noexcept {
    void* p = alloc_oneshot(aligned_size_of<T>());
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) T(std::forward<Args>(args)...);
  }

  //! Helper to duplicate data.
  [[nodiscard]]
  ASMJIT_API void* dup(const void* data, size_t size, bool null_terminate = false) noexcept;

  //! Helper to duplicate a formatted string, maximum size is 256 bytes.
  [[nodiscard]]
  ASMJIT_API char* sformat(const char* str, ...) noexcept;

  //! \}

  //! \name Reusable Allocation
  //! \{

  //! \cond INTERNAL
  [[nodiscard]]
  ASMJIT_API void* _alloc_reusable(size_t size, Out<size_t> allocated_size) noexcept;

  [[nodiscard]]
  ASMJIT_API void* _alloc_reusable_zeroed(size_t size, Out<size_t> allocated_size) noexcept;

  ASMJIT_API void _release_dynamic(void* p, size_t size) noexcept;
  //! \endcond

  //! Allocates `size` bytes of memory, ideally from an available pool.
  //!
  //! \note `size` can't be zero, it will assert in debug mode in such case.
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable(size_t size) noexcept {
    size_t dummy_allocated_size;
    return static_cast<T*>(_alloc_reusable(size, Out(dummy_allocated_size)));
  }

  //! Like `alloc(size)`, but provides a second argument `allocated_size` that provides a way to know how big
  //! the block returned actually is. This is useful for containers to prevent growing too early.
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable(size_t size, Out<size_t> allocated_size) noexcept {
    return static_cast<T*>(_alloc_reusable(size, allocated_size));
  }

  //! Like `alloc(size)`, but returns zeroed memory.
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable_zeroed(size_t size) noexcept {
    size_t dummy_allocated_size;
    return static_cast<T*>(_alloc_reusable_zeroed(size, Out(dummy_allocated_size)));
  }

  //! Like `alloc(size, allocated_size)`, but returns zeroed memory.
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc_reusable_zeroed(size_t size, Out<size_t> allocated_size) noexcept {
    return static_cast<T*>(_alloc_reusable_zeroed(size, allocated_size));
  }

  //! Releases the memory previously allocated by `alloc()`. The `size` argument has to be either the same `size`
  //! as used to call `alloc()` or `allocated_size` returned by `alloc()`.
  inline void free_reusable(void* p, size_t size) noexcept {
    ASMJIT_ASSERT(p != nullptr);
    ASMJIT_ASSERT(size != 0);

    size_t slot;
    if (_get_reusable_slot_index(size, Out(slot))) {
      static_cast<ReusableSlot*>(p)->next = static_cast<ReusableSlot*>(_reusable_slots[slot]);
      _reusable_slots[slot] = static_cast<ReusableSlot*>(p);
    }
    else {
      _release_dynamic(p, size);
    }
  }

  //! \}

  //! \name Statistics
  //! \{

  //! Calculates and returns statistics related to the current use of this \ref Arena.
  //!
  //! \note This function fills all members, but `_pooled_size` member (see \ref ArenaStatistics::pooled_size()
  //! function) would be assigned to zero as \ref Arena has no clue about the use of the requested memory.
  //!
  //! \attention This function could be relatively expensive depending on the number of blocks that is managed by
  //! the allocator. The primary case of this function is to use it during the development to get an idea about
  //! the use of \ref Arena (or use of multiple Arenas if the statistics is aggregated).
  ASMJIT_API ArenaStatistics statistics() const noexcept;

  //! \}
};

//! \ref Arena with `N` bytes of a static storage, used for the initial block.
//!
//! Temporary arenas are used in cases where it's known that some memory will be required, but in many cases it won't
//! exceed N bytes, so the whole operation can be performed without a dynamic memory allocation.
template<size_t N>
class ArenaTmp : public Arena {
public:
  ASMJIT_NONCOPYABLE(ArenaTmp)

  //! Temporary storage, embedded after \ref Arena.
  struct alignas(Arena::kAlignment) Storage {
    uint8_t data[N];
  } _storage;

  //! Creates a temporary arena. Dynamic block size is specified by `min_block_size`.
  ASMJIT_INLINE explicit ArenaTmp(size_t min_block_size) noexcept
    : Arena(min_block_size, Span<uint8_t>(_storage.data, N)) {}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_SUPPORT_ARENA_H_INCLUDED
