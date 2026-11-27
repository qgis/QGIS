// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/opentype/otcmap_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otplatform_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/unicode/unicode_p.h>

namespace bl::OpenType {
namespace CMapImpl {

// bl::OpenType::CMapImpl - None
// =============================

static BLResult BL_CDECL map_text_to_glyphs_none(const BLFontFaceImpl* face_impl, uint32_t* content, size_t count, BLGlyphMappingState* state) noexcept {
  bl_unused(face_impl, content, count);
  state->reset();
  return bl_make_error(BL_ERROR_FONT_NO_CHARACTER_MAPPING);
}

// bl::OpenType::CMapImpl - Format0
// ================================

static BLResult BL_CDECL map_text_to_glyphs_format0(const BLFontFaceImpl* face_impl, uint32_t* content, size_t count, BLGlyphMappingState* state) noexcept {
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  const CMapTable::Format0* sub_table = PtrOps::offset<CMapTable::Format0>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);
  const UInt8* glyph_id_array = sub_table->glyph_id_array;

  uint32_t* ptr = content;
  uint32_t* end = content + count;

  size_t undefined_count = 0;
  state->undefined_first = SIZE_MAX;

  while (ptr != end) {
    uint32_t uc = ptr[0];
    BLGlyphId glyph_id = uc < 256 ? uint32_t(glyph_id_array[uc].value()) : uint32_t(0);

    ptr[0] = glyph_id;
    if (BL_UNLIKELY(glyph_id == 0)) {
      if (!undefined_count)
        state->undefined_first = (size_t)(ptr - content);
      undefined_count++;
    }

    ptr++;
  }

  state->glyph_count = (size_t)(ptr - content);
  state->undefined_count = undefined_count;

  return BL_SUCCESS;
}

// bl::OpenType::CMapImpl - Format4
// ================================

static BL_INLINE const UInt16* findSegmentFormat4(
  uint32_t uc,
  const UInt16* last_char_array,
  size_t num_seg,
  size_t num_searchable_seg,
  uint32_t& uc_first, uint32_t& uc_last) noexcept {

  for (size_t i = num_searchable_seg; i != 0; i >>= 1) {
    const UInt16* end_count_ptr = PtrOps::offset(last_char_array, (i & ~size_t(1)));

    uc_last = end_count_ptr[0].value();
    if (uc_last < uc) {
      last_char_array = end_count_ptr + 1;
      i--;
      continue;
    }

    uc_first = MemOps::readU16uBE(PtrOps::offset(end_count_ptr, 2u + num_seg * 2u));
    if (uc_first <= uc)
      return end_count_ptr;
  }

  return nullptr;
}

static BLResult BL_CDECL map_text_to_glyphs_format4(const BLFontFaceImpl* face_impl, uint32_t* content, size_t count, BLGlyphMappingState* state) noexcept {
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  const CMapTable::Format4* sub_table = PtrOps::offset<CMapTable::Format4>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);

  uint32_t* ptr = content;
  uint32_t* end = content + count;

  size_t undefined_count = 0;
  state->undefined_first = SIZE_MAX;

  size_t num_seg = size_t(sub_table->numSegX2()) >> 1;
  const UInt16* last_char_array = sub_table->last_char_array();

  // We want to read 2 bytes from the glyph_ids_array, that's why we decrement by one here.
  const uint8_t* data_end = reinterpret_cast<const uint8_t*>(sub_table) + (ot_face_impl->cmap.cmap_table.size - ot_face_impl->cmap.encoding.offset - 1);

  size_t num_searchable_seg = ot_face_impl->cmap.encoding.entry_count;
  size_t id_delta_array_offset = 2u + num_seg * 4u;
  size_t id_offset_array_offset = 2u + num_seg * 6u;

  uint32_t uc;
  uint32_t uc_first;
  uint32_t uc_last;

