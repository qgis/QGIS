// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_ARENATREE_P_H_INCLUDED
#define BLEND2D_SUPPORT_ARENATREE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/support/algorithm_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Arena Allocated Tree
//! \{

//! ArenaTree node (base class).
//!
//! The color is stored in a least significant bit of the `left` node.
//!
//! \note Always use accessors to access left and right nodes.
class ArenaTreeNodeBase {
public:
  BL_NONCOPYABLE(ArenaTreeNodeBase)

  //! \name Constants
  //! \{

  static constexpr const uintptr_t kRedMask = 0x1;
  static constexpr const uintptr_t kPtrMask = ~kRedMask;

  //! \}

  //! \name Members
  //! \{

  uintptr_t _tree_nodes[2];

  //! \}

  BL_INLINE ArenaTreeNodeBase() noexcept
    : _tree_nodes { 0, 0 } {}

  //! \name Accessors
  //! \{

  BL_INLINE bool has_child(size_t i) const noexcept { return _tree_nodes[i] > kRedMask; }
  BL_INLINE bool has_left() const noexcept { return _tree_nodes[0] > kRedMask; }
  BL_INLINE bool has_right() const noexcept { return _tree_nodes[1] != 0; }

  BL_INLINE ArenaTreeNodeBase* _get_child(size_t i) const noexcept { return (ArenaTreeNodeBase*)(_tree_nodes[i] & kPtrMask); }
  BL_INLINE ArenaTreeNodeBase* _get_left() const noexcept { return (ArenaTreeNodeBase*)(_tree_nodes[0] & kPtrMask); }
  BL_INLINE ArenaTreeNodeBase* _get_right() const noexcept { return (ArenaTreeNodeBase*)(_tree_nodes[1]); }

  BL_INLINE void _set_child(size_t i, ArenaTreeNodeBase* node) noexcept { _tree_nodes[i] = (_tree_nodes[i] & kRedMask) | (uintptr_t)node; }
  BL_INLINE void _set_left(ArenaTreeNodeBase* node) noexcept { _tree_nodes[0] = (_tree_nodes[0] & kRedMask) | (uintptr_t)node; }
  BL_INLINE void _set_right(ArenaTreeNodeBase* node) noexcept { _tree_nodes[1] = (uintptr_t)node; }

  template<typename T = ArenaTreeNodeBase>
  BL_INLINE T* child(size_t i) const noexcept { return static_cast<T*>(_get_child(i)); }
  template<typename T = ArenaTreeNodeBase>
  BL_INLINE T* left() const noexcept { return static_cast<T*>(_get_left()); }
  template<typename T = ArenaTreeNodeBase>
  BL_INLINE T* right() const noexcept { return static_cast<T*>(_get_right()); }

  BL_INLINE bool is_red() const noexcept { return static_cast<bool>(_tree_nodes[0] & kRedMask); }
  BL_INLINE void _make_red() noexcept { _tree_nodes[0] |= kRedMask; }
  BL_INLINE void _make_black() noexcept { _tree_nodes[0] &= kPtrMask; }

  //! \}

  //! Tests whether the node is RED (RED node must be non-null and must have RED flag set).
  static BL_INLINE bool _is_valid_red(ArenaTreeNodeBase* node) noexcept { return node && node->is_red(); }
};

//! ArenaTree node.
template<typename NodeT>
class ArenaTreeNode : public ArenaTreeNodeBase {
public:
  BL_NONCOPYABLE(ArenaTreeNode)

  BL_INLINE ArenaTreeNode() noexcept
    : ArenaTreeNodeBase() {}

  //! \name Accessors
  //! \{

  BL_INLINE NodeT* child(size_t i) const noexcept { return static_cast<NodeT*>(_get_child(i)); }
  BL_INLINE NodeT* left() const noexcept { return static_cast<NodeT*>(_get_left()); }
  BL_INLINE NodeT* right() const noexcept { return static_cast<NodeT*>(_get_right()); }

  //! \}
};

//! A red-black tree that uses nodes allocated by `ArenaAllocator`.
template<typename NodeT>
class ArenaTree {
public:
  BL_NONCOPYABLE(ArenaTree)

