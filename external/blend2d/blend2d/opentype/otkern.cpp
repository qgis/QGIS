// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/trace_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otkern_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl::OpenType {
namespace KernImpl {

// bl::OpenType::KernImpl - Tracing
// ================================

#if defined(BL_TRACE_OT_ALL) || defined(BL_TRACE_OT_KERN)
  #define Trace BLDebugTrace
#else
  #define Trace BLDummyTrace
#endif

// bl::OpenType::KernImpl - Lookup Tables
// ======================================

static const uint8_t min_kern_sub_table_size[4] = {
  uint8_t(sizeof(KernTable::Format0)),
  uint8_t(sizeof(KernTable::Format1)),
  uint8_t(sizeof(KernTable::Format2) + 6 + 2), // Includes class table and a single kerning value.
  uint8_t(sizeof(KernTable::Format3))
};

// bl::OpenType::KernImpl - Match
// ==============================

struct KernMatch {
  uint32_t combined;
  BL_INLINE KernMatch(uint32_t combined) noexcept : combined(combined) {}
};
static BL_INLINE bool operator==(const KernTable::Pair& a, const KernMatch& b) noexcept { return a.combined() == b.combined; }
static BL_INLINE bool operator<=(const KernTable::Pair& a, const KernMatch& b) noexcept { return a.combined() <= b.combined; }

// bl::OpenType::KernImpl - Utilities
// ==================================

// Used to define a range of unsorted kerning pairs.
struct UnsortedRange {
  BL_INLINE void reset(uint32_t start_, uint32_t end_) noexcept {
    this->start = start_;
    this->end = end_;
  }

