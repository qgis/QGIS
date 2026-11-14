// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/random_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/traits_p.h>

// bl::Math - Tests
// ================

namespace bl {
namespace Tests {

UNIT(math, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  constexpr uint32_t kRoundRandomCount = 1000000;

  const float round_ints_f32[] = {
    -4503599627370496.0f,
    -274877906944.0f,
    -8589934592.0f,
    -536870912.0f,
    -134217728.0f,
    -8388608.0f,
    -8388607.0f,
    -7066973.0f,
    -7066972.0f,
    -6066973.0f,
    -6066972.0f,
    -60672.0f,
    -60673.0f,
    -1001.0f,
    -100.0f,
    -10.5f,
    -2.5f,
    -1.5f,
    -1.3f,
    -1.13f,
    -1.0f,
    -0.9f,
    -0.5f,
    -0.1f,
    0.0f,
    0.1f,
    0.5f,
    0.9f,
    1.0f,
    1.13f,
    1.3f,
    1.5f,
    2.5f,
    10.5f,
    100.0f,
    1001.0f,
    60672.0f,
    60673.0f,
    6066972.0f,
    6066973.0f,
    7066972.0f,
    7066973.0f,
    8388607.0f,
    8388608.0f,
    134217728.0f,
    536870912.0f,
    8589934592.0f,
    274877906944.0f,
    4503599627370496.0f
  };

  const double round_ints_f64[] = {
    -4503599627370496.0,
    -4503599627370491.1,
    -2251799813685248.0,
    -2251799813685247.0,
    -2251799813685246.0,
    -274877906944.1,
    -274877906944.0,
    -8589934592.3,
    -8589934592.0,
    -536870913.5,
    -536870912.0,
    -134217727.5,
    -134217728.0,
    -8388608.0,
    -8388607.0,
    -7066973.0,
    -7066972.0,
    -6066973.0,
    -6066972.0,
    -60672.0,
    -60673.0,
    -1001.0,
    -100.0,
    -1.0,
    0.0,
    1.0,
    100.0,
    1001.0,
    60672.0,
    60673.0,
    6066972.0,
    6066973.0,
    7066972.0,
    7066973.0,
    8388607.0,
    8388608.0,
    8388608.3,
    134217728.0,
    536870912.0,
    536870913.44,
    8589934592.0,
    8589934592.99,
    274877906944.0,
    274877906944.1,
    2251799813685246.0,
    2251799813685247.0,
    2251799813685248.0,
    3390239813685248.0,
    3693847462732321.0,
    3693847462732322.0,
    3893847462732319.0,
    3993847462732321.0,
    3993847462732322.0,
    4193847462732321.0,
    4193847462732322.0,
    4393847462732321.0,
    4393847462732322.0,
    4493847462732321.0,
    4493847462732322.0,
    4503599627370491.1,
    4503599627370496.0,
    8.4499309581281154e+50
  };

  BLRandom rnd(0x123456789ABCDEF);

  auto rnd_f32 = [&]() -> float {
    float sign = rnd.next_double() < 0.5 ? 1.0f : -1.0f;
    return float(rnd.next_double()) * float(1e25) * sign;
  };

  auto rnd_f64 = [&]() -> double {
    double sign = rnd.next_double() < 0.5 ? 1.0 : -1.0;
    return rnd.next_double() * double(1e52) * sign;
  };

  INFO("Testing floating point rounding - trunc(float32)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      float x = i < BL_ARRAY_SIZE(round_ints_f32) ? round_ints_f32[i] : rnd_f32();
      float a = Math::trunc(x);
      float b = ::truncf(x);
      EXPECT_EQ(a, b).message("Failed to trunc float32(%0.20f) %0.20f (Math::trunc) != %0.20f (std::trunc)", double(x), double(a), double(b));
    }
  }

