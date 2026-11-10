// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/array_p.h>
#include <blend2d/core/fontfeaturesettings_p.h>
#include <blend2d/core/fonttagdata_p.h>
#include <blend2d/core/object_p.h>

// bl::FontFeatureSettings - Tests
// ===============================

namespace bl {
namespace Tests {

static void verify_font_feature_settings(const BLFontFeatureSettings& ffs) noexcept {
  BLFontFeatureSettingsView view;
  ffs.get_view(&view);

  if (view.size == 0)
    return;

  uint32_t prev_tag = view.data[0].tag;
  for (size_t i = 1; i < view.size; i++) {
    EXPECT_LT(prev_tag, view.data[i].tag)
      .message("BLFontFeatureSettings is corrupted - previous tag 0x%08X is not less than current tag 0x%08X at [%zu]", prev_tag, view.data[i].tag, i);
    prev_tag = view.data[i].tag;
  }
}

UNIT(fontfeaturesettings_allocation_strategy, BL_TEST_GROUP_TEXT_CONTAINERS) {
  BLFontFeatureSettings ffs;
  size_t capacity = ffs.capacity();

  constexpr uint32_t kCharRange = FontTagData::kCharRangeInTag;
  constexpr uint32_t kNumItems = FontTagData::kUniqueTagCount / 100;

  for (uint32_t i = 0; i < kNumItems; i++) {
    BLTag tag = BL_MAKE_TAG(
      uint32_t(' ') + (i / (kCharRange * kCharRange * kCharRange)),
      uint32_t(' ') + (i / (kCharRange * kCharRange)) % kCharRange,
      uint32_t(' ') + (i / (kCharRange)) % kCharRange,
      uint32_t(' ') + (i % kCharRange));

    ffs.set_value(tag, i & 0xFFFFu);
    if (capacity != ffs.capacity()) {
      size_t impl_size = FontFeatureSettingsInternal::impl_size_from_capacity(ffs.capacity()).value();
      INFO("Capacity increased from %zu to %zu [ImplSize=%zu]\n", capacity, ffs.capacity(), impl_size);
      capacity = ffs.capacity();
    }
  }

  verify_font_feature_settings(ffs);
}

UNIT(fontfeaturesettings, BL_TEST_GROUP_TEXT_CONTAINERS) {
  // These are not sorted on purpose to test whether BLFontFeatureSettings would sort them during insertion.
  static const uint32_t fat_tags[] = {
    BL_MAKE_TAG('r', 'a', 'n', 'd'),
    BL_MAKE_TAG('a', 'a', 'l', 't'),
    BL_MAKE_TAG('s', 's', '0', '9'),
    BL_MAKE_TAG('s', 's', '0', '4')
  };

  INFO("SSO initial state");
  {
    BLFontFeatureSettings ffs;

    EXPECT_TRUE(ffs._d.sso());
    EXPECT_TRUE(ffs.is_empty());
    EXPECT_EQ(ffs.size(), 0u);
    EXPECT_EQ(ffs.capacity(), BLFontFeatureSettings::kSSOCapacity);

    // SSO mode should present all available features as invalid (unassigned).
    for (uint32_t feature_id = 0; feature_id < FontTagData::kFeatureIdCount; feature_id++) {
      BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];
      EXPECT_EQ(ffs.get_value(feature_tag), BL_FONT_FEATURE_INVALID_VALUE);
    }

    // Trying to get an unknown tag should fail.
    EXPECT_EQ(ffs.get_value(BL_MAKE_TAG('-', '-', '-', '-')), BL_FONT_FEATURE_INVALID_VALUE);
    EXPECT_EQ(ffs.get_value(BL_MAKE_TAG('a', 'a', 'a', 'a')), BL_FONT_FEATURE_INVALID_VALUE);
    EXPECT_EQ(ffs.get_value(BL_MAKE_TAG('z', 'z', 'z', 'z')), BL_FONT_FEATURE_INVALID_VALUE);
  }

  INFO("SSO bit tag/value storage");
  {
    BLFontFeatureSettings ffs;

    // SSO storage must allow to store ALL font features that have bit mapping.
    uint32_t num_tags = 0;
    for (uint32_t feature_id = 0; feature_id < FontTagData::kFeatureIdCount; feature_id++) {
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        num_tags++;
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];

        EXPECT_SUCCESS(ffs.set_value(feature_tag, 1u));
        EXPECT_EQ(ffs.get_value(feature_tag), 1u);
        EXPECT_EQ(ffs.size(), num_tags);
        EXPECT_TRUE(ffs._d.sso());

        verify_font_feature_settings(ffs);
      }
    }

    // Set all features to zero (disabled, but still present in the mapping).
    for (uint32_t feature_id = 0; feature_id < FontTagData::kFeatureIdCount; feature_id++) {
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];