  uint32_t start;
  uint32_t end;
};

// Checks whether the pairs in `pair_array` are sorted and can be b-searched. The last `start` arguments
// specifies the start index from which the check should start as this is required by some utilities here.
static size_t check_kern_pairs(const KernTable::Pair* pair_array, size_t pair_count, size_t start) noexcept {
  if (start >= pair_count)
    return pair_count;

  size_t i;
  uint32_t prev = pair_array[start].combined();

  for (i = start; i < pair_count; i++) {
    uint32_t pair = pair_array[i].combined();
    // We must use `prev > pair`, because some fonts have kerning pairs duplicated for no reason (the same
    // values repeated). This doesn't violate the binary search requirements so we are okay with it.
    if (BL_UNLIKELY(prev > pair))
      break;
    prev = pair;
  }

  return i;
}

// Finds ranges of sorted pairs that can be used and creates ranges of unsorted pairs that will be merged into a
// single (synthesized) range of pairs. This function is only called if the kerning data in 'kern' is not sorted,
// and thus has to be fixed.
static BLResult fix_unsorted_kern_pairs(KernCollection& collection, const KernTable::Format0* fmt_data, uint32_t data_offset, uint32_t pair_count, size_t current_index, uint32_t group_flags, Trace trace) noexcept {
  typedef KernTable::Pair Pair;

  enum : uint32_t {
    kMaxGroups    = 8, // Maximum number of sub-ranges of sorted pairs.
    kMinPairCount = 32 // Minimum number of pairs in a sub-range.
  };

  size_t range_start = 0;
  size_t unsorted_start = 0;
  size_t threshold = bl_max<size_t>((pair_count - range_start) / kMaxGroups, kMinPairCount);

  // Small ranges that are unsorted will be copied into a single one and then sorted.
  // Number of ranges must be `kMaxGroups + 1` to consider also a last trailing range.
  UnsortedRange unsorted_ranges[kMaxGroups + 1];
  size_t unsorted_count = 0;
  size_t unsorted_pair_sum = 0;

  BL_PROPAGATE(collection.groups.reserve(collection.groups.size() + kMaxGroups + 1));
  for (;;) {
    size_t range_length = (current_index - range_start);

    if (range_length >= threshold) {
      if (range_start != unsorted_start) {
        BL_ASSERT(unsorted_count < BL_ARRAY_SIZE(unsorted_ranges));

        unsorted_ranges[unsorted_count].reset(uint32_t(unsorted_start), uint32_t(range_start));
        unsorted_pair_sum += range_start - unsorted_start;
        unsorted_count++;
      }

      unsorted_start = current_index;
      uint32_t sub_offset = uint32_t(data_offset + range_start * sizeof(Pair));

      // Cannot fail as we reserved enough.
      trace.warn("Adding Sorted Range [%zu:%zu]\n", range_start, current_index);
      collection.groups.append(KernGroup::make_referenced(0, group_flags, sub_offset, uint32_t(range_length)));
    }

    range_start = current_index;
    if (current_index == pair_count)
      break;

    current_index = check_kern_pairs(fmt_data->pair_array(), pair_count, current_index);
  }

  // Trailing unsorted range.
  if (unsorted_start != pair_count) {
    BL_ASSERT(unsorted_count < BL_ARRAY_SIZE(unsorted_ranges));

    unsorted_ranges[unsorted_count].reset(uint32_t(unsorted_start), uint32_t(range_start));
    unsorted_pair_sum += pair_count - unsorted_start;
    unsorted_count++;
  }

  if (unsorted_pair_sum) {
    Pair* synthesized_pairs = static_cast<Pair*>(malloc(unsorted_pair_sum * sizeof(Pair)));
    if (BL_UNLIKELY(!synthesized_pairs))
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    size_t synthesized_index = 0;
    for (size_t i = 0; i < unsorted_count; i++) {
      UnsortedRange& r = unsorted_ranges[i];
      size_t range_length = (r.end - r.start);

      trace.warn("Adding Synthesized Range [%zu:%zu]\n", size_t(r.start), size_t(r.end));
      memcpy(synthesized_pairs + synthesized_index, fmt_data->pair_array() + r.start, range_length * sizeof(Pair));

      synthesized_index += range_length;
    }
    BL_ASSERT(synthesized_index == unsorted_pair_sum);

    quick_sort(synthesized_pairs, unsorted_pair_sum, [](const Pair& a, const Pair& b) noexcept -> int {
      uint32_t a_combined = a.combined();
      uint32_t b_combined = b.combined();
      return a_combined < b_combined ? -1 : a_combined > b_combined ? 1 : 0;
    });

    // Cannot fail as we reserved enough.
    collection.groups.append(KernGroup::make_synthesized(0, group_flags, synthesized_pairs, uint32_t(unsorted_pair_sum)));
  }

  return BL_SUCCESS;
}

static BL_INLINE size_t find_kern_pair(const KernTable::Pair* pairs, size_t count, uint32_t pair) noexcept {
  return binary_search(pairs, count, KernMatch(pair));
}

// bl::OpenType::KernImpl - Apply
// ==============================

static constexpr int32_t kKernMaskOverride = 0x0;
static constexpr int32_t kKernMaskMinimum = 0x1;
static constexpr int32_t kKernMaskCombine = -1;

// Calculates the mask required by `combine_kern_value()` from coverage `flags`.
static BL_INLINE int32_t mask_from_kern_group_flags(uint32_t flags) noexcept {
  if (flags & KernGroup::kFlagOverride)
    return kKernMaskOverride;
  else if (flags & KernGroup::kFlagMinimum)
    return kKernMaskMinimum;
  else
    return kKernMaskCombine;
}

// There are several options of combining the kerning value with the previous one. The most common is simply adding
// these two together, but there are also minimum and override (aka replace) functions that we handle here.
static BL_INLINE int32_t combine_kern_value(int32_t orig_val, int32_t new_val, int32_t mask) noexcept {
  if (mask == kKernMaskMinimum)
    return bl_min<int32_t>(orig_val, new_val); // Handles 'minimum' function.
  else
    return (orig_val & mask) + new_val;       // Handles both 'add' and 'override' functions.
}

// Kern SubTable Format 0 - Ordered list of kerning pairs.
static BL_INLINE int32_t apply_kern_format0(const OTFaceImpl* ot_face_impl, const void* data_ptr, size_t data_size, uint32_t* glyph_data, BLGlyphPlacement* placement_data, size_t count, int32_t mask) noexcept {
  bl_unused(ot_face_impl);

  // Format0's `data_ptr` is not a pointer to the start of the table, instead it points to kerning pairs that are
  // either references to the original font data or synthesized in case that the data was wrong or not sorted.
  const KernTable::Pair* pair_data = static_cast<const KernTable::Pair*>(data_ptr);
  size_t pair_count = data_size;

  int32_t all_combined = 0;
  uint32_t pair = glyph_data[0] << 16;

  for (size_t i = 1; i < count; i++, pair <<= 16) {
    pair |= glyph_data[i];

    size_t index = find_kern_pair(pair_data, pair_count, pair);
    if (index == SIZE_MAX)
      continue;

    int32_t value = pair_data[index].value();
    int32_t combined = combine_kern_value(placement_data[i].placement.x, value, mask);

    placement_data[i].placement.x = combined;
    all_combined |= combined;
  }

  return all_combined;
}

// Kern SubTable Format 2 - Simple NxM array of kerning values.
static BL_INLINE int32_t apply_kern_format2(const OTFaceImpl* ot_face_impl, const void* data_ptr, size_t data_size, uint32_t* glyph_data, BLGlyphPlacement* placement_data, size_t count, int32_t mask) noexcept {
  typedef KernTable::Format2 Format2;
  typedef Format2::ClassTable ClassTable;

  const Format2* sub_table = PtrOps::offset<const Format2>(data_ptr, ot_face_impl->kern.header_size);
  uint32_t left_class_table_offset = sub_table->left_class_table();
  uint32_t right_class_table_offset = sub_table->right_class_table();

  if (BL_UNLIKELY(bl_max(left_class_table_offset, right_class_table_offset) > data_size - sizeof(ClassTable)))
    return 0;

  const ClassTable* left_class_table = PtrOps::offset<const ClassTable>(data_ptr, left_class_table_offset);
  const ClassTable* right_class_table = PtrOps::offset<const ClassTable>(data_ptr, right_class_table_offset);

  uint32_t left_glyph_count = left_class_table->glyph_count();
  uint32_t right_glyph_count = right_class_table->glyph_count();

  uint32_t left_table_end = left_class_table_offset + 4u + left_glyph_count * 2u;
  uint32_t right_table_end = right_class_table_offset + 4u + right_glyph_count * 2u;

  if (BL_UNLIKELY(bl_max(left_table_end, right_table_end) > data_size))
    return 0;

  uint32_t left_first_glyph = left_class_table->first_glyph();
  uint32_t right_first_glyph = right_class_table->first_glyph();

  int32_t all_combined = 0;
  uint32_t left_glyph = glyph_data[0];
  uint32_t right_glyph = 0;

  for (size_t i = 1; i < count; i++, left_glyph = right_glyph) {
    right_glyph = glyph_data[i];

    uint32_t left_index  = left_glyph - left_first_glyph;
    uint32_t right_index = right_glyph - right_first_glyph;

    if ((left_index >= left_glyph_count) | (right_index >= right_glyph_count))
      continue;

    uint32_t left_class = left_class_table->offset_array()[left_index].value();
    uint32_t right_class = right_class_table->offset_array()[right_index].value();

    // Cannot overflow as both components are unsigned 16-bit integers.
    uint32_t value_offset = left_class + right_class;
    if (left_class * right_class == 0 || value_offset > data_size - 2u)
      continue;

    int32_t value = PtrOps::offset<const FWord>(data_ptr, value_offset)->value();
    int32_t combined = combine_kern_value(placement_data[i].placement.x, value, mask);

    placement_data[i].placement.x = combined;
    all_combined |= combined;
  }

  return all_combined;
}

// Kern SubTable Format 3 - Simple NxM array of kerning indexes.
static BL_INLINE int32_t apply_kern_format3(const OTFaceImpl* ot_face_impl, const void* data_ptr, size_t data_size, uint32_t* glyph_data, BLGlyphPlacement* placement_data, size_t count, int32_t mask) noexcept {
  typedef KernTable::Format3 Format3;

  const Format3* sub_table = PtrOps::offset<const Format3>(data_ptr, ot_face_impl->kern.header_size);
  uint32_t glyph_count = sub_table->glyph_count();
  uint32_t kern_value_count = sub_table->kern_value_count();
  uint32_t left_class_count = sub_table->left_class_count();
  uint32_t right_class_count = sub_table->right_class_count();

  uint32_t required_size = ot_face_impl->kern.header_size + uint32_t(sizeof(Format3)) + kern_value_count * 2u + glyph_count * 2u + left_class_count * right_class_count;
  if (BL_UNLIKELY(required_size < data_size))
    return 0;

  const FWord* value_table = PtrOps::offset<const FWord>(sub_table, sizeof(Format3));
  const UInt8* class_table = PtrOps::offset<const UInt8>(value_table, kern_value_count * 2u);
  const UInt8* index_table = class_table + glyph_count * 2u;

  int32_t all_combined = 0;
  uint32_t left_glyph = glyph_data[0];
  uint32_t right_glyph = 0;

  for (size_t i = 1; i < count; i++, left_glyph = right_glyph) {
    right_glyph = glyph_data[i];
    if (bl_max(left_glyph, right_glyph) >= glyph_count)
      continue;

    uint32_t left_class = class_table[left_glyph].value();
    uint32_t right_class = class_table[glyph_count + right_glyph].value();

    if ((left_class >= left_class_count) | (right_class >= right_class_count))
      continue;

    uint32_t value_index = index_table[left_class * right_class_count + right_class].value();
    if (value_index >= kern_value_count)
      continue;

    int32_t value = value_table[value_index].value();
    int32_t combined = combine_kern_value(placement_data[i].placement.x, value, mask);

    placement_data[i].placement.x = combined;
    all_combined |= combined;
  }

  return all_combined;
}

// Applies the data calculated by apply_kern_formatN.
static BL_INLINE void finish_kern(const OTFaceImpl* ot_face_impl, uint32_t* glyph_data, BLGlyphPlacement* placement_data, size_t count) noexcept {
  bl_unused(ot_face_impl, glyph_data);
  for (size_t i = 1; i < count; i++) {
    placement_data[i - 1].advance += placement_data[i].placement;
    placement_data[i].placement.reset();
  }
}

static BLResult BL_CDECL apply_kern(const BLFontFaceImpl* face_impl, uint32_t* glyph_data, BLGlyphPlacement* placement_data, size_t count) noexcept {
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  if (count < 2)
    return BL_SUCCESS;

  const void* base_ptr = ot_face_impl->kern.table.data;
  const KernCollection& collection = ot_face_impl->kern.collection[0];

  const KernGroup* kern_groups = collection.groups.data();
  size_t group_count = collection.groups.size();

  int32_t all_combined = 0;

  for (size_t group_index = 0; group_index < group_count; group_index++) {
    const KernGroup& kern_group = kern_groups[group_index];

    const void* data_ptr = kern_group.calc_data_ptr(base_ptr);
    size_t data_size = kern_group.data_size();

    uint32_t format = kern_group.format();
    int32_t mask = mask_from_kern_group_flags(kern_group.flags());

    switch (format) {
      case 0: all_combined |= apply_kern_format0(ot_face_impl, data_ptr, data_size, glyph_data, placement_data, count, mask); break;
      case 2: all_combined |= apply_kern_format2(ot_face_impl, data_ptr, data_size, glyph_data, placement_data, count, mask); break;
      case 3: all_combined |= apply_kern_format3(ot_face_impl, data_ptr, data_size, glyph_data, placement_data, count, mask); break;
    }
  }

  // Only finish kerning if we actually did something, if no kerning pair was found or all kerning pairs were
  // zero then there is nothing to do.
  if (all_combined)
    finish_kern(ot_face_impl, glyph_data, placement_data, count);

  return BL_SUCCESS;
}

// bl::OpenType::KernImpl - Init
// =============================

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  typedef KernTable::WinGroupHeader WinGroupHeader;
  typedef KernTable::MacGroupHeader MacGroupHeader;

