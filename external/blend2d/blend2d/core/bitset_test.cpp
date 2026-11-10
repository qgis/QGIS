// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/bitset_p.h>
#include <blend2d/core/random.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/intops_p.h>

// bl::BitSet - Tests
// ==================

namespace bl {
namespace Tests {

static void dump_bit_set(const BLBitSetCore* self) noexcept {
  if (self->_d.sso()) {
    if (self->_d.is_bit_set_range()) {
      BitSetInternal::Range range = BitSetInternal::get_sso_range(self);
      printf("BitSet<SSO_Range> {%u-%u}\n", range.start, range.end);
    }
    else {
      uint32_t word_index = BitSetInternal::get_sso_word_index(self);
      printf("BitSet<SSO_Dense> {%u-%u}\n", word_index, word_index + BitSetInternal::kSSOWordCount);
      for (uint32_t i = 0; i < BitSetInternal::kSSOWordCount; i++) {
        printf("  [%u] %08X\n", i, self->_d.u32_data[i]);
      }
    }
  }
  else {
    const BLBitSetImpl* impl = BitSetInternal::get_impl(self);
    printf("BitSet<Dynamic> {Count=%u Capacity=%u}\n", impl->segment_count, impl->segment_capacity);

    for (uint32_t i = 0; i < impl->segment_count; i++) {
      const BLBitSetSegment* segment = impl->segment_data() + i;
      if (segment->all_ones()) {
          printf("  [%u] {%u-%llu} [ones]\n", i, segment->start_bit(), (unsigned long long)segment->last_bit() + 1);
      }
      else {
        for (uint32_t j = 0; j < BitSetInternal::kSegmentWordCount; j++) {
          uint32_t bit_index = segment->start_bit() + j * 32;
          printf("  [%u] {%u-%llu} [%08X]\n", i, bit_index, (unsigned long long)bit_index + 32, segment->data()[j]);
        }
      }
    }
  }
}

static void test_bits(const BLBitSet& bit_set, uint32_t word_index, const uint32_t* word_data, uint32_t word_count) noexcept {
  for (uint32_t i = 0; i < word_count; i++) {
    for (uint32_t j = 0; j < 32; j++) {
      uint32_t bit_index = (word_index + i) * 32 + j;
      bool bit_value = BitSetOps::has_bit(word_data[i], j);
      EXPECT_EQ(bit_set.has_bit(bit_index), bit_value)
        .message("Failed to test bit [%u] - the bit value is not '%s'", bit_index, bit_value ? "true" : "false");
    }
  }
}

UNIT(bitset, BL_TEST_GROUP_CORE_CONTAINERS) {
  uint32_t kNumBits = 1000000u;
  uint32_t kSSOLastWord = BitSetInternal::kSSOLastWord;

  INFO("Checking SSO BitSet basics");
  {
    BLBitSet set;
    EXPECT_TRUE(set.is_empty());

    EXPECT_SUCCESS(set.add_bit(32));
    EXPECT_TRUE(set._d.sso());
    EXPECT_TRUE(set._d.is_bit_set_range());

    EXPECT_SUCCESS(set.add_bit(33));
    EXPECT_TRUE(set._d.sso());
    EXPECT_TRUE(set._d.is_bit_set_range());

    EXPECT_SUCCESS(set.add_bit(35));
    EXPECT_TRUE(set._d.sso());
    EXPECT_FALSE(set._d.is_bit_set_range());
    EXPECT_EQ(BitSetInternal::get_sso_dense_info(&set).start_bit(), 32u);
    EXPECT_EQ(set._d.u32_data[0], (BitSetOps::index_as_mask(0) | BitSetOps::index_as_mask(1) | BitSetOps::index_as_mask(3)));

    EXPECT_SUCCESS(set.clear_bit(35));
    EXPECT_SUCCESS(set.clear_bit(33));
    EXPECT_TRUE(set._d.sso());
    EXPECT_FALSE(set._d.is_bit_set_range());
    EXPECT_EQ(BitSetInternal::get_sso_dense_info(&set).start_bit(), 32u);
    EXPECT_EQ(set._d.u32_data[0], BitSetOps::index_as_mask(0));

    EXPECT_SUCCESS(set.clear_bit(32));
    EXPECT_TRUE(set.is_empty());
    EXPECT_TRUE(set._d.sso());
    EXPECT_TRUE(set._d.is_bit_set_range());

    EXPECT_SUCCESS(set.add_bit(0xFFFFFFFEu));
    EXPECT_TRUE(set._d.sso());
    EXPECT_TRUE(set._d.is_bit_set_range());

    // Dense SSO representation shouldn't start with a word that would overflow the data.
    EXPECT_SUCCESS(set.add_bit(0xFFFFFFFAu));
    EXPECT_TRUE(set._d.sso());
    EXPECT_FALSE(set._d.is_bit_set_range());
    EXPECT_EQ(BitSetInternal::get_sso_dense_info(&set).start_word(), kSSOLastWord);
    EXPECT_EQ(set._d.u32_data[0], 0u);
    EXPECT_EQ(set._d.u32_data[1], (BitSetOps::index_as_mask(26) | BitSetOps::index_as_mask(30)));

    EXPECT_SUCCESS(set.add_bit(0xFFFFFFD0u));
    EXPECT_TRUE(set._d.sso());
    EXPECT_FALSE(set._d.is_bit_set_range());
    EXPECT_EQ(set._d.u32_data[0], BitSetOps::index_as_mask(16));
    EXPECT_EQ(set._d.u32_data[1], (BitSetOps::index_as_mask(26) | BitSetOps::index_as_mask(30)));

    // Clearing the bit in the first word in this case won't shift the offset, as it would overflow addressable words.
    EXPECT_SUCCESS(set.clear_bit(0xFFFFFFD0u));
    EXPECT_TRUE(set._d.sso());
    EXPECT_FALSE(set._d.is_bit_set_range());
    EXPECT_EQ(set._d.u32_data[0], 0u);
    EXPECT_EQ(set._d.u32_data[1], (BitSetOps::index_as_mask(26) | BitSetOps::index_as_mask(30)));

    // Adding a range that fully subsumes a dense SSO data should result in SSO BitSet.
    EXPECT_SUCCESS(set.clear());
    EXPECT_SUCCESS(set.add_bit(64));
    EXPECT_SUCCESS(set.add_bit(90));
    EXPECT_SUCCESS(set.add_bit(33));
    EXPECT_TRUE(set._d.sso());
    EXPECT_TRUE(set.has_bit(33));
    EXPECT_TRUE(set.has_bit(64));
    EXPECT_TRUE(set.has_bit(90));
    EXPECT_SUCCESS(set.add_range(4, 112));
    EXPECT_TRUE(set._d.sso());
    EXPECT_TRUE(set.has_bit(4));
    EXPECT_TRUE(set.has_bit(111));
    EXPECT_EQ(set, BLBitSet(4, 112));

    EXPECT_SUCCESS(set.chop(5, 111));
    EXPECT_TRUE(set._d.sso());
    EXPECT_FALSE(set.has_bit(4));
    EXPECT_FALSE(set.has_bit(111));
    EXPECT_EQ(set, BLBitSet(5, 111));
  }

  INFO("Checking SSO BitSet ranges");
  {
    uint32_t i;
    BLBitSet set;

    EXPECT_TRUE(set._d.is_bit_set());
    EXPECT_TRUE(set._d.sso());
    EXPECT_TRUE(set.is_empty());

    // This index is invalid in BitSet.
    EXPECT_EQ(set.add_bit(0xFFFFFFFFu), BL_ERROR_INVALID_VALUE);

    for (i = 0; i < kNumBits; i++) {
      EXPECT_SUCCESS(set.add_bit(i));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(0, i + 1);
      EXPECT_EQ(set, range);
    }

    EXPECT_SUCCESS(set.clear());
    for (i = 0; i < kNumBits; i++) {
      EXPECT_SUCCESS(set.add_bit(kNumBits - i - 1));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(kNumBits - i - 1, kNumBits);
      EXPECT_EQ(set, range);
    }

    EXPECT_SUCCESS(set.assign_range(0, kNumBits));
    for (i = 0; i < kNumBits; i++) {
      EXPECT_SUCCESS(set.clear_bit(i));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(i + 1, kNumBits);
      EXPECT_EQ(set, range);
    }

    EXPECT_SUCCESS(set.assign_range(0, kNumBits));
    for (i = 0; i < kNumBits; i++) {
      EXPECT_SUCCESS(set.clear_bit(kNumBits - i - 1));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(0, kNumBits - i - 1);
      EXPECT_EQ(set, range);
    }

    EXPECT_SUCCESS(set.clear());
    for (i = 0; i < 65536; i++) {
      uint32_t start = i * 65536;
      uint32_t end = start + 65536;

      if (end == 0)
        end = 0xFFFFFFFFu;

      EXPECT_SUCCESS(set.add_range(start, end));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(0, end);
      EXPECT_EQ(set, range);
    }

    // Tests whether add_range() handles unions properly.
    EXPECT_SUCCESS(set.clear());
    for (i = 0; i < 65536; i++) {
      uint32_t start = i * 13;
      uint32_t end = i * 65536 + 65536;

      if (end == 0)
        end = 0xFFFFFFFFu;

      EXPECT_SUCCESS(set.add_range(start, end));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(0, end);
      EXPECT_EQ(set, range);
    }

    // Tests whether add_range() handles adding ranges from the end.
    EXPECT_SUCCESS(set.clear());
    for (i = 0; i < 65536; i++) {
      uint32_t start = (65535 - i) * 65536;
      uint32_t end = start + 65536;

      if (end == 0)
        end = 0xFFFFFFFFu;

      EXPECT_SUCCESS(set.add_range(start, end));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(start, 0xFFFFFFFFu);
      EXPECT_EQ(set, range);
    }

    // Tests whether clear_range() handles clearing ranges from the end.
    EXPECT_SUCCESS(set.clear());
    EXPECT_SUCCESS(set.assign_range(0, 0xFFFFFFFFu));
    EXPECT_EQ(set.cardinality(), 0xFFFFFFFFu);

    for (i = 0; i < 65536; i++) {
      uint32_t start = (65535 - i) * 65536;
      uint32_t end = start + 65536;

      if (end == 0)
        end = 0xFFFFFFFFu;

      EXPECT_SUCCESS(set.clear_range(start, end));
      EXPECT_TRUE(set._d.sso());

      BLBitSet range(0, start);
      EXPECT_EQ(set, range);
    }
  }

  INFO("Checking SSO BitSet assign_words()");
  {
    BLBitSet set;

    {
      const uint32_t words[] = { 0x80000000u, 0x01010101u };
      EXPECT_SUCCESS(set.assign_words(0, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), 0u);
      EXPECT_EQ(set._d.u32_data[0], 0x80000000u);
      EXPECT_EQ(set._d.u32_data[1], 0x01010101u);
      EXPECT_EQ(set.cardinality(), 5u);
    }

    {
      const uint32_t words[] = { 0x80000000u, 0x01010101u };
      EXPECT_SUCCESS(set.assign_words(55, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), 55u);
      EXPECT_EQ(set._d.u32_data[0], 0x80000000u);
      EXPECT_EQ(set._d.u32_data[1], 0x01010101u);
      EXPECT_EQ(set.cardinality(), 5u);
    }

    {
      const uint32_t words[] = { 0x00000000u, 0x80000000u, 0x01010101u };
      EXPECT_SUCCESS(set.assign_words(0, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), 1u);
      EXPECT_EQ(set._d.u32_data[0], 0x80000000u);
      EXPECT_EQ(set._d.u32_data[1], 0x01010101u);
      EXPECT_EQ(set.cardinality(), 5u);
    }

    {
      const uint32_t words[] = { 0x00000000u, 0x80000000u, 0x01010101u, 0x00000000u };
      EXPECT_SUCCESS(set.assign_words(0, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), 1u);
      EXPECT_EQ(set._d.u32_data[0], 0x80000000u);
      EXPECT_EQ(set._d.u32_data[1], 0x01010101u);
      EXPECT_EQ(set.cardinality(), 5u);
    }

    {
      const uint32_t words[] = { 0x00000000u, 0x00000000u, 0x80000000u, 0x01010101u, 0x00000000u, 0x00000000u };
      EXPECT_SUCCESS(set.assign_words(0, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), 2u);
      EXPECT_EQ(set._d.u32_data[0], 0x80000000u);
      EXPECT_EQ(set._d.u32_data[1], 0x01010101u);
      EXPECT_EQ(set.cardinality(), 5u);
    }

    {
      const uint32_t words[] = { 0xFFFF0000u };
      EXPECT_SUCCESS(set.assign_words(BitSetInternal::kLastWord, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), BitSetInternal::kSSOLastWord);
      EXPECT_EQ(set._d.u32_data[0], 0x00000000u);
      EXPECT_EQ(set._d.u32_data[1], 0xFFFF0000u);
      EXPECT_EQ(set.cardinality(), 16u);
    }

    {
      const uint32_t words[] = { 0x0000FFFFu, 0xFFFF0000u };
      EXPECT_SUCCESS(set.assign_words(BitSetInternal::kLastWord - 1, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), BitSetInternal::kSSOLastWord);
      EXPECT_EQ(set._d.u32_data[0], 0x0000FFFFu);
      EXPECT_EQ(set._d.u32_data[1], 0xFFFF0000u);
      EXPECT_EQ(set.cardinality(), 32u);
    }

    // Last index of SSO Dense BitSet must be kSSOLastWord even when the first word would be zero.
    // The reason is that if we allowed a higher index it would be possible to address words, which
    // are outside of the addressable range, which is [0, 4294967296).
    {
      const uint32_t words[] = { 0x00000000u, 0x0000FFFFu, 0xFFFF0000u };
      EXPECT_SUCCESS(set.assign_words(BitSetInternal::kLastWord - 2, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), BitSetInternal::kSSOLastWord);
      EXPECT_EQ(set._d.u32_data[0], 0x0000FFFFu);
      EXPECT_EQ(set._d.u32_data[1], 0xFFFF0000u);
      EXPECT_EQ(set.cardinality(), 32u);
    }

    {
      const uint32_t words[] = { 0x00000000u, 0x00000000u, 0x0000FFFFu, 0xFFFF0000u };
      EXPECT_SUCCESS(set.assign_words(BitSetInternal::kLastWord - 3, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(set._d.sso());
      EXPECT_EQ(BitSetInternal::get_sso_word_index(&set), BitSetInternal::kSSOLastWord);
      EXPECT_EQ(set._d.u32_data[0], 0x0000FFFFu);
      EXPECT_EQ(set._d.u32_data[1], 0xFFFF0000u);
      EXPECT_EQ(set.cardinality(), 32u);
    }

    // BitSet should refuse words, which are outside of the addressable range.
    {
      const uint32_t words[] = { 0x0000FFFFu, 0xFFFF0000u };
      EXPECT_EQ(set.assign_words(BitSetInternal::kLastWord, words, BL_ARRAY_SIZE(words)), BL_ERROR_INVALID_VALUE);
    }
  }

  INFO("Checking SSO BitSet chop()");
  {
    BLBitSet set;

    // Range BitSet.
    EXPECT_SUCCESS(set.add_range(0, 1000));

    EXPECT_TRUE(set._d.sso());
    EXPECT_EQ(set.cardinality(), 1000u);

    for (uint32_t i = 0; i < 1000; i++) {
      EXPECT_SUCCESS(set.chop(i, 1000));
      EXPECT_EQ(set.cardinality(), 1000u - i);

      uint32_t start, end;
      EXPECT_TRUE(set.get_range(&start, &end));
      EXPECT_EQ(start, i);
      EXPECT_EQ(end, 1000u);
    }

    EXPECT_SUCCESS(set.clear());

    // Dense BitSet.
    for (uint32_t i = 0; i < 96; i += 2) {
      EXPECT_SUCCESS(set.add_bit(i));
    }

    for (uint32_t i = 0; i < 96; i++) {
      EXPECT_SUCCESS(set.chop(i, 96));
      EXPECT_EQ(set.cardinality(), 96 / 2u - ((i + 1) / 2u));
    }
  }

  INFO("Checking SSO BitSet has_bits_in_range() & cardinality_in_range()");
  {
    BLBitSet set;

    // Dense SSO range data will describe bits in range [992, 1088) - of words range [31, 34).
    EXPECT_SUCCESS(set.add_range(1000, 1022));
    EXPECT_SUCCESS(set.add_range(1029, 1044));
    EXPECT_SUCCESS(set.add_bit(1055));

    EXPECT_TRUE(set._d.sso());
    EXPECT_EQ(set.cardinality(), 38u);

    EXPECT_EQ(set.cardinality_in_range(0, 50), 0u);
    EXPECT_EQ(set.cardinality_in_range(0, 992), 0u);
    EXPECT_EQ(set.cardinality_in_range(0, 1000), 0u);

    EXPECT_EQ(set.cardinality_in_range(1000, 1001), 1u);
    EXPECT_EQ(set.cardinality_in_range(1000, 1010), 10u);
    EXPECT_EQ(set.cardinality_in_range(1000, 1029), 22u);
    EXPECT_EQ(set.cardinality_in_range(1000, 1040), 33u);
    EXPECT_EQ(set.cardinality_in_range(1000, 1100), 38u);

    EXPECT_EQ(set.cardinality_in_range(1050, 2000), 1u);
  }

  INFO("Checking dynamic BitSet basics");
  {
    uint32_t i;
    BLBitSet set;

    for (i = 0; i < kNumBits; i += 2) {
      EXPECT_FALSE(set.has_bit(i));
      EXPECT_SUCCESS(set.add_bit(i));
      EXPECT_TRUE(set.has_bit(i));
      EXPECT_FALSE(set.has_bit(i + 1));
    }

    for (i = 0; i < kNumBits; i += 2) {
      EXPECT_TRUE(set.has_bit(i));
      EXPECT_SUCCESS(set.clear_bit(i));
      EXPECT_FALSE(set.has_bit(i));
    }

    for (i = 0; i < kNumBits; i += 2) {
      EXPECT_FALSE(set.has_bit(kNumBits - i));
      EXPECT_SUCCESS(set.add_bit(kNumBits - i));
      EXPECT_TRUE(set.has_bit(kNumBits - i));
    }

    for (i = 0; i < kNumBits; i += 2) {
      EXPECT_TRUE(set.has_bit(kNumBits - i));
      EXPECT_SUCCESS(set.clear_bit(kNumBits - i));
      EXPECT_FALSE(set.has_bit(kNumBits - i));
    }

    EXPECT_SUCCESS(set.reset());

    for (i = 0; i < kNumBits; i += 4) {
      EXPECT_SUCCESS(set.add_range(i, i + 3));
      EXPECT_TRUE(set.has_bit(i));
      EXPECT_TRUE(set.has_bit(i + 1));
      EXPECT_TRUE(set.has_bit(i + 2));
      EXPECT_FALSE(set.has_bit(i + 3));

      EXPECT_SUCCESS(set.clear_bit(i));
      EXPECT_FALSE(set.has_bit(i));
      EXPECT_SUCCESS(set.clear_range(i, i + 2));
      EXPECT_FALSE(set.has_bit(i));
      EXPECT_FALSE(set.has_bit(i + 1));
      EXPECT_TRUE(set.has_bit(i + 2));
      EXPECT_FALSE(set.has_bit(i + 3));

      EXPECT_SUCCESS(set.add_range(i + 1, i + 4));
      EXPECT_FALSE(set.has_bit(i));
      EXPECT_TRUE(set.has_bit(i + 1));
      EXPECT_TRUE(set.has_bit(i + 2));
      EXPECT_TRUE(set.has_bit(i + 3));
    }

    for (i = 0; i < kNumBits; i += 4) {
      EXPECT_FALSE(set.has_bit(i));
      EXPECT_TRUE(set.has_bit(i + 1));
      EXPECT_TRUE(set.has_bit(i + 2));
      EXPECT_TRUE(set.has_bit(i + 3));
    }
  }

  INFO("Checking dynamic BitSet add_range() & clear_range()");
  {
    uint32_t i;
    BLBitSet set;

    // Add {0-10000} and {20000-30000} range and then add overlapping range.
    EXPECT_SUCCESS(set.add_range(0, 10000));
    for (i = 0; i < 10000; i++)
      EXPECT_TRUE(set.has_bit(i));
    EXPECT_FALSE(set.has_bit(10000));

    EXPECT_SUCCESS(set.add_range(20000, 30000));
    for (i = 0; i < 10000; i++)
      EXPECT_TRUE(set.has_bit(i));
    for (i = 20000; i < 30000; i++)
      EXPECT_TRUE(set.has_bit(i));
    EXPECT_FALSE(set.has_bit(30000));
    EXPECT_EQ(set.segment_count(), 5u);

    EXPECT_SUCCESS(set.add_range(6001, 23999));
    for (i = 0; i < 30000; i++)
      EXPECT_TRUE(set.has_bit(i));
    EXPECT_FALSE(set.has_bit(30000));
    EXPECT_EQ(set.segment_count(), 2u);

    // Turns dense segments into a range ending with a dense segment.
    EXPECT_SUCCESS(set.reset());
    for (i = 0; i < 10000; i += 2)
      EXPECT_SUCCESS(set.add_bit(i));
    EXPECT_EQ(set.segment_count(), 79u);
    EXPECT_SUCCESS(set.add_range(0, 10000));
    for (i = 0; i < 10000; i++)
      EXPECT_TRUE(set.has_bit(i));
    EXPECT_EQ(set.segment_count(), 2u);

    // Sparse bits to ranges.
    EXPECT_SUCCESS(set.reset());
    for (i = 1000; i < 10000000; i += 100000)
      EXPECT_SUCCESS(set.add_bit(i));
    for (i = 1000; i < 10000000; i += 100000)
      EXPECT_TRUE(set.has_bit(i));
    for (i = 1000; i < 10000000; i += 100000)
      EXPECT_SUCCESS(set.add_range(i - 500, i + 500));

    // Verify that clear_range() correctly inserts 4 segments.
    EXPECT_SUCCESS(set.reset());
    EXPECT_SUCCESS(set.add_range(0, 1024 * 1024));
    EXPECT_SUCCESS(set.clear_range(1023, 9999));
    EXPECT_EQ(set.segment_count(), 4u);

    // Verify that clear_range() correctly inserts 3 segments.
    EXPECT_SUCCESS(set.reset());
    EXPECT_SUCCESS(set.add_range(0, 1024 * 1024));
    EXPECT_SUCCESS(set.clear_range(1024, 9999));
    EXPECT_EQ(set.segment_count(), 3u);

    // Verify that clear_range() correctly inserts 2 segments.
    EXPECT_SUCCESS(set.reset());
    EXPECT_SUCCESS(set.add_range(0, 1024 * 1024));
    EXPECT_SUCCESS(set.clear_range(1024, 4096));
    EXPECT_EQ(set.segment_count(), 2u);

    // Verify that clear_range() correctly inserts 1 segment.
    EXPECT_SUCCESS(set.reset());
    EXPECT_SUCCESS(set.add_range(0, 1024 * 1024));
    EXPECT_SUCCESS(set.clear_range(0, 4096));
    EXPECT_EQ(set.segment_count(), 1u);
  }

  INFO("Checking dynamic BitSet assign_words()");
  {
    BLBitSet set;
    uint32_t start_bit, end_bit;

    {
      static const uint32_t words[] = {
        0x80000000u, 0x01010101u, 0x02020202u, 0x04040404u
      };

      EXPECT_SUCCESS(set.assign_words(0, words, BL_ARRAY_SIZE(words)));
      EXPECT_FALSE(set._d.sso());
      EXPECT_EQ(set.segment_count(), 1u);
      EXPECT_EQ(set.cardinality(), 13u);

      EXPECT_TRUE(set.get_range(&start_bit, &end_bit));
      EXPECT_EQ(start_bit, 0u);
      EXPECT_EQ(end_bit, 126u);

      EXPECT_SUCCESS(set.assign_words(33311, words, BL_ARRAY_SIZE(words)));
      EXPECT_FALSE(set._d.sso());
      EXPECT_EQ(set.segment_count(), 2u);
      EXPECT_EQ(set.cardinality(), 13u);

      EXPECT_TRUE(set.get_range(&start_bit, &end_bit));
      EXPECT_EQ(start_bit, 1065952u);
      EXPECT_EQ(end_bit, 1065952u + 126u);
    }

    // Test whether assign_words() results in a Range segment, when possible.
    {
      BLBitSet tmp;

      static const uint32_t words[] = {
        0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
        0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
        0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
        0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu
      };

      // NOTE: 'set' is already dynamic, assign_words() will not turn it to SSO if it's mutable.
      EXPECT_SUCCESS(set.assign_words(0, words, BL_ARRAY_SIZE(words)));
      EXPECT_FALSE(set._d.sso());
      EXPECT_EQ(set.segment_count(), 1u);
      EXPECT_EQ(set.cardinality(), 512u);

      // NOTE: 'tmp' is SSO, if assign_words() forms a range, it will be setup as SSO range.
      EXPECT_SUCCESS(tmp.assign_words(0, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(tmp._d.sso());
      EXPECT_EQ(tmp.cardinality(), 512u);

      EXPECT_TRUE(set.equals(tmp));

      // Verify whether assign_words() works well with arguments not aligned to a segment boundary.
      EXPECT_SUCCESS(set.assign_words(33, words, BL_ARRAY_SIZE(words)));
      EXPECT_FALSE(set._d.sso());
      EXPECT_EQ(set.segment_count(), 3u);
      EXPECT_EQ(set.cardinality(), 512u);

      EXPECT_SUCCESS(tmp.assign_words(33, words, BL_ARRAY_SIZE(words)));
      EXPECT_TRUE(tmp._d.sso());
      EXPECT_EQ(tmp.cardinality(), 512u);

      EXPECT_TRUE(set.equals(tmp));
    }
  }

  INFO("Checking dynamic BitSet add_words() - small BitSet");
  {
    BLBitSet set;

    {
      static const uint32_t words[] = { 0x80000000u, 0x01010101u, 0x02020202u, 0x04040404u };
      static const uint32_t range[] = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, };

      static const uint32_t check1[] = { 0x80000000u, 0x81010101u, 0x03030303u, 0x06060606u, 0x04040404u };
      static const uint32_t check2[] = { 0x80000000u, 0x81010101u, 0x83030303u, 0x07070707u, 0x06060606u, 0x04040404u };

      EXPECT_SUCCESS(set.add_words(8, words, BL_ARRAY_SIZE(words)));
      EXPECT_SUCCESS(set.add_words(9, words, BL_ARRAY_SIZE(words)));

      EXPECT_FALSE(set._d.sso());
      EXPECT_EQ(set.segment_count(), 2u);
      EXPECT_EQ(set.cardinality(), 26u);
      test_bits(set, 8, check1, BL_ARRAY_SIZE(check1));

      EXPECT_SUCCESS(set.add_words(7, words, BL_ARRAY_SIZE(words)));
      EXPECT_EQ(set.segment_count(), 3u);
      EXPECT_EQ(set.cardinality(), 39u);
      test_bits(set, 7, check2, BL_ARRAY_SIZE(check2));

      // Adding a range of words that don't overlap with existing segments must create a range segment.
      EXPECT_SUCCESS(set.add_words(32, range, BL_ARRAY_SIZE(range)));
      EXPECT_EQ(set.segment_count(), 4u);
      EXPECT_EQ(set.cardinality(), 39u + 32 * 8);
      test_bits(set, 7, check2, BL_ARRAY_SIZE(check2));
      test_bits(set, 32, range, BL_ARRAY_SIZE(range));
    }
  }

  INFO("Checking dynamic BitSet add_words() - large BitSet");
  {
    BLBitSet set;
    BLRandom rnd(0x1234);

    constexpr uint32_t kIterationCount = 1000;
    constexpr uint32_t kWordCount = 33;

    for (uint32_t i = 0; i < kIterationCount; i++) {
      uint32_t word_index = rnd.next_uint32() & 0xFFFFu;
      uint32_t word_data[kWordCount];

      // Random pattern... But we also want 0 and all bits set.
      uint32_t pattern = rnd.next_uint32();
      if (pattern < 0x20000000u)
        pattern = 0u;
      else if (pattern > 0xF0000000)
        pattern = 0xFFFFFFFF;

      for (uint32_t j = 0; j < kWordCount; j++) {
        word_data[j] = pattern;
      }

      set.add_words(word_index, word_data, kWordCount);
    }
  }

  INFO("Checking dynamic BitSet add_words() - consecutive");
  {
    BLBitSet set;
    BLRandom rnd(0x1234);

    constexpr uint32_t kIterationCount = 1000;
    constexpr uint32_t kWordCount = 33;

    uint32_t cardinality = 0;

    for (uint32_t i = 0; i < kIterationCount; i++) {
      uint32_t word_data[kWordCount];

      // Random pattern... But we also want 0 and all bits set.
      uint32_t pattern = rnd.next_uint32();
      if (pattern < 0x20000000u)
        pattern = 0u;
      else if (pattern > 0xF0000000)
        pattern = 0xFFFFFFFF;

      for (uint32_t j = 0; j < kWordCount; j++) {
        word_data[j] = pattern;
      }

      set.add_words(i * kWordCount, word_data, kWordCount);
      cardinality += IntOps::pop_count(pattern) * kWordCount;
    }

    EXPECT_EQ(set.cardinality(), cardinality);
  }

  INFO("Checking dynamic BitSet chop()");
  {
    BLBitSet set;

    for (uint32_t i = 0; i < kNumBits; i += 2) {
      EXPECT_SUCCESS(set.add_bit(i));
    }
    EXPECT_FALSE(set._d.sso());
    EXPECT_EQ(set.cardinality(), kNumBits / 2u);

    for (uint32_t i = 0; i < kNumBits / 2; i += 2) {
      EXPECT_TRUE(set.has_bit(i));
      EXPECT_SUCCESS(set.chop(i + 1, kNumBits));
      EXPECT_FALSE(set.has_bit(i));
      EXPECT_TRUE(set.has_bit(i + 2));
    }

    for (uint32_t i = kNumBits - 2; i > kNumBits / 2; i -= 2) {
      EXPECT_TRUE(set.has_bit(i));
      EXPECT_SUCCESS(set.chop(0, i));
      EXPECT_FALSE(set.has_bit(i));
      EXPECT_TRUE(set.has_bit(i - 2));
    }

    // BitSet should end up having a single segment having a single bit set.
    EXPECT_TRUE(set.has_bit(kNumBits / 2u));
    EXPECT_EQ(set.segment_count(), 1u);
    EXPECT_EQ(set.cardinality(), 1u);

    // Let's create a range segment and try to chop it.
    EXPECT_SUCCESS(set.clear());
    EXPECT_EQ(set.segment_count(), 0u);
    EXPECT_EQ(set.cardinality(), 0u);

    EXPECT_SUCCESS(set.add_range(0, 512));
    EXPECT_SUCCESS(set.add_range(1024, 2048));
    EXPECT_SUCCESS(set.add_range(4096, 8192));
    EXPECT_EQ(set.segment_count(), 3u);
    EXPECT_EQ(set.cardinality(), 512u + 1024u + 4096u);

    EXPECT_SUCCESS(set.chop(1025, 2047));
    EXPECT_EQ(set.segment_count(), 3u);
    EXPECT_EQ(set.cardinality(), 1022u);
  }

  INFO("Checking dynamic BitSet has_bits_in_range() & cardinality_in_range()");
  {
    BLBitSet set;

    EXPECT_SUCCESS(set.add_range(0, 512));
    EXPECT_SUCCESS(set.add_range(1024, 2048));
    EXPECT_SUCCESS(set.add_range(4096, 8192));

    EXPECT_TRUE(set.has_bits_in_range(0, 1));
    EXPECT_TRUE(set.has_bits_in_range(0, 512));
    EXPECT_TRUE(set.has_bits_in_range(0, 8192));
    EXPECT_TRUE(set.has_bits_in_range(444, 600));
    EXPECT_TRUE(set.has_bits_in_range(500, 600));
    EXPECT_TRUE(set.has_bits_in_range(1000, 2000));

    EXPECT_FALSE(set.has_bits_in_range(512, 600));
    EXPECT_FALSE(set.has_bits_in_range(512, 1024));
    EXPECT_FALSE(set.has_bits_in_range(2048, 4096));
    EXPECT_FALSE(set.has_bits_in_range(3000, 4011));

    for (uint32_t i = 0; i < 512; i++) {
      EXPECT_EQ(set.cardinality_in_range(0, i), i);
      EXPECT_EQ(set.has_bits_in_range(0, i), i > 0);
    }

    for (uint32_t i = 0; i < 512; i++) {
      EXPECT_EQ(set.cardinality_in_range(i, 512), 512 - i);
      EXPECT_TRUE(set.has_bits_in_range(i, 512));
    }

    for (uint32_t i = 0; i < 1024; i++) {
      EXPECT_EQ(set.cardinality_in_range(1024, 1024 + i), i);
      EXPECT_EQ(set.has_bits_in_range(1024, 1024 + i), i > 0);
    }

    for (uint32_t i = 0; i < 4096; i++) {
      EXPECT_EQ(set.cardinality_in_range(4096, 4096 + i), i);
      EXPECT_EQ(set.has_bits_in_range(4096, 4096 + i), i > 0);
    }

    for (uint32_t i = 0; i < 8192; i++) {
      uint32_t expected_cardinality;
      if (i < 1024)
        expected_cardinality = 4096 + 1024 + 512 - bl_min<uint32_t>(i, 512u);
      else if (i < 4096)
        expected_cardinality = 4096 + 1024 - bl_min(i - 1024u, 1024u);
      else
        expected_cardinality = 8192 - i;

      EXPECT_EQ(set.cardinality_in_range(i, 8192), expected_cardinality);
      EXPECT_TRUE(set.has_bits_in_range(i, 8192));
    }
  }

  INFO("Checking functionality of shrink() & optimize()");
  {
    BLBitSet set;
    uint32_t kCount = BitSetInternal::kSegmentBitCount * 100;

    for (uint32_t i = 0; i < kCount; i += 2)
      EXPECT_SUCCESS(set.add_bit(i));

    EXPECT_EQ(set.cardinality(), kCount / 2);

    for (uint32_t i = 0; i < kCount; i += 2)
      EXPECT_SUCCESS(set.add_bit(i + 1));

    EXPECT_FALSE(set._d.sso());
    EXPECT_EQ(set.cardinality(), kCount);
    EXPECT_GT(set.segment_count(), 1u);

    EXPECT_SUCCESS(set.optimize());
    EXPECT_EQ(set.segment_count(), 1u);
    EXPECT_FALSE(set._d.sso());
    EXPECT_EQ(set.cardinality(), kCount);

    EXPECT_SUCCESS(set.shrink());
    EXPECT_TRUE(set._d.sso());
    EXPECT_EQ(set.cardinality(), kCount);
  }

  INFO("Checking functionality of subsumes() & intersects()");
  {
    BLBitSet a;
    BLBitSet b;
    BLBitSet c;
    BLBitSet empty;

    EXPECT_SUCCESS(a.assign_range(10, 100));
    EXPECT_SUCCESS(b.assign_range(10, 100));
    EXPECT_TRUE(a.subsumes(b));
    EXPECT_TRUE(b.subsumes(a));
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));

    EXPECT_SUCCESS(b.assign_range(11, 100));
    EXPECT_TRUE(a.subsumes(b));
    EXPECT_FALSE(b.subsumes(a));
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));

    EXPECT_SUCCESS(b.assign_range(10, 99));
    EXPECT_TRUE(a.subsumes(b));
    EXPECT_FALSE(b.subsumes(a));
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));

