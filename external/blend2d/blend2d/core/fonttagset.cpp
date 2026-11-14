// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/fonttagset_p.h>
#include <blend2d/support/algorithm_p.h>

namespace bl {
namespace FontTagData {

BLResult finalize_tag_array(BLArray<BLTag>& tags) noexcept {
  size_t size = tags.size();

  if (size > 1) {
    BLTag* data = nullptr;
    BL_PROPAGATE(tags.make_mutable(&data));

    // Sort and deduplicate afterwards.
    quick_sort(data, size);

    size_t j = 1;
    BLTag prev_tag = data[0];

    for (size_t i = 1; i < size; i++) {
      BLTag current_tag = data[i];
      if (current_tag == prev_tag) {
        continue;
      }

      data[j++] = current_tag;
      prev_tag = current_tag;
    }

    size = j;
    tags.resize(size, BLTag(0));
  }

  return tags.shrink();
}

BLResult flatten_tag_set_to(BLArray<BLTag>& dst,
  const BLTag* known_id_to_tag_table,
  const BLBitWord* known_tag_data, size_t known_tag_data_size, size_t known_tag_count,
  const BLTag* unknown_tag_data, size_t unknown_tag_count) noexcept {

  size_t tag_count = known_tag_count + unknown_tag_count;

  BLTag* dst_data = nullptr;
  BL_PROPAGATE(dst.modify_op(BL_MODIFY_OP_ASSIGN_FIT, tag_count, &dst_data));

  size_t dst_data_index = 0;
  size_t unknown_tag_index = 0;
  ParametrizedBitOps<BitOrder::kLSB, BLBitWord>::BitVectorIterator it(known_tag_data, known_tag_data_size);

  while (it.has_next()) {
    uint32_t tag_id = uint32_t(it.next());
    BLTag known_tag = known_id_to_tag_table[tag_id];

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

} // {FontTagData}
} // {bl}
