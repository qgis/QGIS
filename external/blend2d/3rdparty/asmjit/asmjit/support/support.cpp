// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// Support - Tests
// ===============

#if defined(ASMJIT_TEST)
template<typename T>
static void test_arrays(const T* a, const T* b, size_t size) noexcept {
  for (size_t i = 0; i < size; i++)
    EXPECT_EQ(a[i], b[i])
      .message("Mismatch at %u", unsigned(i));
}

static void test_alignment() noexcept {
  INFO("Support::is_aligned()");
  EXPECT_FALSE(Support::is_aligned<size_t>(0xFFFF, 4u));
  EXPECT_TRUE(Support::is_aligned<size_t>(0xFFF4, 4u));
  EXPECT_TRUE(Support::is_aligned<size_t>(0xFFF8, 8u));
  EXPECT_TRUE(Support::is_aligned<size_t>(0xFFF0, 16u));

  INFO("Support::align_up()");
  EXPECT_EQ(Support::align_up<size_t>(0xFFFF, 4), 0x10000u);
  EXPECT_EQ(Support::align_up<size_t>(0xFFF4, 4), 0x0FFF4u);
  EXPECT_EQ(Support::align_up<size_t>(0xFFF8, 8), 0x0FFF8u);
  EXPECT_EQ(Support::align_up<size_t>(0xFFF0, 16), 0x0FFF0u);
  EXPECT_EQ(Support::align_up<size_t>(0xFFF0, 32), 0x10000u);

  INFO("Support::align_up_diff()");
  EXPECT_EQ(Support::align_up_diff<size_t>(0xFFFF, 4), 1u);
  EXPECT_EQ(Support::align_up_diff<size_t>(0xFFF4, 4), 0u);
  EXPECT_EQ(Support::align_up_diff<size_t>(0xFFF8, 8), 0u);
  EXPECT_EQ(Support::align_up_diff<size_t>(0xFFF0, 16), 0u);
  EXPECT_EQ(Support::align_up_diff<size_t>(0xFFF0, 32), 16u);

  INFO("Support::align_up_power_of_2()");
  EXPECT_EQ(Support::align_up_power_of_2<size_t>(0x0000), 0x00000u);
  EXPECT_EQ(Support::align_up_power_of_2<size_t>(0xFFFF), 0x10000u);
  EXPECT_EQ(Support::align_up_power_of_2<size_t>(0xF123), 0x10000u);
  EXPECT_EQ(Support::align_up_power_of_2<size_t>(0x0F00), 0x01000u);
  EXPECT_EQ(Support::align_up_power_of_2<size_t>(0x0100), 0x00100u);
  EXPECT_EQ(Support::align_up_power_of_2<size_t>(0x1001), 0x02000u);
}