  Table<KernTable> kern = tables.kern;
  if (!kern)
    return BL_SUCCESS;

  Trace trace;
  trace.info("bl::OpenType::OTFaceImpl::Init 'kern' [Size=%zu]\n", kern.size);
  trace.indent();

  if (BL_UNLIKELY(!kern.fits())) {
    trace.warn("Table is truncated\n");
    ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_KERN_DATA;
    return BL_SUCCESS;
  }

  const uint8_t* data_ptr = kern.data;
  const uint8_t* data_end = data_ptr + kern.size;

  // Kern Header
  // -----------

  // Detect the header format. Windows header uses 16-bit field describing the version of the table and only defines
  // version 0. Apple uses a different header format which uses a 32-bit version number (`F16x16`). Luckily we can
  // distinguish between these two easily.
  uint32_t major_version = MemOps::readU16uBE(data_ptr);

  uint32_t header_type = 0xFFu;
  uint32_t header_size = 0;
  uint32_t group_count = 0;

  if (major_version == 0) {
    header_type = KernData::kHeaderWindows;
    header_size = uint32_t(sizeof(WinGroupHeader));
    group_count = MemOps::readU16uBE(data_ptr + 2u);

    trace.info("Version: 0 (WINDOWS)\n");
    trace.info("GroupCount: %u\n", group_count);

    // Not forbidden by the spec, just ignore the table if true.
    if (!group_count) {
      trace.warn("No kerning pairs defined\n");
      return BL_SUCCESS;
    }

    data_ptr += 4;
  }
  else if (major_version == 1) {
    uint32_t minor_version = MemOps::readU16uBE(data_ptr + 2u);
    trace.info("Version: 1 (MAC)\n");

    if (minor_version != 0) {
      trace.warn("Invalid minor version (%u)\n", minor_version);
      ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_KERN_DATA;
      return BL_SUCCESS;
    }

    // Minimum mac header is 8 bytes. We have to check this explicitly as the minimum size of "any" header is 4 bytes,
    // so make sure we won't read beyond.
    if (kern.size < 8u) {
      trace.warn("InvalidSize: %zu\n", size_t(kern.size));
      ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_KERN_DATA;
      return BL_SUCCESS;
    }

    header_type = KernData::kHeaderMac;
    header_size = uint32_t(sizeof(MacGroupHeader));

    group_count = MemOps::readU32uBE(data_ptr + 4u);
    trace.info("GroupCount: %u\n", group_count);

    // Not forbidden by the spec, just ignore the table if true.
    if (!group_count) {
      trace.warn("No kerning pairs defined\n");
      return BL_SUCCESS;
    }

    data_ptr += 8;
  }
  else {
    trace.info("Version: %u (UNKNOWN)\n", major_version);

    // No other major version is defined by OpenType. Since KERN table has been superseded by "GPOS" table there will
    // never be any other version.
    trace.fail("Invalid version");
    ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_KERN_DATA;
    return BL_SUCCESS;
  }

