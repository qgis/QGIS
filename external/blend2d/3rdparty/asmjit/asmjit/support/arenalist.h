// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_SUPPORT_ARENALIST_H_INCLUDED
#define ASMJIT_SUPPORT_ARENALIST_H_INCLUDED

#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_support
//! \{

//! Node used by \ref ArenaList template.
template<typename NodeT>
class ArenaListNode {
public:
  ASMJIT_NONCOPYABLE(ArenaListNode)

  //! \name Constants
  //! \{

  static inline constexpr size_t kNodeIndexPrev = 0;
  static inline constexpr size_t kNodeIndexNext = 1;

  //! \}

  //! \name Members
  //! \{

  NodeT* _list_nodes[2];

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ArenaListNode() noexcept
    : _list_nodes{nullptr, nullptr} {}

  ASMJIT_INLINE_NODEBUG ArenaListNode(ArenaListNode&& other) noexcept
    : _list_nodes{other._list_nodes[0], other._list_nodes[1]} {}

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_prev() const noexcept { return _list_nodes[kNodeIndexPrev] != nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_next() const noexcept { return _list_nodes[kNodeIndexNext] != nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* prev() const noexcept { return _list_nodes[kNodeIndexPrev]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* next() const noexcept { return _list_nodes[kNodeIndexNext]; }

  //! \}
};

//! Arena-allocated list container that uses nodes of `NodeT` type.
template <typename NodeT>
class ArenaList {
public:
  ASMJIT_NONCOPYABLE(ArenaList)

  //! \name Constants
  //! \{

  static inline constexpr size_t kNodeIndexFirst = 0;
  static inline constexpr size_t kNodeIndexLast = 1;

  //! \}

  //! \name Members
  //! \{

  NodeT* _nodes[2] {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ArenaList() noexcept {}

  ASMJIT_INLINE_NODEBUG ArenaList(ArenaList&& other) noexcept
    : _nodes { other._nodes[0], other._nodes[1] } {}

  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _nodes[0] = nullptr;
    _nodes[1] = nullptr;
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _nodes[0] == nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* first() const noexcept { return _nodes[kNodeIndexFirst]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeT* last() const noexcept { return _nodes[kNodeIndexLast]; }

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE_NODEBUG void swap(ArenaList& other) noexcept {
    std::swap(_nodes[0], other._nodes[0]);
    std::swap(_nodes[1], other._nodes[1]);
  }

  // Can be used to both append and prepend.
  inline void _add_node(NodeT* node, size_t dir) noexcept {
    NodeT* prev = _nodes[dir];

    node->_list_nodes[!dir] = prev;
    _nodes[dir] = node;
    if (prev) {
      prev->_list_nodes[dir] = node;
    }
    else {
      _nodes[!dir] = node;
    }
  }

  // Can be used to both append and prepend.
  inline void _insert_node(NodeT* ref, NodeT* node, size_t dir) noexcept {
    ASMJIT_ASSERT(ref != nullptr);

    NodeT* prev = ref;
    NodeT* next = ref->_list_nodes[dir];

    prev->_list_nodes[dir] = node;
    if (next) {
      next->_list_nodes[!dir] = node;
    }
    else {
      _nodes[dir] = node;
    }

    node->_list_nodes[!dir] = prev;
    node->_list_nodes[ dir] = next;
  }

  ASMJIT_INLINE_NODEBUG void append(NodeT* node) noexcept { _add_node(node, kNodeIndexLast); }
  ASMJIT_INLINE_NODEBUG void prepend(NodeT* node) noexcept { _add_node(node, kNodeIndexFirst); }

  ASMJIT_INLINE_NODEBUG void insert_after(NodeT* ref, NodeT* node) noexcept { _insert_node(ref, node, NodeT::kNodeIndexNext); }
  ASMJIT_INLINE_NODEBUG void insert_before(NodeT* ref, NodeT* node) noexcept { _insert_node(ref, node, NodeT::kNodeIndexPrev); }

  inline NodeT* unlink(NodeT* node) noexcept {
    NodeT* prev = node->prev();
    NodeT* next = node->next();

    if (prev) { prev->_list_nodes[1] = next; } else { _nodes[0] = next; }
    if (next) { next->_list_nodes[0] = prev; } else { _nodes[1] = prev; }

    node->_list_nodes[0] = nullptr;
    node->_list_nodes[1] = nullptr;

    return node;
  }

  [[nodiscard]]
  inline NodeT* pop_first() noexcept {
    NodeT* node = _nodes[0];
    ASMJIT_ASSERT(node != nullptr);

    NodeT* next = node->next();
    _nodes[0] = next;

    if (next) {
      next->_list_nodes[0] = nullptr;
      node->_list_nodes[1] = nullptr;
    }
    else {
      _nodes[1] = nullptr;
    }

    return node;
  }

  [[nodiscard]]
  inline NodeT* pop() noexcept {
    NodeT* node = _nodes[1];
    ASMJIT_ASSERT(node != nullptr);

    NodeT* prev = node->prev();
    _nodes[1] = prev;

    if (prev) {
      prev->_list_nodes[1] = nullptr;
      node->_list_nodes[0] = nullptr;
    }
    else {
      _nodes[0] = nullptr;
    }

    return node;
  }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_SUPPORT_ARENALIST_H_INCLUDED