  typedef NodeT Node;
  NodeT* _root;

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ArenaTree() noexcept
    : _root(nullptr) {}

  BL_INLINE ArenaTree(ArenaTree&& other) noexcept
    : _root(other._root) {}

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE void swap(ArenaTree& other) noexcept {
    BLInternal::swap(_root, other._root);
  }

  BL_INLINE void reset() noexcept { _root = nullptr; }

  //! \}

  //! \name ArenaTree Functionality
  //! \{

  //! Insert a node into the tree.
  template<typename CompareT = CompareOp<SortOrder::kAscending>>
  void insert(NodeT* node, const CompareT& cmp = CompareT()) noexcept {
    // Node to insert must not contain garbage.
    BL_ASSERT(!node->has_left());
    BL_ASSERT(!node->has_right());
    BL_ASSERT(!node->is_red());

    if (!_root) {
      _root = node;
      return;
    }

    ArenaTreeNodeBase head;          // False root node,
    head._set_right(_root);           // having root on the right.

    ArenaTreeNodeBase* g = nullptr;  // Grandparent.
    ArenaTreeNodeBase* p = nullptr;  // Parent.
    ArenaTreeNodeBase* t = &head;    // Iterator.
    ArenaTreeNodeBase* q = _root;    // Query.

    size_t dir = 0;                  // Direction for accessing child nodes.
    size_t last = 0;                 // Not needed to initialize, but makes some tools happy.
    node->_make_red();                // New nodes are always red and violations fixed appropriately.

    // Search down the tree.
    for (;;) {
      if (!q) {
        // Insert new node at the bottom.
        q = node;
        p->_set_child(dir, node);
      }
      else if (_is_valid_red(q->_get_left()) && _is_valid_red(q->_get_right())) {
        // Color flip.
        q->_make_red();
        q->_get_left()->_make_black();
        q->_get_right()->_make_black();
      }

      // Fix red violation.
      if (_is_valid_red(q) && _is_valid_red(p))
        t->_set_child(t->_get_right() == g, q == p->_get_child(last) ? _single_rotate(g, !last) : _double_rotate(g, !last));

      // Stop if found.
      if (q == node)
        break;

      last = dir;
      dir = cmp(*static_cast<NodeT*>(q), *static_cast<NodeT*>(node)) < 0;

      // Update helpers.
      if (g) t = g;

      g = p;
      p = q;
      q = q->_get_child(dir);
    }

    // Update root and make it black.
    _root = static_cast<NodeT*>(head._get_right());
    _root->_make_black();
  }