  while (ptr != end) {
    uc = ptr[0];

NewMatch:
    const UInt16* match = findSegmentFormat4(uc, last_char_array, num_seg, num_searchable_seg, uc_first, uc_last);
    if (match) {
      // `match` points to `end_char[]`, based on CMAP-Format4:
      //   - match[0             ] == last_char_array [Segment]
      //   - match[2 + num_seg * 2] == first_char_array[Segment]
      //   - match[2 + num_seg * 4] == id_delta_array  [Segment]
      //   - match[2 + num_seg * 6] == id_offset_array [Segment]
      uint32_t offset = PtrOps::offset(match, id_offset_array_offset)->value();
      for (;;) {
        // If the `offset` is not zero then we have to get the GlyphId from the array.
        if (offset != 0) {
          size_t raw_remain = (size_t)(data_end - reinterpret_cast<const uint8_t*>(match));
          size_t raw_offset = id_offset_array_offset + (uc - uc_first) * 2u + offset;

          // This shouldn't happen if the sub-table was properly validated.
          if (BL_UNLIKELY(raw_offset >= raw_remain))
            goto UndefinedGlyph;

          uc = PtrOps::offset(match, raw_offset)->value();
        }

        uc += PtrOps::offset(match, id_delta_array_offset)->value();
        uc &= 0xFFFFu;

        if (BL_UNLIKELY(uc == 0))
          goto UndefinedGlyph;

        ptr[0] = uc;
        if (++ptr == end)
          goto Done;

        uc = ptr[0];
        if (uc < uc_first || uc > uc_last)
          goto NewMatch;
      }
    }
    else {
UndefinedGlyph:
      if (!undefined_count)
        state->undefined_first = (size_t)(ptr - content);

      *ptr++ = 0;
      undefined_count++;
    }
  }

Done:
  state->glyph_count = (size_t)(ptr - content);
  state->undefined_count = undefined_count;

  return BL_SUCCESS;
}

// bl::OpenType::CMapImpl - Format6
// ================================

static BLResult BL_CDECL map_text_to_glyphs_format6(const BLFontFaceImpl* face_impl, uint32_t* content, size_t count, BLGlyphMappingState* state) noexcept {
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  const CMapTable::Format6* sub_table = PtrOps::offset<CMapTable::Format6>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);

  uint32_t* ptr = content;
  uint32_t* end = content + count;

  size_t undefined_count = 0;
  state->undefined_first = SIZE_MAX;

  uint32_t uc_first = sub_table->first();
  uint32_t uc_count = sub_table->count();
  const UInt16* glyph_id_array = sub_table->glyph_id_array();

  while (ptr != end) {
    uint32_t uc = ptr[0] - uc_first;
    BLGlyphId glyph_id = uc < uc_count ? uint32_t(glyph_id_array[uc].value()) : uint32_t(0);

    *ptr++ = glyph_id;
    if (BL_UNLIKELY(glyph_id == 0)) {
      if (!undefined_count)
        state->undefined_first = (size_t)(ptr - content) - 1;
      undefined_count++;
    }
  }

  state->glyph_count = (size_t)(ptr - content);
  state->undefined_count = undefined_count;

  return BL_SUCCESS;
}

// bl::OpenType::CMapImpl - Format10
// =================================

static BLResult BL_CDECL map_text_to_glyphs_format10(const BLFontFaceImpl* face_impl, uint32_t* content, size_t count, BLGlyphMappingState* state) noexcept {
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  const CMapTable::Format10* sub_table = PtrOps::offset<CMapTable::Format10>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);

  uint32_t* ptr = content;
  uint32_t* end = content + count;

  size_t undefined_count = 0;
  state->undefined_first = SIZE_MAX;

  uint32_t uc_first = sub_table->first();
  uint32_t uc_count = sub_table->glyph_ids.count();

  const UInt16* glyph_id_array = sub_table->glyph_ids.array();
  while (ptr != end) {
    uint32_t uc = ptr[0] - uc_first;
    BLGlyphId glyph_id = uc < uc_count ? uint32_t(glyph_id_array[uc].value()) : uint32_t(0);

    *ptr++ = glyph_id;

    if (BL_UNLIKELY(glyph_id == 0)) {
      if (!undefined_count)
        state->undefined_first = (size_t)(ptr - content) - 1;
      undefined_count++;
    }
  }

  state->glyph_count = (size_t)(ptr - content);
  state->undefined_count = undefined_count;

  return BL_SUCCESS;
}

// bl::OpenType::CMapImpl - Format12 & Format13
// ============================================

static BL_INLINE bool findGroupFormat12_13(
  uint32_t uc,
  const CMapTable::Group* start,
  size_t count,
  uint32_t& uc_first, uint32_t& uc_last, uint32_t& first_glyph_id) noexcept {

  for (size_t i = count; i != 0; i >>= 1) {
    const CMapTable::Group* group = start + (i >> 1);

    uc_last = group->last();
    if (uc_last < uc) {
      start = group + 1;
      i--;
      continue;
    }

    uc_first = group->first();
    if (uc_first > uc)
      continue;

    first_glyph_id = group->glyph_id();
    return true;
  }

  return false;
}

