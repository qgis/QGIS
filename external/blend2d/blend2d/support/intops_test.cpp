// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/support/intops_p.h>

// bl::IntOps - Tests
// ==================

namespace bl {
namespace Tests {

UNIT(support_intops_alignment, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  INFO("Testing bl::IntOps::is_aligned()");
  {
    EXPECT_FALSE(IntOps::is_aligned<size_t>(0xFFFF,  4));
    EXPECT_TRUE(IntOps::is_aligned<size_t>(0xFFF4,  4));
    EXPECT_TRUE(IntOps::is_aligned<size_t>(0xFFF8,  8));
    EXPECT_TRUE(IntOps::is_aligned<size_t>(0xFFF0, 16));
  }

  INFO("Testing bl::IntOps::is_power_of_2()");
  {
    for (uint32_t i = 0; i < 64; i++) {
      EXPECT_TRUE(IntOps::is_power_of_2(uint64_t(1) << i));
      EXPECT_FALSE(IntOps::is_power_of_2((uint64_t(1) << i) ^ 0x001101));
    }
  }

  INFO("Testing bl::IntOps::align_up()");
  {
    EXPECT_EQ(IntOps::align_up<size_t>(0xFFFF,  4), 0x10000u);
    EXPECT_EQ(IntOps::align_up<size_t>(0xFFF4,  4), 0x0FFF4u);
    EXPECT_EQ(IntOps::align_up<size_t>(0xFFF8,  8), 0x0FFF8u);
    EXPECT_EQ(IntOps::align_up<size_t>(0xFFF0, 16), 0x0FFF0u);
    EXPECT_EQ(IntOps::align_up<size_t>(0xFFF0, 32), 0x10000u);
  }

  INFO("Testing bl::IntOps::align_up_diff()");
  {
    EXPECT_EQ(IntOps::align_up_diff<size_t>(0xFFFF,  4), 1u);
    EXPECT_EQ(IntOps::align_up_diff<size_t>(0xFFF4,  4), 0u);
    EXPECT_EQ(IntOps::align_up_diff<size_t>(0xFFF8,  8), 0u);
    EXPECT_EQ(IntOps::align_up_diff<size_t>(0xFFF0, 16), 0u);
    EXPECT_EQ(IntOps::align_up_diff<size_t>(0xFFF0, 32), 16u);
  }

  INFO("Testing bl::IntOps::align_up_power_of_2()");
  {
    EXPECT_EQ(IntOps::align_up_power_of_2<size_t>(0x0000), 0x00000u);
    EXPECT_EQ(IntOps::align_up_power_of_2<size_t>(0xFFFF), 0x10000u);
    EXPECT_EQ(IntOps::align_up_power_of_2<size_t>(0xF123), 0x10000u);
    EXPECT_EQ(IntOps::align_up_power_of_2<size_t>(0x0F00), 0x01000u);
    EXPECT_EQ(IntOps::align_up_power_of_2<size_t>(0x0100), 0x00100u);
    EXPECT_EQ(IntOps::align_up_power_of_2<size_t>(0x1001), 0x02000u);
  }
}

UNIT(support_intops_arithmetic, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  INFO("Testing bl::IntOps::add_overflow()");
  {
    OverflowFlag of{};

    EXPECT_TRUE(IntOps::add_overflow<int32_t>(0, 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(0, 1, &of) == 1 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(1, 0, &of) == 1 && !of);

    EXPECT_TRUE(IntOps::add_overflow<int32_t>(2147483647, 0, &of) == 2147483647 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(0, 2147483647, &of) == 2147483647 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(2147483647, -1, &of) == 2147483646 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(-1, 2147483647, &of) == 2147483646 && !of);

    EXPECT_TRUE(IntOps::add_overflow<int32_t>(-2147483647, 0, &of) == -2147483647 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(0, -2147483647, &of) == -2147483647 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(-2147483647, -1, &of) == -2147483647 - 1 && !of);
    EXPECT_TRUE(IntOps::add_overflow<int32_t>(-1, -2147483647, &of) == -2147483647 - 1 && !of);

    (void)IntOps::add_overflow<int32_t>(2147483647, 1, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::add_overflow<int32_t>(1, 2147483647, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::add_overflow<int32_t>(-2147483647, -2, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::add_overflow<int32_t>(-2, -2147483647, &of); EXPECT_NE(of, 0); of = 0;

    EXPECT_TRUE(IntOps::add_overflow<uint32_t>(0u, 0u, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::add_overflow<uint32_t>(0u, 1u, &of) == 1 && !of);
    EXPECT_TRUE(IntOps::add_overflow<uint32_t>(1u, 0u, &of) == 1 && !of);

    EXPECT_TRUE(IntOps::add_overflow<uint32_t>(2147483647u, 1u, &of) == 2147483648u && !of);
    EXPECT_TRUE(IntOps::add_overflow<uint32_t>(1u, 2147483647u, &of) == 2147483648u && !of);
    EXPECT_TRUE(IntOps::add_overflow<uint32_t>(0xFFFFFFFFu, 0u, &of) == 0xFFFFFFFFu && !of);
    EXPECT_TRUE(IntOps::add_overflow<uint32_t>(0u, 0xFFFFFFFFu, &of) == 0xFFFFFFFFu && !of);

    (void)IntOps::add_overflow<uint32_t>(0xFFFFFFFFu, 1u, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::add_overflow<uint32_t>(1u, 0xFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::add_overflow<uint32_t>(0x80000000u, 0xFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::add_overflow<uint32_t>(0xFFFFFFFFu, 0x80000000u, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::add_overflow<uint32_t>(0xFFFFFFFFu, 0xFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;
  }

  INFO("Testing bl::IntOps::sub_overflow()");
  {
    OverflowFlag of{};

    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(0, 0, &of) ==  0 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(0, 1, &of) == -1 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(1, 0, &of) ==  1 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(0, -1, &of) ==  1 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(-1, 0, &of) == -1 && !of);

    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(2147483647, 1, &of) == 2147483646 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(2147483647, 2147483647, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(-2147483647, 1, &of) == -2147483647 - 1 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(-2147483647, -1, &of) == -2147483646 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(-2147483647 - 0, -2147483647 - 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<int32_t>(-2147483647 - 1, -2147483647 - 1, &of) == 0 && !of);

    (void)IntOps::sub_overflow<int32_t>(-2, 2147483647, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::sub_overflow<int32_t>(-2147483647, 2, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::sub_overflow<int32_t>(-2147483647    , 2147483647, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::sub_overflow<int32_t>(-2147483647 - 1, 2147483647, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::sub_overflow<int32_t>(2147483647, -2147483647 - 0, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::sub_overflow<int32_t>(2147483647, -2147483647 - 1, &of); EXPECT_NE(of, 0); of = 0;

    EXPECT_TRUE(IntOps::sub_overflow<uint32_t>(0, 0, &of) ==  0 && !of);
    EXPECT_TRUE(IntOps::sub_overflow<uint32_t>(1, 0, &of) ==  1 && !of);

    EXPECT_TRUE(IntOps::sub_overflow<uint32_t>(0xFFFFFFFFu, 0u, &of) == 0xFFFFFFFFu && !of);
    EXPECT_TRUE(IntOps::sub_overflow<uint32_t>(0xFFFFFFFFu, 0xFFFFFFFFu, &of) == 0u && !of);

    (void)IntOps::sub_overflow<uint32_t>(0u, 1u, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::sub_overflow<uint32_t>(1u, 2u, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::sub_overflow<uint32_t>(0u, 0xFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::sub_overflow<uint32_t>(1u, 0xFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::sub_overflow<uint32_t>(0u, 0x7FFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::sub_overflow<uint32_t>(1u, 0x7FFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::sub_overflow<uint32_t>(0x7FFFFFFEu, 0x7FFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::sub_overflow<uint32_t>(0xFFFFFFFEu, 0xFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;
  }

  INFO("Testing bl::IntOps::mul_overflow()");
  {
    OverflowFlag of{};

    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(0, 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(0, 1, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(1, 0, &of) == 0 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int32_t>( 1,  1, &of) ==  1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>( 1, -1, &of) == -1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(-1,  1, &of) == -1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(-1, -1, &of) ==  1 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int32_t>( 32768,  65535, &of) ==  2147450880 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>( 32768, -65535, &of) == -2147450880 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(-32768,  65535, &of) == -2147450880 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(-32768, -65535, &of) ==  2147450880 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(2147483647, 1, &of) == 2147483647 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(1, 2147483647, &of) == 2147483647 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(-2147483647 - 1, 1, &of) == -2147483647 - 1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int32_t>(1, -2147483647 - 1, &of) == -2147483647 - 1 && !of);

    (void)IntOps::mul_overflow<int32_t>( 65535,  65535, &of); EXPECT_TRUE(of); of = 0;
    (void)IntOps::mul_overflow<int32_t>( 65535, -65535, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int32_t>(-65535,  65535, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int32_t>(-65535, -65535, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::mul_overflow<int32_t>( 2147483647    ,  2147483647    , &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int32_t>( 2147483647    , -2147483647 - 1, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int32_t>(-2147483647 - 1,  2147483647    , &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int32_t>(-2147483647 - 1, -2147483647 - 1, &of); EXPECT_NE(of, 0); of = 0;

    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(0, 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(0, 1, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(1, 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(1, 1, &of) == 1 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(0x10000000u, 15, &of) == 0xF0000000u && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(15, 0x10000000u, &of) == 0xF0000000u && !of);

    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(0xFFFFFFFFu, 1, &of) == 0xFFFFFFFFu && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint32_t>(1, 0xFFFFFFFFu, &of) == 0xFFFFFFFFu && !of);

    (void)IntOps::mul_overflow<uint32_t>(0xFFFFFFFFu, 2, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<uint32_t>(2, 0xFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::mul_overflow<uint32_t>(0x80000000u, 2, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<uint32_t>(2, 0x80000000u, &of); EXPECT_NE(of, 0); of = 0;

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(0, 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(0, 1, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(1, 0, &of) == 0 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 1,  1, &of) ==  1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 1, -1, &of) == -1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-1,  1, &of) == -1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-1, -1, &of) ==  1 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 32768,  65535, &of) ==  2147450880 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 32768, -65535, &of) == -2147450880 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-32768,  65535, &of) == -2147450880 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-32768, -65535, &of) ==  2147450880 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(2147483647, 1, &of) == 2147483647 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(1, 2147483647, &of) == 2147483647 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-2147483647 - 1, 1, &of) == -2147483647 - 1 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(1, -2147483647 - 1, &of) == -2147483647 - 1 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 65535,  65535, &of) ==  int64_t(4294836225) && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 65535, -65535, &of) == -int64_t(4294836225) && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-65535,  65535, &of) == -int64_t(4294836225) && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-65535, -65535, &of) ==  int64_t(4294836225) && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 2147483647    ,  2147483647    , &of) ==  int64_t(4611686014132420609) && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>( 2147483647    , -2147483647 - 1, &of) == -int64_t(4611686016279904256) && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-2147483647 - 1,  2147483647    , &of) == -int64_t(4611686016279904256) && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(-2147483647 - 1, -2147483647 - 1, &of) ==  int64_t(4611686018427387904) && !of);

    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(int64_t(0x7FFFFFFFFFFFFFFF), int64_t(1), &of) == int64_t(0x7FFFFFFFFFFFFFFF) && !of);
    EXPECT_TRUE(IntOps::mul_overflow<int64_t>(int64_t(1), int64_t(0x7FFFFFFFFFFFFFFF), &of) == int64_t(0x7FFFFFFFFFFFFFFF) && !of);

    (void)IntOps::mul_overflow<int64_t>(int64_t(0x7FFFFFFFFFFFFFFF), int64_t(2), &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int64_t>(int64_t(2), int64_t(0x7FFFFFFFFFFFFFFF), &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::mul_overflow<int64_t>(int64_t( 0x7FFFFFFFFFFFFFFF), int64_t( 0x7FFFFFFFFFFFFFFF), &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int64_t>(int64_t( 0x7FFFFFFFFFFFFFFF), int64_t(-0x7FFFFFFFFFFFFFFF), &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int64_t>(int64_t(-0x7FFFFFFFFFFFFFFF), int64_t( 0x7FFFFFFFFFFFFFFF), &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<int64_t>(int64_t(-0x7FFFFFFFFFFFFFFF), int64_t(-0x7FFFFFFFFFFFFFFF), &of); EXPECT_NE(of, 0); of = 0;

    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(0, 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(0, 1, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(1, 0, &of) == 0 && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(1, 1, &of) == 1 && !of);

    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(0x1000000000000000u, 15, &of) == 0xF000000000000000u && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(15, 0x1000000000000000u, &of) == 0xF000000000000000u && !of);

    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(0xFFFFFFFFFFFFFFFFu, 1, &of) == 0xFFFFFFFFFFFFFFFFu && !of);
    EXPECT_TRUE(IntOps::mul_overflow<uint64_t>(1, 0xFFFFFFFFFFFFFFFFu, &of) == 0xFFFFFFFFFFFFFFFFu && !of);

    (void)IntOps::mul_overflow<uint64_t>(0xFFFFFFFFFFFFFFFFu, 2, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<uint64_t>(2, 0xFFFFFFFFFFFFFFFFu, &of); EXPECT_NE(of, 0); of = 0;

    (void)IntOps::mul_overflow<uint64_t>(0x8000000000000000u, 2, &of); EXPECT_NE(of, 0); of = 0;
    (void)IntOps::mul_overflow<uint64_t>(2, 0x8000000000000000u, &of); EXPECT_NE(of, 0); of = 0;
  }
}

template<typename T>
BL_INLINE bool check_consecutive_bit_mask(T x) {
  if (x == 0)
    return false;

  T m = x;
  while ((m & 0x1) == 0)
    m >>= 1;
  return ((m + 1) & m) == 0;
}

UNIT(support_intops_bit_manipulation, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  uint32_t i;

  INFO("Testing bl::IntOps::shl() / IntOps::shr()");
  EXPECT_EQ(IntOps::shl<int32_t >(0x00001111 , 16), 0x11110000 );
  EXPECT_EQ(IntOps::shl<uint32_t>(0x00001111 , 16), 0x11110000u );
  EXPECT_EQ(IntOps::shr<int32_t >(0x11110000u, 16), 0x00001111 );
  EXPECT_EQ(IntOps::shr<uint32_t>(0x11110000u, 16), 0x00001111u);
  EXPECT_EQ(IntOps::sar<uint32_t>(0xFFFF0000u, 16), 0xFFFFFFFFu);

  INFO("Testing bl::IntOps::rol() / IntOps::ror()");
  EXPECT_EQ(IntOps::rol<int32_t >(0x00100000 , 16), 0x00000010 );
  EXPECT_EQ(IntOps::rol<uint32_t>(0x00100000u, 16), 0x00000010u);
  EXPECT_EQ(IntOps::ror<int32_t >(0x00001000 , 16), 0x10000000 );
  EXPECT_EQ(IntOps::ror<uint32_t>(0x00001000u, 16), 0x10000000u);

  INFO("Testing bl::IntOps::clz()");
  EXPECT_EQ(IntOps::clz<uint32_t>(1u), 31u);
  EXPECT_EQ(IntOps::clz<uint32_t>(2u), 30u);
  EXPECT_EQ(IntOps::clz<uint32_t>(3u), 30u);
  EXPECT_EQ(IntOps::clz<uint32_t>(0x80000000u), 0u);
  EXPECT_EQ(IntOps::clz<uint32_t>(0x88888888u), 0u);
  EXPECT_EQ(IntOps::clz<uint32_t>(0x11111111u), 3u);
  EXPECT_EQ(IntOps::clz<uint32_t>(0x12345678u), 3u);
  EXPECT_EQ(IntOps::clz_static<uint32_t>(1u), 31u);
  EXPECT_EQ(IntOps::clz_static<uint32_t>(2u), 30u);
  EXPECT_EQ(IntOps::clz_static<uint32_t>(3u), 30u);
  EXPECT_EQ(IntOps::clz_static<uint32_t>(0x80000000u), 0u);
  EXPECT_EQ(IntOps::clz_static<uint32_t>(0x88888888u), 0u);
  EXPECT_EQ(IntOps::clz_static<uint32_t>(0x11111111u), 3u);
  EXPECT_EQ(IntOps::clz_static<uint32_t>(0x12345678u), 3u);

  for (i = 0; i < 32; i++) {
    EXPECT_EQ(IntOps::clz(uint32_t(1) << i), 31 - i);
    EXPECT_EQ(IntOps::clz(uint32_t(0xFFFFFFFFu) >> i), i);
  }

  INFO("Testing bl::IntOps::ctz()");
  EXPECT_EQ(IntOps::ctz<uint32_t>(1u), 0u);
  EXPECT_EQ(IntOps::ctz<uint32_t>(2u), 1u);
  EXPECT_EQ(IntOps::ctz<uint32_t>(3u), 0u);
  EXPECT_EQ(IntOps::ctz<uint32_t>(0x80000000u), 31u);
  EXPECT_EQ(IntOps::ctz<uint32_t>(0x88888888u), 3u);
  EXPECT_EQ(IntOps::ctz<uint32_t>(0x11111111u), 0u);
  EXPECT_EQ(IntOps::ctz<uint32_t>(0x12345678u), 3u);
  EXPECT_EQ(IntOps::ctz_static<uint32_t>(1u), 0u);
  EXPECT_EQ(IntOps::ctz_static<uint32_t>(2u), 1u);
  EXPECT_EQ(IntOps::ctz_static<uint32_t>(3u), 0u);
  EXPECT_EQ(IntOps::ctz_static<uint32_t>(0x80000000u), 31u);
  EXPECT_EQ(IntOps::ctz_static<uint32_t>(0x88888888u), 3u);
  EXPECT_EQ(IntOps::ctz_static<uint32_t>(0x11111111u), 0u);
  EXPECT_EQ(IntOps::ctz_static<uint32_t>(0x12345678u), 3u);

  for (i = 0; i < 32; i++) {
    EXPECT_EQ(IntOps::ctz(uint32_t(1) << i), i);
    EXPECT_EQ(IntOps::ctz(uint32_t(0xFFFFFFFFu) << i), i);
  }

  INFO("Testing bl::IntOps::is_bit_mask_consecutive()");
  i = 0;
  for (;;) {
    bool result = IntOps::is_bit_mask_consecutive(i);
    bool expect = check_consecutive_bit_mask(i);
    EXPECT_EQ(result, expect);

    if (i == 0xFFFFu)
      break;
    i++;
  }
}

UNIT(support_intops_byteswap, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  INFO("Testing bl::IntOps::byteSwap16()");
  EXPECT_EQ(IntOps::byteSwap16(int16_t(0x0102)), int16_t(0x0201));
  EXPECT_EQ(IntOps::byteSwap16(uint16_t(0x0102)), uint16_t(0x0201));

  INFO("Testing bl::IntOps::byteSwap24()");
  EXPECT_EQ(IntOps::byteSwap24(int32_t(0x00010203)), int32_t(0x00030201));
  EXPECT_EQ(IntOps::byteSwap24(uint32_t(0x00010203)), uint32_t(0x00030201));

  INFO("Testing bl::IntOps::byteSwap32()");
  EXPECT_EQ(IntOps::byteSwap32(int32_t(0x01020304)), int32_t(0x04030201));
  EXPECT_EQ(IntOps::byteSwap32(uint32_t(0x01020304)), uint32_t(0x04030201));

  INFO("Testing bl::IntOps::byteSwap64()");
  EXPECT_EQ(IntOps::byteSwap64(uint64_t(0x0102030405060708)), uint64_t(0x0807060504030201));
}

UNIT(support_intops_clamp, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  INFO("Testing bl::IntOps::clamp()");
  {
    EXPECT_EQ(bl_clamp<int>(-1, 100, 1000), 100);
    EXPECT_EQ(bl_clamp<int>(99, 100, 1000), 100);
    EXPECT_EQ(bl_clamp<int>(1044, 100, 1000), 1000);
    EXPECT_EQ(bl_clamp<double>(-1.0, 100.0, 1000.0), 100.0);
    EXPECT_EQ(bl_clamp<double>(99.0, 100.0, 1000.0), 100.0);
    EXPECT_EQ(bl_clamp<double>(1044.0, 100.0, 1000.0), 1000.0);
  }

  INFO("Testing bl::IntOps::clamp_to_byte()");
  {
    EXPECT_EQ(IntOps::clamp_to_byte(-1), 0u);
    EXPECT_EQ(IntOps::clamp_to_byte(42), 42u);
    EXPECT_EQ(IntOps::clamp_to_byte(255), 0xFFu);
    EXPECT_EQ(IntOps::clamp_to_byte(256), 0xFFu);
    EXPECT_EQ(IntOps::clamp_to_byte(0x7FFFFFFF), 0xFFu);
    EXPECT_EQ(IntOps::clamp_to_byte(0x7FFFFFFFu), 0xFFu);
    EXPECT_EQ(IntOps::clamp_to_byte(0xFFFFFFFFu), 0xFFu);
  }

  INFO("Testing bl::IntOps::clamp_to_word()");
  {
    EXPECT_EQ(IntOps::clamp_to_word(-1), 0u);
    EXPECT_EQ(IntOps::clamp_to_word(42), 42u);
    EXPECT_EQ(IntOps::clamp_to_word(0xFFFF), 0xFFFFu);
    EXPECT_EQ(IntOps::clamp_to_word(0x10000), 0xFFFFu);
    EXPECT_EQ(IntOps::clamp_to_word(0x10000u), 0xFFFFu);
    EXPECT_EQ(IntOps::clamp_to_word(0x7FFFFFFF), 0xFFFFu);
    EXPECT_EQ(IntOps::clamp_to_word(0x7FFFFFFFu), 0xFFFFu);
    EXPECT_EQ(IntOps::clamp_to_word(0xFFFFFFFFu), 0xFFFFu);
  }
}

UNIT(support_intops_popcnt, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  INFO("Testing bl::IntOps::PopCounter<uint32_t>");

  static const uint32_t bit_word_data[] = {
    0xFFFFFFFF, 0x11881111, 0x10000000, 0x08000000,
    0x00000001, 0x00000008, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFEFEFEFE, 0xCCCCCCCC, 0xBACFE1D9, 0x11100111,
    0x12DFEAAA, 0xFE1290AA, 0xF1018021, 0x00000000,
    0x23467111, 0x11111111, 0x137F137F
  };

  for (uint32_t i = 1; i < BL_ARRAY_SIZE(bit_word_data); i++) {
    IntOps::PopCounterSimple<uint32_t> simple;
    IntOps::PopCounterHarleySeal<uint32_t> harley_seal;

    simple.add_array(bit_word_data, i);
    harley_seal.add_array(bit_word_data, i);

    EXPECT_EQ(simple.get(), harley_seal.get());
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