static void test_bit_utils() noexcept {
  INFO("Support::clz()");
  for (uint32_t i = 0; i <  8; i++) EXPECT_EQ(Support::clz_t(uint8_t (1u << i)),  7 - i);
  for (uint32_t i = 0; i < 16; i++) EXPECT_EQ(Support::clz_t(uint16_t(1u << i)), 15 - i);
  for (uint32_t i = 0; i < 32; i++) EXPECT_EQ(Support::clz_t(uint32_t(1u << i)), 31 - i);
  for (uint32_t i = 0; i < 64; i++) EXPECT_EQ(Support::clz_t(uint64_t(1u) << i), 63 - i);

  for (uint32_t i = 0; i <  8; i++) EXPECT_EQ(Support::clz(uint8_t (1u << i)),  7 - i);
  for (uint32_t i = 0; i < 16; i++) EXPECT_EQ(Support::clz(uint16_t(1u << i)), 15 - i);
  for (uint32_t i = 0; i < 32; i++) EXPECT_EQ(Support::clz(uint32_t(1u << i)), 31 - i);
  for (uint32_t i = 0; i < 64; i++) EXPECT_EQ(Support::clz(uint64_t(1u) << i), 63 - i);

  INFO("Support::ctz()");
  for (uint32_t i = 0; i <  8; i++) EXPECT_EQ(Support::ctz_t(uint8_t (1u << i)), i);
  for (uint32_t i = 0; i < 16; i++) EXPECT_EQ(Support::ctz_t(uint16_t(1u << i)), i);
  for (uint32_t i = 0; i < 32; i++) EXPECT_EQ(Support::ctz_t(uint32_t(1u << i)), i);
  for (uint32_t i = 0; i < 64; i++) EXPECT_EQ(Support::ctz_t(uint64_t(1u) << i), i);

  for (uint32_t i = 0; i < 32; i++) EXPECT_EQ(Support::ctz(uint32_t(1) << i), i);
  for (uint32_t i = 0; i < 64; i++) EXPECT_EQ(Support::ctz(uint64_t(1) << i), i);

  INFO("Support::popcnt()");
  for (uint32_t i = 0; i < 32; i++) EXPECT_EQ(Support::popcnt((uint32_t(1) << i)), 1u);
  for (uint32_t i = 0; i < 64; i++) EXPECT_EQ(Support::popcnt((uint64_t(1) << i)), 1u);
  EXPECT_EQ(Support::popcnt(0x000000F0), 4u);
  EXPECT_EQ(Support::popcnt(0x10101010), 4u);
  EXPECT_EQ(Support::popcnt(0xFF000000), 8u);
  EXPECT_EQ(Support::popcnt(0xFFFFFFF7), 31u);
  EXPECT_EQ(Support::popcnt(0x7FFFFFFF), 31u);

  INFO("Support::shl() / shr()");
  EXPECT_EQ(Support::shl(int32_t(0x00001111), 16), int32_t(0x11110000u));
  EXPECT_EQ(Support::shl(uint32_t(0x00001111), 16), uint32_t(0x11110000u));
  EXPECT_EQ(Support::shr(int32_t(0x11110000u), 16), int32_t(0x00001111u));
  EXPECT_EQ(Support::shr(uint32_t(0x11110000u), 16), uint32_t(0x00001111u));
  EXPECT_EQ(Support::sar(int32_t(0xFFFF0000u), 16), int32_t(0xFFFFFFFFu));
  EXPECT_EQ(Support::sar(uint32_t(0xFFFF0000u), 16), uint32_t(0xFFFFFFFFu));

  INFO("Support::blsi()");
  for (uint32_t i = 0; i < 32; i++) EXPECT_EQ(Support::blsi(uint32_t(1) << i), uint32_t(1) << i);
  for (uint32_t i = 0; i < 31; i++) EXPECT_EQ(Support::blsi(uint32_t(3) << i), uint32_t(1) << i);
  for (uint32_t i = 0; i < 64; i++) EXPECT_EQ(Support::blsi(uint64_t(1) << i), uint64_t(1) << i);
  for (uint32_t i = 0; i < 63; i++) EXPECT_EQ(Support::blsi(uint64_t(3) << i), uint64_t(1) << i);

  INFO("Support::bit_mask()");
  EXPECT_EQ(Support::bit_mask<uint32_t>(0, 1, 7), 0x83u);
  for (uint32_t i = 0; i < 32; i++) {
    EXPECT_EQ(Support::bit_mask<uint32_t>(i), (1u << i));
  }

  INFO("Support::bit_test()");
  for (uint32_t i = 0; i < 32; i++) {
    EXPECT_TRUE(Support::bit_test((1 << i), i))
      .message("Support::bit_test(0x%X, %u) should return true", (1u << i), i);
  }

  INFO("Support::lsb_mask<uint32_t>()");
  for (uint32_t i = 0; i < 32; i++) {
    uint32_t expected_bits = 0;
    for (uint32_t b = 0; b < i; b++)
      expected_bits |= uint32_t(1) << b;
    EXPECT_EQ(Support::lsb_mask<uint32_t>(i), expected_bits);
  }

  INFO("Support::lsb_mask<uint64_t>()");
  for (uint32_t i = 0; i < 64; i++) {
    uint64_t expected_bits = 0;
    for (uint32_t b = 0; b < i; b++)
      expected_bits |= uint64_t(1) << b;
    EXPECT_EQ(Support::lsb_mask<uint64_t>(i), expected_bits);
  }

  INFO("Support::fill_trailing_bits()");
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(0u)), uint8_t(0u));
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(1u)), uint8_t(1u));
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(2u)), uint8_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(3u)), uint8_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(4u)), uint8_t(7u));
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(0x80u)), uint8_t(0xFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(0xFEu)), uint8_t(0xFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint8_t(0xFFu)), uint8_t(0xFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(0u)), uint16_t(0u));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(1u)), uint16_t(1u));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(2u)), uint16_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(3u)), uint16_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(4u)), uint16_t(7u));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(0x8000u)), uint16_t(0xFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(0xFFFEu)), uint16_t(0xFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint16_t(0xFFFFu)), uint16_t(0xFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(0u)), uint32_t(0u));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(1u)), uint32_t(1u));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(2u)), uint32_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(3u)), uint32_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(4u)), uint32_t(7u));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(0x80000000u)), uint32_t(0xFFFFFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(0xFFFFFFFEu)), uint32_t(0xFFFFFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint32_t(0xFFFFFFFFu)), uint32_t(0xFFFFFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(0u)), uint64_t(0u));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(1u)), uint64_t(1u));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(2u)), uint64_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(3u)), uint64_t(3u));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(4u)), uint64_t(7u));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(0x8000000000000000u)), uint64_t(0xFFFFFFFFFFFFFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(0xFFFFFFFFFFFFFFFEu)), uint64_t(0xFFFFFFFFFFFFFFFFu));
  EXPECT_EQ(Support::fill_trailing_bits(uint64_t(0xFFFFFFFFFFFFFFFFu)), uint64_t(0xFFFFFFFFFFFFFFFFu));

  INFO("Support::is_power_of_2()");
  EXPECT_FALSE(Support::is_power_of_2(uint8_t(0)));
  EXPECT_FALSE(Support::is_power_of_2(uint16_t(0)));
  EXPECT_FALSE(Support::is_power_of_2(uint32_t(0)));
  EXPECT_FALSE(Support::is_power_of_2(uint64_t(0)));

  EXPECT_FALSE(Support::is_power_of_2(uint8_t(0xFFu)));
  EXPECT_FALSE(Support::is_power_of_2(uint16_t(0xFFFFu)));
  EXPECT_FALSE(Support::is_power_of_2(uint32_t(0xFFFFFFFFu)));
  EXPECT_FALSE(Support::is_power_of_2(uint64_t(0xFFFFFFFFFFFFFFFFu)));

  for (uint32_t i = 0; i < 32; i++) {
    EXPECT_TRUE(Support::is_power_of_2(uint32_t(1) << i));
    EXPECT_FALSE(Support::is_power_of_2((uint32_t(1) << i) ^ 0x001101));
  }

  for (uint32_t i = 0; i < 64; i++) {
    EXPECT_TRUE(Support::is_power_of_2(uint64_t(1) << i));
    EXPECT_FALSE(Support::is_power_of_2((uint64_t(1) << i) ^ 0x001101));
  }

  INFO("Support::is_power_of_2_up_to()");
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint8_t(0), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint16_t(0), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint32_t(0), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint64_t(0), 8));

  EXPECT_FALSE(Support::is_power_of_2_up_to(uint8_t(0xFFu), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint16_t(0xFFFFu), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint32_t(0xFFFFFFFFu), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint64_t(0xFFFFFFFFFFFFFFFFu), 8));

  EXPECT_TRUE(Support::is_power_of_2_up_to(uint32_t(1), 8));
  EXPECT_TRUE(Support::is_power_of_2_up_to(uint32_t(2), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint32_t(3), 8));
  EXPECT_TRUE(Support::is_power_of_2_up_to(uint32_t(4), 8));
  EXPECT_TRUE(Support::is_power_of_2_up_to(uint32_t(8), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint32_t(9), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint32_t(16), 8));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint32_t(0xFFFFFFFFu), 8));

  EXPECT_TRUE(Support::is_power_of_2_up_to(uint32_t(16), 16));
  EXPECT_FALSE(Support::is_power_of_2_up_to(uint32_t(32), 16));

  INFO("Support::is_zero_or_power_of_2()");
  EXPECT_TRUE(Support::is_zero_or_power_of_2(uint8_t(0)));
  EXPECT_TRUE(Support::is_zero_or_power_of_2(uint16_t(0)));
  EXPECT_TRUE(Support::is_zero_or_power_of_2(uint32_t(0)));
  EXPECT_TRUE(Support::is_zero_or_power_of_2(uint64_t(0)));

  EXPECT_FALSE(Support::is_zero_or_power_of_2(uint8_t(0xFFu)));
  EXPECT_FALSE(Support::is_zero_or_power_of_2(uint16_t(0xFFFFu)));
  EXPECT_FALSE(Support::is_zero_or_power_of_2(uint32_t(0xFFFFFFFFu)));
  EXPECT_FALSE(Support::is_zero_or_power_of_2(uint64_t(0xFFFFFFFFFFFFFFFFu)));

  for (uint32_t i = 0; i < 32; i++) {
    EXPECT_TRUE(Support::is_zero_or_power_of_2(uint32_t(1) << i));
    EXPECT_FALSE(Support::is_zero_or_power_of_2((uint32_t(1) << i) ^ 0x001101));
  }

  for (uint32_t i = 0; i < 64; i++) {
    EXPECT_TRUE(Support::is_zero_or_power_of_2(uint64_t(1) << i));
    EXPECT_FALSE(Support::is_zero_or_power_of_2((uint64_t(1) << i) ^ 0x001101));
  }

  INFO("Support::is_zero_or_power_of_2_up_to()");
  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint8_t(0), 8));
  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint16_t(0), 8));
  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint32_t(0), 8));
  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint64_t(0), 8));

  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint8_t(0xFFu), 8));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint16_t(0xFFFFu), 8));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint32_t(0xFFFFFFFFu), 8));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint64_t(0xFFFFFFFFFFFFFFFFu), 8));

  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint32_t(1), 8));
  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint32_t(2), 8));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint32_t(3), 8));
  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint32_t(4), 8));
  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint32_t(8), 8));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint32_t(9), 8));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint32_t(16), 8));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint32_t(0xFFFFFFFFu), 8));

  EXPECT_TRUE(Support::is_zero_or_power_of_2_up_to(uint32_t(16), 16));
  EXPECT_FALSE(Support::is_zero_or_power_of_2_up_to(uint32_t(32), 16));
}

