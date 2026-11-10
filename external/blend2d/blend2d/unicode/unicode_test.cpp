// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/unicode/unicode_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

// bl::Unicode - Tests
// ===================

namespace bl::Unicode::Tests {

UNIT(unicode, BL_TEST_GROUP_CORE_UTILITIES) {
  struct TestEntry {
    char dst[28];
    char src[28];
    uint8_t dst_size;
    uint8_t src_size;
    uint8_t dst_encoding;
    uint8_t src_encoding;
    BLResult result;
  };

  static const TestEntry test_entries[] = {
    #define ENTRY(DST, DST_ENC, SRC, SRC_ENC, ERROR_CODE) { \
      DST,                                                  \
      SRC,                                                  \
      uint8_t(sizeof(DST) - 1),                             \
      uint8_t(sizeof(SRC) - 1),                             \
      uint8_t(BL_TEXT_ENCODING_##DST_ENC),                  \
      uint8_t(BL_TEXT_ENCODING_##SRC_ENC),                  \
      ERROR_CODE                                            \
    }

    ENTRY("Test"                            , LATIN1, "Test"                            , LATIN1, BL_SUCCESS),
    ENTRY("Test"                            , UTF8  , "Test"                            , LATIN1, BL_SUCCESS),
    ENTRY("Test"                            , UTF8  , "Test"                            , UTF8  , BL_SUCCESS),
    #if BL_BYTE_ORDER == 1234
    ENTRY("Test"                            , UTF8  , "T\0e\0s\0t\0"                    , UTF16 , BL_SUCCESS),
    ENTRY("T\0e\0s\0t\0"                    , UTF16 , "Test"                            , UTF8  , BL_SUCCESS),
    #else
    ENTRY("Test"                            , UTF8  , "\0T\0e\0s\0t"                    , UTF16 , BL_SUCCESS),
    ENTRY("\0T\0e\0s\0t"                    , UTF16  , "Test"                           , UTF8  , BL_SUCCESS),
    #endif

    // Tests a Czech word (Rain in english) with diacritic marks, at most 2 BYTEs per character.
    #if BL_BYTE_ORDER == 1234
    ENTRY("\x44\xC3\xA9\xC5\xA1\xC5\xA5"    , UTF8  , "\x44\x00\xE9\x00\x61\x01\x65\x01", UTF16 , BL_SUCCESS),
    ENTRY("\x44\x00\xE9\x00\x61\x01\x65\x01", UTF16 , "\x44\xC3\xA9\xC5\xA1\xC5\xA5"    , UTF8  , BL_SUCCESS),
    #else
    ENTRY("\x44\xC3\xA9\xC5\xA1\xC5\xA5"    , UTF8  , "\x00\x44\x00\xE9\x01\x61\x01\x65", UTF16 , BL_SUCCESS),
    ENTRY("\x00\x44\x00\xE9\x01\x61\x01\x65", UTF16 , "\x44\xC3\xA9\xC5\xA1\xC5\xA5"    , UTF8  , BL_SUCCESS),
    #endif

    // Tests full-width digit zero (3 BYTEs per UTF-8 character).
    #if BL_BYTE_ORDER == 1234
    ENTRY("\xEF\xBC\x90"                    , UTF8  , "\x10\xFF"                        , UTF16 , BL_SUCCESS),
    ENTRY("\x10\xFF"                        , UTF16 , "\xEF\xBC\x90"                    , UTF8  , BL_SUCCESS),
    #else
    ENTRY("\xEF\xBC\x90"                    , UTF8  , "\xFF\x10"                        , UTF16 , BL_SUCCESS),
    ENTRY("\xFF\x10"                        , UTF16 , "\xEF\xBC\x90"                    , UTF8  , BL_SUCCESS),
    #endif

    // Tests `kCharMax` character (4 BYTEs per UTF-8 character, the highest possible unicode code-point).
    #if BL_BYTE_ORDER == 1234
    ENTRY("\xF4\x8F\xBF\xBF"                , UTF8  , "\xFF\xDB\xFF\xDF"                , UTF16 , BL_SUCCESS),
    ENTRY("\xFF\xDB\xFF\xDF"                , UTF16 , "\xF4\x8F\xBF\xBF"                , UTF8  , BL_SUCCESS),
    #else
    ENTRY("\xF4\x8F\xBF\xBF"                , UTF8  , "\xDB\xFF\xDF\xFF"                , UTF16 , BL_SUCCESS),
    ENTRY("\xDB\xFF\xDF\xFF"                , UTF16 , "\xF4\x8F\xBF\xBF"                , UTF8  , BL_SUCCESS),
    #endif

    #if BL_BYTE_ORDER == 1234
    ENTRY("Test"                            , UTF8  , "T\0\0\0e\0\0\0s\0\0\0t\0\0\0"    , UTF32 , BL_SUCCESS),
    ENTRY("T\0e\0s\0t\0"                    , UTF16 , "T\0\0\0e\0\0\0s\0\0\0t\0\0\0"    , UTF32 , BL_SUCCESS),
    ENTRY("T\0\0\0e\0\0\0s\0\0\0t\0\0\0"    , UTF32 , "T\0\0\0e\0\0\0s\0\0\0t\0\0\0"    , UTF32 , BL_SUCCESS),
    ENTRY("T\0\0\0e\0\0\0s\0\0\0t\0\0\0"    , UTF32 , "T\0e\0s\0t\0"                    , UTF16 , BL_SUCCESS),
    ENTRY("T\0\0\0e\0\0\0s\0\0\0t\0\0\0"    , UTF32 , "Test"                            , LATIN1, BL_SUCCESS),
    ENTRY("T\0\0\0e\0\0\0s\0\0\0t\0\0\0"    , UTF32 , "Test"                            , UTF8  , BL_SUCCESS),
    #else
    ENTRY("Test"                            , UTF8  , "\0\0\0T\0\0\0e\0\0\0s\0\0\0t"    , UTF32 , BL_SUCCESS),
    ENTRY("\0T\0e\0s\0t"                    , UTF16 , "\0\0\0T\0\0\0e\0\0\0s\0\0\0t"    , UTF32 , BL_SUCCESS),
    ENTRY("\0\0\0T\0\0\0e\0\0\0s\0\0\0t"    , UTF32 , "\0\0\0T\0\0\0e\0\0\0s\0\0\0t"    , UTF32 , BL_SUCCESS),
    ENTRY("\0\0\0T\0\0\0e\0\0\0s\0\0\0t"    , UTF32 , "\0T\0e\0s\0t"                    , UTF16 , BL_SUCCESS),
    ENTRY("\0\0\0T\0\0\0e\0\0\0s\0\0\0t"    , UTF32 , "Test"                            , LATIN1, BL_SUCCESS),
    ENTRY("\0\0\0T\0\0\0e\0\0\0s\0\0\0t"    , UTF32 , "Test"                            , UTF8  , BL_SUCCESS),
    #endif

    // Truncated characters.
    ENTRY(""                                , UTF8  , "\xC5"                            , UTF8  , BL_ERROR_DATA_TRUNCATED),
    ENTRY(""                                , UTF8  , "\xEF"                            , UTF8  , BL_ERROR_DATA_TRUNCATED),
    ENTRY(""                                , UTF8  , "\xEF\xBC"                        , UTF8  , BL_ERROR_DATA_TRUNCATED),
    ENTRY(""                                , UTF8  , "\xF4"                            , UTF8  , BL_ERROR_DATA_TRUNCATED),
    ENTRY(""                                , UTF8  , "\xF4\x8F"                        , UTF8  , BL_ERROR_DATA_TRUNCATED),
    ENTRY(""                                , UTF8  , "\xF4\x8F\xBF"                    , UTF8  , BL_ERROR_DATA_TRUNCATED),

    // Truncated character at the end (the converter must output the content, which was correct).
    ENTRY("a"                               , UTF8  , "a\xF4\x8F\xBF"                   , UTF8  , BL_ERROR_DATA_TRUNCATED),
    ENTRY("ab"                              , UTF8  , "ab\xF4\x8F\xBF"                  , UTF8  , BL_ERROR_DATA_TRUNCATED),
    ENTRY("TestString"                      , UTF8  , "TestString\xC5"                  , UTF8  , BL_ERROR_DATA_TRUNCATED),
    #if BL_BYTE_ORDER == 1234
    ENTRY("T\0e\0s\0t\0S\0t\0r\0i\0n\0g\0"  , UTF16 , "TestString\xC5"                  , UTF8  , BL_ERROR_DATA_TRUNCATED),
    #else
    ENTRY("\0T\0e\0s\0t\0S\0t\0r\0i\0n\0g"  , UTF16 , "TestString\xC5"                  , UTF8  , BL_ERROR_DATA_TRUNCATED),
    #endif

    // Invalid UTf-8 characters.
    ENTRY(""                                , UTF8  , "\x80"                            , UTF8  , BL_ERROR_INVALID_STRING),
    ENTRY(""                                , UTF8  , "\xC1"                            , UTF8  , BL_ERROR_INVALID_STRING),
    ENTRY(""                                , UTF8  , "\xF5\x8F\xBF\xBF"                , UTF8  , BL_ERROR_INVALID_STRING),
    ENTRY(""                                , UTF8  , "\x91\x8F\xBF\xBF"                , UTF8  , BL_ERROR_INVALID_STRING),
    ENTRY(""                                , UTF8  , "\xF6\x8F\xBF\xBF"                , UTF8  , BL_ERROR_INVALID_STRING),
    ENTRY(""                                , UTF8  , "\xF4\xFF\xBF\xBF"                , UTF8  , BL_ERROR_INVALID_STRING),

    // Overlong UTF-8 characters.
    ENTRY(""                                , UTF8  , "\xC0\xA0"                        , UTF8  , BL_ERROR_INVALID_STRING)

    #undef ENTRY
  };

  for (size_t i = 0; i < BL_ARRAY_SIZE(test_entries); i++) {
    const TestEntry& entry = test_entries[i];
    char output[32];

    Unicode::ConversionState state;
    BLResult result = Unicode::convert_unicode(output, 32, entry.dst_encoding, entry.src, entry.src_size, entry.src_encoding, state);

    bool failed = (result != entry.result) ||
                  (state.dst_index != entry.dst_size) ||
                  (memcmp(output, entry.dst, state.dst_index) != 0);

    if (failed) {
      size_t input_size = entry.src_size;
      size_t output_size = state.dst_index;
      size_t expected_size = entry.dst_size;

      printf("  Failed Entry #%u\n", unsigned(i));

      printf("    Input   :");
      for (size_t j = 0; j < input_size; j++)
        printf(" %02X", uint8_t(entry.src[j]));
      printf("%s\n", input_size ? "" : " (Nothing)");

      printf("    Output  :");
      for (size_t j = 0; j < output_size; j++)
        printf(" %02X", uint8_t(output[j]));
      printf("%s\n", output_size ? "" : " (Nothing)");

      printf("    Expected:");
      for (size_t j = 0; j < expected_size; j++)
        printf(" %02X", uint8_t(entry.dst[j]));
      printf("%s\n", expected_size ? "" : " (Nothing)");
      printf("    ErrorCode: Actual(%u) %s Expected(%u)\n", result, (result == entry.result) ? "==" : "!=", entry.result);
    }

    EXPECT_TRUE(!failed);
  }
}

UNIT(unicode_io, BL_TEST_GROUP_CORE_UTILITIES) {
  INFO("bl::Unicode::Utf8Reader");
  {
    const uint8_t data[] = {
      0xE2, 0x82, 0xAC,      // U+0020AC
      0xF0, 0x90, 0x8D, 0x88 // U+010348
    };

    Unicode::Utf8Reader it(data, BL_ARRAY_SIZE(data));
    uint32_t uc;

    EXPECT_TRUE(it.has_next());
    EXPECT_SUCCESS(it.next<Unicode::IOFlags::kCalcIndex>(uc));
    EXPECT_EQ(uc, 0x0020ACu);

    EXPECT_TRUE(it.has_next());
    EXPECT_SUCCESS(it.next<Unicode::IOFlags::kCalcIndex>(uc));
    EXPECT_EQ(uc, 0x010348u);

    EXPECT_FALSE(it.has_next());

    // Verify that sizes were calculated correctly.
    EXPECT_EQ(it.byte_index(data), 7u);
    EXPECT_EQ(it.utf8_index(data), 7u);
    EXPECT_EQ(it.utf16_index(data), 3u); // 3 code-points (1 BMP and 1 SMP).
    EXPECT_EQ(it.utf32_index(data), 2u); // 2 code-points.

    const uint8_t invalid_data[] = { 0xE2, 0x82 };
    it.reset(invalid_data, BL_ARRAY_SIZE(invalid_data));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(uc), BL_ERROR_DATA_TRUNCATED);

    // After error the iterator should not move.
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.byte_index(invalid_data), 0u);
    EXPECT_EQ(it.utf8_index(invalid_data), 0u);
    EXPECT_EQ(it.utf16_index(invalid_data), 0u);
    EXPECT_EQ(it.utf32_index(invalid_data), 0u);
  }