  ot_face_impl->kern.header_type = uint8_t(header_type);
  ot_face_impl->kern.header_size = uint8_t(header_size);

  // Kern Groups
  // -----------

  uint32_t group_index = 0;
  do {
    size_t remaining_size = PtrOps::bytes_until(data_ptr, data_end);
    if (BL_UNLIKELY(remaining_size < header_size)) {
      trace.warn("No more data for group #%u\n", group_index);
      break;
    }

    uint32_t length = 0;
    uint32_t format = 0;
    uint32_t coverage = 0;

    trace.info("Group #%u\n", group_index);
    trace.indent();

    if (header_type == KernData::kHeaderWindows) {
      const WinGroupHeader* group = reinterpret_cast<const WinGroupHeader*>(data_ptr);

      format = group->format();
      length = group->length();

      // Some fonts having only one group have an incorrect length set to the same value as the as the whole 'kern'
      // table. Detect it and fix it.
      if (length == kern.size && group_count == 1) {
        length = uint32_t(remaining_size);
        trace.warn("Group length is same as the table length, fixed to %u\n", length);
      }

      // The last sub-table can have truncated length to 16 bits even when it needs more to represent all kerning
      // pairs. This is not covered by the specification, but it's a common practice.
      if (length != remaining_size && group_index == group_count - 1) {
        trace.warn("Fixing reported length from %u to %zu\n", length, remaining_size);
        length = uint32_t(remaining_size);
      }

      // Not interested in undefined flags.
      coverage = group->coverage() & ~WinGroupHeader::kCoverageReservedBits;
    }
    else {
      const MacGroupHeader* group = reinterpret_cast<const MacGroupHeader*>(data_ptr);

      format = group->format();
      length = group->length();

      // Translate coverate flags from MAC format to Windows format that we prefer.
      uint32_t mac_coverage = group->coverage();
      if ((mac_coverage & MacGroupHeader::kCoverageVertical   ) == 0) coverage |= WinGroupHeader::kCoverageHorizontal;
      if ((mac_coverage & MacGroupHeader::kCoverageCrossStream) != 0) coverage |= WinGroupHeader::kCoverageCrossStream;
    }

    if (length < header_size) {
      trace.fail("Group length too small [Length=%u RemainingSize=%zu]\n", length, remaining_size);
      ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_KERN_DATA;
      return BL_SUCCESS;
    }

    if (length > remaining_size) {
      trace.fail("Group length exceeds the remaining space [Length=%u RemainingSize=%zu]\n", length, remaining_size);
      ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_KERN_DATA;
      return BL_SUCCESS;
    }

    // Move to the beginning of the content of the group.
    data_ptr += header_size;

    // It's easier to calculate everything without the header (as its size is variable), so make `length` raw data size
    // that we will store in KernData.
    length -= header_size;
    remaining_size -= header_size;

    // Even on 64-bit machine this cannot overflow as a table length in SFNT header is stored as UInt32.
    uint32_t offset = (uint32_t)PtrOps::byte_offset(kern.data, data_ptr);
    uint32_t orientation = (coverage & WinGroupHeader::kCoverageHorizontal) ? BL_ORIENTATION_HORIZONTAL : BL_ORIENTATION_VERTICAL;
    uint32_t group_flags = coverage & (KernGroup::kFlagMinimum | KernGroup::kFlagCrossStream | KernGroup::kFlagOverride);

    trace.info("Format: %u%s\n", format, format > 3 ? " (UNKNOWN)" : "");
    trace.info("Coverage: %u\n", coverage);
    trace.info("Orientation: %s\n", orientation == BL_ORIENTATION_HORIZONTAL ? "Horizontal" : "Vertical");

    if (format < BL_ARRAY_SIZE(min_kern_sub_table_size) && length >= min_kern_sub_table_size[format]) {
      KernCollection& collection = ot_face_impl->kern.collection[orientation];
      switch (format) {
        // Kern SubTable Format 0 - Ordered list of kerning pairs.
        case 0: {
          const KernTable::Format0* fmt_data = reinterpret_cast<const KernTable::Format0*>(data_ptr);
          uint32_t pair_count = fmt_data->pair_count();
          trace.info("PairCount=%zu\n", pair_count);

          if (pair_count == 0)
            break;

          uint32_t pair_data_offset = offset + 8;
          uint32_t pair_data_size = pair_count * uint32_t(sizeof(KernTable::Pair)) + uint32_t(sizeof(KernTable::Format0));

          if (BL_UNLIKELY(pair_data_size > length)) {
            uint32_t fixed_pair_count = (length - uint32_t(sizeof(KernTable::Format0))) / 6;
            trace.warn("Fixing the number of pairs from [%u] to [%u] to match the remaining size [%u]\n", pair_count, fixed_pair_count, length);

            ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_FIXED_KERN_DATA;
            pair_count = fixed_pair_count;
          }

          // Check whether the pairs are sorted.
          const KernTable::Pair* pair_data = fmt_data->pair_array();
          size_t unsorted_index = check_kern_pairs(pair_data, pair_count, 0);

          if (unsorted_index != pair_count) {
            trace.warn("Pair #%zu violates ordering constraint (kerning pairs are not sorted)\n", unsorted_index);

            BLResult result = fix_unsorted_kern_pairs(collection, fmt_data, pair_data_offset, pair_count, unsorted_index, group_flags, trace);
            if (result != BL_SUCCESS) {
              trace.fail("Cannot allocate data for synthesized kerning pairs\n");
              return result;
            }

            ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_FIXED_KERN_DATA;
            break;
          }
          else {
            BLResult result = collection.groups.append(KernGroup::make_referenced(0, group_flags, pair_data_offset, uint32_t(pair_count)));
            if (result != BL_SUCCESS) {
              trace.fail("Cannot allocate data for referenced kerning pairs\n");
              return result;
            }
          }

          break;
        }

        // Kern SubTable Format 2 - Simple NxM array of kerning values.
        case 2: {
          const void* sub_table = static_cast<const uint8_t*>(data_ptr) - header_size;
          size_t sub_table_size = length + header_size;

          const KernTable::Format2* fmt_data = reinterpret_cast<const KernTable::Format2*>(data_ptr);
          uint32_t left_class_table_offset = fmt_data->left_class_table();
          uint32_t right_class_table_offset = fmt_data->right_class_table();
          uint32_t kerning_array_offset = fmt_data->kerning_array();

          if (left_class_table_offset > sub_table_size - 6u) {
            trace.warn("Invalid offset [%u] of left ClassTable\n", unsigned(left_class_table_offset));
            break;
          }

          if (right_class_table_offset > sub_table_size - 6u) {
            trace.warn("Invalid offset [%u] of right ClassTable\n", unsigned(right_class_table_offset));
            break;
          }

          if (kerning_array_offset > sub_table_size - 2u) {
            trace.warn("Invalid offset [%u] of KerningArray\n", unsigned(kerning_array_offset));
            break;
          }

          const KernTable::Format2::ClassTable* left_class_table = PtrOps::offset<const KernTable::Format2::ClassTable>(sub_table, left_class_table_offset);
          const KernTable::Format2::ClassTable* right_class_table = PtrOps::offset<const KernTable::Format2::ClassTable>(sub_table, right_class_table_offset);

          uint32_t left_glyph_count = left_class_table->glyph_count();
          uint32_t right_glyph_count = right_class_table->glyph_count();

          uint32_t left_table_size = left_class_table_offset + 4u + left_glyph_count * 2u;
          uint32_t right_table_size = right_class_table_offset + 4u + right_glyph_count * 2u;

          if (left_table_size > sub_table_size) {
            trace.warn("Left ClassTable's GlyphCount [%u] overflows table size by [%zu] bytes\n", unsigned(left_glyph_count), size_t(left_table_size - sub_table_size));
            break;
          }

          if (right_table_size > sub_table_size) {
            trace.warn("Right ClassTable's GlyphCount [%u] overflows table size by [%zu] bytes\n", unsigned(right_glyph_count), size_t(right_table_size - sub_table_size));
            break;
          }

          BLResult result = collection.groups.append(KernGroup::make_referenced(format, group_flags, offset - header_size, uint32_t(sub_table_size)));
          if (result != BL_SUCCESS) {
            trace.fail("Cannot allocate data for a referenced kerning group of format #%u\n", unsigned(format));
            return result;
          }

          break;
        }

        // Kern SubTable Format 3 - Simple NxM array of kerning indexes.
        case 3: {
          size_t sub_table_size = length + header_size;

          const KernTable::Format3* fmt_data = reinterpret_cast<const KernTable::Format3*>(data_ptr);
          uint32_t glyph_count = fmt_data->glyph_count();
          uint32_t kern_value_count = fmt_data->kern_value_count();
          uint32_t left_class_count = fmt_data->left_class_count();
          uint32_t right_class_count = fmt_data->right_class_count();

          uint32_t required_size = ot_face_impl->kern.header_size + uint32_t(sizeof(KernTable::Format3)) + kern_value_count * 2u + glyph_count * 2u + left_class_count * right_class_count;
          if (BL_UNLIKELY(required_size > sub_table_size)) {
            trace.warn("Kerning table data overflows the table size by [%zu] bytes\n", size_t(required_size - sub_table_size));
            break;
          }

          BLResult result = collection.groups.append(KernGroup::make_referenced(format, group_flags, offset - header_size, uint32_t(sub_table_size)));
          if (result != BL_SUCCESS) {
            trace.fail("Cannot allocate data for a referenced kerning group of format #%u\n", unsigned(format));
            return result;
          }

          break;
        }

        default:
          ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_KERN_DATA;
          break;
      }
    }
    else {
      trace.warn("Skipping subtable\n");
    }

    trace.deindent();
    data_ptr += length;
  } while (++group_index < group_count);

  if (!ot_face_impl->kern.collection[BL_ORIENTATION_HORIZONTAL].is_empty()) {
    ot_face_impl->kern.table = kern;
    ot_face_impl->kern.collection[BL_ORIENTATION_HORIZONTAL].groups.shrink();
    ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_HORIZONTAL_KERNING;
    ot_face_impl->feature_tag_set._add_known_tag_id(uint32_t(FontTagData::FeatureId::kKERN));
    ot_face_impl->funcs.apply_kern = apply_kern;
    ot_face_impl->ot_flags |= OTFaceFlags::kLegacyKernAvailable;
  }

  return BL_SUCCESS;
}

} // {KernImpl}
} // {bl::OpenType}
