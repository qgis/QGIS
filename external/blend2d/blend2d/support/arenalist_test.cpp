// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenalist_p.h>

// bl::ArenaList - Tests
// =====================

namespace bl {
namespace Tests {

class MyListNode : public ArenaListNode<MyListNode> {};

UNIT(arena_list, BL_TEST_GROUP_SUPPORT_CONTAINERS) {
  ArenaAllocator zone(4096);
  ArenaList<MyListNode> list;

  MyListNode* a = zone.new_t<MyListNode>();
  MyListNode* b = zone.new_t<MyListNode>();
  MyListNode* c = zone.new_t<MyListNode>();
  MyListNode* d = zone.new_t<MyListNode>();

  INFO("Append / Unlink");

  // []
  EXPECT_TRUE(list.is_empty());

  // [A]
  list.append(a);
  EXPECT_FALSE(list.is_empty());
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), a);
  EXPECT_EQ(a->prev(), nullptr);
  EXPECT_EQ(a->next(), nullptr);

  // [A, B]
  list.append(b);
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), b);
  EXPECT_EQ(a->prev(), nullptr);
  EXPECT_EQ(a->next(), b);
  EXPECT_EQ(b->prev(), a);
  EXPECT_EQ(b->next(), nullptr);

  // [A, B, C]
  list.append(c);
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), c);
  EXPECT_EQ(a->prev(), nullptr);
  EXPECT_EQ(a->next(), b);
  EXPECT_EQ(b->prev(), a);
  EXPECT_EQ(b->next(), c);
  EXPECT_EQ(c->prev(), b);
  EXPECT_EQ(c->next(), nullptr);

  // [B, C]
  list.unlink(a);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), c);
  EXPECT_EQ(a->prev(), nullptr);
  EXPECT_EQ(a->next(), nullptr);
  EXPECT_EQ(b->prev(), nullptr);
  EXPECT_EQ(b->next(), c);
  EXPECT_EQ(c->prev(), b);
  EXPECT_EQ(c->next(), nullptr);

  // [B]
  list.unlink(c);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), b);
  EXPECT_EQ(b->prev(), nullptr);
  EXPECT_EQ(b->next(), nullptr);
  EXPECT_EQ(c->prev(), nullptr);
  EXPECT_EQ(c->next(), nullptr);

  // []
  list.unlink(b);
  EXPECT_TRUE(list.is_empty());
  EXPECT_EQ(list.first(), nullptr);
  EXPECT_EQ(list.last(), nullptr);
  EXPECT_EQ(b->prev(), nullptr);
  EXPECT_EQ(b->next(), nullptr);

  INFO("Prepend / Unlink");

  // [A]
  list.prepend(a);
  EXPECT_FALSE(list.is_empty());
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), a);
  EXPECT_EQ(a->prev(), nullptr);
  EXPECT_EQ(a->next(), nullptr);

  // [B, A]
  list.prepend(b);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), a);
  EXPECT_EQ(b->prev(), nullptr);
  EXPECT_EQ(b->next(), a);
  EXPECT_EQ(a->prev(), b);
  EXPECT_EQ(a->next(), nullptr);

  INFO("InsertAfter / InsertBefore");

  // [B, A, C]
  list.insert_after(a, c);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), c);
  EXPECT_EQ(b->prev(), nullptr);
  EXPECT_EQ(b->next(), a);
  EXPECT_EQ(a->prev(), b);
  EXPECT_EQ(a->next(), c);
  EXPECT_EQ(c->prev(), a);
  EXPECT_EQ(c->next(), nullptr);

  // [B, D, A, C]
  list.insert_before(a, d);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), c);
  EXPECT_EQ(b->prev(), nullptr);
  EXPECT_EQ(b->next(), d);
  EXPECT_EQ(d->prev(), b);
  EXPECT_EQ(d->next(), a);
  EXPECT_EQ(a->prev(), d);
  EXPECT_EQ(a->next(), c);
  EXPECT_EQ(c->prev(), a);
  EXPECT_EQ(c->next(), nullptr);

  INFO("PopFirst / Pop");

  // [D, A, C]
  EXPECT_EQ(list.pop_first(), b);
  EXPECT_EQ(b->prev(), nullptr);
  EXPECT_EQ(b->next(), nullptr);

  EXPECT_EQ(list.first(), d);
  EXPECT_EQ(list.last(), c);
  EXPECT_EQ(d->prev(), nullptr);
  EXPECT_EQ(d->next(), a);
  EXPECT_EQ(a->prev(), d);
  EXPECT_EQ(a->next(), c);
  EXPECT_EQ(c->prev(), a);
  EXPECT_EQ(c->next(), nullptr);

  // [D, A]
  EXPECT_EQ(list.pop(), c);
  EXPECT_EQ(c->prev(), nullptr);
  EXPECT_EQ(c->next(), nullptr);

  EXPECT_EQ(list.first(), d);
  EXPECT_EQ(list.last(), a);
  EXPECT_EQ(d->prev(), nullptr);
  EXPECT_EQ(d->next(), a);
  EXPECT_EQ(a->prev(), d);
  EXPECT_EQ(a->next(), nullptr);
}

} // {Tests}
} // {bl}

#endif // BL_TEST