  INFO("bl::Unicode::Utf16Reader");
  {
    const uint16_t data[] = {
      0x20AC,                // U+0020AC
      0xD800, 0xDF48         // U+010348
    };

    Unicode::Utf16Reader it(data, BL_ARRAY_SIZE(data) * sizeof(uint16_t));
    uint32_t uc;

    EXPECT_TRUE(it.has_next());
    EXPECT_SUCCESS(it.next<Unicode::IOFlags::kCalcIndex>(uc));
    EXPECT_EQ(uc, 0x0020ACu);

    EXPECT_TRUE(it.has_next());
    EXPECT_SUCCESS(it.next<Unicode::IOFlags::kCalcIndex>(uc));
    EXPECT_EQ(uc, 0x010348u);

    EXPECT_FALSE(it.has_next());

    // Verify that sizes were calculated correctly.
    EXPECT_EQ(it.byte_index(data), 6u);
    EXPECT_EQ(it.utf8_index(data), 7u);
    EXPECT_EQ(it.utf16_index(data), 3u); // 3 code-points (1 BMP and 1 SMP).
    EXPECT_EQ(it.utf32_index(data), 2u); // 2 code-points.

    const uint16_t invalid_data[] = { 0xD800 };
    it.reset(invalid_data, BL_ARRAY_SIZE(invalid_data) * sizeof(uint16_t));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next<Unicode::IOFlags::kCalcIndex | Unicode::IOFlags::kStrict>(uc), BL_ERROR_DATA_TRUNCATED);

