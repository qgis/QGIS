// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTTAGDATAINFO_P_H_INCLUDED
#define BLEND2D_FONTTAGDATAINFO_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/fonttagdataids_p.h>
#include <blend2d/support/lookuptable_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \namespace bl::FontTagData
//! Namespace that provides private information regarding font features.

namespace bl {
namespace FontTagData {

static constexpr uint32_t kInvalidFeatureBitId = 63;

struct FeatureInfo {
  uint8_t enabled_by_default : 1;
  uint8_t user_control : 1;
  uint8_t bit_id : 6;

  BL_INLINE bool has_bit_id() const noexcept { return bit_id != kInvalidFeatureBitId; };
};

extern const LookupTable<FeatureInfo, kFeatureIdCount + 1u> feature_info_table;

extern const uint8_t feature_bit_id_to_feature_id_table[32];

static BL_INLINE FeatureId feature_bit_id_to_feature_id(uint32_t bit_id) noexcept {
  BL_ASSERT(bit_id < 32u);
  return FeatureId(feature_bit_id_to_feature_id_table[bit_id]);
}

static BL_INLINE uint32_t feature_id_to_feature_bit_id(FeatureId feature_id) noexcept {
  BL_ASSERT(uint32_t(feature_id) < kFeatureIdCount);
  return feature_info_table[size_t(feature_id)].bit_id;
}

} // {FontTagData}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FONTTAGDATAINFO_P_H_INCLUDED
