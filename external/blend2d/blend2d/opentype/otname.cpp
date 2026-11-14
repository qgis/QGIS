// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/trace_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otname_p.h>
#include <blend2d/opentype/otplatform_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/unicode/unicode_p.h>

namespace bl::OpenType {
namespace NameImpl {

// bl::OpenType::NameImpl - Tracing
// ================================

#if defined(BL_TRACE_OT_ALL) || defined(BL_TRACE_OT_NAME)
  #define Trace BLDebugTrace
#else
  #define Trace BLDummyTrace
#endif

// bl::OpenType::NameImpl - Utilities
// ==================================

static BLTextEncoding encoding_from_platform_id(uint32_t platform_id) noexcept {
  // Both Unicode and Windows platform use 'UTF16-BE' encoding.
  bool is_unicode = platform_id == Platform::kPlatformUnicode ||
                   platform_id == Platform::kPlatformWindows ;
  return is_unicode ? BL_TEXT_ENCODING_UTF16 : BL_TEXT_ENCODING_LATIN1;
}

static BLResult convert_name_string_to_utf8(BLString& dst, BLArrayView<uint8_t> src, BLTextEncoding encoding) noexcept {
  // Name table should only have 16-bit lengths, so verify it's correct.
  BL_ASSERT(src.size < 65536);

  // We may overapproximate a bit, but it doesn't really matter as the length is limited anyway.
  size_t dst_size = src.size * 2u;
  char* dst_start;
  BL_PROPAGATE(dst.modify_op(BL_MODIFY_OP_ASSIGN_GROW, dst_size, &dst_start));

  Unicode::Utf8Writer dst_writer(dst_start, dst_size);
  size_t null_terminator_count = 0;

  if (encoding == BL_TEXT_ENCODING_LATIN1) {
    const uint8_t* src_ptr = src.data;
    const uint8_t* src_end = src.data + src.size;

    while (src_ptr != src_end) {
      uint32_t uc = *src_ptr++;

      null_terminator_count += (uc == 0);
      if (uc <= 0x7F)
        dst_writer.write_byte_unsafe(uc);
      else
        dst_writer.write2_bytes_unsafe(uc);
    }
  }
  else {
    // UTF16-BE.
    Unicode::Utf16Reader src_reader(src.data, src.size & ~size_t(0x1));
    while (src_reader.has_next()) {
      uint32_t uc;
      BL_PROPAGATE(src_reader.next<Unicode::IOFlags::kUnaligned | Unicode::IOFlags::kByteOrderBE | Unicode::IOFlags::kStrict>(uc));

      null_terminator_count += size_t(uc == 0);
      BL_PROPAGATE(dst_writer.write_unsafe(uc));
    }
  }

  // Remove null terminators at the end of the string. This can happen as some fonts use them as padding. Also, some
  // broken fonts encode data as UTF32-BE, which would produce a lot of null terminators when decoded as UTF16-BE.
  char* dst_ptr = dst_writer._ptr;
  while (dst_ptr != dst_start && dst_ptr[-1] == '\0') {
    dst_ptr--;
    null_terminator_count--;
  }

  dst.truncate((size_t)(dst_ptr - dst_start));
  if (null_terminator_count != 0)
    return bl_make_error(BL_ERROR_INVALID_DATA);

  return BL_SUCCESS;
}

static void normalize_family_and_subfamily(OTFaceImpl* ot_face_impl, Trace trace) noexcept {
  BLString& family_name = ot_face_impl->family_name.dcast();
  BLString& subfamily_name = ot_face_impl->subfamily_name.dcast();

  // Some fonts duplicate font subfamily-name in family-name, we try to match such cases and truncate the sub-family
  // in such case.
  if (family_name.size() >= subfamily_name.size() && !subfamily_name.is_empty()) {
    // Base size is a size of family name after the whole subfamily was removed from it (if matched). It's basically
    // the minimum length we would end up when subfamily-name matches the end of family-name fully.
    size_t base_size = family_name.size() - subfamily_name.size();
    if (memcmp(family_name.data() + base_size, subfamily_name.data(), subfamily_name.size()) == 0) {
      trace.warn("Subfamily '%s' is redundant, removing...\n", subfamily_name.data());
      subfamily_name.reset();
      ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_FIXED_NAME_DATA;
    }
  }
}

// bl::OpenType::NameImpl - Init
// =============================

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  typedef NameTable::NameRecord NameRecord;