template<uint32_t FormatId>
static BLResult map_text_to_glyphs_format12_13(const BLFontFaceImpl* face_impl, uint32_t* content, size_t count, BLGlyphMappingState* state) noexcept {
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  const CMapTable::Format12_13* sub_table = PtrOps::offset<CMapTable::Format12_13>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);

  uint32_t* ptr = content;
  uint32_t* end = content + count;

  size_t undefined_count = 0;
  state->undefined_first = SIZE_MAX;

  const CMapTable::Group* group_array = sub_table->groups.array();
  size_t group_count = ot_face_impl->cmap.encoding.entry_count;

  while (ptr != end) {
    uint32_t uc;
    uint32_t uc_first;
    uint32_t uc_last;
    uint32_t start_glyph_id;

    uc = ptr[0];

NewMatch:
    if (findGroupFormat12_13(uc, group_array, group_count, uc_first, uc_last, start_glyph_id)) {
      for (;;) {
        BLGlyphId glyph_id = (FormatId == 12) ? BLGlyphId((start_glyph_id + uc - uc_first) & 0xFFFFu)
                                             : BLGlyphId((start_glyph_id               ) & 0xFFFFu);
        if (!glyph_id)
          goto UndefinedGlyph;

        *ptr = glyph_id;
        if (++ptr == end)
          goto Done;

        uc = ptr[0];
        if (uc < uc_first || uc > uc_last)
          goto NewMatch;
      }
    }
    else {
UndefinedGlyph:
      if (!undefined_count)
        state->undefined_first = (size_t)(ptr - content);

      *ptr++ = 0;
      undefined_count++;
    }
  }

Done:
  state->glyph_count = (size_t)(ptr - content);
  state->undefined_count = undefined_count;

  return BL_SUCCESS;
}

// bl::OpenType::CMapImpl - Validate
// =================================

