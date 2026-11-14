// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/fonttagdata_p.h>

// bl::FontTagData - Tests
// =======================

namespace bl {
namespace Tests {

typedef uint32_t (*TagToIdFunc)(BLTag tag);

static void verify_tags(const char* category, const BLTag* tags, uint32_t count, TagToIdFunc tag_to_id_func) noexcept {
  if (count == 0)
    return;

  INFO("Verifying whether the %s tag data is sorted", category);
  BLTag prev = tags[0];
  for (uint32_t i = 1; i < count; i++) {
    EXPECT_LT(prev, tags[i]);
    prev = tags[i];
  }

  INFO("Verifying whether the %s tag to id translation is correct", category);
  for (uint32_t i = 0; i < count; i++) {
    uint32_t id = tag_to_id_func(tags[i]);
    EXPECT_EQ(id, i);
  }
}

UNIT(fonttagdata_ids, BL_TEST_GROUP_TEXT_OPENTYPE) {
  verify_tags("table_id", FontTagData::table_id_to_tag_table, FontTagData::kTableIdCount, FontTagData::table_tag_to_id);
  verify_tags("script_id", FontTagData::script_id_to_tag_table, FontTagData::kScriptIdCount, FontTagData::script_tag_to_id);
  verify_tags("language_id", FontTagData::language_id_to_tag_table, FontTagData::kLanguageIdCount, FontTagData::language_tag_to_id);
  verify_tags("feature_id", FontTagData::feature_id_to_tag_table, FontTagData::kFeatureIdCount, FontTagData::feature_tag_to_id);
  verify_tags("baseline_id", FontTagData::baseline_id_to_tag_table, FontTagData::kBaselineIdCount, FontTagData::baseline_tag_to_id);
  verify_tags("variation_id", FontTagData::variation_id_to_tag_table, FontTagData::kVariationIdCount, FontTagData::variation_tag_to_id);
}

} // {Tests}
} // {bl}

#endif // BL_TEST
