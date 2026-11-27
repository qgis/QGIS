// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/support/bitops_p.h>

// bl::BitOps - Tests
// ==================

namespace bl {
namespace Tests {

template<typename T>
using LSBBitOps = ParametrizedBitOps<BitOrder::kLSB, T>;

template<typename T>
using MSBBitOps = ParametrizedBitOps<BitOrder::kMSB, T>;

static void test_bit_array_ops() {
  uint32_t bits[3];

  INFO("bl::ParametrizedBitOps<bl::BitOrder::kLSB>::bit_array_fill");
  memset(bits, 0, sizeof(bits));
  LSBBitOps<uint32_t>::bit_array_fill(bits, 1, 94);
  EXPECT_EQ(bits[0], 0xFFFFFFFEu);
  EXPECT_EQ(bits[1], 0xFFFFFFFFu);
  EXPECT_EQ(bits[2], 0x7FFFFFFFu);

  INFO("bl::ParametrizedBitOps<bl::BitOrder::kMSB>::bit_array_fill");
  memset(bits, 0, sizeof(bits));
  MSBBitOps<uint32_t>::bit_array_fill(bits, 1, 94);
  EXPECT_EQ(bits[0], 0x7FFFFFFFu);
  EXPECT_EQ(bits[1], 0xFFFFFFFFu);
  EXPECT_EQ(bits[2], 0xFFFFFFFEu);
}

static void test_bit_iterator() {
  INFO("bl::ParametrizedBitOps<bl::BitOrder::kLSB>::BitIterator<uint32_t>");
  LSBBitOps<uint32_t>::BitIterator lsb_it(0x40000010u);

  EXPECT_TRUE(lsb_it.has_next());
  EXPECT_EQ(lsb_it.next(), 4u);
  EXPECT_TRUE(lsb_it.has_next());
  EXPECT_EQ(lsb_it.next(), 30u);
  EXPECT_TRUE(!lsb_it.has_next());

  INFO("bl::ParametrizedBitOps<bl::BitOrder::kMSB>::BitIterator<uint32_t>");
  MSBBitOps<uint32_t>::BitIterator msb_it(0x40000010u);

  EXPECT_TRUE(msb_it.has_next());
  EXPECT_EQ(msb_it.next(), 1u);
  EXPECT_TRUE(msb_it.has_next());
  EXPECT_EQ(msb_it.next(), 27u);
  EXPECT_TRUE(!msb_it.has_next());
}

static void test_bit_vector_iterator() {
  static const uint32_t lsb_bits[] = { 0x00000001u, 0x80000000u };
  static const uint32_t msb_bits[] = { 0x00000001u, 0x80000000u };

  INFO("bl::ParametrizedBitOps<bl::BitOrder::kLSB>::BitVectorIterator<uint32_t>");
  LSBBitOps<uint32_t>::BitVectorIterator lsb_it(lsb_bits, BL_ARRAY_SIZE(lsb_bits));

  EXPECT_TRUE(lsb_it.has_next());
  EXPECT_EQ(lsb_it.next(), 0u);
  EXPECT_TRUE(lsb_it.has_next());
  EXPECT_EQ(lsb_it.next(), 63u);
  EXPECT_TRUE(!lsb_it.has_next());

  INFO("bl::ParametrizedBitOps<bl::BitOrder::kMSB>::BitVectorIterator<uint32_t>");
  MSBBitOps<uint32_t>::BitVectorIterator msb_it(msb_bits, BL_ARRAY_SIZE(msb_bits));

  EXPECT_TRUE(msb_it.has_next());
  EXPECT_EQ(msb_it.next(), 31u);
  EXPECT_TRUE(msb_it.has_next());
  EXPECT_EQ(msb_it.next(), 32u);
  EXPECT_TRUE(!msb_it.has_next());
}

static void test_bit_vector_flip_iterator() {
  static const uint32_t lsb_bits[] = { 0xFFFFFFF0u, 0x00FFFFFFu };
  static const uint32_t msb_bits[] = { 0x0FFFFFFFu, 0xFFFFFF00u };

  INFO("bl::ParametrizedBitOps<bl::BitOrder::kLSB>::BitVectorFlipIterator<uint32_t>");
  LSBBitOps<uint32_t>::BitVectorFlipIterator lsb_it(lsb_bits, BL_ARRAY_SIZE(lsb_bits));
  EXPECT_TRUE(lsb_it.has_next());
  EXPECT_EQ(lsb_it.peek_next(), 4u);
  EXPECT_EQ(lsb_it.next_and_flip(), 4u);
  EXPECT_TRUE(lsb_it.has_next());
  EXPECT_EQ(lsb_it.peek_next(), 56u);
  EXPECT_EQ(lsb_it.next_and_flip(), 56u);
  EXPECT_TRUE(!lsb_it.has_next());

  INFO("bl::ParametrizedBitOps<bl::BitOrder::kMSB>::BitVectorFlipIterator<uint32_t>");
  MSBBitOps<uint32_t>::BitVectorFlipIterator msb_it(msb_bits, BL_ARRAY_SIZE(msb_bits));
  EXPECT_TRUE(msb_it.has_next());
  EXPECT_EQ(msb_it.peek_next(), 4u);
  EXPECT_EQ(msb_it.next_and_flip(), 4u);
  EXPECT_TRUE(msb_it.has_next());
  EXPECT_EQ(msb_it.peek_next(), 56u);
  EXPECT_EQ(msb_it.next_and_flip(), 56u);
  EXPECT_TRUE(!msb_it.has_next());
}

UNIT(support_bitops, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  test_bit_array_ops();
  test_bit_iterator();
  test_bit_vector_iterator();
  test_bit_vector_flip_iterator();
}

} // {Tests}
} // {bl}

#endif // BL_TEST