  INFO("Testing floating point rounding - trunc(float64)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      double x = i < BL_ARRAY_SIZE(round_ints_f64) ? round_ints_f64[i] : rnd_f64();
      double a = Math::trunc(x);
      double b = ::trunc(x);
      EXPECT_EQ(a, b).message("Failed to trunc float64(%0.20f) %0.20f (Math::trunc) != %0.20f (std::trunc)", double(x), double(a), double(b));
    }
  }

  INFO("Testing floating point rounding - floor(float32)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      float x = i < BL_ARRAY_SIZE(round_ints_f32) ? round_ints_f32[i] : rnd_f32();
      float a = Math::floor(x);
      float b = ::floorf(x);
      EXPECT_EQ(a, b).message("Failed to floor float32(%0.20f) %0.20f (Math::floor) != %0.20f (std::floor)", double(x), double(a), double(b));
    }
  }

  INFO("Testing floating point rounding - floor(float64)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      double x = i < BL_ARRAY_SIZE(round_ints_f64) ? round_ints_f64[i] : rnd_f64();
      double a = Math::floor(x);
      double b = ::floor(x);
      EXPECT_EQ(a, b).message("Failed to floor float64(%0.20f) %0.20f (Math::floor) != %0.20f (std::floor)", double(x), double(a), double(b));
    }
  }

  INFO("Testing floating point rounding - ceil(float32)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      float x = i < BL_ARRAY_SIZE(round_ints_f32) ? round_ints_f32[i] : rnd_f32();
      float a = Math::ceil(x);
      float b = ::ceilf(x);
      EXPECT_EQ(a, b).message("Failed to ceil float32(%0.20f) %0.20f (Math::ceil) != %0.20f (std::ceil)", double(x), double(a), double(b));
    }
  }

  INFO("Testing floating point rounding - ceil(float64)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      double x = i < BL_ARRAY_SIZE(round_ints_f64) ? round_ints_f64[i] : rnd_f64();
      double a = Math::ceil(x);
      double b = ::ceil(x);
      EXPECT_EQ(a, b).message("Failed to ceil float64(%0.20f) %0.20f (Math::ceil) != %0.20f (std::ceil)", double(x), double(a), double(b));
    }
  }

  INFO("Testing floating point rounding - nearby(float32)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      float x = i < BL_ARRAY_SIZE(round_ints_f32) ? round_ints_f32[i] : rnd_f32();
      float a = Math::nearby(x);
      float b = ::nearbyintf(x);
      EXPECT_EQ(a, b).message("Failed to nearby float32(%0.20f) %0.20f (Math::nearby) != %0.20f (std::nearbyint)", double(x), double(a), double(b));
    }
  }

  INFO("Testing floating point rounding - nearby(float64)");
  {
    for (uint32_t i = 0; i < kRoundRandomCount; i++) {
      double x = i < BL_ARRAY_SIZE(round_ints_f64) ? round_ints_f64[i] : rnd_f64();
      double a = Math::nearby(x);
      double b = ::nearbyint(x);
      EXPECT_EQ(a, b).message("Failed to nearby float64(%0.20f) %0.20f (Math::nearby) != %0.20f (std::nearbyint)", double(x), double(a), double(b));
    }
  }

  INFO("bl::Math::floor_to_int()");
  {
    EXPECT_EQ(Math::floor_to_int(-1.5f),-2);
    EXPECT_EQ(Math::floor_to_int(-1.5 ),-2);
    EXPECT_EQ(Math::floor_to_int(-0.9f),-1);
    EXPECT_EQ(Math::floor_to_int(-0.9 ),-1);
    EXPECT_EQ(Math::floor_to_int(-0.5f),-1);
    EXPECT_EQ(Math::floor_to_int(-0.5 ),-1);
    EXPECT_EQ(Math::floor_to_int(-0.1f),-1);
    EXPECT_EQ(Math::floor_to_int(-0.1 ),-1);
    EXPECT_EQ(Math::floor_to_int( 0.0f), 0);
    EXPECT_EQ(Math::floor_to_int( 0.0 ), 0);
    EXPECT_EQ(Math::floor_to_int( 0.1f), 0);
    EXPECT_EQ(Math::floor_to_int( 0.1 ), 0);
    EXPECT_EQ(Math::floor_to_int( 0.5f), 0);
    EXPECT_EQ(Math::floor_to_int( 0.5 ), 0);
    EXPECT_EQ(Math::floor_to_int( 0.9f), 0);
    EXPECT_EQ(Math::floor_to_int( 0.9 ), 0);
    EXPECT_EQ(Math::floor_to_int( 1.5f), 1);
    EXPECT_EQ(Math::floor_to_int( 1.5 ), 1);
  }

  INFO("bl::Math::ceil_to_int()");
  {
    EXPECT_EQ(Math::ceil_to_int(-1.5f),-1);
    EXPECT_EQ(Math::ceil_to_int(-1.5 ),-1);
    EXPECT_EQ(Math::ceil_to_int(-0.9f), 0);
    EXPECT_EQ(Math::ceil_to_int(-0.9 ), 0);
    EXPECT_EQ(Math::ceil_to_int(-0.5f), 0);
    EXPECT_EQ(Math::ceil_to_int(-0.5 ), 0);
    EXPECT_EQ(Math::ceil_to_int(-0.1f), 0);
    EXPECT_EQ(Math::ceil_to_int(-0.1 ), 0);
    EXPECT_EQ(Math::ceil_to_int( 0.0f), 0);
    EXPECT_EQ(Math::ceil_to_int( 0.0 ), 0);
    EXPECT_EQ(Math::ceil_to_int( 0.1f), 1);
    EXPECT_EQ(Math::ceil_to_int( 0.1 ), 1);
    EXPECT_EQ(Math::ceil_to_int( 0.5f), 1);
    EXPECT_EQ(Math::ceil_to_int( 0.5 ), 1);
    EXPECT_EQ(Math::ceil_to_int( 0.9f), 1);
    EXPECT_EQ(Math::ceil_to_int( 0.9 ), 1);
    EXPECT_EQ(Math::ceil_to_int( 1.5f), 2);
    EXPECT_EQ(Math::ceil_to_int( 1.5 ), 2);
  }

  INFO("bl::Math::trunc_to_int()");
  {
    EXPECT_EQ(Math::trunc_to_int(-1.5f),-1);
    EXPECT_EQ(Math::trunc_to_int(-1.5 ),-1);
    EXPECT_EQ(Math::trunc_to_int(-0.9f), 0);
    EXPECT_EQ(Math::trunc_to_int(-0.9 ), 0);
    EXPECT_EQ(Math::trunc_to_int(-0.5f), 0);
    EXPECT_EQ(Math::trunc_to_int(-0.5 ), 0);
    EXPECT_EQ(Math::trunc_to_int(-0.1f), 0);
    EXPECT_EQ(Math::trunc_to_int(-0.1 ), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.0f), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.0 ), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.1f), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.1 ), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.5f), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.5 ), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.9f), 0);
    EXPECT_EQ(Math::trunc_to_int( 0.9 ), 0);
    EXPECT_EQ(Math::trunc_to_int( 1.5f), 1);
    EXPECT_EQ(Math::trunc_to_int( 1.5 ), 1);
  }

  INFO("bl::Math::round_to_int()");
  {
    EXPECT_EQ(Math::round_to_int(-1.5f),-1);
    EXPECT_EQ(Math::round_to_int(-1.5 ),-1);
    EXPECT_EQ(Math::round_to_int(-0.9f),-1);
    EXPECT_EQ(Math::round_to_int(-0.9 ),-1);
    EXPECT_EQ(Math::round_to_int(-0.5f), 0);
    EXPECT_EQ(Math::round_to_int(-0.5 ), 0);
    EXPECT_EQ(Math::round_to_int(-0.1f), 0);
    EXPECT_EQ(Math::round_to_int(-0.1 ), 0);
    EXPECT_EQ(Math::round_to_int( 0.0f), 0);
    EXPECT_EQ(Math::round_to_int( 0.0 ), 0);
    EXPECT_EQ(Math::round_to_int( 0.1f), 0);
    EXPECT_EQ(Math::round_to_int( 0.1 ), 0);
    EXPECT_EQ(Math::round_to_int( 0.5f), 1);
    EXPECT_EQ(Math::round_to_int( 0.5 ), 1);
    EXPECT_EQ(Math::round_to_int( 0.9f), 1);
    EXPECT_EQ(Math::round_to_int( 0.9 ), 1);
    EXPECT_EQ(Math::round_to_int( 1.5f), 2);
    EXPECT_EQ(Math::round_to_int( 1.5 ), 2);
  }

  INFO("bl::Math::frac()");
  {
    EXPECT_EQ(Math::frac( 0.00f), 0.00f);
    EXPECT_EQ(Math::frac( 0.00 ), 0.00 );
    EXPECT_EQ(Math::frac( 1.00f), 0.00f);
    EXPECT_EQ(Math::frac( 1.00 ), 0.00 );
    EXPECT_EQ(Math::frac( 1.25f), 0.25f);
    EXPECT_EQ(Math::frac( 1.25 ), 0.25 );
    EXPECT_EQ(Math::frac( 1.75f), 0.75f);
    EXPECT_EQ(Math::frac( 1.75 ), 0.75 );
    EXPECT_EQ(Math::frac(-1.00f), 0.00f);
    EXPECT_EQ(Math::frac(-1.00 ), 0.00 );
    EXPECT_EQ(Math::frac(-1.25f), 0.75f);
    EXPECT_EQ(Math::frac(-1.25 ), 0.75 );
    EXPECT_EQ(Math::frac(-1.75f), 0.25f);
    EXPECT_EQ(Math::frac(-1.75 ), 0.25 );
  }

  INFO("bl::Math::is_between_0_and_1()");
  {
    EXPECT_TRUE(Math::is_between_0_and_1( 0.0f  ));
    EXPECT_TRUE(Math::is_between_0_and_1( 0.0   ));
    EXPECT_TRUE(Math::is_between_0_and_1( 0.5f  ));
    EXPECT_TRUE(Math::is_between_0_and_1( 0.5   ));
    EXPECT_TRUE(Math::is_between_0_and_1( 1.0f  ));
    EXPECT_TRUE(Math::is_between_0_and_1( 1.0   ));
    EXPECT_TRUE(Math::is_between_0_and_1(-0.0f  ));
    EXPECT_TRUE(Math::is_between_0_and_1(-0.0   ));
    EXPECT_FALSE(Math::is_between_0_and_1(-1.0f  ));
    EXPECT_FALSE(Math::is_between_0_and_1(-1.0   ));
    EXPECT_FALSE(Math::is_between_0_and_1( 1.001f));
    EXPECT_FALSE(Math::is_between_0_and_1( 1.001 ));
  }

  INFO("bl::Math::quad_roots");
  {
    size_t count;
    double roots[2];

    // x^2 + 4x + 4 == 0
    count = Math::quad_roots(roots, 1.0, 4.0, 4.0, Traits::min_value<double>(), Traits::max_value<double>());

    EXPECT_EQ(count, 1u);
    EXPECT_EQ(roots[0], -2.0);

    // -4x^2 + 8x + 12 == 0
    count = Math::quad_roots(roots, -4.0, 8.0, 12.0, Traits::min_value<double>(), Traits::max_value<double>());

    EXPECT_EQ(count, 2u);
    EXPECT_EQ(roots[0], -1.0);
    EXPECT_EQ(roots[1],  3.0);
  }

  INFO("bl::Math::cubic_roots");
  {
    size_t count;
    double roots[2];

    // 64x^3 - 64 == 1
    count = Math::cubic_roots(roots, 64, 0.0, 0.0, -64, Traits::min_value<double>(), Traits::max_value<double>());

    EXPECT_EQ(count, 1u);
    EXPECT_EQ(roots[0], 1.0);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