    EXPECT_SUCCESS(a.assign_range(10, 100));
    EXPECT_SUCCESS(b.assign_range(1000, 10000));
    EXPECT_FALSE(a.subsumes(b));
    EXPECT_FALSE(b.subsumes(a));
    EXPECT_FALSE(a.intersects(b));
    EXPECT_FALSE(b.intersects(a));

    static const uint32_t a_sso_words[] = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFF8 };
    static const uint32_t b_sso_words[] = { 0xFFFF0000u, 0x0000FFFFu, 0xFFFFFFF8 };

    EXPECT_SUCCESS(a.assign_words(0, a_sso_words, BL_ARRAY_SIZE(a_sso_words)));
    EXPECT_SUCCESS(b.assign_words(0, b_sso_words, BL_ARRAY_SIZE(b_sso_words)));
    EXPECT_SUCCESS(c.assign_range(16, 32));

    EXPECT_TRUE(a.subsumes(empty));
    EXPECT_TRUE(b.subsumes(empty));
    EXPECT_TRUE(c.subsumes(empty));

    EXPECT_FALSE(a.intersects(empty));
    EXPECT_FALSE(b.intersects(empty));
    EXPECT_FALSE(c.intersects(empty));

    EXPECT_TRUE(a.subsumes(b));
    EXPECT_TRUE(a.subsumes(c));
    EXPECT_FALSE(b.subsumes(a));
    EXPECT_FALSE(b.subsumes(c));

    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(a.intersects(c));
    EXPECT_TRUE(b.intersects(a));
    EXPECT_FALSE(b.intersects(c));

    static const uint32_t a_dynamic_words[] = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFF8, 0x00000000u, 0xFFFF0000, 0xFFFFFFFFu, 0xFFFFFFFFu };
    static const uint32_t b_dynamic_words[] = { 0xFFFF0000u, 0x0000FFFFu, 0xFFFFFFF8, 0x00000000u, 0x00FF0000, 0xFF000000u, 0x00000000u };

    EXPECT_SUCCESS(a.assign_words(31, a_dynamic_words, BL_ARRAY_SIZE(a_dynamic_words)));
    EXPECT_SUCCESS(b.assign_words(31, b_dynamic_words, BL_ARRAY_SIZE(b_dynamic_words)));
    EXPECT_SUCCESS(c.assign_range(992, 1184));

    EXPECT_TRUE(a.subsumes(empty));
    EXPECT_TRUE(b.subsumes(empty));
    EXPECT_TRUE(c.subsumes(empty));

    EXPECT_FALSE(a.intersects(empty));
    EXPECT_FALSE(b.intersects(empty));
    EXPECT_FALSE(c.intersects(empty));

    EXPECT_TRUE(a.subsumes(b));
    EXPECT_FALSE(a.subsumes(c));
    EXPECT_FALSE(b.subsumes(a));
    EXPECT_FALSE(b.subsumes(c));

    EXPECT_FALSE(c.subsumes(a));
    EXPECT_TRUE(c.subsumes(b));
  }

  INFO("Checking functionality of BLBitSetBuilder");
  {
    BLBitSet set;

    {
      BLBitSetBuilder builder(&set);
      EXPECT_SUCCESS(builder.add_bit(1024));
      EXPECT_SUCCESS(builder.add_bit(1025));
      EXPECT_SUCCESS(builder.add_bit(1125));
      EXPECT_SUCCESS(builder.add_bit(1126));
      EXPECT_SUCCESS(builder.add_range(1080, 1126));
      EXPECT_SUCCESS(builder.commit());
    }

    EXPECT_TRUE(set.has_bit(1024));
    EXPECT_TRUE(set.has_bit(1025));
    EXPECT_TRUE(set.has_bit(1080));
    EXPECT_TRUE(set.has_bit(1126));
    EXPECT_EQ(set.cardinality(), 49u);
    EXPECT_EQ(set.cardinality_in_range(1024, 1127), 49u);

    {
      BLBitSetBuilder builder(&set);
      for (uint32_t i = 0; i < 4096; i += 2) {
        EXPECT_SUCCESS(builder.add_bit(4096 + i));
      }
      EXPECT_SUCCESS(builder.commit());
    }

    EXPECT_EQ(set.cardinality(), 49u + 2048u);
    EXPECT_EQ(set.cardinality_in_range(1024, 8192), 49u + 2048u);
  }

  INFO("Checking functionality of BLBitSetWordIterator");
  {
    // SSO Range BitSet.
    {
      BLBitSet set;
      EXPECT_SUCCESS(set.add_range(130, 200));

      BLBitSetWordIterator word_iterator(set);
      EXPECT_EQ(word_iterator.next_word(), 0x3FFFFFFFu);
      EXPECT_EQ(word_iterator.bit_index(), 128u);
      EXPECT_EQ(word_iterator.next_word(), 0xFFFFFFFFu);
      EXPECT_EQ(word_iterator.bit_index(), 160u);
      EXPECT_EQ(word_iterator.next_word(), 0xFF000000u);
      EXPECT_EQ(word_iterator.bit_index(), 192u);
      EXPECT_EQ(word_iterator.next_word(), 0u);
    }

    // SSO Dense BitSet.
    {
      BLBitSet set;
      EXPECT_SUCCESS(set.add_range(130, 140));
      EXPECT_SUCCESS(set.add_range(180, 200));

      BLBitSetWordIterator word_iterator(set);
      EXPECT_EQ(word_iterator.next_word(), 0x3FF00000u);
      EXPECT_EQ(word_iterator.bit_index(), 128u);
      EXPECT_EQ(word_iterator.next_word(), 0x00000FFFu);
      EXPECT_EQ(word_iterator.bit_index(), 160u);
      EXPECT_EQ(word_iterator.next_word(), 0xFF000000u);
      EXPECT_EQ(word_iterator.bit_index(), 192u);
      EXPECT_EQ(word_iterator.next_word(), 0u);
    }

    // Dynamic BitSet.
    {
      BLBitSet set;
      EXPECT_SUCCESS(set.add_range(130, 140));
      EXPECT_SUCCESS(set.add_range(1024, 1025));
      EXPECT_SUCCESS(set.add_range(2050, 2060));

      BLBitSetWordIterator word_iterator(set);
      EXPECT_EQ(word_iterator.next_word(), 0x3FF00000u);
      EXPECT_EQ(word_iterator.bit_index(), 128u);
      EXPECT_EQ(word_iterator.next_word(), 0x80000000u);
      EXPECT_EQ(word_iterator.bit_index(), 1024u);
      EXPECT_EQ(word_iterator.next_word(), 0x3FF00000u);
      EXPECT_EQ(word_iterator.bit_index(), 2048u);
      EXPECT_EQ(word_iterator.next_word(), 0u);
    }
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