    // After an error the iterator should not move.
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.byte_index(invalid_data), 0u);
    EXPECT_EQ(it.utf8_index(invalid_data), 0u);
    EXPECT_EQ(it.utf16_index(invalid_data), 0u);
    EXPECT_EQ(it.utf32_index(invalid_data), 0u);

    // However, this should pass in non-strict mode.
    EXPECT_SUCCESS(it.next(uc));
    EXPECT_FALSE(it.has_next());
  }

  INFO("bl::Unicode::Utf32Reader");
  {
    const uint32_t data[] = {
      0x0020AC,
      0x010348
    };

    Unicode::Utf32Reader it(data, BL_ARRAY_SIZE(data) * sizeof(uint32_t));
    uint32_t uc;

    EXPECT_TRUE(it.has_next());
    EXPECT_SUCCESS(it.next<Unicode::IOFlags::kCalcIndex>(uc));
    EXPECT_EQ(uc, 0x0020ACu);

    EXPECT_TRUE(it.has_next());
    EXPECT_SUCCESS(it.next<Unicode::IOFlags::kCalcIndex>(uc));
    EXPECT_EQ(uc, 0x010348u);

    EXPECT_FALSE(it.has_next());

    // Verify that sizes were calculated correctly.
    EXPECT_EQ(it.byte_index(data), 8u);
    EXPECT_EQ(it.utf8_index(data), 7u);
    EXPECT_EQ(it.utf16_index(data), 3u); // 3 code-points (1 BMP and 1 SMP).
    EXPECT_EQ(it.utf32_index(data), 2u); // 2 code-points.

    const uint32_t invalid_data[] = { 0xD800 };
    it.reset(invalid_data, BL_ARRAY_SIZE(invalid_data) * sizeof(uint32_t));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next<Unicode::IOFlags::kCalcIndex | Unicode::IOFlags::kStrict>(uc), BL_ERROR_INVALID_STRING);

    // After an error the iterator should not move.
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.byte_index(invalid_data), 0u);
    EXPECT_EQ(it.utf8_index(invalid_data), 0u);
    EXPECT_EQ(it.utf16_index(invalid_data), 0u);
    EXPECT_EQ(it.utf32_index(invalid_data), 0u);

    // However, this should pass in non-strict mode.
    EXPECT_SUCCESS(it.next(uc));
    EXPECT_FALSE(it.has_next());
  }

  INFO("bl::Unicode::Utf8Writer");
  {
    char dst[7];
    Unicode::Utf8Writer writer(dst, BL_ARRAY_SIZE(dst));

    EXPECT_SUCCESS(writer.write(0x20ACu));
    EXPECT_EQ(uint8_t(dst[0]), 0xE2u);
    EXPECT_EQ(uint8_t(dst[1]), 0x82u);
    EXPECT_EQ(uint8_t(dst[2]), 0xACu);

    EXPECT_SUCCESS(writer.write(0x010348u));
    EXPECT_EQ(uint8_t(dst[3]), 0xF0u);
    EXPECT_EQ(uint8_t(dst[4]), 0x90u);
    EXPECT_EQ(uint8_t(dst[5]), 0x8Du);
    EXPECT_EQ(uint8_t(dst[6]), 0x88u);
    EXPECT_TRUE(writer.at_end());

    writer.reset(dst, 1);
    EXPECT_EQ(writer.write(0x20ACu), BL_ERROR_NO_SPACE_LEFT);
    EXPECT_EQ(writer.write(0x0080u), BL_ERROR_NO_SPACE_LEFT);
    EXPECT_EQ(writer.write(0x00C1u), BL_ERROR_NO_SPACE_LEFT);

    // We have only one byte left so this must pass...
    EXPECT_SUCCESS(writer.write('a'));
    EXPECT_TRUE(writer.at_end());

    writer.reset(dst, 2);
    EXPECT_EQ(writer.write(0x20ACu), BL_ERROR_NO_SPACE_LEFT);
    EXPECT_SUCCESS(writer.write(0x00C1u));
    EXPECT_EQ(uint8_t(dst[0]), 0xC3u);
    EXPECT_EQ(uint8_t(dst[1]), 0x81u);
    EXPECT_TRUE(writer.at_end());
    EXPECT_EQ(writer.write('a'), BL_ERROR_NO_SPACE_LEFT);
  }

  INFO("bl::Unicode::Utf16Writer");
  {
    uint16_t dst[3];
    Unicode::Utf16Writer<> writer(dst, BL_ARRAY_SIZE(dst));

    EXPECT_SUCCESS(writer.write(0x010348u));
    EXPECT_EQ(dst[0], 0xD800u);
    EXPECT_EQ(dst[1], 0xDF48u);

    EXPECT_EQ(writer.write(0x010348u), BL_ERROR_NO_SPACE_LEFT);
    EXPECT_SUCCESS(writer.write(0x20ACu));
    EXPECT_EQ(dst[2], 0x20ACu);
    EXPECT_TRUE(writer.at_end());
  }
}

} // {bl::Unicode::Tests}

#endif // BL_TEST