        EXPECT_SUCCESS(ffs.set_value(feature_tag, 0u));
        EXPECT_EQ(ffs.get_value(feature_tag), 0u);
        EXPECT_EQ(ffs.size(), num_tags);
        EXPECT_TRUE(ffs._d.sso());
        verify_font_feature_settings(ffs);
      }
    }

    // Remove all features.
    for (uint32_t feature_id = 0; feature_id < FontTagData::kFeatureIdCount; feature_id++) {
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        num_tags--;
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];

        EXPECT_SUCCESS(ffs.remove_value(feature_tag));
        EXPECT_EQ(ffs.get_value(feature_tag), BL_FONT_FEATURE_INVALID_VALUE);
        EXPECT_EQ(ffs.size(), num_tags);
        EXPECT_TRUE(ffs._d.sso());

        verify_font_feature_settings(ffs);
      }
    }

    EXPECT_TRUE(ffs.is_empty());
    EXPECT_EQ(ffs, BLFontFeatureSettings());
  }

  INFO("SSO bit tag/value storage limitations");
  {
    BLFontFeatureSettings ffs;

    // Trying to set any other value than 0-1 with bit tags fails.
    for (uint32_t feature_id = 0; feature_id < FontTagData::kFeatureIdCount; feature_id++) {
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];
        EXPECT_EQ(ffs.set_value(feature_tag, 2u), BL_ERROR_INVALID_VALUE);
      }
    }

    EXPECT_TRUE(ffs.is_empty());
  }

  INFO("SSO bit tag/value storage + fat tag/value storage");
  {
    BLFontFeatureSettings ffs;
    uint32_t num_tags = 0;

    // Add fat tag/value data.
    for (uint32_t i = 0; i < BL_ARRAY_SIZE(fat_tags); i++) {
      num_tags++;

      EXPECT_SUCCESS(ffs.set_value(fat_tags[i], 15u));
      EXPECT_EQ(ffs.get_value(fat_tags[i]), 15u);
      EXPECT_EQ(ffs.size(), num_tags);
      EXPECT_TRUE(ffs._d.sso());

      verify_font_feature_settings(ffs);

      // Verify that changing a fat tag's value is working properly (it's bit twiddling).
      EXPECT_SUCCESS(ffs.set_value(fat_tags[i], 1u));
      EXPECT_EQ(ffs.get_value(fat_tags[i]), 1u);
      EXPECT_EQ(ffs.size(), num_tags);
      EXPECT_TRUE(ffs._d.sso());

      verify_font_feature_settings(ffs);
    }

    // Add bit tag/value data.
    for (uint32_t feature_id = 0; feature_id < FontTagData::kFeatureIdCount; feature_id++) {
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        num_tags++;
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];

        EXPECT_SUCCESS(ffs.set_value(feature_tag, 1u));
        EXPECT_EQ(ffs.get_value(feature_tag), 1u);
        EXPECT_EQ(ffs.size(), num_tags);
        EXPECT_TRUE(ffs._d.sso());

        verify_font_feature_settings(ffs);
      }
    }

    // Remove fat tag/value data.
    for (uint32_t i = 0; i < BL_ARRAY_SIZE(fat_tags); i++) {
      num_tags--;

      EXPECT_SUCCESS(ffs.remove_value(fat_tags[i]));
      EXPECT_EQ(ffs.size(), num_tags);
      EXPECT_TRUE(ffs._d.sso());

      verify_font_feature_settings(ffs);
    }

    // Remove bit tag/value data.
    for (uint32_t feature_id = 0; feature_id < FontTagData::kFeatureIdCount; feature_id++) {
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        num_tags--;
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];

        EXPECT_SUCCESS(ffs.remove_value(feature_tag));
        EXPECT_EQ(ffs.size(), num_tags);
        EXPECT_TRUE(ffs._d.sso());

        verify_font_feature_settings(ffs);
      }
    }

    EXPECT_TRUE(ffs.is_empty());
    EXPECT_EQ(ffs, BLFontFeatureSettings());
  }

  INFO("SSO tag/value equality");
  {
    BLFontFeatureSettings ffsA;
    BLFontFeatureSettings ffsB;

    // Assign bit tag/value data.
    for (uint32_t i = 0; i < FontTagData::kFeatureIdCount; i++) {
      uint32_t feature_id = i;
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];
        EXPECT_SUCCESS(ffsA.set_value(feature_tag, 1u));
        verify_font_feature_settings(ffsA);
      }
    }

    for (uint32_t i = 0; i < FontTagData::kFeatureIdCount; i++) {
      uint32_t feature_id = FontTagData::kFeatureIdCount - 1 - i;
      if (FontTagData::feature_info_table[feature_id].has_bit_id()) {
        BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];
        EXPECT_SUCCESS(ffsB.set_value(feature_tag, 1u));
        verify_font_feature_settings(ffsB);
      }
    }

    EXPECT_EQ(ffsA, ffsB);

    // Assign fat tag/value data.
    for (uint32_t i = 0; i < BL_ARRAY_SIZE(fat_tags); i++) {
      EXPECT_SUCCESS(ffsA.set_value(fat_tags[i], i));
      verify_font_feature_settings(ffsA);
    }

    for (uint32_t iRev = 0; iRev < BL_ARRAY_SIZE(fat_tags); iRev++) {
      uint32_t i = BL_ARRAY_SIZE(fat_tags) - 1 - iRev;
      EXPECT_SUCCESS(ffsB.set_value(fat_tags[i], i));
      verify_font_feature_settings(ffsB);
    }

    EXPECT_EQ(ffsA, ffsB);

    // Remove fat tag/value data.
    for (uint32_t i = 0; i < BL_ARRAY_SIZE(fat_tags); i++) {
      EXPECT_SUCCESS(ffsA.remove_value(fat_tags[i]));
      verify_font_feature_settings(ffsA);
    }

    for (uint32_t iRev = 0; iRev < BL_ARRAY_SIZE(fat_tags); iRev++) {
      uint32_t i = BL_ARRAY_SIZE(fat_tags) - 1 - iRev;
      EXPECT_SUCCESS(ffsB.remove_value(fat_tags[i]));
      verify_font_feature_settings(ffsB);
    }

    EXPECT_EQ(ffsA, ffsB);
  }

  INFO("Dynamic representation");
  {
    BLFontFeatureSettings ffs;

    for (uint32_t i = 0; i < FontTagData::kFeatureIdCount; i++) {
      uint32_t feature_id = FontTagData::kFeatureIdCount - 1 - i;
      BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];
      EXPECT_SUCCESS(ffs.set_value(feature_tag, 1u));
      EXPECT_EQ(ffs.get_value(feature_tag), 1u);
      EXPECT_EQ(ffs.size(), i + 1u);
      verify_font_feature_settings(ffs);
    }

    EXPECT_FALSE(ffs._d.sso());

    for (uint32_t i = 0; i < FontTagData::kFeatureIdCount; i++) {
      uint32_t feature_id = FontTagData::kFeatureIdCount - 1 - i;
      BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];

      if (!FontTagData::feature_info_table[feature_id].has_bit_id()) {
        EXPECT_SUCCESS(ffs.set_value(feature_tag, 65535u));
        EXPECT_EQ(ffs.get_value(feature_tag), 65535u);
      }
      else {
        EXPECT_SUCCESS(ffs.set_value(feature_tag, 0u));
        EXPECT_EQ(ffs.get_value(feature_tag), 0u);
      }

      verify_font_feature_settings(ffs);
    }

    EXPECT_FALSE(ffs._d.sso());

    for (uint32_t i = 0; i < FontTagData::kFeatureIdCount; i++) {
      uint32_t feature_id = i;
      BLTag feature_tag = FontTagData::feature_id_to_tag_table[feature_id];

      EXPECT_SUCCESS(ffs.remove_value(feature_tag));
      EXPECT_EQ(ffs.get_value(feature_tag), BL_FONT_FEATURE_INVALID_VALUE);

      verify_font_feature_settings(ffs);
    }

    EXPECT_TRUE(ffs.is_empty());
    EXPECT_EQ(ffs.size(), 0u);
    EXPECT_FALSE(ffs._d.sso());

  }

  INFO("Dynamic tag/value equality");
  {
    BLFontFeatureSettings ffs1;
    BLFontFeatureSettings ffs2;

    for (uint32_t i = 0; i < FontTagData::kFeatureIdCount; i++) {
      EXPECT_SUCCESS(ffs1.set_value(FontTagData::feature_id_to_tag_table[i], 1u));
      EXPECT_SUCCESS(ffs2.set_value(FontTagData::feature_id_to_tag_table[FontTagData::kFeatureIdCount - 1u - i], 1u));

      verify_font_feature_settings(ffs1);
      verify_font_feature_settings(ffs2);
    }

    EXPECT_EQ(ffs1, ffs2);
  }

  INFO("Dynamic tag/value vs SSO tag/value equality");
  {
    BLFontFeatureSettings ffs1;
    BLFontFeatureSettings ffs2;

    for (uint32_t i = 0; i < FontTagData::kFeatureIdCount; i++) {
      if (FontTagData::feature_info_table[i].has_bit_id()) {
        EXPECT_SUCCESS(ffs1.set_value(FontTagData::feature_id_to_tag_table[i], 1u));
        EXPECT_SUCCESS(ffs2.set_value(FontTagData::feature_id_to_tag_table[i], 1u));

        verify_font_feature_settings(ffs1);
        verify_font_feature_settings(ffs2);
      }
    }

    EXPECT_EQ(ffs1, ffs2);

    // Make ffs1 go out of SSO mode.
    EXPECT_SUCCESS(ffs1.set_value(BL_MAKE_TAG('a', 'a', 'a', 'a'), 1000));
    EXPECT_SUCCESS(ffs1.remove_value(BL_MAKE_TAG('a', 'a', 'a', 'a')));
    EXPECT_EQ(ffs1, ffs2);
    EXPECT_EQ(ffs2, ffs1);

    // Make ffs2 go out of SSO mode.
    EXPECT_SUCCESS(ffs2.set_value(BL_MAKE_TAG('a', 'a', 'a', 'a'), 1000));
    EXPECT_SUCCESS(ffs2.remove_value(BL_MAKE_TAG('a', 'a', 'a', 'a')));
    EXPECT_EQ(ffs1, ffs2);
    EXPECT_EQ(ffs2, ffs1);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