BLResult validate_sub_table(RawTable cmap_table, uint32_t sub_table_offset, uint32_t& format_out, CMapEncoding& encoding_out) noexcept {
  if (cmap_table.size < 4u || sub_table_offset > cmap_table.size - 4u)
    return bl_make_error(BL_ERROR_INVALID_DATA);

  uint32_t format = PtrOps::offset<const UInt16>(cmap_table.data, sub_table_offset)->value();
  switch (format) {
    // Format 0 - Byte Encoding Table
    // ------------------------------

    case 0: {
      Table<CMapTable::Format0> sub_table(cmap_table.sub_table_unchecked(sub_table_offset));
      if (!sub_table.fits())
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t length = sub_table->length();
      if (length < CMapTable::Format0::kBaseSize || length > sub_table.size)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      format_out = format;
      encoding_out.offset = sub_table_offset;
      encoding_out.entry_count = 256;
      return BL_SUCCESS;
    }

    // Format 2 - High-Byte Mapping Through Table
    // ------------------------------------------

    case 2: {
      return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);
    }

    // Format 4 - Segment Mapping to Delta Values
    // ------------------------------------------

    case 4: {
      Table<CMapTable::Format4> sub_table(cmap_table.sub_table_unchecked(sub_table_offset));
      if (!sub_table.fits())
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t length = sub_table->length();
      if (length < CMapTable::Format4::kBaseSize || length > sub_table.size)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t numSegX2 = sub_table->numSegX2();
      if (!numSegX2 || (numSegX2 & 1) != 0)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t num_seg = numSegX2 / 2;
      if (length < 16 + num_seg * 8)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      const UInt16* last_char_array = sub_table->last_char_array();
      const UInt16* first_char_array = sub_table->first_char_array(num_seg);
      const UInt16* id_offset_array = sub_table->id_offset_array(num_seg);

      uint32_t previous_end = 0;
      uint32_t num_seg_after_check = num_seg;

      for (uint32_t i = 0; i < num_seg; i++) {
        uint32_t last = last_char_array[i].value();
        uint32_t first = first_char_array[i].value();
        uint32_t id_offset = id_offset_array[i].value();

        if (first == 0xFFFF && last == 0xFFFF) {
          // We prefer number of segments without the ending mark(s). This
          // handles also the case of data with multiple ending marks.
          num_seg_after_check = bl_min(num_seg_after_check, i);
        }
        else {
          if (first < previous_end || first > last)
            return bl_make_error(BL_ERROR_INVALID_DATA);

          if (i != 0 && first == previous_end)
            return bl_make_error(BL_ERROR_INVALID_DATA);

          if (id_offset != 0) {
            // Offset to 16-bit data must be even.
            if (id_offset & 1)
              return bl_make_error(BL_ERROR_INVALID_DATA);

            // This just validates whether the table doesn't want us to jump
            // somewhere outside, it doesn't validate whether GlyphIds are not
            // outside the limit.
            uint32_t index_in_table = 16 + num_seg * 6u + id_offset + (last - first) * 2u;
            if (index_in_table >= length)
              return bl_make_error(BL_ERROR_INVALID_DATA);
          }
        }

        previous_end = last;
      }

      if (!num_seg_after_check)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      format_out = format;
      encoding_out.offset = sub_table_offset;
      encoding_out.entry_count = num_seg_after_check;
      return BL_SUCCESS;
    }

    // Format 6 - Trimmed Table Mapping
    // --------------------------------

    case 6: {
      Table<CMapTable::Format6> sub_table(cmap_table.sub_table_unchecked(sub_table_offset));
      if (!sub_table.fits())
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t length = sub_table->length();
      if (length < CMapTable::Format6::kBaseSize || length > sub_table.size)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t first = sub_table->first();
      uint32_t count = sub_table->count();

      if (!count || first + count > 0xFFFFu)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      if (length < sizeof(CMapTable::Format10) + count * 2u)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      format_out = format;
      encoding_out.offset = sub_table_offset;
      encoding_out.entry_count = count;
      return BL_SUCCESS;
    }

    // Format 8 - Mixed 16-Bit and 32-Bit Coverage
    // -------------------------------------------

    case 8: {
      return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);
    };

    // Format 10 - Trimmed Array
    // -------------------------

    case 10: {
      Table<CMapTable::Format10> sub_table(cmap_table.sub_table_unchecked(sub_table_offset));
      if (!sub_table.fits())
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t length = sub_table->length();
      if (length < CMapTable::Format10::kBaseSize || length > sub_table.size)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t first = sub_table->first();
      uint32_t count = sub_table->glyph_ids.count();

      if (first >= Unicode::kCharMax || !count || count > Unicode::kCharMax || first + count > Unicode::kCharMax)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      if (length < sizeof(CMapTable::Format10) + count * 2u)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      format_out = format;
      encoding_out.offset = sub_table_offset;
      encoding_out.entry_count = count;
      return BL_SUCCESS;
    }

    // Format 12 & 13 - Segmented Coverage / Many-To-One Range Mappings
    // ----------------------------------------------------------------

    case 12:
    case 13: {
      Table<CMapTable::Format12_13> sub_table(cmap_table.sub_table_unchecked(sub_table_offset));
      if (!sub_table.fits())
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t length = sub_table->length();
      if (length < CMapTable::Format12_13::kBaseSize || length > sub_table.size)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      uint32_t count = sub_table->groups.count();
      if (count > Unicode::kCharMax || length < sizeof(CMapTable::Format12_13) + count * sizeof(CMapTable::Group))
        return bl_make_error(BL_ERROR_INVALID_DATA);

      const CMapTable::Group* group_array = sub_table->groups.array();
      uint32_t first = group_array[0].first();
      uint32_t last = group_array[0].last();

      if (first > last || last > Unicode::kCharMax)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      for (uint32_t i = 1; i < count; i++) {
        first = group_array[i].first();
        if (first <= last)
          return bl_make_error(BL_ERROR_INVALID_DATA);

        last = group_array[i].last();
        if (first > last || last > Unicode::kCharMax)
          return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      format_out = format;
      encoding_out.offset = sub_table_offset;
      encoding_out.entry_count = count;
      return BL_SUCCESS;
    }

    // Format 14 - Unicode Variation Sequences
    // ---------------------------------------

    case 14: {
      Table<CMapTable::Format14> sub_table(cmap_table.sub_table_unchecked(sub_table_offset));
      if (!sub_table.fits())
        return bl_make_error(BL_ERROR_INVALID_DATA);

      // TODO: [OpenType] CMAP Format14 not implemented.
      return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);
    }

    // Invalid / Unknown
    // -----------------

    default: {
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }
  }
}

// bl::OpenType::CMapImpl - Populate Character Coverage
// ====================================================

