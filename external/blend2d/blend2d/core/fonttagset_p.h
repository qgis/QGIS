// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTTAGSET_P_H_INCLUDED
#define BLEND2D_FONTTAGSET_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array.h>
#include <blend2d/core/fonttagdataids_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/fixedbitarray_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace FontTagData {

BL_HIDDEN BLResult finalize_tag_array(BLArray<BLTag>& tags) noexcept;

BL_HIDDEN BLResult flatten_tag_set_to(BLArray<BLTag>& dst,
  const BLTag* known_id_to_tag_table,
  const BLBitWord* known_tag_data, size_t known_tag_data_size, size_t known_tag_count,
  const BLTag* unknown_tag_data, size_t unknown_tag_count) noexcept;

//! A set of known and unknown OpenType tags that can be used to build an array of tags regarding a single feature.
//! It optimizes the case for adding known tags (tags that have a corresponding ID in Blend2D tag database) over tags
//! that are not known (such tags are non-standard and could be totally unsupported by Blend2D anyway).
template<size_t kKnownTagCount>
class TagSet {
public:
  BLArray<BLTag> unknown_tags;
  FixedBitArray<BLBitWord, kKnownTagCount> known_tags {};
  size_t known_tag_count {};

  [[nodiscard]]
  BL_INLINE bool _has_tag(BLTag tag, uint32_t id) const noexcept {
    if (id != kInvalidId)
      return known_tags.bit_at(id);

    size_t index = lower_bound(unknown_tags.data(), unknown_tags.size(), id);
    return index < unknown_tags.size() && unknown_tags[index] == tag;
  }

  BL_INLINE BLResult _add_tag(BLTag tag, uint32_t id) noexcept {
    if (id != kInvalidId)
      return _add_known_tag_id(id);
    else
      return _add_unknown_tag(tag);
  }

  BL_INLINE BLResult _add_known_tag_id(uint32_t id) noexcept {
    BL_ASSERT(id < kKnownTagCount);

    known_tag_count += size_t(!known_tags.bit_at(id));
    known_tags.set_at(id);
    return BL_SUCCESS;
  }

  BL_INLINE BLResult _add_unknown_tag(BLTag tag) noexcept {
    return unknown_tags.append(tag);
  }

  // Called when it's known that no more tags will be added.
  BL_INLINE BLResult finalize() noexcept {
    return finalize_tag_array(unknown_tags);
  }

  BL_NOINLINE BLResult flatten_to(BLArray<BLTag>& dst, const BLTag* id_to_tag_table) noexcept {
    const BLTag* unknown_tag_data = unknown_tags.data();
    size_t unknown_tag_count = unknown_tags.size();
    size_t tag_count = known_tag_count + unknown_tag_count;

    BLTag* dst_data = nullptr;
    BL_PROPAGATE(dst.modify_op(BL_MODIFY_OP_ASSIGN_FIT, tag_count, &dst_data));

    size_t dst_data_index = 0;
    size_t unknown_tag_index = 0;
    ParametrizedBitOps<BitOrder::kLSB, BLBitWord>::BitVectorIterator it(known_tags.data, known_tags.size_in_words());

    while (it.has_next()) {
      uint32_t tag_id = it.next();
      BLTag known_tag = id_to_tag_table[tag_id];

      while (unknown_tag_index < unknown_tag_count && unknown_tag_data[unknown_tag_index] < known_tag) {
        dst_data[dst_data_index++] = unknown_tag_data[unknown_tag_index++];
      }

      dst_data[dst_data_index++] = known_tag;
    }

    while (unknown_tag_index < unknown_tag_count) {
      dst_data[dst_data_index++] = unknown_tag_data[unknown_tag_index++];
    }

    return BL_SUCCESS;
  }
};

class ScriptTagSet : public TagSet<kScriptIdCount> {
public:
  [[nodiscard]]
  BL_INLINE bool has_tag(BLTag tag) const noexcept {
    return _has_tag(tag, script_tag_to_id(tag));
  }

