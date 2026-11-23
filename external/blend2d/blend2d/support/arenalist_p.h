// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_ARENALIST_P_H_INCLUDED
#define BLEND2D_SUPPORT_ARENALIST_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Arena Allocated ArenaList
//! \{

//! Arena-allocated double-linked list node.
template<typename NodeT>
class ArenaListNode {
public:
  union {
    NodeT* _list_nodes[2];
    struct {
      NodeT* _list_prev;
      NodeT* _list_next;
    };
  };

  BL_NONCOPYABLE(ArenaListNode)

  BL_INLINE ArenaListNode() noexcept
    : _list_nodes { nullptr, nullptr } {}

  BL_INLINE ArenaListNode(ArenaListNode&& other) noexcept
    : _list_nodes { other._list_nodes[0], other._list_nodes[1] } {}

  BL_INLINE bool has_prev() const noexcept { return _list_prev != nullptr; }
  BL_INLINE bool has_next() const noexcept { return _list_next != nullptr; }

  BL_INLINE NodeT* prev() const noexcept { return _list_prev; }
  BL_INLINE NodeT* next() const noexcept { return _list_next; }
};

//! Arena-allocated double-linked list container.
template <typename NodeT>
class ArenaList {
public:
  BL_NONCOPYABLE(ArenaList)

  NodeT* _nodes[2];

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ArenaList() noexcept
    : _nodes { nullptr, nullptr } {}

  BL_INLINE ArenaList(ArenaList&& other) noexcept
    : _nodes { other._nodes[0], other._nodes[1] } {}

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE void swap(ArenaList& other) noexcept {
    BLInternal::swap(_nodes[0], other._nodes[0]);
    BLInternal::swap(_nodes[1], other._nodes[1]);
  }

  BL_INLINE void reset() noexcept {
    _nodes[0] = nullptr;
    _nodes[1] = nullptr;
  }

  BL_INLINE void reset(NodeT* node) noexcept {
    node->_list_nodes[0] = nullptr;
    node->_list_nodes[1] = nullptr;
    _nodes[0] = node;
    _nodes[1] = node;
  }

  //! \}

  //! \name ArenaList Functionality
  //! \{

  // Can be used to both prepend and append.
  BL_INLINE void _add_node(NodeT* node, size_t dir) noexcept {
    NodeT* prev = _nodes[dir];

    node->_list_nodes[!dir] = prev;
    _nodes[dir] = node;
    if (prev)
      prev->_list_nodes[dir] = node;
    else
      _nodes[!dir] = node;
  }

  // Can be used to both prepend and append.
  BL_INLINE void _insert_node(NodeT* ref, NodeT* node, size_t dir) noexcept {
    BL_ASSERT(ref != nullptr);

    NodeT* prev = ref;
    NodeT* next = ref->_list_nodes[dir];

    prev->_list_nodes[dir] = node;
    if (next)
      next->_list_nodes[!dir] = node;
    else
      _nodes[dir] = node;

    node->_list_nodes[!dir] = prev;
    node->_list_nodes[ dir] = next;
  }

  BL_INLINE void append(NodeT* node) noexcept { _add_node(node, 1); }
  BL_INLINE void prepend(NodeT* node) noexcept { _add_node(node, 0); }

  BL_INLINE void insert_after(NodeT* ref, NodeT* node) noexcept { _insert_node(ref, node, 1); }
  BL_INLINE void insert_before(NodeT* ref, NodeT* node) noexcept { _insert_node(ref, node, 0); }

  BL_INLINE NodeT* unlink(NodeT* node) noexcept {
    NodeT* prev = node->prev();
    NodeT* next = node->next();

    if (prev) { prev->_list_next = next; } else { _nodes[0] = next; }
    if (next) { next->_list_prev = prev; } else { _nodes[1] = prev; }

    node->_list_prev = nullptr;
    node->_list_next = nullptr;

    return node;
  }

  BL_INLINE NodeT* pop_first() noexcept {
    NodeT* node = _nodes[0];
    BL_ASSERT(node != nullptr);

    NodeT* next = node->next();
    _nodes[0] = next;

    if (next) {
      next->_list_prev = nullptr;
      node->_list_next = nullptr;
    }
    else {
      _nodes[1] = nullptr;
    }

    return node;
  }

  BL_INLINE NodeT* pop() noexcept {
    NodeT* node = _nodes[1];
    BL_ASSERT(node != nullptr);

    NodeT* prev = node->prev();
    _nodes[1] = prev;

    if (prev) {
      prev->_list_next = nullptr;
      node->_list_prev = nullptr;
    }
    else {
      _nodes[0] = nullptr;
    }

    return node;
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE bool is_empty() const noexcept { return _nodes[0] == nullptr; }
  BL_INLINE NodeT* first() const noexcept { return _nodes[0]; }
  BL_INLINE NodeT* last() const noexcept { return _nodes[1]; }

  //! \}
};

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_ARENALIST_P_H_INCLUDED