BLResult populate_character_coverage(const OTFaceImpl* ot_face_impl, BLBitSet* out) noexcept {
  out->clear();
  BLBitSetBuilderT<1024> set(out);

  switch (ot_face_impl->cmap_format) {
    // Format 0 - Byte Encoding Table
    // ------------------------------

    case 0: {
      const CMapTable::Format0* sub_table = PtrOps::offset<CMapTable::Format0>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);
      const UInt8* glyph_id_array = sub_table->glyph_id_array;

      uint32_t bit_array[256 / IntOps::bit_size_of<uint32_t>()] {};
      for (uint32_t i = 0; i < 256; i++) {
        BLGlyphId glyph_id = glyph_id_array[i].value();
        if (glyph_id != 0)
          BitSetOps::bit_array_set_bit(bit_array, i);
      }

      return out->assign_words(0, bit_array, BL_ARRAY_SIZE(bit_array));
    }

    // Format 4 - Segment Mapping to Delta Values
    // ------------------------------------------

    case 4: {
      const CMapTable::Format4* sub_table = PtrOps::offset<CMapTable::Format4>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);

      uint32_t numSegX2 = sub_table->numSegX2();
      uint32_t num_seg = numSegX2 / 2;
      uint32_t entry_count = ot_face_impl->cmap.encoding.entry_count;

      // Not sure, we shouldn't proceed if these are zero.
      if (!num_seg || !entry_count)
        return BL_SUCCESS;

      const UInt16* last_char_array = sub_table->last_char_array();
      const UInt16* first_char_array = sub_table->first_char_array(num_seg);

      uint32_t range_start = 0;
      uint32_t range_end = 0;

      uint32_t i = 0;
      while (i < entry_count) {
        uint32_t segment_start = uint32_t(first_char_array[i].value());
        uint32_t segment_end = uint32_t(last_char_array[i].value()) + 1;

        i++;

        if (segment_start == range_end) {
          range_end = segment_end;
          continue;
        }

        if (range_start < range_end)
          BL_PROPAGATE(set.add_range(range_start, range_end));

        range_start = segment_start;
        range_end = segment_end;
      }

      if (range_start < range_end)
        BL_PROPAGATE(set.add_range(range_start, range_end));

      return set.commit();
    }

    // Format 6 - Trimmed Table Mapping
    // --------------------------------

    case 6: {
      const CMapTable::Format6* sub_table = PtrOps::offset<CMapTable::Format6>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);
      const UInt16* glyph_id_array = sub_table->glyph_id_array();

      uint32_t first_char = sub_table->first();
      uint32_t entry_count = ot_face_impl->cmap.encoding.entry_count;

      for (uint32_t i = 0; i < entry_count; i++) {
        BLGlyphId glyph_id = glyph_id_array[i].value();
        if (glyph_id != 0)
          BL_PROPAGATE(set.add_bit(first_char + i));
      }

      return set.commit();
    }

    // Format 10 - Trimmed Array
    // -------------------------

    case 10: {
      const CMapTable::Format10* sub_table = PtrOps::offset<CMapTable::Format10>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);
      const UInt16* glyph_id_array = sub_table->glyph_ids.array();

      uint32_t first_char = sub_table->first();
      uint32_t entry_count = ot_face_impl->cmap.encoding.entry_count;

      for (uint32_t i = 0; i < entry_count; i++) {
        BLGlyphId glyph_id = glyph_id_array[i].value();
        if (glyph_id != 0)
          BL_PROPAGATE(set.add_bit(first_char + i));
      }

      return set.commit();
    }

    // Format 12 / 13 - Segmented Coverage / Many-To-One Range Mappings
    // ----------------------------------------------------------------

    case 12:
    case 13: {
      const CMapTable::Format12_13* sub_table = PtrOps::offset<CMapTable::Format12_13>(ot_face_impl->cmap.cmap_table.data, ot_face_impl->cmap.encoding.offset);
      const CMapTable::Group* group_array = sub_table->groups.array();

      uint32_t entry_count = ot_face_impl->cmap.encoding.entry_count;
      if (!entry_count)
        return BL_SUCCESS;

      uint32_t range_start = 0;
      uint32_t range_end = 0;

      uint32_t i = 0;
      while (i < entry_count) {
        uint32_t segment_start = group_array[i].first();
        uint32_t segment_end = group_array[i].last();

        i++;

        if (segment_end != 0xFFFFFFFFu)
          segment_end++;

        if (segment_start == range_end) {
          range_end = segment_end;
          continue;
        }

        if (range_start < range_end)
          BL_PROPAGATE(set.add_range(range_start, range_end));

        range_start = segment_start;
        range_end = segment_end;
      }

      if (range_start < range_end)
        BL_PROPAGATE(set.add_range(range_start, range_end));

      return set.commit();
    }

    default:
      return bl_make_error(BL_ERROR_FONT_NO_CHARACTER_MAPPING);
  }
}