  [[nodiscard]]
  BL_INLINE bool has_known_tag(ScriptId id) const noexcept {
    return known_tags.bit_at(uint32_t(id));
  }

  BL_INLINE BLResult add_tag(BLTag tag) noexcept {
    return _add_tag(tag, script_tag_to_id(tag));
  }

  BL_INLINE BLResult flatten_to(BLArray<BLTag>& dst) const noexcept {
    return flatten_tag_set_to(dst, script_id_to_tag_table, known_tags.data, known_tags.size_in_words(), known_tag_count, unknown_tags.data(), unknown_tags.size());
  }
};

class LanguageTagSet : public TagSet<kLanguageIdCount> {
public:
  [[nodiscard]]
  BL_INLINE bool has_tag(BLTag tag) const noexcept {
    return _has_tag(tag, language_tag_to_id(tag));
  }

  [[nodiscard]]
  BL_INLINE bool has_known_tag(LanguageId id) const noexcept {
    return known_tags.bit_at(uint32_t(id));
  }

  BL_INLINE BLResult add_tag(BLTag tag) noexcept {
    return _add_tag(tag, language_tag_to_id(tag));
  }

  BL_INLINE BLResult flatten_to(BLArray<BLTag>& dst) const noexcept {
    return flatten_tag_set_to(dst, language_id_to_tag_table, known_tags.data, known_tags.size_in_words(), known_tag_count, unknown_tags.data(), unknown_tags.size());
  }
};

class FeatureTagSet : public TagSet<kFeatureIdCount> {
public:
  [[nodiscard]]
  BL_INLINE bool has_tag(BLTag tag) const noexcept {
    return _has_tag(tag, feature_tag_to_id(tag));
  }

  [[nodiscard]]
  BL_INLINE bool has_known_tag(FeatureId id) const noexcept {
    return known_tags.bit_at(uint32_t(id));
  }

  BL_INLINE BLResult add_tag(BLTag tag) noexcept {
    return _add_tag(tag, feature_tag_to_id(tag));
  }

  BL_INLINE BLResult flatten_to(BLArray<BLTag>& dst) const noexcept {
    return flatten_tag_set_to(dst, feature_id_to_tag_table, known_tags.data, known_tags.size_in_words(), known_tag_count, unknown_tags.data(), unknown_tags.size());
  }
};

class BaselineTagSet : public TagSet<kBaselineIdCount> {
public:
  [[nodiscard]]
  BL_INLINE bool has_tag(BLTag tag) const noexcept {
    return _has_tag(tag, baseline_tag_to_id(tag));
  }

  [[nodiscard]]
  BL_INLINE bool has_known_tag(BaselineId id) const noexcept {
    return known_tags.bit_at(uint32_t(id));
  }

  BL_INLINE BLResult add_tag(BLTag tag) noexcept {
    return _add_tag(tag, baseline_tag_to_id(tag));
  }

  BL_INLINE BLResult flatten_to(BLArray<BLTag>& dst) const noexcept {
    return flatten_tag_set_to(dst, baseline_id_to_tag_table, known_tags.data, known_tags.size_in_words(), known_tag_count, unknown_tags.data(), unknown_tags.size());
  }
};

class VariationTagSet : public TagSet<kVariationIdCount> {
public:
  [[nodiscard]]
  BL_INLINE bool has_tag(BLTag tag) const noexcept {
    return _has_tag(tag, variation_tag_to_id(tag));
  }

  [[nodiscard]]
  BL_INLINE bool has_known_tag(VariationId id) const noexcept {
    return known_tags.bit_at(uint32_t(id));
  }

  BL_INLINE BLResult add_tag(BLTag tag) noexcept {
    return _add_tag(tag, variation_tag_to_id(tag));
  }

  BL_INLINE BLResult flatten_to(BLArray<BLTag>& dst) const noexcept {
    return flatten_tag_set_to(dst, variation_id_to_tag_table, known_tags.data, known_tags.size_in_words(), known_tag_count, unknown_tags.data(), unknown_tags.size());
  }
};

} // {FontTagData}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FONTTAGSET_P_H_INCLUDED