static void test_int_utils() noexcept {
  INFO("Support::byteswap()");
  EXPECT_EQ(Support::byteswap16(0x0102), 0x0201u);
  EXPECT_EQ(Support::byteswap32(0x01020304), 0x04030201u);
  EXPECT_EQ(Support::byteswap32(0x01020304), 0x04030201u);
  EXPECT_EQ(Support::byteswap64(uint64_t(0x0102030405060708)), uint64_t(0x0807060504030201));

  INFO("Support::bytepack()");
  union BytePackData {
    uint8_t bytes[4];
    uint32_t u32;
  } bpdata;

  bpdata.u32 = Support::bytepack32_4x8(0x00, 0x11, 0x22, 0x33);
  EXPECT_EQ(bpdata.bytes[0], 0x00);
  EXPECT_EQ(bpdata.bytes[1], 0x11);
  EXPECT_EQ(bpdata.bytes[2], 0x22);
  EXPECT_EQ(bpdata.bytes[3], 0x33);

  INFO("Support::is_between()");
  EXPECT_TRUE(Support::is_between<int>(10 , 10, 20));
  EXPECT_TRUE(Support::is_between<int>(11 , 10, 20));
  EXPECT_TRUE(Support::is_between<int>(20 , 10, 20));
  EXPECT_FALSE(Support::is_between<int>(9  , 10, 20));
  EXPECT_FALSE(Support::is_between<int>(21 , 10, 20));
  EXPECT_FALSE(Support::is_between<int>(101, 10, 20));

  INFO("Support::is_int_n<8>()");
  EXPECT_TRUE(Support::is_int_n<8>(-128));
  EXPECT_TRUE(Support::is_int_n<8>(127));
  EXPECT_FALSE(Support::is_int_n<8>(-129));
  EXPECT_FALSE(Support::is_int_n<8>(-1000));
  EXPECT_FALSE(Support::is_int_n<8>(128));
  EXPECT_FALSE(Support::is_int_n<8>(1000));

  INFO("Support::is_int_n<9>()");
  EXPECT_TRUE(Support::is_int_n<9>(-256));
  EXPECT_TRUE(Support::is_int_n<9>(255));
  EXPECT_FALSE(Support::is_int_n<9>(-257));
  EXPECT_FALSE(Support::is_int_n<9>(-1000));
  EXPECT_FALSE(Support::is_int_n<9>(256));
  EXPECT_FALSE(Support::is_int_n<9>(1000));

  INFO("Support::is_int_n<10>()");
  EXPECT_TRUE(Support::is_int_n<10>(-512));
  EXPECT_TRUE(Support::is_int_n<10>(511));
  EXPECT_FALSE(Support::is_int_n<10>(-513));
  EXPECT_FALSE(Support::is_int_n<10>(-1000));
  EXPECT_FALSE(Support::is_int_n<10>(512));
  EXPECT_FALSE(Support::is_int_n<10>(1000));

  INFO("Support::is_int_n<16>()");
  EXPECT_TRUE(Support::is_int_n<16>(-32768));
  EXPECT_TRUE(Support::is_int_n<16>(32767));
  EXPECT_FALSE(Support::is_int_n<16>(-32769));
  EXPECT_FALSE(Support::is_int_n<16>(-100000));
  EXPECT_FALSE(Support::is_int_n<16>(32768));
  EXPECT_FALSE(Support::is_int_n<16>(100000));

  INFO("Support::is_int_n<33>()");
  EXPECT_TRUE(Support::is_int_n<33>(int64_t(4294967295)));
  EXPECT_TRUE(Support::is_int_n<33>(int64_t(-4294967296)));
  EXPECT_FALSE(Support::is_int_n<33>(uint64_t(4294967296)));
  EXPECT_FALSE(Support::is_int_n<33>(uint64_t(0x1FFFFFFFFu)));
  EXPECT_FALSE(Support::is_int_n<33>(uint64_t(0x1FFFFFFFFu) + 1));

  INFO("Support::is_int_n<33>()");
  EXPECT_TRUE(Support::is_int_n<32>( 2147483647    ));
  EXPECT_TRUE(Support::is_int_n<32>(-2147483647 - 1));
  EXPECT_FALSE(Support::is_int_n<32>(uint64_t(2147483648u)));
  EXPECT_FALSE(Support::is_int_n<32>(uint64_t(0xFFFFFFFFu)));
  EXPECT_FALSE(Support::is_int_n<32>(uint64_t(0xFFFFFFFFu) + 1));

  INFO("Support::is_uint_n<8>()");
  EXPECT_TRUE(Support::is_uint_n<8>(0));
  EXPECT_TRUE(Support::is_uint_n<8>(255));
  EXPECT_FALSE(Support::is_uint_n<8>(256));
  EXPECT_FALSE(Support::is_uint_n<8>(1000));
  EXPECT_FALSE(Support::is_uint_n<8>(-1));
  EXPECT_FALSE(Support::is_uint_n<8>(-1000));

  INFO("Support::is_uint_n<9>()");
  EXPECT_TRUE(Support::is_uint_n<9>(0)  );
  EXPECT_TRUE(Support::is_uint_n<9>(511));
  EXPECT_FALSE(Support::is_uint_n<9>(512));
  EXPECT_FALSE(Support::is_uint_n<9>(1000));
  EXPECT_FALSE(Support::is_uint_n<9>(-1));
  EXPECT_FALSE(Support::is_uint_n<9>(-1000));

  INFO("Support::is_uint_n<10>()");
  EXPECT_TRUE(Support::is_uint_n<10>(0)  );
  EXPECT_TRUE(Support::is_uint_n<10>(1023));
  EXPECT_FALSE(Support::is_uint_n<10>(1024));
  EXPECT_FALSE(Support::is_uint_n<10>(10000));
  EXPECT_FALSE(Support::is_uint_n<10>(-1));
  EXPECT_FALSE(Support::is_uint_n<10>(-10000));

  INFO("Support::is_uint_n<12>()");
  EXPECT_TRUE(Support::is_uint_n<12>(0));
  EXPECT_TRUE(Support::is_uint_n<12>(4095));
  EXPECT_FALSE(Support::is_uint_n<12>(4096));
  EXPECT_FALSE(Support::is_uint_n<12>(10000));
  EXPECT_FALSE(Support::is_uint_n<12>(-1));
  EXPECT_FALSE(Support::is_uint_n<12>(-1000));

  INFO("Support::is_uint_n<16>()");
  EXPECT_TRUE(Support::is_uint_n<16>(0));
  EXPECT_TRUE(Support::is_uint_n<16>(65535));
  EXPECT_FALSE(Support::is_uint_n<16>(65536));
  EXPECT_FALSE(Support::is_uint_n<16>(100000));
  EXPECT_FALSE(Support::is_uint_n<16>(-1));
  EXPECT_FALSE(Support::is_uint_n<16>(-1000));

  INFO("Support::is_uint_n<32>()");
  EXPECT_TRUE(Support::is_uint_n<32>(uint64_t(0xFFFFFFFF)));
  EXPECT_FALSE(Support::is_uint_n<32>(uint64_t(0xFFFFFFFF) + 1));
  EXPECT_FALSE(Support::is_uint_n<32>(-1));

  INFO("Support::is_uint_n<33>()");
  EXPECT_TRUE(Support::is_uint_n<33>(uint64_t(0x1FFFFFFFF)));
  EXPECT_FALSE(Support::is_uint_n<33>(uint64_t(0x1FFFFFFFF) + 1));
  EXPECT_FALSE(Support::is_uint_n<33>(-1));
}

