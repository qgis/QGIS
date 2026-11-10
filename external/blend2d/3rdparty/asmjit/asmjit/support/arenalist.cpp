// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenalist.h>

ASMJIT_BEGIN_NAMESPACE

// ArenaList - Tests
// ================

#if defined(ASMJIT_TEST)
class MyListNode : public ArenaListNode<MyListNode> {};

UNIT(arena_list) {
  Arena arena(4096);
  ArenaList<MyListNode> list;

  MyListNode* a = arena.new_oneshot<MyListNode>();
  MyListNode* b = arena.new_oneshot<MyListNode>();
  MyListNode* c = arena.new_oneshot<MyListNode>();
  MyListNode* d = arena.new_oneshot<MyListNode>();

  INFO("Append / Unlink");

  // []
  EXPECT_TRUE(list.is_empty());

  // [A]
  list.append(a);
  EXPECT_FALSE(list.is_empty());
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), a);
  EXPECT_NULL(a->prev());
  EXPECT_NULL(a->next());

  // [A, B]
  list.append(b);
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), b);
  EXPECT_NULL(a->prev());
  EXPECT_EQ(a->next(), b);
  EXPECT_EQ(b->prev(), a);
  EXPECT_NULL(b->next());

  // [A, B, C]
  list.append(c);
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), c);
  EXPECT_NULL(a->prev());
  EXPECT_EQ(a->next(), b);
  EXPECT_EQ(b->prev(), a);
  EXPECT_EQ(b->next(), c);
  EXPECT_EQ(c->prev(), b);
  EXPECT_NULL(c->next());

  // [B, C]
  list.unlink(a);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), c);
  EXPECT_NULL(a->prev());
  EXPECT_NULL(a->next());
  EXPECT_NULL(b->prev());
  EXPECT_EQ(b->next(), c);
  EXPECT_EQ(c->prev(), b);
  EXPECT_NULL(c->next());

  // [B]
  list.unlink(c);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), b);
  EXPECT_NULL(b->prev());
  EXPECT_NULL(b->next());
  EXPECT_NULL(c->prev());
  EXPECT_NULL(c->next());

  // []
  list.unlink(b);
  EXPECT_TRUE(list.is_empty());
  EXPECT_NULL(list.first());
  EXPECT_NULL(list.last());
  EXPECT_NULL(b->prev());
  EXPECT_NULL(b->next());

  INFO("Prepend / Unlink");

  // [A]
  list.prepend(a);
  EXPECT_FALSE(list.is_empty());
  EXPECT_EQ(list.first(), a);
  EXPECT_EQ(list.last(), a);
  EXPECT_NULL(a->prev());
  EXPECT_NULL(a->next());

  // [B, A]
  list.prepend(b);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), a);
  EXPECT_NULL(b->prev());
  EXPECT_EQ(b->next(), a);
  EXPECT_EQ(a->prev(), b);
  EXPECT_NULL(a->next());

  INFO("InsertAfter / InsertBefore");

  // [B, A, C]
  list.insert_after(a, c);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), c);
  EXPECT_NULL(b->prev());
  EXPECT_EQ(b->next(), a);
  EXPECT_EQ(a->prev(), b);
  EXPECT_EQ(a->next(), c);
  EXPECT_EQ(c->prev(), a);
  EXPECT_NULL(c->next());

  // [B, D, A, C]
  list.insert_before(a, d);
  EXPECT_EQ(list.first(), b);
  EXPECT_EQ(list.last(), c);
  EXPECT_NULL(b->prev());
  EXPECT_EQ(b->next(), d);
  EXPECT_EQ(d->prev(), b);
  EXPECT_EQ(d->next(), a);
  EXPECT_EQ(a->prev(), d);
  EXPECT_EQ(a->next(), c);
  EXPECT_EQ(c->prev(), a);
  EXPECT_NULL(c->next());

  INFO("PopFirst / Pop");

  // [D, A, C]
  EXPECT_EQ(list.pop_first(), b);
  EXPECT_NULL(b->prev());
  EXPECT_NULL(b->next());

  EXPECT_EQ(list.first(), d);
  EXPECT_EQ(list.last(), c);
  EXPECT_NULL(d->prev());
  EXPECT_EQ(d->next(), a);
  EXPECT_EQ(a->prev(), d);
  EXPECT_EQ(a->next(), c);
  EXPECT_EQ(c->prev(), a);
  EXPECT_NULL(c->next());

  // [D, A]
  EXPECT_EQ(list.pop(), c);
  EXPECT_NULL(c->prev());
  EXPECT_NULL(c->next());

  EXPECT_EQ(list.first(), d);
  EXPECT_EQ(list.last(), a);
  EXPECT_NULL(d->prev());
  EXPECT_EQ(d->next(), a);
  EXPECT_EQ(a->prev(), d);
  EXPECT_NULL(a->next());
}
#endif

ASMJIT_END_NAMESPACE
