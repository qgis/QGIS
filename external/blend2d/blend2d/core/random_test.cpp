// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/random_p.h>
#include <blend2d/simd/simd_p.h>

// bl::Random - Tests
// ==================

namespace bl {
namespace Tests {

UNIT(random, BL_TEST_GROUP_CORE_UTILITIES) {
  // Number of iterations for tests that use loop.
  static constexpr uint32_t kCount = 1000000;

  #if defined(BL_TARGET_OPT_SSE2)
  INFO("Testing whether the SIMD implementation matches the C++ one");
  {
    BLRandom a(0);
    BLRandom b(0);

    SIMD::Vec2xU64 b_lo = RandomInternal::next_uint64AsI128(&b);
    SIMD::Vec2xU64 b_hi = SIMD::swizzle_u32<2, 3, 0, 1>(b_lo);

    uint64_t a_val = a.next_uint64();
    uint64_t b_val = (uint64_t(SIMD::cast_to_i32(b_lo))      ) +
                    (uint64_t(SIMD::cast_to_i32(b_hi)) << 32) ;

    EXPECT_EQ(a_val, b_val);
  }
  #endif

  INFO("Testing whether the random 32-bit integer is the HI part of the 64-bit one");
  {
    BLRandom a(0);
    BLRandom b(0);
    EXPECT_EQ(uint32_t(a.next_uint64() >> 32), b.next_uint32());
  }

  INFO("Test whether returned double precision values satisfy [0..1) condition");
  {
    // Supply a low-entropy seed on purpose.
    BLRandom rnd(3);

    uint32_t below = 0;
    uint32_t above = 0;

    for (uint32_t i = 0; i < kCount; i++) {
      double x = rnd.next_double();
      below += x <  0.5;
      above += x >= 0.5;
      EXPECT_GE(x, 0.0);
      EXPECT_LT(x, 1.0);
    }
    INFO("  Random numbers at [0.0, 0.5): %u of %u", below, kCount);
    INFO("  Random numbers at [0.5, 1.0): %u of %u", above, kCount);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