  Table<NameTable> name(tables.name);
  if (!name)
    return bl_make_error(BL_ERROR_FONT_MISSING_IMPORTANT_TABLE);

  if (!name.fits())
    return bl_make_error(BL_ERROR_INVALID_DATA);

  Trace trace;
  trace.info("bl::OpenType::OTFaceImpl::InitName [Size=%u]\n", name.size);
  trace.indent();

  if (BL_UNLIKELY(name.size < NameTable::kBaseSize)) {
    trace.warn("Table is truncated\n");
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  uint32_t format = name->format();
  uint32_t record_count = name->record_count();

  trace.info("Format: %u\n", format);
  trace.info("RecordCount: %u\n", record_count);

  uint32_t string_region_offset = name->string_offset();
  if (BL_UNLIKELY(string_region_offset >= name.size))
    return bl_make_error(BL_ERROR_INVALID_DATA);

  // Only formats 0 and 1 are defined.
  if (BL_UNLIKELY(format > 1))
    return bl_make_error(BL_ERROR_INVALID_DATA);

  // There must be some names otherwise this table is invalid. Also make sure that the number of records doesn't
  // overflow the size of 'name' itself.
  if (BL_UNLIKELY(!record_count || !name.fits(6u + record_count * uint32_t(sizeof(NameRecord)))))
    return bl_make_error(BL_ERROR_INVALID_DATA);

  // Mask of name IDs which we are interested in.
  //
  // NOTE: We are not interested in WWS family and subfamily names as those may include subfamilies, which we expect
  // to be separate. We would only use WWS names if there is no other choice.
  constexpr uint32_t kImportantNameIdMask = IntOps::lsb_bits_at<uint32_t>(
    BL_FONT_STRING_ID_FAMILY_NAME,
    BL_FONT_STRING_ID_SUBFAMILY_NAME,
    BL_FONT_STRING_ID_FULL_NAME,
    BL_FONT_STRING_ID_POST_SCRIPT_NAME,
    BL_FONT_STRING_ID_TYPOGRAPHIC_FAMILY_NAME,
    BL_FONT_STRING_ID_TYPOGRAPHIC_SUBFAMILY_NAME,
    BL_FONT_STRING_ID_WWS_FAMILY_NAME,
    BL_FONT_STRING_ID_WWS_SUBFAMILY_NAME);

  // Scoring is used to select the best records as the same NameId can be repeated multiple times having a different
  // `platform_id`, `specific_id`, and `language_id`.
  uint16_t name_id_score[BL_FONT_STRING_ID_COMMON_MAX_VALUE + 1] = { 0 }; // Score of each interesting NameId.
  uint32_t name_id_index[BL_FONT_STRING_ID_COMMON_MAX_VALUE + 1];         // Record index of matched NameId.
  uint32_t name_id_mask = 0;                                           // Mask of all matched NameIds.

  BLString tmp_string;

  const NameRecord* name_records = name->name_records();
  uint32_t string_region_size = uint32_t(name.size - string_region_offset);

  for (uint32_t i = 0; i < record_count; i++) {
    const NameRecord& name_record = name_records[i];

    // Don't bother with a NameId we are not interested in.
    uint32_t name_id = name_record.name_id();
    if (name_id > BL_FONT_STRING_ID_COMMON_MAX_VALUE || !(kImportantNameIdMask & (1 << name_id)))
      continue;

    uint32_t string_offset = name_record.offset();
    uint32_t string_length = name_record.length();

    // Offset could be anything if length is zero.
    if (string_length == 0)
      string_offset = 0;

    // Fonts are full of wrong data, if the offset is outside of the string data we simply skip the record.
    if (BL_UNLIKELY(string_offset >= string_region_size || string_region_size - string_offset < string_length)) {
      trace.warn("Invalid Region {NameId=%u Offset=%u Length=%u}", string_offset, string_length);
      continue;
    }

    uint32_t platform_id = name_record.platform_id();
    uint32_t specific_id = name_record.specific_id();
    uint32_t language_id = name_record.language_id();

    uint32_t score = 0;
    switch (platform_id) {
      case Platform::kPlatformUnicode: {
        score = 3;
        break;
      }

      case Platform::kPlatformMac: {
        // Sucks, but better than nothing...
        if (specific_id == Platform::kMacEncodingRoman)
          score = 2;
        else
          continue;

        if (language_id == Platform::kMacLanguageEnglish) {
          score |= (0x01u << 8);
        }

        break;
      }

      case Platform::kPlatformWindows: {
        if (specific_id == Platform::kWindowsEncodingSymbol)
          score = 1;
        else if (specific_id == Platform::kWindowsEncodingUCS2)
          score = 4;
        else
          continue;

        // We use the term "locale" instead of "language" when it comes to Windows platform. Locale specifies both
        // primary language and sub-language, which is usually related to a geographic location.
        uint32_t locale_id = language_id;
        uint32_t primary_lang_id = locale_id & 0xFFu;

        // Check primary language.
        if (primary_lang_id == Platform::kWindowsLanguageEnglish) {
          if (locale_id == Platform::kWindowsLocaleEnglish_US)
            score |= (0x04u << 8);
          else if (locale_id == Platform::kWindowsLocaleEnglish_UK)
            score |= (0x03u << 8);
          else
            score |= (0x02u << 8);
        }
        break;
      }
    }

    if (score) {
      // Make sure this string is decodable before using this entry.
      BLTextEncoding encoding = encoding_from_platform_id(platform_id);

      const uint8_t* src = name.data + string_region_offset + string_offset;
      BLResult result = convert_name_string_to_utf8(tmp_string, BLArrayView<uint8_t> { src, string_length }, encoding);

      if (result != BL_SUCCESS) {
        // Data contains either null terminator(s) or the data is corrupted. There are some fonts that store some names
        // in UTF32-BE encoding, we refuse these names as it's not anywhere in the specification and thus broken.
        if (trace.enabled()) {
          trace.warn("Failed to decode '%s' <- [", tmp_string.data());
          for (size_t j = 0; j < string_length; j++)
            trace.out(" %02X", src[j]);
          trace.out(" ]\n");
        }

        score = 0;
        ot_face_impl->face_info.diag_flags |= BL_FONT_FACE_DIAG_WRONG_NAME_DATA;
      }
      else {
        // If this is a subfamily (NameId=2) on a MAC platform and it's empty we prefer it, because many fonts have
        // this field correctly empty on MAC platform and filled incorrectly on Windows platform.
        if (platform_id == Platform::kPlatformMac && name_id == BL_FONT_STRING_ID_SUBFAMILY_NAME && tmp_string.is_empty())
          score = 0xFFFFu;
      }

      trace.info("[%s] \"%s\" [Size=%u] {NameId=%u PlatformId=%u SpecificId=%u LanguageId=%u Score=%u}\n",
                 score > name_id_score[name_id] ? "SELECT" : "DROP",
                 result != BL_SUCCESS ? "Failed" : tmp_string.data(),
                 string_length,
                 name_id,
                 platform_id,
                 specific_id,
                 language_id,
                 score);

      // Update if we have found a better candidate or this is was the first one.
      if (score > name_id_score[name_id]) {
        name_id_score[name_id] = uint16_t(score);
        name_id_index[name_id] = i;
        name_id_mask |= IntOps::lsb_bit_at<uint32_t>(name_id);
      }
    }
  }

  // Prefer TypographicFamilyName over FamilyName and WWSFamilyName.
  if (IntOps::bit_test(name_id_mask, BL_FONT_STRING_ID_TYPOGRAPHIC_FAMILY_NAME)) {
    name_id_mask &= ~IntOps::lsb_bits_at<uint32_t>(BL_FONT_STRING_ID_FAMILY_NAME, BL_FONT_STRING_ID_WWS_FAMILY_NAME);
  }

  // Prefer TypographicSubfamilyName over SubfamilyName and WWSSubfamilyName.
  if (IntOps::bit_test(name_id_mask, BL_FONT_STRING_ID_TYPOGRAPHIC_SUBFAMILY_NAME)) {
    name_id_mask &= ~IntOps::lsb_bits_at<uint32_t>(BL_FONT_STRING_ID_SUBFAMILY_NAME, BL_FONT_STRING_ID_WWS_SUBFAMILY_NAME);
  }

  if (IntOps::bit_match(name_id_mask, IntOps::lsb_bits_at<uint32_t>(BL_FONT_STRING_ID_TYPOGRAPHIC_FAMILY_NAME, BL_FONT_STRING_ID_TYPOGRAPHIC_SUBFAMILY_NAME))) {
    trace.info("Has Typographic FamilyName and SubfamilyName\n");
    ot_face_impl->face_info.face_flags |= BL_FONT_FACE_FLAG_TYPOGRAPHIC_NAMES;
  }

  BitWordIterator<uint32_t> bit_word_iterator(name_id_mask);
  while (bit_word_iterator.has_next()) {
    uint32_t name_id = bit_word_iterator.next();
    const NameRecord& name_record = name_records[name_id_index[name_id]];

    uint32_t platform_id = name_record.platform_id();
    uint32_t string_offset = name_record.offset();
    uint32_t string_length = name_record.length();

    // Offset could be anything if length is zero.
    if (string_length == 0)
      string_offset = 0;

    // This should have already been filtered out, but one is never sure...
    if (BL_UNLIKELY(string_offset >= string_region_size || string_region_size - string_offset < string_length))
      return bl_make_error(BL_ERROR_INVALID_DATA);

    BLStringCore* dst = nullptr;
    switch (name_id) {
      case BL_FONT_STRING_ID_FULL_NAME:
        dst = &ot_face_impl->full_name;
        break;

      case BL_FONT_STRING_ID_FAMILY_NAME:
      case BL_FONT_STRING_ID_WWS_FAMILY_NAME:
      case BL_FONT_STRING_ID_TYPOGRAPHIC_FAMILY_NAME:
        dst = &ot_face_impl->family_name;
        break;

      case BL_FONT_STRING_ID_SUBFAMILY_NAME:
      case BL_FONT_STRING_ID_WWS_SUBFAMILY_NAME:
      case BL_FONT_STRING_ID_TYPOGRAPHIC_SUBFAMILY_NAME:
        dst = &ot_face_impl->subfamily_name;
        break;

      case BL_FONT_STRING_ID_POST_SCRIPT_NAME:
        dst = &ot_face_impl->post_script_name;
        break;
    }

    if (dst) {
      const uint8_t* src = name.data + string_region_offset + string_offset;
      BLTextEncoding encoding = encoding_from_platform_id(platform_id);
      BL_PROPAGATE(convert_name_string_to_utf8(dst->dcast(), BLArrayView<uint8_t> { src, string_length }, encoding));
    }
  }

  normalize_family_and_subfamily(ot_face_impl, trace);
  trace.info("Family=%s [SubFamily=%s] {PostScriptName=%s}\n",
             ot_face_impl->family_name.dcast().data(),
             ot_face_impl->subfamily_name.dcast().data(),
             ot_face_impl->post_script_name.dcast().data());
  return BL_SUCCESS;
}

} // {NameImpl}
} // {bl::OpenType}
