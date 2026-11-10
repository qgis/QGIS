// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenatree.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// ArenaTreeBase - Tests
// ====================

#if defined(ASMJIT_TEST)
template<typename NodeT>
struct ArenaRBUnit {
  using Tree = ArenaTree<NodeT>;

  static void verify_tree(Tree& tree) noexcept {
    EXPECT_GT(check_height(static_cast<NodeT*>(tree._root)), 0);
  }

  // Check whether the Red-Black tree is valid.
  static int check_height(NodeT* node) noexcept {
    if (!node) return 1;

    NodeT* ln = node->left();
    NodeT* rn = node->right();

    // Invalid tree.
    EXPECT_TRUE(ln == nullptr || *ln < *node);
    EXPECT_TRUE(rn == nullptr || *rn > *node);

    // Red violation.
    EXPECT_TRUE(!node->is_red() || (!ArenaTreeNode::_is_valid_red(ln) && !ArenaTreeNode::_is_valid_red(rn)));

    // Black violation.
    int lh = check_height(ln);
    int rh = check_height(rn);
    EXPECT_TRUE(!lh || !rh || lh == rh);

    // Only count black links.
    return (lh && rh) ? lh + !node->is_red() : 0;
  }
};

class MyRBNode : public ArenaTreeNodeT<MyRBNode> {
public:
  ASMJIT_NONCOPYABLE(MyRBNode)

  inline explicit MyRBNode(uint32_t key) noexcept
    : _key(key) {}

  inline bool operator<(const MyRBNode& other) const noexcept { return _key < other._key; }
  inline bool operator>(const MyRBNode& other) const noexcept { return _key > other._key; }

  inline bool operator<(uint32_t query_key) const noexcept { return _key < query_key; }
  inline bool operator>(uint32_t query_key) const noexcept { return _key > query_key; }

  uint32_t _key;
};

UNIT(arena_rbtree) {
  uint32_t kCount = BrokenAPI::has_arg("--quick") ? 1000 : 10000;

  Arena arena(4096);
  ArenaTree<MyRBNode> rb_tree;

  uint32_t key;
  INFO("Inserting %u elements to RBTree and validating each operation", unsigned(kCount));
  for (key = 0; key < kCount; key++) {
    rb_tree.insert(arena.new_oneshot<MyRBNode>(key));
    ArenaRBUnit<MyRBNode>::verify_tree(rb_tree);
  }

  uint32_t count = kCount;
  INFO("Removing %u elements from RBTree and validating each operation", unsigned(kCount));
  do {
    MyRBNode* node;

    for (key = 0; key < count; key++) {
      node = rb_tree.get(key);
      EXPECT_NOT_NULL(node);
      EXPECT_EQ(node->_key, key);
    }

    node = rb_tree.get(--count);
    rb_tree.remove(node);
    ArenaRBUnit<MyRBNode>::verify_tree(rb_tree);
  } while (count);

  EXPECT_TRUE(rb_tree.is_empty());
}
#endif

ASMJIT_END_NAMESPACE
