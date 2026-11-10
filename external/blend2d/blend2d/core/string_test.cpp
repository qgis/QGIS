// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/array_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/core/runtime_p.h>

// bl::String - Tests
// ==================

namespace bl {
namespace Tests {

static void verify_string(const BLString& s) noexcept {
  size_t size = StringInternal::get_size(&s);
  const char* data = StringInternal::get_data(&s);

  EXPECT_EQ(data[size], 0)
    .message("BLString's data is not null terminated");

  if (s._d.sso())
    for (size_t i = size; i < BLString::kSSOCapacity; i++)
      EXPECT_EQ(data[i], 0)
        .message("BLString's SSO data is invalid - found non-null character at [%zu], after string size %zu", i, size);
}

UNIT(string_allocation_strategy, BL_TEST_GROUP_CORE_CONTAINERS) {
  BLString s;
  size_t kNumItems = 10000000;
  size_t capacity = s.capacity();

  for (size_t i = 0; i < kNumItems; i++) {
    char c = char(size_t('a') + (i % size_t('z' - 'a')));
    s.append(c);
    if (capacity != s.capacity()) {
      size_t impl_size = StringInternal::impl_size_from_capacity(s.capacity()).value();
      INFO("Capacity increased from %zu to %zu [ImplSize=%zu]\n", capacity, s.capacity(), impl_size);
      capacity = s.capacity();
    }
  }
}

UNIT(string, BL_TEST_GROUP_CORE_CONTAINERS) {
  INFO("SSO representation");
  {
    BLString s;

    for (uint32_t i = 0; i < BLString::kSSOCapacity; i++) {
      char c = char('a' + i);
      EXPECT_SUCCESS(s.append(c));
      EXPECT_TRUE(s._d.sso());
      EXPECT_EQ(s._d.char_data[i], c);
      verify_string(s);
    }
  }

  INFO("Assignment and comparison");
  {
    BLString s;

    EXPECT_SUCCESS(s.assign('b'));
    verify_string(s);
    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(s[0], 'b');
    EXPECT_TRUE(s.equals("b"   ));
    EXPECT_TRUE(s.equals("b", 1));
    EXPECT_GT(s.compare("a"    ), 0);
    EXPECT_GT(s.compare("a" , 1), 0);
    EXPECT_GT(s.compare("a?"   ), 0);
    EXPECT_GT(s.compare("a?", 2), 0);
    EXPECT_EQ(s.compare("b"    ), 0);
    EXPECT_EQ(s.compare("b" , 1), 0);
    EXPECT_LT(s.compare("b?"   ), 0);
    EXPECT_LT(s.compare("b?", 2), 0);
    EXPECT_LT(s.compare("c"    ), 0);
    EXPECT_LT(s.compare("c" , 1), 0);
    EXPECT_LT(s.compare("c?"   ), 0);
    EXPECT_LT(s.compare("c?", 2), 0);

    EXPECT_SUCCESS(s.assign('b', 4));
    verify_string(s);
    EXPECT_EQ(s.size(), 4u);
    EXPECT_EQ(s[0], 'b');
    EXPECT_EQ(s[1], 'b');
    EXPECT_EQ(s[2], 'b');
    EXPECT_EQ(s[3], 'b');
    EXPECT_TRUE(s.equals("bbbb"   ));
    EXPECT_TRUE(s.equals("bbbb", 4));
    EXPECT_EQ(s.compare("bbbb"   ), 0);
    EXPECT_EQ(s.compare("bbbb", 4), 0);
    EXPECT_GT(s.compare("bbba"   ), 0);
    EXPECT_GT(s.compare("bbba", 4), 0);
    EXPECT_LT(s.compare("bbbc"   ), 0);
    EXPECT_LT(s.compare("bbbc", 4), 0);

    EXPECT_SUCCESS(s.assign("abc"));
    verify_string(s);
    EXPECT_EQ(s.size(), 3u);
    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s[1], 'b');
    EXPECT_EQ(s[2], 'c');
    EXPECT_TRUE(s.equals("abc"));
    EXPECT_TRUE(s.equals("abc", 3));
  }

  INFO("String manipulation");
  {
    BLString s;

    EXPECT_SUCCESS(s.assign("abc"));
    verify_string(s);
    EXPECT_SUCCESS(s.append("xyz"));
    verify_string(s);
    EXPECT_TRUE(s.equals("abcxyz"));

    EXPECT_SUCCESS(s.insert(2, s.view()));
    verify_string(s);
    EXPECT_TRUE(s.equals("ababcxyzcxyz"));

    EXPECT_SUCCESS(s.remove(BLRange{1, 11}));
    verify_string(s);
    EXPECT_TRUE(s.equals("az"));

    EXPECT_SUCCESS(s.insert(1, s.view()));
    verify_string(s);
    EXPECT_TRUE(s.equals("aazz"));

    EXPECT_SUCCESS(s.insert(1, "xxx"));
    verify_string(s);
    EXPECT_TRUE(s.equals("axxxazz"));

    EXPECT_SUCCESS(s.remove(BLRange{4, 6}));
    verify_string(s);
    EXPECT_TRUE(s.equals("axxxz"));

    BLString x(s);
    EXPECT_SUCCESS(s.insert(3, "INSERTED"));
    verify_string(s);
    EXPECT_TRUE(s.equals("axxINSERTEDxz"));

    x = s;
    verify_string(x);
    EXPECT_SUCCESS(s.remove(BLRange{1, 11}));
    verify_string(s);
    EXPECT_TRUE(s.equals("axz"));

    EXPECT_SUCCESS(s.insert(3, "APPENDED"));
    verify_string(s);
    EXPECT_TRUE(s.equals("axzAPPENDED"));

    EXPECT_SUCCESS(s.reserve(1024));
    EXPECT_GE(s.capacity(), 1024u);
    EXPECT_SUCCESS(s.shrink());
    EXPECT_LT(s.capacity(), 1024u);
  }

  INFO("String formatting");
  {
    BLString s;

    EXPECT_SUCCESS(s.assign_format("%d", 1000));
    EXPECT_TRUE(s.equals("1000"));
  }

  INFO("String search");
  {
    BLString s;

    EXPECT_SUCCESS(s.assign("abcdefghijklmnop-ponmlkjihgfedcba"));
    EXPECT_EQ(s.index_of('a'), 0u);
    EXPECT_EQ(s.index_of('a', 1), 32u);
    EXPECT_EQ(s.index_of('b'), 1u);
    EXPECT_EQ(s.index_of('b', 1), 1u);
    EXPECT_EQ(s.index_of('b', 2), 31u);
    EXPECT_EQ(s.last_index_of('b'), 31u);
    EXPECT_EQ(s.last_index_of('b', 30), 1u);
    EXPECT_EQ(s.index_of('z'), SIZE_MAX);
    EXPECT_EQ(s.index_of('z', SIZE_MAX), SIZE_MAX);
    EXPECT_EQ(s.last_index_of('z'), SIZE_MAX);
    EXPECT_EQ(s.last_index_of('z', 0), SIZE_MAX);
    EXPECT_EQ(s.last_index_of('z', SIZE_MAX), SIZE_MAX);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