static void test_memory_access() noexcept {
  INFO("Support::load() / store()");

  uint8_t arr[32] = { 0 };

  Support::storeu_u16_be(arr + 1, 0x0102u);
  Support::storeu_u16_be(arr + 3, 0x0304u);
  EXPECT_EQ(Support::loadu_u32_be(arr + 1), 0x01020304u);
  EXPECT_EQ(Support::loadu_u32_le(arr + 1), 0x04030201u);
  EXPECT_EQ(Support::loadu_u32_be(arr + 2), 0x02030400u);
  EXPECT_EQ(Support::loadu_u32_le(arr + 2), 0x00040302u);

  Support::storeu_u32_le(arr + 5, 0x05060708u);
  EXPECT_EQ(Support::loadu_u64_be(arr + 1), 0x0102030408070605u);
  EXPECT_EQ(Support::loadu_u64_le(arr + 1), 0x0506070804030201u);

  Support::storeu_u64_le(arr + 7, 0x1122334455667788u);
  EXPECT_EQ(Support::loadu_u32_be(arr + 8), 0x77665544u);

  double d = 134.44;
  Support::storeu(arr, d);
  EXPECT_EQ(Support::loadu<double>(arr), d);
}

static void test_bit_vector() noexcept {
  INFO("Support::bit_vector_op");
  {
    uint32_t vec[3] = { 0 };
    Support::bit_vector_fill(vec, 1, 64);
    EXPECT_EQ(vec[0], 0xFFFFFFFEu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    Support::bit_vector_clear(vec, 1, 1);
    EXPECT_EQ(vec[0], 0xFFFFFFFCu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    Support::bit_vector_fill(vec, 0, 32);
    EXPECT_EQ(vec[0], 0xFFFFFFFFu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    Support::bit_vector_clear(vec, 0, 32);
    EXPECT_EQ(vec[0], 0x00000000u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    Support::bit_vector_fill(vec, 1, 30);
    EXPECT_EQ(vec[0], 0x7FFFFFFEu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    Support::bit_vector_clear(vec, 1, 95);
    EXPECT_EQ(vec[0], 0x00000000u);
    EXPECT_EQ(vec[1], 0x00000000u);
    EXPECT_EQ(vec[2], 0x00000000u);

    Support::bit_vector_fill(vec, 32, 64);
    EXPECT_EQ(vec[0], 0x00000000u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0xFFFFFFFFu);

    Support::bit_vector_set_bit(vec, 1, true);
    EXPECT_EQ(vec[0], 0x00000002u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0xFFFFFFFFu);

    Support::bit_vector_set_bit(vec, 95, false);
    EXPECT_EQ(vec[0], 0x00000002u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x7FFFFFFFu);

    Support::bit_vector_clear(vec, 33, 32);
    EXPECT_EQ(vec[0], 0x00000002u);
    EXPECT_EQ(vec[1], 0x00000001u);
    EXPECT_EQ(vec[2], 0x7FFFFFFEu);
  }

  INFO("Support::bit_vector_index_of");
  {
    uint32_t vec1[1] = { 0x80000000 };
    EXPECT_EQ(Support::bit_vector_index_of(vec1, 0, true), 31u);
    EXPECT_EQ(Support::bit_vector_index_of(vec1, 1, true), 31u);
    EXPECT_EQ(Support::bit_vector_index_of(vec1, 31, true), 31u);

    uint32_t vec2[2] = { 0x00000000, 0x80000000 };
    EXPECT_EQ(Support::bit_vector_index_of(vec2, 0, true), 63u);
    EXPECT_EQ(Support::bit_vector_index_of(vec2, 1, true), 63u);
    EXPECT_EQ(Support::bit_vector_index_of(vec2, 31, true), 63u);
    EXPECT_EQ(Support::bit_vector_index_of(vec2, 32, true), 63u);
    EXPECT_EQ(Support::bit_vector_index_of(vec2, 33, true), 63u);
    EXPECT_EQ(Support::bit_vector_index_of(vec2, 63, true), 63u);

    uint32_t vec3[3] = { 0x00000001, 0x00000000, 0x80000000 };
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 0, true), 0u);
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 1, true), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 2, true), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 31, true), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 32, true), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 63, true), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 64, true), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec3, 95, true), 95u);

    uint32_t vec4[3] = { ~vec3[0], ~vec3[1], ~vec3[2] };
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 0, false), 0u);
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 1, false), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 2, false), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 31, false), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 32, false), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 63, false), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 64, false), 95u);
    EXPECT_EQ(Support::bit_vector_index_of(vec4, 95, false), 95u);
  }

  INFO("Support::BitWordIterator<uint32_t>");
  {
    Support::BitWordIterator<uint32_t> it(0x80000F01u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 0u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 8u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 9u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 10u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 11u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_FALSE(it.has_next());

    // No bits set.
    it.init(0x00000000u);
    EXPECT_FALSE(it.has_next());

    // Only first bit set.
    it.init(0x00000001u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 0u);
    EXPECT_FALSE(it.has_next());

    // Only last bit set (special case).
    it.init(0x80000000u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_FALSE(it.has_next());
  }

  INFO("Support::BitWordIterator<uint64_t>");
  {
    Support::BitWordIterator<uint64_t> it(uint64_t(1) << 63);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_FALSE(it.has_next());
  }

  INFO("Support::BitVectorIterator<uint32_t>");
  {
    // Border cases.
    static const uint32_t bits_none[] = { 0xFFFFFFFFu };
    Support::BitVectorIterator<uint32_t> it(Span<const uint32_t>(bits_none, 0));

    EXPECT_FALSE(it.has_next());
    it.init(Span<const uint32_t>(bits_none, 0), 1);
    EXPECT_FALSE(it.has_next());
    it.init(Span<const uint32_t>(bits_none, 0), 128);
    EXPECT_FALSE(it.has_next());

    static const uint32_t bits1[] = { 0x80000008u, 0x80000001u, 0x00000000u, 0x80000000u, 0x00000000u, 0x00000000u, 0x00003000u };
    it.init(Span<const uint32_t>::from_array(bits1));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 3u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 32u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 204u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 205u);
    EXPECT_FALSE(it.has_next());

    it.init(Span<const uint32_t>::from_array(bits1), 4);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);

    it.init(Span<const uint32_t>::from_array(bits1), 64);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);

    it.init(Span<const uint32_t>::from_array(bits1), 127);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);

    static const uint32_t bits2[] = { 0x80000000u, 0x80000000u, 0x00000000u, 0x80000000u };
    it.init(Span<const uint32_t>::from_array(bits2));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);
    EXPECT_FALSE(it.has_next());

    static const uint32_t bits3[] = { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u };
    it.init(Span<const uint32_t>::from_array(bits3));
    EXPECT_FALSE(it.has_next());

    static const uint32_t bits4[] = { 0x00000000u, 0x00000000u, 0x00000000u, 0x80000000u };
    it.init(Span<const uint32_t>::from_array(bits4));
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);
    EXPECT_FALSE(it.has_next());
  }

  INFO("Support::BitVectorIterator<uint64_t>");
  {
    static const uint64_t bits1[] = { 0x80000000u, 0x80000000u, 0x00000000u, 0x80000000u };
    Support::BitVectorIterator<uint64_t> it(Span<const uint64_t>::from_array(bits1));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 95u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 223u);
    EXPECT_FALSE(it.has_next());

    static const uint64_t bits2[] = { 0x8000000000000000u, 0, 0, 0 };
    it.init(Span<const uint64_t>::from_array(bits2));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_FALSE(it.has_next());
  }
}