// bl::OpenType::CMapImpl - Init
// =============================

static bool is_supported_cmap_format(uint32_t format) noexcept {
  switch (format) {
    case  0:
    case  4:
    case  6:
    case 10:
    case 12:
    case 13:
      return true;
    default:
      return false;
  }
}

static BLResult init_cmap_funcs(OTFaceImpl* ot_face_impl) noexcept {
  switch (ot_face_impl->cmap_format) {
    case  0: ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_format0; break;
    case  4: ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_format4; break;
    case  6: ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_format6; break;
    case 10: ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_format10; break;
    case 12: ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_format12_13<12>; break;
    case 13: ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_format12_13<13>; break;
    default: ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_none; break;
  }
  return BL_SUCCESS;
}

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  Table<CMapTable> cmap(tables.cmap);

  if (!cmap)
    return BL_SUCCESS;

  if (!cmap.fits()) {
    ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_CMAP_DATA;
    return BL_SUCCESS;
  }

  uint32_t encoding_count = cmap->encodings.count();
  if (cmap.size < sizeof(CMapTable) + encoding_count * sizeof(CMapTable::Encoding)) {
    ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_CMAP_DATA;
    return BL_SUCCESS;
  }

  enum Score : uint32_t {
    kScoreNothing    = 0x00000u,
    kScoreMacRoman   = 0x00001u, // Not sure this would ever be used, but OpenType sanitizer passes this one.
    kScoreSymbolFont = 0x00002u,
    kScoreAnyUnicode = 0x10000u,
    kScoreWinUnicode = 0x20000u  // Prefer Windows-Unicode CMAP over Unicode.
  };

  uint32_t matched_score = kScoreNothing;
  uint32_t matched_format = 0xFFFFFFFFu;
  CMapEncoding matched_encoding {};

  for (uint32_t i = 0; i < encoding_count; i++) {
    const CMapTable::Encoding& encoding = cmap->encodings.array()[i];
    uint32_t offset = encoding.offset();

    if (offset >= cmap.size - 4)
      continue;

    uint32_t platform_id = encoding.platform_id();
    uint32_t encoding_id = encoding.encoding_id();
    uint32_t format = PtrOps::offset(cmap.data_as<UInt16>(), offset)->value();

    if (!is_supported_cmap_format(format))
      continue;

    uint32_t this_score = kScoreNothing;
    switch (platform_id) {
      case Platform::kPlatformUnicode:
        this_score = kScoreAnyUnicode + encoding_id;
        break;

      case Platform::kPlatformWindows:
        if (encoding_id == Platform::kWindowsEncodingSymbol) {
          this_score = kScoreSymbolFont;
          ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_SYMBOL_FONT;
        }
        else if (encoding_id == Platform::kWindowsEncodingUCS2 || encoding_id == Platform::kWindowsEncodingUCS4) {
          this_score = kScoreWinUnicode + encoding_id;
        }
        break;

      case Platform::kPlatformMac:
        if (encoding_id == Platform::kMacEncodingRoman && format == 0)
          this_score = kScoreMacRoman;
        break;
    }

    if (this_score > matched_score) {
      uint32_t this_format;
      CMapEncoding this_encoding {};

      BLResult result = validate_sub_table(cmap, offset, this_format, this_encoding);
      if (result != BL_SUCCESS) {
        if (result == BL_ERROR_NOT_IMPLEMENTED)
          ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_CMAP_FORMAT;
        else
          ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_CMAP_DATA;

        // If we had a match before then the match from before can still be used.
        continue;
      }

      matched_score = this_score;
      matched_format = this_format;
      matched_encoding = this_encoding;
    }
  }

  if (matched_score) {
    ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_CHAR_TO_GLYPH_MAPPING;
    ot_face_impl->cmap_format = uint8_t(matched_format);
    ot_face_impl->cmap.cmap_table = cmap;
    ot_face_impl->cmap.encoding = matched_encoding;
    return init_cmap_funcs(ot_face_impl);
  }
  else {
    // No cmap support, diagnostics was already set.
    ot_face_impl->funcs.map_text_to_glyphs = map_text_to_glyphs_none;
    return BL_SUCCESS;
  }
}

} // {CMapImpl}
} // {bl::OpenType}
