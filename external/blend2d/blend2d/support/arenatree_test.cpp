// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenatree_p.h>

// bl::ArenaTree - Tests
// =====================

namespace bl {
namespace Tests {

template<typename NodeT>
struct TreeTestUtils {
  typedef ArenaTree<NodeT> Tree;

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
    EXPECT_TRUE(!node->is_red() || (!ArenaTreeNodeBase::_is_valid_red(ln) && !ArenaTreeNodeBase::_is_valid_red(rn)));

    // Black violation.
    int lh = check_height(ln);
    int rh = check_height(rn);
    EXPECT_TRUE(!lh || !rh || lh == rh);

    // Only count black links.
    return (lh && rh) ? lh + !node->is_red() : 0;
  }
};

class MyTreeNode : public ArenaTreeNode<MyTreeNode> {
public:
  BL_NONCOPYABLE(MyTreeNode)

  BL_INLINE explicit MyTreeNode(uint32_t key) noexcept
    : _key(key) {}

  BL_INLINE bool operator<(const MyTreeNode& other) const noexcept { return _key < other._key; }
  BL_INLINE bool operator>(const MyTreeNode& other) const noexcept { return _key > other._key; }

  BL_INLINE bool operator<(uint32_t query_key) const noexcept { return _key < query_key; }
  BL_INLINE bool operator>(uint32_t query_key) const noexcept { return _key > query_key; }

  uint32_t _key;
};

UNIT(arena_tree, BL_TEST_GROUP_SUPPORT_CONTAINERS) {
  constexpr uint32_t kCount = 2000;

  ArenaAllocator zone(4096);
  ArenaTree<MyTreeNode> rb_tree;

  uint32_t key;
  INFO("Inserting %u elements to the tree and validating each operation", unsigned(kCount));
  for (key = 0; key < kCount; key++) {
    rb_tree.insert(zone.new_t<MyTreeNode>(key));
    TreeTestUtils<MyTreeNode>::verify_tree(rb_tree);
  }

  uint32_t count = kCount;
  INFO("Removing %u elements from the tree and validating each operation", unsigned(kCount));
  do {
    MyTreeNode* node;

    for (key = 0; key < count; key++) {
      node = rb_tree.get(key);
      EXPECT_NE(node, nullptr);
      EXPECT_EQ(node->_key, key);
    }

    node = rb_tree.get(--count);
    rb_tree.remove(node);
    TreeTestUtils<MyTreeNode>::verify_tree(rb_tree);
  } while (count);

  EXPECT_TRUE(rb_tree.is_empty());
}

} // {Tests}
} // {bl}

#endif // BL_TEST
