// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/bitarray_p.h>
#include <blend2d/core/object_p.h>

// bl::BitArray - Tests
// ====================

namespace bl::Tests {

UNIT(bitarray, BL_TEST_GROUP_CORE_CONTAINERS) {
  constexpr uint32_t kSSOBitCapacity = BLBitArray::kSSOWordCount * 32u;
  BLBitArray empty;

  INFO("[SSO] Basic functionality");
  {
    BLBitArray ba;

    EXPECT_TRUE(ba._d.sso());
    EXPECT_TRUE(ba.is_empty());
    EXPECT_EQ(ba.size(), 0u);
    EXPECT_EQ(ba.capacity(), kSSOBitCapacity);
    EXPECT_TRUE(ba.equals(empty));

    for (uint32_t i = 0; i < kSSOBitCapacity; i++) {
      EXPECT_SUCCESS(ba.append_bit(true));
      EXPECT_EQ(ba.size(), i + 1u);
      EXPECT_TRUE(ba._d.sso());
      EXPECT_TRUE(ba.equals(ba));
      EXPECT_FALSE(ba.equals(empty));

      EXPECT_EQ(ba.cardinality(), i + 1u);
      EXPECT_EQ(ba.cardinality_in_range(0, i + 1u), i + 1u);
    }

    ba.clear();
    EXPECT_TRUE(ba._d.sso());
    EXPECT_TRUE(ba.is_empty());
    EXPECT_EQ(ba.size(), 0u);
    EXPECT_EQ(ba.capacity(), kSSOBitCapacity);
    EXPECT_TRUE(ba.equals(empty));

    for (uint32_t i = 0; i < kSSOBitCapacity; i++) {
      EXPECT_SUCCESS(ba.append_bit(i & 1 ? true : false));
      EXPECT_EQ(ba.size(), i + 1u);
      EXPECT_TRUE(ba._d.sso());
      EXPECT_TRUE(ba.equals(ba));
      EXPECT_FALSE(ba.equals(empty));
    }

    EXPECT_EQ(ba.cardinality(), kSSOBitCapacity / 2u);
    EXPECT_EQ(ba.cardinality_in_range(0, kSSOBitCapacity), kSSOBitCapacity / 2u);

    for (uint32_t i = 0; i < kSSOBitCapacity; i++) {
      EXPECT_SUCCESS(ba.set_bit(i));
    }

    EXPECT_EQ(ba.cardinality(), kSSOBitCapacity);
    EXPECT_EQ(ba.cardinality_in_range(0, kSSOBitCapacity), kSSOBitCapacity);

    for (uint32_t i = 0; i < kSSOBitCapacity; i++) {
      EXPECT_SUCCESS(ba.clear_bit(i));
    }

    EXPECT_EQ(ba.cardinality(), 0u);
    EXPECT_EQ(ba.cardinality_in_range(0, kSSOBitCapacity), 0u);
  }

  INFO("[SSO] Using word quantities");
  {
    BLBitArray ba;
    EXPECT_TRUE(ba._d.sso());

    uint32_t words[3] = { 0x80000000, 0x40000000, 0x20000000 };
    EXPECT_SUCCESS(ba.append_words(words, 3));

    EXPECT_TRUE(ba._d.sso());
    EXPECT_EQ(ba.size(), 96u);
    EXPECT_TRUE(ba.has_bit(0));
    EXPECT_TRUE(ba.has_bit(33));
    EXPECT_TRUE(ba.has_bit(66));
    EXPECT_FALSE(ba.has_bit(1));
    EXPECT_FALSE(ba.has_bit(32));
    EXPECT_FALSE(ba.has_bit(34));
    EXPECT_FALSE(ba.has_bit(65));
    EXPECT_FALSE(ba.has_bit(67));

    ba.clear();
    EXPECT_TRUE(ba._d.sso());

    // Appends words to a BitArray that has 1 butm thus the words need to be shifted.
    ba.append_bit(false);
    EXPECT_SUCCESS(ba.append_words(words, 2));

    EXPECT_TRUE(ba.has_bit(1));
    EXPECT_TRUE(ba.has_bit(34));
    EXPECT_FALSE(ba.has_bit(0));
    EXPECT_FALSE(ba.has_bit(2));
    EXPECT_FALSE(ba.has_bit(33));
    EXPECT_FALSE(ba.has_bit(35));
    EXPECT_FALSE(ba.has_bit(66));
    EXPECT_FALSE(ba.has_bit(68));

    EXPECT_SUCCESS(ba.clear());
  }

  INFO("[Dynamic] Basic functionality");
  {
    constexpr uint32_t kCount = uint32_t(100000 & -64);
    BLBitArray ba;

    for (uint32_t i = 0; i < kCount; i++) {
      EXPECT_SUCCESS(ba.append_bit(true));
      EXPECT_EQ(ba.size(), i + 1u);
      EXPECT_TRUE(ba.equals(ba));
    }

    EXPECT_EQ(ba.cardinality(), kCount);
    EXPECT_EQ(ba.cardinality_in_range(0, kCount), kCount);

    for (uint32_t i = 0; i < kCount; i += 2) {
      EXPECT_SUCCESS(ba.clear_bit(i));
      EXPECT_EQ(ba.size(), kCount);
    }

    EXPECT_EQ(ba.cardinality(), kCount / 2u);
    EXPECT_EQ(ba.cardinality_in_range(0, kCount), kCount / 2u);

    for (uint32_t i = 0; i < kCount; i += 2) {
      EXPECT_SUCCESS(ba.replace_bit(i, true));
      EXPECT_EQ(ba.size(), kCount);
    }

    EXPECT_EQ(ba.cardinality(), kCount);
    EXPECT_EQ(ba.cardinality_in_range(0, kCount), kCount);

    uint32_t pattern = 0;
    for (uint32_t i = 0; i < kCount; i += 32) {
      EXPECT_SUCCESS(ba.replace_word(i, pattern));
      pattern ^= 0xFFFFFFFFu;
    }

    EXPECT_EQ(ba.cardinality(), kCount / 2u);
    EXPECT_EQ(ba.cardinality_in_range(0, kCount), kCount / 2u);

    for (uint32_t i = 0; i < kCount; i += 32) {
      EXPECT_SUCCESS(ba.clear_word(i, 0x33333333));
    }

    EXPECT_EQ(ba.cardinality(), kCount / 4u);
    EXPECT_EQ(ba.cardinality_in_range(0, kCount), kCount / 4u);
  }
}

} // {bl::Tests}

#endif // BL_TEST
