// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_CONSTPOOL_H_INCLUDED
#define ASMJIT_CORE_CONSTPOOL_H_INCLUDED

#include <asmjit/support/arena.h>
#include <asmjit/support/arenatree.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_utilities
//! \{

//! Constant pool scope.
enum class ConstPoolScope : uint32_t {
  //! Local constant, always embedded right after the current function.
  kLocal = 0,
  //! Global constant, embedded at the end of the currently compiled code.
  kGlobal = 1,

  //! Maximum value of `ConstPoolScope`.
  kMaxValue = kGlobal
};

//! Constant pool.
//!
//! Constant pool is designed to hold 1, 2, 4, 8, 16, 32, and 64 byte constants. It's not designed to hold constants
//! having arbitrary length like strings and arrays.
class ConstPool {
public:
  ASMJIT_NONCOPYABLE(ConstPool)

  //! \cond INTERNAL

  //! Index of a given size in const-pool table.
  enum Index : uint32_t {
    kIndex1 = 0,
    kIndex2 = 1,
    kIndex4 = 2,
    kIndex8 = 3,
    kIndex16 = 4,
    kIndex32 = 5,
    kIndex64 = 6,
    kIndexCount = 7
  };

  //! Arena-allocated const-pool gap created by two differently aligned constants.
  struct Gap {
    //! Pointer to the next gap
    Gap* _next;
    //! Offset of the gap.
    size_t _offset;
    //! Remaining bytes of the gap (basically a gap size).
    size_t _size;
  };

  //! Arena-allocated const-pool node.
  class Node : public ArenaTreeNodeT<Node> {
  public:
    ASMJIT_NONCOPYABLE(Node)

    //! If this constant is shared with another.
    uint32_t _shared : 1;
    //! Data offset from the beginning of the pool.
    uint32_t _offset;

    ASMJIT_INLINE_NODEBUG Node(size_t offset, bool shared) noexcept
      : ArenaTreeNodeT<Node>(),
        _shared(shared),
        _offset(uint32_t(offset)) {}

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG void* data() noexcept { return Support::offset_ptr<void>(this, sizeof(*this)); }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG const void* data() const noexcept { return Support::offset_ptr<void>(this, sizeof(*this)); }
  };

  //! Data comparer used internally.
  class Compare {
  public:
    size_t _data_size;

    ASMJIT_INLINE_NODEBUG Compare(size_t data_size) noexcept
      : _data_size(data_size) {}

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG int operator()(const Node& a, const Node& b) const noexcept {
      return ::memcmp(a.data(), b.data(), _data_size);
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG int operator()(const Node& a, const void* data) const noexcept {
      return ::memcmp(a.data(), data, _data_size);
    }
  };

  //! Arena-allocated const-pool tree.
  struct Tree {
    //! RB tree.
    ArenaTree<Node> _tree;
    //! Size of the tree (number of nodes).
    size_t _size;
    //! Size of the data.
    size_t _data_size;

    ASMJIT_INLINE_NODEBUG explicit Tree(size_t data_size = 0) noexcept
      : _tree(),
        _size(0),
        _data_size(data_size) {}

    ASMJIT_INLINE_NODEBUG void reset() noexcept {
      _tree.reset();
      _size = 0;
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }

    inline void set_data_size(size_t data_size) noexcept {
      ASMJIT_ASSERT(is_empty());
      _data_size = data_size;
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG Node* get(const void* data) noexcept {
      Compare cmp(_data_size);
      return _tree.get(data, cmp);
    }

    ASMJIT_INLINE_NODEBUG void insert(Node* node) noexcept {
      Compare cmp(_data_size);
      _tree.insert(node, cmp);
      _size++;
    }

    template<typename Visitor>
    inline void for_each(Visitor& visitor) const noexcept {
      Node* node = _tree.root();
      if (!node) return;

      Node* stack[Globals::kMaxTreeHeight];
      size_t top = 0;

      for (;;) {
        Node* left = node->left();
        if (left != nullptr) {
          ASMJIT_ASSERT(top != Globals::kMaxTreeHeight);
          stack[top++] = node;

          node = left;
          continue;
        }

        for (;;) {
          visitor(node);
          node = node->right();

          if (node != nullptr)
            break;

          if (top == 0)
            return;

          node = stack[--top];
        }
      }
    }

    [[nodiscard]]
    static inline Node* new_node_t(Arena& arena, const void* data, size_t size, size_t offset, bool shared) noexcept {
      Node* node = arena.alloc_oneshot<Node>(Arena::aligned_size(sizeof(Node) + size));

      if (ASMJIT_UNLIKELY(!node)) {
        return nullptr;
      }

      node = new(Support::PlacementNew{node}) Node(offset, shared);
      memcpy(node->data(), data, size);
      return node;
    }
  };

  //! \endcond

  //! \name Members
  //! \{

  //! Arena.
  Arena& _arena;
  //! Tree per size.
  Tree _tree[kIndexCount];
  //! Gaps per size.
  Gap* _gaps[kIndexCount];
  //! Gaps pool
  Gap* _gap_pool;

  //! Size of the pool (in bytes).
  size_t _size;
  //! Required pool alignment.
  size_t _alignment;
  //! Minimum item size in the pool.
  size_t _min_item_size;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new constant pool that would use `arena` as a memory allocator.
  ASMJIT_API explicit ConstPool(Arena& arena) noexcept;
  //! Destroys this constant pool.
  ASMJIT_API ~ConstPool() noexcept;

  //! \}

  //! \name Reset
  //! \{

  //! Resets this constant pool.
  ASMJIT_API void reset() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the constant-pool is empty.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }

  //! Returns the size of the constant-pool in bytes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }

  //! Returns minimum alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t alignment() const noexcept { return _alignment; }

  //! Returns the minimum size of all items added to the constant pool.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t min_item_size() const noexcept { return _min_item_size; }

  //! \}

  //! \name Utilities
  //! \{

  //! Adds a constant to the constant pool.
  //!
  //! The constant must have known size, which is 1, 2, 4, 8, 16 or 32 bytes. The constant is added to the pool only
  //! if it doesn't not exist, otherwise cached value is returned.
  //!
  //! AsmJit is able to subdivide added constants, so for example if you add 8-byte constant 0x1122334455667788 it
  //! will create the following slots:
  //!
  //!   8-byte: 0x1122334455667788
  //!   4-byte: 0x11223344, 0x55667788
  //!
  //! The reason is that when combining MMX/SSE/AVX code some patterns are used frequently. However, AsmJit is not
  //! able to reallocate a constant that has been already added. For example if you try to add 4-byte constant and
  //! then 8-byte constant having the same 4-byte pattern as the previous one, two independent slots will be used.
  ASMJIT_API Error add(const void* data, size_t size, Out<size_t> offset_out) noexcept;

  //! Fills the destination with the content of this constant pool.
  ASMJIT_API void fill(void* dst) const noexcept;
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_CONSTPOOL_H_INCLUDED