  //! Remove a node from the tree.
  template<typename CompareT = CompareOp<SortOrder::kAscending>>
  void remove(ArenaTreeNodeBase* node, const CompareT& cmp = CompareT()) noexcept {
    ArenaTreeNodeBase head;          // False root node,
    head._set_right(_root);           // having root on the right.

    ArenaTreeNodeBase* g = nullptr;  // Grandparent.
    ArenaTreeNodeBase* p = nullptr;  // Parent.
    ArenaTreeNodeBase* q = &head;    // Query.

    ArenaTreeNodeBase* f  = nullptr; // Found item.
    ArenaTreeNodeBase* gf = nullptr; // Found grandparent.
    size_t dir = 1;                  // Direction (0 or 1).

    // Search and push a red down.
    while (q->has_child(dir)) {
      size_t last = dir;

      // Update helpers.
      g = p;
      p = q;
      q = q->_get_child(dir);
      dir = cmp(*static_cast<NodeT*>(q), *static_cast<NodeT*>(node)) < 0;

      // Save found node.
      if (q == node) {
        f = q;
        gf = g;
      }

      // Push the red node down.
      if (!_is_valid_red(q) && !_is_valid_red(q->_get_child(dir))) {
        if (_is_valid_red(q->_get_child(!dir))) {
          ArenaTreeNodeBase* child = _single_rotate(q, dir);
          p->_set_child(last, child);
          p = child;
        }
        else if (!_is_valid_red(q->_get_child(!dir)) && p->_get_child(!last)) {
          ArenaTreeNodeBase* s = p->_get_child(!last);
          if (!_is_valid_red(s->_get_child(!last)) && !_is_valid_red(s->_get_child(last))) {
            // Color flip.
            p->_make_black();
            s->_make_red();
            q->_make_red();
          }
          else {
            size_t dir2 = g->_get_right() == p;
            ArenaTreeNodeBase* child = g->_get_child(dir2);

            if (_is_valid_red(s->_get_child(last))) {
              child = _double_rotate(p, last);
              g->_set_child(dir2, child);
            }
            else if (_is_valid_red(s->_get_child(!last))) {
              child = _single_rotate(p, last);
              g->_set_child(dir2, child);
            }

            // Ensure correct coloring.
            q->_make_red();
            child->_make_red();
            child->_get_left()->_make_black();
            child->_get_right()->_make_black();
          }
        }
      }
    }

    // Replace and remove.
    BL_ASSERT(f != nullptr);
    BL_ASSERT(f != &head);
    BL_ASSERT(q != &head);

    p->_set_child(p->_get_right() == q, q->_get_child(q->_get_left() == nullptr));

    // NOTE: The original algorithm used a trick to just copy 'key/value' to `f` and mark `q` for deletion. But
    // this is unacceptable here as we really want to destroy the passed `node`. So, we have to make sure that
    // we have really removed `f` and not `q`.
    if (f != q) {
      BL_ASSERT(f != &head);
      BL_ASSERT(f != gf);

      ArenaTreeNodeBase* n = gf ? gf : &head;
      dir = (n == &head) ? 1  : cmp(*static_cast<NodeT*>(n), *static_cast<NodeT*>(node)) < 0;

      for (;;) {
        if (n->_get_child(dir) == f) {
          n->_set_child(dir, q);
          // RAW copy, including the color.
          q->_tree_nodes[0] = f->_tree_nodes[0];
          q->_tree_nodes[1] = f->_tree_nodes[1];
          break;
        }

        n = n->_get_child(dir);

        // Cannot be true as we know that it must reach `f` in few iterations.
        BL_ASSERT(n != nullptr);
        dir = cmp(*static_cast<NodeT*>(n), *static_cast<NodeT*>(node)) < 0;
      }
    }

    // Update root and make it black.
    _root = static_cast<NodeT*>(head._get_right());
    if (_root) _root->_make_black();
  }

  template<typename KeyT, typename CompareT = CompareOp<SortOrder::kAscending>>
  BL_INLINE NodeT* get(const KeyT& key, const CompareT& cmp = CompareT()) const noexcept {
    ArenaTreeNodeBase* node = _root;
    while (node) {
      auto result = cmp(*static_cast<const NodeT*>(node), key);
      if (result == 0) break;

      // Go left or right depending on the `result`.
      node = node->_get_child(result < 0);
    }
    return static_cast<NodeT*>(node);
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE bool is_empty() const noexcept { return _root == nullptr; }
  BL_INLINE NodeT* root() const noexcept { return static_cast<NodeT*>(_root); }

  //! \}

  //! \name Internals
  //! \{

  static BL_INLINE bool _is_valid_red(ArenaTreeNodeBase* node) noexcept { return ArenaTreeNodeBase::_is_valid_red(node); }

  //! Single rotation.
  static BL_INLINE ArenaTreeNodeBase* _single_rotate(ArenaTreeNodeBase* root, size_t dir) noexcept {
    ArenaTreeNodeBase* save = root->_get_child(!dir);
    root->_set_child(!dir, save->_get_child(dir));
    save->_set_child( dir, root);
    root->_make_red();
    save->_make_black();
    return save;
  }

  //! Double rotation.
  static BL_INLINE ArenaTreeNodeBase* _double_rotate(ArenaTreeNodeBase* root, size_t dir) noexcept {
    root->_set_child(!dir, _single_rotate(root->_get_child(!dir), !dir));
    return _single_rotate(root, dir);
  }

  //! \}
};

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_ARENATREE_P_H_INCLUDED
