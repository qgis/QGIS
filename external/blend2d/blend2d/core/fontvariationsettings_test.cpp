// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/fonttagdata_p.h>
#include <blend2d/core/fontvariationsettings_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/support/math_p.h>

// bl::FontVariationSettings - Tests
// =================================

namespace bl {
namespace Tests {

static void verify_font_variation_settings(const BLFontVariationSettings& ffs) noexcept {
  BLFontVariationSettingsView view;
  ffs.get_view(&view);

  if (view.size == 0)
    return;

  uint32_t prev_tag = view.data[0].tag;
  for (size_t i = 1; i < view.size; i++) {
    EXPECT_LT(prev_tag, view.data[i].tag)
      .message("BLFontVariationSettings is corrupted - previous tag 0x%08X is not less than current tag 0x%08X at [%zu]", prev_tag, view.data[i].tag, i);
  }
}

UNIT(font_variation_settings_allocation_strategy, BL_TEST_GROUP_TEXT_CONTAINERS) {
  BLFontVariationSettings ffs;
  size_t capacity = ffs.capacity();

  constexpr uint32_t kCharRange = FontTagData::kCharRangeInTag;
  constexpr uint32_t kNumItems = FontTagData::kUniqueTagCount / 100;

  for (uint32_t i = 0; i < kNumItems; i++) {
    BLTag tag = BL_MAKE_TAG(
      uint32_t(' ') + (i / (kCharRange * kCharRange * kCharRange)),
      uint32_t(' ') + (i / (kCharRange * kCharRange)) % kCharRange,
      uint32_t(' ') + (i / (kCharRange)) % kCharRange,
      uint32_t(' ') + (i % kCharRange));

    ffs.set_value(tag, float(i & 0xFFFFu));
    if (capacity != ffs.capacity()) {
      size_t impl_size = FontVariationSettingsInternal::impl_size_from_capacity(ffs.capacity()).value();
      INFO("Capacity increased from %zu to %zu [ImplSize=%zu]\n", capacity, ffs.capacity(), impl_size);
      capacity = ffs.capacity();
    }
  }

  verify_font_variation_settings(ffs);
}

UNIT(font_variation_settings, BL_TEST_GROUP_TEXT_CONTAINERS) {
  // These are not sorted on purpose - we want BLFontVariationSettings to sort them.
  static const uint32_t sso_tags[] = {
    BL_MAKE_TAG('w', 'g', 'h', 't'),
    BL_MAKE_TAG('i', 't', 'a', 'l'),
    BL_MAKE_TAG('w', 'd', 't', 'h')
  };

  static const uint32_t dynamic_tags[] = {
    BL_MAKE_TAG('w', 'g', 'h', 't'),
    BL_MAKE_TAG('i', 't', 'a', 'l'),
    BL_MAKE_TAG('w', 'd', 't', 'h'),
    BL_MAKE_TAG('s', 'l', 'n', 't'),
    BL_MAKE_TAG('o', 'p', 's', 'z')
  };

  INFO("SSO representation");
  {
    BLFontVariationSettings ffs;

    EXPECT_TRUE(ffs._d.sso());
    EXPECT_TRUE(ffs.is_empty());
    EXPECT_EQ(ffs.size(), 0u);
    EXPECT_EQ(ffs.capacity(), BLFontVariationSettings::kSSOCapacity);

    // Getting an unknown tag should return invalid value.
    EXPECT_TRUE(Math::is_nan(ffs.get_value(BL_MAKE_TAG('-', '-', '-', '-'))));

    for (uint32_t i = 0; i < BL_ARRAY_SIZE(sso_tags); i++) {
      EXPECT_SUCCESS(ffs.set_value(sso_tags[i], 1u));
      EXPECT_EQ(ffs.get_value(sso_tags[i]), float(1));
      EXPECT_EQ(ffs.size(), i + 1);
      EXPECT_TRUE(ffs._d.sso());
      verify_font_variation_settings(ffs);
    }

    for (uint32_t i = 0; i < BL_ARRAY_SIZE(sso_tags); i++) {
      EXPECT_SUCCESS(ffs.set_value(sso_tags[i], 0u));
      EXPECT_EQ(ffs.get_value(sso_tags[i]), float(0));
      EXPECT_EQ(ffs.size(), BL_ARRAY_SIZE(sso_tags));
      EXPECT_TRUE(ffs._d.sso());
      verify_font_variation_settings(ffs);
    }

    for (uint32_t i = 0; i < BL_ARRAY_SIZE(sso_tags); i++) {
      EXPECT_SUCCESS(ffs.remove_value(sso_tags[i]));
      EXPECT_TRUE(Math::is_nan(ffs.get_value(sso_tags[i])));
      EXPECT_EQ(ffs.size(), BL_ARRAY_SIZE(sso_tags) - i - 1);
      EXPECT_TRUE(ffs._d.sso());
      verify_font_variation_settings(ffs);
    }
  }

  INFO("SSO border cases");
  {
    // First veriation ids use R/I bits, which is used for reference counted dynamic objects.
    // What we want to test here is that this bit is not checked when destroying SSO instances.
    BLFontVariationSettings settings;
    settings.set_value(FontTagData::variation_id_to_tag_table[0], 0.5f);
    settings.set_value(FontTagData::variation_id_to_tag_table[1], 0.5f);
  }

  INFO("Dynamic representation");
  {
    BLFontVariationSettings ffs;

    EXPECT_TRUE(ffs._d.sso());
    EXPECT_TRUE(ffs.is_empty());
    EXPECT_EQ(ffs.size(), 0u);
    EXPECT_EQ(ffs.capacity(), BLFontVariationSettings::kSSOCapacity);

    // Getting an unknown tag should return invalid value.
    EXPECT_TRUE(Math::is_nan(ffs.get_value(BL_MAKE_TAG('-', '-', '-', '-'))));

    for (uint32_t i = 0; i < BL_ARRAY_SIZE(dynamic_tags); i++) {
      EXPECT_SUCCESS(ffs.set_value(dynamic_tags[i], 1u));
      EXPECT_EQ(ffs.get_value(dynamic_tags[i]), float(1));
      EXPECT_EQ(ffs.size(), i + 1);
      verify_font_variation_settings(ffs);
    }

    EXPECT_FALSE(ffs._d.sso());

    for (uint32_t i = 0; i < BL_ARRAY_SIZE(dynamic_tags); i++) {
      EXPECT_SUCCESS(ffs.set_value(dynamic_tags[i], 0u));
      EXPECT_EQ(ffs.get_value(dynamic_tags[i]), float(0));
      EXPECT_EQ(ffs.size(), BL_ARRAY_SIZE(dynamic_tags));
      verify_font_variation_settings(ffs);
    }

    EXPECT_FALSE(ffs._d.sso());

    for (uint32_t i = 0; i < BL_ARRAY_SIZE(dynamic_tags); i++) {
      EXPECT_SUCCESS(ffs.remove_value(dynamic_tags[i]));
      EXPECT_TRUE(Math::is_nan(ffs.get_value(dynamic_tags[i])));
      EXPECT_EQ(ffs.size(), BL_ARRAY_SIZE(dynamic_tags) - i - 1);
      verify_font_variation_settings(ffs);
    }

    EXPECT_FALSE(ffs._d.sso());
  }

  INFO("Equality");
  {
    BLFontVariationSettings ffs1;
    BLFontVariationSettings ffs2;

    for (uint32_t i = 0; i < BL_ARRAY_SIZE(sso_tags); i++) {
      EXPECT_SUCCESS(ffs1.set_value(sso_tags[i], 1u));
      EXPECT_SUCCESS(ffs2.set_value(sso_tags[BL_ARRAY_SIZE(sso_tags) - i - 1], 1u));
    }

    EXPECT_TRUE(ffs1.equals(ffs2));

    // Make ffs1 go out of SSO mode.
    EXPECT_SUCCESS(ffs1.set_value(BL_MAKE_TAG('a', 'a', 'a', 'a'), 1));
    EXPECT_SUCCESS(ffs1.remove_value(BL_MAKE_TAG('a', 'a', 'a', 'a')));
    EXPECT_TRUE(ffs1.equals(ffs2));

    // Make ffs2 go out of SSO mode.
    EXPECT_SUCCESS(ffs2.set_value(BL_MAKE_TAG('a', 'a', 'a', 'a'), 1));
    EXPECT_SUCCESS(ffs2.remove_value(BL_MAKE_TAG('a', 'a', 'a', 'a')));
    EXPECT_TRUE(ffs1.equals(ffs2));
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