static void test_sorting() noexcept {
  INFO("Support::sort() - Testing qsort and isort of predefined arrays");
  {
    constexpr size_t kArraySize = 11;

    int ref_[kArraySize] = { -4, -2, -1, 0, 1, 9, 12, 13, 14, 19, 22 };
    int arr1[kArraySize] = { 0, 1, -1, 19, 22, 14, -4, 9, 12, 13, -2 };
    int arr2[kArraySize];

    memcpy(arr2, arr1, kArraySize * sizeof(int));

    Support::insertion_sort(arr1, kArraySize);
    Support::sort(arr2, kArraySize);
    test_arrays(arr1, ref_, kArraySize);
    test_arrays(arr2, ref_, kArraySize);
  }

  INFO("Support::sort() - Testing qsort and isort of artificial arrays");
  {
    constexpr size_t kArraySize = 200;

    int arr1[kArraySize];
    int arr2[kArraySize];
    int ref_[kArraySize];

    for (size_t size = 2; size < kArraySize; size++) {
      for (size_t i = 0; i < size; i++) {
        arr1[i] = int(size - 1 - i);
        arr2[i] = int(size - 1 - i);
        ref_[i] = int(i);
      }

      Support::insertion_sort(arr1, size);
      Support::sort(arr2, size);
      test_arrays(arr1, ref_, size);
      test_arrays(arr2, ref_, size);
    }
  }

  INFO("Support::sort() - Testing qsort and isort with an unstable compare function");
  {
    constexpr size_t kArraySize = 5;

    float arr1[kArraySize] = { 1.0f, 0.0f, 3.0f, -1.0f, std::numeric_limits<float>::quiet_NaN() };
    float arr2[kArraySize] = { };

    memcpy(arr2, arr1, kArraySize * sizeof(float));

    // We don't test as it's undefined where the NaN would be.
    Support::insertion_sort(arr1, kArraySize);
    Support::sort(arr2, kArraySize);
  }
}

UNIT(support) {
  test_alignment();
  test_bit_utils();
  test_int_utils();
  test_memory_access();
  test_bit_vector();
  test_sorting();
}
#endif

ASMJIT_END_NAMESPACE
