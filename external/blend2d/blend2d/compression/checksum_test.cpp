// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/array.h>
#include <blend2d/compression/checksum_p.h>

namespace bl::Compression::Checksum::Tests {

// bl::Compression - CheckSum - CRC32 - Tests
// ==========================================

static constexpr uint32_t kCheckSumInputSize = 1024u * 256u;
static constexpr uint32_t kCheckSumLargeInputSize = 1024u * 1024u * 4u;

static void fill_array_for_checksum(BLArray<uint8_t>& arr, size_t n) noexcept {
  for (uint32_t i = 0; i < n; i++) {
    EXPECT_SUCCESS(arr.append(uint8_t((i * 17) & 0xFFu)));
  }
}

static void fill_array_with_same_value(BLArray<uint8_t>& arr, uint8_t b, size_t n) noexcept {
  for (uint32_t i = 0; i < n; i++) {
    EXPECT_SUCCESS(arr.append(b));
  }
}

UNIT(compression_checksum_adler32, BL_TEST_GROUP_COMPRESSION_CHECKSUMS) {
  const uint8_t* lowercase_letters = reinterpret_cast<const uint8_t*>("abcdefghijklmnopqrstuvwxyz");

  EXPECT_EQ(adler32(nullptr         ,  0), 0x00000001u);
  EXPECT_EQ(adler32(lowercase_letters,  1), 0x00620062u);
  EXPECT_EQ(adler32(lowercase_letters,  2), 0x012600C4u);
  EXPECT_EQ(adler32(lowercase_letters,  3), 0x024D0127u);
  EXPECT_EQ(adler32(lowercase_letters,  4), 0x03D8018Bu);
  EXPECT_EQ(adler32(lowercase_letters,  5), 0x05C801F0u);
  EXPECT_EQ(adler32(lowercase_letters,  6), 0x081E0256u);
  EXPECT_EQ(adler32(lowercase_letters,  7), 0x0ADB02BDu);
  EXPECT_EQ(adler32(lowercase_letters,  8), 0x0E000325u);
  EXPECT_EQ(adler32(lowercase_letters,  9), 0x118E038Eu);
  EXPECT_EQ(adler32(lowercase_letters, 10), 0x158603F8u);
  EXPECT_EQ(adler32(lowercase_letters, 11), 0x19E90463u);
  EXPECT_EQ(adler32(lowercase_letters, 12), 0x1EB804CFu);
  EXPECT_EQ(adler32(lowercase_letters, 13), 0x23F4053Cu);
  EXPECT_EQ(adler32(lowercase_letters, 14), 0x299E05AAu);
  EXPECT_EQ(adler32(lowercase_letters, 15), 0x2FB70619u);
  EXPECT_EQ(adler32(lowercase_letters, 16), 0x36400689u);
  EXPECT_EQ(adler32(lowercase_letters, 17), 0x3D3A06FAu);
  EXPECT_EQ(adler32(lowercase_letters, 18), 0x44A6076Cu);
  EXPECT_EQ(adler32(lowercase_letters, 19), 0x4C8507DFu);
  EXPECT_EQ(adler32(lowercase_letters, 20), 0x54D80853u);
  EXPECT_EQ(adler32(lowercase_letters, 21), 0x5DA008C8u);
  EXPECT_EQ(adler32(lowercase_letters, 22), 0x66DE093Eu);
  EXPECT_EQ(adler32(lowercase_letters, 23), 0x709309B5u);
  EXPECT_EQ(adler32(lowercase_letters, 24), 0x7AC00A2Du);
  EXPECT_EQ(adler32(lowercase_letters, 25), 0x85660AA6u);
  EXPECT_EQ(adler32(lowercase_letters, 26), 0x90860B20u);

  BLArray<uint8_t> input;
  fill_array_for_checksum(input, kCheckSumInputSize);

  for (uint32_t i = 1; i < kCheckSumInputSize; i += (i >> 10) + 1u) {
    uint32_t checksum = adler32(input.data(), i);
    uint32_t expected = adler32_update_ref(kAdler32Initial, input.data(), i);

    EXPECT_EQ(checksum, expected).message(
      "ADLER32 checksum of %u random bytes doesn't match (checksum=0x%08X expected=0x%08X", i, checksum, expected);
  }

  input.clear();
  fill_array_with_same_value(input, 0xFFu, kCheckSumInputSize);

  for (uint32_t i = 1; i < kCheckSumInputSize; i += (i >> 10) + 1u) {
    uint32_t checksum = adler32(input.data(), i);
    uint32_t expected = adler32_update_ref(kAdler32Initial, input.data(), i);

    EXPECT_EQ(checksum, expected).message(
      "ADLER32 checksum of %u '0xFF' bytes doesn't match (checksum=0x%08X expected=0x%08X", i, checksum, expected);
  }

  input.clear();
  fill_array_with_same_value(input, 0xFFu, kCheckSumLargeInputSize);

  {
    uint32_t checksum = adler32(input.data(), kCheckSumLargeInputSize);
    uint32_t expected = adler32_update_ref(kAdler32Initial, input.data(), kCheckSumLargeInputSize);

    EXPECT_EQ(checksum, expected).message(
      "ADLER32 checksum of %u '0xFF' bytes doesn't match (checksum=0x%08X expected=0x%08X", kCheckSumLargeInputSize, checksum, expected);
  }
}

UNIT(compression_checksum_crc32, BL_TEST_GROUP_COMPRESSION_CHECKSUMS) {
  const uint8_t* lowercase_letters = reinterpret_cast<const uint8_t*>("abcdefghijklmnopqrstuvwxyz");

  EXPECT_EQ(crc32(nullptr         ,  0), 0x00000000u);
  EXPECT_EQ(crc32(lowercase_letters,  1), 0xE8B7BE43u);
  EXPECT_EQ(crc32(lowercase_letters,  2), 0x9E83486Du);
  EXPECT_EQ(crc32(lowercase_letters,  3), 0x352441C2u);
  EXPECT_EQ(crc32(lowercase_letters,  4), 0xED82CD11u);
  EXPECT_EQ(crc32(lowercase_letters,  5), 0x8587D865u);
  EXPECT_EQ(crc32(lowercase_letters,  6), 0x4B8E39EFu);
  EXPECT_EQ(crc32(lowercase_letters,  7), 0x312A6AA6u);
  EXPECT_EQ(crc32(lowercase_letters,  8), 0xAEEF2A50u);
  EXPECT_EQ(crc32(lowercase_letters,  9), 0x8DA988AFu);
  EXPECT_EQ(crc32(lowercase_letters, 10), 0x3981703Au);
  EXPECT_EQ(crc32(lowercase_letters, 11), 0xCE570F9Fu);
  EXPECT_EQ(crc32(lowercase_letters, 12), 0xF6781B24u);
  EXPECT_EQ(crc32(lowercase_letters, 13), 0xDDF46EA2u);
  EXPECT_EQ(crc32(lowercase_letters, 14), 0x400D9578u);
  EXPECT_EQ(crc32(lowercase_letters, 15), 0x519167DFu);
  EXPECT_EQ(crc32(lowercase_letters, 16), 0x943AC093u);
  EXPECT_EQ(crc32(lowercase_letters, 17), 0x9C925619u);
  EXPECT_EQ(crc32(lowercase_letters, 18), 0x08FEC50Bu);
  EXPECT_EQ(crc32(lowercase_letters, 19), 0x8CD4E846u);
  EXPECT_EQ(crc32(lowercase_letters, 20), 0x1A596AE5u);
  EXPECT_EQ(crc32(lowercase_letters, 21), 0x221725A3u);
  EXPECT_EQ(crc32(lowercase_letters, 22), 0x2499DEF3u);
  EXPECT_EQ(crc32(lowercase_letters, 23), 0x38F3316Au);
  EXPECT_EQ(crc32(lowercase_letters, 24), 0x21836DF4u);
  EXPECT_EQ(crc32(lowercase_letters, 25), 0x412A937Du);
  EXPECT_EQ(crc32(lowercase_letters, 26), 0x4C2750BDu);

  BLArray<uint8_t> input;
  fill_array_for_checksum(input, kCheckSumInputSize);

  for (uint32_t i = 1; i < kCheckSumInputSize; i += (i >> 10) + 1u) {
    uint32_t checksum = crc32(input.data(), i);
    uint32_t expected = crc32_finalize(crc32_update_ref(kCrc32Initial, input.data(), i));

    EXPECT_EQ(checksum, expected).message(
      "CRC32 checksum of %u random bytes doesn't match (checksum=0x%08X expected=0x%08X", i, checksum, expected);
  }

  input.clear();
  fill_array_for_checksum(input, kCheckSumLargeInputSize);

  {
    uint32_t checksum = crc32(input.data(), kCheckSumLargeInputSize);
    uint32_t expected = crc32_finalize(crc32_update_ref(kCrc32Initial, input.data(), kCheckSumLargeInputSize));

    EXPECT_EQ(checksum, expected).message(
      "CRC32 checksum of %u random bytes doesn't match (checksum=0x%08X expected=0x%08X", kCheckSumLargeInputSize, checksum, expected);
  }
}

} // {bl::Compression::Checksum::Tests}

#endif // BL_TEST
